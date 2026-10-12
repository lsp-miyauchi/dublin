// htmlout.h : ヘッダー ファイル
//

#pragma once
#ifndef __HTMLOUT_H__
#define __HTMLOUT_H__


/////////////////////////////////////////////////////////////////////////////
// CListOutputDlg ダイアログ

typedef struct tagLISTOUTPARAMS {
	int nOutputType;
	BOOL bListAll;
	BOOL bLaunchApp;
} LISTOUTPARAMS;


class CListOutputDlg : public CDialogImpl<CListOutputDlg>
{
public:
	CString m_strPath;
	BOOL m_bListAll;
	BOOL m_bLaunchApp;
	int m_nOutputType;
	enum { IDD = IDD_LISTOUTPUT };

	CListOutputDlg()
	{
		m_strPath.Empty();
		m_bListAll = TRUE;
		m_bLaunchApp = TRUE;
		m_nOutputType = 0;
	}

BEGIN_MSG_MAP(CListOutputDlg)
	ON_WM_INITDIALOG()
	ON_BN_CLICKED(IDOK, OnOK)
	ON_BN_CLICKED(IDCANCEL, OnCancel)
	ON_BN_CLICKED(IDC_HELPCALL, OnHelpCall);
	ON_NOTIFY(SEN_GETTEXT, IDC_PATH, OnNotifyBrowse)
END_MSG_MAP()

	BOOL OnInitDialog(LPARAM lParam);
	void OnOK();
	void OnCancel() { EndDialog(IDCANCEL); }
	void OnHelpCall() { fvApp.HelpTopic(NULL, HB_DLG, IDD); }
	void OnNotifyBrowse(NMHDR* pNmhdr, LRESULT* pResult);
};

#endif		// __HTMLOUT_H__
