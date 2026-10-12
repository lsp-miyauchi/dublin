// wave_ring_ui.cpp : インプリメンテーション ファイル
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


#define WITHIN(a,b,c)			((((a)<=(b))&&((b)<=(c)))?1:0)
#define M_PI					(3.14159265358979323846)
#define	DEG2RAD(x)				((x) * M_PI / 180.0)

typedef struct tagWATERRING {
	int amplitude;						// default 10  range 0 - 101
	int phase;							// default 0   range 0 - 360
	int wavelength;						// default 10  range 1 - 100
	int wavetype;						// default 0   Type of waves, black/smeared
	POINT pt;
} WATERRING;


static RGBA __fastcall bilinear(double x, double y, RGBA* v)
{
	x = fmod(x, 1.0);
	y = fmod(y, 1.0);

	if (x < 0) x += 1.0;
	if (y < 0) y += 1.0;

	RGBA pix;
	pix.red = (BYTE)((1.0-y)*((1.0-x)*v[0].red+x*v[1].red)+y*((1.0-x)*v[2].red+x*v[3].red));
	pix.green = (BYTE)((1.0-y)*((1.0-x)*v[0].green+x*v[1].green)+y*((1.0-x)*v[2].green+x*v[3].green));
	pix.blue = (BYTE)((1.0-y)*((1.0-x)*v[0].blue+x*v[1].blue)+y*((1.0-x)*v[2].blue+x*v[3].blue));
	return pix;
}

static void __fastcall dowave(mif& src, CRect& rc, double amplitude, double wavelength, double phase, int smear, POINT pt)
{
	int    x, y;
	int    x1_in, y1_in, x2_in, y2_in;

	double cen_x, cen_y;       /* Center of wave */
	double xhsiz, yhsiz;       /* Half size of selection */
	double amnt, d;
	double needx, needy;
	double dx, dy;
	double xscale, yscale;

	int xi, yi;

	RGBA values[4];

	phase = DEG2RAD(phase);

	/* Wave the image.  For each pixel in the destination image
	 * which is inside the selection, we compute the corresponding
	 * waved location in the source image.  We use bilinear
	 * interpolation to avoid ugly jaggies.
	 *
	 * Let's assume that we are operating on a circular area.
	 * Every point within <radius> distance of the wave center is
	 * waveed to its destination position.
	 *
	 * Basically, introduce a wave into the image. I made up the
	 * forumla initially, but it looks good. Quartic added the
	 * wavelength etc. to it while I was asleep :) Just goes to show
	 * we should work with people in different time zones more.
	 *
	 */

	// Center of selection
//	cen_x = (double)(rc.right - 1 + rc.left) / 2.0;
//	cen_y = (double)(rc.bottom - 1 + rc.top) / 2.0;
	cen_x = (double)pt.x;
	cen_y = (double)pt.y;

	// Compute wave radii (semiaxes)
	xhsiz = (double)rc.Width() / 2.0;
	yhsiz = (double)rc.Height() / 2.0;

	// These are the necessary scaling factors to turn the wave
	//  ellipse into a large circle

	if (xhsiz < yhsiz) {
		xscale = yhsiz / xhsiz;
		yscale = 1.0;
	}
	else if (xhsiz > yhsiz) {
		xscale = 1.0;
		yscale = xhsiz / yhsiz;
	}
	else {
		xscale = 1.0;
		yscale = 1.0;
	}

	// Wave the image!
	wavelength = (wavelength * 2);

	progress_init(rc.Height() >> 2);

	mif dst(rc.Width(), rc.Height(), 24);
	RGBA* pd = dst.m_pixels;
	for (y = rc.top; y < rc.bottom; y++) {
		for (x = rc.left; x < rc.right; x++) {
			// Distance from current point to wave center, scaled
			dx = (x - cen_x) * xscale;
			dy = (y - cen_y) * yscale;

			// Distance^2 to center of *circle* (our scaled ellipse)
			d = sqrt(dx * dx + dy * dy);

			// Use the formula described above.
			// Calculate waved point and scale again to ellipsify
			// Reflective waves are strange - the effect is much
			// more like a mirror which is in the shape of
			// the wave than anything else.

			amnt = amplitude * sin(((d / wavelength) * (2.0 * M_PI) + phase));
			needx = (amnt + dx) / xscale + cen_x;
			needy = (amnt + dy) / yscale + cen_y;

			// Calculations complete; now copy the proper pixel

			xi = (int)needx;
			yi = (int)needy;

			if (smear)
			{
				if (xi > src.m_width - 2)
					xi = src.m_width - 2;
				else if (xi < 0)
					xi = 0;
				if (yi > src.m_height - 2)
					yi = src.m_height - 2;
				else if (yi < 0)
					yi = 0;
			}

			x1_in = WITHIN(0, xi, src.m_width - 1);
			y1_in = WITHIN(0, yi, src.m_height - 1);
			x2_in = WITHIN(0, xi + 1, src.m_width - 1);
			y2_in = WITHIN(0, yi + 1, src.m_height - 1);

			memset(values, 0, sizeof(values));
			if (x1_in && y1_in) values[0] = *src.pos(xi, yi);
			if (x2_in && y1_in) values[1] = *src.pos(xi + 1, yi);
			if (x1_in && y2_in) values[2] = *src.pos(xi, yi + 1);
			if (x2_in && y2_in) values[3] = *src.pos(xi + 1, yi + 1);

			*pd = bilinear(needx, needy, values);
			pd++;
		}
		if (!(y & 3))
			progress_continue();
	}
	src.blt(rc.left, rc.top, rc.Width(), rc.Height(), &dst, 0, 0);
	progress_term();
}

