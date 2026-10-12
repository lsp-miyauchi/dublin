///////////////////////////////////////////////////////////////////////
// tone.cpp

#include "stdafx.h"
#include "lsimg.h"
#include "common.h"

extern "C"
BOOL WINAPI imgLagFillEx(int* tbl, LPTONE pt)
{
	register int i, j, k;
	for (i = 0; i < MAXRGB + 1; i++)
		tbl[i] = i;

	if (!pt->count || !memcmp(pt->ref, pt->val, pt->count * sizeof(int)))
		return TRUE;

	int count = pt->count;
	if (pt->count > MAXTONEPOINT)
		count = MAXTONEPOINT;
	int refs = count + 1;

	double s, xi[10], fi[10][10];
	xi[0] = 0;
	fi[0][0] = 0;
	xi[refs] = MAXRGB;
	fi[0][refs] = MAXRGB;
	for (i = 0; i < count; i++) {
		xi[i+1] = (double)pt->ref[i];
		fi[0][i+1] = (double)pt->val[i];
	}

	// 差分商の計算
	for (i = 0; i < refs; i++) {
		for (j = i; j < refs; j++)
			fi[i+1][j-i] = (fi[i][j+1-i] - fi[i][j-i]) / (xi[j+1] - xi[j-i]);
	}

	// kにおけるラグランジュ補完多項式の計算
	for (k = 0; k < MAXRGB + 1; k++) {
		s = fi[refs][0];
		for (i = 0; i < refs; i++) {
			j = refs - 1 - i;
			s = s * (k - xi[j]) + fi[j][0];
		}
		j = (int)(s + 0.5);
		tbl[k] = ADJRGB(j);
	}
	return TRUE;
}

extern "C"
BOOL WINAPI imgLagFill(int* tbl, int ref, int val)
{
	TONE t;
	t.count = 1;
	t.ref[0] = ref;
	t.val[0] = val;
	return imgLagFillEx(tbl, &t);
}

extern "C"
HDIB WINAPI imgToneCurveEx(HDIB hSrc, LPTONEINFO pti)
{
	register int x, y, width;
	int tbl[4][256];

	if (!hSrc)
		return NULL;

	imgLagFillEx(tbl[0], &pti->rgb);
	imgLagFillEx(tbl[1], &pti->red);
	imgLagFillEx(tbl[2], &pti->blue);
	imgLagFillEx(tbl[3], &pti->green);

	HDIB hDst = dib_copy(hSrc);
	if (!hDst)
		return NULL;

	LPBYTE pBase = (LPBYTE)GlobalLock(hDst);
	LPBITMAPINFOHEADER pbh = (LPBITMAPINFOHEADER)pBase;

	if (pbh->biBitCount > 8) {
		register LPBYTE pb;
		width = WIDTHBYTES(pbh->biWidth * pbh->biBitCount);

		pBase = dib_findbits(pBase);
		for (y = 0; y < pbh->biHeight; y++) {
			pb = pBase + width * (pbh->biHeight - y - 1);
			for (x = 0; x < pbh->biWidth; x++) {
				pb[2] = tbl[0][tbl[1][pb[2]]];
				pb[1] = tbl[0][tbl[2][pb[1]]];
				pb[0] = tbl[0][tbl[3][pb[0]]];
				pb += sizeof(BYTE) * 3;
			}
		}
	}
	else {
		LPBITMAPINFO pbi = (LPBITMAPINFO)pBase;
		x = dib_numcolors(pBase);
		for (y = 0; y < x; y++) {
			pbi->bmiColors[y].rgbRed = tbl[0][tbl[1][pbi->bmiColors[y].rgbRed]];
			pbi->bmiColors[y].rgbGreen = tbl[0][tbl[2][pbi->bmiColors[y].rgbGreen]];
			pbi->bmiColors[y].rgbBlue = tbl[0][tbl[3][pbi->bmiColors[y].rgbBlue]];
		}
	}

	GlobalUnlock(hDst);
	return hDst;
}

extern "C"
HDIB WINAPI imgToneCurve(HDIB hSrc, int ref, int val)
{
	TONEINFO ti;
	ti.rgb.count = 1;
	ti.rgb.ref[0] = ref;
	ti.rgb.val[0] = val;
	ti.red.count = 0;
	ti.green.count = 0;
	ti.blue.count = 0;

	return imgToneCurveEx(hSrc, &ti);
}

