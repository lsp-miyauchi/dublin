////////////////////////////////////////////////////////////////////
// fileitem.h

#pragma once
#ifndef __FILEITEM_H__
#define __FILEITEM_H__

#include "matchspec.h"

#define FBR_FILE				0x0001
#define FBR_DIR					0x0002
#define FBR_BOTH				(FBR_FILE | FBR_DIR)

////////////////////////////////////////////////////////////////////
// DisplayNameType
//
enum DisplayNameType {
	DNT_NORMAL,
	DNT_LOWER,
	DNT_TITLE
};

////////////////////////////////////////////////////////////////////
// CWin32FindData

typedef const WIN32_FIND_DATA* LPCWIN32_FIND_DATA;

class CWin32FindData : public WIN32_FIND_DATA
{
public:
	__inline CWin32FindData() { }
	__inline CWin32FindData(const WIN32_FIND_DATA& srcData) { *this = srcData; }

	__inline BOOL IsDot() { return (cFileName[0]=='.' && (cFileName[1]==0 || (cFileName[1]=='.' && cFileName[2]==0))); }
	__inline BOOL IsFolder() { return (dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY); }
	__inline BOOL IsSysHidden() { return (dwFileAttributes & (FILE_ATTRIBUTE_SYSTEM | FILE_ATTRIBUTE_HIDDEN)); }
	__inline BOOL IsHidden() { return (dwFileAttributes & FILE_ATTRIBUTE_HIDDEN); }
	__inline BOOL IsSystem() { return (dwFileAttributes & FILE_ATTRIBUTE_SYSTEM); }
	__inline BOOL IsAttrib(DWORD dwAttr) { return (dwFileAttributes & dwAttr); }
	__inline BOOL LocalCreationTime(LPFILETIME pft) { return FileTimeToLocalFileTime(&ftCreationTime, pft); }
	__inline BOOL LocalLastWriteTime(LPFILETIME pft) { return FileTimeToLocalFileTime(&ftLastWriteTime, pft); }
	__inline BOOL LocalLastAccessTime(LPFILETIME pft) { return FileTimeToLocalFileTime(&ftLastAccessTime, pft); }

	// convert between CRect and LPRECT/LPCRECT (no need for &)
	__inline operator LPWIN32_FIND_DATA() { return this; }
	__inline operator LPCWIN32_FIND_DATA() const { return this; }
	__inline void operator=(const WIN32_FIND_DATA& srcData) { memcpy(this, &srcData, sizeof(WIN32_FIND_DATA)); }
};

////////////////////////////////////////////////////////////////////
// CWin32FileAttributeData

typedef const WIN32_FILE_ATTRIBUTE_DATA* LPCWIN32_FILE_ATTRIBUTE_DATA;

class CWin32FileAttributeData : public WIN32_FILE_ATTRIBUTE_DATA
{
public:
	__inline CWin32FileAttributeData() { }
	__inline CWin32FileAttributeData(const WIN32_FILE_ATTRIBUTE_DATA& srcData) { *this = srcData; }
	__inline CWin32FileAttributeData(const WIN32_FIND_DATA& srcFindData) { *this = (WIN32_FILE_ATTRIBUTE_DATA&)srcFindData; }

	__inline BOOL IsFolder() { return (dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY); }
	__inline BOOL IsSysHidden() { return (dwFileAttributes & (FILE_ATTRIBUTE_SYSTEM | FILE_ATTRIBUTE_HIDDEN)); }
	__inline BOOL IsHidden() { return (dwFileAttributes & FILE_ATTRIBUTE_HIDDEN); }
	__inline BOOL IsSystem() { return (dwFileAttributes & FILE_ATTRIBUTE_SYSTEM); }
	__inline BOOL IsAttrib(DWORD dwAttr) { return (dwFileAttributes & dwAttr); }
	__inline BOOL LocalCreationTime(LPFILETIME pft) { return FileTimeToLocalFileTime(&ftCreationTime, pft); }
	__inline BOOL LocalLastWriteTime(LPFILETIME pft) { return FileTimeToLocalFileTime(&ftLastWriteTime, pft); }
	__inline BOOL LocalLastAccessTime(LPFILETIME pft) { return FileTimeToLocalFileTime(&ftLastAccessTime, pft); }

