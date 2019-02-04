// ML506View.cpp : implementation of the CML506View class
//

#include "stdafx.h"
#include "ML506.h"

#include "ML506View.h"
#include "FolderDlg.h"

#include <math.h>
#include <string.h>
#include "MUBstd.h"
#include <fstream>

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


extern CVirtex5BMD       g_objVirtex5BMD;
extern WDC_DEVICE_HANDLE g_hDevVirtex5;
extern VIDEO_INFO        g_VideoInfo;
extern CView            *g_viewMain;
extern FILE             *g_fp;

extern CString           g_dataReceived;
extern BOOL              g_bDCFloaded;
extern BOOL              g_bDCFsaved;
extern BOOL              g_bGrabStart;

extern BOOL				 g_bDesinusoid;
extern DesinusoidParams	 g_desinusoidParams;

extern BYTE				*g_imgBuffDF;
extern BYTE				*g_imgBuffSD;
extern BOOL				 g_bSampleSW;

/* --------------------------------------------------
    VIRTEX5 configuration registers information
   -------------------------------------------------- */
// Configuration registers information array
WDC_REG gVIRTEX5_CfgRegs[] = {
    { WDC_AD_CFG_SPACE, PCI_VID, WDC_SIZE_16, WDC_READ_WRITE, "VID",
        "Vendor ID                  " },
    { WDC_AD_CFG_SPACE, PCI_DID, WDC_SIZE_16, WDC_READ_WRITE, "DID",
        "Device ID                  " },
    { WDC_AD_CFG_SPACE, PCI_CR, WDC_SIZE_16, WDC_READ_WRITE, "CMD",
        "Command                    " },
    { WDC_AD_CFG_SPACE, PCI_SR, WDC_SIZE_16, WDC_READ_WRITE, "STS", "Status" },
    { WDC_AD_CFG_SPACE, PCI_REV, WDC_SIZE_32, WDC_READ_WRITE, "RID_CLCD",
        "Revision ID & Class Code   " },
    { WDC_AD_CFG_SPACE, PCI_CCSC, WDC_SIZE_8, WDC_READ_WRITE, "SCC",
        "Sub Class Code             " },
    { WDC_AD_CFG_SPACE, PCI_CCBC, WDC_SIZE_8, WDC_READ_WRITE, "BCC",
        "Base Class Code            " },
    { WDC_AD_CFG_SPACE, PCI_CLSR, WDC_SIZE_8, WDC_READ_WRITE, "CALN",
        "Cache Line Size            " },
    { WDC_AD_CFG_SPACE, PCI_LTR, WDC_SIZE_8, WDC_READ_WRITE, "LAT",
        "Latency Timer              " },
    { WDC_AD_CFG_SPACE, PCI_HDR, WDC_SIZE_8, WDC_READ_WRITE, "HDR",
        "Header Type                " },
    { WDC_AD_CFG_SPACE, PCI_BISTR, WDC_SIZE_8, WDC_READ_WRITE, "BIST",
        "Built-in Self Test         " },
    { WDC_AD_CFG_SPACE, PCI_BAR0, WDC_SIZE_32, WDC_READ_WRITE, "BADDR0",
        "Base Address 0             " },
    { WDC_AD_CFG_SPACE, PCI_BAR1, WDC_SIZE_32, WDC_READ_WRITE, "BADDR1",
        "Base Address 1             " },
    { WDC_AD_CFG_SPACE, PCI_BAR2, WDC_SIZE_32, WDC_READ_WRITE, "BADDR2",
        "Base Address 2             " },
    { WDC_AD_CFG_SPACE, PCI_BAR3, WDC_SIZE_32, WDC_READ_WRITE, "BADDR3",
        "Base Address 3             " },
    { WDC_AD_CFG_SPACE, PCI_BAR4, WDC_SIZE_32, WDC_READ_WRITE, "BADDR4",
        "Base Address 4             " },
    { WDC_AD_CFG_SPACE, PCI_BAR5, WDC_SIZE_32, WDC_READ_WRITE, "BADDR5",
        "Base Address 5             " },
    { WDC_AD_CFG_SPACE, PCI_CIS, WDC_SIZE_32, WDC_READ_WRITE, "CIS",
        "CardBus CIS Pointer        " },
    { WDC_AD_CFG_SPACE, PCI_SVID, WDC_SIZE_16, WDC_READ_WRITE, "SVID",
        "Sub-system Vendor ID       " },
    { WDC_AD_CFG_SPACE, PCI_SDID, WDC_SIZE_16, WDC_READ_WRITE, "SDID",
        "Sub-system Device ID       " },
    { WDC_AD_CFG_SPACE, PCI_EROM, WDC_SIZE_32, WDC_READ_WRITE, "EROM",
        "Exp. ROM Base Address      " },
    { WDC_AD_CFG_SPACE, PCI_CAP, WDC_SIZE_8, WDC_READ_WRITE, "NEW_CAP",
        "New Capabilities Pointer   " },
    { WDC_AD_CFG_SPACE, PCI_ILR, WDC_SIZE_32, WDC_READ_WRITE, "INTLN",
        "Interrupt Line             " },
    { WDC_AD_CFG_SPACE, PCI_IPR, WDC_SIZE_32, WDC_READ_WRITE, "INTPIN",
        "Interrupt Pin              " },
    { WDC_AD_CFG_SPACE, PCI_MGR, WDC_SIZE_32, WDC_READ_WRITE, "MINGNT",
        "Min Required Burst Period  " },
    { WDC_AD_CFG_SPACE, PCI_MLR, WDC_SIZE_32, WDC_READ_WRITE, "MAXLAT",
        "Maximum Latency            " },
    };

#define VIRTEX5_CFG_REGS_NUM (sizeof(gVIRTEX5_CfgRegs)/sizeof(WDC_REG))


/* -----------------------------------------------
    VIRTEX5 run-time registers information
   ----------------------------------------------- */
// Run-time registers information array
WDC_REG gVIRTEX5_Regs[] = {
    { VIRTEX5_SPACE, VIRTEX5_DSCR_OFFSET, WDC_SIZE_32, WDC_READ_WRITE, "DSCR",
        "Device Control Status Register" },
    { VIRTEX5_SPACE, VIRTEX5_DDMACR_OFFSET, WDC_SIZE_32, WDC_READ_WRITE,
        "DDMACR", "Device DMA Control Status Register" },
    { VIRTEX5_SPACE, VIRTEX5_WDMATLPA_OFFSET, WDC_SIZE_32, WDC_READ_WRITE,
        "WDMATLPA", "Write DMA TLP Address" },
    { VIRTEX5_SPACE, VIRTEX5_WDMATLPS_OFFSET, WDC_SIZE_32, WDC_READ_WRITE,
        "WDMATLPS", "Write DMA TLP Size" },
    { VIRTEX5_SPACE, VIRTEX5_WDMATLPC_OFFSET, WDC_SIZE_32, WDC_READ_WRITE,
        "WDMATLPC", "Write DMA TLP Count" },
    { VIRTEX5_SPACE, VIRTEX5_WDMATLPP_OFFSET, WDC_SIZE_32, WDC_READ_WRITE,
        "WDMATLPP", "Write DMA Data Pattern" },
    { VIRTEX5_SPACE, VIRTEX5_RDMATLPP_OFFSET, WDC_SIZE_32, WDC_READ_WRITE,
        "RDMATLPP", "Read DMA Expected Data Pattern" },
    { VIRTEX5_SPACE, VIRTEX5_RDMATLPA_OFFSET, WDC_SIZE_32, WDC_READ_WRITE,
        "RDMATLPA", "Read DMA TLP Address" },
    { VIRTEX5_SPACE, VIRTEX5_RDMATLPS_OFFSET, WDC_SIZE_32, WDC_READ_WRITE,
        "RDMATLPS", "Read DMA TLP Size" },
    { VIRTEX5_SPACE, VIRTEX5_RDMATLPC_OFFSET, WDC_SIZE_32, WDC_READ_WRITE,
        "RDMATLPC", "Read DMA TLP Count" },
    { VIRTEX5_SPACE, VIRTEX5_PUPILMASK_OFFSET, WDC_SIZE_32, WDC_READ, "WDMAPERF",
        "Pupil Mask" },
    { VIRTEX5_SPACE, VIRTEX5_TCABOX_OFFSET, WDC_SIZE_32, WDC_READ, "RDMAPERF",
       "TCA Box" },
    { VIRTEX5_SPACE, VIRTEX5_RDMASTAT_OFFSET, WDC_SIZE_32, WDC_READ, "RDMASTAT",
        "Read DMA Status" },
    { VIRTEX5_SPACE, VIRTEX5_LASER_POWER2, WDC_SIZE_32, WDC_READ, "NRDCOMP",
        "Number of Read Completion w/ Data" },
    { VIRTEX5_SPACE, VIRTEX5_STIMULUS_X_BOUND2, WDC_SIZE_32, WDC_READ,
        "RCOMPDSIZE", "Read Completion Data Size" },
    { VIRTEX5_SPACE, VIRTEX5_DLWSTAT_OFFSET, WDC_SIZE_32, WDC_READ, "DLWSTAT",
        "Device Link Width Status" },
    { VIRTEX5_SPACE, VIRTEX5_DLTRSSTAT_OFFSET, WDC_SIZE_32, WDC_READ,
        "DLTRSSTAT", "Device Link Transaction Size Status" },
    { VIRTEX5_SPACE, VIRTEX5_DMISCCONT_OFFSET, WDC_SIZE_32, WDC_READ_WRITE,
        "DMISCCONT", "Device Miscellaneous Control" },
    { VIRTEX5_SPACE, VIRTEX5_I2CINFO_OFFSET, WDC_SIZE_32, WDC_READ_WRITE,
        "I2CINFO", "I2C Slave Register Control" },
    { VIRTEX5_SPACE, VIRTEX5_LINEINFO_OFFSET, WDC_SIZE_32, WDC_READ_WRITE,
        "LINEINFO", "Lines Start Index and Line Counter" },
    { VIRTEX5_SPACE, VIRTEX5_IMAGESIZE_OFFSET, WDC_SIZE_32, WDC_READ_WRITE,
        "IMAGESIZE", "Image Size of Real-Time Video" },
    { VIRTEX5_SPACE, VIRTEX5_IMAGEOS_OFFSET, WDC_SIZE_32, WDC_READ_WRITE,
        "IMAGEOS", "Image Size Offsets (top, left)" },
    { VIRTEX5_SPACE, VIRTEX5_LINECTRL_OFFSET, WDC_SIZE_32, WDC_READ_WRITE,
	"LINECTRL", "Interrupr Line Control" },
    { VIRTEX5_SPACE, VIRTEX5_STIMULUS_LOC, WDC_SIZE_32, WDC_READ_WRITE,
	"STIMLOC", "Stimulus Location" },
    { VIRTEX5_SPACE, VIRTEX5_LASER_POWER, WDC_SIZE_32, WDC_READ_WRITE,
	"LASERPOW", "Laser Power" }
    };

#define VIRTEX5_REGS_NUM (sizeof(gVIRTEX5_Regs)/sizeof(gVIRTEX5_Regs[0]))



/////////////////////////////////////////////////////////////////////////////
// CML506View

IMPLEMENT_DYNCREATE(CML506View, CView)

BEGIN_MESSAGE_MAP(CML506View, CView)
	//{{AFX_MSG_MAP(CML506View)
	ON_WM_CREATE()
	ON_BN_CLICKED(ID_READ_ONE, ReadOneRegister)
	ON_BN_CLICKED(ID_WRITE_ONE, WriteOneRegister)
	ON_BN_CLICKED(ID_READ_ALL, ReadAllRegister)
	ON_BN_CLICKED(ID_LOAD_STIMULUS, LoadStimulus)
	ON_BN_CLICKED(ID_DESINUSOID_CAL, CalibrateSinu)
	ON_BN_CLICKED(ID_VIDEO_SAVE, SaveLiveVideo)
	ON_BN_CLICKED(ID_WRITE_ALL, ReadPCIe)
	ON_BN_CLICKED(ID_BUTTON_FOLDER_R, LoadVideoFolderR)
	ON_BN_CLICKED(ID_BUTTON_FOLDER_G, LoadVideoFolderG)
	ON_BN_CLICKED(ID_BUTTON_FOLDER_B, LoadVideoFolderB)
	ON_BN_CLICKED(ID_VIDEO_FRAMES, VideoInFrames)
	ON_BN_CLICKED(ID_VIDEO_SECONDS, VideoInSeconds)
	ON_BN_CLICKED(ID_CHECK_SAMPLE_BOTH, SampleDualChannel)
	ON_BN_CLICKED(ID_CHECK_CH_G, SampleChannelG)
	ON_BN_CLICKED(ID_CHECK_CH_B, SampleChannelB)
	ON_BN_CLICKED(ID_CHECK_SW_DF_SD, SampleChannelSW)
	ON_BN_CLICKED(ID_CHECK_DISC, DiscardBlinks)
	ON_BN_CLICKED(ID_INTERLEAVE_LINE, InterleaveLines)
	ON_BN_CLICKED(ID_ZERO_INVERSE, ZeroBackward)
	ON_BN_CLICKED(ID_SYMMETRIC_RAMP, SymmetricRamp)
	ON_BN_CLICKED(ID_ENABLE_TRACKING, EnableTracking)
	ON_BN_CLICKED(ID_FRAME_COUNTER_LINES, FrameCounterInLines)
	ON_BN_CLICKED(ID_FRAME_COUNTER_VSYNC, FrameCounterInVsync)
	ON_BN_CLICKED(ID_CALIBRATE_RSFOV, ScaleRSFOV)
	ON_BN_CLICKED(ID_POWER488_CHK, Power488Cal)
	ON_BN_CLICKED(ID_POWER568_CHK, Power568Cal)
	ON_BN_CLICKED(ID_POWER796_CHK, Power796Cal)

	ON_CBN_SELCHANGE(ID_CBO_HSYNCI, HsyncInChange)
	ON_CBN_SELCHANGE(ID_CBO_HSYNCO, HsyncOutChange)
	ON_CBN_SELCHANGE(ID_CBO_VSYNCI, VsyncInChange)
	ON_CBN_SELCHANGE(ID_CBO_VSYNCO, VsyncOutChange)
	ON_NOTIFY(TCN_SELCHANGE, ID_TAB_PCIE, OnSelchangeTabMain)
	ON_WM_PAINT()
	ON_COMMAND(ID_LOAD_DCF, OnLoadDcf)
	ON_COMMAND(ID_SAVE_DCF, OnSaveDcf)
	ON_UPDATE_COMMAND_UI(ID_SAVE_DCF, OnUpdateSaveDCF)
	ON_WM_SIZE()
	ON_WM_HSCROLL()
	ON_COMMAND(ID_NEW_DCF, OnNewDcf)
	ON_MESSAGE(WM_MESSAGE_SEND, OnSendMessage)

	ON_NOTIFY(EN_MSGFILTER, ID_EDIT_BRIGHTNESS_R, UpdateBrightnessR)
	ON_NOTIFY(EN_MSGFILTER, ID_EDIT_CONTRAST_R, UpdateContrastR)
	ON_NOTIFY(EN_MSGFILTER, ID_EDIT_BRIGHTNESS_G, UpdateBrightnessG)
	ON_NOTIFY(EN_MSGFILTER, ID_EDIT_CONTRAST_G, UpdateContrastG)
	ON_NOTIFY(EN_MSGFILTER, ID_EDIT_BRIGHTNESS_B, UpdateBrightnessB)
	ON_NOTIFY(EN_MSGFILTER, ID_EDIT_CONTRAST_B, UpdateContrastB)
	ON_NOTIFY(EN_MSGFILTER, ID_EDIT_PCLOCK, UpdatePixelClock)


	//}}AFX_MSG_MAP
	// Standard printing commands
	ON_COMMAND(ID_FILE_PRINT, CView::OnFilePrint)
	ON_COMMAND(ID_FILE_PRINT_DIRECT, CView::OnFilePrint)
	ON_COMMAND(ID_FILE_PRINT_PREVIEW, CView::OnFilePrintPreview)
	ON_WM_TIMER()
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CML506View construction/destruction

CML506View::CML506View()
{
	int i;

	// TODO: add construction code here
	g_viewMain       = this;
	m_bAviHandlerOnR = FALSE;
	m_bAviHandlerOnG = FALSE;
	m_bAviHandlerOnB = FALSE;
	m_image          = NULL;//new BYTE [2048*2048];
	m_bDualChannel   = FALSE;
	m_bSampleG       = FALSE;
	m_bSampleB       = FALSE;
	m_bSampleSW       = FALSE;
	m_bDiscardBlinks	=FALSE;
	m_pDlgR          = NULL;
	m_pDlgG          = NULL;
	m_pDlgB          = NULL;
	m_pDlgSinu       = NULL;
	m_iVideoNum      = 1;		// in default mode, only one tab of video will be sampled
	m_iChannelNum    = 1;		// in default mode, only forward scanning video will be sampled
	m_iShiftBS       = 1;
/*	m_lutIdx1        = new unsigned short [4096];
	m_lutVal1        = new unsigned short [4096];
	m_lutIdx2        = new unsigned short [4096];
	m_lutVal2        = new unsigned short [4096];*/
	m_iTrackY        = 0;
	m_iTrackY        = 0;
	m_iPLLclks       = 0;
	m_bCalibrate     = FALSE;
	m_bFOVflag       = TRUE;
	m_bZeroBackward  = FALSE;
	m_bSymmetricRamp = FALSE;
	m_bEnableTracking= TRUE;
	m_imgCalibration = NULL;
	m_iSteerY        = 0;
	m_bRotate45      = TRUE;
	m_bPower488      = FALSE;
	m_bPower568      = FALSE;
	m_bPower796      = FALSE;
	m_nPower488      = 0xFF;
	m_nPower568      = 0x3FFF;
	m_nPower796      = 0xFF;
	m_fLaserModulation    = new float [2048];

	g_VideoInfo.nVideoTabs     = 1;
	g_VideoInfo.nVideoChannels = 1;

	// setup DAC on-chip registers
	m_dacTextID[0]  = ID_DAC_TEXT_1CH;
	m_dacTextID[1]  = ID_DAC_TEXT_1DH;
	m_dacTextID[2]  = ID_DAC_TEXT_1EH;
	m_dacTextID[3]  = ID_DAC_TEXT_1FH;
	m_dacTextID[4]  = ID_DAC_TEXT_20H;
	m_dacTextID[5]  = ID_DAC_TEXT_21H;
	m_dacTextID[6]  = ID_DAC_TEXT_23H;
	m_dacTextID[7]  = ID_DAC_TEXT_31H;
	m_dacTextID[8]  = ID_DAC_TEXT_33H;
	m_dacTextID[9]  = ID_DAC_TEXT_34H;
	m_dacTextID[10] = ID_DAC_TEXT_35H;
	m_dacTextID[11] = ID_DAC_TEXT_36H;
	m_dacTextID[12] = ID_DAC_TEXT_37H;
	m_dacTextID[13] = ID_DAC_TEXT_48H;
	m_dacTextID[14] = ID_DAC_TEXT_49H;
	m_dacTextID[15] = ID_DAC_TEXT_4AH;
	m_dacTextID[16] = ID_DAC_TEXT_4BH;
	m_dacTextID[17] = ID_DAC_TEXT_56H;

	m_dacEditID[0]  = ID_DAC_EDIT_1CH;
	m_dacEditID[1]  = ID_DAC_EDIT_1DH;
	m_dacEditID[2]  = ID_DAC_EDIT_1EH;
	m_dacEditID[3]  = ID_DAC_EDIT_1FH;
	m_dacEditID[4]  = ID_DAC_EDIT_20H;
	m_dacEditID[5]  = ID_DAC_EDIT_21H;
	m_dacEditID[6]  = ID_DAC_EDIT_23H;
	m_dacEditID[7]  = ID_DAC_EDIT_31H;
	m_dacEditID[8]  = ID_DAC_EDIT_33H;
	m_dacEditID[9]  = ID_DAC_EDIT_34H;
	m_dacEditID[10] = ID_DAC_EDIT_35H;
	m_dacEditID[11] = ID_DAC_EDIT_36H;
	m_dacEditID[12] = ID_DAC_EDIT_37H;
	m_dacEditID[13] = ID_DAC_EDIT_48H;
	m_dacEditID[14] = ID_DAC_EDIT_49H;
	m_dacEditID[15] = ID_DAC_EDIT_4AH;
	m_dacEditID[16] = ID_DAC_EDIT_4BH;
	m_dacEditID[17] = ID_DAC_EDIT_56H;

	for (i = 0; i < 18; i ++)
		m_dacLabel[i] = new char [128];
	m_dacLabel[0]  = _T("0x1C (CM), Clock Mode");
	m_dacLabel[1]  = _T("0x1D (IC), Input Clock");
	m_dacLabel[2]  = _T("0x1E (GPIO), GPIO Control");
	m_dacLabel[3]  = _T("0x1F (IDF), Input Data Format");
	m_dacLabel[4]  = _T("0x20 (CD)), Connection Detect");
	m_dacLabel[5]  = _T("0x21 (DC)), DAC Control");
	m_dacLabel[6]  = _T("0x23 (HPD), Hot Plus Detection");
	m_dacLabel[7]  = _T("0x31 (TCTL), DVI Control Input");
	m_dacLabel[8]  = _T("0x33 (TPCP), DVI PLL Charge Pump Control");
	m_dacLabel[9]  = _T("0x34 (TPD), DVI PLL Divider");
	m_dacLabel[10] = _T("0x35 (TPVT), DVI PLL Supply Control");
	m_dacLabel[11] = _T("0x36 (TPF), DVI PLL Filter");
	m_dacLabel[12] = _T("0x37 (TCT), DVI Clock Test");
	m_dacLabel[13] = _T("0x48 (TSTP), Test Pattern");
	m_dacLabel[14] = _T("0x49 (PM), Power Management");
	m_dacLabel[15] = _T("0x4A (VID), Version ID");
	m_dacLabel[16] = _T("0x4B (DID), Device ID");
	m_dacLabel[17] = _T("0x56 (DSP), DVI Sync Polority");
	m_dacRegs[0]  = 0x1C;
	m_dacRegs[1]  = 0x1D;
	m_dacRegs[2]  = 0x1E;
	m_dacRegs[3]  = 0x1F;
	m_dacRegs[4]  = 0x20;
	m_dacRegs[5]  = 0x21;
	m_dacRegs[6]  = 0x23;
	m_dacRegs[7]  = 0x31;
	m_dacRegs[8]  = 0x33;
	m_dacRegs[9]  = 0x34;
	m_dacRegs[10] = 0x35;
	m_dacRegs[11] = 0x36;
	m_dacRegs[12] = 0x37;
	m_dacRegs[13] = 0x48;
	m_dacRegs[14] = 0x49;
	m_dacRegs[15] = 0x4A;
	m_dacRegs[16] = 0x4B;
	m_dacRegs[17] = 0x56;


	// setup ADC on-chip registers
	m_adcTextID[0]  = ID_TEXT_0X00;
	m_adcTextID[1]  = ID_TEXT_0X01;
	m_adcTextID[2]  = ID_TEXT_0X02;
	m_adcTextID[3]  = ID_TEXT_0X03;
	m_adcTextID[4]  = ID_TEXT_0X04;
	m_adcTextID[5]  = ID_TEXT_0X05;
	m_adcTextID[6]  = ID_TEXT_0X06;
	m_adcTextID[7]  = ID_TEXT_0X07;
	m_adcTextID[8]  = ID_TEXT_0X08;
	m_adcTextID[9]  = ID_TEXT_0X09;
	m_adcTextID[10] = ID_TEXT_0X0A;
	m_adcTextID[11] = ID_TEXT_0X0B;
	m_adcTextID[12] = ID_TEXT_0X0C;
	m_adcTextID[13] = ID_TEXT_0X0D;
	m_adcTextID[14] = ID_TEXT_0X0E;
	m_adcTextID[15] = ID_TEXT_0X0F;
	m_adcTextID[16] = ID_TEXT_0X10;
	m_adcTextID[17] = ID_TEXT_0X11;
	m_adcTextID[18] = ID_TEXT_0X12;
	m_adcTextID[19] = ID_TEXT_0X13;
	m_adcTextID[20] = ID_TEXT_0X14;
	m_adcTextID[21] = ID_TEXT_0X15;
	m_adcTextID[22] = ID_TEXT_0X16;
	m_adcTextID[23] = ID_TEXT_0X17;
	m_adcTextID[24] = ID_TEXT_0X18;
	m_adcTextID[25] = ID_TEXT_0X19;
	m_adcTextID[26] = ID_TEXT_0X1A;
	m_adcTextID[27] = ID_TEXT_0X1B;
	m_adcTextID[28] = ID_TEXT_0X1C;
	m_adcTextID[29] = ID_TEXT_0X1D;
	m_adcTextID[30] = ID_TEXT_0X1E;
	m_adcTextID[31] = ID_TEXT_0X1F;
	m_adcTextID[32] = ID_TEXT_0X20;
	m_adcTextID[33] = ID_TEXT_0X21;
	m_adcTextID[34] = ID_TEXT_0X22;
	m_adcTextID[35] = ID_TEXT_0X23;
	m_adcTextID[36] = ID_TEXT_0X24;
	m_adcTextID[37] = ID_TEXT_0X25;
	m_adcTextID[38] = ID_TEXT_0X26;
	m_adcTextID[39] = ID_TEXT_0X27;
	m_adcTextID[40] = ID_TEXT_0X28;
	m_adcTextID[41] = ID_TEXT_0X29;
	m_adcTextID[42] = ID_TEXT_0X2A;
	m_adcTextID[43] = ID_TEXT_0X2B;
	m_adcTextID[44] = ID_TEXT_0X2C;
	m_adcTextID[45] = ID_TEXT_0X2D;
	m_adcTextID[46] = ID_TEXT_0X2E;


	m_adcEditID[0]  = ID_EDIT_0X00;
	m_adcEditID[1]  = ID_EDIT_0X01;
	m_adcEditID[2]  = ID_EDIT_0X02;
	m_adcEditID[3]  = ID_EDIT_0X03;
	m_adcEditID[4]  = ID_EDIT_0X04;
	m_adcEditID[5]  = ID_EDIT_0X05;
	m_adcEditID[6]  = ID_EDIT_0X06;
	m_adcEditID[7]  = ID_EDIT_0X07;
	m_adcEditID[8]  = ID_EDIT_0X08;
	m_adcEditID[9]  = ID_EDIT_0X09;
	m_adcEditID[10] = ID_EDIT_0X0A;
	m_adcEditID[11] = ID_EDIT_0X0B;
	m_adcEditID[12] = ID_EDIT_0X0C;
	m_adcEditID[13] = ID_EDIT_0X0D;
	m_adcEditID[14] = ID_EDIT_0X0E;
	m_adcEditID[15] = ID_EDIT_0X0F;
	m_adcEditID[16] = ID_EDIT_0X10;
	m_adcEditID[17] = ID_EDIT_0X11;
	m_adcEditID[18] = ID_EDIT_0X12;
	m_adcEditID[19] = ID_EDIT_0X13;
	m_adcEditID[20] = ID_EDIT_0X14;
	m_adcEditID[21] = ID_EDIT_0X15;
	m_adcEditID[22] = ID_EDIT_0X16;
	m_adcEditID[23] = ID_EDIT_0X17;
	m_adcEditID[24] = ID_EDIT_0X18;
	m_adcEditID[25] = ID_EDIT_0X19;
	m_adcEditID[26] = ID_EDIT_0X1A;
	m_adcEditID[27] = ID_EDIT_0X1B;
	m_adcEditID[28] = ID_EDIT_0X1C;
	m_adcEditID[29] = ID_EDIT_0X1D;
	m_adcEditID[30] = ID_EDIT_0X1E;
	m_adcEditID[31] = ID_EDIT_0X1F;
	m_adcEditID[32] = ID_EDIT_0X20;
	m_adcEditID[33] = ID_EDIT_0X21;
	m_adcEditID[34] = ID_EDIT_0X22;
	m_adcEditID[35] = ID_EDIT_0X23;
	m_adcEditID[36] = ID_EDIT_0X24;
	m_adcEditID[37] = ID_EDIT_0X25;
	m_adcEditID[38] = ID_EDIT_0X26;
	m_adcEditID[39] = ID_EDIT_0X27;
	m_adcEditID[40] = ID_EDIT_0X28;
	m_adcEditID[41] = ID_EDIT_0X29;
	m_adcEditID[42] = ID_EDIT_0X2A;
	m_adcEditID[43] = ID_EDIT_0X2B;
	m_adcEditID[44] = ID_EDIT_0X2C;
	m_adcEditID[45] = ID_EDIT_0X2D;
	m_adcEditID[46] = ID_EDIT_0X2E;

	m_cfgDataID[0] = ID_CONFIG_SPACE_00;
	m_cfgDataID[1] = ID_CONFIG_SPACE_01;
	m_cfgDataID[2] = ID_CONFIG_SPACE_02;
	m_cfgDataID[3] = ID_CONFIG_SPACE_03;
	m_cfgDataID[4] = ID_CONFIG_SPACE_04;
	m_cfgDataID[5] = ID_CONFIG_SPACE_05;
	m_cfgDataID[6] = ID_CONFIG_SPACE_06;
	m_cfgDataID[7] = ID_CONFIG_SPACE_07;
	m_cfgDataID[8] = ID_CONFIG_SPACE_08;
	m_cfgDataID[9] = ID_CONFIG_SPACE_09;
	m_cfgDataID[10] = ID_CONFIG_SPACE_10;
	m_cfgDataID[11] = ID_CONFIG_SPACE_11;
	m_cfgDataID[12] = ID_CONFIG_SPACE_12;
	m_cfgDataID[13] = ID_CONFIG_SPACE_13;
	m_cfgDataID[14] = ID_CONFIG_SPACE_14;
	m_cfgDataID[15] = ID_CONFIG_SPACE_15;
	m_cfgDataID[16] = ID_CONFIG_SPACE_16;
	m_cfgDataID[17] = ID_CONFIG_SPACE_17;
	m_cfgDataID[18] = ID_CONFIG_SPACE_18;
	m_cfgDataID[19] = ID_CONFIG_SPACE_19;
	m_cfgDataID[20] = ID_CONFIG_SPACE_20;
	m_cfgDataID[21] = ID_CONFIG_SPACE_21;
	m_cfgDataID[22] = ID_CONFIG_SPACE_22;
	m_cfgDataID[23] = ID_CONFIG_SPACE_23;
	m_cfgDataID[24] = ID_CONFIG_SPACE_24;
	m_cfgDataID[25] = ID_CONFIG_SPACE_25;
	m_cfgDataID[26] = ID_CONFIG_SPACE_26;
	m_cfgDataID[27] = ID_CONFIG_SPACE_27;

	for (i = 0; i < VIRTEX5_CFG_REGS_NUM+2; i ++)
		m_cfgLabel[i] = new char [128];

	m_cfgLabel[0]  = _T("Offset  size  Description                \t\tValue");
	m_cfgLabel[1]  = _T("---------------------------------------------------------------------------------");
	m_cfgLabel[2]  = _T("0x00    2     Vendor ID                  \t\t");
	m_cfgLabel[3]  = _T("0x02    2     Device ID                  \t\t");
	m_cfgLabel[4]  = _T("0x04    2     Command                    \t\t");
	m_cfgLabel[5]  = _T("0x06    2     Status                     \t\t");
	m_cfgLabel[6]  = _T("0x08    4     Revision ID & Class Code   \t");
	m_cfgLabel[7]  = _T("0x0A    1     Sub Class Code             \t");
	m_cfgLabel[8]  = _T("0x0B    1     Base Class Code            \t");
	m_cfgLabel[9]  = _T("0x0C    1     Cache Line Size            \t");
	m_cfgLabel[10] = _T("0x0D    1     Latency Timer              \t");
	m_cfgLabel[11] = _T("0x0E    1     Header Type                \t");
	m_cfgLabel[12] = _T("0x0F    1     Built-in Self Test         \t\t");
	m_cfgLabel[13] = _T("0x10    4     Base Address 0             \t");
	m_cfgLabel[14] = _T("0x14    4     Base Address 1             \t");
	m_cfgLabel[15] = _T("0x18    4     Base Address 2             \t");
	m_cfgLabel[16] = _T("0x1C    4     Base Address 3             \t");
	m_cfgLabel[17] = _T("0x20    4     Base Address 4             \t");
	m_cfgLabel[18] = _T("0x24    4     Base Address 5             \t");
	m_cfgLabel[19] = _T("0x28    4     CardBus CIS Pointer        \t");
	m_cfgLabel[20] = _T("0x2C    2     Sub-system Vendor ID       \t");
	m_cfgLabel[21] = _T("0x2E    2     Sub-system Device ID       \t");
	m_cfgLabel[22] = _T("0x30    4     Exp. ROM Base Address      \t");
	m_cfgLabel[23] = _T("0x34    1     New Capabilities Pointer   \t");
	m_cfgLabel[24] = _T("0x3C    4     Interrupt Line             \t\t");
	m_cfgLabel[25] = _T("0x3D    4     Interrupt Pin              \t\t");
	m_cfgLabel[26] = _T("0x3E    4     Min Required Burst Period  \t");
	m_cfgLabel[27] = _T("0x3F    4     Maximum Latency            \t");

	m_regTextID[0]  = ID_REG_TEXT_00;
	m_regTextID[1]  = ID_REG_TEXT_01;
	m_regTextID[2]  = ID_REG_TEXT_02;
	m_regTextID[3]  = ID_REG_TEXT_03;
	m_regTextID[4]  = ID_REG_TEXT_04;
	m_regTextID[5]  = ID_REG_TEXT_05;
	m_regTextID[6]  = ID_REG_TEXT_06;
	m_regTextID[7]  = ID_REG_TEXT_07;
	m_regTextID[8]  = ID_REG_TEXT_08;
	m_regTextID[9]  = ID_REG_TEXT_09;
	m_regTextID[10] = ID_REG_TEXT_10;
	m_regTextID[11] = ID_REG_TEXT_11;
	m_regTextID[12] = ID_REG_TEXT_12;
	m_regTextID[13] = ID_REG_TEXT_13;
	m_regTextID[14] = ID_REG_TEXT_14;
	m_regTextID[15] = ID_REG_TEXT_15;
	m_regTextID[16] = ID_REG_TEXT_16;
	m_regTextID[17] = ID_REG_TEXT_17;
	m_regTextID[18] = ID_REG_TEXT_18;
	m_regTextID[19] = ID_REG_TEXT_19;
	m_regTextID[20] = ID_REG_TEXT_20;
	m_regTextID[21] = ID_REG_TEXT_21;
	m_regTextID[22] = ID_REG_TEXT_22;
	m_regTextID[23] = ID_REG_TEXT_23;
	m_regTextID[24] = ID_REG_TEXT_24;
	m_regTextID[25] = ID_REG_TEXT_25;
	m_regTextID[26] = ID_REG_TEXT_26;
	m_regTextID[27] = ID_REG_TEXT_27;


	m_regEditID[0]  = ID_REG_EDIT_00;
	m_regEditID[1]  = ID_REG_EDIT_01;
	m_regEditID[2]  = ID_REG_EDIT_02;
	m_regEditID[3]  = ID_REG_EDIT_03;
	m_regEditID[4]  = ID_REG_EDIT_04;
	m_regEditID[5]  = ID_REG_EDIT_05;
	m_regEditID[6]  = ID_REG_EDIT_06;
	m_regEditID[7]  = ID_REG_EDIT_07;
	m_regEditID[8]  = ID_REG_EDIT_08;
	m_regEditID[9]  = ID_REG_EDIT_09;
	m_regEditID[10] = ID_REG_EDIT_10;
	m_regEditID[11] = ID_REG_EDIT_11;
	m_regEditID[12] = ID_REG_EDIT_12;
	m_regEditID[13] = ID_REG_EDIT_13;
	m_regEditID[14] = ID_REG_EDIT_14;
	m_regEditID[15] = ID_REG_EDIT_15;
	m_regEditID[16] = ID_REG_EDIT_16;
	m_regEditID[17] = ID_REG_EDIT_17;
	m_regEditID[18] = ID_REG_EDIT_18;
	m_regEditID[19] = ID_REG_EDIT_19;
	m_regEditID[20] = ID_REG_EDIT_20;
	m_regEditID[21] = ID_REG_EDIT_21;
	m_regEditID[22] = ID_REG_EDIT_22;
	m_regEditID[23] = ID_REG_EDIT_23;
	m_regEditID[24] = ID_REG_EDIT_24;
	m_regEditID[25] = ID_REG_EDIT_25;
	m_regEditID[26] = ID_REG_EDIT_26;
	m_regEditID[27] = ID_REG_EDIT_27;

	for (i = 0; i < 28; i ++)
		m_regLabel[i] = new char [128];

	m_regLabel[0]  = _T("0x00, Device Control Status Register");
	m_regLabel[1]  = _T("0x04, Device DMA Control Status Register");
	m_regLabel[2]  = _T("0x08, Write DMA TLP Address");
	m_regLabel[3]  = _T("0x0C, Write DMA TLP Size");
	m_regLabel[4]  = _T("0x10, Write DMA TLP Count");
	m_regLabel[5]  = _T("0x14, Write DMA Data Pattern");
	m_regLabel[6]  = _T("0x18, Read DMA Expected Data Pattern");
	m_regLabel[7]  = _T("0x1C, Read DMA TLP Address");
	m_regLabel[8]  = _T("0x20, Read DMA TLP Size");
	m_regLabel[9]  = _T("0x24, Read DMA TLP Count");
	m_regLabel[10] = _T("0x28, Write DMA Performance");
	m_regLabel[11] = _T("0x2C, Read DMA Performance");
	m_regLabel[12] = _T("0x30, Read DMA Status");
	m_regLabel[13] = _T("0x34, Number of Read Completion w/ Data");
	m_regLabel[14] = _T("0x38, Read Completion Data Size");
	m_regLabel[15] = _T("0x3C, Device Link Width Status");
	m_regLabel[16] = _T("0x40, Device Link Transaction Size Status");
	m_regLabel[17] = _T("0x44, Device Miscellaneous Control");
	m_regLabel[18] = _T("0x48, I2C Slave Register Control");
	m_regLabel[19] = _T("0x4C, Stripe bound Lines Indices");
	m_regLabel[20] = _T("0x50, Image Size of Real-Time Video");
	m_regLabel[21] = _T("0x54, Image Offsets");
	m_regLabel[22] = _T("0x58, Interrupt Line Control");
	m_regLabel[23] = _T("0x5C, Stimulus Location");
	m_regLabel[24] = _T("0x60, 14-bit laser power");
	m_regLabel[25] = _T("0x64, Stimulus Address");
	m_regLabel[26] = _T("0x68, Stimulus Data");
	m_regLabel[27] = _T("0x6C, Stimulus Boundary");

	m_tabIndex     = 0;
	m_regVal       = 0;

	// we setup the default data below for both DAC and ADC
	for (i = 0; i < 48; i ++) m_adcVals[i] = 0;
	for (i = 0; i < 18; i ++) m_dacVals[i] = 0;
	m_adcVals[0x01] = 0x4F;//0x9C;		// PLL dividor, force pixel clock to 20MHz
	m_adcVals[0x02] = 0x70;//0x40;
	m_iPLLclks      = ((m_adcVals[0x01]<<4) + (m_adcVals[0x02]>>4)) >> 1;
	m_adcVals[0x03] = 0x18;		// VCO/CPMP
	m_adcVals[0x04] = 0x60;		// Phase adjustment
	m_adcVals[0x05] = 0x20;		// contrast R
	m_adcVals[0x06] = 0x00;
	m_adcVals[0x07] = 0x20;		// contrast G
	m_adcVals[0x08] = 0x00;
	m_adcVals[0x09] = 0x20;		// contrast B
	m_adcVals[0x0A] = 0x00;
	m_adcVals[0x0B] = 0x48;		// brightness R
	m_adcVals[0x0C] = 0x00;
	m_adcVals[0x0D] = 0x48;		// brightness G
	m_adcVals[0x0E] = 0x00;
	m_adcVals[0x0F] = 0x48;		// brightness B
	m_adcVals[0x10] = 0x00;
	m_adcVals[0x12] = 0xB8;		// Hsync control
	m_adcVals[0x14] = 0xB8;		// Vsync control
	m_adcVals[0x18] = 0xF0;		// clamping control
	m_adcVals[0x19] = 0x02;		// clamping placement
	m_adcVals[0x1A] = 0x01;		// clamping duration
	m_adcVals[0x1B] = 0x1B;		// clamping trigger

	m_dacVals[0] = 0x00;		// clock mode
	m_dacVals[1] = 0x48;		// input clock
	m_dacVals[3] = 0x80;		// input data format
	//m_dacVals[4] = 0x2E;		// connection detect
	//m_dacVals[5] = 0x09;		// dac control
	m_dacVals[14] = 0x00;		// power management

	m_osx    = 430;
	m_osy    = 20;

	m_strTitleName = "";
	m_strAppName = "Xilinx ML506 Digitizer Controller - ";
	m_strFileName = "";
	m_bXFGloaded = FALSE;
	m_bClientCreated = FALSE;
	m_bVideoInSeconds = TRUE;
	m_bFrameInLines = TRUE;

	g_objVirtex5BMD.AppSetFrameCounter(g_hDevVirtex5, m_bFrameInLines);

	for (i = 0; i < g_VideoInfo.img_width; i ++) m_fLaserModulation[i] = 1.0;
	g_objVirtex5BMD.AppLaserModulation(g_hDevVirtex5, m_fLaserModulation, g_VideoInfo.img_width);
}


