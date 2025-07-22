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
	CStatic				m_ImageDisp;
	
// Dialog Data
	enum { IDD = IDD_PUPILVIEW };

private:
	int m_frameCount = 0;
	std::chrono::steady_clock::time_point m_frameStartTime;
	double m_framerate = 0.0;

protected:
	virtual void DoDataExchange(CDataExchange* pDX);
	DECLARE_MESSAGE_MAP()

public:
	afx_msg void	OnClose();
	afx_msg BOOL	OnEraseBkgnd(CDC* pDC);
	LRESULT			OnFrameUpdate(WPARAM, LPARAM);
	bool			OnEditPupilcamerasettings();
	void			OnPaint();
	cv::Mat			FindAndDrawPupil(const cv::Mat&);
	virtual BOOL	 OnInitDialog();
};
