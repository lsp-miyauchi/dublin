// oilpaint.cpp : インプリメンテーション ファイル
//

#include "stdafx.h"
#include "..\filterdialog.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#define INTENSITY(r,g,b)	((DWORD)((DWORD)(r)*77+(DWORD)(g)*150+(DWORD)(b)*29)>>8)

extern PROGRESSCALLBACK pfnProgress;
void progress_init(int maxprogress);
void progress_continue();
void progress_term();

static BOOL run_oilpaint(mif& src, LPRECT prc, int rad)
{
	int count, k;
	register int i, j, x, y;
	register RGBA *p, *q, *s;

	CRect rc;
	if (!src.validaterect(prc, &rc))
		return NULL;
	src.m_bits = 24;

	int dx = rc.Width();
	int dy = rc.Height();

	mif dst(src);
	int histogram[256];
progress_init(rc.Height() >> 1);
	for (y = rc.top; y < rc.bottom; y++) {
		if (y < rad)
			continue;
		if (y >= (src.m_height - rad))
			break;

		p = dst.pos(rc.left, y);
		q = src.pos(rc.left, y);
		for (x = rc.left; x < rc.right; x++) {
			if (x < rad) {
				p++;
				q++;
				continue;
			}

			if (x >= (src.m_width - rad))
				break;

			// Determine most frequent color.
			count = 0;
			memset(histogram, 0, sizeof(int) * 256);
			for (i = 0; i < rad; i++) {
				s = p - (rad - i) * src.m_width - 1 - i;
				for (j = 0; j < (i + i + 1); j++) {
					k = INTENSITY(s->red, s->green, s->blue);
					histogram[k]++;
					if (histogram[k] > count) {
						*q = *s;
						count = histogram[k];
					}
					s++;
				}
				s = p + (rad - i) * src.m_width - 1 - i;
				for (j = 0; j < (i + i + 1); j++) {
					k = INTENSITY(s->red, s->green, s->blue);
					histogram[k]++;
					if (histogram[k] > count) {
						*q = *s;
						count = histogram[k];
					}
					s++;
				}
			}
			s = p - rad;
			for (j = 0; j < (rad + rad + 1); j++) {
				k = INTENSITY(s->red, s->green, s->blue);
				histogram[k]++;
				if (histogram[k] > count) {
					*q = *s;
					count = histogram[k];
				}
				s++;
			}
			q++;
			p++;
		}
if (y & 1) progress_continue();
	}
progress_term();
	return TRUE;
}

HDIB oilpaint(HDIB hSrc, LPRECT prc, int rad)
{
	mif src;
	if (src.attachdib(hSrc) && run_oilpaint(src, prc, rad))
		return src.createdib();
	return NULL;
}


/////////////////////////////////////////////////////////////////////////////
// COilPaintDlg ダイアログ

class COilPaintDlg : public CDialogImpl<COilPaintDlg>, public CFilterDialogImpl<COilPaintDlg>
{
public:
	COilPaintDlg() : m_amount(3) { }

	enum { IDD = IDD_OILPAINT };
	HDIB m_hdib;
	CTrackCtrl m_slider;
	int m_amount;

BEGIN_MSG_MAP(COilPaintDlg)
	CHAIN_MSG_MAP(CFilterDialogImpl<COilPaintDlg>)
END_MSG_MAP()

	BOOL InitDialog(LPARAM lParam);
	void HScroll(int val, HWND hScrollBar);
	HDIB GetPreview(HDIB hdib);
};

/////////////////////////////////////////////////////////////////////////////
// COilPaintDlg メッセージ ハンドラ

BOOL COilPaintDlg::InitDialog(LPARAM lParam) 
{
	m_slider = GetDlgItem(IDC_SLIDER);
	m_slider.SetRange(1, 8);
	m_slider.SetPos(m_amount);

	SetPreviewImage(m_hdib);
	UpdatePreview();
	return TRUE;
}

void COilPaintDlg::HScroll(int val, HWND hScrollBar)
{
	if (hScrollBar == m_slider) {
		m_amount = val;
		UpdatePreview();
	}
}

HDIB COilPaintDlg::GetPreview(HDIB hdib)
{
	return oilpaint(hdib, NULL, m_amount);
}

////////////////////////////////////////////////////////////////////////
//

static TCHAR secRoot[] = _T("artistic\\oilpaint");
static TCHAR keyLast[] = _T("last");

HANDLE oilpaint_ui(HDIB hdib, LPRECT prc, FILTER_EXEC* pfe)
{
	regkey reg(pfe->hkey, secRoot);

	COilPaintDlg dlg;
	dlg.m_amount = reg.getvalue(keyLast, 3);

	mif image(hdib);
	image.crop(prc);
	dlg.m_hdib = image.createdib();
	image.destroy();

	HDIB h = NULL;
	if (dlg.DoModal(pfe->hparent) == IDOK) {
		reg.setvalue(keyLast, dlg.m_amount);
		CWaitCursor wc;
pfnProgress = pfe->progress;
		h = oilpaint(hdib, prc, dlg.m_amount);
pfnProgress = NULL;
	}
	GlobalFree(dlg.m_hdib);
	return h;
}
