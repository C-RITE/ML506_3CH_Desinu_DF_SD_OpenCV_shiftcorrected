// ML506Doc.cpp : implementation of the CML506Doc class
//

#include "stdafx.h"
#include "ML506.h"
#include "Afxmt.h"

#include "ML506Doc.h"
#include "math.h"
#include "StabFFT.h"
#include "CalDesinu.h"


#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

const BYTE STIMULUS_SIZE_X = 15;
const BYTE STIMULUS_SIZE_Y = 15;

#define  FFT_SIZE_X     512
#define  FFT_SIZE_Y     256


/////////////////////////////////////////////////////////////////////////////
// CML506Doc

CVirtex5BMD       g_objVirtex5BMD;
WDC_DEVICE_HANDLE g_hDevVirtex5;
CStabFFT          g_objXCorr;

BOOL              g_bDCFsaved;
BOOL              g_bDCFloaded;
BOOL              g_bGrabStart;
BOOL              g_bFirstVsync;
DWORD             g_dwTotalCount;
DIAG_DMA          g_dma;
VIDEO_INFO        g_VideoInfo;

CString           g_dataReceived;
CView							*g_viewMsgVideo;
//CView            *g_viewVideo;
CView            *g_viewMain;

FILE             *g_fp;
LARGE_INTEGER     g_ticksPerSecond;
BOOL              g_bSOF;
POINT             g_stimulus;

HANDLE            g_EventLoadStim;
HANDLE            g_EventLoadLUT;
HANDLE            g_EventEOF;

double            g_CurrentTime;
double            g_PreviousTime;

BOOL			  g_bDesinusoid;
DesinusoidParams	g_desinusoidParams;

BYTE			  *g_imgBuffDF;
BYTE			  *g_imgBuffSD;
BOOL			   g_bSampleSW;


// save 8-bit grayscale BMP file
int g_Save8bitBitmap(int imgW, int imgH, BYTE* image, char *bmpName)
{
	FILE *fp;

	int i, offSize;
	offSize = 54 + sizeof(RGBQUAD)*256;
	int filesize = offSize + imgW*imgH;  //w is your image width, h is image height, both int

	unsigned char bmpfileheader[14] = {'B','M', 0,0,0,0, 0,0, 0,0, 54,0,0,0};
	unsigned char bmpinfoheader[40] = {40,0,0,0, 0,0,0,0, 0,0,0,0, 1,0,     8,0,       0,0,0,0,      0,0,0,0,    0,0,0,0,        0,0,0,0,        0,1,0,0,  0,1,0,0};
//	                                 biSize    biWidth  biHeight biPlanes biBitCount biCompression biSizeImage biXPelsPerMeter biYPelsPerMeter biClrUsed biClrImportant
	unsigned char bmppad[3] = {0,0,0};
	RGBQUAD bmiColors[256];

	for (i = 0; i < 256; i ++) {
		bmiColors[i].rgbRed   = i;
		bmiColors[i].rgbBlue  = i;
		bmiColors[i].rgbGreen = i;
	}

	bmpfileheader[ 2] = (unsigned char)(filesize);
	bmpfileheader[ 3] = (unsigned char)(filesize>> 8);
	bmpfileheader[ 4] = (unsigned char)(filesize>>16);
	bmpfileheader[ 5] = (unsigned char)(filesize>>24);


	bmpfileheader[10] = (unsigned char)(offSize);
	bmpfileheader[11] = (unsigned char)(offSize>> 8);
	bmpfileheader[12] = (unsigned char)(offSize>>16);
	bmpfileheader[13] = (unsigned char)(offSize>>24);


	bmpinfoheader[ 4] = (unsigned char)(imgW);
	bmpinfoheader[ 5] = (unsigned char)(imgW>> 8);
	bmpinfoheader[ 6] = (unsigned char)(imgW>>16);
	bmpinfoheader[ 7] = (unsigned char)(imgW>>24);
	bmpinfoheader[ 8] = (unsigned char)(imgH);
	bmpinfoheader[ 9] = (unsigned char)(imgH>> 8);
	bmpinfoheader[10] = (unsigned char)(imgH>>16);
	bmpinfoheader[11] = (unsigned char)(imgH>>24);

	fp = fopen(bmpName, "wb");
	if (fp == NULL) {
		AfxMessageBox("Can't open file to write", MB_ICONEXCLAMATION);
		return -1;
	}
	fwrite(bmpfileheader, 1, 14, fp);
	fwrite(bmpinfoheader, 1, 40, fp);
	fwrite(bmiColors,     1, sizeof(RGBQUAD)*256, fp);
	for (i = 0; i < imgH; i ++)
	{
		fwrite(image+imgW*i, 1, imgW, fp);
		fwrite(bmppad, 1, (4-imgW%4)%4, fp);
	}

	fclose(fp);

	return 0;
}

void g_GetAppSystemTime(int *hours, int *minutes, int *seconds, double *milliseconds) {
	int           l_hours;
	int           l_minutes;
	int           l_seconds;
	double        l_milliseconds;
	LARGE_INTEGER l_time;
	LARGE_INTEGER l_tick;

	QueryPerformanceCounter(&l_tick);
	l_time.QuadPart = l_tick.QuadPart/g_ticksPerSecond.QuadPart;
	l_hours = (int)(l_time.QuadPart/3600);
	l_time.QuadPart = l_time.QuadPart - (l_hours * 3600);
	l_minutes = (int)(l_time.QuadPart/60);
	l_seconds = (int)(l_time.QuadPart - (l_minutes * 60));
	l_milliseconds = (double) (1000.0 * (l_tick.QuadPart % g_ticksPerSecond.QuadPart) / g_ticksPerSecond.QuadPart);

	*hours        = l_hours;
	*minutes      = l_minutes;
	*seconds      = l_seconds;
	*milliseconds = l_milliseconds;
}


void CALLBACK TimeoutTimerFunc(UINT wTimerID, UINT msg, DWORD dwUser, DWORD pParam, DWORD dw2)
//void CALLBACK TimeoutTimerFunc(UINT wTimerID, UINT msg, DWORD dwUser, LPVOID pParam, DWORD dw2)
{
	static  UINT32 timeout_cnt;

	BYTE   *imageRef, *imageTar;

	if (g_bGrabStart) {
		if (g_objVirtex5BMD.DetectSyncSignals(g_hDevVirtex5) == TRUE)
			timeout_cnt = 0;
		else
			timeout_cnt ++;

		if (timeout_cnt >= 3) {
			g_objVirtex5BMD.AppStopADC(g_hDevVirtex5);
//			g_objVirtex5BMD.DIAG_DMAClose(g_hDevVirtex5, &g_dma);

			g_bGrabStart = FALSE;

			//g_viewVideo->SendMessage(WM_MESSAGE_SEND, 0, ID_GRAB_STOP);
			//g_viewMain->SendMessage(WM_MESSAGE_SEND, 0, ID_GRAB_STOP);

			AfxMessageBox("Sync signals are lost. Video sampling is stopped.");
		}
	}
}

