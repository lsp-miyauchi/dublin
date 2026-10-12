////////////////////////////////////////////////////////////////////
// fileitem.h

#pragma once
#ifndef __FILEITEM_H__
#define __FILEITEM_H__


#define MAXHIAR 				16

#define FBR_FILE				0x0001
#define FBR_DIR					0x0002
#define FBR_BOTH				(FBR_FILE | FBR_DIR)
#define SFGAO_ROOT				(SFGAO_FILESYSANCESTOR | SFGAO_FOLDER | SFGAO_FILESYSTEM)

////////////////////////////////////////////////////////////////////
// FileType
//
enum FileType {
	CE_NORMAL,
	CE_PROGRAM,
	CE_ASSOC,
	CE_DIR,
	CE_CPE
};


////////////////////////////////////////////////////////////////////
// ExecType
//
enum ExecType {
	CE_EXE,
	CE_PIF,
	CE_COM,
	CE_BAT,
	CE_CMD,
	CE_LNK
};


////////////////////////////////////////////////////////////////////
// DisplayNameType
//
enum DisplayNameType {
	DNT_NORMAL,
	DNT_LOWER,
	DNT_TITLE
};



////////////////////////////////////////////////////////////////////
// ChildType
//
enum {
	DIR_NONE,
	DIR_CLOSE,
	DIR_OPEN
};


////////////////////////////////////////////////////////////////////
// FILEINFO.rstat STATUS BIT
//
#define RST_SELECT		0x0001
#define RST_DIRTYICON	0x0002
#define RST_DOT			0x0010
#define RST_SHARE		0x0080
#define RST_ROOT		0x0200
#define RST_SEARCHED	0x1000
#define RST_VIRTUAL		0x8000


///////////////////////////////////////////////////////////////////////
// CWin32FileDataクラス
//

typedef const WIN32_FIND_DATA* LPCWIN32_FIND_DATA;

class CWin32FindData : public WIN32_FIND_DATA
{
public:
	__inline CWin32FindData() { }
	__inline CWin32FindData(const WIN32_FIND_DATA& srcData) { *this = srcData; }

	__inline BOOL isFolder() { return (dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY); }
	__inline BOOL isDot() { return (cFileName[0]=='.' && (cFileName[1]==0 || (cFileName[1]=='.' && cFileName[2]==0))); }
	__inline BOOL isSysHidden() { return (dwFileAttributes & (FILE_ATTRIBUTE_SYSTEM | FILE_ATTRIBUTE_HIDDEN)); }
	__inline BOOL isSystem() { return (dwFileAttributes & FILE_ATTRIBUTE_SYSTEM); }
	__inline BOOL isHidden() { return (dwFileAttributes & FILE_ATTRIBUTE_HIDDEN); }
	__inline BOOL isReadOnly() { return (dwFileAttributes & FILE_ATTRIBUTE_READONLY); }
	__inline BOOL isReparsePoint() { return (dwFileAttributes & FILE_ATTRIBUTE_REPARSE_POINT); }
	__inline BOOL isAttrib(DWORD dwAttr) { return (dwFileAttributes & dwAttr); }
	__inline BOOL LocalCreationTime(LPFILETIME lpft) { return FileTimeToLocalFileTime(&ftCreationTime, lpft); }
	__inline BOOL LocalLastWriteTime(LPFILETIME lpft) { return FileTimeToLocalFileTime(&ftLastWriteTime, lpft); }
	__inline BOOL LocalLastAccessTime(LPFILETIME lpft) { return FileTimeToLocalFileTime(&ftLastAccessTime, lpft); }

	// convert between CRect and LPRECT/LPCRECT (no need for &)
	__inline operator LPWIN32_FIND_DATA() { return this; }
	__inline operator LPCWIN32_FIND_DATA() const { return this; }
	__inline void operator=(const WIN32_FIND_DATA& srcData) { memcpy(this, &srcData, sizeof(WIN32_FIND_DATA)); }

