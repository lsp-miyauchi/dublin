////////////////////////////////////////////////////////////////////////
// histogram_calc.h

#pragma once
#ifndef __HISTORGAM_CALC_H__
#define __HISTORGAM_CALC_H__

#include <math.h>

#define HITOGRAM_X_RANGE		256
#define HITOGRAM_Y_RANGE		128


// #define VALCONV(n)			((float)(n)/log((float)(n)))
// #define VALCONV(n)			(sqrt((float)(n)))
#define VALCONV(n)				(logf((float)(n)) * sqrtf((float)(n)))

////////////////////////////////////////////////////////////////////////
//

inline Bitmap* GdipGetBitmapFromDIB(HDIB hdib)
{
	// HDIB から gdi+ の Bitmapオブジェクトを作成する
	Bitmap* bitmap = NULL;
	BITMAPINFO* pbi = (BITMAPINFO*)GlobalLock(hdib);
	if (pbi) {
		int colors = pbi->bmiHeader.biClrUsed;
		if (!colors) {
			int bits = pbi->bmiHeader.biBitCount;
			colors = (bits <= 8) ? (1 << bits) : 0;
		}
		int palsize = colors * sizeof(RGBQUAD);
		if (pbi->bmiHeader.biCompression == BI_BITFIELDS)
			palsize += sizeof(DWORD) * 3;
		void* dataptr = (void*)((BYTE*)pbi + *(DWORD*)pbi + palsize);
		bitmap = new Bitmap(pbi, dataptr);
		GlobalUnlock(hdib);
	}
	return bitmap;
}

////////////////////////////////////////////////////////////////////////
//

enum {
	HISTOGRAM_VALUE = 1,
	HISTOGRAM_COLOR = 2,
	HISTOGRAM_MULTI = 3,
};

class CHistogramInfo
{
public:
	UINT* m_value;
	UINT* m_red;
	UINT* m_green;
	UINT* m_blue;
	UINT m_max_red;
	UINT m_max_green;
	UINT m_max_blue;
	UINT m_max_value;
	COLORREF m_clrBkgnd;
	bool m_initcompleted:1;
	LPARAM m_userdata;					// 汎用ユーザデータ

	CHistogramInfo()
		: m_value(NULL)
		, m_red(NULL)
		, m_green(NULL)
		, m_blue(NULL)
		, m_max_value(0)
		, m_max_red(0)
		, m_max_green(0)
		, m_max_blue(0)
		, m_clrBkgnd(RGB(0, 0, 0))
		, m_initcompleted(false)
		, m_userdata(0)
	{
		m_value = new UINT[256 * 4];
		if (m_value) {
			m_red   = m_value + 256;
			m_green = m_red + 256;
			m_blue  = m_green + 256;
		}
	}

	~CHistogramInfo()
	{
		if (m_value)
			delete m_value;
	}

	bool Initialize(Bitmap* bitmap)
	{
		m_initcompleted = false;
		if (bitmap) {
			memset(m_value, 0, sizeof(UINT) * 256 * 4);
			m_max_value = m_max_red = m_max_green = m_max_blue = 0;

			int width = bitmap->GetWidth();
			int height = bitmap->GetHeight();
			BitmapData bitmapData;
			Rect rect(0, 0, width, height);

			// すべてのピクセルは計算しない
			int rowStep = (height >= 3072) ? 3 : ((height >= 2048) ? 2 : 1);
			int colStep = (width >= 3072) ? 3 : ((width >= 2048) ? 2 : 1);

			if (bitmap->LockBits(&rect, ImageLockModeRead, PixelFormat32bppARGB, &bitmapData) == Ok) {
				UINT* pixels = (UINT*)bitmapData.Scan0;
				for (int row = 0; row < height; row += rowStep) {
					UINT* p = &pixels[row * bitmapData.Stride / 4];
					for (int col = 0; col < width; col += colStep) {
						UINT r = (*p >> 16) & 0xff;
						UINT g = (*p >>  8) & 0xff;
						UINT b = (*p      ) & 0xff;
						UINT v = (r > g) ? r : g;
						if (b > v) v = b;

						m_value[v]++;
						m_red[r]++;
						m_green[g]++;
						m_blue[b]++;

						if (m_value[v] > m_max_value)	m_max_value = m_value[v];
						if (m_red[r]   > m_max_red)		m_max_red   = m_red[r];
						if (m_green[g] > m_max_green)	m_max_green = m_green[g];
						if (m_blue[b]  > m_max_blue)	m_max_blue  = m_blue[b];

						p += colStep;
					}
				}
				bitmap->UnlockBits(&bitmapData);
				m_initcompleted = true;
			}
		}
		return m_initcompleted;
	}

