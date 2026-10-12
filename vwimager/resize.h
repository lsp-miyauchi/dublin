// resize.h

#pragma once
#ifndef __RESIZE_H__
#define __RESIZE_H__

#include "anchor.h"


HDIB resize_canvas(HWND hParent, HDIB hdib);

/////////////////////////////////////////////////////////////////////////////
// CSizeDlg ダイアログ

class CResizeDlg : public CDialogImpl<CResizeDlg>
{
public:
	CResizeDlg();

	enum { IDD = IDD_RESIZE };

	CAnchorWnd m_anchorwnd;
	int m_anchor;
	int m_bits;
	CSize m_size;
	COLORREF m_cr;
	double m_scale;

BEGIN_MSG_MAP(CResizeDlg)
	ON_WM_INITDIALOG()
	ON_BN_CLICKED(IDOK, OnOK)
	ON_BN_CLICKED(IDCANCEL, OnCancel)
	ON_EN_UPDATE(IDC_XSIZE, OnChangeXsize)
	ON_EN_UPDATE(IDC_YSIZE, OnChangeYsize)
END_MSG_MAP()

	BOOL OnInitDialog(LPARAM lParam);
	void OnOK();
	void OnCancel() { EndDialog(IDCANCEL); }
	void OnChangeXsize();
	void OnChangeYsize();

	void ShowSize();
};


#endif			// __RESIZE_H__
