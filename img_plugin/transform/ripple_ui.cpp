// ripple.cpp : インプリメンテーション ファイル
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

typedef struct tagRIPPLE {
    int period;					// 0 - 200 number of pixels for one wave to complete
    int amplitude;				// 0 - 200 maximum displacement of wave
    int orientation;			// 0 = Horizontal, 1 = Vertical
    int edges;					// edges; 0 = smear, 1 =  wrap, 2 = black
    int waveform;				// 0 = sawtooth, 1 = sine wave
	BOOL antialias;
	BOOL tile;					// TRUE then image will retain it's tilability
} RIPPLE;

enum {
	RippleHorz,
	RippleVert
};

enum {
	RippleSmear,
	RippleWrap,
	RippleBlack
};

enum {
	RippleSawtooth,
	RippleSine
};


static RGBA __fastcall averagetwo(double location, RGBA *v)
{
	RGBA s;
	location = fmod(location, 1.0);
	s.red = (BYTE)((1.0 - location) * v[0].red + location * v[1].red);
	s.green = (BYTE)((1.0 - location) * v[0].green + location * v[1].green);
	s.blue = (BYTE)((1.0 - location) * v[0].blue + location * v[1].blue);
	return s;
}

static RGBA __fastcall averagefour(double location, RGBA *v)
{
	RGBA s;
	location = fmod(location, 1.0);
	s.red = (BYTE)(((1.0 - location) * (v[0].red + v[2].red) + location * (v[1].red + v[3].red)) / 2);
	s.green = (BYTE)(((1.0 - location) * (v[0].green + v[2].green) + location * (v[1].green + v[3].green)) / 2);
	s.blue = (BYTE)(((1.0 - location) * (v[0].blue + v[2].blue) + location * (v[1].blue + v[3].blue)) / 2);
	return s;
}

static double __fastcall displace_amount(RIPPLE* prp, int location)
{
	switch (prp->waveform) {
		case RippleSine:		return prp->amplitude * sin(location * (2*M_PI) / (double)prp->period);
        case RippleSawtooth:	return floor(prp->amplitude * (fabs((((location % prp->period) / (double)prp->period) * 4) - 2) - 1));
	}
	return 0;
}

static void __fastcall ripple_pixel(mif* image, int x, int y, RGBA* pixel)
{
	if (x >= 0 && y >= 0 && x < image->m_width && y < image->m_height)
		*pixel = *(image->pos(x, y));
	else
		pixel->red = pixel->green = pixel->blue = 0;
}

///////////////////////////////////////////////////////////////////////
//

