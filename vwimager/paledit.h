// paledit.h : ヘッダー ファイル
//

#pragma once
#ifndef __PALEDIT_H__
#define __PALEDIT_H__

#include "resource.h"

HDIB WINAPI edit_palette(HWND hwnd, HDIB hdib);


/////////////////////////////////////////////////////////////////////////////
// CPalEditDlg ダイアログ

class CPalEditDlg : public CDialogImpl<CPalEditDlg>
{
public:
	CPalEditDlg();
	~CPalEditDlg();

	enum { IDD = IDD_PALEDIT };

	HPALETTE m_hpal;
	BOOL m_bUpdate;
	COLORREF m_crCur;

	CUpDownCtrl m_spinHue;
	CUpDownCtrl m_spinSat;
	CUpDownCtrl m_spinLum;
	CUpDownCtrl m_spinRed;
	CUpDownCtrl m_spinGreen;
	CUpDownCtrl m_spinBlue;
	CWindow m_palette;


BEGIN_MSG_MAP(CPalEditDlg)
	ON_WM_INITDIALOG()
	ON_WM_VSCROLL()
	ON_NOTIFY(PLTN_CLICK, IDC_PALETTE, OnNotifyPalClick)
	ON_NOTIFY(PLTN_PALETTECHANGED, IDC_PALETTE, OnNotifyPalChanged)
	ON_BN_CLICKED(IDOK, OnOK)
	ON_BN_CLICKED(IDCANCEL, OnCancel)
END_MSG_MAP()

	BOOL OnInitDialog(LPARAM lParam);
	void OnVScroll(UINT nSBCode, UINT nPos, HWND hScrollBar);
	void OnNotifyPalClick(NMHDR* pNmhdr, LRESULT* pResult);
	void OnNotifyPalChanged(NMHDR* pNmhdr, LRESULT* pResult);
	void OnOK();
	void OnCancel() { EndDialog(IDCANCEL); }

	void ShowPalInfo(COLORREF cr);
};


#endif			// __PALEDIT_H__
