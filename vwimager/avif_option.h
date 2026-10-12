//////////////////////////////////////////////////////////////////////
// avif_option.h

#pragma once
#ifndef __AVIF_OPTION_H__
#define __AVIF_OPTION_H__

#include "resource.h"

typedef struct {
	int quality;
	int speed;
} AVIFOPTION;

//////////////////////////////////////////////////////////////////////
//

class CAvifOption : public CDialogImpl<CAvifOption>
{
public:
	enum { IDD = IDD_AVIFOPT };
	AVIFOPTION* m_popt;
	CTrackBarCtrl m_qslider;
	CTrackBarCtrl m_sslider;

BEGIN_MSG_MAP(CAvifOption)
	ON_WM_INITDIALOG()
	ON_WM_HSCROLL()
	ON_BN_CLICKED(IDOK, OnOK)
	ON_BN_CLICKED(IDCANCEL, OnCancel)
END_MSG_MAP()

	BOOL OnInitDialog(LPARAM lParam)
	{
		CenterWindow();
		m_popt = (AVIFOPTION*)lParam;

		m_qslider = GetDlgItem(IDC_QUALITY);
		m_qslider.SetRange(0, 100);
		m_qslider.SetPageSize(10);
		m_qslider.SetPos(m_popt->quality);
		m_qslider.SetTicFreq(10);
		SetDlgItemInt(IDC_QVALUE, m_popt->quality, FALSE);

		m_sslider = GetDlgItem(IDC_SPEED);
		m_sslider.SetRange(0, 10);
		m_sslider.SetPageSize(1);
		m_sslider.SetPos(m_popt->speed);
		m_sslider.SetTicFreq(1);
		SetDlgItemInt(IDC_SVALUE, m_popt->speed, FALSE);

		return TRUE;
	}

	void OnHScroll(UINT code, UINT pos, HWND hctl)
	{
		CTrackBarCtrl& ctl = (hctl == m_qslider) ? m_qslider : m_sslider;
		UINT id = (hctl == m_qslider) ? IDC_QVALUE: IDC_SVALUE;
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
	void OnOK()
	{
		m_popt->quality = m_qslider.GetPos();
		m_popt->speed = m_sslider.GetPos();
		EndDialog(IDOK);
	}
	void OnCancel() { EndDialog(IDCANCEL); }
};



#endif __AVIF_OPTION_H__
