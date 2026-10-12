// Contrast.cpp : インプリメンテーション ファイル
//

#include "stdafx.h"
#include "..\filterdialog.h"

HDIB __fastcall brightness_contrast(HDIB hdib, LPRECT prc, int brightness, int contrast)
{
	mif image;
	if (image.attachdib(hdib)) {
		image.bright_contrast(prc, brightness, contrast);
		return image.createdib();
	}
	return NULL;
}


/////////////////////////////////////////////////////////////////////////////
// CContrastDlg ダイアログ

class CContrastDlg : public CDialogImpl<CContrastDlg>, public CFilterDialogImpl<CContrastDlg>
{
public:
	CContrastDlg();

	enum { IDD = IDD_CONTRAST };
	int m_nBrightness;
	int m_nContrast;
	CTrackCtrl m_bslider;
	CTrackCtrl m_cslider;
	HDIB m_hdib;
	BOOL m_whole;

BEGIN_MSG_MAP(CContrastDlg)
	CHAIN_MSG_MAP(CFilterDialogImpl<CContrastDlg>)
	ON_BN_CLICKED(IDC_WHOLE, OnCheckWhole)
END_MSG_MAP()

	void OnCheckWhole();

	BOOL InitDialog(LPARAM lParam);
	void HScroll(int val, HWND hScrollBar);
	HDIB GetPreview(HDIB hdib);
};

CContrastDlg::CContrastDlg()
{
	m_nBrightness = 0;
	m_nContrast = 0;
	m_hdib = NULL;
	m_whole = TRUE;
}

/////////////////////////////////////////////////////////////////////////////
// CContrastDlg メッセージ ハンドラ

BOOL CContrastDlg::InitDialog(LPARAM lParam) 
{
	m_bslider = GetDlgItem(IDC_BSLIDER);
	m_cslider = GetDlgItem(IDC_CSLIDER);

	m_bslider.SetRange(-100, 100);
	m_bslider.SetPos(0);
	m_bslider.SetTicFreq(50);
	m_cslider.SetRange(-100, 100);
	m_cslider.SetPos(0);
	m_cslider.SetTicFreq(50);

	CheckDlgButton(IDC_WHOLE, m_whole);

	SetWholePreview(m_whole);
	SetPreviewImage(m_hdib);
	UpdatePreview();
	return TRUE;
}

void CContrastDlg::HScroll(int val, HWND hBar) 
{
	if (hBar == m_bslider)		m_nBrightness = val;
	else if (hBar == m_cslider)	m_nContrast = val;
	else						return;
	UpdatePreview();
}

HDIB CContrastDlg::GetPreview(HDIB hdib)
{
	return brightness_contrast(hdib, NULL, m_nBrightness, m_nContrast);
}

void CContrastDlg::OnCheckWhole() 
{
	m_whole = IsDlgButtonChecked(IDC_WHOLE);
	SetWholePreview(m_whole);
	UpdatePreview();
}

/////////////////////////////////////////////////////////////////////////////
//

HANDLE contrast_ui(HDIB hdib, LPRECT prc, FILTER_EXEC* pfe)
{
	if (hdib == NULL) return NULL;

	mif image(hdib);
	image.crop(prc);
	CContrastDlg dlg;
	dlg.m_hdib = image.createdib();
	image.destroy();

	BOOL bOK = (dlg.DoModal(pfe->hparent) == IDOK);
	GlobalFree(dlg.m_hdib);
	if (bOK)
		return brightness_contrast(hdib, prc, dlg.m_nBrightness, dlg.m_nContrast);
	return NULL;
}
