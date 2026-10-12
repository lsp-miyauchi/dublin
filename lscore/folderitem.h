/////////////////////////////////////////////////////////////////////////////
// CFolderItem

#pragma once

#ifndef __FOLDERITEM_H__
#define __FOLDERITEM_H__

#include "idlist.h"

class CFolderItem
{
public:
	// Data Member
	IShellFolder* m_psf;		// デスクトップの場合は NULL
	LPITEMIDLIST m_pidlFq;		// ILCombineで作成されなければダメ
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

	CFolderItem(IShellFolder* psf, PCIDLIST_ABSOLUTE pidlParent, PCUIDLIST_RELATIVE pidl)
	{
		_init_member();

		m_psf = psf;
		if (m_psf)
			m_psf->AddRef();
		m_pidlFq = ILCombine(pidlParent, pidl);
		m_pidl = ILFindLastID(m_pidlFq);
	}

	CFolderItem(PCIDLIST_ABSOLUTE pidlFq)
	{
		_init_member();

		m_pidlFq = ILClone(pidlFq);
		if (SHBindToParent(m_pidlFq, IID_IShellFolder, (void**)&m_psf, &m_pidl) != S_OK) {
			CoTaskMemFree(m_pidlFq);
			_init_member();
		}
	}

	CFolderItem(CFolderItem& src)
	{
		CopyFrom(src);
	}

	~CFolderItem()
	{
		if (m_pidlFq)	CoTaskMemFree(m_pidlFq);
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

	inline BOOL operator ==(const CFolderItem& src) const
	{
		ATLASSERT(m_pidlFq != NULL);
		return ILIsEqual(m_pidlFq, src.m_pidlFq);
	}

	inline BOOL isEmpty() { return (!m_psf && !m_pidl && !m_pidlFq); }

	inline bool isDesktop() { return (m_psf == NULL); }

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
			IShellIconOverlay* psio;
			if (m_psf && SUCCEEDED(m_psf->QueryInterface(IID_IShellIconOverlay, (void**)&psio))) {
				int iImage;
				if (psio->GetOverlayIndex(m_pidl, &iImage) == S_OK)
					m_iOverlayImage = iImage;
				psio->Release();
			}
		}
		return m_iOverlayImage;
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

	BOOL GetRealPathName(CString& strPath)
	{
		if (GetDescriptionId() == SHDID_FS_DIRECTORY)
			return GetPathName(strPath);

		TCHAR buf[MAX_PATH * 2];
		if (SHGetPathFromIDList(m_pidlFq, buf)) {
			strPath = buf;
			return TRUE;
		}
		return FALSE;
	}

	BOOL GetPathName(CString& strPath)
	{
		if (idl_GetName(m_pidlFq, SHGDN_FORPARSING, strPath) &&
			(PathIsRoot(strPath) || _pathfileexists(strPath))) {
			// ターゲットフォルダがリンク属性を持っていることがあるので
			// 実際のリンク先を取得する
			if (m_attr & SFGAO_LINK) {
				CString target;
				if (getlinkinfo(strPath, target)) {
					if (PathIsURL(target)) {
						strPath.Empty();
						return FALSE;
					}
					strPath = target;
				}
			}
			return TRUE;
		}
		return FALSE;
	}

	void SetPathInfo()
	{
		m_fileattr = 0;
		WIN32_FIND_DATA wfd;
		if (SHGetDataFromIDList(m_psf, m_pidl, SHGDFIL_FINDDATA, &wfd, sizeof(wfd)) == NOERROR) {
			m_fileattr = wfd.dwFileAttributes;
			if (m_fileattr & FILE_ATTRIBUTE_READONLY) m_attr |= SFGAO_READONLY;
			if (m_fileattr & FILE_ATTRIBUTE_HIDDEN) m_attr |= SFGAO_HIDDEN;
		}
	}

	inline BOOL HasSubFolder()
	{
		return (m_attr & SFGAO_HASSUBFOLDER);
	}

