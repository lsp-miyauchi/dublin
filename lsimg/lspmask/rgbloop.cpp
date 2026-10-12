///////////////////////////////////////////////////////////////////////
// rgbloop.cpp

#include "stdafx.h"
#include "lspmask.h"
#include "common.h"
#include "dib_sup.h"

///////////////////////////////////////////////////////////////////////
//

extern "C"
HDIB WINAPI mask_rgbloop(HDIB hSrc, LPRECT prc)
{
	register int i, x, y;

	if (!hSrc)
		return NULL;

	LPBITMAPINFOHEADER pbh = (LPBITMAPINFOHEADER)GlobalLock(hSrc);
	int cx = pbh->biWidth;
	int cy = pbh->biHeight;
	int bits = pbh->biBitCount;

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

	pbh = (LPBITMAPINFOHEADER)GlobalLock(hDst);
	BYTE cb;
	if (bits <= 8)
	{
		LPBITMAPINFO pbi = (LPBITMAPINFO)pbh;
		x = dib_numcolors((LPSTR)pbi);
		for (i = 0; i < x; i++)
		{
			cb = pbi->bmiColors[i].rgbRed;
			pbi->bmiColors[i].rgbRed = pbi->bmiColors[i].rgbBlue;
			pbi->bmiColors[i].rgbBlue = pbi->bmiColors[i].rgbGreen;
			pbi->bmiColors[i].rgbGreen = cb;
		}
	}
	else
	{
		// 制御をおこなう矩形の計算
		CRect rc;
		if (!prc || IsRectEmpty(prc))
			rc.SetRect(0, 0, cx, cy);
		else
		{
			rc.CopyRect(prc);
			rc.right = min(rc.right, cx);
			rc.bottom = min(rc.bottom, cy);
		}
		int dx = rc.Width();
		int dy = rc.Height();

		LPBYTE pw, ps = dib_findbits(pbh);
		int width = WIDTHBYTES(cx * bits);
		for (y = rc.top; y < rc.bottom; y++)
		{
			pw = ps + width * (cy - y - 1) + rc.left * sizeof(BYTE) * 3;
			for (x = 0; x < dx; x++)
			{
				cb = pw[0];				// tmp <- b
				pw[0] = pw[1];			// b <- g
				pw[1] = pw[2];			// g <- r
				pw[2] = cb;				// r <- tmp
				pw += sizeof(BYTE) * 3;
			}
		}
	}
	GlobalUnlock(hDst);
	return hDst;
}
