// LiveVideoView.cpp : implementation file
//

#include "stdafx.h"
#include "ML506.h"
#include "LiveVideoView.h"
#include <math.h>

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

extern CVirtex5BMD       g_objVirtex5BMD;
extern WDC_DEVICE_HANDLE g_hDevVirtex5;
extern VIDEO_INFO        g_VideoInfo;
//extern CView            *g_viewVideo;
extern BOOL              g_bGrabStart;
extern POINT             g_stimulus;

/////////////////////////////////////////////////////////////////////////////
// CLiveVideoView

IMPLEMENT_DYNCREATE(CLiveVideoView, CView)

CLiveVideoView::CLiveVideoView()
{
//	g_viewVideo  = this;
	g_stimulus.x = 0;
	g_stimulus.y = 0;
	m_PenGreen.CreatePen(PS_SOLID, 1, RGB(0, 255, 0));
	m_PenBlue.CreatePen(PS_SOLID, 2, RGB(0, 0, 255));
	m_PenWhite.CreatePen(PS_SOLID, 2, RGB(255, 255, 255));
	m_FontText.CreateFont(32, 0, 0, 0, FW_NORMAL, FALSE, FALSE, 0,
							ANSI_CHARSET, OUT_DEFAULT_PRECIS,
							CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY,
							DEFAULT_PITCH | FF_MODERN, "Arial");

	m_HistCordOld = new POINT [256];
	m_HistCordNew = new POINT [256];
	for (int i = 0; i < 256; i ++) {
		m_HistCordOld[i].x = m_HistCordNew[i].x = i*2+16;
		m_HistCordOld[i].y = m_HistCordNew[i].y = 0;
	}

	m_BitmapInfoWarp = NULL;
	m_frameCounter   = 0;
}

CLiveVideoView::~CLiveVideoView()
{
	if(m_BitmapInfoWarp) free(m_BitmapInfoWarp);
	delete [] m_HistCordOld;
	delete [] m_HistCordNew;

//	AfxMessageBox("Exit: CLiveVideoView::~CLiveVideoView()");
}


BEGIN_MESSAGE_MAP(CLiveVideoView, CView)
	//{{AFX_MSG_MAP(CLiveVideoView)
	ON_WM_PAINT()
	ON_WM_LBUTTONDBLCLK()
	ON_MESSAGE(WM_MESSAGE_SEND, OnSendMessage)
	ON_WM_KEYDOWN()
	ON_WM_RBUTTONDBLCLK()
	ON_WM_SIZE()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CLiveVideoView drawing

void CLiveVideoView::OnDraw(CDC* pDC)
{
	CDocument* pDoc = GetDocument();
	// TODO: add draw code here
}

/////////////////////////////////////////////////////////////////////////////
// CLiveVideoView diagnostics

#ifdef _DEBUG
void CLiveVideoView::AssertValid() const
{
	CView::AssertValid();
}

void CLiveVideoView::Dump(CDumpContext& dc) const
{
	CView::Dump(dc);
}
#endif //_DEBUG

/////////////////////////////////////////////////////////////////////////////
// CLiveVideoView message handlers

