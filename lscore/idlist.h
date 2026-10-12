/////////////////////////////////////////////////////////////////////////
// idlist.h

#pragma once
#ifndef __IDLIST_CLASS_H__
#define __IDLIST_CLASS_H__

#include "pidl.h"

////////////////////////////////////////////////////////////////////////
//

template <bool t_bManaged>
class CPidlT
{
public:
	LPITEMIDLIST m_pidl;

	CPidlT(LPITEMIDLIST pidl = NULL) : m_pidl(pidl)
	{
	}

	CPidlT(IShellItem* psi) : m_pidl(NULL)
	{
		if (t_bManaged)
			GetFromShellItem(psi);
	}

	CPidlT(LPCTSTR pszPath) : m_pidl(NULL)
	{
		if (t_bManaged)
			CreateFromPath(pszPath);
	}

	CPidlT(HWND hwndOwner, int nFolder, HANDLE hToken = NULL) : m_pidl(NULL)
	{
		if (t_bManaged)
			GetFolderLocation(hwndOwner, nFolder, hToken);
	}

	CPidlT(REFKNOWNFOLDERID guid, HANDLE hToken = NULL) : m_pidl(NULL)
	{
		if (t_bManaged)
			GetKnownFolder(guid, hToken);
	}

	~CPidlT()
	{
		if (t_bManaged && m_pidl != NULL)
			::CoTaskMemFree(Detach());
	}

	CPidlT<t_bManaged>& operator =(LPITEMIDLIST pidl)
	{
		Attach(pidl);
		return *this;
	}

	void Attach(LPITEMIDLIST pidl)
	{
		if (t_bManaged && m_pidl != NULL && m_pidl != pidl)
			::CoTaskMemFree(m_pidl);
		m_pidl = pidl;
	}

	LPITEMIDLIST Detach()
	{
		LPITEMIDLIST pidl = m_pidl;
		m_pidl = NULL;
		return pidl;
	}

	operator LPITEMIDLIST() const { return m_pidl; }
	operator LPCITEMIDLIST() const { return (LPCITEMIDLIST)m_pidl; }

	inline bool isNull() const { return (m_pidl == NULL); }
	inline bool isEmpty() const { return (m_pidl == NULL || m_pidl->mkid.cb == 0); }

	void Free()
	{
		::CoTaskMemFree(Detach());
	}

	void Alloc(DWORD size)
	{
		Attach((LPITEMIDLIST)::CoTaskMemAlloc(size));
		memset(m_pidl, 0, size);
	}

	inline void* GetData() { return (void*)m_pidl; }
	inline bool isDesktop() { return (m_pidl && (::ILGetSize(m_pidl) == 2)); }

	LPITEMIDLIST Clone(LPCITEMIDLIST pidlSrc)
	{
		Attach(ILClone(pidlSrc));
		return m_pidl;
	}

	LPITEMIDLIST Combine(PCIDLIST_ABSOLUTE pidl1, PCUIDLIST_RELATIVE pidl2)
	{
		Attach(ILCombine(pidl1, pidl2));
		return m_pidl;
	}

	HRESULT GetFolderLocation(HWND hwndOwner, int nFolder, HANDLE hToken = NULL)
	{
		LPITEMIDLIST pidl = NULL;
		HRESULT hr = ::SHGetFolderLocation(hwndOwner, nFolder, hToken, 0, &pidl);
		Attach(pidl);
		return hr;
	}

	HRESULT GetKnownFolder(REFKNOWNFOLDERID guid, HANDLE hToken = NULL)
	{
		HRESULT hr = E_FAIL;
		LPITEMIDLIST pidl = NULL;
		hr = SHGetKnownFolderIDList(guid, 0, hToken, &pidl);
		if (hr == S_OK)
			Attach(pidl);
		return hr;
	}

	HRESULT GetMyDocument()
	{
		if (OsVISTA())
			return GetKnownFolder(FOLDERID_DocumentsLibrary);
		LPITEMIDLIST pidl = NULL;
		HRESULT hr = E_FAIL;
		IShellFolder* psf;
		if (::SHGetDesktopFolder(&psf) == NOERROR) {
			ULONG eat;
			hr = psf->ParseDisplayName(0, 0, STR_MYDOCS_CLSID_W, &eat, &pidl, 0);
			psf->Release();
		}
		Attach(pidl);
		return hr;
	}

	HRESULT GetFromShellItem(IShellItem* psi)
	{
		HRESULT hr = E_FAIL;
		LPITEMIDLIST pidl;
		hr = SHGetIDListFromObject(psi, &pidl);
		if (hr == S_OK)
			Attach(pidl);
		return hr;
	}

	LPITEMIDLIST CreateFromPath(LPCTSTR pszPath)
	{
		Attach(::ILCreateFromPath(pszPath));
		return m_pidl;
	}