// front porch will be adjusted in every five seconds
void CALLBACK HsyncAdjustment(UINT wTimerID, UINT msg, DWORD dwUser, DWORD pParam, DWORD dw2)
{
	int       hours;
	int       minutes;
	int       seconds;
	double    milliseconds;
	int       deltaX, deltaY;
	static    unsigned short iCounter;
	char     *fname;

	fname = new char [120];
	if (g_bGrabStart) {
		if (g_VideoInfo.bDualChannel) {
			iCounter ++;
			g_objXCorr.GetCenterXY(g_VideoInfo.fsbs_buffer, &deltaX, &deltaY);
			g_GetAppSystemTime(&hours, &minutes, &seconds, &milliseconds);
			fprintf(g_fp, "Cross Correlation (%d, %d)			%d:%d:%d:%6.3f\n", deltaX, deltaY, hours, minutes, seconds, milliseconds);

			if (iCounter > 10) {
				iCounter = 10;
				if (abs(deltaY) <= 1 && abs(deltaX) <= 5) {
					g_VideoInfo.offset_pixel -= deltaX/2;
					g_viewMain->SendMessage(WM_MESSAGE_SEND, USER_MESSAGE_NEWOFFSET, 0);
				}
			} else {
				if (abs(deltaY) <= 1 && abs(deltaX) > 1) {
					g_VideoInfo.offset_pixel -= deltaX/2;
					g_viewMain->SendMessage(WM_MESSAGE_SEND, USER_MESSAGE_NEWOFFSET, 0);

				//	sprintf(fname, "C:\\Programs\\PC\\bmp\\raw%04d.bmp", iCounter);
				//	g_Save8bitBitmap(g_VideoInfo.img_width*2, g_VideoInfo.img_height, g_VideoInfo.fsbs_buffer, fname);

				}
			}

			if (g_VideoInfo.bDesinusoid) {
				sprintf(fname, "C:\\Programs\\PC\\bmp\\des%04d.bmp", iCounter);
			//	g_Save8bitBitmap(g_VideoInfo.img_width*2, g_VideoInfo.img_height, g_VideoInfo.desi_buffer, fname);
			}
		}
	}

	delete [] fname;
}




/*
// possibly more operations need to be added here
void g_SplitImage(BYTE *imgBuffer, int width, int height)
{
	int    j, k, idx;

	k = width>>1;
	for (j = 0; j < height; j ++) {
		idx = j*width;
		imgBuffer[idx+k-1] = 0;
	}
}
*/
void g_CalcHistogram(BYTE *imgBuffer, int width, int height, BOOL bDualCh, long *hist)
{
	int    i, j, k, idx, chIdx, lb, rb, midX, imax, offset, pixMax, pixMin;
	long   sum_sq, sum_val;
	BYTE   mval;

	if (bDualCh) {
		ZeroMemory(hist, 520*sizeof(int));
		midX = width>>1;

		// the first cycle is for forward scanning
		// the second cycle is for backward scanning
		for (chIdx = 0; chIdx < 2; chIdx ++) {
			offset = 260*chIdx;
			lb = midX*chIdx;
			rb = lb + midX;
			lb += 16;
			rb -= 16;
			sum_val = 0;

			pixMax = 0;
			pixMin = 255;
			for (j = 0; j < height; j ++) {
				idx = j*width;
				for (i = lb; i < rb; i ++) {
					k = imgBuffer[idx+i];
					if (k < 0 || k > 255) k = 0;
					hist[k+offset] ++;
					sum_val += imgBuffer[idx+i];
					pixMax = (pixMax>imgBuffer[idx+i]) ? pixMax : imgBuffer[idx+i];
					pixMin = (pixMin<imgBuffer[idx+i]) ? pixMin : imgBuffer[idx+i];
				}
			}

			hist[0] = 0;
			hist[255] = 0;

			// calculate mean
			mval = (BYTE)(sum_val*1.0/((midX-16)*height));
			hist[256+offset] = sum_val;//mval;
			hist[258+offset] = pixMax;
			hist[259+offset] = pixMin;

			imax = 0;
			for (i = 0; i < 256; i ++) {
				imax = hist[i+offset] > imax ? hist[i+offset] : imax;
			}

			sum_sq = 0;
			for (j = 0; j < height; j ++) {
				idx = j*width;
				for (i = lb; i < rb; i ++) {
					sum_sq += (imgBuffer[idx+i]-mval)*(imgBuffer[idx+i]-mval);
				}
			}
			// calculate standard deviation
			hist[257+offset] = sum_sq;//(int)(sqrt(sum_sq*1.0/((midX-16)*height-1)));

			// flip over histogram for image rendering
			for (i = 0; i < 256; i ++) {
				hist[i+offset] = (int)(220*(1.0-1.0*hist[i+offset]/imax));
			}
		}
	} else {
		ZeroMemory(hist, 260*sizeof(int));

		sum_val = 0;
		pixMax = -1024;
		pixMin = 1024;
		for (k = 0; k < height; k ++) {
			idx = k * width;
			for (i = 16; i < width-16; i ++) {
				j = imgBuffer[i+idx];
				if (j < 0 || j > 255) j = 0;
				hist[j] ++;
				sum_val += imgBuffer[i+idx];
				pixMax = (pixMax>imgBuffer[i+idx]) ? pixMax : imgBuffer[i+idx];
				pixMin = (pixMin<imgBuffer[i+idx]) ? pixMin : imgBuffer[i+idx];
			}
		}
		// calculate mean
		mval = (BYTE)(sum_val*1.0/((width-16)*height));
		hist[256] = sum_val;//mval;

		imax = 0;
		for (i = 0; i < 256; i ++) {
			imax = hist[i] > imax ? hist[i] : imax;
		}
		sum_sq = 0;
		for (k = 0; k < height; k ++) {
			idx = k * width;
			for (i = 16; i < width-16; i ++) {
				sum_sq += (imgBuffer[i+idx]-mval)*(imgBuffer[i+idx]-mval);
			}
		}
		// calculate standard deviation
		hist[257] = sum_sq;//(int)(sqrt(sum_sq*1.0/((width-16)*height-1)));
		hist[258] = pixMax;
		hist[259] = pixMin;

		// flip over histogram for image rendering
		for (i = 0; i < 256; i ++) {
			hist[i] = (int)(220*(1.0-1.0*hist[i]/imax));
		}
	}
}


void DiagDmaIntHandler(WDC_DEVICE_HANDLE hDev, VIRTEX5_INT_RESULT *pIntResult)
{
}

UINT EOFHandler(LPVOID pParam)
{
	CEvent   WaitEvents;
//	int i, dwTotalCount;
	LARGE_INTEGER l_time;
	LARGE_INTEGER l_tick;

	int       hours;
	int       minutes;
	int       seconds;
	double    milliseconds;
//	PVIRTEX5_DEV_CTX pDevCtx = (PVIRTEX5_DEV_CTX)WDC_GetDevContext(g_hDevVirtex5);

	do {
		::WaitForSingleObject( g_EventEOF, INFINITE);
		g_GetAppSystemTime(&hours, &minutes, &seconds, &milliseconds);
		g_CurrentTime = hours*3600+minutes*60+seconds+milliseconds/1000;
		g_VideoInfo.fFrameInterval = g_CurrentTime - g_PreviousTime;


		g_VideoInfo.frameCounter ++;
		if (g_VideoInfo.bVideoSaving == TRUE) {
			g_VideoInfo.nVideoCounter ++;
			//			g_viewVideo->SendMessage(WM_MESSAGE_SEND, g_VideoInfo.nVideoLength, g_VideoInfo.nVideoCounter);
			g_viewMain->SendMessage(WM_MESSAGE_SEND, USER_MESSAGE_SAVEVIDEO, g_VideoInfo.nVideoCounter);
		} else {
			//			g_viewVideo->SendMessage(WM_MESSAGE_SEND, 0, 0);
			g_viewMain->SendMessage(WM_MESSAGE_SEND, USER_MESSAGE_NEWFRAME, 0);
			g_VideoInfo.nVideoCounter = 0;
		}

		g_PreviousTime = g_CurrentTime;
	} while (g_bGrabStart);

	return 0;
}

