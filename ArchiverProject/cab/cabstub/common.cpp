// common.cpp

#include "stdafx.h"
#include "cabstub.h"


/////////////////////////////////////////////////////////////////////////////
//

int ReplaceString(LPWSTR pszTarget, LPCWSTR pszOld, LPCWSTR pszNew)
{
	if (!pszTarget || *pszTarget == 0 ||
		!pszOld || *pszOld == 0 ||
		!pszNew || *pszNew == 0)
		return 0;

	if (!wcscmp(pszOld, pszNew))
		return 0;

	int numReplace = 0;
	size_t olen = wcslen(pszOld);
	size_t nlen = wcslen(pszNew);

	WCHAR* tmp = new WCHAR[wcslen(pszTarget) + 1];
	WCHAR* p = pszTarget;
	while (p && *p) {
		WCHAR* q = wcsstr(p, pszOld);
		if (!q) break;

		wcscpy(tmp, q + olen);
		wcscpy(q, pszNew);
		p = q + nlen;
		numReplace++;
	}
	return numReplace;
}

int ReplaceChar(LPWSTR pszTarget, const WCHAR chOld, const WCHAR chNew)
{
	if (!pszTarget || *pszTarget == 0)
		return 0;

	if (chOld == chNew)
		return 0;

	int numReplace = 0;
	WCHAR* p = pszTarget;
	while (p && *p) {
		p = wcschr(p, chOld);
		if (!p) break;

		*p++ = chNew;
		numReplace++;
	}
	return numReplace;
}

int AppMessage(UINT idResource, LPCWSTR pszInsert /*=NULL*/, UINT nType /*=MB_STOP*/)
{
	WCHAR* mw = new WCHAR[MAX_PATH * 2];
	WCHAR* cw = new WCHAR[MAX_PATH * 2];

	LoadStringW(ghInst, idResource, mw, MAX_PATH * 2);
	if (pszInsert && *pszInsert)
		ReplaceString(mw, L"%1", pszInsert);
	LoadStringW(ghInst, IDR_APP, cw, MAX_PATH * 2);
	int res = MessageBoxW(::GetActiveWindow(), mw, cw, nType);

	delete [] mw;
	delete [] cw;
	return res;
}

LPWSTR alloc_wcscpy(LPWSTR* ppszDst, LPCWSTR pszSrc)
{
	if (*ppszDst) {
		delete [] *ppszDst;
		*ppszDst = NULL;
	}
	if (pszSrc && *pszSrc) {
		*ppszDst = new WCHAR[wcslen(pszSrc) + 1];
		wcscpy(*ppszDst, pszSrc);
	}
	return *ppszDst;
}

BOOL xCreateProcess(LPWSTR pszCommandLine, LPCWSTR pszCurDir, PROCESS_INFORMATION* ppiProcInfo)
{
	STARTUPINFOW siw = { 0 };
	siw.cb = sizeof(siw);
	return CreateProcessW(NULL, pszCommandLine, NULL, NULL, FALSE,
							CREATE_DEFAULT_ERROR_MODE |
							NORMAL_PRIORITY_CLASS |
							CREATE_NEW_PROCESS_GROUP,
							NULL, pszCurDir, &siw, ppiProcInfo);
}

static void (WINAPI* _rSHChangeNotify)(LONG, UINT, LPCVOID, LPCVOID) = 0;

void xPathChangeNotify(LONG idEvent, LPCWSTR psz)
{
	if (!_rSHChangeNotify) {
		HMODULE hmod = GetModuleHandleW(L"shell32");
		if (hmod)
			(FARPROC&)_rSHChangeNotify = GetProcAddress(hmod, "SHChangeNotify");
	}

	if (_rSHChangeNotify)
		_rSHChangeNotify(idEvent, SHCNF_PATHW, psz, 0);
}

