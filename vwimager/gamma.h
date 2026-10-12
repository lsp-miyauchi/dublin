// Gamma.h : ヘッダー ファイル
//

#pragma once
#ifndef __GAMMA_H__
#define __GAMMA_H__

#include "resource.h"

HDIB gamma_rgb(HDIB hSrc, double r_amt, double g_amt, double b_amt, double rgb_amt);

/////////////////////////////////////////////////////////////////////////////
// CGammaDlg ダイアログ

class CGammaDlg : public CDialogImpl<CGammaDlg>
{
public:
	CGammaDlg();

	enum { IDD = IDD_GAMMA };
	CComboBox m_combo;
	CDegreeCtrl m_slider;
	CWindow m_view;

	int m_nChannel;
	int m_posRGB;
	int m_posRed;
	int m_posGreen;
	int m_posBlue;
	double m_dbRGB;
	double m_dbRed;
	double m_dbGreen;
	double m_dbBlue;
	HDIB m_hdib;
	BOOL m_whole;

BEGIN_MSG_MAP(CGammaDlg)
	ON_WM_INITDIALOG()
	ON_WM_HSCROLL()
	ON_NOTIFY(MVN_GETEFFECT, IDC_PREVIEW, OnNotifyGetEffect)
	ON_CBN_SELCHANGE(IDC_COMBO, OnSelChange)
	ON_BN_CLICKED(IDOK, OnOK)
	ON_BN_CLICKED(IDCANCEL, OnCancel)
	ON_BN_CLICKED(IDC_WHOLE, OnCheckWhole)
END_MSG_MAP()

	BOOL OnInitDialog(LPARAM lParam);
	void OnHScroll(UINT nSBCode, UINT nPos, HWND hScrollBar);
	void OnNotifyGetEffect(NMHDR* pNmhdr, LRESULT* pResult);
	void OnSelChange();
	void OnCheckWhole();
	void OnOK() { EndDialog(IDOK); }
	void OnCancel() { EndDialog(IDCANCEL); }

	void SetValue(int nChannel, int nAmount);
};


#endif			// __GAMMA_H__
