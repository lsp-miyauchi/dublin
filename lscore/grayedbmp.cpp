#include "stdafx.h"
#include <imagehlp.h>
#include "sys\stat.h"
#include "lscore.h"
#include "fnc.h"


#define RGB_TO_RGBQUAD(r,g,b)	(RGB(b,g,r))
#define CLR_TO_RGBQUAD(c)		(RGB(GetBValue(c), GetGValue(c), GetRValue(c)))
#define MAXJSAMPLE				256
#define CENTERJSAMPLE			128
#define SCALEBITS				16		// speediest right-shift on some machines
#define CBCR_OFFSET				((long)CENTERJSAMPLE << SCALEBITS)
#define ONE_HALF				((long)1 << (SCALEBITS - 1))
#define FIX(x)					((long)((x) * (1 << SCALEBITS) + 0.5))
#define R_Y_OFF					0						// offset to R => Y section
#define G_Y_OFF					(1 * (MAXJSAMPLE + 1))	// offset to G => Y section
#define B_Y_OFF					(2 * (MAXJSAMPLE + 1))	// etc.
#define R_CB_OFF				(3 * (MAXJSAMPLE + 1))
#define G_CB_OFF				(4 * (MAXJSAMPLE + 1))
#define B_CB_OFF				(5 * (MAXJSAMPLE + 1))
#define R_CR_OFF				B_CB_OFF				// B=>Cb, R=>Cr are the same
#define G_CR_OFF				(6 * (MAXJSAMPLE + 1))
#define B_CR_OFF				(7 * (MAXJSAMPLE + 1))
#define TABLE_SIZE				(8 * (MAXJSAMPLE + 1))


///////////////////////////////////////////////////////////////////////
// Initialize for RGB->YCC colorspace conversion.

long* rgb_ycc_init()
{
	long *rgb_ycc_tab;
	long i;

	// Allocate and fill in the conversion tables.
	rgb_ycc_tab = (long*)malloc(TABLE_SIZE * sizeof(long));
	if (!rgb_ycc_tab)
		return NULL;

	for (i = 0; i <= MAXJSAMPLE; i++) {
		rgb_ycc_tab[i + R_Y_OFF] = FIX(0.29900) * i;
		rgb_ycc_tab[i + G_Y_OFF] = FIX(0.58700) * i;
		rgb_ycc_tab[i + B_Y_OFF] = FIX(0.11400) * i + ONE_HALF;
		rgb_ycc_tab[i + R_CB_OFF] = (-FIX(0.16874)) * i;
		rgb_ycc_tab[i + G_CB_OFF] = (-FIX(0.33126)) * i;

		// We use a rounding fudge-factor of 0.5-epsilon for Cb and Cr.
		// This ensures that the maximum output will round to MAXJSAMPLE
		// not MAXJSAMPLE+1, and thus that we don't have to range-limit.
		rgb_ycc_tab[i + B_CB_OFF] = FIX(0.50000) * i + CBCR_OFFSET + ONE_HALF - 1;
		rgb_ycc_tab[i + G_CR_OFF] = (-FIX(0.41869)) * i;
		rgb_ycc_tab[i + B_CR_OFF] = (-FIX(0.08131)) * i;
	}
	return rgb_ycc_tab;
}

static _inline int rgb_gray_convert(long* ctab, RGBQUAD* prgbq)
{
	return (BYTE)((ctab[prgbq->rgbRed + R_Y_OFF] +
				   ctab[prgbq->rgbGreen + G_Y_OFF] +
				   ctab[prgbq->rgbBlue + B_Y_OFF]) >> SCALEBITS);
}

extern "C"
HBITMAP WINAPI lccLoadGrayedBitmap(HINSTANCE hInst, UINT id, COLORREF crMask)
{
	HRSRC hRsrc = ::FindResource(hInst, MAKEINTRESOURCE(id), RT_BITMAP);
	if (!hRsrc)
		return NULL;

	HGLOBAL hglb;
	if ((hglb = LoadResource(hInst, hRsrc)) == NULL)
		return NULL;

	BITMAPINFOHEADER* pbh = (LPBITMAPINFOHEADER)LockResource(hglb);
	if (pbh == NULL)
		return NULL;

	int colors = pbh->biClrUsed;
	if (!colors)
		colors = (pbh->biBitCount <= 8) ? (1 << pbh->biBitCount) : 0;

	UINT nSize = pbh->biSize + colors * sizeof(RGBQUAD);
	BITMAPINFOHEADER* pbhNew = (LPBITMAPINFOHEADER)::malloc(nSize);
	if (pbhNew == NULL)
		return NULL;

	memcpy(pbhNew, pbh, nSize);

	long* ctab = rgb_ycc_init();
	DWORD* pTable = (DWORD*)(((BYTE*)pbhNew) + (UINT)pbhNew->biSize);
	DWORD mask = CLR_TO_RGBQUAD(crMask);
	for (int i = 0; i < colors; i++) {
		if (pTable[i] != mask) {
			int gray = rgb_gray_convert(ctab, (RGBQUAD*)&pTable[i]);
			gray = max(0, min(gray + 31, 255));
			pTable[i] = RGB_TO_RGBQUAD(gray, gray, gray);
		}
	}
	free(ctab);

	int nWidth = (int)pbhNew->biWidth;
	int nHeight = (int)pbhNew->biHeight;
	CClientDC dc(nullptr);
	HBITMAP hbm = CreateCompatibleBitmap(dc, nWidth, nHeight);
	if (hbm != nullptr) {
		CDC dcw;
		dcw.CreateCompatibleDC(dc);
		HBITMAP hbmOld = dcw.SelectBitmap(hbm);

		BYTE* pBits = (BYTE*)pbh + *(DWORD*)pbh + colors * sizeof(RGBQUAD);
		dcw.StretchDIBits(0, 0, nWidth, nHeight, 0, 0, nWidth, nHeight,
					pBits, (BITMAPINFO*)pbhNew, DIB_RGB_COLORS, SRCCOPY);
		dcw.SelectBitmap(hbmOld);
	}

	free(pbhNew);
	FreeResource(hglb);
	return hbm;
}
