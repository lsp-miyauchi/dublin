////////////////////////////////////////////////////////////////////////
// osplatform.h

#pragma once
#ifndef __OSPLATFORM_H__
#define __OSPLATFORM_H__

#define _WINVERCMP

inline bool __wvercmp(WORD wMajorVersion, WORD wMinorVersion)
{
	OSVERSIONINFOEXW osv = { sizeof(osv), 0, 0, 0, 0, {0}, 0, 0 };
	DWORDLONG mask = VerSetConditionMask(VerSetConditionMask(0, VER_MAJORVERSION, VER_GREATER_EQUAL), VER_MINORVERSION, VER_GREATER_EQUAL);
	osv.dwMajorVersion = wMajorVersion, osv.dwMinorVersion = wMinorVersion;
	return VerifyVersionInfoW(&osv, VER_MAJORVERSION | VER_MINORVERSION, mask) != FALSE;
}
inline bool _isXPOrGreater() { return __wvercmp(5, 1); }
inline bool _isVistaOrGreater() { return __wvercmp(6, 0); }
inline bool _isWin7OrGreater() { return __wvercmp(6, 1); }
inline bool _isWin8OrGreater() { return __wvercmp(6, 2); }
inline bool _isWin81OrGreater() { return __wvercmp(6, 3); }
inline bool _isWin10OrGreater() { return __wvercmp(10, 0); }


#ifdef _WIN64
	#define MAKEOSVERSP(major,minor,sp)	((ULONGLONG)(((DWORD_PTR)(major)) << 32) | (((DWORD_PTR)(minor)) << 16) | ((DWORD_PTR)(sp)))
#else
	inline ULONGLONG MAKEOSVERSP(DWORD major, DWORD minor, DWORD sp)
	{
		ULARGE_INTEGER uli;
		uli.HighPart = (DWORD)major;
		uli.LowPart = (DWORD)MAKELONG(sp, minor);
		return uli.QuadPart;
	}
#endif

class COSPlatform
{
public:
	BOOL m_bWow64;					// x64ã‚Å32bitƒAƒvƒŠ‚Æ‚µ‚Ä“®ì‚µ‚Ä‚¢‚éê‡‚É‚ÍTRUE
	DWORD m_osver;
	ULONGLONG m_osversp;
	DWORD m_verComCtrl;
	DWORD m_verShell;
	WORD m_sp;
	bool m_bThemeChecked;
	bool m_bThemed;
	bool m_bAlpha;

