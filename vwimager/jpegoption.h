//////////////////////////////////////////////////////////////////////
// jpegoption.cpp

#pragma once
#ifndef __JPEGOPTION_H__
#define __JPEGOPTION_H__

#include "resource.h"

typedef struct tagEFC_JPEGOPTION {
	int type;
	int quality;
	int smoothing;
} EFC_JPEGOPTION;

//////////////////////////////////////////////////////////////////////
//

class CJpegOption : public CDialogImpl<CJpegOption>
{
public:
	enum { IDD = IDD_JPEGOPT };
	EFC_JPEGOPTION* m_pjo;
	CTrackBarCtrl m_qslider;
	CTrackBarCtrl m_sslider;

BEGIN_MSG_MAP(CJpegOption)
	ON_WM_INITDIALOG()
	ON_WM_HSCROLL()
	ON_BN_CLICKED(IDOK, OnOK)
	ON_BN_CLICKED(IDCANCEL, OnCancel)
END_MSG_MAP()

	BOOL OnInitDialog(LPARAM lParam)
	{
		CenterWindow();
		m_qslider = GetDlgItem(IDC_QUALITY);
		m_sslider = GetDlgItem(IDC_SMOOTHING);
		m_pjo = (EFC_JPEGOPTION*)lParam;

		m_qslider.SetRange(0, 100);
		m_qslider.SetPageSize(5);
		m_qslider.SetPos(m_pjo->quality);
		m_qslider.SetTicFreq(5);
		SetDlgItemInt(IDC_QVALUE, m_pjo->quality);

		m_sslider.SetRange(0, 100);
		m_sslider.SetPageSize(5);
		m_sslider.SetPos(m_pjo->smoothing);
		m_sslider.SetTicFreq(5);
		SetDlgItemInt(IDC_SVALUE, m_pjo->smoothing);

		switch (m_pjo->type) {
			case 0: CheckDlgButton(IDC_STANDARD, TRUE); break;
			case 1: CheckDlgButton(IDC_BASELINE, TRUE); break;
			default: CheckDlgButton(IDC_PROGRESSIVE, TRUE); break;
		}
		return TRUE;
	}

	void OnHScroll(UINT code, UINT pos, HWND hctl)
	{
		CTrackBarCtrl& ctl = (hctl == m_qslider) ? m_qslider : m_sslider;
		UINT id = (hctl == m_qslider) ? IDC_QVALUE : IDC_SVALUE;
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

	void CJpegOption::OnOK()
	{
		if (IsDlgButtonChecked(IDC_STANDARD))
			m_pjo->type = 0;
		else if (IsDlgButtonChecked(IDC_BASELINE))
			m_pjo->type = 1;
		else
			m_pjo->type = 2;
		m_pjo->quality = m_qslider.GetPos();
		m_pjo->smoothing = m_sslider.GetPos();
		EndDialog(IDOK);
	}
	void OnCancel() { EndDialog(IDCANCEL); }
};




#endif __JPEGOPTION_H__
