// ML506Doc.h : interface of the CML506Doc class
//
/////////////////////////////////////////////////////////////////////////////
#include "virtex5bmd.h"
#include "vfw.h"
#include "mmsystem.h"
#include "utils/netcomm/SockListener.h"

#if !defined(AFX_ML506DOC_H__DA98F54C_82CA_4552_938A_12A8DDEA211F__INCLUDED_)
#define AFX_ML506DOC_H__DA98F54C_82CA_4552_938A_12A8DDEA211F__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

typedef struct {
	int        *UnMatrixIdx;		// lookup table for possible desinusoiding
	float      *UnMatrixVal;
} DesinusoidParams;

class CML506Doc : public CDocument
{
protected: // create from serialization only
	CML506Doc();
	DECLARE_DYNCREATE(CML506Doc)

	MMRESULT m_idTimerEvent1;
	MMRESULT m_idTimerEvent2;
	UINT    m_nTimerRes;
	ULONG   m_uResolution;

// Attributes
public:
	BOOL    m_bSymbol;
	void    GenerateFileID(CString videoName, int *index);

// Operations
public:

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CML506Doc)
	public:
	virtual BOOL OnNewDocument();
	virtual void Serialize(CArchive& ar);
	//}}AFX_VIRTUAL

// Implementation
public:
	BOOL Load8BITbmp(CString filename, BYTE *stim_buffer, int *width, int *height);
	void LoadSymbol(CString filename);
	void GenSinCurve(float amp, float freq);
	virtual ~CML506Doc();
	bool SendNetMessage(CString message);
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:
	DWORD   thdid_handle[2];
	HANDLE  thd_handle[2];
	static  DWORD WINAPI ThreadLoadData2FPGA(LPVOID pParam);
	static DWORD WINAPI ThreadNetMsgProcess(LPVOID pParam);

	HANDLE *m_eNetMsg;
	CString *m_strNetRecBuff;

// Generated message map functions
protected:
	//{{AFX_MSG(CML506Doc)
	afx_msg void OnCameraConnect();
	afx_msg void OnCameraDisconnect();
	afx_msg void OnUpdateCameraConnect(CCmdUI* pCmdUI);
	afx_msg void OnUpdateCameraDisconnect(CCmdUI* pCmdUI);
	afx_msg void OnSetupDesinusoid();
	afx_msg void OnUpdateSetupDesinusoid(CCmdUI* pCmdUI);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()


private:
	CSockListener *m_ncListener_IGUIDE;
	CSockListener *m_ncListener_AO;

};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_ML506DOC_H__DA98F54C_82CA_4552_938A_12A8DDEA211F__INCLUDED_)
