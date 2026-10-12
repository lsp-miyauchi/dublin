////////////////////////////////////////////////////////////
// password.h

#pragma once
#ifndef __PASSWORD_H__
#define __PASSWORD_H__


////////////////////////////////////////////////////////////
//

class CPasswordDlg : public CDialogImpl<CPasswordDlg>
{
public:
	CString m_pass;

	CPasswordDlg() { }
	~CPasswordDlg() { }

	enum { IDD = IDD_PASSWORD };

BEGIN_MSG_MAP(CPasswordDlg)
	ON_WM_INITDIALOG()
	ON_BN_CLICKED(IDOK, OnOK)
	ON_BN_CLICKED(IDCANCEL, OnCancel)
END_MSG_MAP()

	BOOL OnInitDialog(LPARAM /*lParam*/)
	{
		CenterWindow();
		return TRUE;
	}
	void OnOK()
	{
		CString str;
		if (GetDlgItemText(IDC_EDIT, str) < 1) {
			MessageBeep(0);
			GetDlgItem(IDC_EDIT).SetFocus();
			return;
		}
		m_pass = str;
		EndDialog(IDOK);
	}
	void OnCancel()
	{
		EndDialog(IDCANCEL);
	}

};


#endif			// __PASSWORD_H__
