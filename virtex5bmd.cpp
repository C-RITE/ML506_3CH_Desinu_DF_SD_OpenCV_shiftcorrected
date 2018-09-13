#include "stdafx.h"

#if defined (__KERNEL__)
    #include "include/kpstdlib.h"
#endif 
#include "include/wdc_defs.h"
#include "include/utils.h"
#include "include/status_strings.h"

#include "virtex5bmd.h"

#define VIRTEX5_DEFAULT_LICENSE_STRING "6C3CC2CFE89E7AD042444C655646F21A0BF3D23E.Montana State University"
//"6f1eafddeade6025f0620c070c601c684c5341b7ce19f2.Montana State University"
#define VIRTEX5_DEFAULT_DRIVER_NAME "windrvr6"

void DLLCALLCONV VIRTEX5_IntHandler(PVOID pData);

//extern BOOL           g_bSOF;
extern DWORD          g_dwTotalCount;

//extern BOOL           g_bFirstVsync;
extern VIDEO_INFO     g_VideoInfo;

extern FILE          *g_fp;
extern LARGE_INTEGER  g_ticksPerSecond;
extern HANDLE         g_EventEOF;
//extern POINT          g_stimulus;
extern CVirtex5BMD       g_objVirtex5BMD;
extern WDC_DEVICE_HANDLE g_hDevVirtex5;
extern POINT             g_stimulus;

extern BOOL              g_bGrabStart;


extern void g_GetAppSystemTime(int *hours, int *minutes, int *seconds, double *milliseconds);

BOOL g_bKernelPlugin;

void memory_copy(PVOID mem_i, unsigned char *mem_o, int len, int LineOffset) {
	int     i, buffSize, lineOffset;
	static int prevOffset;

	if (LineOffset >= 0 && LineOffset <= g_VideoInfo.img_height - g_VideoInfo.line_spacing) {
		buffSize   = g_VideoInfo.img_width*g_VideoInfo.line_spacing*g_VideoInfo.nVideoTabs*g_VideoInfo.nVideoChannels;
		lineOffset = LineOffset*g_VideoInfo.img_width*g_VideoInfo.nVideoTabs*g_VideoInfo.nVideoChannels;
		for (i = 0; i < buffSize; i ++) {
			*(mem_o+i+lineOffset) = ((unsigned char *)(mem_i))[i];
		}
	}

	prevOffset = LineOffset;
}

/* -----------------------------------------------
    Interrupts
   ----------------------------------------------- */
void DLLCALLCONV VIRTEX5_IntHandler(PVOID pData)
{
	UINT32  intID, lineID, StartAddrIndex, NextLineIndex, hiAddr, loAddr, int_flag;

	int       hours;
	int       minutes;
	int       seconds;
	double    milliseconds, tcur;
	int       x1, x2;
	int       i, j, k, m, dwTotalCount, buffSize;

	static    UINT32  lineIDold, frameID;
	static    BOOL  bSOF;
	static    double told;

	g_GetAppSystemTime(&hours, &minutes, &seconds, &milliseconds);

    PWDC_DEVICE pDev = (PWDC_DEVICE)pData;
    PVIRTEX5_DEV_CTX pDevCtx = (PVIRTEX5_DEV_CTX)WDC_GetDevContext(pDev);

	WDC_ReadAddr32((WDC_DEVICE_HANDLE)pDev, VIRTEX5_SPACE, VIRTEX5_LINECTRL_OFFSET, &intID);
	int_flag = intID >> 28;
	int_flag = int_flag << 30;
	int_flag = int_flag >> 30;	// keep the lower two bits for telling interrupt types 
								// 0: normal, 1: makeup, 2: SOF, 3: EOF
	lineID = intID << 4;
	lineID = lineID >> 20;

	tcur = 1000*(hours*3600+minutes*60+seconds) + milliseconds;
//	if ((tcur-told) > 20.0f) fprintf(g_fp, "\t\t\t\t\tframe lost: %f\n", tcur-told);
	told = tcur;

//	fprintf(g_fp, "%02d:%02d:%02d:%5.3f   Line: %03d/%d, intID = %d\n", hours, minutes, seconds, milliseconds, lineID, lineID>>4, int_flag);

	if (int_flag == 0 || int_flag == 2) {
		if (lineID == lineIDold) return;
//		if (lineIDold != lineID-16 && lineID != 16) fprintf(g_fp, "  !!!  Line: %d lost\n", lineID-16);
		lineIDold = lineID;
//		fprintf(g_fp, "%02d:%02d:%02d:%5.3f   Line: %03d   intID = %d\n", hours, minutes, seconds, milliseconds, lineID, int_flag);
		bSOF = FALSE;
	} else if (int_flag == 1) {
//		fprintf(g_fp, "%02d:%02d:%02d:%5.3f   SOF\n", hours, minutes, seconds, milliseconds);
		if (bSOF == TRUE) return;
		bSOF = TRUE;
		return;
	} else {
		//fprintf(g_fp, "%02d:%02d:%02d:%5.3f   Unknow interrupt\n", hours, minutes, seconds, milliseconds);
		return;
	}

	memory_copy(pDevCtx->pBuf, g_VideoInfo.video_in, g_dwTotalCount*sizeof(UINT32), lineID-g_VideoInfo.line_spacing);

	// update FPGA register
	if (!g_bKernelPlugin) {
		WDC_ReadAddr32((WDC_DEVICE_HANDLE)pDev, VIRTEX5_SPACE, VIRTEX5_IMAGEOS_OFFSET, &NextLineIndex);

		if (lineID >= g_VideoInfo.img_height) {
			NextLineIndex  = NextLineIndex >> 12;
			NextLineIndex  = NextLineIndex << 12;
		}
		NextLineIndex  += g_VideoInfo.line_spacing;
		
		WDC_WriteAddr32((WDC_DEVICE_HANDLE)pDev, VIRTEX5_SPACE, VIRTEX5_IMAGEOS_OFFSET, NextLineIndex);

		NextLineIndex = NextLineIndex << 20;
		NextLineIndex = NextLineIndex >> 20;

		g_GetAppSystemTime(&hours, &minutes, &seconds, &milliseconds);
//		fprintf(g_fp, "%02d:%02d:%02d:%5.3f   NextLineID = %d\n", hours, minutes, seconds, milliseconds, NextLineIndex);
	}

    // Execute the diagnostics application's interrupt handler routine 
	if (lineID == g_VideoInfo.img_height) {
		frameID ++;
//		fprintf(g_fp, "========================= %d\n", frameID);
		//pDevCtx->funcDiagIntHandler((WDC_DEVICE_HANDLE)pDev, &intResult);
		buffSize = g_VideoInfo.img_width*g_VideoInfo.img_height*g_VideoInfo.nVideoTabs*g_VideoInfo.nVideoChannels;
		memcpy(g_VideoInfo.video_out, g_VideoInfo.video_in, buffSize);
//		std::ofstream myFile ("c:\\data.bin", std::ios::out | std::ios::binary);
//		myFile.write ((const char*)g_VideoInfo.video_out, buffSize);
//		myFile.close();
//		ZeroMemory(g_VideoInfo.video_in, buffSize);
		g_bGrabStart?SetEvent(g_EventEOF):0;
	}
}



BOOL DeviceValidate(PWDC_DEVICE pDev)
{
    DWORD i, dwNumAddrSpaces = pDev->dwNumAddrSpaces;

    // Verify that the device has at least one active address space 
    for (i = 0; i < dwNumAddrSpaces; i++)
    {
        if (WDC_AddrSpaceIsActive(pDev, i))
            return TRUE;
    }
    
    return FALSE;
}

BOOL IsValidDevice(PWDC_DEVICE pDev, const CHAR *sFunc)
{
    if (!pDev || !WDC_GetDevContext(pDev))
    {
        return FALSE;
    }

    return TRUE;
}



CVirtex5BMD::CVirtex5BMD() {
	m_errMsg          = new char [256];
	m_errCode         = WD_STATUS_SUCCESS;
//	g_nInterruptID    = 0;
//	g_nInterruptIDOld = 0;
}
CVirtex5BMD::~CVirtex5BMD() {
	//delete [] m_errMsg;
}

/*************************************************************
  Functions implementation
 *************************************************************/
/* -----------------------------------------------
    VIRTEX5 and WDC library initialize/uninit
   ----------------------------------------------- */
