//////////////////////////////////////////////////////////////////////
// filter.h

#pragma once
#ifndef __PLUGIN_FILTER_H__
#define __PLUGIN_FILTER_H__

#include "..\img_plugin\plugin.h"

#define MAGIC_PLUGINFILTER			0x00504649

class CFilter
{
public:
	DWORD m_magic;
	UINT m_idCommand;
	CString m_strDllName;
	CString m_strFilterName;
	UINT m_iFilterNumber;
	UINT m_nFlags;
	UINT m_menugroup;
	int m_iImage;
	int m_imageStyle;				// -1=N/A, 0=icon, 1=png(or any)
	HBITMAP m_hbmImage;
	HICON m_hicImage;

	CFilter()
		: m_magic(MAGIC_PLUGINFILTER)
		, m_idCommand(0)
		, m_iFilterNumber(-1)
		, m_nFlags(0)
		, m_menugroup(0)
		, m_iImage(-1)
	{}

	~CFilter()
	{}

	HDIB Execute(HANDLE hdib, LPRECT prc, FILTER_EXEC* pfe)
	{
		HDIB hdibNew = NULL;
		HMODULE hdll = LoadLibrary(m_strDllName);
		if (hdll) {
			FILTERPROC pfnExec;
			(FARPROC&)pfnExec = GetProcAddress(hdll, PLUGIN_FILTER_EXEC);
			if (pfnExec)
				hdibNew = (*pfnExec)(m_iFilterNumber, hdib, prc, pfe);
			FreeLibrary(hdll);
		}
		return hdibNew;
	}

	bool IsFilterItem() { return (m_magic == MAGIC_PLUGINFILTER); }
};


#endif			// __PLUGIN_FILTER_H__
