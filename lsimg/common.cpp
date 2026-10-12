//////////////////////////////////////////////////////////////////////
// common.cpp

#include "stdafx.h"
#include "lsimg.h"
#include "common.h"
#include "texture.h"

#define PALVERSION			0x300


//////////////////////////////////////
// for jpeg.lib

FILE _iob[] = { *stdin, *stdout, *stderr };
extern "C" FILE* __cdecl __iob_func(void)
{
	return _iob;
}


/////////////////////////////////////////////////////////////////////
//

COLORREF pal_1bpp[2] = {
	RGB(0, 0, 0),
	RGB(255, 255, 255)
};

COLORREF pal_4bpp[16] = {
	RGB(0,0,0),
	RGB(128,0,0),
	RGB(0,128,0),
	RGB(128,128,0),
	RGB(0,0,128),
	RGB(128,0,128),
	RGB(0,128,128),
	RGB(192,192,192),
	RGB(128,128,128),
	RGB(255,0,0),
	RGB(0,255,0),
	RGB(255,255,0),
	RGB(0,0,255),
	RGB(255,0,255),
	RGB(0,255,255),
	RGB(255,255,255)
};

COLORREF pal_8bpp[256] = {
	RGB(0x00, 0x00, 0x00), RGB(0x80, 0x00, 0x00), RGB(0x00, 0x80, 0x00),
	RGB(0x80, 0x80, 0x00), RGB(0x00, 0x00, 0x80), RGB(0x80, 0x00, 0x80),
	RGB(0x00, 0x80, 0x80), RGB(0xC0, 0xC0, 0xC0), RGB(0xC0, 0xDC, 0xC0),
	RGB(0xA6, 0xCA, 0xF0), RGB(0x04, 0x04, 0x04), RGB(0x08, 0x08, 0x08),
	RGB(0x0C, 0x0C, 0x0C), RGB(0x11, 0x11, 0x11), RGB(0x16, 0x16, 0x16),
	RGB(0x1C, 0x1C, 0x1C), RGB(0x22, 0x22, 0x22), RGB(0x29, 0x29, 0x29),
	RGB(0x55, 0x55, 0x55), RGB(0x4D, 0x4D, 0x4D), RGB(0x42, 0x42, 0x42),
	RGB(0x39, 0x39, 0x39), RGB(0xFF, 0x7C, 0x80), RGB(0xFF, 0x50, 0x50),
	RGB(0xD6, 0x00, 0x93), RGB(0xCC, 0xEC, 0xFF), RGB(0xEF, 0xD6, 0xC6),
	RGB(0xE7, 0xE7, 0xD6), RGB(0xAD, 0xA9, 0x90), RGB(0x33, 0x00, 0x00),
	RGB(0x66, 0x00, 0x00), RGB(0x99, 0x00, 0x00), RGB(0xCC, 0x00, 0x00),
	RGB(0x00, 0x33, 0x00), RGB(0x33, 0x33, 0x00), RGB(0x66, 0x33, 0x00),
	RGB(0x99, 0x33, 0x00), RGB(0xCC, 0x33, 0x00), RGB(0xFF, 0x33, 0x00),
	RGB(0x00, 0x66, 0x00), RGB(0x33, 0x66, 0x00), RGB(0x66, 0x66, 0x00),
	RGB(0x99, 0x66, 0x00), RGB(0xCC, 0x66, 0x00), RGB(0xFF, 0x66, 0x00),
	RGB(0x00, 0x99, 0x00), RGB(0x33, 0x99, 0x00), RGB(0x66, 0x99, 0x00),
	RGB(0x99, 0x99, 0x00), RGB(0xCC, 0x99, 0x00), RGB(0xFF, 0x99, 0x00),
	RGB(0x00, 0xCC, 0x00), RGB(0x33, 0xCC, 0x00), RGB(0x66, 0xCC, 0x00),
	RGB(0x99, 0xCC, 0x00), RGB(0xCC, 0xCC, 0x00), RGB(0xFF, 0xCC, 0x00),
	RGB(0x66, 0xFF, 0x00), RGB(0x99, 0xFF, 0x00), RGB(0xCC, 0xFF, 0x00),
	RGB(0x00, 0x00, 0x33), RGB(0x33, 0x00, 0x33), RGB(0x66, 0x00, 0x33),
	RGB(0x99, 0x00, 0x33), RGB(0xCC, 0x00, 0x33), RGB(0xFF, 0x00, 0x33),
	RGB(0x00, 0x33, 0x33), RGB(0x33, 0x33, 0x33), RGB(0x66, 0x33, 0x33),
	RGB(0x99, 0x33, 0x33), RGB(0xCC, 0x33, 0x33), RGB(0xFF, 0x33, 0x33),
	RGB(0x00, 0x66, 0x33), RGB(0x33, 0x66, 0x33), RGB(0x66, 0x66, 0x33),
	RGB(0x99, 0x66, 0x33), RGB(0xCC, 0x66, 0x33), RGB(0xFF, 0x66, 0x33),
	RGB(0x00, 0x99, 0x33), RGB(0x33, 0x99, 0x33), RGB(0x66, 0x99, 0x33),
	RGB(0x99, 0x99, 0x33), RGB(0xCC, 0x99, 0x33), RGB(0xFF, 0x99, 0x33),
	RGB(0x00, 0xCC, 0x33), RGB(0x33, 0xCC, 0x33), RGB(0x66, 0xCC, 0x33),
	RGB(0x99, 0xCC, 0x33), RGB(0xCC, 0xCC, 0x33), RGB(0xFF, 0xCC, 0x33),
	RGB(0x33, 0xFF, 0x33), RGB(0x66, 0xFF, 0x33), RGB(0x99, 0xFF, 0x33),
	RGB(0xCC, 0xFF, 0x33), RGB(0xFF, 0xFF, 0x33), RGB(0x00, 0x00, 0x66),
	RGB(0x33, 0x00, 0x66), RGB(0x66, 0x00, 0x66), RGB(0x99, 0x00, 0x66),
	RGB(0xCC, 0x00, 0x66), RGB(0xFF, 0x00, 0x66), RGB(0x00, 0x33, 0x66),
	RGB(0x33, 0x33, 0x66), RGB(0x66, 0x33, 0x66), RGB(0x99, 0x33, 0x66),
	RGB(0xCC, 0x33, 0x66), RGB(0xFF, 0x33, 0x66), RGB(0x00, 0x66, 0x66),
	RGB(0x33, 0x66, 0x66), RGB(0x66, 0x66, 0x66), RGB(0x99, 0x66, 0x66),
	RGB(0xCC, 0x66, 0x66), RGB(0x00, 0x99, 0x66), RGB(0x33, 0x99, 0x66),
	RGB(0x66, 0x99, 0x66), RGB(0x99, 0x99, 0x66), RGB(0xCC, 0x99, 0x66),
	RGB(0xFF, 0x99, 0x66), RGB(0x00, 0xCC, 0x66), RGB(0x33, 0xCC, 0x66),
	RGB(0x99, 0xCC, 0x66), RGB(0xCC, 0xCC, 0x66), RGB(0xFF, 0xCC, 0x66),
	RGB(0x00, 0xFF, 0x66), RGB(0x33, 0xFF, 0x66), RGB(0x99, 0xFF, 0x66),
	RGB(0xCC, 0xFF, 0x66), RGB(0xFF, 0x00, 0xCC), RGB(0xCC, 0x00, 0xFF),
	RGB(0x00, 0x99, 0x99), RGB(0x99, 0x33, 0x99), RGB(0x99, 0x00, 0x99),
	RGB(0xCC, 0x00, 0x99), RGB(0x00, 0x00, 0x99), RGB(0x33, 0x33, 0x99),
	RGB(0x66, 0x00, 0x99), RGB(0xCC, 0x33, 0x99), RGB(0xFF, 0x00, 0x99),
	RGB(0x00, 0x66, 0x99), RGB(0x33, 0x66, 0x99), RGB(0x66, 0x33, 0x99),
	RGB(0x99, 0x66, 0x99), RGB(0xCC, 0x66, 0x99), RGB(0xFF, 0x33, 0x99),
	RGB(0x33, 0x99, 0x99), RGB(0x66, 0x99, 0x99), RGB(0x99, 0x99, 0x99),
	RGB(0xCC, 0x99, 0x99), RGB(0xFF, 0x99, 0x99), RGB(0x00, 0xCC, 0x99),
	RGB(0x33, 0xCC, 0x99), RGB(0x66, 0xCC, 0x66), RGB(0x99, 0xCC, 0x99),
	RGB(0xCC, 0xCC, 0x99), RGB(0xFF, 0xCC, 0x99), RGB(0x00, 0xFF, 0x99),
	RGB(0x33, 0xFF, 0x99), RGB(0x66, 0xCC, 0x99), RGB(0x99, 0xFF, 0x99),
	RGB(0xCC, 0xFF, 0x99), RGB(0xFF, 0xFF, 0x99), RGB(0x00, 0x00, 0xCC),
	RGB(0x33, 0x00, 0x99), RGB(0x66, 0x00, 0xCC), RGB(0x99, 0x00, 0xCC),
	RGB(0xCC, 0x00, 0xCC), RGB(0x00, 0x33, 0x99), RGB(0x33, 0x33, 0xCC),
	RGB(0x66, 0x33, 0xCC), RGB(0x99, 0x33, 0xCC), RGB(0xCC, 0x33, 0xCC),
	RGB(0xFF, 0x33, 0xCC), RGB(0x00, 0x66, 0xCC), RGB(0x33, 0x66, 0xCC),
	RGB(0x66, 0x66, 0x99), RGB(0x99, 0x66, 0xCC), RGB(0xCC, 0x66, 0xCC),
	RGB(0xFF, 0x66, 0x99), RGB(0x00, 0x99, 0xCC), RGB(0x33, 0x99, 0xCC),
	RGB(0x66, 0x99, 0xCC), RGB(0x99, 0x99, 0xCC), RGB(0xCC, 0x99, 0xCC),
	RGB(0xFF, 0x99, 0xCC), RGB(0x00, 0xCC, 0xCC), RGB(0x33, 0xCC, 0xCC),
	RGB(0x66, 0xCC, 0xCC), RGB(0x99, 0xCC, 0xCC), RGB(0xCC, 0xCC, 0xCC),
	RGB(0xFF, 0xCC, 0xCC), RGB(0x00, 0xFF, 0xCC), RGB(0x33, 0xFF, 0xCC),
	RGB(0x66, 0xFF, 0x99), RGB(0x99, 0xFF, 0xCC), RGB(0xCC, 0xFF, 0xCC),
	RGB(0xFF, 0xFF, 0xCC), RGB(0x33, 0x00, 0xCC), RGB(0x66, 0x00, 0xFF),
	RGB(0x99, 0x00, 0xFF), RGB(0x00, 0x33, 0xCC), RGB(0x33, 0x33, 0xFF),
	RGB(0x66, 0x33, 0xFF), RGB(0x99, 0x33, 0xFF), RGB(0xCC, 0x33, 0xFF),
	RGB(0xFF, 0x33, 0xFF), RGB(0x00, 0x66, 0xFF), RGB(0x33, 0x66, 0xFF),
	RGB(0x66, 0x66, 0xCC), RGB(0x99, 0x66, 0xFF), RGB(0xCC, 0x66, 0xFF),
	RGB(0xFF, 0x66, 0xCC), RGB(0x00, 0x99, 0xFF), RGB(0x33, 0x99, 0xFF),
	RGB(0x66, 0x99, 0xFF), RGB(0x99, 0x99, 0xFF), RGB(0xCC, 0x99, 0xFF),
	RGB(0xFF, 0x99, 0xFF), RGB(0x00, 0xCC, 0xFF), RGB(0x33, 0xCC, 0xFF),
	RGB(0x66, 0xCC, 0xFF), RGB(0x99, 0xCC, 0xFF), RGB(0xCC, 0xCC, 0xFF),
	RGB(0xFF, 0xCC, 0xFF), RGB(0x33, 0xFF, 0xFF), RGB(0x66, 0xFF, 0xCC),
	RGB(0x99, 0xFF, 0xFF), RGB(0xCC, 0xFF, 0xFF), RGB(0xFF, 0x66, 0x66),
	RGB(0x66, 0xFF, 0x66), RGB(0xFF, 0xFF, 0x66), RGB(0x66, 0x66, 0xFF),
	RGB(0xFF, 0x66, 0xFF), RGB(0x66, 0xFF, 0xFF), RGB(0xA5, 0x00, 0x21),
	RGB(0x5F, 0x5F, 0x5F), RGB(0x77, 0x77, 0x77), RGB(0x86, 0x86, 0x86),
	RGB(0x96, 0x96, 0x96), RGB(0xCB, 0xCB, 0xCB), RGB(0xB2, 0xB2, 0xB2),
	RGB(0xD7, 0xD7, 0xD7), RGB(0xDD, 0xDD, 0xDD), RGB(0xE3, 0xE3, 0xE3),
	RGB(0xEA, 0xEA, 0xEA), RGB(0xF1, 0xF1, 0xF1), RGB(0xF8, 0xF8, 0xF8),
	RGB(0xFF, 0xFB, 0xF0), RGB(0xA0, 0xA0, 0xA4), RGB(0x80, 0x80, 0x80),
	RGB(0xFF, 0x00, 0x00), RGB(0x00, 0xFF, 0x00), RGB(0xFF, 0xFF, 0x00),
	RGB(0x00, 0x00, 0xFF), RGB(0xFF, 0x00, 0xFF), RGB(0x00, 0xFF, 0xFF),
	RGB(0xFF, 0xFF, 0xFF)
};

