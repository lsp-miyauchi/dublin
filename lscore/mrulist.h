////////////////////////////////////////////////////////////////////////
// mrulist.h

#pragma once
#ifndef __MRULIST_H__
#define __MRULIST_H__

#define _SELFMRUPROC			 // 自前のMRU機能を使う場合


typedef int (CALLBACK* MRUCMPPROC)(LPCTSTR, LPCTSTR);
typedef int (CALLBACK* MRUCMPPROC_BIN)(LPCVOID, LPCVOID, DWORD);

typedef struct tagMRUINFO {
	DWORD			cbSize;
	DWORD			nMaxItems;
	DWORD			fFlags;
	HKEY			hKey;
	LPCTSTR			pszSubKey;
	FARPROC			pfnCompare;
} MRUINFO;

#ifdef _SELFMRUPROC 	// comctrlのMRU機能を使う場合 ---------------->

static const TCHAR strMRUList[] = _T("MRUList");

/* fFlags */
#define MRUF_STRING_LIST			0			// list will contain strings
#define MRUF_BINARY_LIST			1			// list will contain binary data
#define MRUF_DELAYED_SAVE			2			// only save list order to reg. is FreeMRUList

/* If list is a string list pfnCompare has the following prototype
 * int CALLBACK MRUCompareString(LPCSTR s1, LPCSTR s2)
 * for binary lists the prototype is
 * int CALLBACK MRUCompareBinary(LPCVOID data1, LPCVOID data2, DWORD cbData)
 * where cbData is the no. of bytes to compare.
 * Need to check what return value means identical - 0?
 */

typedef struct tagMRUITEM {
	DWORD			size;						// size of data stored
	DWORD			itemFlag;					// fFlags
	BYTE			datastart;
} MRUITEM;

////////////////////////////////////////////////////////
// itemFlag
#define WMRUIF_CHANGED				0x0001		// this dataitem changed

////////////////////////////////////////////////////////
// stat
#define WMRUF_CHANGED				0x0001		// MRU list has changed


////////////////////////////////////////////////////////////////////////
//

class CMRUList
{
public:
	MRUINFO m_minfo;							// original create information
	DWORD m_stat;								// internal fFlags
	DWORD m_cursize;							// current size of realMRU
	LPTSTR m_realMRU;							// pointer to string of index names
	MRUITEM** m_array;							// array of pointers to data

	CMRUList()
	{
		init();
	}

	CMRUList(MRUINFO* pmi)
	{
		init();
		if (!build_param(pmi))
			wep();
	}

	~CMRUList()
	{
		wep();
	}

	void init()
	{
		memset(&m_minfo, 0, sizeof(m_minfo));
		m_stat = 0;
		m_cursize = 0;
		m_realMRU = NULL;
		m_array = NULL;
	}

	void wep()
	{
		if (m_stat & WMRUF_CHANGED) {
			// need to open key and then save the info
			SaveChanged();
		}

		if (m_array) {
			DWORD i;
			for (i = 0; i < m_minfo.nMaxItems; i++) {
				if (m_array[i])
					free(m_array[i]);
			}
			free(m_array);
		}
		if (m_realMRU) free(m_realMRU);
		if (m_minfo.pszSubKey) free((void*)m_minfo.pszSubKey);

		memset(&m_minfo, 0, sizeof(m_minfo));
		m_stat = 0;
		m_cursize = 0;
		m_realMRU = NULL;
		m_array = NULL;
	}

