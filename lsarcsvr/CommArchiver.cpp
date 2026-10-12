// CommArchiver.cpp : CCommArchiver の実装

#include "stdafx.h"
#include "CommArchiver.h"
#include "common.h"

////////////////////////////////////////////////////////////////////////
// CCommArchiver

FARPROC CCommArchiver::FindArchiveFunction(LPCWSTR pFuncName)
{
	if (m_hmod) {
		CStringW wfunc = m_wstrPrefix;
		if (pFuncName && *pFuncName)
			wfunc += pFuncName;

		char func[128];
		WideCharToMultiByte(CP_ACP, 0, wfunc, -1, func, ARRAYSIZE(func), NULL, NULL);
		return ::GetProcAddress(m_hmod, func);
	}
	return NULL;
}

void CCommArchiver::Terminate(void)
{
	if (m_hmod) FreeLibrary(m_hmod);

	m_hmod = NULL;
	m_hArc = NULL;
	m_wstrDll.Empty();
	m_wstrPrefix.Empty();
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

	// windowコールバック
	m_SetOwnerWindow = NULL;
	m_ClearOwnerWindow = NULL;
	m_SetOwnerWindowEx = NULL;
	m_KillOwnerWindowEx = NULL;
	m_SetOwnerWindowExTotal = NULL;
}

////////////////////////////////////////////////////////////////////////
//

