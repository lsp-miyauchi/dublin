// fractrace.cpp : インプリメンテーション ファイル
//

#include "stdafx.h"
#include "..\filterdialog.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

enum {
	OUTSIDE_TYPE_WRAP,
	OUTSIDE_TYPE_TRANSPARENT,
	OUTSIDE_TYPE_BLACK,
	OUTSIDE_TYPE_WHITE
};

extern PROGRESSCALLBACK pfnProgress;
void progress_init(int maxprogress);
void progress_continue();
void progress_term();

typedef struct tagFRACTRACE {
	double x1;					// xmin fractal image delimiter
	double x2;					// xmax fractal image delimiter
	double y1;					// ymin fractal image delimiter
	double y2;					// ymax fractal image delimiter
	int depth;					// trace depth
	int outside_type;			// outside type (0=WRAP/1=TRANS/2=BLACK/3=WHITE)
} FRACTRACE;


typedef struct {
	BYTE r;
	BYTE g;
	BYTE b;
} pixel_t;

class fractaltrace
{
public:
	fractaltrace();

	void pixels_init(mif* image);
	void pixels_free();
	void pixels_get(int x, int y, RGBA* pixel);
	void pixels_get_biliner(double x, double y, RGBA* pixel);
	void pixels_set(int x, int y, RGBA* pixel);
	void pixels_store(mif* dst);
	void mandelbrot(double x, double y, double *u, double *v);
	void filter();

	FRACTRACE fp;
	RGBA** spixels;
	RGBA** dpixels;
	int width;
	int height;
};

fractaltrace::fractaltrace()
{
	fp.x1 = -1.0;
	fp.x2 = 0.5;
	fp.y1 = -1.0;
	fp.y2 = 1.0;
	fp.depth = 3;
	fp.outside_type = OUTSIDE_TYPE_WRAP;
};

void fractaltrace::pixels_init(mif* image)
{
	width  = image->m_width;
	height = image->m_height;

	spixels = (RGBA**)malloc(height * sizeof(RGBA*));
	dpixels = (RGBA**)malloc(height * sizeof(RGBA*));
	for (int y = 0; y < height; y++) {
		spixels[y] = (RGBA*)malloc(width * sizeof(RGBA));
		dpixels[y] = (RGBA*)malloc(width * sizeof(RGBA));
		image->getrow(spixels[y], 0, y, width);
	}
}

void fractaltrace::pixels_free()
{
	int y;
	for (y = 0; y < height; y++) {
		free(spixels[y]);
		free(dpixels[y]);
	}
	free(spixels);
	free(dpixels);
}

void fractaltrace::pixels_get(int x, int y, RGBA* pixel)
{
	if (x < 0)				x = 0;
	else if (width <= x)	x = width - 1;
	if (y < 0)				y = 0;
	else if (height <= y)	y = height - 1;

	*pixel = spixels[y][x];
}

void fractaltrace::pixels_get_biliner(double x, double y, RGBA* pixel)
{
	RGBA A, B, C, D;
	double a, b, c, d;
	int x1, y1, x2, y2;
	double dx, dy;

	x1 = (int)floor(x);
	x2 = x1 + 1;
	y1 = (int)floor(y);
	y2 = y1 + 1;

	dx = x - (double)x1;
	dy = y - (double)y1;
	a  = (1.0 - dx) * (1.0 - dy);
	b  = dx * (1.0 - dy);
	c  = (1.0 - dx) * dy;
	d  = dx * dy;

	pixels_get(x1, y1, &A);
	pixels_get(x2, y1, &B);
	pixels_get(x1, y2, &C);
	pixels_get(x2, y2, &D);

	pixel->red = (BYTE)(a * (double)A.red + b * (double)B.red + c * (double)C.red + d * (double)D.red );
	pixel->green = (BYTE)(a * (double)A.green + b * (double)B.green + c * (double)C.green + d * (double)D.green );
	pixel->blue = (BYTE)(a * (double)A.blue + b * (double)B.blue + c * (double)C.blue + d * (double)D.blue );
}

void fractaltrace::pixels_set(int x, int y, RGBA* pixel)
{
	dpixels[y][x] = *pixel;
}

void fractaltrace::pixels_store(mif* dst)
{
	for (int y = 0; y < height; y++)
		dst->setrow(dpixels[y], 0, y, width);
}

void fractaltrace::mandelbrot(double x, double y, double *u, double *v)
{
	int iter = 0;
	double xx = x;
	double yy = y;
	double x2 = xx * xx;
	double y2 = yy * yy;
	while (iter < fp.depth) {
		double tmp = x2 - y2 + x;
		yy  = 2 * xx * yy + y;
		xx  = tmp;
		x2  = xx * xx;
		y2  = yy * yy;
		iter++;
	}
	*u = xx;
	*v = yy;
}