	bool Initialize(HDIB hdib)
	{
		m_initcompleted = false;
		Bitmap* bitmap = GdipGetBitmapFromDIB(hdib);
		if (bitmap) {
			m_initcompleted = Initialize(bitmap);
			delete bitmap;
		}
		return m_initcompleted;
	}

	COLORREF SetBkColor(COLORREF clrBkgnd)
	{
		COLORREF clr = m_clrBkgnd;
		m_clrBkgnd = clrBkgnd;
		return clr;
	}

	HBITMAP MakeHistogramBitmap(int mode, int xRange = HITOGRAM_X_RANGE, int yRange = HITOGRAM_Y_RANGE)
	{
		if (!m_initcompleted) return NULL;

typedef struct tagHISTOGRAMDRAWARRAY {
	HPEN hpen;
	HPEN hpenV;
	UINT* hist;
	float hi;
} HISTOGRAMDRAWARRAY;

		CClientDC dc(NULL);
		CDC dcw;
		dcw.CreateCompatibleDC(dc);

		HBITMAP hbm = CreateCompatibleBitmap(dc, xRange, yRange);
		hbm = dcw.SelectBitmap(hbm);

		dcw.FillSolidRect(0, 0, xRange, yRange, m_clrBkgnd);

		float hi_v, hi_r, hi_g, hi_b;
		if (mode == HISTOGRAM_VALUE) {
			hi_v = (m_max_value > 0) ? VALCONV(m_max_value) : 1.0F;
		}
		else {
			hi_r = (m_max_red   > 0) ? VALCONV(m_max_red)   : 1.0F;
			hi_g = (m_max_green > 0) ? VALCONV(m_max_green) : 1.0F;
			hi_b = (m_max_blue  > 0) ? VALCONV(m_max_blue)  : 1.0F;
		}

#define HST_WHITE		RGB(208, 208, 208)
#define HST_RED			RGB(208,  64,  64)
#define HST_GREEN		RGB(64,  208,  64)
#define HST_BLUE		RGB(64,   64, 208)

#define HST_VWHITE		RGB(240, 240, 240)
#define HST_VRED		RGB(240,  32,  32)
#define HST_VGREEN		RGB(32,  240,  32)
#define HST_VBLUE		RGB(32,   32, 240)

		CPen penWhite;	penWhite.CreatePen(PS_SOLID, 0, HST_WHITE);
		CPen penRed;	penRed.CreatePen(PS_SOLID, 0, HST_RED);
		CPen penGreen;	penGreen.CreatePen(PS_SOLID, 0, HST_GREEN);
		CPen penBlue;	penBlue.CreatePen(PS_SOLID, 0, HST_BLUE);
		CPen penVWhite;	penVWhite.CreatePen(PS_SOLID, 2, HST_VWHITE);
		CPen penVRed;	penVRed.CreatePen(PS_SOLID, 2, HST_VRED);
		CPen penVGreen;	penVGreen.CreatePen(PS_SOLID, 2, HST_VGREEN);
		CPen penVBlue;	penVBlue.CreatePen(PS_SOLID, 2, HST_VBLUE);

		HISTOGRAMDRAWARRAY arr[4] = {
			{ penRed,   penVRed,   m_red,   hi_r },
			{ penGreen, penVGreen, m_green, hi_g },
			{ penBlue,  penVBlue,  m_blue,  hi_b },
			{ penWhite, penVWhite, m_value, hi_v },
		};

		dcw.SetROP2(R2_MERGEPEN);							// ROP2
		POINT* ppt = new POINT[xRange];						// PolyLineバッファ
		HISTOGRAMDRAWARRAY* pa = (mode != HISTOGRAM_VALUE) ? arr : &arr[3];
		int cnt = (mode != HISTOGRAM_VALUE) ? 3 : 1;
		float yDelta = (mode == HISTOGRAM_MULTI) ? ((float)yRange / 3.0F) : (float)yRange;
		float row = 0;
		while (cnt-- > 0) {
			HPEN hpen = dcw.SelectPen(pa->hpen);
			UINT* ph = pa->hist;
			for (int i = 0; i < xRange; i++) {
				int y = (*ph > 0) ? (int)(VALCONV(*ph) * yDelta / pa->hi) : 0;
				dcw.MoveTo(i, (int)(row + yDelta));
				dcw.LineTo(i, (int)(row + yDelta - y));
				ppt[i].x = i; ppt[i].y = (int)(row + yDelta - y);
				ph++;
			}
			dcw.SelectPen(pa->hpenV);
			dcw.Polyline(ppt, xRange);
			dcw.SelectPen(hpen);
			pa++;
			row += (mode == HISTOGRAM_MULTI) ? yDelta : 0;
		}
		delete [] ppt;

		return dcw.SelectBitmap(hbm);
	}
};


#endif		// __HISTORGAM_CALC_H__
