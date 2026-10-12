/////////////////////////////////////////////////////////////////////////////
// CFolderItem

#pragma once
#ifndef __FOLDERITEM_H__
#define __FOLDERITEM_H__

#include <math.h>
#include <float.h>
#include <limits.h>
#include "resource.h"
#include "accesslock.h"
#include "exifinfo.h"

class CFolderItem
{
public:
	// Data Member
	IShellFolder* m_psf;		// デスクトップの場合は NULL
	LPCITEMIDLIST m_pidlFq;		// ILCombineで作成されなければダメ
	LPCITEMIDLIST m_pidl;		// m_pidlFqのLastIDを指す
	SFGAOF m_attr;
	DWORD m_fileattr;
	int m_descid;				// SHGetDataFromIDList(SHGDFIL_DESCRIPTIONID)
	int m_iImage;
	int m_iSelectedImage;
	int m_iOverlayImage;

private:
	void _init_member()
	{
		m_psf = NULL;
		m_pidlFq = NULL;
		m_pidl = NULL;
		m_attr = 0;
		m_fileattr = 0;
		m_descid = -1;
		m_iImage = -1;
		m_iSelectedImage = -1;
		m_iOverlayImage = -1;
	}

public:
	CFolderItem()
	{
		_init_member();
	}

	CFolderItem(IShellFolder* psf, LPCITEMIDLIST pidlParent, LPCITEMIDLIST pidl)
	{
		_init_member();

		m_psf = psf;
		if (m_psf)
			m_psf->AddRef();
		m_pidlFq = ILCombine(pidlParent, pidl);
		m_pidl = ILFindLastID(m_pidlFq);
	}

	CFolderItem(LPCITEMIDLIST pidlFq)
	{
		_init_member();

		m_pidlFq = ILClone(pidlFq);
		if (SHBindToParent(m_pidlFq, IID_PPV_ARGS(&m_psf), &m_pidl) != S_OK) {
			CoTaskMemFree((void*)m_pidlFq);
			_init_member();
		}
	}

	CFolderItem(CFolderItem& src)
	{
		CopyFrom(src);
	}

	~CFolderItem()
	{
		if (m_pidlFq)	CoTaskMemFree((void*)m_pidlFq);
		if (m_psf)		m_psf->Release();
	}

	const CFolderItem& CopyFrom(const CFolderItem& src)
	{
		if (&src != this) {
			m_psf = src.m_psf;
			if (m_psf)
				m_psf->AddRef();
			m_pidlFq = ILClone(src.m_pidlFq);
			m_pidl = ILFindLastID(m_pidlFq);
			m_attr = src.m_attr;
			m_fileattr = src.m_fileattr;
			m_descid = src.m_descid;
			m_iImage = src.m_iImage;
			m_iSelectedImage = src.m_iSelectedImage;
			m_iOverlayImage = src.m_iOverlayImage;
		}
		return *this;
	}

	inline const CFolderItem& operator=(const CFolderItem& src)
	{
		return CopyFrom(src);
	}

	inline BOOL isEqual(PCIDLIST_ABSOLUTE pidl)
	{
		ATLASSERT(m_pidlFq != NULL);
		return ILIsEqual(m_pidlFq, pidl);
	}

	inline BOOL isParent(PCIDLIST_ABSOLUTE pidl, BOOL fImmediate)
	{
		ATLASSERT(m_pidlFq != NULL);
		return ILIsParent(m_pidlFq, pidl, fImmediate);
	}

	inline bool isDesktop() { return (m_psf == NULL); }

	inline void ResetImageIndex() { m_iImage = m_iSelectedImage = m_iOverlayImage = -1; }

	int GetImageIndex()
	{
		if (m_iImage < 0) {
			SHFILEINFO sfi;
			if (m_pidlFq && SHGetFileInfo((LPCTSTR)m_pidlFq, 0, &sfi, sizeof(sfi), SHGFI_PIDL | SHGFI_SYSICONINDEX | SHGFI_SMALLICON | SHGFI_LINKOVERLAY))
				m_iImage = sfi.iIcon;
		}
		return m_iImage;
	}

