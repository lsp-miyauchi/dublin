/////////////////////////////////////////////////////////////////////////////
// recentfilelist

#include "stdafx.h"
#include "reg.h"
#include "recentfilelist.h"


/////////////////////////////////////////////////////////////////////////////
// lpszCanon = C:\MYAPP\DEBUGS\C\TESWIN.C
//
// cchMax   b   Result
// ------   -   ---------
//  1- 7    F   <empty>
//  1- 7    T   TESWIN.C
//  8-14    x   TESWIN.C
// 15-16    x   C:\...\TESWIN.C
// 17-23    x   C:\...\C\TESWIN.C
// 24-25    x   C:\...\DEBUGS\C\TESWIN.C
// 26+      x   C:\MYAPP\DEBUGS\C\TESWIN.C

static void _AbbreviateName(LPTSTR lpszCanon, int cchMax, BOOL bAtLeastName)
{
	int cchFullPath, cchFileName, cchVolName;
	const TCHAR* lpszCur;
	const TCHAR* lpszBase;
	const TCHAR* lpszFileName;

	lpszBase = lpszCanon;
	cchFullPath = lstrlen(lpszCanon);

	cchFileName = AfxGetFileName(lpszCanon, NULL, 0) - 1;
	lpszFileName = lpszBase + (cchFullPath-cchFileName);

	// If cchMax is more than enough to hold the full path name, we're done.
	// This is probably a pretty common case, so we'll put it first.
	if (cchMax >= cchFullPath)
		return;

	// If cchMax isn't enough to hold at least the basename, we're done
	if (cchMax < cchFileName)
	{
		lstrcpy(lpszCanon, (bAtLeastName) ? lpszFileName : _T(""));
		return;
	}

	// Calculate the length of the volume name.  Normally, this is two characters
	// (e.g., "C:", "D:", etc.), but for a UNC name, it could be more (e.g.,
	// "\\server\share").
	//
	// If cchMax isn't enough to hold at least <volume_name>\...\<base_name>, the
	// result is the base filename.

	lpszCur = lpszBase + 2;                 // Skip "C:" or leading "\\"

	if (lpszBase[0] == '\\' && lpszBase[1] == '\\') // UNC pathname
	{
		// First skip to the '\' between the server name and the share name,
		while (*lpszCur != '\\')
		{
			lpszCur = _tcsinc(lpszCur);
			ATLASSERT(*lpszCur != '\0');
		}
	}
	// if a UNC get the share name, if a drive get at least one directory
	ATLASSERT(*lpszCur == '\\');
	// make sure there is another directory, not just c:\filename.ext
	if (cchFullPath - cchFileName > 3)
	{
		lpszCur = _tcsinc(lpszCur);
		while (*lpszCur != '\\')
		{
			lpszCur = _tcsinc(lpszCur);
			ATLASSERT(*lpszCur != '\0');
		}
	}
	ATLASSERT(*lpszCur == '\\');

	cchVolName = int(lpszCur - lpszBase);
	if (cchMax < cchVolName + 5 + cchFileName)
	{
		lstrcpy(lpszCanon, lpszFileName);
		return;
	}

	// Now loop through the remaining directory components until something
	// of the form <volume_name>\...\<one_or_more_dirs>\<base_name> fits.
	//
	// Assert that the whole filename doesn't fit -- this should have been
	// handled earlier.

	ATLASSERT(cchVolName + (int)lstrlen(lpszCur) > cchMax);
	while (cchVolName + 4 + (int)lstrlen(lpszCur) > cchMax)
	{
		do
		{
			lpszCur = _tcsinc(lpszCur);
			ATLASSERT(*lpszCur != '\0');
		}
		while (*lpszCur != '\\');
	}

	// Form the resultant string and we're done.
	lpszCanon[cchVolName] = '\0';
	lstrcat(lpszCanon, _T("\\..."));
	lstrcat(lpszCanon, lpszCur);
}

/////////////////////////////////////////////////////////////////////////////
// CRecentFileList

#pragma warning(disable: 4267)  //REVIEW operator new[] problem
CRecentFileList::CRecentFileList(UINT nStart, LPCTSTR lpszSection,
	LPCTSTR lpszEntryFormat, int nSize, int nMaxDispLen)
{
	ATLASSERT(nSize != 0);
	m_arrNames = new CString[nSize];
	m_nSize = nSize;

	m_nStart = nStart;
	m_strSectionName = lpszSection;
	m_strEntryFormat = lpszEntryFormat;
	m_nMaxDisplayLength = nMaxDispLen;
}