void SavePCIeRegisters() {
	UINT32 reg;

	reg = g_objVirtex5BMD.VIRTEX5_ReadReg32(g_hDevVirtex5, VIRTEX5_DSCR_OFFSET);
	fprintf(g_fp, "VIRTEX5_DSCR_OFFSET: %08X\n", reg);
    reg = g_objVirtex5BMD.VIRTEX5_ReadReg32(g_hDevVirtex5, VIRTEX5_DDMACR_OFFSET);
	fprintf(g_fp, "VIRTEX5_DDMACR_OFFSET: %08X\n", reg);
    reg = g_objVirtex5BMD.VIRTEX5_ReadReg32(g_hDevVirtex5, VIRTEX5_WDMATLPA_OFFSET);
	fprintf(g_fp, "VIRTEX5_WDMATLPA_OFFSET: %08X\n", reg);
    reg = g_objVirtex5BMD.VIRTEX5_ReadReg32(g_hDevVirtex5, VIRTEX5_WDMATLPS_OFFSET);
	fprintf(g_fp, "VIRTEX5_WDMATLPS_OFFSET: %08X\n", reg);
    reg = g_objVirtex5BMD.VIRTEX5_ReadReg32(g_hDevVirtex5, VIRTEX5_WDMATLPC_OFFSET);
	fprintf(g_fp, "VIRTEX5_WDMATLPC_OFFSET: %08X\n", reg);
    reg = g_objVirtex5BMD.VIRTEX5_ReadReg32(g_hDevVirtex5, VIRTEX5_WDMATLPP_OFFSET);
	fprintf(g_fp, "VIRTEX5_WDMATLPP_OFFSET: %08X\n", reg);
    reg = g_objVirtex5BMD.VIRTEX5_ReadReg32(g_hDevVirtex5, VIRTEX5_RDMATLPP_OFFSET);
	fprintf(g_fp, "VIRTEX5_RDMATLPP_OFFSET: %08X\n", reg);
    reg = g_objVirtex5BMD.VIRTEX5_ReadReg32(g_hDevVirtex5, VIRTEX5_RDMATLPA_OFFSET);
	fprintf(g_fp, "VIRTEX5_RDMATLPA_OFFSET: %08X\n", reg);
    reg = g_objVirtex5BMD.VIRTEX5_ReadReg32(g_hDevVirtex5, VIRTEX5_RDMATLPS_OFFSET);
	fprintf(g_fp, "VIRTEX5_RDMATLPS_OFFSET: %08X\n", reg);
    reg = g_objVirtex5BMD.VIRTEX5_ReadReg32(g_hDevVirtex5, VIRTEX5_RDMATLPC_OFFSET);
	fprintf(g_fp, "VIRTEX5_RDMATLPC_OFFSET: %08X\n", reg);
    reg = g_objVirtex5BMD.VIRTEX5_ReadReg32(g_hDevVirtex5, VIRTEX5_PUPILMASK_OFFSET);
	fprintf(g_fp, "VIRTEX5_PUPILMASK_OFFSET: %08X\n", reg);
    reg = g_objVirtex5BMD.VIRTEX5_ReadReg32(g_hDevVirtex5, VIRTEX5_TCABOX_OFFSET);
	fprintf(g_fp, "VIRTEX5_TCABOX_OFFSET: %08X\n", reg);
    reg = g_objVirtex5BMD.VIRTEX5_ReadReg32(g_hDevVirtex5, VIRTEX5_RDMASTAT_OFFSET);
	fprintf(g_fp, "VIRTEX5_RDMASTAT_OFFSET: %08X\n", reg);
    reg = g_objVirtex5BMD.VIRTEX5_ReadReg32(g_hDevVirtex5, VIRTEX5_LASER_POWER2);
	fprintf(g_fp, "VIRTEX5_LASER_POWER2: %08X\n", reg);
    reg = g_objVirtex5BMD.VIRTEX5_ReadReg32(g_hDevVirtex5, VIRTEX5_STIMULUS_X_BOUND2);
	fprintf(g_fp, "VIRTEX5_STIMULUS_X_BOUND2: %08X\n", reg);
    reg = g_objVirtex5BMD.VIRTEX5_ReadReg32(g_hDevVirtex5, VIRTEX5_DLWSTAT_OFFSET);
	fprintf(g_fp, "VIRTEX5_DLWSTAT_OFFSET: %08X\n", reg);
    reg = g_objVirtex5BMD.VIRTEX5_ReadReg32(g_hDevVirtex5, VIRTEX5_DLTRSSTAT_OFFSET);
	fprintf(g_fp, "VIRTEX5_DLTRSSTAT_OFFSET: %08X\n", reg);
    reg = g_objVirtex5BMD.VIRTEX5_ReadReg32(g_hDevVirtex5, VIRTEX5_DMISCCONT_OFFSET);
	fprintf(g_fp, "VIRTEX5_DMISCCONT_OFFSET: %08X\n", reg);
    reg = g_objVirtex5BMD.VIRTEX5_ReadReg32(g_hDevVirtex5, VIRTEX5_I2CINFO_OFFSET);
	fprintf(g_fp, "VIRTEX5_I2CINFO_OFFSET: %08X\n", reg);
    reg = g_objVirtex5BMD.VIRTEX5_ReadReg32(g_hDevVirtex5, VIRTEX5_LINEINFO_OFFSET);
	fprintf(g_fp, "VIRTEX5_LINEINFO_OFFSET: %08X\n", reg);
    reg = g_objVirtex5BMD.VIRTEX5_ReadReg32(g_hDevVirtex5, VIRTEX5_IMAGESIZE_OFFSET);
	fprintf(g_fp, "VIRTEX5_IMAGESIZE_OFFSET: %08X\n", reg);
    reg = g_objVirtex5BMD.VIRTEX5_ReadReg32(g_hDevVirtex5, VIRTEX5_IMAGEOS_OFFSET);
	fprintf(g_fp, "VIRTEX5_IMAGEOS_OFFSET: %08X\n", reg);
    reg = g_objVirtex5BMD.VIRTEX5_ReadReg32(g_hDevVirtex5, VIRTEX5_LINECTRL_OFFSET);
	fprintf(g_fp, "VIRTEX5_LINECTRL_OFFSET: %08X\n", reg);
    reg = g_objVirtex5BMD.VIRTEX5_ReadReg32(g_hDevVirtex5, VIRTEX5_STIMULUS_LOC);
	fprintf(g_fp, "VIRTEX5_STIMULUS_LOC: %08X\n", reg);
    reg = g_objVirtex5BMD.VIRTEX5_ReadReg32(g_hDevVirtex5, VIRTEX5_LASER_POWER);
	fprintf(g_fp, "VIRTEX5_LASER_POWER: %08X\n", reg);
    reg = g_objVirtex5BMD.VIRTEX5_ReadReg32(g_hDevVirtex5, VIRTEX5_STIM_ADDRESS);
	fprintf(g_fp, "VIRTEX5_STIM_ADDRESS: %08X\n", reg);
    reg = g_objVirtex5BMD.VIRTEX5_ReadReg32(g_hDevVirtex5, VIRTEX5_STIM_DATA);
	fprintf(g_fp, "VIRTEX5_STIM_DATA: %08X\n", reg);
    reg = g_objVirtex5BMD.VIRTEX5_ReadReg32(g_hDevVirtex5, VIRTEX5_STIMULUS_X_BOUND);
	fprintf(g_fp, "VIRTEX5_STIMULUS_X_BOUND: %08X\n", reg);
}

