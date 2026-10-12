///////////////////////////////////////////////////////////////////////
// cubism.cpp

#include "stdafx.h"
#include "polygon.h"
#include "cubism.h"

void progress_init(int maxprogress);
void progress_continue();
void progress_term();

#define SQR(a) ((a) * (a))

#ifndef RAND_MAX
#define RAND_MAX			2147483647
#endif /* RAND_MAX */

#define M_PI				(3.14159265358979323846)
#define SUPERSAMPLE			3
#define MAX_POINTS			4
#define RANDOMNESS			5
#define MIXED(a,b,c)		((((a)*(c))+((b)*(255-(c))))/255)

typedef struct tagCUBISM {
	double tile_size;				// 0.0 - 100.0 Average diameter of tiles (in pixels)
	double tile_saturation;			// 0.0 - 10.0 Expand tiles by this amount
	COLORREF bg_color;
	int numpolygon;
} CUBISM;

static double calc_alpha_blend(double *vec, double oneoverdist, double x, double y)
{
	double r;
	if (!oneoverdist)
		return 1.0;
	else {
		r = (vec[0] * x + vec[1] * y) * oneoverdist;
		if (r < 0.2)		r = 0.2;
		else if (r > 1.0)	r = 1.0;
	}
	return r;
}

_inline int int_rand(int val)
{
	return (rand () % val);
}

_inline double fp_rand(double val)
{
	return ((double)rand () / (double)(RAND_MAX - 1)) * val;
}

static void convert_segment (int  x1, int  y1, int  x2, int  y2, int  offset, int *min, int *max)
{
	int ydiff, y, tmp;
	double xinc, xstart;

	if (y1 > y2) {
		tmp = y2; y2 = y1; y1 = tmp;
 		tmp = x2; x2 = x1; x1 = tmp;
	}
	ydiff = (y2 - y1);

	if ( ydiff ) {
		xinc = (double) (x2 - x1) / (double) ydiff;
		xstart = x1 + 0.5 * xinc;
		for (y = y1 ; y < y2; y++) {
			if (xstart < min[y - offset]) min[y - offset] = (int)xstart;
			if (xstart > max[y - offset]) max[y - offset] = (int)xstart;
			xstart += xinc;
		}
	}
}

