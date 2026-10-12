////////////////////////////////////////////////////////////////////////
// tabgroup.cpp

#include "stdafx.h"
#include "fv7.h"

int EnumTabGroup(CCmItemArray& array)
{
	// タブ グループ ファイルの列挙
	array.DeleteAll();

	TCHAR buf[MAX_PATH * 2];
	CString dir;
	GetAppDataFolder(dir);
	PathCombine(buf, dir, _T("*.tabgroup"));

	CFileFinder ff;
	if (ff.BeginFind(buf)) {
		do {
			PathCombine(buf, dir, ff.cFileName);
			CIDListPtr idl;
			if (idl.CreateFromPath(buf)) {
				CCmItem* pci = new CCmItem(idl);
				PathRemoveExtension(buf);
				pci->m_name = PathFindFileName(buf);
				idl.Free();
				array.Add(pci);
			}
		} while (ff.NextFind());
		ff.EndFind();
	}
	return array.GetSize();
}
