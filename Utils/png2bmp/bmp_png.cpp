#include "stdafx.h"
#include "mfmem.h"

#define DIB_HEADER_MARKER   ((WORD) ('M' << 8) | 'B')
#define WIDTHBYTES(bits)	(((bits) + 31) / 32 * 4)


static HGLOBAL dib_load_mem(HGLOBAL hmem)
{
	long memsize = (long)GlobalSize(hmem);
	if (memsize < sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFO)) {
		_tprintf(_T("size error\n"));
		return NULL;
	}

	void* base_ptr = GlobalLock(hmem);
	if (!base_ptr) {
		_tprintf(_T("memory error\n"));
		return NULL;
	}

	BITMAPINFOHEADER* pbh = (BITMAPINFOHEADER*)((BYTE*)base_ptr + sizeof(BITMAPFILEHEADER));
	if (*(WORD*)base_ptr != DIB_HEADER_MARKER) {
		GlobalUnlock(hmem);
		_tprintf(_T("not a bitmap file\n"));
		return NULL;
	}
	if (pbh->biSize != 40) {
		GlobalUnlock(hmem);
		_tprintf(_T("unsupported bitmap type\n"));
		return NULL;
	}
	if (pbh->biCompression != BI_RGB) {
		GlobalUnlock(hmem);
		_tprintf(_T("unsupported compression type\n"));
		return NULL;
	}

	_tprintf(_T("bpp=%d, width=%d, height=%d\n"), pbh->biBitCount, pbh->biWidth, pbh->biHeight);

	long totallength = WIDTHBYTES(pbh->biBitCount * pbh->biWidth) * pbh->biHeight;

	if (memsize < totallength + sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER)) {
		GlobalUnlock(hmem);
		_tprintf(_T("unsufficient dib data\n"));
		return NULL;
	}

	memsize -= sizeof(BITMAPFILEHEADER);
	HGLOBAL hdib = GlobalAlloc(GHND, memsize);
	if (hdib) {
		void* pv = GlobalLock(hdib);
		if (pv) {
			memcpy(pv, pbh, memsize);
			GlobalUnlock(hdib);
		}
		else {
			GlobalFree(hdib);
			hdib = NULL;
		}
	}
	return hdib;
}

static HGLOBAL dib_load(LPCTSTR pszFile)
{
	DWORD dwSize;
	HGLOBAL hdib = NULL;
	HGLOBAL hmem = readfileall(pszFile, &dwSize);
	if (hmem) {
		hdib = dib_load_mem(hmem);
		GlobalFree(hmem);
	}
	else
		_tprintf(_T("cannot read %s\n"), pszFile);
	return hdib;
}

void bmp2png(LPCTSTR pszBmp, LPTSTR pszPng)
{
	HGLOBAL hdib = dib_load(pszBmp);
	if (hdib) {
		_tprintf(_T("output to %s\n"), pszPng);
#ifdef UNICODE
		imgPngSave(pszPng, hdib);
#else
		WCHAR wsz[MAX_PATH];
		MultiByteToWideChar(CP_OEMCP, 0, pszPng, -1, wsz, MAX_PATH);
		imgPngSave(wsz, hdib);
#endif

		GlobalFree(hdib);
	}
}

