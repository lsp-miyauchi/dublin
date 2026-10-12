////////////////////////////////////////////////////////////////////////
// colorcut.cpp

#include "stdafx.h"
#include "lsmif.h"
#include "common.h"
#include "colorspace.h"


BOOL colorcut(mif& src, LPRECT prc, MIF_COLORCUT* pcp)
{
	CRect rc;
	if (!pcp || !src.validaterect(prc, &rc))
		return FALSE;
	src.m_bits = 24;

	int width = rc.Width();
	int height = rc.Height();

	mif dst;
	dst.create(width, height, 24);
	dst.blt(0, 0, width, height, &src, rc.left, rc.top);

	rgb2any ra(pcp->color_space);
	any2rgb ar(pcp->color_space);

	BYTE r_lut[256];
	BYTE g_lut[256];
	BYTE b_lut[256];

	int xstep = pcp->xstep ? pcp->xstep : 1;
	int ystep = pcp->ystep ? pcp->ystep : 1;
	int zstep = pcp->zstep ? pcp->zstep : 1;
	int i;
	for (i = 0; i < 256; i++) {
		r_lut[i] = i / xstep;
		if (i % xstep > xstep / 2)
			r_lut[i]++;
		r_lut[i] = ADJRGB(r_lut[i] * xstep);

		g_lut[i] = i / ystep;
		if (i % xstep > ystep / 2)
			g_lut[i]++;
		g_lut[i] = ADJRGB(g_lut[i] * ystep);

		b_lut[i] = i / zstep;
		if (i % zstep > zstep / 2)
			b_lut[i]++;
		b_lut[i] = ADJRGB(b_lut[i] * zstep);
	}

	register RGBA* p = dst.m_pixels;
	for (i = 0; i < dst.m_packets; i++) {
		ra.xfer((BYTE*)p);
		if (pcp->xstep)
			p->red = r_lut[p->red];
		if (pcp->ystep)
			p->green = g_lut[p->green];
		if (pcp->zstep)
			p->blue = b_lut[p->blue];
		ar.xfer((BYTE*)p);
		p++;
	}

	src.blt(rc.left, rc.top, width, height, &dst, 0, 0);
	return TRUE;
}