BOOL xPathMakeTree(LPCWSTR pszPath)
{
	if (xPathIsRoot(pszPath) || wcslen(pszPath) < 3)
		return TRUE;

	WCHAR wsz[MAX_PATH];
	wcscpy(wsz, pszPath);
	WCHAR* p = wsz;
	if (_ISNETPATHW(p)) {
		p += 2;
		p = xStrChrW(++p, L'\\');	// コンピュータ名をスキップ
		p = xStrChrW(++p, L'\\');	// 共有名をスキップ
	}
	while (p && *p) {
		WCHAR* q = xStrChrW(p, L'\\');
		if (q)
			*q = 0;

		if (wcslen(wsz) >= 3 && !xPathIsRoot(wsz)) {
			if (!xPathIsDirectory(wsz)) {
				if (!CreateDirectoryW(wsz, NULL))
					return FALSE;
				xPathChangeNotify(SHCNE_MKDIR, wsz);
			}
		}
		if (q)
			*q++ = L'\\';
		p = q;
	}
	return TRUE;
}

BOOL xPathGetSpecialFilesDir(LPCWSTR pszName, LPWSTR pszDir, int cchMax)
{
	HKEY hkey;
	DWORD len, type;
	LONG res;
#ifdef _WIN64
	RegOpenKeyExW(HKEY_LOCAL_MACHINE, L"SOFTWARE\\Microsoft\\Windows\\CurrentVersion", 0, KEY_READ, &hkey);

	len = cchMax;
	type = REG_SZ;
	res = RegQueryValueExW(hkey, pszName, 0, &type, (BYTE*)pszDir, &len);
	RegCloseKey(hkey);

	if (res == ERROR_SUCCESS && xPathIsDirectory(pszDir))
		return TRUE;
	return FALSE;
#else
	RegOpenKeyExA(HKEY_LOCAL_MACHINE, "SOFTWARE\\Microsoft\\Windows\\CurrentVersion", 0, KEY_READ, &hkey);

	char dir[MAX_PATH], name[MAX_PATH];
	WideCharToMultiByte(CP_ACP, 0, pszName, -1, name, ARRAYSIZE(name), NULL, NULL);
	len = ARRAYSIZE(dir);
	type = REG_SZ;
	res = RegQueryValueExA(hkey, name, 0, &type, (BYTE*)dir, &len);
	RegCloseKey(hkey);

	if (res == ERROR_SUCCESS) {
		MultiByteToWideChar(CP_ACP, 0, dir, -1, pszDir, cchMax);
		if (xPathIsDirectory(pszDir))
			return TRUE;
	}
	return FALSE;
#endif
}

LPWSTR xStrChrW(LPCWSTR psz, WCHAR ch)
{
	while (psz && *psz) {
		if (*psz == ch)
			return (LPWSTR)psz;
		psz++;
	}
	return NULL;
}

LPSTR xStrChrA(LPCSTR psz, char ch)
{
	while (psz && *psz) {
		if (*psz == ch)
			return (LPSTR)psz;
		psz = CharNextA(psz);
	}
	return NULL;
}

LPWSTR xStrRChrW(LPCWSTR psz, WCHAR ch)
{
	LPCWSTR pos = NULL;
	while (psz && *psz) {
		if (*psz == ch)
			pos = psz;
		psz++;
	}
	return (LPWSTR)pos;
}

LPSTR xStrRChrA(LPCSTR psz, char ch)
{
	LPCSTR pos = NULL;
	while (psz && *psz) {
		if (*psz == ch)
			pos = psz;
		psz = CharNextA(psz);
	}
	return (LPSTR)pos;
}

int xStrCmpNW(LPCWSTR pszStr, LPCWSTR pszComp, int iLen)
{
	int iRet = CompareStringW(GetThreadLocale(), 0, pszStr, iLen, pszComp, iLen);
	return iRet == CSTR_LESS_THAN ? -1 : iRet == CSTR_GREATER_THAN ? 1 : 0;
}

int xStrCmpNA(LPCSTR pszStr, LPCSTR pszComp, int iLen)
{
	int iRet = CompareStringA(GetThreadLocale(), 0, pszStr, iLen, pszComp, iLen);
	return iRet == CSTR_LESS_THAN ? -1 : iRet == CSTR_GREATER_THAN ? 1 : 0;
}

