///////////////////////////////////////////////////////////////////////
// gamma.cpp

#include "stdafx.h"
#include "lsmif.h"
#include "common.h"

static void gamma_param(int* ptbl, double expon)
{
	register int i, newclr;
	double clr;

	for (i = 0; i < 256; i++) {
		clr = 255.0 * pow((i / 255.0), expon);
		newclr = (int)RINT(clr);
		ptbl[i] = ADJRGB(newclr);
	}
}

BOOL gamma_rgb(mif& src, LPRECT prc, double r_amt, double g_amt, double b_amt, double rgb_amt)
{
	CRect rc;
	if (!src.validaterect(prc, &rc))
		return FALSE;

	int r_tbl[256], g_tbl[256], b_tbl[256], rgb_tbl[256];
	for (int x = 0; x < 256; x++) {
		r_tbl[x] = x;
		g_tbl[x] = x;
		b_tbl[x] = x;
		rgb_tbl[x] = x;
	}

	if (r_amt != 1.0)	gamma_param(r_tbl, 1.0 / r_amt);
	if (g_amt != 1.0)	gamma_param(g_tbl, 1.0 / g_amt);
	if (b_amt != 1.0)	gamma_param(b_tbl, 1.0 / b_amt);
	if (rgb_amt != 1.0)	gamma_param(rgb_tbl, 1.0 / rgb_amt);

	register RGBA* p;
	register int cnt;
	if (src.m_bits < 24) {
		p = src.m_map;
		cnt = src.m_colors;
		while (cnt-- > 0) {
			p->red = rgb_tbl[r_tbl[p->red]];
			p->green = rgb_tbl[g_tbl[p->green]];
			p->blue = rgb_tbl[b_tbl[p->blue]];
			p++;
		}
		src.validate_rgb();
	}
	else {
		parallel_for (rc.top, rc.bottom, [&](int y) {
			RGBA* p = src.pos(rc.left, y);
			for (int x = rc.left; x < rc.right; x++) {
				p->red = rgb_tbl[r_tbl[p->red]];
				p->green = rgb_tbl[g_tbl[p->green]];
				p->blue = rgb_tbl[b_tbl[p->blue]];
				p++;
			}
		});
	}
	return TRUE;
}
