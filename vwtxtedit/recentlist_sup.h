///////////////////////////////////////////////////////////////////////////////
// CRecentFileList - MRU List Support

#pragma once
#ifndef __RECENTFILELIST_SUP_H__
#define __RECENTFILELIST_SUP_H__


#ifndef _WTL_MRUEMPTY_TEXT
  #define _WTL_MRUEMPTY_TEXT	_T("(empty)")
#endif


///////////////////////////////////////////////////////////////////////////////
// Global function for compacting a path by replacing parts with ellipsis

inline bool CompactPath(LPTSTR lpstrOut, LPCTSTR lpstrIn, int cchLen)
{
	ATLASSERT(lpstrOut != NULL);
	ATLASSERT(lpstrIn != NULL);
	ATLASSERT(cchLen > 0);

	LPCTSTR szEllipsis = _T("...");
	const int cchEndEllipsis = 3;
	const int cchMidEllipsis = 4;

	if (lstrlen(lpstrIn) < cchLen) {
		SecureHelper::strcpy_x(lpstrOut, cchLen, lpstrIn);
		return true;
	}

	lpstrOut[0] = 0;

	// check if the separator is a slash or a backslash
	TCHAR chSlash = _T('\\');
	for (LPTSTR lpstr = (LPTSTR)lpstrIn; *lpstr != 0; lpstr = ::CharNext(lpstr)) {
		if((*lpstr == _T('/')) || (*lpstr == _T('\\')))
			chSlash = *lpstr;
	}

	// find the filename portion of the path
	LPCTSTR lpstrFileName = lpstrIn;
	for (LPCTSTR pPath = lpstrIn; *pPath; pPath = ::CharNext(pPath)) {
		if((pPath[0] == _T('\\') || pPath[0] == _T(':') || pPath[0] == _T('/'))
				&& pPath[1] && pPath[1] != _T('\\') && pPath[1] != _T('/'))
			lpstrFileName = pPath + 1;
	}
	int cchFileName = lstrlen(lpstrFileName);

	// handle just the filename without a path
	if (lpstrFileName == lpstrIn && cchLen > cchEndEllipsis) {
		bool bRet = (SecureHelper::strncpy_x(lpstrOut, cchLen, lpstrIn, cchLen - cchEndEllipsis - 1) == 0);
		if (bRet) {
			SecureHelper::strcat_x(lpstrOut, cchLen, szEllipsis);
		}
		return bRet;
	}

	// handle just ellipsis
	if ((cchLen < (cchMidEllipsis + cchEndEllipsis))) {
		for(int i = 0; i < cchLen - 1; i++)
			lpstrOut[i] = ((i + 1) == cchMidEllipsis) ? chSlash : _T('.');
		lpstrOut[cchLen - 1] = 0;
		return true;
	}

	// calc how much we have to copy
	int cchToCopy = cchLen - (cchMidEllipsis + cchFileName) - 1;

	if(cchToCopy < 0)
		cchToCopy = 0;

	bool bRet = (SecureHelper::strncpy_x(lpstrOut, cchLen, lpstrIn, cchToCopy) == 0);
	if (!bRet)
		return false;

	// add ellipsis
	SecureHelper::strcat_x(lpstrOut, cchLen, szEllipsis);
	if (!bRet)
		return false;
	TCHAR szSlash[2] = { chSlash, 0 };
	SecureHelper::strcat_x(lpstrOut, cchLen, szSlash);
	if (!bRet)
		return false;

	// add filename (and ellipsis, if needed)
	if (cchLen > (cchMidEllipsis + cchFileName)) {
		SecureHelper::strcat_x(lpstrOut, cchLen, lpstrFileName);
	}
	else {
		cchToCopy = cchLen - cchMidEllipsis - cchEndEllipsis - 1;
		bRet = (SecureHelper::strncpy_x(&lpstrOut[cchMidEllipsis], cchLen - cchMidEllipsis, lpstrFileName, cchToCopy) == 0);
		if (bRet)
			SecureHelper::strcat_x(lpstrOut, cchLen, szEllipsis);
	}
	return bRet;
}


///////////////////////////////////////////////////////////////////////////////
//

template <class T, int t_cchItemLen = MAX_PATH, int t_nFirstID = ID_FILE_MRU_FIRST, int t_nLastID = ID_FILE_MRU_LAST>
class CRecentFileListBase
{
public:
// Declarations
	struct _DocEntry
	{
		TCHAR szDocName[t_cchItemLen];
		int charset;
		bool operator ==(const _DocEntry& de) const
		{ return (lstrcmpi(szDocName, de.szDocName) == 0); }
	};

