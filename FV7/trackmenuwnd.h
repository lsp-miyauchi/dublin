//////////////////////////////////////////////////////////////////////
// CTrackMenuWnd

#pragma once
#ifndef __TRACKMENUWND_H__
#define __TRACKMENUWND_H__


#ifndef TOUPPER
	#define TOUPPER(ch)		(((ch)>='a' && (ch)<='z') ? ((ch)-('a'-'A')) : (ch))
#endif

class CMenuItemData
{
public:
	CMenuItemData() { m_magic = 0x8765; m_strText.Empty(); }
	__inline bool IsMenuItem() { return (m_magic == 0x8765); }

	DWORD m_magic;
	CString m_strText;
	UINT m_fType;
	UINT m_fState;
};

template <class T, class TBase = CWindow, class TWinTraits = CControlWinTraits>
class ATL_NO_VTABLE CTrackMenuWndImpl : public CWindowImpl< T, TBase, TWinTraits>
{
private:
	CSimpleArray<CMenuItemData*> m_array;

public:
	CTrackMenuWndImpl() {}
	~CTrackMenuWndImpl()
	{
		int cnt = m_array.GetSize();
		for (int i = 0; i < cnt; i++)
			delete m_array[i];
		m_array.RemoveAll();
	}

	DECLARE_WND_CLASS(NULL)

BEGIN_MSG_MAP(this)
	MESSAGE_HANDLER(WM_INITMENUPOPUP, OnInitMenuPopup)
	MESSAGE_HANDLER(WM_DRAWITEM, OnDrawItem)
	MESSAGE_HANDLER(WM_MEASUREITEM, OnMeasureItem)
	ON_WM_MENUCHAR()
END_MSG_MAP()

	LRESULT OnInitMenuPopup(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
	{
		if (HIWORD(lParam)) {
			// System menu, do nothing
			bHandled = FALSE;
			return 1;
		}
		GetParent().SendMessage(uMsg, wParam, lParam);
		return 0;
	}

	LRESULT OnMenuChar(UINT nChar, UINT nFlags, HMENU hMenu)
	{
		int cnt = GetMenuItemCount(hMenu);
		CMenuItemInfo mii;
		mii.fMask = MIIM_DATA | MIIM_TYPE | MIIM_STATE;
		for (int i = 0; i < cnt; i++) {
			GetMenuItemInfo(hMenu, i, TRUE, &mii);
			CMenuItemData* pMI = (CMenuItemData*)mii.dwItemData;
			if (mii.fType & MFT_OWNERDRAW) {
				TCHAR* p = StrChr(pMI->m_strText, '&');
				if (p && TOUPPER((TCHAR)nChar) == TOUPPER(*(p + 1)))
					return MAKELONG(i, MNC_EXECUTE);
			}
		}
		return 0;
	}

	LRESULT OnDrawItem(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
	{
		DRAWITEMSTRUCT* pdi = (DRAWITEMSTRUCT*)lParam;
		CMenuItemData* pMI = (CMenuItemData*)pdi->itemData;
		if (pdi->CtlType != ODT_MENU || !pMI->IsMenuItem()) {
			bHandled = FALSE;
			return 1;
		}
		lccDrawCustomMenu(pdi, pMI->m_strText, NULL, -1, DT_NOCLIP);
		return 1;
	}

	LRESULT OnMeasureItem(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
	{
		MEASUREITEMSTRUCT* pmi = (MEASUREITEMSTRUCT*)lParam;
		CMenuItemData* pMI = (CMenuItemData*)pmi->itemData;
		if (pmi->CtlType != ODT_MENU || !pMI->IsMenuItem()) {
			bHandled = FALSE;
			return 1;
		}

		if (pMI->m_fType & MFT_SEPARATOR) {
			pmi->itemHeight = ::GetSystemMetrics(SM_CYMENU) / 2;
			pmi->itemWidth  = 0;
		}
		else {
			pmi->itemWidth = _GCDATA.m_NcMetrics.iMenuWidth;
			pmi->itemHeight = _GCDATA.m_NcMetrics.iMenuHeight;

			CClientDC dc(NULL);
			HFONT hfont = dc.SelectFont(_GCDATA.GetMenuFont());
			TEXTMETRIC tm;
			dc.GetTextMetrics(&tm);

			CSize csz;
			dc.GetTextExtent(pMI->m_strText, pMI->m_strText.GetLength(), &csz);

			pmi->itemWidth = csz.cx + 24;
			pmi->itemHeight = max(pmi->itemHeight, MENU_OWNERDRAW_HEIGHT);
			dc.SelectFont(hfont);
		}
		return 1;
	}
};

class CTrackMenuWnd : public CTrackMenuWndImpl<CTrackMenuWnd>
{
public:
	int DoTrack(HWND hParent, HMENU hMenu, CPoint pt)
	{
		CRect rc(0, 0, 10, 10);
		Create(hParent, rc, NULL, WS_CHILD);
		int idCmd = ::TrackPopupMenu(hMenu, TPM_RETURNCMD, pt.x,  pt.y, 0, m_hWnd, NULL);
		DestroyWindow();
		return idCmd;
	}

	int DoTrack(HWND hParent, _U_STRINGorID menu_id, CPoint pt)
	{
		CMenu menu;
		menu.LoadMenu(menu_id);
		CMenuHandle pop = menu.GetSubMenu(0);
		return DoTrack(hParent, pop, pt);
	}
};


#endif				// __TRACKMENUWND_H__
