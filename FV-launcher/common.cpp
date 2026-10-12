////////////////////////////////////////////////////////////////////////
// common.cpp

#include "stdafx.h"
#include "common.h"

COLORREF mix_color(COLORREF clr1, COLORREF clr2, int clr1_per)
{
	int r = (clr1_per * GetRValue(clr1) + (100 - clr1_per) * GetRValue(clr2)) / 100;
	int g = (clr1_per * GetGValue(clr1) + (100 - clr1_per) * GetGValue(clr2)) / 100;
	int b = (clr1_per * GetBValue(clr1) + (100 - clr1_per) * GetBValue(clr2)) / 100;
	return RGB(r, g, b);
}

void fill_shade_rect(HDC hdc, const CRect& rc, COLORREF crFrom, COLORREF crTo)
{
#ifdef USE_GRADIENTFILL
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
	GradientFill(hdc, vert, 2, &grc, 1, GRADIENT_FILL_RECT_V);
#else
	Graphics gr(hdc);
	LinearGradientBrush br(
							Point(rc.left, rc.top),
							Point(rc.left, rc.bottom), 
							Color(255, GetRValue(crFrom), GetGValue(crFrom), GetBValue(crFrom)),
							Color(255, GetRValue(crTo),   GetGValue(crTo),   GetBValue(crTo))
						);
	gr.FillRectangle(&br, rc.left, rc.top, rc.Width(), rc.Height());
#endif
}

void fill_glass_rect(HDC hdc, const CRect& rect, bool bHot, bool bPushedOrSelected)
{
	CDCHandle dc = hdc;
	CRect rc = rect;
	COLORREF clrBack = dc.GetBkColor();
	dc.FillSolidRect(&rc, GetSysColor(COLOR_3DSHADOW));
	rc.InflateRect(-1, -1);
	dc.FillSolidRect(&rc, GetSysColor(COLOR_WINDOW));
	rc.InflateRect(-1, -1);

	COLORREF clr[4];
	COLORREF clrbase = mix_color(GetSysColor(COLOR_HIGHLIGHT), GetSysColor(COLOR_3DFACE), 70);
	int delta;
	if (bPushedOrSelected)	delta = 60;
	else if (bHot)			delta = 120;
	else					delta = 0;
	clr[0] = ColorAdjustLuma(clrbase, 800 + delta, TRUE);
	clr[1] = ColorAdjustLuma(clrbase, 560 + delta, TRUE);
	clr[2] = ColorAdjustLuma(clrbase, delta, TRUE);
	clr[3] = ColorAdjustLuma(clrbase, 860 + delta, TRUE);

	CRect rcw = rc;
	rcw.bottom = rcw.top + (rc.Height() * 4) / 5;

	fill_shade_rect(hdc, rcw, clr[0], clr[1]);
	rcw.top = rcw.bottom;
	rcw.bottom = rc.bottom;
	fill_shade_rect(hdc, rcw, clr[2], clr[3]);
	dc.SetBkColor(clrBack);
}

void FillShadeRect(HDC hdc, LPRECT prcAll, LPRECT prcClip, BOOL bVert)
{
	COLORREF crTo   = mix_color(GetSysColor(COLOR_3DSHADOW), GetSysColor(COLOR_3DFACE), 50);
	COLORREF crFrom = mix_color(GetSysColor(COLOR_3DFACE), GetSysColor(COLOR_3DHIGHLIGHT), 50);
	int r = (int)GetRValue(crTo) + (int)GetRValue(crFrom);
	int g = (int)GetGValue(crTo) + (int)GetGValue(crFrom);
	int b = (int)GetBValue(crTo) + (int)GetBValue(crFrom);
	crTo = RGB(r >> 1, g >> 1, b >> 1);

	CRgn rgn;
	if (prcClip)
		rgn.CreateRectRgnIndirect(prcClip);
#ifdef USE_GRADIENTFILL
	CDCHandle dc = hdc;
	dc.SelectClipRgn(rgn);

	TRIVERTEX vert[2];
	GRADIENT_RECT grc;

	vert[0].x     = prcAll->left;
	vert[0].y     = prcAll->top;
	vert[0].Red   = GetRValue(crFrom) << 8;
	vert[0].Green = GetGValue(crFrom) << 8;
	vert[0].Blue  = GetBValue(crFrom) << 8;
	vert[0].Alpha = 0x0000;

	vert[1].x     = prcAll->right;
	vert[1].y     = prcAll->bottom;
	vert[1].Red   = GetRValue(crTo) << 8;
	vert[1].Green = GetGValue(crTo) << 8;
	vert[1].Blue  = GetBValue(crTo) << 8;
	vert[1].Alpha = 0x0000;

	grc.UpperLeft  = 0;
	grc.LowerRight = 1;
	dc.GradientFill(vert, 2, &grc, 1, bVert ? GRADIENT_FILL_RECT_V : GRADIENT_FILL_RECT_H);
	dc.SelectClipRgn(NULL);
#else
	Graphics gr(hdc);
	LinearGradientBrush br(
							Point(prcAll->left, prcAll->top),
							Point(prcAll->right, prcAll->bottom), 
							Color(255, GetRValue(crFrom), GetGValue(crFrom), GetBValue(crFrom)),
							Color(255, GetRValue(crTo),   GetGValue(crTo),   GetBValue(crTo))
						);

	// Set the clipping region with hRegion.
	if (!rgn.IsNull()) gr.SetClip(rgn);
	gr.FillRectangle(&br, prcAll->left, prcAll->top, prcAll->right - prcAll->left, prcAll->bottom - prcAll->top);
	if (!rgn.IsNull()) gr.ResetClip();
#endif
}

////////////////////////////////////////////////////////////////////////
//

void idl_Free(void* pv)
{
	if (pv) CoTaskMemFree(pv);
}

IShellFolder* idl_GetSpecialFolder(UINT id, LPITEMIDLIST* ppidl)
{
	if (ppidl)
		*ppidl = NULL;

	IShellFolder* psf = NULL;
	IShellFolder* psfDesktop;
	if (SHGetDesktopFolder(&psfDesktop) == NOERROR) {
		LPITEMIDLIST pidl;
		SHGetFolderLocation(NULL, id, NULL, 0, &pidl);
		if (id == CSIDL_DESKTOP) {
			if (ppidl)
				*ppidl = pidl;
			else
				idl_Free(pidl);
			return psfDesktop;
		}

		if (pidl) {
			psfDesktop->BindToObject(pidl, NULL, IID_IShellFolder, (LPVOID*)&psf);
			if (ppidl)
				*ppidl = pidl;
			else
				idl_Free(pidl);
		}
		psfDesktop->Release();
	}
	return psf;
}

BOOL idl_GetName(LPSHELLFOLDER psf, LPITEMIDLIST pidl, DWORD dwFlags, LPTSTR pName, UINT cchName)
{
	STRRET str;
	if (NOERROR == psf->GetDisplayNameOf(pidl, dwFlags, &str)) {
		LPTSTR psz;
		if (StrRetToStr(&str, pidl, &psz) == S_OK) {
			lstrcpyn(pName, psz, cchName);
			CoTaskMemFree(psz);
			return TRUE;
		}
	}
	return FALSE;
}

