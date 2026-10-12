////////////////////////////////////////////////////////////////////////
// imagelist_sup.cpp

#include "stdafx.h"
#include "lscore.h"
#include "dib.h"
#include "gdiplus_sup.h"
#include "hue-saturation.h"

void* FindLoadLockResource(HINSTANCE hInst, LPCTSTR resType, LPCTSTR resName, DWORD* pSize = NULL)
{
	HRSRC hRsrc = ::FindResource(hInst, resName, resType);
	if (hRsrc) {
		HGLOBAL hmem = ::LoadResource(hInst, hRsrc);
		if (hmem) {
			if (pSize) {
				*pSize = SizeofResource(hInst, hRsrc);
			}
			return ::LockResource(hmem);
		}
	}
	return NULL;
}

IStream* StreamFromResource(HINSTANCE hInst, LPCTSTR resType, LPCTSTR resName, DWORD* pSize)
{
	// リソースからイメージをロードする
	// 元データは bmp, png, jpeg, tiff 等何でもOK
	DWORD sizeResource;
	void* pResource = FindLoadLockResource(hInst, resType, resName, &sizeResource);
	if (pResource) {
		HGLOBAL hmem = GlobalAlloc(GHND, sizeResource);
		if (hmem) {
			memcpy(GlobalLock(hmem), pResource, sizeResource);
			GlobalUnlock(hmem);
			IStream* pstrm;
			if (CreateStreamOnHGlobal(hmem, TRUE, &pstrm) == S_OK) {	// free memory at pstrm release
				if (pSize) {
					*pSize = sizeResource;
				}
				return pstrm;
			}
			GlobalUnlock(hmem);
			GlobalFree(hmem);
		}
	}
	return NULL;
}


static Bitmap* loadimage_core(HINSTANCE hInst, LPCTSTR resType, LPCTSTR resName)
{
	// リソースからイメージをロードする
	// 元データは bmp, png, jpeg, tiff 等何でもOK
	Bitmap* pbm = NULL;
	IStream* pstrm = StreamFromResource(hInst, resType, resName, NULL);
	if (pstrm) {
		pbm = Bitmap::FromStream(pstrm);
		pstrm->Release();
	}
	return pbm;
}

static bool load_dibimage(HINSTANCE hInst, LPCTSTR resType, LPCTSTR resName, CDib& dib)
{
	// リソースからイメージをロードする
	// 元データは bmp, png, jpeg, tiff 等何でもOK
	Bitmap* pbm = loadimage_core(hInst, resType, resName);
	if (pbm) {
		dib.AttachGdiplusBitmap(pbm);
		delete pbm;
	}
	return !dib.isNull();
}

static HBITMAP load_bitmap(HINSTANCE hInst, LPCTSTR resType, LPCTSTR resName)
{
	// リソースからイメージをロードする
	// 元データは bmp, png, jpeg, tiff 等何でもOK
	HBITMAP hbm = NULL;
	Bitmap* pbm = loadimage_core(hInst, resType, resName);
	if (pbm) {
		pbm->GetHBITMAP(Color(0,0,0,0), &hbm);
		delete pbm;
	}
	return hbm;
}


extern "C"
HBITMAP WINAPI lccLoadImage(HINSTANCE hInst, LPCTSTR resType, LPCTSTR resName)
{
	return load_bitmap(hInst, resType, resName);
}

static HIMAGELIST load_ImageList_core(Bitmap* pbm, int cxWidth, COLORREF clrMask)
{
	HIMAGELIST him = NULL;
	HBITMAP hbm;
	if (pbm->GetHBITMAP(Color(0, 0, 0, 0), &hbm) == Ok) {
		int width = pbm->GetWidth();
		int height = pbm->GetHeight();
		PixelFormat fmt = pbm->GetPixelFormat();
		bool alpha = (fmt == PixelFormat32bppARGB || fmt == PixelFormat32bppPARGB);
		UINT flag = alpha ? ILC_COLOR32 : (ILC_COLOR24 | ILC_MASK);
		him = ImageList_Create(cxWidth, height, flag, width / cxWidth, 1);
		if (him) {
			if (alpha) {
				clrMask = CLR_NONE;
				ImageList_SetBkColor(him, clrMask);
			}

			HBITMAP hbmMask = NULL;
			if (!alpha) {
				CDC dcs; dcs.CreateCompatibleDC(0);
				HBITMAP hbms = dcs.SelectBitmap(hbm);

				hbmMask = CreateBitmap(width, height, 1, 1, NULL);
				CDC dcm; dcm.CreateCompatibleDC(0);
				HBITMAP hbmm = dcm.SelectBitmap(hbmMask);

				clrMask = (clrMask != CLR_DEFAULT) ? clrMask : dcs.GetPixel(0, 0);
				dcs.SetBkColor(clrMask);
				dcm.BitBlt(0, 0, width, height, dcs, 0, 0, SRCCOPY);
				dcm.SelectBitmap(hbmm);
				dcs.SelectBitmap(hbms);
			}
			ImageList_Add(him, hbm, hbmMask);
			if (hbmMask) ::DeleteObject(hbmMask);
		}
		::DeleteObject(hbm);
	}
	return him;
}


