// colorbalance.cpp : インプリメンテーション ファイル
//

#include "stdafx.h"
#include "..\filterdialog.h"

enum {
	SHADOWS,
	MIDTONES,
	HIGHLIGHTS
};

static HDIB colorbalance(HDIB hsrc, LPRECT prc, MIF_COLORBALANCE* pcb)
{
	mif image;
	if (image.attachdib(hsrc) && image.colorbalance(prc, pcb))
		return image.createdib();
	return NULL;
}

/////////////////////////////////////////////////////////////////////////////
// CColorBalanceDlg ダイアログ

class CColorBalanceDlg : public CDialogImpl<CColorBalanceDlg>, public CFilterDialogImpl<CColorBalanceDlg>
{
public:
	CColorBalanceDlg();

	enum { IDD = IDD_COLORBALANCE };
	CTrackCtrl m_bslider;
	CTrackCtrl m_gslider;
	CTrackCtrl m_rslider;
	MIF_COLORBALANCE m_cb;
	int m_tone;
	HDIB m_hdib;
	BOOL m_whole;

BEGIN_MSG_MAP(CColorBalanceDlg)
	CHAIN_MSG_MAP(CFilterDialogImpl<CColorBalanceDlg>)
	ON_BN_CLICKED(IDC_LUM, OnLum)
	ON_BN_CLICKED(IDC_SHADOW, OnToneChange)
	ON_BN_CLICKED(IDC_MIDTONE, OnToneChange)
	ON_BN_CLICKED(IDC_HILIGHT, OnToneChange)
	ON_BN_CLICKED(IDC_WHOLE, OnCheckWhole)
END_MSG_MAP()

	void OnToneChange();
	void OnLum();
	void OnCheckWhole();

	BOOL InitDialog(LPARAM lParam);
	void HScroll(int val, HWND hScrollBar);
	HDIB GetPreview(HDIB hdib);

	void SetValue();
};

CColorBalanceDlg::CColorBalanceDlg()
{
	for (int i = 0; i < 3; i++) {
		m_cb.cyan_red[i] = 0;
		m_cb.magenta_green[i] = 0;
		m_cb.yellow_blue[i] = 0;
	}
	m_cb.preserve_lum = FALSE;
	m_tone = MIDTONES;
	m_whole = FALSE;
}

/////////////////////////////////////////////////////////////////////////////
// CColorBalanceDlg メッセージ ハンドラ

BOOL CColorBalanceDlg::InitDialog(LPARAM lParam)
{
	m_bslider = GetDlgItem(IDC_BSLIDER);
	m_gslider = GetDlgItem(IDC_GSLIDER);
	m_rslider = GetDlgItem(IDC_RSLIDER);

	m_rslider.SetRange(-100, 100);
	m_rslider.SetTicFreq(50);
	m_gslider.SetRange(-100, 100);
	m_gslider.SetTicFreq(50);
	m_bslider.SetRange(-100, 100);
	m_bslider.SetTicFreq(50);

	if (m_tone == SHADOWS)			CheckDlgButton(IDC_SHADOW, TRUE);
	else if (m_tone == MIDTONES)	CheckDlgButton(IDC_MIDTONE, TRUE);
	else							CheckDlgButton(IDC_HILIGHT, TRUE);
	SetValue();

	CheckDlgButton(IDC_WHOLE, m_whole);

	SetWholePreview(m_whole);
	SetPreviewImage(m_hdib);
	UpdatePreview();
	return TRUE;
}

void CColorBalanceDlg::HScroll(int val, HWND hBar)
{
	if (hBar == m_rslider)		m_cb.cyan_red[m_tone] = val;
	else if (hBar == m_gslider)	m_cb.magenta_green[m_tone] = val;
	else if (hBar == m_bslider)	m_cb.yellow_blue[m_tone] = val;
	else						return;
	UpdatePreview();
}

void CColorBalanceDlg::OnToneChange() 
{
	if (IsDlgButtonChecked(IDC_SHADOW))			m_tone = SHADOWS;
	else if (IsDlgButtonChecked(IDC_MIDTONE))	m_tone = MIDTONES;
	else										m_tone = HIGHLIGHTS;

	SetValue();
	UpdatePreview();
}

void CColorBalanceDlg::OnLum() 
{
	m_cb.preserve_lum = IsDlgButtonChecked(IDC_LUM);
	UpdatePreview();
}

void CColorBalanceDlg::SetValue()
{
	m_rslider.SetPos(m_cb.cyan_red[m_tone]);
	m_gslider.SetPos(m_cb.magenta_green[m_tone]);
	m_bslider.SetPos(m_cb.yellow_blue[m_tone]);
}

HDIB CColorBalanceDlg::GetPreview(HDIB hdib)
{
	return colorbalance(hdib, NULL, &m_cb);
}

void CColorBalanceDlg::OnCheckWhole()
{
	m_whole = IsDlgButtonChecked(IDC_WHOLE);
	SetWholePreview(m_whole);
	UpdatePreview();
}

/////////////////////////////////////////////////////////////////////////////
//

HANDLE color_balance_ui(HDIB hdib, LPRECT prc, FILTER_EXEC* pfe)
{
	if (!hdib) return NULL;

	CColorBalanceDlg dlg;

	mif image(hdib);
	image.crop(prc);
	dlg.m_hdib = image.createdib();
	image.destroy();

	BOOL bOK = (dlg.DoModal(pfe->hparent) == IDOK);
	GlobalFree(dlg.m_hdib);
	if (bOK) {
		return colorbalance(hdib, prc, &dlg.m_cb);
	}
	return NULL;
}