extern "C"
HPALETTE WINAPI imgCreatePalette(HDIB hdib)
{
	if (hdib == NULL)
		return NULL;

	HPALETTE hpal = NULL;
	LPBITMAPINFO pbi = dib_lockinfo(hdib);
	int colors = dib_numcolors(pbi);
	if (colors != 0) {
		LOGPALETTE* ppal = (LOGPALETTE*)malloc(sizeof(LOGPALETTE) + sizeof(PALETTEENTRY) * colors);
		if (!ppal) {
			dib_unlock(hdib);
			return NULL;
		}

		ppal->palVersion = PALVERSION;
		ppal->palNumEntries = (uint16_t)colors;

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
	dib_unlock(hdib);
	return hpal;
}

extern "C"
BOOL WINAPI imgCreateBitmapFromDIB(HDIB hdib, HBITMAP* phbm, HPALETTE* phpal)
{
	if (!hdib || !phbm)
		return FALSE;

	*phbm = NULL;
	if (phpal)
		*phpal = NULL;

	LPBITMAPINFOHEADER pbh = dib_lockheader(hdib);
	LPBYTE pBits = dib_findbits(pbh);
	HPALETTE hpal = imgCreatePalette(hdib);

	HDC hdc = GetDC(NULL);
	HPALETTE hpalw;
	if (hpal)
		hpalw = SelectPalette(hdc, hpal, FALSE);
	RealizePalette(hdc);
	*phbm = CreateDIBitmap(hdc, pbh, CBM_INIT, pBits, (BITMAPINFO*)pbh, DIB_RGB_COLORS);
	if (*phbm == NULL) {
		// Windows Meでは16M以上のビットマップはCreateDIBitmapでは作成できない
		void* pv;
		*phbm = CreateDIBSection(hdc, (BITMAPINFO*)pbh, DIB_RGB_COLORS, &pv, NULL, 0);
		if (*phbm)
			SetDIBits(hdc, *phbm, 0, pbh->biHeight, pBits, (BITMAPINFO*)pbh, DIB_RGB_COLORS);
	}
	if (hpal)
		SelectPalette(hdc, hpalw, FALSE);

	if (phpal)
		*phpal = hpal;
	else
		DeletePalette(hpal);

	dib_unlock(hdib);
	ReleaseDC(NULL, hdc);
	return TRUE;
}

extern "C"
HDIB WINAPI imgCreateDIBFromBitmap(HBITMAP hbm, HPALETTE hpal)
{
	return dib_createfrombitmap(hbm, hpal);
}

extern "C"
BOOL WINAPI imgGetDIBInfo(HDIB hdib, int* dx, int* dy, int* colors, int* bits)
{
	return dib_getinfo(hdib, dx, dy, colors, bits);
}

extern "C"
BOOL WINAPI imgRetrieveHistgram(HDIB hdib, int* pred, int* pgreen, int* pblue)
{
	// 旧方式
	mif image;
	if (!image.attachdib(hdib)) return FALSE;

	if (pred)	memset(pred, 0, 256 * sizeof(int));
	if (pgreen)	memset(pgreen, 0, 256 * sizeof(int));
	if (pblue)	memset(pblue, 0, 256 * sizeof(int));

	register RGBA* p = image.m_pixels;
	for (int i = 0; i < image.m_packets; i++) {
		if (pred)	pred[p->red]++;
		if (pgreen)	pgreen[p->green]++;
		if (pblue)	pblue[p->blue]++;
		p++;
	}
	return TRUE;
}

extern "C"
BOOL WINAPI imgCalculateHistogram(HDIB hdib, HistogramArray* ha)
{
	// 新方式
	mif image;
	if (!ha || !image.attachdib(hdib)) return FALSE;

	memset(ha, 0, sizeof(HistogramArray));
	register RGBA* p = image.m_pixels;
	for (int i = 0; i < image.m_packets; i++) {
		ha->red[p->red]++;
		ha->green[p->green]++;
		ha->blue[p->blue]++;

		int vmax = (p->red > p->green) ? p->red : p->green;
		if (p->blue > vmax)
			ha->value[p->blue]++;
		else
			ha->value[vmax]++;
		p++;
	}
	return TRUE;
}


extern "C"
HDIB WINAPI imgReplacePalette(HDIB hsrc, HPALETTE hpal)
{
	if (!hsrc || !hpal)
		return NULL;

	HDIB hdst = dib_copy(hsrc);
	LPBITMAPINFO pbi = dib_lockinfo(hdst);
	uint16_t src_colors = (uint16_t)dib_numcolors(pbi);
	uint16_t dst_colors;
	GetObject(hpal, sizeof(uint16_t), &dst_colors);

	if (!src_colors || src_colors != dst_colors) {
		dib_unlock(hdst);
		dib_destroy(hdst);
		return NULL;
	}

	PALETTEENTRY* ppe = new PALETTEENTRY[dst_colors];
	GetPaletteEntries(hpal, 0, dst_colors, ppe);
	for (int i = 0; i < (int)dst_colors; i++) {
		pbi->bmiColors[i].rgbRed = ppe[i].peRed;
		pbi->bmiColors[i].rgbGreen = ppe[i].peGreen;
		pbi->bmiColors[i].rgbBlue = ppe[i].peBlue;
	}
	delete [] ppe;

	dib_unlock(hdst);
	return hdst;
}

extern "C"
void WINAPI imgPaintDIB(HDC hdc, LPRECT prcDst, HDIB hdib, LPRECT prcSrc, HPALETTE hpal)
{
	if (!hdib)
		return;

	BITMAPINFO* pbi  = dib_lockinfo(hdib);
	int y = pbi->bmiHeader.biHeight;
	BYTE* pbits = dib_findbits(pbi);

	CRect rcs = *prcSrc;
	CRect rcd = *prcDst;

	HPALETTE hpalOld = NULL;
	if (hpal)
		hpalOld = SelectPalette(hdc, hpal, TRUE);

	SetStretchBltMode(hdc, COLORONCOLOR);
	if (rcs.Width() == rcd.Width() && rcs.Height() == rcd.Height())
		SetDIBitsToDevice(hdc, rcd.left, rcd.top, rcd.Width(), rcd.Height(),
							rcs.left, y - rcs.top - rcs.Height(), 0, y,
							pbits, pbi, DIB_RGB_COLORS);
	else
		StretchDIBits(hdc, rcd.left, rcd.top, rcd.Width(), rcd.Height(),
							rcs.left, rcs.top, rcs.Width(), rcs.Height(),
							pbits, pbi, DIB_RGB_COLORS, SRCCOPY);
	dib_unlock(hdib);
	if (hpalOld)
		SelectPalette(hdc, hpalOld, TRUE);
}

extern "C"
void WINAPI imgPaintBitmap(HDC hdc, LPRECT prcDst, HBITMAP hbm, LPRECT prcSrc, HPALETTE hpal)
{
	if (!hbm)
		return;

	HDC hdcw = CreateCompatibleDC(hdc);
	if (!hdcw)
		return;

	HPALETTE hpal1 = NULL, hpal2 = NULL;
	if (hpal) {
		hpal1 = SelectPalette(hdcw, hpal, TRUE);
		hpal2 = SelectPalette(hdc, hpal, TRUE);
		RealizePalette(hdc);
	}

	HBITMAP hbmOld = SelectBitmap(hdcw, hbm);
	CRect rcs = *prcSrc;
	CRect rcd = *prcDst;

	if (rcs.Width() == rcd.Width() && rcs.Height() == rcd.Height())
		BitBlt(hdc, rcd.left, rcd.top, rcd.Width(), rcd.Height(),
					hdcw, rcs.left, rcs.top, SRCCOPY);
	else {
		SetStretchBltMode(hdc, COLORONCOLOR);
		StretchBlt(hdc, rcd.left, rcd.top, rcd.Width(), rcd.Height(),
					hdcw, rcs.left, rcs.top, rcs.Width(), rcs.Height(), SRCCOPY);
	}
	SelectObject(hdcw, hbmOld);
	if (hpal1)
		SelectPalette(hdcw, hpal1, FALSE);
	if (hpal2)
		SelectPalette(hdc, hpal2, FALSE);

	DeleteDC(hdcw);
}

extern "C"
void WINAPI imgTransparentBlt(HDC hdc, HBITMAP hbm, int x, int y, COLORREF color)
{
	BITMAP bm;
	GetObject(hbm, sizeof(bm), &bm);
	int nWidth = bm.bmWidth;
	int nHeight = bm.bmHeight;

	COLORREF crOldBack = SetBkColor(hdc, RGB(255, 255, 255));
	COLORREF crOldText = SetTextColor(hdc, RGB(0, 0, 0));

	// Create two memory dcs for the image and the mask
	HDC hdcImage = CreateCompatibleDC(hdc);
	HDC hdcTrans = CreateCompatibleDC(hdc);

	// Select the image into the appropriate dc
	HBITMAP hOldBitmapImage = SelectBitmap(hdcImage, hbm);

	// Create the mask bitmap
	HBITMAP hbitmapTrans = CreateBitmap(nWidth, nHeight, 1, 1, NULL);

	// Select the mask bitmap into the appropriate dc
	HBITMAP hOldBitmapTrans = SelectBitmap(hdcTrans, hbitmapTrans);

	// Build mask based on transparent colour
	SetBkColor(hdcImage, color);
	BitBlt(hdcTrans, 0, 0, nWidth, nHeight, hdcImage, 0, 0, SRCCOPY);

	// Do the work - True Mask method - cool if not actual display
	BitBlt(hdc, x, y, nWidth, nHeight, hdcImage, 0, 0, SRCINVERT);
	BitBlt(hdc, x, y, nWidth, nHeight, hdcTrans, 0, 0, SRCAND);
	BitBlt(hdc, x, y, nWidth, nHeight, hdcImage, 0, 0, SRCINVERT);

	// Restore settings
	SelectObject(hdcImage, hOldBitmapImage);
	SelectObject(hdcTrans, hOldBitmapTrans);
	SetBkColor(hdc, crOldBack);
	SetTextColor(hdc, crOldText);

	DeleteBitmap(hbitmapTrans);
	DeleteDC(hdcImage);
	DeleteDC(hdcTrans);
}

extern "C"
void WINAPI imgDrawBitmap(HDC hdc, int x, int y, HBITMAP hbm, HPALETTE hpal)
{
	if (!hbm)
		return;

	BITMAP bm;
	GetObject(hbm, sizeof(BITMAP), &bm);
	HDC hdcw = CreateCompatibleDC(hdc);

	HPALETTE hpal1 = NULL, hpal2 = NULL;
	if (hpal)
    {
		hpal1 = SelectPalette(hdcw, hpal, TRUE);
		hpal2 = SelectPalette(hdc, hpal, TRUE);
		RealizePalette(hdc);
	}

	HBITMAP hbmOld = SelectBitmap(hdcw, hbm);
	BitBlt(hdc, x, y, bm.bmWidth, bm.bmHeight, hdcw, 0, 0, SRCCOPY);
	SelectObject(hdcw, hbmOld);

	if (hpal1)
		SelectPalette(hdcw, hpal2, FALSE);
	if (hpal2)
		SelectPalette(hdc, hpal2, FALSE);

	DeleteDC(hdcw);
}

HANDLE allocdibspace(BITMAPINFOHEADER bi, HBITMAP hbm)
{
	DWORD len = bi.biSize + dib_palsize(&bi);
    HANDLE hdib = GlobalAlloc(GHND, len);
	if (!hdib)
		return NULL;

	BITMAPINFOHEADER* pbh = dib_lockheader(hdib);
	*pbh = bi;

	HDC hdc = GetDC(NULL);
	GetDIBits(hdc, hbm, 0, (UINT)bi.biHeight, NULL, (BITMAPINFO*)pbh, DIB_RGB_COLORS);
	ReleaseDC(NULL, hdc);

    // If the driver did not fill in the biSizeImage field,
    // fill it in -- NOTE: this is a bug in the driver!
	if (pbh->biSizeImage == 0)
        pbh->biSizeImage = WIDTHBYTES((DWORD)pbh->biWidth * pbh->biBitCount) * pbh->biHeight;

	// Get the size of the memory block we need
	len = pbh->biSize + dib_palsize(&bi) + pbh->biSizeImage;
	dib_unlock(hdib);

	// ReAlloc the buffer big enough to hold all the bits 
	HANDLE h = GlobalReAlloc(hdib, len, 0);
	if (h)
		return h;

	dib_destroy(hdib);
	return NULL;
}

extern "C"
HDIB WINAPI imgChangeDIBFormat(HDIB hdib, int bits, DWORD compression)
{
    HDC                hdc;             // Handle to DC
    HBITMAP            hbm;         // Handle to bitmap
    BITMAP             bm;				// BITMAP data structure
    BITMAPINFOHEADER   bi;              // Bitmap info header
    LPBITMAPINFOHEADER pbh;            // Pointer to bitmap info
    HDIB               hdibNew = NULL;  // Handle to new DIB
    HPALETTE           hPal, hpalOld;   // Handle to palette, prev pal
    WORD               bitsDIB, bitsNew;	// DIB bits per pixel, new bpp
    DWORD              compNew;				// new compression

    if (!hdib)
        return NULL;

    // Get the old DIB's bits per pixel and compression format
	pbh = dib_lockheader(hdib);
	bitsDIB = pbh->biBitCount;
	dib_unlock(hdib);

	// Validate bits and compression
	// They must match correctly (i.e., BI_RLE4 and 4 BPP or
	// BI_RLE8 and 8BPP, etc.) or we return failure
	if (bits == 0) {
		bitsNew = bitsDIB;
		if ((compression == BI_RLE4 && bitsNew == 4) ||
			(compression == BI_RLE8 && bitsNew == 8) ||
			(compression == BI_RGB))
			compNew = compression;
		else
			return NULL;
	}
	else if (bits == 1 && compression == BI_RGB) {
		bitsNew = (uint16_t)bits;
		compNew = BI_RGB;
	}
	else if (bits == 4) {
		bitsNew = (uint16_t)bits;
		if (compression == BI_RGB || compression == BI_RLE4)
			compNew = compression;
		else
			return NULL;
	}
	else if (bits == 8) {
		bitsNew = (uint16_t)bits;
		if (compression == BI_RGB || compression == BI_RLE8)
			compNew = compression;
		else
			return NULL;
	}
	else if (bits == 24 && compression == BI_RGB) {
		bitsNew = (uint16_t)bits;
		compNew = BI_RGB;
	}
	else
		return NULL;

	hPal = NULL;
	hbm = NULL;
	if (!imgCreateBitmapFromDIB(hdib, &hbm, &hPal)) {
		if (hPal)
			DeleteObject(hPal);
		if (hbm)
			DeleteObject(hbm);
		return NULL;
	}

	GetObject(hbm, sizeof(BITMAP), (LPSTR)&bm);

	// Fill in the BITMAPINFOHEADER appropriately
	bi.biSize               = sizeof(BITMAPINFOHEADER);
	bi.biWidth              = bm.bmWidth;
	bi.biHeight             = bm.bmHeight;
	bi.biPlanes             = 1;
	bi.biBitCount           = bitsNew;
	bi.biCompression        = compNew;
	bi.biSizeImage          = 0;
	bi.biXPelsPerMeter      = 0;
	bi.biYPelsPerMeter      = 0;
	bi.biClrUsed            = 0;
	bi.biClrImportant       = 0;

	// Go allocate room for the new DIB
	hdibNew = allocdibspace(bi, hbm);
	if (!hdibNew)
		return NULL;

	// Get a pointer to the new DIB
	pbh = dib_lockheader(hdibNew);

	// Get a DC and select/realize our palette in it
	hdc  = GetDC(NULL);
	hpalOld = SelectPalette(hdc, hPal, FALSE);
	RealizePalette(hdc);

	// Call GetDIBits and get the new DIB bits
	if (!GetDIBits(hdc, hbm, 0, (UINT) pbh->biHeight,
			(LPSTR)pbh + (WORD)pbh->biSize + dib_palsize(pbh),
			(BITMAPINFO*)pbh, DIB_RGB_COLORS))
    {
		dib_unlock(hdibNew);
		dib_destroy(hdibNew);
    }

    // Clean up and return
	SelectPalette(hdc, hpalOld, TRUE);
	RealizePalette(hdc);
	ReleaseDC(NULL, hdc);

	// Unlock the new DIB's memory block
	if (hdibNew)
		dib_unlock(hdibNew);

	DeleteObject(hbm);
	DeleteObject(hPal);

	return hdibNew;
}

extern "C"
HDIB WINAPI imgChangeBitmapFormat(HBITMAP hbm, WORD bits, DWORD compression, HPALETTE hPal)
{
    HDC                hdc;          // Screen DC
    HDIB               hdibNew=NULL; // Handle to new DIB
    BITMAP             bm;       // BITMAP data structure
    BITMAPINFOHEADER   bi;           // Bitmap info. header
    LPBITMAPINFOHEADER pbh;         // Pointer to bitmap header
    HPALETTE           hpalOld=NULL; // Handle to palette
    int                bitsNew;       // New bits per pixel
    DWORD              compNew;      // New compression format

    if (!hbm)
        return NULL;

    // Validate bits and compression
    // They must match correctly (i.e., BI_RLE4 and 4 BPP or
    // BI_RLE8 and 8BPP, etc.) or we return failure
    if (bits == 0)
    {
		compNew = compression;
		if (compNew == BI_RLE4)
			bitsNew = 4;
        else if (compNew == BI_RLE8)
			bitsNew = 8;
		else
			return NULL;
    }
	else if (bits == 1 && compression == BI_RGB)
	{
		bitsNew = bits;
		compNew = BI_RGB;
    }
    else if (bits == 4)
    {
        bitsNew = bits;
        if (compression == BI_RGB || compression == BI_RLE4)
            compNew = compression;
        else
            return NULL;
    }
    else if (bits == 8)
    {
        bitsNew = bits;
        if (compression == BI_RGB || compression == BI_RLE8)
            compNew = compression;
        else
            return NULL;
    }
    else if (bits == 24 && compression == BI_RGB)
    {
        bitsNew = bits;
        compNew = BI_RGB;
    }
    else
        return NULL;

    GetObject(hbm, sizeof(BITMAP), (LPSTR)&bm);
	memset(&bi, 0, sizeof(bi));
    bi.biSize               = sizeof(BITMAPINFOHEADER);
    bi.biWidth              = bm.bmWidth;
    bi.biHeight             = bm.bmHeight;
    bi.biPlanes             = 1;
    bi.biBitCount           = (uint16_t)bitsNew;
    bi.biCompression        = compNew;

	hdibNew = allocdibspace(bi, hbm);
    if (!hdibNew)
        return NULL;

	pbh = dib_lockheader(hdibNew);
	if (hPal) {
		hdc  = GetDC(NULL);
		hpalOld = SelectPalette(hdc, hPal, FALSE);
		RealizePalette(hdc);
	}

    // Call GetDIBits and get the new DIB bits
	if (!GetDIBits(hdc, hbm, 0, (UINT) pbh->biHeight,
				(LPSTR)pbh + (WORD)pbh->biSize + dib_palsize((LPSTR)pbh),
				(BITMAPINFO*)pbh, DIB_RGB_COLORS))
	{
		dib_unlock(hdibNew);
		dib_destroy(hdibNew);
	}

	if (hpalOld) {
		SelectPalette(hdc, hpalOld, TRUE);
		RealizePalette(hdc);
		ReleaseDC(NULL, hdc);
	}

	if (hdibNew)
		dib_unlock(hdibNew);
	return hdibNew;
}

int palentriesondevice(HDC hdc)
{
	int colors = GetDeviceCaps(hdc, SIZEPALETTE);
	if (!colors)
		colors = GetDeviceCaps(hdc, NUMCOLORS);
	return colors;
}

extern "C"
HPALETTE WINAPI imgGetSystemPalette()
{
	HPALETTE hpal = NULL;
	HDC hdc = GetDC(NULL);
	if (GetDeviceCaps(hdc, RASTERCAPS) & RC_PALETTE) {
		int col = palentriesondevice(hdc);
		if (col > 0) {
			LOGPALETTE* plp = (LOGPALETTE*)malloc(sizeof(LOGPALETTE) + col * sizeof(PALETTEENTRY));
			if (plp) {
				plp->palVersion = PALVERSION;
				plp->palNumEntries = (uint16_t)col;
				GetSystemPaletteEntries(hdc, 0, col, (PALETTEENTRY*)(plp->palPalEntry));
				hpal = CreatePalette(plp);
				free(plp);
			}
		}
	}
	ReleaseDC(NULL, hdc);
    return hpal;
}

extern "C"
HBITMAP WINAPI imgCopyBitmap(HBITMAP hbm)
{
	BITMAP bm;
	GetObject(hbm, sizeof(BITMAP), &bm);
	return CreateBitmap(bm.bmWidth, bm.bmHeight, bm.bmPlanes, bm.bmBitsPixel, bm.bmBits);
}

extern "C"
HPALETTE WINAPI imgCopyPalette(HPALETTE hpalSrc)
{
	if (!hpalSrc)
		return NULL;

	WORD cnt;
	GetObject(hpalSrc, sizeof(WORD), &cnt);
	LOGPALETTE* plp = (LOGPALETTE*)malloc(sizeof(LOGPALETTE) + sizeof(PALETTEENTRY) * cnt);
	HPALETTE hpalDst = NULL;
	if (plp) {
		plp->palVersion = PALVERSION;
		plp->palNumEntries = cnt;
		PALETTEENTRY* ppe = (PALETTEENTRY*)((BYTE*)plp + sizeof(LOGPALETTE));
		GetPaletteEntries(hpalSrc, 0, cnt, ppe);
		hpalDst = CreatePalette(plp);
		free(plp);
	}
	return hpalDst;
}

extern "C"
HDIB WINAPI imgCreateSolid(DWORD dwWidth, DWORD dwHeight, COLORREF rgb)
{
	return dib_createsolid(dwWidth, dwHeight, rgb);
}

BOOL __fastcall GetFileData(LPCTSTR pszName, WIN32_FIND_DATA* pwfd)
{
	HANDLE hFind = FindFirstFile(pszName, pwfd);
	if (hFind != INVALID_HANDLE_VALUE) {
		FindClose(hFind);

		FILETIME ft;

		// ローカル時間に変換
		FileTimeToLocalFileTime(&pwfd->ftCreationTime, &ft);
		pwfd->ftCreationTime = ft;
		FileTimeToLocalFileTime(&pwfd->ftLastWriteTime, &ft);
		pwfd->ftLastWriteTime = ft;

		return TRUE;
	}
	return FALSE;
}

static TCHAR szTypeBuffer[] = _T("BMP DIB RLE JPG JPEG JPE GIF TIF TIFF TGA PCX MAG MKI RAS PNG PSD XJG CAM SGI RGB WMF EMF PNM PPM PGM ICO TXR XPM XBM FPX JP2 WEBP AVIF");

extern "C"
void WINAPI imgGetReadableTypes(LPTSTR pTypes)
{
	if (pTypes)
		lstrcpy(pTypes, szTypeBuffer);
}

static inline uint32_t getBE32(const uint8_t* data) {
	return ((uint32_t)data[0] << 24) | ((uint32_t)data[1] << 16) | ((uint32_t)data[2] << 8) | (uint32_t)data[3];
}

static int check_image_type(HANDLE hf)
{
	int type = IMG_NOTSUPPORTED;

	BYTE cb[32];
	DWORD dw;
	if (ReadFile(hf, cb, 16, &dw, NULL) && dw == 16) {
		const uint32_t magic1 = getBE32(cb);
		const uint32_t magic2 = getBE32(cb + 8);
		const uint32_t magic3 = getBE32(cb + 4);

		if (magic1 == 0x89504E47U)
			type = IMG_PNG;
		else if (magic1 >= 0xFFD8FF00 && magic1 <= 0xFFD8FFFF)
			type = IMG_JPEG;
		else if (magic1 == 0x49492A00 || magic1 == 0x4D4D002A)
			type = IMG_TIFF;
		else if (magic1 == 0x52494646 && magic2 == 0x57454250)
			type = IMG_WEBP;
		else if (magic3 == 0x66747970 && magic2 == 0x61766966)
			type = IMG_AVIF;
		else if (((BITMAPFILEHEADER*)cb)->bfType == 0x4D42 &&
				 ((BITMAPFILEHEADER*)cb)->bfSize != 0 &&
				 ((BITMAPFILEHEADER*)cb)->bfReserved1 == 0 &&
				 ((BITMAPFILEHEADER*)cb)->bfReserved2 == 0)
			type = IMG_BMP;							// BMP
		else if (*((WORD*)cb) == XJGMAGIC)
			type = IMG_XJG;							// XJG
		else if (magic1 == 0x47494638 && (cb[4] == '7' || cb[4] == '9'))
			type = IMG_GIF;							// GIF
		else if (magic1 == 0x38425053)
			type = IMG_PSD;							// PSD
		else if (magic1 == 0x59A66A95)
			type = IMG_RASTER;						// RASTER
		else if (magic1 == 0x4D414B49 && cb[4] == '0') {
			if (cb[5] == '1' && (cb[6] == 'A' || cb[6] == 'B'))
				type = IMG_MAKI;					// MAKI
			else if (cb[5] == '2' && cb[6] == ' ' && cb[7] == ' ')
				type = IMG_MAG;						// MAG
		}
		else if (cb[0] == 0x0A &&
				 (cb[1] == 0 || cb[1] == 2 || cb[1] == 3 ||
				  cb[1] == 4 || cb[1] == 5) &&
				 (cb[2] == 0 || cb[2] == 1) &&
				 (cb[3] == 1 || cb[3] == 2 || cb[3] == 4 || cb[3] == 8))
				type = IMG_PCX;						// PCX
		else if (magic1 == 0x07204D4D)
			type = IMG_CAM;							// CAM
		else if (*((WORD*)cb) == 0xda01)
			type = IMG_SGI;							// SGI
		else if (*((DWORD*)cb) == TEXTURE_MAGIC)
			type = IMG_TXR;							// TXR
	}
	return type;
}

extern "C"
int WINAPI imgCheckImageType(LPCTSTR pszFile)
{
	int type = IMG_NOTSUPPORTED;
	if (pszFile && *pszFile) {
		HANDLE hf = CreateFile(pszFile, GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, 0, NULL);
		if (hf != INVALID_HANDLE_VALUE) {
			type = check_image_type(hf);
			CloseHandle(hf);
		}
	}
	// 上記に該当しないもの
	if (type == IMG_NOTSUPPORTED)
		type = imgSpecifyImageType(pszFile);
	return type;
}

#include "imgspecify.cpp"

int __fastcall mixcolor(int src, int dst, int dst_opaque)
{
	return ADJRGB((src * (100 - dst_opaque) + dst * dst_opaque) / 100);
}

void msb1stwritelong(HANDLE hf, ULONG value)
{
	BYTE buf[4];
	DWORD dw;
	buf[0] = (BYTE)((value >> 24) & 0xFF);
	buf[1] = (BYTE)((value >> 16) & 0xFF);
	buf[2] = (BYTE)((value >>  8) & 0xFF);
	buf[3] = (BYTE)( value        & 0xFF);
	WriteFile(hf, buf, 4, &dw, NULL);
}

void msb1stwriteshort(HANDLE hf, UINT value)
{
	BYTE buf[2];
	DWORD dw;

	buf[0] = (BYTE)((value >> 8) & 0xFF);
	buf[1] = (BYTE)( value       & 0xFF);
	WriteFile(hf, buf, 2, &dw, NULL);
}

void __cdecl MsgBox(LPCTSTR lpszFormat, ...)
{
	TCHAR* pOutput = new TCHAR[1024];
	va_list argList;
	va_start(argList, lpszFormat);
	_vstprintf(pOutput, lpszFormat, argList);
	va_end(argList);
	MessageBox(GetFocus(), pOutput, _T("attn"), MB_OK);
	delete [] pOutput;
}

void DumpBox(LPBYTE pDump, int len)
{
	TCHAR* pOutput = new TCHAR[len * 4];
	*pOutput = '\0';
	TCHAR* p= pOutput;
	for (int i = 0; i < len; i++) {
		if (i && (i % 16) == 0)
			p += wsprintf(p, _T("\n"));
		p += wsprintf(p, _T("%02X "), *pDump++);
	}
	MessageBox(GetFocus(), pOutput, _T("attn"), MB_OK);
	delete [] pOutput;
}

extern "C"
HBITMAP WINAPI imgTiledBitmap(HDC hdc, HBITMAP hbm, HPALETTE hpal, int cx, int cy)
{
	HDC hdc1 = CreateCompatibleDC(hdc);
	HDC hdc2 = CreateCompatibleDC(hdc);
	HPALETTE hpal1 = NULL;
	HPALETTE hpal2 = NULL;

	if (hpal) {
		hpal1 = SelectPalette(hdc1, hpal, FALSE);
		hpal2 = SelectPalette(hdc2, hpal, FALSE);
		RealizePalette(hdc1);
		RealizePalette(hdc2);
	}

	HBITMAP hbm1 = SelectBitmap(hdc1, hbm);
	HBITMAP hbmTile = CreateCompatibleBitmap(hdc1, cx, cy);
	HBITMAP hbm2 = SelectBitmap(hdc2, hbmTile);

	BITMAP bm;
	GetObject(hbm, sizeof(BITMAP), (LPSTR)&bm);
	int dx = cx / bm.bmWidth;
	if (cx % bm.bmWidth)
		dx++;
	int dy = cy / bm.bmHeight;
	if (cy % bm.bmHeight)
		dy++;

	CRect rc;
	for (int y = 0; y < dy; y++) {
		rc.SetRect(0, y * bm.bmHeight, dx, (y + 1) * bm.bmHeight);
		for (int x = 0; x < dx; x++) {
			BitBlt(hdc2, rc.left, rc.top, bm.bmWidth, bm.bmHeight, hdc1, 0, 0, SRCCOPY);
			rc.OffsetRect(bm.bmWidth, 0);
		}
	}
	SelectObject(hdc1, hbm1);
	SelectObject(hdc2, hbm2);

	if (hpal) {
		SelectPalette(hdc1, hpal1, FALSE);
		SelectPalette(hdc2, hpal2, FALSE);
	}
	DeleteDC(hdc1);
	DeleteDC(hdc2);

	return hbmTile;
}

extern "C"
HBITMAP WINAPI imgLoadBitmapFromResource(HINSTANCE hInst, LPCTSTR pszResource, HPALETTE* phPal)
{
	HDIB hdib;
	if (phPal)
		*phPal = NULL;

	hdib = imgLoadDIBFromResource(hInst, pszResource);
	if (hdib) {
		HBITMAP hbm;
		HPALETTE hpal;
		imgCreateBitmapFromDIB(hdib, &hbm, &hpal);
		dib_destroy(hdib);

		if (phPal)
			*phPal = hpal;
		else
			DeleteObject(hpal);
		return hbm;
	}
	return NULL;
}

extern "C"
HDIB WINAPI imgLoadDIBFromResource(HINSTANCE hInst, LPCTSTR pszResource)
{
	HRSRC hr = FindResource(hInst, pszResource, RT_BITMAP);
	if (hr == NULL) return NULL;

	HGLOBAL hglb = LoadResource(hInst, hr);
	if (hglb == NULL) return NULL;

	BITMAPINFO* pbiSrc = (BITMAPINFO*)LockResource(hglb);
	if (pbiSrc == NULL) return NULL;

	BITMAPINFOHEADER* pbhSrc = (BITMAPINFOHEADER*)pbiSrc;
	HDIB hdib = dib_create(pbhSrc->biWidth, pbhSrc->biHeight, pbhSrc->biBitCount);
	if (hdib == NULL) return NULL;

	BITMAPINFOHEADER* pbhDst = dib_lockheader(hdib);
	BITMAPINFO* pbiDst = (BITMAPINFO*)pbhDst;

	// ヘッダコピー
	memcpy(pbhSrc, pbhDst, sizeof(BITMAPINFOHEADER));
	pbhDst->biClrUsed = pbhDst->biClrImportant = dib_numcolors(pbiSrc);

	// パレットのコピー
	memcpy(pbiDst->bmiColors, pbiSrc->bmiColors, sizeof(RGBQUAD) * pbhDst->biClrUsed);

	// ビットマップデータのコピー
	DWORD dwSize;
	BYTE* pSrc = dib_findbits(pbiSrc);
	BYTE* pDst = dib_findbits(pbiDst);
	if (pbhSrc->biSizeImage)
		dwSize = pbhSrc->biSizeImage;
	else
		dwSize = WIDTHBYTES(pbhSrc->biBitCount * pbhSrc->biWidth) * pbhSrc->biHeight;
	memcpy(pDst, pSrc, dwSize);
	dib_unlock(hdib);
	return hdib;
}

#define PAL_MAGIC			0x214c4150

typedef struct tagPALHEADER {
	DWORD magic;
	DWORD entries;
} PALHEADER;

extern "C"
BOOL WINAPI imgSavePalette(HPALETTE hpal, LPCTSTR pszName)
{
	if (!hpal)
		return FALSE;

	WORD cnt;
	GetObject(hpal, sizeof(WORD), &cnt);
	if (!cnt)
		return FALSE;

	LOGPALETTE* plp = (LOGPALETTE*)malloc(sizeof(LOGPALETTE) + sizeof(PALETTEENTRY) * cnt);
	if (!plp)
		return FALSE;

	HANDLE hf = CreateFile(pszName, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_ARCHIVE, NULL);
	if (hf == INVALID_HANDLE_VALUE) {
		free(plp);
		return FALSE;
	}

	GetPaletteEntries(hpal, 0, cnt, plp->palPalEntry);

	PALHEADER ph;
	ph.magic = PAL_MAGIC;
	ph.entries = cnt;
	DWORD dw;
	WriteFile(hf, &ph, sizeof(ph), &dw, NULL);
	WriteFile(hf, plp->palPalEntry, sizeof(PALETTEENTRY) * cnt, &dw, NULL);
	CloseHandle(hf);
	free(plp);

	return TRUE;
}

extern "C"
HPALETTE WINAPI imgLoadPalette(LPCTSTR pszName)
{
	if (imgCheckImageType(pszName) == IMG_GIF) {
		HDIB h = imgGifLoad(pszName);
		if (h) {
			HPALETTE hp = imgCreatePalette(h);
			GlobalFree(h);
			return hp;
		}
	}

	HANDLE hf = CreateFile(pszName, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, 0, NULL);
	if (hf == INVALID_HANDLE_VALUE)
		return NULL;

	PALHEADER ph;
	DWORD dw;
	if (!ReadFile(hf, &ph, sizeof(ph), &dw, NULL) ||
		dw != sizeof(ph) ||
		ph.magic != PAL_MAGIC)
	{
		CloseHandle(hf);
		return NULL;
	}

	DWORD size = sizeof(PALETTEENTRY) * ph.entries;
	LOGPALETTE* plp = (LOGPALETTE*)malloc(sizeof(LOGPALETTE) + size);
	if (!plp) {
		CloseHandle(hf);
		return NULL;
	}

	if (!ReadFile(hf, plp->palPalEntry, size, &dw, NULL) || dw != size) {
		CloseHandle(hf);
		free(plp);
		return NULL;
	}
	CloseHandle(hf);

	plp->palVersion = 0x300;
	plp->palNumEntries = LOWORD(ph.entries);
	HPALETTE hpal = CreatePalette(plp);
	free(plp);
	return hpal;
}

/* Macro to determine the bytes in a DWORD aligned DIB scanline */
#define BYTESPERLINE(Width, BPP) ((WORD)((((DWORD)(Width) * (DWORD)(BPP) + 31) >> 5)) << 2)

extern "C"
int WINAPI imgStretchDIBitsByBand(HDC hdc, int XDest, int YDest, int nDestWidth, int nDestHeight, int XSrc, int /*YSrc*/, int nSrcWidth, int nSrcHeight, VOID * lpBits, BITMAPINFO * lpBitsInfo, UINT iUsage, DWORD /*dwRop*/)
{
    BITMAPINFOHEADER  bmiTemp;
    float fDestYDelta;
    LPBYTE lpNewBits;
    int i;

    // Check for NULL pointers and return error
    if (lpBits == NULL)
		return 0;
    if (lpBitsInfo == NULL)
		return 0;

	// Get increment value for Y axis of destination
	fDestYDelta = (float)nDestHeight / (float)nSrcHeight;

    // Make backup copy of BITMAPINFOHEADER
    bmiTemp = lpBitsInfo->bmiHeader;

	// Adjust image sizes for one scan line
    lpBitsInfo->bmiHeader.biSizeImage =
                       BYTESPERLINE(lpBitsInfo->bmiHeader.biWidth,
                                    lpBitsInfo->bmiHeader.biBitCount);
    lpBitsInfo->bmiHeader.biHeight = 1;

    // Initialize pointer to the image data
    lpNewBits = (LPBYTE)lpBits;

    // Do the stretching
	for (i = 0; i < nSrcHeight; i++)
	{
		if (!StretchDIBits(hdc,
				XDest, YDest + (int)floor(fDestYDelta * (nSrcHeight - (i+1))),
				nDestWidth, (int)ceil(fDestYDelta),
				XSrc, 0, nSrcWidth, 1,
				lpNewBits, lpBitsInfo, iUsage, SRCCOPY))
			break; // Error!
		else
			// Increment image pointer by one scan line
			lpNewBits += lpBitsInfo->bmiHeader.biSizeImage;
	}

	// Restore BITMAPINFOHEADER
	lpBitsInfo->bmiHeader = bmiTemp;
    return i;
}

extern "C"
HDIB WINAPI imgSetXparentColor(HDIB hsrc, int* indexfrom, int entries, int indexto, COLORREF xcolor)
{
	mif image;
	if (image.attachdib(hsrc)) {
		if (image.m_bits < 24) {
			if (indexfrom && entries > 0) {
				image.m_map[indexto].red = GetRValue(xcolor);
				image.m_map[indexto].green = GetGValue(xcolor);
				image.m_map[indexto].blue = GetBValue(xcolor);

				register RGBA* p;
				for (int i = 0; i < entries; i++) {
					p = image.m_pixels;
					for (int j = 0; j < image.m_packets; j++) {
						if (p->alpha == indexfrom[i])
							p->alpha = (uint8_t)indexto;
						p++;
					}
				}
			}
			return image.createdib();
		}
	}
	return NULL;
}

extern "C"
HDIB WINAPI imgMapIndexColor(HDIB hsrc, int* idxtbl, int entries, int mapidx, COLORREF xcolor)
{
	mif image;
	if (image.attachdib(hsrc)) {
		if (image.m_bits < 24) {
			RGBA cmap[256];
			BYTE remap[256];

			cmap[mapidx].red = GetRValue(xcolor);
			cmap[mapidx].green = GetGValue(xcolor);
			cmap[mapidx].blue = GetBValue(xcolor);

			int i, j;
			int idst = 0;
			for (i = 0; i < image.m_colors; i++) {
				BOOL match = FALSE;
				for (j = 0; j < entries; j++) {
					if (i == idxtbl[j]) {
						match = TRUE;
						break;
					}
				}

				if (match)
					remap[i] = (uint8_t)mapidx;
				else {
					if (idst == mapidx)
						idst++;
					cmap[idst].red = image.m_map[i].red;
					cmap[idst].green = image.m_map[i].green;
					cmap[idst].blue = image.m_map[i].blue;
					remap[i] = (uint8_t)idst++;
				}
			}

			RGBA* ps = image.m_pixels;
			for (i = 0; i < image.m_packets; i++) {
				ps->alpha = remap[ps->alpha];
				ps++;
			}
			memcpy(image.m_map, cmap, image.m_colors * sizeof(RGBA));
			return image.createdib();
		}
	}
	return NULL;
}


BOOL __fastcall SaveMemoryToFile(LPCTSTR pszName, void* pdata, size_t memsize)
{
	BOOL ok = FALSE;
	HANDLE hf = CreateFile(pszName, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_ARCHIVE, NULL);
	if (hf != INVALID_HANDLE_VALUE) {
		DWORD written;
		ok = (WriteFile(hf, pdata, (uint32_t)memsize, &written, NULL) && written == memsize);
		CloseHandle(hf);
		if (!ok) {
			DeleteFile(pszName);
		}
	}
	return ok;
}

BOOL __fastcall SaveMemToFileAndWep(LPCTSTR pszName, HANDLE hmem, DWORD dwSize)
{
	BOOL ok = FALSE;
	if (hmem && dwSize) {
		void* pv = GlobalLock(hmem);
		ok = SaveMemoryToFile(pszName, pv, (size_t)dwSize);
		GlobalUnlock(hmem);
		GlobalFree(hmem);
	}
	return ok;
}

extern "C"
BOOL WINAPI imgCopyImageToClipboard(HWND hwnd, LPCTSTR pszName)
{
	BOOL bOK = FALSE;
	HDIB hdib = imgLoadImage(pszName);
	if (hdib) {
		bOK = imgDibToClipboard(hwnd, hdib);
		if (!bOK)
			GlobalFree(hdib);
	}
	return bOK;
}

RGB_PIXEL* __fastcall RGBAtoRGBPixels(const RGBA* srcp, size_t packets, bool applyAlpha)
{
	RGB_PIXEL* pixs = (RGB_PIXEL*)malloc(packets * sizeof(RGB_PIXEL));
	if (pixs) {
		size_t len = packets;
		BYTE* dstp = (BYTE*)pixs;
		while (len-- > 0) {
			*dstp++ = applyAlpha ? APPLYALPHA(srcp->red, srcp->alpha) : srcp->red;
			*dstp++ = applyAlpha ? APPLYALPHA(srcp->green, srcp->alpha) : srcp->green;
			*dstp++ = applyAlpha ? APPLYALPHA(srcp->blue, srcp->alpha) : srcp->blue;
			srcp++;
		}
	}
	return pixs;
}

BGR_PIXEL* __fastcall RGBAtoBGRPixels(const RGBA* srcp, size_t packets, bool applyAlpha)
{
	BGR_PIXEL* pixs = (BGR_PIXEL*)malloc(packets * sizeof(BGR_PIXEL));
	if (pixs) {
		size_t len = packets;
		BYTE* dstp = (BYTE*)pixs;
		while (len-- > 0) {
			*dstp++ = applyAlpha ? APPLYALPHA(srcp->blue, srcp->alpha) : srcp->blue;
			*dstp++ = applyAlpha ? APPLYALPHA(srcp->green, srcp->alpha) : srcp->green;
			*dstp++ = applyAlpha ? APPLYALPHA(srcp->red, srcp->alpha) : srcp->red;
			srcp++;
		}
	}
	return pixs;
}