extern "C"
HIMAGELIST WINAPI lccLoadImageList(HINSTANCE hInst, LPCTSTR resType, LPCTSTR resName, int cxWidth, COLORREF clrMask)
{
	HIMAGELIST him = NULL;
	Bitmap* pbm = loadimage_core(hInst, resType, resName);
	if (pbm) {
		him = load_ImageList_core(pbm, cxWidth, clrMask);
		delete pbm;
	}
	return him;
}


extern "C"
HIMAGELIST WINAPI lccLoadAlphaImageList(HINSTANCE hInst, LPCTSTR resType, LPCTSTR resName, int cxWidth, BOOL bReverse)
{
	HIMAGELIST him = NULL;
	Bitmap* pbm = loadimage_core(hInst, resType, resName);
	if (pbm) {
		if (bReverse) {
			int width = pbm->GetWidth();
			int height = pbm->GetHeight();
			Rect rc(0, 0, width, height);
			BitmapData bd;
			if (pbm->LockBits(&rc, ImageLockModeRead | ImageLockModeWrite, PixelFormat32bppARGB, &bd) == Ok) {
				HueSaturation hs;
				hs.init_table(0, -180, 0, 0);
				int len;
				for (int y = 0; y < height; y++) {
					ARGBPixel* p = (ARGBPixel*)((BYTE*)bd.Scan0 + bd.Stride * y);
					len = width;
					while (len-- > 0) {
						p->invert();
						hs.pixel_convert(p);
						p++;
					}
				}
				pbm->UnlockBits(&bd);
			}
		}
		him = load_ImageList_core(pbm, cxWidth, CLR_NONE);
		delete pbm;
	}
	return him;
}

//////////////////////////////////////////////////////////////////////////
//

static HWND create_toolbar(HWND hWndParent, HIMAGELIST him, LSCTOOLBARDATA* ptbData, DWORD wndStyle, UINT idCtrl)
{
	// リソースからイメージをロードしてツールバーを作成する
	// 元データは bmp, png, jpeg, tiff 等何でもOK
	if (ptbData == NULL) return NULL;
	ATLASSERT(ptbData->wVersion == 1);

	WORD* pItems = ptbData->items();
	int nItems = ptbData->wItemCount;
	TBBUTTON* pTBBtn = new TBBUTTON[nItems];
	ATLASSERT(pTBBtn != NULL);
	if (pTBBtn == NULL) return NULL;

	const int cxSeparator = 8;

	int numImages = 0;
	for (int i = 0; i < ptbData->wItemCount; ++i) {
		if (pItems[i] != 0) {
			pTBBtn[i].iBitmap = numImages++;
			pTBBtn[i].idCommand = pItems[i];
			pTBBtn[i].fsState = TBSTATE_ENABLED;
			pTBBtn[i].fsStyle = TBSTYLE_BUTTON;
			pTBBtn[i].dwData = 0;
			pTBBtn[i].iString = 0;
		}
		else {
			pTBBtn[i].iBitmap = cxSeparator;
			pTBBtn[i].idCommand = 0;
			pTBBtn[i].fsState = 0;
			pTBBtn[i].fsStyle = TBSTYLE_SEP;
			pTBBtn[i].dwData = 0;
			pTBBtn[i].iString = 0;
		}
	}

	wndStyle |= ATL_SIMPLE_TOOLBAR_PANE_STYLE;
	if (wndStyle & TBSTYLE_LIST) {
		wndStyle &= ~TBSTYLE_FLAT;
	}
	HWND hWnd = ::CreateWindowEx(0, TOOLBARCLASSNAME, NULL, wndStyle, 0, 0, 100, 100, hWndParent, (HMENU)LongToHandle(idCtrl), MODINST, NULL);
	if (hWnd != NULL) {
		::SendMessage(hWnd, TB_BUTTONSTRUCTSIZE, sizeof(TBBUTTON), 0L);
		// check if font is taller than our bitmaps
		CFontHandle font = GetWindowFont(hWnd);
		if (font.IsNull())
			font = GetStockFont(DEFAULT_GUI_FONT);
		LOGFONT lf = { 0 };
		font.GetLogFont(lf);
		WORD cyFontHeight = (WORD)abs(lf.lfHeight);

		::SendMessage(hWnd, TB_SETIMAGELIST, 0, (LPARAM)him);
		::SendMessage(hWnd, TB_ADDBUTTONS, nItems, (LPARAM)pTBBtn);
		::SendMessage(hWnd, TB_SETBITMAPSIZE, 0, MAKELPARAM(ptbData->wWidth, max(ptbData->wHeight, cyFontHeight)));

		const int cxyButtonMargin = 7;
		::SendMessage(hWnd, TB_SETBUTTONSIZE, 0, MAKELPARAM(ptbData->wWidth + cxyButtonMargin, max(ptbData->wHeight, cyFontHeight) + cxyButtonMargin));
	}
	delete [] pTBBtn;
	return hWnd;
}

