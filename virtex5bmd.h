#ifndef AFX_VIRTEX5BMD_H
#define AFX_VIRTEX5BMD_H

#include "include/wdc_lib.h"
#include "samples/shared/pci_regs.h"
#include "samples/shared/bits.h"

#define VIRTEX5_DEFAULT_VENDOR_ID 0x10EE /* Vendor ID */
#define VIRTEX5_DEFAULT_DEVICE_ID 0x7 /* Device ID */
#define KP_VRTX5_DRIVER_NAME "KP_VRTX5"
#define VIRTEX5_SPACE AD_PCI_BAR0

#define  MSG_LENGTH 256
#define  STIM_CH796    0
#define  STIM_CH568A   1
#define  STIM_CH568B   2
#define  STIM_CHALL    3

#define	 MODULATION_SHIFT  50;

enum {
    VIRTEX5_DSCR_OFFSET = 0x0,
    VIRTEX5_DDMACR_OFFSET = 0x4,
    VIRTEX5_WDMATLPA_OFFSET = 0x8,
    VIRTEX5_WDMATLPS_OFFSET = 0xc,
    VIRTEX5_WDMATLPC_OFFSET = 0x10,
    VIRTEX5_WDMATLPP_OFFSET = 0x14,
    VIRTEX5_RDMATLPP_OFFSET = 0x18,
    VIRTEX5_RDMATLPA_OFFSET = 0x1c,
    VIRTEX5_RDMATLPS_OFFSET = 0x20,
    VIRTEX5_RDMATLPC_OFFSET = 0x24,
//    VIRTEX5_WDMAPERF_OFFSET = 0x28,
  //  VIRTEX5_RDMAPERF_OFFSET = 0x2c,
	VIRTEX5_PUPILMASK_OFFSET  = 0x28,			// not used
    VIRTEX5_TCABOX_OFFSET     = 0x2c,
    VIRTEX5_RDMASTAT_OFFSET = 0x30,
    //VIRTEX5_NRDCOMP_OFFSET = 0x34,
    //VIRTEX5_RCOMPDSIZE_OFFSET = 0x38,
	VIRTEX5_LASER_POWER2 = 0x34,
	VIRTEX5_STIMULUS_X_BOUND2 = 0x38,			// not used
    VIRTEX5_DLWSTAT_OFFSET = 0x3c,
    VIRTEX5_DLTRSSTAT_OFFSET = 0x40, 
    VIRTEX5_DMISCCONT_OFFSET = 0x44,
	VIRTEX5_I2CINFO_OFFSET = 0x48,
	VIRTEX5_LINEINFO_OFFSET = 0x4c,				// not used
	VIRTEX5_IMAGESIZE_OFFSET = 0x50,
	VIRTEX5_IMAGEOS_OFFSET = 0x54,
	VIRTEX5_LINECTRL_OFFSET = 0x58,
	VIRTEX5_STIMULUS_LOC = 0x5c,
	VIRTEX5_LASER_POWER = 0x60,
	VIRTEX5_STIM_ADDRESS = 0x64,				// not used
	VIRTEX5_STIM_DATA = 0x68,
	VIRTEX5_STIMULUS_X_BOUND = 0x6c
};

/* Kernel PlugIn messages - used in WDC_CallKerPlug() calls (user-mode) /
 * KP_VRTX5_Call() (kernel-mode) */
enum {
    KP_VRTX5_MSG_VERSION = 1, /* Query the version of the Kernel PlugIn */
	KP_VRTX5_MSG_DATA = 2, /* Pass user-mode data to Kernel Plugin */
	KP_VRTX5_MSG_LINEID = 3,
};

/* Kernel Plugin messages status */
enum {
    KP_VRTX5_STATUS_OK = 0x1,
	KP_VRTX5_DATA_RECEIVED = 0x2,
    KP_VRTX5_STATUS_MSG_NO_IMPL = 0x1000,
};

// Register information struct 
typedef struct {
    DWORD dwAddrSpace;       // Number of address space in which the register resides 
                             // For PCI configuration registers, use WDC_AD_CFG_SPACE 
    DWORD dwOffset;          // Offset of the register in the dwAddrSpace address space 
    DWORD dwSize;            // Register's size (in bytes) 
    WDC_DIRECTION direction; // Read/write access mode - see WDC_DIRECTION options 
    CHAR  sName[MAX_NAME];   // Register's name 
    CHAR  sDesc[MAX_DESC];   // Register's description 
} WDC_REG;

// Direct Memory Access (DMA) 
typedef struct {
    WD_DMA *pDma;
    WDC_DEVICE_HANDLE hDev;
} VIRTEX5_DMA_STRUCT, *VIRTEX5_DMA_HANDLE;
 
// Kernel PlugIn version information struct 
typedef struct {
    DWORD dwVer;
    CHAR cVer[100];
} KP_VRTX5_VERSION;

// Address space information struct 
#define MAX_TYPE 8
typedef struct {
    DWORD dwAddrSpace;
    CHAR sType[MAX_TYPE];
    CHAR sName[MAX_NAME];
    CHAR sDesc[MAX_DESC];
} VIRTEX5_ADDR_SPACE_INFO;

