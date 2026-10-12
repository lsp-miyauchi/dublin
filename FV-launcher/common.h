////////////////////////////////////////////////////////////////////////
// common.h

#pragma once
#ifndef __FVLCH_COMMON_H__
#define __FVLCH_COMMON_H__

extern COSPlatform gosp;

COLORREF mix_color(COLORREF clr1, COLORREF clr2, int clr1_per);
void fill_shade_rect(HDC hdc, const CRect& rc, COLORREF crFrom, COLORREF crTo);
void fill_glass_rect(HDC hdc, const CRect& rc, bool bHot, bool bPushedOrSelected);
void FillShadeRect(HDC hdc, LPRECT prcAll, LPRECT prcClip, BOOL bVert);

void idl_Free(void* pv);
IShellFolder* idl_GetSpecialFolder(UINT id, LPITEMIDLIST* ppidl);
BOOL idl_GetName(IShellFolder* psf, LPITEMIDLIST pidl, DWORD dwFlags, LPTSTR pName, UINT cchName);

#endif		// __FVLCH_COMMON_H__
