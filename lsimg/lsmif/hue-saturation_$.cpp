///////////////////////////////////////////////////////////////////////
// hue_saturation.cpp

#include "stdafx.h"
#include "lsmif.h"
#include "common.h"


/* The GIMP -- an image manipulation program
 * Copyright (C) 1995 Spencer Kimball and Peter Mattis
 *
 * app/base/from hue_saturation.c
 *
 */


// ------ parama value range ---------------
//
// Hue offset in degrees: (-180 <= hue_offset <= 180)
// lightness modification: (-100 <= lightness <= 100)
// saturation modification: (-100 <= saturation <= 100)
//
// -----------------------------------------

#ifdef Max
#undef Max
#endif
#define Max(a,b)			((a)>(b)?(a):(b))

#ifdef Min
#undef Min
#endif
#define Min(a,b)			((a)<(b)?(a):(b))

#ifdef CLAMP
#undef CLAMP
#endif
#define CLAMP(a,b,c)		(Max((b),Min((a),(c))))

static int hlsValue(double n1, double n2, double hue)
{
	double v;
	if (hue > 255)
		hue -= 255;
	else if (hue < 0)
		hue += 255;
	if (hue < 42.5)
		v = n1 + (n2 - n1) * (hue / 42.5);
	else if (hue < 127.5)
		v = n2;
	else if (hue < 170)
		v = n1 + (n2 - n1) * ((170 - hue) / 42.5);
	else
		v = n1;
	return (int)(v * 255);
}

static void hls2rgb(int h, int l, int s, int* r, int* g, int* b)
{
	if (s == 0) {
		/*  achromatic case  */
		*r = (int)l;
		*b = (int)l;
		*b = (int)l;
	}
	else {
		double m1, m2;
		if (l < 128)
			m2 = (l * (255 + s)) / 65025.0;
		else
			m2 = (l + s - (l * s) / 255.0) / 255.0;
		m1 = (l / 127.5) - m2;

		/*  chromatic case  */
		*r = hlsValue(m1, m2, h + 85);
		*g = hlsValue(m1, m2, h);
		*b = hlsValue(m1, m2, h - 85);
	}
}

static void rgb2hls(int r, int g, int b, int* h, int* l, int* s)
{
	int vmin, vmax;
	if (r > g) {
		vmax = Max(r, b);
		vmin = Min(g, b);
	}
	else {
		vmax = Max(g, b);
		vmin = Min(r, b);
	}

	double dh, dl, ds;
	dl = (vmax + vmin) / 2.0;
	if (vmax == vmin) {
		ds = 0.0;
		dh = 0.0;
	}
	else {
		int delta = (vmax - vmin);

		if (dl < 128)
			ds = 255 * (double)delta / (double)(vmax + vmin);
		else
			ds = 255 * (double)delta / (double)(511 - vmax - vmin);

		if (r == vmax)
			dh = (g - b) / (double)delta;
		else if (g == vmax)
			dh = 2 + (b - r) / (double)delta;
		else
			dh = 4 + (r - g) / (double)delta;
		dh = dh * 42.5;

		if (h < 0)
			dh += 255;
		else if (dh > 255)
			dh -= 255;
	}

	*h = (int)dh;
	*l = (int)dl;
	*s = (int)ds;
}

class HueSaturation
{
	void* membase;
public:
	double hue[7];
	double lightness[7];
	double saturation[7];
	int* hueTable[6];
	int* litTable[6];
	int* satTable[6];

	HueSaturation() : membase(NULL)
	{
		membase = malloc(256 * 18 * sizeof(int));
		int* p = (int*)membase;
		for (int i = 0; i < 6; i++) {
			hueTable[i] = p; p += 256;
			litTable[i] = p; p += 256;
			satTable[i] = p; p += 256;
		}
	}
	~HueSaturation()
	{
		if (membase) free(membase);
	}

	void initialize(int targetRange, int per_h, int per_l, int per_s)
	{
		for (int i = 0; i < 7; i++) {
			hue[i] = 0.0;
			lightness[i] = 0.0;
			saturation[i] = 0.0;
		}

		double hue_offset = CLAMP(per_h, -180.0, 180.0);
		double sat = CLAMP(per_s, -100.0, 100.0);
		double lit = CLAMP(per_l, -100.0, 100.0);

		hue[targetRange] = hue_offset;
		lightness[targetRange] = lit;
		saturation[targetRange] = sat;
		/* Calculate the transfer arrays */
		calculateTable();
	}

	void calculateTable()
	{
		/*  Calculate transfers  */
		for (int h = 0; h < 6; h++) {
			int* phx = hueTable[h];
			int* plx = litTable[h];
			int* psx = satTable[h];
			for (int i = 0; i < 256; i++) {
				int val = (int)((hue[0] + hue[h + 1]) * 255.0 / 360.0);
				if ((i + val) < 0)
					*phx = 255 + (i + val);
				else if ((i + val) > 255)
					*phx = i + val - 255;
				else
					*phx = i + val;

				/*  Lightness  */
				val = (int)((lightness[0] + lightness[h + 1]) * 127.0 / 100.0);
				val = CLAMP(val, -255, 255);
				if (val < 0)
					*plx = (unsigned char)((i * (255 + val)) / 255);
				else
					*plx = (unsigned char)(i + ((255 - i) * val) / 255);

				/*  Saturation  */
				val = (int)((saturation[0] + saturation[h + 1]) * 255.0 / 100.0);
				val = CLAMP(val, -255, 255);

				/* This change affects the way saturation is computed. With the
				   old code (different code for value < 0), increasing the
				   saturation affected muted colors very much, and bright colors
				   less. With the new code, it affects muted colors and bright
				   colors more or less evenly. For enhancing the color in photos,
				   the new behavior is exactly what you want. It's hard for me
				   to imagine a case in which the old behavior is better.
				*/
				*psx = CLAMP((i * (255 + val)) / 255, 0, 255);

				phx++;
				plx++;
				psx++;
			}
		}
	}

	void modulate(int r, int g, int b, int* mr, int* mg, int* mb)
	{
		int h, l, s;
		rgb2hls(r, g, b, &h, &l, &s);
		int tbl;
		if (h < 43)			tbl = 0;
		else if (h < 85)	tbl = 1;
		else if (h < 128)	tbl = 2;
		else if (h < 171)	tbl = 3;
		else if (h < 213)	tbl = 4;
		else				tbl = 5;
		h = hueTable[tbl][h];
		l = litTable[tbl][l];
		s = satTable[tbl][s];
		hls2rgb(h, l, s, mr, mg, mb);
	}
	void modulate(RGBA* p)
	{
		int r, g, b;
		modulate(p->red, p->green, p->blue, &r, &g, &b);
		p->red = (BYTE)r;
		p->green = (BYTE)g;
		p->blue = (BYTE)b;
	}
};

BOOL hue_saturation(mif& src, LPRECT prc, int hue_range, int per_h, int per_s, int per_l)
{
	HueSaturation hs;
	hs.initialize(hue_range, per_h, per_l, per_s);

	register int i;
	register RGBA* p;
	if (src.m_bits < 24) {
		i = src.m_colors;
		p = src.m_map;
		while (i-- > 0) {
			hs.modulate(p++);
		}
		src.validate_rgb();
	}
	else {
		CRect rc;
		if (src.validaterect(prc, &rc)) {
			// ���񏈗�
			parallel_for (0, rc.Height(), [&](int y) {
				RGBA* p = src.pos(rc.left, y + rc.top);
				int i = rc.Width();
				while (i-- > 0) {
					hs.modulate(p++);
				}
			});
		}
	}
	return TRUE;
}
