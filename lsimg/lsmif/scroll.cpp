//////////////////////////////////////////////////////////////////////
// scroll.cpp

#include "stdafx.h"
#include "lsmif.h"
#include "common.h"

BOOL scroll(mif& src, int sx, int sy, BOOL bWrap)
{
	sx %= src.m_width;
	sy %= src.m_height;
	if (!sx && !sy)
		return TRUE;

	if (!bWrap && src.m_bits < 24)
		src.m_bits = 24;

	int width = src.m_width;
	int height = src.m_height;

	mif dst(src);
	if (sx < 0)
	{
		sx = -sx;
		src.blt(0, 0, width - sx, height, &dst, sx, 0);
		if (!bWrap)
			src.solidfill(width - sx, 0, sx, height, RGB(255,255,255), 255);
		else
			src.blt(width - sx, 0, sx, height, &dst, 0, 0);
	}
	else if (sx > 0)
	{
		src.blt(sx, 0, width - sx, height, &dst, 0, 0);
		if (!bWrap)
			src.solidfill(0, 0, sx, height, RGB(255, 255, 255), 255);
		else
			src.blt(0, 0, sx, height, &dst, width - sx, 0);
	}

	if (sy < 0)
	{
		dst.blt(0, 0, width, height, &src, 0, 0);

		sy = -sy;
		src.blt(0, 0, width, height - sy, &dst, 0, sy);
		if (!bWrap)
			src.solidfill(0, height - sy, width, sy, RGB(255, 255, 255), 255);
		else
			src.blt(0, height - sy, width, sy, &dst, 0, 0);
	}
	else if (sy > 0)
	{
		dst.blt(0, 0, width, height, &src, 0, 0);

		src.blt(0, sy, width, height - sy, &dst, 0, 0);
		if (!bWrap)
			src.solidfill(0, 0, width, sy, RGB(255, 255, 255), 255);
		else
			src.blt(0, 0, width, sy, &dst, 0, height - sy);
	}
	return TRUE;
}
