///////////////////////////////////////////////////////////////////////
// zoom.cpp

#include "stdafx.h"
#include "lsmif.h"
#include "common.h"

static double gaus = sqrt(2.0 / M_PI);

typedef struct _FILTERINFO {
	double (__fastcall *pfn)(double);
	double width;
} FILTERINFO;

#pragma pack(1)

typedef struct _PIXEL {
	BYTE blue;
	BYTE green;
	BYTE red;
} PIXEL;

typedef struct tagCONTRIBINFO {
	int pixel;
	double weight;
} CONTRIBINFO;

typedef struct tagDBLPOINT {
	double x;
	double y;
} DBLPOINT;

typedef struct tagPOINTINFO {
	double x;
	double y;
	double z;
} POINTINFO;

typedef struct tagRECTINFO {
	int width;
	int height;
	int x;
	int y;
} RECTINFO;

#pragma pack()

static double __fastcall J1(double x)
{
	double p, q;
	register int i;

	static const double Pone[] = {
       0.581199354001606143928050809e+21,
      -0.6672106568924916298020941484e+20,
       0.2316433580634002297931815435e+19,
      -0.3588817569910106050743641413e+17,
       0.2908795263834775409737601689e+15,
      -0.1322983480332126453125473247e+13,
       0.3413234182301700539091292655e+10,
      -0.4695753530642995859767162166e+7,
       0.270112271089232341485679099e+4
    };

	static const double Qone[] = {
      0.11623987080032122878585294e+22,
      0.1185770712190320999837113348e+20,
      0.6092061398917521746105196863e+17,
      0.2081661221307607351240184229e+15,
      0.5243710262167649715406728642e+12,
      0.1013863514358673989967045588e+10,
      0.1501793594998585505921097578e+7,
      0.1606931573481487801970916749e+4,
      0.1e+1
    };

	p = Pone[8];
	q = Qone[8];
	for (i=7; i >= 0; i--)
	{
		p = p * x * x + Pone[i];
		q = q * x * x + Qone[i];
	}
	return p / q;
}

static double __fastcall P1(double x)
{
	double p, q;
	register int i;

	static const double Pone[] = {
      0.352246649133679798341724373e+5,
      0.62758845247161281269005675e+5,
      0.313539631109159574238669888e+5,
      0.49854832060594338434500455e+4,
      0.2111529182853962382105718e+3,
      0.12571716929145341558495e+1
	};

	static const double Qone[] = {
      0.352246649133679798068390431e+5,
      0.626943469593560511888833731e+5,
      0.312404063819041039923015703e+5,
      0.4930396490181088979386097e+4,
      0.2030775189134759322293574e+3,
      0.1e+1
	};

	p = Pone[5];
	q = Qone[5];
	for (i = 4; i >= 0; i--) {
		p = p * (8.0 / x) * (8.0 / x) + Pone[i];
		q = q * (8.0 / x) * (8.0 / x) + Qone[i];
	}
	return p / q;
}

static double __fastcall Q1(double x)
{
	double p, q;
	register int i;

	static const double Pone[] = {
      0.3511751914303552822533318e+3,
      0.7210391804904475039280863e+3,
      0.4259873011654442389886993e+3,
      0.831898957673850827325226e+2,
      0.45681716295512267064405e+1,
      0.3532840052740123642735e-1
	};
	static const double Qone[] = {
      0.74917374171809127714519505e+4,
      0.154141773392650970499848051e+5,
      0.91522317015169922705904727e+4,
      0.18111867005523513506724158e+4,
      0.1038187585462133728776636e+3,
      0.1e+1
	};

	p = Pone[5];
	q = Qone[5];
	for (i = 4; i >= 0; i--)
	{
		p = p * (8.0 / x) * (8.0 / x) + Pone[i];
		q = q * (8.0 / x) * (8.0 / x) + Qone[i];
	}
	return p / q;
}

static double __fastcall BesselOrderOne(double x)
{
	double p, q;
	if (x == 0.0)
		return(0.0);
	p = x;
	if (x < 0.0)
		x = (-x);
	if (x < 8.0)
		return (p * J1(x));
	q = sqrt(2.0 / (M_PI * x)) * (P1(x) * (1.0 / sqrt(2.0) * (sin(x) - cos(x))) -
				8.0 / x * Q1(x) * (-1.0 / sqrt(2.0) * (sin(x) + cos(x))));
	if (p < 0.0)
		q = (-q);
	return q;
}

static double __fastcall mfnBox(double x)
{
	if ((x > -0.5) && (x <= 0.5))
		return (1.0);
	return (0.0);
}

static double __fastcall mfnBessel(double x)
{
	if (x == 0.0)
		return (M_PI / 4.0);
	return (BesselOrderOne(M_PI * x) / (2.0 * x));
}

static double __fastcall mfnBlackman(double x)
{
	return (0.42 + 0.50 * cos(M_PI * x) + 0.08 * cos(2.0 * M_PI * x));
}

