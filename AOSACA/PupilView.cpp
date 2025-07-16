// PupilView.cpp : implementation file
//

#include "stdafx.h"
#include "AOSACA.h"
#include "PupilView.h"
#include "afxdialogex.h"
#include "opencv2/highgui.hpp"
#include "opencv2/imgproc.hpp"
#include <Vfw.h>
#include <vector>

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

extern AOSACAParams*g_AOSACAParams;
// CPupilView dialog

IMPLEMENT_DYNAMIC(CPupilView, CDialogEx)

CPupilView::CPupilView(CWnd* pParent /*=NULL*/)
	: CDialogEx(CPupilView::IDD, pParent)
{

}

CPupilView::~CPupilView()
{
	OnClose();
}


void CPupilView::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDL_PUPILVIEW, m_ImageDisp);
}


BEGIN_MESSAGE_MAP(CPupilView, CDialogEx)
    ON_WM_CLOSE()
    ON_WM_ERASEBKGND()
    ON_WM_PAINT()
    ON_MESSAGE(WM_USER + 100, &CPupilView::OnFrameUpdate)
END_MESSAGE_MAP()


void CPupilView::OnClose()
{
    m_stopCapture = true;

    if (m_captureThread.joinable())
        m_captureThread.join();

    if (m_cap.isOpened())
        m_cap.release();

    CDialogEx::OnClose();
}


BOOL CPupilView::OnEraseBkgnd(CDC* pDC)
{
    return TRUE;
}

LRESULT CPupilView::OnFrameUpdate(WPARAM, LPARAM)
{
    Invalidate(FALSE); // trigger repaint, no erase background to reduce flicker
    return 0;
}

cv::Size ComputeScaledSizeKeepAspect(int srcW, int srcH, int dstW, int dstH)
{
    double scaleX = static_cast<double>(dstW) / srcW;
    double scaleY = static_cast<double>(dstH) / srcH;
    double scale = std::min(scaleX, scaleY);
    return cv::Size(static_cast<int>(srcW * scale), static_cast<int>(srcH * scale));
}

BOOL CPupilView::OnInitDialog()
{
	CDialogEx::OnInitDialog();

    // change this to connect a different camera (eventually 0 = internal webcam)
	if (!m_cap.open(0, cv::CAP_DSHOW)) 
	{
		g_AOSACAParams->g_stAppErrBuff.Empty();
		g_AOSACAParams->g_stAppErrBuff = _T("No supported USB camera found!");
		g_AOSACAParams->ShowError(MB_ICONINFORMATION);
		return false;
	}
	// set dimensions and auto exposure
	m_cap.set(cv::CAP_PROP_FRAME_WIDTH, PUPIL_CAM_WIDTH);
	m_cap.set(cv::CAP_PROP_FRAME_HEIGHT, PUPIL_CAM_HEIGHT);
	m_cap.set(cv::CAP_PROP_AUTO_EXPOSURE, 3);
	
	m_stopCapture = false;
    m_captureThread = std::thread([this]()
        {
            cv::Mat frame;
            while (!m_stopCapture && m_cap.isOpened())
            {
                if (!m_cap.read(frame) || frame.empty())
                    break;

                // Lock and update shared frame
                {
                    std::lock_guard<std::mutex> lock(m_frameMutex);
                    frame.copyTo(m_latestFrame);
                }

                // Post message to UI thread to repaint
                PostMessage(WM_USER + 100, 0, 0);  // Custom user message
            }
        });

	return TRUE;
}


void CPupilView::OnPaint()
{
    CPaintDC dc(&m_ImageDisp);

    cv::Mat frameCopy;
    {
        std::lock_guard<std::mutex> lock(m_frameMutex);
        if (m_latestFrame.empty())
            return;
        m_latestFrame.copyTo(frameCopy);
    }
    cv::Mat frameWithPupil = FindAndDrawPupil(frameCopy);

    CRect rect;
    m_ImageDisp.GetClientRect(&rect);
    cv::Size targetSize = ComputeScaledSizeKeepAspect(frameWithPupil.cols, frameWithPupil.rows, rect.Width(), rect.Height());
    cv::Mat resized, resizedBgra;
    cv::resize(frameWithPupil, resized, targetSize, 0, 0, cv::INTER_LINEAR);
    cv::cvtColor(resized, resizedBgra, cv::COLOR_BGR2BGRA);

    int offsetX = (rect.Width() - targetSize.width) / 2;
    int offsetY = (rect.Height() - targetSize.height) / 2;

    BITMAPINFO bmi = {};
    bmi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
    bmi.bmiHeader.biWidth = resizedBgra.cols;
    bmi.bmiHeader.biHeight = -resizedBgra.rows;
    bmi.bmiHeader.biPlanes = 1;
    bmi.bmiHeader.biBitCount = 32;
    bmi.bmiHeader.biCompression = BI_RGB;

    SetStretchBltMode(dc.m_hDC, HALFTONE);
    SetBrushOrgEx(dc.m_hDC, 0, 0, nullptr);

    //dc.FillSolidRect(rect, RGB(0, 0, 0));  // clear frame
    StretchDIBits(
        dc.m_hDC,
        offsetX, offsetY, targetSize.width, targetSize.height,
        0, 0, resizedBgra.cols, resizedBgra.rows,
        resizedBgra.data,
        &bmi,
        DIB_RGB_COLORS,
        SRCCOPY
    );
}

// very basic pupil detection, might improve later
cv::Mat CPupilView::FindAndDrawPupil(const cv::Mat& src)
{
    cv::Mat display = src.clone();
    cv::Mat gray(src.cols, src.rows, CV_8UC1);
    cv::Mat blurred(src.cols, src.rows, CV_8UC1);
    cv::cvtColor(src, gray, cv::COLOR_BGR2GRAY);

    // reduce noise
    cv::GaussianBlur(gray, blurred, cv::Size(9, 9), 2, 2);

    // detect circles (ideally pupils)
    std::vector<cv::Vec3f> circles;
    cv::HoughCircles(blurred, circles, cv::HOUGH_GRADIENT, 1,
        gray.rows / 8,     // min dist between centers
        100, 30,           // param1 (Canny), param2 (accumulator)
        int(src.rows / 4), int(src.rows / 2));           // min/max radius

    // draw first detected pupil as a bright orange circle
    if (!circles.empty()) {
        cv::Point center(cvRound(circles[0][0]), cvRound(circles[0][1]));
        int radius = cvRound(circles[0][2]);
        cv::circle(display, center, radius, cv::Scalar(0, 128, 255), 2);
    }
    return display;
}