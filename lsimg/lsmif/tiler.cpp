///////////////////////////////////////////////////////////////////////
// tiler.cpp

#include "stdafx.h"
#include "lsmif.h"
#include "common.h"

static void __fastcall do_tile(mif& src, int xblock, int yblock)
{
	mif dst;
	dst.create(src.m_width * xblock, src.m_height * yblock, src.m_bits);
	if (src.m_bits < 24)
		memcpy(dst.m_map, src.m_map, sizeof(RGBA) * src.m_colors);
	for (int y = 0; y < yblock; y++)
		for (int x = 0; x < xblock; x++)
			dst.blt(x * src.m_width, y * src.m_height,
					src.m_width, src.m_height, &src, 0, 0);
	src.movefrom(&dst);
}

BOOL tile_divide(mif& src, LPRECT prc, int divide)
{
	CRect rc;
	if (divide < 2 || !src.validaterect(prc, &rc) || !src.crop(&rc))
		return FALSE;
	src.m_bits = 24;

	if (src.zoom(src.m_width / divide, src.m_height / divide, zoomMitchell, FALSE))
	{
		do_tile(src, divide, divide);
		return TRUE;
	}
	return FALSE;
}

BOOL tile(mif& src, LPRECT prc, int xblock, int yblock)
{
	CRect rc;
	if (!xblock || !yblock || !src.crop(prc))
		return FALSE;
	do_tile(src, xblock, yblock);
	return TRUE;
}
