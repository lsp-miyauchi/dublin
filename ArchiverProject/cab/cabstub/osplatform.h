////////////////////////////////////////////////////////////////////////
// osplatform.h

#pragma once
#ifndef __OSPLATFORM_H__
#define __OSPLATFORM_H__

////////////////////////////////////////////////////////////////////////
//

#ifndef __ATLBASE_H__

#ifndef DLLVERSIONINFO

typedef struct _DLLVERSIONINFO {
    DWORD cbSize;
    DWORD dwMajorVersion;                   // Major version
    DWORD dwMinorVersion;                   // Minor version
    DWORD dwBuildNumber;                    // Build number
    DWORD dwPlatformID;                     // DLLVER_PLATFORM_*
} DLLVERSIONINFO;

typedef HRESULT (CALLBACK* DLLGETVERSIONPROC)(DLLVERSIONINFO *);

#endif		// ndef DLLVERSIONINFO

inline HRESULT AtlGetDllVersion(HINSTANCE hInstDLL, DLLVERSIONINFO* pDllVersionInfo)
{
	if (pDllVersionInfo == NULL)
		return E_INVALIDARG;
	DLLGETVERSIONPROC pfnDllGetVersion = (DLLGETVERSIONPROC)::GetProcAddress(hInstDLL, "DllGetVersion");
	if (pfnDllGetVersion == NULL)
		return E_NOTIMPL;
	return (*pfnDllGetVersion)(pDllVersionInfo);
}

inline HRESULT AtlGetDllVersion(LPCTSTR lpstrDllName, DLLVERSIONINFO* pDllVersionInfo)
{
	HINSTANCE hInstDLL = ::LoadLibrary(lpstrDllName);
	if (hInstDLL == NULL)
		return E_FAIL;
	HRESULT hRet = AtlGetDllVersion(hInstDLL, pDllVersionInfo);
	::FreeLibrary(hInstDLL);
	return hRet;
}

inline HRESULT AtlGetCommCtrlVersion(LPDWORD pdwMajor, LPDWORD pdwMinor)
{
	if (pdwMajor == NULL || pdwMinor == NULL)
		return E_INVALIDARG;

	DLLVERSIONINFO dvi;
	::ZeroMemory(&dvi, sizeof(dvi));
	dvi.cbSize = sizeof(dvi);
	HRESULT hRet = AtlGetDllVersion(_T("comctl32.dll"), &dvi);

	if(SUCCEEDED(hRet)) {
		*pdwMajor = dvi.dwMajorVersion;
		*pdwMinor = dvi.dwMinorVersion;
	}
	else if(hRet == E_NOTIMPL) {
		// If DllGetVersion is not there, then the DLL is a version
		// previous to the one shipped with IE 3.x
		*pdwMajor = 4;
		*pdwMinor = 0;
		hRet = S_OK;
	}

	return hRet;
}

inline HRESULT AtlGetShellVersion(LPDWORD pdwMajor, LPDWORD pdwMinor)
{
	if (pdwMajor == NULL || pdwMinor == NULL)
		return E_INVALIDARG;

	DLLVERSIONINFO dvi;
	::ZeroMemory(&dvi, sizeof(dvi));
	dvi.cbSize = sizeof(dvi);
	HRESULT hRet = AtlGetDllVersion(_T("shell32.dll"), &dvi);

	if(SUCCEEDED(hRet)) {
		*pdwMajor = dvi.dwMajorVersion;
		*pdwMinor = dvi.dwMinorVersion;
	}
	else if(hRet == E_NOTIMPL) {
		// If DllGetVersion is not there, then the DLL is a version
		// previous to the one shipped with IE 4.x
		*pdwMajor = 4;
		*pdwMinor = 0;
		hRet = S_OK;
	}

	return hRet;
}

#endif		// __ATLBASE_H__

////////////////////////////////////////////////////////////////////////
//

class COSPlatform
{
public:
	BOOL m_bWow64;
	DWORD m_osver;
	DWORD m_verComCtrl;
	DWORD m_verShell;
	DWORD m_idPlatform;

	COSPlatform()
		: m_bWow64(FALSE)
		, m_osver(MAKELONG(0, 5))
		, m_verComCtrl(MAKELONG(0, 4))
		, m_verShell(MAKELONG(0, 4))
		, m_idPlatform(0)
	{
		OSVERSIONINFO osv;
		memset(&osv, 0, sizeof(osv));
		osv.dwOSVersionInfoSize = sizeof(osv);
		GetVersionEx(&osv);
		m_osver = MAKELONG(osv.dwMinorVersion, osv.dwMajorVersion);
		m_idPlatform = osv.dwPlatformId;

		BOOL (WINAPI* pfn)(HANDLE, PBOOL);
		(FARPROC&)pfn = GetProcAddress(GetModuleHandle(_T("kernel32")), "IsWow64Process");
		if (pfn)
			(*pfn)(GetCurrentProcess(), &m_bWow64);

		DWORD major, minor;
		// Commctrl
		if (SUCCEEDED(AtlGetCommCtrlVersion(&major, &minor)))
			m_verComCtrl = MAKELONG(minor, major);
		// shellバージョン
		if (SUCCEEDED(AtlGetShellVersion(&major, &minor)))
			m_verShell = MAKELONG(minor, major);
	}
	__inline bool IsWow64() { return (m_bWow64 != FALSE); }
	__inline bool IsComm6() { return (m_verComCtrl >= MAKELONG(0, 6)); }
	__inline bool OsNT() { return (m_idPlatform == VER_PLATFORM_WIN32_NT); }
	__inline bool OsXP() { return (m_osver >= MAKELONG(1, 5)); }
	__inline bool OsVISTA() { return (m_osver >= MAKELONG(0, 6)); }
	__inline DWORD GetShellVersion() { return m_verShell; }
	__inline DWORD GetComCtrlVersion() { return m_verComCtrl; }
};


#endif			// __OSPLATFORM_H__
