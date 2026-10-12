/////////////////////////////////////////////////////////////////////////////
// shortcut-list.h

#pragma once
#ifndef __SHORTCUT_LIST_H__
#define __SHORTCUT_LIST_H__


#define MAXSHORTCUT			32

////////////////////////////////////////////////////////////////////////
//

class CShortcutList
{
public:
	CShortcutList()
		: m_nSectionID(0)
		, m_nKeyID(0)
	{
	}
	~CShortcutList()
	{
	}
	void Initial(UINT nSection, UINT nKey);
	void Update();
	void Load();
	void Save();
	void Replace(const CSimpleArray<UINT>& arr);
	void SetDefault(int nEntry, UINT* idArray);
	HMENU CreateShortcutMenu();
	void DestroyShortcutMenu();
	inline HMENU GetShortcutMenu() { return m_menu; }

public:
	UINT m_nSectionID;
	UINT m_nKeyID;
	CSimpleArray<UINT> m_arrDefault;
	CSimpleArray<UINT> m_arrCommand;
	CMenu m_menu;
};


#endif		// __SHORTCUT_LIST_H__
