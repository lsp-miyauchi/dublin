// CommArchiver.h : CCommArchiver の宣言

#pragma once
#include "resource.h"       // メイン シンボル
#include "lsarcsvr_i.h"
#include "comarcdef.h"


////////////////////////////////////////////////////////////////////////
// CCommArchiver

class ATL_NO_VTABLE CCommArchiver :
	public CComObjectRootEx<CComSingleThreadModel>,
	public CComCoClass<CCommArchiver, &CLSID_CommArchiver>,
	public IDispatchImpl<ICommArchiver, &IID_ICommArchiver, &LIBID_lsarcsvrLib, /*wMajor =*/ 1, /*wMinor =*/ 0>
{
private:
	HWND m_hWnd;
	HANDLE m_hArc;
	CStringW m_wstrDll;
	CStringW m_wstrPrefix;
	HMODULE m_hmod;
	LONG m_lLastError;
	HANDLE (WINAPI* m_OpenArchive)(HWND, LPCSTR, DWORD);
	HANDLE (WINAPI* m_OpenArchiveW)(HWND, LPCWSTR, DWORD);
	int (WINAPI* m_CloseArchive)(HANDLE);
	int (WINAPI* m_FindFirst)(HANDLE, LPCSTR, INDIVIDUALINFO*);
	int (WINAPI* m_FindFirstW)(HANDLE, LPCWSTR, INDIVIDUALINFOW*);
	int (WINAPI* m_FindNext)(HANDLE, INDIVIDUALINFO*);
	int (WINAPI* m_FindNextW)(HANDLE, INDIVIDUALINFOW*);
	int (WINAPI* m_GetArcFileName)(HANDLE, LPSTR, int);
	int (WINAPI* m_GetArcFileNameW)(HANDLE, LPWSTR, int);
	DWORD (WINAPI* m_GetArcFileSize)(HANDLE);
	DWORD (WINAPI* m_GetArcOriginalSize)(HANDLE);
	DWORD (WINAPI* m_GetArcCompressedSize)(HANDLE);
	WORD (WINAPI* m_GetArcRatio)(HANDLE);
	WORD (WINAPI* m_GetArcDate)(HANDLE);
	WORD (WINAPI* m_GetArcTime)(HANDLE);
	UINT (WINAPI* m_GetArcOSType)(HANDLE);
	BOOL (WINAPI* m_GetArcCreateTimeEx)(HANDLE, FILETIME*);
	BOOL (WINAPI* m_GetArcAccessTimeEx)(HANDLE, FILETIME*);
	BOOL (WINAPI* m_GetArcWriteTimeEx)(HANDLE, FILETIME*);
	int (WINAPI* m_GetAttribute)(HANDLE);
	int (WINAPI* m_GetFileName)(HANDLE, LPSTR, int);
	int (WINAPI* m_GetFileNameW)(HANDLE, LPWSTR, int);
	int (WINAPI* m_GetMethod)(HANDLE, LPSTR, int);
	int (WINAPI* m_GetMethodW)(HANDLE, LPWSTR, int);
	DWORD (WINAPI* m_GetOriginalSize)(HANDLE);
	DWORD (WINAPI* m_GetCompressedSize)(HANDLE);
	WORD (WINAPI* m_GetRatio)(HANDLE);
	WORD (WINAPI* m_GetDate)(HANDLE);
	WORD (WINAPI* m_GetTime)(HANDLE);
	DWORD (WINAPI* m_GetWriteTime)(HANDLE);
	DWORD (WINAPI* m_GetCreateTime)(HANDLE);
	DWORD (WINAPI* m_GetAccessTime)(HANDLE);
	DWORD (WINAPI* m_GetCRC)(HANDLE);
	UINT (WINAPI* m_GetOSType)(HANDLE);
	BOOL (WINAPI* m_GetWriteTimeEx)(HANDLE, FILETIME*);
	BOOL (WINAPI* m_GetCreateTimeEx)(HANDLE, FILETIME*);
	BOOL (WINAPI* m_GetAccessTimeEx)(HANDLE, FILETIME*);
	BOOL (WINAPI* m_SetUnicodeMode)(BOOL);
	BOOL (WINAPI* m_SetOwnerWindow)(HWND);
	BOOL (WINAPI* m_ClearOwnerWindow)(void);
	BOOL (WINAPI* m_SetOwnerWindowEx)(HWND, FARPROC);
	BOOL (WINAPI* m_KillOwnerWindowEx)(HWND);
	BOOL (WINAPI* m_SetOwnerWindowExTotal)(HWND);

public:
	CCommArchiver()
	{
	}

DECLARE_REGISTRY_RESOURCEID(IDR_COMMARCHIVER)


BEGIN_COM_MAP(CCommArchiver)
	COM_INTERFACE_ENTRY(ICommArchiver)
	COM_INTERFACE_ENTRY(IDispatch)
END_COM_MAP()

	DECLARE_PROTECT_FINAL_CONSTRUCT()

	HRESULT FinalConstruct()
	{
		m_hWnd = NULL;
		m_hArc = NULL;
		m_wstrDll.Empty();
		m_wstrPrefix.Empty();
		m_hmod = NULL;
		m_lLastError = 0;

		m_OpenArchive = NULL;
		m_OpenArchiveW = NULL;
		m_CloseArchive = NULL;
		m_FindFirst = NULL;
		m_FindFirstW = NULL;
		m_FindNext = NULL;
		m_FindNextW = NULL;

		// 書庫全体に関するもの
		m_GetArcFileName = NULL;
		m_GetArcFileNameW = NULL;
		m_GetArcFileSize = NULL;
		m_GetArcOriginalSize = NULL;
		m_GetArcCompressedSize = NULL;
		m_GetArcRatio = NULL;
		m_GetArcDate = NULL;
		m_GetArcTime = NULL;
		m_GetArcOSType = NULL;
		m_GetArcCreateTimeEx = NULL;
		m_GetArcAccessTimeEx = NULL;
		m_GetArcWriteTimeEx = NULL;

		// 格納ファイル毎のもの
		m_GetAttribute = NULL;
		m_GetFileName = NULL;
		m_GetFileNameW = NULL;
		m_GetMethod = NULL;
		m_GetMethodW = NULL;
		m_GetOriginalSize = NULL;
		m_GetCompressedSize = NULL;
		m_GetRatio = NULL;
		m_GetDate = NULL;
		m_GetTime = NULL;
		m_GetWriteTime = NULL;
		m_GetCreateTime = NULL;
		m_GetAccessTime = NULL;
		m_GetCRC = NULL;
		m_GetOSType = NULL;
		m_GetWriteTimeEx = NULL;
		m_GetCreateTimeEx = NULL;
		m_GetAccessTimeEx = NULL;
		m_SetOwnerWindow = NULL;
		m_ClearOwnerWindow = NULL;
		m_SetOwnerWindowEx = NULL;
		m_KillOwnerWindowEx = NULL;
		m_SetOwnerWindowExTotal = NULL;

		// その他個別ファンクションなど
		m_SetUnicodeMode = NULL;

		return S_OK;
	}

	void FinalRelease()
	{
		Terminate();
	}

	FARPROC FindArchiveFunction(LPCWSTR pFuncName);
	void Terminate(void);

public:
	STDMETHOD(Initialize)(HWND hwnd, LPCWSTR pszPrefix, LPCWSTR pszDll);
	STDMETHOD(Locked)(DWORD dwMarker);
	STDMETHOD(Command)(LPCWSTR pCmd, LPSTR pResultBuf, ULONG sizeResult);
	STDMETHOD(CommandA)(LPCSTR pCmd, LPSTR pResultBuf, ULONG sizeResult);
	STDMETHOD(CommandW)(LPCWSTR pCmd, LPWSTR pResultBuf, ULONG sizeResult);
	STDMETHOD(GetVersion)(WORD* pwVersion);
	STDMETHOD(OpenArchive)(LPCWSTR pszFile, DWORD dwFlags);
	STDMETHOD(CloseArchive)(void);
	STDMETHOD(FindFirst)(LPCWSTR pszFilter, BYTE* piia);
	STDMETHOD(FindFirstW)(LPCWSTR pszFilter, BYTE* piiw);
	STDMETHOD(FindNext)(BYTE* piia);
	STDMETHOD(FindNextW)(BYTE* piiw);
	STDMETHOD(GetAttribute)(DWORD* pdwAttr);
	STDMETHOD(GetArcFileName)(LPWSTR pszBuffer, LONG cchMax);
	STDMETHOD(GetArcFileSize)(DWORD* pdwSize);
	STDMETHOD(GetArcOriginalSize)(DWORD* pdwSize);
	STDMETHOD(GetArcCompressedSize)(DWORD* pdwSize);
	STDMETHOD(GetArcRatio)(WORD* pwRatio);
	STDMETHOD(GetArcDateTime)(FILETIME* ft);
	STDMETHOD(GetArcOSType)(LONG* plType);
	STDMETHOD(GetArcCreateTimeEx)(FILETIME* ft);
	STDMETHOD(GetArcAccessTimeEx)(FILETIME* ft);
	STDMETHOD(GetArcWriteTimeEx)(FILETIME* ft);
	STDMETHOD(GetFileName)(LPWSTR pszName, LONG cchMax);
	STDMETHOD(GetMethod)(LPWSTR pszMethod, LONG cchMax);
	STDMETHOD(GetOriginalSize)(DWORD* pdwSize);
	STDMETHOD(GetCompressedSize)(DWORD* pdwSize);
	STDMETHOD(GetRatio)(WORD* pwRatio);
	STDMETHOD(GetDateTime)(FILETIME* ft);
	STDMETHOD(GetWriteTime)(LONG* tm);
	STDMETHOD(GetCreateTime)(LONG* tm);
	STDMETHOD(GetAccessTime)(LONG* tm);
	STDMETHOD(GetCRC)(DWORD* pdwCRC);
	STDMETHOD(GetOSType)(ULONG* plType);
	STDMETHOD(GetWriteTimeEx)(FILETIME* ft);
	STDMETHOD(GetCreateTimeEx)(FILETIME* ft);
	STDMETHOD(GetAccessTimeEx)(FILETIME* ft);
	STDMETHOD(SetBkgndMode)(BOOL bBkgnd);
	STDMETHOD(SetCursorMode)(BOOL bOn);
	STDMETHOD(IsSFXFile)(void);
	STDMETHOD(CheckArchive)(LPCWSTR pszFile, ULONG mode);
	STDMETHOD(SetOwnerWindow)(HWND hwnd);
	STDMETHOD(ClearOwnerWindow)(void);
	STDMETHOD(SetOwnerWindowEx)(HWND hwnd, LONG_PTR	lpArcProcArg);
	STDMETHOD(KillOwnerWindowEx)(HWND hwnd);
	STDMETHOD(SetOwnerWindowExTotal)(HWND hwnd, LONG_PTR lpArcProcArg, BOOL bEnableTotalProgressArg);
	STDMETHOD(GetLastError)(LONG* plErrorCode);
};

OBJECT_ENTRY_AUTO(__uuidof(CommArchiver), CCommArchiver)