UINT g_GrabVideo(LPVOID pParam)
{
	CEvent    WaitForEvents;
    DWORD     dwStatus, dwOptions;
    UINT32    vsync_bit, ddmacr, u32Pattern = 0xfeedbeef;
    WORD      wSize, wCount;
    BOOL      fIsRead;
    BOOL      fEnable64bit;
    BYTE      bTrafficClass;
	CString   msg;
/*
	int       hours;
	int       minutes;
	int       seconds;
	double    milliseconds;
*/
	g_VideoInfo.nVideoCounter = 0;

    // Get input for user
    wCount    = g_VideoInfo.tlp_counts;
    fIsRead   = FALSE;
	//dwOptions = DMA_FROM_DEVICE;
	dwOptions = DMA_TO_FROM_DEVICE;

	// Get the max payload size from the device
    wSize     = g_objVirtex5BMD.DMAGetMaxPacketSize(g_hDevVirtex5, fIsRead) / sizeof(UINT32);
    g_dwTotalCount = (DWORD)wCount * (DWORD)wSize;


	g_objVirtex5BMD.DIAG_DMAClose(g_hDevVirtex5, &g_dma);
    // Open DMA handle
	BZERO(g_dma);
    dwStatus = g_objVirtex5BMD.VIRTEX5_DMAOpen(g_hDevVirtex5, &g_dma.pBuf, dwOptions, g_dwTotalCount*sizeof(UINT32)*3, &g_dma.hDma);

    if (WD_STATUS_SUCCESS != dwStatus)
    {
		msg.Format("Failed to open DMA handle. Error 0x%lx", dwStatus);
        AfxMessageBox(msg);
        return -1;
    }

    fEnable64bit = FALSE;
    bTrafficClass = 0;
    g_objVirtex5BMD.VIRTEX5_DMADevicePrepare(g_dma.hDma, fIsRead, wSize, wCount, u32Pattern, fEnable64bit, bTrafficClass);

    // Enable DMA interrupts (if not polling)
    g_objVirtex5BMD.VIRTEX5_DmaIntEnable(g_hDevVirtex5, fIsRead);

    if (!g_objVirtex5BMD.VIRTEX5_IntIsEnabled(g_hDevVirtex5))
    {
        dwStatus = g_objVirtex5BMD.VIRTEX5_IntEnable(g_hDevVirtex5, DiagDmaIntHandler);

        if (WD_STATUS_SUCCESS != dwStatus)
        {
            msg.Format("Failed enabling DMA interrupts. Error 0x%lx", dwStatus);
			AfxMessageBox(msg);
            return -1;
        }
    }

	dwStatus = WDC_DMASyncCpu(g_dma.hDma->pDma);

	// Start sampling
	ddmacr = g_objVirtex5BMD.VIRTEX5_ReadReg32(g_dma.hDma->hDev, VIRTEX5_LINECTRL_OFFSET);
	ddmacr |= BIT0;
    g_objVirtex5BMD.VIRTEX5_WriteReg32(g_hDevVirtex5, VIRTEX5_LINECTRL_OFFSET, ddmacr);

	g_fp = fopen("data.txt", "w");

//	SavePCIeRegisters();

	g_objVirtex5BMD.AOShutter(g_hDevVirtex5, TRUE);

	g_VideoInfo.frameCounter = 0;
	do {
		::WaitForSingleObject(WaitForEvents, 10);
	} while (g_bGrabStart);

	g_objVirtex5BMD.AOShutter(g_hDevVirtex5, FALSE);

	// stop sampling
	ddmacr = g_objVirtex5BMD.VIRTEX5_ReadReg32(g_dma.hDma->hDev, VIRTEX5_LINECTRL_OFFSET);
	ddmacr |= BIT0;
	ddmacr -= BIT0;
    g_objVirtex5BMD.VIRTEX5_WriteReg32(g_hDevVirtex5, VIRTEX5_LINECTRL_OFFSET, ddmacr);

//	g_objVirtex5BMD.DIAG_DMAClose(g_hDevVirtex5, &g_dma);

	fclose(g_fp);
/*

	int *hist, width;
	if (g_VideoInfo.bDualChannel) {
		hist = new int [520];
		width = 2*g_VideoInfo.img_width;
		ZeroMemory(hist, 520*sizeof(int));
	} else {
		hist = new int [260];
		width = g_VideoInfo.img_width;
		ZeroMemory(hist, 260*sizeof(int));
	}
	g_CalcHistogram(g_VideoInfo.video_out, width, g_VideoInfo.img_height, g_VideoInfo.bDualChannel, hist);

	delete [] hist;

	char *fname;
	fname = new char [80];
	strcpy(fname, "e:\\bs.bmp");
	g_Save8bitBitmap(width, g_VideoInfo.img_height, g_VideoInfo.video_out, fname);
	delete [] fname;
*/


	return 0;
}



DWORD WINAPI CML506Doc::ThreadLoadData2FPGA(LPVOID pParam)
{
	HANDLE fpgaHandles[2];
	unsigned short  *lut_loc_buf;
	int    i, deltax, nx;

	fpgaHandles[0] = g_EventLoadStim;
	fpgaHandles[1] = g_EventLoadLUT;

	do {
		// waiting for event
		switch (::MsgWaitForMultipleObjects(2, fpgaHandles, FALSE, INFINITE, QS_ALLEVENTS)) {
		case WAIT_OBJECT_0:
			g_objVirtex5BMD.AppLoadStimulus(g_hDevVirtex5, g_VideoInfo.stim_buffer, g_VideoInfo.stim_nx, g_VideoInfo.stim_ny, STIM_CHALL);
			break;
		case WAIT_OBJECT_0+1:
			// load lookup table for warping stimulus pattern
			nx = g_VideoInfo.stim_nx;
			lut_loc_buf = new unsigned short [nx];
			deltax = nx;

			for (i = 0; i < nx; i ++) lut_loc_buf[i] = i;

			// upload warp LUT for Red stimulus pattern
			g_objVirtex5BMD.AppWriteSinusoidLUT(g_hDevVirtex5, nx, deltax, lut_loc_buf, STIM_CHALL);

			delete [] lut_loc_buf;

			break;

		default:
			break;
		}
	} while (true);

	CloseHandle(g_EventLoadStim);
	CloseHandle(g_EventLoadLUT);

	return 0;
}