	LPCTSTR FormatFileAttributes(CString& str)
	{
		str = _T("------");
		if (dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)		str.SetAt(0, _T('d'));
		if (dwFileAttributes & FILE_ATTRIBUTE_ARCHIVE)			str.SetAt(1, _T('a'));
		if (dwFileAttributes & FILE_ATTRIBUTE_READONLY)			str.SetAt(2, _T('r'));
		if (dwFileAttributes & FILE_ATTRIBUTE_HIDDEN)			str.SetAt(3, _T('h'));
		if (dwFileAttributes & FILE_ATTRIBUTE_SYSTEM)			str.SetAt(4, _T('s'));
		if (dwFileAttributes & FILE_ATTRIBUTE_REPARSE_POINT)	str.SetAt(5, _T('l'));

		if (dwFileAttributes & FILE_ATTRIBUTE_COMPRESSED)		str += _T('C');
		if (dwFileAttributes & FILE_ATTRIBUTE_ENCRYPTED)		str += _T('E');
		if (dwFileAttributes & FILE_ATTRIBUTE_OFFLINE)			str += _T('O');
		if (dwFileAttributes & FILE_ATTRIBUTE_SPARSE_FILE)		str += _T('P');
		if (dwFileAttributes & FILE_ATTRIBUTE_NOT_CONTENT_INDEXED)	str += _T('I');
/*
		// 属性長
		LPTSTR p = str.GetBuffer(16);
		if (dwFileAttributes & FILE_ATTRIBUTE_REPARSE_POINT)		*p++ = _T('J');
		if (dwFileAttributes & FILE_ATTRIBUTE_SYSTEM)				*p++ = _T('S');
		if (dwFileAttributes & FILE_ATTRIBUTE_HIDDEN)				*p++ = _T('H');
		if (dwFileAttributes & FILE_ATTRIBUTE_READONLY)				*p++ = _T('R');
		if (dwFileAttributes & FILE_ATTRIBUTE_ARCHIVE)				*p++ = _T('A');
		if (dwFileAttributes & FILE_ATTRIBUTE_COMPRESSED)			*p++ = _T('C');
		if (dwFileAttributes & FILE_ATTRIBUTE_ENCRYPTED)			*p++ = _T('E');
		if (dwFileAttributes & FILE_ATTRIBUTE_OFFLINE)				*p++ = _T('O');
		if (dwFileAttributes & FILE_ATTRIBUTE_SPARSE_FILE)			*p++ = _T('P');
		if (dwFileAttributes & FILE_ATTRIBUTE_NOT_CONTENT_INDEXED)	*p++ = _T('I');
		*p = 0;
		str.ReleaseBuffer();
*/
		return str.GetString();
	}
};


////////////////////////////////////////////////////////////////////
// CFileFinder クラス

class CFileFinder : public CWin32FindData
{
public:
	__inline CFileFinder() { m_hFind = NULL; }
	__inline ~CFileFinder() { if (m_hFind) FindClose(m_hFind); }
	BOOL BeginFind(LPCTSTR lpFilter)
	{
		m_hFind = FindFirstFile(lpFilter, this);
		if (!VALIDHANDLE(m_hFind)) m_hFind = NULL;
		return (m_hFind != NULL);
	}
	__inline BOOL NextFind() { return (FindNextFile(m_hFind, this) || GetLastError() == ERROR_MORE_DATA); }
	__inline void EndFind() { FindClose(m_hFind); m_hFind = NULL; }

	HANDLE m_hFind;
};


/////////////////////////////////////////////////////////////////////
// TDropFiles

enum {
	DFCP_NAMEONLY = 0,
	DFCP_SHORTPATH,
	DFCP_FULLPATH,
	DFCP_UNCPATH,
};

class TDropFiles
{
private:
	HDROP m_hDrop;
	int m_nEntry;

public:
	__inline TDropFiles() { m_hDrop = NULL; m_nEntry = 0; }
	__inline ~TDropFiles() { if (m_hDrop) ::DragFinish(m_hDrop); }

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

	BOOL DragQueryFile(int index, LPTSTR pszFile, int cchMax)
	{
		if (m_hDrop && index >= 0 && index < m_nEntry && pszFile && cchMax > 0) {
			::DragQueryFile(m_hDrop, index, pszFile, cchMax);
			return TRUE;
		}
		return FALSE;
	}

	void DragQueryFile(int index, CString& str)
	{
		str.Empty();
		if (m_hDrop && index >= 0 && index < m_nEntry) {
			UINT size = ::DragQueryFile(m_hDrop, index, NULL, 0) + 1;
			LPTSTR p = str.GetBuffer(size);
			::DragQueryFile(m_hDrop, index, p, size);
			str.ReleaseBuffer();
		}
	}

	void GetFileNames(CSimpleArray<CString>& arr)
	{
		CString str;
		for (int i = 0; i < m_nEntry; i++) {
			DragQueryFile(i, str);
			arr.Add(str);
		}
	}

	BOOL DragInitHeader(POINT pt, BOOL fInNonClientArea);
	BOOL DragAppendFile(LPCTSTR pszPath);

	BOOL IsFolderInclude();

	__inline operator HDROP() const { return m_hDrop; }
	__inline int GetCount() { return m_nEntry; }

	void CopyPathToClipboard(HWND hwnd, UINT copymode, bool bAppend = false);
	void CopyToClipboard(HWND hwnd, bool bAppend = false);
};

/////////////////////////////////////////////////////////////////////
// CEnterFolder

class CEnterFolder
{
private:
	CString m_strSave;
public:
	CEnterFolder()
	{
		m_strSave.Empty();
	}
	~CEnterFolder()
	{
		Leave();
	}
	BOOL Enter(LPCTSTR pszNewPath)
	{
		if (!pszNewPath || *pszNewPath == 0)
			return FALSE;

		if (m_strSave.IsEmpty()) {
			LPTSTR p = m_strSave.GetBuffer(MAX_PATH * 2);
			GetCurrentDirectory(MAX_PATH * 2, p);
			m_strSave.ReleaseBuffer();
		}

		return SetCurrentDirectory(pszNewPath);
	}
	void Leave()
	{
		if (!m_strSave.IsEmpty()) {
			SetCurrentDirectory(m_strSave);
			m_strSave.Empty();
		}
	}
};


#endif			// __FILEITEM_H__
