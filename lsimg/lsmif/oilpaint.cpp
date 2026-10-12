///////////////////////////////////////////////////////////////////////
// oilpaint.cpp

#include "stdafx.h"
#include "lsmif.h"
#include "common.h"


///////////////////////////////////////////////////////////////////////
//

/* @@
BOOL oilpaint(mif& src, LPRECT prc, int rad)
{
	int count, k;
	register int i, j, x, y;
	register RGBA *p, *q, *s;

	CRect rc;
	if (!src.validaterect(prc, &rc))
		return NULL;
	src.m_bits = 24;

	int dx = rc.Width();
	int dy = rc.Height();

	// Paint each row of the image.

	RGBA* data = (RGBA*)malloc(dx * dy * sizeof(RGBA));
	src.getpacket(data, rc.left, rc.top, dx, dy);

	int histogram[256];
	p = data;
	for (y = 0; y < dy; y++)
	{
		q = src.m_pixels + (rc.top + y) * src.m_width + rc.left;
		for (x = 0; x < dx; x++)
		{
			if (y < rad || y >= (dy - rad) || x < rad || x >= (dx - rad))
			{
				*q++ = *p++;
				continue;
			}

			// Determine most frequent color.
			count = 0;
			memset(histogram, 0, sizeof(int) * 256);
			for (i = 0; i < rad; i++)
			{
				s = p - (rad - i) * dx - 1 - i;
				for (j = 0; j < (i + i + 1); j++)
				{
					k = INTENSITY(s->red, s->green, s->blue);
					histogram[k]++;
					if (histogram[k] > count)
					{
						*q = *s;
						count = histogram[k];
					}
					s++;
				}
				s = p + (rad - i) * dx - 1 - i;
				for (j = 0; j < (i + i + 1); j++)
				{
					k = INTENSITY(s->red, s->green, s->blue);
					histogram[k]++;
					if (histogram[k] > count)
					{
						*q = *s;
						count = histogram[k];
					}
					s++;
				}
			}
			s = p - rad;
			for (j = 0; j < (rad + rad + 1); j++)
			{
				k = INTENSITY(s->red, s->green, s->blue);
				histogram[k]++;
				if (histogram[k] > count)
				{
					*q = *s;
					count = histogram[k];
				}
				s++;
			}
			q++;
			p++;
		}
	}
	free(data);
	return TRUE;
}
@@ */

BOOL oilpaint(mif& src, LPRECT prc, int rad)
{
	CRect rc;
	if (!src.validaterect(prc, &rc))
		return NULL;
	src.m_bits = 24;

	mif dst(src);
	int* histogram_ptr = new int[256 * rc.Height()];

	int y1 = max(rad, rc.top);
	int y2 = min(src.m_height - rad, rc.bottom);
	parallel_for (y1, y2, [&](int y) {
		int* histogram = histogram_ptr + (y - rc.top) * 256;
		RGBA* p = dst.pos(rc.left, y);
		RGBA* q = src.pos(rc.left, y);
		for (int x = rc.left; x < rc.right; x++) {
			if (x < rad) {
				p++;
				q++;
				continue;
			}

			if (x >= (src.m_width - rad))
				break;

			// Determine most frequent color.
			int i, j, k, count = 0;
			RGBA* s;
			memset(histogram, 0, sizeof(int) * 256);
			for (i = 0; i < rad; i++) {
				s = p - (rad - i) * src.m_width - 1 - i;
				for (j = 0; j < (i + i + 1); j++) {
					k = INTENSITY(s->red, s->green, s->blue);
					histogram[k]++;
					if (histogram[k] > count) {
						*q = *s;
						count = histogram[k];
					}
					s++;
				}
				s = p + (rad - i) * src.m_width - 1 - i;
				for (j = 0; j < (i + i + 1); j++) {
					k = INTENSITY(s->red, s->green, s->blue);
					histogram[k]++;
					if (histogram[k] > count) {
						*q = *s;
						count = histogram[k];
					}
					s++;
				}
			}
			s = p - rad;
			for (j = 0; j < (rad + rad + 1); j++) {
				k = INTENSITY(s->red, s->green, s->blue);
				histogram[k]++;
				if (histogram[k] > count) {
					*q = *s;
					count = histogram[k];
				}
				s++;
			}
			q++;
			p++;
		}
	});
	delete [] histogram_ptr;
	return TRUE;
}
