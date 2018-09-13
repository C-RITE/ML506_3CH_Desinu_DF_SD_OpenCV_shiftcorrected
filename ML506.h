// ML506.h : main header file for the ML506 application
//

#if !defined(AFX_ML506_H__ED7A2718_15CC_4984_9A77_45B31E8EB914__INCLUDED_)
#define AFX_ML506_H__ED7A2718_15CC_4984_9A77_45B31E8EB914__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#ifndef __AFXWIN_H__
	#error include 'stdafx.h' before including this file for PCH
#endif

#include "resource.h"       // main symbols

#define  WM_MESSAGE_SEND            WM_USER+1
#define  USER_MESSAGE_BRIGHTNESS    100
#define  USER_MESSAGE_CONTRAST      101
#define  USER_MESSAGE_IMGWIDTH      102
#define  USER_MESSAGE_CONNECT       103
#define  USER_MESSAGE_DISCONNECT    104
#define  USER_MESSAGE_SAVEVIDEO     105
#define  USER_MESSAGE_NEWFRAME      106
#define  USER_MESSAGE_NEWOFFSET     107
#define  USER_MESSAGE_FOV_CALIBRATION 108
#define  IGUIDE_MESSAGE_SAVE		109

/////////////////////////////////////////////////////////////////////////////
// CML506App:
// See ML506.cpp for the implementation of this class
//

class CML506App : public CWinApp
{
public:
	CML506App();

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CML506App)
	public:
	virtual BOOL InitInstance();
	virtual int ExitInstance();
	//}}AFX_VIRTUAL

// Implementation
	//{{AFX_MSG(CML506App)
	afx_msg void OnAppAbout();
		// NOTE - the ClassWizard will add and remove member functions here.
		//    DO NOT EDIT what you see in these blocks of generated code !
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};


/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_ML506_H__ED7A2718_15CC_4984_9A77_45B31E8EB914__INCLUDED_)
