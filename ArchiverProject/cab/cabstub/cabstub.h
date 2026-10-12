// autoexp.h

#include "..\sfx.h"
#include "resource.h"
#include "meter.h"
#include "filemap.h"

#define MB_STOP						(MB_OK | MB_ICONSTOP)

enum {
	ACTION_CANCEL = -1,
	ACTION_IGNORE = 0,
	ACTION_OVERWRITE,
};



////////////////////////////////////////////////////////////////////////
//

class CStringArray
{
public:
	LPCWSTR* m_arrptr;
	int m_arrsize;
	int m_entries;

public:
	CStringArray()
		: m_arrptr(NULL)
		, m_arrsize(0)
		, m_entries(0)
	{
	}

	~CStringArray()
	{
		RemoveAll();
	}

	void RemoveAll()
	{
		if (m_arrsize && m_arrptr) {
			int i;
			for (i = 0; i < m_entries; i++) {
				LPCWSTR psz = m_arrptr[i];
				if (psz) {
					free((void*)psz);
					m_arrptr[i] = NULL;
				}
			}
			free(m_arrptr);
		}
		m_arrptr = NULL;
		m_arrsize = 0;
		m_entries = 0;
	}

	BOOL Add(LPCWSTR psz)
	{
		if (m_entries + 1 > m_arrsize) {
			int newsize = m_arrsize + 32;
			LPCWSTR* arrptr = (LPCWSTR*)realloc(m_arrptr, newsize * sizeof(LPCWSTR));
			if (!arrptr) return FALSE;

			m_arrptr = arrptr;
			m_arrsize = newsize;
		}

		LPWSTR p = NULL;
		if (psz && *psz) {
			p = (LPWSTR)malloc((wcslen(psz) + 1) * sizeof(WCHAR));
			wcscpy(p, psz);
		}
		m_arrptr[m_entries++] = p;
		return TRUE;
	}

	BOOL RemoveAt(int index)
	{
		if (index < 0 || index >= m_entries)
			return FALSE;

		LPCWSTR psz = m_arrptr[index];
		if (psz)
			free((void*)psz);

		if (index != (m_entries - 1))
			memmove((void*)(m_arrptr + index), (void*)(m_arrptr + index + 1), (m_entries - (index + 1)) * sizeof(LPCWSTR));
		m_entries--;
		return TRUE;
	}

	const LPCWSTR& operator[] (int index) const
	{
		return m_arrptr[index];
	}

	LPCWSTR& operator[] (int index)
	{
		return m_arrptr[index];
	}

	int Find(const LPCWSTR psz) const
	{
		for (int i = 0; i < m_entries; i++) {
			if (!wcscmp(m_arrptr[i], psz))
				return i;
		}
		return -1;  // not found
	}

	BOOL SetAtIndex(int index, const LPCWSTR psz)
	{
		if (index < 0 || index >= m_entries)
			return FALSE;

		if (m_arrptr[index]) {
			free((void*)m_arrptr[index]);
			m_arrptr[index] = NULL;
		}
		if (psz && *psz) {
			LPWSTR p = (LPWSTR)malloc((wcslen(psz) + 1) * sizeof(WCHAR));
			wcscpy(p, psz);
			m_arrptr[index] = p;
		}
		return TRUE;
	}

	inline int GetSize() { return m_entries; }
};


////////////////////////////////////////////////////////////////////////
//

class CSfxInfo
{
public:
	CSfxInfo()
	{
		m_lTotalUncompressedSize = 0;
		m_lCabDataSize = 0;
		m_wStat = 0;

		m_strTitle = NULL;
		m_strMessage = NULL;
		m_strPath = NULL;
		m_strCommand = NULL;
	}
	~CSfxInfo()
	{
		if (m_strTitle)   delete [] m_strTitle;
		if (m_strMessage) delete [] m_strMessage;
		if (m_strPath)    delete [] m_strPath;
		if (m_strCommand) delete [] m_strCommand;
	}

	DWORD m_lTotalUncompressedSize;
	DWORD m_lCabDataSize;
	WORD m_wStat;
	CStringArray m_filearray;
	WCHAR* m_strTitle;
	WCHAR* m_strMessage;
	WCHAR* m_strPath;
	WCHAR* m_strCommand;
};

extern HINSTANCE ghInst;

#include "func.h"
