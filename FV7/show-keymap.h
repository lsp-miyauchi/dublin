/////////////////////////////////////////////////////////////////////////////
// CShowKeymapWnd window

#pragma once

#ifndef __SHOW_KEYMAP_H__
#define __SHOW_KEYMAP_H__

class CKeymapMenuItem
{
public:
	int m_cmd;
	HIMAGELIST m_him;
	int m_iImage;
	CString m_str;

	CKeymapMenuItem()
		: m_cmd(0)
		, m_him(NULL)
		, m_iImage(-1)
	{
	}
};

class CShowKeymapWnd : public CWindowImpl<CShowKeymapWnd>, public CThemeImpl<CShowKeymapWnd>
{
private:
	CSimpleMap<int, CKeymapMenuItem*> m_array;
	CSimpleArray<HBITMAP> m_hbmArray;
public:
	CShowKeymapWnd();
	~CShowKeymapWnd();

	BEGIN_MSG_MAP(CShowKeymapWnd)
		CHAIN_MSG_MAP(CThemeImpl<CShowKeymapWnd>)
		ON_WM_MENUCHAR()
	END_MSG_MAP()

	LRESULT OnMenuChar(UINT nChar, UINT nFlags, HMENU hMenu);

	int _InsertMenuItems(HMENU hmenu, int idCmdStart, UINT* cmdTable, int iStart, int maxEntry);
	int TrackKeymapMenu(HWND hParent);
};

#endif		// __SHOW_KEYMAP_H__
