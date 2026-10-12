////////////////////////////////////////////////////////////
// setup.h

#pragma once

#include "padview.h"

/////////////////////////////////////////////////////////////////////////////
// CSetupDlg ダイアログ

class CSetupDlg : public CDialogImpl<CSetupDlg>
{
public:
	CSetupDlg()
	{
	}
	~CSetupDlg()
	{
	}

	enum { IDD = IDD_SETUP };

	VConfig m_vc;
	CComboBox m_wrapline;
	CComboBox m_linedenom;
	CListBox m_list;
	CFont m_font;

BEGIN_MSG_MAP(CSetupDlg)
	ON_WM_INITDIALOG()
	ON_BN_CLICKED(IDOK, OnOK)
	ON_BN_CLICKED(IDCANCEL, OnCancel)
	ON_BN_CLICKED(IDC_BTN_TEXT, OnTextColor)
	ON_BN_CLICKED(IDC_BTN_BKGND, OnBkgndColor)
	ON_CBN_SELCHANGE(IDC_WRAPLINETYPE, OnSelChangeWrapType)
	ON_LBN_SELCHANGE(IDC_LIST, OnSelChangeColorList)
	ON_CBN_SELCHANGE(IDC_FONT, OnSelChangeFont)
END_MSG_MAP()

	BOOL OnInitDialog(LPARAM lParam);
	void OnOK();
	void OnCancel() { EndDialog(IDCANCEL); }
	void OnTextColor();
	void OnBkgndColor();
	void OnSelChangeWrapType();
	void OnSelChangeColorList();
	void OnSelChangeFont();

	void UpdateSample();
	COLORREF SelectColor(UINT id, COLORREF cr);
};


