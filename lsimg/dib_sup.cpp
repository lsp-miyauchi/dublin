//////////////////////////////////////////////////////////////////////
// dib_sup.cpp

#include "stdafx.h"
#include "lsimg.h"
#include "common.h"

#define PALVERSION			0x300


HDIB __fastcall dib_create(int width, int height, int bits)
{
	BITMAPINFOHEADER bi;
	memset(&bi, 0, sizeof(bi));
	bi.biSize = sizeof(bi);
	bi.biWidth = width;
	bi.biHeight = height;
	bi.biPlanes = 1;
	bi.biBitCount = (WORD)bits;
	bi.biCompression = bits == 16 ? BI_BITFIELDS : BI_RGB;

	width = WIDTHBYTES(bits * width);
	size_t size = bi.biSize + dib_palsize(&bi) + (width * (height + 16));
	bi.biSizeImage = width * height;
	HDIB hdib = GlobalAlloc(GMEM_MOVEABLE, size);
	if (hdib) {
		BITMAPINFOHEADER* pbh = dib_lockheader(hdib);
		memset(pbh, 0, size);
		memcpy(pbh, &bi, sizeof(bi));
		dib_unlock(hdib);
	}
	return hdib;
}

HDIB __fastcall dib_copy(HDIB hsrc)
{
	HDIB hdst = NULL;
	if (hsrc) {
		size_t memsize = GlobalSize(hsrc);
		hdst = GlobalAlloc(GMEM_MOVEABLE, memsize);
		if (hdst) {
			void* ps = GlobalLock(hsrc);
			void* pd = GlobalLock(hdst);
			memcpy(pd, ps, memsize);
			GlobalUnlock(hsrc);
			GlobalUnlock(hdst);
		}
	}
	return hdst;
}

void* __fastcall dib_copyptr(void* pSrc)
{
	void* pDst = NULL;
	if (pSrc) {
		size_t memsize = GlobalSize(GlobalPtrHandle(pSrc));
		pDst = GlobalAllocPtr(GMEM_MOVEABLE, memsize);
		memcpy(pDst, pSrc, memsize);
	}
	return pDst;
}

int __fastcall dib_numcolors(void* pv)
{
	int colors = ((BITMAPINFOHEADER*)pv)->biClrUsed;
	if (!colors) {
		int bits = ((BITMAPINFOHEADER*)pv)->biBitCount;
		colors = (bits <= 8) ? (1 << bits) : 0;
	}
	return colors;
}

int __fastcall dib_palsize(void* pdib)
{
	int size = dib_numcolors(pdib) * sizeof(RGBQUAD);
	if (((BITMAPINFOHEADER*)pdib)->biCompression == BI_BITFIELDS)
		size += sizeof(DWORD) * 3;
	return size;
}

BOOL __fastcall dib_getinfo(HDIB hdib, int* dx, int* dy, int* colors, int* bits)
{
	BOOL bOK = FALSE;
	if (hdib) {
		BITMAPINFOHEADER* pbh = dib_lockheader(hdib);
		if (pbh) {
			if (dx)		*dx = pbh->biWidth;
			if (dy)		*dy = pbh->biHeight;
			if (colors)	*colors = dib_numcolors(pbh);
			if (bits)	*bits = pbh->biBitCount;
			bOK= TRUE;
		}
		dib_unlock(hdib);
	}
	return bOK;
}

HDIB __fastcall dib_createsolid(DWORD dwWidth, DWORD dwHeight, COLORREF rgb)
{
	register BYTE* pb;
	register DWORD x, y;

	HDIB hdib = dib_create(dwWidth, dwHeight, 24);
	if (hdib) {
		int widthbytes = WIDTHBYTES(dwWidth * 24);
		BITMAPINFOHEADER* pbh = dib_lockheader(hdib);
		BYTE* base = dib_findbits(pbh);
		for (y = 0; y < dwHeight; y++) {
			pb = base + (dwHeight - y - 1) * widthbytes;
			for (x = 0; x < dwWidth; x++) {
				*pb++ = GetBValue(rgb);
				*pb++ = GetGValue(rgb);
				*pb++ = GetRValue(rgb);
			}
		}
		dib_unlock(hdib);
	}
	return hdib;
}

