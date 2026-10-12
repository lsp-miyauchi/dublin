///////////////////////////////////////////////////////////////////////
// flmask.cpp

#include "stdafx.h"
#include "lspmask.h"
#include "common.h"
#include "dib_sup.h"

typedef struct tagFLS{
	int n;
	int id;
} FLS, *LPFLS;

static int comp_fls(const void *s, const void *d)
{
	FLS* fs1 = (FLS*)s;
	FLS* fs2 = (FLS*)d;
	return fs1->id - fs2->id;
}

LPVOID fltable(int w, int h)
{
	register int i, ct;
	int nc, d, now;

	register FLS* fs = (FLS*)malloc(h * w * sizeof(FLS));
	if (!fs)
		return NULL;

	ct = 0;
	d = 0;
	nc = h * w;
    now = (h - 1) * w;
    while(1) {
	/* 右行き */
	if(w - d > 0)
	    for(i = 0; i < w - d; i++) {
		fs[ct++].n = now + i;
		if(ct == nc)
		    goto jp;
	    }
	/* 上行き */
	now = fs[ct - 1].n - w;
	d++;
	if(h - d > 0)
	    for(i = 0; i < h - d; i++) {
		fs[ct++].n = now - i * w;
		if(ct == nc)
		    goto jp;
	    }
	/* 左行き */
	now = fs[ct - 1].n - 1;
	if(w - d > 0)
	    for(i = 0; i < w - d; i++) {
		fs[ct++].n = now - i;
		if(ct == nc)
		    goto jp;
	    }
	/* 下行き */
	now = fs[ct - 1].n + w;
	d++;
	if(h - d > 0)
	    for(i = 0; i < h - d; i++) {
		fs[ct++].n = now + i * w;
		if(ct == nc)
		    goto jp;
	    }
	now = fs[ct - 1].n + 1;
    }
 jp:
    for(i = 0; i < nc; i++)
    	fs[i].id = fs[nc - i - 1].n;
    qsort(fs, nc, sizeof(FLS), comp_fls);

	int* pi = (int*)malloc(h * w * sizeof(int));
	if (pi)
	{
		for (i = 0; i < nc; i++)
			pi[i] = fs[i].n;
	}

	free(fs);
	return pi;
}

void flmask(LPBYTE pData, int* pArray, int dx, int dy, BOOL bInvert)
{
	register int x, y, sx, sy, n, xBlock, yBlock;
	register LPBYTE pWrk;
	BYTE cbw[64];

	xBlock = dx >> 3;
	yBlock = dy >> 3;
	pWrk = (LPBYTE)malloc(dx * dy);
	memcpy(pWrk, pData, dx * dy);
	for (y = 0; y < yBlock; y++)
	{
		for (x = 0; x < xBlock; x++)
		{
			n = pArray[y * xBlock + x];
			sx = (n % xBlock) << 3;
			sy = (n / xBlock) << 3;
			loadblock8(cbw, pWrk, x << 3, y << 3, dx, dy);
			saveblock8(cbw, pData, sx, sy, dx, dy);
		}
	}
	free(pWrk);

	if (bInvert)
	{
		for (x = 0; x < dx * dy; x++)
			pData[x] = ~pData[x];
	}
}

void flmask_pkt(LPRGBPK pkt, int* pArray, int dx, int dy)
{
	register int x, y, sx, sy, n, xBlock, yBlock;
	register LPRGBPK pw;
	RGBPK cbw[64];

	xBlock = dx >> 3;
	yBlock = dy >> 3;
	pw = new RGBPK[dx * dy];
	memcpy(pw, pkt, dx * dy * sizeof(RGBPK));
	for (y = 0; y < yBlock; y++)
	{
		for (x = 0; x < xBlock; x++)
		{
			n = pArray[y * xBlock + x];
			sx = n % xBlock;
			sy = n / xBlock;
			if (sx != x || sy != y)
			{
				loadpkt8(cbw, pw, x << 3, y << 3, dx, dy);
				for (n = 0; n < 64; n++)
				{
					cbw[n].red = ~cbw[n].red;
					cbw[n].green = ~cbw[n].green;
					cbw[n].blue = ~cbw[n].blue;
				}
				savepkt8(cbw, pkt, sx << 3, sy << 3, dx, dy);
			}
		}
	}
	delete [] pw;
}

extern "C"
HDIB WINAPI mask_fl(HDIB hSrc, LPRECT prc, BOOL bInvert)
{
	HDIB hDst;
	LPBITMAPINFOHEADER pbh;
	int cx, cy, dx, dy, i, bits, plane;
	RECT rc;
	int* pArray;

	if (!hSrc)
		return NULL;

	pbh = (LPBITMAPINFOHEADER)GlobalLock(hSrc);
	cx = pbh->biWidth;
	cy = pbh->biHeight;
	bits = pbh->biBitCount;
	plane = (bits <= 8) ? 1 : 3;

	// エンコードされているものは対象外
	if (pbh->biCompression != BI_RGB)
	{
		GlobalUnlock(hSrc);
		return NULL;
	}
	GlobalUnlock(hSrc);

	if (!prc || IsRectEmpty(prc))
		SetRect(&rc, 0, 0, cx, cy);
	else
		CopyRect(&rc, prc);
	rc.right = min(rc.right, cx);
	rc.bottom = min(rc.bottom, cy);
	dx = rc.right - rc.left;
	dy = rc.bottom - rc.top;
	dx &= 0xFFFFFFF8;
	dy &= 0xFFFFFFF8;
	rc.right = rc.left + dx;
	rc.bottom = rc.top + dy;

	if (dx < 8 || dy < 8)
		return NULL;

	hDst = dib_copy(hSrc);
	if (!hDst)
		return NULL;

	pArray = (int*)fltable(dx >> 3, dy >> 3);
	if (!pArray)
	{
		GlobalFree(hDst);
		return NULL;
	}

	// 24ビットカラー以外はネガ反転しない
	if (bits != 24)
	{
		LPBYTE pPlane = (LPBYTE)malloc(cx * cy);
		LPBYTE pClip = (LPBYTE)malloc(dx * dy);
		for (i = 0; i < plane; i++)
		{
			dib_getplane(hDst, i, pPlane);
			dib_planeblt(pClip, 0, 0, dx, dy,
						pPlane, rc.left, rc.top, cx, cy, dx, dy);
			flmask(pClip, pArray, dx, dy, bInvert);
			dib_planeblt(pPlane, rc.left, rc.top, cx, cy,
						pClip, 0, 0, dx, dy, dx, dy);
			dib_setplane(hDst, i, pPlane);
		}
		free(pPlane);
		free(pClip);
	}
	else
	{
		RGBPK* pkt = dib_getpacket(hDst, rc.left, rc.top, dx, dy);
		flmask_pkt(pkt, pArray, dx, dy);
		dib_setpacket(hDst, pkt, rc.left, rc.top, dx, dy);
		dib_freepacket(pkt);
	}
	free(pArray);
	return hDst;
}
