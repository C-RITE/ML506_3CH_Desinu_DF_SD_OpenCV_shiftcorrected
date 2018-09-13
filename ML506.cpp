// ML506.cpp : Defines the class behaviors for the application.
//

#include "stdafx.h"
#include "ML506.h"

#include "MainFrm.h"
#include "ML506Doc.h"
#include "ML506View.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


extern CVirtex5BMD       g_objVirtex5BMD;
extern WDC_DEVICE_HANDLE g_hDevVirtex5;
extern DIAG_DMA          g_dma;
extern BOOL              g_bGrabStart;

// this routine is to handle the case with abnormal shutdown of the software
// where we have to turn off FPGA video sampling
LONG AppExceptionHandler(LPEXCEPTION_POINTERS p)
{
	UINT32 regR;

	g_objVirtex5BMD.AOShutter(g_hDevVirtex5, FALSE);

	regR = g_objVirtex5BMD.VIRTEX5_ReadReg32(g_hDevVirtex5, VIRTEX5_LINECTRL_OFFSET);
	regR |= BIT0;
	regR -= BIT0;
    g_objVirtex5BMD.VIRTEX5_WriteReg32(g_hDevVirtex5, VIRTEX5_LINECTRL_OFFSET, regR);	
//	if (g_bGrabStart == TRUE) 
//		g_objVirtex5BMD.DIAG_DMAClose(g_hDevVirtex5, &g_dma);

    AfxMessageBox("Program is closed abnormally.\n");

    return EXCEPTION_EXECUTE_HANDLER;
}

/////////////////////////////////////////////////////////////////////////////
// CML506App

BEGIN_MESSAGE_MAP(CML506App, CWinApp)
	//{{AFX_MSG_MAP(CML506App)
	ON_COMMAND(ID_APP_ABOUT, OnAppAbout)
		// NOTE - the ClassWizard will add and remove mapping macros here.
		//    DO NOT EDIT what you see in these blocks of generated code!
	//}}AFX_MSG_MAP
	// Standard file based document commands
	ON_COMMAND(ID_FILE_NEW, CWinApp::OnFileNew)
	ON_COMMAND(ID_FILE_OPEN, CWinApp::OnFileOpen)
	// Standard print setup command
	ON_COMMAND(ID_FILE_PRINT_SETUP, CWinApp::OnFilePrintSetup)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CML506App construction

CML506App::CML506App()
{
	// TODO: add construction code here,
	// Place all significant initialization in InitInstance
}

/////////////////////////////////////////////////////////////////////////////
// The one and only CML506App object

CML506App theApp;

/////////////////////////////////////////////////////////////////////////////
// CML506App initialization