	LPCTSTR GetDisplayName(CString& strName, bool bUseFullyQualPidl = false)
	{
		if (!bUseFullyQualPidl && m_psf)
			idl_GetName(m_psf, m_pidl, SHGDN_INFOLDER, strName);
		else {
			SHFILEINFO sfi;
			SHGetFileInfo((LPCTSTR)m_pidlFq, 0, &sfi, sizeof(sfi), SHGFI_DISPLAYNAME | SHGFI_PIDL);
			strName = sfi.szDisplayName;
		}

#ifdef _DEBUG
#ifdef SHOW_ATTR_VALUE
		strName.AppendFormat(_T(" (%08X)(%02X:%02X)"), m_attr, GetDescriptionId(), *((BYTE*)m_pidl + 2));
#endif
#endif
		return (LPCTSTR)strName;
	}

	inline BOOL IsFileSystem()
	{
		return (m_attr & SFGAO_FILESYSTEM);
	}

	BOOL IsValid() { return TRUE; }

	inline BOOL IsDesktop()
	{
		return (m_psf == NULL);
	}

	void* GetUIObjectOf(HWND hwnd, REFIID riid)
	{
		void* pv = NULL;
		if (m_psf) {
			if (m_psf->GetUIObjectOf(hwnd, 1, &m_pidl, riid, NULL, &pv) != NOERROR)
				pv = NULL;
		}
		else {
			IShellFolder* psf;
			SHGetDesktopFolder(&psf);
			if (psf->GetUIObjectOf(hwnd, 1, (LPCITEMIDLIST*)&m_pidlFq, riid, NULL, &pv) != NOERROR)
				pv = NULL;
			psf->Release();
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

/*
	BOOL CFolderItem::ContextMenu(HWND hwnd, LPPOINT ppt)
	{
		BOOL bOK = FALSE;
		IContextMenu* pcm = (IContextMenu*)GetUIObjectOf(hwnd, IID_IContextMenu);
		if (pcm) {
			UINT flags = CMF_EXPLORE;
			if (GetKeyState(VK_SHIFT) & 0x8000)
				flags |= CMF_EXTENDEDVERBS;
			int cmdBack = TrackContextMenu(hwnd, pcm, ppt, flags, m_attr);
			pcm->Release();
			bOK = (cmdBack >=0);
		}
		return bOK;
	}
*/

	HGLOBAL GetDataFromDataObject(IDataObject* pdo, CLIPFORMAT cf)
	{
		HGLOBAL hmem = NULL;
		FORMATETC fe;
		SETFormatEtc(fe, cf, TYMED_HGLOBAL, -1);
		if (pdo->QueryGetData(&fe) == S_OK) {
			STGMEDIUM sm;
			if (pdo->GetData(&fe, &sm) == S_OK) {
				size_t size = GlobalSize(sm.hGlobal);
				void* ps = GlobalLock(sm.hGlobal);
				void* pd = GlobalAllocPtr(GMEM_SHARE | GMEM_MOVEABLE, size);
				if (pd) {
					hmem = (HGLOBAL)GlobalHandle(pd);
					memcpy(pd, ps, size);
					GlobalUnlockPtr(pd);
				}
				GlobalUnlock(sm.hGlobal);
				ReleaseStgMedium(&sm);
			}
		}
		return hmem;
	}

	HDROP MakeDropFiles()
	{
		HDROP hdrop = NULL;
		IDataObject* pdo = (IDataObject*)GetUIObjectOf(NULL, IID_IDataObject);
		if (pdo) {
			hdrop = (HDROP)GetDataFromDataObject(pdo, CF_HDROP);
			pdo->Release();
		}
		return hdrop;
	}

	LPTSTR MakeFileNames(LPTSTR pszNames, int cchMax)
	{
		CString str;
		if (pszNames && cchMax > 1 && m_fileattr && GetPathName(str)) {
			lstrcpyn(pszNames, str, cchMax - 1);
			pszNames[lstrlen(pszNames) + 1] = 0;
			return pszNames;
		}
		return NULL;
	}

	HRESULT BindToShellFolder(IShellFolder** ppsf)
	{
		if (m_psf == NULL)
			return SHGetDesktopFolder(ppsf);
		else 
			return m_psf->BindToObject(m_pidl, NULL, IID_IShellFolder, (void**)ppsf);
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


#endif		// __FOLDERITEM_H__
