///////////////////////////////////////////////////////////////////////
// blur.cpp

#include "stdafx.h"
#include "lsmif.h"
#include "common.h"

void blur(mif& src, LPRECT prc, int x_factor, int y_factor)
{
	CRect rc;
	if (!src.validaterect(prc, &rc))
		return;

	src.m_bits = 24;
	mif dst;
	dst.copyfrom(&src);

	register RGBA *ps, *pd;
	int fx = x_factor >> 1;
	int fy = y_factor >> 1;
	for (int y = rc.top; y < rc.bottom; y++) {
		pd = dst.pos(rc.left, y);
		for (int x = rc.left; x < rc.right; x++) {
			int r = 0;
			int g = 0;
			int b = 0;
			int numdiv = 0;

			if (x - fx >= 0 && y - fy >= 0) {
				ps = src.pos(x - fx, y - fy);
				r += ps->red;
				g += ps->green;
				b += ps->blue;
				numdiv++;
			}
			if (x - fx >= 0 && y + fy < src.m_height) {
				ps = src.pos(x - fx, y + fy);
				r += ps->red;
				g += ps->green;
				b += ps->blue;
				numdiv++;
			}
			if (x + fx < src.m_width && y - fy >= 0) {
				ps = src.pos(x + fx, y - fy);
				r += ps->red;
				g += ps->green;
				b += ps->blue;
				numdiv++;
			}
			if (x + fx < src.m_width && y + fy < src.m_height) {
				ps = src.pos(x + fx, y + fy);
				r += ps->red;
				g += ps->green;
				b += ps->blue;
				numdiv++;
			}

			if (numdiv) {
				pd->red = ADJRGB(r / numdiv);
				pd->green = ADJRGB(g / numdiv);
				pd->blue = ADJRGB(b / numdiv);
			}
			pd++;
		}
	}
	src.blt(rc.left, rc.top, rc.Width(), rc.Height(), &dst, rc.left, rc.top);
}

BOOL blur_simple(mif& src, RECT* prc, int radius)
{
	CRect rc;
	if (!src.validaterect(prc, &rc))
		return FALSE;

	int width = rc.Width();
	int height = rc.Height();
	if (src.m_bits < 24)
		src.m_bits = 24;

	mif dst(width, height, 24);

	int* curve_base = (int*)malloc((radius * 2 + 1) * sizeof(int));
	int* curve = curve_base + radius;
	for (int z = -radius; z <= radius; z++)
		curve[z] = (radius + 1) - abs(z);

	parallel_for (rc.top, rc.bottom, [&](int y) {
		RGBA* pd = dst.pos(0, y - rc.top);
		RGBA* ps = src.pos(rc.left, y);
		for (int x = rc.left; x < rc.right; x++) {
			int cnt = 0, r = 0, g = 0, b = 0, a = 0;
			for (int z = -radius; z <= radius; z++) {
				if (x + z >= 0 && x + z < src.m_width) {
					r += curve[z] * (ps + z)->red;
					g += curve[z] * (ps + z)->green;
					b += curve[z] * (ps + z)->blue;
					cnt += curve[z];
				}
			}

			pd->red = r / cnt;
			pd->green = g / cnt;
			pd->blue = b / cnt;
			pd->alpha = a / cnt;
			ps++;
			pd++;
		}
	});

	parallel_for (0, width, [&](int x) {
		for (int y = 0; y < height; y++) {
			RGBA* ps = src.pos(x + rc.left, y + rc.top);
			RGBA* pd = dst.pos(x, y);

			int cnt = 0, r = 0, g = 0, b = 0, a = 0;
			for (int z = -radius; z <= radius; z++) {
				if (y + z >= 0 && y + z < dst.m_height) {
					r += curve[z] * (pd + z * dst.m_width)->red;
					g += curve[z] * (pd + z * dst.m_width)->green;
					b += curve[z] * (pd + z * dst.m_width)->blue;
					cnt += curve[z];
				}
			}

			ps->red = r / cnt;
			ps->green = g / cnt;
			ps->blue = b / cnt;
			ps->alpha = a / cnt;
		}
	});
	free(curve_base);
	return TRUE;
}