	enum
	{
		m_nMaxEntries_Min = 2,
		m_nMaxEntries_Max = t_nLastID - t_nFirstID + 1,
		m_cchMaxItemLen_Min = 6,
		m_cchMaxItemLen_Max = t_cchItemLen,
		m_cchItemNameLen = 11
	};

// Data members
	ATL::CSimpleArray<_DocEntry> m_arrDocs;
	int m_nMaxEntries;   // default is 8
	HMENU m_hMenu;

	TCHAR m_szNoEntries[t_cchItemLen];

	int m_cchMaxItemLen;

// Constructor
	CRecentFileListBase()
		: m_hMenu(NULL)
		, m_nMaxEntries(8)
		, m_cchMaxItemLen(-1)
	{
		// These ASSERTs verify values of the template arguments
		ATLASSERT(t_cchItemLen > m_cchMaxItemLen_Min);
		ATLASSERT(m_nMaxEntries_Max > m_nMaxEntries_Min);
	}

// Attributes
	HMENU GetMenuHandle() const
	{
		return m_hMenu;
	}

	void SetMenuHandle(HMENU hMenu)
	{
		ATLASSERT(hMenu == NULL || ::IsMenu(hMenu));
		m_hMenu = hMenu;
		if(m_hMenu == NULL || (::GetMenuString(m_hMenu, t_nFirstID, m_szNoEntries, t_cchItemLen, MF_BYCOMMAND) == 0))
		{
			T* pT = static_cast<T*>(this);
			pT;   // avoid level 4 warning
			SecureHelper::strncpy_x(m_szNoEntries, _countof(m_szNoEntries), pT->GetMRUEmptyText(), _TRUNCATE);
		}
	}

	int GetMaxEntries() const
	{
		return m_nMaxEntries;
	}

	void SetMaxEntries(int nMaxEntries)
	{
		ATLASSERT(nMaxEntries >= m_nMaxEntries_Min && nMaxEntries <= m_nMaxEntries_Max);
		if(nMaxEntries < m_nMaxEntries_Min)
			nMaxEntries = m_nMaxEntries_Min;
		else if(nMaxEntries > m_nMaxEntries_Max)
			nMaxEntries = m_nMaxEntries_Max;
		m_nMaxEntries = nMaxEntries;
	}

	int GetMaxItemLength() const
	{
		return m_cchMaxItemLen;
	}

	void SetMaxItemLength(int cchMaxLen)
	{
		ATLASSERT((cchMaxLen >= m_cchMaxItemLen_Min && cchMaxLen <= m_cchMaxItemLen_Max) || cchMaxLen == -1);
		if(cchMaxLen != -1)
		{
			if(cchMaxLen < m_cchMaxItemLen_Min)
				cchMaxLen = m_cchMaxItemLen_Min;
			else if(cchMaxLen > m_cchMaxItemLen_Max)
				cchMaxLen = m_cchMaxItemLen_Max;
		}
		m_cchMaxItemLen = cchMaxLen;
		T* pT = static_cast<T*>(this);
		pT->UpdateMenu();
	}

// Operations
	BOOL AddToList(LPCTSTR lpstrDocName, int charset)
	{
		_DocEntry de;
		de.charset = charset;
		errno_t nRet = SecureHelper::strncpy_x(de.szDocName, _countof(de.szDocName), lpstrDocName, _TRUNCATE);
		if (nRet != 0 && nRet != STRUNCATE)
			return FALSE;

		for (int i = 0; i < m_arrDocs.GetSize(); i++) {
			if (lstrcmpi(m_arrDocs[i].szDocName, lpstrDocName) == 0) {
				m_arrDocs.RemoveAt(i);
				break;
			}
		}

		if (m_arrDocs.GetSize() == m_nMaxEntries)
			m_arrDocs.RemoveAt(0);

		BOOL bRet = m_arrDocs.Add(de);
		if (bRet) {
			T* pT = static_cast<T*>(this);
			bRet = pT->UpdateMenu();
		}
		return bRet;
	}

	// This function is deprecated because it is not safe. 
	// Use the version below that accepts the buffer length.
	__declspec(deprecated)
	BOOL GetFromList(int /*nItemID*/, LPTSTR /*lpstrDocName*/)
	{
		ATLASSERT(FALSE);
		return FALSE;
	}

	BOOL GetFromList(int nItemID, LPTSTR lpstrDocName, int cchLength, int& charset)
	{
		int nIndex = m_arrDocs.GetSize() - (nItemID - t_nFirstID) - 1;
		if (nIndex < 0 || nIndex >= m_arrDocs.GetSize())
			return FALSE;
		if (lstrlen(m_arrDocs[nIndex].szDocName) >= cchLength)
			return FALSE;
		SecureHelper::strcpy_x(lpstrDocName, cchLength, m_arrDocs[nIndex].szDocName);
		charset = m_arrDocs[nIndex].charset;
		return TRUE;
	}

