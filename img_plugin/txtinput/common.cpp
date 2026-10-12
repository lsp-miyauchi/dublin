#include "stdafx.h"
#include "common.h"


///////////////////////////////////////////////////////////////////////
// COMMON FUNCTIONS
//

HGLOBAL __fastcall copyhandle(HGLOBAL hsrc)
{
	HGLOBAL hdst = NULL;
	if (hsrc) {
		SIZE_T size = GlobalSize(hsrc);
		if (hdst = GlobalAlloc(GMEM_MOVEABLE, size)) {
			void* ps = GlobalLock(hsrc);
			void* pd = GlobalLock(hdst);
			memcpy(pd, ps, size);
			GlobalUnlock(hsrc);
			GlobalUnlock(hdst);
		}
	}
	return hdst;
}

int __fastcall dib_numcolors(void* pv)
{
	int colors = ((LPBITMAPINFOHEADER)pv)->biClrUsed;
	if (!colors) {
		int bits = ((LPBITMAPINFOHEADER)pv)->biBitCount;
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
		LPBITMAPINFOHEADER pbh = (LPBITMAPINFOHEADER)GlobalLock(hdib);
		if (pbh) {
			if (dx)		*dx = pbh->biWidth;
			if (dy)		*dy = pbh->biHeight;
			if (colors)	*colors = dib_numcolors(pbh);
			if (bits)	*bits = pbh->biBitCount;
			bOK= TRUE;
		}
		GlobalUnlock(hdib);
	}
	return bOK;
}