	BOOL build_param(MRUINFO* info)
	{
		if (!info || info->cbSize != sizeof(MRUINFO) || !info->hKey || IsBadStringPtr(info->pszSubKey, (UINT_PTR)-1))
			return FALSE;

		m_minfo = *info;
		m_minfo.pszSubKey = (LPCTSTR)malloc((lstrlen(info->pszSubKey) + 1) * sizeof(TCHAR));
		lstrcpy((LPTSTR)m_minfo.pszSubKey, info->pszSubKey);

		// get space to save indices that will turn into names but in order of most to least recently used
		m_realMRU = (LPTSTR)malloc((m_minfo.nMaxItems + 2) * sizeof(TCHAR));

		// get space to save pointers to actual data in order of 'a' to 'z' (0 to n).
		size_t arraysize = m_minfo.nMaxItems * sizeof(LPVOID);
		m_array = (MRUITEM**)malloc(arraysize);
		memset(m_array, 0, arraysize);

		// open the sub key
		DWORD dispo;
		HKEY hkey;
		if (RegCreateKeyEx(m_minfo.hKey, m_minfo.pszSubKey, 0, NULL, REG_OPTION_NON_VOLATILE, KEY_READ | KEY_WRITE, 0, &hkey, &dispo) != ERROR_SUCCESS) {
			return FALSE;
		}

		// get values from key 'MRUList'
		if (hkey) {
	 		DWORD type;
			DWORD size = (m_minfo.nMaxItems + 1) * sizeof(TCHAR);
			if (RegQueryValueEx(hkey, strMRUList, 0, &type, (BYTE*)m_realMRU, &size) != ERROR_SUCCESS) {
				*m_realMRU = 0;
			}
			// size now has number of items in the MRUList
			m_cursize = lstrlen(m_realMRU);

			// get actual values for each entry
			for (DWORD i = 0; i < m_cursize; i++) {
				TCHAR name[2];
				name[0] = 'a' + (TCHAR)i;
				name[1] = 0;
				// データサイズを取得
				if (RegQueryValueEx(hkey, name, NULL, &type, 0, &size)) {
					// not present - what to do ???
				}
			    MRUITEM* item = (MRUITEM*)malloc(size + sizeof(MRUITEM));
				m_array[i] = item;
				item->size = size;
				RegQueryValueEx(hkey, name, NULL, &type, &item->datastart, &size);
			}
			RegCloseKey(hkey);
		}
		else
			m_cursize = 0;

		return TRUE;
	}

	void SaveChanged()
	{
		HKEY hkey;
		if (RegOpenKeyEx(m_minfo.hKey, m_minfo.pszSubKey, 0, KEY_WRITE, &hkey) != ERROR_SUCCESS) {
			DWORD dispo;
			if (RegCreateKeyEx(m_minfo.hKey, m_minfo.pszSubKey, 0, NULL, REG_OPTION_NON_VOLATILE, KEY_READ | KEY_WRITE, NULL, &hkey, &dispo) != ERROR_SUCCESS) {
				return;
			}
		}
		if (m_stat & WMRUF_CHANGED) {
			m_stat &= ~WMRUF_CHANGED;
			RegSetValueEx(hkey, strMRUList, 0, REG_SZ, (BYTE*)m_realMRU, (lstrlen(m_realMRU) + 1) * sizeof(TCHAR));
		}

		UINT i;
		TCHAR name[2];
		name[1] = 0;
		for (i = 0; i < m_cursize; i++) {
			MRUITEM* item = m_array[i];
			if (item->itemFlag & WMRUIF_CHANGED) {
				item->itemFlag &= ~WMRUIF_CHANGED;
				name[0] = (TCHAR)('a' + i);
				RegSetValueEx(hkey, name, 0, (m_minfo.fFlags & MRUF_BINARY_LIST) ? REG_BINARY : REG_SZ, &item->datastart, item->size);
			}
		}
		RegCloseKey(hkey);
	}

	static int CALLBACK CompareString(LPCTSTR s1, LPCTSTR s2)
	{
		return lstrcmpi(s1, s2);
	}

	static int CALLBACK CompareBinary(LPCVOID data1, LPCVOID data2, DWORD cbData)
	{
		return memcmp(data1, data2, cbData);
	}

