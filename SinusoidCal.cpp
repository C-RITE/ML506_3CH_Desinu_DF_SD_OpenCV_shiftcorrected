// SinusoidCal.cpp : implementation file
//

#include "stdafx.h"
#include "ML506.h"
#include "SinusoidCal.h"


// CSinusoidCal dialog

IMPLEMENT_DYNAMIC(CSinusoidCal, CDialog)

CSinusoidCal::CSinusoidCal(CWnd* pParent /*=NULL*/)
	: CDialog(CSinusoidCal::IDD, pParent)
{
	m_BitmapInfoWarp = NULL;
	m_imgBuffer      = NULL;
	m_dispBuffer     = NULL;
	m_imgWidth       = 0;
	m_imgHeight      = 0;
	m_bCalibrationOn = FALSE;
	m_bThreshold     = FALSE;
	m_fSparcity      = 0.8f;
	m_penGreen.CreatePen(PS_SOLID, 1, RGB(0, 191, 0));
}

CSinusoidCal::~CSinusoidCal()
{
	if(m_BitmapInfoWarp) {
		free(m_BitmapInfoWarp);
		m_BitmapInfoWarp = NULL;
	}
	if (m_imgBuffer) {
		delete [] m_imgBuffer;
		m_imgBuffer    = NULL;
	}
	if (m_dispBuffer) {
		delete [] m_dispBuffer;
		m_dispBuffer    = NULL;
	}
}

void CSinusoidCal::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_APPLY, m_btnApply);
	DDX_Control(pDX, IDOK, m_btnOK);
	DDX_Control(pDX, IDC_THRESHOLD, m_chkThreshold);
	DDX_Control(pDX, IDC_SPARCITY_S, m_scrSparcity);
	DDX_Control(pDX, IDC_SPARCITY_T, m_lblSparcity);
}


BEGIN_MESSAGE_MAP(CSinusoidCal, CDialog)
	ON_WM_PAINT()
	ON_BN_CLICKED(IDOK, &CSinusoidCal::OnBnClickedOk)
	ON_BN_CLICKED(IDC_THRESHOLD, &CSinusoidCal::OnBnClickedThreshold)
	ON_WM_HSCROLL()
	ON_BN_CLICKED(IDC_APPLY, &CSinusoidCal::OnBnClickedApply)
	ON_WM_LBUTTONDBLCLK()
	ON_WM_LBUTTONDOWN()
	ON_WM_RBUTTONDBLCLK()
END_MESSAGE_MAP()


// CSinusoidCal message handlers

void CSinusoidCal::OnPaint()
{
	CPaintDC dc(this); // device context for painting

	if (m_dispBuffer != NULL) {
		::StretchDIBits(m_hdc, 10, 10, m_imgWidth, m_imgHeight,
							0, 0, m_imgWidth, m_imgHeight, m_dispBuffer,
							m_BitmapInfoWarp, NULL, SRCCOPY);

		if (m_ptLoc.x>10 && m_ptLoc.x<m_imgWidth+10 && m_ptLoc.y>10 && m_ptLoc.y<m_imgHeight+10) {
			POINT  start;

			start.x     = m_ptLoc.x-5;
			start.y     = m_ptLoc.y;
			m_pPenOld = dc.SelectObject(&m_penGreen);
			dc.Arc(m_ptLoc.x-5, m_ptLoc.y-5, m_ptLoc.x+5, m_ptLoc.y+5, start.x, start.y, start.x, start.y);
			dc.SelectObject(m_pPenOld);
		}
	}
}


LPBITMAPINFO CSinusoidCal::CreateDIB(int cx, int cy)
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
	//lpBmi->bmiHeader.biBitCount = 24;
	//lpBmi->bmiHeader.biSizeImage = cx*cy*3;
	//lpBmi->bmiHeader.biCompression = BI_RGB;

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



void CSinusoidCal::UpdateImageInfo(int width, int height) {
	int sizeX, sizeY;

	if (width != m_imgWidth || height != m_imgHeight) {
		if(m_BitmapInfoWarp) {
			free(m_BitmapInfoWarp);
			m_BitmapInfoWarp = NULL;
		}
		if (m_imgBuffer) {
			delete [] m_imgBuffer;
			m_imgBuffer    = NULL;
		}
		if (m_dispBuffer) {
			delete [] m_dispBuffer;
			m_dispBuffer    = NULL;
		}

		m_imgWidth  = width;
		m_imgHeight = height;

		m_BitmapInfoWarp = CreateDIB(width, height);
		m_imgBuffer = new BYTE [width*height*2];
		ZeroMemory(m_imgBuffer, width*height*2);
		m_dispBuffer = new BYTE [width*height];
		ZeroMemory(m_dispBuffer, width*height);
	} 

	sizeX = m_imgWidth + 30;
	sizeY = height + 70;

	MoveWindow(0, 0, sizeX, sizeY, TRUE);
	m_btnApply.MoveWindow(20, sizeY-50, 100, 20, 0);
	m_btnOK.MoveWindow(sizeX-120, sizeY-50, 100, 20, 0);
	m_chkThreshold.MoveWindow(150, sizeY-50, 80, 20, 0);
	m_scrSparcity.MoveWindow(240, sizeY-50, 200, 20, 0);
	m_lblSparcity.MoveWindow(450, sizeY-50, 50, 20, 0);
}



