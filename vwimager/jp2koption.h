#pragma once

#include "resource.h"

//////////////////////////////////////////////////////////////////////
//

class CJp2Option : public CDialogImpl<CJp2Option>
{
public:
	enum { IDD = IDD_JP2OPT };
	EFC_JPEGOPTION* m_pjo;
	CTrackBarCtrl m_qslider;

	BEGIN_MSG_MAP(CJp2Option)
		ON_WM_INITDIALOG()
		ON_WM_HSCROLL()
		ON_BN_CLICKED(IDOK, OnOK)
		ON_BN_CLICKED(IDCANCEL, OnCancel)
	END_MSG_MAP()

	BOOL OnInitDialog(LPARAM lParam)
	{
		CenterWindow();
		m_qslider = GetDlgItem(IDC_QUALITY);
		m_pjo = (EFC_JPEGOPTION*)lParam;

		TCHAR buf[MAX_PATH * 2];
		m_qslider.SetRange(1, 100);
		m_qslider.SetSelection(2, 30);
		m_qslider.SetPageSize(5);
		m_qslider.SetPos(m_pjo->quality);
		m_qslider.SetTicFreq(5);
		wsprintf(buf, _T("1/%d"), m_pjo->quality);
		SetDlgItemText(IDC_QVALUE, buf);

		return TRUE;
	}

	void OnHScroll(UINT nSBCode, UINT nPos, HWND hScrollBar)
	{
		if (hScrollBar != m_qslider) {
			DefWindowProc();
			return;
		}

		int val;
		switch (nSBCode) {
			case TB_THUMBTRACK:
				val = (int)nPos;
				break;
			case TB_TOP:
			case TB_BOTTOM:
			case TB_LINEUP:
			case TB_LINEDOWN:
			case TB_PAGEUP:
			case TB_PAGEDOWN:
				val = m_qslider.GetPos();
				break;
			default:
				return;
		}

		TCHAR buf[MAX_PATH * 2];
		wsprintf(buf, _T("1/%d"), val);
		SetDlgItemText(IDC_QVALUE, buf);
	}

	void OnOK()
	{
		m_pjo->quality = m_qslider.GetPos();
		EndDialog(IDOK);
	}

	void OnCancel() {
		EndDialog(IDCANCEL);
	}
};



