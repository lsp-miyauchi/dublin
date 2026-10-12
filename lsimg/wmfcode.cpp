#include "stdafx.h"
#include <ocidl.h>
#include <olectl.h>
#include "lsimg.h"
#include "common.h"

#define HIMETRIC_INCH	2540

static TCHAR sszOleAut[] = _T("oleaut32.dll");

extern "C"
HDIB WINAPI imgWmfLoadMem(HGLOBAL hmem, DWORD dwSize)
{
	BOOL bLoaded = FALSE;
	HMODULE hmod = GetModuleHandle(sszOleAut);
	if (!hmod) {
		hmod = LoadLibrary(sszOleAut);
		if (!hmod)
			return NULL;
		bLoaded = TRUE;
	}

	HRESULT (WINAPI* pfn)(IStream*, LONG, BOOL, REFIID, VOID**);
	(FARPROC&)pfn = GetProcAddress(hmod, "OleLoadPicture");
	if (!pfn) {
		if (bLoaded)
			FreeLibrary(hmod);
		return NULL;
	}

	HDIB hdib = NULL;

	// create IStream* from global memory
	LPSTREAM pstm;
	HRESULT hr = CreateStreamOnHGlobal(hmem, TRUE, &pstm);
	if (SUCCEEDED(hr)) {
		// Create IPicture from image file
		LPPICTURE pPict;
		hr = (*pfn)(pstm, dwSize, FALSE, IID_IPicture, (LPVOID*)&pPict);
		if (SUCCEEDED(hr)) {
			long hw, hh;
			pPict->get_Width(&hw);
			pPict->get_Height(&hh);

			// convert himetric to pixels
			HDC hdc = CreateDC(_T("DISPLAY"), NULL, NULL, NULL);
			HDC hdcw = CreateCompatibleDC(hdc);
			DeleteDC(hdc);

			int width = MulDiv(hw, GetDeviceCaps(hdcw, LOGPIXELSX), HIMETRIC_INCH);
			int height = MulDiv(hh, GetDeviceCaps(hdcw, LOGPIXELSY), HIMETRIC_INCH);

			HDIB h = dib_createsolid(width, height, RGB(255, 255, 255));
			HBITMAP hbm;
			HPALETTE hpal;
			imgCreateBitmapFromDIB(h, &hbm, &hpal);
			if (hpal)
				DeleteObject(hpal);
			GlobalFree(h);

			HBITMAP hbmOld = SelectBitmap(hdcw, hbm);

			// display picture using IPicture::Render
			CRect rc(0, 0, width, height);
			pPict->Render(hdcw, 0, 0, width, height, 0, hh, hw, -hh, &rc);
			pPict->get_hPal((OLE_HANDLE*)&hpal);

			SelectBitmap(hdcw, hbmOld);
			hdib = dib_createfrombitmap(hbm, hpal);
			DeleteObject(hbm);
			DeleteObject(hpal);
			DeleteDC(hdcw);

			pPict->Release();
		}
		pstm->Release();
	}

	if (bLoaded)
		FreeLibrary(hmod);
	return hdib;
}

extern "C"
HDIB WINAPI imgWmfLoad(LPCTSTR fullname)
{
	MemStream* pms = _mfcreate(fullname);
	if (pms) {
		size_t size;
		HGLOBAL hmem = _mfdetach(pms, &size);
		HDIB hdib = imgWmfLoadMem(hmem, size);
		GlobalFree(hmem);
		return hdib;
	}
	return NULL;
}

