//////////////////////////////////////////////////////////////////////
// enhance.cpp

#include "stdafx.h"
#include "lsmif.h"
#include "common.h"

#define THRESHOLD  2500

BOOL enhance(mif& src, LPRECT prc, int threshold)
{
#define Enhance(weight) \
	dist = (int)s->red - r; \
	dist_sqr = squares[dist]; \
	dist = (int)s->green - g; \
	dist_sqr += squares[dist]; \
	dist = (int)s->blue - b; \
	dist_sqr += squares[dist]; \
	if (dist_sqr < threshold) \
	{ \
		total_red += (weight) * (s->red); \
		total_green += (weight) * (s->green); \
		total_blue += (weight) * (s->blue); \
		total_weight += (weight); \
	} \
	s++;

	register int i, x;
	register RGBA *p, *q, *s, *s0, *s1, *s2, *s3, *s4;
	register int* squares;
	int cx, cy, y, r, g, b;
	int dist, dist_sqr, total_blue, total_green, total_red, total_weight;

	CRect rc;
	if (!src.validaterect(prc, &rc))
		return FALSE;
	if (src.m_bits < 24)
		src.m_bits = 24;

	cx = rc.Width();
	cy = rc.Height();
	if (cx < 5 || cy < 5)
		return NULL;

	RGBA* pSrc = (RGBA*)malloc(cx * cy * sizeof(RGBA));
	src.getpacket(pSrc, rc.left, rc.top, cx, cy);
	RGBA* pDst = (RGBA*)malloc(cx * cy * sizeof(RGBA));
	squares = new int[511];
	squares += 255;
	for (i = -255; i <= 255; i++)
		squares[i] = i * i;

	// Read the first 4 rows of the image.
	RGBA* scanline = (RGBA*)malloc(cx * 5 * sizeof(RGBA));
	s = scanline;
	p = pSrc;
	for (x = 0; x < (cx << 2); x++)
		*s++ = *p++;

	// Dump first 2 scanlines of image.
	q = pDst;
	s = scanline;
	for (x = 0; x < (cx << 1); x++)
		*q++ = *s++;

	// Enhance each row.
	for (y = 2; y < (cy - 2); y++)
	{
		// Initialize sliding window pointers.
		s0 = scanline + cx * ((y - 2) % 5);
		s1 = scanline + cx * ((y - 1) % 5);
		s2 = scanline + cx * (y % 5);
		s3 = scanline + cx * ((y + 1) % 5);
		s4 = scanline + cx * ((y + 2) % 5);

		// Read another scan line.
		s = s4;
		for (x = 0; x < cx; x++)
			*s++ = *p++;

		// Transfer first 2 pixels of the scanline.
		s = s2;
		for (x = 0; x < 2; x++)
			*q++ = *s++;

		for (x = 2; x < (cx - 2); x++)
		{
			// Compute weighted average of target pixel color components.
			total_red = 0;
			total_green = 0;
			total_blue = 0;
			total_weight = 0;
			s = s2 + 2;
			r = s->red;
			g = s->green;
			b = s->blue;

			s = s0;
			Enhance(5);  Enhance(8);  Enhance(10); Enhance(8);  Enhance(5);
			s = s1;
			Enhance(8);  Enhance(20); Enhance(40); Enhance(20); Enhance(8);
			s = s2;
			Enhance(10); Enhance(40); Enhance(80); Enhance(40); Enhance(10);
			s = s3;
			Enhance(8);  Enhance(20); Enhance(40); Enhance(20); Enhance(8);
			s = s4;
			Enhance(5);  Enhance(8);  Enhance(10); Enhance(8);  Enhance(5);

			q->red = (BYTE)((total_red + (total_weight >> 1) - 1) / total_weight);
			q->green = (BYTE)((total_green + (total_weight >> 1) - 1) / total_weight);
			q->blue = (BYTE)((total_blue + (total_weight >> 1) - 1) / total_weight);
			q++;
			s0++;
			s1++;
			s2++;
			s3++;
			s4++;
		}

		// Transfer last 2 pixels of the scanline.
		s = s2;
		for (x = 0; x < 2; x++)
			*q++ = *s++;
	}

	// Dump last 2 scanlines of pixels.
	s = scanline + cx * (y % 5);
	for (x = 0; x < cx; x++)
		*q++ = *s++;
	y++;
	s = scanline + cx * (y % 5);
	for (x = 0; x < cx; x++)
		*q++ = *s++;
	free(scanline);

	squares -= 255;
	delete squares;

	free(pSrc);
	src.setpacket(pDst, rc.left, rc.top, cx, cy);
	free(pDst);
	return TRUE;
}