static void fill_poly_color(polygon* poly, mif& src, RGBA col)
{
	int xs, ys;
	int xe, ye;
	int val;
	int alpha;
	int i, j, x, y;
	double xx, yy;
	double vec[2];
	int *vals, *vals_iter, *vals_end;

	double sx = poly->pts[0].x;
	double sy = poly->pts[0].y;
//	double ex = poly->pts[1].x;
//	double ey = poly->pts[1].y;
	double ex = poly->pts[poly->npts / 2].x;
	double ey = poly->pts[poly->npts / 2].y;

	double dist = sqrt(SQR(ex - sx) + SQR (ey - sy));
	double one_over_dist;
	if (dist > 0.0) {
		one_over_dist = 1 / dist;
		vec[0] = (ex - sx) * one_over_dist;
		vec[1] = (ey - sy) * one_over_dist;
	}
	else
		one_over_dist = 0.0;

	int supersample = SUPERSAMPLE;
	int supersample2 = SQR(supersample);

	double dmin_x, dmin_y, dmax_x, dmax_y;
	poly->extents(&dmin_x, &dmin_y, &dmax_x, &dmax_y);

	int min_x = (int)dmin_x;
	int min_y = (int)dmin_y;
	int max_x = (int)dmax_x;
	int max_y = (int)dmax_y;

	int size_y = (max_y - min_y) * supersample;
	int size_x = (max_x - min_x) * supersample;

	int *max_scanlines, *max_scanlines_iter;
	int *min_scanlines, *min_scanlines_iter;
	min_scanlines = min_scanlines_iter = (int*)malloc(sizeof(int) * size_y);
	max_scanlines = max_scanlines_iter = (int*)malloc(sizeof(int) * size_y);
	for (i = 0; i < size_y; i++) {
		min_scanlines[i] = max_x * supersample;
		max_scanlines[i] = min_x * supersample;
	}

	if (poly->npts) {
		int poly_npts = poly->npts;
		VERTEX* curptr;

		xs = (int)(poly->pts[poly_npts - 1].x);
		ys = (int)(poly->pts[poly_npts - 1].y);
		xe = (int)poly->pts[0].x;
		ye = (int)poly->pts[0].y;

		xs *= supersample;
		ys *= supersample;
		xe *= supersample;
		ye *= supersample;

		convert_segment(xs, ys, xe, ye, min_y * supersample, min_scanlines, max_scanlines);

		for (i = 1, curptr = &poly->pts[0]; i < poly_npts; i++) {
			xs = (int)curptr->x;
			ys = (int)curptr->y;
			curptr++;
			xe = (int)curptr->x;
			ye = (int)curptr->y;

			xs *= supersample;
			ys *= supersample;
			xe *= supersample;
			ye *= supersample;
			convert_segment(xs, ys, xe, ye, min_y * supersample, min_scanlines, max_scanlines);
		}
	}

	vals = (int*)malloc(sizeof(int) * size_x);
	for (i = 0; i < size_y; i++, min_scanlines_iter++, max_scanlines_iter++) {
		if (! (i % supersample))
			memset(vals, 0, sizeof(int) * size_x);

		yy = (double)i / (double)supersample + min_y;

		for (j = *min_scanlines_iter; j < *max_scanlines_iter; j++) {
			x = j - min_x * supersample;
			vals[x] += 255;
		}

		if (!((i + 1) % supersample)) {
			y = (i / supersample) + min_y;

			if (y >= 0 && y < src.m_height) {
				for (j = 0; j < size_x; j += supersample) {
					x = (j / supersample) + min_x;

					if (x >= 0 && x < src.m_width) {
						val = 0;
						vals_iter = &vals[j];
						vals_end = &vals_iter[supersample];
						for ( ; vals_iter < vals_end; vals_iter++)
							val += *vals_iter;

						val /= supersample2;

		 				if (val > 0) {
							xx = (double) j / (double) supersample + min_x;
							alpha = (int)(val * calc_alpha_blend(vec, one_over_dist, xx - sx, yy - sy));
							RGBA* ps = src.pos(x, y);
							ps->red = MIXED(col.red, ps->red, alpha);
							ps->green = MIXED(col.green, ps->green, alpha);
							ps->blue = MIXED(col.blue, ps->blue, alpha);
						}
					}
				}
			}
		}
    }
	free (vals);
	free (min_scanlines);
	free (max_scanlines);
}

static void randomize_indices (int count, int *indices)
{
	for (int i = 0; i < count * RANDOMNESS; i++) {
		int index1 = int_rand(count);
		int index2 = int_rand(count);
		int tmp = indices[index1];
		indices[index1] = indices[index2];
		indices[index2] = tmp;
	}
}