DWORD CVirtex5BMD::VIRTEX5_LibInit(void)
{
    DWORD dwStatus;
 
    // Set WDC library's debug options
    // (default: level TRACE, output to Debug Monitor) 
    dwStatus = WDC_SetDebugOptions(WDC_DBG_DEFAULT, NULL);
//	dwStatus = WDC_SetDebugOptions(WDC_DBG_DBM_TRACE, "kp_dbg.txt");

    if (WD_STATUS_SUCCESS != dwStatus)
    {
        sprintf_s(m_errMsg, MSG_LENGTH, "Failed to initialize debug options for WDC library.\n"
						  "Error 0x%lx - %s\n", dwStatus, Stat2Str(dwStatus));
        m_errCode = dwStatus;

        return dwStatus;
    }

    // Open a handle to the driver and initialize the WDC library 
    dwStatus = WDC_DriverOpen(WDC_DRV_OPEN_DEFAULT,
        VIRTEX5_DEFAULT_LICENSE_STRING);
    if (WD_STATUS_SUCCESS != dwStatus)
    {
        sprintf_s(m_errMsg, MSG_LENGTH, "Failed to initialize the WDC library. Error 0x%lx - %s\n",
						  dwStatus, Stat2Str(dwStatus));
		m_errCode = dwStatus;
        
        return dwStatus;
    }

    return WD_STATUS_SUCCESS;
}


char * CVirtex5BMD::GetErrMsg() {
	return m_errMsg;
}

DWORD CVirtex5BMD::GetErrCode() {
	return m_errCode;
}


// Find and open a VIRTEX5 device 
WDC_DEVICE_HANDLE CVirtex5BMD::DeviceFindAndOpen(DWORD dwVendorId, DWORD dwDeviceId)
{
    WD_PCI_SLOT slot;
    
    if (!DeviceFind(dwVendorId, dwDeviceId, &slot))
        return NULL;

    return DeviceOpen(&slot);
}

// Find a VIRTEX5 device 
BOOL CVirtex5BMD::DeviceFind(DWORD dwVendorId, DWORD dwDeviceId, WD_PCI_SLOT *pSlot)
{
    DWORD dwStatus;
    //DWORD i, dwNumDevices;
	DWORD dwNumDevices;
    WDC_PCI_SCAN_RESULT scanResult;

    BZERO(scanResult);
    dwStatus = WDC_PciScanDevices(dwVendorId, dwDeviceId, &scanResult);
    if (WD_STATUS_SUCCESS != dwStatus)
    {
        sprintf_s(m_errMsg, MSG_LENGTH, "DeviceFind: Failed scanning the PCI bus.\n"
            "Error: 0x%lx - %s\n", dwStatus, Stat2Str(dwStatus));
		m_errCode = dwStatus;

        return FALSE;
    }

    dwNumDevices = scanResult.dwNumDevices;
    if (!dwNumDevices)
    {
        sprintf_s(m_errMsg, MSG_LENGTH, "No matching device was found for search criteria "
            "(Vendor ID 0x%lX, Device ID 0x%lX)\n", dwVendorId, dwDeviceId);
		m_errCode = dwStatus;

        return FALSE;
    }
    
    if (dwNumDevices > 1)
    {
		// found multiple devices
    }

    *pSlot = scanResult.deviceSlot[0];

    return TRUE;
}

// Open a handle to a VIRTEX5 device 
WDC_DEVICE_HANDLE CVirtex5BMD::DeviceOpen(const WD_PCI_SLOT *pSlot)
{
    WDC_DEVICE_HANDLE hDev;
    DWORD dwStatus;
    WD_PCI_CARD_INFO deviceInfo;
    
    // Retrieve the device's resources information 
    BZERO(deviceInfo);
    deviceInfo.pciSlot = *pSlot;
    dwStatus = WDC_PciGetDeviceInfo(&deviceInfo);
    if (WD_STATUS_SUCCESS != dwStatus)
    {
        sprintf_s(m_errMsg, MSG_LENGTH, "DeviceOpen: Failed retrieving the device's resources "
            "information.\nError 0x%lx - %s\n", dwStatus, Stat2Str(dwStatus));
		m_errCode = dwStatus;

        return NULL;
    }

//	deviceInfo.Card.Item[0].dwOptions = WD_ITEM_DO_NOT_MAP_KERNEL;

    // Open a handle to the device 
    hDev = VIRTEX5_DeviceOpen(&deviceInfo);
    if (!hDev)
    {
        return NULL;
    }

    return hDev;
}

// Close handle to a VIRTEX5 device 
void CVirtex5BMD::DeviceClose(WDC_DEVICE_HANDLE hDev, PDIAG_DMA pDma)          
{
    if (!hDev)
        return;

     if (pDma)   
         DIAG_DMAClose(hDev, pDma);      
         
    if (!VIRTEX5_DeviceClose(hDev))
    {
        sprintf_s(m_errMsg, MSG_LENGTH, "DeviceClose: Failed closing VIRTEX5 device");
    }
}


/* -----------------------------------------------
    Device open/close
   ----------------------------------------------- */
WDC_DEVICE_HANDLE CVirtex5BMD::VIRTEX5_DeviceOpen(const WD_PCI_CARD_INFO *pDeviceInfo)
{
    DWORD dwStatus;
    PVIRTEX5_DEV_CTX pDevCtx = NULL;
    WDC_DEVICE_HANDLE hDev = NULL;

    // Validate arguments 
    if (!pDeviceInfo)
    {
        sprintf_s(m_errMsg, MSG_LENGTH, "VIRTEX5_DeviceOpen: Error - NULL device information struct. pointer\n");
        return NULL;
    }

    // Allocate memory for the VIRTEX5 device context 
    pDevCtx = (PVIRTEX5_DEV_CTX)calloc(1, sizeof(VIRTEX5_DEV_CTX));
    if (!pDevCtx)
    {
        sprintf_s(m_errMsg, MSG_LENGTH, "Failed allocating memory for VIRTEX5 device context\n");
        return NULL;
    }

    // Open a Kernel PlugIn WDC device handle 
	g_bKernelPlugin = TRUE;
#if _DEBUG
	g_bKernelPlugin = FALSE;
	dwStatus = -1;
#else
    dwStatus = WDC_PciDeviceOpen(&hDev, pDeviceInfo, pDevCtx, NULL, KP_VRTX5_DRIVER_NAME, &hDev);
#endif
    
    // if failed, try opening a WDC device handle without using Kernel PlugIn 
    if(dwStatus != WD_STATUS_SUCCESS)
    {
		g_bKernelPlugin = FALSE;
        sprintf_s(m_errMsg, MSG_LENGTH, "Error 0x%lx - %s\n", dwStatus, Stat2Str(dwStatus));

        dwStatus = WDC_PciDeviceOpen(&hDev, pDeviceInfo, pDevCtx, NULL, NULL, NULL);
    }
    if (WD_STATUS_SUCCESS != dwStatus)
    {
        sprintf_s(m_errMsg, MSG_LENGTH, "Failed opening a WDC device handle. Error 0x%lx - %s\n",
            dwStatus, Stat2Str(dwStatus));
		m_errCode = dwStatus;

        goto Error;
    }

    // Validate device information 
    if (!DeviceValidate((PWDC_DEVICE)hDev)) {
		sprintf_s(m_errMsg, MSG_LENGTH, "Device does not have any active memory or I/O address spaces\n");
        goto Error;
	}

    // Return handle to the new device 
/*    TraceLog("VIRTEX5_DeviceOpen: Opened a VIRTEX5 device (handle 0x%p)\n"
        "Device is %s using a Kernel PlugIn driver (%s)\n", hDev,
        (WDC_IS_KP(hDev))? "" : "not" , KP_VRTX5_DRIVER_NAME);
*/
    return hDev;

Error:    
    if (hDev)
        VIRTEX5_DeviceClose(hDev);
    else
        free(pDevCtx);
    
    return NULL;
}

