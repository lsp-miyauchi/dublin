////////////////////////////////////////////////////////////////////////
// bumpmap.cpp

#include "stdafx.h"
#include "lsmif.h"
#include "common.h"

#define CLAMP(a,b,c)	min((c), max((a),(b)))

/* The GIMP -- an image manipulation program
 * Copyright (C) 1995 Spencer Kimball and Peter Mattis
 *
 * Bump map plug-in --- emboss an image by using another image as a bump map
 * Copyright (C) 2000 Sven Neumann <sven@gimp.org>
 */


/* This plug-in uses the algorithm described by John Schlag, "Fast
 * Embossing Effects on Raster Image Data" in Graphics Gems IV (ISBN
 * 0-12-336155-9).  It takes a grayscale image to be applied as a
 * bump-map to another image, producing a nice embossing effect.
 */

/* Version 3.0-pre1-ac2:
 *
 * - waterlevel/ambient restricted to 0-255
 * - correctly initialize bumpmap_offsets
 */   


typedef struct {
	double lx, ly;			/* X and Y components of light vector */
	double nz2, nzlz;		/* nz^2, nz*lz */
	double background;		/* Shade for vertical normals */
	double compensation;	/* Background compensation */
	BYTE lut[256];			/* Look-up table for modes */
} bumpmap_params_t;

static MIF_BUMPMAP bmvals;
static int sel_x1, sel_y1;
static int sel_x2, sel_y2;
static int sel_width, sel_height;

static void bumpmap_init_params(bumpmap_params_t *params)
{
	double lz, nz;
	int i;
	double n;

	/* Convert to radians */
	double azimuth   = M_PI * bmvals.azimuth / 180.0;
	double elevation = M_PI * bmvals.elevation / 180.0;

	/* Calculate the light vector */
	params->lx = cos(azimuth) * cos(elevation) * 255.0;
	params->ly = sin(azimuth) * cos(elevation) * 255.0;
	lz         = sin(elevation) * 255.0;

	/* Calculate constant Z component of surface normal */
	nz           = (6 * 255) / bmvals.depth;
	params->nz2  = nz * nz;
	params->nzlz = nz * lz;

	/* Optimize for vertical normals */
	params->background = lz;

	/* Calculate darkness compensation factor */
	params->compensation = sin(elevation);

	/* Create look-up table for map type */
	for (i = 0; i < 256; i++) {
		switch (bmvals.type) {
			case bumpmapSpherical:
				n = i / 255.0 - 1.0;
				params->lut[i] = (int)(255.0 * sqrt(1.0 - n * n) + 0.5);
				break;

			case bumpmapSinuosidal:
				n = i / 255.0;
				params->lut[i] = (int)(255.0 * (sin((-M_PI / 2.0) + M_PI * n) + 1.0) / 2.0 + 0.5);
				break;

			case bumpmapLinear:
			default:
				params->lut[i] = i;
		}

		if (bmvals.invert)
			params->lut[i] = 255 - params->lut[i];
	}
}

