//////////////////////////////////////////////////////////////////////
// dib_sup.cpp

#include "stdafx.h"
#include "lsmif.h"
#include "dib_sup.h"
#include "common.h"

#define PALVERSION			0x300


HANDLE dib_create(DWORD dwWidth, DWORD dwHeight, DWORD dwBits)
{
	if (dwBits <= 1)
		dwBits = 1;
    else if (dwBits <= 4)
		dwBits = 4;
    else if (dwBits <= 8)
        dwBits = 8;
    else if (dwBits <= 24)
        dwBits = 24;
    else if (dwBits <= 32)
        dwBits = 32;
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
	HANDLE hdib = GlobalAlloc(GMEM_MOVEABLE, size);
	if (!hdib)
		return NULL;

	LPBITMAPINFOHEADER pbh = (LPBITMAPINFOHEADER)GlobalLock(hdib);
	memset(pbh, 0, size);
	memcpy(pbh, &bi, sizeof(bi));
	GlobalUnlock(hdib);
    return hdib;
}

int dib_numcolors(void* pv)
{
	int colors = ((LPBITMAPINFOHEADER)pv)->biClrUsed;
	if (!colors) {
		int bits = ((LPBITMAPINFOHEADER)pv)->biBitCount;
		colors = (bits <= 8) ? (1 << bits) : 0;
	}
	return colors;
}

int dib_palsize(void* pdib)
{
	int size = dib_numcolors(pdib) * sizeof(RGBQUAD);
	if (((BITMAPINFOHEADER*)pdib)->biCompression == BI_BITFIELDS)
		size += sizeof(DWORD) * 3;
	return size;
}

HANDLE dib_createfrombitmap(HBITMAP hbm, HPALETTE hpal)
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
    if (!hdib) {
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

	h = GlobalReAlloc(hdib, dwLen, 0);
	if (h) {
		hdib = h;
	} else {
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

HPALETTE dib_createpalette(HANDLE hdib)
{
	if (hdib == NULL)
		return NULL;

	HPALETTE hpal = NULL;
	LPBITMAPINFO pbi = (LPBITMAPINFO)GlobalLock(hdib);
	int colors = dib_numcolors(pbi);
	if (colors != 0) {
		LPLOGPALETTE ppal = (LPLOGPALETTE)malloc(sizeof(LOGPALETTE) +
									sizeof(PALETTEENTRY) * colors);
		if (!ppal) {
			GlobalUnlock(hdib);
			return NULL;
		}

		ppal->palVersion = PALVERSION;
		ppal->palNumEntries = colors;

		for (int i = 0; i < colors; i++) {
			ppal->palPalEntry[i].peRed = pbi->bmiColors[i].rgbRed;
			ppal->palPalEntry[i].peGreen = pbi->bmiColors[i].rgbGreen;
			ppal->palPalEntry[i].peBlue = pbi->bmiColors[i].rgbBlue;
			ppal->palPalEntry[i].peFlags = 0;
		}

		// create the palette and get handle to it
		hpal = CreatePalette(ppal);
		free(ppal);
	}
	GlobalUnlock(hdib);
	return hpal;
}
