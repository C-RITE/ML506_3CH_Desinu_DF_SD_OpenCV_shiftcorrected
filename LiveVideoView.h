#if !defined(AFX_LIVEVIDEOVIEW_H__20CC0B00_1427_412F_A521_55F701F66F11__INCLUDED_)
#define AFX_LIVEVIDEOVIEW_H__20CC0B00_1427_412F_A521_55F701F66F11__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// LiveVideoView.h : header file
//
#include "ML506Doc.h"

/////////////////////////////////////////////////////////////////////////////
// CLiveVideoView view

class CLiveVideoView : public CView
{
protected:
	CLiveVideoView();           // protected constructor used by dynamic creation
	DECLARE_DYNCREATE(CLiveVideoView)

// Attributes
public:

private:
	LPBITMAPINFO CreateDIB(int cx, int cy);
	HDC           m_hdc;
	LPBITMAPINFO  m_BitmapInfoWarp;
	CPen          m_PenGreen;
	CPen          m_PenBlue;
	CPen         *m_PenOld;
	CPen          m_PenWhite;
	CFont         m_FontText;
	CFont        *m_FontOld;
	POINT        *m_HistCordOld;
	POINT        *m_HistCordNew;
	UINT32        m_frameCounter;
	int           m_sd;
	BYTE          m_mean_val;
	CString       m_SavingMsg;


// Operations
public:
	void UpdateImageWidth();
	void CalcHistogram(BYTE *mean_val, int *sd);

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CLiveVideoView)
	public:
	virtual void OnInitialUpdate();
	protected:
	virtual void OnDraw(CDC* pDC);      // overridden to draw this view
	virtual void OnUpdate(CView* pSender, LPARAM lHint, CObject* pHint);
	//}}AFX_VIRTUAL

// Implementation
protected:
	virtual ~CLiveVideoView();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

	// Generated message map functions
protected:
	//{{AFX_MSG(CLiveVideoView)
	afx_msg void OnPaint();
	afx_msg void OnLButtonDblClk(UINT nFlags, CPoint point);
	afx_msg LRESULT OnSendMessage(WPARAM wParam, LPARAM lParam);
	afx_msg void OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags);
	afx_msg void OnRButtonDblClk(UINT nFlags, CPoint point);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_LIVEVIDEOVIEW_H__20CC0B00_1427_412F_A521_55F701F66F11__INCLUDED_)
