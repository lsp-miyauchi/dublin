// setup.h : header file
//

#pragma once

#include "common.h"

/////////////////////////////////////////////////////////////////////////////
// CSetupDlg dialog

class CSetupDlg : public CDialogImpl<CSetupDlg>
{
public:
	CSetupDlg() { }
	~CSetupDlg() { }

	enum { IDD = IDD_SETUP };

BEGIN_MSG_MAP(CSetupDlg)
	ON_WM_INITDIALOG()
	ON_BN_CLICKED(IDOK, OnOK)
	ON_BN_CLICKED(IDCANCEL, OnCancel)
END_MSG_MAP()

	BOOL OnInitDialog(LPARAM lParam);
	void OnOK();
	void OnCancel() { EndDialog(IDCANCEL); }

	int m_status;
};
