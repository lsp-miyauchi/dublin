////////////////////////////////////////////////////////////////////////
// shell_util.h

#pragma once
#ifndef __SHELL_UTIL_H__
#define __SHELL_UTIL_H__

class CSimpleIDListArray : public CSimpleArray<PCIDLIST_ABSOLUTE>
{
public:
	~CSimpleIDListArray()
	{
		int cnt = GetSize();
		for (int i = 0; i < cnt; i++) {
			PCIDLIST_ABSOLUTE pidl = (*this)[i];
			if (pidl)
				::CoTaskMemFree((void*)pidl);
		}
		RemoveAll();
	}
};



HGLOBAL GetDataFromDataObject(IDataObject* pdo, CLIPFORMAT cf);
int GetIDListsFromDataObject(IDataObject* pdo, CSimpleIDListArray& array);

#endif		// __SHELL_UTIL_H__
