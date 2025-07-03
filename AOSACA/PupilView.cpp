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
	capSetCallbackOnFrame(hPupilCamWindow, NULL);
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
END_MESSAGE_MAP()


// CPupilView message handlers


void CPupilView::OnClose()
{
	// TODO: Add your message handler code here and/or call default
	capPreview(hPupilCamWindow, FALSE);
	CDialogEx::OnClose();
}

bool CPupilView::OnEditPupilcamerasettings()
{
	
	// TODO: Add your command handler code here
	CAPDRIVERCAPS CapDriverCaps = { }; 
	capDriverGetCaps(hPupilCamWindow, &CapDriverCaps, sizeof(CAPDRIVERCAPS)); 

	// Video source dialog box. 
	if (CapDriverCaps.fHasDlgVideoSource)
	{
		capDlgVideoSource(hPupilCamWindow); 
		return true;
	}
	return false;
}

BOOL CPupilView::OnEraseBkgnd(CDC* pDC)
{
	// TODO: Add your message handler code here and/or call default
	CRect rect;
    GetClientRect(&rect);
    CBrush *pOld = pDC->SelectObject(&g_AOSACAParams->m_bkgndbrush);
    BOOL bRes  = pDC->PatBlt(0, 0, rect.Width(), rect.Height(), PATCOPY);
    pDC->SelectObject(pOld);    // restore old brush
    return bRes;
//	return CDialogEx::OnEraseBkgnd(pDC);
}


LRESULT PASCAL FrameCallbackProc(HWND hWnd, LPVIDEOHDR lpVHdr)
{
	CPupilView *pParent = (CPupilView*)hWnd;

	if (!hWnd)
		return FALSE;
	cv::Mat* input = new cv::Mat();
	input->create(PUPIL_CAM_HEIGHT, PUPIL_CAM_WIDTH, CV_8UC3);
	cv::Mat single(input->rows, input->cols, CV_8UC1);
	memcpy(input->data, lpVHdr->lpData, lpVHdr->dwBytesUsed);
	int from_to[] = {0,0};
	cv::mixChannels(input, 1, &single, 1, from_to, 1);
	GaussianBlur(single, single, cv::Size(9,9), 2, 2);
	std::vector<cv::Vec3f> circles;
	HoughCircles(single, circles, cv::HOUGH_GRADIENT, 1, single.rows / 8, 200, 100, 0, 0);
	memcpy(lpVHdr->lpData, input->data, lpVHdr->dwBytesUsed);

	return (LRESULT)TRUE;
}

BOOL CPupilView::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	return false; // RETURN AT ONCE, CODE BELOW LEAKS MEMORY

	// TODO:  Add extra initialization here
	m_bHasVideoDlg = false;
	m_ImageDisp.MoveWindow(5,5,PUPIL_CAM_WIDTH,PUPIL_CAM_HEIGHT, 1);
	hPupilCamWindow = capCreateCaptureWindow(_T("PupilCamView Handle"), WS_CHILD | WS_VISIBLE, 0, 0, PUPIL_CAM_WIDTH, PUPIL_CAM_HEIGHT, m_ImageDisp, 1);
    // connect the driver with the camera 
	if (capDriverConnect(hPupilCamWindow,0))
	{	 
		LPBITMAPINFO lpbi;
		DWORD dwSize;
		dwSize = capGetVideoFormatSize(hPupilCamWindow);
		lpbi = (LPBITMAPINFO)malloc (dwSize);
		capGetVideoFormat(hPupilCamWindow, lpbi, dwSize); 
		lpbi->bmiHeader.biHeight = PUPIL_CAM_HEIGHT;
		lpbi->bmiHeader.biWidth = PUPIL_CAM_WIDTH;
		//lpbi->bmiHeader.biBitCount = 8;
		lpbi->bmiHeader.biSizeImage = PUPIL_CAM_HEIGHT*PUPIL_CAM_WIDTH*(lpbi->bmiHeader.biBitCount/8);
		capSetVideoFormat(hPupilCamWindow, lpbi, dwSize);
		CAPTUREPARMS CaptureParms;
		capCaptureGetSetup(hPupilCamWindow, &CaptureParms, sizeof(CAPTUREPARMS));
		CaptureParms.dwAudioBufferSize = 0;
		CaptureParms.fCaptureAudio = FALSE;
		capCaptureSetSetup(hPupilCamWindow, &CaptureParms, sizeof (CAPTUREPARMS));
		CAPDRIVERCAPS CapDriverCaps = { }; 
		capDriverGetCaps(hPupilCamWindow, &CapDriverCaps, sizeof(CAPDRIVERCAPS)); 
		m_bHasVideoDlg = CapDriverCaps.fHasDlgVideoSource?true:false;
		capPreviewRate(hPupilCamWindow, 24);     // rate, in milliseconds
		capPreview(hPupilCamWindow, TRUE);
		
		FARPROC fpFrameCallback;
		fpFrameCallback = MakeProcInstance((FARPROC)FrameCallbackProc, hInst);
		capSetCallbackOnFrame(hPupilCamWindow, fpFrameCallback);
	}

	return TRUE;  // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE


}