static double __fastcall mfnCatrom(double x)
{
	if (x < 0)
		x = (-x);
	if (x < 1.0)
		return (0.5 * (2.0 + x * x * (-5.0 + x * 3.0)));
	if (x < 2.0)
		return(0.5 * (4.0 + x * (-8.0 + x * (5.0 - x))));
	return (0.0);
}

static double __fastcall mfnCubic(double x)
{
	if (x < 0)
		x = (-x);
	if (x < 1.0)
		return ((0.5 * x * x * x) - x * x + (2.0 / 3.0));
	if (x < 2.0)
	{
		x = 2.0 - x;
		return((1.0 / 6.0) * x * x * x);
	}
	return (0.0);
}

static double __fastcall mfnGaussian(double x)
{
	return (exp(-2.0 * x * x ) * gaus);
}

static double __fastcall mfnHanning(double x)
{
	return (0.5 + 0.5 * cos(M_PI * x));
}

static double __fastcall mfnHamming(double x)
{
	return (0.54 + 0.46 * cos(M_PI * x));
}

static double __fastcall mfnHermite(double x)
{
	if (x < 0)
		x = (-x);
	if (x < 1.0)
		return ((2.0 * x - 3.0) * x * x + 1.0);
	return (0.0);
}

static double __fastcall mfnSinc(double x)
{
	x *= M_PI;
	if (x != 0.0)
		return (sin(x) / x);
	return (1.0);
}

static double __fastcall mfnLanczos(double x)
{
	if (x < 0)
		x = (-x);
	if (x < 3.0)
		return (mfnSinc(x) * mfnSinc(x / 3.0));
	return (0.0);
}

static double __fastcall mfnMitchell(double x)
{
	double b, c;

	b = 1.0 / 3.0;
	c = 1.0 / 3.0;
	if (x < 0)
		x = (-x);
	if (x < 1.0) {
		x = ((12.0 - 9.0 * b - 6.0 * c) * (x * x * x)) +
				((-18.0 + 12.0 * b + 6.0 * c) * x * x) + (6.0 - 2.0 * b);
		return (x / 6.0);
	}
	if (x < 2.0) {
		x = ((-1.0 * b - 6.0 * c) * (x * x * x)) +
				((6.0 * b + 30.0 * c) * x * x) + ((-12.0 * b - 48.0 * c) * x) +
				(8.0 * b + 24.0 * c);
		return (x / 6.0);
	}
	return (0.0);
}

static double __fastcall mfnQuadratic(double x)
{
	if (x < 0)
		x = (-x);
	if (x < 0.5)
		return (0.75 - x * x);
	if (x < 1.5) {
		x -= 1.5;
		return (0.5 * x * x);
	}
	return (0.0);
}

static double __fastcall mfnTriangle(double x)
{
	if (x < 0.0)
		x = (-x);
	if (x < 1.0)
		return (1.0 - x);
	return (0.0);
}

static void horz_filter(mif* src, mif* dst, double x_fac, FILTERINFO *pfi, double ciw)
{
	// Apply filter to zoom horizontally from source to destination.
	double width = pfi->width;
	double scale_fac = 1.0;
	if (x_fac < 1.0) {
		width /= x_fac;
		scale_fac /= x_fac;
	}

	int iw = (int)(ciw * 2 + 1);
	CONTRIBINFO* pci_ptr = new CONTRIBINFO[iw * dst->m_width];
	parallel_for (0, dst->m_width, [&](int x) {
		CONTRIBINFO* pci = pci_ptr + (x * iw);
		double sum = 0.0;
		int n = 0;
		double mid = (double)x / x_fac;
		for (int i = (int)(mid - width + 0.5); i < (int)(mid + width + 0.5); i++) {
			int j = i;
			if (j < 0)
				j = (-j);
			else if (j >= src->m_width)
				j = (src->m_width << 1) - j - 1;

			pci[n].pixel = j;
			pci[n].weight = pfi->pfn(((double)i - mid + 0.5) / scale_fac);
			pci[n].weight /= scale_fac;
			sum += pci[n].weight;
			n++;
		}

		if ((sum != 0.0) && (sum != 1.0)) {
			for (int i = 0; i < n; i++)
				pci[i].weight /= sum;			// normalize
		}

		for (int y = 0; y < dst->m_height; y++) {
			RGBA* q = dst->pos(x, y);
			double blue_weight = 0.0;
			double green_weight = 0.0;
			double red_weight = 0.0;
			for (int k = 0; k < n; k++) {
				RGBA* p = src->pos(pci[k].pixel, y);
				red_weight += pci[k].weight * p->red;
				green_weight += pci[k].weight * p->green;
				blue_weight += pci[k].weight * p->blue;
			}
			q->blue = ADJRGB((int)(blue_weight + 0.5));
			q->green = ADJRGB((int)(green_weight + 0.5));
			q->red = ADJRGB((int)(red_weight + 0.5));
			q++;
		}
	});
	delete [] pci_ptr;
}