	// convert between CRect and LPRECT/LPCRECT (no need for &)
	__inline operator LPWIN32_FILE_ATTRIBUTE_DATA() { return this; }
	__inline operator LPCWIN32_FILE_ATTRIBUTE_DATA() const { return this; }
	__inline void operator=(const WIN32_FILE_ATTRIBUTE_DATA& srcData) { memcpy(this, &srcData, sizeof(WIN32_FILE_ATTRIBUTE_DATA)); }
	__inline void operator=(const WIN32_FIND_DATA& srcFindData) { memcpy(this, &srcFindData, sizeof(WIN32_FILE_ATTRIBUTE_DATA)); }
};

/////////////////////////////////////////////////////////////////////
// CDirStack

class CDirStack
{
private:
	CString m_strStack;
public:
	CDirStack()
	{
		LPTSTR p = m_strStack.GetBuffer(MAX_PATH * 2);
		GetCurrentDirectory(MAX_PATH * 2, p);
		m_strStack.ReleaseBuffer();
	}
	~CDirStack()
	{
		SetCurrentDirectory(m_strStack);
	}
};


/////////////////////////////////////////////////////////////////////
// CEnterFolder

class CEnterFolder : public CDirStack
{
private:
	CString m_strNewPath;
public:
	CEnterFolder()
	{
		m_strNewPath.Empty();
	}

	~CEnterFolder()
	{
	}
	BOOL Enter(LPCTSTR pszNewPath)
	{
		BOOL bOK = SetCurrentDirectory(pszNewPath);
		if (bOK)
			m_strNewPath = pszNewPath;
		return bOK;
	}
};


////////////////////////////////////////////////////////////////////
// CLASS CFileInfo : ファイルクラス
//
class CFileInfo
{
public:
	CString m_strName;
	CString m_strPath;
	CString m_strDos;
	CString m_strType;
	DWORDLONG m_dlSize;
	FILETIME m_ftCreationTime;
	FILETIME m_ftLastWriteTime;
	DWORD m_dwAttr;
	int m_nSerial;
	int m_iImage;

public:
	CFileInfo()
	{
		m_strName.Empty();
		m_strPath.Empty();
		m_strDos.Empty();
		m_strType.Empty();
		m_dwAttr = 0;
		m_dlSize = 0;
		memset(&m_ftLastWriteTime, 0, sizeof(FILETIME));
		memset(&m_ftCreationTime, 0, sizeof(FILETIME));
		m_iImage = -1;
		m_nSerial = 0;
	}

	CFileInfo(LPCTSTR pszPath, WIN32_FIND_DATA& wfd)
	{
		SetFile(pszPath, wfd);
	}

	__inline CFileInfo(const CFileInfo& a) { CopyFrom(a); }

	~CFileInfo()
	{
	}

	__inline CFileInfo& operator=(const CFileInfo& a) { CopyFrom(a); return *this; }

	void CopyFrom(const CFileInfo& a)
	{
		if (&a != this) {
			m_strName = a.m_strName;
			m_strPath = a.m_strPath;
			m_strDos = a.m_strDos;
			m_strType = a.m_strType;
			m_dwAttr = a.m_dwAttr;
			m_dlSize = a.m_dlSize;
			m_ftLastWriteTime = a.m_ftLastWriteTime;
			m_ftCreationTime = a.m_ftCreationTime;
			m_nSerial = a.m_nSerial;
			m_iImage = a.m_iImage;
		}
	}

	void CopyTo(CFileInfo& a)
	{
		if (&a != this) {
			a.m_strName = m_strName;
			a.m_strPath = m_strPath;
			a.m_strDos = m_strDos;
			a.m_strType = m_strType;
			a.m_dwAttr = m_dwAttr;
			a.m_dlSize = m_dlSize;
			a.m_ftLastWriteTime = m_ftLastWriteTime;
			a.m_ftCreationTime = m_ftCreationTime;
			a.m_nSerial = m_nSerial;
			a.m_iImage = m_iImage;
		}
	}

