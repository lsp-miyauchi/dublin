// paste.h : ヘッダー ファイル
//

#pragma once
#ifndef __PASTE_H__
#define __PASTE_H__

#include "anchor.h"
#include "resource.h"

typedef struct tagARITHINFO {
	int method;
	int opaque;
	BOOL shadow;
	int shadow_gauss;
	int shadow_distance;
	COLORREF shadow_color;
} ARITHINFO;

typedef struct tagPASTEPARAM {
	ARITHINFO ai;
	BOOL noalign;
	int anchor;
	CPoint ptOffset;
} PASTEPARAM;

/////////////////////////////////////////////////////////////////////////////
// CPasteDlg ダイアログ

class CPasteDlg : public CDialogImpl<CPasteDlg>
{
public:
	CPasteDlg();

	enum { IDD = IDD_PASTE };
	PASTEPARAM m_pp;

	CComboBox m_combo;
	CTrackCtrl m_oslider;
	CTrackCtrl m_gslider;
	CTrackCtrl m_dslider;
	CAnchorWnd m_anchorwnd;

BEGIN_MSG_MAP(CPasteDlg)
	ON_WM_INITDIALOG()
	ON_BN_CLICKED(IDOK, OnOK)
	ON_BN_CLICKED(IDCANCEL, OnCancel)
	ON_BN_CLICKED(IDC_DROPSHADOW, OnDropShadow)
	ON_BN_CLICKED(IDC_NOALIGN, OnCheckAlign)
	ON_BN_CLICKED(IDC_ALIGN, OnCheckAlign)
END_MSG_MAP()

	BOOL OnInitDialog(LPARAM lParam);
	void OnOK();
	void OnCancel() { EndDialog(IDCANCEL); }
	void OnDropShadow();
	void OnCheckAlign();
};

BOOL __fastcall paste_image(mif& src, mif& dst, int x, int y, ARITHINFO* pai);
HDIB __fastcall PasteImage(HDIB hsrc, HDIB hdst, int x, int y, ARITHINFO* pai);


#endif				// __PASTE_H__
