#include "stdafx.h"
#include "CSetupSelectionDlg.h"
#include "AOSACA.h"
#include "afxdialogex.h"

IMPLEMENT_DYNAMIC(CSetupSelectionDlg, CDialog)

CSetupSelectionDlg::CSetupSelectionDlg(CWnd* pParent /*=nullptr*/)
	: CDialog(IDD_SETUPSELECTION_DLG, pParent)
{
}

CSetupSelectionDlg::~CSetupSelectionDlg()
{
}

void CSetupSelectionDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CSetupSelectionDlg, CDialog)
	ON_BN_CLICKED(IDC_INVIVO, &CSetupSelectionDlg::OnBnClickedInvivo)
	ON_BN_CLICKED(IDC_EXVIVO, &CSetupSelectionDlg::OnBnClickedExvivo)
END_MESSAGE_MAP()

BOOL CSetupSelectionDlg::OnInitDialog()
{
	CDialog::OnInitDialog();
	return TRUE;
}

void CSetupSelectionDlg::OnBnClickedInvivo()
{
	m_SelectedSetup = _T("in-vivo");
	EndDialog(IDOK);
}

void CSetupSelectionDlg::OnBnClickedExvivo()
{
	m_SelectedSetup = _T("ex-vivo");
	EndDialog(IDOK);
}