STDMETHODIMP CCommArchiver::Initialize(HWND hwnd, LPCWSTR pszPrefix, LPCWSTR pszDll)
{
	Terminate();

	m_wstrDll = pszDll;
	m_hWnd = hwnd;
	m_hmod = LoadLibrary(m_wstrDll);
	if (m_hmod == NULL)
		return E_FAIL;

	if (pszPrefix && *pszPrefix)
		m_wstrPrefix = pszPrefix;
	else {
		char prx[128];
		BOOL (WINAPI* pfn)(LPSTR, LONG);
		(FARPROC&)pfn = GetProcAddress(m_hmod, "DllGetFunctionPrefix");
		if (!pfn || !pfn(prx, ARRAYSIZE(prx))) {
			Terminate();
			return E_INVALIDARG;
		}
		WCHAR wsz[128];
		MultiByteToWideChar(CP_ACP, 0, prx, -1, wsz, ARRAYSIZE(wsz));
		m_wstrPrefix = wsz;
	}

	Locked(TRUE);

	(FARPROC&)m_OpenArchive				= FindArchiveFunction(_T("OpenArchive"));
	(FARPROC&)m_OpenArchiveW			= FindArchiveFunction(_T("OpenArchiveW"));
	(FARPROC&)m_CloseArchive			= FindArchiveFunction(_T("CloseArchive"));
	(FARPROC&)m_FindFirst				= FindArchiveFunction(_T("FindFirst"));
	(FARPROC&)m_FindFirstW				= FindArchiveFunction(_T("FindFirstW"));
	(FARPROC&)m_FindNext				= FindArchiveFunction(_T("FindNext"));
	(FARPROC&)m_FindNextW				= FindArchiveFunction(_T("FindNextW"));
	(FARPROC&)m_GetArcFileName			= FindArchiveFunction(_T("GetArcFileName"));
	(FARPROC&)m_GetArcFileNameW			= FindArchiveFunction(_T("GetArcFileNameW"));
	(FARPROC&)m_GetArcFileSize			= FindArchiveFunction(_T("GetArcFileSize"));
	(FARPROC&)m_GetArcOriginalSize		= FindArchiveFunction(_T("GetArcOriginalSize"));
	(FARPROC&)m_GetArcCompressedSize	= FindArchiveFunction(_T("GetArcCompressedSize"));
	(FARPROC&)m_GetArcRatio				= FindArchiveFunction(_T("GetArcRatio"));
	(FARPROC&)m_GetArcDate				= FindArchiveFunction(_T("GetArcDate"));
	(FARPROC&)m_GetArcTime				= FindArchiveFunction(_T("GetArcTime"));
	(FARPROC&)m_GetArcOSType			= FindArchiveFunction(_T("GetArcOSType"));
	(FARPROC&)m_GetArcCreateTimeEx		= FindArchiveFunction(_T("GetArcCreateTimeEx"));
	(FARPROC&)m_GetArcAccessTimeEx		= FindArchiveFunction(_T("GetArcAccessTimeEx"));
	(FARPROC&)m_GetWriteTimeEx			= FindArchiveFunction(_T("GetArcWriteTimeEx"));
	(FARPROC&)m_GetFileName				= FindArchiveFunction(_T("GetFileName"));
	(FARPROC&)m_GetFileNameW			= FindArchiveFunction(_T("GetFileNameW"));
	(FARPROC&)m_GetMethod				= FindArchiveFunction(_T("GetMethod"));
	(FARPROC&)m_GetMethodW				= FindArchiveFunction(_T("GetMethodW"));
	(FARPROC&)m_GetOriginalSize			= FindArchiveFunction(_T("GetOriginalSize"));
	(FARPROC&)m_GetCompressedSize		= FindArchiveFunction(_T("GetCompressedSize"));
	(FARPROC&)m_GetRatio				= FindArchiveFunction(_T("GetRatio"));
	(FARPROC&)m_GetDate					= FindArchiveFunction(_T("GetDate"));
	(FARPROC&)m_GetTime					= FindArchiveFunction(_T("GetTime"));
	(FARPROC&)m_GetWriteTime			= FindArchiveFunction(_T("GetWriteTime"));
	(FARPROC&)m_GetCreateTime			= FindArchiveFunction(_T("GetCreateTime"));
	(FARPROC&)m_GetAccessTime			= FindArchiveFunction(_T("GetAccessTime"));
	(FARPROC&)m_GetWriteTimeEx			= FindArchiveFunction(_T("GetWriteTimeEx"));
	(FARPROC&)m_GetCreateTimeEx			= FindArchiveFunction(_T("GetCreateTimeEx"));
	(FARPROC&)m_GetAccessTimeEx			= FindArchiveFunction(_T("GetAccessTimeEx"));
	(FARPROC&)m_GetCRC					= FindArchiveFunction(_T("GetCRC"));
	(FARPROC&)m_GetOSType				= FindArchiveFunction(_T("GetOSType"));
	(FARPROC&)m_GetAttribute			= FindArchiveFunction(_T("GetAttribute"));
	(FARPROC&)m_SetUnicodeMode			= FindArchiveFunction(_T("SetUnicodeMode"));

	(FARPROC&)m_SetOwnerWindow			= FindArchiveFunction(_T("SetOwnerWindow"));
	(FARPROC&)m_ClearOwnerWindow		= FindArchiveFunction(_T("ClearOwnerWindow"));
	(FARPROC&)m_SetOwnerWindowEx		= FindArchiveFunction(_T("SetOwnerWindowEx"));
	(FARPROC&)m_KillOwnerWindowEx		= FindArchiveFunction(_T("KillOwnerWindowEx"));
	(FARPROC&)m_SetOwnerWindowExTotal	= FindArchiveFunction(_T("SetOwnerWindowExTotal"));

	if (m_SetUnicodeMode != NULL)
		m_SetUnicodeMode(TRUE);
	return S_OK;
}

STDMETHODIMP CCommArchiver::Locked(DWORD dwMarker)
{
	HRESULT hr = E_NOINTERFACE;

	BOOL (WINAPI* func)(DWORD);
	(FARPROC&)func = FindArchiveFunction(_T("Locked"));
	if (func) {
		hr = (func(dwMarker) == TRUE) ? S_OK : E_FAIL;
	}

	return hr;
}

STDMETHODIMP CCommArchiver::CommandA(LPCSTR pCmd, CHAR* pResultBuf, ULONG sizeResult)
{
	HRESULT hr = E_NOINTERFACE;
	if (!m_wstrPrefix.Compare(L"Ish")) {
		int (WINAPI* func)(LPCSTR, LPSTR, DWORD);
		(FARPROC&)func = FindArchiveFunction(_T("Ex"));
		if (func) {
			m_lLastError = (LONG)func(pCmd, pResultBuf, (DWORD)sizeResult);
			hr = (m_lLastError == 0) ? S_OK : E_FAIL;
		}
	}
	else {
		int (WINAPI* func)(HWND, LPCSTR, LPSTR, DWORD);
		(FARPROC&)func = FindArchiveFunction(NULL);
		if (func) {
			m_lLastError = (LONG)func(m_hWnd, pCmd, pResultBuf, (DWORD)sizeResult);
			hr = (m_lLastError == 0) ? S_OK : E_FAIL;
		}
	}
	return hr;
}