int xStrCmpNIW(LPCWSTR pszStr, LPCWSTR pszComp, int iLen)
{
	int iRet = CompareStringW(GetThreadLocale(), NORM_IGNORECASE, pszStr, iLen, pszComp, iLen);
	return iRet == CSTR_LESS_THAN ? -1 : iRet == CSTR_GREATER_THAN ? 1 : 0;
}

int xStrCmpNIA(LPCSTR pszStr, LPCSTR pszComp, int iLen)
{
	int iRet = CompareStringA(GetThreadLocale(), NORM_IGNORECASE, pszStr, iLen, pszComp, iLen);
	return iRet == CSTR_LESS_THAN ? -1 : iRet == CSTR_GREATER_THAN ? 1 : 0;
}

LPWSTR xPathFindFileNameW(LPCWSTR psz)
{
	LPWSTR p = xStrRChrW(psz, L'\\');
	if (p)
		return (p + 1);
	return (LPWSTR)psz;
}

LPSTR xPathFindFileNameA(LPCSTR psz)
{
	LPSTR p = xStrRChrA(psz, '\\');
	if (p)
		return (p + 1);
	return (LPSTR)psz;
}

BOOL xPathRemoveFileSpecW(LPCWSTR psz)
{
	LPWSTR p = xStrRChrW(psz, L'\\');
	if (!p) 
		return FALSE;
	*p = 0;
	return TRUE;
}

BOOL xPathRemoveFileSpecA(LPCSTR psz)
{
	LPSTR p = xStrRChrA(psz, '\\');
	if (!p) 
		return FALSE;
	*p = 0;
	return TRUE;
}

LPWSTR xPathFindExtensionW(LPCWSTR psz)
{
	return xStrRChrW(xPathFindFileNameW(psz), L'.');
}

LPSTR xPathFindExtensionA(LPCSTR psz)
{
	return xStrRChrA(xPathFindFileNameA(psz), '.');
}

void xPathRemoveExtensionW(LPCWSTR psz)
{
	LPWSTR p = xPathFindExtensionW(psz);
	if (p)
		*p = 0;
}

void xPathRemoveExtensionA(LPCSTR psz)
{
	LPSTR p = xPathFindExtensionA(psz);
	if (p)
		*p = 0;
}

BOOL xPathRenameExtensionW(LPWSTR psz, LPCWSTR ext)
{
	xPathRemoveExtensionW(psz);
	if (ext && *ext == L'.') {
		wcscat(psz, ext);
	}
	return TRUE;
}

BOOL xPathRenameExtensionA(LPSTR psz, LPCSTR ext)
{
	xPathRemoveExtensionA(psz);
	if (ext && *ext == '.') {
		strcat(psz, ext);
	}
	return TRUE;
}

BOOL xPathGetParentW(LPCWSTR psz, LPWSTR parent)
{
	wcscpy(parent, psz);
	BOOL bOK = xPathRemoveFileSpecW(parent);
	if (bOK) {
		if (wcslen(parent) == 2 && parent[1] == L':') {
			parent[2] = L'\\';
			parent[3] = 0;
		}
	}
	return bOK;
}

BOOL xPathGetParentA(LPCSTR psz, LPSTR parent)
{
	strcpy(parent, psz);
	BOOL bOK = xPathRemoveFileSpecA(parent);
	if (bOK) {
		if (strlen(parent) == 2 && parent[1] == ':') {
			parent[2] = '\\';
			parent[3] = 0;
		}
	}
	return bOK;
}

LPWSTR xPathAddBackslashW(LPWSTR psz)
{
	if (psz) {
		size_t len = wcslen(psz);
		if (len) {
			psz += len;
			if (*(psz - 1) != L'\\') {
				*psz++ = L'\\';
				*psz = 0;
			}
		}
	}
	return psz;
}

LPSTR xPathAddBackslashA(LPSTR psz)
{
	if (psz) {
		size_t len = strlen(psz);
		if (len) {
			LPSTR p = CharPrevA(psz, psz + len);
			psz = CharNextA(p);
			if (*p != '\\') {
				*psz++ = '\\';
				*psz = 0;
			}
		}
	}
	return psz;
}

