////////////////////////////////////////////////////////////////////
// fileitem.h

#pragma once
#ifndef __FILEITEM_H__
#define __FILEITEM_H__

#define MAXHIAR 				16

long __fastcall _comparefiletime(const FILETIME* pftSrc, const FILETIME* pftDst, BOOL bRaw);
int __fastcall FormatFileDate(FILETIME* pft, LPTSTR pszDate, int cch);
int __fastcall FormatFileTime(FILETIME* pft, LPTSTR pszTime, int cch);
int __fastcall FormatFileDateTime(FILETIME* pft, LPTSTR pszDate, int cch);
int __fastcall FormatFileDateTimeWin(FILETIME* pft, LPTSTR pszDate, int cchMax);


///////////////////////////////////////////////////////////////////////
// CWin32FileDataクラス
//

#ifndef _CHECKDOT
	#define _CHECKDOT(a)	((a)[0]=='.' && ((a)[1]==0 || ((a)[1]=='.' && (a)[2]==0)))
#endif

typedef const WIN32_FIND_DATA* LPCWIN32_FIND_DATA;

class CWin32FindData : public WIN32_FIND_DATA
{
public:
	CWin32FindData() { }
	__inline CWin32FindData(const WIN32_FIND_DATA& srcData) { *this = srcData; }

	__inline BOOL IsFolder() { return (dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY); }
	__inline BOOL IsDot() { return _CHECKDOT(cFileName); }
	__inline BOOL IsSysHidden() { return (dwFileAttributes & (FILE_ATTRIBUTE_SYSTEM|FILE_ATTRIBUTE_HIDDEN)); }
	__inline BOOL IsAttrib(DWORD dwAttr) { return (dwFileAttributes & dwAttr); }
	__inline BOOL LocalCreationTime(LPFILETIME pft) { return FileTimeToLocalFileTime(&ftCreationTime, pft); }
	__inline BOOL LocalLastWriteTime(LPFILETIME pft) { return FileTimeToLocalFileTime(&ftLastWriteTime, pft); }
	__inline BOOL LocalLastAccessTime(LPFILETIME pft) { return FileTimeToLocalFileTime(&ftLastAccessTime, pft); }

	__inline long CompareLastWriteTime(const CWin32FindData& rw, BOOL bRaw) { return _comparefiletime(&ftLastWriteTime, &rw.ftLastWriteTime, bRaw); }

	// convert between CRect and LPRECT/LPCRECT (no need for &)
	__inline operator LPWIN32_FIND_DATA() { return this; }
	__inline operator LPCWIN32_FIND_DATA() const { return this; }

	__inline void operator=(const WIN32_FIND_DATA& srcData) { memcpy(this, &srcData, sizeof(WIN32_FIND_DATA)); }
};



////////////////////////////////////////////////////////////////////
// CFileFind クラス

class CFileFinder : public CWin32FindData
{
private:
	HANDLE m_hFind;
public:
	__inline CFileFinder() { m_hFind = NULL; }
	__inline ~CFileFinder() { if (m_hFind) FindClose(m_hFind); }
	BOOL BeginFind(LPCTSTR pFilter)
	{
		m_hFind = ::FindFirstFile(pFilter, this);
		if (m_hFind == INVALID_HANDLE_VALUE)
			m_hFind = NULL;
		return m_hFind ? TRUE : FALSE;
	}
	BOOL BeginFind(LPCTSTR pszTargetDir, LPCTSTR pszFilter)
	{
		CString str;
		LPTSTR p = str.GetBuffer(lstrlen(pszTargetDir) + lstrlen(pszFilter) + 8);
		PathCombine(p, pszTargetDir, pszFilter);
		str.ReleaseBuffer();
		return BeginFind(str);
	}
	__inline BOOL NextFind() { return (FindNextFile(m_hFind, this) || GetLastError() == ERROR_MORE_DATA); }
	__inline void EndFind() { FindClose(m_hFind); m_hFind = NULL; }
};


////////////////////////////////////////////////////////////////////
// CLASS CFileInfo : ファイルクラス
//
class CFileInfo
{
// コンストラクタ 
public:
	CFileInfo()
	{
		m_strFullPathName.Empty();
		m_pszName = NULL;
		m_dwAttr = 0;
		m_dlSize = 0;
		memset(&m_ftLastWriteTime, 0, sizeof(FILETIME));
		memset(&m_ftCreationTime, 0, sizeof(FILETIME));
	}
	__inline CFileInfo(const CFileInfo& a) { CopyFrom(a); }

// デストラクタ
public:
	~CFileInfo()
	{
	}

	void CopyFrom(const CFileInfo& a)
	{
		if (&a != this) {
			m_strFullPathName = a.m_strFullPathName;
			m_pszName = PathFindFileName(m_strFullPathName);
			if (!m_pszName) m_pszName = m_strFullPathName;
			m_dwAttr = a.m_dwAttr;
			m_dlSize = a.m_dlSize;
			m_ftLastWriteTime = a.m_ftLastWriteTime;
			m_ftCreationTime = a.m_ftCreationTime;
		}
	}

	__inline CFileInfo& operator=(const CFileInfo& a) { CopyFrom(a); return *this; }
	LPTSTR GetFullPathName(LPTSTR pszFullPath, int cchMax)
	{
		return lstrcpyn(pszFullPath, m_strFullPathName, cchMax);
	}

public:
	void SetFile(WIN32_FIND_DATA& wfd, LPCTSTR pszPath = NULL)
	{
		if (pszPath) {
			LPTSTR p = m_strFullPathName.GetBuffer(MAX_PATH * 2);
			PathCombine(p, pszPath, wfd.cFileName);
			m_strFullPathName.ReleaseBuffer();
			m_pszName = PathFindFileName(m_strFullPathName);
		}
		else {
			m_strFullPathName = wfd.cFileName;
			m_pszName = m_strFullPathName;
		}
		SETDWORDLONG(m_dlSize, wfd.nFileSizeLow, wfd.nFileSizeHigh);
		m_dwAttr = wfd.dwFileAttributes;
		FileTimeToLocalFileTime(&wfd.ftCreationTime, &m_ftCreationTime);
		FileTimeToLocalFileTime(&wfd.ftLastWriteTime, &m_ftLastWriteTime);
	}