RGBPixel* __fastcall dib_getpacket(HDIB hdib, int sx, int sy, int dx, int dy)
{
	if (!hdib) return NULL;

	register int x, y;
	register BYTE* ps;
	register RGBPixel* pw;

	BITMAPINFOHEADER* pbh = dib_lockheader(hdib);
	RGBPixel* pkt = NULL;
	if (pbh->biBitCount > 8) {
		int cx = pbh->biWidth;
		int cy = pbh->biHeight;
		int width = WIDTHBYTES(cx * pbh->biBitCount);
		if (dx == 0) dx = cx;
		if (dy == 0) dy = cy;

		dx = min(dx, cx - sx);
		dy = min(dy, cy - sy);

		pkt = (RGBPixel*)GlobalAllocPtr(GMEM_MOVEABLE, cx * cy * 3);
		if (pkt) {
			pw = pkt;
			LPBYTE pBase = dib_findbits(pbh);
			for (y = sy; y < sy + dy; y++) {
				ps = pBase + (cy - y - 1) * width + sx * 3;
				for (x = 0; x < dx; x++) {
					pw->blue = *ps++;
					pw->green = *ps++;
					pw->red = *ps++;
					pw++;
				}
			}
		}
	}
	else {
		int colors = dib_numcolors(pbh);
		pkt = (RGBPixel*)GlobalAllocPtr(GMEM_MOVEABLE, colors * 3);
		if (pkt) {
			pw = pkt;
			LPBITMAPINFO pbi = (LPBITMAPINFO)pbh;
			for (x = 0; x < colors; x++) {
				pw->blue = pbi->bmiColors[x].rgbBlue;
				pw->green = pbi->bmiColors[x].rgbGreen;
				pw->red = pbi->bmiColors[x].rgbRed;
				pw++;
			}
		}
	}
	dib_unlock(hdib);
	return pkt;
}

void __fastcall dib_setpacket(HDIB hdib, RGBPixel* pkt, int sx, int sy, int dx, int dy)
{
	if (!hdib || !pkt) return;

	register int x, y;
	register LPBYTE ps;
	register RGBPixel* pw = pkt;

	BITMAPINFOHEADER* pbh = dib_lockheader(hdib);
	if (pbh->biBitCount > 8) {
		int cx = pbh->biWidth;
		int cy = pbh->biHeight;
		int width = WIDTHBYTES(cx * pbh->biBitCount);
		if (dx == 0) dx = cx;
		if (dy == 0) dy = cy;

		dx = min(dx, cx - sx);
		dy = min(dy, cy - sy);

		BYTE* pBase = dib_findbits(pbh);
		for (y = sy; y < sy + dy; y++) {
			ps = pBase + (cy - y - 1) * width + sx * 3;
			for (x = 0; x < dx; x++) {
				*ps++ = pw->blue;
				*ps++ = pw->green;
				*ps++ = pw->red;
				pw++;
			}
		}
	}
	else {
		int colors = dib_numcolors(pbh);
		LPBITMAPINFO pbi = (LPBITMAPINFO)pbh;
		for (x = 0; x < colors; x++) {
			pbi->bmiColors[x].rgbBlue = pw->blue;
			pbi->bmiColors[x].rgbGreen = pw->green;
			pbi->bmiColors[x].rgbRed = pw->red;
			pw++;
		}
	}
	dib_unlock(hdib);
}

