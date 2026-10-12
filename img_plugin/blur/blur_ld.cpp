// blur_ld.cpp : インプリメンテーション ファイル
//

#include "stdafx.h"
#include "..\filterdialog.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#define MIXED(a,b,c,d)		(((a)*(c)+(b)*((d)-(c)))/(d))

static inline int yuvY(RGBA* ps)
	{ return (2990 * (int)ps->red + 5870 * (int)ps->green + 1140 * (int)ps->blue) / 10000; }

static BOOL __fastcall blur_lightendarken(mif& src, LPRECT prc, double radius, BOOL lighten)
{
	CRect rc;
	if (!src.validaterect(prc, &rc)) return FALSE;
	src.m_bits = 24;

	int width = rc.Width();
	int height = rc.Height();

	mif dst(width, height, 24);
dst.m_progress = src.m_progress;
	dst.blt(0, 0, width, height, &src, rc.left, rc.top);
	dst.blur_gauss_rle(NULL, radius);

src.progress_init(rc.Height() >> 3);
int prog = 0;

	register RGBA* pd = dst.m_pixels;
	for (int y = rc.top; y < rc.bottom; y++) {
		register RGBA* ps = src.pos(rc.left, y);
		for (int x = rc.left; x < rc.right; x++) {
			int i = yuvY(ps);

			if (lighten) {
				ps->red = MIXED(pd->red, ps->red, i, 255);
				ps->green = MIXED(pd->green, ps->green, i, 255);
				ps->blue = MIXED(pd->blue, ps->blue, i, 255);
			}
			else {
				ps->red = MIXED(ps->red, pd->red, i, 255);
				ps->green = MIXED(ps->green, pd->green, i, 255);
				ps->blue = MIXED(ps->blue, pd->blue, i, 255);
			}
			ps++;
			pd++;
		}
if ((++prog & 7) == 0) src.progress_continue();
	}
src.progress_term();
	return TRUE;
}

HDIB __fastcall blur_ld(HDIB hdib, LPRECT prc, double radius, BOOL lighten, PROGRESSCALLBACK progress)
{
	mif image;
	if (image.attachdib(hdib)) {
		image.m_progress = progress;
		if (blur_lightendarken(image, prc, radius, lighten)) return image.createdib();
	}
	return NULL;
}

/////////////////////////////////////////////////////////////////////////////
// CBlurLDDlg ダイアログ

typedef struct tagBLURLD {
	int radius;
	BOOL lighten;
} BLURLD;


class CBlurLDDlg : public CDialogImpl<CBlurLDDlg>, public CFilterDialogImpl<CBlurLDDlg>
{
public:
	CBlurLDDlg();

	enum { IDD = IDD_BLUR_LD };
	CTrackCtrl m_slider;
	HDIB m_hdib;
	int m_radius;
	BOOL m_lighten;

BEGIN_MSG_MAP(CBlurLDDlg)
	CHAIN_MSG_MAP(CFilterDialogImpl<CBlurLDDlg>)
	ON_BN_CLICKED(IDC_LIGHTEN, OnCheckLighten)
	ON_BN_CLICKED(IDC_DARKEN, OnCheckLighten)
END_MSG_MAP()

	void OnCheckLighten();

	BOOL InitDialog(LPARAM lParam);
	void HScroll(int val, HWND hScrollBar);
	HDIB GetPreview(HDIB hdib);
};

CBlurLDDlg::CBlurLDDlg()
{
	m_radius = 16;
	m_lighten = TRUE;
}

/////////////////////////////////////////////////////////////////////////////
// CBlurLDDlg メッセージ ハンドラ

BOOL CBlurLDDlg::InitDialog(LPARAM lParam) 
{
	m_slider = GetDlgItem(IDC_SLIDER);

	m_slider.SetRange(1, 100);
	m_slider.SetPageSize(5);
	m_slider.SetTicFreq(25);
	m_slider.SetPos(m_radius);
	m_slider.SetBuddyBase(1);

	CheckDlgButton(m_lighten ? IDC_LIGHTEN : IDC_DARKEN, TRUE);

	SetPreviewImage(m_hdib);
	UpdatePreview();

	return TRUE;
}

void CBlurLDDlg::OnCheckLighten() 
{
	m_lighten = IsDlgButtonChecked(IDC_LIGHTEN);
	UpdatePreview();
}

void CBlurLDDlg::HScroll(int val, HWND hScrollBar)
{
	if (::GetDlgCtrlID(hScrollBar) == IDC_SLIDER) {
		m_radius = val;
		UpdatePreview();
	}
}

HDIB CBlurLDDlg::GetPreview(HDIB hdib)
{
	return blur_ld(hdib, NULL, (double)m_radius/10.0, m_lighten, NULL);
}

///////////////////////////////////////////////////////////////////////
//
static TCHAR sszKey[] = _T("GaussLightenDarken");

HANDLE blur_gauss_ld_ui(HDIB hdib, LPRECT prc, FILTER_EXEC* pfe)
{
	CBlurLDDlg dlg;

	mif image(hdib);
	image.crop(prc);
	dlg.m_hdib = image.createdib();
	image.destroy();

	regkey reg(pfe->hkey, _T("blur\\blur_ld"));
	BLURLD bld;
	if (reg.getbinary(sszKey, &bld, sizeof(bld)) == sizeof(bld)) {
		dlg.m_radius = bld.radius;
		dlg.m_lighten = bld.lighten;
	}

	BOOL bOK = (dlg.DoModal(pfe->hparent) == IDOK);
	GlobalFree(dlg.m_hdib);
	if (bOK) {
		bld.radius = dlg.m_radius;
		bld.lighten = dlg.m_lighten;
		reg.setbinary(sszKey, &bld, sizeof(bld));

		CWaitCursor wc;
		return blur_ld(hdib, prc, (double)dlg.m_radius / 10.0, dlg.m_lighten, pfe->progress);
	}
	return NULL;
}