	int FormatSize(LPTSTR pszSize, int cch);
	inline int FormatDate(LPTSTR pszDate, int cch) { return FormatFileDate(&m_ftLastWriteTime, pszDate, cch); }
	inline int FormatTime(LPTSTR pszTime, int cch) { return FormatFileTime(&m_ftLastWriteTime, pszTime, cch); }
	inline int FormatDateTime(LPTSTR pszDate, int cch) { return FormatFileDateTime(&m_ftLastWriteTime, pszDate, cch); }
	inline int FormatDateTimeWin(LPTSTR pszDate, int cchMax) { return FormatFileDateTimeWin(&m_ftLastWriteTime, pszDate, cchMax); }
	inline int FormatAttr(LPTSTR pszAttr) { lccFileAttrToStr(m_dwAttr, pszAttr); return lstrlen(pszAttr); }
	inline long CompareLastWriteTime(const CFileInfo& rf, BOOL bRaw) { return _comparefiletime(&m_ftLastWriteTime, &rf.m_ftLastWriteTime, bRaw); }
	inline BOOL IsAttrib(DWORD dwAttr) { return (m_dwAttr & dwAttr); }
	inline BOOL IsFolder() { return (m_dwAttr & FILE_ATTRIBUTE_DIRECTORY); }
	inline BOOL IsSysHidden() { return (m_dwAttr & (FILE_ATTRIBUTE_SYSTEM | FILE_ATTRIBUTE_HIDDEN)); }

public:
	CString m_strFullPathName;
	LPCTSTR m_pszName;
	DWORDLONG m_dlSize;
	FILETIME m_ftCreationTime;
	FILETIME m_ftLastWriteTime;
	DWORD m_dwAttr;
};


/////////////////////////////////////////////////////////////////////
// CFileArray

class CFileArray : public CAtlArray<CFileInfo*>
{
public:
	CFileArray() { }
	~CFileArray() { DeleteAll(); }

	BOOL AddFile(LPCTSTR lpFile);
	BOOL AddFile(CFileInfo& fi);
	int GetItemNameIndex(LPCTSTR lpName);
	int Filter(LPCTSTR lpFilter, BOOL bExclude, BOOL bRangeCase = FALSE);
	int BrowseFiles(LPCTSTR lpPath, LPCTSTR lpFilter = NULL, BOOL bUnmatchFilter = FALSE, BOOL bRangeCase = FALSE, BOOL bCaseSense = FALSE);
	int EnumSubFolder(LPCTSTR lpPath);
	void Sort();
	void DeleteAll();

public:
	HDROP MakeDropFiles();
	BOOL IsFolderInclude();
};

/////////////////////////////////////////////////////////////////////
// CDropFiles

class CDropFiles
{
public:
	HDROP m_hDrop;
	int m_nEntry;
private:
	DWORD m_dropEffect;
public:
	CDropFiles()
		: m_hDrop(NULL)
		, m_nEntry(0)
		, m_dropEffect(DROPEFFECT_NONE)
	{
	}
	CDropFiles(HDROP hDrop, DWORD eff = DROPEFFECT_NONE);
	CDropFiles(LPCTSTR pszFile, DWORD eff = DROPEFFECT_NONE);
	CDropFiles(CFileInfo* pf, DWORD eff = DROPEFFECT_NONE);
	CDropFiles(CFileArray* pArray, DWORD eff = DROPEFFECT_NONE);
	__inline ~CDropFiles() { if (m_hDrop) ::DragFinish(m_hDrop); }

	HDROP Attach(HDROP hDrop, DWORD eff = DROPEFFECT_NONE);
	HDROP Detach();

	void DragFinish();
	BOOL DragQueryFile(int nIndex, LPTSTR lpFile, int cchMax);
	UINT DragQueryFile(int nIndex, CString& string);
	BOOL DragInitHeader(POINT pt, BOOL fInNonClientArea);
	BOOL DragAppendFile(LPCTSTR pszPath);
	__inline BOOL DragAppendFile(CFileInfo* pf) { return DragAppendFile(pf->m_strFullPathName); }
	int EnumItemToStr(CString& csFiles);
	LPTSTR MakeFileListBuffer(int* numFiles = NULL);
	BOOL IsFolderInclude();
	__inline int size() { return m_nEntry; }
	DWORD effect() { return m_dropEffect; }

	__inline operator HDROP() const { return m_hDrop; }
};




/////////////////////////////////////////////////////////////////////
// CEnterFolder

class CEnterFolder
{
private:
	TCHAR m_szOldPath[MAX_PATH * 2];
public:
	CEnterFolder(LPCTSTR lpNewPath)
	{
		GetCurrentDirectory(ARRAYSIZE(m_szOldPath), m_szOldPath);
		if (!SetCurrentDirectory(lpNewPath))
			m_szOldPath[0] = 0;
	}
	__inline ~CEnterFolder() { Exit(); }
	void Exit()
	{
		if (m_szOldPath[0]) {
			SetCurrentDirectory(m_szOldPath);
			m_szOldPath[0] = 0;
		}
	}
	__inline BOOL IsFolderChanged() { return (m_szOldPath[0] != 0); }
};


#endif			// __FILEITEM_H__