LPWSTR xPathRemoveBackslash(LPWSTR psz)
{
	LPWSTR p = NULL;
	if (psz) {
		size_t len = wcslen(psz);
		if (len) {
			psz += len;
			if (*(psz - 1) == L'\\')
				*(psz - 1) = 0;
		}
	}
	return p;
}

LPWSTR xPathCombineW(LPWSTR pszDst, LPCWSTR pszDir, LPCWSTR pszFile)
{
	if (pszDir && *pszDir) {
		wcscpy(pszDst, pszDir);
		xPathAddBackslashW(pszDst);
	}
	else
		*pszDst = 0;
	return wcscat(pszDst, pszFile);
}

LPSTR xPathCombineA(LPSTR pszDst, LPCSTR pszDir, LPCSTR pszFile)
{
	if (pszDir && *pszDir) {
		strcpy(pszDst, pszDir);
		xPathAddBackslashA(pszDst);
	}
	else
		*pszDst = 0;
	return strcat(pszDst, pszFile);
}

LPWSTR xPathAppendW(LPWSTR pszPath, LPCWSTR pszMore)
{
	xPathAddBackslashW(pszPath);
	return wcscat(pszPath, pszMore);
}

LPSTR xPathAppendA(LPSTR pszPath, LPCSTR pszMore)
{
	xPathAddBackslashA(pszPath);
	return strcat(pszPath, pszMore);
}

void xPathQuoteSpacesW(LPWSTR psz)
{
	if (psz && xStrChrW(psz, L' ')) {
		size_t len = wcslen(psz) + 1;
		memmove(psz + 1, psz, len * sizeof(WCHAR));
		psz[0] = L'"';
		psz[len] = L'"';
		psz[len + 1] = 0;
	}
}

void xPathQuoteSpacesA(LPSTR psz)
{
	if (psz && xStrChrA(psz, ' ')) {
		size_t len = strlen(psz) + 1;
		memmove(psz + 1, psz, len * sizeof(char));
		psz[0] = '"';
		psz[len] = '"';
		psz[len + 1] = 0;
	}
}

void xPathUnquoteSpacesW(LPWSTR psz)
{
	if (psz && *psz == L'"') {
		size_t len = wcslen(psz) - 1;
		if (psz[len] == L'"') {
			psz[len] = 0;
			memmove(psz, psz + 1, len * sizeof(WCHAR));
		}
	}
}

void xPathUnquoteSpacesA(LPSTR psz)
{
	if (psz && *psz == '"') {
		size_t len = strlen(psz) - 1;
		if (psz[len] == '"') {
			psz[len] = 0;
			memmove(psz, psz + 1, len * sizeof(char));
		}
	}
}

BOOL xPathIsRoot(LPCWSTR pszPath)
{
	if (pszPath && *pszPath) {
		if (*pszPath == L'\\') {
			if (!pszPath[1])
				return TRUE; /* \ */
			else if (pszPath[1] == L'\\') {
				BOOL bSeenSlash = FALSE;
				pszPath += 2;

				/* Check for UNC root path */
				while (*pszPath) {
					if (*pszPath == L'\\') {
						if (bSeenSlash)
							return FALSE;
						bSeenSlash = TRUE;
					}
					pszPath++;
				}
				return TRUE;
			}
		}
		else if (pszPath[1] == L':' && pszPath[2] == L'\\' && pszPath[3] == 0)
			return TRUE; /* X:\ */
	}
	return FALSE;
}

static DWORD FileAttr(LPCWSTR psz)
{
	UINT prevmode = SetErrorMode(SEM_FAILCRITICALERRORS);
	DWORD attr = GetFileAttributesW(psz);
	SetErrorMode(prevmode);
	return attr;
}

BOOL xPathIsDirectory(LPCWSTR psz)
{
	if (!psz || xPathIsUNCServer(psz) || xPathIsUNCServerShare(psz))
		return FALSE;

	if (xPathIsRoot(psz))
		return TRUE;

	WCHAR buf[MAX_PATH * 2];
	wcscpy(buf, psz);
	xPathRemoveBackslash(buf);

	DWORD attr = FileAttr(buf);
	return ((attr != INVALID_FILE_ATTRIBUTES) && (attr & FILE_ATTRIBUTE_DIRECTORY)) ? TRUE : FALSE;
}