CML506View::~CML506View()
{
	if (m_image != NULL) delete [] m_image;
	if (m_pDlgR != NULL) {
		m_pDlgR->DestroyWindow();
		delete m_pDlgR; // destroys window if not already destroyed
	}
	if (m_pDlgG != NULL) {
		m_pDlgG->DestroyWindow();
		delete m_pDlgG; // destroys window if not already destroyed
	}
	if (m_pDlgB != NULL) {
		m_pDlgB->DestroyWindow();
		delete m_pDlgB; // destroys window if not already destroyed
	}
	if (m_pDlgSinu != NULL) {
		m_pDlgSinu->DestroyWindow();
		delete m_pDlgSinu; // destroys window if not already destroyed
	}

/*	delete [] 	m_lutIdx1;
	delete [] 	m_lutVal1;
	delete [] 	m_lutIdx2;
	delete [] 	m_lutVal2;*/
	delete []   m_fLaserModulation;
}

BOOL CML506View::PreCreateWindow(CREATESTRUCT& cs)
{
	// TODO: Modify the Window class or styles here by modifying
	//  the CREATESTRUCT cs

	return CView::PreCreateWindow(cs);
}

/////////////////////////////////////////////////////////////////////////////
// CML506View drawing

void CML506View::OnDraw(CDC* pDC)
{
	CML506Doc* pDoc = GetDocument();
	ASSERT_VALID(pDoc);
	// TODO: add draw code for native data here
}

/////////////////////////////////////////////////////////////////////////////
// CML506View printing

BOOL CML506View::OnPreparePrinting(CPrintInfo* pInfo)
{
	// default preparation
	return DoPreparePrinting(pInfo);
}

void CML506View::OnBeginPrinting(CDC* /*pDC*/, CPrintInfo* /*pInfo*/)
{
	// TODO: add extra initialization before printing
}

void CML506View::OnEndPrinting(CDC* /*pDC*/, CPrintInfo* /*pInfo*/)
{
	// TODO: add cleanup after printing
}

/////////////////////////////////////////////////////////////////////////////
// CML506View diagnostics

#ifdef _DEBUG
void CML506View::AssertValid() const
{
	CView::AssertValid();
}

void CML506View::Dump(CDumpContext& dc) const
{
	CView::Dump(dc);
}

CML506Doc* CML506View::GetDocument() // non-debug version is inline
{
	ASSERT(m_pDocument->IsKindOf(RUNTIME_CLASS(CML506Doc)));
	return (CML506Doc*)m_pDocument;
}
#endif //_DEBUG

/////////////////////////////////////////////////////////////////////////////
// CML506View message handlers

void CML506View::CreateADCregisters()
{
	RECT        rect;
	CString     msg;
	int         i;

	rect.top  = 40 - (DLG_ITEM_HEIGHT + DLG_ITEM_MARGIN_Y);
	for (i = 0; i < 24; i ++) {
		// draw labels
		rect.left   = 10;
		rect.right  = rect.left + DLG_ITEM_WIDTH;
		rect.top    = rect.top + DLG_ITEM_HEIGHT + DLG_ITEM_MARGIN_Y;
		rect.bottom = rect.top + DLG_ITEM_HEIGHT;
		msg.Format("0x%02X:", i);
		m_adcText[i].Create(msg, WS_CHILD, rect, this, m_adcTextID[i]);

		// draw edit boxes
		rect.left   = 10 + DLG_ITEM_WIDTH + DLG_ITEM_MARGIN_X;
		rect.right  = rect.left + DLG_ITEM_WIDTH;
		if (i == 0) {
			m_adcEdit[i].Create(WS_CHILD | ES_LEFT | WS_BORDER | WS_TABSTOP | ES_READONLY, rect, this, m_adcEditID[i]);
			m_adcEdit[i].SetBackgroundColor(FALSE, RGB(255, 255, 191));
		} else {
			m_adcEdit[i].Create(WS_CHILD | ES_LEFT | WS_BORDER | WS_TABSTOP | ES_READONLY, rect, this, m_adcEditID[i]);
		}
		m_adcEdit[i].LimitText(8);
	}

	rect.top  = 40 - (DLG_ITEM_HEIGHT + DLG_ITEM_MARGIN_Y);
	for (i = 24; i < 47; i ++) {
		// draw labels
		rect.left   = 10 + 2*(DLG_ITEM_WIDTH + DLG_ITEM_MARGIN_X);
		rect.right  = rect.left + DLG_ITEM_WIDTH;
		rect.top    = rect.top + DLG_ITEM_HEIGHT + DLG_ITEM_MARGIN_Y;
		rect.bottom = rect.top + DLG_ITEM_HEIGHT;
		msg.Format("0x%02X:", i);
		m_adcText[i].Create(msg, WS_CHILD, rect, this, m_adcTextID[i]);

		// draw edit boxes
		rect.left   = 10 + 3*(DLG_ITEM_WIDTH + DLG_ITEM_MARGIN_X);
		rect.right  = rect.left + DLG_ITEM_WIDTH;
		if (i == 36 || i == 37 || i == 38 || i == 39 || i == 42 || i == 43) {
			m_adcEdit[i].Create(WS_CHILD | ES_LEFT | WS_BORDER | WS_TABSTOP | ES_READONLY, rect, this, m_adcEditID[i]);
			m_adcEdit[i].SetBackgroundColor(FALSE, RGB(255, 255, 191));
		} else {
			m_adcEdit[i].Create(WS_CHILD | ES_LEFT | WS_BORDER | WS_TABSTOP | ES_READONLY, rect, this, m_adcEditID[i]);
		}
		m_adcEdit[i].LimitText(8);
	}

}

void CML506View::CreateDACregisters()
{
	RECT        rect;
	CString     msg;
	int         i;

	rect.top  = 40 - (DLG_ITEM_HEIGHT + DLG_ITEM_MARGIN_Y);
	for (i = 0; i < 18; i ++) {
		// draw labels
		rect.left   = 10;
		rect.right  = rect.left + DLG_ITEM_WIDTH*3.5;
		rect.top    = rect.top + DLG_ITEM_HEIGHT + DLG_ITEM_MARGIN_Y;
		rect.bottom = rect.top + DLG_ITEM_HEIGHT;
		m_dacText[i].Create(m_dacLabel[i], WS_CHILD, rect, this, m_dacTextID[i]);

		// draw edit boxes
		rect.left   = 10 + DLG_ITEM_WIDTH*3.5 + DLG_ITEM_MARGIN_X;
		rect.right  = rect.left + DLG_ITEM_WIDTH;
		m_dacEdit[i].Create(WS_CHILD | ES_LEFT | WS_BORDER | WS_TABSTOP | ES_READONLY, rect, this, m_dacEditID[i]);
		m_dacEdit[i].LimitText(8);
	}

}

void CML506View::CreateGrabberParams()
{
	RECT        rect;
	int         top;

	rect.left   = 10;
	rect.right  = rect.left + DLG_ITEM_WIDTH*2.5;
	rect.top    = 40;
	rect.bottom = rect.top + DLG_ITEM_HEIGHT;
	m_lblImgSizeX.Create("Image Width", WS_CHILD|WS_VISIBLE, rect, this, ID_LBL_IMAGEWIDTH);
	rect.left   = rect.right + 5;
	rect.right  = rect.left + DLG_ITEM_WIDTH*2+50;
	m_scrImgSizeX.Create(WS_CHILD|SBS_HORZ|WS_VISIBLE, rect, this, ID_SCROLL_IMAGEWIDTH);
	m_scrImgSizeX.SetScrollRange(1, 2048, TRUE);
	m_scrImgSizeX.EnableWindow(FALSE);

	rect.left   = 10;
	rect.right  = rect.left + DLG_ITEM_WIDTH*2.5;
	rect.top    = rect.bottom + 5;
	rect.bottom = rect.top + DLG_ITEM_HEIGHT;
	m_lblImgSizeY.Create("Image Height", WS_CHILD|WS_VISIBLE, rect, this, ID_LBL_IMAGEHEIGHT);
	rect.left   = rect.right + 5;
	rect.right  = rect.left + DLG_ITEM_WIDTH*2+50;
	m_scrImgSizeY.Create(WS_CHILD|SBS_HORZ|WS_VISIBLE, rect, this, ID_SCROLL_IMAGEHEIGHT);
	m_scrImgSizeY.SetScrollRange(1, 1000, TRUE);
	m_scrImgSizeY.EnableWindow(FALSE);


	rect.top    = rect.bottom + 15;
	rect.bottom = rect.top + DLG_ITEM_HEIGHT;
	rect.left   = 10;
	rect.right  = rect.left + DLG_ITEM_WIDTH*1.5;
	m_chkChannelR.Create("1. Reflection", WS_CHILD|BS_CHECKBOX|WS_VISIBLE, rect, this, ID_CHECK_CH_R);
	rect.left   = rect.right+ 5;
	rect.right  = rect.left + DLG_ITEM_WIDTH*1.5;
	m_chkChannelG.Create("2. Channel 2", WS_CHILD|BS_CHECKBOX|WS_VISIBLE, rect, this, ID_CHECK_CH_G);
	rect.left   = rect.right + 5;
	rect.right  = rect.left + DLG_ITEM_WIDTH*1.5;
	m_chkChannelB.Create("3. Channel 3", WS_CHILD|BS_CHECKBOX|WS_VISIBLE, rect, this, ID_CHECK_CH_B);
	m_chkChannelR.SetCheck(1);
	m_chkChannelG.SetCheck(0);
	m_chkChannelB.SetCheck(0);

	rect.top    = rect.bottom + 5;
	rect.bottom = rect.top + DLG_ITEM_HEIGHT;
	rect.left   = 175;
	rect.right  = rect.left + DLG_ITEM_WIDTH*1.6;
	m_chkChannelSW.Create("Switch to DF/SD", WS_CHILD|BS_CHECKBOX|!WS_VISIBLE, rect, this, ID_CHECK_SW_DF_SD);
	m_chkChannelSW.SetCheck(0);

	rect.left =10;
	rect.right =rect.left +DLG_ITEM_WIDTH*1.5;
	m_chkDiscardBlinks.Create(" Discard Blinks: MPV<45", WS_CHILD|BS_CHECKBOX|WS_VISIBLE, rect, this, ID_CHECK_DISC);
	m_chkDiscardBlinks.SetCheck(0);

	rect.left   = 10;
	rect.right  = rect.left + DLG_ITEM_WIDTH*2.5;
	rect.top    = rect.bottom + 15;
	rect.bottom = rect.top + DLG_ITEM_HEIGHT;
	m_lblBrightnessR.Create("Brightness\tCh1, 0-255", WS_CHILD|WS_VISIBLE, rect, this, ID_LBL_BRIGHTNESS_R);
	rect.left   = rect.right + 5;
	rect.right  = rect.left + DLG_ITEM_WIDTH*2+50;
	m_edtBrightnessR.Create(WS_CHILD|ES_LEFT|WS_BORDER|WS_TABSTOP|WS_VISIBLE, rect, this, ID_EDIT_BRIGHTNESS_R);
	m_edtBrightnessR.SetWindowText("127");
	m_edtBrightnessR.EnableWindow(FALSE);
	m_edtBrightnessR.SendMessage(EM_SETEVENTMASK, 0, ENM_KEYEVENTS);

	rect.left   = 10;
	rect.right  = rect.left + DLG_ITEM_WIDTH*2.5;
	rect.top    = rect.bottom + 5;
	rect.bottom = rect.top + DLG_ITEM_HEIGHT;
	m_lblContrastR.Create("Contrast\t\tCh1, 0-127", WS_CHILD|WS_VISIBLE, rect, this, ID_LBL_CONTRAST_R);
	rect.left   = rect.right + 5;
	rect.right  = rect.left + DLG_ITEM_WIDTH*2+50;
	m_edtContrastR.Create(WS_CHILD|ES_LEFT|WS_BORDER|WS_TABSTOP|WS_VISIBLE, rect, this, ID_EDIT_CONTRAST_R);
	m_edtContrastR.SetWindowText("63");
	m_edtContrastR.EnableWindow(FALSE);
	m_edtContrastR.SendMessage(EM_SETEVENTMASK, 0, ENM_KEYEVENTS);

	rect.left   = 10;
	rect.right  = rect.left + DLG_ITEM_WIDTH*2.5;
	rect.top    = rect.bottom + 5;
	rect.bottom = rect.top + DLG_ITEM_HEIGHT;
	m_lblBrightnessG.Create("Brightness\tCh2, 0-255", WS_CHILD|WS_VISIBLE, rect, this, ID_LBL_BRIGHTNESS_G);
	rect.left   = rect.right + 5;
	rect.right  = rect.left + DLG_ITEM_WIDTH*2+50;
	m_edtBrightnessG.Create(WS_CHILD|ES_LEFT|WS_BORDER|WS_TABSTOP|WS_VISIBLE, rect, this, ID_EDIT_BRIGHTNESS_G);
	m_edtBrightnessG.SetWindowText("127");
	m_edtBrightnessG.EnableWindow(FALSE);
	m_edtBrightnessG.SendMessage(EM_SETEVENTMASK, 0, ENM_KEYEVENTS);

	rect.left   = 10;
	rect.right  = rect.left + DLG_ITEM_WIDTH*2.5;
	rect.top    = rect.bottom + 5;
	rect.bottom = rect.top + DLG_ITEM_HEIGHT;
	m_lblContrastG.Create("Contrast\t\tCh2, 0-127", WS_CHILD|WS_VISIBLE, rect, this, ID_LBL_CONTRAST_G);
	rect.left   = rect.right + 5;
	rect.right  = rect.left + DLG_ITEM_WIDTH*2+50;
	m_edtContrastG.Create(WS_CHILD|ES_LEFT|WS_BORDER|WS_TABSTOP|WS_VISIBLE, rect, this, ID_EDIT_CONTRAST_G);
	m_edtContrastG.SetWindowText("63");
	m_edtContrastG.EnableWindow(FALSE);
	m_edtContrastG.SendMessage(EM_SETEVENTMASK, 0, ENM_KEYEVENTS);

	rect.left   = 10;
	rect.right  = rect.left + DLG_ITEM_WIDTH*2.5;
	rect.top    = rect.bottom + 5;
	rect.bottom = rect.top + DLG_ITEM_HEIGHT;
	m_lblBrightnessB.Create("Brightness\tCh3, 0-255", WS_CHILD|WS_VISIBLE, rect, this, ID_LBL_BRIGHTNESS_B);
	rect.left   = rect.right + 5;
	rect.right  = rect.left + DLG_ITEM_WIDTH*2+50;
	m_edtBrightnessB.Create(WS_CHILD|ES_LEFT|WS_BORDER|WS_TABSTOP|WS_VISIBLE, rect, this, ID_EDIT_BRIGHTNESS_B);
	m_edtBrightnessB.SetWindowText("127");
	m_edtBrightnessB.EnableWindow(FALSE);
	m_edtBrightnessB.SendMessage(EM_SETEVENTMASK, 0, ENM_KEYEVENTS);

	rect.left   = 10;
	rect.right  = rect.left + DLG_ITEM_WIDTH*2.5;
	rect.top    = rect.bottom + 5;
	rect.bottom = rect.top + DLG_ITEM_HEIGHT;
	m_lblContrastB.Create("Contrast\t\tCh3, 0-127", WS_CHILD|WS_VISIBLE, rect, this, ID_LBL_CONTRAST_B);
	rect.left   = rect.right + 5;
	rect.right  = rect.left + DLG_ITEM_WIDTH*2+50;
	m_edtContrastB.Create(WS_CHILD|ES_LEFT|WS_BORDER|WS_TABSTOP|WS_VISIBLE, rect, this, ID_EDIT_CONTRAST_B);
	m_edtContrastB.SetWindowText("63");
	m_edtContrastB.EnableWindow(FALSE);
	m_edtContrastB.SendMessage(EM_SETEVENTMASK, 0, ENM_KEYEVENTS);


	rect.left   = 10;
	rect.right  = rect.left + DLG_ITEM_WIDTH*2.5;
	rect.top    = rect.bottom + 5;
	rect.bottom = rect.top + DLG_ITEM_HEIGHT;
	m_lblOffsetX.Create("Img Offset-X (pixels)", WS_CHILD|WS_VISIBLE, rect, this, ID_LBL_OFFSETX);
	rect.left   = rect.right + 5;
	rect.right  = rect.left + DLG_ITEM_WIDTH*2+50;
	m_scrOffsetX.Create(WS_CHILD|SBS_HORZ|WS_VISIBLE|WS_TABSTOP, rect, this, ID_SCROLL_OFFSETX);
	m_scrOffsetX.SetScrollRange(0, 1023, TRUE);

	rect.left   = 10;
	rect.right  = rect.left + DLG_ITEM_WIDTH*2.5;
	rect.top    = rect.bottom + 5;
	rect.bottom = rect.top + DLG_ITEM_HEIGHT;
	m_lblOffsetY.Create("Img Offset-Y (lines)", WS_CHILD|WS_VISIBLE, rect, this, ID_LBL_OFFSETY);
	rect.left   = rect.right + 5;
	rect.right  = rect.left + DLG_ITEM_WIDTH*2+50;
	m_scrOffsetY.Create(WS_CHILD|SBS_HORZ|WS_VISIBLE|WS_TABSTOP, rect, this, ID_SCROLL_OFFSETY);
	m_scrOffsetY.SetScrollRange(0, 255, TRUE);

	rect.left   = 10;
	rect.right  = rect.left + DLG_ITEM_WIDTH+20;
	rect.top    = rect.bottom;
	rect.bottom = rect.top + DLG_ITEM_HEIGHT;
//	m_chkBidirection.Create("Dual FS/BS", WS_CHILD|BS_CHECKBOX|!WS_VISIBLE, rect, this, ID_CHECK_SAMPLE_BOTH);
	m_chkBidirection.Create("Desinusoid", WS_CHILD|BS_CHECKBOX|WS_VISIBLE, rect, this, ID_CHECK_SAMPLE_BOTH);
	rect.top    = rect.top + 5;
	rect.bottom = rect.bottom + 5;
	rect.left   = rect.right + 5;
	rect.right  = rect.left + DLG_ITEM_WIDTH*1.5-25;
	m_lblBlankPixels.Create("BS offset,0", WS_CHILD|!WS_VISIBLE, rect, this, ID_LBL_SAMPLE_BOTH);
	rect.left   = rect.right + 5;
	rect.right  = rect.left + DLG_ITEM_WIDTH*2+50;
	m_scrBlankPixels.Create(WS_CHILD|SBS_HORZ|!WS_VISIBLE|WS_TABSTOP, rect, this, ID_SCROLL_SAMPLE_BOTH);
	m_scrBlankPixels.SetScrollRange(1, 1024, TRUE);
	m_scrBlankPixels.EnableWindow(FALSE);
	m_iShiftBS = 1;

	// frame counter option: by v-sync or by scanning line number
	rect.left   = 10;
	rect.right  = rect.left + DLG_ITEM_WIDTH*2.5;
	rect.top    = rect.bottom + 5;
	rect.bottom = rect.top + DLG_ITEM_HEIGHT;
	m_lblVstart.Create("Frame Counter By:", WS_CHILD|WS_VISIBLE, rect, this, ID_LBL_FRAME_COUNTER);
	// add items for saving video
	rect.left   = rect.right + 5;
	rect.right  = rect.left + DLG_ITEM_WIDTH+50;
	m_radLines.Create("Scannning lines", WS_CHILD|BS_RADIOBUTTON|WS_TABSTOP|WS_VISIBLE, rect, this, ID_FRAME_COUNTER_LINES);
	m_radLines.SetCheck(1);
	rect.left   = rect.right + 5;
	rect.right  = rect.left + DLG_ITEM_WIDTH;
	m_radVsync.Create("V-sync", WS_CHILD|BS_RADIOBUTTON|WS_TABSTOP|WS_VISIBLE, rect, this, ID_FRAME_COUNTER_VSYNC);
	m_radVsync.SetCheck(0);
	m_radLines.EnableWindow(FALSE);
	m_radVsync.EnableWindow(FALSE);

	rect.left   = 10;
	rect.right  = rect.left + DLG_ITEM_WIDTH*2.5;
	rect.top    = rect.bottom + 5;
	rect.bottom = rect.top + DLG_ITEM_HEIGHT;
	m_lblPixelClock.Create("Pixels per line, 512-4095", WS_CHILD|!WS_VISIBLE, rect, this, ID_LBL_PCLOCK);
	rect.left   = rect.right + 5;
	rect.right  = rect.left + DLG_ITEM_WIDTH*2+50;
	m_edtPixelClock.Create(WS_CHILD|ES_LEFT|WS_BORDER|WS_TABSTOP|!WS_VISIBLE, rect, this, ID_EDIT_PCLOCK);
	CString text;
	text.Format(_T("%d"), m_iPLLclks<<1);
	m_edtPixelClock.SetWindowText(text);
	m_edtPixelClock.SendMessage(EM_SETEVENTMASK, 0, ENM_KEYEVENTS);


	rect.left   = 10;
	rect.right  = rect.left + DLG_ITEM_WIDTH*2.5;
	rect.top    = rect.bottom + 5;
	rect.bottom = rect.top + DLG_ITEM_HEIGHT;
	top         = rect.bottom;
	m_lblHsyncI.Create("Hsync Input", WS_CHILD|WS_VISIBLE, rect, this, ID_LBL_HSYNCI);
	rect.left   = rect.right + 5;
	rect.right  = rect.left + DLG_ITEM_WIDTH*2+50;
	rect.bottom = rect.top + DLG_ITEM_HEIGHT*3;
	m_cboHsyncI.Create(WS_CHILD|CBS_DROPDOWNLIST|WS_VISIBLE, rect, this, ID_CBO_HSYNCI);
	m_cboHsyncI.InsertString(0, "Active Low");
	m_cboHsyncI.InsertString(1, "Active High");

	rect.left   = 10;
	rect.right  = rect.left + DLG_ITEM_WIDTH*2.5;
	rect.top    = top + 5;
	rect.bottom = rect.top + DLG_ITEM_HEIGHT;
	top         = rect.bottom;
	m_lblVsyncI.Create("Vsync Input", WS_CHILD|WS_VISIBLE, rect, this, ID_LBL_VSYNCI);
	rect.left   = rect.right + 5;
	rect.right  = rect.left + DLG_ITEM_WIDTH*2+50;
	rect.bottom = rect.top + DLG_ITEM_HEIGHT*3;
	m_cboVsyncI.Create(WS_CHILD|CBS_DROPDOWNLIST|WS_VISIBLE, rect, this, ID_CBO_VSYNCI);
	m_cboVsyncI.InsertString(0, "Active Low");
	m_cboVsyncI.InsertString(1, "Active High");

	rect.left   = 10;
	rect.right  = rect.left + DLG_ITEM_WIDTH*2.5;
	rect.top    = top + 5;
	rect.bottom = rect.top + DLG_ITEM_HEIGHT;
	top         = rect.bottom;
	m_lblHsyncO.Create("Hsync Output", WS_CHILD/*|WS_VISIBLE*/, rect, this, ID_LBL_HSYNCO);
	rect.left   = rect.right + 5;
	rect.right  = rect.left + DLG_ITEM_WIDTH*2+50;
	rect.bottom = rect.top + DLG_ITEM_HEIGHT*3;
	m_cboHsyncO.Create(WS_CHILD|CBS_DROPDOWNLIST/*|WS_VISIBLE*/, rect, this, ID_CBO_HSYNCO);
	m_cboHsyncO.InsertString(0, "Active Low");
	m_cboHsyncO.InsertString(1, "Active High");
	m_cboHsyncO.EnableWindow(FALSE);

	rect.left   = 10;
	rect.right  = rect.left + DLG_ITEM_WIDTH*2.5;
	rect.top    = top + 10;
	rect.bottom = rect.top + DLG_ITEM_HEIGHT;
	top         = rect.bottom;
	m_lblVsyncO.Create("Vsync Output", WS_CHILD/*|WS_VISIBLE*/, rect, this, ID_LBL_VSYNCO);
	rect.left   = rect.right + 5;
	rect.right  = rect.left + DLG_ITEM_WIDTH*2;
	rect.bottom = rect.top + DLG_ITEM_HEIGHT*3;
	m_cboVsyncO.Create(WS_CHILD|CBS_DROPDOWNLIST/*|WS_VISIBLE*/, rect, this, ID_CBO_VSYNCO);
	m_cboVsyncO.InsertString(0, "Active Low");
	m_cboVsyncO.InsertString(1, "Active High");
	m_cboVsyncO.EnableWindow(FALSE);
/*
	rect.top    = top + 10;
	rect.bottom = rect.top + DLG_ITEM_HEIGHT;
	m_chkExternalClock.Create("External Pixel Clock", WS_CHILD|BS_CHECKBOX, rect, this, ID_CHECK_EXTCLOCK);
	m_chkExternalClock.SetCheck(1);
	m_chkExternalClock.EnableWindow(FALSE);
*/
	rect.left   = 10;
	rect.right  = rect.left + DLG_ITEM_WIDTH*2;
	rect.top    = rect.bottom + 5;
	rect.bottom = rect.top + DLG_ITEM_HEIGHT;
	m_btnLoadStimulus.Create("Load Stimulus", WS_CHILD|BS_DEFPUSHBUTTON|WS_TABSTOP|!WS_VISIBLE|WS_DISABLED, rect, this, ID_LOAD_STIMULUS);
//	m_btnLoadStimulus.EnableWindow(FALSE);

//	m_btnDesinusoidCal.Create("Sinusoidal Calibration", WS_CHILD|BS_DEFPUSHBUTTON|WS_TABSTOP/*|WS_VISIBLE*/, rect, this, ID_DESINUSOID_CAL);


	rect.left   = rect.right + 50;
	rect.right  = rect.left + DLG_ITEM_WIDTH*2;
	m_btnSaveVideo.Create("Save Video", WS_CHILD|BS_DEFPUSHBUTTON|WS_TABSTOP|WS_VISIBLE, rect, this, ID_VIDEO_SAVE);

	// add items for saving video
	rect.top    = rect.bottom + 20;
	rect.bottom = rect.top + DLG_ITEM_HEIGHT;
	rect.left   = 10;
	rect.right  = rect.left + DLG_ITEM_WIDTH*2.3;
	m_rdoLenthInSeconds.Create("Video length in seconds", WS_CHILD|BS_RADIOBUTTON|WS_TABSTOP|WS_VISIBLE, rect, this, ID_VIDEO_SECONDS);
	rect.left   = rect.right + 5;
	rect.right  = rect.left + DLG_ITEM_WIDTH*2.3;
	m_rdoLenthInFrames.Create("Video length in frames", WS_CHILD|BS_RADIOBUTTON|WS_TABSTOP|WS_VISIBLE, rect, this, ID_VIDEO_FRAMES);
	m_rdoLenthInSeconds.SetCheck(1);

	rect.left   = 10;
	rect.right  = rect.left + DLG_ITEM_WIDTH*2;
	rect.top    = rect.bottom + 5;
	rect.bottom = rect.top + DLG_ITEM_HEIGHT;
	m_lblVideoName.Create("Video Folder", WS_CHILD|WS_VISIBLE, rect, this, ID_VIDEO_FILENAME_LBL);
	rect.left   = rect.right + DLG_ITEM_WIDTH/2;
	rect.right  = rect.left + DLG_ITEM_WIDTH;
	m_lblVideoLength.Create("Length(s)", WS_CHILD|WS_VISIBLE, rect, this, ID_VIDEO_LENGTH_LBL);
	rect.left   = rect.right;
	rect.right  = rect.left + DLG_ITEM_WIDTH+60;
//	m_lblVideoPrefix.Create("Prefix    Flags(fs/bs)", WS_CHILD|WS_VISIBLE, rect, this, ID_VIDEO_PREFIX_LBL);
	m_lblVideoPrefix.Create("Prefix", WS_CHILD|WS_VISIBLE, rect, this, ID_VIDEO_PREFIX_LBL);

	rect.left   = 10;
	rect.right  = rect.left + DLG_ITEM_WIDTH*2;
	rect.top    = rect.bottom;
	rect.bottom = rect.top + DLG_ITEM_HEIGHT;
	m_edtVideoNameR.Create(WS_CHILD|ES_LEFT|WS_BORDER|WS_TABSTOP|WS_VISIBLE|ES_AUTOHSCROLL, rect, this, ID_EDIT_FOLDER_R);
	rect.left   = rect.right + DLG_ITEM_MARGIN_X;
	rect.right  = rect.left + DLG_ITEM_WIDTH/2-10;
	m_btnVideoFolderR.Create("...", WS_CHILD|BS_DEFPUSHBUTTON|WS_TABSTOP|WS_VISIBLE, rect, this, ID_BUTTON_FOLDER_R);
	rect.left   = rect.right + DLG_ITEM_MARGIN_X;
	rect.right  = rect.left + DLG_ITEM_WIDTH-20;
	m_edtVideoLengthR.Create(WS_CHILD|ES_LEFT|WS_BORDER|WS_TABSTOP|WS_VISIBLE|ES_AUTOHSCROLL, rect, this, ID_EDIT_VIDEO_LENGTH_R);
	m_edtVideoLengthR.SetWindowText(_T("10"));
	rect.left   = rect.right + DLG_ITEM_MARGIN_X;
	rect.right  = rect.left + DLG_ITEM_WIDTH-20;
	m_edtVideoPrefixR.Create(WS_CHILD|ES_LEFT|WS_BORDER|WS_TABSTOP|WS_VISIBLE|ES_AUTOHSCROLL, rect, this, ID_EDIT_VIDEO_PREFIX_R);
	m_edtVideoPrefixR.SetWindowText(_T("ch1vid"));
	rect.left   = rect.right + DLG_ITEM_MARGIN_X;
	rect.right  = rect.left + DLG_ITEM_WIDTH/2;
	m_edtVideoFlagsR1.Create(WS_CHILD|ES_LEFT|WS_BORDER|WS_TABSTOP|!WS_VISIBLE|ES_AUTOHSCROLL, rect, this, ID_EDIT_VIDEO_FLAG_R1);
	m_edtVideoFlagsR1.SetWindowText(_T("fs"));
	rect.left   = rect.right + DLG_ITEM_MARGIN_X;
	rect.right  = rect.left + DLG_ITEM_WIDTH/2;
	m_edtVideoFlagsR2.Create(WS_CHILD|ES_LEFT|WS_BORDER|WS_TABSTOP|!WS_VISIBLE|ES_AUTOHSCROLL, rect, this, ID_EDIT_VIDEO_FLAG_R2);
	m_edtVideoFlagsR2.SetWindowText(_T("bs"));
	m_edtVideoFlagsR1.EnableWindow(FALSE);
	m_edtVideoFlagsR2.EnableWindow(FALSE);

	rect.left   = 10;
	rect.right  = rect.left + DLG_ITEM_WIDTH*2;
	rect.top    = rect.bottom;
	rect.bottom = rect.top + DLG_ITEM_HEIGHT;
	m_edtVideoNameG.Create(WS_CHILD|ES_LEFT|WS_BORDER|WS_TABSTOP|!WS_VISIBLE|ES_AUTOHSCROLL, rect, this, ID_EDIT_FOLDER_G);
	rect.left   = rect.right + DLG_ITEM_MARGIN_X;
	rect.right  = rect.left + DLG_ITEM_WIDTH/2-10;
	m_btnVideoFolderG.Create("...", WS_CHILD|BS_DEFPUSHBUTTON|WS_TABSTOP|!WS_VISIBLE, rect, this, ID_BUTTON_FOLDER_G);
	rect.left   = rect.right + DLG_ITEM_MARGIN_X;
	rect.right  = rect.left + DLG_ITEM_WIDTH-20;
	m_edtVideoLengthG.Create(WS_CHILD|ES_LEFT|WS_BORDER|WS_TABSTOP|!WS_VISIBLE|ES_AUTOHSCROLL, rect, this, ID_EDIT_VIDEO_LENGTH_G);
	rect.left   = rect.right + DLG_ITEM_MARGIN_X;
	rect.right  = rect.left + DLG_ITEM_WIDTH-20;
	m_edtVideoPrefixG.Create(WS_CHILD|ES_LEFT|WS_BORDER|WS_TABSTOP|WS_VISIBLE|ES_AUTOHSCROLL, rect, this, ID_EDIT_VIDEO_PREFIX_G);
	m_edtVideoPrefixG.SetWindowText(_T("ch2vid"));
	rect.left   = rect.right + DLG_ITEM_MARGIN_X;
	rect.right  = rect.left + DLG_ITEM_WIDTH/2;
	m_edtVideoFlagsG1.Create(WS_CHILD|ES_LEFT|WS_BORDER|WS_TABSTOP|!WS_VISIBLE|ES_AUTOHSCROLL, rect, this, ID_EDIT_VIDEO_FLAG_G1);
	m_edtVideoFlagsG1.SetWindowText(_T("fs"));
	rect.left   = rect.right + DLG_ITEM_MARGIN_X;
	rect.right  = rect.left + DLG_ITEM_WIDTH/2;
	m_edtVideoFlagsG2.Create(WS_CHILD|ES_LEFT|WS_BORDER|WS_TABSTOP|!WS_VISIBLE|ES_AUTOHSCROLL, rect, this, ID_EDIT_VIDEO_FLAG_G2);
	m_edtVideoFlagsG2.SetWindowText(_T("bs"));
	m_edtVideoLengthG.EnableWindow(FALSE);
	m_edtVideoNameG.EnableWindow(FALSE);
	m_btnVideoFolderG.EnableWindow(FALSE);
	m_edtVideoPrefixG.EnableWindow(FALSE);
	m_edtVideoFlagsG1.EnableWindow(FALSE);
	m_edtVideoFlagsG2.EnableWindow(FALSE);

	rect.left   = 10;
	rect.right  = rect.left + DLG_ITEM_WIDTH*2;
	rect.top    = rect.bottom;
	rect.bottom = rect.top + DLG_ITEM_HEIGHT;
	m_edtVideoNameB.Create(WS_CHILD|ES_LEFT|WS_BORDER|WS_TABSTOP|!WS_VISIBLE|ES_AUTOHSCROLL, rect, this, ID_EDIT_FOLDER_B);
	rect.left   = rect.right + DLG_ITEM_MARGIN_X;
	rect.right  = rect.left + DLG_ITEM_WIDTH/2-10;
	m_btnVideoFolderB.Create("...", WS_CHILD|BS_DEFPUSHBUTTON|WS_TABSTOP|!WS_VISIBLE, rect, this, ID_BUTTON_FOLDER_B);
	rect.left   = rect.right + DLG_ITEM_MARGIN_X;
	rect.right  = rect.left + DLG_ITEM_WIDTH-20;
	m_edtVideoLengthB.Create(WS_CHILD|ES_LEFT|WS_BORDER|WS_TABSTOP|!WS_VISIBLE|ES_AUTOHSCROLL, rect, this, ID_EDIT_VIDEO_LENGTH_B);
	rect.left   = rect.right + DLG_ITEM_MARGIN_X;
	rect.right  = rect.left + DLG_ITEM_WIDTH-20;
	m_edtVideoPrefixB.Create(WS_CHILD|ES_LEFT|WS_BORDER|WS_TABSTOP|WS_VISIBLE|ES_AUTOHSCROLL, rect, this, ID_EDIT_VIDEO_PREFIX_B);
	m_edtVideoPrefixB.SetWindowText(_T("ch3vid"));
	rect.left   = rect.right + DLG_ITEM_MARGIN_X;
	rect.right  = rect.left + DLG_ITEM_WIDTH/2;
	m_edtVideoFlagsB1.Create(WS_CHILD|ES_LEFT|WS_BORDER|WS_TABSTOP|!WS_VISIBLE|ES_AUTOHSCROLL, rect, this, ID_EDIT_VIDEO_FLAG_B1);
	m_edtVideoFlagsB1.SetWindowText(_T("fs"));
	rect.left   = rect.right + DLG_ITEM_MARGIN_X;
	rect.right  = rect.left + DLG_ITEM_WIDTH/2;
	m_edtVideoFlagsB2.Create(WS_CHILD|ES_LEFT|WS_BORDER|WS_TABSTOP|!WS_VISIBLE|ES_AUTOHSCROLL, rect, this, ID_EDIT_VIDEO_FLAG_B2);
	m_edtVideoFlagsB2.SetWindowText(_T("bs"));
	m_edtVideoLengthB.EnableWindow(FALSE);
	m_edtVideoNameB.EnableWindow(FALSE);
	m_btnVideoFolderB.EnableWindow(FALSE);
	m_edtVideoPrefixB.EnableWindow(FALSE);
	m_edtVideoFlagsB1.EnableWindow(FALSE);
	m_edtVideoFlagsB2.EnableWindow(FALSE);

}

