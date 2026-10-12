////////////////////////////////////////////////////////////
// jump.h

#pragma once

/////////////////////////////////////////////////////////////////////////////
// CJumpDlg ダイアログ

class CJumpDlg : public CDialogImpl<CJumpDlg>
{
public:
	CJumpDlg() { }
	~CJumpDlg() { }

	enum { IDD = IDD_JUMP };

	int m_line;

BEGIN_MSG_MAP(CJumpDlg)
	ON_WM_INITDIALOG()
	ON_BN_CLICKED(IDOK, OnOK)
	ON_BN_CLICKED(IDCANCEL, OnCancel)
END_MSG_MAP()

	BOOL OnInitDialog(LPARAM lParam) { CenterWindow(); return TRUE; }
	void OnOK() { m_line = GetDlgItemInt(IDC_TEXT, NULL, FALSE); EndDialog(IDOK); }
	void OnCancel() { EndDialog(IDCANCEL); }
};

