///////////////////////////////////////////////////////////////////////
// CDib

#pragma once
#ifndef __DIB_H__
#define __DIB_H__

#define WIDTHBYTES(bits)		(((bits) + 31) / 32 * 4)

class CDib
{
public:
	HANDLE m_hdib;
	BITMAPINFO* m_pbi;

	CDib(HANDLE hdib = NULL) : m_hdib(hdib), m_pbi(NULL)
	{
		if (m_hdib) m_pbi = (BITMAPINFO*)GlobalLock(m_hdib);
	}

	~CDib()
	{
		Detach();
	}

	operator HANDLE() const { return m_hdib; }
	bool IsNull() const { return (m_hdib == NULL); }

	HANDLE Attach(HANDLE hdib)
	{
		HANDLE hdibOld = Detach();
		m_hdib = hdib;
		if (m_hdib) m_pbi = (BITMAPINFO*)GlobalLock(m_hdib);
		return hdibOld;
	}

	HANDLE Detach()
	{
		HANDLE hdib = m_hdib;
		if (m_hdib && m_pbi)
			GlobalUnlock(m_hdib);
		m_hdib = NULL;
		m_pbi = NULL;
		return hdib;
	}

	int NumColors()
	{
		int colors = 0;
		if (m_pbi) {
			colors = ((BITMAPINFOHEADER*)m_pbi)->biClrUsed;
			if (!colors) {
				int bits = ((BITMAPINFOHEADER*)m_pbi)->biBitCount;
				colors = (bits <= 8) ? (1 << bits) : 0;
			}
		}
		return colors;
	}

	int PalSize()
	{
		int size = NumColors() * sizeof(RGBQUAD);
		if (m_pbi && ((BITMAPINFOHEADER*)m_pbi)->biCompression == BI_BITFIELDS)
			size += sizeof(DWORD) * 3;
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
			ATLASSERT(hbm != NULL);
		}
		return hbm;
	}

	HBITMAP GetHBitmap(const Color &colorBackground)
	{
		HBITMAP hbm = NULL;
		if (m_pbi) {
			Bitmap bitmap(BitmapInfo(), FindBits());
			bitmap.GetHBITMAP(colorBackground, &hbm);
			ATLASSERT(hbm != NULL);
		}
		return hbm;
	}

	inline BITMAPINFO* BitmapInfo()
	{
		return m_pbi;
	}

	inline void* FindBits()
	{
		return m_pbi ? (void*)((BYTE*)m_pbi + *(DWORD*)m_pbi + PalSize()) : NULL;
	}

	inline int NumBits()
	{
		return (m_pbi != NULL) ? (int)((BITMAPINFOHEADER*)m_pbi)->biBitCount : 0;
	}

	inline int Width()
	{
		return (m_pbi != NULL) ? (int)((BITMAPINFOHEADER*)m_pbi)->biWidth : 0;
	}

	inline int Height()
	{
		return (m_pbi != NULL) ? (int)((BITMAPINFOHEADER*)m_pbi)->biHeight : 0;
	}
};

class memdib
{
public:
	BITMAPINFO* m_pbi;
	BITMAPINFOHEADER* m_pbh;
	BYTE* m_pixels;
	int m_widthbytes;

	memdib() : m_pbi(NULL), m_pbh(NULL), m_pixels(NULL), m_widthbytes(0)
	{
	}

	memdib(int width, int height, int bits)
	{
		m_widthbytes = WIDTHBYTES(width * bits);
		int headersize = sizeof(BITMAPINFOHEADER) + ((bits <= 8) ? sizeof(RGBQUAD) * ((_int64)1 << bits) : 0);
		int imagesize = m_widthbytes * height;
		m_pbi = (BITMAPINFO*)malloc(headersize + imagesize);
		if (m_pbi) {
			memset(m_pbi, 0, headersize);
			m_pbh = (BITMAPINFOHEADER*)m_pbi;
			m_pbh->biSize = sizeof(BITMAPINFOHEADER);
			m_pbh->biWidth = width;
			m_pbh->biHeight = height;
			m_pbh->biBitCount = (WORD)bits;
			m_pbh->biCompression = BI_RGB;
			m_pbh->biSizeImage = imagesize;
			m_pbh->biPlanes = 1;
			if (bits <= 8)
				m_pbh->biClrUsed = m_pbh->biClrImportant = (1 << bits);
			m_pixels = (BYTE*)m_pbi + headersize;
		}
	}

	~memdib()
	{
		if (m_pbi) free(m_pbi);
	}

	__inline bool IsNull() { return (m_pbi == NULL); }
	__inline int Width() { return (int)m_pbh->biWidth; }
	__inline int Height() { return (int)m_pbh->biHeight; }
	__inline int BitCount() { return (int)m_pbh->biBitCount; }
	__inline int WidthBytes() { return m_widthbytes; }
	__inline BITMAPINFO* BitmapInfo() { return m_pbi; }
	__inline BITMAPINFOHEADER* BitmapInfoHeader() { return m_pbh; }
	__inline BYTE* BitmapBits() { return m_pixels; }
	__inline BYTE* RowPosition(int row) { return (m_pixels + m_widthbytes * (m_pbh->biHeight - row - 1)); }
	__inline RGBQUAD* ColorMap() { return m_pbi->bmiColors; }

	Bitmap* GetBitmap()
	{
		if (IsNull()) return NULL;
		return new Bitmap(m_pbi, m_pixels);
	}

	Bitmap* GetThumbnail(int width, int height, bool rotate = false)
	{
		Bitmap* bitmap = GetBitmap();
		if (!bitmap) return NULL;

		int cx = m_pbh->biWidth;
		int cy = m_pbh->biHeight;
		if (rotate) {
			bitmap->RotateFlip(Rotate270FlipNone);
			cx = m_pbh->biHeight;
			cy = m_pbh->biWidth;
		}

		Bitmap* bm = NULL;
		if (cx > width || cy > height) {
			CalcInnerImageSize(width, height, &cx, &cy);
			Image* image = bitmap->GetThumbnailImage(cx, cy, NULL, NULL);
			Bitmap* bitmapCast = (Bitmap*)image;
			bm = bitmapCast->Clone(0, 0, bitmapCast->GetWidth(), bitmapCast->GetHeight(), bitmapCast->GetPixelFormat());
			delete image;
			delete bitmap;
		}
		else
			bm = bitmap;
		return bm;
	}
};

#endif				// __DIB_H__
