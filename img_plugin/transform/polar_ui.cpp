// polar_ui.cpp : インプリメンテーション ファイル
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


#define M_PI				(3.14159265358979323846)
#define sqr(x)				((x) * (x))
#define WITHIN(a, b, c)		((((a) <= (b)) && ((b) <= (c))) ? 1 : 0)

typedef struct tagPOLAR {
	double circle;				// 0.0 - 100.0  Circle depth in %
	double angle;				// 0.0 - 360.0  Offset angle
	BOOL backwards;				// Map backwards?
	BOOL inverse;				// Map from top?
	BOOL polrec;				// Polar to rectangular?
} POLAR;

typedef struct tagPOLARCONFIG {
	POLAR pcvals;
	int width;
	int height;
	double cen_x;
	double cen_y;
} POLARCONFIG;

static void get_pixel(mif& image, int x, int y, RGBA* pixel)
{
	if (0 <= x && x < image.m_width && 0 <= y && y < image.m_height)
		*pixel = *(image.pos(x, y));
	else
		pixel->red = pixel->green = pixel->blue = 255;
}

static BYTE bilinear(double x, double y, BYTE *values)
{
	x = fmod(x, 1.0);
	y = fmod(y, 1.0);
	if (x < 0.0) x += 1.0;
	if (y < 0.0) y += 1.0;
	double m0 = (double)values[0] + x * ((double)values[1] - values[0]);
	double m1 = (double)values[2] + x * ((double)values[3] - values[2]);

	return (BYTE)(m0 + y * (m1 - m0));
}

static int calc_undistorted_coords(POLARCONFIG& cfg, double wx, double wy, double *x, double *y)
{
	int    inside;
	double phi2;
	double xx, yy;
	double m;
	double xmax, ymax, rmax;
	double x_calc, y_calc;
	double xi, yi;
	double t;

	/* initialize */
	double phi = 0.0;
	double r = 0.0;

	int x1 = 0;
	int y1 = 0;
	int x2 = cfg.width;
	int y2 = cfg.height;
	int xdiff = x2 - x1;
	int ydiff = y2 - y1;
	double xm = xdiff / 2.0;
	double ym = ydiff / 2.0;
	double circle = cfg.pcvals.circle;
	double angl = cfg.pcvals.angle / 180.0 * M_PI;

	if (cfg.pcvals.polrec) {
		if (wx >= cfg.cen_x) {
			if (wy > cfg.cen_y) {
				phi = M_PI - atan (((double)(wx - cfg.cen_x))/((double)(wy - cfg.cen_y)));
				r   = sqrt (sqr (wx - cfg.cen_x) + sqr (wy - cfg.cen_y));
			}
			else if (wy < cfg.cen_y) {
				phi = atan (((double)(wx - cfg.cen_x))/((double)(cfg.cen_y - wy)));
				r   = sqrt (sqr (wx - cfg.cen_x) + sqr (cfg.cen_y - wy));
			}
			else {
				phi = M_PI / 2;
				r   = wx - cfg.cen_x; /* cen_x - x1; */
			}
		}
		else if (wx < cfg.cen_x) {
			if (wy < cfg.cen_y) {
				phi = 2 * M_PI - atan (((double)(cfg.cen_x -wx))/((double)(cfg.cen_y - wy)));
				r   = sqrt (sqr (cfg.cen_x - wx) + sqr (cfg.cen_y - wy));
			}
			else if (wy > cfg.cen_y) {
				phi = M_PI + atan (((double)(cfg.cen_x - wx))/((double)(wy - cfg.cen_y)));
				r   = sqrt (sqr (cfg.cen_x - wx) + sqr (wy - cfg.cen_y));
			}
			else {
				phi = 1.5 * M_PI;
				r   = cfg.cen_x - wx; /* cen_x - x1; */
			}
		}
		if (wx != cfg.cen_x)
			m = fabs (((double)(wy - cfg.cen_y)) / ((double)(wx - cfg.cen_x)));
		else
			m = 0;

		if (m <= ((double)(y2 - y1) / (double)(x2 - x1))) {
			if (wx == cfg.cen_x) {
				xmax = 0;
				ymax = cfg.cen_y - y1;
			}
			else {
				xmax = cfg.cen_x - x1;
				ymax = m * xmax;
			}
		}
		else {
			ymax = cfg.cen_y - y1;
			xmax = ymax / m;
		}

		rmax = sqrt ( (double)(sqr (xmax) + sqr (ymax)) );
		t = ((cfg.cen_y - y1) < (cfg.cen_x - x1)) ? (cfg.cen_y - y1) : (cfg.cen_x - x1);
		rmax = (rmax - t) / 100 * (100 - circle) + t;
		phi = fmod (phi + angl, 2*M_PI);

		if (cfg.pcvals.backwards)
			x_calc = x2 - 1 - (x2 - x1 - 1)/(2*M_PI) * phi;
		else
			x_calc = (x2 - x1 - 1)/(2*M_PI) * phi + x1;

		if (cfg.pcvals.inverse)
			y_calc = (y2 - y1)/rmax   * r   + y1;
		else
			y_calc = y2 - (y2 - y1)/rmax * r;

		xi = (int) (x_calc+0.5);
		yi = (int) (y_calc+0.5);

		if (WITHIN(0, xi, cfg.width - 1) && WITHIN(0, yi, cfg.height - 1)) {
			*x = x_calc;
			*y = y_calc;
			inside = TRUE;
		}
		else
			inside = FALSE;
	}
	else {
		if (cfg.pcvals.backwards)
			phi = (2 * M_PI) * (x2 - wx) / xdiff;
		else
			phi = (2 * M_PI) * (wx - x1) / xdiff;

		phi = fmod (phi + angl, 2 * M_PI);
		if (phi >= 1.5 * M_PI)
			phi2 = 2 * M_PI - phi;
		else {
			if (phi >= M_PI)
				phi2 = phi - M_PI;
			else {
				if (phi >= 0.5 * M_PI)
					phi2 = M_PI - phi;
				else
					phi2 = phi;
			}
		}

		xx = tan (phi2);
		if (xx != 0)
			m = (double) 1.0 / xx;
		else
			m = 0;

		if (m <= ((double)(ydiff) / (double)(xdiff))) {
			if (phi2 == 0) {
				xmax = 0;
				ymax = ym - y1;
			}
			else {
				xmax = xm - x1;
				ymax = m * xmax;
			}
		}
		else {
			ymax = ym - y1;
			xmax = ymax / m;
		}

		rmax = sqrt ((double)(sqr (xmax) + sqr (ymax)));
		t = ((ym - y1) < (xm - x1)) ? (ym - y1) : (xm - x1);

		rmax = (rmax - t) / 100.0 * (100 - circle) + t;

		if (cfg.pcvals.inverse)
			r = rmax * (double)((wy - y1) / (double)(ydiff));
		else
			r = rmax * (double)((y2 - wy) / (double)(ydiff));

		xx = r * sin (phi2);
		yy = r * cos (phi2);

		if (phi >= 1.5 * M_PI) {
			x_calc = (double)xm - xx;
			y_calc = (double)ym - yy;
		}
		else {
			if (phi >= M_PI) {
				x_calc = (double)xm - xx;
				y_calc = (double)ym + yy;
			}
			else {
				if (phi >= 0.5 * M_PI) {
					x_calc = (double)xm + xx;
					y_calc = (double)ym + yy;
				}
				else {
					x_calc = (double)xm + xx;
					y_calc = (double)ym - yy;
				}
			}
		}

		xi = (int)(x_calc + 0.5);
		yi = (int)(y_calc + 0.5);

		if (WITHIN(0, xi, cfg.width - 1) && WITHIN(0, yi, cfg.height - 1)) {
			*x = x_calc;
			*y = y_calc;
			inside = TRUE;
		}
		else
			inside = FALSE;
	}
	return inside;
}

