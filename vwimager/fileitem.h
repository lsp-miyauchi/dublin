///////////////////////////////////////////////////////////////////////
// CWin32FileDataクラス
//

#pragma once
#ifndef __FILEITEM_H__
#define __FILEITEM_H__

////////////////////////////////////////////////////////////////////
//

typedef const WIN32_FIND_DATA* LPCWIN32_FIND_DATA;

class CWin32FindData : public WIN32_FIND_DATA
{
public:
	inline CWin32FindData() { }
	inline CWin32FindData(const WIN32_FIND_DATA& srcData) { *this = srcData; }

	inline BOOL IsFolder() { return (dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY); }
	inline BOOL IsDot() { return (cFileName[0]=='.' && (cFileName[1]==0 || (cFileName[1]=='.' && cFileName[2]==0))); }
	inline BOOL IsSysHidden() { return (dwFileAttributes & (FILE_ATTRIBUTE_SYSTEM | FILE_ATTRIBUTE_HIDDEN)); }
	inline BOOL IsSystem() { return (dwFileAttributes & FILE_ATTRIBUTE_SYSTEM); }
	inline BOOL IsHidden() { return (dwFileAttributes & FILE_ATTRIBUTE_HIDDEN); }
	inline BOOL IsReparsePoint() { return (dwFileAttributes & FILE_ATTRIBUTE_REPARSE_POINT); }
	inline BOOL IsAttrib(DWORD dwAttr) { return (dwFileAttributes & dwAttr); }
	inline BOOL LocalCreationTime(LPFILETIME lpft) { return FileTimeToLocalFileTime(&ftCreationTime, lpft); }
	inline BOOL LocalLastWriteTime(LPFILETIME lpft) { return FileTimeToLocalFileTime(&ftLastWriteTime, lpft); }
	inline BOOL LocalLastAccessTime(LPFILETIME lpft) { return FileTimeToLocalFileTime(&ftLastAccessTime, lpft); }

	// convert between CRect and LPRECT/LPCRECT (no need for &)
	inline operator LPWIN32_FIND_DATA() { return this; }
	inline operator LPCWIN32_FIND_DATA() const { return this; }
	inline void operator=(const WIN32_FIND_DATA& srcData) { memcpy(this, &srcData, sizeof(WIN32_FIND_DATA)); }
};


////////////////////////////////////////////////////////////////////
// CFileFinder クラス

class CFileFinder : public CWin32FindData
{
public:
	inline CFileFinder() { m_hFind = NULL; }
	inline ~CFileFinder() { if (m_hFind) FindClose(m_hFind); }
	BOOL BeginFind(LPCTSTR pszFilter)
	{
		m_hFind = FindFirstFile(pszFilter, this);
		if (m_hFind == INVALID_HANDLE_VALUE) m_hFind = NULL;
		return (m_hFind != NULL);
	}
	inline BOOL NextFind() { return (FindNextFile(m_hFind, this) || GetLastError() == ERROR_MORE_DATA); }
	inline void EndFind() { FindClose(m_hFind); m_hFind = NULL; }
	HANDLE m_hFind;
};


#endif		// __FILEITEM_H__
