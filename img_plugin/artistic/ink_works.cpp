//////////////////////////////////////////////////////////////////
// filter.cpp

#include "stdafx.h"

//////////////////////////////////////////////////////////////////
// ink_illust

HANDLE ink_illust(HANDLE hdib, LPRECT prc, FILTER_EXEC* pfe)
{
	if (!hdib) return NULL;

	CRect rc;
	mif image;
	if (!image.attachdib(hdib) || !image.validaterect(prc, &rc))
		return NULL;
	image.m_bits = 24;

	CWaitCursor wc;

	// 減色
	MIF_COLORCUT cp;
	cp.color_space = clrspcYUV;
	cp.xstep = 32;
	cp.ystep = 16;
	cp.zstep = 16;
	image.colorcut(&rc, &cp);

	// インク処理
	mif work(rc.Width(), rc.Height(), 24);
	work.blt(0, 0, rc.Width(), rc.Height(), &image, rc.left, rc.top);
	work.filter_3x3(NULL, filterEdgeLaplacian);
	work.negative(NULL);
	work.desaturate(NULL);
	register RGBA* p = work.m_pixels;
	for (int i = 0; i < work.m_packets; i++) {
		p->alpha = 255;
		p++;
	}
	image.addlayer(&work, rc.left, rc.top, blendBurn, 255);

	return image.createdib();
}


//////////////////////////////////////////////////////////////////////
//

typedef struct tagARITHINFO {
	int method;
	int opaque;
	BOOL shadow;
	int shadow_gauss;
	int shadow_distance;
	COLORREF shadow_color;
} ARITHINFO;

#define CBLACK		RGB(0,0,0)
#define CWHITE		RGB(255,255,255)

static BOOL __fastcall paste_image(mif& src, mif& dst, int x, int y, ARITHINFO* pai)
{
	if (!pai || pai->opaque == 0) return FALSE;

	int opaque = pai->opaque * 255 / 100;

	src.m_bits = 24;
	dst.m_bits = 24;

	register RGBA* p = dst.m_pixels;
	for (int i = 0; i < dst.m_packets; i++) {
		p->alpha = 255;
		p++;
	}

	if (pai->shadow) {
		// ぼかしの適用範囲を保証する意味で、直径ピクセル分を確保する
		int delta;
		if (pai->shadow_gauss > 0)
			delta = (int)floor((double)(pai->shadow_gauss + 10) / 5.0);
		else
			delta = 0;

		mif shadow(dst.m_width + delta * 2, dst.m_height + delta * 2, 24);
		shadow.fillsolidrect(0, 0, shadow.m_width, shadow.m_height, CBLACK, 255);
		shadow.fillsolidrect(delta, delta, dst.m_width, dst.m_height, CWHITE, 255);
		if (pai->shadow_gauss > 0)
			shadow.blur_gauss_rle(NULL, (double)pai->shadow_gauss / 10.0);

		CRect rc(0, 0, dst.m_width, dst.m_height);
		rc.OffsetRect(delta - pai->shadow_distance, delta - pai->shadow_distance);
		shadow.validaterect(&rc, &rc);
		shadow.fillsolidrect(&rc, CBLACK, 255);
		shadow.settoalpha();
		shadow.fillsolidrect(NULL, pai->shadow_color, 255);

		src.addlayer(&shadow,
					x - delta + pai->shadow_distance,
					y - delta + pai->shadow_distance,
					blendNormal, opaque);
	}

	src.addlayer(&dst, x, y, pai->method, opaque);
	return TRUE;
}


//////////////////////////////////////////////////////////////////////
//

HANDLE ink_trace(HANDLE hdib, LPRECT prc, FILTER_EXEC* pfe)
{
	if (!hdib) return NULL;

	CRect rc;
	mif image;
	if (!image.attachdib(hdib) || !image.validaterect(prc, &rc))
		return NULL;
	image.m_bits = 24;

	mif work(image);
	work.crop(&rc);
	work.filter_3x3(NULL, filterEdgeLaplacian);
	work.negative(NULL);
	work.desaturate(NULL);

	ARITHINFO ai;
	ai.method = blendBurn;
	ai.shadow = FALSE;
	ai.opaque = 100;
	paste_image(image, work, rc.left, rc.top, &ai);

	return image.createdib();
}

//////////////////////////////////////////////////////////////////////
//

#define MATCHRGB(a,b)	((a).red==(b).red&&(a).green==(b).green&&(a).blue==(b).blue)

HANDLE ink_wax(HANDLE hdib, LPRECT prc, FILTER_EXEC* pfe)
{
	if (!hdib) return NULL;

	CRect rc;
	mif image;
	if (!image.attachdib(hdib) || !image.validaterect(prc, &rc))
		return NULL;
	image.m_bits = 24;

	mif worker(image);
	worker.crop(&rc);
	worker.edge(NULL, 10.0);
	worker.negative(NULL);

	RGBA* ps = worker.m_pixels;
	int cnt = worker.m_packets;
	RGBA black = { 0, 0, 0, 0 };
	while (cnt-- > 0) {
		if (MATCHRGB(*ps, black))
			ps->red = ps->green = ps->blue = 255;
		ps++;
	}

	ARITHINFO ai;
	ai.method = blendBurn;
	ai.shadow = FALSE;
	ai.opaque = 100;
	paste_image(image, worker, rc.left, rc.top, &ai);

	return image.createdib();
}

//////////////////////////////////////////////////////////////////////
//

HANDLE ink_charcoal(HANDLE hdib, LPRECT prc, FILTER_EXEC* pfe)
{
	if (!hdib) return NULL;

	CRect rc;
	mif image;
	if (!image.attachdib(hdib) || !image.validaterect(prc, &rc))
		return NULL;
	image.m_bits = 24;

	image.edge(&rc, 10.0);
	image.desaturate(&rc);
	image.negative(&rc);
	image.spread(&rc, 1);

	return image.createdib();
}