void fractaltrace::filter()
{
	int x, y;
	RGBA pixel;
	double scale_x, scale_y;
	double cx, cy;
	double px, py;

	scale_x = ( fp.x2 - fp.x1 ) / width;
	scale_y = ( fp.y2 - fp.y1 ) / height;

progress_init(height >> 4);
	for (y = 0; y < height; y++) {
		cy = fp.y1 + y * scale_y;
		for (x = 0; x < width; x++) {
			cx = fp.x1 + x * scale_x;
			mandelbrot(cx, cy, &px, &py);
			px = (px - fp.x1) / scale_x;
			py = (py - fp.y1) / scale_y;
			if (0 <= px && px < width && 0 <= py && py < height)
				pixels_get_biliner(px, py, &pixel);
			else {
				switch( fp.outside_type ) {
					case OUTSIDE_TYPE_WRAP:
						px = fmod( px, width );
						py = fmod( py, height );
						if( px < 0.0 )
							px += width;
						if( py < 0.0 )
							py += height;
						pixels_get_biliner( px, py, &pixel );
						break;

					case OUTSIDE_TYPE_TRANSPARENT:
						pixel.red = pixel.green = pixel.blue = 0;
						break;

					case OUTSIDE_TYPE_BLACK:
						pixel.red = pixel.green = pixel.blue = 0;
						break;

					case OUTSIDE_TYPE_WHITE:
						pixel.red = pixel.green = pixel.blue = 255;
						break;
				}
			}
			pixels_set(x, y, &pixel);
		}
if (!(y & 0xf)) progress_continue();
	}
progress_term();
}

HDIB fractal_trace(HDIB hdib, LPRECT prc, FRACTRACE* pfp)
{
	CRect rc;
	mif dst(hdib);
	if (!dst.validaterect(prc, &rc)) return FALSE;
	dst.crop(prc);

	fractaltrace ft;
	if (pfp) memcpy(&ft.fp, pfp, sizeof(FRACTRACE));

	ft.pixels_init(&dst);
	ft.filter();
	ft.pixels_store(&dst);
	ft.pixels_free();

	mif src(hdib);
	src.blt(rc.left, rc.top, rc.Width(), rc.Height(), &dst, 0, 0);

	return src.createdib();
}

/////////////////////////////////////////////////////////////////////////////
// CFracTraceDlg ダイアログ


class CFracTraceDlg : public CDialogImpl<CFracTraceDlg>, public CFilterDialogImpl<CFracTraceDlg>
{
public:
	CFracTraceDlg();

	enum { IDD = IDD_FRACTRACE };
	CTrackCtrl m_slider;
	int m_amount;
	HDIB m_hdib;

BEGIN_MSG_MAP(CFracTraceDlg)
	CHAIN_MSG_MAP(CFilterDialogImpl<CFracTraceDlg>)
END_MSG_MAP()

	BOOL InitDialog(LPARAM lParam);
	void HScroll(int val, HWND hScrollBar);
	HDIB GetPreview(HDIB hdib);
};

CFracTraceDlg::CFracTraceDlg()
{
	m_amount = 3;
	m_hdib = NULL;
}

/////////////////////////////////////////////////////////////////////////////
// CFracTraceDlg メッセージ ハンドラ

BOOL CFracTraceDlg::InitDialog(LPARAM lParam)
{
	m_slider = GetDlgItem(IDC_SLIDER);
	m_slider.SetRange(1, 5);
	m_slider.SetPos(m_amount);

	SetWholePreview(TRUE);
	SetPreviewImage(m_hdib);
	UpdatePreview();
	return TRUE;
}

void CFracTraceDlg::HScroll(int val, HWND hScrollBar) 
{
	if (hScrollBar == m_slider) {
		m_amount = val;
		UpdatePreview();
	}
}

HDIB CFracTraceDlg::GetPreview(HDIB hdib)
{
	FRACTRACE fp;
	fp.x1 = -1.0;
	fp.x2 = 0.5;
	fp.y1 = -1.0;
	fp.y2 = 1.0;
	fp.depth = m_amount;
	fp.outside_type = OUTSIDE_TYPE_WRAP;
	return fractal_trace(hdib, NULL, &fp);
}

////////////////////////////////////////////////////////////////////////
//

static TCHAR secRoot[] = _T("expression\\fractal_trace");
static TCHAR keyLast[] = _T("last");

HANDLE fractal_trace_ui(HDIB hdib, LPRECT prc, FILTER_EXEC* pfe)
{
	regkey reg(pfe->hkey, secRoot);

	CFracTraceDlg dlg;
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
		FRACTRACE fp;
		fp.x1 = -1.0;
		fp.x2 = 0.5;
		fp.y1 = -1.0;
		fp.y2 = 1.0;
		fp.depth = dlg.m_amount;
		fp.outside_type = OUTSIDE_TYPE_WRAP;
		HDIB h = fractal_trace(hdib, prc, &fp);
pfnProgress = NULL;
		return h;
	}
	return NULL;
}
