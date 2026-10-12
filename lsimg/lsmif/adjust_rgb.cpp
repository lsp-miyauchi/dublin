///////////////////////////////////////////////////////////////////////
// adjust_rgb.cpp

#include "stdafx.h"
#include "lsmif.h"
#include "common.h"

#define DOWNSCALE(a)		(((WORD)(a)) >> 8)
#define UPSCALE(a)			(((WORD)(a)) * 257)
#define ADJRGB16(a)			(min(65535, max(0, (int)(a))))

static void __fastcall adjust_pixel(RGBA* p, int red, int green, int blue)
{
	int r = UPSCALE(p->red);
	r += (r * red) / 100;
	r = ADJRGB16(r);
	p->red = DOWNSCALE(r);

	int g = UPSCALE(p->green);
	g += (g * green) / 100;
	g = ADJRGB16(g);
	p->green = DOWNSCALE(g);

	int b = UPSCALE(p->blue);
	b += (b * blue) / 100;
	b = ADJRGB16(b);
	p->blue = DOWNSCALE(b);
}

BOOL adjust_rgb(mif& src, LPRECT prc, int red, int green, int blue)
{
	CRect rc;
	if (!src.validaterect(prc, &rc))
		return FALSE;

	register RGBA* p;
	if (src.m_bits == 24)
	{
		for (int y = rc.top; y < rc.bottom; y++)
		{
			p = src.pos(rc.left, y);
			for (int x = rc.left; x < rc.right; x++)
			{
				adjust_pixel(p, red, green, blue);
				p++;
			}
		}
	}
	else
	{
		p = src.m_map;
		for (int i = 0; i < src.m_colors; i++)
		{
			adjust_pixel(p, red, green, blue);
			p++;
		}
		src.validate_rgb();
	}
	return TRUE;
}