STDMETHODIMP CCommArchiver::CommandW(LPCWSTR pCmd, WCHAR* pResultBuf, ULONG sizeResult)
{
	CStringA cmd;
	if (m_SetUnicodeMode) {
		LPSTR p = cmd.GetBuffer(2048);
		EncodeWStrToUTF(pCmd, -1, (BYTE*)p, 2048);
		cmd.ReleaseBuffer();
	}
	else
		WStrToStr(pCmd, cmd);

	CStringA res;
	ULONG size = sizeResult * sizeof(WCHAR);
	LPSTR pr = NULL;

	HRESULT hr = E_NOINTERFACE;
	int (WINAPI* funcW)(HWND, LPCWSTR, LPWSTR, DWORD);
	(FARPROC&)funcW = FindArchiveFunction(_T("W"));
	if (!m_wstrPrefix.Compare(L"Ish") || !funcW) {
		pr = res.GetBuffer(size);
		hr = CommandA(cmd, pr, size);
	}
	else {
		m_lLastError = (LONG)funcW(m_hWnd, pCmd, pResultBuf, (DWORD)sizeResult);
		hr = (m_lLastError == 0) ? S_OK : E_FAIL;
	}

	if (pr) {
		res.ReleaseBuffer();
		if (hr == S_OK && pResultBuf && sizeResult) {
			if (m_SetUnicodeMode)
				DecodeUTFToWStr((BYTE*)pr, -1, pResultBuf, sizeResult);
			else
				MultiByteToWideChar(CP_ACP, 0, pr, -1, pResultBuf, sizeResult);
		}
	}
	return hr;
}

STDMETHODIMP CCommArchiver::Command(LPCWSTR pCmd, CHAR* pResultBuf, ULONG sizeResult)
{
	CStringA cmd;
	if (m_SetUnicodeMode) {
		LPSTR p = cmd.GetBuffer(2048);
		EncodeWStrToUTF(pCmd, -1, (BYTE*)p, 2048);
		cmd.ReleaseBuffer();
	}
	else
		WStrToStr(pCmd, cmd);

	if (pResultBuf && sizeResult) *pResultBuf = 0;

	HRESULT hr = E_NOINTERFACE;
	if (!m_wstrPrefix.Compare(L"Ish")) {
		int (WINAPI* func)(LPCSTR, LPSTR, DWORD);
		(FARPROC&)func = FindArchiveFunction(_T("Ex"));
		if (func) {
			m_lLastError = (LONG)func(cmd, pResultBuf, (DWORD)sizeResult);
			hr = (m_lLastError == 0) ? S_OK : E_FAIL;
		}
	}
	else {
		int (WINAPI* func)(HWND, LPCSTR, LPSTR, DWORD);
		(FARPROC&)func = FindArchiveFunction(NULL);
		if (func) {
			m_lLastError = (LONG)func(m_hWnd, cmd, pResultBuf, (DWORD)sizeResult);
			hr = (m_lLastError == 0) ? S_OK : E_FAIL;

			if (m_SetUnicodeMode) {
				if (pResultBuf && sizeResult && *pResultBuf) {
					CStringW wstr;
					LPWSTR pw = wstr.GetBuffer(sizeResult * 2);
					DecodeUTFToWStr((BYTE*)pResultBuf, -1, pw, sizeResult * 2);
					wstr.ReleaseBuffer();
					WideCharToMultiByte(CP_ACP, 0, wstr, -1, pResultBuf, sizeResult, NULL, NULL);
				}
			}
		}
	}
	return hr;
}


