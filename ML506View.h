// ML506View.h : interface of the CML506View class
//
/////////////////////////////////////////////////////////////////////////////

#if !defined(AFX_ML506VIEW_H__1448BE67_E276_4054_8704_C31A266E3E10__INCLUDED_)
#define AFX_ML506VIEW_H__1448BE67_E276_4054_8704_C31A266E3E10__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "ML506Doc.h"
#include "VideoChannelR.h"
#include "VideoChannelG.h"
#include "VideoChannelB.h"
#include "SinusoidCal.h"
#include "opencv2/highgui/highgui.hpp"

#define  DLG_ITEM_WIDTH      80
#define  DLG_ITEM_HEIGHT     25
#define  DLG_ITEM_MARGIN_X    5
#define  DLG_ITEM_MARGIN_Y    3


class CML506View : public CView
{
protected: // create from serialization only
	CML506View();
	DECLARE_DYNCREATE(CML506View)

// Attributes
public:
	CML506Doc* GetDocument();

// Operations
public:

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CML506View)
	public:
	virtual void OnDraw(CDC* pDC);  // overridden to draw this view
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
	virtual void OnInitialUpdate();
	protected:
	virtual BOOL OnPreparePrinting(CPrintInfo* pInfo);
	virtual void OnBeginPrinting(CDC* pDC, CPrintInfo* pInfo);
	virtual void OnEndPrinting(CDC* pDC, CPrintInfo* pInfo);
	virtual void OnUpdate(CView* pSender, LPARAM lHint, CObject* pHint);
	//}}AFX_VIRTUAL

// Implementation
public:
	void EnableGuiControls(BOOL xfgLoaded);
	int UpdateRuntimeRegisters();
	BOOL VerifyI2CVals();
	LPBITMAPINFO CreateDIB(int cx, int cy);
	void CreateRTregisters();
	void CreateCfgSpace();
	void CreateScannerParams();
	void UpdateScannerParams();
	BOOL VerifyVal();
	int  VerifyAddr(BOOL bWrite);
	void CreateADCregisters();
	void CreateDACregisters();
	void CreateGrabberParams();
	void UpdateGuiControls();
	void UpdateVidSize(BOOL bNewDialog);
	void ParseVideo();
	void GenDesinusoidLUT();
	void ResetTrackingPos();

	virtual ~CML506View();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

private:
	int  ReadWriteI2CRegister(BOOL bRead, UINT64 slave_addr);
	void ReadWritePCIeRegister(BOOL bRead);
	char *Dec2Bin(BYTE val);
	int  I2CController(UINT64 regDataW, BYTE *reg_val, BOOL bRead);
	void ReadI2CRegisters(BOOL bADC);
	void ReadPCIeRegisters(BOOL bCfgSpc);
	BOOL Hex2Dec(char *text, int len, long *val);
	void ShowTabItems(int index);
	BOOL ReadLUT(CString lutFilename, int fWidth, int fHeight, int *UnMatrixIdx, float *UnMatrixVal);


	float        *m_fLaserModulation;

	// PCIe configuration space
	CStatic       m_cfgData[28];
	UINT          m_cfgDataID[28];
	char         *m_cfgLabel[28];

	// Slave Registers
	CStatic       m_adcText[48];
	CRichEditCtrl m_adcEdit[48];
	UINT          m_adcTextID[48];
	UINT          m_adcEditID[48];

	CStatic       m_dacText[18];
	CRichEditCtrl m_dacEdit[18];
	UINT          m_dacTextID[18];
	UINT          m_dacEditID[18];
	char         *m_dacLabel[18];
	BYTE          m_dacRegs[18];
	
	// run-time registers
	CStatic       m_regText[28];
	CRichEditCtrl m_regEdit[28];
	UINT          m_regTextID[28];
	UINT          m_regEditID[28];
	char         *m_regLabel[28];

	CStatic       m_TextCurrentReg;
	CStatic       m_TextCurrentRegAddr;
	CStatic       m_TextCurrentRegVal;
	CRichEditCtrl m_EditCurrentRegAddr;
	CRichEditCtrl m_EditCurrentRegVal;
	CButton       m_ReadOne;
	CButton       m_ReadAll;
	CButton       m_WriteOne;
	CButton       m_WriteAll;
	CButton       m_chkChannelR;
	CButton       m_chkChannelG;
	CButton       m_chkChannelB;
	CButton       m_chkChannelSW;
	CButton		  m_chkDiscardBlinks;
	
	CStatic       m_frames;
	CTabCtrl      m_tabPCIe;

	CStatic       m_lblImgSizeX;
	CStatic       m_lblImgSizeY;
	CStatic       m_lblBrightnessR;
	CStatic       m_lblContrastR;
	CStatic       m_lblBrightnessG;
	CStatic       m_lblContrastG;
	CStatic       m_lblBrightnessB;
	CStatic       m_lblContrastB;
	CStatic       m_lblOffsetX;
	CStatic       m_lblOffsetY;
	CStatic       m_lblHsyncI;
	CStatic       m_lblHsyncO;
	CStatic       m_lblVsyncI;
	CStatic       m_lblVsyncO;
	CStatic       m_lblPixelClock;
	CStatic       m_lblVstart;
	CStatic       m_lblResonant;
	CStatic       m_lblGalvo;
	CStatic       m_lblGalvoForward;
	CStatic       m_lblGalvoBackward;
	CStatic       m_lblRampOffset;

	CButton       m_btnCalibrateH;
	CButton       m_chkInterleave;
	CButton       m_chkZeroInverse;
	CButton       m_chkSymmetricRamp;
	CButton       m_chkEnableTracking;
	CScrollBar    m_scrResonant;
	CScrollBar    m_scrGalvo;
	CScrollBar    m_scrGalvoForward;
	CScrollBar    m_scrGalvoBackward;
	CScrollBar    m_scrRampOffset;
	CScrollBar    m_scrImgSizeX;
	CScrollBar    m_scrImgSizeY;
	CRichEditCtrl m_edtBrightnessR;
	CRichEditCtrl m_edtContrastR;
	CRichEditCtrl m_edtBrightnessG;
	CRichEditCtrl m_edtContrastG;
	CRichEditCtrl m_edtBrightnessB;
	CRichEditCtrl m_edtContrastB;
	CRichEditCtrl m_edtPixelClock;
	CScrollBar    m_scrOffsetX;
	CScrollBar    m_scrOffsetY;
	CComboBox     m_cboHsyncI;
	CComboBox     m_cboHsyncO;
	CComboBox     m_cboVsyncI;
	CComboBox     m_cboVsyncO;