	int FindData(const void* pvData, DWORD cbData, int* pRegNum)
	{
		DWORD i;
		for (i = 0; i < m_cursize; i++) {
			MRUITEM* item = m_array[i];
			if (m_minfo.fFlags & MRUF_BINARY_LIST) {
				MRUCMPPROC_BIN pfn = (MRUCMPPROC_BIN)m_minfo.pfnCompare;
				if (!pfn) pfn = CompareBinary;
				DWORD cmplen = min(cbData, item->size);
				if (!pfn(pvData, &item->datastart, cmplen)) {
					if (item->size == cbData)
						break;
				}
			}
			else {
				MRUCMPPROC pfn = (MRUCMPPROC)m_minfo.pfnCompare;
				if (!pfn) pfn = CompareString;
				if (!pfn((LPCTSTR)pvData, (LPCTSTR)&item->datastart))
					break;
			}
		}

		int res = (i < m_cursize) ? i : -1;
		if (pRegNum && (res != -1))
			*pRegNum = 'a' + i;
		return res;
	}

	int FindString(LPCTSTR pszString, int* pRegNum)
	{
		return FindData(pszString, (lstrlen(pszString) + 1) * sizeof(TCHAR), pRegNum);
	}

	int AddData(const void* pvData, DWORD cbData)
	{
		int replace_index = FindData(pvData, cbData, NULL);
		if (replace_index >= 0) {
			// Item exists, just move it to the front
			LPTSTR pos = StrChr(m_realMRU, (TCHAR)(replace_index + 'a'));
			while (pos > m_realMRU) {
				pos[0] = pos[-1];
				pos--;
			}
		}
		else {
			// either add a new entry or replace oldest
			if (m_cursize < m_minfo.nMaxItems) {
				// Add in a new item
				replace_index = m_cursize;
				m_realMRU[++m_cursize] = 0;
			}
			else {
				// get the oldest entry and replace data
				replace_index = m_realMRU[m_cursize - 1] - 'a';
				free(m_array[replace_index]);
			}

			// Allocate space for new item and move in the data
			MRUITEM* item = (MRUITEM*)malloc(cbData + sizeof(MRUITEM));
			m_array[replace_index] = item;
			item->itemFlag |= WMRUIF_CHANGED;
			item->size = cbData;
			memcpy(&item->datastart, pvData, cbData);

			// now rotate MRU list
			int i;
			for (i = m_cursize - 1; i >= 1; i--)
				m_realMRU[i] = m_realMRU[i - 1];
		}

		// The new item gets the front spot
		m_stat |= WMRUF_CHANGED;
		m_realMRU[0] = (TCHAR)(replace_index + 'a');

		if (!(m_minfo.fFlags & MRUF_DELAYED_SAVE)) {
			// save changed stuff right now
			SaveChanged();
		}
		return replace_index;
	}

	int AddString(LPCTSTR pszString)
	{
		if (!pszString || IsBadStringPtr(pszString, (UINT_PTR)-1)) {
			SetLastError(ERROR_INVALID_PARAMETER);
			return 0;
		}
		return AddData(pszString, (lstrlen(pszString) + 1) * sizeof(TCHAR));
	}

	BOOL DelString(int nItemPos)
	{
		return FALSE;
	}

	int EnumList(int nItemPos, void* pBuffer, DWORD nBufferSize)
	{
		if (nItemPos < 0) return m_cursize;
		if (nItemPos >= (int)m_cursize) return -1;

		int desired = m_realMRU[nItemPos];
		desired -= 'a';

		MRUITEM* item = m_array[desired];
		int size = item->size;
		if (pBuffer) {
			size = min(size, (int)nBufferSize);
			memcpy(pBuffer, &item->datastart, size);
		}
		return size;
	}
};

///////////////////////////////////////////////////////////////////////////
//

class CMRUProc
{
public:
	CMRUList* m_listptr;

	CMRUProc() : m_listptr(NULL)
	{
	}

	~CMRUProc()
	{
		Close();
	}

	BOOL Create(HKEY hkey, LPCTSTR pszSubKey, UINT uMaxEntry, BOOL fBinary = FALSE)
	{
		Close();

		MRUINFO info;
		info.cbSize = sizeof(info);
		info.nMaxItems = uMaxEntry;
		info.fFlags = fBinary ? MRUF_BINARY_LIST : 0;
		info.hKey = hkey;
		info.pszSubKey = pszSubKey;
		info.pfnCompare = NULL;

		m_listptr = new CMRUList;
		if (!m_listptr->build_param(&info)) {
			delete m_listptr;
			m_listptr = NULL;
		}
		return (m_listptr != NULL);
	}