static void bumpmap_row(const BYTE *src, BYTE *dst, int width,
	BYTE* bm_row1, BYTE* bm_row2, BYTE* bm_row3, int bm_width,
	BOOL tiled, BOOL row_in_bumpmap, bumpmap_params_t *params)
{
	int xofs1, xofs2, xofs3;
	double shade;
	double ndotl;
	int nx, ny;
	int x, k;
	int result;

	xofs2 = sel_x1;
	xofs1 = !xofs2 ? xofs2 : xofs2 - 1;
	xofs3 = xofs2 + 1;

	for (x = 0; x < width; x++) {
		/* Calculate surface normal from bump map */
		if (tiled || (row_in_bumpmap && x == CLAMP(x, 0, bm_width))) {
			nx = (bm_row1[xofs1] + bm_row2[xofs1] + bm_row3[xofs1] -
						bm_row1[xofs3] - bm_row2[xofs3] - bm_row3[xofs3]);
			ny = (bm_row3[xofs1] + bm_row3[xofs2] + bm_row3[xofs3] -
						bm_row1[xofs1] - bm_row1[xofs2] - bm_row1[xofs3]);
		}
		else {
			nx = ny = 0;
		}

		/* Shade */
		if ((nx == 0) && (ny == 0))
			shade = params->background;
		else {
			ndotl = nx * params->lx + ny * params->ly + params->nzlz;

			if (ndotl < 0)
				shade = params->compensation * bmvals.ambient;
			else {
				shade = ndotl / sqrt(nx * nx + ny * ny + params->nz2);
				shade = shade + MAX(0, (255 * params->compensation - shade)) * bmvals.ambient / 255;
			}
		}

		/* Paint */
		if (bmvals.compensate) {
			for (k = 3; k; k--) {
				result = (int)((*src++ * shade) / (params->compensation * 255.0));
				*dst++ = MIN(255, result);
			}
		}
		else {
			for (k = 3; k; k--)
				*dst++ = (int)(*src++ * shade / 255.0);
		}

		/* Next pixel */
		if (++xofs1 == bm_width)
			xofs1 = bm_width - 1;

		if (++xofs2 == bm_width)
			xofs2 = bm_width - 1;

		if (++xofs3 == bm_width)
			xofs3 = bm_width - 1;
	}
}

static void bumpmap_convert_row(BYTE* row, int width, BYTE* lut)
{
	BYTE *p = row;
	for (; width; width--) {
		*p++ = lut[(int)(INTENSITY(row[0], row[1], row[2]) + 0.5)];
		row += 3;
	}
}

BOOL bumpmap(mif& src, LPRECT prc, MIF_BUMPMAP* pbp)
{
	CRect rc;
	if (!src.validaterect(prc, &rc))
		return FALSE;
	src.m_bits = 24;

	sel_x1 = rc.left;
	sel_y1 = rc.top;
	sel_y2 = rc.bottom;
	sel_width = rc.Width();
	sel_height = rc.Height();

	mif dst(src);
	bmvals.azimuth    = 135.0;
	bmvals.elevation  = 45.0;
	bmvals.depth      = 3;
	bmvals.xofs       = 0;
	bmvals.yofs       = 0;
	bmvals.waterlevel = 0;
	bmvals.ambient    = 0;
	bmvals.compensate = FALSE;
	bmvals.invert     = FALSE;
	bmvals.type       = bumpmapLinear;
	bmvals.tiled      = FALSE;
	if (pbp)
		memcpy(&bmvals, pbp, sizeof(MIF_BUMPMAP));

	/* Get image information */
	int bm_width = src.m_width;
	int bm_height = src.m_height;

	/* Initialize offsets */
	int yofs2 = sel_y1;
	int yofs1 = !yofs2 ? yofs2 : yofs2 - 1;
	int yofs3 = yofs2 + 1;

	/* Bumpmap */
	bumpmap_params_t params;
	bumpmap_init_params(&params);

	/* Initialize row buffers */
	BYTE* base_ptr = (BYTE*)malloc(bm_width * 5 * 3);
	BYTE* bm_row1 = base_ptr;
	BYTE* bm_row2 = bm_row1 + bm_width * 3;
	BYTE* bm_row3 = bm_row2 + bm_width * 3;
	BYTE* src_row = bm_row3 + bm_width * 3;
	BYTE* dst_row = src_row + bm_width * 3;

	src.getrow_rgb(bm_row1, 0, yofs1, bm_width);
	src.getrow_rgb(bm_row2, 0, yofs2, bm_width);
	src.getrow_rgb(bm_row3, 0, yofs3, bm_width);

	bumpmap_convert_row(bm_row1, bm_width, params.lut);
	bumpmap_convert_row(bm_row2, bm_width, params.lut);
	bumpmap_convert_row(bm_row3, bm_width, params.lut);

	for (int y = sel_y1; y < sel_y2; y++) {
		src.getrow_rgb(src_row, sel_x1, y, sel_width);

		bumpmap_row(src_row, dst_row, sel_width,
					bm_row1, bm_row2, bm_row3, bm_width,
					bmvals.tiled, 
					y == CLAMP(y, 0, bm_height),
					&params);

		src.setrow_rgb(dst_row, sel_x1, y, sel_width);

		/* Next line */
		BYTE* bm_tmprow = bm_row1;
		bm_row1 = bm_row2;
		bm_row2 = bm_row3;
		bm_row3 = bm_tmprow;

		if (++yofs3 == bm_height)
			yofs3 = bm_height - 1;

		src.getrow_rgb(bm_row3, 0, yofs3, bm_width);
		bumpmap_convert_row(bm_row3, bm_width, params.lut);
	}
	free(base_ptr);

	return TRUE;
}

