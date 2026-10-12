////////////////////////////////////////////////////////////////////////
// psdread.cpp - Converts from/to Adobe PSD image

#include "stdafx.h"
#include "lsimg.h"
#include "common.h"
#include "filemap.h"

extern "C" {
HDIB WINAPI psd_load(LPCSTR pszName);
HDIB WINAPI psd_loadW(LPCWSTR pszName);
BOOL WINAPI psd_save(LPCSTR pszName, HDIB hdib);
BOOL WINAPI psd_saveW(LPCWSTR pszName, HDIB hdib);
}

extern "C"
HDIB WINAPI imgPsdLoad(LPCTSTR pszName) 
{
#ifdef UNICODE
	return psd_loadW(pszName);
#else
	return psd_load(pszName);
#endif
}


extern "C"
BOOL WINAPI imgPsdSave(LPCTSTR pszName, HDIB hdib)
{
#ifdef UNICODE
	return psd_saveW(pszName, hdib);
#else
	return psd_save(pszName, hdib);
#endif
}

#pragma pack(1)

typedef struct _THUMBINFO
{
	ULONG format;			// format = 1 (Jpeg). RawRGB (0).
	ULONG width;			// Width of thumbnail in pixels.
	ULONG height;			// Height of thumbnail in pixels.
	ULONG widthbytes;		// Padded row bytes as (width * bits + 31) / 32 * 4.
	ULONG size;				// Total size as widthbytes * height * planes
	ULONG compressedsize;	// Size after compression. Used for consistentcy check.
	USHORT bitspixel;		// = 24. Bits per pixel.
	USHORT planes;			// = 1. Number of planes.
	// and follow JFIF or BGR
} THUMBINFO;

typedef struct _PSDHEADER				// PSD v3.0 HEADER
{
	char signature[4];					// "8BPS"
	USHORT version;						// only '1'
	BYTE reserved[6];
	USHORT channels;					// range 1 - 24
	int rows;							// range 1 - 30,000
	int columns;						// range 1 - 30,000
	USHORT depth;						// 1, 8, 16
	USHORT mode;						//
} PSDHEADER;

#pragma pack()


static HDIB psd_thumb_load(filemap& fm, int* pwidth, int* pheight)
{
	char type[4];
	long count, length, size;
	PSDHEADER head;
	register int i;
	UINT status;

	// イメージヘッダを読む
	status = fm.read(head.signature, 4);
	head.version = fm.getmsbshort();
	if (!status ||
		strncmp(head.signature, "8BPS", 4) ||
		head.version != 1)
	{
		return NULL;
	}

	fm.read(head.reserved, 6);
	head.channels = fm.getmsbshort();
	head.rows = fm.getmsblong();
	head.columns = fm.getmsblong();
	head.depth = fm.getmsbshort();
	head.mode = fm.getmsbshort();

	// カラーマップセクションを読む
	length = fm.getmsblong();
	if (length > 0)
		fm.seek( length, SEEK_CUR);

	// イメージリソースセクション
	HDIB hdib = NULL;
	length = fm.getmsblong();
	while (length > 0) {
		// リソースブロックの読み込み
		status = fm.read(type, 4);
		if (!status || strncmp(type,"8BIM",4))
			break;

		// リソースID
		int id = fm.getmsbshort();

		// リソース文字列（偶数サイズ）
		count = fm.getc();
		if (count > 0)
			fm.seek(count, SEEK_CUR);
		if (!(count & 0x01)) {
			fm.seek(1, SEEK_CUR);
			length--;
		}

		// リソースブロックサイズ（偶数サイズ）
		size = fm.getmsblong();

		if (id == 1033 || id == 1036) {					// psp4.0/5.0 サムネイル
			THUMBINFO ti;
			ti.format = fm.getmsblong();
			ti.width = fm.getmsblong();
			ti.height = fm.getmsblong();
			ti.widthbytes = fm.getmsblong();
			ti.size = fm.getmsblong();
			ti.compressedsize = fm.getmsblong();
			ti.bitspixel = fm.getmsbshort();
			ti.planes = fm.getmsbshort();

			LPBYTE pb = (LPBYTE)GlobalAllocPtr(GHND, size);
			fm.read(pb, size);
			if (ti.format != 0)							// JFIF形式
				hdib = imgJpgLoadMem(GlobalPtrHandle(pb), 0, 1);
			else {										// BGR形式
				hdib = dib_create(ti.width, ti.height, 24);
				LPBYTE pSrc = pb;
				LPBYTE pDst = (LPBYTE)GlobalLock(hdib);
				pDst = dib_findbits(pDst) + (ti.height - 1) * ti.widthbytes;
				for (i = 0; i < ti.height; i++) {
					memcpy(pDst, pSrc, 3 * ti.width);
					pSrc += ti.widthbytes;
					pDst -= ti.widthbytes;
				}
				GlobalUnlock(hdib);
			}
			GlobalFreePtr(pb);
			break;
		} else {
			// リソースブロックデータを読み飛ばす
			if (size & 0x01)
				size++;
			fm.seek(size, SEEK_CUR);
			length -= (count + size + 11);
		}
	}

	if (pwidth)
		*pwidth = head.columns;
	if (pheight)
		*pheight = head.rows;

	if (!hdib) {
		float f;
		int dx, dy;
		for (f = 1.25F; ; f += 0.25F) {
			dx = (int)((float)*pwidth / f);
			dy = (int)((float)*pheight / f);
			if (dx <= 112 && dy <= 112)
				break;
		}

		hdib = dib_createsolid(dx, dy, RGB(255, 255, 255));
	}
	return hdib;
}

extern "C"
HDIB WINAPI imgPsdThumbLoad(LPCTSTR pszName, int* pwidth, int* pheight)
{
	HDIB hdib = NULL;
	filemap fm;
	if (fm.open(pszName))
		hdib = psd_thumb_load(fm, pwidth, pheight);
	return hdib;
}
