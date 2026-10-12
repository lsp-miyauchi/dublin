//////////////////////////////////////////////////////////////////////
// colorify.cpp

#include "stdafx.h"
#include "lsmif.h"
#include "common.h"


BOOL colorify(mif& image, LPRECT prc, COLORREF rgb)
{
	CRect rc;
	if (!image.validaterect(prc, &rc))
		return FALSE;
	image.m_bits = 24;
	int width = rc.Width();
	int height = rc.Height();

	mif dst;
	dst.create(width, height, 24);
	dst.blt(0, 0, width, height, &image, rc.left, rc.top);

	double lum_red_lookup[256], lum_green_lookup[256], lum_blue_lookup[256];
	int final_red_lookup[256], final_green_lookup[256], final_blue_lookup[256];
	for (int i = 0; i < 256; i ++) {
		lum_red_lookup[i] = i * 0.30;
		lum_green_lookup[i] = i * 0.59;
		lum_blue_lookup[i] = i * 0.11;
		final_red_lookup[i] = i * GetRValue(rgb) / 255;
		final_green_lookup[i] = i * GetGValue(rgb) / 255;
		final_blue_lookup[i] = i * GetBValue(rgb) / 255;
	}

	register RGBA* p = dst.m_pixels;
	int size = dst.m_packets;
	while (size--) {
		int lum = (int)(lum_red_lookup[p->red] +
						lum_green_lookup[p->green] +
						lum_blue_lookup[p->blue]);
		lum = ADJRGB(lum);
		p->red = final_red_lookup[lum];
		p->green = final_green_lookup[lum];
		p->blue = final_blue_lookup[lum];
		p++;
	}
	image.blt(rc.left, rc.top, width, height, &dst, 0, 0);
	return TRUE;
}