BOOL CVirtex5BMD::VIRTEX5_DeviceClose(WDC_DEVICE_HANDLE hDev)
{
    DWORD dwStatus;
    PWDC_DEVICE pDev = (PWDC_DEVICE)hDev;
    PVIRTEX5_DEV_CTX pDevCtx;
    
    //TraceLog("VIRTEX5_DeviceClose entered. Device handle: 0x%p\n", hDev);

    if (!hDev)
    {
        sprintf_s(m_errMsg, MSG_LENGTH, "VIRTEX5_DeviceClose: Error - NULL device handle\n");
        return FALSE;
    }

    pDevCtx = (PVIRTEX5_DEV_CTX)WDC_GetDevContext(pDev);
    
    // Disable interrupts 
    if (WDC_IntIsEnabled(hDev))
    {
        dwStatus = VIRTEX5_IntDisable(hDev);
        if (WD_STATUS_SUCCESS != dwStatus)
        {
            sprintf_s(m_errMsg, MSG_LENGTH, "Failed disabling interrupts. Error 0x%lx - %s\n", dwStatus,
                Stat2Str(dwStatus));
			m_errCode = dwStatus;
        }
    }

    // Close the device 
    dwStatus = WDC_PciDeviceClose(hDev);
    if (WD_STATUS_SUCCESS != dwStatus)
    {
        sprintf_s(m_errMsg, MSG_LENGTH, "Failed closing a WDC device handle (0x%p). Error 0x%lx - %s\n",
            hDev, dwStatus, Stat2Str(dwStatus));
		m_errCode = dwStatus;
    }

    // Free VIRTEX5 device context memory 
    if (pDevCtx)
        free(pDevCtx);
    
    return (WD_STATUS_SUCCESS == dwStatus);
}


DWORD CVirtex5BMD::VIRTEX5_IntDisable(WDC_DEVICE_HANDLE hDev)
{
    DWORD dwStatus;
    PWDC_DEVICE pDev = (PWDC_DEVICE)hDev;

    if (!IsValidDevice(pDev, "VIRTEX5_IntDisable")) {
		sprintf_s(m_errMsg, MSG_LENGTH, "VIRTEX5_IntDisable: NULL device %s\n", !pDev ? "handle" : "context");
        return WD_INVALID_PARAMETER;
	}

    if (!WDC_IntIsEnabled(hDev))
    {
        sprintf_s(m_errMsg, MSG_LENGTH, "Interrupts are already disabled ...\n");
		m_errCode = WD_OPERATION_ALREADY_DONE;
        return WD_OPERATION_ALREADY_DONE;
    }

    // Disable the interrupts 
    dwStatus = WDC_IntDisable(hDev);
    if (WD_STATUS_SUCCESS != dwStatus)
    {
        sprintf_s(m_errMsg, MSG_LENGTH, "Failed disabling interrupts. Error 0x%lx - %s\n",
            dwStatus, Stat2Str(dwStatus));
		m_errCode = WD_OPERATION_ALREADY_DONE;
    }

    return dwStatus;
}


void CVirtex5BMD::DIAG_DMAClose(WDC_DEVICE_HANDLE hDev, PDIAG_DMA pDma)
{
    DWORD dwStatus;

    if (!pDma)
        return;
    
    if (VIRTEX5_IntIsEnabled(hDev))
    {
        dwStatus = VIRTEX5_IntDisable(hDev);
		m_errCode = dwStatus;
    }

    if (pDma->hDma)
    {
        VIRTEX5_DMAClose(pDma->hDma);
    }
    
    BZERO(*pDma);
}


void CVirtex5BMD::VIRTEX5_DMAClose(VIRTEX5_DMA_HANDLE hDma)
{
    DWORD dwStatus = WD_STATUS_SUCCESS;
    PVIRTEX5_DEV_CTX pDevCtx;

//    TraceLog("VIRTEX5_DMAClose entered.");
    
    if (!IsValidDmaHandle(hDma, "VIRTEX5_DMAClose"))
        return;
    
//    TraceLog(" Device handle: 0x%p\n", hDma->hDev);
    
    if (hDma->pDma)
    {
        dwStatus = WDC_DMABufUnlock(hDma->pDma);
        if (WD_STATUS_SUCCESS != dwStatus)
        {
            sprintf_s(m_errMsg, MSG_LENGTH, "VIRTEX5_DMAClose: Failed unlocking DMA buffer. "
                "Error 0x%lX - %s\n", dwStatus, Stat2Str(dwStatus));
			m_errCode = dwStatus;
        }
    }
    else
    {
        //TraceLog("VIRTEX5_DMAClose: DMA is not currently open ... "
         //   "(device handle 0x%p, DMA handle 0x%p)\n", hDma->hDev, hDma);
    }
     
    pDevCtx = (PVIRTEX5_DEV_CTX)WDC_GetDevContext(hDma->hDev);
    pDevCtx->hDma = NULL;
    pDevCtx->pBuf = NULL;

    free(hDma);
}

// -----------------------------------------------
//   Direct Memory Access (DMA)
//   ----------------------------------------------- 
BOOL CVirtex5BMD::IsValidDmaHandle(VIRTEX5_DMA_HANDLE hDma, CHAR *sFunc)
{
    BOOL ret = (hDma && IsValidDevice((PWDC_DEVICE)hDma->hDev, sFunc)) ? TRUE : FALSE;

    if (!hDma)
        sprintf_s(m_errMsg, MSG_LENGTH, "%s: NULL DMA Handle\n", sFunc);

    return ret;
}


BOOL CVirtex5BMD::VIRTEX5_IntIsEnabled(WDC_DEVICE_HANDLE hDev)
{
    if (!IsValidDevice((PWDC_DEVICE)hDev, "VIRTEX5_IntIsEnabled")) {
		sprintf_s(m_errMsg, MSG_LENGTH, "VIRTEX5_IntIsEnabled: NULL device %s\n", !hDev ? "handle" : "context");
        return FALSE;
	}

    return WDC_IntIsEnabled(hDev);
}


DWORD CVirtex5BMD::VIRTEX5_LibUninit(void)
{
    DWORD dwStatus;

    // Uninit the WDC library and close the handle to WinDriver 
    dwStatus = WDC_DriverClose();
    if (WD_STATUS_SUCCESS != dwStatus)
    {
        sprintf_s(m_errMsg, MSG_LENGTH, "Failed to uninit the WDC library. Error 0x%lx - %s\n",
            dwStatus, Stat2Str(dwStatus));
    }

    return dwStatus;
}