STDMETHODIMP CCommArchiver::GetVersion(WORD* pwVersion)
{
	HRESULT hr = E_NOINTERFACE;

	WORD (WINAPI* func)(void);
	(FARPROC&)func = FindArchiveFunction(_T("GetVersion"));
	if (func) {
		*pwVersion = func();
		hr = S_OK;
	}

	return hr;
}

STDMETHODIMP CCommArchiver::OpenArchive(LPCWSTR pwzFile, DWORD flags)
{
	HRESULT hr = E_NOINTERFACE;
	if (m_OpenArchiveW) {
		if (m_hArc) CloseArchive();

		m_hArc = m_OpenArchiveW(m_hWnd, pwzFile, flags);
		hr = (m_hArc != NULL) ? S_OK : E_FAIL;
	}
	else if (m_OpenArchive) {
		if (m_hArc) CloseArchive();

		CStringA str;
		if (m_SetUnicodeMode) {
			LPCSTR psz = str.GetBuffer(MAX_PATH * 2);
			EncodeWStrToUTF(pwzFile, -1, (BYTE*)psz, MAX_PATH * 2);
			str.ReleaseBuffer();
		}
		else
			WStrToStr(pwzFile, str);

		m_hArc = m_OpenArchive(m_hWnd, str, flags);
		hr = (m_hArc != NULL) ? S_OK : E_FAIL;
	}
	return hr;
}

STDMETHODIMP CCommArchiver::CloseArchive(void)
{
	HRESULT hr = E_NOINTERFACE;
	if (m_CloseArchive) {
		hr = E_ACCESSDENIED;
		if (m_hArc) {
			m_lLastError = (LONG)m_CloseArchive(m_hArc);
			hr = (m_lLastError == 0) ? S_OK : E_FAIL;
		}
	}
	return hr;
}

STDMETHODIMP CCommArchiver::FindFirst(LPCWSTR pszFilter, BYTE* piia)
{
	if (!m_hArc) return E_ACCESSDENIED;

	HRESULT hr = E_NOINTERFACE;
	if (m_FindFirst) {
		CStringA str;
		WStrToStr(pszFilter, str);

		m_lLastError = (LONG)m_FindFirst(m_hArc, str, (INDIVIDUALINFO*)piia);
		hr = (m_lLastError == 0) ? S_OK : E_FAIL;
	}
	return hr;
}

STDMETHODIMP CCommArchiver::FindFirstW(LPCWSTR pszFilter, BYTE* piiw)
{
	if (!m_hArc) return E_ACCESSDENIED;

	HRESULT hr = E_NOINTERFACE;
	if (m_FindFirstW) {
		m_lLastError = (LONG)m_FindFirstW(m_hArc, pszFilter, (INDIVIDUALINFOW*)piiw);
		hr = (m_lLastError == 0) ? S_OK : E_FAIL;
	}
	else {
		INDIVIDUALINFO iia;
		hr = FindFirst(pszFilter, (BYTE*)&iia);
		if (hr == S_OK)
			iia2iiw(&iia, (INDIVIDUALINFOW*)piiw, (m_SetUnicodeMode != NULL));
	}
	return hr;
}

STDMETHODIMP CCommArchiver::FindNext(BYTE* piia)
{
	if (!m_hArc) return E_ACCESSDENIED;

	HRESULT hr = E_NOINTERFACE;
	if (m_FindNext) {
		m_lLastError = (LONG)m_FindNext(m_hArc, (INDIVIDUALINFO*)piia);
		hr = (m_lLastError == 0) ? S_OK : E_FAIL;
	}
	return hr;
}

STDMETHODIMP CCommArchiver::FindNextW(BYTE* piiw)
{
	if (!m_hArc) return E_ACCESSDENIED;

	HRESULT hr = E_NOINTERFACE;
	if (m_FindNextW) {
		m_lLastError = (LONG)m_FindNextW(m_hArc, (INDIVIDUALINFOW*)piiw);
		hr = (m_lLastError ==0) ? S_OK : E_FAIL;
	}
	else {
		INDIVIDUALINFO iia;
		hr = FindNext((BYTE*)&iia);
		if (hr == S_OK)
			iia2iiw(&iia, (INDIVIDUALINFOW*)piiw, (m_SetUnicodeMode != NULL));
	}
	return hr;
}

