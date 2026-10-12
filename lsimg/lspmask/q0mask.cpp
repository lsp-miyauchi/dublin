///////////////////////////////////////////////////////////////////////
// q0mask.cpp

#include "stdafx.h"
#include "lspmask.h"
#include "common.h"
#include "dib_sup.h"

void mirror_pkt(LPRGBPK pkt, int dx, int dy)
{
	register int x, y;
	register LPRGBPK ps, pd;
	LPRGBPK pw = new RGBPK[dx * dy];
	memcpy(pw, pkt, dx * dy * sizeof(RGBPK));
	for (y = 0; y < dy; y++)
	{
		ps = pw + y * dx;
		pd = pkt + ((y + 1) * dx) - 1;
		for (x = 0; x < dx; x++)
			*pd-- = *ps++;
	}
	delete [] pw;
}

void mirror_pkt8(LPRGBPK pkt)
{
	register int x, y;
	register LPRGBPK ps, pd;
	RGBPK cbw[64];
	memcpy(cbw, pkt, sizeof(cbw));
	for (y = 0; y < 8; y++)
	{
		ps = cbw + (y << 3);
		pd = pkt + ((y + 1) << 3) - 1;
		for (x = 0; x < 8; x++)
			*pd-- = *ps++;
	}
}

void flip_pkt(LPRGBPK pkt, int dx, int dy)
{
	register int y;
	register LPRGBPK ps, pd;
	LPRGBPK pw = new RGBPK[dx * dy];
	memcpy(pw, pkt, dx * dy * sizeof(RGBPK));
	ps = pw;
	pd = pkt + dx * dy - 1;
	for (y = 0; y < dy; y++)
	{
		memcpy(pd, ps, dx * sizeof(RGBPK));
		ps += dx;
		pd -= dy;
	}
	delete [] pw;
}

void flip_pkt8(LPRGBPK pkt)
{
	register int x, y;
	register LPRGBPK ps, pd;
	RGBPK cbw[64];
	memcpy(cbw, pkt, sizeof(cbw));
	for (y = 0; y < 8; y++)
	{
		ps = cbw + (y << 3);
		pd = pkt + ((8 - y - 1) << 3);
		for (x = 0; x < 8; x++)
			*pd++ = *ps++;
	}
}

static void flip(LPRGBPK pkt, int dx, int dy)
{
	register int x, y, xBlock, yBlock;
	RGBPK cbw[64];

	xBlock = dx >> 3;
	yBlock = dy >> 3;
	for (y = 0; y < yBlock; y++)
	{
		for (x = 0; x < xBlock; x++)
		{
			loadpkt8(cbw, pkt, x << 3, y << 3, dx, dy);
			flip_pkt8(cbw);
			savepkt8(cbw, pkt, x << 3, y << 3, dx, dy);
		}
	}
}

static void mirror(LPRGBPK pkt, int dx, int dy)
{
	register int x, y, xBlock, yBlock;
	RGBPK cbw[64];

	xBlock = dx >> 3;
	yBlock = dy >> 3;
	for (y = 0; y < yBlock; y++) {
		for (x = 0; x < xBlock; x++) {
			loadpkt8(cbw, pkt, x << 3, y << 3, dx, dy);
			mirror_pkt8(cbw);
			savepkt8(cbw, pkt, x << 3, y << 3, dx, dy);
		}
	}
}

static void negative(register LPRGBPK pkt, int len)
{
	register int i;
	for (i = 0; i < len; i++) {
		pkt->red = ~pkt->red;
		pkt->green = ~pkt->green;
		pkt->blue = ~pkt->blue;
		pkt++;
	}
}

static void xor128(register LPRGBPK pkt, int len)
{
	register int i;
	for (i = 0; i < len; i++) {
		pkt->red = pkt->red ^ 0x80;
		pkt->green = pkt->green ^ 0x80;
		pkt->blue = pkt->blue ^ 0x80;
		pkt++;
	}
}

static void rgbloop(register LPRGBPK pkt, int len)
{
	BYTE cb;
	register int i;
	for (i = 0; i < len; i++) {
		cb = pkt->red;
		pkt->red = pkt->blue;
		pkt->blue = pkt->green;
		pkt->green = cb;
		pkt++;
	}
}

extern "C"
HDIB WINAPI mask_q0(HDIB hSrc, LPRECT prc, DWORD stat)
{
	HDIB hDst;
	LPBITMAPINFOHEADER pbh;
	int cx, cy, dx, dy, bits;
	CRect rc;

	if (!hSrc)
		return NULL;

	pbh = (LPBITMAPINFOHEADER)GlobalLock(hSrc);
	cx = pbh->biWidth;
	cy = pbh->biHeight;
	bits = pbh->biBitCount;

	// エンコードされているものは対象外
	if (bits != 24 || pbh->biCompression != BI_RGB)
	{
		GlobalUnlock(hSrc);
		return NULL;
	}
	GlobalUnlock(hSrc);

	if (!prc || IsRectEmpty(prc))
		rc.SetRect(0, 0, cx, cy);
	else
		rc = *prc;
	rc.right = min(rc.right, cx);
	rc.bottom = min(rc.bottom, cy);
	dx = rc.Width();
	dy = rc.Height();
	dx &= 0xFFFFFFF8;
	dy &= 0xFFFFFFF8;
	rc.right = rc.left + dx;
	rc.bottom = rc.top + dy;

	if (dx < 8 || dy < 8)
		return NULL;

	hDst = dib_copy(hSrc);
	if (!hDst)
		return NULL;

	RGBPK* pkt = dib_getpacket(hDst, rc.left, rc.top, dx, dy);

	// 横ガラス
	if (stat & Q0_GLASSHORZ)
		flip(pkt, dx, dy);

	// 縦ガラス
	if (stat & Q0_GLASSVERT)
		mirror(pkt, dx, dy);

	// ネガティブ反転
	if (stat & Q0_NEGATIVE)
		negative(pkt, dx * dy);

	// XOR128
	if (stat & Q0_XOR)
		xor128(pkt, dx * dy);

	// ミラー
	if (stat & Q0_MIRROR)
		mirror_pkt(pkt, dx, dy);

	// フリップ
	if (stat & Q0_FLIP)
		flip_pkt(pkt, dx, dy);

	// RGB入替え
	if (stat & Q0_RGB)
		rgbloop(pkt, dx * dy);

	dib_setpacket(hDst, pkt, rc.left, rc.top, dx, dy);
	dib_freepacket(pkt);
	return hDst;
}