static HDIB ripple(HDIB hsrc, LPRECT prc, RIPPLE* prpp)
{
	CRect rc;
	mif image;
	if (!image.attachdib(hsrc) || !image.validaterect(prc, &rc)) return NULL;

	if (image.m_bits < 24) image.m_bits = 24;

	mif src, dst;
	src.create(rc.Width(), rc.Height(), 24);
	src.blt(0, 0, src.m_width, src.m_height, &image, rc.left, rc.top);
	dst.create(rc.Width(), rc.Height(), 24);

	RIPPLE rp;
	rp.period = 20;
	rp.amplitude = 5;
	rp.orientation = RippleHorz;
	rp.edges = RippleWrap;
	rp.waveform = RippleSine;
	rp.antialias = TRUE;
	rp.tile = TRUE;
	if (prpp)
		memcpy(&rp, prpp, sizeof(rp));

	RGBA *destline;
	RGBA *dest;
	RGBA *otherdest;
	RGBA pixel[4];
	double needx, needy;
	int x, y, xi, yi;

	if (rp.tile) {
		rp.edges = RippleWrap;
		rp.period = src.m_width / (src.m_width / rp.period) *
						(rp.orientation == RippleHorz) + 
						src.m_height / (src.m_height / rp.period) *
						(rp.orientation == RippleVert);
	}

	if (rp.orientation == RippleVert) {
progress_init(dst.m_width);
		destline = dst.m_pixels;
		for (x = 0; x < dst.m_width; x++) {
			dest = destline;
			for (y = 0; y < dst.m_height; y++) {
				otherdest = dest;
				needy = y + displace_amount(&rp, x);
				yi = (int)floor(needy);

				if (rp.edges == RippleWrap) {
					// Tile the image.
					needy = fmod(needy + src.m_height, src.m_height);
					yi = (yi + src.m_height) % src.m_height;
				}
				else if (rp.edges == RippleSmear) {
					// Smear out the edges of the image by repeating pixels.
					if (yi < 0)
						yi = 0;
					else if (yi > src.m_height - 1)
						yi = src.m_height - 1;
				}

				if (rp.antialias) {
					if (yi == src.m_height - 1) {
						ripple_pixel(&src, x, yi, otherdest);
						otherdest++;
					}
					else if (needy < 0 && needy > -1) {
						ripple_pixel(&src, x, 0, otherdest);
						otherdest++;
					}
					else if (yi == src.m_height - 2 || yi == 0) {
						ripple_pixel(&src, x, yi, &pixel[0]);
						ripple_pixel(&src, x, yi + 1, &pixel[1]);
						*otherdest++ = averagetwo(needy, pixel);
					}
					else {
						ripple_pixel(&src, x, yi, &pixel[0]);
						ripple_pixel(&src, x, yi + 1, &pixel[1]);
						ripple_pixel(&src, x, yi - 1, &pixel[2]);
						ripple_pixel(&src, x, yi + 2, &pixel[3]);
						*otherdest++ = averagefour(needy, pixel);
					}
				}
				else {
					ripple_pixel(&src, x, yi, otherdest);
					otherdest++;
				}
				dest += dst.m_width;
			}
			destline++;
progress_continue();
		}
	}
	else {
progress_init(dst.m_height);
		destline = dst.m_pixels;
		for (y = 0; y < dst.m_height; y++) {
			dest = destline;
			for (x = 0; x < dst.m_width; x++) {
				needx = x + displace_amount(&rp, y);
				xi = (int)floor(needx);

				if (rp.edges == RippleWrap) {
					// Tile the image.
					needx = fmod((needx + src.m_width), src.m_width);
					xi = (xi + src.m_width) % src.m_width;
				}
				else if (rp.edges == RippleSmear) {
					// Smear out the edges of the image by repeating pixels.
					if (xi < 0)
						xi = 0;
					else if (xi > src.m_width - 1)
						xi = src.m_width - 1;
                }

                if (rp.antialias) {
                    if (xi == src.m_width - 1) {
                        ripple_pixel(&src, xi, y, dest);
						dest++;
                    }
                    else if (floor(needx) ==  -1) {
						ripple_pixel(&src, 0, y, dest);
						dest++;
                    }
					else if (xi == src.m_width - 2 || xi == 0) {
						ripple_pixel(&src, xi, y, &pixel[0]);
						ripple_pixel(&src, xi + 1, y, &pixel[1]);
						*dest++ = averagetwo(needx, pixel);
                    }
                    else {
                        ripple_pixel(&src, xi, y, &pixel[0]);
                        ripple_pixel(&src, xi + 1, y, &pixel[1]);
                        ripple_pixel(&src, xi - 1 , y, &pixel[2]);
                        ripple_pixel(&src, xi + 2, y, &pixel[3]);
						*dest++ = averagefour(needx, pixel);
                    }
				}
				else {
                    ripple_pixel(&src, xi, y, dest);
					dest++;
                }
			}
			destline += dst.m_width;
progress_continue();
		}
	}
progress_term();

	src.destroy();
	image.blt(rc.left, rc.top, rc.Width(), rc.Height(), &dst, 0, 0);
	dst.destroy();
	return image.createdib();
}


/////////////////////////////////////////////////////////////////////////////
// CRippleDlg ダイアログ

class CRippleDlg : public CDialogImpl<CRippleDlg>, public CFilterDialogImpl<CRippleDlg>
{
public:
	CRippleDlg();

	enum { IDD = IDD_RIPPLE };
	CComboBox	m_ocombo;
	CComboBox	m_ecombo;
	CComboBox	m_wcombo;

