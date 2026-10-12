//////////////////////////////////////////////////////////////////////
// sildemenu.h

#pragma once
#ifndef __SLIDEMENU_H__
#define __SLIDEMENU_H__


//////////////////////////////////////////////////////////////////////
//

class CSlideItem
{
public:
	CSlideItem(LPCTSTR pszName)
	{
		m_csName = pszName;
		SHFILEINFO sfi;
		SHGetFileInfo(pszName, 0, &sfi, sizeof(sfi), SHGFI_SYSICONINDEX | SHGFI_SMALLICON);
		m_iImage = sfi.iIcon;
	}

	long m_magic;
	int m_iImage;
	CString m_csName;
};

//////////////////////////////////////////////////////////////////////
//

template <class T, class TBase = CWindow, class TWinTraits = CControlWinTraits>
class ATL_NO_VTABLE CSlideWndImpl : public CWindowImpl< T, TBase, TWinTraits>
{
public:
	CSlideWndImpl() {}
	~CSlideWndImpl() {}

	DECLARE_WND_CLASS(NULL)

	HIMAGELIST m_hImage;
	NONCLIENTMETRICS m_ncm;

BEGIN_MSG_MAP(this)
	ON_WM_DRAWITEM()
	ON_WM_MEASUREITEM()
END_MSG_MAP()

	void OnDrawItem(int nIDCtrl, LPDRAWITEMSTRUCT lpdi)
	{
		T* pT = static_cast<T*>(this);
		pT->DrawItem(nIDCtrl, lpdi);
	}

	void OnMeasureItem(int nIDCtrl, LPMEASUREITEMSTRUCT lpmi)
	{
		T* pT = static_cast<T*>(this);
		pT->MeasureItem(nIDCtrl, lpmi);
	}

	int DoTrack(HWND hParent, CSimpleArray<CString>& arrFile, CPoint pt)
	{
		int num = arrFile.GetSize();
		if (num < 1) return -1;

		HIMAGELIST dumm;
		Shell_GetImageLists(&dumm, &m_hImage);

		m_ncm.cbSize = RunTimeHelper::SizeOf_NONCLIENTMETRICS();
		SystemParametersInfo(SPI_GETNONCLIENTMETRICS, sizeof(m_ncm), &m_ncm, FALSE);

		int sep = (num >= 20) ? 20 : 0;
		CMenu pop;
		pop.CreatePopupMenu();
		CSlideItem* psi;
		CSimpleArray<CSlideItem*> array;
		for (int i = 0; i < num; i++) {
			psi = new CSlideItem(arrFile[i]);
			pop.AppendMenu(MF_OWNERDRAW | ((i > 0 && sep > 0 && (i % sep) == 0) ? MF_MENUBARBREAK : 0), i + 1, (LPCTSTR)psi);
			array.Add(psi);
		}

		CRect rc(0, 0, 10, 10);
		Create(hParent, rc, NULL, WS_CHILD);
		int idCmd = TrackPopupMenu(pop, TPM_RETURNCMD, pt.x,  pt.y, 0, m_hWnd, NULL) - 1;
		DestroyWindow();

		for (int i = 0; i < num; i++)
			delete array[i];

		return idCmd;
	}

	virtual void DrawItem(int nIDCtl, LPDRAWITEMSTRUCT pdi)
	{
		if (pdi->CtlType != ODT_MENU) return;

		if (pdi->itemData) {
			CSlideItem* psi = (CSlideItem*)pdi->itemData;
			lccDrawCustomMenu(pdi, PathFindFileName(psi->m_csName), m_hImage, psi->m_iImage, DT_PATH_ELLIPSIS);
		}
		else
			lccDrawCustomMenu(pdi, NULL, NULL, -1, DT_NOCLIP);
	}

	virtual void MeasureItem(int /*nIDCtl*/, LPMEASUREITEMSTRUCT lpmi)
	{
		if (lpmi->CtlType != ODT_MENU) return;

		if (lpmi->itemData) {
			lpmi->itemWidth = m_ncm.iMenuWidth;
			lpmi->itemHeight = m_ncm.iMenuHeight;

			CClientDC dc(NULL);
			HFONT hfont = dc.SelectFont((HFONT)GetStockObject(DEFAULT_GUI_FONT));
			TEXTMETRIC tm;
			dc.GetTextMetrics(&tm);

			CSlideItem* psi = (CSlideItem*)lpmi->itemData;
			LPCTSTR p = PathFindFileName(psi->m_csName);
			CSize csz;
			dc.GetTextExtent(p, lstrlen(p), &csz);

			lpmi->itemWidth = min(csz.cx, tm.tmAveCharWidth * 50) + 34;
			lpmi->itemHeight = max(lpmi->itemHeight, 16 + 4);
			dc.SelectFont(hfont);
		}
		else {
			lpmi->itemHeight = ::GetSystemMetrics(SM_CYMENU) / 2;
			lpmi->itemWidth  = 0;
		}
	}
};

class CSlideWnd : public CSlideWndImpl<CSlideWnd>
{
public:
};






#endif			// __SLIDEMENU_H__
