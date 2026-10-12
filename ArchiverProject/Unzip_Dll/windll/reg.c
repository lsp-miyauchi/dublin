#include <windows.h>
#include <malloc.h>

static char szSoftware[] = "Software";
static char szCompany[] = "ArchiverDll";
static char szApp[] = "LspUzp";

/////////////////////////////////////////////////////////////////
//

static HKEY getappkey()
{
	HKEY hAppKey = NULL;
	HKEY hSoftKey = NULL;
	HKEY hCompanyKey = NULL;
	if (RegOpenKeyEx(HKEY_CURRENT_USER, szSoftware, 0, KEY_WRITE|KEY_READ,
		&hSoftKey) == ERROR_SUCCESS)
	{
		DWORD dw;
		if (RegCreateKeyEx(hSoftKey, szCompany, 0, REG_NONE,
			REG_OPTION_NON_VOLATILE, KEY_WRITE|KEY_READ, NULL,
			&hCompanyKey, &dw) == ERROR_SUCCESS)
		{
			RegCreateKeyEx(hCompanyKey, szApp, 0, REG_NONE,
				REG_OPTION_NON_VOLATILE, KEY_WRITE|KEY_READ, NULL,
				&hAppKey, &dw);
		}
	}
	if (hSoftKey != NULL)
		RegCloseKey(hSoftKey);
	if (hCompanyKey != NULL)
		RegCloseKey(hCompanyKey);

	return hAppKey;
}

static HKEY getseckey(LPCTSTR lpszSection)
{
	DWORD dw;
	HKEY hSectionKey = NULL;
	HKEY hAppKey = getappkey(szApp);
	if (hAppKey == NULL)
		return NULL;

	RegCreateKeyEx(hAppKey, lpszSection, 0, REG_NONE,
		REG_OPTION_NON_VOLATILE, KEY_WRITE|KEY_READ, NULL,
		&hSectionKey, &dw);
	RegCloseKey(hAppKey);
	return hSectionKey;
}

int rgGetBinary(LPCSTR pSec, LPCSTR pKey, LPBYTE pData, int nLen)
{
	HKEY hKey = getseckey(pSec);
	if (hKey)
	{
		DWORD dwLen = nLen;
		DWORD dwType = REG_BINARY;
		LONG lRes = RegQueryValueEx(hKey,
									(char*)pKey,
									0,
									&dwType,
									pData,
									&dwLen);
		RegCloseKey(hKey);
		if (lRes == ERROR_SUCCESS)
			return dwLen;
	}
	return 0;
}

LPBYTE rgGetBinaryEx(LPCSTR pSec, LPCSTR pKey, LPINT pnLen)
{
	LPBYTE pd = NULL;
	int len = rgGetBinary(pSec, pKey, NULL, 4);
	if (len > 0)
	{
		pd = (LPBYTE)malloc(len);
		rgGetBinary(pSec, pKey, pd, len);
	}
	if (pnLen)
		*pnLen = len;
	return pd;
}

BOOL rgSetBinary(LPCSTR pSec, LPCSTR pKey, LPBYTE pData, int nLen)
{
	HKEY hKey = getseckey(pSec);
	if (!hKey)
		return FALSE;

	RegSetValueEx(hKey, pKey, 0, REG_BINARY, pData, nLen);
	RegCloseKey(hKey);
	return TRUE;
}

int rgGetString(LPCSTR pSec, LPCSTR pKey, LPSTR pStr, int nLen)
{
	HKEY hKey;
	*pStr = '\0';
	hKey = getseckey(pSec);
	if (hKey)
	{
		DWORD dwLen = nLen;
		DWORD dwType = REG_SZ;
		LONG lRes = RegQueryValueEx(hKey,
									(char*)pKey,
									0,
									&dwType,
									(LPBYTE)pStr,
									&dwLen);
		RegCloseKey(hKey);
		if (lRes == ERROR_SUCCESS)
			return dwLen - 1;
	}
	return 0;
}

BOOL rgSetString(LPCSTR pSec, LPCSTR pKey, LPCSTR pStr)
{
	HKEY hKey = getseckey(pSec);
	if (!hKey)
		return FALSE;

	if (!pStr)
		RegDeleteValue(hKey, (char*)pKey);
	else
		RegSetValueEx(hKey, pKey, 0, REG_SZ, (const LPBYTE)pStr, lstrlen(pStr) + 1);
	RegCloseKey(hKey);
	return TRUE;
}

BOOL rgGetValue(LPCSTR pSec, LPCSTR pKey, LPINT pData)
{
	HKEY hKey = getseckey(pSec);
	if (hKey)
	{
		DWORD dwLen = sizeof(DWORD);
		DWORD dwType = REG_DWORD;
		LONG lRes = RegQueryValueEx(hKey,
									(char*)pKey,
									0,
									&dwType,
									(LPBYTE)pData,
									&dwLen);
		RegCloseKey(hKey);
		if (lRes == ERROR_SUCCESS)
			return TRUE;
	}
	return FALSE;
}

BOOL rgSetValue(LPCSTR pSec, LPCSTR pKey, int nData)
{
	HKEY hKey = getseckey(pSec);
	if (!hKey)
		return FALSE;

	RegSetValueEx(hKey, pKey, 0, REG_DWORD, (const LPBYTE)&nData, sizeof(DWORD));
	RegCloseKey(hKey);
	return TRUE;
}