// write/read pre-defined run-time or PCI configuration registers
BOOL CVirtex5BMD::ReadWriteReg(WDC_DEVICE_HANDLE hDev, WDC_REG *pRegs, DWORD dwReg,
								DWORD dwNumRegs, WDC_DIRECTION direction, BOOL fPciCfg,
								UINT64 *u64Rdata, UINT u64Wdata)
{
    DWORD  dwStatus; 
    WDC_REG *pReg; 
    BYTE   bData = 0;
    WORD   wData = 0;
    UINT32 u32Data = 0;
    UINT64 u64Data = 0;
    
    if (!hDev)
    {
        sprintf_s(m_errMsg, MSG_LENGTH, "WDC_DIAG_ReadWriteReg: Error - NULL WDC device handle");
        return FALSE;
    }

    if (!dwNumRegs || !pRegs)
    {
        sprintf_s(m_errMsg, MSG_LENGTH, "WDC_DIAG_ReadWriteReg: %s",
						!dwNumRegs ? "No registers to read/write (dwNumRegs == 0)" :
						"Error - NULL registers pointer");
        return FALSE;
    }

    // Read/write register 
    if (dwReg > dwNumRegs)
    {
        sprintf_s(m_errMsg, MSG_LENGTH, "Selection (%ld) is out of range (1 - %ld)", dwReg, dwNumRegs);
        return FALSE;
    }

    pReg = &pRegs[dwReg - 1];


    if ( ((WDC_READ == direction) && (WDC_WRITE == pReg->direction)) ||
        ((WDC_WRITE == direction) && (WDC_READ == pReg->direction)))
    {
        sprintf_s(m_errMsg, MSG_LENGTH, "Error - you have selected to %s a %s-only register\n",
            (WDC_READ == direction) ? "read from" : "write to",
            (WDC_WRITE == pReg->direction) ? "write" : "read");
        return FALSE;
    }
/*
    if ((WDC_WRITE == direction) &&
        !WDC_DIAG_InputWriteData((WDC_SIZE_8 == pReg->dwSize) ? (PVOID)&bData :
        (WDC_SIZE_16 == pReg->dwSize) ? (PVOID)&wData :
        (WDC_SIZE_32 == pReg->dwSize) ? (PVOID)&u32Data : (PVOID)&u64Data,
        pReg->dwSize))
    {
        goto Exit;
    }
*/
    switch (pReg->dwSize)
    {
    case WDC_SIZE_8:
        if (WDC_READ == direction) {
            dwStatus = fPciCfg ? WDC_PciReadCfg8(hDev, pReg->dwOffset, &bData) :
                WDC_ReadAddr8(hDev, pReg->dwAddrSpace, pReg->dwOffset, &bData);
			*u64Rdata = bData;
        } else {
			bData = (BYTE)u64Wdata;
            dwStatus = fPciCfg ? WDC_PciWriteCfg8(hDev, pReg->dwOffset, bData) :
                WDC_WriteAddr8(hDev, pReg->dwAddrSpace, pReg->dwOffset, bData);
		}
        break;
    case WDC_SIZE_16:
        if (WDC_READ == direction) {
            dwStatus = fPciCfg ? WDC_PciReadCfg16(hDev, pReg->dwOffset, &wData) :
                WDC_ReadAddr16(hDev, pReg->dwAddrSpace, pReg->dwOffset, &wData);
			*u64Rdata = wData;
        } else {
			wData = (WORD)u64Wdata;
            dwStatus = fPciCfg ? WDC_PciWriteCfg16(hDev, pReg->dwOffset, wData) :
                WDC_WriteAddr16(hDev, pReg->dwAddrSpace, pReg->dwOffset, wData);
		}
        break;
    case WDC_SIZE_32:
        if (WDC_READ == direction) {
            dwStatus = fPciCfg ? WDC_PciReadCfg32(hDev, pReg->dwOffset, &u32Data) :
                WDC_ReadAddr32(hDev, pReg->dwAddrSpace, pReg->dwOffset, &u32Data);
			*u64Rdata = u32Data;
        } else {
			u32Data = (UINT32)u64Wdata;
            dwStatus = fPciCfg ? WDC_PciWriteCfg32(hDev, pReg->dwOffset, u32Data) :
                WDC_WriteAddr32(hDev, pReg->dwAddrSpace, pReg->dwOffset, u32Data);
		}
        break;
    case WDC_SIZE_64:
        if (WDC_READ == direction) {
            dwStatus = fPciCfg ? WDC_PciReadCfg64(hDev, pReg->dwOffset, &u64Data) :
                WDC_ReadAddr64(hDev, pReg->dwAddrSpace, pReg->dwOffset, &u64Data);
			*u64Rdata = u64Data;
        } else {
			u64Data = u64Wdata;
            dwStatus = fPciCfg ? WDC_PciWriteCfg64(hDev, pReg->dwOffset, u64Data) :
                WDC_WriteAddr64(hDev, pReg->dwAddrSpace, pReg->dwOffset, u64Data);
		}
        break;
    default:
        sprintf_s(m_errMsg, MSG_LENGTH, "Invalid register size (%ld)\n", pReg->dwSize);
        return FALSE;
    }

    if (WD_STATUS_SUCCESS != dwStatus)
    {
        sprintf_s(m_errMsg, MSG_LENGTH, "Failed %s %s. Error 0x%lx - %s\n",
            (WDC_READ == direction) ? "reading data from" : "writing data to",
            pReg->sName, dwStatus, Stat2Str(dwStatus));
		return FALSE;
    }

	return TRUE;
}


UINT32 CVirtex5BMD::VIRTEX5_ReadReg32(WDC_DEVICE_HANDLE hDev, DWORD offset)
{
    UINT32 data;

    WDC_ReadAddr32(hDev, VIRTEX5_SPACE, offset, &data);
    return data;
}


WORD CVirtex5BMD::code2size(BYTE bCode)
{
    if (bCode > 0x05)
        return 0;
    return (WORD)(128 << bCode);
}


WORD CVirtex5BMD::DMAGetMaxPacketSize(WDC_DEVICE_HANDLE hDev, BOOL fIsRead)
{
    UINT32 dltrsstat;
    WORD wByteCount;
	PWDC_DEVICE pDev = (PWDC_DEVICE)hDev;

    //if (!IsValidDevice(hDev, "VIRTEX5_DMAGetMaxPacketSize"))
	if (!IsValidDevice(pDev, "VIRTEX5_DMAGetMaxPacketSize"))
        return 0;
    
    // Read encoded max payload sizes 
    dltrsstat = VIRTEX5_ReadReg32(hDev, VIRTEX5_DLTRSSTAT_OFFSET);
    
    // Convert encoded max payload sizes into byte count 
    if (fIsRead)
    {
        // bits 18:16 
        wByteCount = code2size((BYTE)((dltrsstat >> 16) & 0x7));
    }
    else
    {
        /* bits 2:0 */
        WORD wMaxCapPayload = code2size((BYTE)(dltrsstat & 0x7)); 
        /* bits 10:8 */
        WORD wMaxProgPayload = code2size((BYTE)((dltrsstat >> 8) & 0x7));

        wByteCount = MIN(wMaxCapPayload, wMaxProgPayload);
    }

    return wByteCount;
}


DWORD CVirtex5BMD::VIRTEX5_DMAOpen(WDC_DEVICE_HANDLE hDev, PVOID *ppBuf, DWORD dwOptions,
									DWORD dwBytes, VIRTEX5_DMA_HANDLE *ppDmaHandle)
{
    DWORD dwStatus;
    PVIRTEX5_DEV_CTX pDevCtx;
    VIRTEX5_DMA_HANDLE pVIRTEX5Dma = NULL;

    if (!IsValidDevice((PWDC_DEVICE)hDev, "VIRTEX5_DMAOpen"))
        return WD_INVALID_PARAMETER;
    
    if (!ppBuf)
    {
        sprintf_s(m_errMsg, MSG_LENGTH, "VIRTEX5_DMAOpen: Error - NULL DMA buffer pointer");
        return WD_INVALID_PARAMETER;
    }

    pDevCtx = (PVIRTEX5_DEV_CTX)WDC_GetDevContext(hDev);
    if (pDevCtx->hDma)
    {
        sprintf_s(m_errMsg, MSG_LENGTH, "VIRTEX5_DMAOpen: Error - DMA already open");
        return WD_TOO_MANY_HANDLES;
    }

    pVIRTEX5Dma = (VIRTEX5_DMA_STRUCT *)calloc(1, sizeof(VIRTEX5_DMA_STRUCT));
    if (!pVIRTEX5Dma)
    {
        sprintf_s(m_errMsg, MSG_LENGTH, "VIRTEX5_DMAOpen: Failed allocating memory for VIRTEX5 DMA struct");
        return WD_INSUFFICIENT_RESOURCES;
    }
    pVIRTEX5Dma->hDev = hDev;
    
    // Allocate and lock a DMA buffer 
    dwStatus = WDC_DMAContigBufLock(hDev, ppBuf, dwOptions, dwBytes, &pVIRTEX5Dma->pDma);

    if (WD_STATUS_SUCCESS != dwStatus) 
    {
        sprintf_s(m_errMsg, MSG_LENGTH, "VIRTEX5_DMAOpen: Failed locking a DMA buffer. Error 0x%lx", dwStatus);
        goto Error;
    }
    
    *ppDmaHandle = (VIRTEX5_DMA_HANDLE)pVIRTEX5Dma;

    // update the device context 
    pDevCtx->hDma = pVIRTEX5Dma;
    pDevCtx->pBuf = *ppBuf;
    
    return WD_STATUS_SUCCESS;

Error:
    if (pVIRTEX5Dma)
        VIRTEX5_DMAClose((VIRTEX5_DMA_HANDLE)pVIRTEX5Dma);
    
    return dwStatus;
}


void CVirtex5BMD::VIRTEX5_WriteReg32(WDC_DEVICE_HANDLE hDev, DWORD offset, UINT32 data)
{
    WDC_WriteAddr32(hDev, VIRTEX5_SPACE, offset, data);
}

void CVirtex5BMD::VIRTEX5_WriteReg16(WDC_DEVICE_HANDLE hDev, DWORD offset, WORD data)
{
    WDC_WriteAddr16(hDev, VIRTEX5_SPACE, offset, data);
}


