#include "stdafx.h"
#include "lscore.h"
#include "common.h"


extern TCHAR gszSoftware[];
extern TCHAR gszCompany[];
extern "C" WORD WINAPI lccCirRed_us(WORD crc, BYTE* pData, int len);

#define KEY_NO_NOTIFY		((KEY_READ | KEY_WRITE) & ~KEY_NOTIFY)


/////////////////////////////////////////////////////////////////
//

static HKEY GetAppRegistryKey(LPCTSTR pszApp, bool bForRead)
{
	CString str;
	str.Format(_T("Software\\%s\\%s"), gszCompany, pszApp);

	HKEY hAppKey = NULL;
	if (bForRead) {
		if (RegOpenKeyEx(HKEY_CURRENT_USER, str.GetString(), 0, KEY_NO_NOTIFY, &hAppKey) != ERROR_SUCCESS)
			hAppKey = NULL;
	}
	else {
		DWORD dw;
		if (RegCreateKeyEx(HKEY_CURRENT_USER, str.GetString(), 0, REG_NONE, REG_OPTION_NON_VOLATILE, KEY_NO_NOTIFY, NULL, &hAppKey, &dw) != ERROR_SUCCESS)
			hAppKey = NULL;
	}
	return hAppKey;
}

static HKEY GetSectionKey(LPCTSTR pszApp, LPCTSTR pszSection, bool bForRead)
{
	HKEY hSectionKey = NULL;
	HKEY hAppKey = GetAppRegistryKey(pszApp, bForRead);
	if (hAppKey != NULL) {
		if (bForRead) {
			if (RegOpenKeyEx(hAppKey, pszSection, 0, KEY_NO_NOTIFY, &hSectionKey) != ERROR_SUCCESS)
				hSectionKey = NULL;
		}
		else {
			DWORD dw;
			if (RegCreateKeyEx(hAppKey, pszSection, 0, REG_NONE, REG_OPTION_NON_VOLATILE, KEY_NO_NOTIFY, NULL, &hSectionKey, &dw) != ERROR_SUCCESS)
				hSectionKey = NULL;
		}
		RegCloseKey(hAppKey);
	}
	return hSectionKey;
}

extern "C"
int WINAPI lccRegGetBinary(LPCTSTR pApp, LPCTSTR pSec, LPCTSTR pKey, LPBYTE pData, int nLen)
{
	HKEY hKey = GetSectionKey(pApp, pSec, true);
	if (hKey) {
		DWORD dwLen = nLen;
		DWORD dwType = REG_BINARY;
		LONG lRes = RegQueryValueEx(hKey, (LPTSTR)pKey, 0, &dwType, pData, &dwLen);
		RegCloseKey(hKey);
		if (lRes == ERROR_SUCCESS)
			return dwLen;
	}
	return 0;
}

static LPBYTE RegGetBinaryEx(LPCTSTR pApp, LPCTSTR pSec, LPCTSTR pKey, LPINT pnLen)
{
	BYTE* pd = NULL;
	int len = lccRegGetBinary(pApp, pSec, pKey, NULL, 4);
	if (len > 0) {
		pd = (BYTE*)malloc(len);
		lccRegGetBinary(pApp, pSec, pKey, pd, len);
	}
	if (pnLen)
		*pnLen = len;
	return pd;
}

extern "C"
BOOL WINAPI lccRegSetBinary(LPCTSTR pApp, LPCTSTR pSec, LPCTSTR pKey, LPBYTE pData, int nLen)
{
	HKEY hKey = GetSectionKey(pApp, pSec, false);
	if (!hKey) return FALSE;

	RegSetValueEx(hKey, pKey, 0, REG_BINARY, pData, nLen);
	RegCloseKey(hKey);
	return TRUE;
}

