// ProgressDlg.cpp : implementation file
//

#include "stdafx.h"
#include "AOSACA.h"
#include "ProgressDlg.h"
#include "afxdialogex.h"

extern AOSACAParams		*g_AOSACAParams;
extern CControlPanelDlg	*g_controlpanel;

// CProgressDlg dialog

IMPLEMENT_DYNAMIC(CProgressDlg, CDialogEx)

CProgressDlg::CProgressDlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(CProgressDlg::IDD, pParent)
{
	m_pParent = (CAOSACADlg *)pParent;
}

CProgressDlg::~CProgressDlg()
{
	OnClose();
}

void CProgressDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDP_PROGRESSBAR, m_pProgress);
}


BEGIN_MESSAGE_MAP(CProgressDlg, CDialogEx)
	ON_WM_CLOSE()
	ON_MESSAGE(WM_UPDATE_WINDOW, &CProgressDlg::OnUpdateWindow)
	ON_BN_CLICKED(IDCANCEL, &CProgressDlg::OnBnClickedCancel)
END_MESSAGE_MAP()


// CProgressDlg message handlers
void CProgressDlg::SetBarParameters(short max)
{
	m_pProgress.SetRange(0, max);
	m_pProgress.SetPos(0);
	m_pProgress.SetStep(1);
	m_pProgress.SetState(PBST_NORMAL);
}

void CProgressDlg::OnClose()
{
	// TODO: Add your message handler code here and/or call default
	g_AOSACAParams->g_bPokeMatGeneration = false;
	m_pParent->UpdateMouseMode(MOUSE_NORMAL);
	g_controlpanel->EnableWindow(TRUE);
	EndDialog(IDOK);
//	CDialogEx::OnClose();
}

void CProgressDlg::Close() 
{
	OnClose();
}

afx_msg LRESULT CProgressDlg::OnUpdateWindow(WPARAM wParam, LPARAM lParam)
{
	switch (lParam) {
	case UPDATE_PGB_WINDOW:
		m_pProgress.StepIt();
		break;
	case UPDATE_PGBD_WINDOW:
		SetDlgItemText(IDCANCEL, _T("Done"));
		break;
	case UPDATE_PGBC_WINDOW:
		SetDlgItemText(IDCANCEL, _T("Cancelled"));
		break;
	}
	return 0;
}


void CProgressDlg::OnBnClickedCancel()
{
	// TODO: Add your control notification handler code here
	m_pParent->OnUpdateGeneratePokeMatrixMenu();
	OnClose();
}

void CProgressDlg::OnDoneGeneration() 
{
	// TODO: Add your message handler code here and/or call default
	SetDlgItemText(IDCANCEL,_T("Done"));
}