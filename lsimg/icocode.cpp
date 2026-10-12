//////////////////////////////////////////////////////////////////////
// icocode.cpp


#include "stdafx.h"
#include "shellapi.h"
#include "lsimg.h"
#include "common.h"
#include "filemap.h"

#define MAXICONS		256

#pragma pack(1)

typedef struct tagICONENTRY
{
	BYTE width;
	BYTE height;
	BYTE colors;
	BYTE reserved;
	WORD planes;
	WORD bits;
	DWORD size;
	DWORD offset;
	HICON hicon;
} ICONENTRY;

typedef struct tagICONFILE
{
	WORD reserved;
	WORD resource_type;
	WORD count;
	ICONENTRY dir[MAXICONS];
} ICONFILE;

#pragma pack()

static HDIB __fastcall ico_load_core(HICON hicon)
{
	HDIB hdib = NULL;
	if (hicon) {
		ICONINFO ii;
		if (GetIconInfo(hicon, &ii)) {
			BITMAP bm;
			if (GetObject(ii.hbmColor, sizeof(BITMAP), &bm)) {
				HDC hdc = GetDC(NULL);
				HBITMAP hbm = CreateCompatibleBitmap(hdc, bm.bmWidth, bm.bmHeight);
				HDC hdcw = CreateCompatibleDC(hdc);
				ReleaseDC(NULL, hdc);

				HBITMAP hold = SelectBitmap(hdcw, hbm);
				PatBlt(hdcw, 0, 0, bm.bmWidth, bm.bmHeight, WHITENESS);
				DrawIcon(hdcw, 0, 0, hicon);
				SelectBitmap(hdcw, hold);
				hdib = dib_createfrombitmap(hbm, NULL);
				DeleteObject(hbm);
				DeleteDC(hdcw);
			}
			if (ii.hbmColor) DeleteObject(ii.hbmColor);
			if (ii.hbmMask) DeleteObject(ii.hbmMask);
		}
	}
	return hdib;
}

extern "C"
HDIB WINAPI imgIcoLoad(LPCTSTR pszName)
{
	HDIB hdib = NULL;
	HICON hicon = (HICON)LoadImage(ghInst, pszName, IMAGE_ICON, 0, 0, LR_DEFAULTSIZE | LR_LOADFROMFILE);
	if (hicon) {
		hdib = ico_load_core(hicon);
		DestroyIcon(hicon);
	}
	return hdib;
}
