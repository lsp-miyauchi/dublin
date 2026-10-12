//////////////////////////////////////////////////////////////////////
// webpoption.h

#pragma once
#ifndef __WEBPOPTION_H__
#define __WEBPOPTION_H__

#include "resource.h"

typedef struct {
	int quality;
	int method;
	WEBP_ENCODE_PRESET preset;
} WEBPOPTION;

//////////////////////////////////////////////////////////////////////
//

class CWebpOption : public CDialogImpl<CWebpOption>
{
public:
	enum { IDD = IDD_WEBPOPT };
	WEBPOPTION* m_pwo;
	CTrackBarCtrl m_qslider;
	CTrackBarCtrl m_mslider;
	CComboBox m_presets;

BEGIN_MSG_MAP(CWebpOption)
	ON_WM_INITDIALOG()
	ON_WM_HSCROLL()
	ON_BN_CLICKED(IDOK, OnOK)
	ON_BN_CLICKED(IDCANCEL, OnCancel)
END_MSG_MAP()

	BOOL OnInitDialog(LPARAM lParam)
	{
		CenterWindow();
		m_presets = GetDlgItem(IDC_PRESETS);
		m_pwo = (WEBPOPTION*)lParam;

		m_qslider = GetDlgItem(IDC_QUALITY);
		m_qslider.SetRange(0, 100);
		m_qslider.SetPageSize(5);
		m_qslider.SetPos(m_pwo->quality);
		m_qslider.SetTicFreq(5);
		SetDlgItemInt(IDC_QVALUE, m_pwo->quality, FALSE);

		m_mslider = GetDlgItem(IDC_METHOD);
		m_mslider.SetRange(0, 6);
		m_mslider.SetPageSize(1);
		m_mslider.SetPos(m_pwo->method);
		m_mslider.SetTicFreq(1);
		SetDlgItemInt(IDC_MVALUE, m_pwo->method, FALSE);

		CString str, sets;
		str.LoadString(IDS_WEBP_ENCODE_PRESETS);
		int pos = 0;
		sets = str.Tokenize(_T("|"), pos);
		while (sets != "") {
			m_presets.AddString(sets);
			sets = str.Tokenize(_T("|"), pos);
		}
		m_presets.SetCurSel(m_pwo->preset);
		return TRUE;
	}

	void OnHScroll(UINT code, UINT pos, HWND hctl)
	{
		CTrackBarCtrl& ctl = (hctl == m_qslider) ? m_qslider : m_mslider;
		UINT id = (hctl == m_qslider) ? IDC_QVALUE : IDC_MVALUE;
		switch (code) {
			case TB_TOP:
			case TB_BOTTOM:
			case TB_ENDTRACK:
			case TB_LINEDOWN:
			case TB_LINEUP:
			case TB_PAGEDOWN:
			case TB_PAGEUP:
				SetDlgItemInt(id, ctl.GetPos(), FALSE);
				break;
			case TB_THUMBPOSITION:
			case TB_THUMBTRACK:
				SetDlgItemInt(id, pos, FALSE);
				break;
		}
	}
	void CWebpOption::OnOK()
	{
		m_pwo->quality = m_qslider.GetPos();
		m_pwo->method = m_mslider.GetPos();
		m_pwo->preset = (WEBP_ENCODE_PRESET)m_presets.GetCurSel();
		EndDialog(IDOK);
	}
	void OnCancel() { EndDialog(IDCANCEL); }
};



#endif __WEBPOPTION_H__