int CML506View::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CView::OnCreate(lpCreateStruct) == -1)
		return -1;

	RECT  rect;

	rect.left   = 0;
	rect.top    = 0;
	rect.bottom = 800;
	rect.right  = 20;
	m_tabPCIe.Create(TCS_MULTILINE | WS_CHILD | WS_VISIBLE, rect, this, ID_TAB_PCIE);
	m_tabPCIe.InsertItem(0, "Grabber Params");
//	m_tabPCIe.InsertItem(1, "Scanner Ctrls");
	m_tabPCIe.InsertItem(2, "PCIe Regs");
	m_tabPCIe.InsertItem(3, "ADC Regs");
//	m_tabPCIe.InsertItem(4, "DAC Regs");
	//m_tabPCIe.InsertItem(4, "Cfg Space");

//	CreateCfgSpace();
	CreateScannerParams();
	CreateRTregisters();
	CreateADCregisters();
	CreateDACregisters();
	CreateGrabberParams();


	// label "current register"
	rect.top    = 750;
	rect.bottom = rect.top + DLG_ITEM_HEIGHT;
	rect.left   = 10;
	rect.right  = rect.left + DLG_ITEM_WIDTH*2;
	m_TextCurrentReg.Create("Current Position: 0x", WS_CHILD | ES_RIGHT, rect, this, ID_CURRENT_REG);

	// edit box "register address"
	rect.left   = 10 + rect.right;
	rect.right  = rect.left + DLG_ITEM_WIDTH;
	m_EditCurrentRegAddr.Create(WS_CHILD | ES_LEFT | WS_BORDER | WS_TABSTOP, rect, this, ID_CURRENT_REG_ADDR);
	m_EditCurrentRegAddr.SetBackgroundColor(FALSE, RGB(0, 255, 0));
	m_EditCurrentRegAddr.LimitText(2);

	// edit box "register value"
	rect.left   = 10 + rect.right;
	rect.right  = rect.left + DLG_ITEM_WIDTH;
	m_EditCurrentRegVal.Create(WS_CHILD | ES_LEFT | WS_BORDER | WS_TABSTOP, rect, this, ID_CURRENT_REG_VAL);
	m_EditCurrentRegVal.SetBackgroundColor(FALSE, RGB(0, 255, 0));
	m_EditCurrentRegVal.LimitText(8);

	rect.top    = rect.top - DLG_ITEM_HEIGHT - DLG_ITEM_MARGIN_Y;
	rect.bottom = rect.top + DLG_ITEM_HEIGHT;
	rect.left   = 10 + (DLG_ITEM_WIDTH + DLG_ITEM_MARGIN_X) + DLG_ITEM_WIDTH;
	rect.right  = rect.left + DLG_ITEM_WIDTH;
	m_TextCurrentRegAddr.Create("Offset", WS_CHILD | ES_CENTER, rect, this, ID_CURRENT_REG_TADDR);

	rect.left   = 10 + 2*(DLG_ITEM_WIDTH + DLG_ITEM_MARGIN_X) + DLG_ITEM_WIDTH;
	rect.right  = rect.left + DLG_ITEM_WIDTH;
	m_TextCurrentRegVal.Create("Value", WS_CHILD | ES_CENTER, rect, this, ID_CURRENT_REG_TVAL);

	// push button "Write one register"
	rect.left   = 10 + DLG_ITEM_WIDTH + DLG_ITEM_MARGIN_X;
	rect.right  = rect.left + DLG_ITEM_WIDTH;
	rect.top    = 730 + 2*(DLG_ITEM_HEIGHT + DLG_ITEM_MARGIN_Y);
	rect.bottom = rect.top + DLG_ITEM_HEIGHT;
	m_WriteOne.Create("Write One", WS_CHILD | BS_DEFPUSHBUTTON | WS_TABSTOP, rect, this, ID_WRITE_ONE);

	// push button "read one register"
	rect.left   = rect.right + DLG_ITEM_MARGIN_X;
	rect.right  = rect.left + DLG_ITEM_WIDTH;
	m_ReadOne.Create("Read One", WS_CHILD | BS_DEFPUSHBUTTON | WS_TABSTOP, rect, this, ID_READ_ONE);

	// push button "read all registers"
	rect.left   = rect.right + DLG_ITEM_MARGIN_X;
	rect.right  = rect.left + DLG_ITEM_WIDTH;
	m_ReadAll.Create("Read All", WS_CHILD | BS_DEFPUSHBUTTON | WS_TABSTOP, rect, this, ID_READ_ALL);


	EnableGuiControls(FALSE);
	/*
	// edit box "PCIe data"
	rect.left   = 425;
	rect.top    = 25;
	rect.right  = rect.left + 790;
	rect.bottom = rect.top + 590;
	m_pcieData.Create(WS_CHILD | ES_LEFT | WS_BORDER | WS_VISIBLE | ES_MULTILINE | ES_AUTOHSCROLL | ES_AUTOVSCROLL, rect, this, ID_PCIE_DATA);

	*/
/*
	m_pDlgR         = new CVideoChannelR(this);
	if (m_pDlgR->GetSafeHwnd() == 0) {
        m_pDlgR->Create(IDD_DIALOG_CH1); // displays the dialog window
		m_pDlgR->ShowWindow(SW_SHOW);
	}

    if (m_pDlgG->GetSafeHwnd() == 0) {
        m_pDlgG->Create(IDD_DIALOG_CH2); // displays the dialog window
		//m_pDlgG->ShowWindow(SW_SHOW);
	}

    if (m_pDlgB->GetSafeHwnd() == 0) {
        m_pDlgB->Create(IDD_DIALOG_CH3); // displays the dialog window
		//m_pDlgB->ShowWindow(SW_SHOW);
	}
*/
	return 0;
}


void CML506View::CreateCfgSpace()
{
	RECT        rect;
	int         i;
	UINT64      cfgData;
	char       *label;
	char       *data;
	BOOL        ret;

	label = new char [256];
	data  = new char [17];

	rect.top  = 40 - (DLG_ITEM_HEIGHT + DLG_ITEM_MARGIN_Y);
	for (i = 0; i < VIRTEX5_CFG_REGS_NUM+2; i ++) {
		// draw labels
		rect.left   = 10;
		rect.right  = 395;//rect.left + (int)(DLG_ITEM_WIDTH * 4);
		rect.top    = rect.top + DLG_ITEM_HEIGHT-2;
		rect.bottom = rect.top + DLG_ITEM_HEIGHT-2;

		strcpy(label, m_cfgLabel[i]);
		if (i >= 2 && g_hDevVirtex5 != NULL) {
			ret = g_objVirtex5BMD.ReadWriteReg(g_hDevVirtex5, gVIRTEX5_CfgRegs, i-1, VIRTEX5_CFG_REGS_NUM, WDC_READ, TRUE, &cfgData, NULL);
			if (ret == TRUE) {
				switch (gVIRTEX5_CfgRegs[i-2].dwSize)
				{
				case WDC_SIZE_8:
					sprintf_s(data, 17, "0x%02X", cfgData);
					break;
				case WDC_SIZE_16:
					sprintf_s(data, 17, "0x%04X", cfgData);
					break;
				case WDC_SIZE_32:
					sprintf_s(data, 17, "0x%08X", cfgData);
					break;
				case WDC_SIZE_64:
					sprintf_s(data, 17, "0x%16X", cfgData);
					break;
				default:
					strcpy(data, "error !!!");
				}
			} else {
				strcpy(data, "failed !!!");
			}
			strcat(label, data);
		}

		//m_cfgData[i].Create(label, WS_CHILD, rect, this, m_cfgDataID[i]);
	}

	delete [] label;
	delete [] data;
}

void CML506View::CreateScannerParams()
{
	m_fovH         = g_VideoInfo.fovH;
	m_fovV         = g_VideoInfo.fovV;
	m_forwardLines = g_VideoInfo.fLines;
	m_backLines    = (int)(0.5+1.0*g_VideoInfo.fLines/g_VideoInfo.bStepSize);
	m_rampOffset   = g_VideoInfo.vOffset;
	m_bInterleaveLines    = g_VideoInfo.bDoubleLine;

	RECT        rect;
	int         top;

	// resonant scanner FOV
	rect.left   = 10;
	rect.right  = rect.left + DLG_ITEM_WIDTH*2.5;
	rect.top    = 40;
	rect.bottom = rect.top + DLG_ITEM_HEIGHT;
	m_lblResonant.Create("Resonant FOV", WS_CHILD|WS_VISIBLE, rect, this, ID_LBL_RESONANT);
	rect.left   = rect.right + 5;
	rect.right  = rect.left + DLG_ITEM_WIDTH*2+50;
	m_scrResonant.Create(WS_CHILD|SBS_HORZ|WS_VISIBLE|WS_TABSTOP, rect, this, ID_SCR_RESONANT);
	m_scrResonant.SetScrollRange(0, 255, TRUE);
	m_scrResonant.SetScrollPos(m_fovH);
	m_scrResonant.EnableWindow(FALSE);

	// galvo scanner FOV
	rect.left   = 10;
	rect.right  = rect.left + DLG_ITEM_WIDTH*2.5;
	rect.top    = rect.bottom + 5;
	rect.bottom = rect.top + DLG_ITEM_HEIGHT;
	m_lblGalvo.Create("Slow Scan FOV", WS_CHILD|WS_VISIBLE, rect, this, ID_LBL_GALVO);
	rect.left   = rect.right + 5;
	rect.right  = rect.left + DLG_ITEM_WIDTH*2+50;
	m_scrGalvo.Create(WS_CHILD|SBS_HORZ|WS_VISIBLE|WS_TABSTOP, rect, this, ID_SCR_GALVO);
	m_scrGalvo.SetScrollRange(0, 31, TRUE);
	m_scrGalvo.SetScrollPos(m_fovV);
	m_scrGalvo.EnableWindow(FALSE);

	// # of lines in galvo forward scanning
	rect.left   = 10;
	rect.right  = rect.left + DLG_ITEM_WIDTH*2.5;
	rect.top    = rect.bottom + 5;
	rect.bottom = rect.top + DLG_ITEM_HEIGHT;
	m_lblGalvoForward.Create("Forward Lines", WS_CHILD|WS_VISIBLE, rect, this, ID_LBL_GALVOFORWARD);
	rect.left   = rect.right + 5;
	rect.right  = rect.left + DLG_ITEM_WIDTH*2+50;
	m_scrGalvoForward.Create(WS_CHILD|SBS_HORZ|WS_VISIBLE|WS_TABSTOP, rect, this, ID_SCR_GALVOFORWARD);
	m_scrGalvoForward.SetScrollRange(64, 1000, TRUE);
	m_scrGalvoForward.SetScrollPos(m_forwardLines);
	m_scrGalvoForward.EnableWindow(FALSE);

	// # of lines in galvo backward scanning
	rect.left   = 10;
	rect.right  = rect.left + DLG_ITEM_WIDTH*2.5;
	rect.top    = rect.bottom + 5;
	rect.bottom = rect.top + DLG_ITEM_HEIGHT;
	m_lblGalvoBackward.Create("Backward Lines", WS_CHILD|WS_VISIBLE, rect, this, ID_LBL_GALVOBACKWARD);
	rect.left   = rect.right + 5;
	rect.right  = rect.left + DLG_ITEM_WIDTH*2+50;
	m_scrGalvoBackward.Create(WS_CHILD|SBS_HORZ|WS_VISIBLE|WS_TABSTOP, rect, this, ID_SCR_GALVOBACKWARD);
	m_scrGalvoBackward.SetScrollRange(3, 1000, TRUE);
	m_scrGalvoBackward.SetScrollPos(m_backLines);
	m_scrGalvoBackward.EnableWindow(FALSE);

	// ramp offset
	rect.left   = 10;
	rect.right  = rect.left + DLG_ITEM_WIDTH*2.5;
	rect.top    = rect.bottom + 5;
	rect.bottom = rect.top + DLG_ITEM_HEIGHT;
	m_lblRampOffset.Create("Ramp offset", WS_CHILD|WS_VISIBLE, rect, this, ID_LBL_RAMPOFFSET);
	rect.left   = rect.right + 5;
	rect.right  = rect.left + DLG_ITEM_WIDTH*2+50;
	m_scrRampOffset.Create(WS_CHILD|SBS_HORZ|WS_VISIBLE|WS_TABSTOP, rect, this, ID_SCR_RAMPOFFSET);
	m_scrRampOffset.SetScrollRange(32, 768, TRUE);
	m_scrRampOffset.SetScrollPos(m_rampOffset);
	m_scrRampOffset.EnableWindow(FALSE);
/*
	// y-offset, for tracking
	rect.left   = 10;
	rect.right  = rect.left + DLG_ITEM_WIDTH*2.5;
	rect.top    = rect.bottom + 5;
	rect.bottom = rect.top + DLG_ITEM_HEIGHT;
	m_lblTrackY.Create("Tracking X: 0.000", WS_CHILD|WS_VISIBLE, rect, this, ID_LBL_YTRACK);
	rect.left   = rect.right + 5;
	rect.right  = rect.left + DLG_ITEM_WIDTH*2+50;
	m_scrTrackY.Create(WS_CHILD|SBS_HORZ|WS_VISIBLE|WS_TABSTOP, rect, this, ID_SCR_YTRACK);
	m_scrTrackY.SetScrollRange(-8000, 8000, TRUE);
	m_scrTrackY.SetScrollPos(m_iTrackY);
	m_scrTrackY.EnableWindow(FALSE);
*/
	// y-offset, for tracking
	rect.left   = 10;
	rect.right  = rect.left + DLG_ITEM_WIDTH*2.5;
	rect.top    = rect.bottom + 5;
	rect.bottom = rect.top + DLG_ITEM_HEIGHT;
	m_lblTrackY.Create("Tracking Y: 0.000", WS_CHILD|WS_VISIBLE, rect, this, ID_LBL_YTRACK);
	rect.left   = rect.right + 5;
	rect.right  = rect.left + DLG_ITEM_WIDTH*2+50;
	m_scrTrackY.Create(WS_CHILD|SBS_HORZ|WS_VISIBLE|WS_TABSTOP, rect, this, ID_LBL_YTRACK);
	m_scrTrackY.SetScrollRange(-8000, 8000, TRUE);
	m_scrTrackY.SetScrollPos(m_iTrackY);
	m_scrTrackY.EnableWindow(FALSE);

/*
	// steering
	rect.left   = 10;
	rect.right  = rect.left + DLG_ITEM_WIDTH*2.5;
	rect.top    = rect.bottom + 5;
	rect.bottom = rect.top + DLG_ITEM_HEIGHT;
	m_lblSteerY.Create("Steering: 0.00 deg", WS_CHILD|WS_VISIBLE, rect, this, ID_STEER_LABEL);
	rect.left   = rect.right + 5;
	rect.right  = rect.left + DLG_ITEM_WIDTH*2+50;
	m_scrSteerY.Create(WS_CHILD|SBS_HORZ|WS_VISIBLE|WS_TABSTOP, rect, this, ID_STEER_SCROLL);
	m_scrSteerY.SetScrollRange(-255, 255, TRUE);
	m_scrSteerY.SetScrollPos(m_iSteerY);
*/

	// interleaving scanning lines
	rect.left   = 10;
	rect.right  = rect.left + DLG_ITEM_WIDTH*2.5;
	rect.top    = rect.bottom + 5;
	rect.bottom = rect.top + DLG_ITEM_HEIGHT;
	m_chkInterleave.Create("Interleave scanning lines", WS_CHILD|BS_CHECKBOX|WS_VISIBLE, rect, this, ID_INTERLEAVE_LINE);
	m_chkInterleave.EnableWindow(FALSE);
	m_chkInterleave.SetCheck(m_bInterleaveLines);

	// enable mirror tracking
	rect.left   = rect.right + 5;
	rect.right  = rect.left + DLG_ITEM_WIDTH*2+50;
	m_chkEnableTracking.Create("Disable Mirror Tracking", WS_CHILD|BS_CHECKBOX|WS_VISIBLE, rect, this, ID_ENABLE_TRACKING);
	m_chkEnableTracking.EnableWindow(TRUE);
	m_chkEnableTracking.SetCheck(!m_bEnableTracking);
	g_objVirtex5BMD.EnableMirrorTracking(g_hDevVirtex5, m_bEnableTracking);

	// calibration laser power from all three wavelengths
	// 488 nm
	rect.left   = 10;
	rect.right  = rect.left + DLG_ITEM_WIDTH*1.5;
	rect.top    = rect.bottom + 50;
	rect.bottom = rect.top + DLG_ITEM_HEIGHT;
	m_chkPower488.Create("Calibrate 488nm Laser", WS_CHILD|BS_CHECKBOX|WS_VISIBLE, rect, this, ID_POWER488_CHK);
	m_chkPower488.EnableWindow(FALSE);
	m_chkPower488.SetCheck(m_bPower488);

	// 568 nm
	rect.left   = rect.right + 15;
	rect.right  = rect.left + DLG_ITEM_WIDTH*1.5;
	m_chkPower568.Create("Calibrate 568nm Laser", WS_CHILD|BS_CHECKBOX|WS_VISIBLE, rect, this, ID_POWER568_CHK);
	m_chkPower568.EnableWindow(FALSE);
	m_chkPower568.SetCheck(m_bPower568);

	// 796 nm
	rect.left   = rect.right + 15;
	rect.right  = rect.left + DLG_ITEM_WIDTH*1.5;
	m_chkPower796.Create("Calibrate 796nm Laser", WS_CHILD|BS_CHECKBOX|WS_VISIBLE, rect, this, ID_POWER796_CHK);
	m_chkPower796.EnableWindow(FALSE);
	m_chkPower796.SetCheck(m_bPower796);

	// 488nm power adjustment
	rect.left   = 10;
	rect.right  = rect.left + DLG_ITEM_WIDTH*2.5;
	rect.top    = rect.bottom + 5;
	rect.bottom = rect.top + DLG_ITEM_HEIGHT;
	m_lblPower488.Create("488nm Laser voltage: 1.50 V", WS_CHILD|WS_VISIBLE, rect, this, ID_POWER488_LBL);
	rect.left   = rect.right + 5;
	rect.right  = rect.left + DLG_ITEM_WIDTH*2+50;
	m_scrPower488.Create(WS_CHILD|SBS_HORZ|WS_VISIBLE|WS_TABSTOP, rect, this, ID_POWER488_SCR);
	m_scrPower488.SetScrollRange(0, 150, TRUE);
	m_scrPower488.SetScrollPos(150);
	m_scrPower488.EnableWindow(FALSE);

	// 568nm power adjustment
	rect.left   = 10;
	rect.right  = rect.left + DLG_ITEM_WIDTH*2.5;
	rect.top    = rect.bottom + 5;
	rect.bottom = rect.top + DLG_ITEM_HEIGHT;
	m_lblPower568.Create("568nm Laser voltage: 1.000 V", WS_CHILD|WS_VISIBLE, rect, this, ID_POWER568_LBL);
	rect.left   = rect.right + 5;
	rect.right  = rect.left + DLG_ITEM_WIDTH*2+50;
	m_scrPower568.Create(WS_CHILD|SBS_HORZ|WS_VISIBLE|WS_TABSTOP, rect, this, ID_POWER568_SCR);
	m_scrPower568.SetScrollRange(0, 1000, TRUE);
	m_scrPower568.SetScrollPos(1000);
	m_scrPower568.EnableWindow(FALSE);

	// 796nm power adjustment
	rect.left   = 10;
	rect.right  = rect.left + DLG_ITEM_WIDTH*2.5;
	rect.top    = rect.bottom + 5;
	rect.bottom = rect.top + DLG_ITEM_HEIGHT;
	m_lblPower796.Create("796nm Laser voltage: 1.50 V", WS_CHILD|WS_VISIBLE, rect, this, ID_POWER796_LBL);
	rect.left   = rect.right + 5;
	rect.right  = rect.left + DLG_ITEM_WIDTH*2+50;
	m_scrPower796.Create(WS_CHILD|SBS_HORZ|WS_VISIBLE|WS_TABSTOP, rect, this, ID_POWER796_SCR);
	m_scrPower796.SetScrollRange(0, 150, TRUE);
	m_scrPower796.SetScrollPos(150);
	m_scrPower796.EnableWindow(FALSE);

	/*
	// set symmetric ramp
	rect.left   = 10;
	rect.right  = rect.left + DLG_ITEM_WIDTH*2.5;
	rect.top    = rect.bottom + 5;
	rect.bottom = rect.top + DLG_ITEM_HEIGHT;
	m_chkSymmetricRamp.Create("Set Symmetric Ramp", WS_CHILD|BS_CHECKBOX|WS_VISIBLE, rect, this, ID_SYMMETRIC_RAMP);
	m_chkSymmetricRamp.EnableWindow(FALSE);
	m_chkSymmetricRamp.SetCheck(m_bSymmetricRamp);

	// set reverse of ramp to zero
	rect.left   = rect.right + 5;
	rect.right  = rect.left + DLG_ITEM_WIDTH*2+50;
	m_chkZeroInverse.Create("Disable slow scanning", WS_CHILD|BS_CHECKBOX|WS_VISIBLE, rect, this, ID_ZERO_INVERSE);
	m_chkZeroInverse.EnableWindow(FALSE);
	m_chkZeroInverse.SetCheck(m_bZeroBackward);

	rect.left   = 10;
	rect.right  = rect.left + DLG_ITEM_WIDTH*5+10;
	rect.top    = rect.bottom + 5;
	rect.bottom = rect.top + DLG_ITEM_HEIGHT;
	m_btnCalibrateH.Create("Scale FOV of the fast scanner to FOV the slow scanner", WS_CHILD|BS_DEFPUSHBUTTON|WS_VISIBLE, rect, this, ID_CALIBRATE_RSFOV);
	m_btnCalibrateH.EnableWindow(FALSE);

*/

	m_lblResonant.ShowWindow(SW_HIDE);
	m_lblGalvo.ShowWindow(SW_HIDE);
	m_lblGalvoForward.ShowWindow(SW_HIDE);
	m_lblGalvoBackward.ShowWindow(SW_HIDE);
	m_lblRampOffset.ShowWindow(SW_HIDE);
	m_scrResonant.ShowWindow(SW_HIDE);
	m_scrGalvo.ShowWindow(SW_HIDE);
	m_scrGalvoForward.ShowWindow(SW_HIDE);
	m_scrGalvoBackward.ShowWindow(SW_HIDE);
	m_scrRampOffset.ShowWindow(SW_HIDE);
	m_chkInterleave.ShowWindow(SW_HIDE);
//	m_chkZeroInverse.ShowWindow(SW_HIDE);
//	m_chkSymmetricRamp.ShowWindow(SW_HIDE);
	m_chkEnableTracking.ShowWindow(SW_HIDE);
	m_lblTrackY.ShowWindow(SW_HIDE);
	m_scrTrackY.ShowWindow(SW_HIDE);

	m_chkPower488.ShowWindow(SW_HIDE);
	m_chkPower568.ShowWindow(SW_HIDE);
	m_chkPower796.ShowWindow(SW_HIDE);
	m_lblPower488.ShowWindow(SW_HIDE);
	m_lblPower568.ShowWindow(SW_HIDE);
	m_lblPower796.ShowWindow(SW_HIDE);
	m_scrPower488.ShowWindow(SW_HIDE);
	m_scrPower568.ShowWindow(SW_HIDE);
	m_scrPower796.ShowWindow(SW_HIDE);
//	m_lblTrackY.ShowWindow(SW_HIDE);
//	m_scrTrackY.ShowWindow(SW_HIDE);
//	m_lblSteerY.ShowWindow(SW_HIDE);
//	m_scrSteerY.ShowWindow(SW_HIDE);
//	m_btnCalibrateH.ShowWindow(SW_HIDE);
}

