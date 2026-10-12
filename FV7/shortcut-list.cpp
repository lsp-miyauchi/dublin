/////////////////////////////////////////////////////////////////////////////
// shortcut-list.cpp

#include "stdafx.h"
#include "fv7.h"
#include "multiple_imagelist.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
//@ #define new DEBUG_NEW
#endif

/////////////////////////////////////////////////////////////////////////////
//

void CShortcutList::Initial(UINT nSection, UINT nKey)
{
	m_nSectionID = nSection;
	m_nKeyID = nKey;
}

void CShortcutList::Load()
{
	m_arrCommand.RemoveAll();
	int i, cnt, len;
	BYTE* pd = GetRegBinaryEx(m_nSectionID, m_nKeyID, &len);
	if (pd) {
		cnt = len / sizeof(UINT);
		UINT* ids = (UINT*)pd;
		for (i = 0; i < cnt; i++)
			m_arrCommand.Add(ids[i]);
		free(pd);
	}
	else {
		for (i = 0; i < m_arrDefault.GetSize(); i++)
			m_arrCommand.Add(m_arrDefault[i]);
	}
}

void CShortcutList::Save()
{
	int cnt = m_arrCommand.GetSize();
	if (cnt > 0)
		SetRegBinary(m_nSectionID, m_nKeyID, m_arrCommand.GetData(), cnt * sizeof(UINT));
	else
		SetRegText(m_nSectionID, m_nKeyID, NULL);
}

void CShortcutList::Replace(const CSimpleArray<UINT>& arr)
{
	m_arrCommand = arr;
	Save();
}

HMENU CShortcutList::CreateShortcutMenu()
{
	Load();
	int cnt = m_arrCommand.GetSize();
	if (cnt < 1) return NULL;

	m_menu.DestroyMenu();
	m_menu.CreatePopupMenu();
	CString str;
	for (int i = 0; i < cnt; i++) {
		UINT idCmd = m_arrCommand[i];
		if (idCmd != ID_SEPARATOR) {
			if (GetCommandString(idCmd, str) > 0)
				m_menu.AppendMenu(MF_STRING, idCmd, str.GetString());
		}
		else
			m_menu.AppendMenu(MF_SEPARATOR);
	}
	return m_menu;
}

void CShortcutList::DestroyShortcutMenu()
{
	m_menu.DestroyMenu();
}

void CShortcutList::SetDefault(int nEntry, UINT* idArray)
{
	m_arrDefault.RemoveAll();
	for (int i = 0; i < nEntry; i++)
		m_arrDefault.Add(idArray[i]);
}
