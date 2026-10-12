// addin.h : header file
//

#pragma once
#ifndef __ADDIN_H__
#define __ADDIN_H__

/////////////////////////////////////////////////////////////////////////////

class CAddinTools
{
public:
	CAddinTools() { }
	CAddinTools(LPCTSTR pszExt, LPCTSTR pszTools) { m_strExt = pszExt, m_strPath = pszTools; }
	CAddinTools(const CAddinTools& src) { *this = src; }

	CString m_strExt;
	CString m_strPath;

	CAddinTools& operator =(const CAddinTools& src)
	{
		if (&src != this) {
			m_strExt = src.m_strExt;
			m_strPath = src.m_strPath;
		}
		return *this;
	}
};

/////////////////////////////////////////////////////////////////////////////
// CAddinToolsDlg dialog

class CAddinToolsDlg : public CDialogImpl<CAddinToolsDlg>
{
public:
	CAddinToolsDlg();
	~CAddinToolsDlg();

	enum { IDD = IDD_ADDINVIEWER };

	CAddinTools m_at;
	BOOL m_bAdd;
	TCHAR m_szOlddir[MAX_PATH * 2];

BEGIN_MSG_MAP(CAddinToolsDlg)
	ON_WM_INITDIALOG()
	ON_BN_CLICKED(IDOK, OnOK)
	ON_BN_CLICKED(IDCANCEL, OnCancel)
	ON_BN_CLICKED(IDC_CONFIRM, OnConfirm)
	ON_BN_CLICKED(IDC_HELPCALL, OnHelpCall)
	ON_NOTIFY(SEN_GETTEXT, IDC_PATH, OnNotifyBrowse)
END_MSG_MAP()

	BOOL OnInitDialog(LPARAM lParam);
	void OnOK();
	void OnCancel() { EndDialog(IDCANCEL); }
	void OnConfirm();
	void OnNotifyBrowse(NMHDR* pNmhdr, LRESULT* pResult);
	void OnHelpCall() { fvApp.HelpTopic(NULL, HB_DLG, IDD); }
};

#endif		// __ADDIN_H__