	COSPlatform()
		: m_bWow64(FALSE)
		, m_osver(MAKELONG(0, 5))
		, m_osversp(MAKEOSVERSP(5, 0, 0))
		, m_sp(0)
		, m_verComCtrl(MAKELONG(0, 4))
		, m_bThemeChecked(false)
		, m_bThemed(false)
		, m_bAlpha(false)
	{
		HMODULE hmod = GetModuleHandle(_T("kernel32"));
		BOOL (WINAPI* pfngv)(OSVERSIONINFO*);
		BOOL (WINAPI* pfnwo)(HANDLE, PBOOL);
		(FARPROC&)pfngv = GetProcAddress(hmod, "GetVersionExW");
		(FARPROC&)pfnwo = GetProcAddress(hmod, "IsWow64Process");
		// wow64
		if (pfnwo) {
			pfnwo(GetCurrentProcess(), &m_bWow64);
		}
		OSVERSIONINFOEX osv = {0};
		osv.dwOSVersionInfoSize = sizeof(osv);
		if (pfngv) {
			pfngv((OSVERSIONINFO*)&osv);
			m_osver = MAKELONG(osv.dwMinorVersion, osv.dwMajorVersion);
			m_osversp = MAKEOSVERSP(osv.dwMajorVersion, osv.dwMinorVersion, osv.wServicePackMajor);
			m_sp = osv.wServicePackMajor;
		}
		DWORDLONG cdm = 0;
		VER_SET_CONDITION(cdm, VER_MAJORVERSION, VER_GREATER_EQUAL);
		VER_SET_CONDITION(cdm, VER_MINORVERSION, VER_GREATER_EQUAL);
		// VISTA or 2008
		osv.dwMajorVersion = 6, osv.dwMinorVersion = 0;
		if (VerifyVersionInfo(&osv, VER_MAJORVERSION | VER_MINORVERSION, cdm)) { m_osver = MAKELONG(0, 6); }
		// 7 or 2008R2
		osv.dwMajorVersion = 6, osv.dwMinorVersion = 1;
		if (VerifyVersionInfo(&osv, VER_MAJORVERSION | VER_MINORVERSION, cdm)) { m_osver = MAKELONG(1, 6); }
		// 8 or 2012
		osv.dwMajorVersion = 6, osv.dwMinorVersion = 2;
		if (VerifyVersionInfo(&osv, VER_MAJORVERSION | VER_MINORVERSION, cdm)) { m_osver = MAKELONG(2, 6); }
		// 8.1 or 2012R2
		osv.dwMajorVersion = 6, osv.dwMinorVersion = 3;
		if (VerifyVersionInfo(&osv, VER_MAJORVERSION | VER_MINORVERSION, cdm)) { m_osver = MAKELONG(3, 6); }
		// 10 or 2016
		osv.dwMajorVersion = 10, osv.dwMinorVersion = 0;
		if (VerifyVersionInfo(&osv, VER_MAJORVERSION | VER_MINORVERSION, cdm)) { m_osver = MAKELONG(0, 10); }
		// Commctrl
		DWORD major, minor;
		if (SUCCEEDED(AtlGetCommCtrlVersion(&major, &minor))) { m_verComCtrl = MAKELONG(minor, major); }
		// 32bitRGBA
		CClientDC dc(NULL);
		m_bAlpha = (dc.GetDeviceCaps(BITSPIXEL) > 24);
	}

	bool IsThemed()
	{
		if (!m_bThemeChecked) {
			m_bThemeChecked = true;
			BOOL bTheme = FALSE;
			if (IsComm6()) {
				HMODULE hmod = ::LoadLibrary(_T("uxtheme.dll"));
				if (hmod) {
					BOOL (WINAPI* rIsThemeActive)();
					BOOL (WINAPI* rIsAppThemed)();

					(FARPROC&)rIsThemeActive = GetProcAddress(hmod, "IsThemeActive");
					bTheme = (rIsThemeActive != NULL) && (rIsThemeActive() != FALSE);
					(FARPROC&)rIsAppThemed = GetProcAddress(hmod, "IsAppThemed");
					bTheme = (bTheme && (rIsAppThemed != NULL) && (rIsAppThemed() != FALSE));

					::FreeLibrary(hmod);
				}
			}
			m_bThemed = (bTheme != FALSE);
		}
		return m_bThemed;
	}

	__inline bool IsWow64() { return (m_bWow64 != FALSE); }
	__inline bool IsComm6() { return (m_verComCtrl >= MAKELONG(0, 6)); }
	__inline bool OsXP() { return (m_osver >= MAKELONG(1, 5)); }
	__inline bool Os2000() { return (m_osver < MAKELONG(1, 5)); }
	__inline bool OsVISTA() { return (m_osver >= MAKELONG(0, 6)); }
	__inline bool Os7() { return (m_osver >= MAKELONG(1, 6)); }
	__inline bool Os8() { return (m_osver >= MAKELONG(2, 6)); }
	__inline bool Os81() { return (m_osver >= MAKELONG(3, 6)); }
	__inline bool Os10() { return (m_osver >= MAKELONG(0, 10)); }
	__inline bool IsVistaThemed() { return (OsVISTA() && IsThemed()); }
	__inline bool IsRGBA() { return (IsComm6() && m_bAlpha); }
	__inline DWORD GetComCtrlVersion() { return m_verComCtrl; }
	__inline ULONGLONG OsVersionSP() { return m_osversp; }

#ifdef _WIN64
	__inline bool IsWin64() { return true; }
#else
	__inline bool IsWin64() { return false; }
#endif
};


#endif			// __OSPLATFORM_H__
