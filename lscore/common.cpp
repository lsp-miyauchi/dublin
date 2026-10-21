///////////////////////////////////////////////////////
// common.cpp

#include "stdafx.h"
#include "gdiplus_sup.h"
#include "resource.h"
#include "idlist.h"
#include "lscore.h"
#include "fnc.h"
#include "common.h"


void FillShadeRect(HDC hdc, CRect& rc, COLORREF crFrom, COLORREF crTo, BOOL bVert)
{
	TRIVERTEX vert[2];
	GRADIENT_RECT grc;

	vert[0].x     = rc.left;
	vert[0].y     = rc.top;
	vert[0].Red   = GetRValue(crFrom) << 8;
	vert[0].Green = GetGValue(crFrom) << 8;
	vert[0].Blue  = GetBValue(crFrom) << 8;
	vert[0].Alpha = 0x0000;

	vert[1].x     = rc.right;
	vert[1].y     = rc.bottom;
	vert[1].Red   = GetRValue(crTo) << 8;
	vert[1].Green = GetGValue(crTo) << 8;
	vert[1].Blue  = GetBValue(crTo) << 8;
	vert[1].Alpha = 0x0000;

	grc.UpperLeft  = 0;
	grc.LowerRight = 1;
	GradientFill(hdc, vert, 2, &grc, 1, bVert ? GRADIENT_FILL_RECT_V : GRADIENT_FILL_RECT_H);
}

void FillShadeRectEx(HDC hdc, CRect& rc, COLORREF* crs, int numcolors, BOOL bVert)
{
	if (!crs)
		return;

	CDCHandle dc = hdc;
	if (numcolors == 1) {
		dc.FillSolidRect(&rc, *crs);
		return;
	}

	TRIVERTEX vert[2];
	GRADIENT_RECT grc;

	double dt = bVert ? (double)rc.Height() / (numcolors - 1): (double)rc.Width() / (numcolors - 1);
	for (int i = 0; i < numcolors - 1; i++) {
		if (bVert) {
			vert[0].x = rc.left;
			vert[0].y = (long)(rc.top + dt * i);
		}
		else {
			vert[0].x = (long)(rc.left + dt * i);
			vert[0].y = rc.top;
		}
		vert[0].Red   = GetRValue(*crs) << 8;
		vert[0].Green = GetGValue(*crs) << 8;
		vert[0].Blue  = GetBValue(*crs) << 8;
		vert[0].Alpha = 0x0000;

		if (bVert) {
			vert[1].x = rc.right;
			if (i + 2 == numcolors)
				vert[1].y = rc.bottom;
			else
				vert[1].y = (long)(rc.top + dt * (i + 1));
		}
		else {
			if (i + 2 == numcolors)
				vert[1].x = rc.right;
			else
				vert[1].x = (long)(rc.left + dt * (i + 1));
			vert[1].y = rc.bottom;
		}
		vert[1].Red   = GetRValue(*(crs + 1)) << 8;
		vert[1].Green = GetGValue(*(crs + 1)) << 8;
		vert[1].Blue  = GetBValue(*(crs + 1)) << 8;
		vert[1].Alpha = 0x0000;

		grc.UpperLeft  = 0;
		grc.LowerRight = 1;
		dc.GradientFill(vert, 2, &grc, 1, bVert ? GRADIENT_FILL_RECT_V : GRADIENT_FILL_RECT_H);

		crs++;
	}
}

COLORREF __fastcall mix_color(COLORREF clr1, COLORREF clr2, int clr1_per)
{
	int r = (clr1_per * GetRValue(clr1) + (100 - clr1_per) * GetRValue(clr2)) / 100;
	int g = (clr1_per * GetGValue(clr1) + (100 - clr1_per) * GetGValue(clr2)) / 100;
	int b = (clr1_per * GetBValue(clr1) + (100 - clr1_per) * GetBValue(clr2)) / 100;
	return RGB(r, g, b);
}

void _FillG(HDC hdc, CRect& rc, COLORREF cr)
{
	COLORREF crB = RGB(255, 255, 255);
	COLORREF cr1, cr2;
	cr1 = mix_color(crB, cr, 86);
	cr2 = mix_color(crB, cr, 40);
	CRect r = rc;
	r.bottom = rc.top + (int)(2.0F * ((float)rc.Height() / 5.0F));
	FillShadeRect(hdc, r, cr1, cr2, TRUE);

	cr2 = mix_color(crB, cr, 50);
	r.top = r.bottom;
	r.bottom = rc.bottom;
	FillShadeRect(hdc, r, cr, cr2, TRUE);
}


