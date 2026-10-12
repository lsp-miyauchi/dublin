#include "stdafx.h"
#include "cab.h"

static WCHAR sszSoftware[] = L"Software";
static WCHAR sszCompany[]  = L"ArchiverDll";
static WCHAR sszAppSec[]   = L"Cab32";

HKEY GetAppRegistryKey()
{
	HKEY hSoftKey = NULL;
	HKEY hCompanyKey = NULL;
	if (RegOpenKeyEx(HKEY_CURRENT_USER, sszSoftware, 0, KEY_WRITE|KEY_READ, &hSoftKey) == ERROR_SUCCESS) {
		DWORD dw;
		RegCreateKeyEx(hSoftKey, sszCompany, 0, REG_NONE,
			REG_OPTION_NON_VOLATILE, KEY_WRITE|KEY_READ, NULL,
			&hCompanyKey, &dw);
	}
	if (hSoftKey != NULL)
		RegCloseKey(hSoftKey);
	return hCompanyKey;
}

HKEY GetSectionKey(LPCWSTR lpszSection)
{
	HKEY hSectionKey = NULL;
	HKEY hAppKey = GetAppRegistryKey();
	if (hAppKey == NULL)
		return NULL;

	if (lpszSection == NULL)
		lpszSection = sszAppSec;

	DWORD dw;
	RegCreateKeyEx(hAppKey, lpszSection, 0, REG_NONE,
		REG_OPTION_NON_VOLATILE, KEY_WRITE|KEY_READ, NULL,
		&hSectionKey, &dw);
	RegCloseKey(hAppKey);
	return hSectionKey;
}

int GetRegBinary(LPCWSTR pSec, LPCWSTR pKey, BYTE* lpData, int nLen)
{
	HKEY hKey = GetSectionKey(pSec);
	if (hKey) {
		DWORD dwLen = nLen;
		DWORD dwType = REG_BINARY;
		LONG lRes = RegQueryValueEx(hKey,
									(WCHAR*)pKey,
									0,
									&dwType,
									lpData,
									&dwLen);
		RegCloseKey(hKey);
		if (lRes == ERROR_SUCCESS)
			return dwLen;
	}
	return 0;
}

LPBYTE GetRegBinaryEx(LPCWSTR pSec, LPCWSTR pKey, LPINT lpnLen)
{
	BYTE* pd = NULL;
	int len = GetRegBinary(pSec, pKey, NULL, 4);
	if (len > 0) {
		pd = (BYTE*)malloc(len);
		GetRegBinary(pSec, pKey, pd, len);
	}
	if (lpnLen)
		*lpnLen = len;
	return pd;
}

BOOL SetRegBinary(LPCWSTR pSec, LPCWSTR pKey, BYTE* lpData, int nLen)
{
	HKEY hKey = GetSectionKey(pSec);
	if (!hKey)
		return FALSE;

	RegSetValueEx(hKey, pKey, 0, REG_BINARY, lpData, nLen);
	RegCloseKey(hKey);
	return TRUE;
}

int GetRegString(LPCWSTR pSec, LPCWSTR pKey, LPWSTR pStr, int cchMax)
{
	int res = 0;
	*pStr = 0;
	HKEY hKey = GetSectionKey(pSec);
	if (hKey) {
		DWORD dwLen = cchMax * sizeof(WCHAR);
		DWORD dwType = REG_SZ;
		LONG lRes = RegQueryValueEx(hKey, (WCHAR*)pKey, 0, &dwType, (BYTE*)pStr, &dwLen);
		RegCloseKey(hKey);
		if (lRes == ERROR_SUCCESS) {
			res = (dwLen / sizeof(WCHAR)) - 1;
		}
	}
	return res;
}

BOOL SetRegString(LPCWSTR pSec, LPCWSTR pKey, LPCWSTR pStr)
{
	HKEY hKey = GetSectionKey(pSec);
	if (!hKey)
		return FALSE;

	int len = (int)(wcslen(pStr) + 1) * sizeof(WCHAR);
	RegSetValueEx(hKey, pKey, 0, REG_SZ, (const BYTE*)pStr, len);
	RegCloseKey(hKey);
	return TRUE;
}

int GetRegInt(LPCWSTR pSec, LPCWSTR pKey, int nDefValue)
{
	HKEY hKey = GetSectionKey(pSec);
	if (hKey) {
		DWORD dwLen = sizeof(DWORD);
		DWORD dwType = REG_DWORD;
		int nValue;
		LONG lRes = RegQueryValueEx(hKey,
									(WCHAR*)pKey,
									0,
									&dwType,
									(LPBYTE)&nValue,
									&dwLen);
		RegCloseKey(hKey);
		if (lRes == ERROR_SUCCESS)
			return nValue;
	}
	return nDefValue;
}

BOOL SetRegInt(LPCWSTR pSec, LPCWSTR pKey, int nData)
{
	HKEY hKey = GetSectionKey(pSec);
	if (!hKey)
		return FALSE;

	RegSetValueEx(hKey, pKey, 0, REG_DWORD, (const LPBYTE)&nData, sizeof(DWORD));
	RegCloseKey(hKey);
	return TRUE;
}