STDMETHODIMP CCommArchiver::GetAttribute(DWORD* pdwAttr)
{
	HRESULT hr = E_NOINTERFACE;
	if (m_GetAttribute) {
		hr = E_ACCESSDENIED;
		if (m_hArc) {
			hr = E_FAIL;
			int attr = m_GetAttribute(m_hArc);
			if (attr >= 0) {
				*pdwAttr = (DWORD)attr;
				hr = S_OK;
			}
		}
 	}
	return hr;
}

STDMETHODIMP CCommArchiver::GetArcFileName(WCHAR* pszBuffer, LONG cchMax)
{
	HRESULT hr = E_NOINTERFACE;
	if (m_GetArcFileName) {
		hr = E_ACCESSDENIED;
		if (m_hArc) {
			hr = E_FAIL;
			char name[MAX_PATH * 2];
			if (m_GetArcFileName(m_hArc, name, ARRAYSIZE(name)) == 0) {
				MultiByteToWideChar(CP_ACP, 0, name, -1, pszBuffer, cchMax);
				hr = S_OK;
			}
		}
	}
	return hr;
}

STDMETHODIMP CCommArchiver::GetArcFileSize(DWORD* pdwSize)
{
	HRESULT hr = E_NOINTERFACE;
	if (m_GetArcFileSize) {
		hr = E_ACCESSDENIED;
		if (m_hArc) {
			*pdwSize = m_GetArcFileSize(m_hArc);
			hr = (*pdwSize != (DWORD)-1) ? S_OK : E_FAIL;
		}
	}
	return hr;
}

STDMETHODIMP CCommArchiver::GetArcOriginalSize(DWORD* pdwSize)
{
	HRESULT hr = E_NOINTERFACE;
	if (m_GetArcOriginalSize) {
		hr = E_ACCESSDENIED;
		if (m_hArc) {
			*pdwSize = m_GetArcOriginalSize(m_hArc);
			hr = (*pdwSize != (DWORD)-1) ? S_OK : E_FAIL;
		}
	}
	return hr;
}

STDMETHODIMP CCommArchiver::GetArcCompressedSize(DWORD* pdwSize)
{
	HRESULT hr = E_NOINTERFACE;
	if (m_GetArcCompressedSize) {
		hr = E_ACCESSDENIED;
		if (m_hArc) {
			*pdwSize = m_GetArcCompressedSize(m_hArc);
			hr = (*pdwSize != (DWORD)-1) ? S_OK : E_FAIL;
		}
	}
	return hr;
}

STDMETHODIMP CCommArchiver::GetArcRatio(WORD* pwRatio)
{
	HRESULT hr = E_NOINTERFACE;
	if (m_GetArcRatio) {
		hr = E_ACCESSDENIED;
		if (m_hArc) {
			*pwRatio = m_GetArcRatio(m_hArc);
			hr = (*pwRatio != (WORD)-1) ? S_OK : E_FAIL;
		}
	}
	return hr;
}

STDMETHODIMP CCommArchiver::GetArcDateTime(FILETIME* ft)
{
	HRESULT hr = E_NOINTERFACE;
	if (m_GetArcDate && m_GetArcTime) {
		hr = E_ACCESSDENIED;
		if (m_hArc) {
			hr = E_FAIL;
			WORD dt = m_GetArcDate(m_hArc);
			WORD tm = m_GetArcTime(m_hArc);
			if (dt != (WORD)-1 && tm != (WORD)-1)
				hr = (DosDateTimeToFileTime(dt, tm, ft) == TRUE) ? S_OK : E_FAIL;
		}
	}
	return hr;
}

STDMETHODIMP CCommArchiver::GetArcOSType(LONG* plType)
{
	HRESULT hr = E_NOINTERFACE;
	if (m_GetArcOSType) {
		hr = E_ACCESSDENIED;
		if (m_hArc) {
			hr = E_FAIL;
			UINT type = m_GetArcOSType(m_hArc);
			if (type != (UINT)-1) {
				*plType = (LONG)type;
				hr = S_OK;
			}
		}
	}
	return hr;
}