void CML506View::OnSelchangeTabMain(NMHDR* pNMHDR, LRESULT* pResult)
{
	int   idx = m_tabPCIe.GetCurSel();

	m_tabIndex = idx;

	ShowTabItems(idx);

	*pResult = 0;
}

void CML506View::ShowTabItems(int index)
{
	int  i;

	m_scrImgSizeX.ShowWindow(SW_HIDE);
	m_scrImgSizeY.ShowWindow(SW_HIDE);
	m_lblImgSizeX.ShowWindow(SW_HIDE);
	m_lblImgSizeY.ShowWindow(SW_HIDE);
	m_lblBrightnessR.ShowWindow(SW_HIDE);
	m_lblContrastR.ShowWindow(SW_HIDE);
	m_lblBrightnessG.ShowWindow(SW_HIDE);
	m_lblContrastG.ShowWindow(SW_HIDE);
	m_lblBrightnessB.ShowWindow(SW_HIDE);
	m_lblContrastB.ShowWindow(SW_HIDE);
	m_lblOffsetX.ShowWindow(SW_HIDE);
	m_lblOffsetY.ShowWindow(SW_HIDE);
	m_lblHsyncI.ShowWindow(SW_HIDE);
	m_lblHsyncO.ShowWindow(SW_HIDE);
	m_lblVsyncI.ShowWindow(SW_HIDE);
	m_lblVsyncO.ShowWindow(SW_HIDE);
//	m_lblPixelClock.ShowWindow(SW_HIDE);
	m_edtBrightnessR.ShowWindow(SW_HIDE);
	m_edtContrastR.ShowWindow(SW_HIDE);
	m_edtBrightnessG.ShowWindow(SW_HIDE);
	m_edtContrastG.ShowWindow(SW_HIDE);
	m_edtBrightnessB.ShowWindow(SW_HIDE);
	m_edtContrastB.ShowWindow(SW_HIDE);
	m_scrOffsetX.ShowWindow(SW_HIDE);
	m_scrOffsetY.ShowWindow(SW_HIDE);
	m_cboHsyncI.ShowWindow(SW_HIDE);
	m_cboHsyncO.ShowWindow(SW_HIDE);
	m_cboVsyncI.ShowWindow(SW_HIDE);
	m_cboVsyncO.ShowWindow(SW_HIDE);
//	m_edtPixelClock.ShowWindow(SW_HIDE);
//	m_chkExternalClock.ShowWindow(SW_HIDE);
	m_btnLoadStimulus.ShowWindow(SW_HIDE);
//	m_btnDesinusoidCal.ShowWindow(SW_HIDE);
	m_btnSaveVideo.ShowWindow(SW_HIDE);
	m_rdoLenthInFrames.ShowWindow(SW_HIDE);
	m_rdoLenthInSeconds.ShowWindow(SW_HIDE);
	m_radLines.ShowWindow(SW_HIDE);
	m_radVsync.ShowWindow(SW_HIDE);
	m_lblVstart.ShowWindow(SW_HIDE);
	m_lblVideoName.ShowWindow(SW_HIDE);
	m_lblVideoPrefix.ShowWindow(SW_HIDE);
	m_lblVideoLength.ShowWindow(SW_HIDE);
	m_edtVideoNameR.ShowWindow(SW_HIDE);
	m_btnVideoFolderR.ShowWindow(SW_HIDE);
	m_edtVideoPrefixR.ShowWindow(SW_HIDE);
	m_edtVideoLengthR.ShowWindow(SW_HIDE);
//	m_edtVideoFlagsR1.ShowWindow(SW_HIDE);
//	m_edtVideoFlagsR2.ShowWindow(SW_HIDE);
//	m_edtVideoNameG.ShowWindow(SW_HIDE);
//	m_btnVideoFolderG.ShowWindow(SW_HIDE);
	m_edtVideoPrefixG.ShowWindow(SW_HIDE);
//	m_edtVideoLengthG.ShowWindow(SW_HIDE);
//	m_edtVideoFlagsG1.ShowWindow(SW_HIDE);
//	m_edtVideoFlagsG2.ShowWindow(SW_HIDE);
//	m_edtVideoNameB.ShowWindow(SW_HIDE);
//	m_btnVideoFolderB.ShowWindow(SW_HIDE);
	m_edtVideoPrefixB.ShowWindow(SW_HIDE);
//	m_edtVideoLengthB.ShowWindow(SW_HIDE);
//	m_edtVideoFlagsB1.ShowWindow(SW_HIDE);
//	m_edtVideoFlagsB2.ShowWindow(SW_HIDE);
	m_chkChannelR.ShowWindow(SW_HIDE);
	m_chkChannelG.ShowWindow(SW_HIDE);
	m_chkChannelB.ShowWindow(SW_HIDE);
	m_chkChannelSW.ShowWindow(SW_HIDE);
	m_chkDiscardBlinks.ShowWindow(SW_HIDE);
	m_chkBidirection.ShowWindow(SW_HIDE);
//	m_lblBlankPixels.ShowWindow(SW_HIDE);
//	m_scrBlankPixels.ShowWindow(SW_HIDE);

	m_TextCurrentReg.ShowWindow(SW_HIDE);
	m_EditCurrentRegAddr.ShowWindow(SW_HIDE);
	m_EditCurrentRegVal.ShowWindow(SW_HIDE);
	m_WriteOne.ShowWindow(SW_HIDE);
	m_ReadOne.ShowWindow(SW_HIDE);
	m_ReadAll.ShowWindow(SW_HIDE);
	m_TextCurrentRegAddr.ShowWindow(SW_HIDE);
	m_TextCurrentRegVal.ShowWindow(SW_HIDE);

	m_lblResonant.ShowWindow(SW_HIDE);
	m_lblGalvo.ShowWindow(SW_HIDE);
	m_lblGalvoForward.ShowWindow(SW_HIDE);
	m_lblGalvoBackward.ShowWindow(SW_HIDE);
	m_lblRampOffset.ShowWindow(SW_HIDE);
	m_scrResonant.ShowWindow(SW_HIDE);
	m_scrGalvo.ShowWindow(SW_HIDE);
	m_scrGalvoForward.ShowWindow(SW_HIDE);
	m_scrGalvoBackward.ShowWindow(SW_HIDE);
	m_scrRampOffset.ShowWindow(SW_HIDE);
	m_chkInterleave.ShowWindow(SW_HIDE);
//	m_chkZeroInverse.ShowWindow(SW_HIDE);
//	m_chkSymmetricRamp.ShowWindow(SW_HIDE);
	m_chkEnableTracking.ShowWindow(SW_HIDE);
	m_lblTrackY.ShowWindow(SW_HIDE);
	m_scrTrackY.ShowWindow(SW_HIDE);

	m_chkPower488.ShowWindow(SW_HIDE);
	m_chkPower568.ShowWindow(SW_HIDE);
	m_chkPower796.ShowWindow(SW_HIDE);
	m_lblPower488.ShowWindow(SW_HIDE);
	m_lblPower568.ShowWindow(SW_HIDE);
	m_lblPower796.ShowWindow(SW_HIDE);
	m_scrPower488.ShowWindow(SW_HIDE);
	m_scrPower568.ShowWindow(SW_HIDE);
	m_scrPower796.ShowWindow(SW_HIDE);
//	m_lblTrackY.ShowWindow(SW_HIDE);
//	m_scrTrackY.ShowWindow(SW_HIDE);
//	m_lblSteerY.ShowWindow(SW_HIDE);
//	m_scrSteerY.ShowWindow(SW_HIDE);
//	m_btnCalibrateH.ShowWindow(SW_HIDE);


	for (i = 0; i < 47; i ++) {
		m_adcText[i].ShowWindow(SW_HIDE);
		m_adcEdit[i].ShowWindow(SW_HIDE);
	}
	for (i = 0; i < 18; i ++) {
		m_dacText[i].ShowWindow(SW_HIDE);
		m_dacEdit[i].ShowWindow(SW_HIDE);
	}
/*	for (i = 0; i < VIRTEX5_CFG_REGS_NUM+2; i ++) {
		m_cfgData[i].ShowWindow(SW_HIDE);
	}*/
	for (i = 0; i < VIRTEX5_REGS_NUM; i ++) {
		m_regText[i].ShowWindow(SW_HIDE);
		m_regEdit[i].ShowWindow(SW_HIDE);
	}

	switch (index) {
	case 0:
		m_scrImgSizeX.ShowWindow(SW_SHOW);
		m_scrImgSizeY.ShowWindow(SW_SHOW);
		m_lblImgSizeX.ShowWindow(SW_SHOW);
		m_lblImgSizeY.ShowWindow(SW_SHOW);
		m_lblBrightnessR.ShowWindow(SW_SHOW);
		m_lblContrastR.ShowWindow(SW_SHOW);
		m_lblBrightnessG.ShowWindow(SW_SHOW);
		m_lblContrastG.ShowWindow(SW_SHOW);
		m_lblBrightnessB.ShowWindow(SW_SHOW);
		m_lblContrastB.ShowWindow(SW_SHOW);
		m_lblOffsetX.ShowWindow(SW_SHOW);
		m_lblOffsetY.ShowWindow(SW_SHOW);
		m_lblHsyncI.ShowWindow(SW_SHOW);
//		m_lblHsyncO.ShowWindow(SW_SHOW);
		m_lblVsyncI.ShowWindow(SW_SHOW);
//		m_lblVsyncO.ShowWindow(SW_SHOW);
		m_edtBrightnessR.ShowWindow(SW_SHOW);
		m_edtContrastR.ShowWindow(SW_SHOW);
		m_edtBrightnessG.ShowWindow(SW_SHOW);
		m_edtContrastG.ShowWindow(SW_SHOW);
		m_edtBrightnessB.ShowWindow(SW_SHOW);
		m_edtContrastB.ShowWindow(SW_SHOW);
		m_scrOffsetX.ShowWindow(SW_SHOW);
		m_scrOffsetY.ShowWindow(SW_SHOW);
		m_cboHsyncI.ShowWindow(SW_SHOW);
//		m_cboHsyncO.ShowWindow(SW_SHOW);
		m_cboVsyncI.ShowWindow(SW_SHOW);
//		m_cboVsyncO.ShowWindow(SW_SHOW);
	//	m_lblPixelClock.ShowWindow(SW_SHOW);
	//	m_edtPixelClock.ShowWindow(SW_SHOW);
//		m_chkExternalClock.ShowWindow(SW_SHOW);
		m_btnLoadStimulus.ShowWindow(SW_SHOW);
//		m_btnDesinusoidCal.ShowWindow(SW_SHOW);
		m_btnSaveVideo.ShowWindow(SW_SHOW);
		m_rdoLenthInFrames.ShowWindow(SW_SHOW);
		m_rdoLenthInSeconds.ShowWindow(SW_SHOW);
		m_radLines.ShowWindow(SW_SHOW);
		m_radVsync.ShowWindow(SW_SHOW);
		m_lblVstart.ShowWindow(SW_SHOW);
		m_lblVideoName.ShowWindow(SW_SHOW);
		m_lblVideoLength.ShowWindow(SW_SHOW);
		m_lblVideoPrefix.ShowWindow(SW_SHOW);
		m_edtVideoNameR.ShowWindow(SW_SHOW);
		m_btnVideoFolderR.ShowWindow(SW_SHOW);
		m_edtVideoPrefixR.ShowWindow(SW_SHOW);
		m_edtVideoLengthR.ShowWindow(SW_SHOW);
	//	m_edtVideoFlagsR1.ShowWindow(SW_SHOW);
	//	m_edtVideoFlagsR2.ShowWindow(SW_SHOW);
	//	m_edtVideoNameG.ShowWindow(SW_SHOW);
	//	m_btnVideoFolderG.ShowWindow(SW_SHOW);
		m_edtVideoPrefixG.ShowWindow(SW_SHOW);
	//	m_edtVideoLengthG.ShowWindow(SW_SHOW);
	//	m_edtVideoFlagsG1.ShowWindow(SW_SHOW);
	//	m_edtVideoFlagsG2.ShowWindow(SW_SHOW);
	//	m_edtVideoNameB.ShowWindow(SW_SHOW);
	//	m_btnVideoFolderB.ShowWindow(SW_SHOW);
		m_edtVideoPrefixB.ShowWindow(SW_SHOW);
	//	m_edtVideoLengthB.ShowWindow(SW_SHOW);
	//	m_edtVideoFlagsB1.ShowWindow(SW_SHOW);
	//	m_edtVideoFlagsB2.ShowWindow(SW_SHOW);
		m_chkChannelR.ShowWindow(SW_SHOW);
		m_chkChannelG.ShowWindow(SW_SHOW);
		m_chkChannelB.ShowWindow(SW_SHOW);
		m_chkChannelSW.ShowWindow(m_iVideoNum==3?SW_SHOW:SW_HIDE);
		m_chkDiscardBlinks.ShowWindow(SW_SHOW);
		m_chkBidirection.ShowWindow(SW_SHOW);
	//	m_lblBlankPixels.ShowWindow(SW_SHOW);
	//	m_scrBlankPixels.ShowWindow(SW_SHOW);
		break;
	case 2:
		for (i = 0; i < VIRTEX5_REGS_NUM; i ++) {
			m_regText[i].ShowWindow(SW_SHOW);
			m_regEdit[i].ShowWindow(SW_SHOW);
		}
		m_TextCurrentReg.ShowWindow(SW_SHOW);
		m_EditCurrentRegAddr.ShowWindow(SW_SHOW);
		m_EditCurrentRegVal.ShowWindow(SW_SHOW);
		m_WriteOne.ShowWindow(SW_SHOW);
		m_ReadOne.ShowWindow(SW_SHOW);
		m_ReadAll.ShowWindow(SW_SHOW);
		m_TextCurrentRegAddr.ShowWindow(SW_SHOW);
		m_TextCurrentRegVal.ShowWindow(SW_SHOW);
		m_TextCurrentRegVal.SetWindowText("Value, Hex");
		break;
	case 3:
		for (i = 0; i < 47; i ++) {
			m_adcText[i].ShowWindow(SW_SHOW);
			m_adcEdit[i].ShowWindow(SW_SHOW);
		}
		m_TextCurrentReg.ShowWindow(SW_SHOW);
		m_EditCurrentRegAddr.ShowWindow(SW_SHOW);
		m_EditCurrentRegVal.ShowWindow(SW_SHOW);
		m_WriteOne.ShowWindow(SW_SHOW);
		m_ReadOne.ShowWindow(SW_SHOW);
		m_ReadAll.ShowWindow(SW_SHOW);
		m_TextCurrentRegAddr.ShowWindow(SW_SHOW);
		m_TextCurrentRegVal.ShowWindow(SW_SHOW);
		m_TextCurrentRegVal.SetWindowText("Value, Bin");
		break;
	case 4:
		for (i = 0; i < 18; i ++) {
			m_dacText[i].ShowWindow(SW_SHOW);
			m_dacEdit[i].ShowWindow(SW_SHOW);
		}
		m_TextCurrentReg.ShowWindow(SW_SHOW);
		m_EditCurrentRegAddr.ShowWindow(SW_SHOW);
		m_EditCurrentRegVal.ShowWindow(SW_SHOW);
		m_WriteOne.ShowWindow(SW_SHOW);
		m_ReadOne.ShowWindow(SW_SHOW);
		m_ReadAll.ShowWindow(SW_SHOW);
		m_TextCurrentRegAddr.ShowWindow(SW_SHOW);
		m_TextCurrentRegVal.ShowWindow(SW_SHOW);
		m_TextCurrentRegVal.SetWindowText("Value, Bin");
		break;
	case 1:
/*		for (i = 0; i < VIRTEX5_CFG_REGS_NUM+2; i ++) {
			m_cfgData[i].ShowWindow(SW_SHOW);
		}
		m_ReadAll.ShowWindow(SW_SHOW);*/
		m_lblResonant.ShowWindow(SW_SHOW);
		m_lblGalvo.ShowWindow(SW_SHOW);
		m_lblGalvoForward.ShowWindow(SW_SHOW);
		m_lblGalvoBackward.ShowWindow(SW_SHOW);
		m_lblRampOffset.ShowWindow(SW_SHOW);
		m_scrResonant.ShowWindow(SW_SHOW);
		m_scrGalvo.ShowWindow(SW_SHOW);
		m_scrGalvoForward.ShowWindow(SW_SHOW);
		m_scrGalvoBackward.ShowWindow(SW_SHOW);
		m_scrRampOffset.ShowWindow(SW_SHOW);
		m_chkInterleave.ShowWindow(SW_SHOW);
		//m_chkZeroInverse.ShowWindow(SW_SHOW);
		//m_chkSymmetricRamp.ShowWindow(SW_SHOW);
		m_chkEnableTracking.ShowWindow(SW_SHOW);
		m_lblTrackY.ShowWindow(SW_SHOW);
		m_scrTrackY.ShowWindow(SW_SHOW);

		m_chkPower488.ShowWindow(SW_SHOW);
		m_chkPower568.ShowWindow(SW_SHOW);
		m_chkPower796.ShowWindow(SW_SHOW);
		m_lblPower488.ShowWindow(SW_SHOW);
		m_lblPower568.ShowWindow(SW_SHOW);
		m_lblPower796.ShowWindow(SW_SHOW);
		m_scrPower488.ShowWindow(SW_SHOW);
		m_scrPower568.ShowWindow(SW_SHOW);
		m_scrPower796.ShowWindow(SW_SHOW);
//		m_lblTrackY.ShowWindow(SW_SHOW);
//		m_scrTrackY.ShowWindow(SW_SHOW);
//		m_lblSteerY.ShowWindow(SW_SHOW);
//		m_scrSteerY.ShowWindow(SW_SHOW);
//		m_btnCalibrateH.ShowWindow(SW_SHOW);
		break;
	default:
		break;
	}
}

void CML506View::CreateRTregisters()
{
	RECT        rect;
	CString     msg;
	int         i;
	BOOL        ret;
	UINT64      regData;
	UINT32      reg32, regTemp;

	// assign register with information of start line address and end line address;
//	regTemp  = g_VideoInfo.line_start_addr;
//	reg32    = regTemp << 16;
//	reg32    = reg32 | g_VideoInfo.line_end_addr;
//	g_objVirtex5BMD.VIRTEX5_WriteReg32(g_hDevVirtex5, VIRTEX5_LINEINFO_OFFSET, reg32);

	// assign register with information of image height and end line address;
	regTemp  = g_VideoInfo.line_spacing;
	reg32    = regTemp << 24;
	regTemp  = g_VideoInfo.img_width << 12;
	reg32    = reg32 | regTemp;
	reg32    = reg32 | g_VideoInfo.img_height;
	g_objVirtex5BMD.VIRTEX5_WriteReg32(g_hDevVirtex5, VIRTEX5_IMAGESIZE_OFFSET, reg32);

	// assign register with image offsets and end line ID
//	regTemp  = g_VideoInfo.offset_pixel;
//	reg32    = regTemp << 24;
//	regTemp  = g_VideoInfo.offset_line;
//	regTemp  = regTemp << 16;
	regTemp  = g_VideoInfo.offset_line;
	reg32    = regTemp << 24;
	regTemp  = g_VideoInfo.offset_pixel;
	regTemp  = regTemp << 14;
	reg32    = reg32 | regTemp;
	//reg32    = reg32 | BIT14;
	reg32    = reg32 | BIT13 | BIT12;		// choose red channel
	reg32    = reg32 - BIT12;
	reg32    = reg32 | g_VideoInfo.end_line_ID;
	g_objVirtex5BMD.VIRTEX5_WriteReg32(g_hDevVirtex5, VIRTEX5_IMAGEOS_OFFSET, reg32);

	// assign register with TLP counts
	reg32    = g_VideoInfo.tlp_counts/2;
	g_objVirtex5BMD.VIRTEX5_WriteReg32(g_hDevVirtex5, VIRTEX5_WDMATLPC_OFFSET, reg32);


	rect.top  = 40 - (DLG_ITEM_HEIGHT + DLG_ITEM_MARGIN_Y);
	for (i = 0; i < VIRTEX5_REGS_NUM; i ++) {
		rect.left   = 10;
		rect.right  = rect.left + DLG_ITEM_WIDTH;
		rect.top    = rect.top + DLG_ITEM_HEIGHT + DLG_ITEM_MARGIN_Y;
		rect.bottom = rect.top + DLG_ITEM_HEIGHT;
		// draw edit boxes
		if (i >= 10 && i <= 17) {
			m_regEdit[i].Create(WS_CHILD | ES_LEFT | WS_BORDER | WS_TABSTOP | ES_READONLY, rect, this, m_regEditID[i]);
			m_regEdit[i].SetBackgroundColor(FALSE, RGB(255, 255, 191));
		} else {
			m_regEdit[i].Create(WS_CHILD | ES_LEFT | WS_BORDER | WS_TABSTOP | ES_READONLY, rect, this, m_regEditID[i]);
		}
		m_regEdit[i].LimitText(8);

		if (g_hDevVirtex5 != NULL) {
			ret = g_objVirtex5BMD.ReadWriteReg(g_hDevVirtex5, gVIRTEX5_Regs, i+1, VIRTEX5_REGS_NUM, WDC_READ, FALSE, &regData, NULL);
			if (ret == TRUE) {
				switch (gVIRTEX5_Regs[i].dwSize)
				{
				case WDC_SIZE_8:
					msg.Format("%02X", regData);
					break;
				case WDC_SIZE_16:
					msg.Format("%04X", regData);
					break;
				case WDC_SIZE_32:
					msg.Format("%08X", regData);
					break;
				case WDC_SIZE_64:
					msg.Format("%16X", regData);
					break;
				default:
					msg = _T("error !!");
				}
			} else {
				msg = _T("failed !");
			}
		}
		m_regEdit[i].SetWindowText(msg);


		rect.left   = 10 + DLG_ITEM_WIDTH + DLG_ITEM_MARGIN_X;
		rect.right  = 395;
		m_regText[i].Create(m_regLabel[i], WS_CHILD, rect, this, m_regTextID[i]);

	}
}


void CML506View::ReadOneRegister()
{
	CString msg;

	if (VerifyAddr(FALSE) != 0) return;

	m_EditCurrentRegAddr.SetBackgroundColor(FALSE, RGB(0, 255, 0));
	//msg.Format("Register Address = [0x%02X]", m_regAddr);
	//MessageBox(msg, "Read Register", MB_ICONINFORMATION);

	switch (m_tabIndex) {
	case 2:
		ReadWritePCIeRegister(TRUE);
		break;
	case 3:
		ReadWriteI2CRegister(TRUE, 0x4c000000);
		break;
	case 4:
		ReadWriteI2CRegister(TRUE, 0x76000000);
		break;
	default:
		MessageBox("Unknown registers", "Read Register", MB_ICONWARNING);
		break;
	}
}

void CML506View::WriteOneRegister()
{
	int     ret;
	CString msg;

	if (VerifyAddr(TRUE) != 0) return;
	m_EditCurrentRegAddr.SetBackgroundColor(FALSE, RGB(0, 255, 0));

	ret = VerifyVal();
	if (ret == FALSE) {
		m_EditCurrentRegVal.SetBackgroundColor(FALSE, RGB(255, 0, 0));
		MessageBox("Invalid Register Value.", "Write Register", MB_ICONWARNING);
		return;
	}

	m_EditCurrentRegVal.SetBackgroundColor(FALSE, RGB(0, 255, 0));

	//MessageBox(msg, "Write Register", MB_ICONINFORMATION);
	switch (m_tabIndex) {
	case 2:
		ReadWritePCIeRegister(FALSE);
		break;
	case 3:
		ReadWriteI2CRegister(FALSE, 0x4c000000);
		break;
	case 4:
		ReadWriteI2CRegister(FALSE, 0x76000000);
		break;
	default:
		MessageBox("Unknown registers", "Write Register", MB_ICONWARNING);
		break;
	}

	g_bDCFsaved = FALSE;			// run-time registers have been updated

	m_strTitleName = m_strAppName + m_strFileName + " *";
	GetParentOwner()->SetWindowText(m_strTitleName);
}

void CML506View::ReadAllRegister()
{
	switch (m_tabIndex) {
	case 0:
//		ReadPCIeRegisters(TRUE);
//		MessageBox("PCIe Configuration Space Registers", "Read All Registers");
		break;
	case 2:
		ReadPCIeRegisters(FALSE);
//		MessageBox("PCIe Run-Time Registers", "Read All Registers");
		break;
	case 3:
		ReadI2CRegisters(TRUE);
		break;
	case 4:
		ReadI2CRegisters(FALSE);
		break;
	default:
		MessageBox("Unknown Registers", "Read All Registers");
		break;
	}
}

void CML506View::ReadPCIe()
{

}


int CML506View::VerifyAddr(BOOL bWrite)
{
	char     *text;
	int       val;
	CString   msg;

	text = new char [3];
	GetDlgItemText(ID_CURRENT_REG_ADDR, text, 3);

	switch (text[0]) {
	case 0x30:	val = 0;	break;			// digit 0;
	case 0x31:	val = 16;	break;			// digit 1;
	case 0x32:	val = 32;	break;			// digit 2;
	case 0x33:	val = 48;	break;			// digit 3;
	case 0x34:	val = 64;	break;			// digit 4;
	case 0x35:	val = 80;	break;			// digit 5;
	case 0x36:	val = 96;	break;			// digit 6;
	case 0x37:	val = 112;	break;			// digit 7;
	case 0x38:	val = 128;	break;
	case 0x39:	val = 144;	break;
	case 0x41:	val = 160;	break;
	case 0x42:	val = 176;	break;
	case 0x43:	val = 192;	break;
	case 0x44:	val = 208;	break;
	case 0x45:	val = 224;	break;
	case 0x46:	val = 240;	break;
	case 0x61:	val = 160;	break;
	case 0x62:	val = 176;	break;
	case 0x63:	val = 192;	break;
	case 0x64:	val = 208;	break;
	case 0x65:	val = 224;	break;
	case 0x66:	val = 240;	break;
	default:
		m_EditCurrentRegAddr.SetBackgroundColor(FALSE, RGB(255, 0, 0));
		MessageBox("Invalid Register Address", "Access Registers", MB_ICONWARNING);
		return 11;
	}

	switch (text[1]) {
		case 0x30:	break;
		case 0x31:	val = val + 1;	break;
		case 0x32:	val = val + 2;	break;
		case 0x33:	val = val + 3;	break;
		case 0x34:	val = val + 4;	break;
		case 0x35:	val = val + 5;	break;
		case 0x36:	val = val + 6;	break;
		case 0x37:	val = val + 7;	break;
		case 0x38:	val = val + 8;	break;
		case 0x39:	val = val + 9;	break;
		case 0x41:	val = val + 10;	break;
		case 0x42:	val = val + 11;	break;
		case 0x43:	val = val + 12;	break;
		case 0x44:	val = val + 13;	break;
		case 0x45:	val = val + 14;	break;
		case 0x46:	val = val + 15;	break;
		case 0x61:	val = val + 10;	break;
		case 0x62:	val = val + 11;	break;
		case 0x63:	val = val + 12;	break;
		case 0x64:	val = val + 13;	break;
		case 0x65:	val = val + 14;	break;
		case 0x66:	val = val + 15;	break;
		default:
			m_EditCurrentRegAddr.SetBackgroundColor(FALSE, RGB(255, 0, 0));
			MessageBox("Invalid Register Address", "Access Registers", MB_ICONWARNING);
			return 11;
	}

	delete [] text;

	m_regAddr = (BYTE)(val);

	switch (m_tabIndex) {
	case 2:
		if (m_regAddr >= 4*VIRTEX5_REGS_NUM || (m_regAddr%4 != 0)) {
			msg.Format("Invalid Register Address. Enter any data between 0x00-0x%02X with an integer of 4", 4*(VIRTEX5_REGS_NUM-1));
			m_EditCurrentRegAddr.SetBackgroundColor(FALSE, RGB(255, 0, 0));
			MessageBox(msg, "Access Run-Time Registers", MB_ICONWARNING);
			return 12;
		}
		if (bWrite == TRUE && m_regAddr >= 0x28 && m_regAddr <= 0x44) {
			m_EditCurrentRegAddr.SetBackgroundColor(FALSE, RGB(255, 0, 0));
			msg.Format("0x%02X is a read-only register", m_regAddr);
			MessageBox(msg, "Access Run-Time Registers", MB_ICONWARNING);
			return 2;
		}
		break;
	case 3:
		if (m_regAddr > 0x2E) {
			msg.Format("Invalid Register Address. Enter any data between 0x00-0x2E.");
			m_EditCurrentRegAddr.SetBackgroundColor(FALSE, RGB(255, 0, 0));
			MessageBox(msg, "Access I2C Slave (ADC) Registers", MB_ICONWARNING);
			return 13;
		}
		if (bWrite == TRUE && (val==0x00||val==0x24||val==0x25||val==0x26||val==0x27||val==0x2a||val==0x2b)) {
			m_EditCurrentRegAddr.SetBackgroundColor(FALSE, RGB(255, 0, 0));
			msg.Format("0x%02X is a read-only register", m_regAddr);
			MessageBox(msg, "Write to I2C Slave (ADC) Registers", MB_ICONWARNING);
			return 3;
		}
		break;
	case 4:
/*		if (m_regAddr < 0 || m_regAddr > 0x2E) {		// ????????????
			msg.Format("Invalid Register Address. Enter any data between 0x00-0x2E.????");
			m_EditCurrentRegAddr.SetBackgroundColor(FALSE, RGB(255, 0, 0));
			MessageBox(msg, "Access I2C Slave (DAC) Registers", MB_ICONWARNING);
			return 14;
		}
		if (bWrite == TRUE && (val==0x00||val==0x06||val==0x07||val==0x08)) {
			m_EditCurrentRegAddr.SetBackgroundColor(FALSE, RGB(255, 0, 0));
			msg.Format("0x%02X is a read-only register", m_regAddr);
			MessageBox(msg, "Write to I2C Slave (DAC) Registers", MB_ICONWARNING);
			return 4;
		}*/
		break;
	default:
		break;
	}

	return 0;
}

BOOL CML506View::VerifyVal()
{
	char     *text;
	long      val = 0, i;

	text = new char [9];
	GetDlgItemText(ID_CURRENT_REG_VAL, text, 9);

	if (m_tabIndex == 3 || m_tabIndex == 4) {
		for (i = 0; i < 8; i ++) {
			if (text[i] == '0') {
			} else if (text[i] == '1') {
				val = val + (int)(pow(2.0, 7-i));
			} else {
				return FALSE;
			}
		}
		m_regVal = (BYTE)(val);
	} else {
		if (Hex2Dec(text, 8, &val) == FALSE) return FALSE;
		m_regVal = (UINT)(val);
	}

	delete [] text;

	return TRUE;
}

BOOL CML506View::Hex2Dec(char *text, int len, long *val)
{
	int   i;
	long  dec = 0, hex;

	for (i = 0; i < len; i ++) {
		hex = (long)(pow(16.0, 7-i));
		switch (text[i]) {
			case '0': break;
			case '1': dec = dec +  1*hex; break;
			case '2': dec = dec +  2*hex; break;
			case '3': dec = dec +  3*hex; break;
			case '4': dec = dec +  4*hex; break;
			case '5': dec = dec +  5*hex; break;
			case '6': dec = dec +  6*hex; break;
			case '7': dec = dec +  7*hex; break;
			case '8': dec = dec +  8*hex; break;
			case '9': dec = dec +  9*hex; break;
			case 'a': dec = dec + 10*hex; break;
			case 'b': dec = dec + 11*hex; break;
			case 'c': dec = dec + 12*hex; break;
			case 'd': dec = dec + 13*hex; break;
			case 'e': dec = dec + 14*hex; break;
			case 'f': dec = dec + 15*hex; break;
			case 'A': dec = dec + 10*hex; break;
			case 'B': dec = dec + 11*hex; break;
			case 'C': dec = dec + 12*hex; break;
			case 'D': dec = dec + 13*hex; break;
			case 'E': dec = dec + 14*hex; break;
			case 'F': dec = dec + 15*hex; break;
			default: return FALSE;
		}
	}

	*val = dec;
	return TRUE;
}

void CML506View::ReadPCIeRegisters(BOOL bCfgSpc)
{
	int      i, regNums, ret;
	UINT64   regData;
	DWORD    dwSize;
	char    *msg, *label;

	msg   = new char [64];
	label = new char [256];

	regNums = bCfgSpc ? VIRTEX5_CFG_REGS_NUM : VIRTEX5_REGS_NUM;

	for (i = 0; i < regNums; i ++) {
		if (g_hDevVirtex5 != NULL) {
			if (bCfgSpc == FALSE) {
				ret = g_objVirtex5BMD.ReadWriteReg(g_hDevVirtex5, gVIRTEX5_Regs, i+1, VIRTEX5_REGS_NUM, WDC_READ, bCfgSpc, &regData, NULL);
				dwSize = gVIRTEX5_Regs[i].dwSize;
			} else {
				ret = g_objVirtex5BMD.ReadWriteReg(g_hDevVirtex5, gVIRTEX5_CfgRegs, i+1, VIRTEX5_CFG_REGS_NUM, WDC_READ, bCfgSpc, &regData, NULL);
				dwSize = gVIRTEX5_CfgRegs[i].dwSize;
			}

			if (ret == TRUE) {
				switch (dwSize)
				{
				case WDC_SIZE_8:
					sprintf_s(msg, 64, bCfgSpc?"0x%02X":"%02X", regData);
					break;
				case WDC_SIZE_16:
					sprintf_s(msg, 64, bCfgSpc?"0x%04X":"%04X", regData);
					break;
				case WDC_SIZE_32:
					sprintf_s(msg, 64, bCfgSpc?"0x%08X":"%08X", regData);
					break;
				case WDC_SIZE_64:
					sprintf_s(msg, 64, bCfgSpc?"0x%16X":"%16X", regData);
					break;
				default:
					strcpy(msg, "error !!");
				}
			} else {
				strcpy(msg, "failed !");
			}
		}

		if (bCfgSpc == FALSE) {
			m_regEdit[i].SetWindowText(msg);
		} else {
			strcpy(label, m_cfgLabel[i+2]);
			strcat(label, msg);
//			m_cfgData[i+2].SetWindowText(label);
		}
	}

}