/*
 * lccGetToobarResourceData: 戻り値のメモリは CoTaskMemFreeで開放すること
 */
extern "C"
const LSCTOOLBARDATA* WINAPI lccGetToobarResourceData(HINSTANCE hInst, LPCTSTR resName)
{
	void* memptr = NULL;
	DWORD len;
	void* pResource = FindLoadLockResource(hInst, RT_TOOLBAR, resName, &len);
	if (pResource) {
		memptr = CoTaskMemAlloc((size_t)len);
		if (memptr) {
			memcpy(memptr, pResource, len);
		}
	}
	return (const LSCTOOLBARDATA*)memptr;
}

HWND lccCreateToolBar(HWND hWndParent, HINSTANCE hInst, LPCTSTR resName, DWORD wndStyle, UINT idCtrl)
{
	// リソースからイメージをロードしてツールバーを作成する
	// 元データは bmp, png, jpeg, tiff 等何でもOK
	HWND hwnd = NULL;
	LSCTOOLBARDATA* ptbData = (LSCTOOLBARDATA*)FindLoadLockResource(hInst, RT_TOOLBAR, resName);
	if (ptbData) {
		HIMAGELIST him = lccLoadAlphaImageList(hInst, RT_RCDATA, resName, ptbData->wWidth, FALSE);
		if (him) {
			hwnd = create_toolbar(hWndParent, him, ptbData, wndStyle, idCtrl);
			if (!hwnd) {
				ImageList_Destroy(him);
			}
		}
	}
	return hwnd;
}

extern "C"
HIMAGELIST WINAPI lccAddToolBarDisabledImageList(HWND hbar, HINSTANCE hInst, LPCTSTR resName)
{
	HIMAGELIST him = (HIMAGELIST)::SendMessage(hbar, TB_GETIMAGELIST, 0, 0);
	if (him) {
		int cx, cy;
		if (::ImageList_GetIconSize(him, &cx, &cy)) {
			him = lccLoadImageList(RESINST, RT_RCDATA, resName, cx, CLR_DEFAULT);
			if (him) {
				return (HIMAGELIST)::SendMessage(hbar, TB_SETDISABLEDIMAGELIST, 0, (LPARAM)him);
			}
		}
	}
	return _INVALID_HIMAGELIST;
}

extern "C"
HIMAGELIST WINAPI lccAddToolBarHotImageList(HWND hbar, HINSTANCE hInst, LPCTSTR resName)
{
	HIMAGELIST him = (HIMAGELIST)::SendMessage(hbar, TB_GETIMAGELIST, 0, 0);
	if (him) {
		int cx, cy;
		if (::ImageList_GetIconSize(him, &cx, &cy)) {
			him = lccLoadImageList(RESINST, RT_RCDATA, resName, cx, CLR_DEFAULT);
			if (him) {
				return (HIMAGELIST)::SendMessage(hbar, TB_SETHOTIMAGELIST, 0, (LPARAM)him);
			}
		}
	}
	return _INVALID_HIMAGELIST;
}

extern "C"
void WINAPI lccReplaceToolBarImageList(HWND hbar, HINSTANCE hInst, LPCTSTR resName)
{
	LSCTOOLBARDATA* ptbData = (LSCTOOLBARDATA*)FindLoadLockResource(hInst, RT_TOOLBAR, resName);
	if (ptbData) {
		CToolBarCtrl bar = hbar;
		int i, numButtons = bar.GetButtonCount();
		TBBUTTON* ptbb = new TBBUTTON[numButtons];
		for (i = numButtons - 1; i >= 0; i--) {
			bar.GetButton(i, &ptbb[i]);
			bar.DeleteButton(i);
		}

		// check if font is taller than our bitmaps
		CFontHandle font = bar.GetFont();
		if (font.IsNull())
			font = (HFONT)GetStockObject(DEFAULT_GUI_FONT);
		LOGFONT lf = { 0 };
		font.GetLogFont(lf);
		int bmy = max(ptbData->wHeight, abs(lf.lfHeight));

		HIMAGELIST him = lccLoadImageList(hInst, RT_RCDATA, resName, ptbData->wWidth, CLR_DEFAULT);
		ATLASSERT(him != NULL);
		HIMAGELIST himPrev = bar.SetImageList(him);
		if (himPrev) {
			ImageList_Destroy(himPrev);
		}
		bar.AddButtons(numButtons, ptbb);
		bar.SetBitmapSize(ptbData->wWidth, bmy);

		const int cxyButtonMargin = 7;
		bar.SetButtonSize(ptbData->wWidth + cxyButtonMargin, bmy + cxyButtonMargin);
		delete [] ptbb;
	}
}
