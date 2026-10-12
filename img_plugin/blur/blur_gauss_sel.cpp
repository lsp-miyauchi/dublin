// BlurGaussSel.cpp : インプリメンテーション ファイル
//

#include "stdafx.h"
#include "..\filterdialog.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

typedef struct _GAUSSSEL {
	int radius;
	int maxdelta;
} GAUSSSEL;

static HDIB gauss_selective(HDIB hsrc, LPRECT prc, double radius, int maxdelta, PROGRESSCALLBACK progress)
{
	// 1.0 <= radius <= 30.0
	// 0 <= maxdelta <= 255
	mif image;
	if (image.attachdib(hsrc)) {
		image.m_progress = progress;
		if (image.blur_gauss_sel(prc, radius, maxdelta)) return image.createdib();
	}
	return NULL;
}

/////////////////////////////////////////////////////////////////////////////
// CBlurGaussSelDlg ダイアログ

class CBlurGaussSelDlg : public CDialogImpl<CBlurGaussSelDlg>, public CFilterDialogImpl<CBlurGaussSelDlg>
{
public:
	CBlurGaussSelDlg();

	enum { IDD = IDD_BLURGAUSS_SEL };
	CTrackCtrl m_rslider;
	CTrackCtrl m_dslider;

	double m_radius;
	int m_maxdelta;
	HDIB m_hdib;

BEGIN_MSG_MAP(CBlurGaussSelDlg)
	CHAIN_MSG_MAP(CFilterDialogImpl<CBlurGaussSelDlg>)
END_MSG_MAP()

	BOOL InitDialog(LPARAM lParam);
	void HScroll(int val, HWND hScrollBar);
	HDIB GetPreview(HDIB hdib);
};

CBlurGaussSelDlg::CBlurGaussSelDlg()
{
	m_radius = 5.0;
	m_maxdelta = 128;
}

/////////////////////////////////////////////////////////////////////////////
// CBlurGaussSelDlg メッセージ ハンドラ

BOOL CBlurGaussSelDlg::InitDialog(LPARAM lParam) 
{
	m_rslider = GetDlgItem(IDC_RSLIDER);
	m_dslider = GetDlgItem(IDC_DSLIDER);

	m_rslider.SetRange(10, 300);
	m_rslider.SetPageSize(40);
	m_rslider.SetTicFreq(40);
	m_rslider.SetPos((int)(m_radius * 10.0));
	m_rslider.SetBuddyBase(1);

	m_dslider.SetRange(0, 255);
	m_dslider.SetPageSize(40);
	m_dslider.SetTicFreq(64);
	m_dslider.SetPos(m_maxdelta);

	SetPreviewImage(m_hdib);
	UpdatePreview();
	return TRUE;
}

void CBlurGaussSelDlg::HScroll(int val, HWND hBar) 
{
	if (hBar == m_rslider)		m_radius = (double)val / 10.0;
	else if (hBar == m_dslider)	m_maxdelta = val;
	else						return;
	UpdatePreview();
}

HDIB CBlurGaussSelDlg::GetPreview(HDIB hdib)
{
	return gauss_selective(hdib, NULL, m_radius, m_maxdelta, NULL);
}

///////////////////////////////////////////////////////////////////////
//

static TCHAR sszKey[] = _T("GaussSelective");

HANDLE blur_gauss_sel_ui(HDIB hdib, LPRECT prc, FILTER_EXEC* pfe)
{
	CBlurGaussSelDlg dlg;

	mif image(hdib);
	image.crop(prc);
	dlg.m_hdib = image.createdib();
	image.destroy();

	regkey reg(pfe->hkey, _T("blur\\blur_gauss_sel"));
	GAUSSSEL gs;
	if (reg.getbinary(sszKey, &gs, sizeof(gs)) == sizeof(gs)) {
		dlg.m_radius = (double)gs.radius / 10.0;
		dlg.m_maxdelta = gs.maxdelta;
	}

	BOOL bOK = (dlg.DoModal(pfe->hparent) == IDOK);
	GlobalFree(dlg.m_hdib);
	if (bOK) {
		gs.radius = (int)(dlg.m_radius * 10.0);
		gs.maxdelta = dlg.m_maxdelta;
		reg.setbinary(sszKey, &gs, sizeof(gs));

		CWaitCursor wc;
		return gauss_selective(hdib, prc, dlg.m_radius, dlg.m_maxdelta, pfe->progress);
	}
	return NULL;
}
