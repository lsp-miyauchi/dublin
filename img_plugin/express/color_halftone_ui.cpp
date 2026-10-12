// color_halftone.cpp : インプリメンテーション ファイル
//

#include "stdafx.h"
#include "..\filterdialog.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

extern PROGRESSCALLBACK pfnProgress;

BOOL newsprint(mif& src, LPRECT prc, int cell_width, int oversample);

static HDIB WINAPI color_halftone(HDIB hdib, LPRECT prc, int cell_width, int oversample)
{
	mif image;
	if (image.attachdib(hdib) && newsprint(image, prc, cell_width, oversample))
		return image.createdib();
	return NULL;
}

/////////////////////////////////////////////////////////////////////////////
// CColorHalfToneDlg ダイアログ

class CColorHalfToneDlg : public CDialogImpl<CColorHalfToneDlg>, public CFilterDialogImpl<CColorHalfToneDlg>
{
public:
	CColorHalfToneDlg();

	enum { IDD = IDD_COLORHALFTONE };
	CTrackCtrl m_cslider;
	CTrackCtrl m_sslider;
	HDIB m_hdib;

	int m_cellwidth;
	int m_oversample;

BEGIN_MSG_MAP(CColorHalfToneDlg)
	CHAIN_MSG_MAP(CFilterDialogImpl<CColorHalfToneDlg>)
END_MSG_MAP()

	BOOL InitDialog(LPARAM lParam);
	void HScroll(int val, HWND hScrollBar);
	HDIB GetPreview(HDIB hdib);
};

CColorHalfToneDlg::CColorHalfToneDlg()
{
	m_cellwidth = 8;
	m_oversample = 4;
}

/////////////////////////////////////////////////////////////////////////////
// CColorHalfToneDlg メッセージ ハンドラ

BOOL CColorHalfToneDlg::InitDialog(LPARAM lParam)
{
	CenterWindow();
	m_cslider = GetDlgItem(IDC_CSLIDER);
	m_sslider = GetDlgItem(IDC_SSLIDER);

	m_cslider.SetRange(3, 60);
	m_cslider.SetPageSize(6);
	m_cslider.SetTicFreq(12);
	m_cslider.SetPos(m_cellwidth);

	m_sslider.SetRange(1, 15);
	m_sslider.SetPageSize(4);
	m_sslider.SetTicFreq(4);
	m_sslider.SetPos(m_oversample);

	SetPreviewImage(m_hdib);
	UpdatePreview();
	return TRUE;
}

void CColorHalfToneDlg::HScroll(int val, HWND hBar)
{
	if (hBar == m_cslider)		m_cellwidth = val;
	else if (hBar == m_sslider)	m_oversample = val;
	else						return;
	UpdatePreview();
}

HDIB CColorHalfToneDlg::GetPreview(HDIB hdib)
{
	return color_halftone(hdib, NULL, m_cellwidth, m_oversample);
}

//////////////////////////////////////////////////////////////////////
//

static TCHAR secRoot[] = _T("expression\\color_halftone");
static TCHAR keyLast[] = _T("last");

HANDLE color_halftone_ui(HDIB hdib, LPRECT prc, FILTER_EXEC* pfe)
{
	CColorHalfToneDlg dlg;

	regkey reg(pfe->hkey, secRoot);
	DWORD value = reg.getvalue(keyLast, MAKELONG(dlg.m_cellwidth, dlg.m_oversample));
	dlg.m_cellwidth = (int)LOWORD(value);
	dlg.m_oversample = (int)HIWORD(value);

	mif image(hdib);
	image.crop(prc);
	dlg.m_hdib = image.createdib();
	image.destroy();

	HDIB h = NULL;
	BOOL bOK = (dlg.DoModal(pfe->hparent) == IDOK);
	GlobalFree(dlg.m_hdib);
	if (bOK) {
		reg.setvalue(keyLast, MAKELONG(dlg.m_cellwidth, dlg.m_oversample));

		CWaitCursor wc;
pfnProgress = pfe->progress;
		h = color_halftone(hdib, prc, dlg.m_cellwidth, dlg.m_oversample);
pfnProgress = NULL;
	}
	return h;
}
