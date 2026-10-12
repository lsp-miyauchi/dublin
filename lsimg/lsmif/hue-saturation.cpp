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

#ifndef MAX
	#define MAX(a,b)		((a)>(b)?(a):(b))
#endif

#ifndef MIN
	#define MIN(a,b)		((a)<(b)?(a):(b))
#endif

#ifdef CLAMP
	#undef CLAMP
#endif
#define CLAMP(a,b,c)		(MAX((b),MIN((a),(c))))

class HueSaturation
{
	void* membase;
public:
	double hue[7];
	double lightness[7];
	double saturation[7];
	int* hue_table[6];
	int* lit_table[6];
	int* sat_table[6];

	HueSaturation() : membase(NULL)
	{
		membase = malloc(256 * 18 * sizeof(int));
		int* p = (int*)membase;
		for (int i = 0; i < 6; i++) {
			hue_table[i] = p; p += 256;
			lit_table[i] = p; p += 256;
			sat_table[i] = p; p += 256;
		}
	}
	~HueSaturation()
	{
		if (membase) free(membase);
	}
protected:
	void partition_reset(int partition)
	{
		hue[partition] = 0.0;
		lightness[partition] = 0.0;
		saturation[partition] = 0.0;
	}
	void calculate_tables()
	{
		int value;
		int i;
		/*  Calculate transfers  */
		for (int h = 0; h < 6; h++) {
			for (i = 0; i < 256; i++) {
				value = (int)((hue[0] + hue[h + 1]) * 255.0 / 360.0);
				if ((i + value) < 0)
					hue_table[h][i] = 255 + (i + value);
				else if ((i + value) > 255)
					hue_table[h][i] = i + value - 255;
				else
					hue_table[h][i] = i + value;

				/*  Lightness  */
				value = (int)((lightness[0] + lightness[h + 1]) * 127.0 / 100.0);
				value = CLAMP(value, -255, 255);
				if (value < 0)
					lit_table[h][i] = (unsigned char)((i * (255 + value)) / 255);
				else
					lit_table[h][i] = (unsigned char)(i + ((255 - i) * value) / 255);

				/*  Saturation  */
				value = (int)((saturation[0] + saturation[h + 1]) * 255.0 / 100.0);
				value = CLAMP(value, -255, 255);

				/* This change affects the way saturation is computed. With the
				   old code (different code for value < 0), increasing the
				   saturation affected muted colors very much, and bright colors
				   less. With the new code, it affects muted colors and bright
				   colors more or less evenly. For enhancing the color in photos,
				   the new behavior is exactly what you want. It's hard for me
				   to imagine a case in which the old behavior is better.
				*/
				sat_table[h][i] = CLAMP((i * (255 + value)) / 255, 0, 255);
			}
		}
	}
public:
	void init_table(int hue_range, int per_h, int per_s, int per_l)
	{
		for (int partition = huesAll; partition <= huesMagenta; partition++)
			partition_reset(partition);

		hue[hue_range] = CLAMP(per_h, -180.0, 180.0);
		lightness[hue_range] = CLAMP(per_l, -100.0, 100.0);
		saturation[hue_range] = CLAMP(per_s, -100.0, 100.0);
		/* Calculate the transfer arrays */
		calculate_tables();
	}
	void pixel_convert(RGBA* pix)
	{
		int h, l, s;
		rgb2hls(pix->red, pix->green, pix->blue, &h, &l, &s);

		int t;
		if (h < 43)			t = 0;
		else if (h < 85)	t = 1;
		else if (h < 128)	t = 2;
		else if (h < 171)	t = 3;
		else if (h < 213)	t = 4;
		else				t = 5;

		h = hue_table[t][h];
		l = lit_table[t][l];
		s = sat_table[t][s];

		int r, g, b;
		hls2rgb(h, l, s, &r, &g, &b);

		pix->red = (BYTE)r;
		pix->green = (BYTE)g;
		pix->blue = (BYTE)b;
	}

	static int hlsValue(double n1, double n2, double hue)
	{
		double v;
		if (hue > 255)	hue -= 255;
		else if (hue < 0)	hue += 255;

		if (hue < 42.5)	v = n1 + (n2 - n1) * (hue / 42.5);
		else if (hue < 127.5)	v = n2;
		else if (hue < 170)		v = n1 + (n2 - n1) * ((170 - hue) / 42.5);
		else					v = n1;
		return (int)(v * 255);
	}

	static void hls2rgb(double h, double l, double s, int* r, int* g, int* b)
	{
		double m1, m2;

		if (s == 0) {
			/*  achromatic case  */
			*r = (int)l;
			*g = (int)l;
			*b = (int)l;
		}
		else {
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
			vmax = MAX(r, b);
			vmin = MIN(g, b);
		}
		else {
			vmax = MAX(g, b);
			vmin = MIN(r, b);
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

			if (dh < 0)
				dh += 255;
			else if (dh > 255)
				dh -= 255;
		}

		*h = (int)dh;
		*l = (int)dl;
		*s = (int)ds;
	}
};

BOOL hue_saturation(mif& src, LPRECT prc, int hue_range, int per_h, int per_s, int per_l)
{
	register int i;
	register RGBA* p;

	HueSaturation hs;
	hs.init_table(hue_range, per_h, per_s, per_l);

	if (src.m_bits < 24) {
		i = src.m_colors;
		p = src.m_map;
		while (i-- > 0) {
			hs.pixel_convert(p++);
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
					hs.pixel_convert(p++);
				}
			});
		}
	}
	return TRUE;
}