BOOL CSinusoidCal::OnInitDialog()
{
	CDialog::OnInitDialog();

	m_hdc = GetDC()->m_hDC;
	m_scrSparcity.SetScrollRange(0, 100, TRUE);
	m_scrSparcity.SetScrollPos(80, TRUE);

	return TRUE;  // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}


void CSinusoidCal::RefreshVideo() {
	// addup fs and bs data
	LineupImage();

	OnPaint();
	Invalidate(FALSE);
}

void CSinusoidCal::OnBnClickedOk()
{
	ShowWindow(SW_HIDE);
	m_bCalibrationOn = FALSE;
}

void CSinusoidCal::LineupImage()
{
	int    i, j, idx1, idx2, byte, bMax, b1, b2;

	if (m_bThreshold) {
		bMax = 0;
		for (i = 0; i < m_imgWidth*m_imgHeight*2; i ++)
			bMax = (bMax > m_imgBuffer[i]) ? bMax : m_imgBuffer[i];

		bMax = (BYTE)(bMax * (1.0-m_fSparcity));

		for (j = 0; j < m_imgHeight; j ++) {
			idx1 = j*m_imgWidth;
			idx2 = idx1<<1;
			for (i = 0; i < m_imgWidth; i ++) {
							// fs				bs
				b1 = (m_imgBuffer[idx2+i]>=bMax) ? m_imgBuffer[idx2+i] : 0;
				b2 = (m_imgBuffer[idx2+2*m_imgWidth-1-i]>=bMax) ? m_imgBuffer[idx2+2*m_imgWidth-1-i] : 0;

				m_dispBuffer[idx1+i] = (BYTE)((b1+b2)>>1);
			}
		}
	} else {
		for (j = 0; j < m_imgHeight; j ++) {
			idx1 = j*m_imgWidth;
			idx2 = idx1<<1;
			for (i = 0; i < m_imgWidth; i ++) {
							// fs				bs
				byte = m_imgBuffer[idx2+i] + m_imgBuffer[idx2+2*m_imgWidth-1-i];

				m_dispBuffer[idx1+i] = (BYTE)(byte>>1);
			}
		}
	}
}

void CSinusoidCal::OnBnClickedThreshold()
{
	if (m_chkThreshold.GetCheck() == 1) {
		m_bThreshold = TRUE;
		m_scrSparcity.EnableWindow(TRUE);
	} else {
		m_bThreshold = FALSE;
		m_scrSparcity.EnableWindow(FALSE);
	}
}


void CSinusoidCal::OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar)
{
    int		nTemp1, nTemp2;
	int     i, nDlgCtrlID;
	CString sText;

    nTemp1 = pScrollBar->GetScrollPos();

    switch(nSBCode) {
    case SB_THUMBPOSITION:
        pScrollBar->SetScrollPos(nPos, TRUE);
		nTemp1 = nPos;
        break;
    case SB_LINEUP: // left arrow button
        nTemp2 = 1;
        if ((nTemp1 - nTemp2) > 0) {
            nTemp1 -= nTemp2;
        } else {
            nTemp1 = 1;
        }
        pScrollBar->SetScrollPos(nTemp1, TRUE);
        break;
    case SB_LINEDOWN: // right arrow button
        nTemp2 = 1;
        if ((nTemp1 + nTemp2) < 100) {
            nTemp1 += nTemp2;
        } else {
            nTemp1 = 100;
        }
        pScrollBar->SetScrollPos(nTemp1, TRUE);
        break;
    case SB_PAGEUP: // left arrow button
        nTemp2 = 10;
        if ((nTemp1 - nTemp2) > 0) {
            nTemp1 -= nTemp2;
        } else {
            nTemp1 = 1;
        }
        pScrollBar->SetScrollPos(nTemp1, TRUE);
        break;
    case SB_PAGEDOWN: // right arrow button
        nTemp2 = 10;
        if ((nTemp1 + nTemp2) < 100) {
            nTemp1 += nTemp2;
        } else {
            nTemp1 = 100;
        }
        pScrollBar->SetScrollPos(nTemp1, TRUE);
        break;
	case SB_THUMBTRACK:
        pScrollBar->SetScrollPos(nPos, TRUE);
		nTemp1 = nPos;
		break;
    } 
	
	nDlgCtrlID = pScrollBar->GetDlgCtrlID();	
	if (nDlgCtrlID == IDC_SPARCITY_S) {
		m_fSparcity = nTemp1/100.0f;
		sText.Format("%3.2f", m_fSparcity);
		m_lblSparcity.SetWindowText(sText);
	}
}

void CSinusoidCal::OnBnClickedApply()
{
	// when clock drifting, one has to guarantee the corrected image has to keep the same pixel density
	m_nP0 = m_nPN - m_nP1 - m_nP2 - m_nP3;
	if (m_nP0 < 0) {
		MessageBox("Invalid clock offset", "Apply desinusoid", MB_ICONWARNING);
		return;
	}

	CString msg;
	msg.Format("%d+%d+%d+%d=%d", m_nP0, m_nP1, m_nP2, m_nP3, m_nPN);
	MessageBox(msg);
}

void CSinusoidCal::OnLButtonDblClk(UINT nFlags, CPoint point)
{
	m_ptLoc.x = point.x;
	m_ptLoc.y = point.y;

	CDialog::OnLButtonDblClk(nFlags, point);
}

void CSinusoidCal::OnRButtonDblClk(UINT nFlags, CPoint point)
{
	m_ptLoc.x        = -1;
	m_ptLoc.y        = -1;

	CDialog::OnRButtonDblClk(nFlags, point);
}
