///////////////////////////////////////////////////////////////////////
// CDib

#pragma once
#ifndef __LSP_DIB_H__
#define __LSP_DIB_H__

#include <gdiplus.h>
using namespace Gdiplus;


#ifndef WIDTHBYTES
#define WIDTHBYTES(bits)		(((bits) + 31) / 32 * 4)
#endif

class CDib
{
public:
	HGLOBAL m_hdib;
	BITMAPINFO* m_pbi;

	CDib(HGLOBAL hdib = NULL) : m_hdib(hdib), m_pbi(NULL)
	{
		if (m_hdib) m_pbi = (BITMAPINFO*)GlobalLock(m_hdib);
	}

	CDib(int width, int height, int bits) : m_hdib(NULL), m_pbi(NULL)
	{
		Create(width, height, bits);
	}

	CDib(Gdiplus::Bitmap* bitmap) : m_hdib(NULL), m_pbi(NULL)
	{
		AttachGdiplusBitmap(bitmap);
	}

	CDib(HBITMAP hbm, HPALETTE hpal) : m_hdib(NULL), m_pbi(NULL)
	{
		createfrombitmap(hbm, hpal);
	}

	~CDib()
	{
		Detach();
	}

	operator HGLOBAL() const { return m_hdib; }
	bool isNull() const { return (m_hdib == NULL); }

	HGLOBAL Attach(HGLOBAL hdib)
	{
		HGLOBAL hdibOld = Detach();
		m_hdib = hdib;
		if (m_hdib) m_pbi = (BITMAPINFO*)GlobalLock(m_hdib);
		return hdibOld;
	}

	HGLOBAL Detach()
	{
		HGLOBAL hdib = m_hdib;
		if (m_hdib && m_pbi)
			GlobalUnlock(m_hdib);
		m_hdib = NULL;
		m_pbi = NULL;
		return hdib;
	}

	HGLOBAL AttachGdiplusBitmap(Gdiplus::Bitmap* bitmap)
	{
		HGLOBAL hdibOld = Detach();

		int bits = (bitmap->GetPixelFormat() == PixelFormat32bppARGB) ? 32 : 24;
		int width = bitmap->GetWidth();
		int height = bitmap->GetHeight();

		BitmapData bitmapData;
		Rect rect(0, 0, width, height);
		if (bitmap->LockBits(&rect, ImageLockModeRead, PixelFormat32bppARGB, &bitmapData) == Ok) {
			if (Create(width, height, bits)) {
				UINT* pixels = (UINT*)bitmapData.Scan0;
				for (int y = 0; y < height; ++y) {
					BYTE* row = (BYTE*)RowBits(y);
					UINT* p = &pixels[y * bitmapData.Stride / 4];
					for (int x = 0; x < width; ++x) {
						*row++ = (*p      ) & 0xff;
						*row++ = (*p >>  8) & 0xff;
						*row++ = (*p >> 16) & 0xff;
						if (bits == 32)
							*row++ = (*p >> 24) & 0xff;
						p++;
					}
				}
			}
			bitmap->UnlockBits(&bitmapData);
		}

		return hdibOld;
	}

	bool Create(int width, int height, int bits)
	{
		Detach();

		if      (bits <= 1)		bits = 1;
		else if (bits <= 4)		bits = 4;
		else if (bits <= 8)		bits = 8;
		else if (bits <= 24)	bits = 24;
		else if (bits <= 32)	bits = 32;
		else					bits = 24;

		BITMAPINFOHEADER bi;
		memset(&bi, 0, sizeof(bi));
		bi.biSize = sizeof(bi);
		bi.biWidth = width;
		bi.biHeight = height;
		bi.biPlanes = 1;
		bi.biBitCount = (WORD)bits;
		bi.biCompression = BI_RGB;

		DWORD widthbytes = WIDTHBYTES(bits * width);
		int colors = (bits <= 8) ? (1 << bits) : 0;
		DWORD size = bi.biSize + colors * sizeof(RGBQUAD) + (widthbytes * (height + 16));
		bi.biSizeImage = widthbytes * height;
		m_hdib = GlobalAlloc(GMEM_MOVEABLE, size);
		if (m_hdib) {
			m_pbi = (BITMAPINFO*)GlobalLock(m_hdib);
			memset(m_pbi, 0, size);
			memcpy(m_pbi, &bi, sizeof(bi));
		}
		return (m_pbi != NULL);
	}

	int NumColors()
	{
		int colors = 0;
		if (m_pbi) {
			colors = m_pbi->bmiHeader.biClrUsed;
			if (!colors) {
				int bits = m_pbi->bmiHeader.biBitCount;
				colors = (bits <= 8) ? (1 << bits) : 0;
			}
		}
		return colors;
	}

