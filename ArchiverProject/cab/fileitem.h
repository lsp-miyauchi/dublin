////////////////////////////////////////////////////////////////////
// fileitem.h

#pragma once
#ifndef __FILEITEM_H__
#define __FILEITEM_H__

#define ISDOT(a)		(((a)[0]=='.'&&(a)[1]==0)||((a)[0]=='.'&&(a)[1]=='.'&&(a)[2]==0))


int WStrToStr(
			LPCWSTR pwz,
			CStringA& str
	);

////////////////////////////////////////////////////////////////////
//

typedef const WIN32_FIND_DATA* LPCWIN32_FIND_DATA;

class CWin32FindData : public WIN32_FIND_DATA
{
public:
	CWin32FindData() { }
	__inline CWin32FindData(const WIN32_FIND_DATA& srcData) { *this = srcData; }

	__inline BOOL IsFolder() { return (dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY); }
	__inline BOOL IsSysHidden() { return (dwFileAttributes & (FILE_ATTRIBUTE_SYSTEM | FILE_ATTRIBUTE_HIDDEN)); }
	__inline BOOL IsHidden() { return (dwFileAttributes & FILE_ATTRIBUTE_HIDDEN); }
	__inline BOOL IsSystem() { return (dwFileAttributes & FILE_ATTRIBUTE_SYSTEM); }
	__inline BOOL IsAttrib(DWORD dwAttr) { return (dwFileAttributes & dwAttr); }
	__inline DWORD Attrib() { return dwFileAttributes; }
	__inline BOOL LocalCreationTime(LPFILETIME lpft) { return FileTimeToLocalFileTime(&ftCreationTime, lpft); }
	__inline BOOL LocalLastWriteTime(LPFILETIME lpft) { return FileTimeToLocalFileTime(&ftLastWriteTime, lpft); }
	__inline BOOL LocalLastAccessTime(LPFILETIME lpft) { return FileTimeToLocalFileTime(&ftLastAccessTime, lpft); }

	BOOL IsDot()
	{
		return (IsFolder() && ISDOT(cFileName));
	}

	// convert between CRect and LPRECT/LPCRECT (no need for &)
	__inline operator LPWIN32_FIND_DATA() { return this; }
	__inline operator LPCWIN32_FIND_DATA() const { return this; }
	__inline void operator=(const WIN32_FIND_DATA& srcData) { memcpy(this, &srcData, sizeof(WIN32_FIND_DATA)); }
	void CopyFrom(const WIN32_FIND_DATAA* src)
	{
		memset(this, 0, sizeof(WIN32_FIND_DATAW));
		dwFileAttributes = src->dwFileAttributes;
		ftCreationTime = src->ftCreationTime;
		ftLastAccessTime = src->ftLastAccessTime;
		ftLastWriteTime = src->ftLastWriteTime;
		nFileSizeHigh = src->nFileSizeHigh;
		nFileSizeLow = src->nFileSizeLow;
		MultiByteToWideChar(CP_ACP, 0, src->cFileName, -1, cFileName, ARRAYSIZE(cFileName));
		MultiByteToWideChar(CP_ACP, 0, src->cAlternateFileName, -1, cAlternateFileName, ARRAYSIZE(cAlternateFileName));
	}
};

////////////////////////////////////////////////////////////////////
// CFileFinder �N���X

class CFileFinder : public CWin32FindData
{
public:
	CFileFinder() : m_hFind(NULL)
	{
	}
	~CFileFinder()
	{
		if (m_hFind) FindClose(m_hFind);
	}
	BOOL BeginFind(LPCWSTR pszFilter = NULL)
	{
		if (!pszFilter || *pszFilter == 0)
			pszFilter = L"*.*";
		m_hFind = FindFirstFile(pszFilter, this);
		return (m_hFind != 0);
	}
	__inline BOOL NextFind() { return FindNextFile(m_hFind, this); }
	__inline void EndFind() { FindClose(m_hFind); m_hFind = NULL; }

	HANDLE m_hFind;
};

#endif		// __FILEITEM_H__
