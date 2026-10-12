//////////////////////////////////////////////////////////////////////
// dib_sup.cpp

#include "stdafx.h"
#include "lspmask.h"
#include "common.h"
#include "dib_sup.h"

#define PALVERSION			0x300


HDIB dib_create(DWORD dwWidth, DWORD dwHeight, DWORD dwBits)
{
	if (dwBits <= 1)
		dwBits = 1;
    else if (dwBits <= 4)
		dwBits = 4;
    else if (dwBits <= 8)
        dwBits = 8;
    else if (dwBits <= 24)
        dwBits = 24;
    else
        dwBits = 4;					// set default value to 4 if parameter is bogus

	BITMAPINFOHEADER bi;
	memset(&bi, 0, sizeof(bi));
	bi.biSize = sizeof(bi);
    bi.biWidth = dwWidth;
    bi.biHeight = dwHeight;
    bi.biPlanes = 1;
	bi.biBitCount = LOWORD(dwBits);
	bi.biCompression = BI_RGB;

	DWORD width = WIDTHBYTES(dwBits * dwWidth);
	DWORD size = bi.biSize + dib_palsize(&bi) + (width * (dwHeight + 16));
	HDIB hdib = GlobalAlloc(GMEM_MOVEABLE, size);
	if (!hdib)
		return NULL;

	LPBITMAPINFOHEADER pbh = (LPBITMAPINFOHEADER)GlobalLock(hdib);
	memset(pbh, 0, size);
	memcpy(pbh, &bi, sizeof(bi));
	GlobalUnlock(hdib);
    return hdib;
}

HDIB dib_copy(HDIB hsrc)
{
	HDIB hdst = NULL;
	if (hsrc)
	{
		DWORD dwSize = GlobalSize(hsrc);
		if (hdst = GlobalAlloc(GMEM_MOVEABLE, dwSize))
		{
			void* ps = GlobalLock(hsrc);
			void* pd = GlobalLock(hdst);
			memcpy(pd, ps, dwSize);
			GlobalUnlock(hsrc);
			GlobalUnlock(hdst);
		}
	}
	return hdst;
}

void* dib_copyptr(void* pSrc)
{
	void* pDst = NULL;
	if (pSrc)
	{
		DWORD dwSize = GlobalSize(GlobalPtrHandle(pSrc));
		pDst = GlobalAllocPtr(GMEM_MOVEABLE, dwSize);
		memcpy(pDst, pSrc, dwSize);
	}
	return pDst;
}

int dib_numcolors(void* pv)
{
	int colors = ((LPBITMAPINFOHEADER)pv)->biClrUsed;
	if (!colors)
	{
		int bits = ((LPBITMAPINFOHEADER)pv)->biBitCount;
		colors = (bits <= 8) ? (1 << bits) : 0;
	}
	return colors;
}

BOOL dib_getinfo(HDIB hdib, int* dx, int* dy, int* colors, int* bits)
{
	BOOL bOK = FALSE;
	if (hdib)
	{
		LPBITMAPINFOHEADER pbh = (LPBITMAPINFOHEADER)GlobalLock(hdib);
		if (pbh)
		{
			if (dx)
				*dx = pbh->biWidth;
			if (dy)
				*dy = pbh->biHeight;
			if (colors)
				*colors = dib_numcolors(pbh);
			if (bits)
				*bits = pbh->biBitCount;
			bOK= TRUE;
		}
		GlobalUnlock(hdib);
	}
	return bOK;
}

HDIB dib_createsolid(DWORD dwWidth, DWORD dwHeight, COLORREF rgb)
{
	register BYTE* pb;
	register DWORD x, y;

	HDIB hdib = dib_create(dwWidth, dwHeight, 24);
	if (hdib)
	{
		int widthbytes = WIDTHBYTES(dwWidth * 24);
		LPBITMAPINFOHEADER pbh = (LPBITMAPINFOHEADER)GlobalLock(hdib);
		BYTE* base = dib_findbits(pbh);
		for (y = 0; y < dwHeight; y++)
		{
			pb = base + (dwHeight - y - 1) * widthbytes;
			for (x = 0; x < dwWidth; x++)
			{
				*pb++ = GetBValue(rgb);
				*pb++ = GetGValue(rgb);
				*pb++ = GetRValue(rgb);
			}
		}
		GlobalLock(hdib);
	}
	return hdib;
}