	void Close()
	{
		if (m_listptr) {
			delete m_listptr;
			m_listptr = NULL;
		}
	}

	int Enum(int nItem, void* pData, UINT uLen)
	{
		return (m_listptr) ? m_listptr->EnumList(nItem, pData, uLen) : -1;
	}

	int GetCount()
	{
		return Enum(-1, NULL, 0);
	}

	int Add(LPCTSTR pszString)
	{
		return (m_listptr) ? m_listptr->AddString(pszString) : -1;
	}

	int AddBinary(void* pData, int len)
	{
		return (m_listptr) ? m_listptr->AddData(pData, len) : -1;
	}

	int Find(LPCTSTR pszString, int* pRegNum)
	{
		return (m_listptr) ? m_listptr->FindString(pszString, pRegNum) : -1;
	}

	int FindBinary(const void* pData, DWORD cbData, int* pRegNum)
	{
		return (m_listptr) ? m_listptr->FindData(pData, cbData, pRegNum) : -1;
	}

	BOOL Del(int nItemPos)
	{
		return (m_listptr) ? m_listptr->DelString(nItemPos) : FALSE;
	}
};

#else	// !_SELFMRUPROC comctrlのMRU機能を使う場合 ---------------------------->


#define MRU_BINARY			0x00000001
#define MRU_CACHEWRITE		0x00000010

/* ----
	cbSize
		The size of the structure.
	uMax
		The maximum number of entries in the MRU list.
	fFlags
		One or more of the following flags.
		MRU_BINARY
			Data is stored in the registry as binary data rather than string data.
		MRU_CACHEWRITE
			Write changes to the version of the MRU stored in the registry only when
			a new item is added or the MRU list's resources are freed from memory.
			Note that the active version of the MRU in memory is updated immediately
			in response to any change in contents or ordering.
	hKey
		Handle to the currently open key, or one of the following predefined values
		under which to store the MRU data.
		HKEY_CURRENT_USER
		HKEY_LOCAL_MACHINE
	lpszSubKey
		The subkey under which to store the MRU data.
	lpfnCompare
		Pointer to an optional data comparison function that can be used to determine
		whether an item is present in the MRU list.
		This is useful when the MRU list was created with the MRU_BINARY flag.
		If this member is NULL, standard string comparison functions are used;
		for binary data, a direct memory comparison is used.
---- */

// ordinal:400  extern "C" HANDLE WINAPI CreateMRUListW(MRUINFO* pmi);
// ordinal:401  extern "C" int WINAPI AddMRUStringW(HANDLE hMRU, LPCTSTR szString);
// ordinal:152  extern "C" int WINAPI FreeMRUList(HANDLE hMRU);
// ordinal:156  extern "C" BOOL WINAPI DelMRUString(HANDLE hList, INT nItemPos);
// ordinal:169  extern "C" int WINAPI FindMRUData (HANDLE hList, LPCVOID lpData, DWORD cbData, LPINT lpRegNum)
// ordinal:402  extern "C" int WINAPI FindMRUStringW(HANDLE hList, LPCWSTR lpszString, LPINT lpRegNum)
// ordinal:403  extern "C" int WINAPI EnumMRUListW(HANDLE hMRU, int nItem, void* lpData, UINT uLen);

class CMRUProc
{
public:
	HANDLE m_hmru;
	HANDLE (WINAPI* m_CreateMRUListW)(MRUINFO*);
	int (WINAPI* m_AddMRUStringW)(HANDLE, LPCTSTR);
	int (WINAPI* m_FreeMRUList)(HANDLE);
	BOOL (WINAPI* m_DelMRUString)(HANDLE, INT);
	int (WINAPI* m_FindMRUData)(HANDLE, LPCVOID pData, DWORD cbData, int* pRegNum);
	int (WINAPI* m_EnumMRUListW)(HANDLE, int, void*, UINT);
	int (WINAPI* m_FindMRUListW)(HANDLE, LPCTSTR, int*);

