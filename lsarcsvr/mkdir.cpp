// mkdir.cpp : CUnpackDlg の実装

#include "stdafx.h"
#include "common.h"
#include "resource.h"

////////////////////////////////////////////////////////////////////////
// CMakeDirDlg

class CMakeDirDlg : public CDialogImpl<CMakeDirDlg>
{
public:
	enum { IDD = IDD_MKDIR };
	LPCTSTR m_path;
	BOOL m_single;
	int m_action;

	CMakeDirDlg() : m_path(NULL), m_single(TRUE), m_action(CR_NO) { }

BEGIN_MSG_MAP(CMakeDirkDlg)
	ON_WM_INITDIALOG()
	ON_BN_CLICKED(IDOK, OnOK)
	ON_BN_CLICKED(IDCANCEL, OnCancel)
	ON_BN_CLICKED(IDC_ALL, OnOKAll)
END_MSG_MAP()

	BOOL OnInitDialog(LPARAM lParam)
	{
		CenterWindow();
		SetDlgItemText(IDC_PATH, m_path);
		if (m_single) GetDlgItem(IDC_ALL).EnableWindow(FALSE);
		return TRUE;
	}

	void OnOK()  { m_action = CR_YES; EndDialog(IDOK); }
	void OnCancel() { m_action = CR_NO; EndDialog(IDCANCEL); }
	void OnOKAll()  { m_action = CR_YESALL; EndDialog(IDOK); }
};

////////////////////////////////////////////////////////////////////////
//

int QueryMakeDir(HWND hwnd, LPCTSTR pszText, BOOL bSingle)
{
	CMakeDirDlg dlg;
	dlg.m_path = pszText;
	dlg.m_single = bSingle;
	return (dlg.DoModal(hwnd) == IDOK) ? dlg.m_action : CR_NO;
}
