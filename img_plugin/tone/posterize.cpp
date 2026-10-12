// Posterize.cpp : インプリメンテーション ファイル
//

#include "stdafx.h"
#include "..\filterdialog.h"

/////////////////////////////////////////////////////////////////////////////
// CPosterizeDlg ダイアログ


class CPosterizeDlg : public CDialogImpl<CPosterizeDlg>, public CFilterDialogImpl<CPosterizeDlg>
{
public:
	CPosterizeDlg();

	enum { IDD = IDD_POSTERIZE };
	CTrackCtrl m_slider;
	HDIB m_hdib;
	int m_levels;
	BOOL m_whole;

BEGIN_MSG_MAP(CPosterizeDlg)
	CHAIN_MSG_MAP(CFilterDialogImpl<CPosterizeDlg>)
	ON_BN_CLICKED(IDC_WHOLE, OnCheckWhole)
END_MSG_MAP()

	void OnCheckWhole();

	BOOL InitDialog(LPARAM lParam);
	void HScroll(int val, HWND hScrollBar);
	HDIB GetPreview(HDIB hdib);
};


/////////////////////////////////////////////////////////////////////////////
// CPosterizeDlg メッセージ ハンドラ

CPosterizeDlg::CPosterizeDlg()
{
	m_hdib = NULL;
	m_whole = FALSE;
	m_levels = 8;
}

BOOL CPosterizeDlg::InitDialog(LPARAM lParam)
{
	// スライダーの設定
	m_slider = GetDlgItem(IDC_SLIDER);
	m_slider.SetRange(2, 50);
	m_slider.SetTicFreq(8);
	m_slider.SetPos(m_levels);

	CheckDlgButton(IDC_WHOLE, m_whole);
	SetWholePreview(m_whole);
	SetPreviewImage(m_hdib);
	UpdatePreview();
	return TRUE;
}

void CPosterizeDlg::HScroll(int val, HWND hScrollBar)
{
	if (hScrollBar == m_slider) {
		m_levels = val;
		UpdatePreview();
	}
}

HDIB CPosterizeDlg::GetPreview(HDIB hdib)
{
	mif image(hdib);
	image.posterize(NULL, m_levels);
	return image.createdib();
}

void CPosterizeDlg::OnCheckWhole()
{
	m_whole = IsDlgButtonChecked(IDC_WHOLE);
	SetWholePreview(m_whole);
	UpdatePreview();
}

////////////////////////////////////////////////////////////////////////////
//

static TCHAR secRoot[] = _T("tone\\posterize");
static TCHAR keyParam[] = _T("param");

HANDLE posterize_ui(HDIB hdib, LPRECT prc, FILTER_EXEC* pfe)
{
	if (!hdib) return NULL;

	regkey reg(pfe->hkey, secRoot);

	CPosterizeDlg dlg;
	dlg.m_levels = reg.getvalue(keyParam, dlg.m_levels);

	mif image(hdib);
	image.crop(prc);
	dlg.m_hdib = image.createdib();
	image.destroy();

imgEnableCallback(FALSE);
	BOOL bOK = (dlg.DoModal(pfe->hparent) == IDOK);
	GlobalFree(dlg.m_hdib);
imgEnableCallback(TRUE);

	if (bOK) {
		reg.setvalue(keyParam, dlg.m_levels);
		image.attachdib(hdib);
		image.posterize(prc, dlg.m_levels);
		return image.createdib();
	}
	return NULL;
}