// Enable DMA interrupts 
void CVirtex5BMD::VIRTEX5_DmaIntEnable(WDC_DEVICE_HANDLE hDev, BOOL fIsRead)
{
    UINT32 ddmacr = VIRTEX5_ReadReg32(hDev, VIRTEX5_DDMACR_OFFSET);

    if (!IsValidDevice((PWDC_DEVICE)hDev, "VIRTEX5_DmaIntEnable"))
        return;

    //ddmacr &= fIsRead ? ~BIT23 : ~BIT7;
	ddmacr = ddmacr & ~BIT23 & ~BIT7;
    VIRTEX5_WriteReg32(hDev, VIRTEX5_DDMACR_OFFSET, ddmacr);
}

DWORD CVirtex5BMD::VIRTEX5_IntEnable(WDC_DEVICE_HANDLE hDev, VIRTEX5_INT_HANDLER funcIntHandler)
{
    DWORD dwStatus;
    PWDC_DEVICE pDev = (PWDC_DEVICE)hDev;
    PVIRTEX5_DEV_CTX pDevCtx;

    //TraceLog("VIRTEX5_IntEnable entered. Device handle: 0x%p\n", hDev);

    if (!IsValidDevice(pDev, "VIRTEX5_IntEnable"))
        return WD_INVALID_PARAMETER;

    pDevCtx = (PVIRTEX5_DEV_CTX)WDC_GetDevContext(pDev);

    /* Check if interrupts are already enabled */
    if (WDC_IntIsEnabled(hDev))
    {
        sprintf_s(m_errMsg, MSG_LENGTH, "Interrupts are already enabled ...");
        return WD_OPERATION_ALREADY_DONE;
    }

    /* Message Signalled interrupts need not be acknowledged,
     * so no transfer commands are needed */
    #define NUM_TRANS_CMDS 0

    /* Store the diag interrupt handler routine, which will be executed by
       VIRTEX5_IntHandler() when an interrupt is received */
    pDevCtx->funcDiagIntHandler = funcIntHandler;
    
    /* Enable the interrupts */
    dwStatus = WDC_IntEnable(hDev, NULL, NUM_TRANS_CMDS, 0, VIRTEX5_IntHandler, (PVOID)pDev, WDC_IS_KP(hDev));

    if (WD_STATUS_SUCCESS != dwStatus)
    {
        sprintf_s(m_errMsg, MSG_LENGTH, "Failed enabling interrupts. Error 0x%lx", dwStatus);
        return dwStatus;
    }

    /* TODO: You can add code here to write to the device in order
             to physically enable the hardware interrupts */

    //TraceLog("VIRTEX5_IntEnable: Interrupts enabled\n");

    return WD_STATUS_SUCCESS;
}

// Disable DMA interrupts 
void CVirtex5BMD::VIRTEX5_DmaIntDisable(WDC_DEVICE_HANDLE hDev, BOOL fIsRead)
{
    UINT32 ddmacr = VIRTEX5_ReadReg32(hDev, VIRTEX5_DDMACR_OFFSET);

    if (!IsValidDevice((PWDC_DEVICE)hDev, "VIRTEX5_DmaIntDisable"))
        return;

    ddmacr |= fIsRead ? BIT23 : BIT7;
    VIRTEX5_WriteReg32(hDev, VIRTEX5_DDMACR_OFFSET, ddmacr);
}


void CVirtex5BMD::VIRTEX5_DMADevicePrepare(VIRTEX5_DMA_HANDLE hDma, BOOL fIsRead, WORD wSize,
    WORD wCount, UINT32 u32Pattern, BOOL fEnable64bit, BYTE bTrafficClass)
{
    UINT32 tlps;
    UINT32 LowerAddr, AddrOffset;
    BYTE UpperAddr;
    WDC_DEVICE_HANDLE hDev;
    PVIRTEX5_DEV_CTX pDevCtx;
	WORD   rSize, rCount;

	rSize  = wSize<<2;
	rCount = wCount>>1;
	AddrOffset = wSize*wCount*sizeof(UINT32);

    if (!IsValidDmaHandle(hDma, "VIRTEX5_DMADevicePrepare"))
        return;

    hDev = hDma->hDev;

    /* Assert Initiator Reset */
    VIRTEX5_WriteReg32(hDev, VIRTEX5_DSCR_OFFSET, 0x1);

    /* De-assert Initiator Reset */
    VIRTEX5_WriteReg32(hDev, VIRTEX5_DSCR_OFFSET, 0x0);

    LowerAddr = (UINT32)hDma->pDma->Page[0].pPhysicalAddr;
    UpperAddr = (BYTE)((hDma->pDma->Page[0].pPhysicalAddr >> 32) & 0xFF);

    tlps = (rSize & 0x1FFF) | /* tlps[0:12] - DMA TLP size */
        ((bTrafficClass & 0x7) << 16) | /* tlps[16:18] -
                                           DMA TLP Traffic Class */
        (fEnable64bit ? BIT19 : 0) | /* tlps[19] enable 64bit TLP */
        (UpperAddr << 24); /* tlps[24:31] - upper bits 33:39 of DMA addr*/

//    if (fIsRead)
  //  {
        /* Set lower 32bit of DMA address */
        VIRTEX5_WriteReg32(hDev, VIRTEX5_RDMATLPA_OFFSET, LowerAddr+AddrOffset);

        /* Set size, traffic class, 64bit enable, upper 8bit of DMA address */
        VIRTEX5_WriteReg32(hDev, VIRTEX5_RDMATLPS_OFFSET, tlps);

        /* Set TLP count */
        VIRTEX5_WriteReg16(hDev, VIRTEX5_RDMATLPC_OFFSET, rCount);

        /* Set Read DMA pattern */
        VIRTEX5_WriteReg32(hDev, VIRTEX5_RDMATLPP_OFFSET, u32Pattern);
//    }
  //  else
    //{
    tlps = (wSize & 0x1FFF) | /* tlps[0:12] - DMA TLP size */
        ((bTrafficClass & 0x7) << 16) | /* tlps[16:18] -
                                           DMA TLP Traffic Class */
        (fEnable64bit ? BIT19 : 0) | /* tlps[19] enable 64bit TLP */
        (UpperAddr << 24); /* tlps[24:31] - upper bits 33:39 of DMA addr*/


		/* Set lower 32bit of DMA address */
        VIRTEX5_WriteReg32(hDev, VIRTEX5_WDMATLPA_OFFSET, LowerAddr);

        /* Set size, traffic class, 64bit enable, upper 8bit of DMA address */
        VIRTEX5_WriteReg32(hDev, VIRTEX5_WDMATLPS_OFFSET, tlps);

        /* Set TLP count */
        VIRTEX5_WriteReg16(hDev, VIRTEX5_WDMATLPC_OFFSET, wCount);

        /* Set Read DMA pattern */
        VIRTEX5_WriteReg32(hDev, VIRTEX5_WDMATLPP_OFFSET, u32Pattern);
  //  }

    pDevCtx = (PVIRTEX5_DEV_CTX)WDC_GetDevContext(hDev);
//    pDevCtx->fIsRead = fIsRead;
    pDevCtx->u32Pattern = u32Pattern;
    pDevCtx->dwTotalCount = (DWORD)wCount * (DWORD)wSize;
}

// this stimulus pattern is not stretched
void CVirtex5BMD::AppLoadStimulus(WDC_DEVICE_HANDLE hDev, unsigned short *buffer, int nx, int ny, int channelID)
{
	UINT32 regData1, regData2, reg1, reg2, reg3, reg4, address, addr16, ctrlBits;
	int    i, j, dword;
	BYTE   bytes[2];

	// set ram status to "read/write"
	switch (channelID) {
	case STIM_CH796:
		ctrlBits  = BIT0;		// for 488nm stimulus
		break;
	case STIM_CH568A:
		ctrlBits  = BIT1;		// for 568nm stimulus, odd channel
		break;
	case STIM_CH568B:
		ctrlBits  = BIT2;		// for 568nm stimulus, even channel
		break;
	case STIM_CHALL:
		ctrlBits = BIT0+BIT1+BIT2;
		break;
	default:
		return;
		break;
	}

	// reserve high 29 bits
	reg1 = VIRTEX5_ReadReg32(hDev, VIRTEX5_TCABOX_OFFSET);
	reg1 = reg1>>3;
	reg1 = reg1<<3;
	// write enable bit(s)
	VIRTEX5_WriteReg32(hDev, VIRTEX5_TCABOX_OFFSET, reg1+ctrlBits);

	dword = nx*ny/2+1;
	reg1 = reg2 = reg3 = 0;
	for (i = 0; i < dword; i ++) {
		for (j = 0; j < 2; j ++) {
			if (2*i+j < nx*ny)
				bytes[j] = (BYTE)(buffer[2*i+j]>>6);
			else 
				bytes[j] = 0;
		}
		reg1 = bytes[0]<<16; 
		reg2 = bytes[1]<<24;
		regData1 = reg1 + reg2;

		// write 32-bit data
		VIRTEX5_WriteReg32(hDev, VIRTEX5_STIM_DATA, regData1+i);
	}

	// set ram status to "read only"
	reg1 = reg1 - ctrlBits;
	VIRTEX5_WriteReg32(hDev, VIRTEX5_TCABOX_OFFSET, reg1);
}