	BOOL GetFromList(int nItemID, _CSTRING_NS::CString& strDocName, int& charset)
	{
		int nIndex = m_arrDocs.GetSize() - (nItemID - t_nFirstID) - 1;
		if (nIndex < 0 || nIndex >= m_arrDocs.GetSize())
			return FALSE;

		strDocName = m_arrDocs[nIndex].szDocName;
		charset = m_arrDocs[nIndex].charset;
		return TRUE;
	}

	int FindFromList(LPCTSTR lpstrDocName, int& charset)
	{
		int cnt = m_arrDocs.GetSize();
		for (int i = 0; i < cnt; i++) {
			if (!lstrcmpi(lpstrDocName, m_arrDocs[i].szDocName)) {
				charset = m_arrDocs[i].charset;
				// itemidを返す
				return (m_arrDocs.GetSize() - i + t_nFirstID - 1);
			}
		}
		return -1;
	}

	BOOL RemoveFromList(int nItemID)
	{
		int nIndex = m_arrDocs.GetSize() - (nItemID - t_nFirstID) - 1;
		BOOL bRet = m_arrDocs.RemoveAt(nIndex);
		if (bRet) {
			T* pT = static_cast<T*>(this);
			bRet = pT->UpdateMenu();
		}
		return bRet;
	}

	BOOL MoveToTop(int nItemID)
	{
		int nIndex = m_arrDocs.GetSize() - (nItemID - t_nFirstID) - 1;
		if (nIndex < 0 || nIndex >= m_arrDocs.GetSize())
			return FALSE;
		_DocEntry de;
		de = m_arrDocs[nIndex];
		m_arrDocs.RemoveAt(nIndex);
		BOOL bRet = m_arrDocs.Add(de);
		if (bRet) {
			T* pT = static_cast<T*>(this);
			bRet = pT->UpdateMenu();
		}
		return bRet;
	}

	BOOL ReadFromRegistry(LPCTSTR lpstrRegKey)
	{
		T* pT = static_cast<T*>(this);
		ATL::CRegKey rkParent;
		ATL::CRegKey rk;

		LONG lRet = rkParent.Open(HKEY_CURRENT_USER, lpstrRegKey);
		if (lRet != ERROR_SUCCESS)
			return FALSE;
		lRet = rk.Open(rkParent, pT->GetRegKeyName());
		if (lRet != ERROR_SUCCESS)
			return FALSE;

		DWORD dwRet = 0;
		lRet = rk.QueryDWORDValue(pT->GetRegCountName(), dwRet);
		if (lRet != ERROR_SUCCESS)
			return FALSE;
		SetMaxEntries(dwRet);

		m_arrDocs.RemoveAll();

		TCHAR szRetString[t_cchItemLen] = { 0 };
		_DocEntry de;

		for (int nItem = m_nMaxEntries; nItem > 0; nItem--) {
			TCHAR szBuff[m_cchItemNameLen] = { 0 };
			SecureHelper::wsprintf_x(szBuff, m_cchItemNameLen, pT->GetRegItemName(), nItem);
			ULONG ulCount = t_cchItemLen;
			lRet = rk.QueryStringValue(szBuff, szRetString, &ulCount);
			if (lRet == ERROR_SUCCESS) {
				de.charset = 0;
				LPTSTR psz = szRetString;
				if (*psz == _T('@')) {
					psz++;
					LPTSTR p = psz;
					while (_T('0') <= *psz && *psz <= _T('9')) {
						de.charset *= 10;
						de.charset += *psz - _T('0');
						psz++;
					}
					*psz++ = 0;
				}
				SecureHelper::strcpy_x(de.szDocName, _countof(de.szDocName), psz);
				m_arrDocs.Add(de);
			}
		}

		rk.Close();
		rkParent.Close();

		return pT->UpdateMenu();
	}