BOOL __fastcall dib_getplane(HDIB hDIB, UINT nPlane, LPBYTE pPlane)
{
	register int x, y, off;
	register LPBYTE pBase, ps, pt;

	BITMAPINFOHEADER* pbh = dib_lockheader(hDIB);
	pBase = (BYTE*)pbh;
	int cx = pbh->biWidth;
	int cy = pbh->biHeight;
	int bits = pbh->biBitCount;

	if (bits > 8) {
		off = 0;
		if (nPlane == PLANE_GREEN)
			off = sizeof(BYTE);
		else if (nPlane == PLANE_RED)
			off += sizeof(BYTE) * 2;
	}

	pBase = dib_findbits(pBase);
	int width = WIDTHBYTES(cx * bits);
	pt = pPlane;

	if (bits >  8) {
		for (y = 0; y < cy; y++) {
			ps = pBase + width * (cy - y - 1) + off;
			for (x = 0; x < cx; x++) {
				*pt++ = *ps;
				ps += sizeof(BYTE) * 3;
			}
		}
	}
	else if (bits == 8) {
		ps = pBase + width * (cy - 1);
		for (y = 0; y < cy; y++) {
			memcpy(pt, ps, cx);
			ps -= width;
			pt += cx;
		}
	}
	else if (bits == 4) {
		for (y = 0; y < cy; y++) {
			ps = pBase + width * (cy - y - 1);
			for (x = 0; x < cx; x++) {
				if (x & 1) {
					*pt++ = (*ps) & 0xf;
					ps++;
				}
				else
					*pt++ = ((*ps) >> 4) & 0xf;
			}
		}
	}
	else if (bits == 1) {
		for (y = 0; y < cy; y++) {
			ps = pBase + width * (cy - y - 1);
			for (x = 0; x < cx; x++) {
				*pt++ = ((*ps) >> (7 - (x & 7))) & 1;
				if ((x & 7) == 7)
					ps++;
			}
		}
	}
	dib_unlock(hDIB);
	return TRUE;
}

BOOL __fastcall dib_setplane(HDIB hDIB, UINT nPlane, LPBYTE pPlane)
{
	register int x, y, off;
	register LPBYTE pBase, ps, pt;

	BITMAPINFOHEADER* pbh = dib_lockheader(hDIB);
	pBase = (BYTE*)pbh;
	int cx = pbh->biWidth;
	int cy = pbh->biHeight;
	int bits = pbh->biBitCount;

	if (bits > 8) {
		off = 0;
		if (nPlane == PLANE_GREEN)
			off = sizeof(BYTE);
		else if (nPlane == PLANE_RED)
			off += sizeof(BYTE) * 2;
	}

	pBase = dib_findbits(pBase);
	int width = WIDTHBYTES(cx * bits);
	pt = pPlane;

	if (bits > 8) {
		for (y = 0; y < cy; y++) {
			ps = pBase + width * (cy - y - 1) + off;
			for (x = 0; x < cx; x++) {
				*ps = *pt++;
				ps += sizeof(BYTE) * 3;
			}
		}
	}
	else if (bits == 8) {
		ps = pBase + width * (cy - 1);
		for (y = 0; y < cy; y++) {
			memcpy(ps, pt, cx);
			ps -= width;
			pt += cx;
		}
	}
	else if (bits == 4) {
		for (y = 0; y < cy; y++) {
			ps = pBase + width * (cy - y - 1);
			for (x = 0; x < cx; x++) {
				if (x & 1) {
					off |= *pt++;
					*ps++ = (BYTE)off;
				}
				else {
					off = *pt << 4;
					*pt++;
				}
			}
		}
	}
	else if (bits == 1) {
		for (y = 0; y < cy; y++) {
			ps = pBase + width * (cy - y - 1);
			for (x = 0; x < cx; x++) {
				if (!(x & 7))
					*ps = 0;

				*ps |= (*pt & 1) << (7 - (x & 7));
				pt++;
				if ((x & 7) == 7)
					ps++;
			}
		}
	}
	dib_unlock(hDIB);
	return TRUE;
}