void CLiveVideoView::OnPaint() 
{
	CPaintDC dc(this); // device context for painting
	RECT     rect;
	CString  msg1, msg2;

//	::StretchDIBits(m_hdc, 0, 0, g_VideoInfo.img_width-8, g_VideoInfo.img_height,
//					4, 0, g_VideoInfo.img_width-8, g_VideoInfo.img_height, g_VideoInfo.video_out,
//					m_BitmapInfoWarp, NULL, SRCCOPY);
	::StretchDIBits(m_hdc, 0, 0, g_VideoInfo.img_width, g_VideoInfo.img_height,
					0, 0, g_VideoInfo.img_width, g_VideoInfo.img_height, g_VideoInfo.video_out,
					m_BitmapInfoWarp, NULL, SRCCOPY);

	if (g_bGrabStart == TRUE) {
		if (g_stimulus.x > 0 && g_stimulus.y > 0) {
			CPen *pOldPen = dc.SelectObject(&m_PenGreen);
			dc.MoveTo(g_stimulus.x-8, g_stimulus.y);
			dc.LineTo(g_stimulus.x+8, g_stimulus.y);
			dc.MoveTo(g_stimulus.x, g_stimulus.y-8);
			dc.LineTo(g_stimulus.x, g_stimulus.y+8);
			dc.SelectObject(pOldPen);
		
		}

		if (g_VideoInfo.frameCounter == m_frameCounter) {
		} else {
			CalcHistogram(&m_mean_val, &m_sd);
			m_frameCounter = g_VideoInfo.frameCounter;
		}
		// erase old curve
		m_PenOld = dc.SelectObject(&m_PenWhite);
		dc.Polyline(m_HistCordOld, 256);
		dc.SelectObject(m_PenOld);
		// draw new curves
		m_PenOld = dc.SelectObject(&m_PenBlue);
		dc.Polyline(m_HistCordNew, 256);
		dc.SelectObject(m_PenOld);
		// display mean value at upper right conner
		msg1.Format("Mean = %d     ", m_mean_val);
		msg2.Format("SD = %d", m_sd);
		m_FontOld = dc.SelectObject(&m_FontText);
		dc.TextOut(384, g_VideoInfo.img_height+60, msg1);
		dc.TextOut(384, g_VideoInfo.img_height+95, msg2);

		msg1.Format("sx=%d, sy=%d, ny=%d", g_stimulus.x, g_stimulus.y, g_VideoInfo.stim_ny);
		dc.TextOut(600, g_VideoInfo.img_height+95, msg1);

		dc.SelectObject(m_FontOld);

		dc.TextOut(10, g_VideoInfo.img_height+10, m_SavingMsg);
	}

}

void CLiveVideoView::OnInitialUpdate() 
{
	CView::OnInitialUpdate();
	
	m_hdc = GetDC()->m_hDC;	

	m_BitmapInfoWarp = CreateDIB(g_VideoInfo.img_width, g_VideoInfo.img_height);
	ZeroMemory(g_VideoInfo.video_out, g_VideoInfo.img_width*g_VideoInfo.img_height);
}


LPBITMAPINFO CLiveVideoView::CreateDIB(int cx, int cy)
{
	LPBITMAPINFO lpBmi;
	int iBmiSize;

	iBmiSize = sizeof(BITMAPINFO) + sizeof(RGBQUAD) * 256;

	// Allocate memory for the bitmap info header.
	if((lpBmi = (LPBITMAPINFO)malloc(iBmiSize)) == NULL)
	{
		MessageBox("Error allocating BitmapInfo!", "Warning", MB_ICONWARNING);
		return NULL;
	}
	ZeroMemory(lpBmi, iBmiSize);

	// Initialize bitmap info header
	lpBmi->bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
	lpBmi->bmiHeader.biWidth = cx;
	lpBmi->bmiHeader.biHeight = -cy;
	lpBmi->bmiHeader.biPlanes = 1;
	lpBmi->bmiHeader.biSizeImage = 0;
	lpBmi->bmiHeader.biXPelsPerMeter = 0;
	lpBmi->bmiHeader.biYPelsPerMeter = 0;
	lpBmi->bmiHeader.biClrUsed = 0;
	lpBmi->bmiHeader.biClrImportant = 0;
	lpBmi->bmiHeader.biCompression = 0;

	// After initializing the bitmap info header we need to store some
	// more information depending on the bpp of the bitmap.
	// For the 8bpp DIB we will create a simple grayscale palette.
	for(int i = 0; i < 256; i++)
	{
		lpBmi->bmiColors[i].rgbRed		= (BYTE)i;
		lpBmi->bmiColors[i].rgbGreen	= (BYTE)i;
		lpBmi->bmiColors[i].rgbBlue		= (BYTE)i;
		lpBmi->bmiColors[i].rgbReserved	= (BYTE)0;
	}

	// Set the bpp for this DIB to 8bpp.
	lpBmi->bmiHeader.biBitCount = 8;

	return lpBmi;
}



