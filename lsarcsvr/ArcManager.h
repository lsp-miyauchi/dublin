// ArcManager.h : CArcManager の宣言

#pragma once
#include "resource.h"       // メイン シンボル
#include "lsarcsvr_i.h"

extern COSPlatform gosp;

////////////////////////////////////////////////////////////////////////
// CArcManager

class ATL_NO_VTABLE CArcManager :
	public CComObjectRootEx<CComSingleThreadModel>,
	public CComCoClass<CArcManager, &CLSID_ArcManager>,
	public IDispatchImpl<IArcManager, &IID_IArcManager, &LIBID_lsarcsvrLib, /*wMajor =*/ 1, /*wMinor =*/ 0>
{
public:
	CArcManager()
	{
	}

DECLARE_REGISTRY_RESOURCEID(IDR_ARCMANAGER)


BEGIN_COM_MAP(CArcManager)
	COM_INTERFACE_ENTRY(IArcManager)
	COM_INTERFACE_ENTRY(IDispatch)
END_COM_MAP()


	DECLARE_PROTECT_FINAL_CONSTRUCT()

	HRESULT FinalConstruct()
	{
		lccGetVersion();
		return S_OK;
	}

	void FinalRelease()
	{
	}

protected:
	BOOL GetArchiverTypeInfo(ULONG type, LPWSTR pszPrefix, LPWSTR pszDll);
	HRESULT CompressProc(HWND hwndParent, void* pd);

public:
	STDMETHOD(Archive)(HWND hwnd, ULONG type, LPCWSTR pszCmd, CHAR* pszResult, ULONG sizeResult, LONG* plArcError);
	STDMETHOD(ArchiveW)(HWND hwnd, ULONG type, LPCWSTR pszCmd, WCHAR* pszResult, ULONG sizeResult, LONG* plArcError);
	STDMETHOD(AddFiles)(HWND hwnd, LPCWSTR pszArc, LPCWSTR pszBase, LONG_PTR hdrop_ptr);
	STDMETHOD(CheckFiles)(HWND hwnd, LONG_PTR hdrop_ptr);
	STDMETHOD(CompressFiles)(HWND hwnd, LPCWSTR pszBase, LONG_PTR hdrop);
	STDMETHOD(CompressFilesExec)(HWND hwnd, LPCWSTR pszBase, LONG_PTR hdrop);
	STDMETHOD(ExtractFiles)(HWND hwnd, LPCWSTR pszPath, LONG_PTR hdrop_ptr);
	STDMETHOD(ExtractFilesExec)(HWND hwnd, LPCWSTR pszPath, LONG_PTR hdrop_ptr);
	STDMETHOD(ExtractFromArchive)(HWND hwnd, ULONG type, LPCWSTR pszFile, LPCWSTR pszList);
	STDMETHOD(BeginFind)(HWND hwnd, ULONG type, LPCWSTR pszFile, LPCWSTR pszFilter, BYTE* piiw, IUnknown** ppInterface);
	STDMETHOD(NextFind)(IUnknown* pInterface, BYTE* piiw);
	STDMETHOD(EndFind)(IUnknown* pInterface);
	STDMETHOD(GetArchiveType)(LPCWSTR pszFile, LONG* type, BOOL* pbUnicodeSupported, BOOL* pbUtf8Supported);
	STDMETHOD(GetDefaultFolder)(LPWSTR pszFolder);
	STDMETHOD(SetDefaultFolder)(LPCWSTR pszFolder);
	STDMETHOD(MakeSFX)(HWND hwnd, LPCWSTR pszFile, LPCWSTR pszTargetPath);
	STDMETHOD(IsArchive)(HWND hwnd, ULONG type, LPCWSTR pszArc);
	STDMETHOD(IsLibraryAvailable)(ULONG type);
	STDMETHOD(ListArchive)(HWND hwnd, ULONG type, LPCWSTR pszFile, CHAR* pszResult, ULONG size);
};

OBJECT_ENTRY_AUTO(__uuidof(ArcManager), CArcManager)