CRecentFileList::~CRecentFileList()
{
	delete[] m_arrNames;
}

// Operations
void CRecentFileList::Add(LPCTSTR lpszPathName)
{
	ATLASSERT(m_arrNames != NULL);
	ATLASSERT(AfxIsValidString(lpszPathName));

	// fully qualify the path name
	TCHAR szTemp[_MAX_PATH];
	if ( lpszPathName == NULL || lstrlen(lpszPathName) >= _MAX_PATH )
	{
		ATLASSERT(FALSE);
		// MFC requires paths with length < _MAX_PATH
		// No other way to handle the error from a void function
		AfxThrowFileException(CFileException::badPath);
	}

	if( AfxFullPath(szTemp, lpszPathName) == FALSE )
	{
		ATLASSERT(FALSE);
		// MFC requires paths with length < _MAX_PATH
		// No other way to handle the error from a void function
		AfxThrowFileException(CFileException::badPath);
	}
	
	// update the MRU list, if an existing MRU string matches file name
	int iMRU;
	for (iMRU = 0; iMRU < m_nSize-1; iMRU++)
	{
		if (AfxComparePath(m_arrNames[iMRU], szTemp))
			break;      // iMRU will point to matching entry
	}
	// move MRU strings before this one down
	for (; iMRU > 0; iMRU--)
	{
		ATLASSERT(iMRU > 0);
		ATLASSERT(iMRU < m_nSize);
		m_arrNames[iMRU] = m_arrNames[iMRU-1];
	}
	// place this one at the beginning
	m_arrNames[0] = szTemp;
}

void CRecentFileList::Remove(int nIndex)
{
	ATLASSERT(nIndex >= 0);
	ATLASSERT(nIndex < m_nSize);

	m_arrNames[nIndex].Empty();
	int iMRU;
	for (iMRU = nIndex; iMRU < m_nSize-1; iMRU++)
		m_arrNames[iMRU] = m_arrNames[iMRU+1];

	ATLASSERT(iMRU < m_nSize);
	m_arrNames[iMRU].Empty();
}

BOOL CRecentFileList::GetDisplayName(CString& strName, int nIndex,
	LPCTSTR lpszCurDir, int nCurDir, BOOL bAtLeastName) const
{
	ATLASSERT(lpszCurDir == NULL || AfxIsValidString(lpszCurDir, nCurDir));

	ATLASSERT(m_arrNames != NULL);
	ATLASSERT(nIndex < m_nSize);
	if (lpszCurDir == NULL || m_arrNames[nIndex].IsEmpty())
		return FALSE;

	int nLenName = m_arrNames[nIndex].GetLength();
	LPTSTR lpch = strName.GetBuffer( nLenName + 1);
	if (lpch == NULL)
	{
		AfxThrowMemoryException();
		return FALSE;
	}
	lstrcpyn(lpch, m_arrNames[nIndex],nLenName + 1 );
	// nLenDir is the length of the directory part of the full path
	int nLenDir = nLenName - (AfxGetFileName(lpch, NULL, 0) - 1);
	BOOL bSameDir = FALSE;
	if (nLenDir == nCurDir)
	{
		TCHAR chSave = lpch[nLenDir];
		lpch[nCurDir] = 0;  // terminate at same location as current dir
		bSameDir = lstrcmpi(lpszCurDir, lpch) == 0;
		lpch[nLenDir] = chSave;
	}
	// copy the full path, otherwise abbreviate the name
	if (bSameDir)
	{
		// copy file name only since directories are same
		TCHAR szTemp[_MAX_PATH];
		AfxGetFileTitle(lpch+nCurDir, szTemp, _countof(szTemp));
		lstrcpyn(lpch, szTemp, _MAX_PATH);
	}
	else if (m_nMaxDisplayLength != -1)
	{
		// strip the extension if the system calls for it
		TCHAR szTemp[_MAX_PATH];
		AfxGetFileTitle(lpch+nLenDir, szTemp, _countof(szTemp));
		lstrcpyn(lpch+nLenDir, szTemp, _MAX_PATH-nLenDir);

		// abbreviate name based on what will fit in limited space
		_AbbreviateName(lpch, m_nMaxDisplayLength, bAtLeastName);
	}
	strName.ReleaseBuffer();
	return TRUE;
}

