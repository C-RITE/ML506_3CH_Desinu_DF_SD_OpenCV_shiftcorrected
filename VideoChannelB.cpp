// VideoChannelB.cpp : implementation file
//

#include "stdafx.h"
#include "ML506.h"
#include "VideoChannelB.h"
#include "math.h"

//extern void g_SplitImage(BYTE *imgBuffer, int width, int height);
extern void g_CalcHistogram(BYTE *imgBuffer, int width, int height, BOOL bDualCh, long *hist);
extern VIDEO_INFO        g_VideoInfo;
extern BOOL	g_bDesinusoid;
extern DesinusoidParams	 g_desinusoidParams;

extern BYTE				*g_imgBuffDF;
extern BYTE				*g_imgBuffSD;
extern BOOL				 g_bSampleSW;

// CVideoChannelB dialog

IMPLEMENT_DYNAMIC(CVideoChannelB, CDialog)

CVideoChannelB::CVideoChannelB(CWnd* pParent /*=NULL*/)
	: CDialog(CVideoChannelB::IDD, pParent)
{
	m_BitmapInfoWarp = NULL;
	m_imgBuffer      = NULL;
	m_desBuffer      = NULL;
	m_disBuffer		 = NULL;
	m_bWithBS        = FALSE;
	m_bShowHist      = FALSE;
	m_bDesinusoid    = FALSE;
	m_imgWidth       = 0;
	m_imgHeight      = 0;
	m_fFrameInterval = 1000000.0;
	m_hist           = new long [520];
	m_ptLoc.x        = -1;
	m_ptLoc.y        = -1;
	m_iOffsetY       = 35;
	m_iOffsetX       = 10;

	m_PenBlue.CreatePen(PS_SOLID, 2, RGB(0, 0, 255));
	m_PenBack.CreatePen(PS_SOLID, 2, RGB(255, 255, 255));
	m_PenGreen.CreatePen(PS_SOLID, 1, RGB(0, 191, 0));

	m_HistCordOldFS  = new POINT [256];
	m_HistCordNewFS  = new POINT [256];
	m_HistCordOldBS  = new POINT [256];
	m_HistCordNewBS  = new POINT [256];
	for (int i = 0; i < 256; i ++) {
		m_HistCordOldFS[i].x = m_HistCordNewFS[i].x = i+16;
		m_HistCordOldFS[i].y = m_HistCordNewFS[i].y = 0;
		m_HistCordOldBS[i].x = m_HistCordNewBS[i].x = 0;
		m_HistCordOldBS[i].y = m_HistCordNewBS[i].y = 0;
	}
}

CVideoChannelB::~CVideoChannelB()
{
	if(m_BitmapInfoWarp) {
		free(m_BitmapInfoWarp);
		m_BitmapInfoWarp = NULL;
	}
	if (m_imgBuffer) {
		delete [] m_imgBuffer;
		m_imgBuffer    = NULL;
	}
	if (m_desBuffer) {
		delete [] m_desBuffer;
		m_desBuffer    = NULL;
	}
	if (m_disBuffer) {
		delete [] m_disBuffer;
		m_disBuffer    = NULL;
	}
	delete [] m_hist;
	delete [] m_HistCordOldFS;
	delete [] m_HistCordNewFS;
	delete [] m_HistCordOldBS;
	delete [] m_HistCordNewBS;
}

void CVideoChannelB::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_FPS_LABLE1, m_lblFPS1);
	DDX_Control(pDX, IDC_FPS_LABLE2, m_lblFPS2);
	DDX_Control(pDX, IDC_SHOW_HIST, m_btnShowHist);
	DDX_Control(pDX, IDC_HIST_INFO, m_lblHistInfo1);
	DDX_Control(pDX, IDC_HIST_INFO2, m_lblHistInfo2);
	DDX_Control(pDX, IDC_DESINUSOID3, m_btnDesinusoid);
	DDX_Control(pDX, IDC_IMGSIZE, m_lblImageSize);
}