	int GetSelectedImageIndex()
	{
		if (m_iSelectedImage < 0) {
			SHFILEINFO sfi;
			if (m_pidlFq && SHGetFileInfo((LPCTSTR)m_pidlFq, 0, &sfi, sizeof(sfi), SHGFI_PIDL | SHGFI_SYSICONINDEX | SHGFI_SMALLICON | SHGFI_OPENICON))
				m_iSelectedImage = sfi.iIcon;
		}
		return m_iSelectedImage;
	}

	int GetOverlayImageIndex()
	{
		if (m_iOverlayImage < 0) {
			// オーバーレイアイコンの有無
			m_iOverlayImage = 0;
			if (m_psf) {
				CComQIPtr<IShellIconOverlay> psio = m_psf;
				if (psio.p != NULL) {
					int iImage;
					if (psio->GetOverlayIndex(m_pidl, &iImage) == S_OK)
						m_iOverlayImage = iImage;
				}
			}
		}
		return m_iOverlayImage;
	}

	DWORD OverlayImageMask()
	{
		return INDEXTOOVERLAYMASK((DWORD)GetOverlayImageIndex());
	}

	bool hasSubFolder()
	{
		if (m_psf) {
			// 書庫？
			if ((GetAttributes() & (SFGAO_STREAM | SFGAO_LINK)) == SFGAO_STREAM)
				return true;
			SFGAOF attr = SFGAO_HASSUBFOLDER;
			return (m_psf->GetAttributesOf(1, &m_pidl, &attr) == S_OK && (attr & SFGAO_HASSUBFOLDER));
		}
		// デスクトップ
		return true;
	}

	SFGAOF GetAttributes()
	{
		if (m_attr == 0) {
			if (m_psf) {
				SFGAOF attr = SFGAO_FILESYSANCESTOR | SFGAO_FOLDER | SFGAO_HIDDEN | SFGAO_SYSTEM | SFGAO_STREAM | SFGAO_LINK | SFGAO_CAPABILITYMASK;
				if (m_psf->GetAttributesOf(1, &m_pidl, &attr) == S_OK)
					m_attr = attr;
			}
			else {
				// デスクトップ
				m_attr = SFGAO_FOLDER | SFGAO_FILESYSTEM | SFGAO_DROPTARGET | SFGAO_DROPTARGET;
			}
		}
		return m_attr;
	}

	BOOL GetRealPathName(CString& path);
	BOOL GetPathName(CString& strPath);
	void SetPathInfo();
	LPCTSTR GetDisplayName(CString& strName, bool bUseFullyQualPidl = false);
	inline BOOL isFileSystem() { return (m_attr & SFGAO_FILESYSTEM); }
	bool isValid() { return (m_pidlFq && idl_IDListIsValid(m_pidlFq)); }
	inline BOOL IsDesktop() { return (m_psf == NULL); }
	void* GetUIObjectOf(HWND hwnd, REFIID riid)
	{
		void* pv = NULL;
		if (m_psf) {
			if (m_psf->GetUIObjectOf(hwnd, 1, &m_pidl, riid, NULL, &pv) != S_OK)
				pv = NULL;
		}
		else {
			CComPtr<IShellFolder> psf;
			SHGetDesktopFolder(&psf);
			if (psf->GetUIObjectOf(hwnd, 1, (LPCITEMIDLIST*)&m_pidlFq, riid, NULL, &pv) != S_OK)
				pv = NULL;
		}
		return pv;
	}
	HRESULT CompareIDs(LPCITEMIDLIST pidlOther)
	{
		if (pidlOther && m_psf)
			return m_psf->CompareIDs(0, m_pidl, pidlOther);
		return E_FAIL;
	}
	void ValidateCacheFolderInformation()
	{
		if (m_psf) {
			SFGAOF attr = SFGAO_VALIDATE;
			m_psf->GetAttributesOf(0, NULL, &attr);
		}
	}

	BOOL ContextMenu(HWND hwnd, LPPOINT ppt);
	HDROP MakeDropFiles();
	LPTSTR MakeFileNames(LPTSTR pszNames, int cch);
	HRESULT BindToShellFolder(IShellFolder** ppsf)
	{
		if (m_psf == NULL)
			return SHGetDesktopFolder(ppsf);
		else 
			return m_psf->BindToObject(m_pidl, NULL, IID_PPV_ARGS(ppsf));
	}
	inline BOOL GetName(DWORD flags, CString& str) 
	{
		return m_psf ? idl_GetName(m_psf, m_pidl, flags, str) : idl_GetName(m_pidlFq, flags, str);
	}
	int GetDescriptionId()
	{
		if (m_descid < 0 && m_psf)
			m_descid = idl_GetDescriptionId(m_psf, m_pidl);
		return m_descid;
	}
};

