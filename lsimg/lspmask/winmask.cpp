///////////////////////////////////////////////////////////////////////
// winmask.cpp

#include "stdafx.h"
#include "lspmask.h"
#include "common.h"
#include "dib_sup.h"

int maskpatt[] = {
	12, 8, 6, 15, 9, 13, 2, 11, 1, 4, 14, 7, 0, 5, 10, 3
};

void winmask(LPBYTE pData, int cy)
{
	register int x, y;
	register LPBYTE ps, pd;
	LPBYTE pw = (LPBYTE)malloc(cy << 4);
	memcpy(pw, pData, cy << 4);
	for (y = 0; y < cy; y++)
	{
		ps = pw + (y << 4);
		pd = pData + (y << 4);
		for (x = 0; x < 16; x++)
			*pd++ = ps[maskpatt[x]];
	}
	free(pw);
}

extern "C"
HDIB WINAPI mask_win(HDIB hSrc, LPRECT prc)
{
	if (!hSrc)
		return NULL;

	LPBITMAPINFOHEADER pbh = (LPBITMAPINFOHEADER)GlobalLock(hSrc);
	int cx = pbh->biWidth;
	int cy = pbh->biHeight;
	int bits = pbh->biBitCount;
	int plane = (bits <= 8) ? 1 : 3;

	// エンコードされているものは対象外
	if (pbh->biCompression != BI_RGB)
	{
		GlobalUnlock(hSrc);
		return NULL;
	}
	GlobalUnlock(hSrc);

	RECT rc;
	if (!prc || IsRectEmpty(prc))
		SetRect(&rc, 0, 0, cx, cy);
	else
	{
		CopyRect(&rc, prc);
		rc.right = min(rc.right, cx);
		rc.bottom = min(rc.bottom, cy);
	}
	int dx = ((rc.right - rc.left) >> 4) << 4;
	int dy = rc.bottom - rc.top;
	if (dx < 16)
		return NULL;
	rc.right = rc.left + dx;

	HDIB hDst = dib_copy(hSrc);
	if (!hDst)
		return NULL;

	LPBYTE pPlane = (LPBYTE)malloc(cx * cy);
	LPBYTE pVert = (LPBYTE)malloc(16 * dy);

	int i, j, blk;
	for (i = 0; i < plane; i++)
	{
		dib_getplane(hDst, i, pPlane);
		blk = dx >> 4;
		for (j = 0; j < blk; j++)
		{
			dib_planeblt(pVert, 0, 0, 16, dy,
						pPlane, rc.left + (j << 4), rc.top,
						cx, cy, 16, dy);
			winmask(pVert, dy);
			dib_planeblt(pPlane, rc.left + (j << 4), rc.top, cx, cy,
						pVert, 0, 0, 16, dy, 16, dy);
		}
		dib_setplane(hDst, i, pPlane);
	}
	free(pPlane);
	free(pVert);

	return hDst;
}

void winmaskhorz(LPBYTE pData, int cx)
{
	register int y;
	register LPBYTE ps, pd;
	LPBYTE pw = (LPBYTE)malloc(cx << 4);
	memcpy(pw, pData, cx << 4);
	for (y = 0; y < 16; y++)
	{
		ps = pw + maskpatt[y] * cx;
		pd = pData + y * cx;
		memcpy(pd, ps, cx);
	}
	free(pw);
}

extern "C"
HDIB WINAPI mask_win_horz(HDIB hSrc, LPRECT prc)
{
	if (!hSrc)
		return NULL;

	LPBITMAPINFOHEADER pbh = (LPBITMAPINFOHEADER)GlobalLock(hSrc);
	int cx = pbh->biWidth;
	int cy = pbh->biHeight;
	int bits = pbh->biBitCount;
	int plane = (bits <= 8) ? 1 : 3;

	// エンコードされているものは対象外
	if (pbh->biCompression != BI_RGB)
	{
		GlobalUnlock(hSrc);
		return NULL;
	}
	GlobalUnlock(hSrc);

	CRect rc;
	if (!prc || IsRectEmpty(prc))
		SetRect(&rc, 0, 0, cx, cy);
	else
	{
		CopyRect(&rc, prc);
		rc.right = min(rc.right, cx);
		rc.bottom = min(rc.bottom, cy);
	}
	int dx = rc.Width();
	int dy = (rc.Height() >> 4) << 4;
	if (dy < 16)
		return NULL;
	rc.bottom = rc.top + dy;

	HDIB hDst = dib_copy(hSrc);
	if (!hDst)
		return NULL;

	LPBYTE pPlane = (LPBYTE)malloc(cx * cy);
	LPBYTE pHorz = (LPBYTE)malloc(dx * 16);

	int i, j, blk = dy >> 4;
	for (i = 0; i < plane; i++)
	{
		dib_getplane(hDst, i, pPlane);
		for (j = 0; j < blk; j++)
		{
			dib_planeblt(pHorz, 0, 0, dx, 16,
						pPlane, rc.left, rc.top + (j << 4),
						cx, cy, dx, 16);
			winmaskhorz(pHorz, dx);
			dib_planeblt(pPlane, rc.left, rc.top + (j << 4), cx, cy,
						pHorz, 0, 0, dx, 16, dx, 16);
		}
		dib_setplane(hDst, i, pPlane);
	}
	free(pPlane);
	free(pHorz);

	return hDst;
}