DWORD WINAPI CML506Doc::ThreadNetMsgProcess(LPVOID pParam)
{
	CML506Doc *parent = (CML506Doc *)pParam;
	CButton *wnd;
	float fGain = -1.;
	int i,len,ind, dewarp_sx, dewarp_sy;
	CString msg, initials, ext, folder;
	char command;
	char seps[] = "\t", seps1[] = ","; //for parsing matlab sequence
	BOOL bUpdate, bLoop, bTrigger;

	while(TRUE){
		switch(::WaitForMultipleObjects(3, parent->m_eNetMsg, FALSE, INFINITE)) {//Process the message
		case WAIT_OBJECT_0: //AO message
			msg = parent->m_strNetRecBuff[0];
			command = msg[0];
			msg = msg.Right(msg.GetLength()-1);
			switch (command) {
				/* Not sure if still need to accept commands from AO but this is copy
				case 'C': //Create directory with new prefix
					initials = msg.Left(msg.Find("\\",0)); //gives the prefix
					g_viewMsgVideo->SetDlgItemText(IDC_EDITBOX_PREFIX, initials);
					CreateDirectory((g_ML506Params.VideoFolderPath+initials), NULL);
					parent->m_VideoFolder = g_ML506Params.VideoFolderPath +msg;
					CreateDirectory(parent->m_VideoFolder, NULL);
					initials.Empty();
					break;
				case 'G':
					ind = msg.ReverseFind('\\');
					g_viewMsgVideo->SetDlgItemText(IDC_EDITBOX_VIDEOLEN, msg.Right(msg.GetLength()-(ind+1)));
					msg = msg.Left(msg.GetLength() - (msg.GetLength()-ind));
					ind = msg.ReverseFind('\\');
					parent->m_videoFileName = msg.Right(msg.GetLength()-(ind+1));
					ind = parent->m_videoFileName.ReverseFind(_T('_V'));
					parent->m_bExtCtrl = TRUE;
					parent->m_nVideoNum = atoi(parent->m_videoFileName.Right(parent->m_videoFileName.GetLength()-(ind+2)));
					g_viewMsgVideo->PostMessage(WM_MOVIE_SEND, 0, SAVE_VIDEO_FLAG);
					break;
				case 'P':
					msg = g_ML506Params.VideoFolderPath + msg;
					g_viewMsgVideo->SetDlgItemText(IDL_VIDEO_FILENAME, msg);
					parent->PlaybackMovie(LPCTSTR(msg));
					break;
				case 'D':
					//write_ScreenText(m_CFtxtfont2,m_CRDefocusValue,text,RGB(255,255,0));
					break;
				case 'F':
					if (parent->m_bPlayback == TRUE)
						g_AnimateCtrl->Seek(atoi(msg));
					break;
				case 'U': //start stabilization
					if (parent->m_bPlayback == TRUE)
						parent->StopPlayback();
					if (parent->m_bCameraConnected == FALSE)
						parent->OnCameraConnect();
					g_viewMsgVideo->SendMessage(WM_MOVIE_SEND, 0, STABILIZATION_GO);
					break;
				case 'O': //stop stabilization
					parent->OnStablizeSuspend();
					break;
				case 'E': //reset reference frame
					g_viewMsgVideo->SendMessage(WM_MOVIE_SEND, 0, STABILIZATION_GO);
					break;
				default:
					break;*/
				
			break;
		}
		case WAIT_OBJECT_0 + 1: // getting remote controlled by IGUIDE
			msg = parent->m_strNetRecBuff[2];
			command = msg[0];
			msg = msg.Right(msg.GetLength()-1);
			switch (command) {

				case 'V': //record video
					g_viewMain->PostMessage(WM_MESSAGE_SEND, IGUIDE_MESSAGE_SAVE,0);
				break;
		}
	}
	msg.Empty();
	}
}


IMPLEMENT_DYNCREATE(CML506Doc, CDocument)

BEGIN_MESSAGE_MAP(CML506Doc, CDocument)
	//{{AFX_MSG_MAP(CML506Doc)
	ON_COMMAND(ID_CAMERA_CONNECT, OnCameraConnect)
	ON_COMMAND(ID_CAMERA_DISCONNECT, OnCameraDisconnect)
	ON_UPDATE_COMMAND_UI(ID_CAMERA_CONNECT, OnUpdateCameraConnect)
	ON_UPDATE_COMMAND_UI(ID_CAMERA_DISCONNECT, OnUpdateCameraDisconnect)
	ON_COMMAND(ID_DESINUSOID, OnSetupDesinusoid)
	ON_UPDATE_COMMAND_UI(ID_DESINUSOID, OnUpdateSetupDesinusoid)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CML506Doc construction/destruction

