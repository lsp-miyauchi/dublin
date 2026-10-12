/////////////////////////////////////////////////////////////////////////
// loadimage.cpp

#include "stdafx.h"

HBITMAP LoadBitmapFromResourceData(HINSTANCE hInst, LPCTSTR typeResource, UINT idResource)
{
	HBITMAP hbm = NULL;
	HRSRC hrsrc = FindResource(hInst, MAKEINTRESOURCE(idResource), typeResource);
	if (hrsrc) {
		DWORD sizeResource = SizeofResource(hInst, hrsrc);
		if (sizeResource) {
			HGLOBAL hglb = LoadResource(hInst, hrsrc);
			if (hglb) {
				void* pr = LockResource(hglb);
				if (pr) {
					HGLOBAL hmem = GlobalAlloc(GHND, sizeResource);
					if (hmem) {
						void* pd = GlobalLock(hmem);
						memcpy(pd, pr, sizeResource);
						GlobalUnlock(hmem);

						IStream* pstrm;
						if (CreateStreamOnHGlobal(hmem, FALSE, &pstrm) == S_OK) {
							Bitmap bitmap(pstrm);
							Color color;
							if (bitmap.GetHBITMAP(color, &hbm) != Ok)
								hbm = NULL;
							pstrm->Release();
						}
						GlobalFree(hmem);
					}
				}
			}
		}
	}
	return hbm;
}

HIMAGELIST ImageList_LoadFromResourceData(UINT idResource, int itemWidth, COLORREF clrMask)
{
	HIMAGELIST him = NULL;
	HBITMAP hbm = LoadBitmapFromResourceData(RESINST, RT_RCDATA, idResource);
	if (hbm) {
		BITMAP bm;
		GetObject(hbm, sizeof(bm), &bm);

		UINT flag = ILC_MASK;
		if (bm.bmBitsPixel == 4)		flag |= ILC_COLOR4;
		else if (bm.bmBitsPixel == 8)	flag |= ILC_COLOR8;
		else if (bm.bmBitsPixel == 16)	flag |= ILC_COLOR16;
		else if (bm.bmBitsPixel == 24)	flag |= ILC_COLOR24;
		else if (bm.bmBitsPixel == 32)	flag |= ILC_COLOR32;
		else							flag |= ILC_COLORDDB;

		him = ImageList_Create(itemWidth, bm.bmHeight, flag, bm.bmWidth / itemWidth, 0);
		if (him) {
			if (bm.bmBitsPixel != 32)
				ImageList_AddMasked(him, hbm, clrMask);
			else
				ImageList_Add(him, hbm, NULL);
		}
		DeleteObject(hbm);
	}
	return him;
}