	__inline LPCTSTR GetTypeName()
	{
		if (m_strType.IsEmpty())
			SetTypeName();
		return (LPCTSTR)m_strType;
	}

	void SetTypeName();

	void SetFile(LPCTSTR pszPath, WIN32_FIND_DATA& wfd)
	{
		m_strPath = pszPath;
		m_strName = wfd.cFileName;
		if (wfd.cAlternateFileName[0])
			m_strDos = wfd.cAlternateFileName;
		else
			m_strDos.Empty();
		SETDWORDLONG(m_dlSize, wfd.nFileSizeLow, wfd.nFileSizeHigh);
		m_dwAttr = wfd.dwFileAttributes;
		FileTimeToLocalFileTime(&wfd.ftCreationTime, &m_ftCreationTime);
		FileTimeToLocalFileTime(&wfd.ftLastWriteTime, &m_ftLastWriteTime);
	}

	__inline BOOL IsAttrib(DWORD dwAttr) { return (m_dwAttr & dwAttr); }
	__inline BOOL IsFolder() { return (m_dwAttr & FILE_ATTRIBUTE_DIRECTORY); }
	__inline BOOL IsSysHidden() { return (m_dwAttr & (FILE_ATTRIBUTE_SYSTEM | FILE_ATTRIBUTE_HIDDEN)); }
	__inline BOOL IsSystem() { return (m_dwAttr & FILE_ATTRIBUTE_SYSTEM); }
	__inline BOOL IsHidden() { return (m_dwAttr & FILE_ATTRIBUTE_HIDDEN); }
	__inline LPTSTR GetRealName(LPTSTR buffer) { return PathCombine(buffer, m_strPath, m_strName); }

	int ComparePathName(LPCTSTR pszName)
	{
		TCHAR src[MAX_PATH * 2];
		GetRealName(src);
		return lccComparePath(src, pszName);
	}
};

////////////////////////////////////////////////////////////////////
// CFileFinder クラス

class CFileFinder : public CWin32FindData
{
public:
	HANDLE m_hFind;
public:
	__inline CFileFinder() { m_hFind = NULL; }
	__inline ~CFileFinder() { if (m_hFind) FindClose(m_hFind); }
	BOOL BeginFind(LPCTSTR lpFilter)
	{
		m_hFind = FindFirstFile(lpFilter, this);
		if (m_hFind == INVALID_HANDLE_VALUE) m_hFind = NULL;
		return (m_hFind != NULL);
	}
	__inline BOOL NextFind() { return (FindNextFile(m_hFind, this) || GetLastError() == ERROR_MORE_DATA); }
	__inline void EndFind() { FindClose(m_hFind); m_hFind = NULL; }
};



/////////////////////////////////////////////////////////////////////
// CFileArray

BOOL __fastcall GetFileInfo(LPCTSTR pszName, CFileInfo& fi);

class CFileArray : public CAtlArray<CFileInfo*>
{
public:
	__inline CFileArray() { }
	__inline ~CFileArray() { DeleteAll(); }

	BOOL AddFile(CFileInfo* pf)
	{
		CFileInfo* pfNew = new CFileInfo(*pf);
		Add(pfNew);
		return TRUE;
	}

	int MoveArray(CFileArray* pa)
	{
		int num = 0;
		int cnt = (int)pa->GetCount();
		for (int i = 0; i < cnt; i++) {
			CFileInfo* pf = pa->GetAt(i);
			if (pf) { Add(pf); num++; }
		}
		pa->RemoveAll();
		return num;
	}

	int BrowseFiles(LPCTSTR lpPath, LPCTSTR lpFilter, DWORD dwFbr, BOOL bRangeCase = FALSE, BOOL bCaseSense = FALSE)
	{
		// 条件を指定して検索
		CEnterFolder ef;
		if (!ef.Enter(lpPath)) return -1;

		CMatchSpec ms(lpFilter, bRangeCase, bCaseSense);

		int cnt = 0;
		CFileInfo fi;
		CFileFinder finder;
		if (finder.BeginFind(gszAst)) {
			do {
				if (!finder.IsDot()) {
					if ( ((dwFbr & FBR_DIR) && finder.IsFolder()) ||
						 ((dwFbr & FBR_FILE) && !finder.IsFolder()) ) {
						if (ms.Match(finder.cFileName)) {
							fi.SetFile(lpPath, finder);
							AddFile(&fi);
							cnt++;
						}
					}
				}
			} while (finder.NextFind());
			finder.EndFind();
		}
		return cnt;
	}