void CML506View::ReadI2CRegisters(BOOL bADC)
{
	int     i, ret;
	BYTE    reg_val;
	UINT64  slave_addr, reg_addr, regDataR, regDataW;
	CString sRegHex, msg;
	char   *sRegBin;
	DWORD   regIdx;

	regIdx = VIRTEX5_I2CINFO_OFFSET/4 + 1;

	sRegBin = new char [10];
	strcpy(sRegBin, "");

	ret = g_objVirtex5BMD.ReadWriteReg(g_hDevVirtex5, gVIRTEX5_Regs, regIdx, VIRTEX5_REGS_NUM, WDC_READ, FALSE, &regDataR, NULL);
	if (ret == FALSE) {
		MessageBox("Failed to read the run-time register for I2C bus controller", "Read I2C Bus", MB_ICONWARNING);
		return;
	}
	// clear the high 24 bits
	regDataR = regDataR << 62;
	regDataR = regDataR >> 62;
	ret = g_objVirtex5BMD.ReadWriteReg(g_hDevVirtex5, gVIRTEX5_Regs, regIdx, VIRTEX5_REGS_NUM, WDC_WRITE, FALSE, NULL, regDataR);
	if (ret == FALSE) {
		MessageBox("Failed to read the run-time register for I2C bus controller", "Read I2C Bus", MB_ICONWARNING);
		return;
	}

	// Set bit 7 and clear bit 6 and bit 5 of the command
	regDataR |= BIT7;
	regDataR = regDataR&BIT6 ? regDataR-BIT6 : regDataR;
	regDataR = regDataR&BIT5 ? regDataR-BIT5 : regDataR;

	if (bADC) {
		slave_addr = 0x4c000000;
		regDataR |= slave_addr;
		for (i = 0; i < 47; i ++) {
			reg_addr = i << 16;
			regDataW = regDataR | reg_addr;
			sRegHex.Format("0x%08X", regDataW);
			ret = I2CController(regDataW, &reg_val, TRUE);
			switch (ret) {
			case 0:
				sRegHex.Format("0x%02X (0x%02X)", i, reg_val);
				sRegBin = Dec2Bin(reg_val);
				break;
			case 1:
				// pcie driver error
				sRegHex.Format("0x%02X", i);
				strcpy(sRegBin, "drv err");
				break;
			case 2:
				// i2c access error
				strcpy(sRegBin, "I2C err");
				sRegHex.Format("0x%02X", i);
				break;
			case 3:
				// time out error
				strcpy(sRegBin, "time out");
				sRegHex.Format("0x%02X", i);
				break;
			default:
				// other error
				strcpy(sRegBin, "error!");
				sRegHex.Format("0x%02X", i);
				break;
			}
			msg.Format("%s", sRegBin);
			m_adcText[i].SetWindowText(sRegHex);
			m_adcEdit[i].SetWindowText(msg);
			m_adcVals[i] = reg_val;
		}

	} else {
		slave_addr = 0x76000000;
		regDataR |= slave_addr;
		for (i = 0; i < 18; i ++) {
			reg_addr = m_dacRegs[i];
			reg_addr = reg_addr << 16;
			regDataW = regDataR | reg_addr;
			sRegHex.Format("0x%08X", regDataW);
			ret = I2CController(regDataW, &reg_val, TRUE);
			switch (ret) {
			case 0:
				sRegHex.Format("0x%02X (0x%02X)", i, reg_val);
				sRegBin = Dec2Bin(reg_val);
				break;
			case 1:
				// pcie driver error
				sRegHex.Format("0x%02X", i);
				strcpy(sRegBin, "drv err");
				break;
			case 2:
				// i2c access error
				strcpy(sRegBin, "I2C err");
				sRegHex.Format("0x%02X", i);
				break;
			case 3:
				// time out error
				strcpy(sRegBin, "time out");
				sRegHex.Format("0x%02X", i);
				break;
			default:
				// other error
				strcpy(sRegBin, "error!");
				sRegHex.Format("0x%02X", i);
				break;
			}
			msg.Format("%s", sRegBin);
			m_dacEdit[i].SetWindowText(msg);
			m_dacVals[i] = reg_val;
		}

	}

	delete [] sRegBin;
}

int CML506View::I2CController(UINT64 regDataW, BYTE *reg_val, BOOL bI2Cread)
{
	BOOL    bI2Cdone;
	int     ret, timeout;
	UINT64  regDataR, regVal, doneBit, errBit;
	CString msg;
	DWORD   regIdx;

	if (bI2Cread) {
		doneBit = BIT5;
		errBit  = BIT6;
	} else {
		doneBit = BIT2;
		errBit  = BIT3;
	}

	regIdx = VIRTEX5_I2CINFO_OFFSET/4 + 1;
	ret = g_objVirtex5BMD.ReadWriteReg(g_hDevVirtex5, gVIRTEX5_Regs, regIdx, VIRTEX5_REGS_NUM, WDC_WRITE, FALSE, NULL, regDataW);
	if (ret == FALSE) {
		MessageBox("Failed to write the run-time register for I2C bus controller", "Read I2C Bus", MB_ICONWARNING);
		*reg_val = 0xff;
		return 1;
	}
	bI2Cdone = FALSE;
	timeout  = 0;
	do {
		Sleep(1);
		ret = g_objVirtex5BMD.ReadWriteReg(g_hDevVirtex5, gVIRTEX5_Regs, regIdx, VIRTEX5_REGS_NUM, WDC_READ, FALSE, &regDataR, NULL);
		if (ret == FALSE) {
			MessageBox("Failed to read the run-time register for I2C bus controller", "Read I2C Bus", MB_ICONWARNING);
			*reg_val = 0xff;
			return 1;
		}
		msg.Format("0x%16X", regDataR);

		// I2C bus controller has finished data access
		//if (regDataR & BIT5) {
		if (regDataR & doneBit) {
			bI2Cdone = TRUE;
			//if (regDataR & BIT6) {
			if (regDataR & errBit) {
				*reg_val = 0xff;
				return 2;
			} else {
				regVal = regDataR << 48;
				regVal = regVal >> 56;
				msg.Format("0x%16X", regVal);
				*reg_val = (BYTE)regVal;
			}
			msg.Format("0x%16X", regDataR);
		}
		timeout ++;
		if (timeout > 10) {
			bI2Cdone = TRUE;
			*reg_val = 0xff;
			return 3;
		}
	} while (bI2Cdone == FALSE);

	regDataR = regDataR << 62;
	regDataR = regDataR >> 62;
	ret = g_objVirtex5BMD.ReadWriteReg(g_hDevVirtex5, gVIRTEX5_Regs, regIdx, VIRTEX5_REGS_NUM, WDC_WRITE, FALSE, NULL, regDataR);
	if (ret == FALSE) {
		MessageBox("Failed to write the run-time register for I2C bus controller", "Read I2C Bus", MB_ICONWARNING);
		*reg_val = 0xff;
		return 2;
	}
	msg.Format("0x%16X", regDataR);

	return 0;
}

char * CML506View::Dec2Bin(BYTE val)
{
	char *msg;

	msg = new char [10];
	strcpy(msg, "");

	if (val & BIT7) msg[0] = '1';
	else msg[0] = '0';
	if (val & BIT6) msg[1] = '1';
	else msg[1] = '0';
	if (val & BIT5) msg[2] = '1';
	else msg[2] = '0';
	if (val & BIT4) msg[3] = '1';
	else msg[3] = '0';
	if (val & BIT3) msg[4] = '1';
	else msg[4] = '0';
	if (val & BIT2) msg[5] = '1';
	else msg[5] = '0';
	if (val & BIT1) msg[6] = '1';
	else msg[6] = '0';
	if (val & BIT0) msg[7] = '1';
	else msg[7] = '0';
	msg[8] = ' ';
	msg[9] = ' ';

	return msg;
}

void CML506View::ReadWritePCIeRegister(BOOL bRead)
{
	DWORD   regIndex;
	UINT64  regValue;
	CString msg;
	int     ret;

	regIndex = m_regAddr/4;
	if (bRead == TRUE) {
		ret = g_objVirtex5BMD.ReadWriteReg(g_hDevVirtex5, gVIRTEX5_Regs, regIndex+1, VIRTEX5_REGS_NUM, WDC_READ, FALSE, &regValue, NULL);
		if (ret == TRUE) {
			msg.Format("%08X", regValue);
		} else {
			msg = _T("error !!");
		}
		m_EditCurrentRegVal.SetWindowText(msg);
	} else {
		regValue = m_regVal;
		ret = g_objVirtex5BMD.ReadWriteReg(g_hDevVirtex5, gVIRTEX5_Regs, regIndex+1, VIRTEX5_REGS_NUM, WDC_WRITE, FALSE, NULL, regValue);
		if (ret == TRUE) {
			msg.Format("%08X", regValue);
		} else {
			msg = _T("error !!");
		}
		m_regEdit[regIndex].SetWindowText(msg);
	}
}


int CML506View::ReadWriteI2CRegister(BOOL bRead, UINT64 slave_addr)
{
	UINT64  regData, regAddr, regVal;
	int     ret, i, idx;
	CString msg;
	BYTE    reg_val;
	char   *sRegBin;
	DWORD   regIdx;
	BOOL    rwFlag;

	sRegBin = new char [10];
	strcpy(sRegBin, "");

	regIdx = VIRTEX5_I2CINFO_OFFSET/4 + 1;
	ret = g_objVirtex5BMD.ReadWriteReg(g_hDevVirtex5, gVIRTEX5_Regs, regIdx, VIRTEX5_REGS_NUM, WDC_READ, FALSE, &regData, NULL);
	if (ret == FALSE) {
		MessageBox("Failed to read the run-time register for I2C bus controller", "Read I2C Bus", MB_ICONWARNING);
		return 4;
	}

	// clear the high 30 bits
	//regData = regData << 62;
	//regData = regData >> 62;
	regData = 0;
	ret = g_objVirtex5BMD.ReadWriteReg(g_hDevVirtex5, gVIRTEX5_Regs, regIdx, VIRTEX5_REGS_NUM, WDC_WRITE, FALSE, NULL, regData);
	if (ret == FALSE) {
		MessageBox("Failed to read the run-time register for I2C bus controller", "Read I2C Bus", MB_ICONWARNING);
		return 4;
	}

	// move "register address" to bits 23-16
	regAddr = m_regAddr;
	regAddr = regAddr << 16;

	// move "register address" to bits 15-8
	regVal = m_regVal;
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

	msg.Format("0x%16X", regData);

	if (bRead)
		ret = I2CController(regData, &reg_val, TRUE);
	else
		ret = I2CController(regData, &reg_val, FALSE);

	switch (ret) {
	case 0:
		sRegBin = Dec2Bin(reg_val);
		rwFlag  = TRUE;
		break;
	case 1:
		// pcie driver error
		strcpy(sRegBin, "drv err");
		rwFlag  = FALSE;
		break;
	case 2:
		// i2c access error
		strcpy(sRegBin, "I2C err");
		rwFlag  = FALSE;
		break;
	case 3:
		// time out error
		strcpy(sRegBin, "time out");
		rwFlag  = FALSE;
		break;
	default:
		// other error
		strcpy(sRegBin, "error!");
		rwFlag  = FALSE;
		break;
	}

	if (slave_addr == 0x4c000000) {
		m_adcEdit[m_regAddr].SetWindowText(sRegBin);
		msg.Format("0x%02X (0x%02X)", m_regAddr, reg_val);
		m_adcText[m_regAddr].SetWindowText(msg);
		if (rwFlag) m_adcVals[m_regAddr] = reg_val;
	} else {
		for (i = 0; i < 18; i ++) {
			if (m_regAddr == m_dacRegs[i]) {
				m_dacVals[i] = reg_val;
				break;
			}
		}
		if (i < 18)	m_dacEdit[i].SetWindowText(sRegBin);
	}

	m_EditCurrentRegVal.SetWindowText(sRegBin);

	delete [] sRegBin;

	return ret;
}

void CML506View::OnNewDcf()
{
	char titleName[120];
	int  i;

	g_bDCFloaded = TRUE;
	g_bDCFsaved  = FALSE;
	UpdateRuntimeRegisters();
	UpdateGuiControls();
	EnableGuiControls(TRUE);

	strcpy(titleName, "Xilinx ML506 Digitizer Controller - *.*");
	m_strTitleName = titleName;
	GetParentOwner()->SetWindowText(m_strTitleName);
	m_bXFGloaded = TRUE;

	if (m_pDlgR != NULL) return;
	m_pDlgR = new CVideoChannelR(this);
	if (m_pDlgR->GetSafeHwnd() == 0) {
        m_pDlgR->Create(IDD_DIALOG_CH1, GetDesktopWindow()); // displays the dialog window
		m_pDlgR->ShowWindow(SW_SHOW);
	}
/*
	m_iShiftBS = m_iPLLclks - g_VideoInfo.img_width;
	CString msg;
	msg.Format("BS offset,%d", m_iShiftBS-1);
	m_lblBlankPixels.SetWindowText(msg);
	m_scrBlankPixels.SetScrollPos(m_iShiftBS);
*/
	UpdateVidSize(TRUE);
}

void CML506View::OnLoadDcf()
{
	char     BASED_CODE szFilter[] = "Image Grabber Configuration File(*.xgf)|*.xgf|";
	char     xgfFileName[120], titleName[240];
	int      ret, tmpInt;
	BOOL     tmpBool;
	FILE    *fp;
	unsigned short tmpA;
	UINT32   tmpB;
	UINT32   regTemp, reg32, regData, regData1, regData2;
	BYTE     tmpC, tmpD[47];
	CML506Doc* pDoc = GetDocument();
	CString  msg;
/*
	// open an .avi file
	CFileDialog fd(TRUE, "xgf", NULL, NULL, szFilter);
	if (fd.DoModal() != IDOK) return;*/

	strcpy (xgfFileName, "c:\\Programs\\ml506_3CH_03082017.xgf");// <<<---- update XGF filename

//	strcpy(xgfFileName, fd.GetPathName());
	fp = fopen(xgfFileName, "rb");
	if (fp == NULL) {
		MessageBox("Invalid Image Grabber Configuration File", "Save Image Grabber File", MB_ICONWARNING);
		return;
	}

	ret = fread(&tmpA, sizeof(unsigned short), 1, fp);
	g_VideoInfo.img_width = tmpA;
	if (g_VideoInfo.img_width<128 || g_VideoInfo.img_width>2048) {
		msg.Format(_T("Unreasonable image width: %d pixels"), g_VideoInfo.img_width);
		MessageBox(msg, "Read Image Width", MB_ICONWARNING);
	}
	ret = fread(&tmpA, sizeof(unsigned short), 1, fp);
	g_VideoInfo.img_height = tmpA;
	if (g_VideoInfo.img_height<1 || g_VideoInfo.img_height>1000) {
		msg.Format(_T("Unreasonable image height: %d lines"), g_VideoInfo.img_height);
		MessageBox(msg, "Read Image Height", MB_ICONWARNING);
	}
	ret = fread(&tmpC, sizeof(unsigned char), 1, fp);
	g_VideoInfo.line_spacing = tmpC;
	if (g_VideoInfo.line_spacing<1 || g_VideoInfo.line_spacing>32) {
		msg.Format(_T("Unreasonable strip height: %d lines"), g_VideoInfo.line_spacing);
		MessageBox(msg, "Read strip height", MB_ICONWARNING);
	}
	ret = fread(&tmpB, sizeof(UINT32), 1, fp);
	g_VideoInfo.addr_interval = tmpB;
	ret = fread(&tmpA, sizeof(unsigned short), 1, fp);
	g_VideoInfo.offset_line = tmpA;
	if (g_VideoInfo.offset_line<0 || g_VideoInfo.offset_line>g_VideoInfo.img_height) {
		msg.Format(_T("Unreasonable line offset: %d lines"), g_VideoInfo.offset_line);
		MessageBox(msg, "Read line offset", MB_ICONWARNING);
	}
	ret = fread(&tmpA, sizeof(unsigned short), 1, fp);
	g_VideoInfo.offset_pixel = tmpA;
	if (g_VideoInfo.offset_pixel<0 || g_VideoInfo.offset_pixel>m_iPLLclks-g_VideoInfo.img_width) {
		msg.Format(_T("Unreasonable pixel offset: %d pixels"), g_VideoInfo.offset_pixel);
		MessageBox(msg, "Read pixel offset", MB_ICONWARNING);
	}

	ret = fread(&tmpBool, sizeof(BOOL), 1, fp);
	m_bDualChannel = tmpBool;
	g_VideoInfo.bDualChannel = m_bDualChannel;
	ret = fread(&tmpBool, sizeof(BOOL), 1, fp);
	m_bSampleG = tmpBool;
	ret = fread(&tmpBool, sizeof(BOOL), 1, fp);
	m_bSampleB = tmpBool;
	ret = fread(&tmpInt, sizeof(int), 1, fp);
	m_iVideoNum = tmpInt;
	if (m_iVideoNum<1 || m_iVideoNum>3) {
		msg.Format(_T("Unreasonable sampling channel numbers: %d"), m_iVideoNum);
		MessageBox(msg, "ADC sampling channels", MB_ICONWARNING);
	}
	ret = fread(&tmpInt, sizeof(int), 1, fp);
	m_iChannelNum = tmpInt;
	if (m_iChannelNum<1 || m_iChannelNum>2) {
		msg.Format(_T("Unreasonable sampling edges: %d"), m_iChannelNum);
		MessageBox(msg, "ADC sampling edges", MB_ICONWARNING);
	}
	ret = fread(&tmpInt, sizeof(int), 1, fp);
	m_iShiftBS = tmpInt;
	if (m_iShiftBS<0 || m_iShiftBS>m_iPLLclks/2) {
		msg.Format(_T("Unreasonable spacing between forward scanning and backward scanning: %d"), m_iShiftBS);
		MessageBox(msg, "Slow scanning zone", MB_ICONWARNING);
	}

	ret = fread(&tmpInt, sizeof(int), 1, fp);
	g_VideoInfo.nVideoTabs = tmpInt;
	if (g_VideoInfo.nVideoTabs<1 || g_VideoInfo.nVideoTabs>3) {
		msg.Format(_T("Unreasonable sampling channel numbers: %d"), g_VideoInfo.nVideoTabs);
		MessageBox(msg, "ADC sampling channels", MB_ICONWARNING);
	}
	ret = fread(&tmpInt, sizeof(int), 1, fp);
	g_VideoInfo.nVideoChannels = tmpInt;
	if (g_VideoInfo.nVideoChannels<1 || g_VideoInfo.nVideoChannels>2) {
		msg.Format(_T("Unreasonable sampling edges: %d"), g_VideoInfo.nVideoChannels);
		MessageBox(msg, "ADC sampling edges", MB_ICONWARNING);
	}

	// resonant FOV
	ret = fread(&tmpC, sizeof(BYTE), 1, fp);
	g_VideoInfo.fovH = tmpC;
	// galvo scanner FOV
	ret = fread(&tmpC, sizeof(BYTE), 1, fp);
	g_VideoInfo.fovV = tmpC;
	// forward scanning lines
	ret = fread(&tmpA, sizeof(unsigned short), 1, fp);
	g_VideoInfo.fLines = tmpA;
	if (g_VideoInfo.fLines<1 || g_VideoInfo.fLines>1000) {
		msg.Format(_T("Unreasonable scanning lines per frame: %d"), g_VideoInfo.fLines);
		MessageBox(msg, "Forward scanning lines", MB_ICONWARNING);
	}
	// backward scanning steps
	ret = fread(&tmpA, sizeof(unsigned short), 1, fp);
	g_VideoInfo.bStepSize = tmpA;
	if (g_VideoInfo.bStepSize<2 || g_VideoInfo.bStepSize>g_VideoInfo.fLines) {
		msg.Format(_T("Unreasonable reverse scanning step: %d"), g_VideoInfo.bStepSize);
		MessageBox(msg, "Reverse scanning step size", MB_ICONWARNING);
	}
	// ramp offset
	ret = fread(&tmpA, sizeof(unsigned short), 1, fp);
	g_VideoInfo.vOffset = tmpA;
	// flag of interlacing line
	ret = fread(&tmpBool, sizeof(BOOL), 1, fp);
	g_VideoInfo.bDoubleLine = tmpBool;
	// flag of setting backward slow scanning to zero
	ret = fread(&tmpBool, sizeof(BOOL), 1, fp);
	g_VideoInfo.bZeroBackward = tmpBool;
	// flag of symmetric ramp
	ret = fread(&tmpBool, sizeof(BOOL), 1, fp);
	g_VideoInfo.bSymmetricRamp = tmpBool;

	m_fovH         = g_VideoInfo.fovH;
	m_fovV         = g_VideoInfo.fovV;
	m_forwardLines = g_VideoInfo.fLines;
	m_backLines    = (int)(0.5+1.0*g_VideoInfo.fLines/g_VideoInfo.bStepSize);
	m_rampOffset   = g_VideoInfo.vOffset;
	m_bInterleaveLines    = g_VideoInfo.bDoubleLine;
	m_bZeroBackward       = g_VideoInfo.bZeroBackward;
	m_bSymmetricRamp      = g_VideoInfo.bSymmetricRamp;

	UpdateScannerParams();

	ret = fread(tmpD, sizeof(BYTE), 47, fp);

	m_adcVals[0x01] = tmpD[0x01];		// PLL dividor, force pixel clock to 20MHz
	m_adcVals[0x02] = tmpD[0x02];
	m_adcVals[0x03] = tmpD[0x03];		// VCO/CPMP
	m_adcVals[0x04] = tmpD[0x04];		// Phase adjustment
	m_adcVals[0x05] = tmpD[0x05];		// contrast R
	m_adcVals[0x06] = tmpD[0x06];
	m_adcVals[0x07] = tmpD[0x07];		// contrast G
	m_adcVals[0x08] = tmpD[0x08];
	m_adcVals[0x09] = tmpD[0x09];		// contrast B
	m_adcVals[0x0A] = tmpD[0x0A];
	m_adcVals[0x0B] = tmpD[0x0B];		// brightness R
	m_adcVals[0x0C] = tmpD[0x0C];
	m_adcVals[0x0D] = tmpD[0x0D];		// brightness G
	m_adcVals[0x0E] = tmpD[0x0E];
	m_adcVals[0x0F] = tmpD[0x0F];		// brightness B
	m_adcVals[0x10] = tmpD[0x10];
	m_adcVals[0x12] = tmpD[0x12];		// Hsync control
	m_adcVals[0x14] = tmpD[0x14];		// Vsync control
	m_adcVals[0x18] = tmpD[0x18];		// clamping control
	m_adcVals[0x19] = tmpD[0x19];		// clamping placement
	m_adcVals[0x1A] = tmpD[0x1A];		// clamping duration
	m_adcVals[0x1B] = tmpD[0x1B];		// clamping trigger

	m_iPLLclks      = ((m_adcVals[0x01]<<4) + (m_adcVals[0x02]>>4)) >> 1;
	GenDesinusoidLUT();

	// assign register with information of start line address and end line address;
//	regTemp  = g_VideoInfo.line_start_addr;
//	reg32    = regTemp << 16;
//	reg32    = reg32 | g_VideoInfo.line_end_addr;
//	g_objVirtex5BMD.VIRTEX5_WriteReg32(g_hDevVirtex5, VIRTEX5_LINEINFO_OFFSET, reg32);

	// assign register with information of image height and end line address;
	regTemp  = g_VideoInfo.line_spacing;
	reg32    = regTemp << 24;
	regTemp  = g_VideoInfo.img_width << 12;
	reg32    = reg32 | regTemp;
	reg32    = reg32 | g_VideoInfo.img_height;
	g_objVirtex5BMD.VIRTEX5_WriteReg32(g_hDevVirtex5, VIRTEX5_IMAGESIZE_OFFSET, reg32);

	// assign register with image offsets and end line ID
//	regTemp  = g_VideoInfo.offset_pixel;
//	reg32    = regTemp << 24;
//	regTemp  = g_VideoInfo.offset_line;
//	regTemp  = regTemp << 16;
	regTemp  = g_VideoInfo.offset_line;
	reg32    = regTemp << 24;
	regTemp  = g_VideoInfo.offset_pixel;
	regTemp  = regTemp << 14;
	reg32    = reg32 | regTemp;
	//reg32    = reg32 | BIT14;
	reg32    = reg32 | BIT13 | BIT12;		// choose red channel
	reg32    = reg32 - BIT12;
	reg32    = reg32 | g_VideoInfo.end_line_ID;
	g_objVirtex5BMD.VIRTEX5_WriteReg32(g_hDevVirtex5, VIRTEX5_IMAGEOS_OFFSET, reg32);



	//g_VideoInfo.tlp_counts  = g_VideoInfo.img_width/8+3;		// this one has to be updated
	g_VideoInfo.tlp_counts = (g_VideoInfo.img_width*m_iVideoNum*m_iChannelNum+8)/8+3;


	// write TLP counts to PCIe register
	regTemp  = g_VideoInfo.tlp_counts/2;
	g_objVirtex5BMD.VIRTEX5_WriteReg32(g_hDevVirtex5, VIRTEX5_WDMATLPC_OFFSET, regTemp);

	// update active window on GUI
//	pDoc->UpdateAllViews(this, USER_MESSAGE_IMGWIDTH, NULL);

	g_bDCFloaded = TRUE;
	UpdateRuntimeRegisters();
	UpdateGuiControls();
	EnableGuiControls(TRUE);

	// setup title name
	m_strFileName = xgfFileName;
	m_strTitleName = m_strAppName + m_strFileName;
	GetParentOwner()->SetWindowText(m_strTitleName);

	fclose(fp);
	m_bXFGloaded = TRUE;

	if (m_pDlgR == NULL) {
		m_pDlgR = new CVideoChannelR(this);
		if (m_pDlgR->GetSafeHwnd() == 0) {
			m_pDlgR->Create(IDD_DIALOG_CH1, GetDesktopWindow()); // displays the dialog window
			m_pDlgR->ShowWindow(SW_SHOW);
		}
	}

	if (m_pDlgG == NULL && m_bSampleG) {
		m_pDlgG = new CVideoChannelG(this);
		if (m_pDlgG->GetSafeHwnd() == 0) {
			m_pDlgG->Create(IDD_DIALOG_CH2, GetDesktopWindow()); // displays the dialog window
			m_pDlgG->ShowWindow(SW_SHOW);
		}
	}

	if (m_pDlgB == NULL && m_bSampleB) {
		m_pDlgB = new CVideoChannelB(this);
		if (m_pDlgB->GetSafeHwnd() == 0) {
			m_pDlgB->Create(IDD_DIALOG_CH3, GetDesktopWindow()); // displays the dialog window
			m_pDlgB->ShowWindow(SW_SHOW);
		}
	}

	CheckChannelG(m_bSampleG, FALSE);
	CheckChannelB(m_bSampleB, FALSE);
//	CheckDualChannel(m_bDualChannel);
	UpdateVidSize(TRUE);

	msg.Format("BS offset,%d", m_iShiftBS);
	m_lblBlankPixels.SetWindowText(msg);
	m_scrBlankPixels.SetScrollPos(m_iShiftBS);

	regData  = g_objVirtex5BMD.VIRTEX5_ReadReg32(g_hDevVirtex5, VIRTEX5_LINECTRL_OFFSET);
	regData1 = regData << 26;
	regData1 = regData1 >> 26;		// low 6 its;
	regData2 = regData >> 16;
	regData2 = regData2 << 16;		// high 16 bits;
	regData  = regData2 + (m_iShiftBS<<6) + regData1;
	g_objVirtex5BMD.VIRTEX5_WriteReg32(g_hDevVirtex5, VIRTEX5_LINECTRL_OFFSET, regData);

}

void CML506View::OnSaveDcf()
{
	char     BASED_CODE szFilter[] = "Image Grabber Configuration File(*.xgf)|*.xgf|";
	char     pname[120]="", fname[40] = "", txtname[120]="";
	CString  filename, titleName, txtFile, text;
	int      len, i, ret, brightness, contrast;
	FILE    *fp;
/*
	if (VerifyI2CVals() == FALSE) {
		MessageBox("Register values of ADC are incomplete. Load/update all register values before saving the image grabber configuration file",
					"Save Image Grabber File", MB_ICONWARNING);
		return;
	}
*/
	if (m_strFileName.GetLength() == 0) {
		// open an .xfg file
		CFileDialog fd(FALSE, "xgf", NULL, NULL, szFilter);
		if (fd.DoModal() != IDOK) return;

		// get the path and file name of this .avi file
		filename = fd.GetFileName();
		len = filename.GetLength();
		for (i = 0; i < len; i ++) {
			fname[i] = filename.GetAt(i);
		}
		::GetCurrentDirectory(120, pname);
		strcat(pname, "\\");
		strcat(pname, fname);
	} else {
		len = m_strFileName.GetLength();
		for (i = 0; i < len; i ++) {
			pname[i] = m_strFileName.GetAt(i);
		}
	}

	m_strFileName = pname;
	for (i = 0; i < m_strFileName.GetLength()-3; i ++)
		txtname[i] = m_strFileName.GetAt(i);
	strcat(txtname, "txt");


	fp = fopen(pname, "wb");
	if (fp == NULL) {
		filename.Format("Error: Can't open file <%s> for writting.", pname);
		AfxMessageBox(filename, MB_ICONWARNING);
		return;
	}
	// write image width
	ret = fwrite(&g_VideoInfo.img_width, sizeof(unsigned short), 1, fp);
	// write image height
	ret = fwrite(&g_VideoInfo.img_height, sizeof(unsigned short), 1, fp);
	// write line spacing
	ret = fwrite(&g_VideoInfo.line_spacing, sizeof(unsigned char), 1, fp);
	// write address interval
	ret = fwrite(&g_VideoInfo.addr_interval, sizeof(UINT32), 1, fp);
	// line offset
	ret = fwrite(&g_VideoInfo.offset_line, sizeof(unsigned short), 1, fp);
	// pixel offset
	ret = fwrite(&g_VideoInfo.offset_pixel, sizeof(unsigned short), 1, fp);
	// flag of forward&backward scanning
	ret = fwrite(&m_bDualChannel, sizeof(BOOL), 1, fp);
	// flag of sampling channel green
	ret = fwrite(&m_bSampleG, sizeof(BOOL), 1, fp);
	// flag of sampling channel blue
	ret = fwrite(&m_bSampleB, sizeof(BOOL), 1, fp);
	// number of ADC channels used
	ret = fwrite(&m_iVideoNum, sizeof(int), 1, fp);
	// forward and/or backward scanning
	ret = fwrite(&m_iChannelNum, sizeof(int), 1, fp);
	// black area between forward scanning and backward scanning
	ret = fwrite(&m_iShiftBS, sizeof(int), 1, fp);
	// number of ADC channels used
	ret = fwrite(&g_VideoInfo.nVideoTabs, sizeof(int), 1, fp);
	// forward and/or backward scanning
	ret = fwrite(&g_VideoInfo.nVideoChannels, sizeof(int), 1, fp);

	// resonant FOV
	ret = fwrite(&g_VideoInfo.fovH, sizeof(BYTE), 1, fp);
	// galvo FOV
	ret = fwrite(&g_VideoInfo.fovV, sizeof(BYTE), 1, fp);
	// forward scanning lines
	ret = fwrite(&g_VideoInfo.fLines, sizeof(unsigned short), 1, fp);
	// backward scanning step size
	ret = fwrite(&g_VideoInfo.bStepSize, sizeof(unsigned short), 1, fp);
	// ramp offset
	ret = fwrite(&g_VideoInfo.vOffset, sizeof(unsigned short), 1, fp);
	// flag of interlacing lines
	ret = fwrite(&g_VideoInfo.bDoubleLine, sizeof(BOOL), 1, fp);
	// flag of setting backward slow scanning to zero
	ret = fwrite(&g_VideoInfo.bZeroBackward, sizeof(BOOL), 1, fp);
	// flag of setting symmetric ramp signal
	ret = fwrite(&g_VideoInfo.bSymmetricRamp, sizeof(BOOL), 1, fp);

	ret = fwrite(m_adcVals, sizeof(BYTE), 47, fp);

	fclose(fp);

	g_bDCFsaved = TRUE;
//	strcpy(m_LUTfname, pname);

	m_strTitleName = m_strAppName + pname;
	GetParentOwner()->SetWindowText(m_strTitleName);


	fp = fopen(txtname, "w");
	fprintf(fp, "Image width:\t\t%d\n",  g_VideoInfo.img_width);
	fprintf(fp, "Image height:\t\t%d\n", g_VideoInfo.img_height);
	fprintf(fp, "Image width offset:\t%d\n",  g_VideoInfo.offset_pixel);
	fprintf(fp, "Image height offset:\t%d\n", g_VideoInfo.offset_line);

	fprintf(fp, "Channel 1:\t\tOn\n");
	fprintf(fp, "Channel 2:\t\t%s\n",    m_bSampleG?"On":"Off");
	fprintf(fp, "Channel 3:\t\t%s\n",    m_bSampleB?"On":"Off");

	m_edtBrightnessR.GetWindowText(text);
	brightness = atoi(text);
	fprintf(fp, "Brightness 1:\t\t%d\n", brightness);
	m_edtContrastR.GetWindowText(text);
	contrast = atoi(text);
	fprintf(fp, "Contrast 1:\t\t%d, %3.2fmV\n",   contrast, 500+contrast/0.254);

	m_edtBrightnessG.GetWindowText(text);
	brightness = atoi(text);
	fprintf(fp, "Brightness 2:\t\t%d\n", brightness);
	m_edtContrastG.GetWindowText(text);
	contrast = atoi(text);
	fprintf(fp, "Contrast 2:\t\t%d, %3.2fmV\n",   contrast, 500+contrast/0.254);

	m_edtBrightnessB.GetWindowText(text);
	brightness = atoi(text);
	fprintf(fp, "Brightness 3:\t\t%d\n", brightness);
	m_edtContrastB.GetWindowText(text);
	contrast = atoi(text);
	fprintf(fp, "Contrast 3:\t\t%d, %3.2fmV\n",   contrast, 500+contrast/0.254);
/*
	// forward and/or backward scanning
	fprintf(fp, "fast scanning:\t\t%s\n", m_iChannelNum==2?"FS/BS":"FS");
	// black area between forward scanning and backward scanning
	fprintf(fp, "Back scan shift:\t%d\n", m_iShiftBS);
	// resonant FOV
	fprintf(fp, "Resonant scanner FOV:\t%d (uint)\n", g_VideoInfo.fovH);
	// galvo FOV
	fprintf(fp, "Slow scanner FOV:\t%d (uint)\n", g_VideoInfo.fovV); */

	fclose(fp);
}

