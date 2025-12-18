#pragma once
#include "afxwin.h"
#include <resource.h>

class CSetupSelectionDlg : public CDialog
{
	DECLARE_DYNAMIC(CSetupSelectionDlg)

public:
	CSetupSelectionDlg(CWnd* pParent = nullptr);
	virtual ~CSetupSelectionDlg();

	enum { IDD = IDD_SETUPSELECTION_DLG };

	CString m_SelectedSetup; // "in-vivo" or "ex-vivo"

protected:
	virtual void DoDataExchange(CDataExchange* pDX);
	virtual BOOL OnInitDialog();
	afx_msg void OnBnClickedInvivo();
	afx_msg void OnBnClickedExvivo();
	DECLARE_MESSAGE_MAP()
};