static void polarize(mif& src, mif& dst, POLARCONFIG& cfg)
{
	RGBA pixel[4];
	BYTE values[4];
	double cx, cy;
	int x, y;

progress_init(dst.m_height >> 3);
	RGBA* pd = dst.m_pixels;
	for (y = 0; y < dst.m_height; y++) {
		for (x = 0; x < dst.m_width; x++) {
			if (calc_undistorted_coords(cfg, x, y, &cx, &cy)) {
				get_pixel(src, (int)cx,     (int)cy,     &pixel[0]);
				get_pixel(src, (int)cx + 1, (int)cy,     &pixel[1]);
				get_pixel(src, (int)cx,     (int)cy + 1, &pixel[2]);
				get_pixel(src, (int)cx + 1, (int)cy + 1, &pixel[3]);

				values[0] = pixel[0].red;
				values[1] = pixel[1].red;
				values[2] = pixel[2].red;
				values[3] = pixel[3].red;
				pd->red = bilinear(cx, cy, values);

				values[0] = pixel[0].green;
				values[1] = pixel[1].green;
				values[2] = pixel[2].green;
				values[3] = pixel[3].green;
				pd->green = bilinear(cx, cy, values);

				values[0] = pixel[0].blue;
				values[1] = pixel[1].blue;
				values[2] = pixel[2].blue;
				values[3] = pixel[3].blue;
				pd->blue = bilinear(cx, cy, values);
			}
			else
				pd->red = pd->green = pd->blue = 255;
			pd++;
		}

if (!(y & 7)) progress_continue();
	}
progress_term();
}

