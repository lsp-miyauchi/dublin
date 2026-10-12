///////////////////////////////////////////////////////////////////////
// color_enhanse.cpp

#include "stdafx.h"
#include "lsmif.h"
#include "common.h"

static void __fastcall calc_color_enhance(RGBA* pixel, int packets, double* hi, double* lo)
{
	*hi = 0.0, *lo = 1.0;
	double h, s, v;
	BYTE c, m, y, k;
	while (packets-- > 0)
	{
		c = 255 - pixel->red;
		m = 255 - pixel->green;
		y = 255 - pixel->blue;
		k = c;
		if (m < k)
			k = m;
		if (y < k)
			k = y;
		rgb_to_hsv(c - k, m - k, y - k, &h, &s, &v);
		if (v > *hi)
			*hi = v;
		if (v < *lo)
			*lo = v;
		pixel++;
	}
}

static void __fastcall apply_color_enhance(RGBA* pixel, int packets, double hi, double lo)
{
	double h, s, v;
	BYTE c, m, y, k;
	while (packets-- > 0)
	{
		c = 255 - pixel->red;
		m = 255 - pixel->green;
		y = 255 - pixel->blue;
		k = c;
		if (m < k)
			k = m;
		if (y < k)
			k = y;
		rgb_to_hsv(c - k, m - k, y - k, &h, &s, &v);
		if (hi != lo)
			v = (v - lo) / (hi - lo);
		hsv_to_rgb(&c, &m, &y, h, s, v);
		c += k;
		if (c > 255)
			c = 255;
		m += k;
		if (m > 255)
			m = 255;
		y += k;
		if (y > 255)
			y = 255;
		pixel->red = 255 - c;
		pixel->green = 255 - m;
		pixel->blue = 255 - y;
		pixel++;
	}
}

BOOL autocolorenhance(mif& src, LPRECT prc)
{
	double hi, lo;
	if (src.m_bits < 24)
	{
		calc_color_enhance(src.m_map, src.m_colors, &hi, &lo);
		apply_color_enhance(src.m_map, src.m_colors, hi, lo);
	}
	else
	{
		CRect rc;
		if (!src.validaterect(prc, &rc))
			return FALSE;

		mif dst;
		dst.create(rc.Width(), rc.Height(), src.m_bits);
		dst.blt(0, 0, rc.Width(), rc.Height(), &src, rc.left, rc.top);
		calc_color_enhance(dst.m_pixels, dst.m_packets, &hi, &lo);
		apply_color_enhance(dst.m_pixels, dst.m_packets, hi, lo);
		src.blt(rc.left, rc.top, rc.Width(), rc.Height(), &dst, 0, 0);
	}
	return TRUE;
}