	CTrackCtrl m_pslider;
	CTrackCtrl m_aslider;
	RIPPLE m_rp;
	HDIB m_hdib;

BEGIN_MSG_MAP(CRippleDlg)
	CHAIN_MSG_MAP(CFilterDialogImpl<CRippleDlg>)
	ON_CBN_SELCHANGE(IDC_EDGES, OnPreviewUpdate)
	ON_CBN_SELCHANGE(IDC_ORIENTATION, OnPreviewUpdate)
	ON_CBN_SELCHANGE(IDC_WAVEFORM, OnPreviewUpdate)
	ON_BN_CLICKED(IDC_TILE, OnPreviewUpdate)
END_MSG_MAP()

	void OnPreviewUpdate();

	BOOL InitDialog(LPARAM lParam);
	void HScroll(int val, HWND hScrollBar);
	HDIB GetPreview(HDIB hdib);
};

CRippleDlg::CRippleDlg()
{
	m_rp.period = 20;
	m_rp.amplitude = 5;
	m_rp.orientation = RippleHorz;
	m_rp.edges = RippleWrap;
	m_rp.waveform = RippleSine;
	m_rp.antialias = TRUE;
	m_rp.tile = TRUE;
}

/////////////////////////////////////////////////////////////////////////////
// CRippleDlg メッセージ ハンドラ

BOOL CRippleDlg::InitDialog(LPARAM lParam)
{
	m_ocombo = GetDlgItem(IDC_ORIENTATION);
	m_ecombo = GetDlgItem(IDC_EDGES);
	m_wcombo = GetDlgItem(IDC_WAVEFORM);
	m_pslider = GetDlgItem(IDC_PSLIDER);
	m_aslider = GetDlgItem(IDC_ASLIDER);

	m_aslider.SetRange(1, 100);
	m_aslider.SetPageSize(10);
	m_aslider.SetTicFreq(10);
	m_aslider.SetPos(m_rp.amplitude);

	m_pslider.SetRange(1, 100);
	m_pslider.SetPageSize(10);
	m_pslider.SetTicFreq(10);
	m_pslider.SetPos(m_rp.period);

	m_ocombo.SetCurSel(m_rp.orientation);
	m_ecombo.SetCurSel(m_rp.edges);
	m_wcombo.SetCurSel(m_rp.waveform);
	CheckDlgButton(IDC_TILE, m_rp.tile);

	SetPreviewImage(m_hdib);
	UpdatePreview();
	return TRUE;
}

void CRippleDlg::HScroll(int val, HWND hScrollBar)
{
	if (hScrollBar == m_aslider)		m_rp.amplitude = val;
	else if (hScrollBar == m_pslider)	m_rp.period = val;
	else								return;
	UpdatePreview();
}

void CRippleDlg::OnPreviewUpdate() 
{
	m_rp.orientation = m_ocombo.GetCurSel();
	m_rp.edges = m_ecombo.GetCurSel();
	m_rp.waveform = m_wcombo.GetCurSel();
	m_rp.tile = IsDlgButtonChecked(IDC_TILE);
	UpdatePreview();
}

HDIB CRippleDlg::GetPreview(HDIB hdib)
{
	return ripple(hdib, NULL, &m_rp);
}


//////////////////////////////////////////////////////////////////////
//

static TCHAR secRoot[] = _T("transform\\ripple");
static TCHAR keyLast[] = _T("last");

HANDLE ripple_ui(HDIB hdib, LPRECT prc, FILTER_EXEC* pfe)
{
	CRippleDlg dlg;

	RIPPLE r;
	regkey reg(pfe->hkey, secRoot);
	if (reg.getbinary(keyLast, &r, sizeof(r)) == sizeof(r))
		memcpy(&dlg.m_rp, &r, sizeof(r));

	mif image(hdib);
	image.crop(prc);
	dlg.m_hdib = image.createdib();
	image.destroy();

	BOOL bOK = (dlg.DoModal(pfe->hparent) == IDOK);
	GlobalFree(dlg.m_hdib);
	if (bOK) {
		reg.setbinary(keyLast, &dlg.m_rp, sizeof(dlg.m_rp));

		CWaitCursor wc;
pfnProgress = pfe->progress;
		HDIB h = ripple(hdib, prc, &dlg.m_rp);
pfnProgress = NULL;
		return h;
	}
	return NULL;
}
