////////////////////////////////////////////////////////////////////////
// gauss_sel.cpp
//      from gimp(sel_gauss.c)
//

#include "stdafx.h"
#include "lsmif.h"
#include "common.h"


static void __fastcall init_matrix(double radius, double** mat, int num)
{
	int    dx, dy;
	double sd, c1, c2;

	// This formula isn't really correct, but it'll do
	sd = radius / 3.329042969;
	c1 = 1.0 / sqrt(2.0 * M_PI * sd);
	c2 = -2.0 * (sd * sd);

	for (dy = 0; dy < num; dy++) {
		for (dx = dy; dx < num; dx++) {
			mat[dx][dy] = c1 * exp(((dx * dx)+ (dy * dy)) / c2);
			mat[dy][dx] = mat[dx][dy];
		}
	}
}

static void __fastcall matrixmult(mif& src, CRect& rc, double** mat, int numrad, int maxdelta)
{
src.progress_init(rc.Height() >> 3);
int prog = 0;

	mif dest(src);
	for (int y = rc.top; y < rc.bottom; y++) {
		RGBA* ps = dest.pos(rc.left, y);
		RGBA* pd = src.pos(rc.left, y);
		for (int x = rc.left; x < rc.right; x++) {
			double sum_r = 0.0, sum_g = 0.0, sum_b = 0.0;
			double fact_r = 0.0, fact_g = 0.0, fact_b = 0.0;
			for (int i = 1 - numrad; i < numrad; i++) {
				if (((x + i) < 0) || ((x + i) >= src.m_width))
					continue;

				int ii = abs(i);
				for (int j = 1 - numrad; j < numrad; j++) {
					if (((y + j) < 0)||((y + j) >= src.m_height))
						continue;

					int jj = abs(j);
					double d = mat[ii][jj];

					RGBA* psw = dest.pos(x + i, y + j);
					int tmp = (int)ps->red - (int)psw->red;
					if ((tmp <= maxdelta) && (tmp >= -maxdelta)) {
						sum_r += d * psw->red;
						fact_r += d;
					}

					tmp = (int)ps->green - (int)psw->green;
					if ((tmp <= maxdelta) && (tmp >= -maxdelta)) {
						sum_g += d * psw->green;
						fact_g += d;
					}

					tmp = (int)ps->blue - (int)psw->blue;
					if ((tmp <= maxdelta) && (tmp >= -maxdelta)) {
						sum_b += d * psw->blue;
						fact_b += d;
					}
				}
			}
			if (fact_r != 0.0) pd->red = (BYTE)(sum_r / fact_r);
			if (fact_g != 0.0) pd->green = (BYTE)(sum_g / fact_g);
			if (fact_b != 0.0) pd->blue = (BYTE)(sum_b / fact_b);
			ps++;
			pd++;
		}
if ((++prog & 7) == 0) src.progress_continue();
	}
src.progress_term();
}

BOOL blur_gauss_sel(mif& image, LPRECT prc, double radius, int maxdelta)
{
	// 1.0 <= radius <= 30.0
	// 0 <= maxdelta <= 255

	CRect rc;
	if (!image.validaterect(prc, &rc) || radius > 127.0) return FALSE;
	image.m_bits = 24;

	int numrad = (int)(radius + 1.0);

	double* mat_table = (double*)malloc(sizeof(double) * numrad * numrad);

	double* mat[128];
	for (int i = 0; i < numrad; i++)
		mat[i] = mat_table + i * numrad;
	init_matrix(radius, mat, numrad);

	matrixmult(image, rc, mat, numrad, maxdelta);

	free(mat_table);
	return TRUE;
}