static HDIB wave_ring(HDIB hsrc, LPRECT prc, WATERRING* pwp)
{
	CRect rc;
	mif src;
	if (!src.attachdib(hsrc) || !src.validaterect(prc, &rc))
		return NULL;
	src.m_bits = 24;

	double amplitude;					// default 10.0  range 0.0 - 101.0
	double phase;						// default 0.0   range 0.0 - 360.0
	double wavelength;					// default 10.0  range 0.1 - 50.0
	int wavetype;						// default 0     Type of waves, black/smeared

	amplitude = (double)pwp->amplitude;
	phase = (double)pwp->phase;
	wavelength = (double)pwp->wavelength;
	wavetype = pwp->wavetype;

	dowave(src, rc, amplitude, wavelength, phase, wavetype == 0, pwp->pt);
	return src.createdib();
}


/////////////////////////////////////////////////////////////////////////////
// CWaterRingDlg ダイアログ


class CWaterRingDlg : public CDialogImpl<CWaterRingDlg>, public CFilterDialogImpl<CWaterRingDlg>
{
public:
	CWaterRingDlg();

	enum { IDD = IDD_WATER_RING };
	CTrackCtrl m_aslider;
	CTrackCtrl m_pslider;
	CTrackCtrl m_wslider;
	WATERRING m_wr;

BEGIN_MSG_MAP(CWaterRingDlg)
	CHAIN_MSG_MAP(CFilterDialogImpl<CWaterRingDlg>)
	ON_BN_CLICKED(IDC_BLACK, OnCheck)
	ON_BN_CLICKED(IDC_SMEARED, OnCheck)
END_MSG_MAP()

	void OnCheck();

	BOOL InitDialog(LPARAM lParam);
	void HScroll(int val, HWND hScrollBar);

	void SetValue();
};

CWaterRingDlg::CWaterRingDlg()
{
	m_wr.amplitude = 10;
	m_wr.phase = 0;
	m_wr.wavelength = 20;
	m_wr.wavetype = 0;
}

/////////////////////////////////////////////////////////////////////////////
// CWaterRingDlg メッセージ ハンドラ

BOOL CWaterRingDlg::InitDialog(LPARAM lParam) 
{
	m_aslider = GetDlgItem(IDC_AMPLITUDE);
	m_pslider = GetDlgItem(IDC_PHASE);
	m_wslider = GetDlgItem(IDC_WAVELENGTH);

	m_aslider.SetRange(1, 100);
	m_aslider.SetPageSize(10);
	m_aslider.SetTicFreq(10);
	m_aslider.SetPos(m_wr.amplitude);

	m_pslider.SetRange(0, 360);
	m_pslider.SetPageSize(30);
	m_pslider.SetTicFreq(45);
	m_pslider.SetPos(m_wr.phase);

	m_wslider.SetRange(1, 100);
	m_wslider.SetPageSize(10);
	m_wslider.SetTicFreq(10);
	m_wslider.SetPos(m_wr.wavelength);

	if (m_wr.wavetype)
		CheckDlgButton(IDC_BLACK, TRUE);
	else
		CheckDlgButton(IDC_SMEARED, TRUE);

	return TRUE;
}

void CWaterRingDlg::OnCheck()
{
	m_wr.wavetype = IsDlgButtonChecked(IDC_BLACK);
}

void CWaterRingDlg::HScroll(int val, HWND hBar)
{
	if (hBar == m_aslider)		m_wr.amplitude = val;
	else if (hBar == m_pslider)	m_wr.phase = val;
	else if (hBar == m_wslider)	m_wr.wavelength = val;
	else 						return;
}

//////////////////////////////////////////////////////////////////////
//

static TCHAR secRoot[] = _T("transform\\water_ring");
static TCHAR keyLast[] = _T("last");

HANDLE water_ring_ui(HDIB hdib, LPRECT prc, FILTER_EXEC* pfe)
{
	CWaterRingDlg dlg;

	WATERRING w;
	regkey reg(pfe->hkey, secRoot);
	if (reg.getbinary(keyLast, &w, sizeof(w)) == sizeof(w))
		memcpy(&dlg.m_wr, &w, sizeof(w));

	BOOL bOK = (dlg.DoModal(pfe->hparent) == IDOK);
	if (bOK) {
		reg.setbinary(keyLast, &dlg.m_wr, sizeof(dlg.m_wr));

		CWaitCursor wc;
pfnProgress = pfe->progress;
		HDIB h = wave_ring(hdib, prc, &dlg.m_wr);
pfnProgress = NULL;
		return h;
	}
	return NULL;
}