BOOL bumpmap_merge(mif& src, mif& bmp, POINT pt, MIF_BUMPMAP* pbp)
{
	if (pt.x >= src.m_width || pt.y >= src.m_height ||
		pt.x + bmp.m_width < 0 || pt.y + bmp.m_height < 0)
		return FALSE;

	src.m_bits = 24;

	bmvals.azimuth    = 135.0;
	bmvals.elevation  = 45.0;
	bmvals.depth      = 3;
	bmvals.xofs       = 0;
	bmvals.yofs       = 0;
	bmvals.waterlevel = 0;
	bmvals.ambient    = 0;
	bmvals.compensate = FALSE;
	bmvals.invert     = FALSE;
	bmvals.type       = bumpmapLinear;
	bmvals.tiled      = FALSE;
	if (pbp)
		memcpy(&bmvals, pbp, sizeof(MIF_BUMPMAP));

	/* Get image information */
	int bm_width = bmp.m_width;
	int bm_height = bmp.m_height;
	sel_x1 = 0;
	sel_y1 = 0;
	sel_x2 = bmp.m_width;
	sel_y2 = bmp.m_height;
	sel_width = bmp.m_width;
	sel_height = bmp.m_height;

	/* Bumpmap */
	bumpmap_params_t params;
	bumpmap_init_params(&params);

	/* Initialize row buffers */
	BYTE* base_ptr = (BYTE*)malloc(bm_width * 5 * 3);
	BYTE* bm_row1 = base_ptr;
	BYTE* bm_row2 = bm_row1 + bm_width * 3;
	BYTE* bm_row3 = bm_row2 + bm_width * 3;
	BYTE* src_row = bm_row3 + bm_width * 3;
	BYTE* dst_row = src_row + bm_width * 3;

	bmp.getrow_rgb(bm_row1, 0, 0, bm_width);
	bmp.getrow_rgb(bm_row2, 0, 0, bm_width);
	bmp.getrow_rgb(bm_row3, 0, 1, bm_width);

	bumpmap_convert_row(bm_row1, bm_width, params.lut);
	bumpmap_convert_row(bm_row2, bm_width, params.lut);
	bumpmap_convert_row(bm_row3, bm_width, params.lut);

	mif dst(bm_width, bm_height, 24);
	dst.blt(0, 0, bm_width, bm_height, &src, pt.x, pt.y);
	int yofs = 1;
	for (int y = 0; y < bm_height; y++)
	{
		dst.getrow_rgb(src_row, 0, y, bm_width);

		bumpmap_row(src_row, dst_row, bm_width,
					bm_row1, bm_row2, bm_row3, bm_width,
					bmvals.tiled, 
					y == CLAMP(y, 0, bm_height),
					&params);

		dst.setrow_rgb(dst_row, 0, y, bm_width);

		/* Next line */
		BYTE* bm_tmprow = bm_row1;
		bm_row1 = bm_row2;
		bm_row2 = bm_row3;
		bm_row3 = bm_tmprow;

		if (++yofs == bm_height)
			yofs = bm_height - 1;

		bmp.getrow_rgb(bm_row3, 0, yofs, bm_width);
		bumpmap_convert_row(bm_row3, bm_width, params.lut);
	}
	free(base_ptr);
	src.blt(pt.x, pt.y, bm_width, bm_height, &dst, 0, 0);

	return TRUE;
}