// Interrupt result information struct 
typedef struct
{
    DWORD dwCounter; /* Number of interrupts received */
    DWORD dwLost;    /* Number of interrupts not yet handled */
    WD_INTERRUPT_WAIT_RESULT waitResult; /* See WD_INTERRUPT_WAIT_RESULT values
                                            in windrvr.h */
    BOOL fIsMessageBased;
    DWORD dwLastMessage;

    PVOID pBuf;
    UINT32 u32Pattern;
    DWORD dwTotalCount;
    BOOL fIsRead;
} VIRTEX5_INT_RESULT;

// VIRTEX5 diagnostics interrupt handler function type 
typedef void (*VIRTEX5_INT_HANDLER)(WDC_DEVICE_HANDLE hDev,
    VIRTEX5_INT_RESULT *pIntResult);

// VIRTEX5 diagnostics plug-and-play and power management events handler function type 
typedef void (*VIRTEX5_EVENT_HANDLER)(WDC_DEVICE_HANDLE hDev,
    DWORD dwAction);

// VIRTEX5 device information struct 
typedef struct {
    VIRTEX5_INT_HANDLER funcDiagIntHandler;
    VIRTEX5_EVENT_HANDLER funcDiagEventHandler;
    VIRTEX5_DMA_HANDLE hDma;
    PVOID pBuf;
    BOOL fIsRead;
    UINT32 u32Pattern;
    DWORD dwTotalCount;
} VIRTEX5_DEV_CTX, *PVIRTEX5_DEV_CTX;


typedef struct {
    VIRTEX5_DMA_HANDLE hDma;
    PVOID pBuf;
} DIAG_DMA, *PDIAG_DMA;


typedef struct {
	UINT32    line_start_addr;
	UINT32    line_end_addr;
	UINT32    addr_interval;
	unsigned short   end_line_ID;
	unsigned short   img_width;
	unsigned short   img_height;
	unsigned short   offset_line;
	unsigned short   offset_pixel;
	unsigned short   tlp_counts;
	unsigned char    line_spacing;
	unsigned char   *video_in;
	unsigned char   *video_out;
	unsigned short  *stim_buffer;
	unsigned char   *fsbs_buffer;
	unsigned char   *desi_buffer;
	UINT32    *weightsRed;
	UINT32    *weightsIR;
	int        stim_nx;
	int        stim_ny;
	UINT32     frameCounter;
	int        nVideoTabs;
	int        nVideoChannels;
	double     fFrameInterval;
	BOOL       bDualChannel;
	BOOL       bDesinusoid;
	
	int        nVideoLength;
	int        nVideoCounter;
	BOOL       bVideoSaving;

	BYTE       fovH;
	BYTE       fovV;
	unsigned short fLines;
	unsigned short bStepSize;
	unsigned short vOffset;
	BOOL       bDoubleLine;
	BOOL       bZeroBackward;
	BOOL       bSymmetricRamp;

	int       *LeftIndex;
	int       *RightIndex;
	double    *LeftWeigh;
	double    *RightWeigh;
	int        DesinusoidLen;
} VIDEO_INFO;
/*
typedef struct {
	UINT32			 addr_interval;
	unsigned short   img_width;
	unsigned short   img_height;
	unsigned char    line_spacing;
} VIDEO_HEADER_INFO;
*/
class CVirtex5BMD {
private:
	char     *m_errMsg;
	DWORD     m_errCode;

public:
	void AppSetFrameCounter(WDC_DEVICE_HANDLE hDev, BOOL bByLines);
	void AppWriteStimLoc(WDC_DEVICE_HANDLE hDev, int x, int y);
	void AppStartADC(WDC_DEVICE_HANDLE hDev);
	void AppStopADC(WDC_DEVICE_HANDLE hDev);
	BOOL DetectSyncSignals(WDC_DEVICE_HANDLE hDev);
	int I2CController(WDC_DEVICE_HANDLE hDev, UINT32 regDataW, BYTE *reg_val, BOOL bI2Cread);
	int ReadWriteI2CRegister(WDC_DEVICE_HANDLE hDev, BOOL bRead, UINT32 slave_addr, BYTE m_regAddr, BYTE regValI, BYTE *regValO);

	void AppWriteSinusoidLUT(WDC_DEVICE_HANDLE hDev, int nSizeX, int nStretchedX, unsigned short *lut_buf, int nChannelID);
	void AppLoadStimulus(WDC_DEVICE_HANDLE hDev, unsigned short *buffer, int nx, int ny, int channelID);
	void AppWriteStimScaleY(WDC_DEVICE_HANDLE hDev, int y1, int y2, int y3, int stimNX, int stripeH, int nChannelID);
	void AppWriteStimLocX(WDC_DEVICE_HANDLE hDev, int latency, int x1, int x2, int y1, int y2, int y3, int xL, int xR, int nChannelID);

	CVirtex5BMD();
	~CVirtex5BMD();

