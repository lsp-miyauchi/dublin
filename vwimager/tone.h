// tone.h : ヘッダー ファイル
//

#pragma once
#ifndef __TONE_H__
#define __TONE_H__

#include "resource.h"

HDIB tone_effect(HWND hwnd, HDIB hdib);


/////////////////////////////////////////////////////////////////////////////
// CToneDlg ダイアログ

class CToneDlg : public CDialogImpl<CToneDlg>
{
public:
	CToneDlg();

	enum { IDD = IDD_TONE };
	CComboBox m_combo;
	CWindow m_view;

	TONEINFO m_ti;
	HDIB m_hdib;

BEGIN_MSG_MAP(CToneDlg)
	ON_WM_INITDIALOG()
	ON_NOTIFY(MGN_SELCHANGE, IDC_GRAPH, OnNotifyGraphChanged)
	ON_NOTIFY(MVN_GETEFFECT, IDC_PREVIEW, OnNotifyGetEffect)
	ON_CBN_SELCHANGE(IDC_COMBO, OnSelChange)
	ON_BN_CLICKED(IDOK, OnOK)
	ON_BN_CLICKED(IDCANCEL, OnCancel)
	ON_BN_CLICKED(IDC_WHOLE, OnCheckWhole)
	ON_BN_CLICKED(IDC_RESET, OnReset)
END_MSG_MAP()

	BOOL OnInitDialog(LPARAM lParam);
	void OnNotifyGraphChanged(NMHDR* pNmhdr, LRESULT* pResult);
	void OnNotifyGetEffect(NMHDR* pNmhdr, LRESULT* pResult);
	void OnSelChange();
	void OnCheckWhole();
	void OnReset();
	void OnOK() { EndDialog(IDOK); }
	void OnCancel() { EndDialog(IDCANCEL); }

	void SetValue(int ref, int val);
	void ShowParam(int chn);
};


#endif			// __TONE_H__
