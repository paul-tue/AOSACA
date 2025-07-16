#pragma once
#define PUPIL_CAM_WIDTH		640
#define PUPIL_CAM_HEIGHT	480
#include "afxwin.h"
#include <opencv2/highgui.hpp>
// CPupilView dialog

class CPupilView : public CDialogEx
{
	DECLARE_DYNAMIC(CPupilView)

public:
	CPupilView(CWnd* pParent = NULL);
	virtual ~CPupilView();
	cv::VideoCapture	m_cap;
	std::atomic_bool	m_stopCapture{ false };
	std::thread			m_captureThread;
	cv::Mat				m_latestFrame;
	std::mutex			m_frameMutex;
	CStatic				m_ImageDisp;
	
// Dialog Data
	enum { IDD = IDD_PUPILVIEW };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);
	DECLARE_MESSAGE_MAP()

public:
	afx_msg void OnClose();
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	LRESULT OnFrameUpdate(WPARAM, LPARAM);
	void OnPaint();
	cv::Mat FindAndDrawPupil(const cv::Mat&);
	virtual BOOL OnInitDialog();
};