void draw_rect(HDC hdc, CRect& rcPane, int mode)
{
	CRect rc(rcPane);

	CDCHandle dc = hdc;
	if (mode == SEL_POP)
		dc.DrawEdge(&rc, BDR_RAISEDINNER, BF_TOPLEFT | BF_BOTTOMRIGHT);
	else if (mode == SEL_PUSH)
		dc.DrawEdge(&rc, BDR_SUNKENOUTER, BF_TOPLEFT | BF_BOTTOMRIGHT);
	else if (mode == SEL_BORDER) {
		COLORREF rgb = GetSysColor(COLOR_BTNSHADOW);
		dc.FillSolidRect(rc.left, rc.top, rc.Width()-1, 1, rgb);
		dc.FillSolidRect(rc.left, rc.top, 1, rc.Height()-1, rgb);
		dc.FillSolidRect(rc.right-1, rc.top, 1,rc.Height()-1, rgb);
		dc.FillSolidRect(rc.left, rc.bottom-1, rc.Width(), 1, rgb);
	}
	else {
		COLORREF rgb = GetSysColor(COLOR_BTNFACE);
		dc.FillSolidRect(rc.left, rc.top, rc.Width()-1, 1, rgb);
		dc.FillSolidRect(rc.left, rc.top, 1, rc.Height()-1, rgb);
		dc.FillSolidRect(rc.right-1, rc.top, 1,rc.Height()-1, rgb);
		dc.FillSolidRect(rc.left, rc.bottom-1, rc.Width(), 1, rgb);
	}
}

void draw_rect(HDC hdc, LPRECT prc, COLORREF rgbBorder, COLORREF rgbInner)
{
	CRect rc = *prc;
	CDCHandle dc = hdc;
	if (rgbBorder != INVALID_COLOR) {
		dc.FillSolidRect(rc.left, rc.top, 1, rc.Height(), rgbBorder);
		dc.FillSolidRect(rc.left, rc.top, rc.Width(), 1, rgbBorder);
		dc.FillSolidRect(rc.right - 1, rc.top, 1, rc.Height(), rgbBorder);
		dc.FillSolidRect(rc.left, rc.bottom - 1, rc.Width(), 1, rgbBorder);
	}
	if (rgbInner != INVALID_COLOR) {
		rc.InflateRect(-1, -1);
		HBRUSH hbr = dc.SelectBrush(CreateSolidBrush(rgbInner));
		dc.PatBlt(rc.left, rc.top, rc.Width(), rc.Height(), PATCOPY);
		DeleteObject(dc.SelectBrush(hbr));
	}
}

COLORREF __fastcall LightenColor(const COLORREF crColor, UINT nIncreaseVal)
{
	BYTE byRed = GetRValue(crColor);
	BYTE byGreen = GetGValue(crColor);
	BYTE byBlue = GetBValue(crColor);

	if ((byRed + nIncreaseVal) <= 255)
		byRed = BYTE(byRed + nIncreaseVal);
	if ((byGreen + nIncreaseVal) <= 255)
		byGreen = BYTE(byGreen + nIncreaseVal);
	if ((byBlue + nIncreaseVal) <= 255)
		byBlue = BYTE(byBlue + nIncreaseVal);

	return RGB(byRed, byGreen, byBlue);
}

COLORREF __fastcall DarkenColor(const COLORREF crColor, UINT nReduceVal)
{
	BYTE byRed = GetRValue(crColor);
	BYTE byGreen = GetGValue(crColor);
	BYTE byBlue = GetBValue(crColor);

	if (byRed >= nReduceVal)
		byRed = BYTE(byRed - nReduceVal);
	if (byGreen >= nReduceVal)
		byGreen = BYTE(byGreen - nReduceVal);
	if (byBlue >= nReduceVal)
		byBlue = BYTE(byBlue - nReduceVal);

	return RGB(byRed, byGreen, byBlue);
}