static void vert_filter(mif* src, mif* dst, double y_fac, FILTERINFO *pfi, double ciw)
{
	// Apply filter to zoom vertically from source to destination.
	double width = pfi->width;
	double scale_fac = 1.0;
	if (y_fac < 1.0) {
		width /= y_fac;
		scale_fac /= y_fac;
	}

	int iw = (int)(ciw * 2 + 1);
	CONTRIBINFO* pci_ptr = new CONTRIBINFO[iw * dst->m_height];
	parallel_for (0, dst->m_height, [&](int y) {
		CONTRIBINFO* pci = pci_ptr + (y * iw);
		double sum = 0.0;
		double mid = (double)y / y_fac;
		int n = 0;
		for (int i = (int)(mid - width + 0.5); i < (int)(mid + width + 0.5); i++) {
			int j = i;
			if (j < 0)
				j = (-j);
			else if (j >= src->m_height)
				j = (src->m_height << 1) - j - 1;
			pci[n].pixel = j;
			pci[n].weight = pfi->pfn(((double)i - mid + 0.5) / scale_fac);
			pci[n].weight /= scale_fac;
			sum += pci[n].weight;
			n++;
		}

		if ((sum != 0.0) && (sum != 1.0)) {
			for (int i = 0; i < n; i++)
				pci[i].weight /= sum;			// normalize
		}

		parallel_for (0, dst->m_width, [&](int x) {
			RGBA* q = dst->pos(x, y);
			double blue_weight = 0.0;
			double green_weight = 0.0;
			double red_weight = 0.0;
			for (int k = 0; k < n; k++) {
				RGBA* p = src->pos(x, pci[k].pixel);
				red_weight += pci[k].weight * p->red;
				green_weight += pci[k].weight * p->green;
				blue_weight += pci[k].weight * p->blue;
			}
			q->blue = ADJRGB((int)(blue_weight + 0.5));
			q->green = ADJRGB((int)(green_weight + 0.5));
			q->red = ADJRGB((int)(red_weight + 0.5));
		});
	});
	delete [] pci_ptr;
}

static FILTERINFO filters[zoomLast] =
{
	{ mfnBox, 0.5 },					// 0	ZOOM_BOX
	{ mfnTriangle, 1.0 },				// 1	ZOOM_TRIANGLE
	{ mfnHermite, 1.0 },				// 2	ZOOM_HERMITE
	{ mfnHanning, 1.0 },				// 3	ZOOM_HANNING
	{ mfnHamming, 1.0 },				// 4	ZOOM_HAMMING
	{ mfnBlackman, 1.0 },				// 5	ZOOM_BLACKMAN
	{ mfnGaussian, 1.25 },				// 6	ZOOM_GAUSSIAN
	{ mfnQuadratic, 1.5 },				// 7	ZOOM_QUADRATIC
	{ mfnCubic, 2.0 },					// 8	ZOOM_CUBIC
	{ mfnCatrom, 2.0 },					// 9	ZOOM_CATROM
	{ mfnMitchell, 2.0 },				// 11	ZOOM_MITCHELL
	{ mfnLanczos, 3.0 },				// 12	ZOOM_LANCZOS
	{ mfnBessel, 3.2383 },				// 13	ZOOM_BESSEL
	{ mfnSinc, 4.0 },					// 14	ZOOM_SINC
	{ NULL, 0.0 }						// 15	ZOOM_CUSTOM
};

BOOL zoom_filter(mif& src, int cx, int cy, int method, BOOL ondisk)
{
	if (cx < 1 || cy < 1)
		return FALSE;

	if (src.m_width == cx && src.m_height == cy)
		return TRUE;

	mif dst;
	if (cy >= src.m_height)
		dst.create(cx, src.m_height, 24);
	else
		dst.create(src.m_width, cy, 24);

	// Allocate filter info list.
	double x_fac = (double)cx / (double)src.m_width;
	double y_fac = (double)cy / (double)src.m_height;
	double width = MAX(filters[method].width / x_fac, filters[method].width / y_fac);
	if (width < filters[method].width)
		width = filters[method].width;

	// Zoom image.
	mif zoom;
	if (cy >= (int)src.m_height) {
		horz_filter(&src, &dst, x_fac, &filters[method], width);

		if (ondisk)
			zoom.create_width_mapfile(cx, cy, 24);
		else
			zoom.create(cx, cy, 24);
		vert_filter(&dst, &zoom, y_fac, &filters[method], width);
	}
	else {
		vert_filter(&src, &dst, y_fac, &filters[method], width);

		if (ondisk)
			zoom.create_width_mapfile(cx, cy, 24);
		else
			zoom.create(cx, cy, 24);
		horz_filter(&dst, &zoom, x_fac, &filters[method], width);
	}
	dst.destroy();

	src.movefrom(&zoom);
	return TRUE;
}