	BOOL WriteToRegistry(LPCTSTR lpstrRegKey)
	{
		T* pT = static_cast<T*>(this);
		pT;   // avoid level 4 warning
		ATL::CRegKey rkParent;
		ATL::CRegKey rk;

		LONG lRet = rkParent.Create(HKEY_CURRENT_USER, lpstrRegKey);
		if (lRet != ERROR_SUCCESS)
			return FALSE;
		lRet = rk.Create(rkParent, pT->GetRegKeyName());
		if (lRet != ERROR_SUCCESS)
			return FALSE;

		lRet = rk.SetDWORDValue(pT->GetRegCountName(), m_nMaxEntries);
		ATLASSERT(lRet == ERROR_SUCCESS);

		// set new values
		int nItem;
		for (nItem = m_arrDocs.GetSize(); nItem > 0; nItem--) {
			TCHAR szBuff[m_cchItemNameLen] = { 0 };
			SecureHelper::wsprintf_x(szBuff, m_cchItemNameLen, pT->GetRegItemName(), nItem);

			TCHAR szDocName[t_cchItemLen] = { 0 };
			int charset = 0;
			GetFromList(t_nFirstID + nItem - 1, szDocName, t_cchItemLen, charset);

			// "@xx,<filename>" の形式でレジストリに書き込む
			CString str;
			str.Format(_T("@%d,%s"), charset, szDocName);
			lRet = rk.SetStringValue(szBuff, str);
			ATLASSERT(lRet == ERROR_SUCCESS);
		}

		// delete unused keys
		for (nItem = m_arrDocs.GetSize() + 1; nItem < m_nMaxEntries_Max; nItem++) {
			TCHAR szBuff[m_cchItemNameLen] = { 0 };
			SecureHelper::wsprintf_x(szBuff, m_cchItemNameLen, pT->GetRegItemName(), nItem);
			rk.DeleteValue(szBuff);
		}

		rk.Close();
		rkParent.Close();

		return TRUE;
	}

// Implementation
	BOOL UpdateMenu()
	{
		if(m_hMenu == NULL)
			return FALSE;
		ATLASSERT(::IsMenu(m_hMenu));

		int nItems = ::GetMenuItemCount(m_hMenu);
		int nInsertPoint;
		for (nInsertPoint = 0; nInsertPoint < nItems; nInsertPoint++) {
			CMenuItemInfo mi;
			mi.fMask = MIIM_ID;
			::GetMenuItemInfo(m_hMenu, nInsertPoint, TRUE, &mi);
			if (mi.wID == t_nFirstID)
				break;
		}
		ATLASSERT(nInsertPoint < nItems && "You need a menu item with an ID = t_nFirstID");

		int nItem;
		for (nItem = t_nFirstID; nItem < t_nFirstID + m_nMaxEntries; nItem++) {
			// keep the first one as an insertion point
			if (nItem != t_nFirstID)
				::DeleteMenu(m_hMenu, nItem, MF_BYCOMMAND);
		}

		TCHAR szItemText[t_cchItemLen + 6] = { 0 };   // add space for &, 2 digits, and a space
		int nSize = m_arrDocs.GetSize();
		nItem = 0;
		if (nSize > 0) {
			for (nItem = 0; nItem < nSize; nItem++) {
				if (m_cchMaxItemLen == -1) {
					SecureHelper::wsprintf_x(szItemText, t_cchItemLen + 6, _T("&%i %s"), nItem + 1, m_arrDocs[nSize - 1 - nItem].szDocName);
				}
				else {
					TCHAR szBuff[t_cchItemLen] = { 0 };
					T* pT = static_cast<T*>(this);
					pT;   // avoid level 4 warning
					bool bRet = pT->CompactDocumentName(szBuff, m_arrDocs[nSize - 1 - nItem].szDocName, m_cchMaxItemLen);
					bRet;   // avoid level 4 warning
					ATLASSERT(bRet);
					SecureHelper::wsprintf_x(szItemText, t_cchItemLen + 6, _T("&%i %s"), nItem + 1, szBuff);
				}
				::InsertMenu(m_hMenu, nInsertPoint + nItem, MF_BYPOSITION | MF_STRING, t_nFirstID + nItem, szItemText);
			}
		}
		else {	// empty
			::InsertMenu(m_hMenu, nInsertPoint, MF_BYPOSITION | MF_STRING, t_nFirstID, m_szNoEntries);
			::EnableMenuItem(m_hMenu, t_nFirstID, MF_GRAYED);
			nItem++;
		}
		::DeleteMenu(m_hMenu, nInsertPoint + nItem, MF_BYPOSITION);

		return TRUE;
	}

// Overrideables
	// override to provide a different method of compacting document names
	static bool CompactDocumentName(LPTSTR lpstrOut, LPCTSTR lpstrIn, int cchLen)
	{
		return CompactPath(lpstrOut, lpstrIn, cchLen);
	}

	static LPCTSTR GetRegKeyName()
	{
		return _T("Recent File List");
	}

	static LPCTSTR GetRegCountName()
	{
		return _T("FileCount");
	}

	static LPCTSTR GetRegItemName()
	{
		// Note: This string is a format string used with wsprintf().
		// Resulting formatted string must be m_cchItemNameLen or less 
		// characters long, including the terminating null character.
		return _T("Document%i");
	}

	static LPCTSTR GetMRUEmptyText()
	{
		return _WTL_MRUEMPTY_TEXT;
	}
};

class CRecentFileList : public CRecentFileListBase<CRecentFileList>
{
public:
// nothing here
};



#endif // __RECENTFILELIST_SUP_H__

