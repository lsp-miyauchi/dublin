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
	BOOL m_bWow64;					// x64上で32bitアプリとして動作している場合にはTRUE
	DWORD m_osver;
	DWORD m_verComCtrl;
	DWORD m_verShell;
	WORD m_sp;
	bool m_bThemeChecked;
	bool m_bThemed;
	bool m_bAlpha;

	COSPlatform()
		: m_bWow64(FALSE)
		, m_osver(_WIN32_WINNT_WINTHRESHOLD)
		, m_sp(0)
		, m_verComCtrl(MAKELONG(0, 6))
		, m_bThemeChecked(false)
		, m_bThemed(false)
		, m_bAlpha(false)
	{
		HMODULE hmod = GetModuleHandle(_T("kernel32"));
		// wow64
		BOOL (WINAPI* pIsWow)(HANDLE, USHORT*, USHORT*);
		(FARPROC&)pIsWow = GetProcAddress(hmod, "IsWow64Process2");
		if (pIsWow) {
			USHORT usProcessMachine, usNativeMachine;
			pIsWow(GetCurrentProcess(), &usProcessMachine, &usNativeMachine);
			m_bWow64 = (usProcessMachine != IMAGE_FILE_MACHINE_UNKNOWN);
		} else {
			IsWow64Process(GetCurrentProcess(), &m_bWow64);
		}

		if (IsWindowsVistaOrGreater())   { m_osver = _WIN32_WINNT_VISTA; m_sp = 0; }		// VISTA or 2008
		if (IsWindowsVistaSP1OrGreater()){ m_osver = _WIN32_WINNT_VISTA; m_sp = 1; }		// VISTA sp1
		if (IsWindowsVistaSP2OrGreater()){ m_osver = _WIN32_WINNT_VISTA; m_sp = 2; }		// VISTA sp2
		if (IsWindows7OrGreater())       { m_osver = _WIN32_WINNT_WIN7; m_sp = 0; }			// 7 or 2008R2
		if (IsWindows7SP1OrGreater())    { m_osver = _WIN32_WINNT_WIN7; m_sp = 1; }			// 7 sp1
		if (IsWindows8OrGreater())       { m_osver = _WIN32_WINNT_WIN8; m_sp = 0; }			// 8 or 2012
		if (IsWindows8Point1OrGreater()) { m_osver = _WIN32_WINNT_WINBLUE; m_sp = 0; }		// 8.1
		if (IsWindows10OrGreater())      { m_osver = _WIN32_WINNT_WINTHRESHOLD; m_sp = 0; }	// 10 2016
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
	__inline bool OsVISTA() { return (m_osver >= _WIN32_WINNT_VISTA); }
	__inline bool Os7() { return (m_osver >= _WIN32_WINNT_WIN7); }
	__inline bool Os8() { return (m_osver >= _WIN32_WINNT_WIN8); }
	__inline bool Os81() { return (m_osver >= _WIN32_WINNT_WINBLUE); }
	__inline bool Os10() { return (m_osver >= _WIN32_WINNT_WINTHRESHOLD); }
	__inline bool IsWindowThemed() { return IsThemed(); }
	__inline bool IsRGBA() { return (IsComm6() && m_bAlpha); }
	__inline DWORD GetComCtrlVersion() { return m_verComCtrl; }

#ifdef _WIN64
	__inline bool IsWin64() { return true; }
#else
	__inline bool IsWin64() { return false; }
#endif
};


#endif			// __OSPLATFORM_H__