extern "C"
int WINAPI lccRegGetString(LPCTSTR pApp, LPCTSTR pSec, LPCTSTR pKey, LPTSTR pStr, int nLen)
{
	*pStr = '\0';
	HKEY hKey = GetSectionKey(pApp, pSec, true);
	if (hKey) {
		DWORD dwLen = nLen;
		DWORD dwType = REG_SZ;
		LONG lRes = RegQueryValueEx(hKey, (LPTSTR)pKey, 0, &dwType, (LPBYTE)pStr, &dwLen);
		RegCloseKey(hKey);
		if (lRes == ERROR_SUCCESS) return dwLen - 1;
	}
	return 0;
}

extern "C"
BOOL WINAPI lccRegSetString(LPCTSTR pApp, LPCTSTR pSec, LPCTSTR pKey, LPCTSTR pStr)
{
	HKEY hKey = GetSectionKey(pApp, pSec, false);
	if (!hKey) return FALSE;

	if (!pStr)	RegDeleteValue(hKey, (LPTSTR)pKey);
	else		RegSetValueEx(hKey, pKey, 0, REG_SZ, (const LPBYTE)pStr, lstrlen(pStr) + 1);
	RegCloseKey(hKey);
	return TRUE;
}

extern "C"
int WINAPI lccRegGetStrJum(LPCTSTR pApp, LPCTSTR pSec, LPCTSTR pKey, char* pStr, int nLen)
{
	*pStr = '\0';
	int len;
	BYTE* pd = RegGetBinaryEx(pApp, pSec, pKey, &len);
	if (pd) {
		if (len > 2) {
			len -= 2;
			WORD crc = *((WORD*)pd);
			BYTE* p = pd + 2;
			BYTE* q = (BYTE*)pStr;
			for (int i = 0; i < len; i++) {
				*q = *p + 0x1E;
				p++;
				q++;
			}
			*q = '\0';
			if (crc != lccCirRed_us(0, (BYTE*)pStr, len))
				*pStr = '\0';
		}
		free(pd);
	}
	return lstrlenA(pStr);
}

extern "C"
BOOL WINAPI lccRegSetStrJum(LPCTSTR pApp, LPCTSTR pSec, LPCTSTR pKey, const char* pStr)
{
	if (!pStr) return lccRegSetString(pApp, pSec, pKey, NULL);

	char buf[MAX_PATH];
	lstrcpyA(buf, pStr);
	register char* p = buf;
	int len = lstrlenA(pStr);
	*((WORD*)p) = lccCirRed_us(0, (BYTE*)pStr, len);
	p += 2;
	while (*pStr) {
		*p = *pStr - 0x1E;
		pStr++;
		p++;
	}
	return lccRegSetBinary(pApp, pSec, pKey, (LPBYTE)buf, len + 2);
}

extern "C"
BOOL WINAPI lccRegGetInt(LPCTSTR pApp, LPCTSTR pSec, LPCTSTR pKey, LPINT pData)
{
	HKEY hKey = GetSectionKey(pApp, pSec, true);
	if (hKey) {
		DWORD dwLen = sizeof(DWORD);
		DWORD dwType = REG_DWORD;
		LONG lRes = RegQueryValueEx(hKey, (LPTSTR)pKey, 0, &dwType, (LPBYTE)pData, &dwLen);
		RegCloseKey(hKey);
		if (lRes == ERROR_SUCCESS) return TRUE;
	}
	return FALSE;
}

extern "C"
BOOL WINAPI lccRegSetInt(LPCTSTR pApp, LPCTSTR pSec, LPCTSTR pKey, int nData)
{
	HKEY hKey = GetSectionKey(pApp, pSec, false);
	if (!hKey) return FALSE;

	RegSetValueEx(hKey, pKey, 0, REG_DWORD, (const LPBYTE)&nData, sizeof(DWORD));
	RegCloseKey(hKey);
	return TRUE;
}