RGBPK* dib_getpacket(HDIB hdib, int sx, int sy, int dx, int dy)
{
	register int x, y;
	register BYTE* ps;
	register RGBPK* pw;

	if (!hdib)
		return NULL;

	LPBITMAPINFOHEADER pbh = (LPBITMAPINFOHEADER)GlobalLock(hdib);
	RGBPK* pkt = NULL;
	if (pbh->biBitCount > 8)
	{
		int cx = pbh->biWidth;
		int cy = pbh->biHeight;
		int width = WIDTHBYTES(cx * pbh->biBitCount);
		if (dx == 0)
			dx = cx;
		if (dy == 0)
			dy = cy;

		dx = min(dx, cx - sx);
		dy = min(dy, cy - sy);

		pkt = (LPRGBPK)GlobalAllocPtr(GMEM_MOVEABLE, cx * cy * 3);
		if (pkt)
		{
			pw = pkt;
			LPBYTE pBase = dib_findbits(pbh);
			for (y = sy; y < sy + dy; y++)
			{
				ps = pBase + (cy - y - 1) * width + sx * 3;
				for (x = 0; x < dx; x++)
				{
					pw->blue = *ps++;
					pw->green = *ps++;
					pw->red = *ps++;
					pw++;
				}
			}
		}
	}
	else
	{
		int colors = dib_numcolors(pbh);
		pkt = (LPRGBPK)GlobalAllocPtr(GMEM_MOVEABLE, colors * 3);
		if (pkt)
		{
			pw = pkt;
			LPBITMAPINFO pbi = (LPBITMAPINFO)pbh;
			for (x = 0; x < colors; x++)
			{
				pw->blue = pbi->bmiColors[x].rgbBlue;
				pw->green = pbi->bmiColors[x].rgbGreen;
				pw->red = pbi->bmiColors[x].rgbRed;
				pw++;
			}
		}
	}
	GlobalUnlock(hdib);
	return pkt;
}

void dib_setpacket(HDIB hdib, RGBPK* pkt, int sx, int sy, int dx, int dy)
{
	register int x, y;
	register LPBYTE ps;
	register LPRGBPK pw = pkt;

	if (!hdib || !pkt)
		return;

	LPBITMAPINFOHEADER pbh = (LPBITMAPINFOHEADER)GlobalLock(hdib);
	if (pbh->biBitCount > 8)
	{
		int cx = pbh->biWidth;
		int cy = pbh->biHeight;
		int width = WIDTHBYTES(cx * pbh->biBitCount);
		if (dx == 0)
			dx = cx;
		if (dy == 0)
			dy = cy;

		dx = min(dx, cx - sx);
		dy = min(dy, cy - sy);

		BYTE* pBase = dib_findbits(pbh);
		for (y = sy; y < sy + dy; y++)
		{
			ps = pBase + (cy - y - 1) * width + sx * 3;
			for (x = 0; x < dx; x++)
			{
				*ps++ = pw->blue;
				*ps++ = pw->green;
				*ps++ = pw->red;
				pw++;
			}
		}
	}
	else
	{
		int colors = dib_numcolors(pbh);
		LPBITMAPINFO pbi = (LPBITMAPINFO)pbh;
		for (x = 0; x < colors; x++)
		{
			pbi->bmiColors[x].rgbBlue = pw->blue;
			pbi->bmiColors[x].rgbGreen = pw->green;
			pbi->bmiColors[x].rgbRed = pw->red;
			pw++;
		}
	}
}

