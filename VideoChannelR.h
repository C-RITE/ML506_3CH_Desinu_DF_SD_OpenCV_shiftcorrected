#pragma once
#include "afxwin.h"
#include "ML506Doc.h"
#include "afxcmn.h"

// CVideoChannelR dialog

class CVideoChannelR : public CDialog
{
	DECLARE_DYNAMIC(CVideoChannelR)


public:
	CVideoChannelR(CWnd* pParent = NULL);   // standard constructor
	virtual ~CVideoChannelR();

	LPBITMAPINFO  CreateDIB(int cx, int cy);
	LPBITMAPINFO  m_BitmapInfoWarp;
	HDC           m_hdc;
	void          UpdateImageInfo(int width, int height, int m_nOffsetX, int m_nOffsetY, BOOL bBSflag);
	void          RunDesinusoid();
	void          RefreshVideo();
	int           m_imgWidth;
	int           m_imgHeight;
	int           m_nOffsetX;
	int           m_nOffsetY;
	BOOL          m_bWithBS;
	BOOL          m_bShowHist;
	BOOL          m_bDesinusoid;
	BYTE         *m_imgBuffer;
	BYTE         *m_desBuffer;
	double        m_fFrameInterval;
	long         *m_hist;
	POINT        *m_HistCordOldFS;
	POINT        *m_HistCordNewFS;
	POINT        *m_HistCordOldBS;
	POINT        *m_HistCordNewBS;
	CPen          m_PenBlue;
	CPen         *m_PenOld;
	CPen          m_PenBack;
	RECT          m_clientRect;
	CPen          m_PenGreen;
	POINT         m_ptLoc;
	BYTE          m_iOffsetY;
	BYTE          m_iOffsetX;

// Dialog Data
	enum { IDD = IDD_DIALOG_CH1 };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
public:
	virtual BOOL OnInitDialog();
	afx_msg void OnPaint();
protected:
	virtual void OnCancel();
	virtual void OnOK();
public:
	CButton m_btnShowHist;
	CStatic m_lblFPS1;
	CStatic m_lblFPS2;
	afx_msg void OnBnClickedShowHist();
	void    MapHistCurves();
	CStatic m_lblHistInfo1;
	CStatic m_lblHistInfo2;
	CButton m_btnDesinusoid;
	afx_msg void OnBnClickedDesinusoid();
	afx_msg void OnLButtonDblClk(UINT nFlags, CPoint point);
	afx_msg void OnRButtonDblClk(UINT nFlags, CPoint point);
	afx_msg void OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags);
	CStatic m_lblImageSize;
	afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
	CRichEditCtrl m_edtMessage;
};