void CLiveVideoView::OnLButtonDblClk(UINT nFlags, CPoint point) 
{
	UINT32  regRData, regWData, ctrlBits, reg, weaBIT;
	int     x1, x2;

	if (point.x >= 0 && point.x < g_VideoInfo.img_width-8 &&
		point.y >= 0 && point.y < g_VideoInfo.img_height-8) {
		g_stimulus.x = point.x;
		g_stimulus.y = point.y;

		x1 = x2 = g_VideoInfo.stim_nx/2;
		g_objVirtex5BMD.AppWriteStimAddrShift(g_hDevVirtex5, 0, g_VideoInfo.img_width, 0, 0, 0, -1, -1);
		g_objVirtex5BMD.AppWriteStimLUT(g_hDevVirtex5, 0, 0, 0, 0, g_VideoInfo.img_width, g_VideoInfo.img_height, 0, 0, 0);
		g_objVirtex5BMD.AppWriteStimAddrShift(g_hDevVirtex5, point.y-g_VideoInfo.stim_ny/2, point.y+g_VideoInfo.stim_ny/2, 0, g_VideoInfo.stim_nx, g_VideoInfo.stim_nx, -1, -1);
		g_objVirtex5BMD.AppWriteStimLUT(g_hDevVirtex5, 0, point.x, point.x, point.y-g_VideoInfo.stim_ny/2, point.y+g_VideoInfo.stim_ny/2, 0, 0, x1, x2);

		//g_objVirtex5BMD.AppWriteStimLoc(g_hDevVirtex5, point.x, point.y);
	}

	CView::OnLButtonDblClk(nFlags, point);
}

LRESULT CLiveVideoView::OnSendMessage(WPARAM wParam, LPARAM lParam) 
{
	int nMsgIDH = wParam;
	int nMsgIDL = lParam;

	if (g_bGrabStart) {
		if (nMsgIDH > 0) {
			if (nMsgIDL % 30 == 0) 
				m_SavingMsg.Format("%d/%d saved        ", nMsgIDL, nMsgIDH);
		} else {
			m_SavingMsg = _T("                        ");
		}

		OnPaint();
		Invalidate(FALSE);
	}

	return 0;
}


void CLiveVideoView::OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags) 
{
	int     xDir, yDir, x1, x2;
	UINT32  regRData, regWData, ctrlBits;

	// TODO: Add your message handler code here and/or call default
	if (g_bGrabStart == TRUE) {
		if (g_stimulus.x > 0 && g_stimulus.y > 0 &&
			g_stimulus.x < g_VideoInfo.img_width-1 && g_stimulus.y < g_VideoInfo.img_height-1) {
			switch(nChar){
			case VK_LEFT:		// left arrow key
				if ((xDir=g_stimulus.x-1) > 0)
					--g_stimulus.x;
				break;
			case VK_UP:		// up arrow key
				if ((yDir=g_stimulus.y-1) > 0)
					--g_stimulus.y;
				break;
			case VK_RIGHT:		// right arrow key
				if ((xDir=g_stimulus.x+1) < g_VideoInfo.img_width)
					++g_stimulus.x;
				break;
			case VK_DOWN:		// down arrow key
				if ((yDir=g_stimulus.y+1) < g_VideoInfo.img_height)
					++g_stimulus.y;
				break;
			default:
				return;
			}

			x1 = x2 = g_VideoInfo.stim_nx/2;
			g_objVirtex5BMD.AppWriteStimAddrShift(g_hDevVirtex5, 0, g_VideoInfo.img_width, 0, 0, 0, -1, -1);
			g_objVirtex5BMD.AppWriteStimLUT(g_hDevVirtex5, 0, 0, 0, 0, g_VideoInfo.img_width, g_VideoInfo.img_height, 0, 0, 0);
			g_objVirtex5BMD.AppWriteStimAddrShift(g_hDevVirtex5, g_stimulus.y-g_VideoInfo.stim_ny/2, g_stimulus.y+g_VideoInfo.stim_ny/2, 0, g_VideoInfo.stim_nx, g_VideoInfo.stim_nx, -1, -1);
			g_objVirtex5BMD.AppWriteStimLUT(g_hDevVirtex5, 0, g_stimulus.x, g_stimulus.x, g_stimulus.y-g_VideoInfo.stim_ny/2, g_stimulus.y+g_VideoInfo.stim_ny/2, 0, 0, x1, x2);
		}
	}
		
	CView::OnKeyDown(nChar, nRepCnt, nFlags);
}