	UINT GetNumIds()
	{
		CPidlT<false> idl = m_pidl;
		UINT cIds = 0;
		while (!idl.isEmpty()) {
			cIds++;
			idl = idl.GetNext();
		}
		return cIds;
	}

	inline BOOL RemoveLastID() { return ::ILRemoveLastID(m_pidl); }
	inline LPCITEMIDLIST FindLastID() { return ::ILFindLastID(m_pidl); }
	inline LPCITEMIDLIST GetNext() { return ::ILGetNext(m_pidl); }
	inline BOOL isEqual(LPCITEMIDLIST pidlTarget) { return ::ILIsEqual(m_pidl, pidlTarget); }
	inline BOOL isParent(LPCITEMIDLIST pidlTarget, BOOL fImmediate = FALSE) { return ::ILIsParent(pidlTarget, m_pidl, fImmediate); }
	inline BOOL isChild(LPCITEMIDLIST pidlTarget, BOOL fImmediate = FALSE) { return ::ILIsParent(m_pidl, pidlTarget, fImmediate); }
	inline UINT GetSize() { return ::ILGetSize(m_pidl); }
	inline bool operator==(const CPidlT& item) const { return (::ILIsEqual(m_pidl, item.m_pidl) != FALSE); }
	inline bool operator!=(const CPidlT& item) const { return (::ILIsEqual(m_pidl, item.m_pidl) == FALSE); }

	BOOL GetPath(CString& str)
	{
		LPTSTR p = str.GetBuffer(MAX_PATH * 2);
		*p = 0;
		BOOL bOK = ::SHGetPathFromIDList(m_pidl, p);
		str.ReleaseBuffer();
		return bOK;
	}

	inline DWORD_PTR GetFileInfo(DWORD dwFileAttr, SHFILEINFO* psfi, UINT cbFileInfo, UINT uFlags) { return ::SHGetFileInfo((LPCTSTR)m_pidl, dwFileAttr, psfi, cbFileInfo, uFlags | SHGFI_PIDL); }

	SFGAOF GetAttributes(SFGAOF mask)
	{
		SHFILEINFO sfi = { 0 };
		sfi.dwAttributes = mask;
		GetFileInfo(0, &sfi, sizeof(sfi), SHGFI_ATTR_SPECIFIED | SHGFI_ATTRIBUTES);
		return (SFGAOF)sfi.dwAttributes;
	}

	void* GetUIObjectOf(HWND hwnd, REFIID riid)
	{
		void* pv = NULL;
		IShellFolder* psf;
		LPCITEMIDLIST pidl;
		if (::SHBindToParent(m_pidl, IID_PPV_ARGS(&psf), &pidl) == S_OK) {
			if (NOERROR != psf->GetUIObjectOf(hwnd, 1, &pidl, riid, 0, &pv))
				pv = NULL;
			psf->Release();
		}
		return pv;
	}

	IShellFolder* GetShellFolder()
	{
		if (!m_pidl) return NULL;

		IShellFolder* psf = NULL;
		IShellFolder* psfDesk;
		if (::SHGetDesktopFolder(&psfDesk) == NOERROR) {
			if (isDesktop())
				return psfDesk;

			if (psfDesk->BindToObject(m_pidl, NULL, IID_IShellFolder, (LPVOID*)&psf) != NOERROR)
				psf = NULL;
			psfDesk->Release();
		}
		if (!psf) {
			IShellFolder* psfParent;
			LPCITEMIDLIST pidlSingle;
			if (::SHBindToParent(m_pidl, IID_PPV_ARGS(&psfParent), &pidlSingle) == S_OK) {
				if (psfParent->BindToObject(pidlSingle, 0, IID_PPV_ARGS(&psf)) != S_OK)
					psf = NULL;
				psfParent->Release();
			}
		}
		return psf;
	}

	IShellFolder* BindToParent(LPCITEMIDLIST* ppidl)
	{
		IShellFolder* psf;
		if (m_pidl && SUCCEEDED(::SHBindToParent(m_pidl, IID_PPV_ARGS(&psf), ppidl)))
			return psf;
		return NULL;
	}

	LPITEMIDLIST MakeRoot()
	{
		if (!m_pidl) return NULL;

		// パス名を取得
		CString dir;
		if (GetPath(dir)) {
			CPidl idlTemp;
			idlTemp.CreateFromPath(dir);
			if (idlTemp.isEqual(m_pidl)) {
				// 一致すればパスからルートパスを取得
				PathStripToRoot((LPTSTR)dir.GetString());
				idlTemp.CreateFromPath(dir);
				Attach(idlTemp.Detach());
			}
			else {
				idlTemp.GetMyDocument();
				if (::ILIsParent(idlTemp, m_pidl, false))
					Attach(idlTemp.Detach());
			}
		}
		return m_pidl;

	}
};

typedef CPidlT<false>   CPidlHandle;
typedef CPidlT<true>    CPidl;




#endif		// __IDLIST_CLASS_H__