BOOL __fastcall dib_planeblt(BYTE* pDst, int sxDst, int syDst, int cxDst, int cyDst, BYTE* pSrc, int sxSrc, int sySrc, int cxSrc, int cySrc, int xDelta, int yDelta)
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
	for (y = 0; y < yDelta; y++) {
		memcpy(pd, ps, xDelta);
		ps += cxSrc;
		pd += cxDst;
	}
	return TRUE;
}

HDIB __fastcall dib_createfrombitmap(HBITMAP hbm, HPALETTE hpal)
{
	if (!hbm) return NULL;

	BITMAP bm;
	if (!GetObject(hbm, sizeof(bm), (LPSTR)&bm)) return NULL;
	if (!hpal) hpal = (HPALETTE)GetStockObject(DEFAULT_PALETTE);

	uint16_t bits = bm.bmPlanes * bm.bmBitsPixel;
	if (bits <= 1)		bits = 1;
    else if (bits <= 4)	bits = 4;
    else if (bits <= 8)	bits = 8;
    else				bits = 24;

	BITMAPINFOHEADER bi;
	memset(&bi, 0, sizeof(bi));
    bi.biSize = sizeof(BITMAPINFOHEADER);
    bi.biWidth = bm.bmWidth;
    bi.biHeight = bm.bmHeight;
    bi.biPlanes = 1;
    bi.biBitCount = bits;
    bi.biCompression = BI_RGB;

	uint32_t dibsize = bi.biSize + dib_palsize(&bi);
	HDC hdc = GetDC(NULL);
	hpal = SelectPalette(hdc, hpal, FALSE);
	RealizePalette(hdc);

	HANDLE hdib = GlobalAlloc(GMEM_MOVEABLE, dibsize);
	if (!hdib) {
		SelectPalette(hdc, hpal, TRUE);
		RealizePalette(hdc);
		ReleaseDC(NULL, hdc);
		return NULL;
	}

	BITMAPINFOHEADER* lpbi = dib_lockheader(hdib);
    *lpbi = bi;

	// call GetDIBits with a NULL lpBits param, so it will calculate the
	// biSizeImage field for us    
	GetDIBits(hdc, hbm, 0, (UINT)bi.biHeight, NULL, (BITMAPINFO*)lpbi, DIB_RGB_COLORS);

	// get the info. returned by GetDIBits and unlock memory block
	bi = *lpbi;
	dib_unlock(hdib);

	// if the driver did not fill in the biSizeImage field, make one up 
	if (bi.biSizeImage == 0)
		bi.biSizeImage = WIDTHBYTES((uint32_t)bm.bmWidth * bits) * (uint32_t)bm.bmHeight;

	// realloc the buffer big enough to hold all the bits
	dibsize = bi.biSize + dib_palsize((LPSTR)&bi) + bi.biSizeImage;

	HANDLE h = GlobalReAlloc(hdib, dibsize, 0);
	if (h)
		hdib = h;
	else {
		dib_destroy(hdib);
		SelectPalette(hdc, hpal, TRUE);
		RealizePalette(hdc);
		ReleaseDC(NULL, hdc);
		return NULL;
	}

	lpbi = dib_lockheader(hdib);

	// call GetDIBits with a NON-NULL lpBits param, and actualy get the
	// bits this time
	if (GetDIBits(hdc, hbm, 0, (UINT)bi.biHeight,
				(LPSTR)lpbi + (WORD)lpbi->biSize + dib_palsize(lpbi),
				(BITMAPINFO*)lpbi, DIB_RGB_COLORS) == 0)
	{
		dib_unlock(hdib);
		hdib = NULL;
	}
	else {
		bi = *lpbi;
		dib_unlock(hdib);
	}
	SelectPalette(hdc, hpal, TRUE);
	RealizePalette(hdc);
	ReleaseDC(NULL, hdc);

	// return handle to the DIB
	return hdib;
}
