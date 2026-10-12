////////////////////////////////////////////////////////////////////////
// emboss.cpp

#include "stdafx.h"
#include "lsmif.h"
#include "common.h"

struct embossFilter {
   double Lx;
   double Ly;
   double Lz;
   double Nz;
   double Nz2;
   double NzLz;
   double bg;
} Filter;


#define DtoR(d)		((d)*(M_PI/(double)180))
#define PIXELSCALE	255.9

static void get_my_row(mif* image, RGBA* dest, int x, int y, int w)
{
	int width, height;
	int i;
	width = image->width();
	height = image->height();
	RGBA* p;

	y = max(0, min(y, height - 1));
	if (x < 0)
	{
		p = image->pos(0, y);
		*dest = *p;
		x++;
		w--;
		dest++;
		while (x < 0 && w)
		{
			*dest = *(dest - 1);
			dest++;
			x++;
			w--;
		}
	}
	if (w)
	{
		i = MIN(w, width);
		image->getrow(dest, x, y, i);
		w -= i;
		dest += i;
	}
	while (w)
	{
		*dest = *(dest - 1);
		dest++;
		x++;
		w--;
	}
}

static void get_my_rect(mif* image, RGBA* dest, int x, int y, int w, int h)
{
	RGBA* p = dest;
	for (int i = 0; i < h; i++)
	{
		get_my_row(image, p, x, y + i, w);
		p += w;
	}
}

static void emboss_init(double azimuth, double elevation, USHORT width45)
{
	//compute the light vector from the input parameters.
	// normalize the length to PIXELSCALE for fast shading calculation.
	Filter.Lx = cos(azimuth) * cos(elevation) * PIXELSCALE;
	Filter.Ly = sin(azimuth) * cos(elevation) * PIXELSCALE;
	Filter.Lz = sin(elevation) * PIXELSCALE;

	// constant z component of image surface normal - this depends on the
	// image slope we wish to associate with an angle of 45 degrees, which
	// depends on the width of the filter used to produce the source image.
	Filter.Nz = (6 * 255) / width45;
	Filter.Nz2 = Filter.Nz * Filter.Nz;
	Filter.NzLz = Filter.Nz * Filter.Lz;

	// optimization for vertical normals: L.[0 0 1]
	Filter.bg = Filter.Lz;
}

static void emboss_row(RGBA* src, RGBA* texture, RGBA* dst, int xSize)
{
	long Nx, Ny, NdotL;
	RGBA *s1, *s2, *s3;
	int x, shade;

	// mung pixels, avoiding edge pixels

	s1 = src + 1;
	s2 = s1 + xSize;
	s3 = s2 + xSize;
	dst++;

	if (texture)
		texture++;
	for (x = 1; x < xSize - 1; x++, s1++, s2++, s3++) {
		// compute the normal from the src map. the type of the
		// expression before the cast is compiler dependent. in
		// some cases the sum is unsigned, in others it is
		// signed. ergo, cast to signed.

		Nx = (int)(s1[-1].red + s2[-1].red + s3[-1].red - s1[1].red - s2[1].red - s3[1].red);
		Ny = (int)(s3[-1].red + s3[0].red + s3[1].red - s1[-1].red - s1[0].red - s1[1].red);

		// shade with distant light source
		if (Nx == 0 && Ny == 0)
			shade = (int)Filter.bg;
		else if ( (NdotL = (long)(Nx * Filter.Lx + Ny * Filter.Ly + Filter.NzLz)) < 0 )
			shade = 0;
		else
			shade = (int)((double)NdotL / sqrt(Nx * Nx + Ny * Ny + Filter.Nz2));

		// do something with the shading result
		if (texture) {
			dst->red   = (texture->red   * shade) >> 8;
			dst->green = (texture->green * shade) >> 8;
			dst->blue  = (texture->blue  * shade) >> 8;
			dst++;
			texture++;
		}
		else {
			dst->red   = shade;
			dst->green = shade;
			dst->blue  = shade;
			dst++;
		}
	}
	if (texture)
		texture++;
}

BOOL emboss(mif& src, LPRECT prc, MIF_EMBOSS* pep)
{
	CRect rc;
	if (!src.validaterect(prc, &rc))
		return FALSE;
	src.m_bits = 24;

	mif dst;
	dst.copyfrom(&src);

	MIF_EMBOSS ep;
	ep.azimuth = 30.0;
	ep.elevation = 45.0;
	ep.depth = 20;
	ep.embossp = TRUE;
	if (pep)
		memcpy(&ep, pep, sizeof(ep));

	int x1 = rc.left;
	int x2 = rc.right;
	int y1 = rc.top;
	int y2 = rc.bottom;
	int width = x2 - x1;
	int height = y2 - y1;
	int rowsize = (width + 2) * sizeof(RGBA);
	RGBA* srcbuf = (RGBA*)malloc(rowsize * 3);
	RGBA* dstbuf = (RGBA*)malloc(rowsize);

	memset(srcbuf, 0, rowsize * 3);
	memset(dstbuf, 0, rowsize);

	emboss_init(DtoR(ep.azimuth), DtoR(ep.elevation), ep.depth);

	for (int y = 0 ; y < height; y++) {
		get_my_rect(&dst, srcbuf, x1 - 1, y1 + y - 1, width + 2, 3);
		emboss_row(srcbuf, ep.embossp ? NULL : srcbuf, dstbuf, width + 2);
		src.setrow(dstbuf + 1, x1, y1 + y, width);
	}
	free(srcbuf);
	free(dstbuf);

	return TRUE;
}