//	CButton       m_chkExternalClock;
	CButton       m_btnLoadStimulus;
	CButton       m_btnDesinusoidCal;

	CButton       m_chkBidirection;
	CStatic       m_lblBlankPixels;
	CScrollBar    m_scrBlankPixels;

	CStatic       m_lblTrackX;
	CScrollBar    m_scrTrackX;
	CStatic       m_lblTrackY;
	CScrollBar    m_scrTrackY;
	CStatic       m_lblSteerY;
	CScrollBar    m_scrSteerY;

	CStatic       m_lblPower488;
	CScrollBar    m_scrPower488;
	CButton       m_chkPower488;
	CStatic       m_lblPower568;
	CScrollBar    m_scrPower568;
	CButton       m_chkPower568;
	CStatic       m_lblPower796;
	CScrollBar    m_scrPower796;
	CButton       m_chkPower796;


	CButton       m_btnSaveVideo;
	CButton       m_rdoLenthInFrames;
	CButton       m_rdoLenthInSeconds;
	CButton       m_radLines;
	CButton       m_radVsync;
	CStatic       m_lblVideoName;
	CStatic       m_lblVideoPrefix;
	CStatic       m_lblVideoLength;
	CRichEditCtrl m_edtVideoNameR;
	CButton       m_btnVideoFolderR;
	CRichEditCtrl m_edtVideoLengthR;
	CRichEditCtrl m_edtVideoPrefixR;
	CRichEditCtrl m_edtVideoFlagsR1;
	CRichEditCtrl m_edtVideoFlagsR2;
	CRichEditCtrl m_edtVideoNameG;
	CButton       m_btnVideoFolderG;
	CRichEditCtrl m_edtVideoLengthG;
	CRichEditCtrl m_edtVideoPrefixG;
	CRichEditCtrl m_edtVideoFlagsG1;
	CRichEditCtrl m_edtVideoFlagsG2;
	CRichEditCtrl m_edtVideoNameB;
	CButton       m_btnVideoFolderB;
	CRichEditCtrl m_edtVideoLengthB;
	CRichEditCtrl m_edtVideoPrefixB;
	CRichEditCtrl m_edtVideoFlagsB1;
	CRichEditCtrl m_edtVideoFlagsB2;



	BYTE          m_regAddr;           
	UINT          m_regVal;
	BYTE          m_adcVals[48];
	BYTE          m_dacVals[18];

	int           m_tabIndex;
	int           m_iVideoNum;
	int           m_iChannelNum;
	int           m_iShiftBS;
	UINT          m_iPLLclks;

	int           m_osx;
	int           m_osy;
