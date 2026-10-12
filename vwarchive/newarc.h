// NewArcDlg.h : CNewArcDlg の宣言

#pragma once

#include "resource.h"       // メイン シンボル

// CNewArcDlg

typedef struct tagNEWARCINFO {
	TCHAR name[MAX_PATH * 2];
	int arctype;
} NEWARCINFO;

class CNewArcDlg : 
	public CDialogImpl<CNewArcDlg>
{
public:
	CNewArcDlg()
	{
		memset(&m_nai, 0, sizeof(m_nai));
	}

	~CNewArcDlg()
	{
	}

	enum { IDD = IDD_NEWARC };

BEGIN_MSG_MAP(CNewArcDlg)
	ON_WM_INITDIALOG()
	ON_BN_CLICKED(IDOK, OnOK)
	ON_BN_CLICKED(IDCANCEL, OnCancel)
	ON_NOTIFY(SEN_GETTEXT, IDC_EDIT, OnNotifyBrowse)
END_MSG_MAP()

	BOOL OnInitDialog(LPARAM lParam);
	void OnOK();
	void OnCancel();
	void OnNotifyBrowse(NMHDR* pNmhdr, LRESULT* pResult);

	NEWARCINFO m_nai;
};