STDMETHODIMP CCommArchiver::GetArcCreateTimeEx(FILETIME* ft)
{
	HRESULT hr = E_NOINTERFACE;
	if (m_GetArcCreateTimeEx) {
		hr = E_ACCESSDENIED;
		if (m_hArc)
			hr = (m_GetArcCreateTimeEx(m_hArc, ft) == TRUE) ? S_OK : E_FAIL;
	}
	return hr;
}

STDMETHODIMP CCommArchiver::GetArcAccessTimeEx(FILETIME* ft)
{
	HRESULT hr = E_NOINTERFACE;
	if (m_GetArcAccessTimeEx) {
		hr = E_ACCESSDENIED;
		if (m_hArc)
			hr = (m_GetArcAccessTimeEx(m_hArc, ft) == TRUE) ? S_OK : E_FAIL;
	}
	return hr;
}

STDMETHODIMP CCommArchiver::GetArcWriteTimeEx(FILETIME* ft)
{
	HRESULT hr = E_NOINTERFACE;
	if (m_GetArcWriteTimeEx) {
		hr = E_ACCESSDENIED;
		if (m_hArc)
			hr = (m_GetArcWriteTimeEx(m_hArc, ft) == TRUE) ? S_OK : E_FAIL;
	}
	return hr;
}

STDMETHODIMP CCommArchiver::GetFileName(WCHAR* pszName, LONG cchMax)
{
	HRESULT hr = E_NOINTERFACE;
	if (m_GetFileName) {
		hr = E_ACCESSDENIED;
		if (m_hArc) {
			hr = E_FAIL;
			char sz[MAX_PATH];
			if (m_GetFileName(m_hArc, sz, ARRAYSIZE(sz)) == 0) {
				MultiByteToWideChar(CP_ACP, 0, sz, -1, pszName, cchMax);
				hr = S_OK;
			}
		}
	}
	return hr;
}

STDMETHODIMP CCommArchiver::GetMethod(WCHAR* pszMethod, LONG cchMax)
{
	if (!m_hArc)
		return E_ACCESSDENIED;

	HRESULT hr = E_NOINTERFACE;
	if (m_GetMethodW) {
		hr = (m_GetMethodW(m_hArc, pszMethod, cchMax) == 0) ? S_OK : E_FAIL;
	}
	else if (m_GetMethod) {
		hr = E_FAIL;
		char sz[MAX_PATH];
		if (m_GetMethod(m_hArc, sz, ARRAYSIZE(sz)) == 0) {
			MultiByteToWideChar(CP_ACP, 0, sz, -1, pszMethod, cchMax);
			hr = S_OK;
		}
	}
	return hr;
}

STDMETHODIMP CCommArchiver::GetOriginalSize(DWORD* pdwSize)
{
	HRESULT hr = E_NOINTERFACE;
	if (m_GetOriginalSize) {
		hr = E_ACCESSDENIED;
		if (m_hArc) {
			*pdwSize = m_GetOriginalSize(m_hArc);
			hr = (*pdwSize != (DWORD)-1) ? S_OK : E_FAIL;
		}
	}
	return hr;
}

STDMETHODIMP CCommArchiver::GetCompressedSize(DWORD* pdwSize)
{
	HRESULT hr = E_NOINTERFACE;
	if (m_GetCompressedSize) {
		hr = E_ACCESSDENIED;
		if (m_hArc) {
			*pdwSize = m_GetCompressedSize(m_hArc);
			hr = (*pdwSize != (DWORD)-1) ? S_OK : E_FAIL;
		}
	}
	return hr;
}

STDMETHODIMP CCommArchiver::GetRatio(WORD* pwRatio)
{
	HRESULT hr = E_NOINTERFACE;
	if (m_GetRatio) {
		hr = E_ACCESSDENIED;
		if (m_hArc) {
			*pwRatio = m_GetRatio(m_hArc);
			hr = (*pwRatio != (WORD)-1) ? S_OK : E_FAIL;
		}
	}
	return hr;
}