//	UINT          m_stimulus_x;
//	UINT          m_stimulus_y;
//	CPen          m_PenGreen;
/*	unsigned short *m_lutIdx1;
	unsigned short *m_lutVal1;
	unsigned short *m_lutIdx2;
	unsigned short *m_lutVal2;*/

	BOOL          m_bHsyncI;
	BOOL          m_bHsyncO;
	BOOL          m_bVsyncI;
	BOOL          m_bVsyncO;

	BOOL		  m_bPower488;
	BOOL		  m_bPower568;
	BOOL		  m_bPower796;
	unsigned short m_nPower488;
	unsigned short m_nPower568;
	unsigned short m_nPower796;

	CString       m_strAppName;
	CString       m_strFileName;
	CString       m_strTitleName;
	BOOL          m_bXFGloaded;
	BOOL          m_bClientCreated;

	BOOL          m_bEnableTracking;
	BOOL          m_bFrameInLines;
	BOOL          m_bVideoInSeconds;	
	cv::Mat		  frameR;
	cv::Mat		  frameG;	
	cv::Mat		  frameB;
	cv::Size	  m_frameSize;
	cv::VideoWriter   m_aviHandlerR;				// AVI hander for raw video
	cv::VideoWriter   m_aviHandlerG;				// AVI hander for raw video
	cv::VideoWriter   m_aviHandlerB;				// AVI hander for raw video
	BOOL          m_bAviHandlerOnR;
	BOOL          m_bAviHandlerOnG;
	BOOL          m_bAviHandlerOnB;
	BYTE         *m_image;

	CString       m_aviFileNameR;
	CString       m_aviFileNameG;
	CString       m_aviFileNameB;

	CString       m_aviFolderNameR;
	CString       m_aviFolderNameG;
	CString       m_aviFolderNameB;

	BOOL          m_bDualChannel;
	BOOL          m_bSampleG;
	BOOL          m_bSampleB;
	BOOL          m_bSampleSW;

	BOOL          m_bSymmetricRamp;
	BOOL          m_bZeroBackward;
	BOOL          m_bInterleaveLines;
	BYTE          m_fovH;
	BYTE          m_fovV;
	unsigned short m_forwardLines;
	unsigned short m_backLines;
	unsigned short m_rampOffset;
	short         m_iTrackX;
	short         m_iTrackY;

	UINT          m_nTimerID;
	BOOL          m_bCalibrate;
	BOOL          m_bFOVflag;
	BYTE         *m_imgCalibration;

	int           m_iSteerY;
	BOOL          m_bRotate45;//rotate axes of the tracking mirror 45 degrees
	BYTE		 *m_imgBuffsub;
	
	CVideoChannelR *m_pDlgR;
	CVideoChannelG *m_pDlgG;
	CVideoChannelB *m_pDlgB;
	CSinusoidCal   *m_pDlgSinu;

protected:

// Generated message map functions
protected:
	//{{AFX_MSG(CML506View)
	afx_msg int  OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void ReadOneRegister();
	afx_msg void WriteOneRegister();
	afx_msg void ReadAllRegister();
	afx_msg void LoadStimulus();
	afx_msg void CalibrateSinu();
	afx_msg void ReadPCIe();
	afx_msg void OnSelchangeTabMain(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg LRESULT OnSendMessage(WPARAM wParam, LPARAM lParam);
	afx_msg void LoadVideoFolderR();
	afx_msg void LoadVideoFolderG();
	afx_msg void LoadVideoFolderB();
	afx_msg void SaveLiveVideo();
	afx_msg void VideoInFrames();
	afx_msg void VideoInSeconds();
	afx_msg void FrameCounterInVsync();
	afx_msg void FrameCounterInLines();
	afx_msg void OnLoadDcf();
	afx_msg void OnUpdateSaveDCF(CCmdUI* pCmdUI);
	afx_msg void OnSaveDcf();
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
	afx_msg void OnNewDcf();
	afx_msg void HsyncInChange();
	afx_msg void HsyncOutChange();
	afx_msg void VsyncInChange();
	afx_msg void VsyncOutChange();
	afx_msg void UpdateBrightnessR(NMHDR* wParam, LRESULT *plr);
	afx_msg void UpdateContrastR(NMHDR* wParam, LRESULT *plr);
	afx_msg void UpdateBrightnessG(NMHDR* wParam, LRESULT *plr);
	afx_msg void UpdateContrastG(NMHDR* wParam, LRESULT *plr);
	afx_msg void UpdateBrightnessB(NMHDR* wParam, LRESULT *plr);
	afx_msg void UpdateContrastB(NMHDR* wParam, LRESULT *plr);
	afx_msg void UpdatePixelClock(NMHDR* wParam, LRESULT *plr);
	afx_msg void SampleDualChannel();
	afx_msg void SampleChannelG();
	afx_msg void SampleChannelB();
	afx_msg void SampleChannelSW();
	afx_msg void InterleaveLines();
	afx_msg void ZeroBackward();
	afx_msg void SymmetricRamp();
	afx_msg void EnableTracking();
	afx_msg void ScaleRSFOV();
	afx_msg void Power488Cal();
	afx_msg void Power568Cal();
	afx_msg void Power796Cal();

	void CheckChannelG(BOOL flag, BOOL bFromGUI);
	void CheckChannelB(BOOL flag, BOOL bFromGUI);
	void CheckDualChannel(BOOL flag);
	
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnTimer(UINT_PTR nIDEvent);
};

#ifndef _DEBUG  // debug version in ML506View.cpp
inline CML506Doc* CML506View::GetDocument()
   { return (CML506Doc*)m_pDocument; }
#endif

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_ML506VIEW_H__1448BE67_E276_4054_8704_C31A266E3E10__INCLUDED_)
