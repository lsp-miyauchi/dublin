///////////////////////////////////////////////////////////////////////
// modulate.cpp

#include "stdafx.h"
#include "lsmif.h"
#include "common.h"

static void __fastcall modulate_pixel(RGBA* pix, int per_h, int per_s, int per_l)
{
	int h = pix->red;
	int l = pix->green;
	int s = pix->blue;
	rgb_to_hls(&h, &l, &s);

	l = ADJRGB(l + per_l);
	s = ADJRGB(s + ((s * per_s) >> 7));
	if (h != 0) {
		h += per_h;
		if (h < 0)
			h += 255;
		else if (h > 255)
			h -= 255;
	}
	hls_to_rgb(&h, &l, &s);

	pix->red = ADJRGB(h);
	pix->green = ADJRGB(l);
	pix->blue = ADJRGB(s);
}

BOOL modulate(mif& src, LPRECT prc, int per_h, int per_s, int per_l)
{
	// �͈͂͂��ׂ� -128 �` 128
	if (src.m_bits < 24) {
		int numcol = src.m_colors;
		RGBA* p = src.m_map;
		while (numcol-- > 0) {
			modulate_pixel(p, per_h, per_s, per_l);
			p++;
		}
		src.validate_rgb();
		return TRUE;
	}
	else {
		CRect rc;
		if (src.validaterect(prc, &rc)) {
			parallel_for (0, rc.Height(), [&](int y) {
				RGBA* p = src.pos(rc.left, y + rc.top);
				int width = rc.Width();
				while (width-- > 0) {
					modulate_pixel(p, per_h, per_s, per_l);
					p++;
				}
			});
			return TRUE;
		}
	}
	return FALSE;
}
