///////////////////////////////////////////////////////////////////////
// pyramid.cpp

#include "stdafx.h"
#include "lsmif.h"
#include "common.h"

#define F_ACC			((int)135)
#define F_HIL			((int)190)
#define F_SHD			((int)190)
#define F_THR			((int)135)

#define	fg				((BYTE)255)
#define	bg				((BYTE)0)


static void __fastcall hil(RGBA* p)
{
	int cb = ((int)p->red * F_HIL + fg * (255 - F_HIL)) / 255;
	p->red = ADJRGB(cb);
	cb = ((int)p->green * F_HIL + fg * (255 - F_HIL)) / 255;
	p->green = ADJRGB(cb);
	cb = ((int)p->blue * F_HIL + fg * (255 - F_HIL)) / 255;
	p->blue = ADJRGB(cb);
}

static void __fastcall acc(RGBA* p)
{
	int cb = ((int)p->red * F_ACC + fg * (255 - F_ACC)) / 255;
	p->red = ADJRGB(cb);
	cb = ((int)p->green * F_ACC + fg * (255 - F_ACC)) / 255;
	p->green = ADJRGB(cb);
	cb = ((int)p->blue * F_ACC + fg * (255 - F_ACC)) / 255;
	p->blue = ADJRGB(cb);
}

static void __fastcall shd(RGBA* p)
{
	int cb = ((int)p->red * F_SHD + bg * (255 - F_SHD)) / 255;
	p->red = ADJRGB(cb);
	cb = ((int)p->green * F_SHD + bg * (255 - F_SHD)) / 255;
	p->green = ADJRGB(cb);
	cb = ((int)p->blue * F_SHD + bg * (255 - F_SHD)) / 255;
	p->blue = ADJRGB(cb);
}

static void __fastcall thr(RGBA* p)
{
	int cb = ((int)p->red * F_THR + bg * (255 - F_THR)) / 255;
	p->red = ADJRGB(cb);
	cb = ((int)p->green * F_THR + bg * (255 - F_THR)) / 255;
	p->green = ADJRGB(cb);
	cb = ((int)p->blue * F_THR + bg * (255 - F_THR)) / 255;
	p->blue = ADJRGB(cb);
}

static void __fastcall raiseblock(RGBA* pData, int cx, int cy, int delta)
{
	register int x, y;
	register RGBA* p = pData;
	int height = cy - (delta << 1);

	for (y = 0; y < delta; y++)
	{
		for (x = 0; x < y; x++)
		{
			hil(p);
			p++;
		}

		for (x = 0; x < cx - (y << 1); x++)
		{
			acc(p);
			p++;
		}

		for (x = 0; x < y; x++)
		{
			shd(p);
			p++;
		}
	}
	for (y = 0; y < height; y++)
	{
		for (x = 0; x < delta; x++)
		{
			hil(p);
			p++;
		}

		p += cx - (delta << 1);

		for (x = 0; x < delta; x++)
		{
			shd(p);
			p++;
		}
	}
	for (y = 0; y < delta; y++)
	{
		for (x = 0; x < delta - y; x++)
		{
			hil(p);
			p++;
		}

		for (x = 0; x < cx - ((delta - y) << 1); x++)
		{
			thr(p);
			p++;
		}

		for (x = 0; x < delta - y; x++)
		{
			shd(p);
			p++;
		}
	}
}

BOOL pyramid(mif& src, LPRECT prc, int block)
{
	CRect rc;
	if (!src.validaterect(prc, &rc) || block < 4)
		return NULL;
	src.m_bits = 24;

	mif dst;
	dst.create(block, block, 24);

	for (int y = rc.top; y < rc.bottom; y += block) {
		for (int x = rc.left; x < rc.right; x += block) {
			dst.blt(0, 0, block, block, &src, x, y);
			raiseblock(dst.m_pixels, block, block, block >> 1);
			src.blt(x, y, block, block, &dst, 0, 0);
		}
	}
	return TRUE;
}

BOOL raise_edge(mif& src, LPRECT prc, int delta)
{
	CRect rc;
	if (!src.validaterect(prc, &rc) ||
		rc.Width() < (delta << 1) ||
		rc.Height() < (delta << 1))
		return FALSE;
	src.m_bits = 24;

	mif clip;
	clip.create(rc.Width(), rc.Height(), 24);
	clip.blt(0, 0, rc.Width(), rc.Height(), &src, rc.left, rc.top);
	raiseblock(clip.m_pixels, rc.Width(), rc.Height(), delta);
	src.blt(rc.left, rc.top, rc.Width(), rc.Height(), &clip, 0, 0);
	return TRUE;
}