extern "C"
BOOL WINAPI lccRegDeleteKeyAll(HKEY hRootKey, LPCTSTR pszSubKey)
{
	HKEY hSubKey;
	if (RegOpenKeyEx(hRootKey, pszSubKey, 0, KEY_ALL_ACCESS, &hSubKey) != ERROR_SUCCESS) return FALSE;

	LPTSTR array[256];

	// キーを列挙して、そのキーを削除
	FILETIME ft;
	TCHAR name[MAX_PATH];
	DWORD dw;
	DWORD idw = 0;
	for (;;) {
		dw = MAX_PATH;
		if (RegEnumKeyEx(hSubKey, idw, name, &dw, 0, 0, 0, &ft) != ERROR_SUCCESS)
			break;
		array[idw++] = StrDup(name);
	}

	int nError = 0;
	DWORD i;
	for (i = 0; i < idw; i++) {
		// サブキーを削除
		if (!lccRegDeleteKeyAll(hSubKey, array[i]))
			nError++;
		LocalFree(array[i]);
	}

	if (nError > 0) {
		RegCloseKey(hSubKey);
		return FALSE;
	}

	// 現在のキーにある値を列挙
	idw = 0;
	for (;;) {
		dw = MAX_PATH;
		if (RegEnumValue(hSubKey, idw, name, &dw, 0, 0, 0, 0) != ERROR_SUCCESS)
			break;
		array[idw++] = StrDup(name);
	}

	nError = 0;
	for (i = 0; i < idw; i++) {
		// 値を削除
		if (RegDeleteValue(hSubKey, array[i]) != ERROR_SUCCESS)
			nError++;
		LocalFree(array[i]);
	}
	RegCloseKey(hSubKey);

	if (nError > 0)
		return FALSE;

	return (RegDeleteKey(hRootKey, pszSubKey) == ERROR_SUCCESS) ? TRUE : FALSE;
}

extern "C"
BOOL WINAPI lccRegDeleteValueAll(HKEY hRootKey, LPCTSTR pszSubKey)
{
	BOOL bOK = FALSE;
	HKEY hSubKey;
	if (RegOpenKeyEx(hRootKey, pszSubKey, 0, KEY_ALL_ACCESS, &hSubKey) == ERROR_SUCCESS) {
		// 現在のキーにある値を列挙
		LPTSTR array[256];
		DWORD idw = 0;
		for (;;) {
			TCHAR name[MAX_PATH];
			DWORD dw = MAX_PATH;
			if (RegEnumValue(hSubKey, idw, name, &dw, 0, 0, 0, 0) != ERROR_SUCCESS)
				break;
			array[idw++] = StrDup(name);
		}

		int nError = 0;
		for (DWORD i = 0; i < idw; i++) {
			// 値を削除
			if (RegDeleteValue(hSubKey, array[i]) != ERROR_SUCCESS)
				nError++;
			LocalFree(array[i]);
		}
		if (!nError)
			bOK = TRUE;
		RegCloseKey(hSubKey);
	}
	return bOK;
}

extern "C"
BOOL WINAPI lccRegDeleteValue(HKEY hRootKey, LPCTSTR pszSubKey, LPCTSTR pszValue)
{
	BOOL bOK = FALSE;
	HKEY hSubKey;
	if (RegOpenKeyEx(hRootKey, pszSubKey, 0, KEY_ALL_ACCESS, &hSubKey) == ERROR_SUCCESS) {
		if (RegDeleteValue(hSubKey, pszValue) == ERROR_SUCCESS)
			bOK = TRUE;
		RegCloseKey(hSubKey);
	}
	return bOK;
}

extern "C"
int WINAPI lccRegGetEnumKey(LPCTSTR pApp, LPCTSTR pSec, LPTSTR pKey, DWORD dwRegType, int nLen)
{
	HKEY hKey = GetSectionKey(pApp, pSec, true);
	if (!hKey) return 0;

	*pKey = '\0';

	DWORD dwLen, dwType;
	TCHAR buf[MAX_PATH];
	LPTSTR p = pKey;
	for (DWORD dw = 0; ; dw++) {
		dwLen = MAX_PATH;
		dwType = dwRegType;
		if (RegEnumValue(hKey, dw, buf, &dwLen, NULL, &dwType, NULL, NULL) == ERROR_NO_MORE_ITEMS)
			break;

		if (lstrlen(pKey) + lstrlen(buf) >= nLen)
			break;

		lstrcpy(p, buf);
		p += lstrlen(buf) + 1;
	}
	*p = '\0';

	RegCloseKey(hKey);
	return lstrlen(pKey);
}