STDMETHODIMP CCommArchiver::GetDateTime(FILETIME* ft)
{
	HRESULT hr = E_NOINTERFACE;
	if (m_GetDate && m_GetTime) {
		hr = E_ACCESSDENIED;
		if (m_hArc) {
			hr = E_FAIL;
			WORD dt = m_GetDate(m_hArc);
			WORD tm = m_GetTime(m_hArc);
			if (dt != (WORD)-1 && tm != (WORD)-1)
				hr = (DosDateTimeToFileTime(dt, tm, ft) == TRUE) ? S_OK : E_FAIL;
		}
	}
	return hr;
}

STDMETHODIMP CCommArchiver::GetWriteTime(LONG* tm)
{
	HRESULT hr = E_NOINTERFACE;
	if (m_GetWriteTime) {
		hr = E_ACCESSDENIED;
		if (m_hArc) {
			*tm = m_GetWriteTime(m_hArc);
			hr = (*tm != -1) ? S_OK : E_FAIL;
		}
	}
	return hr;
}

STDMETHODIMP CCommArchiver::GetCreateTime(LONG* tm)
{
	HRESULT hr = E_NOINTERFACE;
	if (m_GetCreateTime) {
		hr = E_ACCESSDENIED;
		if (m_hArc) {
			*tm = m_GetCreateTime(m_hArc);
			hr = (*tm != -1) ? S_OK : E_FAIL;
		}
	}
	return hr;
}

STDMETHODIMP CCommArchiver::GetAccessTime(LONG* tm)
{
	HRESULT hr = E_NOINTERFACE;
	if (m_GetAccessTime) {
		hr = E_ACCESSDENIED;
		if (m_hArc) {
			*tm = m_GetAccessTime(m_hArc);
			hr = (*tm != -1) ? S_OK : E_FAIL;
		}
	}
	return hr;
}

STDMETHODIMP CCommArchiver::GetCRC(DWORD* pdwCRC)
{
	HRESULT hr = E_NOINTERFACE;
	if (m_GetCRC) {
		hr = E_ACCESSDENIED;
		if (m_hArc) {
			*pdwCRC = m_GetCRC(m_hArc);
			hr = (*pdwCRC != (DWORD)-1) ? S_OK : E_FAIL;
		}
	}
	return hr;
}

STDMETHODIMP CCommArchiver::GetOSType(ULONG* plType)
{
	HRESULT hr = E_NOINTERFACE;
	if (m_GetOSType) {
		hr = E_ACCESSDENIED;
		if (m_hArc) {
			hr = E_FAIL;
			UINT type = m_GetOSType(m_hArc);
			if (type != (UINT)-1) {
				*plType = (LONG)type;
				hr = S_OK;
			}
		}
	}
	return hr;
}

STDMETHODIMP CCommArchiver::GetWriteTimeEx(FILETIME* ft)
{
	HRESULT hr = E_NOINTERFACE;
	if (m_GetWriteTimeEx) {
		hr = E_ACCESSDENIED;
		if (m_hArc)
			hr = (m_GetWriteTimeEx(m_hArc, ft) == TRUE) ? S_OK : E_FAIL;
	}
	return hr;
}

STDMETHODIMP CCommArchiver::GetCreateTimeEx(FILETIME* ft)
{
	HRESULT hr = E_NOINTERFACE;
	if (m_GetCreateTimeEx) {
		hr = E_ACCESSDENIED;
		if (m_hArc)
			hr = (m_GetCreateTimeEx(m_hArc, ft) == TRUE) ? S_OK : E_FAIL;
	}
	return hr;
}

STDMETHODIMP CCommArchiver::GetAccessTimeEx(FILETIME* ft)
{
	HRESULT hr = E_NOINTERFACE;
	if (m_GetAccessTimeEx) {
		hr = E_ACCESSDENIED;
		if (m_hArc)
			hr = (m_GetAccessTimeEx(m_hArc, ft) == TRUE) ? S_OK : E_FAIL;
	}
	return hr;
}

STDMETHODIMP CCommArchiver::SetBkgndMode(BOOL bBkgnd)
{
	HRESULT hr = E_NOINTERFACE;
	BOOL (WINAPI* func)(BOOL);
	(FARPROC&)func = FindArchiveFunction(_T("SetBackGroundMode"));
	if (func)
		hr = (func(bBkgnd) == TRUE) ? S_OK : E_FAIL;
	return hr;
}