CML506Doc::CML506Doc()
{
	// setup parameters for the fast scanner and the slow scanner
	unsigned short vDivider, length, turnpt, *scannerBuf, vOffset;
	BYTE fovH, fovV, stepSize;
	BOOL twoLines;

	fovH     = 91;	//0xff;
	fovV     = 0x06;	// amplitude
	vDivider = 606;//414;//208; // duty cycle on 0x10;		// the 14-bit DAC shall output 30Hz ramp signal
	stepSize = 202;// 138;	//duty cycle off
	twoLines = FALSE;		// no interlace

	// force the smallest of the ramp signal to 0
	if (twoLines)
		vOffset  = (int)(1.0*fovV*vDivider/16+0.5);
	else
		vOffset  = (int)(1.0*fovV*vDivider/32+0.5);
	vOffset = 511;		// set ramp offet to the middle range of volage +0.5V

	// setup other parameters
	g_bGrabStart      = FALSE;
	g_bSOF            = FALSE;
	//g_bEOFHandler     = FALSE;
	g_bDCFloaded      = FALSE;
	g_bDCFsaved       = TRUE;
	m_bSymbol         = FALSE;

	g_VideoInfo.DesinusoidLen   = 0;
	g_VideoInfo.LeftIndex       = NULL;
	g_VideoInfo.RightIndex      = NULL;
	g_VideoInfo.LeftWeigh       = NULL;
	g_VideoInfo.RightWeigh      = NULL;

	g_VideoInfo.bDesinusoid     = FALSE;
	g_VideoInfo.bDualChannel    = FALSE;
	g_VideoInfo.fFrameInterval  = 1.0;
	g_VideoInfo.bVideoSaving    = FALSE;
	g_VideoInfo.nVideoLength    = 0;
	g_VideoInfo.nVideoCounter   = 0;
	g_VideoInfo.end_line_ID     = 16;		// update this variable dynamically
	g_VideoInfo.img_width       = 520;//1032;//
	g_VideoInfo.img_height      = 496;//384;//(vDivider/16)*16;		// round image height to a factor of 16
	g_VideoInfo.line_spacing    = 16;
	g_VideoInfo.line_start_addr = 0;
	g_VideoInfo.line_end_addr   = 0;				// update this variable dynamically
	g_VideoInfo.addr_interval   = 1616;//2064;//
	g_VideoInfo.offset_line     = 0x06;//0x20;
	g_VideoInfo.offset_pixel    = 0x58;//0xb9
	g_VideoInfo.tlp_counts      = 105;//132;//
	g_VideoInfo.stim_nx         = STIMULUS_SIZE_X;
	g_VideoInfo.stim_ny         = STIMULUS_SIZE_Y;
	g_VideoInfo.video_in        = new unsigned char [2048*2048];
	g_VideoInfo.video_out       = new unsigned char [2048*2048];
	g_VideoInfo.fsbs_buffer     = new unsigned char [2048*1024];
	g_VideoInfo.desi_buffer     = new unsigned char [2048*1024];
	g_VideoInfo.stim_buffer     = new unsigned short [g_VideoInfo.img_width*g_VideoInfo.img_height];
	if (g_VideoInfo.video_in == NULL || g_VideoInfo.video_out == NULL || g_VideoInfo.stim_buffer == NULL)
		AfxMessageBox("Error! Can't allocate memory space for video.");
	g_VideoInfo.weightsRed      = new UINT32 [1024];
	g_VideoInfo.weightsIR       = new UINT32 [1024];

	DWORD dwStatus, dwResult;
	dwStatus = WDC_CallKerPlug(g_hDevVirtex5, KP_VRTX5_MSG_DATA, &g_VideoInfo, &dwResult);

	int x, y;
	x = FFT_SIZE_X;
	y = FFT_SIZE_Y;
	g_objXCorr.ParamsInit(x, y, g_VideoInfo.img_width, g_VideoInfo.img_height, 0.0f);

	// write a temporary stimulus pattern with 17x17 pixels square
	UINT32 regData1, regData2;
	//regData1 = STIMULUS_SIZE_X;
	regData1 = STIMULUS_SIZE_X/2;
	regData1 = (regData1 << 2) + BIT0 + BIT1;
	g_objVirtex5BMD.VIRTEX5_WriteReg32(g_hDevVirtex5, VIRTEX5_STIMULUS_LOC, regData1);
	regData2 = 0x0000ffff;
	g_objVirtex5BMD.VIRTEX5_WriteReg32(g_hDevVirtex5, VIRTEX5_LASER_POWER,  regData2);
	regData1 = MODULATION_SHIFT;
	regData2 = (regData1<<24) + 0x0000ffff;
	g_objVirtex5BMD.VIRTEX5_WriteReg32(g_hDevVirtex5, VIRTEX5_LASER_POWER2,  regData2);
	g_objVirtex5BMD.VIRTEX5_WriteReg32(g_hDevVirtex5, VIRTEX5_PUPILMASK_OFFSET, 0);
	g_objVirtex5BMD.VIRTEX5_WriteReg32(g_hDevVirtex5, VIRTEX5_TCABOX_OFFSET, 0);
	g_objVirtex5BMD.VIRTEX5_WriteReg32(g_hDevVirtex5, VIRTEX5_IMAGEOS_OFFSET, 0);
	g_objVirtex5BMD.VIRTEX5_WriteReg32(g_hDevVirtex5, VIRTEX5_LINECTRL_OFFSET, 0);

	regData1 = 0;
	g_objVirtex5BMD.VIRTEX5_WriteReg32(g_hDevVirtex5, VIRTEX5_STIM_ADDRESS, regData1);		// bit 15 controls IR stimulus, bit 14 controls stimulus pattern write/read
	g_objVirtex5BMD.VIRTEX5_WriteReg32(g_hDevVirtex5, VIRTEX5_STIM_DATA, 0);
	g_objVirtex5BMD.VIRTEX5_WriteReg32(g_hDevVirtex5, VIRTEX5_STIMULUS_X_BOUND, BIT3+BIT11+BIT19+BIT27);
	g_objVirtex5BMD.VIRTEX5_WriteReg32(g_hDevVirtex5, VIRTEX5_STIMULUS_X_BOUND2, 0);

	// this is default stimulus pattern
	unsigned short *stim_buffer;
	stim_buffer = new unsigned short [g_VideoInfo.stim_nx*g_VideoInfo.stim_ny];
	int i;
	for (i = 0; i < g_VideoInfo.stim_nx*g_VideoInfo.stim_ny; i ++) stim_buffer[i] = 0x1fff;
	g_objVirtex5BMD.AppLoadStimulus(g_hDevVirtex5, stim_buffer, g_VideoInfo.stim_nx, g_VideoInfo.stim_ny, STIM_CHALL);
	delete [] stim_buffer;

	// write default LUT for warping stimulus pattern
	unsigned short *lut_buf;
	lut_buf = new unsigned short [g_VideoInfo.stim_nx];
	for (i = 0; i < g_VideoInfo.stim_nx; i ++) lut_buf[i] = i;
	g_objVirtex5BMD.AppWriteSinusoidLUT(g_hDevVirtex5, g_VideoInfo.stim_nx, g_VideoInfo.stim_nx, lut_buf, STIM_CHALL);
	delete [] lut_buf;

	int stripH, channelID;
	stripH    = -1;
	channelID = STIM_CH796;
	g_objVirtex5BMD.AppWriteStimScaleY(g_hDevVirtex5, 0, g_VideoInfo.img_height, 0, 0, stripH, channelID);
	g_objVirtex5BMD.AppWriteStimLocX(g_hDevVirtex5, 0, 0, 0, 0, g_VideoInfo.img_width, g_VideoInfo.img_height, 0, 0, channelID);


	g_EventLoadStim = CreateEvent(NULL, FALSE, FALSE, "GUI_LOADSTIM_EVENT");
	if (!g_EventLoadStim) {
		AfxMessageBox("Failed to create an event for loading stimulus", MB_ICONEXCLAMATION);
	}
	g_EventLoadLUT = CreateEvent(NULL, FALSE, FALSE, "GUI_LOADLUT_EVENT");
	if (!g_EventLoadLUT) {
		AfxMessageBox("Failed to create an event for loading lookup table for warping stimulus pattern", MB_ICONEXCLAMATION);
	}
	g_EventEOF = CreateEvent(NULL, FALSE, FALSE, "GUI_EOF_EVENT");
	if (!g_EventEOF) {
		AfxMessageBox("Failed to create an event for EOF", MB_ICONEXCLAMATION);
	}

//	thd_handle = CreateThread(NULL, 0, ThreadLoadData2FPGA, this, 0, &thdid_handle);
//	SetThreadPriority(thd_handle, THREAD_PRIORITY_LOWEST);

	UINT32  power_rd, power_ir, nw1, nw2, reg;
	power_rd = 0x3fff;
	power_ir = 0x3fff;

	for (i = 0; i < g_VideoInfo.stim_nx; i ++) {
		nw1 = power_rd;// >> 1;
		nw2 = nw1 << 16;
		g_VideoInfo.weightsRed[i] = nw1 | nw2;
		nw1 = power_ir;// >> 1;
		nw2 = nw1 << 16;
		g_VideoInfo.weightsIR[i]  = nw1 | nw2;
	}

	// create a timeout timer for video sampling
    TIMECAPS tc;
    timeGetDevCaps(&tc, sizeof(TIMECAPS));
    m_uResolution = min(max(tc.wPeriodMin, 0), tc.wPeriodMax);
    timeBeginPeriod(m_nTimerRes);

    // create the timer
    m_idTimerEvent1 = timeSetEvent(500, m_nTimerRes, TimeoutTimerFunc, (DWORD)this, TIME_PERIODIC);
//	m_idTimerEvent2 = timeSetEvent(5000, m_nTimerRes, HsyncAdjustment, (DWORD)this, TIME_PERIODIC);


	//reset to forward scanning data only
	reg = g_objVirtex5BMD.VIRTEX5_ReadReg32(g_hDevVirtex5, VIRTEX5_LINECTRL_OFFSET);
	reg = reg | BIT30;
	reg = reg - BIT30;
	g_objVirtex5BMD.VIRTEX5_WriteReg32(g_hDevVirtex5, VIRTEX5_LINECTRL_OFFSET, reg);

//return;

	g_objVirtex5BMD.SetScannerParams(g_hDevVirtex5, fovH, fovV,  vDivider, vOffset, stepSize, twoLines);

	g_VideoInfo.fovH           = fovH;
	g_VideoInfo.fovV           = fovV;
	g_VideoInfo.fLines         = vDivider;
	g_VideoInfo.bStepSize      = stepSize;
	g_VideoInfo.vOffset        = vOffset;
	g_VideoInfo.bDoubleLine    = twoLines;
	g_VideoInfo.bZeroBackward  = FALSE;
	g_VideoInfo.bSymmetricRamp = FALSE;

	g_bDesinusoid = FALSE;
	g_desinusoidParams.UnMatrixIdx = NULL;
	g_desinusoidParams.UnMatrixVal = NULL;

	g_imgBuffDF = NULL;
	g_imgBuffSD = NULL;
	g_bSampleSW = FALSE;
	
	//Enable network message processing thread and create network listening sockets for AO and Matlab
	m_eNetMsg = new HANDLE[2];
	m_eNetMsg[0] = CreateEvent(NULL, FALSE, FALSE, "ICANDI_GUI_NETCOMMMSGAO_EVENT");
	if (!m_eNetMsg[0]) {
		AfxMessageBox("Failed to create an event for monitoring AO Comm", MB_ICONEXCLAMATION);
		return;
	}
	m_eNetMsg[2] = CreateEvent(NULL, FALSE, FALSE, "ICANDI_GUI_NETCOMMMSGIGUIDE_EVENT");
	if (!m_eNetMsg[1]) {
		AfxMessageBox("Failed to create an event for monitoring IGUIDE Comm", MB_ICONEXCLAMATION);
		return;
	}

	m_strNetRecBuff = new CString[2];
	m_ncListener_AO = NULL;
	m_ncListener_IGUIDE = NULL;

	if (CSockClient::SocketInit() != 0)
	{
		AfxMessageBox("Unable to initialize Windows Sockets", MB_OK|MB_ICONERROR, 0);
		return;
	}
	//Create a listener for AO
	m_ncListener_AO = new CSockListener(&m_strNetRecBuff[0], &m_eNetMsg[0]);
	if (!m_ncListener_AO->InitPort("10.7.216.212", 23))
	//if (!m_ncListener_AO->InitPort("153.90.109.35", 23))
	{
	//	AfxMessageBox("Unable to Open port 23 for AO comm", MB_OK|MB_ICONERROR, 0);
	//	return;
	}
	else if (!m_ncListener_AO->Listen())
	{
	//	AfxMessageBox("Unable to Listen on port 23 for AO comm", MB_OK|MB_ICONERROR, 0);
	//	return;
	}
	//Create a listener for IGUIDE
	m_ncListener_IGUIDE = new CSockListener(&m_strNetRecBuff[2], &m_eNetMsg[1]);
	if (!m_ncListener_IGUIDE->InitPort("127.0.0.1", 1400))
	{
		AfxMessageBox("Unable to Open port 1400 for IGUIDE comm", MB_OK|MB_ICONERROR, 0);
	//	return;
	}
	else if (!m_ncListener_IGUIDE->Listen())
	{
		AfxMessageBox("Unable to Listen on port 1400 for IGUIDE comm", MB_OK|MB_ICONERROR, 0);
	//	return;
	}

	thd_handle[0] = CreateThread(NULL, 0, ThreadNetMsgProcess, this, 0, &thdid_handle[0]);
	SetThreadPriority(thd_handle[0], THREAD_PRIORITY_NORMAL);
}

