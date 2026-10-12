// halftone.cpp : インプリメンテーション ファイル
//

#include "stdafx.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

static __forceinline int yuvY(RGBA* ps)
	{ return (2990 * (int)ps->red + 5870 * (int)ps->green + 1140 * (int)ps->blue) / 10000; }

static int lut[18] = {
	0, 1, 2, 0, 1, 2, 0, 1, 2,
	0, 0, 0, 1, 1, 1, 2, 2, 2
};

static BYTE tone[15][15] = {
	255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 251, 221, 237, 255,
	255, 255, 255, 255, 255, 255, 255, 239, 255, 255, 255, 213, 164, 181, 238,
	255, 255, 255, 255, 255, 255, 240, 215, 240, 255, 255, 210, 145, 162, 221,
	255, 255, 255, 255, 255, 255, 251, 241, 251, 255, 251, 237, 212, 210, 244,
	255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 247, 251, 255,
	255, 247, 219, 236, 254, 255, 234, 212, 234, 255, 254, 237, 212, 234, 255,
	255, 208, 148, 173, 231, 223, 167, 122, 167, 223, 226, 166, 122, 153, 226,
	253, 186, 108, 133, 203, 200, 123,  73, 123, 198, 200, 116,  53,  89, 193,
	255, 204, 147, 169, 228, 223, 163, 123, 164, 221, 220, 148,  78, 103, 197,
	253, 230, 203, 221, 252, 249, 213, 186, 212, 248, 239, 183, 129, 147, 219,
	238, 190, 179, 217, 244, 194, 144, 121, 151, 184, 136,  69,  18,  56, 127,
	201, 108,  88, 145, 208, 147,  50,   6,  46, 111,  57,   1,   1,   3,  51,
	196,  82,  27,  80, 159, 116,   8,   3,   1,  75,  14,   1,   1,   0,   2,
	228, 148,  83, 108, 181, 136,  47,  11,  45, 112,  57,   3,   0,   0,  47,
	255, 229, 169, 198, 230, 197, 159, 138, 156, 189, 130,  53,   2,  50, 130,
};

static HDIB halftone(HDIB hdib, LPRECT prc)
{
	CRect rc;
	mif src;
	if (!src.attachdib(hdib) || !src.validaterect(prc, &rc))
		return NULL;

	for (int y = rc.top; y < rc.bottom; y++) {
		register RGBA* ps = src.pos(rc.left, y);
		for (int x = rc.left; x < rc.right; x++) {
			int i = yuvY(ps);
			int n = 8 - min(i / 30, 8);
			ps->red = ps->green = ps->blue = tone[5 * lut[9 + n] + y % 5][5 * lut[n] + x % 5];
			ps++;
		}
	}
	return src.createdib();
}

HANDLE halftone_ui(HDIB hdib, LPRECT prc, FILTER_EXEC* pfe)
{
	return halftone(hdib, prc);
}