BOOL CML506App::InitInstance()
{
//TODO: call AfxInitRichEdit2() to initialize richedit2 library.
	AfxEnableControlContainer();

	// Standard initialization
	// If you are not using these features and wish to reduce the size
	//  of your final executable, you should remove from the following
	//  the specific initialization routines you do not need.

#ifdef _AFXDLL
	Enable3dControls();			// Call this when using MFC in a shared DLL
#else
	Enable3dControlsStatic();	// Call this when linking to MFC statically
#endif

	// open Virtex-5 device.
	DWORD   status, nCode;
	char   *msg;
	CString errMsg;

	msg = new char [256];
	g_hDevVirtex5 = NULL;

	status = g_objVirtex5BMD.VIRTEX5_LibInit();
	msg = g_objVirtex5BMD.GetErrMsg();
	if (status != WD_STATUS_SUCCESS) {
		errMsg.Format("Can't initialize device Xilinx ML-506. %s", msg);
		AfxMessageBox(errMsg, MB_OK);
		delete [] msg;
		return FALSE;
	}

	g_hDevVirtex5 = g_objVirtex5BMD.DeviceFindAndOpen(VIRTEX5_DEFAULT_VENDOR_ID, VIRTEX5_DEFAULT_DEVICE_ID);
	msg   = g_objVirtex5BMD.GetErrMsg();
	nCode = g_objVirtex5BMD.GetErrCode();
	if (nCode != WD_STATUS_SUCCESS) {
		errMsg.Format("Can't open device Xilinx ML-506. Error: %s", msg);
		AfxMessageBox(errMsg, MB_OK);

		if (g_hDevVirtex5)
			g_objVirtex5BMD.DeviceClose(g_hDevVirtex5, NULL);
 
		status = g_objVirtex5BMD.VIRTEX5_LibUninit();
		if (WD_STATUS_SUCCESS != status)
		{
			msg   = g_objVirtex5BMD.GetErrMsg();
			errMsg.Format("Failed to uninit the VIRTEX5 library: %s", msg);
			AfxMessageBox(errMsg, MB_OK);
		}
	}

	delete [] msg;

	// Change the registry key under which our settings are stored.
	// TODO: You should modify this string to be something appropriate
	// such as the name of your company or organization.
	SetRegistryKey(_T("Local AppWizard-Generated Applications"));

	LoadStdProfileSettings();  // Load standard INI file options (including MRU)

	// Register the application's document templates.  Document templates
	//  serve as the connection between documents, frame windows and views.

	CSingleDocTemplate* pDocTemplate;
	pDocTemplate = new CSingleDocTemplate(
		IDR_MAINFRAME,
		RUNTIME_CLASS(CML506Doc),
		RUNTIME_CLASS(CMainFrame),       // main SDI frame window
		RUNTIME_CLASS(CML506View));
	AddDocTemplate(pDocTemplate);

	// Parse command line for standard shell commands, DDE, file open
	CCommandLineInfo cmdInfo;
	ParseCommandLine(cmdInfo);

	// Dispatch commands specified on the command line
	if (!ProcessShellCommand(cmdInfo))
		return FALSE;

	// The one and only window has been initialized, so show and update it.
	m_pMainWnd->ShowWindow(SW_SHOW);
	m_pMainWnd->UpdateWindow();

	SetUnhandledExceptionFilter((LPTOP_LEVEL_EXCEPTION_FILTER)&AppExceptionHandler); 

	return TRUE;
}


/////////////////////////////////////////////////////////////////////////////
// CAboutDlg dialog used for App About

class CAboutDlg : public CDialog
{
public:
	CAboutDlg();

// Dialog Data
	//{{AFX_DATA(CAboutDlg)
	enum { IDD = IDD_ABOUTBOX };
	//}}AFX_DATA

	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CAboutDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	//{{AFX_MSG(CAboutDlg)
		// No message handlers
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CDialog(CAboutDlg::IDD)
{
	//{{AFX_DATA_INIT(CAboutDlg)
	//}}AFX_DATA_INIT
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CAboutDlg)
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialog)
	//{{AFX_MSG_MAP(CAboutDlg)
		// No message handlers
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

// App command to run the dialog
void CML506App::OnAppAbout()
{
	CAboutDlg aboutDlg;
	aboutDlg.DoModal();
}

/////////////////////////////////////////////////////////////////////////////
// CML506App message handlers


int CML506App::ExitInstance() 
{
	DWORD   dwStatus;
	char   *msg = NULL;
	CString errMsg;

	msg = new char [256];

	if (g_hDevVirtex5)
		g_objVirtex5BMD.DeviceClose(g_hDevVirtex5, NULL);

	dwStatus = g_objVirtex5BMD.VIRTEX5_LibUninit();
	if (WD_STATUS_SUCCESS != dwStatus)
	{
		msg   = g_objVirtex5BMD.GetErrMsg();
		errMsg.Format("virtex5_diag: Failed to uninit the VIRTEX5 library: %s", msg);
		AfxMessageBox(errMsg);
	}

	delete [] msg;
	msg = NULL;
	
//	AfxMessageBox("Exit: CML506App::ExitInstance()");

	return CWinApp::ExitInstance();
}



