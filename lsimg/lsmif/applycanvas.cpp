//////////////////////////////////////////////////////////////////////
// applycanvas.cpp (GIMP/plug-ins/common/struc.c  ver1.01)

#include "stdafx.h"
#include "lsmif.h"
#include "common.h"
#include "texture.h"

BOOL texturize(mif& src, LPRECT prc, char* txr, int twidth, int theight, int dir, int depth)
{
// dir	-- Light dir	0 - 3		default: 0
// depth		-- Texure depth		1 - 100		default: 8
// texture      -- データ は -128 ～ 127 までの char 型

	CRect rc;
	if (dir < 0 || dir > 3 ||
		depth < 1 || depth > 50 ||
		!src.validaterect(prc, &rc))
		return FALSE;
	src.m_bits = 24;

	float mult = (float)depth * 0.1F;
	int xm, ym, offs, tx, ty;
	switch (dir) {
		case 0:						// TOP_RIGHT
			xm = 1;
			ym = twidth;
			offs = 0;
			tx = twidth;
			ty = theight;
			break;

		case 1:						// TOP_LEFT
			xm = -1;
			ym = twidth;
			offs = twidth - 1;
			tx = twidth;
			ty = theight;
			break;

		case 2:						// BOTTOM_LEFT
			xm = theight;
			ym = 1;
			offs = 0;
			tx = theight;
			ty = twidth;
			break;

		case 3:						// BOTTOM_RIGHT
		default:
			xm = theight;
			ym = -1;
			offs = theight;
			tx = theight;
			ty = twidth;
			break;

	}

	int rrow = 0, rcol = 0;
	for (int y = 0; y < rc.Height(); y++) {
		RGBA* ps = src.pos(rc.left, y + rc.top);
		rcol = 0;
		for (int x = 0; x < rc.Width(); x++) {
			int addvalue = (int)(mult * txr[rcol * xm + rrow * ym + offs]);
			ps->red   = (BYTE)ADJRGB(ps->red   + addvalue);
			ps->green = (BYTE)ADJRGB(ps->green + addvalue);
			ps->blue  = (BYTE)ADJRGB(ps->blue  + addvalue);
			rcol++;
			if (rcol == tx)
				rcol = 0;
			ps++;
		}
		rrow++;
		if (rrow == ty)
			rrow = 0;
	}
	return TRUE;
}

static BOOL __texturize_by_data(mif& src, LPRECT prc, TEXTURE* t, int direction, int depth)
{
	if (!t) return FALSE;

	int size = t->width * t->height;
	BYTE* ps = t->map;
	char* pd = (char*)t->map;
	for (int i = 0; i < size; i++) {
		int n = *ps;
		*pd = (char)(n - 128);
		ps++;
		pd++;
	}
	BOOL bOK = texturize(src, prc, (char*)t->map, t->width, t->height, direction, depth);
	return bOK;
}

BOOL texturize_by_file(mif& src, LPRECT prc, LPCSTR txrfile, int direction, int depth)
{
	TEXTURE* t = txr_load(txrfile);
	BOOL bOK = __texturize_by_data(src, prc, t, direction, depth);
	txr_destroy(t);
	return bOK;
}

BOOL texturize_by_resource(mif& src, LPRECT prc, HINSTANCE hInst, UINT typeResource, UINT idResource, int direction, int depth)
{
	TEXTURE* t = txr_loadfromresource(hInst, typeResource, idResource);
	BOOL bOK = __texturize_by_data(src, prc, t, direction, depth);
	txr_destroy(t);
	return bOK;
}

BOOL texture_to_file(mif& src, LPCSTR title, LPCSTR txrfile, BOOL gray)
{
	BOOL bOK = FALSE;
	TEXTURE* t = txr_create(src.m_width, src.m_height, gray ? 1 : 3);
	if (t) {
		char* p = t->comment;
		while (*title)
			*p++ = *title++;
		*p = '\0';

		mif work(src);
		work.m_bits = 24;
		if (gray)
			work.desaturate(NULL);

		BYTE* s = t->map;
		int size = work.m_packets;
		RGBA* q = work.m_pixels;
		while (size--) 	{
			*s++ = q->red;
			if (!gray) {
				*s++ = q->green;
				*s++ = q->blue;
			}
			q++;
		}
		bOK = txr_save(t, txrfile);
		txr_destroy(t);
	}
	return bOK;
}
