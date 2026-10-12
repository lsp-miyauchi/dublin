// blur_simple.cpp : インプリメンテーション ファイル
//

#include "stdafx.h"
#include "..\filterdialog.h"


HDIB __fastcall blur(HDIB hdib, LPRECT prc, int radius)
{
	mif image;
	if (image.attachdib(hdib) && image.blur_simple(prc, radius))
		return image.createdib();
	return NULL;
}

/////////////////////////////////////////////////////////////////////////////
// CBlurSimpleDlg ダイアログ

class CBlurSimpleDlg : public CDialogImpl<CBlurSimpleDlg>, public CFilterDialogImpl<CBlurSimpleDlg>
{
public:
	CBlurSimpleDlg();

	enum { IDD = IDD_BLUR };
	CTrackCtrl m_slider;
	int m_radius;
	HDIB m_hdib;

BEGIN_MSG_MAP(CBlurSimpleDlg)
	CHAIN_MSG_MAP(CFilterDialogImpl<CBlurSimpleDlg>)
END_MSG_MAP()

	BOOL InitDialog(LPARAM lParam);
	void HScroll(int val, HWND hScrollBar);
	HDIB GetPreview(HDIB hdib);
};


CBlurSimpleDlg::CBlurSimpleDlg()
{
	m_radius = 3;
}

/////////////////////////////////////////////////////////////////////////////
// CBlurSimpleDlg メッセージ ハンドラ

BOOL CBlurSimpleDlg::InitDialog(LPARAM lParam) 
{
	m_slider = GetDlgItem(IDC_SLIDER);

	m_slider.SetRange(1, 50);
	m_slider.SetPageSize(5);
	m_slider.SetTicFreq(10);
	m_slider.SetPos(m_radius);

	SetPreviewImage(m_hdib);
	UpdatePreview();
	return TRUE;
}

void CBlurSimpleDlg::HScroll(int val, HWND hScrollBar)
{
	if (::GetDlgCtrlID(hScrollBar) == IDC_SLIDER) {
		m_radius = val;
		UpdatePreview();
	}
}

HDIB CBlurSimpleDlg::GetPreview(HDIB hdib)
{
	return blur(hdib, NULL, m_radius);
}

///////////////////////////////////////////////////////////////////////
//
static TCHAR sszKey[] = _T("BlurSimple");

HANDLE blur_simple_ui(HDIB hdib, LPRECT prc, FILTER_EXEC* pfe)
{
	CBlurSimpleDlg dlg;

	mif image(hdib);
	image.crop(prc);
	dlg.m_hdib = image.createdib();
	image.destroy();

	regkey reg(pfe->hkey, _T("blur\\blur_simple"));
	dlg.m_radius = reg.getvalue(sszKey, 3);

	BOOL bOK = (dlg.DoModal(pfe->hparent) == IDOK);
	GlobalFree(dlg.m_hdib);
	if (bOK) {
		reg.setvalue(sszKey, dlg.m_radius);

		CWaitCursor wc;
		return blur(hdib, prc, dlg.m_radius);
	}
	return NULL;
}
