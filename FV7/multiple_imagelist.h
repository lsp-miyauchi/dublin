#pragma once

class MultipleImageList : public CImageList
{
public:
	CSimpleArray<UINT> m_commands;
	CSimpleArray<HBITMAP> m_bitmaps;		// for bitmap menu
	CSize m_sizeIcon;
	bool m_bAlphaImages;

	MultipleImageList() :m_bAlphaImages(false) {}
	~MultipleImageList() {}

	inline const CSize& GetIconSize() const { return m_sizeIcon; }
	inline HICON LookupIcon(UINT id)
	{
		int index = m_commands.Find(id);
		return (index < 0) ? NULL : GetIcon(index);
	}

	bool SetImageSize(int cx, int cy)
	{
		if (!IsNull()) {
			if (GetImageCount() == 0) {
				Destroy();
				m_commands.RemoveAll();
			}
			else
				return false;
		}

		if ((cx == 0) || (cy == 0))
			return false;
		m_sizeIcon.cx = cx;
		m_sizeIcon.cy = cy;
		return true;
	}
	const SIZE& GetImageSize() { return m_sizeIcon; }

	HBITMAP _CreateBitmapHelper(int nIndex, HDC hdcSrc, HDC hdcDst)
	{
		// Create 32-bit bitmap
		BITMAPINFO bi;
		memset((void*)&bi, 0, sizeof(bi));
		bi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
		bi.bmiHeader.biWidth = m_sizeIcon.cx;
		bi.bmiHeader.biHeight = m_sizeIcon.cy;
		bi.bmiHeader.biPlanes = 1;
		bi.bmiHeader.biBitCount = 32;
		bi.bmiHeader.biCompression = BI_RGB;
		HBITMAP hBitmap = ::CreateDIBSection(hdcSrc, &bi, DIB_RGB_COLORS, NULL, NULL, 0);
		if (hBitmap != NULL) {
			::SelectObject(hdcDst, hBitmap);
			IMAGELISTDRAWPARAMS ildp = {};
			memset(&ildp, 0, sizeof(ildp));
			ildp.cbSize = sizeof(IMAGELISTDRAWPARAMS);
			ildp.himl = m_hImageList;
			ildp.i = nIndex;
			ildp.hdcDst = hdcDst;
			ildp.fStyle = ILD_TRANSPARENT;
			ildp.fState = ILS_ALPHA;
			ildp.Frame = 255;
			DrawIndirect(&ildp);
		}
		return hBitmap;
	}

	void _AddBitmapsFromImageList(int nStartIndex, int nCount = 1)
	{
		// Create display compatible memory DC
		CClientDC dc(NULL);
		CDC dcMem;
		dcMem.CreateCompatibleDC(dc);
		HBITMAP hBitmapSave = dcMem.GetCurrentBitmap();

		// Create bitmaps for all menu items
		for (int i = 0; i < nCount; i++) {
			HBITMAP hBitmap = _CreateBitmapHelper(nStartIndex + i, dc, dcMem);
			dcMem.SelectBitmap(hBitmapSave);
			m_bitmaps.Add(hBitmap);
		}
	}

	void _ReplaceBitmapFromImageList(int nIndex)
	{
		// Delete existing bitmap
		if (m_bitmaps[nIndex] != NULL)
			::DeleteObject(m_bitmaps[nIndex]);
		// Create display compatible memory DC
		CClientDC dc(NULL);
		CDC dcMem;
		dcMem.CreateCompatibleDC(dc);
		HBITMAP hBitmapSave = dcMem.GetCurrentBitmap();

		// Create bitmap for menu item
		HBITMAP hBitmap = _CreateBitmapHelper(nIndex, dc, dcMem);
		// Select saved bitmap back and replace bitmap in the array
		dcMem.SelectBitmap(hBitmapSave);
		m_bitmaps.SetAtIndex(nIndex, hBitmap);
	}

	void _DeleteBitmaps()
	{
		for (int i = 0; i < m_bitmaps.GetSize(); i++) {
			if (m_bitmaps[i] != NULL)
				::DeleteObject(m_bitmaps[i]);
		}
		m_bitmaps.RemoveAll();
	}

	bool AddImageList(_U_STRINGorID id)
	{
		const LSCTOOLBARDATA* pData = lccGetToobarResourceData(RESINST, id.m_lpstr);
		if (!pData) return false;

		CMemTask mt = (void*)pData;		// auto free mem
		// Set internal data
		SetImageSize(pData->wWidth, pData->wHeight);

		WORD* pItems = (WORD*)(pData + 1);
		int nItems = pData->wItemCount;
		CImageList il = lccLoadAlphaImageList(RESINST, RT_RCDATA, id.m_lpstr, pData->wWidth, FALSE);
		if (il.IsNull()) return false;

		int nOldImageCount = !IsNull() ? GetImageCount() : 0;
		if (IsNull()) {
			IMAGEINFO info;
			il.GetImageInfo(0, &info);
			BITMAP bm;
			::GetObject(info.hbmImage, sizeof(bm), &bm);
			m_bAlphaImages = (bm.bmBitsPixel == 32);
			Attach(il);
		} else {
			// Add bitmap to our image list
			int dstCount = il.GetImageCount();
			for (int i = 0; i < dstCount; i++) {
				CIcon ic = il.GetIcon(i, ILD_TRANSPARENT);
				ReplaceIcon(-1, ic);
			}
			il.Destroy();
		}

		// Fill the array with command IDs
		for (int i = 0; i < nItems; i++) {
			if (pItems[i] != 0)
				m_commands.Add(pItems[i]);
		}

		int nImageCount = GetImageCount();
		if (nImageCount != m_commands.GetSize()) return false;

		_AddBitmapsFromImageList(nOldImageCount, nImageCount - nOldImageCount);
		return true;
	}

	int FindCommand(UINT idCommand)
	{
		return m_commands.Find(idCommand);
	}
	HBITMAP GetHBitmap(UINT idCommand)
	{
		int index = m_commands.Find(idCommand);
		return (index >= 0) ? m_bitmaps[index] : NULL;
	}
};
