//////////////////////////////////////////////////////////
// dib_sup.h

#pragma once
#ifndef __DIB_SUP_H__
#define __DIB_SUP_H__

HDIB __fastcall dib_create(int width, int height, int bits);
HDIB __fastcall dib_copy(HDIB hsrc);
void* __fastcall dib_copyptr(void* pSrc);
int __fastcall dib_numcolors(void* pv);
int __fastcall dib_palsize(void* pv);
BOOL __fastcall dib_getinfo(HDIB hdib, int* dx, int* dy, int* colors, int* bits);
HDIB __fastcall dib_createsolid(DWORD dwWidth, DWORD dwHeight, COLORREF rgb);
RGBPixel* __fastcall dib_getpacket(HDIB hdib, int sx, int sy, int dx, int dy);
void __fastcall dib_setpacket(HDIB hdib, RGBPixel* pkt, int sx, int sy, int dx, int dy);
BOOL __fastcall dib_getplane(HDIB hDIB, UINT nPlane, LPBYTE pPlane);
BOOL __fastcall dib_setplane(HDIB hDIB, UINT nPlane, LPBYTE pPlane);
BOOL __fastcall dib_planeblt(BYTE* pDst, int sxDst, int syDst, int cxDst, int cyDst, BYTE* pSrc, int sxSrc, int sySrc, int cxSrc, int cySrc, int xDelta, int yDelta);
HDIB __fastcall dib_createfrombitmap(HBITMAP hbm, HPALETTE hpal);

__inline BYTE* __fastcall dib_findbits(void* pv)
	{ return ((BYTE*)pv + *(DWORD*)pv + dib_palsize(pv)); }
__inline RGBPixel* __fastcall dib_makepacket(int width, int height)
	{ return (RGBPixel*)GlobalAllocPtr(GHND, width * height * sizeof(RGBPixel)); }
__inline void __fastcall dib_freepacket(RGBPixel* pkt)
	{ GlobalFreePtr(pkt); }
__inline void __fastcall dib_destroy(HDIB& hdib)
	{ GlobalFree(hdib); hdib = NULL; }

#define dib_lock(h)				((void*)GlobalLock((h)))
#define dib_lockheader(h)		((BITMAPINFOHEADER*)GlobalLock((h)))
#define dib_lockinfo(h)			((BITMAPINFO*)GlobalLock((h)))
#define dib_unlock(h)			(GlobalUnlock((h)))

#ifndef	WIDTHBYTES
#define WIDTHBYTES(bits)		(((bits) + 31) / 32 * 4)
#endif

class CDibHandle
{
private:
	void Unlock()
	{
		if (m_baseptr) dib_unlock(m_hdib);

		m_hdib = NULL;
		m_baseptr = NULL;
		m_bitsptr = NULL;
		m_width = 0;
		m_height = 0;
		m_bits = 0;
		m_colors = 0;
		m_palsize = 0;
		m_widthbytes = 0;
	}

	void Lock()
	{
		if (m_hdib && !m_baseptr) {
			m_baseptr = dib_lock(m_hdib);

			BITMAPINFOHEADER* pbh = (BITMAPINFOHEADER*)m_baseptr;
			m_width = pbh->biWidth;
			m_height = pbh->biHeight;
			m_bits = pbh->biBitCount;
			m_widthbytes = WIDTHBYTES(m_bits * m_width);
			m_colors = pbh->biClrUsed;
			if (!m_colors)
				m_colors = (m_bits <= 8) ? (1 << m_bits) : 0;
			m_palsize = m_colors * sizeof(RGBQUAD);
			if (pbh->biCompression == BI_BITFIELDS)
				m_palsize += sizeof(DWORD) * 3;

			m_bitsptr = (BYTE*)m_baseptr + *(DWORD*)m_baseptr + m_palsize;
		}
	}

public:
// Data members
	HDIB m_hdib;
	void* m_baseptr;
	BYTE* m_bitsptr;
	int m_width;
	int m_height;
	int m_bits;
	int m_colors;
	int m_palsize;
	int m_widthbytes;

// Constructor/destructor/operators
	CDibHandle(HDIB hdib = NULL)
		: m_hdib(hdib)
		, m_baseptr(NULL)
		, m_bitsptr(NULL)
		, m_width(0)
		, m_height(0)
		, m_bits(0)
		, m_colors(0)
		, m_palsize(0)
		, m_widthbytes(0)
	{
		Lock();
	}

	~CDibHandle()
	{
		Unlock();
	}

	HDIB Detach()
	{
		HDIB hdib = m_hdib;
		Unlock();
		return hdib;
	}

	CDibHandle& operator=(HDIB hdib)
	{
		Unlock();
		m_hdib = hdib;
		Lock();
		return *this;
	}

	__inline bool IsNull() const { return (m_hdib == NULL); }
	__inline BITMAPINFO* BitmaoInfo() { return (BITMAPINFO*)m_baseptr; }
	__inline BITMAPINFOHEADER* BitmaoInfoHeader() { return (BITMAPINFOHEADER*)m_baseptr; }
	__inline RGBQUAD* ColorMap() { return (m_baseptr ? ((BITMAPINFO*)m_baseptr)->bmiColors : NULL); }
	__inline void* BitmapBits() { return m_bitsptr; }
	__inline int WidthBytes() { return m_widthbytes; }
	__inline void* RowPosition(int row) { return (void*)((BYTE*)m_bitsptr + m_widthbytes * (m_height - row - 1)); }
	__inline void* RowUp(void* pos) { return (void*)((BYTE*)pos + m_widthbytes); }
	__inline void* RowDown(void* pos) { return (void*)((BYTE*)pos - m_widthbytes); }
	__inline int BitCount() { return m_bits; }
	__inline int Width() { return m_width; }
	__inline int Height() { return m_height; }
};

#endif		// __DIB_SUP_H__
