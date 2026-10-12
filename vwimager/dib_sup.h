////////////////////////////////////////////////////////////////////////
// dib_sup.h

#pragma once
#ifndef __DIB_SUP_H__
#define __DIB_SUP_H__

#ifndef WIDTHBYTES
	#define WIDTHBYTES(bits)    (((bits) + 31) / 32 * 4)
#endif

#define dib_lock(h)				((void*)GlobalLock((h)))
#define dib_lockheader(h)		((BITMAPINFOHEADER*)GlobalLock((h)))
#define dib_lockinfo(h)			((BITMAPINFO*)GlobalLock((h)))
#define dib_unlock(h)			(GlobalUnlock((h)))

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
	__inline int BitCount() { return m_bits; }
	__inline int Width() { return m_width; }
	__inline int Height() { return m_height; }

	Image* GetThumbnail(int width, int height)
	{
		Image* image = NULL;
		if (!IsNull()) {
			Bitmap* bitmap = new Bitmap(BitmaoInfo(), BitmapBits());
			if (bitmap) {
				int cx = Width();
				int cy = Height();
				if (cx > width || cy > height)
					CalcInnerImageSize(width, height, &cx, &cy);
				else {
					cx = width;
					cy = height;
				}
				image = bitmap->GetThumbnailImage(cx, cy, NULL, NULL);
				delete bitmap;
			}
		}
		return image;
	}
};


#endif		// __DIB_SUP_H__