	int Browse(LPCTSTR pszPath)
	{
		// 無条件(*.*)で検索
		CEnterFolder ef;
		if (!ef.Enter(pszPath)) return -1;

		int cnt = 0;
		CFileInfo fi;
		CFileFinder finder;
		if (finder.BeginFind(gszAst)) {
			do {
				if (!finder.IsDot()) {
					fi.SetFile(pszPath, finder);
					AddFile(&fi);
					cnt++;
				}
			} while (finder.NextFind());
			finder.EndFind();
		}
		return cnt;
	}

	void DeleteAll()
	{
		CFileInfo* pf;
		int cnt = (int)GetCount();
		for (int i = 0; i < cnt; i++) {
			pf = GetAt(i);
			if (pf) delete pf;
		}
		RemoveAll();
	}
};

/////////////////////////////////////////////////////////////////////
// CDropFiles

class CDropFiles
{
public:
	__inline CDropFiles() { m_hDrop = NULL; m_nEntry = 0; }
	__inline ~CDropFiles() { if (m_hDrop) ::DragFinish(m_hDrop); }

	HDROP Attach(HDROP hDrop)
	{
		if (hDrop == NULL || hDrop == m_hDrop)
			return NULL;

		HDROP hDropTemp = m_hDrop;
		m_hDrop = hDrop;
		if (m_hDrop)
			m_nEntry = ::DragQueryFile(m_hDrop, 0xFFFFFFFF, NULL, 0);
		else
			m_nEntry = 0;
		return hDropTemp;
	}

	HDROP Detach()
	{
		HDROP hDropTemp = m_hDrop;
		m_nEntry = 0;
		m_hDrop = NULL;
		return hDropTemp;
	}

	void DragFinish()
	{
		if (m_hDrop)
			::DragFinish(m_hDrop);
		m_hDrop = NULL;
		m_nEntry = 0;
	}

	BOOL DragQueryFile(int nIndex, LPTSTR lpFile, int cchMax)
	{
		if (m_hDrop && nIndex >= 0 && nIndex < m_nEntry) {
			::DragQueryFile(m_hDrop, nIndex, lpFile, cchMax);
			return TRUE;
		}
		return FALSE;
	}

	void DragQueryFile(int index, CString& str)
	{
		str.Empty();
		if (m_hDrop && index >= 0 && index < m_nEntry) {
			LPTSTR p = str.GetBuffer(MAX_PATH * 2);
			::DragQueryFile(m_hDrop, index, p, MAX_PATH * 2);
			str.ReleaseBuffer();
		}
	}

	BOOL DragInitHeader(POINT pt, BOOL fInNonClientArea)
	{
		DragFinish();
		m_hDrop = fopDragInitBlock(pt, fInNonClientArea);
		return (m_hDrop != NULL);
	}

	BOOL DragAppendFile(LPCTSTR pszPath)
	{
		if (!m_hDrop)
			DragInitHeader(CPoint(0, 0), FALSE);
		HDROP hdw = fopDragAppendFile(m_hDrop, pszPath);
		if (hdw) {
			m_hDrop = hdw;
			m_nEntry++;
			return TRUE;
		}
		return FALSE;
	}

	BOOL DragAppendFile(CFileInfo* pf)
	{
		TCHAR buf[MAX_PATH * 2];
		pf->GetRealName(buf);
		return DragAppendFile(buf);
	}

	LPTSTR MakeFileNames()
	{
		LPTSTR p = NULL;
		if (m_hDrop) {
			TCHAR buf[MAX_PATH * 2];

			p = tchar_malloc(m_nEntry * MAX_PATH * 2);
			int pos = 0;
			for (int i = 0; i < m_nEntry; i++) {
				::DragQueryFile(m_hDrop, i, buf, _COUNTOF(buf));
				lstrcpy(p + pos, buf);
				pos += lstrlen(buf) + 1;
			}
			p[pos] = 0;

			LPTSTR pw = tchar_realloc(p, pos + 2);
			if (pw)
				p = pw;
		}
		return p;
	}

