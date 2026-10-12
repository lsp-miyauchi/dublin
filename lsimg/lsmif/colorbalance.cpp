////////////////////////////////////////////////////////////////////////
// colorbalance.cpp

#include "stdafx.h"
#include "lsmif.h"
#include "common.h"

#define SQR(x)			((x) * (x))
#define MINVAL(a,b)		((a)<(b)?(a):(b))
#define MAXVAL(a,b)		((a)>(b)?(a):(b))

enum {
	SHADOWS,
	MIDTONES,
	HIGHLIGHTS
};

typedef struct tagCOLORBALANCE {
	double cyan_red[3];
	double magenta_green[3];
	double yellow_blue[3];
	BYTE r_lookup[256];
	BYTE g_lookup[256];
	BYTE b_lookup[256];
	BOOL preserve_luminosity;
} COLORBALANCE;

static BOOL bColor_Init = FALSE;

// for lightening
static double  highlights_add[256];
static double  midtones_add[256];
static double  shadows_add[256];

// for darkening
static double  highlights_sub[256];
static double  midtones_sub[256];
static double  shadows_sub[256];

static void color_transfer_init()
{
	if (bColor_Init)
		return;

	int i;
	for (i = 0; i < 256; i++) {
		shadows_sub[255 - i] = (1.075 - 1 / ((double)i / 16.0 + 1));
		highlights_add[i] = shadows_sub[255 - i];
		midtones_sub[i] = 0.667 * (1 - SQR(((double)i - 127.0) / 127.0));
		midtones_add[i] = midtones_sub[i];
		highlights_sub[i] = 0.667 * (1 - SQR(((double)i - 127.0) / 127.0));
		shadows_add[i] = highlights_sub[i];
	}
	bColor_Init = TRUE;
}

static void make_lookup_tables(COLORBALANCE* cbd)
{
	color_transfer_init();

	double* cyan_red_xfer[3];
	double* magenta_green_xfer[3];
	double* yellow_blue_xfer[3];
	int i;
	int r_n, g_n, b_n;

	// Set the transfer arrays  (for speed)
	cyan_red_xfer[SHADOWS] =
			(cbd->cyan_red[SHADOWS] > 0) ? shadows_add : shadows_sub;
	cyan_red_xfer[MIDTONES] =
			(cbd->cyan_red[MIDTONES] > 0) ? midtones_add : midtones_sub;
	cyan_red_xfer[HIGHLIGHTS] =
			(cbd->cyan_red[HIGHLIGHTS] > 0) ? highlights_add : highlights_sub;

	magenta_green_xfer[SHADOWS] =
			(cbd->magenta_green[SHADOWS] > 0) ? shadows_add : shadows_sub;
	magenta_green_xfer[MIDTONES] =
			(cbd->magenta_green[MIDTONES] > 0) ? midtones_add : midtones_sub;
	magenta_green_xfer[HIGHLIGHTS] =
			(cbd->magenta_green[HIGHLIGHTS] > 0) ? highlights_add : highlights_sub;

	yellow_blue_xfer[SHADOWS] =
			(cbd->yellow_blue[SHADOWS] > 0) ? shadows_add : shadows_sub;
	yellow_blue_xfer[MIDTONES] =
			(cbd->yellow_blue[MIDTONES] > 0) ? midtones_add : midtones_sub;
	yellow_blue_xfer[HIGHLIGHTS] =
			(cbd->yellow_blue[HIGHLIGHTS] > 0) ? highlights_add : highlights_sub;

	for (i = 0; i < 256; i++) {
		r_n = i;
		g_n = i;
		b_n = i;

		r_n += (int)((double)cbd->cyan_red[SHADOWS] * cyan_red_xfer[SHADOWS][r_n]);
		r_n = ADJRGB(r_n);
		r_n += (int)((double)cbd->cyan_red[MIDTONES] * cyan_red_xfer[MIDTONES][r_n]);
		r_n = ADJRGB(r_n);
		r_n += (int)((double)cbd->cyan_red[HIGHLIGHTS] * cyan_red_xfer[HIGHLIGHTS][r_n]);
		r_n = ADJRGB(r_n);

		g_n += (int)((double)cbd->magenta_green[SHADOWS] * magenta_green_xfer[SHADOWS][g_n]);
		g_n = ADJRGB(g_n);
		g_n += (int)((double)cbd->magenta_green[MIDTONES] * magenta_green_xfer[MIDTONES][g_n]);
		g_n = ADJRGB(g_n);
		g_n += (int)((double)cbd->magenta_green[HIGHLIGHTS] * magenta_green_xfer[HIGHLIGHTS][g_n]);
		g_n = ADJRGB(g_n);

		b_n += (int)((double)cbd->yellow_blue[SHADOWS] * yellow_blue_xfer[SHADOWS][b_n]);
		b_n = ADJRGB(b_n);
		b_n += (int)((double)cbd->yellow_blue[MIDTONES] * yellow_blue_xfer[MIDTONES][b_n]);
		b_n = ADJRGB(b_n);
		b_n += (int)((double)cbd->yellow_blue[HIGHLIGHTS] * yellow_blue_xfer[HIGHLIGHTS][b_n]);
		b_n = ADJRGB(b_n);

		cbd->r_lookup[i] = r_n;
		cbd->g_lookup[i] = g_n;
		cbd->b_lookup[i] = b_n;
	}
}

static void color_balance(RGBA* pdata, int packets, COLORBALANCE* cbd)
{
	register RGBA* p = pdata;
	while (packets--) {
		int r = p->red;
		int g = p->green;
		int b = p->blue;
		int r_n = cbd->r_lookup[r];
		int g_n = cbd->g_lookup[g];
		int b_n = cbd->b_lookup[b];

		if (cbd->preserve_luminosity) {
			rgb_to_hls(&r_n, &g_n, &b_n);
			g_n = rgb_to_l(r, g, b);
			hls_to_rgb(&r_n, &g_n, &b_n);
		}

		p->red = r_n;
		p->green = g_n;
		p->blue = b_n;
		p++;
	}
}

BOOL colorbalance(mif& image, LPRECT prc, MIF_COLORBALANCE* pcb)
{
	if (!pcb)
		return FALSE;

	COLORBALANCE cbd;
	for (int i = 0; i < 3; i++) {
		cbd.cyan_red[i] = MAXVAL(-100, MINVAL(100, pcb->cyan_red[i]));
		cbd.magenta_green[i] = MAXVAL(-100, MINVAL(100, pcb->magenta_green[i]));
		cbd.yellow_blue[i] = MAXVAL(-100, MINVAL(100, pcb->yellow_blue[i]));
	}
	cbd.preserve_luminosity = pcb->preserve_lum;
	make_lookup_tables(&cbd);

	if (image.m_bits < 24)
		color_balance(image.m_map, image.m_colors, &cbd);
	else {
		CRect rc;
		if (!image.validaterect(prc, &rc))
			return FALSE;

		mif cb;
		cb.create(rc.Width(), rc.Height(), 24);
		cb.blt(0, 0, rc.Width(), rc.Height(), &image, rc.left, rc.top);
		color_balance(cb.m_pixels, cb.m_packets, &cbd);
		image.blt(rc.left, rc.top, rc.Width(), rc.Height(), &cb, 0, 0);
		cb.destroy();
	}
	return TRUE;
}
