///////////////////////////////////////////////////////////////////////
// modulate_lab.cpp

#include "stdafx.h"
#include "lsmif.h"
#include "common.h"

// 0 <= L <= 100
// -85 <= a <= 98
// -107 <= b <= 94

static void __fastcall labmod_pixel(RGBA* pix, int per_L, int per_a, int per_b)
{
	int L, a, b;
	rgb_lab(pix->red, pix->green, pix->blue, &L, &a, &b);

	// 処理

	double R, G, B;

	lab_rgb(L, a, b, &R, &G, &B);
	pix->red = (int)R;
	pix->green = (int)G;
	pix->blue = (int)B;
}

BOOL labmodulate(mif& src, LPRECT prc, int per_L, int per_a, int per_b)
{
	// 範囲はすべて -128 ～ 128
	if (src.m_bits < 24) {
		int i = src.m_colors;
		RGBA* p = src.m_map;
		while (i-- > 0) {
			labmod_pixel(p, per_L, per_a, per_b);
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
				int i = rc.Width();
				while (i-- > 0) {
					labmod_pixel(p, per_L, per_a, per_b);
					p++;
				}
			});
			return TRUE;
		}
	}
	return FALSE;
}