STDMETHODIMP CCommArchiver::SetCursorMode(BOOL bOn)
{
	HRESULT hr = E_NOINTERFACE;
	BOOL (WINAPI* func)(BOOL);
	(FARPROC&)func = FindArchiveFunction(_T("SetCursorMode"));
	if (func)
		hr = (func(bOn) == TRUE) ? S_OK : E_FAIL;
	return hr;
}

STDMETHODIMP CCommArchiver::IsSFXFile(void)
{
	HRESULT hr = E_NOINTERFACE;

	int (WINAPI* func)(HANDLE);
	(FARPROC&)func = FindArchiveFunction(_T("IsSFXFile"));
	if (func)
		hr = (func(m_hArc) != 0) ? S_OK : E_FAIL;

	return hr;
}

STDMETHODIMP CCommArchiver::CheckArchive(LPCWSTR pszFile, ULONG mode)
{
	HRESULT hr = E_NOINTERFACE;

	BOOL (WINAPI* funcW)(LPCWSTR, ULONG);
	(FARPROC&)funcW = FindArchiveFunction(_T("CheckArchiveW"));
	if (funcW)
		hr = (funcW(pszFile, mode) == TRUE) ? S_OK : E_FAIL;
	else {
		BOOL (WINAPI* func)(LPCSTR, ULONG);
		(FARPROC&)func = FindArchiveFunction(_T("CheckArchive"));
		if (func) {
			CStringA str;
			WStrToStr(pszFile, str);
			hr = (func(str, mode) == TRUE) ? S_OK : E_FAIL;
		}
	}
	return hr;
}

STDMETHODIMP CCommArchiver::SetOwnerWindow(HWND hwnd)
{
	HRESULT hr = E_NOINTERFACE;
	BOOL (WINAPI* func)(HWND);
	(FARPROC&)func = FindArchiveFunction(_T("SetOwnerWindow"));
	if (func)
		hr = (func(hwnd) == TRUE) ? S_OK : E_FAIL;
	return hr;
}

STDMETHODIMP CCommArchiver::ClearOwnerWindow(void)
{
	HRESULT hr = E_NOINTERFACE;
	BOOL (WINAPI* func)(void);
	(FARPROC&)func = FindArchiveFunction(_T("ClearOwnerWindow"));
	if (func)
		hr = (func() == TRUE) ? S_OK : E_FAIL;
	return hr;
}

STDMETHODIMP CCommArchiver::SetOwnerWindowEx(HWND hwnd, LONG_PTR lpArcProcArg)
{
	HRESULT hr = E_NOINTERFACE;
	BOOL (WINAPI* func)(HWND, FARPROC);
	(FARPROC&)func = FindArchiveFunction(_T("SetOwnerWindowEx"));
	if (func)
		hr = (func(hwnd, (FARPROC)lpArcProcArg) == TRUE) ? S_OK : E_FAIL;
	return hr;
}

STDMETHODIMP CCommArchiver::SetOwnerWindowExTotal(HWND hwnd, LONG_PTR lpArcProcArg, BOOL bEnableTotalProgressArg)
{
	HRESULT hr = E_NOINTERFACE;
	BOOL(WINAPI * func)(HWND, FARPROC, BOOL);
	(FARPROC&)func = FindArchiveFunction(_T("SetOwnerWindowExTotal"));
	if (func)
		hr = (func(hwnd, (FARPROC)lpArcProcArg, bEnableTotalProgressArg) == TRUE) ? S_OK : E_FAIL;
	return hr;
}

STDMETHODIMP CCommArchiver::KillOwnerWindowEx(HWND hwnd)
{
	HRESULT hr = E_NOINTERFACE;
	BOOL (WINAPI* func)(HWND);
	(FARPROC&)func = FindArchiveFunction(_T("KillOwnerWindowEx"));
	if (func)
		hr = (func(hwnd) == TRUE) ? S_OK : E_FAIL;
	return hr;
}

STDMETHODIMP CCommArchiver::GetLastError(LONG* plErrorCode)
{
	*plErrorCode = m_lLastError;
	return S_OK;
}