// write pre-sinusoiding lookup table to FPGA
void CVirtex5BMD::AppWriteSinusoidLUT(WDC_DEVICE_HANDLE hDev, int nSizeX, int nStretchedX, unsigned short *lut_buf, int nChannelID)
{
	UINT32 regData1, reg1, wbits;
	int    i;

	// set ram status to "read/write"
	switch (nChannelID) {
	case STIM_CH796:
		wbits  = BIT9;		// for 488nm stimulus
		break;
	case STIM_CH568A:
		wbits  = BIT6;		// for 568nm stimulus, odd channel
		break;
	case STIM_CH568B:
		wbits  = BIT6;		// for 568nm stimulus, even channel
		break;
	case STIM_CHALL:
		wbits = BIT9+BIT6;
		break;
	default:
		return;
		break;
	}

	regData1 = VIRTEX5_ReadReg32(hDev, VIRTEX5_STIMULUS_LOC);
	regData1 = regData1 << 22;
	regData1 = regData1 >> 22;
	regData1 = regData1 | wbits;

	for (i = 0; i < nStretchedX; i ++) {
		reg1 = (i<<21) + (lut_buf[i]<<10) + regData1;
		// write 32-bit data
		VIRTEX5_WriteReg32(hDev, VIRTEX5_STIMULUS_LOC, reg1);
	}

	regData1 = regData1 - wbits;
	VIRTEX5_WriteReg32(hDev, VIRTEX5_STIMULUS_LOC, regData1);	
}


// do interpolation of stimulus location x in the direction of y
void CVirtex5BMD::AppWriteStimLocX(WDC_DEVICE_HANDLE hDev, int latency, int x1, int x2, int y1, int y2, int y3, int xL, int xR, int nChannelID)
{
	UINT32  regData1, regData2, regLocY, regLocX, ctrlBits, wbits;
	float   deltax;
	int     i;

	if (y2 <= y1) return;

	// set ram status to "read/write"
	switch (nChannelID) {
	case STIM_CH796:
		wbits  = BIT8;		// for 488nm stimulus
		break;
	case STIM_CH568A:
		wbits  = BIT5;		// for 568nm stimulus, odd channel
		break;
	case STIM_CH568B:
		wbits  = BIT5;		// for 568nm stimulus, even channel
		break;
	case STIM_CHALL:
		wbits = BIT8+BIT5;
		break;
	default:
		return;
		break;
	}

	ctrlBits = VIRTEX5_ReadReg32(hDev, VIRTEX5_STIMULUS_LOC);
	ctrlBits = ctrlBits << 22;
	ctrlBits = ctrlBits >> 22;

	// set "we" bit for LUT RAM, so that it is in write status
	ctrlBits = ctrlBits | wbits;

	deltax = (float)(1.0*(x2-x1)/(y2-y1));

	for (i = y1; i < y2; i ++) {
		regLocY = (i << 21);							// address
		regLocX = (UINT32)(x1 + (i-y1)*deltax + 0.5);
		regLocX = (regLocX-latency) << 10;	// data
		VIRTEX5_WriteReg32(hDev, VIRTEX5_STIMULUS_LOC, regLocY+regLocX+ctrlBits);
	}

	// if there is image stretching, extrapolate location of x along the stretched part
	if (y3 > y2) {
		for (i = y2; i < y3; i ++) {
			regLocY = (i << 21);							// address
			regLocX = (x2-latency) << 10;	// data
			VIRTEX5_WriteReg32(hDev, VIRTEX5_STIMULUS_LOC, regLocY+regLocX+ctrlBits);
		}
	}

	// clear "we" bit for LUT RAM so that it is in read status
	ctrlBits = ctrlBits - wbits;
	VIRTEX5_WriteReg32(hDev, VIRTEX5_STIMULUS_LOC, ctrlBits);	


	regData1 = xL + (xR<<8);
	regData2 = (xL<<16) + (xR<<24);
	VIRTEX5_WriteReg32(hDev, VIRTEX5_STIMULUS_X_BOUND, regData1+regData2);
//	VIRTEX5_WriteReg32(hDev, VIRTEX5_STIMULUS_X_BOUND2, regData1+regData2);
}


// write location of stimulus delta_x along the direction of Y, e.g.,
// 
//        y=y1,  
//
//
void CVirtex5BMD::AppWriteStimScaleY(WDC_DEVICE_HANDLE hDev, int y1, int y2, int y3, int stimNX, int stripeH, int nChannelID)
{
	UINT32  regLocY, regLocX, ctrlBits, wbits;
	float   deltax;
	int     i, delta;

	if (y2 <= y1) return;

	// set ram status to "read/write"
	switch (nChannelID) {
	case STIM_CH796:
		wbits  = BIT7;		// for 488nm stimulus
		break;
	case STIM_CH568A:
		wbits  = BIT4;		// for 568nm stimulus, odd channel
		break;
	case STIM_CH568B:
		wbits  = BIT4;		// for 568nm stimulus, even channel
		break;
	case STIM_CHALL:
		wbits = BIT7+BIT4;
		break;
	default:
		return;
		break;
	}

	// reserve low 10 bits
	ctrlBits = VIRTEX5_ReadReg32(hDev, VIRTEX5_STIMULUS_LOC);
	ctrlBits = ctrlBits << 22;
	ctrlBits = ctrlBits >> 22;

	// set "we" bit for stimulus location RAM, so that it is in write status
	ctrlBits = ctrlBits | wbits;

	for (i = y1; i < y2; i ++) {
		regLocY = i << 21;							// address
		regLocX = stimNX << 10;
		VIRTEX5_WriteReg32(hDev, VIRTEX5_STIMULUS_LOC, regLocY+regLocX+ctrlBits);
	}
	// image is stretched in y direction
	for (i = y2; i < y3; i ++) {
		regLocY = i << 21;							// address
		regLocX = 0;
		VIRTEX5_WriteReg32(hDev, VIRTEX5_STIMULUS_LOC, regLocY+regLocX+ctrlBits);
	}
	// image is compressed in y direction
	if (y2-y1 < stripeH) {
		delta = stripeH - (y2-y1) + 1;
		regLocY = y1 << 21;							// address
		regLocX = (delta*stimNX) << 10;
		VIRTEX5_WriteReg32(hDev, VIRTEX5_STIMULUS_LOC, regLocY+regLocX+ctrlBits);
	}

	ctrlBits = ctrlBits - wbits;
	VIRTEX5_WriteReg32(hDev, VIRTEX5_STIMULUS_LOC, ctrlBits);	
}


int CVirtex5BMD::ReadWriteI2CRegister(WDC_DEVICE_HANDLE hDev, BOOL bRead, UINT32 slave_addr, BYTE m_regAddr, BYTE regValI, BYTE *regValO)
{
	UINT32  regData, regAddr, regVal;
	int     ret;
	BYTE    reg_val;
	DWORD   regIdx;

	regData = VIRTEX5_ReadReg32(hDev, VIRTEX5_I2CINFO_OFFSET);

	// clear the high 30 bits
	regData = regData << 30;
	regData = regData >> 30;
	VIRTEX5_WriteReg32(hDev, VIRTEX5_I2CINFO_OFFSET, regData);

	// move "register address" to bits 23-16
	regAddr = m_regAddr;
	regAddr = regAddr << 16;

	// move "register address" to bits 15-8
	regVal = regValI;
	regVal = regVal << 8;

	// add I2C slave address, I2C register address and I2C register value to the PCIe register data
	regData |= slave_addr;
	regData |= regAddr;
	regData |= regVal;

	if (bRead) {
		// Set bit 7 and clear bit 6 and bit 5 of the command
		regData |= BIT7;
		regData = regData&BIT6 ? regData-BIT6 : regData;
		regData = regData&BIT5 ? regData-BIT5 : regData;
	} else {
		// Set bit 4 and clear bit 3 and bit 2 of the command
		regData |= BIT4;
		regData = regData&BIT3 ? regData-BIT3 : regData;
		regData = regData&BIT2 ? regData-BIT2 : regData;
	}

	if (bRead)
		ret = I2CController(hDev, regData, &reg_val, TRUE);
	else
		ret = I2CController(hDev, regData, &reg_val, FALSE);

	if (ret == 0) 
		*regValO = reg_val;
	else
		*regValO = 0xff;

	return ret;
}