BOOL dib_getplane(HDIB hDIB, UINT nPlane, LPBYTE pPlane)
{
	register int x, y, off;
	register LPBYTE pBase, ps, pt;

	pBase = (LPBYTE)GlobalLock(hDIB);
	LPBITMAPINFOHEADER pbh = (LPBITMAPINFOHEADER)pBase;
	int cx = pbh->biWidth;
	int cy = pbh->biHeight;
	int bits = pbh->biBitCount;

	// エンコードされている画像は対象外
	if (pbh->biCompression != BI_RGB)
	{
		GlobalUnlock(hDIB);
		return FALSE;
	}

	if (bits > 8)
	{
		off = 0;
		if (nPlane == PLANE_GREEN)
			off = sizeof(BYTE);
		else if (nPlane == PLANE_RED)
			off += sizeof(BYTE) * 2;
	}

	pBase = dib_findbits(pBase);
	int width = WIDTHBYTES(cx * bits);
	pt = pPlane;

	if (bits >  8)
	{
		for (y = 0; y < cy; y++)
		{
			ps = pBase + width * (cy - y - 1) + off;
			for (x = 0; x < cx; x++)
			{
				*pt++ = *ps;
				ps += sizeof(BYTE) * 3;
			}
		}
	}
	else if (bits == 8)
	{
		ps = pBase + width * (cy - 1);
		for (y = 0; y < cy; y++)
		{
			memcpy(pt, ps, cx);
			ps -= width;
			pt += cx;
		}
	}
	else if (bits == 4)
	{
		for (y = 0; y < cy; y++)
		{
			ps = pBase + width * (cy - y - 1);
			for (x = 0; x < cx; x++)
			{
				if (x & 1)
				{
					*pt++ = (*ps) & 0xf;
					ps++;
				}
				else
					*pt++ = ((*ps) >> 4) & 0xf;
			}
		}
	}
	else if (bits == 1)
	{
		for (y = 0; y < cy; y++)
		{
			ps = pBase + width * (cy - y - 1);
			for (x = 0; x < cx; x++)
			{
				*pt++ = ((*ps) >> (7 - (x & 7))) & 1;
				if ((x & 7) == 7)
					ps++;
			}
		}
	}
	GlobalUnlock(hDIB);
	return TRUE;
}

BOOL dib_setplane(HDIB hDIB, UINT nPlane, LPBYTE pPlane)
{
	register int x, y, off;
	register LPBYTE pBase, ps, pt;

	pBase = (LPBYTE)GlobalLock(hDIB);
	LPBITMAPINFOHEADER pbh = (LPBITMAPINFOHEADER)pBase;
	int cx = pbh->biWidth;
	int cy = pbh->biHeight;
	int bits = pbh->biBitCount;

	// エンコードされている画像は対象外
	if (pbh->biCompression != BI_RGB)
	{
		GlobalUnlock(hDIB);
		return FALSE;
	}

	if (bits > 8)
	{
		off = 0;
		if (nPlane == PLANE_GREEN)
			off = sizeof(BYTE);
		else if (nPlane == PLANE_RED)
			off += sizeof(BYTE) * 2;
	}

	pBase = dib_findbits(pBase);
	int width = WIDTHBYTES(cx * bits);
	pt = pPlane;

	if (bits > 8)
	{
		for (y = 0; y < cy; y++)
		{
			ps = pBase + width * (cy - y - 1) + off;
			for (x = 0; x < cx; x++)
			{
				*ps = *pt++;
				ps += sizeof(BYTE) * 3;
			}
		}
	}
	else if (bits == 8)
	{
		ps = pBase + width * (cy - 1);
		for (y = 0; y < cy; y++)
		{
			memcpy(ps, pt, cx);
			ps -= width;
			pt += cx;
		}
	}
	else if (bits == 4)
	{
		for (y = 0; y < cy; y++)
		{
			ps = pBase + width * (cy - y - 1);
			for (x = 0; x < cx; x++)
			{
				if (x & 1)
				{
					off |= *pt++;
					*ps++ = (BYTE)off;
				}
				else
				{
					off = *pt << 4;
					*pt++;
				}
			}
		}
	}
	else if (bits == 1)
	{
		for (y = 0; y < cy; y++)
		{
			ps = pBase + width * (cy - y - 1);
			for (x = 0; x < cx; x++)
			{
				if (!(x & 7))
					*ps = 0;

				*ps |= (*pt & 1) << (7 - (x & 7));
				pt++;
				if ((x & 7) == 7)
					ps++;
			}
		}
	}
	GlobalUnlock(hDIB);
	return TRUE;
}

BOOL dib_planeblt(BYTE* pDst, int sxDst, int syDst, int cxDst, int cyDst, BYTE* pSrc, int sxSrc, int sySrc, int cxSrc, int cySrc, int xDelta, int yDelta)
{
	if (!pSrc || !pDst)
		return FALSE;

	xDelta = min(xDelta, cxSrc - sxSrc);
	xDelta = min(xDelta, cxDst - sxDst);
	yDelta = min(yDelta, cySrc - sySrc);
	yDelta = min(yDelta, cyDst - syDst);

	register LPBYTE ps, pd;
	register int y;

	ps = pSrc + sySrc * cxSrc + sxSrc;
	pd = pDst + syDst * cxDst + sxDst;
	for (y = 0; y < yDelta; y++)
	{
		memcpy(pd, ps, xDelta);
		ps += cxSrc;
		pd += cxDst;
	}
	return TRUE;
}

