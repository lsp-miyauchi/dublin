///////////////////////////////////////////////////////////////////////
// itemnamelimits.h

#pragma once
#ifndef __ITEMNAMELIMITS_H__
#define __ITEMNAMELIMITS_H__

MIDL_INTERFACE("28997A04-63A4-421d-BF58-154F7BD82E8A")
ILimitEditChars : public IUnknown
{
public:
	CStringW m_strLimitChars;

	STDMETHOD(SetLimitChars)(LPCWSTR pszLimitChars)
	{
		m_strLimitChars = pszLimitChars;
		return S_OK;
	}
};


class ATL_NO_VTABLE CLimitEditChars : 
	public CComObjectRootEx<CComSingleThreadModel>,
	public IShellFolder,
	public IItemNameLimits,
	public ILimitEditChars
{
public:
	CLimitEditChars()
	{
	}

	BEGIN_COM_MAP(CLimitEditChars)
		COM_INTERFACE_ENTRY(ILimitEditChars)
		COM_INTERFACE_ENTRY(IShellFolder)
		COM_INTERFACE_ENTRY(IItemNameLimits)
	END_COM_MAP()

	DECLARE_PROTECT_FINAL_CONSTRUCT()

	// IItemNameLimits
	STDMETHOD(GetMaxLength)(LPCWSTR pszName,int *piMaxNameLen)
	{
		return E_NOTIMPL;
	}

	STDMETHOD(GetValidCharacters)(LPWSTR* ppwszValidChars, LPWSTR* ppwszInvalidChars)
	{
/*
		// 「"」「/」「?」「*」「<」「>」を入力不可能な文字にする
		*ppwszValidChars = NULL;
		*ppwszInvalidChars = (WCHAR*)::CoTaskMemAlloc(sizeof(WCHAR) * 20);
		if (*ppwszInvalidChars == NULL)
			return E_FAIL;
		lstrcpyW(*ppwszInvalidChars, L"\"/?*<>");
		return S_OK;
*/
		*ppwszValidChars = NULL;
		*ppwszInvalidChars = (WCHAR*)::CoTaskMemAlloc((m_strLimitChars.GetLength() + 1) * sizeof(WCHAR));
		if (*ppwszInvalidChars == NULL)
			return E_FAIL;
		lstrcpyW(*ppwszInvalidChars, m_strLimitChars);
		return S_OK;
	}

	// IShellFolder
	STDMETHOD(BindToObject)(LPCITEMIDLIST pidl,LPBC pbc,REFIID riid,VOID **ppvOut) { return E_NOTIMPL; }
	STDMETHOD(BindToStorage)(LPCITEMIDLIST pidl,LPBC pbc,REFIID riid,VOID **ppvOut) { return E_NOTIMPL; }
	STDMETHOD(CompareIDs)(LPARAM lParam,LPCITEMIDLIST pidl1,LPCITEMIDLIST pidl2) { return E_NOTIMPL; }
	STDMETHOD(CreateViewObject)(HWND hwndOwner,REFIID riid,VOID **ppvOut) { return E_NOTIMPL; }
	STDMETHOD(EnumObjects)(HWND hwndOwner,SHCONTF grfFlags,IEnumIDList **ppenumIDList) { return E_NOTIMPL; }
	STDMETHOD(GetAttributesOf)(UINT cidl,LPCITEMIDLIST *apidl,SFGAOF *rgfInOut) { return E_NOTIMPL; }
	STDMETHOD(GetDisplayNameOf)(LPCITEMIDLIST pidl,DWORD uFlags,LPSTRRET lpName) { return E_NOTIMPL; }
	STDMETHOD(GetUIObjectOf)(HWND hwndOwner,UINT cidl,LPCITEMIDLIST *apidl,REFIID riid,UINT *rgfReserved,VOID **ppv) { return E_NOTIMPL; }
	STDMETHOD(ParseDisplayName)(HWND hwnd,LPBC pbc,LPOLESTR pwszDisplayName,ULONG *pchEaten,LPITEMIDLIST *ppidl,ULONG *pdwAttributes) { return E_NOTIMPL; }
	STDMETHOD(SetNameOf)(HWND hwndOwner,LPCITEMIDLIST pidl,LPCOLESTR lpszName,DWORD uFlags,LPITEMIDLIST *ppidlOut) { return E_NOTIMPL; }
};


#endif		// __ITEMNAMELIMITS_H__