/////////////////////////////////////////////////////////////////////////////
// CFileItem

#define MAXCOLUMNINFO			10

BOOL __fastcall GetFileData(LPCTSTR pszName, WIN32_FIND_DATA* pwfd);
HBITMAP CloneBitmap(HBITMAP hbm);

#define PERCEIVED_TYPE_NOTCHECKED		(PERCEIVED)-9999

class CFileItem
{
public:
	LPITEMIDLIST m_pidlFq;
	LPCITEMIDLIST m_pidl;				// ItemIDList
	PERCEIVED m_perceived;				// perceived type
	SFGAOF m_attr;						// attributes
	DWORDLONG m_filesize;				// file size
	DWORDLONG m_totalsize;				// total file size for directory
	FILETIME m_ftLastWrite;				// last write time
	DWORD m_fileattr;					// file attributes
	int m_iImage;						// system imagelist index
	int m_iOverlayImage;				// system imagelist overlay index
	int m_serial;						// internal serial number
	int m_descid;						// SHGetDataFromIDList(SHGDFIL_DESCRIPTIONID)
	bool m_fUpdir:1;
	bool m_fDirtyIcon:1;
	bool m_fRawFile:1;
	bool m_fChecked:1;
	bool m_fNameCompacted:1;
	CSimpleArray<CString> m_column;
	DWORDLONG m_columnvalue[MAXCOLUMNINFO];
	CString m_infotip;					// tooltip text
	Bitmap* m_bmp;						// bitmap image
	CExifInformation* m_exifptr;

public:
	CFileItem()
		: m_pidlFq(NULL)
		, m_pidl(NULL)
		, m_perceived(PERCEIVED_TYPE_NOTCHECKED)
		, m_attr(0)
		, m_filesize(0)
		, m_totalsize(0)
		, m_fileattr(0)
		, m_iImage(-1)
		, m_iOverlayImage(-1)
		, m_descid(-1)
		, m_exifptr(NULL)
		, m_fUpdir(false)
		, m_fDirtyIcon(false)
		, m_fRawFile(false)
		, m_fChecked(false)
		, m_fNameCompacted(false)
		, m_bmp(NULL)
	{
		*((DWORDLONG*)&m_ftLastWrite) = 0;
		for (int i = 0; i < MAXCOLUMNINFO; i++) {
			m_column.Add(_T(""));
			m_columnvalue[i] = 0;
		}
		ATLASSERT(m_column.GetSize() == MAXCOLUMNINFO);
	}
	~CFileItem()
	{
		if (m_pidlFq) CoTaskMemFree(m_pidlFq);
		if (m_bmp) delete m_bmp;
		if (m_exifptr) delete m_exifptr;
	}

	PERCEIVED GetPerceivedType()
	{
		if (m_perceived == PERCEIVED_TYPE_NOTCHECKED) {
			m_perceived = PERCEIVED_TYPE_UNKNOWN;
			if (isFolder())
				m_perceived = PERCEIVED_TYPE_FOLDER;
			else if (isRawFile()) {
				LPCTSTR ext = StrRChr(m_column[0], NULL, _T('.'));
				if (ext) {
					PERCEIVEDFLAG flag;
					if (AssocGetPerceivedType(ext, &m_perceived, &flag, NULL) != S_OK)
						m_perceived = PERCEIVED_TYPE_UNKNOWN;
				}
			}
		}
		return m_perceived;
	}

	int GetDescriptionId(IShellFolder* psf)
	{
		if (m_descid < 0) m_descid = idl_GetDescriptionId(psf, m_pidl);
		return m_descid;
	}

	LPCTSTR GetTypeName(CString& str)
	{
		// リパースポイント
		if (isFSDir() && m_fileattr & FILE_ATTRIBUTE_REPARSE_POINT) {
			str.LoadString(IDS_JUNCTION);
			return str.GetString();
		}

		SHFILEINFO sfi;
		SHGetFileInfo((LPCTSTR)m_pidlFq, 0, &sfi, sizeof(sfi), SHGFI_PIDL | SHGFI_TYPENAME);
		str = sfi.szTypeName;
		return str.GetString();
	}

