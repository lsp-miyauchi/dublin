//////////////////////////////////////////////////////////////////////
// posterize.cpp

#include "stdafx.h"
#include "lsmif.h"
#include "common.h"

static double posterize_lut_func(int *ilevels, double value)
{
	int levels;
	if (*ilevels < 2)
		levels = 2;
	else
		levels = *ilevels;

	value = RINT(value * (levels - 1.0)) / (levels - 1.0);
	return value;
}

static void posterize_lut_setup(BYTE* lut, int levels)
{
	for (int i = 0; i < 256; i++) {
		// to add gamma correction use func(v ^ g) ^ 1/g instead.
		double val = 255.0 * posterize_lut_func(&levels, i / 255.0) + 0.5;
		if (val < 0.0)
			lut[i] = 0;
		else if (val >= 255.0)
			lut[i] = 255;
		else
			lut[i] = (int)val;
	}
}

BOOL posterize(mif& src, LPRECT prc, int levels)
{
	// 2 <= levels <= 10

	BYTE lut[256];
	posterize_lut_setup(lut, levels);

	if (src.m_bits < 24) {
		RGBA* p = src.m_map;
		for (int i = 0; i < src.m_colors; i++) {
			p->red = lut[p->red];
			p->green = lut[p->green];
			p->blue = lut[p->blue];
			p++;
		}
		src.validate_rgb();
	}
	else {
		CRect rc;
		if (!src.validaterect(prc, &rc))
			return FALSE;

		parallel_for (rc.top, rc.bottom, [&](int y) {
			RGBA* p = src.pos(rc.left, y);
			for (int x = rc.left; x < rc.right; x++) {
				p->red = lut[p->red];
				p->green = lut[p->green];
				p->blue = lut[p->blue];
				p++;
			}
		});
	}
	return TRUE;
}