void CML506View::OnUpdateSaveDCF(CCmdUI* pCmdUI) {
	pCmdUI->Enable(m_bXFGloaded);
}


BOOL CML506View::VerifyI2CVals()
{
	int       i;
    CString   text;

	for (i = 0; i < 47; i ++) {
		GetDlgItemText(m_adcEditID[i], text);
		text.TrimLeft();
		text.TrimRight();
		if (text.GetLength() == 0) return FALSE;
	}

	return TRUE;
}

void CML506View::OnSize(UINT nType, int cx, int cy)
{
	if (m_bClientCreated == TRUE) {
		m_tabPCIe.SetWindowPos(&wndBottom, 0, 0, cx, cy, SWP_SHOWWINDOW);
	}

	CView::OnSize(nType, cx, cy);
}

void CML506View::OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar)
{
	CML506Doc* pDoc = GetDocument();
	CString msg;
    int		i, nTemp1, nTemp2, doBound, upBound;
	UINT32  regData, regData1, regData2;
	BYTE    hiByte, loByte;
	DWORD   dwStatus, dwResult;
	BYTE    stepSize;
	int     rampLow, rampHigh;
	float   fPos;

	nTemp1 = pScrollBar->GetScrollPos();
	pScrollBar->GetScrollRange(&doBound, &upBound);

    switch(nSBCode) {
    case SB_THUMBPOSITION:
        pScrollBar->SetScrollPos(nPos);
		nTemp1 = nPos;
        break;
    case SB_LINELEFT: // left arrow button
        nTemp2 = 1;
        if ((nTemp1 - nTemp2) > doBound) {
            nTemp1 -= nTemp2;
        } else {
            nTemp1 = doBound;
        }
        pScrollBar->SetScrollPos(nTemp1);
        break;
    case SB_LINERIGHT: // right arrow button
        nTemp2 = 1;
        if ((nTemp1 + nTemp2) < upBound) {
            nTemp1 += nTemp2;
        } else {
            nTemp1 = upBound;
        }
        pScrollBar->SetScrollPos(nTemp1);
        break;
    case SB_PAGELEFT: // left arrow button
        nTemp2 = 10;
        if ((nTemp1 - nTemp2) > doBound) {
            nTemp1 -= nTemp2;
        } else {
            nTemp1 = doBound;
        }
        pScrollBar->SetScrollPos(nTemp1);
        break;
    case SB_PAGERIGHT: // right arrow button
        nTemp2 = 10;
        if ((nTemp1 + nTemp2) < upBound) {
            nTemp1 += nTemp2;
        } else {
            nTemp1 = upBound;
        }
        pScrollBar->SetScrollPos(nTemp1);
        break;
	case SB_THUMBTRACK:
        pScrollBar->SetScrollPos(nPos);
		nTemp1 = nPos;
		break;
    }


	switch (pScrollBar->GetDlgCtrlID()) {
	case ID_SCROLL_IMAGEWIDTH:
		pScrollBar->SetScrollPos(nTemp1);
		nTemp1 = (nTemp1/8)*8;
		msg.Format("Image Width, \t%d pixels", nTemp1);
		m_lblImgSizeX.SetWindowText(msg);

		g_VideoInfo.img_width       = nTemp1+8;

		// setup image width on FPGA registers
		regData = g_objVirtex5BMD.VIRTEX5_ReadReg32(g_hDevVirtex5, VIRTEX5_IMAGESIZE_OFFSET);
		msg.Format("%08X", regData);
		regData1 = regData >> 24;	// reserve high 8 bits
		regData1 = regData1 << 24;
		regData2 = regData << 20;	// reserve low 12 bits
		regData2 = regData2 >> 20;
		regData  = g_VideoInfo.img_width;
		regData  = regData << 12;
		regData  = regData + regData1 + regData2;
		msg.Format("%08X", regData);
		// write image width to PCIe register
		g_objVirtex5BMD.VIRTEX5_WriteReg32(g_hDevVirtex5, VIRTEX5_IMAGESIZE_OFFSET, regData);

		g_VideoInfo.tlp_counts      = (g_VideoInfo.img_width*m_iVideoNum*m_iChannelNum+8)/8+3;

		// write TLP counts to PCIe register
		regData  = g_VideoInfo.tlp_counts/2;
		g_objVirtex5BMD.VIRTEX5_WriteReg32(g_hDevVirtex5, VIRTEX5_WDMATLPC_OFFSET, regData);

		// update active window on GUI
		UpdateVidSize(FALSE);

		ResetTrackingPos();

		break;
	case ID_SCROLL_IMAGEHEIGHT:
		pScrollBar->SetScrollPos(nTemp1);
		nTemp1 = (nTemp1/16)*16;

		if (nTemp1+g_VideoInfo.offset_line > g_VideoInfo.fLines) {
			pScrollBar->SetScrollPos(g_VideoInfo.img_height);
			MessageBox(_T("Image Height plus line offset can't be greater than number of scanning lines!"), _T("Image Height"), MB_ICONWARNING);
			return;
		}

		msg.Format("Image Height, \t%d lines", nTemp1);
		m_lblImgSizeY.SetWindowText(msg);

		g_VideoInfo.img_height       = nTemp1;

		// setup image width on FPGA registers
		regData = g_objVirtex5BMD.VIRTEX5_ReadReg32(g_hDevVirtex5, VIRTEX5_IMAGESIZE_OFFSET);
		regData = regData >> 12;	// reserve high 20 bits
		regData = regData << 12;
		regData = regData + g_VideoInfo.img_height;
		// write image width to PCIe register
		g_objVirtex5BMD.VIRTEX5_WriteReg32(g_hDevVirtex5, VIRTEX5_IMAGESIZE_OFFSET, regData);
		// kernel plugin needs to be updated as well
		DWORD dwStatus, dwResult;
		dwStatus = WDC_CallKerPlug(g_hDevVirtex5, KP_VRTX5_MSG_DATA, &g_VideoInfo, &dwResult);

		// update active window on GUI
		UpdateVidSize(FALSE);

		ResetTrackingPos();

		break;
	case ID_SCROLL_OFFSETX:
		msg.Format("Image Offset X, \t%d pixels", nTemp1);
		m_lblOffsetX.SetWindowText(msg);
		g_VideoInfo.offset_pixel = nTemp1;
		regData1 = m_scrOffsetX.GetScrollPos();
		regData2 = m_scrOffsetY.GetScrollPos();
		regData  = g_objVirtex5BMD.VIRTEX5_ReadReg32(g_hDevVirtex5, VIRTEX5_IMAGEOS_OFFSET);
		regData  = regData  << 18;//14;
		regData  = regData  >> 18;//14;
		regData1 = regData1 << 14;
		regData2 = regData2 << 24;
		regData  = regData + regData1 + regData2;
		g_objVirtex5BMD.VIRTEX5_WriteReg32(g_hDevVirtex5, VIRTEX5_IMAGEOS_OFFSET, regData);
		break;
	case ID_SCROLL_OFFSETY:
		if (nTemp1 > g_VideoInfo.fLines-g_VideoInfo.img_height) {
			pScrollBar->SetScrollPos(g_VideoInfo.offset_line);
			MessageBox(_T("Over margin!"), _T("Image Offset Y"), MB_ICONWARNING);
			return;
		}
		msg.Format("Image Offset Y, \t%d lines", nTemp1);
		m_lblOffsetY.SetWindowText(msg);
		g_VideoInfo.offset_line = nTemp1;
		regData1 = m_scrOffsetX.GetScrollPos();
		regData2 = m_scrOffsetY.GetScrollPos();
		regData  = g_objVirtex5BMD.VIRTEX5_ReadReg32(g_hDevVirtex5, VIRTEX5_IMAGEOS_OFFSET);
		regData  = regData  << 18;//14;
		regData  = regData  >> 18;//14;
		regData1 = regData1 << 14;
		regData2 = regData2 << 24;
		regData  = regData + regData1 + regData2;
		g_objVirtex5BMD.VIRTEX5_WriteReg32(g_hDevVirtex5, VIRTEX5_IMAGEOS_OFFSET, regData);

		break;
	case ID_SCROLL_SAMPLE_BOTH:
		msg.Format("BS offset,%d", nTemp1);
		m_iShiftBS = nTemp1;
		m_lblBlankPixels.SetWindowText(msg);

		regData  = g_objVirtex5BMD.VIRTEX5_ReadReg32(g_hDevVirtex5, VIRTEX5_LINECTRL_OFFSET);
		regData1 = regData << 26;
		regData1 = regData1 >> 26;		// low 6 its;
		regData2 = regData >> 16;
		regData2 = regData2 << 16;		// high 16 bits;
		regData  = regData2 + (m_iShiftBS<<6) + regData1;
		g_objVirtex5BMD.VIRTEX5_WriteReg32(g_hDevVirtex5, VIRTEX5_LINECTRL_OFFSET, regData);

		break;
	case ID_SCR_RESONANT:
		msg.Format("Resonant FOV: %d/[0~255]", nTemp1);
		m_lblResonant.SetWindowText(msg);
		m_fovH = nTemp1;
		g_VideoInfo.fovH = m_fovH;
//		stepSize = (int)(0.5+1.0*m_forwardLines/m_backLines);	// duty cycle off
		stepSize = m_bSymmetricRamp ? m_backLines : (int)(0.5+1.0*m_forwardLines/m_backLines);	// duty cycle off
		g_objVirtex5BMD.SetScannerParams(g_hDevVirtex5, m_fovH, m_fovV, m_forwardLines, m_rampOffset, stepSize, m_bInterleaveLines);

		ResetTrackingPos();
		break;
	case ID_SCR_GALVO:
		// galvo scanner FOV
		if (m_bInterleaveLines) {
			rampLow  = (m_rampOffset<<4) - nTemp1*m_forwardLines;
			rampHigh = (m_rampOffset<<4) + nTemp1*m_forwardLines;
		} else {
			rampLow  = (m_rampOffset<<4) - nTemp1*m_forwardLines/2;
			rampHigh = (m_rampOffset<<4) + nTemp1*m_forwardLines/2;
		}
		if (rampLow < 0 || rampHigh > 0x3fff) {
			pScrollBar->SetScrollPos(m_fovV);
			MessageBox(_T("Try to update ramp signal DC offset!"), _T("Slow Scan FOV"), MB_ICONWARNING);
			return;
		}
		msg.Format("Slow Scan FOV: %d/[0~31]", nTemp1);
		m_lblGalvo.SetWindowText(msg);
		m_fovV = nTemp1;
		g_VideoInfo.fovV = m_fovV;
//		stepSize = (int)(0.5+1.0*m_forwardLines/m_backLines);	// duty cycle off
		stepSize = m_bSymmetricRamp ? m_backLines : (int)(0.5+1.0*m_forwardLines/m_backLines);	// duty cycle off
		g_objVirtex5BMD.SetScannerParams(g_hDevVirtex5, m_fovH, m_fovV, m_forwardLines, m_rampOffset, stepSize, m_bInterleaveLines);

		ResetTrackingPos();
		break;
	case ID_SCR_GALVOFORWARD:
		// # of lines in galvo forward scanning
		if (nTemp1 < g_VideoInfo.img_height) {
			pScrollBar->SetScrollPos(m_forwardLines);
			MessageBox(_T("# of scanning lines have to be greater than image height!"), _T("Scanning lines"), MB_ICONWARNING);
			return;
		}
		msg.Format("Forward Lines: %d/[64~1000]", nTemp1);
		m_lblGalvoForward.SetWindowText(msg);
		m_forwardLines     = nTemp1;
		g_VideoInfo.fLines = nTemp1;
//		stepSize = (int)(0.5+1.0*m_forwardLines/m_backLines);	// duty cycle off
		stepSize = m_bSymmetricRamp ? m_backLines : (int)(0.5+1.0*m_forwardLines/m_backLines);	// duty cycle off
		g_objVirtex5BMD.SetScannerParams(g_hDevVirtex5, m_fovH, m_fovV, m_forwardLines, m_rampOffset, stepSize, m_bInterleaveLines);

		ResetTrackingPos();

		break;
	case ID_SCR_GALVOBACKWARD:
		// # of lines in galvo backward scanning
		msg.Format("Backward Lines: %d/[3~1000]", nTemp1);
		m_lblGalvoBackward.SetWindowText(msg);
		m_backLines = nTemp1;
		stepSize = (int)(0.5+1.0*m_forwardLines/m_backLines);	// duty cycle off
		g_VideoInfo.bStepSize = stepSize;
		stepSize = m_bSymmetricRamp ? m_backLines : (int)(0.5+1.0*m_forwardLines/m_backLines);	// duty cycle off
		g_objVirtex5BMD.SetScannerParams(g_hDevVirtex5, m_fovH, m_fovV, m_forwardLines, m_rampOffset, stepSize, m_bInterleaveLines);

		ResetTrackingPos();
		break;
	case ID_SCR_RAMPOFFSET:
		// ramp offset
		if (m_bInterleaveLines) {
			rampLow  = (nTemp1<<4) - m_fovV*m_forwardLines;
			rampHigh = (nTemp1<<4) + m_fovV*m_forwardLines;
		} else {
			rampLow  = (nTemp1<<4) - m_fovV*m_forwardLines/2;
			rampHigh = (nTemp1<<4) + m_fovV*m_forwardLines/2;
		}
		if (rampLow < 0 || rampHigh > 0x3fff) {
			pScrollBar->SetScrollPos(m_rampOffset);
			MessageBox(_T("Ramp signal gain!"), _T("Galvo Ramp Offset"), MB_ICONWARNING);
			return;
		}
		msg.Format("Ramp offset: %d/[64~768]", nTemp1);
		m_lblRampOffset.SetWindowText(msg);
		m_rampOffset = nTemp1;
		g_VideoInfo.vOffset   = m_rampOffset;
//		stepSize = (int)(0.5+1.0*m_forwardLines/m_backLines);	// duty cycle off
		stepSize = m_bSymmetricRamp ? m_backLines : (int)(0.5+1.0*m_forwardLines/m_backLines);	// duty cycle off
		g_objVirtex5BMD.SetScannerParams(g_hDevVirtex5, m_fovH, m_fovV, m_forwardLines, m_rampOffset, stepSize, m_bInterleaveLines);

		ResetTrackingPos();

		break;

	case ID_LBL_YTRACK:
		fPos = 5.0*nTemp1/80;
		msg.Format("Tracking Y: %5.3f", fPos);
		m_lblTrackY.SetWindowText(msg);
		m_iTrackY = nTemp1;
		g_objVirtex5BMD.ApplyTrackingPos(g_hDevVirtex5, m_iTrackY, m_iTrackY, m_bRotate45);
		break;
/*
	case ID_SCR_YTRACK:
		fPos = 5.0*nTemp1/80;
		msg.Format("Tracking X: %5.3f", fPos);
		m_lblTrackY.SetWindowText(msg);
		m_iTrackY = nTemp1;
		g_objVirtex5BMD.ApplyTrackingPos(g_hDevVirtex5, m_iTrackY, m_iTrackY, m_bRotate45);
		break;

	case ID_STEER_SCROLL:
		m_iSteerY = nTemp1;
		msg.Format("Steer: %3.2f deg", m_iSteerY*7.5/255);
		m_lblSteerY.SetWindowText(msg);
		g_objVirtex5BMD.ApplySteeringX(g_hDevVirtex5, nTemp1);
		break;
*/
	case ID_POWER488_SCR:
		m_nPower488 = (BYTE)(0xff*nTemp1/150);
		msg.Format("488nm Laser voltage: %3.2f V", nTemp1*1.5/150);
		m_lblPower488.SetWindowTextA(msg);
		g_objVirtex5BMD.UpdatePower488(g_hDevVirtex5, m_nPower488);
		break;
	case ID_POWER568_SCR:
		m_nPower568 = (unsigned short)(0x3fff*nTemp1/1000);
		msg.Format("568nm Laser voltage: %4.3f V", nTemp1*1.0/1000);
		m_lblPower568.SetWindowTextA(msg);
		g_objVirtex5BMD.UpdatePower568(g_hDevVirtex5, m_nPower568);

		for (i = 0; i < g_VideoInfo.img_width; i ++) m_fLaserModulation[i] = nTemp1/1000.0;
		g_objVirtex5BMD.AppLaserModulation(g_hDevVirtex5, m_fLaserModulation, g_VideoInfo.img_width);
		break;
	case ID_POWER796_SCR:
		m_nPower796 = (BYTE)(0xff*nTemp1/150);
		msg.Format("796nm Laser voltage: %3.2f V", nTemp1*1.5/150);
		m_lblPower796.SetWindowTextA(msg);
		g_objVirtex5BMD.UpdatePower796(g_hDevVirtex5, m_nPower796);
		break;
	default:
		return;
	}

	g_bDCFsaved = FALSE;			// run-time registers have been updated
	m_strTitleName = m_strAppName + m_strFileName + " *";
	GetParentOwner()->SetWindowText(m_strTitleName);

	CView::OnHScroll(nSBCode, nPos, pScrollBar);
}


void CML506View::UpdateGuiControls()
{
	int     tmp;
	UINT32  imgoffset, imgSize, width, height, reg32, regData, regData1, regData2;;
	CString msg;

	// read single registers from ADC

	imgSize = g_objVirtex5BMD.VIRTEX5_ReadReg32(g_hDevVirtex5, VIRTEX5_IMAGESIZE_OFFSET);
	// 1. image width
	width = imgSize >> 12;
	width = width << 20;
	width = width >> 20;
	msg.Format("Image Width, \t%d pixels", width-8);
	m_lblImgSizeX.SetWindowText(msg);
	if (width > 8)
		m_scrImgSizeX.SetScrollPos(width-8);
	else
		m_scrImgSizeX.SetScrollPos(1);

	// 2. image height
	height = imgSize << 20;
	height = height >> 20;
	msg.Format("Image Height, \t%d lines", height);
	m_lblImgSizeY.SetWindowText(msg);
	if (height > 0)
		m_scrImgSizeY.SetScrollPos(height);
	else
		m_scrImgSizeY.SetScrollPos(1);
	m_scrImgSizeY.EnableWindow(FALSE);

	// 3r. Brightness R (0x0B, 0x0C)
	tmp = m_adcVals[0x0B];
	tmp = tmp << 8;
	tmp = tmp + m_adcVals[0x0C];
	tmp = tmp >> 7;
	if (tmp < 0)   tmp = 0;
	if (tmp > 255) tmp = 255;
	msg.Format("%d", tmp);
	m_edtBrightnessR.SetWindowText(msg);

	// 4r. Constrast R (0x05, 0x06)
	msg.Format("%d", m_adcVals[0x05]);
	m_edtContrastR.SetWindowText(msg);

	// 3g. Brightness G (0x0D, 0x0E)
	tmp = m_adcVals[0x0D];
	tmp = tmp << 8;
	tmp = tmp + m_adcVals[0x0E];
	tmp = tmp >> 7;
	if (tmp < 0)   tmp = 0;
	if (tmp > 255) tmp = 255;
	msg.Format("%d", tmp);
	m_edtBrightnessG.SetWindowText(msg);

	// 4g. Constrast G (0x07, 0x08)
	msg.Format("%d", m_adcVals[0x07]);
	m_edtContrastG.SetWindowText(msg);

	// 3b. Brightness B (0x0F, 0x10)
	tmp = m_adcVals[0x0F];
	tmp = tmp << 8;
	tmp = tmp + m_adcVals[0x10];
	tmp = tmp >> 7;
	if (tmp < 0)   tmp = 0;
	if (tmp > 255) tmp = 255;
	msg.Format("%d", tmp);
	m_edtBrightnessB.SetWindowText(msg);

	// 4b. Constrast B (0x09, 0x0A)
	msg.Format("%d", m_adcVals[0x09]);
	m_edtContrastB.SetWindowText(msg);


	g_VideoInfo.offset_pixel = 2;
	m_scrOffsetX.SetScrollPos(g_VideoInfo.offset_pixel);
	m_scrOffsetY.SetScrollPos(15);
	regData1 = m_scrOffsetX.GetScrollPos();
	regData2 = m_scrOffsetY.GetScrollPos();
	regData  = g_objVirtex5BMD.VIRTEX5_ReadReg32(g_hDevVirtex5, VIRTEX5_IMAGEOS_OFFSET);
	regData  = regData  << 18;//14;
	regData  = regData  >> 18;//14;
	regData1 = regData1 << 14;
	regData2 = regData2 << 24;
	regData  = regData + regData1 + regData2;
	g_objVirtex5BMD.VIRTEX5_WriteReg32(g_hDevVirtex5, VIRTEX5_IMAGEOS_OFFSET, regData);

	imgoffset = g_objVirtex5BMD.VIRTEX5_ReadReg32(g_hDevVirtex5, VIRTEX5_IMAGEOS_OFFSET);

	// 5. Image OffsetX
	//tmp = imgoffset >> 24;
	tmp = imgoffset << 8;
	tmp = tmp >> 22;
	m_scrOffsetX.SetScrollPos(tmp);
	msg.Format("Image Offset X, \t%d pixels", tmp);
	m_lblOffsetX.SetWindowText(msg);

	// 6. Image OffsetY
//	tmp = imgoffset << 8;
//	tmp = tmp >> 24;
	tmp = imgoffset >> 24;
	m_scrOffsetY.SetScrollPos(tmp);
	msg.Format("Image Offset Y, \t%d lines", tmp);
	m_lblOffsetY.SetWindowText(msg);

	// 7. Hsync control
	if (m_adcVals[0x12] & BIT4) {
		m_cboHsyncI.SetCurSel(1);
		m_bHsyncI = TRUE;
	} else {
		m_cboHsyncI.SetCurSel(0);
		m_bHsyncI = FALSE;
	}

	imgoffset = g_objVirtex5BMD.VIRTEX5_ReadReg32(g_hDevVirtex5, VIRTEX5_LINECTRL_OFFSET);
	reg32 = imgoffset | BIT1;
	if (m_adcVals[0x12] & BIT3) {
		m_cboHsyncO.SetCurSel(1);
		m_bHsyncO = TRUE;
		reg32 = reg32 - BIT1;
	} else {
		m_cboHsyncO.SetCurSel(0);
		m_bHsyncO = FALSE;
	}
	g_objVirtex5BMD.VIRTEX5_WriteReg32(g_hDevVirtex5, VIRTEX5_LINECTRL_OFFSET, reg32);

	// 7. Vsync control
	if (m_adcVals[0x14] & BIT4) {
		m_cboVsyncI.SetCurSel(1);
		m_bVsyncI = TRUE;
	} else {
		m_cboVsyncI.SetCurSel(0);
		m_bVsyncI = FALSE;
	}

	reg32 = imgoffset | BIT2;
	if (m_adcVals[0x14] & BIT3) {
		m_cboVsyncO.SetCurSel(1);
		m_bVsyncO = TRUE;
		reg32 = reg32 - BIT2;
	} else {
		m_cboVsyncO.SetCurSel(0);
		m_bVsyncO = FALSE;
	}
	g_objVirtex5BMD.VIRTEX5_WriteReg32(g_hDevVirtex5, VIRTEX5_LINECTRL_OFFSET, reg32);


	// update scanner parameters
	// resonant scanner FOV
	msg.Format("Resonant FOV: %d/[0~255]", m_fovH);
	m_lblResonant.SetWindowText(msg);
	// galvo scanner FOV
	msg.Format("Slow Scan FOV: %d/[0~31]", m_fovV);
	m_lblGalvo.SetWindowText(msg);
	// # of lines in galvo forward scanning
	msg.Format("Forward Lines: %d/[64~1000]", m_forwardLines);
	m_lblGalvoForward.SetWindowText(msg);
	// # of lines in galvo backward scanning
	msg.Format("Backward Lines: %d/[3~1000]", m_backLines);
	m_lblGalvoBackward.SetWindowText(msg);
	// ramp offset
	msg.Format("Ramp offset: %d/[32~768]", m_rampOffset);
	m_lblRampOffset.SetWindowText(msg);
}


int CML506View::UpdateRuntimeRegisters()
{
	// Update ADC registers
	int  ret;

	// PLL dividor, force pixel clock to 33MHz
	m_regAddr = 0x01; m_regVal = m_adcVals[0x01];
	ret = ReadWriteI2CRegister(FALSE, 0x4c000000);
	Sleep(1);
	if (ret != 0) {
		ret = ReadWriteI2CRegister(FALSE, 0x4c000000);
		Sleep(1);
	}
	m_regAddr = 0x02; m_regVal = m_adcVals[0x02];
	ret = ReadWriteI2CRegister(FALSE, 0x4c000000);
	Sleep(1);
	if (ret != 0) {
		ret = ReadWriteI2CRegister(FALSE, 0x4c000000);
		Sleep(1);
	}

	// VCO/CPMP
	m_regAddr = 0x03; m_regVal = m_adcVals[0x03];
	ret = ReadWriteI2CRegister(FALSE, 0x4c000000);
	Sleep(1);
	if (ret != 0) {
		ret = ReadWriteI2CRegister(FALSE, 0x4c000000);
		Sleep(1);
	}

	// Phase adjustment
	m_regAddr = 0x04; m_regVal = m_adcVals[0x04];
	ret = ReadWriteI2CRegister(FALSE, 0x4c000000);
	Sleep(1);
	if (ret != 0) {
		ret = ReadWriteI2CRegister(FALSE, 0x4c000000);
		Sleep(1);
	}

	// contrast R
	m_regAddr = 0x05; m_regVal = m_adcVals[0x05];
	ret = ReadWriteI2CRegister(FALSE, 0x4c000000);
	Sleep(1);
	// brightness R
	m_regAddr = 0x0B; m_regVal = m_adcVals[0x0B];
	ret = ReadWriteI2CRegister(FALSE, 0x4c000000);
	Sleep(1);
	m_regAddr = 0x0C; m_regVal = m_adcVals[0x0C];
	ret = ReadWriteI2CRegister(FALSE, 0x4c000000);
	Sleep(1);

	// contrast G
	m_regAddr = 0x07; m_regVal = m_adcVals[0x07];
	ret = ReadWriteI2CRegister(FALSE, 0x4c000000);
	Sleep(1);
	// brightness G
	m_regAddr = 0x0D; m_regVal = m_adcVals[0x0D];
	ret = ReadWriteI2CRegister(FALSE, 0x4c000000);
	Sleep(1);
	m_regAddr = 0x0E; m_regVal = m_adcVals[0x0E];
	ret = ReadWriteI2CRegister(FALSE, 0x4c000000);
	Sleep(1);

	// contrast B
	m_regAddr = 0x09; m_regVal = m_adcVals[0x09];
	ret = ReadWriteI2CRegister(FALSE, 0x4c000000);
	Sleep(1);
	// brightness B
	m_regAddr = 0x0F; m_regVal = m_adcVals[0x0F];
	ret = ReadWriteI2CRegister(FALSE, 0x4c000000);
	Sleep(1);
	m_regAddr = 0x10; m_regVal = m_adcVals[0x10];
	ret = ReadWriteI2CRegister(FALSE, 0x4c000000);
	Sleep(1);

	// Hsync control
	m_regAddr = 0x12; m_regVal = m_adcVals[0x12];
	ret = ReadWriteI2CRegister(FALSE, 0x4c000000);
	Sleep(1);
	if (ret != 0) {
		ret = ReadWriteI2CRegister(FALSE, 0x4c000000);
		Sleep(1);
	}

	//Vsync control
	m_regAddr = 0x14; m_regVal = m_adcVals[0x14];
	ret = ReadWriteI2CRegister(FALSE, 0x4c000000);
	Sleep(1);
	if (ret != 0) {
		ret = ReadWriteI2CRegister(FALSE, 0x4c000000);
		Sleep(1);
	}

	// clamping control
	m_regAddr = 0x18; m_regVal = m_adcVals[0x18];
	ret = ReadWriteI2CRegister(FALSE, 0x4c000000);
	Sleep(1);
	if (ret != 0) {
		ret = ReadWriteI2CRegister(FALSE, 0x4c000000);
		Sleep(1);
	}

	// clamping placement
	m_regAddr = 0x19; m_regVal = m_adcVals[0x19];
	ret = ReadWriteI2CRegister(FALSE, 0x4c000000);
	Sleep(1);
	if (ret != 0) {
		ret = ReadWriteI2CRegister(FALSE, 0x4c000000);
		Sleep(1);
	}

	// clamping duration
	m_regAddr = 0x1A; m_regVal = m_adcVals[0x1A];
	ret = ReadWriteI2CRegister(FALSE, 0x4c000000);
	Sleep(1);
	if (ret != 0) {
		ret = ReadWriteI2CRegister(FALSE, 0x4c000000);
		Sleep(1);
	}

	// clamping trigger
	m_regAddr = 0x1B; m_regVal = m_adcVals[0x1B];
	ret = ReadWriteI2CRegister(FALSE, 0x4c000000);
	Sleep(1);
	if (ret != 0) {
		ret = ReadWriteI2CRegister(FALSE, 0x4c000000);
		Sleep(1);
	}

	// Update DAC registers
	// 1. set power management register to enable all DAC's.
	m_regAddr = 0x49; m_regVal = m_dacVals[14];
	ReadWriteI2CRegister(FALSE, 0x76000000);
	Sleep(1);
	// 2. set dac control register 21h[0] = '0'
	m_regAddr = 0x21; m_regVal = 0x00;
	ReadWriteI2CRegister(FALSE, 0x76000000);
	Sleep(1);
	// 3. set sense bit to '1', connection detect
	m_regAddr = 0x20; m_regVal = 0x01;
	ReadWriteI2CRegister(FALSE, 0x76000000);
	Sleep(1);
	// 4. reset sense bit to '0', connection detect
	m_regAddr = 0x20; m_regVal = 0x00;
	ReadWriteI2CRegister(FALSE, 0x76000000);
	Sleep(1);
	// 5. set dac control register 21h[0] = '0'
	m_regAddr = 0x21; m_regVal = 0x09;
	ReadWriteI2CRegister(FALSE, 0x76000000);
	// 6. read sense bit
	m_regAddr = 0x20; m_regVal = 0x01;
	ReadWriteI2CRegister(TRUE, 0x76000000);
	Sleep(1);
	m_regAddr = 0x20; m_regVal  = m_dacVals[4] | BIT0;
	// 7. set sense bit to '1'
	ReadWriteI2CRegister(FALSE, 0x76000000);
	Sleep(1);
	// 8. reset sense bit to '0'
	m_regAddr = 0x20; m_regVal  = m_regVal - BIT0;
	ReadWriteI2CRegister(FALSE, 0x76000000);
	Sleep(1);
	// clock mode
	m_regAddr = 0x1C; m_regVal = m_dacVals[0];
	ReadWriteI2CRegister(FALSE, 0x76000000);
	Sleep(1);
	// input clock
	m_regAddr = 0x1D; m_regVal = m_dacVals[1];
	ReadWriteI2CRegister(FALSE, 0x76000000);
	Sleep(1);
	// input data format
	m_regAddr = 0x1F; m_regVal = m_dacVals[3];
	ReadWriteI2CRegister(FALSE, 0x76000000);
	Sleep(1);

	return 0;
}


void CML506View::HsyncInChange()
{
	int  ret;
	int  index = m_cboHsyncI.GetCurSel();

	if ((index==1 && m_bHsyncI==FALSE) || (index==0 && m_bHsyncI==TRUE)) {
		m_bHsyncI = !m_bHsyncI;
		// update ADC register
		m_regAddr = 0x12;
		if (m_bHsyncI == TRUE) {
			m_regVal = m_adcVals[0x12] | BIT4;
		} else {
			m_regVal = m_adcVals[0x12] | BIT4;
			m_regVal = m_regVal - BIT4;
		}

		ret = ReadWriteI2CRegister(FALSE, 0x4c000000);
		Sleep(1);
		// Hsync control
		if (ret == 0) {
			g_bDCFsaved     = FALSE;
			m_adcVals[0x12] = m_regVal;
			m_strTitleName = m_strAppName + m_strFileName + " *";
			GetParentOwner()->SetWindowText(m_strTitleName);
		} else {
			MessageBox("Write ADC register", "Failed to write ADC on-chip register", MB_ICONWARNING);
			// roll back to the old value
			m_bHsyncI = !m_bHsyncI;
			if (m_bHsyncI == TRUE) {
				m_cboHsyncI.SetCurSel(1);
			} else {
				m_cboHsyncI.SetCurSel(0);
			}
		}
	}
}