void CRecentFileList::UpdateMenu(CCmdUI* pCmdUI)
{
	ATLASSERT(m_arrNames != NULL);

	CMenu* pMenu = pCmdUI->m_pMenu;
	if (m_strOriginal.IsEmpty() && pMenu != NULL)
		pMenu->GetMenuString(pCmdUI->m_nID, m_strOriginal, MF_BYCOMMAND);

	if (m_arrNames[0].IsEmpty())
	{
		// no MRU files
		if (!m_strOriginal.IsEmpty())
			pCmdUI->SetText(m_strOriginal);
		pCmdUI->Enable(FALSE);
		return;
	}

	if (pCmdUI->m_pMenu == NULL)
		return;

	int iMRU;
	for (iMRU = 0; iMRU < m_nSize; iMRU++)
		pCmdUI->m_pMenu->DeleteMenu(pCmdUI->m_nID + iMRU, MF_BYCOMMAND);

	TCHAR szCurDir[_MAX_PATH];
	DWORD dwDirLen = GetCurrentDirectory(_MAX_PATH, szCurDir);
	if( dwDirLen == 0 || dwDirLen >= _MAX_PATH )
		return;	// Path too long

	int nCurDir = lstrlen(szCurDir);
	ATLASSERT(nCurDir >= 0);
	szCurDir[nCurDir] = '\\';
	szCurDir[++nCurDir] = '\0';

	CString strName;
	CString strTemp;
	for (iMRU = 0; iMRU < m_nSize; iMRU++) {
		if (!GetDisplayName(strName, iMRU, szCurDir, nCurDir))
			break;

		// double up any '&' characters so they are not underlined
		LPCTSTR lpszSrc = strName;
		LPTSTR lpszDest = strTemp.GetBuffer(strName.GetLength()*2);
		while (*lpszSrc != 0) {
			if (*lpszSrc == '&')
				*lpszDest++ = '&';
			if (_istlead(*lpszSrc))
				*lpszDest++ = *lpszSrc++;
			*lpszDest++ = *lpszSrc++;
		}
		*lpszDest = 0;
		strTemp.ReleaseBuffer();

		// insert mnemonic + the file name
		TCHAR buf[10];
		int nItem = (iMRU + 1 + m_nStart) % _AFX_MRU_MAX_COUNT;

		// number &1 thru &9, then 1&0, then 11 thru ...
		if (nItem > 10)
			wsprintf(buf, _T("%d "), nItem);
		else if (nItem == 10)
			lstrcpy(buf, _T("1&0 "));
		else
			wsprintf(buf, _T("&%d "), nItem);

		pCmdUI->m_pMenu->InsertMenu(pCmdUI->m_nIndex++,
			MF_STRING | MF_BYPOSITION, pCmdUI->m_nID++,
			CString(buf) + strTemp);
	}

	// update end menu count
	pCmdUI->m_nIndex--; // point to last menu added
	pCmdUI->m_nIndexMax = pCmdUI->m_pMenu->GetMenuItemCount();

	pCmdUI->m_bEnableChanged = TRUE;    // all the added items are enabled
}

void CRecentFileList::WriteList()
{
	ATLASSERT(m_arrNames != NULL);
	ATLASSERT(!m_strSectionName.IsEmpty());
	ATLASSERT(!m_strEntryFormat.IsEmpty());
	LPTSTR pszEntry = new TCHAR[m_strEntryFormat.GetLength()+10];
	SetRegText(m_strSectionName, NULL, NULL);
	for (int iMRU = 0; iMRU < m_nSize; iMRU++) {
		wsprintf(pszEntry, m_strEntryFormat, iMRU + 1);
		if (!m_arrNames[iMRU].IsEmpty()) {
			SetRegText(m_strSectionName, pszEntry, m_arrNames[iMRU]);
		}
	}
	delete[] pszEntry;
}

void CRecentFileList::ReadList()
{
	ATLASSERT(m_arrNames != NULL);
	ATLASSERT(!m_strSectionName.IsEmpty());
	ATLASSERT(!m_strEntryFormat.IsEmpty());
	LPTSTR pszEntry = new TCHAR[m_strEntryFormat.GetLength()+10];
	for (int iMRU = 0; iMRU < m_nSize; iMRU++) {
		wsprintf(pszEntry, m_strEntryFormat, iMRU + 1);
		m_arrNames[iMRU] = GetRegText(m_strSectionName, pszEntry, _T(""));
	}
	delete[] pszEntry;
}

