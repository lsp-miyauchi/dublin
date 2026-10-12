//////////////////////////////////////////////////////////////////////
// frame_fuzzy.cpp

#include "stdafx.h"
#include "..\filterdialog.h"

extern PROGRESSCALLBACK pfnProgress;
void progress_init(int maxprogress);
void progress_continue();
void progress_term();

typedef struct tagFRAMEFUZZY {
	int size;
	int spread_radius;
	int gauss;
	COLORREF frame_color;
} FRAMEFUZZY;

#define MIXRGB(a,b,c) (((int)(a)*(255-(c))+(int)(b)*(c))/255)

BOOL __fastcall frame_fuzzy(mif& src, LPRECT prc, FRAMEFUZZY* pf)
{
	CRect rc;
	if (!src.validaterect(prc, &rc))
		return FALSE;

	src.m_bits = 24;
	mif dst;
	dst.create(rc.Width(), rc.Height(), 24);
	CRect rcw(0, 0, dst.m_width, dst.m_height);
	rcw.DeflateRect(pf->size, pf->size);
	dst.solidfill(&rcw, RGB(255, 255, 255), 255);
	dst.spread(NULL, pf->spread_radius);
	dst.copyplane(planeRed, planeAlpha);
	if (pf->gauss > 0)
		dst.blur_gauss_rle_plane(NULL, (double)pf->gauss / 10.0, bltAlpha);

	int r = GetRValue(pf->frame_color);
	int g = GetGValue(pf->frame_color);
	int b = GetBValue(pf->frame_color);

	RGBA* pd = dst.m_pixels;
	for (int y = 0; y < rc.Height(); y++) {
		RGBA* ps = src.pos(rc.left, y + rc.top);
		for (int x = 0; x < rc.Width(); x++) {
			ps->red = MIXRGB(r, ps->red, pd->alpha);
			ps->green = MIXRGB(g, ps->green, pd->alpha);
			ps->blue = MIXRGB(b, ps->blue, pd->alpha);
			ps++;
			pd++;
		}
	}
	return TRUE;
}

/////////////////////////////////////////////////////////////////////
//

class CFuzzyFrameDlg : public CDialogImpl<CFuzzyFrameDlg>, public CFilterDialogImpl<CFuzzyFrameDlg>
{
public:
	CFuzzyFrameDlg();
	enum { IDD = IDD_FUZZYFRAME };
	FRAMEFUZZY m_fz;

BEGIN_MSG_MAP(CFuzzyFrameDlg)
	CHAIN_MSG_MAP(CFilterDialogImpl<CFuzzyFrameDlg>)
END_MSG_MAP()

	BOOL InitDialog(LPARAM lParam);
	void HScroll(int val, HWND hScrollBar);
};

CFuzzyFrameDlg::CFuzzyFrameDlg()
{
	m_fz.size = 16;
	m_fz.spread_radius = 8;
	m_fz.gauss = 16;
	m_fz.frame_color = RGB(255, 255, 255);
}

BOOL CFuzzyFrameDlg::InitDialog(LPARAM lParam)
{
	CTrackCtrl track = GetDlgItem(IDC_SSLIDER);
	track.SetRange(4, 50);
	track.SetPageSize(5);
	track.SetTicFreq(5);
	track.SetPos(m_fz.size);

	track = GetDlgItem(IDC_ASLIDER);
	track.SetRange(1, 16);
	track.SetPageSize(4);
	track.SetTicFreq(5);
	track.SetPos(m_fz.spread_radius);

	track = GetDlgItem(IDC_GSLIDER);
	track.SetRange(0, 160);
	track.SetPageSize(20);
	track.SetTicFreq(20);
	track.SetPos(m_fz.gauss);
	track.SetBuddyBase(1);

	ColorPicker_SetBkColor(GetDlgItem(IDC_COLOR), m_fz.frame_color);
	return TRUE;
}

void CFuzzyFrameDlg::HScroll(int val, HWND hScrollBar)
{
	switch (::GetDlgCtrlID(hScrollBar)) {
		case IDC_ASLIDER: m_fz.spread_radius = val; break;
		case IDC_SSLIDER: m_fz.size = val; break;
		case IDC_GSLIDER: m_fz.gauss = val; break;
		default: return;
	}
}

//////////////////////////////////////////////////////////////////////
//

static TCHAR secRoot[] = _T("frame\\frame_fuzzy");
static TCHAR keyLast[] = _T("last");

HANDLE framefuzzy_ui(HDIB hdib, LPRECT prc, FILTER_EXEC* pfe)
{
	CFuzzyFrameDlg dlg;
	FRAMEFUZZY fz;
	regkey reg(pfe->hkey, secRoot);
	if (reg.getbinary(keyLast, &fz, sizeof(fz)) == sizeof(fz))
		memcpy(&dlg.m_fz, &fz, sizeof(fz));

	if (dlg.DoModal(pfe->hparent) == IDOK) {
		CWaitCursor wc;
pfnProgress = pfe->progress;
		mif image;
		BOOL bOK = (image.attachdib(hdib) && frame_fuzzy(image, prc, &dlg.m_fz));
pfnProgress = NULL;
		if (bOK) {
			reg.setbinary(keyLast, &dlg.m_fz, sizeof(dlg.m_fz));
			return image.createdib();
		}
	}
	return NULL;
}

