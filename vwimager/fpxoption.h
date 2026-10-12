#pragma once

#include "resource.h"

//////////////////////////////////////////////////////////////////////
//

class CFpxOption : public CDialogImpl<CFpxOption>
{
public:
	enum { IDD = IDD_FPXOPT };
	EFC_JPEGOPTION* m_pjo;
	CTrackBarCtrl m_qslider;

	BEGIN_MSG_MAP(CFpxOption)
		ON_WM_INITDIALOG()
		ON_WM_HSCROLL()
		ON_BN_CLICKED(IDOK, OnOK)
		ON_BN_CLICKED(IDCANCEL, OnCancel)
		ON_BN_CLICKED(IDC_USEJPEG, OnUseJpeg)
	END_MSG_MAP()

	BOOL OnInitDialog(LPARAM lParam)
	{
		CenterWindow();
		m_qslider = GetDlgItem(IDC_QUALITY);
		m_pjo = (EFC_JPEGOPTION*)lParam;

		m_qslider.SetRange(0, 100);
		m_qslider.SetPageSize(5);
		m_qslider.SetPos(m_pjo->quality);
		m_qslider.SetTicFreq(5);
		SetDlgItemInt(IDC_QVALUE, m_pjo->quality);

		CheckDlgButton(IDC_USEJPEG, m_pjo->type);
		OnUseJpeg();
		return TRUE;
	}

	void OnHScroll(UINT code, UINT pos, HWND hctl)
	{
		if (hctl != m_qslider)
			return;

		switch (code) {
			case TB_TOP:
			case TB_BOTTOM:
			case TB_ENDTRACK:
			case TB_LINEDOWN:
			case TB_LINEUP:
			case TB_PAGEDOWN:
			case TB_PAGEUP:
				SetDlgItemInt(IDC_QVALUE, m_qslider.GetPos(), FALSE);
				break;
			case TB_THUMBPOSITION:
			case TB_THUMBTRACK:
				SetDlgItemInt(IDC_QVALUE, pos, FALSE);
				break;
		}
	}

	void OnOK()
	{
		m_pjo->type = IsDlgButtonChecked(IDC_USEJPEG);
		m_pjo->quality = m_qslider.GetPos();
		EndDialog(IDOK);
	}

	void OnCancel() {
		EndDialog(IDCANCEL);
	}

	void OnUseJpeg()
	{
		m_qslider.EnableWindow(IsDlgButtonChecked(IDC_USEJPEG));
	}
};
