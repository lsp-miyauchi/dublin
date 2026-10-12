// Modulate.cpp : インプリメンテーション ファイル
//

#include "stdafx.h"
#include "resource.h"

void InitDlgCtrl(HWND hctl, UINT idResource);

/////////////////////////////////////////////////////////////////////////////
//

HDIB hue_saturation(HDIB hsrc, LPRECT prc, int hue_range, int per_h, int per_s, int per_l, PROGRESSCALLBACK proc)
{
	mif image;
	if (image.attachdib(hsrc)) {
image.m_progress = proc;
		image.hue_saturation(prc, hue_range, per_h, per_s, per_l);
		return image.createdib();
	}
	return NULL;
}

/////////////////////////////////////////////////////////////////////////////
// ModulateDlg メッセージ ハンドラ

class CModulateDlg : public CDialogImpl<CModulateDlg>
{
public:
	enum { IDD = IDD_MODULATE };

	CDegreeCtrl m_hslider;
	CDegreeCtrl m_sslider;
	CDegreeCtrl m_bslider;
	CComboBox m_part;
	CWindow m_view;

	int m_hue_range;
	int m_hue;
	int m_sat;
	int m_brt;
	HDIB m_hdib;
	BOOL m_whole;

BEGIN_MSG_MAP(CModulateDlg)
	ON_WM_INITDIALOG()
	ON_WM_HSCROLL()
	ON_NOTIFY(MVN_GETEFFECT, IDC_PREVIEW, OnNotifyGetEffect)
	ON_BN_CLICKED(IDOK, OnOK)
	ON_BN_CLICKED(IDCANCEL, OnCancel)
	ON_BN_CLICKED(IDC_WHOLE, OnCheckWhole)
	ON_CBN_SELCHANGE(IDC_COLORPART, OnPartChanged)
END_MSG_MAP()

	BOOL OnInitDialog(LPARAM lParam);
	void OnHScroll(UINT nSBCode, UINT nPos, HWND hScrollBar);
	void OnNotifyGetEffect(NMHDR* pNmhdr, LRESULT* pResult);
	void OnOK() { EndDialog(IDOK); }
	void OnCancel() { EndDialog(IDCANCEL); }
	void OnCheckWhole();
	void OnPartChanged();
};

BOOL CModulateDlg::OnInitDialog(LPARAM lParam)
{
	CenterWindow();

	m_part = GetDlgItem(IDC_COLORPART);
	m_hslider = GetDlgItem(IDC_HSLIDER);
	m_sslider = GetDlgItem(IDC_SSLIDER);
	m_bslider = GetDlgItem(IDC_BSLIDER);
	m_view = GetDlgItem(IDC_PREVIEW);

	InitDlgCtrl(m_part, IDS_MODULATE_COLORPART);
	m_part.SetCurSel(0);

	m_hue_range = 0;
	m_hue = 0;
	m_sat = 0;
	m_brt = 0;

	COLORREF crs[7];
	m_hslider.SetRange(-180, 180);
	m_hslider.SetPos(0);
	crs[0] = RGB(255, 0, 0);
	crs[1] = RGB(255, 255, 0);
	crs[2] = RGB(0, 255, 0);
	crs[3] = RGB(0, 255, 255);
	crs[4] = RGB(0, 0, 255);
	crs[5] = RGB(255, 0, 255);
	crs[6] = RGB(255, 0, 0);
	m_hslider.SetBkColorsEx(7, crs);
	SetDlgItemInt(IDC_HUE, 0, TRUE);

	m_sslider.SetRange(-100, 100);
	m_sslider.SetPos(0);
	crs[0] = RGB(128, 128, 128);
	crs[1] = RGB(255, 255, 255);
	m_sslider.SetBkColorsEx(2, crs);
	SetDlgItemInt(IDC_SAT, 0, TRUE);

	m_bslider.SetRange(-100, 100);
	m_bslider.SetPos(0);
	crs[0] = RGB(0, 0, 0);
	crs[1] = RGB(255, 255, 255);
	m_bslider.SetBkColorsEx(2, crs);
	SetDlgItemInt(IDC_BRT, 0, TRUE);

	m_view.SendMessage(IVM_SETWHOLEVIEW, 0, 0);
	m_view.SendMessage(IVM_SETDIB, 0, (LPARAM)m_hdib);
	m_view.SendMessage(IVM_UPDATEEFFECT, TRUE, 0);

	return TRUE;
}

void CModulateDlg::OnHScroll(UINT nSBCode, UINT nPos, HWND hScrollBar)
{
	CDegreeCtrl bar = hScrollBar;
	int cur;
	switch (nSBCode) {
		case TB_TOP:
		case TB_BOTTOM:
		case TB_LINEUP:
		case TB_LINEDOWN:
		case TB_PAGEUP:
		case TB_PAGEDOWN:
			cur = bar.GetPos();
			break;

		case TB_THUMBTRACK:
			cur = (int)nPos;
			break;

		default:
			return;
	}

	if (hScrollBar == m_hslider) {
		SetDlgItemInt(IDC_HUE, cur, TRUE);
		m_hue = cur;
	}
	else if (hScrollBar == m_sslider) {
		SetDlgItemInt(IDC_SAT, cur, TRUE);
		m_sat = cur;
	}
	else if (hScrollBar == m_bslider) {
		SetDlgItemInt(IDC_BRT, cur, TRUE);
		m_brt = cur;
	}
	m_view.SendMessage(IVM_UPDATEEFFECT, TRUE, 0);
}

void CModulateDlg::OnPartChanged()
{
	m_hue_range = m_part.GetCurSel();
	m_view.SendMessage(IVM_UPDATEEFFECT, TRUE, 0);
}

void CModulateDlg::OnCheckWhole()
{
	BOOL whole = IsDlgButtonChecked(IDC_WHOLE);
	m_view.SendMessage(IVM_SETWHOLEVIEW, whole, 0);
	m_view.SendMessage(IVM_UPDATEEFFECT, TRUE, 0);
}

void CModulateDlg::OnNotifyGetEffect(NMHDR* pNmhdr, LRESULT* pResult)
{
	*pResult = FALSE;
	MV_GETEFFECT* pmg = (MV_GETEFFECT*)pNmhdr;
	if (pmg->hdibSrc) {
		pmg->hdibDst = hue_saturation(pmg->hdibSrc, NULL, m_hue_range, m_hue, m_sat, m_brt, NULL);
		*pResult = TRUE;
	}
}

////////////////////////////////////////////////////////////////////////
//

class DIBman {
public:
	HANDLE hdib;
	DIBman() : hdib(NULL) {}
	DIBman(HANDLE h) { hdib = h; }
	~DIBman() { if (hdib) GlobalFree(hdib);	}
	DIBman& operator=(HANDLE h) { hdib = h; return *this; }
	operator HANDLE() const { return hdib; }
};

HANDLE modulate_ui(HDIB hdib, LPRECT prc, FILTER_EXEC* pfe)
{
	if (hdib == NULL) return NULL;

	// クロップ
	DIBman prv = mifCrop(hdib, prc);
	CModulateDlg dlg;
	dlg.m_hdib = prv;
	if (dlg.DoModal() == IDOK)
		return hue_saturation(hdib, prc, dlg.m_hue_range, dlg.m_hue, dlg.m_sat, dlg.m_brt, pfe->progress);
	return NULL;
}