void __fastcall rpl_str(LPTSTR pText, LPCTSTR pSrc, LPCTSTR pDst)
{
	int lenSrc = lstrlen(pSrc);
	int lenDst = lstrlen(pDst);
	LPTSTR p = pText;
	LPTSTR rep = new TCHAR[lstrlen(pText) + MAX_PATH * 4];
	LPTSTR q = rep;
	while (*p) {
		if (*p != *pSrc || StrCmpN(p, pSrc, lenSrc))
			*q++ = *p++;
		else {
			lstrcpy(q, pDst);
			q += lenDst;
			p += lenSrc;
		}
	}
	*q = '\0';
	lstrcpy(pText, rep);
	delete [] rep;
}

BOOL __fastcall _pathfileexists(LPCTSTR pszName)
{
	WIN32_FILE_ATTRIBUTE_DATA wfad;
	return GetFileAttributesEx(pszName, GetFileExInfoStandard, &wfad);
}

BOOL __fastcall _pathisfile(LPCTSTR pszName)
{
	WIN32_FILE_ATTRIBUTE_DATA wfad;
	return (GetFileAttributesEx(pszName, GetFileExInfoStandard, &wfad) &&
			!(wfad.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY));

}

BOOL __fastcall _pathisdirectory(LPCTSTR pszName)
{
	WIN32_FILE_ATTRIBUTE_DATA wfad;
	return (pszName && *pszName &&
			GetFileAttributesEx(pszName, GetFileExInfoStandard, &wfad) &&
			(wfad.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY));
}

BOOL __fastcall parselinkpath(LPCTSTR pszName, LPTSTR pszTarget, int cchMax)
{
	BOOL bOK = FALSE;
	IShellLink* psl;
	if (SUCCEEDED(CoCreateInstance(CLSID_ShellLink, NULL,
						CLSCTX_INPROC_SERVER, IID_IShellLink, (LPVOID*)&psl)))
	{
		IPersistFile* ppf;
		if (SUCCEEDED(psl->QueryInterface(IID_IPersistFile, (LPVOID*)&ppf)))
		{
			if (ppf->Load(pszName, STGM_READ) == S_OK)
			{
				WIN32_FIND_DATA wfd;
				if (psl->GetPath(pszTarget, cchMax, &wfd, 0) == NOERROR)
					bOK = TRUE;
			}
			ppf->Release();
		}
		psl->Release();
	}
	return bOK;
}

BOOL getlinkinfo(LPCTSTR pszName, CString& target)
{
	BOOL bOK = FALSE;
	CComPtr<IShellLink> psl;
	if (psl.CoCreateInstance(CLSID_ShellLink) == S_OK) {
		CComQIPtr<IPersistFile, &IID_IPersistFile> ppf(psl);
		if (ppf.p != NULL) {
			if (ppf->Load(pszName, STGM_READ) == S_OK) {
				LPITEMIDLIST pidl;
				if (NOERROR == psl->GetIDList(&pidl)) {
					TCHAR buf[MAX_PATH * 2];
					if (SHGetPathFromIDList(pidl, buf)) {
						target = buf;
						bOK = TRUE;
					}
					else
						bOK = idl_GetName(pidl, SHGDN_FORPARSING | SHGDN_NORMAL, target);
					CoTaskMemFree(pidl);
				}
			}
		}
	}
	return bOK;
}

HGLOBAL __fastcall dupmemhandle(HGLOBAL hsrc)
{
	SIZE_T size = GlobalSize(hsrc);
	HGLOBAL hdst = GlobalAlloc(GMEM_SHARE|GMEM_MOVEABLE, size);
	if (hdst) {
		void* ps = GlobalLock(hsrc);
		void* pd = GlobalLock(hdst);
		memcpy(pd, ps, size);
		GlobalUnlock(hsrc);
		GlobalUnlock(hdst);
	}
	return hdst;
}

HWND __fastcall gettopparent(HWND hwnd)
{
	HWND hpar = nullptr;
	while (hwnd) {
		hwnd = GetParent(hwnd);
		if (hwnd) hpar = hwnd;
	}
	return hpar;
}

HBITMAP CreateBitmapFromImageList(HIMAGELIST him, int iImage)
{
	CClientDC dc(nullptr);
	CDC dcw;
	dcw.CreateCompatibleDC(dc);

	int cx;
	int cy;
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

HBITMAP LoadImageFromResourceData(HINSTANCE hInst, LPCTSTR typeResource, UINT idResource)
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
