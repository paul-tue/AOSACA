#pragma once
#include "afxwin.h"
// CPupilView dialog

class CPupilView : public CDialogEx
{
    DECLARE_DYNAMIC(CPupilView)

public:
    CPupilView(CWnd* pParent = nullptr);
    virtual ~CPupilView();

    enum { IDD = IDD_PUPILVIEW };
    CStatic m_ImageDisp;
    CWnd* Display;
    bool m_BUpdate;

    BOOL m_bDisableZ1;
    BOOL m_bDisableZ2;
    BOOL m_bDisableZ3;
    BOOL m_bDisableZ4;
    BOOL m_bDisableZ5;
    BOOL m_bDisableZ6;

    double m_dSliderScale;

protected:
    virtual void DoDataExchange(CDataExchange* pDX);
    CBrush m_bkgndBrush;   // black
    CBrush m_frgndBrush;
    CFont m_BigFont;
    DECLARE_MESSAGE_MAP()

public:
    virtual BOOL OnInitDialog();
    afx_msg void OnClose();
    afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
    afx_msg void OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
    void ApplyZernikeDisableFlags();
    void OnZernikeToggle();
    afx_msg LRESULT OnUpdateWindow(WPARAM wParam, LPARAM lParam);
    void EnableZernikeControls(BOOL enable);
    CSliderCtrl m_ZernikeSlider[5];   

    static constexpr int UIToZernike[5] =
    {
        4, // Defocus
        3, // Astig 45°
        5, // Astig 90°
        7, // Coma vertical
        8  // Coma horizontal
    };

    void InitParam();
};
