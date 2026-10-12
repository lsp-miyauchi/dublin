///////////////////////////////////////////////////////////////////////
// glass.cpp

#include "stdafx.h"
#include "lspmask.h"
#include "common.h"
#include "dib_sup.h"

extern "C"
HDIB WINAPI mask_glass(HDIB hSrc, LPRECT prc, int nBlock, DWORD dwFlags)
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

	HDIB hDst = dib_copy(hSrc);
	if (!hDst)
		return NULL;

	RECT rc;
	if (!prc || IsRectEmpty(prc))
		SetRect(&rc, 0, 0, cx, cy);
	else
	{
		CopyRect(&rc, prc);
		rc.right = min(rc.right, cx);
		rc.bottom = min(rc.bottom, cy);
	}
	int dx = rc.right - rc.left;
	int dy = rc.bottom - rc.top;

	if (dwFlags & GLASS_HORZ)
	{
		dy = (dy / nBlock) * nBlock;
		rc.bottom = rc.top + dy;
	}
	if (dwFlags & GLASS_VERT)
	{
		dx = (dx / nBlock) * nBlock;
		rc.right = rc.left + dx;
	}

	LPBYTE pPlane = (LPBYTE)malloc(cx * cy);
	LPBYTE pHorz = (LPBYTE)malloc(dx * nBlock);
	LPBYTE pVert = (LPBYTE)malloc(nBlock * dy);

	int i, j, blk;
	for (i = 0; i < plane; i++)
	{
		dib_getplane(hDst, i, pPlane);
		if (dwFlags & GLASS_HORZ)
		{
			blk = dy / nBlock;
			for (j = 0; j < blk; j++)
			{
				dib_planeblt(pHorz, 0, 0, dx, nBlock,
							pPlane, rc.left, rc.top + j * nBlock,
							cx, cy, dx, nBlock);
				form_flip(pHorz, dx, nBlock);
				dib_planeblt(pPlane, rc.left, rc.top + j * nBlock, cx, cy,
							pHorz, 0, 0, dx, nBlock, dx, nBlock);
			}
		}
		if (dwFlags & GLASS_VERT)
		{
			blk = dx / nBlock;
			for (j = 0; j < blk; j++)
			{
				dib_planeblt(pVert, 0, 0, nBlock, dy,
							pPlane, rc.left + j * nBlock, rc.top,
							cx, cy, nBlock, dy);
				form_mirror(pVert, nBlock, dy);
				dib_planeblt(pPlane, rc.left + j * nBlock, rc.top, cx, cy,
							pVert, 0, 0, nBlock, dy, nBlock, dy);
			}
		}
		dib_setplane(hDst, i, pPlane);
	}
	free(pPlane);
	free(pHorz);
	free(pVert);

	return hDst;
}
