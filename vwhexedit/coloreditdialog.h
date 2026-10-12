////////////////////////////////////////////////////////////////////////
// ColorEditDialog.h : ヘッダー ファイル
//

#pragma once
#ifndef __COLOREDITDIALOG_H__
#define __COLOREDITDIALOG_H__

/////////////////////////////////////////////////////////////////////////////
// CColorEditDialog ダイアログ

class CColorEditDialog : public CDialogImpl<CColorEditDialog>
{
public:
	CColorEditDialog() { }
	~CColorEditDialog() { }

	enum { IDD = IDD_COLOR };

BEGIN_MSG_MAP(CColorEditDialog)
	ON_WM_INITDIALOG()
	ON_WM_DRAWITEM()
	ON_BN_CLICKED(IDOK, OnOK)
	ON_BN_CLICKED(IDCANCEL, OnCancel)
	ON_BN_CLICKED(IDC_POINT, OnPointColor)
	ON_BN_CLICKED(IDC_TEXT1, OnTextColor)
	ON_BN_CLICKED(IDC_BACK1, OnBackColor)
	ON_BN_CLICKED(IDC_MARKTEXT, OnMarkTextColor)
	ON_BN_CLICKED(IDC_MARKBACK, OnMarkBackColor)
	ON_CBN_SELCHANGE(IDC_FONT, OnFont)
	ON_BN_CLICKED(IDC_LINE, OnLine)
END_MSG_MAP()

	BOOL OnInitDialog(LPARAM lParam);
	void OnDrawItem(int nIDCtl, LPDRAWITEMSTRUCT lpDrawItemStruct);
	void OnOK() { EndDialog(IDOK); }
	void OnCancel() { EndDialog(IDCANCEL); }
	void OnPointColor();
	void OnTextColor();
	void OnBackColor();
	void OnMarkTextColor();
	void OnMarkBackColor();
	void OnFont();
	void OnLine();

	void DrawFontButton(LPDRAWITEMSTRUCT lpdis);
	COLORREF DoSelectColor(UINT nIDCtrl, COLORREF rgb);

	LOGFONT m_lf;
	VIEWCOLOR m_vc;
};

#endif		// __COLOREDITDIALOG_H__
