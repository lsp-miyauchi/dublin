///////////////////////////////////////////////////////////////////////
// xor128.cpp

#include "stdafx.h"
#include "lspmask.h"
#include "common.h"
#include "dib_sup.h"


///////////////////////////////////////////////////////////////////////
//

extern "C"
HDIB WINAPI mask_xor128(HDIB hSrc, LPRECT prc)
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

	// 8ビットカラー以下またはクリップ矩形が定義されているものは対象外
	if (bits <= 8)
	{
		if (prc && !IsRectEmpty(prc))
		{
			if (prc->left != 0 || prc->top != 0 || 
				prc->right != cx || prc->bottom != cy)
			{
				GlobalUnlock(hSrc);
				return FALSE;
			}
		}
	}

	// 制御をおこなう矩形の計算
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

	HDIB hDst = dib_copy(hSrc);
	if (!hDst)
		return NULL;

	pbh = (LPBITMAPINFOHEADER)GlobalLock(hDst);
	if (bits <= 8)
	{
		LPBITMAPINFO pbi = (LPBITMAPINFO)pbh;
		x = dib_numcolors((LPSTR)pbi);
		for (i = 0; i < x; i++)
		{
			pbi->bmiColors[i].rgbRed = pbi->bmiColors[i].rgbRed ^ 0x80;
			pbi->bmiColors[i].rgbGreen = pbi->bmiColors[i].rgbGreen ^ 0x80;
			pbi->bmiColors[i].rgbBlue = pbi->bmiColors[i].rgbBlue ^ 0x80;
		}
	}
	else
	{
		LPBYTE ps, pw;
		int width;

		ps = dib_findbits((LPSTR)pbh);
		width = WIDTHBYTES(cx * bits);
		for (y = rc.top; y < rc.bottom; y++)
		{
			pw = ps + width * (cy - y - 1) + rc.left * sizeof(BYTE) * 3;
			for (x = 0; x < dx; x++)
			{
				*pw++ ^= 0x80;
				*pw++ ^= 0x80;
				*pw++ ^= 0x80;
			}
		}
	}

	GlobalUnlock(hDst);
	return hDst;
}
