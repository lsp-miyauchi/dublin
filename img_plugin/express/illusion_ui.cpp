// illusion.cpp : インプリメンテーション ファイル
//

#include "stdafx.h"
#include "..\filterdialog.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

extern PROGRESSCALLBACK pfnProgress;
void progress_init(int maxprogress);
void progress_continue();
void progress_term();

#ifndef PI
#define PI 3.141592653589793238462643383279
#endif
#ifndef PI_2
#define PI_2 (PI*2)
#endif

static HDIB illusion(HDIB hsrc, LPRECT prc, int division)
{
	CRect rc;
	mif src;
	if (!src.attachdib(hsrc) || !src.validaterect(prc, &rc)) return NULL;

	int cx = rc.Width();
	int cy = rc.Height();
	if (src.m_bits < 24) src.m_bits = 24;

	double center_x = (double)cx / 2;
	double center_y = (double)cy / 2;

	RGBA** srcpix = (RGBA**)malloc(cy * sizeof(RGBA*));
	RGBA** dstpix = (RGBA**)malloc(cy * sizeof(RGBA*));
	int y;
	for (y = 0; y < cy; y++) {
		srcpix[y] = (RGBA*)malloc(cx * sizeof(RGBA));
		dstpix[y] = (RGBA*)malloc(cx * sizeof(RGBA));
		src.getrow(srcpix[y], rc.left, rc.top + y, cx);
	}

	double scale = sqrt((double)(cx * cx + cy * cy)) / 2;
	double offset = (int)(scale / 2);
	double div_pi2 = division / PI_2;
	double pi2_div = PI_2 / division;
	double pi_div = PI / division;

progress_init(cy >> 3);
	for (y = 0; y < cy; y++ ) {
		double dcy = ((double)y - center_y) / scale; 
		for (int x = 0; x < cx; x++) {
			double dcx = ((double)x - center_x) / scale;
			double angle = floor(atan2(dcy, dcx) * div_pi2) * pi2_div + pi_div;
			double radius = sqrt(dcx * dcx + dcy * dcy);
			int xx = x - offset * cos(angle);
			int yy = y - offset * sin(angle);
			xx = min(cx - 1, max(0, xx));
			yy = min(cy - 1, max(0, yy));

			dstpix[y][x].red = (1 - radius) * srcpix[y][x].red + radius * srcpix[yy][xx].red;
			dstpix[y][x].green = (1 - radius) * srcpix[y][x].green + radius * srcpix[yy][xx].green;
			dstpix[y][x].blue = (1 - radius) * srcpix[y][x].blue + radius * srcpix[yy][xx].blue;
		}
		src.setrow(dstpix[y], rc.left, rc.top + y, cx);

if (!(y & 7)) progress_continue();
	}
progress_term();

	for (y = 0; y < cy; y++ ) free(srcpix[y]);
	free(srcpix);
	for (y = 0; y < cy; y++) free(dstpix[y]);
	free(dstpix);

	return src.createdib();
}

/////////////////////////////////////////////////////////////////////////////
// CIllusionDlg ダイアログ

class CIllusionDlg : public CDialogImpl<CIllusionDlg>, public CFilterDialogImpl<CIllusionDlg>
{
public:
	CIllusionDlg();

	enum { IDD = IDD_ILLUSION };
	CTrackCtrl m_slider;
	int m_amount;
	HDIB m_hdib;

BEGIN_MSG_MAP(CIllusionDlg)
	CHAIN_MSG_MAP(CFilterDialogImpl<CIllusionDlg>)
END_MSG_MAP()

	BOOL InitDialog(LPARAM lParam);
	void HScroll(int val, HWND hScrollBar);
	HDIB GetPreview(HDIB hdib);

	void SetValue();
};

CIllusionDlg::CIllusionDlg()
{
	m_amount = 8;
	m_hdib = NULL;
}

/////////////////////////////////////////////////////////////////////////////
// CIllusionDlg メッセージ ハンドラ

BOOL CIllusionDlg::InitDialog(LPARAM lParam)
{
	m_slider = GetDlgItem(IDC_SLIDER);
	m_slider.SetRange(2, 32);
	m_slider.SetTicFreq(8);
	m_slider.SetPos(m_amount);

	SetWholePreview(TRUE);
	SetPreviewImage(m_hdib);
	UpdatePreview();
	return TRUE;
}

void CIllusionDlg::HScroll(int val, HWND hScrollBar)
{
	if (hScrollBar == m_slider) {
		m_amount = val;
		UpdatePreview();
	}
}

HDIB CIllusionDlg::GetPreview(HDIB hdib)
{
	return illusion(hdib, NULL, m_amount);
}

////////////////////////////////////////////////////////////////////////
//

static TCHAR secRoot[] = _T("expression\\iluusion");
static TCHAR keyLast[] = _T("last");

HANDLE illusion_ui(HDIB hdib, LPRECT prc, FILTER_EXEC* pfe)
{
	regkey reg(pfe->hkey, secRoot);

	CIllusionDlg dlg;
	dlg.m_amount = reg.getvalue(keyLast, dlg.m_amount);

	mif image(hdib);
	image.crop(prc);
	dlg.m_hdib = image.createdib();
	image.destroy();

	BOOL bOK = (dlg.DoModal(pfe->hparent) == IDOK);
	GlobalFree(dlg.m_hdib);
	if (bOK) {
		reg.setvalue(keyLast, dlg.m_amount);
		CWaitCursor wc;
pfnProgress = pfe->progress;
		HDIB h = illusion(hdib, prc, dlg.m_amount);
pfnProgress = NULL;
		return h;
	}
	return NULL;
}
