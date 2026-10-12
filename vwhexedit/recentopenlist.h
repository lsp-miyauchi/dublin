/////////////////////////////////////////////////////////////////////////////
// recentopenlist.h

#pragma once
#ifndef __RECENTOPENLIST_H__
#define __RECENTOPENLIST_H__

/////////////////////////////////////////////////////////////////////////////
//

#if _WTL_CMDBAR_VISTA_MENUS

class CRecentOpenFileList : public CRecentDocumentListBase<CRecentOpenFileList>
{
private:
	CSimpleArray<HBITMAP> m_arrBitmap;

public:
	~CRecentOpenFileList()
	{
		ClearBitmaps();
	}
	void ClearBitmaps()
	{
		int cnt = m_arrBitmap.GetSize();
		for (int i = 0; i < cnt; i++) {
			HBITMAP hbm = m_arrBitmap[i];
			if (hbm) ::DeleteObject(hbm);
		}
		m_arrBitmap.RemoveAll();
	}
	BOOL UpdateMenu()
	{
		TCHAR sFile[MAX_PATH * 2];
		BOOL bOK = CRecentDocumentListBase<CRecentOpenFileList>::UpdateMenu();

		ClearBitmaps();
		if (bOK && OsVISTA() && m_arrDocs.GetSize() > 0) {
			int cnt = ::GetMenuItemCount(m_hMenu);
			int i;
			for (i = 0; i < cnt; i++) {
				CMenuItemInfo mi;
				mi.fMask = MIIM_ID;
				::GetMenuItemInfo(m_hMenu, i, TRUE, &mi);
				if (GetFromList(mi.wID, sFile, ARRAYSIZE(sFile))) {
					SHFILEINFO sfi;
					HIMAGELIST him = (HIMAGELIST)SHGetFileInfo(sFile, 0, &sfi, sizeof(sfi), SHGFI_SYSICONINDEX | SHGFI_USEFILEATTRIBUTES | SHGFI_SMALLICON);
					HBITMAP hbm = CreateBitmapFromSysImage(him, sfi.iIcon, true);
					if (hbm) {
						m_arrBitmap.Add(hbm);
						mi.fMask = MIIM_BITMAP;
						mi.hbmpItem = hbm;
						::SetMenuItemInfo(m_hMenu, i, TRUE, &mi);
					}
				}
			}
		}
		return bOK;
	}
	HBITMAP CreateBitmapFromSysImage(HIMAGELIST him, int iImage, bool bSmall)
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
};

#endif	// _WTL_CMDBAR_VISTA_MENUS


#endif		// __RECENTOPENLIST_H__