////////////////////////////////////////////////////////////////////////
//

extern "C"
BOOL WINAPI lccRegCheckShellVerb(LPCTSTR pszFile)
{
	BOOL bOK = FALSE;
	if (pszFile && *pszFile) {
		LPTSTR p = PathFindExtension(pszFile);
		TCHAR exe[MAX_PATH * 2];
		DWORD cch = ARRAYSIZE(exe);
		if (*p == '.' && AssocQueryString(0, ASSOCSTR_EXECUTABLE, p, NULL, exe, &cch) == S_OK) {
			bOK = TRUE;
		}
	}
	return bOK;
}

extern "C"
BOOL WINAPI lccRegGetPerceivedType(LPCTSTR pszFile, LPTSTR typeStr, int cchMax)
{
	BOOL bOK = FALSE;
	LPTSTR p = PathFindExtension(pszFile);
	if (*p == _T('.')) {
		CRegKey reg;
		if (reg.Open(HKEY_CLASSES_ROOT, p, KEY_READ) == ERROR_SUCCESS) {
			ULONG len = cchMax;
			bOK = (reg.QueryStringValue(_T("PerceivedType"), typeStr, &len) == ERROR_SUCCESS);
		}
	}
	return bOK;
}

static TCHAR sszLPtit[] = _T("Common Library path not found.");

extern "C"
BOOL WINAPI lccRegGetLibraryPath(LPTSTR pszPath)
{
	*pszPath = '\0';

	TCHAR buf[MAX_PATH * 2];
	if (SHGetFolderPath(NULL, CSIDL_PROGRAM_FILES_COMMON, NULL, SHGFP_TYPE_CURRENT, buf) == S_OK) {
		PathAppend(buf, gszCompany);
		if (_pathisdirectory(buf)) {
			lstrcpy(pszPath, buf);
			return TRUE;
		}
		else
			lccErrorBox(GetFocus(), ERROR_DIRECTORY, sszLPtit, buf, MB_OK | MB_ICONSTOP);
	}
	else
		lccErrorBox(GetFocus(), ERROR_CANTREAD, sszLPtit, NULL, MB_OK | MB_ICONSTOP);
	return FALSE;
}

static TCHAR sszMsAppPath[] = _T("SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\App Paths");

extern "C"
BOOL WINAPI lccRegGetAppPath(LPCTSTR pszAppName, LPTSTR pszAppPath, int cchMax)
{
	if (!pszAppName || *pszAppName == 0) return FALSE;

	TCHAR buf[MAX_PATH * 2];
	lstrcpy(buf, sszMsAppPath);
	PathAppend(buf, pszAppName);

	BOOL bOK = FALSE;
	HKEY hkey;
	if (RegOpenKeyEx(HKEY_LOCAL_MACHINE, buf, 0, KEY_READ, &hkey) == ERROR_SUCCESS) {
		DWORD type;
		DWORD len = sizeof(buf);
		if (RegQueryValueEx(hkey, NULL, NULL, &type, (BYTE*)buf, &len) == ERROR_SUCCESS) {
			if (type == REG_EXPAND_SZ) {
				TCHAR tmp[MAX_PATH * 2];
				ExpandEnvironmentStrings(buf, tmp, ARRAYSIZE(tmp));
				lstrcpy(buf, tmp);
			}
			if (_pathfileexists(buf)) {
				if (pszAppPath && cchMax > 0)
					lstrcpyn(pszAppPath, buf, cchMax);
				bOK = TRUE;
			}
		}
		RegCloseKey(hkey);
	}
	return bOK;
}
