// SetupDlg.h : CSetupDlg の宣言

#pragma once

#include "resource.h"       // メイン シンボル

// CSetupDlg

typedef struct tagARCVIEWINFO {
	WORD stat;
	TCHAR path[MAX_PATH * 2];
	COLORREF colors[5];
} ARCVIEWINFO;

class CSetupDlg : 
	public CDialogImpl<CSetupDlg>
{
public:
	CSetupDlg()
	{
		ZeroMemory(&m_ai, sizeof(m_ai));
	}

	~CSetupDlg()
	{
	}

	enum { IDD = IDD_SETUP };

BEGIN_MSG_MAP(CSetupDlg)
	ON_WM_INITDIALOG()
	ON_BN_CLICKED(IDOK, OnOK)
	ON_BN_CLICKED(IDCANCEL, OnCancel)
	ON_BN_CLICKED(IDC_BTN_TEXT, OnTextColor)
	ON_BN_CLICKED(IDC_BTN_BACK, OnBackColor)
	ON_BN_CLICKED(IDC_BTN_RTEXT, OnRTextColor)
	ON_BN_CLICKED(IDC_BTN_RBACK, OnRBackColor)
	ON_NOTIFY(SEN_GETTEXT, IDC_EDIT, OnNotifyBrowse)
END_MSG_MAP()

	COLORREF SelectColor(UINT id, COLORREF cr);

	BOOL OnInitDialog(LPARAM lParam);
	void OnOK();
	void OnCancel();
	void OnNotifyBrowse(NMHDR* pNmhdr, LRESULT* pResult);
	void OnTextColor();
	void OnBackColor();
	void OnRTextColor();
	void OnRBackColor();


	ARCVIEWINFO m_ai;
};


