//////////////////////////////////////////////////////////////////////
// curtain_ui.cpp

#include "stdafx.h"

static HDIB curtain(HDIB hsrc, LPRECT prc)
{
	mif src;
	if (!src.attachdib(hsrc))
		return NULL;

	CRect rc;
	if (src.m_bits < 24)
		rc.SetRect(0, 0, src.m_width, src.m_height);
	else if (!src.validaterect(prc, &rc))
		return NULL;

	int cx = rc.Width();
	RGBA* data = (RGBA*)malloc(sizeof(RGBA) * cx);
	register RGBA *p, *q;
	for (int y = rc.top; y < rc.bottom; y++) {
		if (y & 1) {
			p = src.m_pixels + src.m_width * y + rc.left;
			memcpy(data, p, sizeof(RGBA) * cx);
			q = data + cx - 1;
			for (int x = 0; x < cx; x++)
				*p++ = *q--;
		}
	}
	free(data);
	return src.createdib();
}

HANDLE curtain_ui(HDIB hdib, LPRECT prc, FILTER_EXEC* pfe)
{
	return curtain(hdib, prc);
}