void CLiveVideoView::OnRButtonDblClk(UINT nFlags, CPoint point) 
{
	g_stimulus.x = g_stimulus.y = 0;	

	// clear stimulus location
	g_objVirtex5BMD.AppWriteStimAddrShift(g_hDevVirtex5, 0, g_VideoInfo.img_width, 0, 0, 0, -1, -1);
	g_objVirtex5BMD.AppWriteStimLUT(g_hDevVirtex5, 0, 0, 0, 0, g_VideoInfo.img_width, g_VideoInfo.img_height, 0, 0, 0);

	CView::OnRButtonDblClk(nFlags, point);
}

void CLiveVideoView::CalcHistogram(BYTE *mean_val, int *sd)
{
	int   *hist;
	int    i, j, imax, sum_val;
	long   sum_sq;
	BYTE   mval;

	hist = new int [256];

	for (i = 0; i < 256; i ++) hist[i] = 0;
	sum_val = 0;
	for (i = 0; i < g_VideoInfo.img_width*g_VideoInfo.img_height; i ++) {
		j = g_VideoInfo.video_out[i];
		if (j < 0 || j > 255) j = 0;
		hist[j] ++;
		sum_val += g_VideoInfo.video_out[i];
	}
	// calculate mean
	*mean_val = (BYTE)(sum_val*1.0/(g_VideoInfo.img_width*g_VideoInfo.img_height));
	mval = *mean_val;

	imax = 0;
	for (i = 0; i < 256; i ++) {
		imax = hist[i] > imax ? hist[i] : imax;
	}
	sum_sq = 0;
	for (i = 0; i < g_VideoInfo.img_width*g_VideoInfo.img_height; i ++) {
		sum_sq += (g_VideoInfo.video_out[i]-mval)*(g_VideoInfo.video_out[i]-mval);
	}
	// calculate standard deviation
	*sd = (int)(sqrt(sum_sq*1.0/(g_VideoInfo.img_width*g_VideoInfo.img_height-1)));

	// map histogram to client area
	for (i = 0; i < 256; i ++) {
		m_HistCordOld[i].y = m_HistCordNew[i].y;
		m_HistCordNew[i].y = g_VideoInfo.img_height + 256*(1-1.0*hist[i]/imax) + 16;
	}

	delete [] hist;
}

void CLiveVideoView::OnUpdate(CView* pSender, LPARAM lHint, CObject* pHint) 
{
	switch (lHint) {
	case USER_MESSAGE_IMGWIDTH:
		UpdateImageWidth();
		OnPaint();
		Invalidate(TRUE);
		break;
	case USER_MESSAGE_BRIGHTNESS:
		OnPaint();
		Invalidate(TRUE);
		break;
	case USER_MESSAGE_CONTRAST:
		OnPaint();
		Invalidate(TRUE);
		break;
	default:
		break;
	}
}

void CLiveVideoView::UpdateImageWidth()
{
	if(m_BitmapInfoWarp) free(m_BitmapInfoWarp);

	m_BitmapInfoWarp = CreateDIB(g_VideoInfo.img_width, g_VideoInfo.img_height);
	ZeroMemory(g_VideoInfo.video_out, g_VideoInfo.img_width*g_VideoInfo.img_height);


}
