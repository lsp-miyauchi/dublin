///////////////////////////////////////////////////////////////////////
// cpmask.c

#include "stdafx.h"
#include "lspmask.h"
#include "common.h"
#include "dib_sup.h"

#define CP_MAX	16  /* CP code length max */

static BYTE cbCPtbl[] = {
	0x10, 0x17, 0x13, 0x15, 0x09,
	0x08, 0x0a, 0x14, 0x06, 0x05,
	0x16, 0x02, 0x0d, 0x03, 0x01,
	0x04, 0x19, 0x0c, 0x0f, 0x0e,
	0x12, 0x07, 0x0b, 0x18, 0x11, 0x1a
};

typedef struct tagCPS {
	int index;
	BOOL flg;
} CPS, *LPCPS;

LPVOID cptable(int count, char *pszCode)
{
	register int i, k, v1, v2, len;
	register int* tbl;
	CPS* cps;

	cps = (CPS*)malloc(sizeof(CPS) * count);
	tbl = (int*)malloc(sizeof(int) * count);
	for (i = 0; i < count; i++)
	{
		tbl[i] = -1;
		cps[i].index = i;
		cps[i].flg = FALSE;
	}

	len = lstrlen(pszCode);
	v1 = count - 1;
	v2 = len + count % len;
	for (k = 0; k < count; k++)
	{
		v1 = cbCPtbl[pszCode[k % len] - 'A'] + v1 + v2;
		if (v1 >= count)
			v1 %= count;

		while (tbl[v1] != -1)
		{
			if (k & 01)
			{
				if (v1 == 0)
					v1 = count;
				v1--;
			}
			else
			{
				if (++v1 >= count)
					v1 = 0;
			}
		}
		tbl[v1] = k;
		v2++;
	}
	for (i = 0, k = count - 1; i < k; i++, k--)
	{
		cps[tbl[i]].index = tbl[k];
		cps[tbl[k]].index = tbl[i];
		if ((tbl[i] ^ tbl[k]) & 0x01)
			cps[tbl[i]].flg = cps[tbl[k]].flg = TRUE;
	}
	free(tbl);
	return cps;
}

void mrt(LPRGBPK pcb)
{
	register int x, y;
	RGBPK cbw[64];
	memcpy(cbw, pcb, 64 * sizeof(RGBPK));
	for (y = 0; y < 8; y++)
		for (x = 0; x < 8; x++)
			pcb[(x << 3) + y] = cbw[(y << 3) + x];
}

void cpmask(LPRGBPK pkt, CPS* cps, int dx, int dy)
{
	register int x, y, xBlock, yBlock;
	int n, sx, sy;
	BYTE cb;

	xBlock = dx >> 3;
	yBlock = dy >> 3;

	LPRGBPK pcbw = new RGBPK[64];
	LPRGBPK pw = new RGBPK[dx * dy];
	memcpy(pw, pkt, dx * dy * sizeof(RGBPK));
	for (y = 0; y < yBlock; y++)
	{
		for (x = 0; x < xBlock; x++)
		{
			n = y * xBlock + x;
			sx = (cps[n].index % xBlock) << 3;
			sy = (cps[n].index / xBlock) << 3;

			loadpkt8(pcbw, pw, x << 3, y << 3, dx, dy);
			if (cps[n].flg)
				mrt(pcbw);
			savepkt8(pcbw, pkt, sx, sy, dx, dy);
		}
	}
	delete [] pw;
	delete [] pcbw;

	for (x = 0; x < dx * dy; x++)
	{
		cb = pkt->red;
		pkt->red = ~pkt->green;
		pkt->green = ~cb;
		pkt->blue = ~pkt->blue;
		pkt++;
	}
}

extern "C"
HDIB WINAPI mask_cp_ex(HDIB hSrc, LPRECT prc, int nCount, LPCSTR* pszCode)
{
	if (!hSrc || nCount < 1 || !pszCode)
		return NULL;

	LPBITMAPINFOHEADER pbh = (LPBITMAPINFOHEADER)GlobalLock(hSrc);
	int cx = pbh->biWidth;
	int cy = pbh->biHeight;
	int bits = pbh->biBitCount;

	// 24ビット未満またはエンコードされているものは対象外
	if (bits != 24 || pbh->biCompression != BI_RGB)
	{
		GlobalUnlock(hSrc);
		return NULL;
	}
	GlobalUnlock(hSrc);

	CRect rc;
	if (!prc || IsRectEmpty(prc))
		rc.SetRect(0, 0, cx, cy);
	else
		CopyRect(&rc, prc);
	rc.right = min(rc.right, cx);
	rc.bottom = min(rc.bottom, cy);

	int dx = rc.Width();
	int dy = rc.Height();
	dx = (dx >> 3) << 3;
	dy = (dy >> 3) << 3;
	rc.right = rc.left + dx;
	rc.bottom = rc.top + dy;

	if (dx < 8 || dy < 8)
		return NULL;

	HDIB hDst = dib_copy(hSrc);
	if (!hDst)
		return NULL;

	BOOL bOK = TRUE;
	RGBPK* pkt = dib_getpacket(hDst, rc.left, rc.top, dx, dy);
	CPS* cps;
	char *p, code[MAX_PATH];
	for (int i = 0; i < nCount; i++)
	{
		if (!(*pszCode) || !lstrlen(*pszCode) || lstrlen(*pszCode) > CP_MAX)
		{
			bOK = FALSE;
			break;
		}

		lstrcpy(code, *pszCode++);
		for (p = code; *p; p++)
		{
			if (!isalpha(*p))
			{
				bOK = FALSE;
				break;
			}
		}
		if (!bOK)
			break;

		CharUpper(code);

		cps = (CPS*)cptable((dx >> 3) * (dy >> 3), code);
		if (!cps)
		{
			bOK = FALSE;
			break;
		}

		cpmask(pkt, cps, dx, dy);
		dib_setpacket(hDst, pkt, rc.left, rc.top, dx, dy);
		free(cps);
	}
	dib_freepacket(pkt);

	if (!bOK)
	{
		GlobalFree(hDst);
		hDst = NULL;
	}
	return hDst;
}

extern "C"
HDIB WINAPI mask_cp(HDIB hSrc, LPRECT prc, LPCSTR pszCode)
{
	return mask_cp_ex(hSrc, prc, 1, &pszCode);
}

