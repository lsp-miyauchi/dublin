///////////////////////////////////////////////////////////////////////
//
#include "stdafx.h"
#include "stdio.h"
#include "fopstd.h"


///////////////////////////////////////////////////////////////////////
// LOCAL DRAG & DROP PROCEDURE
//

extern "C"
HDROP WINAPI fopDragInitBlock(POINT pt, BOOL fNC)
{
	HGLOBAL hdrop = GlobalAlloc(GHND | GMEM_SHARE, sizeof(DROPFILES) + sizeof(TCHAR));
	if (hdrop) {
		DROPFILES* pdf = (DROPFILES*)GlobalLock(hdrop);
		pdf->pFiles = sizeof(DROPFILES);
		pdf->pt = pt;
		pdf->fNC = (WORD)fNC;
		pdf->fWide = TRUE;					// UNICODE filename
		GlobalUnlock(hdrop);
	}
	return (HDROP)hdrop;
}

extern "C"
HDROP WINAPI fopDragAppendFile(HDROP hdrop, LPCTSTR pszPath)
{
	int len = lstrlen(pszPath);
	HDROP hdropT = (HDROP)GlobalReAlloc(hdrop, GlobalSize(hdrop) + (len + 2) * sizeof(TCHAR), GHND | GMEM_SHARE);
	if (hdropT) {
		DROPFILES* pdf = (DROPFILES*)GlobalLock(hdropT);
		LPTSTR p = (LPTSTR)((BYTE*)pdf + pdf->pFiles);
		while (*p) p += lstrlen(p) + 1;
		lstrcpy(p, pszPath);
		p[len + 1] = 0;
		GlobalUnlock(hdropT);
		hdrop = hdropT;
	}
	return hdrop;
}