BEGIN_MESSAGE_MAP(CVideoChannelB, CDialog)
	ON_WM_PAINT()
	ON_BN_CLICKED(IDC_SHOW_HIST, &CVideoChannelB::OnBnClickedShowHist)
	ON_BN_CLICKED(IDC_DESINUSOID3, &CVideoChannelB::OnBnClickedDesinusoid3)
	ON_WM_LBUTTONDBLCLK()
	ON_WM_RBUTTONDBLCLK()
	ON_WM_KEYDOWN()
END_MESSAGE_MAP()


// CVideoChannelB message handlers

BOOL CVideoChannelB::OnInitDialog()
{
	CDialog::OnInitDialog();

	m_hdc = GetDC()->m_hDC;

	CFont *m_pFont = new CFont();
	m_pFont->CreatePointFont(255, _T("Arial"));
	m_lblHistInfo1.SetFont(m_pFont, TRUE);
	m_lblHistInfo2.SetFont(m_pFont, TRUE);

	return TRUE;  // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}

LPBITMAPINFO CVideoChannelB::CreateDIB(int cx, int cy)
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



void CVideoChannelB::UpdateImageInfo(int width, int height, int offsetX, int offsetY, BOOL bBSflag) {
	int sizeX, sizeY;

	if (offsetX == -9000 && offsetY == -9000) {
		RECT pos;
		GetWindowRect(&pos);
		m_nOffsetX = pos.left;
		m_nOffsetY = pos.top;
	} else {
		m_nOffsetX = offsetX;
		m_nOffsetY = offsetY;
	}


	if (width != m_imgWidth || height != m_imgHeight) {
		if(m_BitmapInfoWarp) {
			free(m_BitmapInfoWarp);
			m_BitmapInfoWarp = NULL;
		}
		if (m_imgBuffer) {
			delete [] m_imgBuffer;
			m_imgBuffer    = NULL;
		}
		if (m_desBuffer) {
			delete [] m_desBuffer;
			m_desBuffer    = NULL;
		}
		
		if (m_disBuffer) {
			delete [] m_disBuffer;
			m_disBuffer    = NULL;
		}

		m_imgWidth  = width;
		m_imgHeight = height;

		m_BitmapInfoWarp = CreateDIB(width, height);
		m_imgBuffer = new BYTE [width*height];
		m_desBuffer = new BYTE [width*height];
		m_disBuffer = new BYTE [width*height];
		ZeroMemory(m_imgBuffer,  width*height);
		ZeroMemory(m_desBuffer,  width*height);
		ZeroMemory(m_disBuffer,  width*height);
	} 

	m_bWithBS = bBSflag;

	sizeX = width + 30;
	sizeY = height + 100;
	m_lblFPS1.MoveWindow(m_iOffsetX, sizeY-50, 100, 20, 0);
	m_lblFPS2.MoveWindow(sizeX-110, sizeY-50, 100, 20, 0);
	m_btnShowHist.MoveWindow(sizeX/2-50, sizeY-50, 100, 20);
	m_btnDesinusoid.MoveWindow(sizeX/2+120, sizeY-50, 100, 20);
	m_lblImageSize.MoveWindow(sizeX/2-170, sizeY-50, 100, 20);

	if (m_bShowHist) {
		MoveWindow(m_nOffsetX, m_nOffsetY, sizeX, sizeY+260, TRUE);
		m_lblHistInfo1.ShowWindow(TRUE);
		if (m_bWithBS) {
			m_lblHistInfo2.ShowWindow(TRUE);
			m_lblHistInfo1.MoveWindow(m_iOffsetX,              0, m_imgWidth/2, m_iOffsetY, TRUE);
			m_lblHistInfo2.MoveWindow(m_iOffsetX+m_imgWidth/2, 0, m_imgWidth/2, m_iOffsetY, TRUE);
		} else {
			m_lblHistInfo2.ShowWindow(FALSE);
			m_lblHistInfo1.MoveWindow(m_iOffsetX, 0, m_imgWidth, m_iOffsetY, TRUE);
			//m_lblHistInfo2.MoveWindow(10+m_imgWidth, 10, 150, 100, TRUE);
		}
		//m_lblHistInfo1.MoveWindow(30+256, m_imgHeight+70, 100, 40, TRUE);
		//m_lblHistInfo2.MoveWindow(30+m_imgWidth/2+256, m_imgHeight+70, 100, 40, TRUE);
	} else {
		MoveWindow(m_nOffsetX, m_nOffsetY, sizeX, sizeY, TRUE);
		m_lblHistInfo1.ShowWindow(FALSE);
		m_lblHistInfo2.ShowWindow(FALSE);
	}

	if (g_VideoInfo.DesinusoidLen == 0 ||
		g_VideoInfo.LeftIndex  == NULL ||
		g_VideoInfo.RightIndex == NULL ||
		g_VideoInfo.LeftWeigh  == NULL ||
		g_VideoInfo.RightWeigh == NULL) {
		m_btnDesinusoid.EnableWindow(FALSE);
		m_btnDesinusoid.SetCheck(0);
	} else {
		m_btnDesinusoid.EnableWindow(TRUE);
	}

	OnPaint();
	Invalidate(TRUE);
}


void CVideoChannelB::OnPaint()
{
	CPaintDC dc(this); // device context for painting
	CString msg;
	float   mean;
	float   sd;
	int     width, fps;

	// add a black line to split the FS and BS image
	if (m_imgBuffer) {
/*		if (m_bDesinusoid) {
			RunDesinusoid();
			::StretchDIBits(m_hdc, m_iOffsetX, m_iOffsetY, m_imgWidth, m_imgHeight,
								0, 0, m_imgWidth, m_imgHeight, m_desBuffer,
								m_BitmapInfoWarp, NULL, SRCCOPY);
		} else {
			::StretchDIBits(m_hdc, m_iOffsetX, m_iOffsetY, m_imgWidth, m_imgHeight,
								0, 0, m_imgWidth, m_imgHeight, m_imgBuffer,
								m_BitmapInfoWarp, NULL, SRCCOPY);
		}*/
		
		if (g_bDesinusoid) {
			RunDesinusoid();
			::StretchDIBits(m_hdc, m_iOffsetX, m_iOffsetY, m_imgWidth, m_imgHeight,
				0, 0, m_imgWidth, m_imgHeight, g_bSampleSW?m_disBuffer:m_desBuffer,
								m_BitmapInfoWarp, NULL, SRCCOPY);
		} else {
			memcpy(m_desBuffer, m_imgBuffer, m_imgWidth*m_imgHeight);
			::StretchDIBits(m_hdc, m_iOffsetX, m_iOffsetY, m_imgWidth, m_imgHeight,
								0, 0, m_imgWidth, m_imgHeight, g_bSampleSW?g_imgBuffSD:m_desBuffer,
								m_BitmapInfoWarp, NULL, SRCCOPY);
		}		

		if (m_bWithBS) {
			m_PenOld   = dc.SelectObject(&m_PenGreen);
			dc.MoveTo(m_iOffsetX+m_imgWidth/2-1, m_iOffsetY);
			dc.LineTo(m_iOffsetX+m_imgWidth/2-1, m_iOffsetY+m_imgHeight);
			dc.SelectObject(m_PenOld);

			if (m_bDesinusoid) {
				msg.Format(_T("%d x %d"), g_VideoInfo.DesinusoidLen, m_imgHeight);
			} else {
				msg.Format(_T("%d x %d"), m_imgWidth/2, m_imgHeight);
			}
			width = m_imgWidth/2;
		} else {
			msg.Format(_T("%d x %d"), m_imgWidth, m_imgHeight);
			width = m_imgWidth;
		}
		m_lblImageSize.SetWindowText(msg);

		if (m_ptLoc.x>m_iOffsetX && m_ptLoc.x<m_imgWidth+m_iOffsetX && m_ptLoc.y>m_iOffsetY && m_ptLoc.y<m_imgHeight+m_iOffsetX) {
			POINT  start;

			start.x     = m_ptLoc.x-5;
			start.y     = m_ptLoc.y;
			m_PenOld   = dc.SelectObject(&m_PenGreen);
			dc.Arc(m_ptLoc.x-5, m_ptLoc.y-5, m_ptLoc.x+5, m_ptLoc.y+5, start.x, start.y, start.x, start.y);
			dc.SelectObject(m_PenOld);
		}	

		if (g_VideoInfo.frameCounter%10==0) {
			msg.Format(_T("%4.3f f/s"), 1.0/m_fFrameInterval);
			m_lblFPS1.SetWindowText(msg);
//			m_lblFPS2.SetWindowText(msg);
		}

		if (m_bShowHist) {
			MapHistCurves();

			m_clientRect.left   = m_iOffsetX;
			m_clientRect.top    = m_imgHeight + 80;
			m_clientRect.right  = m_clientRect.left + 266;
			m_clientRect.bottom = m_clientRect.top + 250;
			dc.FillSolidRect(&m_clientRect, RGB(255, 255, 255));

			ZeroMemory(m_hist, 520*sizeof(long));
			g_CalcHistogram(m_imgBuffer, m_imgWidth, m_imgHeight, m_bWithBS, m_hist);

			mean = m_hist[256]*1.0/((width-32)*m_imgHeight);
			sd   = sqrt(m_hist[257]*1.0/((width-32)*m_imgHeight-1));
			//msg.Format(_T("Mean=%d, SD=%d\nMax=%d, Min=%d"), m_hist[256], m_hist[257], m_hist[258], m_hist[259]);
			msg.Format(_T("Mean=%5.4f, SD=%5.4f, Max=%d, Min=%d"), mean, sd, m_hist[258], m_hist[259]);
			//dc.TextOut(30+256, m_imgHeight+70, msg);
			if (g_VideoInfo.frameCounter%10==0) {
				m_lblHistInfo1.SetWindowText(msg);
				m_lblHistInfo2.SetWindowText(_T(""));
			}
/*
			msg.Format(_T("Mean=%5.4f"), mean);
			dc.TextOutA(20, 20, msg);
			msg.Format(_T("SD    =%5.4f"), sd);
			dc.TextOutA(20, 35, msg);
			msg.Format(_T("Max  =%d"), m_hist[258]);
			dc.TextOutA(20, 50, msg);
			msg.Format(_T("Min   =%d"), m_hist[259]);
			dc.TextOutA(20, 65, msg);
*/
			// erase old curve
			m_PenOld = dc.SelectObject(&m_PenBack);
			dc.Polyline(m_HistCordOldFS, 256);
			dc.SelectObject(m_PenOld);
			// draw new curves
			m_PenOld = dc.SelectObject(&m_PenBlue);
			dc.Polyline(m_HistCordNewFS, 256);
			dc.SelectObject(m_PenOld);

			if (m_bWithBS) {
				m_clientRect.left  = m_iOffsetX+m_imgWidth/2;
				m_clientRect.top = m_imgHeight + 80;
				m_clientRect.right = m_clientRect.left + 266;
				m_clientRect.bottom = m_clientRect.top + 250;
				dc.FillSolidRect(&m_clientRect, RGB(255, 255, 255));

				mean = m_hist[516]*1.0/((width-32)*m_imgHeight);
				sd   = sqrt(m_hist[517]*1.0/((width-32)*m_imgHeight-1));

				//msg.Format(_T("Mean=%d, SD=%d\nMax=%d, Min=%d"), m_hist[516], m_hist[517], m_hist[518], m_hist[519]);
				msg.Format(_T("Mean=%5.4f, SD=%5.4f, Max=%d, Min=%d"), mean, sd, m_hist[518], m_hist[519]);
				//dc.TextOut(30+m_imgWidth/2+256, m_imgHeight+70, msg);
				if (g_VideoInfo.frameCounter%10==0) 
					m_lblHistInfo2.SetWindowText(msg);
/*
				msg.Format(_T("Mean=%5.4f"), mean);
				dc.TextOutA(20+width, 20, msg);
				msg.Format(_T("SD    =%5.4f"), sd);
				dc.TextOutA(20+width, 35, msg);
				msg.Format(_T("Max  =%d"), m_hist[518]);
				dc.TextOutA(20+width, 50, msg);
				msg.Format(_T("Min   =%d"), m_hist[519]);
				dc.TextOutA(20+width, 65, msg);
*/
				// erase old curve
				m_PenOld = dc.SelectObject(&m_PenBack);
				dc.Polyline(m_HistCordOldBS, 256);
				dc.SelectObject(m_PenOld);
				// draw new curves
				m_PenOld = dc.SelectObject(&m_PenBlue);
				dc.Polyline(m_HistCordNewBS, 256);
				dc.SelectObject(m_PenOld);
			}

		}
	}
}

void CVideoChannelB::RefreshVideo() {
	OnPaint();
	Invalidate(FALSE);
}

void CVideoChannelB::OnOK()
{
	// TODO: Add your specialized code here and/or call the base class

//	CDialog::OnOK();
}

void CVideoChannelB::OnCancel()
{
	// TODO: Add your specialized code here and/or call the base class

//	CDialog::OnCancel();
}

void CVideoChannelB::OnBnClickedShowHist()
{
	// TODO: Add your control notification handler code here
	m_bShowHist = !m_bShowHist;

	int sizeX, sizeY;

	RECT pos;
	GetWindowRect(&pos);

	sizeX = m_imgWidth + 30;
	sizeY = m_imgHeight + 100;
	if (m_bShowHist) {
		MoveWindow(pos.left, pos.top, sizeX, sizeY+260, TRUE);
		//m_lblHistInfo1.MoveWindow(30+256, m_imgHeight+70, 100, 40, TRUE);
		//m_lblHistInfo2.MoveWindow(30+m_imgWidth/2+256, m_imgHeight+70, 100, 60, TRUE);
		m_lblHistInfo1.ShowWindow(TRUE);
		if (m_bWithBS) {
			m_lblHistInfo2.ShowWindow(TRUE);
			m_lblHistInfo1.MoveWindow(m_iOffsetX,              0, m_imgWidth/2, m_iOffsetY, TRUE);
			m_lblHistInfo2.MoveWindow(m_iOffsetX+m_imgWidth/2, 0, m_imgWidth/2, m_iOffsetY, TRUE);
		} else {
			m_lblHistInfo1.MoveWindow(m_iOffsetX, 0, m_imgWidth, m_iOffsetY, TRUE);
			m_lblHistInfo2.ShowWindow(FALSE);
		}
	} else {
		MoveWindow(pos.left, pos.top, sizeX, sizeY, TRUE);
		m_lblHistInfo1.ShowWindow(FALSE);
		m_lblHistInfo2.ShowWindow(FALSE);
	}
}


void CVideoChannelB::MapHistCurves()
{
	int i, offset = 260;

	for (i = 0; i < 256; i ++) {
		m_HistCordOldFS[i].y = m_HistCordNewFS[i].y;
		m_HistCordNewFS[i].y = m_imgHeight + 90 + m_hist[i];
	}

	// map histogram to client area
	if (m_bWithBS) {
		for (i = 0; i < 256; i ++) {
			m_HistCordOldBS[i].x = m_HistCordNewBS[i].x = m_imgWidth/2+i+16;
			m_HistCordOldBS[i].y = m_HistCordNewBS[i].y;
			m_HistCordNewBS[i].y = m_imgHeight + 90 + m_hist[i+offset];
		}
	}
}

void CVideoChannelB::OnBnClickedDesinusoid3()
{
	m_bDesinusoid = !m_bDesinusoid;
}

void CVideoChannelB::RunDesinusoid() {
/*	int     i, j, idxSin, ll, rr, it;
	BYTE    pLeft, pRight;
	double  wL, wR;

	if (m_bDesinusoid) {

		for (j = 0; j < m_imgHeight; j ++) {
			idxSin = j*m_imgWidth;
			for (i = 0; i < g_VideoInfo.DesinusoidLen; i ++) {
				ll = g_VideoInfo.LeftIndex[i];
				rr = g_VideoInfo.RightIndex[i];
				wL = g_VideoInfo.LeftWeigh[i];
				wR = g_VideoInfo.RightWeigh[i];

				// forward scanning
				pLeft  = m_imgBuffer[idxSin+ll];
				pRight = m_imgBuffer[idxSin+rr];
				it     = (int)(pLeft*wL+pRight*wR);
				m_desBuffer[idxSin+i] = it<255 ? it : 255;

				// backward scanning
				pLeft  = m_imgBuffer[idxSin+m_imgWidth-1-ll];
				pRight = m_imgBuffer[idxSin+m_imgWidth-1-rr];
				it     = (int)(pLeft*wL+pRight*wR);
				m_desBuffer[idxSin+i+m_imgWidth/2] = it<255 ? it : 255;
			}
		}

		memcpy(g_VideoInfo.desi_buffer, m_desBuffer, m_imgWidth*m_imgHeight);
	}*/
	int     i, j, p, idx1, idx2;
	float   val1, val2, val;

	if (g_bDesinusoid) {
		// we need to do desinusoiding here
		for (j = 0; j < m_imgHeight; j ++) {
			p = j * m_imgWidth;
			for (i = 0; i < m_imgWidth; i ++) {
				idx1 = g_desinusoidParams.UnMatrixIdx[m_imgWidth-1-i];
				idx2 = g_desinusoidParams.UnMatrixIdx[2*m_imgWidth-1-i];
				val1 = g_desinusoidParams.UnMatrixVal[m_imgWidth-1-i];
				val2 = g_desinusoidParams.UnMatrixVal[2*m_imgWidth-1-i]; 
				*(m_desBuffer+p+m_imgWidth-1-i) = ((*(m_imgBuffer+p+idx1))*val1 + (*(m_imgBuffer+p+idx2))*val2);
			//	*(m_desBuffer+p+m_imgWidth-1-i) = (BYTE)val;
				g_bSampleSW?*(m_disBuffer+p+m_imgWidth-1-i) = ((*(g_imgBuffSD+p+idx1))*val1 + (*(g_imgBuffSD+p+idx2))*val2):0;
			//	*(m_disBuffer+p+m_imgWidth-1-i) = (BYTE)val;
			}
		}
	}
}

void CVideoChannelB::OnLButtonDblClk(UINT nFlags, CPoint point)
{
	m_ptLoc.x = point.x;
	m_ptLoc.y = point.y;

	CDialog::OnLButtonDblClk(nFlags, point);
}

void CVideoChannelB::OnRButtonDblClk(UINT nFlags, CPoint point)
{
	m_ptLoc.x = -1;
	m_ptLoc.y = -1;

	CDialog::OnRButtonDblClk(nFlags, point);
}

void CVideoChannelB::OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags)
{
	switch(nChar){
	case VK_LEFT:		// left arrow key
		if (m_ptLoc.x > m_iOffsetX && m_ptLoc.x < m_imgWidth+m_iOffsetX)
			m_ptLoc.x --;
		break;
	case VK_UP:		// up arrow key
		if (m_ptLoc.y > m_iOffsetY && m_ptLoc.y < m_imgHeight+m_iOffsetY)
			m_ptLoc.y --;
		break;
	case VK_RIGHT:		// right arrow key
		if (m_ptLoc.x > m_iOffsetX && m_ptLoc.x < m_imgWidth+m_iOffsetX)
			m_ptLoc.x ++;
		break;
	case VK_DOWN:		// down arrow key
		if (m_ptLoc.y > m_iOffsetY && m_ptLoc.y < m_imgHeight+m_iOffsetY)
			m_ptLoc.y ++;
		break;
	default:
		return;
	}

	CDialog::OnKeyDown(nChar, nRepCnt, nFlags);
}
