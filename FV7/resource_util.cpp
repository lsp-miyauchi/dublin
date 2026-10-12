// image_util.cpp
#include "stdafx.h"
#include "fv7.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
//@ #define new DEBUG_NEW
#endif


#pragma pack(1)
struct __ToolBarData {
	WORD wVersion;
	WORD wWidth;
	WORD wHeight;
	WORD wItemCount;
	//WORD aItems[wItemCount]
	WORD* items() { return (WORD*)(this + 1); }
};
#pragma pack()

int GetToolbarCommandFromResource(UINT idToolbar, CSimpleArray<UINT>& array)
{
	array.RemoveAll();
	HRSRC hRsrc = ::FindResource(RESINST, MAKEINTRESOURCE(idToolbar), RT_TOOLBAR);
	if (hRsrc) {
		HGLOBAL hGlobal = ::LoadResource(RESINST, hRsrc);
		if (hGlobal) {
			__ToolBarData* pData = (__ToolBarData*)::LockResource(hGlobal);
			if (pData) {
				ATLASSERT(pData->wVersion == 1);
				WORD* pItems = pData->items();
				for (WORD i = 0; i < pData->wItemCount; i++) {
					if (pItems[i] != 0)
						array.Add(pItems[i]);
				}
			}
		}
	}
	return array.GetSize();
}

BOOL SetBitmapToMenuItem(HMENU hmenu, UINT id, HBITMAP hbm)
{
	CMenuItemInfo mii;
	mii.fMask = MIIM_BITMAP;
	mii.hbmpItem = hbm;
	return ::SetMenuItemInfo(hmenu, id, FALSE, &mii);
}

HBITMAP CreateBitmapFromImageList(HIMAGELIST him, int iImage)
{
	CClientDC dc(NULL);
	CDC dcw;
	dcw.CreateCompatibleDC(dc);

	int cx, cy;
	ImageList_GetIconSize(him, &cx, &cy);

	// Create 32-bit bitmap
	BITMAPINFO bi = { 0 };
	bi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
	bi.bmiHeader.biWidth = cx;
	bi.bmiHeader.biHeight = cy;
	bi.bmiHeader.biPlanes = 1;
	bi.bmiHeader.biBitCount = 32;
	bi.bmiHeader.biCompression = BI_RGB;
	bi.bmiHeader.biSizeImage = 0;
	bi.bmiHeader.biXPelsPerMeter = 0;
	bi.bmiHeader.biYPelsPerMeter = 0;
	bi.bmiHeader.biClrUsed = 0;
	bi.bmiHeader.biClrImportant = 0;
	HBITMAP hBitmap = ::CreateDIBSection(dc, &bi, DIB_RGB_COLORS, NULL, NULL, 0);
	ATLASSERT(hBitmap != NULL);

	// Select bitmap into target DC and draw from image list to it
	if (hBitmap != NULL) {
		HBITMAP temp = dcw.SelectBitmap(hBitmap);
		IMAGELISTDRAWPARAMS ildp = { 0 };
		ildp.cbSize = sizeof(IMAGELISTDRAWPARAMS);
		ildp.himl = him;
		ildp.i = iImage;
		ildp.hdcDst = dcw;
		ildp.x = 0;
		ildp.y = 0;
		ildp.cx = 0;
		ildp.cy = 0;
		ildp.xBitmap = 0;
		ildp.yBitmap = 0;
		ildp.fStyle = ILD_TRANSPARENT;
		ildp.fState = ILS_ALPHA;
		ildp.Frame = 255;
		::ImageList_DrawIndirect(&ildp);
		dcw.SelectBitmap(temp);
	}
	return hBitmap;
}

HBITMAP CloneBitmap(HBITMAP hbm)
{
	if (!hbm) return NULL;

	// ビットマップを複製する
	CClientDC dc(NULL);
	BITMAP bm;
	GetObject(hbm, sizeof(bm), &bm);
	HBITMAP hbmClone = CreateCompatibleBitmap(dc, bm.bmWidth, bm.bmHeight);
	if (hbmClone) {
		CDC dc1; dc1.CreateCompatibleDC(dc);
		CDC dc2; dc2.CreateCompatibleDC(dc);
		HBITMAP hbm1 = dc1.SelectBitmap(hbmClone);
		HBITMAP hbm2 = dc2.SelectBitmap(hbm);
		dc1.BitBlt(0, 0, bm.bmWidth, bm.bmHeight, dc2, 0, 0, SRCCOPY);

		dc2.SelectBitmap(hbm2);
		dc1.SelectBitmap(hbm1);
	}
	return hbmClone;
}

HCURSOR CreateCursorFromIcon(HICON hic, const POINT* pSpot)
{
	HCURSOR hcur = NULL;
	ICONINFO ii;
	if (GetIconInfo(hic, &ii)) {
		ii.fIcon = FALSE;
		ii.xHotspot = pSpot ? pSpot->x : 0;
		ii.yHotspot = pSpot ? pSpot->y : 0;
		hcur = (HCURSOR)::CreateIconIndirect(&ii);
		if (ii.hbmColor) ::DeleteObject(ii.hbmColor);
		if (ii.hbmMask) ::DeleteObject(ii.hbmMask);
	}
	return hcur;
}

HCURSOR ImageList_GetCursor(HIMAGELIST him, int index, UINT flags, const POINT* pSpot)
{
	HCURSOR hcur = NULL;
	HICON hic = ImageList_GetIcon(him, index, flags);
	if (hic) {
		hcur = CreateCursorFromIcon(hic, pSpot);
		DestroyIcon(hic);
	}
	return hcur;
}

void __fastcall SetDialogIcon(HWND hdlg, UINT idIcon)
{
	HINSTANCE hInst = RESINST;
	HICON hic;
	hic = (HICON)LoadImage(hInst, _MKIR(idIcon), IMAGE_ICON,
							GetSystemMetrics(SM_CXSMICON),
							GetSystemMetrics(SM_CYSMICON), LR_DEFAULTCOLOR);
	SendMessage(hdlg, WM_SETICON, FALSE, (LPARAM)hic);
	hic = (HICON)LoadImage(hInst, _MKIR(idIcon), IMAGE_ICON,
							GetSystemMetrics(SM_CXICON),
							GetSystemMetrics(SM_CYICON), LR_DEFAULTCOLOR);
	SendMessage(hdlg, WM_SETICON, TRUE, (LPARAM)hic);
}

void __fastcall RemoveDialogIcon(HWND hdlg)
{
	HICON hic = (HICON)SendMessage(hdlg, WM_GETICON, TRUE, 0);
	if (hic) DestroyIcon(hic);
	hic = (HICON)SendMessage(hdlg, WM_GETICON, FALSE, 0);
	if (hic) DestroyIcon(hic);
}

HIMAGELIST __fastcall MakeDummyImageList(int cx, int cy)
{
	const COLORREF clrMask = RGB(255, 0, 255);

	// ダミービットマップ作成
	HIMAGELIST him = ImageList_Create(cx, cy, ILC_COLORDDB | ILC_MASK, 1, 0);
	if (him) {
		CClientDC dc(NULL);
		HBITMAP hbm = ::CreateCompatibleBitmap(dc, cx, cy);
		CDC dcw;
		dcw.CreateCompatibleDC(dc);
		hbm = dcw.SelectBitmap(hbm);
		dcw.FillSolidRect(0, 0, cx, cy, clrMask);
		hbm = dcw.SelectBitmap(hbm);
		ImageList_AddMasked(him, hbm, clrMask);
		DeleteObject(hbm);
		ImageList_SetBkColor(him, CLR_NONE);
	}
	return him;
}
