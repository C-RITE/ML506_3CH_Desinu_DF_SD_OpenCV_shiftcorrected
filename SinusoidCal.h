#pragma once
#include "afxwin.h"


// CSinusoidCal dialog

class CSinusoidCal : public CDialog
{
	DECLARE_DYNAMIC(CSinusoidCal)

public:
	CSinusoidCal(CWnd* pParent = NULL);   // standard constructor
	virtual ~CSinusoidCal();

// Dialog Data
	enum { IDD = IDD_SINUSOIDAL_CAL };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
public:
	afx_msg void  OnPaint();
	afx_msg void  OnBnClickedOk();
	afx_msg void  OnBnClickedThreshold();
	virtual BOOL  OnInitDialog();

	LPBITMAPINFO  CreateDIB(int cx, int cy);
	LPBITMAPINFO  m_BitmapInfoWarp;
	HDC           m_hdc;
	void          UpdateImageInfo(int width, int height);
	void          RefreshVideo();
	void          LineupImage();

	int           m_nP0;		// shift of digital clock from actual scanner start location
	int           m_nP1;		// start location of data sampling 
	int           m_nP2;		// sampling length
	int           m_nP3;		// half length between ending point of forward scanning and starting point of backward scanning
	int           m_nPN;		// # of pixels in a half sinusoidal cycle
	CPen          m_penGreen;
	CPen         *m_pPenOld;
	POINT         m_ptLoc;

	int           m_imgWidth;
	int           m_imgHeight;
	int           m_nOffsetX;
	int           m_nOffsetY;
	BYTE         *m_imgBuffer;
	BYTE         *m_dispBuffer;
	RECT          m_clientRect;
	CButton       m_btnApply;
	CButton       m_btnOK;
	BOOL          m_bCalibrationOn;
	BOOL          m_bThreshold;
	float         m_fSparcity;
	CButton m_chkThreshold;
	CScrollBar m_scrSparcity;
	CStatic m_lblSparcity;
	afx_msg void OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
	afx_msg void OnBnClickedApply();
	afx_msg void OnLButtonDblClk(UINT nFlags, CPoint point);
	afx_msg void OnRButtonDblClk(UINT nFlags, CPoint point);
};