CML506Doc::~CML506Doc()
{
	g_bDesinusoid = FALSE;
	delete [] g_VideoInfo.video_in;
	delete [] g_VideoInfo.video_out;
	delete [] g_VideoInfo.fsbs_buffer;
	delete [] g_VideoInfo.desi_buffer;
	delete [] g_VideoInfo.stim_buffer;
	delete [] g_VideoInfo.weightsRed;
	delete [] g_VideoInfo.weightsIR;

	if (g_VideoInfo.LeftIndex  != NULL) delete [] g_VideoInfo.LeftIndex;
	if (g_VideoInfo.RightIndex != NULL) delete [] g_VideoInfo.RightIndex;
	if (g_VideoInfo.LeftWeigh  != NULL) delete [] g_VideoInfo.LeftWeigh;
	if (g_VideoInfo.RightWeigh != NULL) delete [] g_VideoInfo.RightWeigh;

	g_objVirtex5BMD.VIRTEX5_WriteReg32(g_hDevVirtex5, VIRTEX5_STIMULUS_LOC, 0);

    timeKillEvent(m_idTimerEvent1);
	timeKillEvent(m_idTimerEvent2);
    // reset the timer
    timeEndPeriod (m_uResolution);

	if (g_desinusoidParams.UnMatrixIdx != NULL) delete [] g_desinusoidParams.UnMatrixIdx;
	if (g_desinusoidParams.UnMatrixVal != NULL) delete [] g_desinusoidParams.UnMatrixVal;

	if (g_imgBuffDF != NULL) {
		delete [] g_imgBuffDF;
		delete [] g_imgBuffSD;
	}
	delete m_ncListener_AO;
	delete m_ncListener_IGUIDE;
	CloseHandle(m_eNetMsg[0]);
	CloseHandle(m_eNetMsg[1]);
	delete [] m_eNetMsg;

	//AfxMessageBox("Exit: CML506Doc::~CML506Doc()");

	g_objVirtex5BMD.DIAG_DMAClose(g_hDevVirtex5, &g_dma);

}

BOOL CML506Doc::OnNewDocument()
{
	if (!CDocument::OnNewDocument())
		return FALSE;

	// TODO: add reinitialization code here
	// (SDI documents will reuse this document)

	return TRUE;
}



/////////////////////////////////////////////////////////////////////////////
// CML506Doc serialization

void CML506Doc::Serialize(CArchive& ar)
{
	if (ar.IsStoring())
	{
		// TODO: add storing code here
	}
	else
	{
		// TODO: add loading code here
	}
}

/////////////////////////////////////////////////////////////////////////////
// CML506Doc diagnostics

#ifdef _DEBUG
void CML506Doc::AssertValid() const
{
	CDocument::AssertValid();
}

void CML506Doc::Dump(CDumpContext& dc) const
{
	CDocument::Dump(dc);
}
#endif //_DEBUG

/////////////////////////////////////////////////////////////////////////////
// CML506Doc commands

void CML506Doc::OnCameraConnect()
{
//	POINT  point;
	int    x1, x2, stripH, channelID;
//	point.x = 310;
//	point.y = 256;
//	g_stimulus.x = point.x;
//	g_stimulus.y = point.y;

//	x1 = x2 = g_VideoInfo.stim_nx/2;
/*	stripH    = -1;
	channelID = STIM_CHALL;
	g_objVirtex5BMD.AppWriteStimScaleY(g_hDevVirtex5, 0, g_VideoInfo.img_height, 0, 0, stripH, channelID);
	g_objVirtex5BMD.AppWriteStimLocX(g_hDevVirtex5, 0, 0, 0, 0, g_VideoInfo.img_width, g_VideoInfo.img_height, 0, 0, channelID);*/
//	g_objVirtex5BMD.AppWriteStimScaleY(g_hDevVirtex5, point.y-g_VideoInfo.stim_ny/2, point.y+g_VideoInfo.stim_ny/2, 0, g_VideoInfo.stim_nx, stripH, channelID);
//	g_objVirtex5BMD.AppWriteStimLocX(g_hDevVirtex5, 0, point.x, point.x, point.y-g_VideoInfo.stim_ny/2, point.y+g_VideoInfo.stim_ny/2, 0, x1, x2, channelID);

//	g_objVirtex5BMD.AppWriteStimLoc(g_hDevVirtex5, point.x, point.y);

	// CHECK if there is valid h-sync and v-sync.
	if (g_objVirtex5BMD.DetectSyncSignals(g_hDevVirtex5) == FALSE) {
		AfxMessageBox("No sync signals found, sampling aborted.", MB_ICONEXCLAMATION);
		return;
	}

	// TODO: Add your command handler code here
	g_bGrabStart = TRUE;
	QueryPerformanceFrequency(&g_ticksPerSecond);

	AfxBeginThread(g_GrabVideo, THREAD_PRIORITY_NORMAL);
	AfxBeginThread(EOFHandler, THREAD_PRIORITY_NORMAL);
	UpdateAllViews(NULL, USER_MESSAGE_CONNECT, NULL);
}

