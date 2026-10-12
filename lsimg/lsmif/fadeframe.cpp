////////////////////////////////////////////////////////////////////////
// fadeframe.cpp

#include "stdafx.h"
#include "lsmif.h"
#include "common.h"

#define MINVAL(a,b)		((a)<(b)?(a):(b))

BOOL fadeinrect(mif& image, LPRECT prc, int delta, COLORREF cr)
{
	CRect rc;
	if (delta < 4 ||
		((delta * 2) > image.m_width) ||
		((delta * 2) > image.m_height) ||
		!image.validaterect(prc, &rc))
		return FALSE;
	image.m_bits = 24;

	int width = rc.Width();
	int height = rc.Height();

	mif fade;
	fade.create(width, height, 24);

	CRect rcf(0, 0, width, height);
	rcf.InflateRect(-delta, -delta);
	fade.solidfill(&rcf, RGB(255, 255, 255), 255);

	fade.blur_gauss_rle(NULL, MINVAL(4.0, (double)delta / 2.0));
	fade.negative(NULL);
	fade.settoalpha();

	fade.solidfill(0, 0, width, height, cr, 255);
	image.addlayer(&fade, rc.left, rc.top, blendNormal, 255);
	return TRUE;
}

BOOL fadepaste(mif& src, mif& dst, int delta, int xpos, int ypos)
{
	if (delta < 4 ||
		((delta * 2) > dst.m_width) ||
		((delta * 2) > dst.m_height) ||
		(xpos + dst.m_width < 0) ||
		(xpos > src.m_width) ||
		(ypos + dst.m_height < 0) ||
		(ypos > src.m_height))
		return FALSE;
	src.m_bits = 24;
	dst.m_bits = 24;

	mif fade;
	fade.create(dst.m_width, dst.m_height, 24);

	CRect rcf(0, 0, dst.m_width, dst.m_height);
	rcf.InflateRect(-delta, -delta);
	fade.solidfill(&rcf, RGB(255, 255, 255), 255);
	fade.blur_gauss_rle(NULL, MINVAL(4.0, (double)delta / 2.0));
	fade.settoalpha();

	BYTE* p = (BYTE*)malloc(dst.m_packets);
	fade.getplane(p, 0, 0, dst.m_width, dst.m_height, planeAlpha);
	dst.setplane(p, 0, 0, dst.m_width, dst.m_height, planeAlpha);
	free(p);

	src.addlayer(&dst, xpos, ypos, blendNormal, 255);
	return TRUE;
}
