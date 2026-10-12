// Gamma.cpp : インプリメンテーション ファイル
//

#include "stdafx.h"
#include "resource.h"

void InitDlgCtrl(HWND hctl, UINT idResource);

/////////////////////////////////////////////////////////////////////////////
//

HDIB gamma_rgb(HDIB hSrc, double r_amt, double g_amt, double b_amt, double rgb_amt)
{
	mif src;
	if (src.attachdib(hSrc) && src.gamma_rgb(NULL, r_amt, g_amt, b_amt, rgb_amt))
		return src.createdib();
	return NULL;
}

/////////////////////////////////////////////////////////////////////////////
// CGammaDlg ダイアログ

COLORREF rgbShade[] = {
	RGB(255, 255, 255),
	RGB(255,   0,   0),
	RGB(  0, 255,   0),
	RGB(  0,   0, 255),
};

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

CGammaDlg::CGammaDlg()
{
	m_nChannel = 0;
	m_posRGB = 0;
	m_posRed = 0;
	m_posGreen = 0;
	m_posBlue = 0;
	m_dbRGB = 1.0;
	m_dbRed = 1.0;
	m_dbGreen = 1.0;
	m_dbBlue = 1.0;
	m_hdib = NULL;
	m_whole = FALSE;
}

/////////////////////////////////////////////////////////////////////////////
// CGammaDlg メッセージ ハンドラ

BOOL CGammaDlg::OnInitDialog(LPARAM lParam) 
{
	CenterWindow();

	m_combo = GetDlgItem(IDC_COMBO);
	InitDlgCtrl(m_combo, IDS_TYPE_RGB);
	m_slider = GetDlgItem(IDC_SLIDER);
	m_view = GetDlgItem(IDC_PREVIEW);

	if (!m_hdib) {
		EndDialog(IDCANCEL);
		return TRUE;
	}

	m_slider.SetRange(-80, 80);
	m_slider.SetPos(0);

	m_combo.SetCurSel(0);
	SetValue(0, 0);

	BITMAPINFOHEADER* pbh = (BITMAPINFOHEADER*)GlobalLock(m_hdib);
	if (pbh->biBitCount < 24)
		m_combo.EnableWindow(FALSE);
	GlobalUnlock(m_hdib);

	CheckDlgButton(IDC_WHOLE, m_whole);
	m_view.SendMessage(IVM_SETWHOLEVIEW, m_whole);
	m_view.SendMessage(IVM_SETDIB, 0, (LPARAM)m_hdib);

	return TRUE;
}

void CGammaDlg::OnHScroll(UINT nSBCode, UINT nPos, HWND hScrollBar) 
{
	int nCur;
	switch (nSBCode) {
		case TB_TOP:
		case TB_BOTTOM:
		case TB_LINEUP:
		case TB_LINEDOWN:
		case TB_PAGEUP:
		case TB_PAGEDOWN:
			nCur = m_slider.GetPos();
			break;

		case TB_THUMBTRACK:
			nCur = (int)nPos;
			break;

		default:
			DefWindowProc();
			return;
	}
	SetValue(m_nChannel, nCur);
}

void CGammaDlg::SetValue(int nChannel, int nAmount)
{
	double dbm;
	if (nAmount == 0)		dbm = 1.0;
	else if (nAmount > 0)	dbm = 1.0 + 0.05 * (double)nAmount;
	else					dbm = 1.0 + 0.01 * (double)nAmount;

	TCHAR buf[MAX_PATH];
	_stprintf(buf, _T("%.2f"), dbm);
	SetDlgItemText(IDC_EDIT, buf);
	GetDlgItem(IDC_EDIT).UpdateWindow();

	UINT nID;
	if (nChannel == 0) {
		m_posRGB = nAmount;
		m_dbRGB = dbm;
		nID = IDC_RGB;
	}
	else if (nChannel == 1) {
		m_posRed = nAmount;
		m_dbRed = dbm;
		nID = IDC_RED;
	}
	else if (nChannel == 2) {
		m_posGreen = nAmount;
		m_dbGreen = dbm;
		nID = IDC_GREEN;
	}
	else {
		m_posBlue = nAmount;
		m_dbBlue = dbm;
		nID = IDC_BLUE;
	}
	SetDlgItemText(nID, buf);
	GetDlgItem(nID).UpdateWindow();

	m_slider.SetBkColors(RGB(0, 0, 0), rgbShade[nChannel]);
	m_view.SendMessage(IVM_UPDATEEFFECT, TRUE, 0);
}

void CGammaDlg::OnSelChange() 
{
	int pos;
	m_nChannel = m_combo.GetCurSel();
	switch (m_nChannel) {
		case 0:					// RGB
			pos = m_posRGB;
			break;

		case 1:					// 赤
			pos = m_posRed;
			break;

		case 2:					// 緑
			pos = m_posGreen;
			break;

		case 3:					// 青
			pos = m_posBlue;
			break;

		default:
			return;
	}

	m_slider.SetPos(pos);
	SetValue(m_nChannel, pos);
}

void CGammaDlg::OnNotifyGetEffect(NMHDR* pNmhdr, LRESULT* pResult)
{
	*pResult = FALSE;

	MV_GETEFFECT* pmg = (MV_GETEFFECT*)pNmhdr;
	if (pmg->hdibSrc) {
		pmg->hdibDst = gamma_rgb(pmg->hdibSrc, m_dbRed, m_dbGreen, m_dbBlue, m_dbRGB);
		*pResult = TRUE;
	}
}

void CGammaDlg::OnCheckWhole()
{
	m_whole = IsDlgButtonChecked(IDC_WHOLE);
	m_view.SendMessage(IVM_SETWHOLEVIEW, m_whole);
	m_view.SendMessage(IVM_UPDATEEFFECT, TRUE, 0);
}

////////////////////////////////////////////////////////////////////////
//

HANDLE gamma_ui(HDIB hdib, LPRECT prc, FILTER_EXEC* pfe)
{
	if (hdib == NULL) return NULL;

	CGammaDlg dlg;
	dlg.m_hdib = hdib;
	if (dlg.DoModal(pfe->hparent) == IDOK)
		return gamma_rgb(hdib, dlg.m_dbRed, dlg.m_dbGreen, dlg.m_dbBlue, dlg.m_dbRGB);

	return NULL;
}
