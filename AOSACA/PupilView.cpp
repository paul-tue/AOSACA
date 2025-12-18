// PupilView.cpp : implementation file
//

#include "stdafx.h"
#include "AOSACA.h"
#include "Centroid.h"
#include "PupilView.h"
#include "afxdialogex.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

extern CCentroid* g_centroids;
extern AOSACAParams* g_AOSACAParams;
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
    DDX_Check(pDX, IDC_ZACT_2, m_bDisableZ2);
    DDX_Check(pDX, IDC_ZACT_3, m_bDisableZ3);
    DDX_Check(pDX, IDC_ZACT_4, m_bDisableZ4);
    DDX_Check(pDX, IDC_ZACT_5, m_bDisableZ5);
    DDX_Check(pDX, IDC_ZACT_6, m_bDisableZ6);
    DDX_Control(pDX, IDC_SLIDER_DEFOCUS, m_ZernikeSlider[0]);
    DDX_Control(pDX, IDC_SLIDER_ASTIGM, m_ZernikeSlider[1]);
    DDX_Control(pDX, IDC_SLIDER_ASTIGM2, m_ZernikeSlider[2]);
    DDX_Control(pDX, IDC_SLIDER_COMA, m_ZernikeSlider[3]);
    DDX_Control(pDX, IDC_SLIDER_COMA2, m_ZernikeSlider[4]);
}


BEGIN_MESSAGE_MAP(CPupilView, CDialogEx)
    ON_WM_CLOSE()
    ON_WM_CTLCOLOR()    
    ON_WM_PAINT()
    ON_BN_CLICKED(IDC_ZACT_2, &CPupilView::OnZernikeToggle)
    ON_BN_CLICKED(IDC_ZACT_3, &CPupilView::OnZernikeToggle)
    ON_BN_CLICKED(IDC_ZACT_4, &CPupilView::OnZernikeToggle)
    ON_BN_CLICKED(IDC_ZACT_5, &CPupilView::OnZernikeToggle)
    ON_BN_CLICKED(IDC_ZACT_6, &CPupilView::OnZernikeToggle)
    ON_WM_HSCROLL()
END_MESSAGE_MAP()


void CPupilView::OnClose()
{
    CDialogEx::OnClose();
}

BOOL CPupilView::OnInitDialog()
{
    CDialog::OnInitDialog();
    m_bkgndBrush.CreateSolidBrush(RGB(0, 0, 0)); // black background
    m_frgndBrush.CreateSolidBrush(FOREGROUNDCOLOR);
    m_BigFont.CreatePointFont(
        120,            // 11 pt (value × 10)
        L"Segoe UI"     // or Arial
    );

    GetDlgItem(IDC_STATIC_TITLE)->SetFont(&m_BigFont);

    m_bDisableZ2 = g_centroids->m_bDisableZernike[2];
    m_bDisableZ3 = g_centroids->m_bDisableZernike[3];
    m_bDisableZ4 = g_centroids->m_bDisableZernike[4];
    m_bDisableZ5 = g_centroids->m_bDisableZernike[5];
    m_bDisableZ6 = g_centroids->m_bDisableZernike[6];

    constexpr int SLIDER_RANGE = 100; // symmetric

    for (int i = 0; i < 5; ++i)
    {
        m_ZernikeSlider[i].SetRange(-SLIDER_RANGE, SLIDER_RANGE);
        m_ZernikeSlider[i].SetTicFreq(20);
        m_ZernikeSlider[i].SetPos(0);
    }

    m_dSliderScale = 0.01; // µm per step

    UpdateData(FALSE);

    return TRUE;
}

void CPupilView::EnableZernikeControls(BOOL enable)
{
    GetDlgItem(IDC_ZACT_2)->EnableWindow(enable);
    GetDlgItem(IDC_ZACT_3)->EnableWindow(enable);
    GetDlgItem(IDC_ZACT_4)->EnableWindow(enable);
    GetDlgItem(IDC_ZACT_5)->EnableWindow(enable);
    GetDlgItem(IDC_ZACT_6)->EnableWindow(enable);
    // sliders
    for (int i = 0; i < 5; ++i)
        m_ZernikeSlider[i].EnableWindow(enable);
}

void CPupilView::OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar)
{
    CDialogEx::OnHScroll(nSBCode, nPos, pScrollBar);

    if (!pScrollBar)
        return;

    for (int i = 0; i < 5; ++i)
    {
        if (pScrollBar == (CScrollBar*)&m_ZernikeSlider[i])
        {
            const int zernikeIndex = UIToZernike[i];
            const int pos = m_ZernikeSlider[i].GetPos();

            const double value_um = pos * m_dSliderScale;

            g_centroids->SetZernikeBias(zernikeIndex, value_um);
            break;
        }
    }
}


HBRUSH CPupilView::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor)
{
    if (nCtlColor == CTLCOLOR_STATIC)
    {
        const int id = pWnd->GetDlgCtrlID();

        // Group box caption → opaque colored background
        if (id == IDG_ZERNIKE_MODES)
        {
            pDC->SetBkMode(OPAQUE);
            pDC->SetBkColor(FOREGROUNDCOLOR); // blue
            pDC->SetTextColor(RGB(0, 0, 0));  // black text
            return (HBRUSH)m_frgndBrush.GetSafeHandle();
        }

        // Normal static text (checkbox labels etc.)
        pDC->SetBkMode(TRANSPARENT);
        pDC->SetTextColor(TEXTCOLOR); // orange
        return (HBRUSH)m_bkgndBrush.GetSafeHandle();
    }

    if (nCtlColor == CTLCOLOR_DLG)
    {
        pDC->SetBkColor(RGB(0, 0, 0));
        return (HBRUSH)m_bkgndBrush.GetSafeHandle();
    }

    if (nCtlColor == CTLCOLOR_BTN)
    {
        pDC->SetBkMode(TRANSPARENT);
        pDC->SetTextColor(TEXTCOLOR);
        return (HBRUSH)m_bkgndBrush.GetSafeHandle();
    }

    return CDialogEx::OnCtlColor(pDC, pWnd, nCtlColor);
}


void CPupilView::ApplyZernikeDisableFlags()
{
    const BOOL uiFlags[5] =
    {
        m_bDisableZ2,
        m_bDisableZ3,
        m_bDisableZ4,
        m_bDisableZ5,
        m_bDisableZ6
    };

    for (int i = 0; i < 5; ++i)
    {
        g_centroids->m_bDisableZernike[UIToZernike[i]] = uiFlags[i];
    }
}

void CPupilView::OnZernikeToggle()
{
    UpdateData(TRUE);
    ApplyZernikeDisableFlags();

    // trigger recompute / redraw
    GetParent()->PostMessage(WM_UPDATE_WINDOW, 0, UPDATE_DMVMAP_WINDOW);
}