int CVirtex5BMD::I2CController(WDC_DEVICE_HANDLE hDev, UINT32 regDataW, BYTE *reg_val, BOOL bI2Cread)
{
	BOOL    bI2Cdone;
	int     ret, timeout;
	UINT32  regDataR, regVal, doneBit, errBit;
//	CString msg;
	DWORD   regIdx;

	if (bI2Cread) {
		doneBit = BIT5;
		errBit  = BIT6;
	} else {
		doneBit = BIT2;
		errBit  = BIT3;
	}

	VIRTEX5_WriteReg32(hDev, VIRTEX5_I2CINFO_OFFSET, regDataW);
	bI2Cdone = FALSE;
	timeout  = 0;
	do {
		Sleep(1);
		regDataR = VIRTEX5_ReadReg32(hDev, VIRTEX5_I2CINFO_OFFSET);
//		msg.Format("0x%8X", regDataR);

		// I2C bus controller has finished data access
		if (regDataR & doneBit) {
			bI2Cdone = TRUE;
			//if (regDataR & BIT6) {
			if (regDataR & errBit) {
				*reg_val = 0xff;
				return 2;
			} else {
				regVal = regDataR << 16;
				regVal = regVal >> 24;
//				msg.Format("0x%8X", regVal);
				*reg_val = (BYTE)regVal;
			}
//			msg.Format("0x%8X", regDataR);
		}
		timeout ++;
		if (timeout > 10) {
			bI2Cdone = TRUE;
			*reg_val = 0xff;
			return 3;
		}
	} while (bI2Cdone == FALSE);

	regDataR = regDataR << 30;
	regDataR = regDataR >> 30;
	VIRTEX5_WriteReg32(hDev, VIRTEX5_I2CINFO_OFFSET, regDataR);
//	msg.Format("0x%8X", regDataR);

	return 0;
}

BOOL CVirtex5BMD::DetectSyncSignals(WDC_DEVICE_HANDLE hDev)
{
	BYTE    regVal, hsync, vsync;

	if (ReadWriteI2CRegister(hDev, TRUE, 0x4c000000, 0x24, 0, &regVal) == 0) {
		hsync = regVal & BIT7;
		vsync = regVal & BIT5;
		if (hsync == 0 || vsync == 0) 
			return FALSE;
		else 
			return TRUE;
	} else {
		return TRUE;
	}
}

void CVirtex5BMD::AppStopADC(WDC_DEVICE_HANDLE hDev)
{
	UINT32 regR;

	regR = VIRTEX5_ReadReg32(hDev, VIRTEX5_LINECTRL_OFFSET);
	regR |= BIT0;
	regR -= BIT0;
    VIRTEX5_WriteReg32(hDev, VIRTEX5_LINECTRL_OFFSET, regR);	
}

void CVirtex5BMD::AppStartADC(WDC_DEVICE_HANDLE hDev)
{
	UINT32 ddmacr;

	ddmacr = VIRTEX5_ReadReg32(hDev, VIRTEX5_LINECTRL_OFFSET);
	ddmacr |= BIT0;
    VIRTEX5_WriteReg32(hDev, VIRTEX5_LINECTRL_OFFSET, ddmacr);
}

void CVirtex5BMD::AppWriteStimLoc(WDC_DEVICE_HANDLE hDev, int x, int y)
{
	UINT32 ctrlBits, reg;

	if (x < 0 || y < 0) return;

	ctrlBits = VIRTEX5_ReadReg32(hDev, VIRTEX5_STIMULUS_LOC);
	ctrlBits = ctrlBits << 22;
	ctrlBits = ctrlBits >> 22;
	
	reg = (y << 21) + (x << 10) + ctrlBits;
	VIRTEX5_WriteReg32(hDev, VIRTEX5_STIMULUS_LOC, reg);
}


void CVirtex5BMD::AppSetFrameCounter(WDC_DEVICE_HANDLE hDev, BOOL bByLines)
{
	UINT32 reg, high, low;

	reg = VIRTEX5_ReadReg32(hDev, VIRTEX5_LINECTRL_OFFSET);
	high = reg  >> 6;
	high = high << 6;
	low  = reg  << 27;
	low  = low  >> 27;
	
	if (bByLines) 
		reg = high + BIT5 + low;
	else
		reg = high + low;

	VIRTEX5_WriteReg32(hDev, VIRTEX5_LINECTRL_OFFSET, reg);
}


void CVirtex5BMD::SetScannerParams(WDC_DEVICE_HANDLE hDev, BYTE fovH, BYTE fovV, unsigned short vDivider, unsigned short vOffset, BYTE backStepSize, BOOL doubleLine) 
{
	UINT32 reg, regM, regH;

	reg  = VIRTEX5_ReadReg32(hDev, VIRTEX5_LASER_POWER2);
	regM = reg;
	reg  = reg << 16;	// reserve low 16 bits
	reg  = reg >> 16;

	regH = regM >> 24;
	regH = regH << 24;

	regM = regM>>21;	// reserve bits [23:21]
	regM = regM<<29;
	regM = regM<<21;

	//reg  = (fovH<<24) + regM + (fovV<<16) + reg;
	reg  = regH + regM + (fovV<<16) + reg;
	
	VIRTEX5_WriteReg32(hDev, VIRTEX5_LASER_POWER2, reg);

	reg  = VIRTEX5_ReadReg32(hDev, VIRTEX5_TCABOX_OFFSET);
	reg  = reg << 29;		// reserve the low 3 bit
	reg  = reg >> 29;
	reg  = (backStepSize<<24) + (vOffset<<14) + (vDivider<<4) + reg;
	if (doubleLine)
		reg  = reg | BIT3;
	VIRTEX5_WriteReg32(hDev, VIRTEX5_TCABOX_OFFSET, reg);
}

void CVirtex5BMD::LoadSignalBuffer(WDC_DEVICE_HANDLE hDev, unsigned short length, unsigned short *buffer) 
{
	UINT32 address, addr, data;
	int i;

	if (length > 0x4000) return;

	address  = BIT30;

	for (i = 0; i < length; i ++) {
		// write 14-bit address
		addr = address + i;
		VIRTEX5_WriteReg32(hDev, VIRTEX5_STIM_ADDRESS, addr);
		// write 14-bit data
		data = 0;//buffer[i];
		VIRTEX5_WriteReg32(hDev, VIRTEX5_STIM_DATA, data);
	}

	for (i = 0; i < length; i ++) {
		// write 14-bit address
		addr = address + i;
		VIRTEX5_WriteReg32(hDev, VIRTEX5_STIM_ADDRESS, addr);
		// write 14-bit data
		data = buffer[i];
		VIRTEX5_WriteReg32(hDev, VIRTEX5_STIM_DATA, data);
	}

	// set ram status to "read only"
	VIRTEX5_WriteReg32(hDev, VIRTEX5_STIM_ADDRESS, 0);
}


void CVirtex5BMD::SetSigFreq(WDC_DEVICE_HANDLE hDev, float freq) 
{
	float  freqConst;
	int    freqDivider;
	UINT32 reg;

	freqConst = 3814.69; // = 62.5MHz (pixel clock) / 16384 (bit depth);
	freqDivider = (int)(freqConst/(freq));

	reg  = VIRTEX5_ReadReg32(hDev, VIRTEX5_TCABOX_OFFSET);
	reg  = reg >> 14;		// reserve the high 18 bits
	reg  = reg << 14;
	reg  = reg + freqDivider;
	VIRTEX5_WriteReg32(hDev, VIRTEX5_TCABOX_OFFSET, reg);
}

