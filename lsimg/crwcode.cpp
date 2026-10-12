////////////////////////////////////////////////////////////////////////
// crwcode.cpp

#include "stdafx.h"
#include <shlwapi.h>
#include "lsimg.h"
#include "common.h"
#include "..\lspexif\lspexifapi.h"

extern "C" HDIB WINAPI imgJpgLoadMem(HGLOBAL hmem, int colors, int scale);
extern "C" HDIB WINAPI imgDibLoadMem(HANDLE hmem);

extern "C"
HDIB WINAPI zzzimgCrwLoad(LPCTSTR pszFile)
{
	TCHAR dll[MAX_PATH * 2];
	GetModuleFileName(ghInst, dll, ARRAYSIZE(dll));
	PathRemoveFileSpec(dll);
	PathAppend(dll, _T("lspexif.dll"));

	HDIB hdib = NULL;
	HMODULE hmod = LoadLibraryEx(dll, NULL, 0);
	if (hmod) {
		BOOL (WINAPI* pfn)(LPCTSTR, IStream*);
		(FARPROC&)pfn = GetProcAddress(hmod, "ExifGetPrimaryImage");
		if (pfn) {
			IStream* pstrm;
			if (CreateStreamOnHGlobal(NULL, TRUE, &pstrm) == S_OK) {
				if ((*pfn)(pszFile, pstrm)) {
					HGLOBAL h;
					if (GetHGlobalFromStream(pstrm, &h) == S_OK) {
						WORD* pw = (WORD*)GlobalLock(h);
						if (*pw == 0x4d42)
							hdib = imgDibLoadMem(h);
						else if (*pw == 0xd8ff)
							hdib = imgJpgLoadMem(h, 0, 1);
						GlobalUnlock(h);
					}
				}
				pstrm->Release();
			}
		}
		FreeLibrary(hmod);
	}
	return hdib;
}

extern "C"
HDIB WINAPI imgCrwLoad(LPCTSTR pszFile)
{
	TCHAR dll[MAX_PATH * 2];
	GetModuleFileName(ghInst, dll, ARRAYSIZE(dll));
	PathRemoveFileSpec(dll);
	PathAppend(dll, _T("lspexif.dll"));

	HDIB hdib = NULL;
	IStream* pstrm;
	if (CreateStreamOnHGlobal(NULL, TRUE, &pstrm) == S_OK) {
		HMODULE hmod = LoadLibraryEx(dll, NULL, 0);
		if (hmod) {
			int nOrientation = ORIENTATION_NONE;
			BOOL bOK = FALSE;

			BOOL (WINAPI* pfn)(LPCTSTR, IStream*, int*, BOOL, LPTSTR, int);
			(FARPROC&)pfn = GetProcAddress(hmod, "ExifGetThumbnailImage");
			if (pfn)
				bOK = pfn(pszFile, pstrm, &nOrientation, TRUE, NULL, 0);
			FreeLibrary(hmod);

			if (bOK) {
				HGLOBAL h;
				if (GetHGlobalFromStream(pstrm, &h) == S_OK) {
					WORD* pw = (WORD*)GlobalLock(h);
					if (*pw == 0x4d42) {
						hdib = imgDibLoadMem(h);
					}
					else if (*pw == 0xd8ff) {
						hdib = imgJpgLoadMem(h, 0, 1);
					}
					GlobalUnlock(h);
				}
			}

			if (hdib) {
				mif image;
				switch (nOrientation) {
				case ORIENTATION_FLIPX:					// 左右反転
					if (image.attachdib(hdib)) {
						GlobalFree(hdib);
						image.mirror(0, 0, image.width(), image.height());
						hdib = image.createdib();
					}
					break;
				case ORIENTATION_ROTATE180:				// 180°回転
					if (image.attachdib(hdib)) {
						image.rotate180(0, 0, image.width(), image.height());
						hdib = image.createdib();
					}
					break;
				case ORIENTATION_FLIPY:					// 上下反転
					if (image.attachdib(hdib)) {
						image.flip(0, 0, image.width(), image.height());
						hdib = image.createdib();
					}
					break;
				case ORIENTATION_ROTATELEFTFLIPY:		// 左へ90°回転して上下反転
					if (image.attachdib(hdib)) {
						image.rotate(TRUE);
						image.flip(0, 0, image.width(), image.height());
						hdib = image.createdib();
					}
					break;
				case ORIENTATION_ROTATERIGHT:			// 右へ90°回転する
					if (image.attachdib(hdib)) {
						image.rotate(FALSE);
						hdib = image.createdib();
					}
					break;
				case ORIENTATION_ROTATERIGHTFLIPY:		// 右へ90°回転して上下反転
					if (image.attachdib(hdib)) {
						image.rotate(FALSE);
						image.flip(0, 0, image.width(), image.height());
						hdib = image.createdib();
					}
					break;
				case ORIENTATION_ROTATELEFT:			// 左へ90°回転する
					if (image.attachdib(hdib)) {
						image.rotate(TRUE);
						hdib = image.createdib();
					}
					break;
				}
			}
		}
		pstrm->Release();
	}
	return hdib;
}

extern "C"
HDIB WINAPI imgDCRawLoad(LPCTSTR pszFile)
{
	TCHAR dll[MAX_PATH * 2];
	GetModuleFileName(ghInst, dll, ARRAYSIZE(dll));
	PathRemoveFileSpec(dll);
	PathAppend(dll, _T("lspdcraw.dll"));

	HDIB hdib = NULL;
	HMODULE hmod = LoadLibraryEx(dll, NULL, 0);
	if (hmod) {
		HDIB (WINAPI* pfn)(LPCTSTR);
#ifdef UNICODE
		(FARPROC&)pfn = GetProcAddress(hmod, "DCRawConvertW");
#else
		(FARPROC&)pfn = GetProcAddress(hmod, "DCRawConvertA");
#endif
		if (pfn) {
			hdib = (*pfn)(pszFile);
		}
		FreeLibrary(hmod);
	}

	if (!hdib) hdib = imgCrwLoad(pszFile);

	return hdib;
}