	bool GetPathName(IShellFolder* psf, CString& path, WIN32_FIND_DATA* pwfd = NULL)
	{
		if (psf && m_pidl) {
			if (idl_GetName(psf, m_pidl, SHGDN_FORPARSING, path)) {
				WIN32_FIND_DATA wfd = { 0 };
				if (PathIsRoot(path) || GetFileData(path, &wfd)) {
					if (pwfd) *pwfd = wfd;
					return true;
				}
			}
		}
		path.Empty();
		return false;
	}

	__inline bool isUpdir() { return m_fUpdir; }
	__inline bool isEXIF() { return (m_exifptr != NULL); }
	__inline bool isRawFile() { return m_fRawFile; }
	__inline void SetDirtyIcon(bool bSet) { m_fDirtyIcon = bSet; }
	__inline bool isFSFile() { return (m_descid == SHDID_FS_FILE); }
	__inline bool isFSDir() { return (m_descid == SHDID_FS_DIRECTORY); }
	__inline bool isFolder() { return ((m_attr & SFGAO_FOLDER) != 0); }
	__inline bool isLink() { return ((m_attr & SFGAO_LINK) != 0); }
	__inline bool hasStream() { return ((m_attr & SFGAO_STREAM) != 0); }
	__inline bool isDirtyIcon() { return m_fDirtyIcon; }
	__inline bool isDrive() { return ((SHDID_COMPUTER_DRIVE35 <= m_descid) && (m_descid <= SHDID_COMPUTER_RAMDISK)); }
	__inline bool isFixedDrive() { return (m_descid == SHDID_COMPUTER_FIXED); }
	__inline bool isNetDrive() { return (m_descid == SHDID_COMPUTER_NETDRIVE); }
	void SetExifInfo(EXIFINFOSTRUCT* info)
	{
		if (!m_exifptr)
			m_exifptr = new CExifInformation;
		m_exifptr->Set(info);
	}
	__inline LPCTSTR GetExifLabel() { return (m_exifptr != NULL) ? m_exifptr->GetLabel() : NULL; }
	__inline bool isMatch(const CFileItem& dst) { return ((dst.m_filesize == m_filesize) && !CompareFileTime(&m_ftLastWrite, &dst.m_ftLastWrite)); }
	__inline bool hasFileAttr(DWORD attr) { return ((m_fileattr & attr) != 0); }
	__inline bool hasLastWriteTime() { return ((m_ftLastWrite.dwLowDateTime | m_ftLastWrite.dwHighDateTime) != 0); }

	bool GetDriveUsage(CString& str)
	{
		if (isDrive() && m_totalsize > 0) {
			TCHAR szFree[64], szTotal[64];
			StrFormatByteSize64(m_filesize, szFree, 64);
			StrFormatByteSize64(m_totalsize, szTotal, 64);
			str.Format(IDS_INDICATOR_SPACE, szFree, szTotal);
			return true;
		}
		return false;
	}

	__inline DWORD OverlayImageMask() { return m_iOverlayImage > 0 ? (DWORD)INDEXTOOVERLAYMASK(m_iOverlayImage): 0; }
};



/////////////////////////////////////////////////////////////////////
//

class CFileStack : public CSimpleArray<CFileItem*>
{
public:
	UINT m_numcol;
	SHELLCOLUMNINFO m_colinfo[MAXCOLUMNINFO];
public:
	CFileStack() :  m_numcol(0)
	{
		memset(m_colinfo, 0, sizeof(m_colinfo));
	}

	void DeleteAll(bool bDeleteColumInfo = true)
	{
		int cnt = GetSize();
		for (int i = 0; i < cnt; i++) {
			CFileItem* pf = (*this)[i];
			SetAtIndex(i, (CFileItem*)NULL);
			if (pf)
				delete pf;
		}
		RemoveAll();

		if (bDeleteColumInfo) {
			m_numcol = 0;
			memset(m_colinfo, 0, sizeof(m_colinfo));
		}
	}
	BOOL SetSize(int newSize)
	{
		if (newSize < 0 || newSize <= m_nAllocSize || newSize > INT_MAX / sizeof(CFileItem*))
			return FALSE;

		CFileItem** aT = (CFileItem**)_recalloc(m_aT, newSize, sizeof(CFileItem*));
		if (aT == NULL)
			return FALSE;
		m_nAllocSize = newSize;
		m_aT = aT;
		return TRUE;
	}

