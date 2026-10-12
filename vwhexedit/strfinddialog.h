#if !defined(AFX_STRFINDDIALOG_H__AD8AA3D4_7F75_11D2_BF48_0060972103E0__INCLUDED_)
#define AFX_STRFINDDIALOG_H__AD8AA3D4_7F75_11D2_BF48_0060972103E0__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000
// StrFindDialog.h : ヘッダー ファイル
//


enum {
	FIND_PREV,
	FIND_NEXT,
	FIND_ALL,
};


/////////////////////////////////////////////////////////////////////////////
// CStrFindDialog ダイアログ


class CStrFindDialog : public CDialogImpl<CStrFindDialog>
{
public:
	CStrFindDialog();
	~CStrFindDialog() { }

	enum { IDD = IDD_STRFIND };

BEGIN_MSG_MAP(CStrFindDialog)
	ON_WM_INITDIALOG()
	ON_BN_CLICKED(IDOK, OnOK)
	ON_BN_CLICKED(IDCANCEL, OnCancel)
	ON_BN_CLICKED(IDC_HELPCALL, OnHelpCall)
	ON_BN_CLICKED(IDC_FINDDOWN, OnFinddown)
	ON_BN_CLICKED(IDC_FINDUP, OnFindup)
	ON_BN_CLICKED(IDC_TCHECK, OnSearchMode)
	ON_BN_CLICKED(IDC_BCHECK, OnSearchMode)
END_MSG_MAP()

	BOOL OnInitDialog(LPARAM lParam);
	void OnOK();
	void OnCancel();
	void OnFinddown();
	void OnFindup();
	void OnHelpCall();
	void OnSearchMode();

	CComboBox m_combo;
	CString m_csSearch;
	int m_ifindtype;
	BOOL m_bBinarySearch;
	BOOL m_bCase;
};

#endif // !defined(AFX_STRFINDDIALOG_H__AD8AA3D4_7F75_11D2_BF48_0060972103E0__INCLUDED_)