void CVirtex5BMD::ApplyTrackingPos(WDC_DEVICE_HANDLE hDev, short PosX, short PosY, BOOL rotate45) 
{
	short  sx, sy;
	UINT32 reg, regHi, regLo, tx, ty;

	if (rotate45) {
		// no coordinate transform is needed, when the PI mirror is installed at 45 degree.
		sx = PosX;
		sy = PosY;
	} else {
		// when the PI mirror is installed at 0- or 90- degree,
		// the tracking location needs coordinate transform
		sx = (short)((PosX-PosY)*0.70710678118654752440084436210485);
		sy = (short)((PosX+PosY)*0.70710678118654752440084436210485);
	}

	reg = VIRTEX5_ReadReg32(hDev, VIRTEX5_LASER_POWER);
	regHi = reg & 0xC0000000; // keep bits 31 and 30
	regLo = reg & 0x0000FFFF; // keep low 16 bits
	if (sx < 0) 
		tx = 0x2000 - sx;		// set bit 13 to '1'
	else 
		tx = sx;
	tx = (tx << 16);
/*
	if (sy < 0) 
		ty = 0x2000 - sy;
	else 
		ty = sy;
*/
	VIRTEX5_WriteReg32(hDev, VIRTEX5_LASER_POWER, regHi+tx+regLo);
}


void CVirtex5BMD::UpdateSlowScanner(WDC_DEVICE_HANDLE hDev, BOOL bZeroBackward) 
{
	UINT32   reg;

	reg = VIRTEX5_ReadReg32(hDev, VIRTEX5_LASER_POWER);
	if (bZeroBackward) 
		reg = reg | BIT31;
	else
		reg = reg & 0x7fffffff;
	VIRTEX5_WriteReg32(hDev, VIRTEX5_LASER_POWER, reg);
}


void CVirtex5BMD::SetSymmetricRamp(WDC_DEVICE_HANDLE hDev, BOOL bSymmetric) 
{
	UINT32   reg;
/*
	reg = VIRTEX5_ReadReg32(hDev, VIRTEX5_LASER_POWER);
	if (bSymmetric) 
		reg = reg | BIT30;
	else
		reg = reg & 0xbfffffff;
	VIRTEX5_WriteReg32(hDev, VIRTEX5_LASER_POWER, reg);*/
}


void CVirtex5BMD::EnableMirrorTracking(WDC_DEVICE_HANDLE hDev, BOOL bFlag) 
{
	UINT32   reg;

	reg = VIRTEX5_ReadReg32(hDev, VIRTEX5_LASER_POWER);
	if (bFlag) 
		reg = reg | BIT30;
	else
		reg = reg & 0xbfffffff;
	VIRTEX5_WriteReg32(hDev, VIRTEX5_LASER_POWER, reg);
}


void CVirtex5BMD::ApplySteeringX(WDC_DEVICE_HANDLE hDev, short PosX) 
{
	UINT32 reg, regNeg, regPos;
/*
	reg = VIRTEX5_ReadReg32(hDev, VIRTEX5_STIMULUS_X_BOUND2);
	reg = (reg << 16);
	reg = (reg >> 16);

	if (PosX < 0) {
		regNeg = -PosX;
		regPos = 0;
	} else if (PosX == 0) {
		regNeg = 0;
		regPos = 0;
	} else {
		regNeg = 0;
		regPos = PosX;
	}

	regNeg = (regNeg << 24);
	regPos = (regPos << 16);

	VIRTEX5_WriteReg32(hDev, VIRTEX5_STIMULUS_X_BOUND2, regNeg+regPos);*/
}


void CVirtex5BMD::AOShutter(WDC_DEVICE_HANDLE hDev, BOOL bFlagOn) {
	UINT32 reg;

	reg = VIRTEX5_ReadReg32(hDev, VIRTEX5_STIM_ADDRESS);
	if (bFlagOn) 
		reg = reg | BIT31;
	else
		reg = reg & 0x7FFFFFFF;
	VIRTEX5_WriteReg32(hDev, VIRTEX5_STIM_ADDRESS, reg);
}


void CVirtex5BMD::UpdatePower488(WDC_DEVICE_HANDLE hDev, BYTE power) {
	UINT32 reg;

	reg  = VIRTEX5_ReadReg32(hDev, VIRTEX5_LASER_POWER2);
	reg  = reg >> 8;	// reserve high 24 bits
	reg  = reg << 8;
	VIRTEX5_WriteReg32(hDev, VIRTEX5_LASER_POWER2, reg+power);
}


void CVirtex5BMD::UpdatePower796(WDC_DEVICE_HANDLE hDev, BYTE power) {
	UINT32 reg, regH, regL, regM;

	reg  = VIRTEX5_ReadReg32(hDev, VIRTEX5_LASER_POWER2);
	regH = reg & 0xffff0000;	// reserve high 16 bits
	regL = reg & 0x000000ff;	// reserve low 8 bits
	regM = power << 8;
	reg = regH + regM + regL;

	VIRTEX5_WriteReg32(hDev, VIRTEX5_LASER_POWER2, reg);
}


void CVirtex5BMD::UpdatePower568(WDC_DEVICE_HANDLE hDev, unsigned short power) {
	UINT32 reg;
	unsigned short temp;

	temp = (power<=0x3fff) ? power : 0x3fff;

	reg  = VIRTEX5_ReadReg32(hDev, VIRTEX5_LASER_POWER);
	reg  = reg >> 14;	// reserve high 18 bits
	reg  = reg << 14;
	VIRTEX5_WriteReg32(hDev, VIRTEX5_LASER_POWER, reg+temp);
}


void CVirtex5BMD::CalibratePower488(WDC_DEVICE_HANDLE hDev, BOOL flag) {
	UINT32 reg;

	reg  = VIRTEX5_ReadReg32(hDev, VIRTEX5_LASER_POWER2);
	if (flag) {
		reg = reg | BIT21;
	} else {
		reg = (reg|BIT21)-BIT21;
	}

	VIRTEX5_WriteReg32(hDev, VIRTEX5_LASER_POWER2, reg);
}


void CVirtex5BMD::CalibratePower568(WDC_DEVICE_HANDLE hDev, BOOL flag) {
	UINT32 reg;

	reg  = VIRTEX5_ReadReg32(hDev, VIRTEX5_LASER_POWER2);
	if (flag) {
		reg = reg | BIT22;
	} else {
		reg = (reg|BIT22)-BIT22;
	}

	VIRTEX5_WriteReg32(hDev, VIRTEX5_LASER_POWER2, reg);
}


void CVirtex5BMD::CalibratePower796(WDC_DEVICE_HANDLE hDev, BOOL flag) {
	UINT32 reg;

	reg  = VIRTEX5_ReadReg32(hDev, VIRTEX5_LASER_POWER2);
	if (flag) {
		reg = reg | BIT23;
	} else {
		reg = (reg|BIT23)-BIT23;
	}

	VIRTEX5_WriteReg32(hDev, VIRTEX5_LASER_POWER2, reg);
}


// initialize laser modulation across the fast scanning FOV
void CVirtex5BMD::AppLaserModulation(WDC_DEVICE_HANDLE hDev, float *buffer, int length)
{
	UINT32 regData1, reg1, wbits;
	int    i, power;

	wbits = BIT3+BIT2;

	regData1 = VIRTEX5_ReadReg32(hDev, VIRTEX5_STIMULUS_LOC);

	// reserve low 10 bits
	regData1 = regData1 << 22;
	regData1 = regData1 >> 22;
	regData1 = regData1 | wbits;

	for (i = 0; i < length; i ++) {
		power = 0x7ff * buffer[i];
		reg1 = (i<<21) + (power<<10) + regData1;
		// write 32-bit data
		VIRTEX5_WriteReg32(hDev, VIRTEX5_STIMULUS_LOC, reg1);
	}

	regData1 = regData1 - wbits;
	VIRTEX5_WriteReg32(hDev, VIRTEX5_STIMULUS_LOC, regData1);	
}


void CVirtex5BMD::SwitchShutter(WDC_DEVICE_HANDLE hDev, BOOL flag) {
	UINT32 reg;

	reg = VIRTEX5_ReadReg32(hDev, VIRTEX5_LASER_POWER);
	if (flag) reg = reg | BIT31;
	else reg = reg & 0x7fffffff;
	VIRTEX5_WriteReg32(hDev, VIRTEX5_LASER_POWER, reg);
}