HDIB WINAPI polarlize(HDIB hsrc, LPRECT prc, POLAR* ppp)
{
	CRect rc;
	mif image;
	if (!image.attachdib(hsrc) || !image.validaterect(prc, &rc))
		return NULL;
	if (image.m_bits < 24)
		image.m_bits = 24;

	mif src, dst;
	src.create(rc.Width(), rc.Height(), 24);
	src.blt(0, 0, rc.Width(), rc.Height(), &image, rc.left, rc.top);
	dst.create(rc.Width(), rc.Height(), 24);

	POLARCONFIG cfg;
	cfg.pcvals.circle = 100.0;
	cfg.pcvals.angle = 0.0;
	cfg.pcvals.backwards = FALSE;
	cfg.pcvals.inverse = TRUE;
	cfg.pcvals.polrec = TRUE;
	if (ppp)
		memcpy(&cfg.pcvals, ppp, sizeof(POLAR));

	cfg.width = src.m_width;
	cfg.height = src.m_height;

	cfg.cen_x = (double)(cfg.width - 1) / 2.0;
	cfg.cen_y = (double)(cfg.height - 1) / 2.0;

	polarize(src, dst, cfg);

	image.blt(rc.left, rc.top, rc.Width(), rc.Height(), &dst, 0, 0);
	return image.createdib();
}


static POLAR defpp = {
	100.0, 0.0, FALSE, TRUE, TRUE
};

/////////////////////////////////////////////////////////////////////////////
// CPolarDlg ダイアログ

class CPolarDlg : public CDialogImpl<CPolarDlg>, public CFilterDialogImpl<CPolarDlg>
{
public:
	CPolarDlg();

	enum { IDD = IDD_POLAR };
	CTrackCtrl m_cslider;
	CTrackCtrl m_aslider;
	POLAR m_pp;
	HDIB m_hdib;

BEGIN_MSG_MAP(CPolarDlg)
	CHAIN_MSG_MAP(CFilterDialogImpl<CPolarDlg>)
	ON_BN_CLICKED(IDC_BACKWARDS, OnCheckButton)
	ON_BN_CLICKED(IDC_INVERT, OnCheckButton)
	ON_BN_CLICKED(IDC_RECTANGLE, OnCheckButton)
END_MSG_MAP()

	void OnCheckButton();

	BOOL InitDialog(LPARAM lParam);
	void HScroll(int val, HWND hScrollBar);
	HDIB GetPreview(HDIB hdib);
};

CPolarDlg::CPolarDlg()
{
	memcpy(&m_pp, &defpp, sizeof(m_pp));
	m_hdib = NULL;
}

/////////////////////////////////////////////////////////////////////////////
// CPolarDlg メッセージ ハンドラ

BOOL CPolarDlg::InitDialog(LPARAM lParam) 
{
	m_cslider = GetDlgItem(IDC_CIRCLE);
	m_aslider = GetDlgItem(IDC_ANGLE);

	m_cslider.SetRange(0, 1000);
	m_cslider.SetPageSize(200);
	m_cslider.SetTicFreq(100);
	m_cslider.SetPos((int)(m_pp.circle * 10));
	m_cslider.SetBuddyBase(1);

	m_aslider.SetRange(0, 3600);
	m_aslider.SetPageSize(300);
	m_aslider.SetTicFreq(600);
	m_aslider.SetPos((int)(m_pp.angle * 10));
	m_aslider.SetBuddyBase(1);

	if (m_pp.backwards)	CheckDlgButton(IDC_BACKWARDS, TRUE);
	if (m_pp.inverse)	CheckDlgButton(IDC_INVERT, TRUE);
	if (!m_pp.polrec)	CheckDlgButton(IDC_RECTANGLE, TRUE);

	SetWholePreview(TRUE);
	SetPreviewImage(m_hdib);
	UpdatePreview();
	return TRUE;
}

void CPolarDlg::HScroll(int val, HWND hBar) 
{
	if (hBar == m_cslider)		m_pp.circle = (double)val / 10.0;
	else if (hBar == m_aslider)	m_pp.angle = (double)val / 10.0;
	else						return;

	UpdatePreview();
}

void CPolarDlg::OnCheckButton() 
{
	m_pp.backwards = IsDlgButtonChecked(IDC_BACKWARDS);
	m_pp.inverse = IsDlgButtonChecked(IDC_INVERT);
	m_pp.polrec = !IsDlgButtonChecked(IDC_RECTANGLE);
	UpdatePreview();
}

HDIB CPolarDlg::GetPreview(HDIB hdib)
{
	return polarlize(hdib, NULL, &m_pp);
}

//////////////////////////////////////////////////////////////////////
//

static TCHAR secRoot[] = _T("transform\\polar");
static TCHAR keyLast[] = _T("last");

HANDLE polar_ui(HDIB hdib, LPRECT prc, FILTER_EXEC* pfe)
{
	CPolarDlg dlg;

	POLAR p;
	regkey reg(pfe->hkey, secRoot);
	if (reg.getbinary(keyLast, &p, sizeof(p)) == sizeof(p))
		memcpy(&dlg.m_pp, &p, sizeof(p));

	mif image(hdib);
	image.crop(prc);
	dlg.m_hdib = image.createdib();
	image.destroy();

	BOOL bOK = (dlg.DoModal(pfe->hparent) == IDOK);
	GlobalFree(dlg.m_hdib);
	if (bOK) {
		reg.setbinary(keyLast, &dlg.m_pp, sizeof(dlg.m_pp));

		CWaitCursor wc;
pfnProgress = pfe->progress;
		HDIB h = polarlize(hdib, prc, &dlg.m_pp);
pfnProgress = NULL;
		return h;
	}
	return NULL;
}