HDIB dib_createfrombitmap(HBITMAP hbm, HPALETTE hpal)
{
    BITMAP              bm;         // bitmap structure
    BITMAPINFOHEADER    bi;         // bitmap header
    LPBITMAPINFOHEADER  lpbi;       // pointer to BITMAPINFOHEADER
    DWORD               dwLen;      // size of memory block
    HANDLE              hdib, h;    // handle to DIB, temp handle
    HDC                 hdc;        // handle to DC
    WORD                biBits;     // bits per pixel

    if (!hbm)
        return NULL;

    if (!GetObject(hbm, sizeof(bm), (LPSTR)&bm))
        return NULL;

    if (hpal == NULL)
        hpal = (HPALETTE)GetStockObject(DEFAULT_PALETTE);

	biBits = bm.bmPlanes * bm.bmBitsPixel;
	if (biBits <= 1)
        biBits = 1;
    else if (biBits <= 4)
        biBits = 4;
    else if (biBits <= 8)
        biBits = 8;
    else // if greater than 8-bit, force to 24-bit
        biBits = 24;

	memset(&bi, 0, sizeof(bi));
    bi.biSize = sizeof(BITMAPINFOHEADER);
    bi.biWidth = bm.bmWidth;
    bi.biHeight = bm.bmHeight;
    bi.biPlanes = 1;
    bi.biBitCount = biBits;
    bi.biCompression = BI_RGB;

    dwLen = bi.biSize + dib_palsize(&bi);
    hdc = GetDC(NULL);
	hpal = SelectPalette(hdc, hpal, FALSE);
    RealizePalette(hdc);

    hdib = GlobalAlloc(GMEM_MOVEABLE, dwLen);
    if (!hdib)
    {
		SelectPalette(hdc, hpal, TRUE);
		RealizePalette(hdc);
		ReleaseDC(NULL, hdc);
		return NULL;
    }

    lpbi = (LPBITMAPINFOHEADER)GlobalLock(hdib);
    *lpbi = bi;

    // call GetDIBits with a NULL lpBits param, so it will calculate the
    // biSizeImage field for us    
	GetDIBits(hdc, hbm, 0, (UINT)bi.biHeight, NULL,
				(LPBITMAPINFO)lpbi, DIB_RGB_COLORS);

    // get the info. returned by GetDIBits and unlock memory block
    bi = *lpbi;
    GlobalUnlock(hdib);

    // if the driver did not fill in the biSizeImage field, make one up 
    if (bi.biSizeImage == 0)
        bi.biSizeImage = WIDTHBYTES((DWORD)bm.bmWidth * biBits) * bm.bmHeight;

    // realloc the buffer big enough to hold all the bits
    dwLen = bi.biSize + dib_palsize((LPSTR)&bi) + bi.biSizeImage;

    if (h = GlobalReAlloc(hdib, dwLen, 0))
        hdib = h;
    else
    {
        GlobalFree(hdib);
        hdib = NULL;
        SelectPalette(hdc, hpal, TRUE);
        RealizePalette(hdc);
        ReleaseDC(NULL, hdc);
        return NULL;
    }

    lpbi = (LPBITMAPINFOHEADER)GlobalLock(hdib);

    // call GetDIBits with a NON-NULL lpBits param, and actualy get the
    // bits this time
    if (GetDIBits(hdc, hbm, 0, (UINT)bi.biHeight, (LPSTR)lpbi +
            (WORD)lpbi->biSize + dib_palsize(lpbi), (LPBITMAPINFO)lpbi,
            DIB_RGB_COLORS) == 0)
    {
        GlobalUnlock(hdib);
        hdib = NULL;
        SelectPalette(hdc, hpal, TRUE);
        RealizePalette(hdc);
        ReleaseDC(NULL, hdc);
        return NULL;
    }

    bi = *lpbi;

    // clean up 
    GlobalUnlock(hdib);
    SelectPalette(hdc, hpal, TRUE);
    RealizePalette(hdc);
    ReleaseDC(NULL, hdc);

    // return handle to the DIB
    return hdib;
}