void CML506View::HsyncOutChange()
{
	int  ret;
	int  index = m_cboHsyncO.GetCurSel();
	UINT32 reg32;

	if ((index==1 && m_bHsyncO==FALSE) || (index==0 && m_bHsyncO==TRUE)) {
		m_bHsyncO = !m_bHsyncO;
		// update ADC register
		m_regAddr = 0x12;
		if (m_bHsyncO == TRUE) {
			m_regVal = m_adcVals[0x12] | BIT3;
		} else {
			m_regVal = m_adcVals[0x12] | BIT3;
			m_regVal = m_regVal - BIT3;
		}

		ret = ReadWriteI2CRegister(FALSE, 0x4c000000);
		Sleep(1);
		// Hsync control
		if (ret == 0) {
			g_bDCFsaved     = FALSE;
			m_adcVals[0x12] = m_regVal;
			m_strTitleName = m_strAppName + m_strFileName + " *";
			GetParentOwner()->SetWindowText(m_strTitleName);
		} else {
			MessageBox("Write ADC register", "Failed to write ADC on-chip register", MB_ICONWARNING);
			// roll back to the old value
			m_bHsyncO = !m_bHsyncO;
			if (m_bHsyncO == TRUE) {
				m_cboHsyncO.SetCurSel(1);
			} else {
				m_cboHsyncO.SetCurSel(0);
			}
		}

		reg32 = g_objVirtex5BMD.VIRTEX5_ReadReg32(g_hDevVirtex5, VIRTEX5_LINECTRL_OFFSET);
		if (m_bHsyncO) {
			reg32 = reg32 | BIT1;
			reg32 = reg32 - BIT1;
		} else {
			reg32 = reg32 | BIT1;
		}
		g_objVirtex5BMD.VIRTEX5_WriteReg32(g_hDevVirtex5, VIRTEX5_LINECTRL_OFFSET, reg32);
	}
}

void CML506View::VsyncInChange()
{
	int  ret;
	int  index = m_cboVsyncI.GetCurSel();

	if ((index==1 && m_bVsyncI==FALSE) || (index==0 && m_bVsyncI==TRUE)) {
		m_bVsyncI = !m_bVsyncI;
		// update ADC register
		m_regAddr = 0x14;
		if (m_bVsyncI == TRUE) {
			m_regVal = m_adcVals[0x14] | BIT4;
		} else {
			m_regVal = m_adcVals[0x14] | BIT4;
			m_regVal = m_regVal - BIT4;
		}

		ret = ReadWriteI2CRegister(FALSE, 0x4c000000);
		Sleep(1);
		// Hsync control
		if (ret == 0) {
			g_bDCFsaved     = FALSE;
			m_adcVals[0x14] = m_regVal;
			m_strTitleName = m_strAppName + m_strFileName + " *";
			GetParentOwner()->SetWindowText(m_strTitleName);
		} else {
			MessageBox("Write ADC register", "Failed to write ADC on-chip register", MB_ICONWARNING);
			// roll back to the old value
			m_bVsyncI = !m_bVsyncI;
			if (m_bVsyncI == TRUE) {
				m_cboVsyncI.SetCurSel(1);
			} else {
				m_cboVsyncI.SetCurSel(0);
			}
		}
	}
}

void CML506View::VsyncOutChange()
{
	int  ret;
	int  index = m_cboVsyncO.GetCurSel();
	UINT32  reg32;

	if ((index==1 && m_bVsyncO==FALSE) || (index==0 && m_bVsyncO==TRUE)) {
		m_bVsyncO = !m_bVsyncO;
		// update ADC register
		m_regAddr = 0x14;
		if (m_bVsyncO == TRUE) {
			m_regVal = m_adcVals[0x14] | BIT3;
		} else {
			m_regVal = m_adcVals[0x14] | BIT3;
			m_regVal = m_regVal - BIT3;
		}

		ret = ReadWriteI2CRegister(FALSE, 0x4c000000);
		Sleep(1);
		// Hsync control
		if (ret == 0) {
			g_bDCFsaved     = FALSE;
			m_adcVals[0x14] = m_regVal;
			m_strTitleName = m_strAppName + m_strFileName + " *";
			GetParentOwner()->SetWindowText(m_strTitleName);
		} else {
			MessageBox("Write ADC register", "Failed to write ADC on-chip register", MB_ICONWARNING);
			// roll back to the old value
			m_bVsyncO = !m_bVsyncO;
			if (m_bVsyncO == TRUE) {
				m_cboVsyncO.SetCurSel(1);
			} else {
				m_cboVsyncO.SetCurSel(0);
			}
		}

		reg32 = g_objVirtex5BMD.VIRTEX5_ReadReg32(g_hDevVirtex5, VIRTEX5_LINECTRL_OFFSET);
		if (m_bVsyncO) {
			reg32 = reg32 | BIT2;
			reg32 = reg32 - BIT2;
		} else {
			reg32 = reg32 | BIT2;
		}
		g_objVirtex5BMD.VIRTEX5_WriteReg32(g_hDevVirtex5, VIRTEX5_LINECTRL_OFFSET, reg32);
	}
}


void CML506View::EnableGuiControls(BOOL xfgLoaded)
{
	if (xfgLoaded == TRUE) {
		m_radLines.EnableWindow(TRUE);
		m_radVsync.EnableWindow(TRUE);
		m_scrImgSizeX.EnableWindow(TRUE);
		m_scrImgSizeY.EnableWindow(TRUE);
		m_edtBrightnessR.EnableWindow(TRUE);
		m_edtContrastR.EnableWindow(TRUE);
		m_scrOffsetX.EnableWindow(TRUE);
		m_scrOffsetY.EnableWindow(TRUE);
		m_cboHsyncI.EnableWindow(TRUE);
		m_cboHsyncO.EnableWindow(TRUE);
		m_cboVsyncI.EnableWindow(TRUE);
		m_cboVsyncO.EnableWindow(TRUE);
		m_chkChannelR.EnableWindow(TRUE);
		m_chkChannelG.EnableWindow(TRUE);
		m_chkChannelB.EnableWindow(TRUE);
		m_chkChannelSW.EnableWindow(m_iVideoNum==3?TRUE:FALSE);
		m_chkDiscardBlinks.EnableWindow(TRUE);
		m_chkBidirection.EnableWindow(TRUE);
		m_scrResonant.EnableWindow(TRUE);
		m_scrGalvo.EnableWindow(TRUE);
		m_scrGalvoForward.EnableWindow(TRUE);
		m_scrGalvoBackward.EnableWindow(TRUE);
		m_scrRampOffset.EnableWindow(TRUE);
		m_chkInterleave.EnableWindow(TRUE);
		//m_chkZeroInverse.EnableWindow(TRUE);
		//m_chkSymmetricRamp.EnableWindow(TRUE);
		m_chkEnableTracking.EnableWindow(TRUE);
		m_scrTrackY.EnableWindow(TRUE);

		m_chkPower488.EnableWindow(TRUE);
		m_chkPower568.EnableWindow(TRUE);
		m_chkPower796.EnableWindow(TRUE);
		m_scrPower488.EnableWindow(TRUE);
		m_scrPower568.EnableWindow(TRUE);
		m_scrPower796.EnableWindow(TRUE);
//		m_scrTrackY.EnableWindow(TRUE);
//		m_btnCalibrateH.EnableWindow(TRUE);

		if (m_bSampleG) {
			m_edtBrightnessG.EnableWindow(TRUE);
			m_edtContrastG.EnableWindow(TRUE);
		}
		if (m_bSampleB) {
			m_edtBrightnessB.EnableWindow(TRUE);
			m_edtContrastB.EnableWindow(TRUE);
		}
//		if (m_bDualChannel)
//			m_scrBlankPixels.EnableWindow(TRUE);

	} else {
		m_radLines.EnableWindow(FALSE);
		m_radVsync.EnableWindow(FALSE);
		m_scrImgSizeX.EnableWindow(FALSE);
		m_scrImgSizeY.EnableWindow(FALSE);
		m_edtBrightnessR.EnableWindow(FALSE);
		m_edtContrastR.EnableWindow(FALSE);
		m_scrOffsetX.EnableWindow(FALSE);
		m_scrOffsetY.EnableWindow(FALSE);
		m_cboHsyncI.EnableWindow(FALSE);
		m_cboHsyncO.EnableWindow(FALSE);
		m_cboVsyncI.EnableWindow(FALSE);
		m_cboVsyncO.EnableWindow(FALSE);

		m_edtBrightnessG.EnableWindow(FALSE);
		m_edtContrastG.EnableWindow(FALSE);
		m_edtBrightnessB.EnableWindow(FALSE);
		m_edtContrastB.EnableWindow(FALSE);
		m_scrBlankPixels.EnableWindow(FALSE);
		m_chkChannelR.EnableWindow(FALSE);
		m_chkChannelG.EnableWindow(FALSE);
		m_chkChannelB.EnableWindow(FALSE);
		m_chkChannelSW.EnableWindow(FALSE);
		m_chkChannelSW.ShowWindow(SW_HIDE);
		m_chkDiscardBlinks.EnableWindow(FALSE);
		m_chkBidirection.EnableWindow(FALSE);

		m_scrResonant.EnableWindow(FALSE);
		m_scrGalvo.EnableWindow(FALSE);
		m_scrGalvoForward.EnableWindow(FALSE);
		m_scrGalvoBackward.EnableWindow(FALSE);
		m_scrRampOffset.EnableWindow(FALSE);
		m_chkInterleave.EnableWindow(FALSE);
	//	m_chkZeroInverse.EnableWindow(FALSE);
	//	m_chkSymmetricRamp.EnableWindow(FALSE);
		m_chkEnableTracking.EnableWindow(FALSE);
		m_scrTrackY.EnableWindow(FALSE);

		m_chkPower488.EnableWindow(FALSE);
		m_chkPower568.EnableWindow(FALSE);
		m_chkPower796.EnableWindow(FALSE);
		m_scrPower488.EnableWindow(FALSE);
		m_scrPower568.EnableWindow(FALSE);
		m_scrPower796.EnableWindow(FALSE);
//		m_scrTrackY.EnableWindow(FALSE);
//		m_btnCalibrateH.EnableWindow(FALSE);
	}
}


void CML506View::LoadStimulus()
{
	CML506Doc* pDoc = GetDocument();
	CString    filename;

	// open a stimulus file
	//CFileDialog fd(TRUE, NULL, NULL, NULL, "14 Bit Files (*.buf)|*.buf|Bitmaps (*.bmp)|*.bmp|Targa Files (*.tif)|*.tif|");
	CFileDialog fd(TRUE, NULL, NULL, NULL, "8-bit Bitmaps (*.bmp)|*.bmp");

	if (fd.DoModal() != IDOK) {
		if (!pDoc->m_bSymbol)
			MessageBox("No stimulus file is selected, a default 17x17 square will be used as the stimulus pattern", "Loading Stimulus", MB_ICONWARNING);

		return;
	} else {
		pDoc->m_bSymbol = TRUE;
		// get the path and file name of this stimulus file
		filename = fd.GetPathName();
		filename.TrimLeft();
		filename.TrimRight();
		pDoc->LoadSymbol(filename);
	}
}

void CML506View::CalibrateSinu() {
	if (g_bGrabStart == FALSE) {
		MessageBox("No live video is found", "Sinusoidal calibration", MB_ICONWARNING);
		return;
	}
	if (m_bDualChannel == FALSE) {
		MessageBox("Both forward and backward videos are required for sinusoidal calibration", "Sinusoidal calibration", MB_ICONWARNING);
		return;
	}

	if (m_pDlgSinu != NULL) {
		m_pDlgSinu->ShowWindow(SW_SHOW);
	} else {
		m_pDlgSinu = new CSinusoidCal(this);
		if (m_pDlgSinu->GetSafeHwnd() == 0) {
			m_pDlgSinu->Create(IDD_SINUSOIDAL_CAL, GetDesktopWindow()); // displays the dialog window
			m_pDlgSinu->ShowWindow(SW_SHOW);
		} else {
			return;
		}
	}

	m_pDlgSinu->m_nP1 = g_VideoInfo.offset_pixel;
	m_pDlgSinu->m_nP2 = g_VideoInfo.img_width;
	m_pDlgSinu->m_nP3 = (m_iShiftBS >> 1);
	m_pDlgSinu->m_nPN = m_iPLLclks;
	m_pDlgSinu->m_bCalibrationOn = TRUE;
	m_pDlgSinu->UpdateImageInfo(g_VideoInfo.img_width, g_VideoInfo.img_height);
}

void CML506View::OnInitialUpdate()
{
	CView::OnInitialUpdate();

	m_bClientCreated = TRUE;
}

void CML506View::OnUpdate(CView* pSender, LPARAM lHint, CObject* pHint)
{
	switch (lHint) {
	case USER_MESSAGE_CONNECT:
		m_scrImgSizeX.EnableWindow(FALSE);
		m_scrImgSizeY.EnableWindow(FALSE);
		m_radLines.EnableWindow(FALSE);
		m_radVsync.EnableWindow(FALSE);
	//	m_chkBidirection.EnableWindow(FALSE);
		m_chkChannelR.EnableWindow(FALSE);
		m_chkChannelG.EnableWindow(FALSE);
		m_chkChannelB.EnableWindow(FALSE);
		m_chkChannelSW.EnableWindow(m_iVideoNum==3?TRUE:FALSE);
		m_chkDiscardBlinks.EnableWindow(FALSE);
		break;
	case USER_MESSAGE_DISCONNECT:
		m_scrImgSizeX.EnableWindow(TRUE);
		m_scrImgSizeY.EnableWindow(TRUE);
		m_radLines.EnableWindow(TRUE);
		m_radVsync.EnableWindow(TRUE);
	//	m_chkBidirection.EnableWindow(TRUE);
		m_chkChannelR.EnableWindow(TRUE);
		m_chkChannelG.EnableWindow(TRUE);
		m_chkChannelB.EnableWindow(TRUE);
		m_chkChannelSW.EnableWindow(m_iVideoNum==3?TRUE:FALSE);
		m_chkChannelSW.ShowWindow(m_iVideoNum==3?TRUE:FALSE);
		m_chkDiscardBlinks.EnableWindow(TRUE);
		break;
	default:
		break;
	}
}


void CML506View::LoadVideoFolderR() {
	CFolderDlg dlg(TRUE, m_aviFolderNameR, this);
	dlg.SetTitle("Folder Browser");
	if (dlg.DoModal() == IDCANCEL) return;
	m_aviFolderNameR = dlg.GetFolderName();
	SetDlgItemText(ID_EDIT_FOLDER_R, m_aviFolderNameR);
}

void CML506View::LoadVideoFolderG() {
	CFolderDlg dlg(TRUE, m_aviFolderNameG, this);
	dlg.SetTitle("Folder Browser");
	if (dlg.DoModal() == IDCANCEL) return;
	m_aviFolderNameG = dlg.GetFolderName();
	SetDlgItemText(ID_EDIT_FOLDER_G, m_aviFolderNameG);
}

void CML506View::LoadVideoFolderB() {
	CFolderDlg dlg(TRUE, m_aviFolderNameB, this);
	dlg.SetTitle("Folder Browser");
	if (dlg.DoModal() == IDCANCEL) return;
	m_aviFolderNameB = dlg.GetFolderName();
	SetDlgItemText(ID_EDIT_FOLDER_B, m_aviFolderNameB);
}

void CML506View::VideoInFrames()
{
	m_rdoLenthInFrames.SetCheck(1);
	m_rdoLenthInSeconds.SetCheck(0);
	m_lblVideoLength.SetWindowText("Length(fra)");
	m_bVideoInSeconds = FALSE;
}

void CML506View::VideoInSeconds()
{
	m_rdoLenthInFrames.SetCheck(0);
	m_rdoLenthInSeconds.SetCheck(1);
	m_lblVideoLength.SetWindowText("Length(s)");
	m_bVideoInSeconds = TRUE;
}

void CML506View::FrameCounterInLines()
{
	m_radLines.SetCheck(1);
	m_radVsync.SetCheck(0);
	m_bFrameInLines = TRUE;
	g_objVirtex5BMD.AppSetFrameCounter(g_hDevVirtex5, m_bFrameInLines);
}

void CML506View::FrameCounterInVsync()
{
	m_radLines.SetCheck(0);
	m_radVsync.SetCheck(1);
	m_bFrameInLines = FALSE;
	g_objVirtex5BMD.AppSetFrameCounter(g_hDevVirtex5, m_bFrameInLines);
}

void CML506View::SaveLiveVideo()
{
	CString     text, folder_name, vids;
	SYSTEMTIME  tt;
	int         i, nLen, fps, fidR, fidG, fidB;
	CML506Doc*  pDoc = GetDocument();

	fps = (int)(1.0/g_VideoInfo.fFrameInterval+0.5);

	GetDlgItemText(ID_VIDEO_SAVE, text);
	if (text == _T("Save Video")) {
		g_VideoInfo.nVideoCounter = 0;

		if (g_bGrabStart == FALSE) {
			MessageBox("No live video is found.", "Save Live Video", MB_ICONWARNING);
			return;
		}
		m_edtVideoLengthR.GetWindowText(text);
		text.TrimLeft();
		text.TrimRight();
		if (text.GetLength() == 0) {
			MessageBox("Enter length of videos to be saved.", "Sample Live Video", MB_ICONWARNING);
			return;
		}
		nLen = atoi(text);
		if (nLen <= 0) {
			MessageBox("Invalid video length.", "Save Live Video", MB_ICONWARNING);
			return;
		}


//		GetSystemTime(&tt);
//		vids.Format(_T("%04d%02d%02d_%02d%02d%02d"), tt.wYear, tt.wMonth, tt.wDay, tt.wHour, tt.wMinute, tt.wSecond);

		m_edtVideoPrefixR.GetWindowText(text);
		text.Trim();
		if (text.GetLength() == 0)
			text = _T("vid");

		vids = m_aviFolderNameR + "\\" + text + "_";
		pDoc->GenerateFileID(vids, &fidR);
		vids.Format(_T("%04d"), fidR);

		m_aviFileNameR = m_aviFolderNameR + "\\" + text + "_" + vids + ".avi";

		m_edtVideoPrefixG.GetWindowText(text);
		text.Trim();
		if (text.GetLength() == 0)
			text = _T("vid");
	//	m_aviFileNameG = m_aviFolderNameG + "\\" + text + "_" + vids + ".avi";
		m_aviFileNameG = m_aviFolderNameR + "\\" + text + "_" + vids + ".avi";

		m_edtVideoPrefixB.GetWindowText(text);
		text.Trim();
		if (text.GetLength() == 0)
			text = _T("vid");
	//	m_aviFileNameB = m_aviFolderNameB + "\\" + text + "_" + vids + ".avi";
		m_aviFileNameB = m_aviFolderNameR + "\\" + text + "_" + vids + ".avi";

		//MessageBox(m_aviFileNameR+m_aviFileNameG+m_aviFileNameB);

		g_VideoInfo.nVideoLength = (m_bVideoInSeconds == TRUE) ? nLen*fps : nLen;
		g_VideoInfo.nVideoLength = (g_VideoInfo.nVideoLength > 9000) ? 9000 : g_VideoInfo.nVideoLength;
		if (m_bVideoInSeconds == TRUE)
			text.Format("%d", g_VideoInfo.nVideoLength/fps);
		else
			text.Format("%d", g_VideoInfo.nVideoLength);
		m_edtVideoLengthR.SetWindowText(text);

		if (m_bAviHandlerOnR == TRUE) m_aviHandlerR.release();
		if (m_bDualChannel) {
			nLen = g_VideoInfo.img_width*2;
		} else {
			nLen = g_VideoInfo.img_width;
		}

		CT2CA szaviFileR(_T(m_aviFileNameR));
		std::string straviFileR(szaviFileR);
		m_aviHandlerR.open(straviFileR, 0, fps, m_frameSize, false);
		if (m_aviHandlerR.isOpened() == TRUE) {
			m_bAviHandlerOnR = TRUE;
		} else {
			m_bAviHandlerOnR = FALSE;
			MessageBox("Channel-1 AVI initialization failed.", "Save Live Video", MB_ICONWARNING);
			return;
		}
		if (m_bSampleG) {
			if (m_bAviHandlerOnG == TRUE) m_aviHandlerG.release();
			CT2CA szaviFileG(_T(m_aviFileNameG));
			std::string straviFileG(szaviFileG);
			m_aviHandlerG.open(straviFileG, 0, fps, m_frameSize, false);
			if (m_aviHandlerG.isOpened() == TRUE) {
				m_bAviHandlerOnG = TRUE;
			} else {
				m_bAviHandlerOnG = FALSE;
				MessageBox("Channel-2 AVI initialization failed.", "Save Live Video", MB_ICONWARNING);
				return;
			}
		}
		if (m_bSampleB) {
			if (m_bAviHandlerOnB == TRUE) m_aviHandlerB.release();
			CT2CA szaviFileB(_T(m_aviFileNameB));
			std::string straviFileB(szaviFileB);
			m_aviHandlerB.open(straviFileB, 0, fps, m_frameSize, false);
			if (m_aviHandlerB.isOpened() == TRUE) {
				m_bAviHandlerOnB = TRUE;
			} else {
				m_bAviHandlerOnB = FALSE;
				MessageBox("Channel-3 AVI initialization failed.", "Save Live Video", MB_ICONWARNING);
				return;
			}
		}

		// open video saving handler
		g_VideoInfo.bVideoSaving = TRUE;

		m_edtVideoLengthR.EnableWindow(FALSE);
		m_edtVideoNameR.EnableWindow(FALSE);
		m_edtVideoPrefixR.EnableWindow(FALSE);
		m_btnVideoFolderR.EnableWindow(FALSE);
		m_rdoLenthInFrames.EnableWindow(FALSE);
		m_rdoLenthInSeconds.EnableWindow(FALSE);

		//SetDlgItemText(ID_VIDEO_SAVE, "Stop Saving...");
	} else {
		// close video saving handler
		if (m_bAviHandlerOnR == TRUE) {
			m_aviHandlerR.release();
			m_bAviHandlerOnR = FALSE;
		}
		if (m_bSampleG) {
			if (m_bAviHandlerOnG == TRUE) {
				m_aviHandlerG.release();
				m_bAviHandlerOnG = FALSE;
			}
		}
		if (m_bSampleB) {
			if (m_bAviHandlerOnB == TRUE) {
				m_aviHandlerB.release();
				m_bAviHandlerOnB = FALSE;
			}
		}

		m_edtVideoLengthR.EnableWindow(TRUE);
		m_edtVideoNameR.EnableWindow(TRUE);
		m_edtVideoPrefixR.EnableWindow(TRUE);
		m_btnVideoFolderR.EnableWindow(TRUE);
		m_rdoLenthInFrames.EnableWindow(TRUE);
		m_rdoLenthInSeconds.EnableWindow(TRUE);

		g_VideoInfo.bVideoSaving = FALSE;
		SetDlgItemText(ID_VIDEO_SAVE, "Save Video");
	}
}



LRESULT CML506View::OnSendMessage(WPARAM wParam, LPARAM lParam)
{
	int   nMsgID = lParam;
	int   nMsgType = wParam;
	int   i, j, idx1, idx2, sizeX;
	static int pos;
	CString msg;
	UINT32 regData, regData1, regData2;

	switch (nMsgType) {
	case USER_MESSAGE_SAVEVIDEO:
		ParseVideo();

		if (nMsgID >= g_VideoInfo.nVideoLength) {
			if (m_bAviHandlerOnR == TRUE) {
				m_aviHandlerR.release();
				m_bAviHandlerOnR = FALSE;
			}
			if (m_bSampleG) {
				if (m_bAviHandlerOnG == TRUE) {
					m_aviHandlerG.release();
					m_bAviHandlerOnG = FALSE;
				}
			}
			if (m_bSampleB) {
				if (m_bAviHandlerOnB == TRUE) {
					m_aviHandlerB.release();
					m_bAviHandlerOnB = FALSE;
				}
			}

			m_edtVideoLengthR.EnableWindow(TRUE);
			m_edtVideoNameR.EnableWindow(TRUE);
			m_edtVideoPrefixR.EnableWindow(TRUE);
			m_btnVideoFolderR.EnableWindow(TRUE);
			m_rdoLenthInFrames.EnableWindow(TRUE);
			m_rdoLenthInSeconds.EnableWindow(TRUE);

			g_VideoInfo.bVideoSaving = FALSE;
			SetDlgItemText(ID_VIDEO_SAVE, "Save Video");
		} else {
			float tempMean = mean(frameR).val[0];
			if((m_bDiscardBlinks) && (tempMean<45)){

			}else{
				
				m_aviHandlerR.write(frameR);

				if (m_bSampleG)
						m_aviHandlerG.write(frameG);

				if (m_bSampleB)
						m_aviHandlerB.write(frameB);

				msg.Format(_T("Stop %d/%d"), nMsgID, g_VideoInfo.nVideoLength);
				SetDlgItemText(ID_VIDEO_SAVE, msg);
			}
		}

		break;

	case USER_MESSAGE_NEWFRAME:
		ParseVideo();
		break;

	case USER_MESSAGE_NEWOFFSET:
		msg.Format("Image Offset X, \t%d pixels", g_VideoInfo.offset_pixel);
		m_lblOffsetX.SetWindowText(msg);
		regData1 = g_VideoInfo.offset_pixel;
		m_scrOffsetX.SetScrollPos(g_VideoInfo.offset_pixel);
		regData2 = m_scrOffsetY.GetScrollPos();
		regData  = g_objVirtex5BMD.VIRTEX5_ReadReg32(g_hDevVirtex5, VIRTEX5_IMAGEOS_OFFSET);
		regData  = regData  << 18;
		regData  = regData  >> 18;	// reserve the low 14 bits;
		regData1 = regData1 << 14;
		regData2 = regData2 << 24;
		regData  = regData + regData1 + regData2;
		g_objVirtex5BMD.VIRTEX5_WriteReg32(g_hDevVirtex5, VIRTEX5_IMAGEOS_OFFSET, regData);

		break;

	case USER_MESSAGE_FOV_CALIBRATION:
		break;

  case IGUIDE_MESSAGE_SAVE:
      SaveLiveVideo();
    break;

	default:
		break;
	}

	return 0;
}


void CML506View::UpdateBrightnessR(NMHDR* wParam, LRESULT *plr) {
	CString text;
	int     brightness;
	UINT32  regData;
	BYTE    hiByte, loByte;

	CML506Doc* pDoc = GetDocument();

	MSGFILTER * lpMsgFilter = (MSGFILTER *)wParam;

	switch (lpMsgFilter->msg) {
	case WM_KEYDOWN:{
			int data=lpMsgFilter->wParam;
			if (data == 13){
				//MessageBox("update brightness");
				m_edtBrightnessR.GetWindowText(text);
				brightness = atoi(text);
				if (brightness<0 || brightness > 255) {
					MessageBox("Enter a value between 0 and 255", "Invalid data", MB_ICONWARNING);
					return;
				}

				regData = (UINT32)(brightness);
				regData = regData << 23;
				hiByte = (BYTE)(regData >> 24);
				m_regAddr = 0x0B; m_regVal = hiByte;
				ReadWriteI2CRegister(FALSE, 0x4c000000);
				Sleep(1);
				regData = (UINT32)(brightness);
				regData = regData << 31;
				loByte = (BYTE)(regData >> 24);
				m_regAddr = 0x0C; m_regVal = loByte;
				ReadWriteI2CRegister(FALSE, 0x4c000000);
//				Sleep(1);
//				pDoc->UpdateAllViews(this, USER_MESSAGE_BRIGHTNESS, NULL);
			}
		}
		break;
	}
}

void CML506View::UpdateContrastR(NMHDR* wParam, LRESULT *plr) {
	CString text;
	int     contrast;
	UINT32  regData;
	BYTE    hiByte, loByte;

	CML506Doc* pDoc = GetDocument();

	MSGFILTER * lpMsgFilter = (MSGFILTER *)wParam;

	switch (lpMsgFilter->msg) {
	case WM_KEYDOWN:{
			int data=lpMsgFilter->wParam;
			if (data == 13){
				m_edtContrastR.GetWindowText(text);
				contrast = atoi(text);
				if (contrast<0 || contrast > 127) {
					MessageBox("Enter a value between 0 and 127", "Invalid data", MB_ICONWARNING);
					return;
				}
				m_regAddr = 0x05; m_regVal = contrast;
				ReadWriteI2CRegister(FALSE, 0x4c000000);
//				Sleep(1);
//				pDoc->UpdateAllViews(this, USER_MESSAGE_CONTRAST, NULL);
			}
		}
		break;
	}
}



void CML506View::UpdateBrightnessG(NMHDR* wParam, LRESULT *plr) {
	CString text;
	int     brightness;
	UINT32  regData;
	BYTE    hiByte, loByte;

	CML506Doc* pDoc = GetDocument();

	MSGFILTER * lpMsgFilter = (MSGFILTER *)wParam;

	switch (lpMsgFilter->msg) {
	case WM_KEYDOWN:{
			int data=lpMsgFilter->wParam;
			if (data == 13){
				//MessageBox("update brightness");
				m_edtBrightnessG.GetWindowText(text);
				brightness = atoi(text);
				if (brightness<0 || brightness > 255) {
					MessageBox("Enter a value between 0 and 255", "Invalid data", MB_ICONWARNING);
					return;
				}

				regData = (UINT32)(brightness);
				regData = regData << 23;
				hiByte = (BYTE)(regData >> 24);
				m_regAddr = 0x0D; m_regVal = hiByte;
				ReadWriteI2CRegister(FALSE, 0x4c000000);
				Sleep(1);
				regData = (UINT32)(brightness);
				regData = regData << 31;
				loByte = (BYTE)(regData >> 24);
				m_regAddr = 0x0E; m_regVal = loByte;
				ReadWriteI2CRegister(FALSE, 0x4c000000);
//				Sleep(1);
//				pDoc->UpdateAllViews(this, USER_MESSAGE_BRIGHTNESS, NULL);
			}
		}
		break;
	}
}

void CML506View::UpdateContrastG(NMHDR* wParam, LRESULT *plr) {
	CString text;
	int     contrast;
	UINT32  regData;
	BYTE    hiByte, loByte;

	CML506Doc* pDoc = GetDocument();

	MSGFILTER * lpMsgFilter = (MSGFILTER *)wParam;

	switch (lpMsgFilter->msg) {
	case WM_KEYDOWN:{
			int data=lpMsgFilter->wParam;
			if (data == 13){
				m_edtContrastG.GetWindowText(text);
				contrast = atoi(text);
				if (contrast<0 || contrast > 127) {
					MessageBox("Enter a value between 0 and 127", "Invalid data", MB_ICONWARNING);
					return;
				}
				m_regAddr = 0x07; m_regVal = contrast;
				ReadWriteI2CRegister(FALSE, 0x4c000000);
//				Sleep(1);
//				pDoc->UpdateAllViews(this, USER_MESSAGE_CONTRAST, NULL);
			}
		}
		break;
	}
}

void CML506View::UpdateBrightnessB(NMHDR* wParam, LRESULT *plr) {
	CString text;
	int     brightness;
	UINT32  regData;
	BYTE    hiByte, loByte;

	CML506Doc* pDoc = GetDocument();

	MSGFILTER * lpMsgFilter = (MSGFILTER *)wParam;

	switch (lpMsgFilter->msg) {
	case WM_KEYDOWN:{
			int data=lpMsgFilter->wParam;
			if (data == 13){
				//MessageBox("update brightness");
				m_edtBrightnessB.GetWindowText(text);
				brightness = atoi(text);
				if (brightness<0 || brightness > 255) {
					MessageBox("Enter a value between 0 and 255", "Invalid data", MB_ICONWARNING);
					return;
				}

				regData = (UINT32)(brightness);
				regData = regData << 23;
				hiByte = (BYTE)(regData >> 24);
				m_regAddr = 0x0F; m_regVal = hiByte;
				ReadWriteI2CRegister(FALSE, 0x4c000000);
				Sleep(1);
				regData = (UINT32)(brightness);
				regData = regData << 31;
				loByte = (BYTE)(regData >> 24);
				m_regAddr = 0x10; m_regVal = loByte;
				ReadWriteI2CRegister(FALSE, 0x4c000000);
//				Sleep(1);
//				pDoc->UpdateAllViews(this, USER_MESSAGE_BRIGHTNESS, NULL);
			}
		}
		break;
	}
}

void CML506View::UpdateContrastB(NMHDR* wParam, LRESULT *plr) {
	CString text;
	int     contrast;
	UINT32  regData;
	BYTE    hiByte, loByte;

	CML506Doc* pDoc = GetDocument();

	MSGFILTER * lpMsgFilter = (MSGFILTER *)wParam;

	switch (lpMsgFilter->msg) {
	case WM_KEYDOWN:{
			int data=lpMsgFilter->wParam;
			if (data == 13){
				m_edtContrastB.GetWindowText(text);
				contrast = atoi(text);
				if (contrast<0 || contrast > 127) {
					MessageBox("Enter a value between 0 and 127", "Invalid data", MB_ICONWARNING);
					return;
				}
				m_regAddr = 0x09; m_regVal = contrast;
				ReadWriteI2CRegister(FALSE, 0x4c000000);
//				Sleep(1);
//				pDoc->UpdateAllViews(this, USER_MESSAGE_CONTRAST, NULL);
			}
		}
		break;
	}
}

void CML506View::UpdatePixelClock(NMHDR* wParam, LRESULT *plr) {
	CString text;
	UINT32  pixels;
	int     val, ret;

	CML506Doc* pDoc = GetDocument();

	MSGFILTER * lpMsgFilter = (MSGFILTER *)wParam;

	switch (lpMsgFilter->msg) {
	case WM_KEYDOWN:{
			int data=lpMsgFilter->wParam;
			if (data == 13){
				m_edtPixelClock.GetWindowText(text);
				val = atoi(text);
				if (val<512 || val>4095) {
					MessageBox("Enter a value between 512 and 4095", "Invalid data", MB_ICONWARNING);
					return;
				}
				pixels = val;
				m_iPLLclks = pixels/2;

				m_regAddr = 0x01; m_regVal = (BYTE)(pixels>>4);
				ret = ReadWriteI2CRegister(FALSE, 0x4c000000);
				Sleep(1);
				pixels = pixels<<28;	// high 8 bits
				pixels = pixels>>24;	// low 4 bits
				m_regAddr = 0x02; m_regVal = (BYTE)pixels;
				ret = ReadWriteI2CRegister(FALSE, 0x4c000000);
			}
		}
		break;
	}
}