BOOL xPathFileExists(LPCWSTR psz)
{
	return (FileAttr(psz) != INVALID_FILE_ATTRIBUTES) ? TRUE : FALSE;
}

BOOL xPathIsUNC(LPCWSTR psz)
{
	if (psz && (psz[0] == L'\\') && (psz[1] == L'\\'))
		return TRUE;
	return FALSE;
}

BOOL xPathIsUNCServer(LPCWSTR psz)
{
	if (psz && *psz++ == L'\\' && *psz++ == L'\\') {
		while (*psz) {
			if (*psz == L'\\')
				return FALSE;
			psz++;
		}
		return TRUE;
	}
	return FALSE;
}

BOOL xPathIsUNCServerShare(LPCWSTR psz)
{
	if (psz && *psz++ == L'\\' && *psz++ == L'\\') {
		BOOL bSeenSlash = FALSE;
		while (*psz) {
			if (*psz == L'\\') {
				if (bSeenSlash)
					return FALSE;
				bSeenSlash = TRUE;
			}
			psz++;
		}
		return bSeenSlash;
	}
	return FALSE;
}

int xPathGetDriveNumber(LPCWSTR psz)
{
	if (psz && psz[1] == L':') {
		WCHAR ch = *psz;
		if (L'a' <= ch && ch <= L'z')
			ch -= 0x20;
		if (ch >= L'A' && ch <= L'Z')
			return (ch - L'A');
	}
	return -1;
}

BOOL CenterWindow(HWND hWnd, HWND hWndCenter)
{
	// determine owner window to center against
	DWORD dwStyle = ::GetWindowLong(hWnd, GWL_STYLE);
	if (hWndCenter == NULL) {
		if (dwStyle & WS_CHILD)
			hWndCenter = ::GetParent(hWnd);
		else
			hWndCenter = ::GetWindow(hWnd, GW_OWNER);
	}

	// get coordinates of the window relative to its parent
	RECT rcDlg;
	::GetWindowRect(hWnd, &rcDlg);
	RECT rcArea;
	RECT rcCenter;
	HWND hWndParent;
	if (!(dwStyle & WS_CHILD)) {
		// don't center against invisible or minimized windows
		if (hWndCenter != NULL) {
			DWORD dwStyleCenter = ::GetWindowLong(hWndCenter, GWL_STYLE);
			if (!(dwStyleCenter & WS_VISIBLE) || (dwStyleCenter & WS_MINIMIZE))
				hWndCenter = NULL;
		}

		// center within screen coordinates
		::SystemParametersInfo(SPI_GETWORKAREA, NULL, &rcArea, NULL);
		if (hWndCenter == NULL)
			rcCenter = rcArea;
		else
			::GetWindowRect(hWndCenter, &rcCenter);
	}
	else {
		// center within parent client coordinates
		hWndParent = ::GetParent(hWnd);

		::GetClientRect(hWndParent, &rcArea);
		::GetClientRect(hWndCenter, &rcCenter);
		::MapWindowPoints(hWndCenter, hWndParent, (POINT*)&rcCenter, 2);
	}

	int DlgWidth = rcDlg.right - rcDlg.left;
	int DlgHeight = rcDlg.bottom - rcDlg.top;

	// find dialog's upper left based on rcCenter
	int xLeft = (rcCenter.left + rcCenter.right) / 2 - DlgWidth / 2;
	int yTop = (rcCenter.top + rcCenter.bottom) / 2 - DlgHeight / 2;

	// if the dialog is outside the screen, move it inside
	if (xLeft < rcArea.left)
		xLeft = rcArea.left;
	else if (xLeft + DlgWidth > rcArea.right)
		xLeft = rcArea.right - DlgWidth;

	if (yTop < rcArea.top)
		yTop = rcArea.top;
	else if (yTop + DlgHeight > rcArea.bottom)
		yTop = rcArea.bottom - DlgHeight;

	// map screen coordinates to child coordinates
	return ::SetWindowPos(hWnd, NULL, xLeft, yTop, -1, -1, SWP_NOSIZE | SWP_NOZORDER | SWP_NOACTIVATE);
}