void CML506Doc::OnCameraDisconnect()
{
	// TODO: Add your command handler code here
	g_bGrabStart = FALSE;
	UpdateAllViews(NULL, USER_MESSAGE_DISCONNECT, NULL);
}

void CML506Doc::OnUpdateCameraConnect(CCmdUI* pCmdUI)
{
	pCmdUI->Enable(!g_bGrabStart && g_bDCFloaded);
}

void CML506Doc::OnUpdateCameraDisconnect(CCmdUI* pCmdUI)
{
	pCmdUI->Enable(g_bGrabStart && g_bDCFloaded);
}



BOOL CML506Doc::Load8BITbmp(CString filename, BYTE *stim_buffer, int *width, int *height)
{
	BYTE     *buffer;
	CBitmap   bmp;
	BITMAP    bitmap;
	int       retVal, n, i, k;

	// read stimulus pattern from a bitmap file
	HBITMAP hBmp = (HBITMAP)::LoadImage(NULL, filename, IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE);
	if (hBmp == NULL) {
		AfxMessageBox(_T("Open file <") + filename + _T("> error"), MB_ICONEXCLAMATION);
		return FALSE;
	}

	CBitmap *pBmp = bmp.FromHandle(hBmp);
	pBmp->GetBitmap(&bitmap);

	if (bitmap.bmHeight>361 || bitmap.bmWidth>361) {
		AfxMessageBox("Stimulus Pattern is too big. Set width<361 and height<361", MB_ICONEXCLAMATION);
		return FALSE;
	}

	buffer = new BYTE [bitmap.bmWidthBytes*bitmap.bmHeight];

	retVal = pBmp->GetBitmapBits(bitmap.bmWidthBytes*bitmap.bmHeight, buffer);

	n = bitmap.bmWidthBytes/bitmap.bmWidth;

	for (i = 0; i < bitmap.bmWidth*bitmap.bmHeight; i ++) {
		k = (buffer[n*i]+buffer[n*i+1]+buffer[n*i+2])/3;
		stim_buffer[i] = (BYTE)k;
	}

	*width  = bitmap.bmWidth;
	*height = bitmap.bmHeight;

	delete [] buffer;

	return TRUE;
}

void CML506Doc::LoadSymbol(CString filename)
{
	int       retVal, i, k, n, m, sizeX, sizeY, width, height, x1, x2;
	BYTE     *stim_buffer;

	stim_buffer = new BYTE [512*512];
	if (Load8BITbmp(filename, stim_buffer, &width, &height) == FALSE) {
		delete [] stim_buffer;
		return;
	}

	if (width % 2 == 0) sizeX = width;
	else sizeX = width + 1;
	sizeY = height;


	g_VideoInfo.stim_nx = sizeX;
	g_VideoInfo.stim_ny = sizeY;

	ZeroMemory(g_VideoInfo.stim_buffer, sizeX*sizeY*sizeof(unsigned short));
	for (k = 0; k < height; k ++) {
		n = k * sizeX;
		m = k * width;
		for (i = 0; i < width; i ++) {
			g_VideoInfo.stim_buffer[n+i] = (stim_buffer[m+i]<<6);
		}
	}

	delete [] stim_buffer;

	// upload stimulus pattern to FPGA, and saves it in a block RAM
	SetEvent(g_EventLoadStim);
	SetEvent(g_EventLoadLUT);

	m_bSymbol = true;

	UINT32  power_rd, power_ir, nw1, nw2;
	power_rd = 0xffff;
	power_ir = 0xffff;

	for (i = 0; i < g_VideoInfo.stim_nx; i ++) {
		nw1 = power_rd;// >> 1;
		nw2 = nw1 << 16;
		g_VideoInfo.weightsRed[i] = nw1 | nw2;
		nw1 = power_ir;// >> 1;
		nw2 = nw1 << 16;
		g_VideoInfo.weightsIR[i]  = nw1 | nw2;
	}
}


void CML506Doc::GenSinCurve(float amp, float freq)
{
	int             length, i, temp;
	unsigned short *sinBuffer, stemp;
	float           scaledAmp;

	length    = 0x4000;
	sinBuffer = new unsigned short [length];

	scaledAmp = amp * length / 2;
	for (i = 0; i < length; i++) {
		temp = length/2 + (int)(scaledAmp*sin(i*3.1415926*2/length));
		if (temp < 0)
			sinBuffer[i] = 0;
		else if (temp > 0x3fff)
			sinBuffer[i] = 0x3fff;
		else
			sinBuffer[i] = temp;

//		sinBuffer[i] = (unsigned short)(amp*0x3fff);
	}
	g_objVirtex5BMD.LoadSignalBuffer(g_hDevVirtex5, length, sinBuffer);
	delete [] sinBuffer;

	g_objVirtex5BMD.SetSigFreq(g_hDevVirtex5, freq);
}

// generate file ID
void CML506Doc::GenerateFileID(CString videoName, int *index) {
	CString    strFile, fileID;
	int        snID, maxID;
	CFileFind  find;

	strFile = videoName + _T("*.avi");

	BOOL bFound = find.FindFile(strFile);
	maxID   = 0;
	while(bFound)
	{
		bFound = find.FindNextFile();

		CString strFileName;
		if(find.IsDirectory())
			continue;

		strFileName = find.GetFileName();
		strFileName.Trim();

		if(!strFileName.IsEmpty() && (strFileName.GetLength()>0))
		{
			fileID = strFileName.Right(8);
			fileID = fileID.Left(4);
			snID = _tstoi(fileID);
			maxID = (maxID > snID) ? maxID : snID;
		}
	}

	*index = maxID + 1;
}

void CML506Doc::OnSetupDesinusoid()
{
	CCalDesinu   dlg;
	char         temp[80];
	CString      filename;

//	dlg.m_strCurrentDir = m_VideoFolder;
	if (dlg.DoModal() == TRUE) {
	//	SetCurrentDirectory(m_strCurrentDir);
	/*	filename = dlg.m_LUTfname;
		filename.TrimLeft(' ');
		filename.TrimRight(' ');
		if (filename.GetLength() == 0) return;

		// get ML506 parameter .ini file name
		filename = _T("\\AppParams.ini");
		filename = g_ML506Params.m_strConfigPath + filename;

		g_ML506Params.fnameLUT = dlg.m_LUTfname;
		// desinusoiding look up table
		::WritePrivateProfileString("FrameInfo", "DesinusoidLUT", g_ML506Params.fnameLUT, filename);
		// Pixels Per Degree X
		g_ML506Params.PixPerDegX = (float)dlg.m_PixPerDegX;
		sprintf_s(temp, "%1.3f", g_ML506Params.PixPerDegX);
		::WritePrivateProfileString("ApplicationInfo", "PixelsperDegX", temp, filename);
		g_Motion_ScalerX = (float)(g_ML506Params.VRangePerDegX*128./g_ML506Params.PixPerDegX);
		// Pixels Per Degree Y
		g_ML506Params.PixPerDegY = (float)dlg.m_PixPerDegY;
		sprintf_s(temp, "%1.3f", g_ML506Params.PixPerDegY);
		::WritePrivateProfileString("ApplicationInfo", "PixelsperDegY", temp, filename);
		g_Motion_ScalerY = (float)(g_ML506Params.VRangePerDegY*128./g_ML506Params.PixPerDegY);*/
	}
//	SetCurrentDirectory(m_strCurrentDir);
}

void CML506Doc::OnUpdateSetupDesinusoid(CCmdUI* pCmdUI)
{
	// TODO: Add your command update UI handler code here
	pCmdUI->Enable(g_bDCFloaded);
}