void CML506View::SampleDualChannel()
{
	if (m_chkBidirection.GetCheck() == 1) {
		g_bDesinusoid = FALSE;
		m_chkBidirection.SetCheck(0);
	} else {
		// Get a LUT file from the disk
		char     BASED_CODE szFilter[] = "Look up table files (*.lut)|*.lut|";
		CString  lutFileName;

		CFileDialog fd(TRUE,"lut",NULL, OFN_FILEMUSTEXIST | OFN_PATHMUSTEXIST | OFN_HIDEREADONLY | OFN_NOREADONLYRETURN, "Look up table files (*.lut)|*.lut||");

		if (fd.DoModal() != IDOK) {
			lutFileName =  "";
			g_bDesinusoid = FALSE;
			m_chkBidirection.SetCheck(0);
			return;
		} else {
			lutFileName = fd.GetPathName();
			int        *UnMatrixIdx;
			float      *UnMatrixVal;
			UnMatrixIdx = new int [((g_VideoInfo.img_width-8)+g_VideoInfo.img_height)*2];
			UnMatrixVal = new float [((g_VideoInfo.img_width-8)+g_VideoInfo.img_height)*2];

			if (ReadLUT(lutFileName, g_VideoInfo.img_width-8, g_VideoInfo.img_height, UnMatrixIdx, UnMatrixVal) == FALSE) {
				g_bDesinusoid = FALSE;
				m_chkBidirection.SetCheck(0);
			} else {
				g_bDesinusoid = FALSE;
				if (g_desinusoidParams.UnMatrixIdx == NULL && g_desinusoidParams.UnMatrixVal == NULL) {
					g_desinusoidParams.UnMatrixIdx = new int [((g_VideoInfo.img_width-8)+g_VideoInfo.img_height)*2];
					g_desinusoidParams.UnMatrixVal = new float [((g_VideoInfo.img_width-8)+g_VideoInfo.img_height)*2];
				}
				memcpy(g_desinusoidParams.UnMatrixIdx, UnMatrixIdx, (((g_VideoInfo.img_width-8)+g_VideoInfo.img_height)*2)*sizeof(int));
				memcpy(g_desinusoidParams.UnMatrixVal, UnMatrixVal, (((g_VideoInfo.img_width-8)+g_VideoInfo.img_height)*2)*sizeof(float));

				g_bDesinusoid = TRUE;
				m_chkBidirection.SetCheck(1);
			}

			delete [] UnMatrixIdx;
			delete [] UnMatrixVal;
		}
	}
/*	m_bDualChannel = !m_bDualChannel;

	g_objVirtex5BMD.SwitchShutter(g_hDevVirtex5, m_bDualChannel);

	if (m_bDualChannel && m_iShiftBS < 10) {
		int i;
		CString msg;
		UINT32  regData, regData1, regData2;

		// invalid sampling zone is optimized
		m_iShiftBS = m_iPLLclks - g_VideoInfo.img_width;
		msg.Format("BS offset,%d", m_iShiftBS);
		m_lblBlankPixels.SetWindowText(msg);
//		m_scrBlankPixels.SetScrollPos(m_iShiftBS);

		regData  = g_objVirtex5BMD.VIRTEX5_ReadReg32(g_hDevVirtex5, VIRTEX5_LINECTRL_OFFSET);
		regData1 = regData << 26;
		regData1 = regData1 >> 26;		// low 6 its;
		regData2 = regData >> 16;
		regData2 = regData2 << 16;		// high 16 bits;
		regData  = regData2 + (m_iShiftBS<<6) + regData1;
		g_objVirtex5BMD.VIRTEX5_WriteReg32(g_hDevVirtex5, VIRTEX5_LINECTRL_OFFSET, regData);


		GenDesinusoidLUT();
	}

	CheckDualChannel(m_bDualChannel);
	g_VideoInfo.bDualChannel = m_bDualChannel;*/

}


void CML506View::CheckDualChannel(BOOL bDual) { // Load desinusoid LUT file

	UINT32 regData, regData1, regData2;

	m_chkBidirection.SetCheck(bDual);

	regData = g_objVirtex5BMD.VIRTEX5_ReadReg32(g_hDevVirtex5, VIRTEX5_LINECTRL_OFFSET);

	if (bDual) {
//		m_scrBlankPixels.EnableWindow(TRUE);
		m_edtVideoFlagsR1.EnableWindow(TRUE);
		m_edtVideoFlagsR2.EnableWindow(TRUE);
		if (m_bSampleG) {
			m_edtVideoFlagsG1.EnableWindow(TRUE);
			m_edtVideoFlagsG2.EnableWindow(TRUE);
		}
		if (m_bSampleB) {
			m_edtVideoFlagsB1.EnableWindow(TRUE);
			m_edtVideoFlagsB2.EnableWindow(TRUE);
		}

		regData = regData | BIT30;
		m_iChannelNum = 2;
	} else {
		m_scrBlankPixels.EnableWindow(FALSE);
		m_edtVideoFlagsR1.EnableWindow(FALSE);
		m_edtVideoFlagsR2.EnableWindow(FALSE);
		m_edtVideoFlagsG1.EnableWindow(FALSE);
		m_edtVideoFlagsG2.EnableWindow(FALSE);
		m_edtVideoFlagsB1.EnableWindow(FALSE);
		m_edtVideoFlagsB2.EnableWindow(FALSE);

		regData = (regData|BIT30) - BIT30;
		m_iChannelNum = 1;
	}

	regData1 = regData << 26;
	regData1 = regData1 >> 26;		// low 6 its;
	regData2 = regData >> 16;
	regData2 = regData2 << 16;		// high 16 bits;
	regData  = regData2 + (m_iShiftBS<<6) + regData1;

	g_objVirtex5BMD.VIRTEX5_WriteReg32(g_hDevVirtex5, VIRTEX5_LINECTRL_OFFSET, regData);

	// update TLP counts
	g_VideoInfo.tlp_counts = (g_VideoInfo.img_width*m_iVideoNum*m_iChannelNum+8)/8+3;
	g_VideoInfo.nVideoChannels = m_iChannelNum;

	UpdateVidSize(FALSE);
}

void CML506View::SampleChannelG()
{
	m_bSampleG = !m_bSampleG;

	CheckChannelG(m_bSampleG, TRUE);
}


void CML506View::CheckChannelG(BOOL flagG, BOOL bFromGUI) {
	UINT32 regData;

	m_chkChannelG.SetCheck(flagG);

	regData = g_objVirtex5BMD.VIRTEX5_ReadReg32(g_hDevVirtex5, VIRTEX5_IMAGEOS_OFFSET);

	m_edtVideoLengthG.EnableWindow(FALSE);
	if (flagG) {
		m_edtVideoNameG.EnableWindow(TRUE);
		m_btnVideoFolderG.EnableWindow(TRUE);
		m_edtVideoPrefixG.EnableWindow(TRUE);
		m_edtBrightnessG.EnableWindow(TRUE);
		m_edtContrastG.EnableWindow(TRUE);
		if (m_bDualChannel) {
			m_edtVideoFlagsG1.EnableWindow(TRUE);
			m_edtVideoFlagsG2.EnableWindow(TRUE);
		}

		if (bFromGUI) m_iVideoNum ++;

		regData = regData | BIT12;

		// create and show video window for the second channel
		if (m_pDlgG == NULL) {
			m_pDlgG = new CVideoChannelG(this);
			if (m_pDlgG->GetSafeHwnd() == 0) {
				m_pDlgG->Create(IDD_DIALOG_CH2, GetDesktopWindow()); // displays the dialog window
				m_pDlgG->ShowWindow(SW_SHOW);
			}
		} else {
			m_pDlgG->ShowWindow(SW_SHOW);
		}
		UpdateVidSize(TRUE);

		if (m_iVideoNum==3) {
			m_chkChannelSW.ShowWindow(SW_SHOW);
			m_chkChannelSW.EnableWindow(TRUE);
			g_bSampleSW = m_bSampleSW = FALSE;
			m_chkChannelSW.SetCheck(m_bSampleSW);
		}

	} else {
		m_edtBrightnessG.EnableWindow(FALSE);
		m_edtContrastG.EnableWindow(FALSE);
		m_edtVideoNameG.EnableWindow(FALSE);
		m_btnVideoFolderG.EnableWindow(FALSE);
		m_edtVideoPrefixG.EnableWindow(FALSE);
		m_edtVideoFlagsG1.EnableWindow(FALSE);
		m_edtVideoFlagsG2.EnableWindow(FALSE);

		// hide video window for the second channel
		if (m_pDlgG != NULL) m_pDlgG->ShowWindow(SW_HIDE);
		if (bFromGUI) m_iVideoNum --;

		regData = (regData|BIT12) - BIT12;

		// hide switch checkbox
		m_chkChannelSW.ShowWindow(SW_HIDE);
		m_chkChannelSW.EnableWindow(FALSE);
		g_bSampleSW = m_bSampleSW = FALSE;
		m_chkChannelSW.SetCheck(m_bSampleSW);
	}

	g_objVirtex5BMD.VIRTEX5_WriteReg32(g_hDevVirtex5, VIRTEX5_IMAGEOS_OFFSET, regData);

	// update TLP counts
	g_VideoInfo.tlp_counts = (g_VideoInfo.img_width*m_iVideoNum*m_iChannelNum+8)/8+3;
	g_VideoInfo.nVideoTabs = m_iVideoNum;
}

void CML506View::SampleChannelB()
{
	m_bSampleB = !m_bSampleB;
	CheckChannelB(m_bSampleB, TRUE);

}

void CML506View::SampleChannelSW()
{
	if (!g_bSampleSW) {
		if (g_imgBuffDF == NULL) {
			g_imgBuffDF = new BYTE [g_VideoInfo.img_width*g_VideoInfo.img_height];
			g_imgBuffSD = new BYTE [g_VideoInfo.img_width*g_VideoInfo.img_height];
		}
	}

	g_bSampleSW = m_bSampleSW = !m_bSampleSW;
	m_chkChannelSW.SetCheck(m_bSampleSW);
}

void CML506View::DiscardBlinks()
{
	if(m_bDiscardBlinks){
		m_chkDiscardBlinks.SetCheck(TRUE);
	}else{
		m_chkDiscardBlinks.SetCheck(FALSE);
	}
	m_bDiscardBlinks = !m_bDiscardBlinks;
	
}

void CML506View::CheckChannelB(BOOL flagB, BOOL bFromGUI) {
	UINT32  regData;

	m_chkChannelB.SetCheck(flagB);
	m_edtVideoLengthB.EnableWindow(FALSE);

	regData = g_objVirtex5BMD.VIRTEX5_ReadReg32(g_hDevVirtex5, VIRTEX5_LINECTRL_OFFSET);

	if (flagB) {
		m_edtVideoNameB.EnableWindow(TRUE);
		m_btnVideoFolderB.EnableWindow(TRUE);
		m_edtVideoPrefixB.EnableWindow(TRUE);
		m_edtBrightnessB.EnableWindow(TRUE);
		m_edtContrastB.EnableWindow(TRUE);
		if (m_bDualChannel) {
			m_edtVideoFlagsB1.EnableWindow(TRUE);
			m_edtVideoFlagsB2.EnableWindow(TRUE);
		}

		// show video window for the third channel
		if (bFromGUI) m_iVideoNum ++;
		regData = regData | BIT31;

		if (m_pDlgB == NULL) {
			m_pDlgB = new CVideoChannelB(this);
			if (m_pDlgB->GetSafeHwnd() == 0) {
				m_pDlgB->Create(IDD_DIALOG_CH3, GetDesktopWindow()); // displays the dialog window
				m_pDlgB->ShowWindow(SW_SHOW);
			}
		} else {
			m_pDlgB->ShowWindow(SW_SHOW);
		}
		UpdateVidSize(TRUE);
		if (m_iVideoNum==3) {
			m_chkChannelSW.ShowWindow(SW_SHOW);
			m_chkChannelSW.EnableWindow(TRUE);
			g_bSampleSW = m_bSampleSW = FALSE;
			m_chkChannelSW.SetCheck(m_bSampleSW);
		}
	} else {
		m_edtContrastB.EnableWindow(FALSE);
		m_edtBrightnessB.EnableWindow(FALSE);
		m_edtVideoNameB.EnableWindow(FALSE);
		m_btnVideoFolderB.EnableWindow(FALSE);
		m_edtVideoPrefixB.EnableWindow(FALSE);
		m_edtVideoFlagsB1.EnableWindow(FALSE);
		m_edtVideoFlagsB2.EnableWindow(FALSE);

		// hide video window for the third channel
		if (m_pDlgB != NULL) m_pDlgB->ShowWindow(SW_HIDE);
		if (bFromGUI) m_iVideoNum --;
		regData = (regData|BIT31) - BIT31;

		// update switch check box
		m_chkChannelSW.ShowWindow(SW_HIDE);
		m_chkChannelSW.EnableWindow(FALSE);
		g_bSampleSW = m_bSampleSW = FALSE;
		m_chkChannelSW.SetCheck(m_bSampleSW);
	}

	g_objVirtex5BMD.VIRTEX5_WriteReg32(g_hDevVirtex5, VIRTEX5_LINECTRL_OFFSET, regData);

	// update TLP counts
	g_VideoInfo.tlp_counts = (g_VideoInfo.img_width*m_iVideoNum*m_iChannelNum+8)/8+3;
	g_VideoInfo.nVideoTabs = m_iVideoNum;
}

void CML506View::UpdateVidSize(BOOL bNewDialog) {
	int  osX, osY;
	RECT rect;

	if (bNewDialog) {
		GetWindowRect(&rect);
		osX = rect.right;
		osY = rect.top;
	} else {
		osX = -9000;
		osY = -9000;
	}

	m_pDlgR->UpdateImageInfo(g_VideoInfo.img_width*g_VideoInfo.nVideoChannels-8, g_VideoInfo.img_height, osX, osY, m_bDualChannel);
	m_frameSize = cv::Size(static_cast<int>(g_VideoInfo.img_width-8), static_cast<int>(g_VideoInfo.img_height));
	frameR = cv::Mat(g_VideoInfo.img_height, g_VideoInfo.img_width-8, CV_8UC1, m_pDlgR->m_desBuffer, 0);

	if (m_bSampleG) {
		if (bNewDialog) {
			osX += 20;
			osY += 20;
		} else {
			osX = -9000;
			osY = -9000;
		}
		m_pDlgG->UpdateImageInfo(g_VideoInfo.img_width*g_VideoInfo.nVideoChannels-8, g_VideoInfo.img_height, osX, osY, m_bDualChannel);
		frameG = cv::Mat(g_VideoInfo.img_height, g_VideoInfo.img_width-8, CV_8UC1, m_pDlgG->m_desBuffer, 0);
	}
	if (m_bSampleB) {
		if (bNewDialog) {
			osX += 20;
			osY += 20;
		} else {
			osX = -9000;
			osY = -9000;
		}
		m_pDlgB->UpdateImageInfo(g_VideoInfo.img_width*g_VideoInfo.nVideoChannels-8, g_VideoInfo.img_height, osX, osY, m_bDualChannel);
		frameB = cv::Mat(g_VideoInfo.img_height, g_VideoInfo.img_width-8, CV_8UC1, m_pDlgB->m_desBuffer, 0);
	}
/*
	}*/
}


void CML506View::ParseVideo()
{
	int    qws, idx1, idx2;
	int    i, j;

	qws = g_VideoInfo.img_width*g_VideoInfo.nVideoChannels/8-1;

	if (m_iVideoNum == 1) {
		if (m_bDualChannel) {
			memcpy(m_pDlgR->m_imgBuffer, g_VideoInfo.video_out, g_VideoInfo.img_width*g_VideoInfo.img_height*2);
		} else {
		//	memcpy(m_pDlgR->m_imgBuffer, g_VideoInfo.video_out, g_VideoInfo.img_width*g_VideoInfo.img_height);
			MUB_submatrix( &m_pDlgR->m_imgBuffer, g_VideoInfo.img_height, g_VideoInfo.img_width-8, &g_VideoInfo.video_out, g_VideoInfo.img_height, g_VideoInfo.img_width, 0, 1, 0, 1 );
		}
	} else if (m_iVideoNum == 2) {
		// red and green channel
		if (m_bSampleG) {
			for (j = 0; j < g_VideoInfo.img_height; j ++) {
				idx1 = j*g_VideoInfo.img_width*g_VideoInfo.nVideoChannels;
				idx2 = j*(g_VideoInfo.img_width-8)*g_VideoInfo.nVideoChannels;
				for (i = 0; i < qws; i ++) {
					memcpy(m_pDlgG->m_imgBuffer+idx2+i*8, g_VideoInfo.video_out+(idx1+(8*(i+1)))*2+4,  8);
					memcpy(m_pDlgR->m_imgBuffer+idx2+i*8, g_VideoInfo.video_out+(idx1+8*i)*2+12, 8);
				}
			}
			m_pDlgG->m_fFrameInterval = g_VideoInfo.fFrameInterval;
			m_pDlgG->RefreshVideo();
		// red and blue channel
		} else {
			for (j = 0; j < g_VideoInfo.img_height; j ++) {
				idx1 = j*g_VideoInfo.img_width*g_VideoInfo.nVideoChannels;
				idx2 = j*(g_VideoInfo.img_width-8)*g_VideoInfo.nVideoChannels;
				for (i = 0; i < qws; i ++) {
					memcpy(m_pDlgB->m_imgBuffer+idx2+i*8, g_VideoInfo.video_out+(idx1+(8*(i+1)))*2+4,  8);
					memcpy(m_pDlgR->m_imgBuffer+idx2+i*8, g_VideoInfo.video_out+(idx1+8*i)*2+12, 8);
				}
			}
			m_pDlgB->m_fFrameInterval = g_VideoInfo.fFrameInterval;
			m_pDlgB->RefreshVideo();
		}
	} else if (m_iVideoNum == 3) {
		// copy image to three seperate channels
		for (j = 0; j < g_VideoInfo.img_height; j ++) {
			idx1 = j*g_VideoInfo.img_width*g_VideoInfo.nVideoChannels;
				idx2 = j*(g_VideoInfo.img_width-8)*g_VideoInfo.nVideoChannels;
			for (i = 0; i < qws; i ++) {
				memcpy(m_pDlgG->m_imgBuffer+idx2+i*8, g_VideoInfo.video_out+(idx1+(8*(i+1)))*3+4,  8);
				memcpy(m_pDlgB->m_imgBuffer+idx2+i*8, g_VideoInfo.video_out+(idx1+(8*(i+1)))*3+12, 8);
				memcpy(m_pDlgR->m_imgBuffer+idx2+i*8, g_VideoInfo.video_out+(idx1+8*i)*3+20, 8);
			}
		}

		m_pDlgG->m_fFrameInterval = g_VideoInfo.fFrameInterval;
		m_pDlgB->m_fFrameInterval = g_VideoInfo.fFrameInterval;
		if (m_bSampleSW) {
			for (i=0; i<g_VideoInfo.img_width * g_VideoInfo.img_height; i++) {
				if (!m_pDlgG->m_imgBuffer[i] && !m_pDlgB->m_imgBuffer[i]) {
					g_imgBuffDF[i] = g_imgBuffSD[i] = 0;
				} else {
					g_imgBuffDF[i] = (m_pDlgG->m_imgBuffer[i] + m_pDlgB->m_imgBuffer[i])>>1;
					g_imgBuffSD[i] = floor( ((((m_pDlgG->m_imgBuffer[i] - m_pDlgB->m_imgBuffer[i])*1.) / (((m_pDlgG->m_imgBuffer[i] + m_pDlgB->m_imgBuffer[i])*1.))+1.) * 127.5 ));
				}
			}
		}
		m_pDlgG->RefreshVideo();
		m_pDlgB->RefreshVideo();
	}

	m_pDlgR->m_fFrameInterval = g_VideoInfo.fFrameInterval;
	m_pDlgR->RefreshVideo();
/*
	if (m_pDlgSinu != NULL) {
		if (m_pDlgSinu->m_bCalibrationOn) {
			memcpy(m_pDlgSinu->m_imgBuffer, m_pDlgR->m_imgBuffer, g_VideoInfo.img_width*g_VideoInfo.img_height*2);
			m_pDlgSinu->RefreshVideo();
		}
	}
	// this buffer will be used to line up two images from forward scanning and backward scanning
	if (m_bDualChannel)
		memcpy(g_VideoInfo.fsbs_buffer, m_pDlgR->m_imgBuffer, g_VideoInfo.img_width*g_VideoInfo.img_height*2);*/
}


void CML506View::GenDesinusoidLUT() {
	double    pi, fovH, fov1, fov2, scaler, pos, fi, a, b;
	int       i, j, N0, N2, N3, ND, *index;

	pi = 2*asin(1.0);
	N2 = g_VideoInfo.img_width;
	N3 = (m_iShiftBS >> 1);
	N0 = m_iPLLclks;

	fov1 = 1-cos(pi*N3/N0);
	fov2 = 1-cos(pi*(N2+N3)/N0);
	fovH = (fov2 - fov1) * 0.74 / 2;

	ND = g_VideoInfo.img_height;// 576;//g_VideoInfo.img_height;// (int)(scaler * fovH);


//	FILE *fp;
//	fp = fopen("curve.csv", "w");

	index = new int [N2];
	for (i = 0; i < N2; i ++) {
		pos = 1-cos(pi*(N3+i)/N0);				// current location
		index[i] = N2*(pos-fov1)/(fov2-fov1);	// index of pixel locations on sinusoidal space
//		fprintf(fp, "%d,%d\n", i, index[i]);
	}

	if (g_VideoInfo.DesinusoidLen != ND) {
		if (g_VideoInfo.LeftIndex  != NULL) delete [] g_VideoInfo.LeftIndex;
		if (g_VideoInfo.RightIndex != NULL) delete [] g_VideoInfo.RightIndex;
		if (g_VideoInfo.LeftWeigh  != NULL) delete [] g_VideoInfo.LeftWeigh;
		if (g_VideoInfo.RightWeigh != NULL) delete [] g_VideoInfo.RightWeigh;

		g_VideoInfo.LeftIndex  = new int [ND];
		g_VideoInfo.RightIndex = new int [ND];
		g_VideoInfo.LeftWeigh  = new double [ND];
		g_VideoInfo.RightWeigh = new double [ND];
	}

	scaler = N2*1.0/ND;
	g_VideoInfo.DesinusoidLen = ND;		// pixel number on desinusoid space
	// scale the pixel density in fast scanning to the same pixel density in slow scanning
	for (i = 0; i < ND; i ++) {
		fi = i*scaler;

		for (j = 0; j < N2; j ++) {
			if (index[j] >= fi) break;
		}
		a = (int)(fi+1) - fi;		// weigh for the left index
		b = 1.0 - a;					// weigh for the right index

		g_VideoInfo.LeftIndex[i]  = j> 0 ? j-1 : 0;
		g_VideoInfo.RightIndex[i] = j<N2 ? j : N2-1;
		g_VideoInfo.LeftWeigh[i]  = a;
		g_VideoInfo.RightWeigh[i] = b;

//		fprintf(fp, "%d, %d,%d,%f,%f\n", i, g_VideoInfo.LeftIndex[i], g_VideoInfo.RightIndex[i], g_VideoInfo.LeftWeigh[i], g_VideoInfo.RightWeigh[i]);
	}

//	fclose(fp);
}

void CML506View::InterleaveLines()
{
	BYTE    stepSize;
	int     rampLow, rampHigh;

	m_bInterleaveLines = !m_bInterleaveLines;

	if (m_bInterleaveLines) {
		rampLow  = (m_rampOffset<<4) - m_fovV*m_forwardLines;
		rampHigh = (m_rampOffset<<4) + m_fovV*m_forwardLines;
		if (rampLow < 0) {
			MessageBox(_T("Try to increase ramp signal DC offset!"), _T("Interlace"), MB_ICONWARNING);
			m_bInterleaveLines = !m_bInterleaveLines;
			return;
		}
		if (rampHigh > 0x3fff) {
			MessageBox(_T("Try to decrease ramp signal DC offset!"), _T("Interlace"), MB_ICONWARNING);
			m_bInterleaveLines = !m_bInterleaveLines;
			return;
		}
	}

	m_chkInterleave.SetCheck(m_bInterleaveLines);

//	stepSize = (int)(0.5+1.0*m_forwardLines/m_backLines);	// duty cycle off
	stepSize = m_bSymmetricRamp ? m_backLines : (int)(0.5+1.0*m_forwardLines/m_backLines);	// duty cycle off
	g_objVirtex5BMD.SetScannerParams(g_hDevVirtex5, m_fovH, m_fovV, m_forwardLines, m_rampOffset, stepSize, m_bInterleaveLines);
}

void CML506View::ZeroBackward()
{
	m_bZeroBackward = !m_bZeroBackward;

	g_VideoInfo.bZeroBackward = m_bZeroBackward;
	m_chkZeroInverse.SetCheck(m_bZeroBackward);
	g_objVirtex5BMD.UpdateSlowScanner(g_hDevVirtex5, m_bZeroBackward);
}

void CML506View::SymmetricRamp()
{
	int stepSize;

	m_bSymmetricRamp = !m_bSymmetricRamp;

	g_VideoInfo.bSymmetricRamp = m_bSymmetricRamp;
	m_chkSymmetricRamp.SetCheck(m_bSymmetricRamp);
	g_objVirtex5BMD.SetSymmetricRamp(g_hDevVirtex5, m_bSymmetricRamp);

	stepSize = m_bSymmetricRamp ? m_backLines : (int)(0.5+1.0*m_forwardLines/m_backLines);	// duty cycle off
	g_objVirtex5BMD.SetScannerParams(g_hDevVirtex5, m_fovH, m_fovV, m_forwardLines, m_rampOffset, stepSize, m_bInterleaveLines);
}

void CML506View::EnableTracking()
{
	m_bEnableTracking = !m_bEnableTracking;

	if (m_bEnableTracking) {
		m_scrTrackY.EnableWindow(TRUE);
//		m_scrTrackY.EnableWindow(TRUE);
	} else {
		m_scrTrackY.EnableWindow(FALSE);
//		m_scrTrackY.EnableWindow(FALSE);
	}
	m_chkEnableTracking.SetCheck(!m_bEnableTracking);

	g_objVirtex5BMD.EnableMirrorTracking(g_hDevVirtex5, m_bEnableTracking);
}

void CML506View::ResetTrackingPos()
{
	m_iTrackY = 0;
	m_iTrackY = 0;
	m_scrTrackY.SetScrollPos(m_iTrackY);
//	m_scrTrackY.SetScrollPos(m_iTrackY);
	m_lblTrackY.SetWindowText(_T("Tracking Y: 0.000"));
//	m_lblTrackY.SetWindowText(_T("Tracking X: 0.000"));

	g_objVirtex5BMD.ApplyTrackingPos(g_hDevVirtex5, m_iTrackY, m_iTrackY, m_bRotate45);
}

void CML506View::UpdateScannerParams()
{
	CString        msg;
	int            stepSize;

	m_fovH         = g_VideoInfo.fovH;
	m_fovV         = g_VideoInfo.fovV;
	m_forwardLines = g_VideoInfo.fLines;
	m_backLines    = (int)(0.5+1.0*g_VideoInfo.fLines/g_VideoInfo.bStepSize);
	m_rampOffset   = g_VideoInfo.vOffset;
	m_bInterleaveLines    = g_VideoInfo.bDoubleLine;

	// resonant scanner FOV
	msg.Format("Resonant FOV: %d/[0~255]", m_fovH);
	m_lblResonant.SetWindowText(msg);
	m_scrResonant.SetScrollPos(m_fovH);

	// galvo scanner FOV
	msg.Format("Slow Scan FOV: %d/[0~31]", m_fovV);
	m_lblGalvo.SetWindowText(msg);
	m_scrGalvo.SetScrollPos(m_fovV);

	// # of lines in galvo forward scanning
	msg.Format("Forward Lines: %d/[64~1000]", m_forwardLines);
	m_lblGalvoForward.SetWindowText(msg);
	m_scrGalvoForward.SetScrollPos(m_forwardLines);

	// # of lines in galvo backward scanning
	msg.Format("Backward Lines: %d/[3~1000]", m_backLines);
	m_lblGalvoBackward.SetWindowText(msg);
	m_scrGalvoBackward.SetScrollPos(m_backLines);

	// ramp offset
	msg.Format("Ramp offset: %d/[64~768]", m_rampOffset);
	m_lblRampOffset.SetWindowText(msg);
	m_scrRampOffset.SetScrollPos(m_rampOffset);

	// interleaving scanning lines
	m_chkInterleave.SetCheck(m_bInterleaveLines);

	// backward slow scanning setup
//	m_chkZeroInverse.SetCheck(m_bZeroBackward);

	// symmetric ramp signal
//	m_chkSymmetricRamp.SetCheck(m_bSymmetricRamp);

	//stepSize = (int)(0.5+1.0*m_forwardLines/m_backLines);	// duty cycle off
	stepSize = m_bSymmetricRamp ? m_backLines : (int)(0.5+1.0*m_forwardLines/m_backLines);	// duty cycle off
	g_objVirtex5BMD.SetScannerParams(g_hDevVirtex5, m_fovH, m_fovV, m_forwardLines, m_rampOffset, stepSize, m_bInterleaveLines);

//	g_objVirtex5BMD.SetSymmetricRamp(g_hDevVirtex5, m_bSymmetricRamp);
//	g_objVirtex5BMD.UpdateSlowScanner(g_hDevVirtex5, m_bZeroBackward);
}


void CML506View::ScaleRSFOV()
{
	if (!g_VideoInfo.bDesinusoid) {
		MessageBox(_T("Desinsoiding has to be turned on"), _T("Calibration"), MB_ICONWARNING);
		return;
	}
return;
	// copy reference frame
	if (m_imgCalibration != NULL) delete [] m_imgCalibration;
	m_imgCalibration = new BYTE [g_VideoInfo.img_width*g_VideoInfo.img_height*2];
	memcpy(m_imgCalibration, m_pDlgR->m_desBuffer, g_VideoInfo.img_width*g_VideoInfo.img_height*2);
	m_bCalibrate = TRUE;

//	m_btnCalibrateH.EnableWindow(FALSE);

	// run a timer function to calibration FOV
	SetTimer(m_nTimerID, 500, NULL);
}


void CML506View::OnTimer(UINT_PTR nIDEvent)
{
	while (m_bCalibrate) {
		fprintf(g_fp, "timer running\n");
		if (m_bFOVflag) {
			m_bFOVflag       = FALSE;
		} else {
			m_bFOVflag       = TRUE;
		}
	};

	KillTimer(m_nTimerID);
//	m_btnCalibrateH.EnableWindow(TRUE);

	CView::OnTimer(nIDEvent);
}




void CML506View::Power488Cal()
{
	m_bPower488 = ~m_bPower488;
	m_chkPower488.SetCheck(m_bPower488);
	g_objVirtex5BMD.CalibratePower488(g_hDevVirtex5, m_bPower488);
}


void CML506View::Power568Cal()
{
	m_bPower568 = ~m_bPower568;
	m_chkPower568.SetCheck(m_bPower568);
	g_objVirtex5BMD.CalibratePower568(g_hDevVirtex5, m_bPower568);
}


void CML506View::Power796Cal()
{
	m_bPower796 = ~m_bPower796;
	m_chkPower796.SetCheck(m_bPower796);
	g_objVirtex5BMD.CalibratePower796(g_hDevVirtex5, m_bPower796);
}

BOOL CML506View::ReadLUT(CString lutFileName, int fWidth, int fHeight, int *UnMatrixIdx, float *UnMatrixVal) {
	FILE    *fp;
	int     *LUTheader, ret;
	CString  msg;

	lutFileName.TrimLeft(' ');
	lutFileName.TrimRight(' ');
	if (lutFileName.GetLength() == 0) {
		AfxMessageBox("Error: no lookup table file name is loaded. Please go to menu [Setup]->[FFT Parameters] or [Setup]->[Desinusoid] to load a lookup table file", MB_ICONEXCLAMATION);
		return FALSE;
	}

	LUTheader = new int [2];
	fopen_s(&fp, lutFileName, "rb");
	if (fp == NULL) {
		msg.Format("Error: can't find file <%s>", lutFileName);
		AfxMessageBox(msg, MB_ICONEXCLAMATION);
		return FALSE;
	}

	// read lookup table size
	ret = fread(LUTheader, sizeof(int), 2, fp);
	if (LUTheader[0] != fWidth || LUTheader[1] != fHeight) {
		msg.Format("Error: the look up table shows different video size [%,%d] to the loaded video file [%d,%d]", LUTheader[0], LUTheader[1], fWidth, fHeight);
		AfxMessageBox(msg, MB_ICONEXCLAMATION);
		fclose(fp);
		return FALSE;
	}

	// read lookup table index
	ret = fread(UnMatrixIdx, sizeof(int), (fWidth+fHeight)*2, fp);
	if (ret != (fWidth+fHeight)*2) {
		AfxMessageBox("Error: data in this lookup table are not correct", MB_ICONEXCLAMATION);
		fclose(fp);
		return FALSE;
	}

	// read lookup table values
	ret = fread(UnMatrixVal, sizeof(float), (fWidth+fHeight)*2, fp);
	if (ret != (fWidth+fHeight)*2) {
		AfxMessageBox("Error: data in this lookup table are not correct", MB_ICONEXCLAMATION);
		fclose(fp);
		return FALSE;
	}

	fclose(fp);
	delete [] LUTheader;

	return TRUE;
}