	CMRUProc()
	{
		m_hmru = NULL;
		m_CreateMRUListW = NULL;
		m_AddMRUStringW = NULL;
		m_FreeMRUList = NULL;
		m_EnumMRUListW = NULL;
		HMODULE hmod = GetModuleHandle(_T("comctl32.dll"));
		if (hmod) {
			(FARPROC&)m_CreateMRUListW = GetProcAddress(hmod, (LPCSTR)400 /*"CreateMRUListW"*/);
			(FARPROC&)m_AddMRUStringW = GetProcAddress(hmod, (LPCSTR)401 /*"AddMRUStringW"*/);
			(FARPROC&)m_FreeMRUList = GetProcAddress(hmod, (LPCSTR)152 /*"FreeMRUList"*/);
			(FARPROC&)m_DelMRUString = GetProcAddress(hmod, (LPCSTR)156 /*"DelMRUString"*/);
			(FARPROC&)m_FindMRUData = GetProcAddress(hmod, (LPCSTR)159 /*"FindMRUData"*/);
			(FARPROC&)m_FindMRUListW = GetProcAddress(hmod, (LPCSTR)402 /*"FindMRUListW"*/);
			(FARPROC&)m_EnumMRUListW = GetProcAddress(hmod, (LPCSTR)403 /*"EnumMRUListW"*/);
		}
	}

	~CMRUProc()
	{
		Close();
	}

	BOOL Create(HKEY hkey, LPCTSTR pszSubKey, UINT uMaxEntry, BOOL fBinary = FALSE)
	{
		Close();

		if (m_CreateMRUListW) {
			MRUINFO info;
			info.cbSize = sizeof(info);
			info.nMaxItems = uMaxEntry;
			info.fFlags = fBinary ? MRU_BINARY : 0;
			info.hKey = hkey;
			info.pszSubKey = pszSubKey;
			info.pfnCompare = NULL;
			m_hmru = m_CreateMRUListW(&info);
		}
		return (m_hmru != NULL);
	}

	void Close()
	{
		if (m_hmru && m_FreeMRUList) {
			m_FreeMRUList(m_hmru);
			m_hmru = NULL;
		}
	}

	int Enum(int nItem, void* pData, UINT uLen)
	{
		return (m_hmru && m_EnumMRUListW) ? m_EnumMRUListW(m_hmru, nItem, pData, uLen) : -1;
	}

	int GetCount()
	{
		return Enum(-1, NULL, 0);
	}

	int Add(LPCTSTR pszString)
	{
		return (m_hmru && m_AddMRUStringW) ? m_AddMRUStringW(m_hmru, pszString) : -1;
	}

	int AddBinary(void* pData, int len)
	{
		int res = -1;
		if (m_hmru && m_AddMRUStringW) {
			BYTE* tmp = new BYTE[len + sizeof(DWORD)];
			memcpy(tmp + sizeof(DWORD), pData, len);
			*((DWORD*)tmp) = len;
			res = m_AddMRUStringW(m_hmru, (LPCTSTR)tmp);
			delete [] tmp;
		}
		return res;
	}

	int Find(LPCTSTR pszString, LPINT pRegNum)
	{
		return (m_hmru && m_FindMRUListW) ? m_FindMRUListW(m_hmru, pszString, pRegNum) : -1;
	}

	int FindBinary(LPCVOID lpData, DWORD cbData, LPINT lpRegNum)
	{
		return (m_hmru && m_FindMRUData) ? m_FindMRUData(m_hmru, lpData, cbData, lpRegNum) : -1;
	}

	BOOL Del(int nItemPos)
	{
		return (m_hmru && m_DelMRUString) ? m_DelMRUString(m_hmru, nItemPos) : -1;
	}
};


#endif	// _SELFMRUPROC <-------------------------------------------------------
#endif	// __MRULIST_H__