static void run_cubism(mif& src, CRect& rc, CUBISM& cfg)
{
	srand(0);

	mif dst(rc.Width(), rc.Height(), 24);
	RGBA* pd = dst.m_pixels;
	int count = dst.m_packets;
	RGBA bk;
	bk.red = GetRValue(cfg.bg_color);
	bk.green = GetGValue(cfg.bg_color);
	bk.blue = GetBValue(cfg.bg_color);
	while (count-- > 0)
		*pd++ = bk;

	int cols = (int)((dst.m_width + cfg.tile_size - 1) / cfg.tile_size);
	int rows = (int)((dst.m_height + cfg.tile_size - 1) / cfg.tile_size);

	int num_tiles = (rows + 1) * (cols + 1);
	int* random_indices = (int*)malloc(sizeof(int) * num_tiles);
	for (int i = 0; i < num_tiles; i++)
		random_indices[i] = i;
	randomize_indices(num_tiles, random_indices);

	progress_init(num_tiles >> 4);

	polygon poly(cfg.numpolygon);
	count = 0;
	while (count < num_tiles) {
		int i = random_indices[count] / (cols + 1);
		int j = random_indices[count] % (cols + 1);
		double x = j * cfg.tile_size + (cfg.tile_size / 4.0) - fp_rand(cfg.tile_size / 2.0);
		double y = i * cfg.tile_size + (cfg.tile_size / 4.0) - fp_rand (cfg.tile_size / 2.0);
		double width = (cfg.tile_size + fp_rand(cfg.tile_size / 4.0) - cfg.tile_size / 8.0) * cfg.tile_saturation;
		double height = (cfg.tile_size + fp_rand(cfg.tile_size / 4.0) - cfg.tile_size / 8.0) * cfg.tile_saturation;
		double theta = fp_rand(2 * M_PI);
		poly.reset();

		int numpol = cfg.numpolygon;
		if (numpol < 3 || numpol > 8)
			numpol = int_rand(6) + 3;

		if (numpol == 3) {
			poly.add_point(0, -height / 2.0);
			poly.add_point(width / 2.0, height / 2.0);
			poly.add_point(-width / 2.0, height / 2.0);
		}
		else if (numpol == 5) {
			poly.add_point(0, -height / 2.0);
			poly.add_point(-width / 2.0, 0);
			poly.add_point(-width / 3.0, height / 2.0);
			poly.add_point(width / 3.0, height / 2.0);
			poly.add_point(width / 2.0, 0);
		}
		else if (numpol == 6) {
			poly.add_point(width/6.0, -height / 2.0);
			poly.add_point(-width/6.0, -height / 2.0);
			poly.add_point(-width / 2.0, 0);
			poly.add_point(-width/6.0, height / 2.0);
			poly.add_point(width/6.0, height / 2.0);
			poly.add_point(width / 2.0, 0);
		}
		else if (numpol == 7) {
			poly.add_point(0, -height / 2.0);
			poly.add_point(-width/ 2.0, -height/6.0);
			poly.add_point(-width/ 2.0, height/6.0);
			poly.add_point(-width/6.0, height / 2.0);
			poly.add_point(width/6.0, height / 2.0);
			poly.add_point(width/ 2.0, height/6.0);
			poly.add_point(width/ 2.0, -height/6.0);
		}
		else if (numpol == 8) {
			poly.add_point(width/6.0, -height / 2.0);
			poly.add_point(-width/6.0, -height / 2.0);
			poly.add_point(-width/ 2.0, -height/6.0);
			poly.add_point(-width/ 2.0, height/6.0);
			poly.add_point(-width/6.0, height / 2.0);
			poly.add_point(width/6.0, height / 2.0);
			poly.add_point(width/ 2.0, height/6.0);
			poly.add_point(width/ 2.0, -height/6.0);
		}
		else {
			poly.add_point(-width / 2.0, -height / 2.0);
			poly.add_point(width / 2.0, -height / 2.0);
			poly.add_point(width / 2.0, height / 2.0);
			poly.add_point(-width / 2.0, height / 2.0);
		}
		poly.rotate(theta);
		poly.translate(x, y);

		// bounds check on x, y
		int ix = (int)x;
		int iy = (int)y;
		if (ix < 0)				ix = 0;
		if (ix >= dst.m_width)	ix = dst.m_width - 1;
		if (iy < 0)				iy = 0;
		if (iy >= dst.m_height)	iy = dst.m_height - 1;

		fill_poly_color(&poly, dst, *(src.pos(ix + rc.left, iy + rc.top)));
		count++;

		if (!(count & 0xf))
			progress_continue();
	}
	free(random_indices);
	src.blt(rc.left, rc.top, dst.m_width, dst.m_height, &dst, 0, 0);

	progress_term();
}

HDIB cubism(HDIB hsrc, LPRECT prc, CUBISMINFO* pcp)
{
	CRect rc;
	mif src;
	if (!src.attachdib(hsrc) || !src.validaterect(prc, &rc))
		return NULL;
	src.m_bits = 24;

	CUBISM cfg;
	cfg.tile_size = 5.0;
	cfg.tile_saturation = 2.5;
	cfg.bg_color = RGB(0, 0, 0);
	cfg.numpolygon = 5;
	if (pcp) {
		cfg.tile_size = (double)pcp->tile_size;
		cfg.tile_saturation = (double)pcp->tile_saturation / 100.0;
		cfg.bg_color = pcp->bg_color;
		cfg.numpolygon = pcp->numpolygon;
	}

	run_cubism(src, rc, cfg);
	return src.createdib();
}