	int PalSize()
	{
		int size = 0;
		if (m_pbi) {
			size = NumColors() * sizeof(RGBQUAD);
			if (m_pbi->bmiHeader.biCompression == BI_BITFIELDS)
				size += sizeof(DWORD) * 3;
		}
		return size;
	}

	HBITMAP GetHBitmap(HDC hdc)
	{
		HBITMAP hbm = NULL;
		if (m_pbi) {
			BITMAPINFOHEADER* pbh = (BITMAPINFOHEADER*)m_pbi;
			void* pbits = FindBits();
			hbm = CreateDIBitmap(hdc, pbh, CBM_INIT, pbits, m_pbi, DIB_RGB_COLORS);
			if (!hbm) {
				void* pv;
				hbm = CreateDIBSection(hdc, m_pbi, DIB_RGB_COLORS, &pv, NULL, 0);
				if (hbm)
					SetDIBits(hdc, hbm, 0, pbh->biHeight, pbits, m_pbi, DIB_RGB_COLORS);
			}
		}
		return hbm;
	}

	_inline BITMAPINFO* BitmapInfo()
	{
		return m_pbi;
	}

	_inline BITMAPINFOHEADER* BitmapInfoHeader()
	{
		return (m_pbi != NULL) ? &m_pbi->bmiHeader : NULL;
	}

	_inline void* FindBits()
	{
		return m_pbi ? (void*)((BYTE*)m_pbi + m_pbi->bmiHeader.biSize + PalSize()) : NULL;
	}

	void* RowBits(int row)
	{
		BYTE* pb = (BYTE*)FindBits();
		if (pb) {
			DWORD widthbytes = WIDTHBYTES(NumBits() * Width());
			pb += (Height() - row - 1) * widthbytes;
		}
		return (void*)pb;
	}

	_inline int NumBits()
	{
		return (m_pbi != NULL) ? (int)m_pbi->bmiHeader.biBitCount : 0;
	}

	_inline int Width()
	{
		return (m_pbi != NULL) ? (int)m_pbi->bmiHeader.biWidth : 0;
	}

	_inline int Height()
	{
		return (m_pbi != NULL) ? (int)m_pbi->bmiHeader.biHeight : 0;
	}

	_inline int WidthBytes()
	{
		return WIDTHBYTES(NumBits() * Width());
	}

private:
	bool createfrombitmap(HBITMAP hbm, HPALETTE hpal)
	{
		if (!hbm) return false;

		BITMAPINFO bi;
		memset(&bi, 0, sizeof(bi));
	    bi.bmiHeader.biSize = sizeof(bi);

		// call GetDIBits with a NULL lpBits param, so it will calculate the
		// biSizeImage field for us    
		HDC hdc = GetDC(NULL);
		if (hpal) {
			hpal = SelectPalette(hdc, hpal, FALSE);
			RealizePalette(hdc);
		}
		if (GetDIBits(hdc, hbm, 0, 0, NULL, &bi, DIB_RGB_COLORS) != 0) {
			// if the driver did not fill in the biSizeImage field, make one up 
			if (bi.bmiHeader.biSizeImage == 0)
				bi.bmiHeader.biSizeImage = WIDTHBYTES((DWORD)bi.bmiHeader.biWidth * bi.bmiHeader.biBitCount) * bi.bmiHeader.biHeight;
			// 仮定義
			m_pbi = (BITMAPINFO*)&bi;
			// alloc the buffer big enough to hold all the bits
			size_t totalsize = bi.bmiHeader.biSize + PalSize() + bi.bmiHeader.biSizeImage;
			m_hdib = GlobalAlloc(GMEM_MOVEABLE, totalsize);
			if (m_hdib) {
				// ここで正式にビットマップヘッダを定義
				m_pbi = (BITMAPINFO*)GlobalLock(m_hdib);
			    memcpy(m_pbi, &bi, sizeof(bi));
				// call GetDIBits with a NON-NULL lpBits param, and actualy get the
				// bits this time
				if (GetDIBits(hdc, hbm, 0, Height(), FindBits(), m_pbi, DIB_RGB_COLORS) == 0) {
					GlobalUnlock(m_hdib);
					GlobalFree(m_hdib);
					m_hdib = NULL;
					m_pbi = NULL;
				}
			}
		}
		if (hpal) {
			SelectPalette(hdc, hpal, TRUE);
			RealizePalette(hdc);
		}
		ReleaseDC(NULL, hdc);
		// return handle to the DIB
		return (m_hdib != NULL);
	}
};


#endif				// __LSP_DIB_H__