	BOOL IsFolderInclude()
	{
		if (m_hDrop) {
			TCHAR buf[MAX_PATH * 2];
			for (int i = 0; i < m_nEntry; i++) {
				::DragQueryFile(m_hDrop, i, buf, _COUNTOF(buf));
				if (PathIsDirectory(buf))
					return TRUE;
			}
		}
		return FALSE;
	}

	__inline operator HDROP() const { return m_hDrop; }
	__inline int GetCount() { return m_nEntry; }

	void CopyFullPathToClipboard(HWND hwnd, BOOL bFullPath = TRUE)
	{
		if (m_hDrop) {
			TCHAR* lp = new TCHAR[m_nEntry * MAX_PATH * 2];
			TCHAR path[MAX_PATH * 2 + 4];
			LPTSTR p;
			int pos = 0;
			for (int i = 0; i < m_nEntry; i++) {
				::DragQueryFile(m_hDrop, i, path, _COUNTOF(path));
				if (bFullPath)
					p = path;
				else
					p = PathFindFileName(path);
				lstrcat(path, gszCrLf);
				lstrcpy(&lp[pos], p);
				pos += lstrlen(p);
			}

			lccSetStringToClipboard(hwnd, lp);
			delete [] lp;
		}
	}

	void CopyShortPathToClipboard(HWND hwnd)
	{
		if (m_hDrop) {
			TCHAR* lp = new TCHAR[m_nEntry * MAX_PATH * 2];
			TCHAR path[MAX_PATH * 2 + 4];
			int pos = 0;
			for (int i = 0; i < m_nEntry; i++) {
				::DragQueryFile(m_hDrop, i, path, _COUNTOF(path));
				GetShortPathName(path, path, _COUNTOF(path));
				lstrcat(path, gszCrLf);
				lstrcpy(&lp[pos], path);
				pos += lstrlen(path);
			}

			lccSetStringToClipboard(hwnd, lp);
			delete [] lp;
		}
	}

	void CopyToClipboard(HWND hwnd);

	HDROP m_hDrop;
	int m_nEntry;
};


/////////////////////////////////////////////////////////////////////
//

int __cdecl CompareFileInfo(const void* p1, const void* p2, const void* typeseq);


/////////////////////////////////////////////////////////////////////
//

class CExtMap : public CAtlArray<LPCTSTR>
{
public:
	CExtMap() { }
	~CExtMap() { DeleteAll(); }

	void DeleteAll()
	{
		LPCTSTR p;
		int i, cnt = (int)GetCount();
		for (i = 0; i < cnt; i++) {
			p = GetAt(i);
			if (p) LocalFree((HLOCAL)p);
		}
		RemoveAll();

		cnt = (int)m_typename.GetCount();
		for (i = 0; i < cnt; i++) {
			p = m_typename[i];
			if (p) LocalFree((HLOCAL)p);
		}
		m_typename.RemoveAll();
	}

	BOOL Lookup(LPCTSTR ext, LPCTSTR* pp)
	{
		*pp = NULL;
		int i, cnt = (int)GetCount();
		for (i = 0; i < cnt; i++) {
			LPCTSTR p = GetAt(i);
			if (p && !StrCmpI(p, ext)) {
				*pp = m_typename[i];
				return TRUE;
			}
		}
		return FALSE;
	}

	LPCTSTR AddType(LPCTSTR ext, LPCTSTR name)
	{
		LPCTSTR p;
		if (!Lookup(ext, &p)) {
			Add(StrDup(ext));
			p = name ? (LPCTSTR)StrDup(name) : NULL;
			m_typename.Add(p);
		}
		return p;
	}

	CAtlArray<LPCTSTR> m_typename;
};

LPCTSTR __fastcall GetMapType(LPCTSTR key);
LPCTSTR __fastcall AddMapType(LPCTSTR key, LPCTSTR name);
void __fastcall InitMapType();


#endif				// __FILEITEM_H__