	int FindScid(const SHCOLUMNID& scid)
	{
		for (UINT i = 0; i < m_numcol; i++) {
			if (m_colinfo[i].scid == scid)
				return i;
		}
		return -1;
	}
	SHCOLUMNID GetScid(int index) const
	{
		return (0 <= index && index < (int)m_numcol) ? m_colinfo[index].scid : PKEY_Null;
	}
	inline SHCOLSTATEF GetColumnType(int index)
	{
		return (0 <= index && index < (int)m_numcol) ? (m_colinfo[index].state & SHCOLSTATE_TYPEMASK) : 0;
	}
	inline LPCWSTR GetColumnName(int index)
	{
		return (0 <= index && index < (int)m_numcol) ? m_colinfo[index].title : NULL;
	}
	int FindColumnState(SHCOLSTATEF coltype, int start = -1)
	{
		UINT i = (start < 0) ? 0 : (UINT)start;
		for (; i < m_numcol; i++) {
			if ((m_colinfo[i].state & SHCOLSTATE_TYPEMASK) == coltype)
				return i;
		}
		return -1;
	}
	int FindColumnName(LPCWSTR name, LPCTSTR delim, int start = -1)
	{
		CString keywords = name;
		UINT i = (start < 0) ? 0 : (UINT)start;
		for (; i < m_numcol; i++) {
			int pos = 0;
			CString token = keywords.Tokenize(delim, pos);
			while (token != _T("")) {
				if (StrStrI(m_colinfo[i].title, token) != NULL)
					return i;
				token = keywords.Tokenize(delim, pos);
			}
		}
		return -1;
	}
};


#ifdef USE_DPA_ARRAY

/////////////////////////////////////////////////////////////////////
// CDpa - DPA_xxx Dynamic Pointer Array Function Wrapper
//

class CDpa
{
public:
	__inline CDpa() { Init(); }
	__inline CDpa(int delta) { Init(); Create(delta); }
	__inline CDpa(int delta, HANDLE hHeap) { Init(); CreateEx(delta, hHeap); }
	__inline ~CDpa() { Destroy(); }

	void Init();
	BOOL Create(int delta);
	BOOL CreateEx(int delta, HANDLE hHeap);
	BOOL Destroy();
	__inline int GetSize() { return (m_handle ? ((HDPA)m_handle)->nItems : 0); }
	__inline int Add(void* pItem) { return (*_fDPA_InsertPtr)(m_handle, INT_MAX, pItem); }
	__inline int InsertAt(int nPos, void* pItem) { return (*_fDPA_InsertPtr)(m_handle, nPos, pItem); }
	__inline void* GetAt(int nPos) { return (*_fDPA_GetPtr)(m_handle, nPos); }
	__inline BOOL SetAt(int nPos, void* pItem) { return (*_fDPA_SetPtr)(m_handle, nPos, pItem); }
	__inline BOOL Grow(int nCapa) { return (*_fDPA_Grow)(m_handle, nCapa); }
	__inline void* RemoveAt(int nPos) { return (*_fDPA_DeletePtr)(m_handle, nPos); }
	__inline BOOL RemoveAll() { return (*_fDPA_DeleteAllPtrs)(m_handle); }

	__inline BOOL Sort(PFNDPACOMPARE pfnComp, LPARAM lParam) { return (*_fDPA_Sort)(m_handle, (FARPROC)pfnComp, lParam); }
	__inline int Search(void* pItem, int nStartPos, PFNDPACOMPARE pfnComp, LPARAM lParam, DWORD dwFlag) { return (*_fDPA_Search)(m_handle, pItem, nStartPos, (FARPROC)pfnComp, lParam, dwFlag); }
	__inline int GetPtrIndex(void* pItem) { return (*_fDPA_GetPtrIndex)(m_handle, pItem); }

	HANDLE m_handle;
};


#endif		// USE_DPA_ARRAY


#endif		// __FOLDERITEM_H__