	DWORD VIRTEX5_LibInit(void);

	WDC_DEVICE_HANDLE DeviceFindAndOpen(DWORD dwVendorId, DWORD dwDeviceId);
	BOOL DeviceFind(DWORD dwVendorId, DWORD dwDeviceId, WD_PCI_SLOT *pSlot);
	WDC_DEVICE_HANDLE DeviceOpen(const WD_PCI_SLOT *pSlot);
	void DeviceClose(WDC_DEVICE_HANDLE hDev, PDIAG_DMA pDma);
	WDC_DEVICE_HANDLE VIRTEX5_DeviceOpen(const WD_PCI_CARD_INFO *pDeviceInfo);
	BOOL VIRTEX5_DeviceClose(WDC_DEVICE_HANDLE hDev);
	void DIAG_DMAClose(WDC_DEVICE_HANDLE hDev, PDIAG_DMA pDma);
	DWORD VIRTEX5_IntDisable(WDC_DEVICE_HANDLE hDev);
	void VIRTEX5_DMAClose(VIRTEX5_DMA_HANDLE hDma);
	BOOL IsValidDmaHandle(VIRTEX5_DMA_HANDLE hDma, CHAR *sFunc);
	BOOL VIRTEX5_IntIsEnabled(WDC_DEVICE_HANDLE hDev);
	DWORD VIRTEX5_LibUninit(void);
	BOOL ReadWriteReg(WDC_DEVICE_HANDLE hDev, WDC_REG *pRegs, DWORD dwReg,
						DWORD dwNumRegs, WDC_DIRECTION direction, BOOL fPciCfg,
						UINT64 *u64Rdata, UINT u64Wdata);
	WORD DMAGetMaxPacketSize(WDC_DEVICE_HANDLE hDev, BOOL fIsRead);
	UINT32 VIRTEX5_ReadReg32(WDC_DEVICE_HANDLE hDev, DWORD offset);
	WORD code2size(BYTE bCode);
	DWORD VIRTEX5_DMAOpen(WDC_DEVICE_HANDLE hDev, PVOID *ppBuf, DWORD dwOptions,
						DWORD dwBytes, VIRTEX5_DMA_HANDLE *ppDmaHandle);
	void VIRTEX5_DmaIntEnable(WDC_DEVICE_HANDLE hDev, BOOL fIsRead);
	void VIRTEX5_DmaIntDisable(WDC_DEVICE_HANDLE hDev, BOOL fIsRead);
	void VIRTEX5_WriteReg32(WDC_DEVICE_HANDLE hDev, DWORD offset, UINT32 data);
	void VIRTEX5_DMADevicePrepare(VIRTEX5_DMA_HANDLE hDma, BOOL fIsRead, WORD wSize,
							WORD wCount, UINT32 u32Pattern, BOOL fEnable64bit, BYTE bTrafficClass);
	void VIRTEX5_WriteReg16(WDC_DEVICE_HANDLE hDev, DWORD offset, WORD data);
	DWORD VIRTEX5_IntEnable(WDC_DEVICE_HANDLE hDev, VIRTEX5_INT_HANDLER funcIntHandler);

	void SetScannerParams(WDC_DEVICE_HANDLE hDev, BYTE fovH, BYTE fovV, unsigned short vDivider, unsigned short vOffset, BYTE backward, BOOL doubleLine);
	void LoadSignalBuffer(WDC_DEVICE_HANDLE hDev, unsigned short length, unsigned short *buffer);
	void ApplyTrackingPos(WDC_DEVICE_HANDLE hDev, short PosX, short PosY, BOOL rotate45);
	void UpdateSlowScanner(WDC_DEVICE_HANDLE hDev, BOOL bZeroBackward);
	void SetSymmetricRamp(WDC_DEVICE_HANDLE hDev, BOOL bSymmetric);
	void SetSigFreq(WDC_DEVICE_HANDLE hDev, float freq);
	void EnableMirrorTracking(WDC_DEVICE_HANDLE hDev, BOOL bSymmetric);
	void ApplySteeringX(WDC_DEVICE_HANDLE hDev, short PosX);
	void AOShutter(WDC_DEVICE_HANDLE hDev, BOOL bFlagOn);
	void UpdatePower488(WDC_DEVICE_HANDLE hDev, BYTE power);
	void UpdatePower568(WDC_DEVICE_HANDLE hDev, unsigned short power);
	void UpdatePower796(WDC_DEVICE_HANDLE hDev, BYTE power);
	void CalibratePower488(WDC_DEVICE_HANDLE hDev, BOOL flag);
	void CalibratePower568(WDC_DEVICE_HANDLE hDev, BOOL flag);
	void CalibratePower796(WDC_DEVICE_HANDLE hDev, BOOL flag);
	void AppLaserModulation(WDC_DEVICE_HANDLE hDev, float *buffer, int length);
	void SwitchShutter(WDC_DEVICE_HANDLE hDev, BOOL flag);

	char   *GetErrMsg();
	DWORD   GetErrCode();
};

#endif