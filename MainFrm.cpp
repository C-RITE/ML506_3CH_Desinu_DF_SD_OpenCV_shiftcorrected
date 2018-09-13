// MainFrm.cpp : implementation of the CMainFrame class
//

#include "stdafx.h"
#include "ML506.h"
#include "MainFrm.h"
#include "LiveVideoView.h"
#include "ML506View.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

extern BOOL              g_bGrabStart;
extern BOOL              g_bDCFsaved;

/////////////////////////////////////////////////////////////////////////////
// CMainFrame

IMPLEMENT_DYNCREATE(CMainFrame, CFrameWnd)

BEGIN_MESSAGE_MAP(CMainFrame, CFrameWnd)
	//{{AFX_MSG_MAP(CMainFrame)
	ON_WM_CREATE()
	ON_WM_CLOSE()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

static UINT indicators[] =
{
	ID_SEPARATOR,           // status line indicator
	ID_INDICATOR_CAPS,
	ID_INDICATOR_NUM,
	ID_INDICATOR_SCRL,
};

/////////////////////////////////////////////////////////////////////////////
// CMainFrame construction/destruction

CMainFrame::CMainFrame()
{
	// TODO: add member initialization code here
	
}

CMainFrame::~CMainFrame()
{
}

int CMainFrame::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CFrameWnd::OnCreate(lpCreateStruct) == -1)
		return -1;
	
	if (!m_wndToolBar.CreateEx(this, TBSTYLE_FLAT, WS_CHILD | WS_VISIBLE | CBRS_TOP
		| CBRS_GRIPPER | CBRS_TOOLTIPS | CBRS_FLYBY | CBRS_SIZE_DYNAMIC) ||
		!m_wndToolBar.LoadToolBar(IDR_MAINFRAME))
	{
		TRACE0("Failed to create toolbar\n");
		return -1;      // fail to create
	}

	if (!m_wndStatusBar.Create(this) ||
		!m_wndStatusBar.SetIndicators(indicators,
		  sizeof(indicators)/sizeof(UINT)))
	{
		TRACE0("Failed to create status bar\n");
		return -1;      // fail to create
	}

	// TODO: Delete these three lines if you don't want the toolbar to
	//  be dockable
	m_wndToolBar.EnableDocking(CBRS_ALIGN_ANY);
	EnableDocking(CBRS_ALIGN_ANY);
	DockControlBar(&m_wndToolBar);

	return 0;
}

BOOL CMainFrame::PreCreateWindow(CREATESTRUCT& cs)
{
	if( !CFrameWnd::PreCreateWindow(cs) )
		return FALSE;
	// TODO: Modify the Window class or styles here by modifying
	//  the CREATESTRUCT cs

	//cs.style = WS_BORDER | WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX | WS_MAXIMIZEBOX | WS_SIZEBOX;
	cs.style = WS_BORDER | WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU;
	cs.cx = 480;
	cs.cy = 960;
	cs.x = 0;
	cs.y = 0;
	cs.lpszName = _T("Xilinx ML506 Digitizer Controller");

	return TRUE;
}

/////////////////////////////////////////////////////////////////////////////
// CMainFrame diagnostics

#ifdef _DEBUG
void CMainFrame::AssertValid() const
{
	CFrameWnd::AssertValid();
}

void CMainFrame::Dump(CDumpContext& dc) const
{
	CFrameWnd::Dump(dc);
}

#endif //_DEBUG

/////////////////////////////////////////////////////////////////////////////
// CMainFrame message handlers


void CMainFrame::OnClose() 
{

	if (g_bGrabStart == TRUE) {
		AfxMessageBox("Disconnect camera before exiting the application", MB_ICONEXCLAMATION);
		return;
	}
	
	if (g_bDCFsaved == FALSE) {
		if (AfxMessageBox("DCF file has not been saved. Do you want to save it?", MB_YESNO) == IDYES)
			return;
	}

//	AfxMessageBox("Exit: CMainFrame::OnClose()");

	CFrameWnd::OnClose();
}

void CMainFrame::SetAppName(LPCTSTR title)
{
	m_strTitle = title;
}
/*
BOOL CMainFrame::OnCreateClient(LPCREATESTRUCT lpcs, CCreateContext* pContext) 
{
	if (!m_wndSplitter.CreateStatic(this, 1, 2)) return FALSE;

	if (!m_wndSplitter.CreateView(0, 0, RUNTIME_CLASS(CML506View), CSize(450, 900), pContext) ||
		!m_wndSplitter.CreateView(0, 1, RUNTIME_CLASS(CLiveVideoView), CSize(550, 900), pContext))
	{
		m_wndSplitter.DestroyWindow();
		return FALSE;
	}
	
	return TRUE;
}
*/