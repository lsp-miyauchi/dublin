///////////////////////////////////////////////////////////////////////
// reg.cpp

#include "stdafx.h"

#define RESINST			(_AtlBaseModule.GetModuleInstance())

static TCHAR sszProfileKey[] = _T("ArcManager");
TCHAR gszAppRegRootKey[] = _T("Software\\LightShip\\ArcManager");

////////////////////////////////////////////////////////////////
//

static LPTSTR _loadstring(UINT id)
{
	LPTSTR pstr = NULL;
	for(int len = 256; ; len *= 2) {
		pstr = new TCHAR[len];
		if (pstr == NULL) return NULL;

		int res = ::LoadString(RESINST, id, pstr, len);
		if (res < len - 1) break;

		delete [] pstr;
		pstr = NULL;
	}
	return pstr;
}

class CSecKey
{
private:
	LPTSTR m_secAlloc;
	LPTSTR m_keyAlloc;
public:
	LPCTSTR m_sec;
	LPCTSTR m_key;

public:
	CSecKey(_U_STRINGorID sec, _U_STRINGorID key)
	{
		if (IS_INTRESOURCE(sec.m_lpstr)) {
			m_secAlloc = _loadstring(LOWORD(sec.m_lpstr));
			m_sec = m_secAlloc;
		}
		else {
			m_secAlloc = NULL;
			m_sec = sec.m_lpstr;
		}

		if (IS_INTRESOURCE(key.m_lpstr)) {
			m_keyAlloc = _loadstring(LOWORD(key.m_lpstr));
			m_key = m_keyAlloc;
		}
		else {
			m_keyAlloc = NULL;
			m_key = key.m_lpstr;
		}
	}

	~CSecKey()
	{
		if (m_secAlloc) delete [] m_secAlloc;
		if (m_keyAlloc) delete [] m_keyAlloc;
	}
};

////////////////////////////////////////////////////////////////
//

static HKEY GetAppRegistryKey()
{
	HKEY hAppKey = NULL;
	HKEY hSoftKey = NULL;
	HKEY hCompanyKey = NULL;
	if (RegOpenKeyEx(HKEY_CURRENT_USER, _T("software"), 0, KEY_WRITE|KEY_READ, &hSoftKey) == ERROR_SUCCESS) {
		DWORD dw;
		if (RegCreateKeyEx(hSoftKey, APP_COMPANY, 0, REG_NONE, REG_OPTION_NON_VOLATILE, KEY_WRITE|KEY_READ, NULL, &hCompanyKey, &dw) == ERROR_SUCCESS) {
			RegCreateKeyEx(hCompanyKey, sszProfileKey, 0, REG_NONE, REG_OPTION_NON_VOLATILE, KEY_WRITE|KEY_READ, NULL, &hAppKey, &dw);
		}
	}
	if (hSoftKey != NULL) RegCloseKey(hSoftKey);
	if (hCompanyKey != NULL) RegCloseKey(hCompanyKey);

	return hAppKey;
}

static HKEY GetSectionKey(LPCTSTR lpszSection)
{
	HKEY hSectionKey = NULL;
	HKEY hAppKey = GetAppRegistryKey();
	if (hAppKey == NULL) return NULL;

	DWORD dw;
	RegCreateKeyEx(hAppKey, lpszSection, 0, REG_NONE, REG_OPTION_NON_VOLATILE, KEY_WRITE|KEY_READ, NULL, &hSectionKey, &dw);
	RegCloseKey(hAppKey);
	return hSectionKey;
}

//////////////////////////////////////////////////////////////////////
//

int GetRegText(_U_STRINGorID sec, _U_STRINGorID key, LPTSTR pszText, int cchMax)
{
	CSecKey z(sec, key);
	if (!z.m_sec || !z.m_key || !pszText) return 0;

	int reslen = 0;
	HKEY hSecKey = GetSectionKey(z.m_sec);
	if (hSecKey) {
		DWORD type;
		DWORD len = (DWORD)cchMax;
		if (RegQueryValueEx(hSecKey, z.m_key, NULL, &type, (LPBYTE)pszText, &len) == ERROR_SUCCESS)
			reslen = lstrlen(pszText);
		RegCloseKey(hSecKey);
	}
	return reslen;
}

BOOL SetRegText(_U_STRINGorID sec, _U_STRINGorID key, LPCTSTR pszText)
{
	CSecKey z(sec, key);
	if (!z.m_sec || !z.m_key) return FALSE;

	HKEY hSecKey = GetSectionKey(z.m_sec);
	if (hSecKey == NULL) return FALSE;

	LONG res;
	if (pszText == NULL)
		res = RegDeleteValue(hSecKey, (LPTSTR)z.m_key);
	else
		res = RegSetValueEx(hSecKey, z.m_key, NULL, REG_SZ, (LPBYTE)pszText, (lstrlen(pszText) + 1) * sizeof(TCHAR));
	RegCloseKey(hSecKey);
	return (res == ERROR_SUCCESS);
}

int GetRegValue(_U_STRINGorID sec, _U_STRINGorID key, int nDefValue)
{
	CSecKey z(sec, key);
	if (!z.m_sec || !z.m_key) return nDefValue;

	HKEY hSecKey = GetSectionKey(z.m_sec);
	if (hSecKey == NULL) return nDefValue;

	DWORD dwValue;
	DWORD dwType;
	DWORD dwCount = sizeof(DWORD);
	LONG res = RegQueryValueEx(hSecKey, (LPTSTR)z.m_key, NULL, &dwType, (LPBYTE)&dwValue, &dwCount);
	RegCloseKey(hSecKey);
	if (res == ERROR_SUCCESS) return (UINT)dwValue;
	return nDefValue;
}

BOOL SetRegValue(_U_STRINGorID sec, _U_STRINGorID key, int nValue)
{
	CSecKey z(sec, key);
	if (!z.m_sec || !z.m_key) return FALSE;

	HKEY hSecKey = GetSectionKey(z.m_sec);
	if (hSecKey == NULL) return FALSE;

	LONG res = RegSetValueEx(hSecKey, (LPTSTR)z.m_key, NULL, REG_DWORD, (LPBYTE)&nValue, sizeof(nValue));
	RegCloseKey(hSecKey);
	return (res == ERROR_SUCCESS);
}

int GetRegBinary(_U_STRINGorID sec, _U_STRINGorID key, LPVOID lpData, int nLen)
{
	CSecKey z(sec, key);
	if (!z.m_sec || !z.m_key) return 0;

	HKEY hKey = GetSectionKey(z.m_sec);
	if (hKey) {
		DWORD dwLen = nLen;
		DWORD dwType = REG_BINARY;
		LONG lRes = RegQueryValueEx(hKey, (LPTSTR)z.m_key, 0, &dwType, (LPBYTE)lpData, &dwLen);
		RegCloseKey(hKey);
		if (lRes == ERROR_SUCCESS) return dwLen;
	}
	return 0;
}

LPBYTE GetRegBinaryEx(_U_STRINGorID sec, _U_STRINGorID key, LPINT lpnLen)
{
	LPBYTE pd = NULL;
	int len = GetRegBinary(sec, key, NULL, 4);
	if (len > 0) {
		pd = (LPBYTE)malloc(len);
		GetRegBinary(sec, key, pd, len);
	}
	if (lpnLen)
		*lpnLen = len;
	return pd;
}

BOOL SetRegBinary(_U_STRINGorID sec, _U_STRINGorID key, LPVOID lpData, int nLen)
{
	CSecKey z(sec, key);
	if (!z.m_sec || !z.m_key) return FALSE;

	HKEY hKey = GetSectionKey(z.m_sec);
	if (!hKey) return FALSE;

	RegSetValueEx(hKey, z.m_key, 0, REG_BINARY, (LPBYTE)lpData, nLen);
	RegCloseKey(hKey);
	return TRUE;
}

LPBYTE GetRegBinarySeqEx(_U_STRINGorID sec, _U_STRINGorID key, int nEntry, LPINT pnLen)
{
	CSecKey z(sec, key);
	if (!z.m_sec || !z.m_key) return NULL;

	CString k;
	k.Format(_T("%s%d"), z.m_key, nEntry);
	return GetRegBinaryEx(sec, (LPTSTR)(LPCTSTR)k, pnLen);
}

BOOL SetRegBinarySeq(_U_STRINGorID sec, _U_STRINGorID key, int nEntry, void* pd, int nLen)
{
	CSecKey z(sec, key);
	if (!z.m_sec || !z.m_key) return FALSE;

	CString k;
	k.Format(_T("%s%d"), z.m_key, nEntry);
	return SetRegBinary(sec, (LPTSTR)(LPCTSTR)k, pd, nLen);
}

void xxRegLoadComboHistory(HWND hCombo, _U_STRINGorID sec, _U_STRINGorID key, int nMaxEntry)
{
	CComboBox combo = hCombo;
	if (!combo.IsWindow()) return;

	int len;
	LPBYTE pd = GetRegBinaryEx(sec, key, &len);
	if (pd && len > 0) {
		len /= sizeof(TCHAR);
		int i = 0;
		TCHAR* p = (TCHAR*)pd;
		while (len > 0 && p && *p && i < nMaxEntry) {
			combo.AddString(p);
			len -= (lstrlen(p) + 1);
			p += lstrlen(p) + 1;
			i++;
		}
		free(pd);
	}
}

BOOL xxRegSaveComboHistory(HWND hCombo, _U_STRINGorID sec, _U_STRINGorID key, int nMaxEntry, LPCTSTR pszString)
{
	CComboBox combo = hCombo;
	if (!combo.IsWindow()) return FALSE;

	int cnt = min(combo.GetCount(), nMaxEntry);

	// ヒストリの保存
	// 現在の文字列を最初に設定
	TCHAR* pd = new TCHAR[(cnt + 1) * (MAX_PATH + 1) + 1];
	TCHAR* psz = (TCHAR*)pd;
	lstrcpy(psz, pszString);
	int len = lstrlen(pszString) + 1;
	psz += len;

	// リスト内のファイル名を追加
	if (cnt > 0) {
		for (int i = 0; i < cnt; i++) {
			len = combo.GetLBText(i, psz) + 1;
			if (!lstrcmpi(pszString, psz))
				continue;

			psz += len;
		}
	}

	// \0\0で終端を確定して保存
	*psz++ = '\0';
#ifdef _WIN64
	SetRegBinary(sec, key, pd, (int)(psz - pd));
#else
	SetRegBinary(sec, key, pd, PtrToInt(psz - pd));
#endif
	delete [] pd;

	return TRUE;
}

int RegKeyCombine(_U_STRINGorID sec, _U_STRINGorID key, CString& strKey)
{
	CSecKey z(sec, key);
	if (!z.m_sec || !z.m_key) return -1;

	strKey.Format(_T("%s\\%s\\%s"), gszAppRegRootKey, z.m_sec, z.m_key);
	return strKey.GetLength();
}

void RegLoadHistoryEx(HWND hCombo, _U_STRINGorID sec, _U_STRINGorID key, int nMaxEntry)
{
	CComboBox combo = hCombo;
	if (!combo.IsWindow()) return;

	CString strKey;
	RegKeyCombine(sec, key, strKey);

	CMRUProc mru;
	if (mru.Create(HKEY_CURRENT_USER, strKey, nMaxEntry)) {
		int n = mru.GetCount();
		TCHAR buf[MAX_PATH * 2];
		for (int i = 0; i < n; i++) {
			if (mru.Enum(i, buf, ARRAYSIZE(buf)) > 0)
				combo.AddString(buf);
		}
		mru.Close();
	}
}

BOOL RegSaveHistoryEx(_U_STRINGorID sec, _U_STRINGorID key, int nMaxEntry, LPCTSTR pszString)
{
	CString strKey;
	RegKeyCombine(sec, key, strKey);

	CMRUProc mru;
	if (mru.Create(HKEY_CURRENT_USER, strKey, nMaxEntry)) {
		mru.Add(pszString);
		mru.Close();
	}
	return TRUE;
}

void RegClearHistoryEx(HWND hCombo, _U_STRINGorID sec, _U_STRINGorID key)
{
	CComboBox combo = hCombo;
	if (combo.IsWindow())
		combo.ResetContent();

	CSecKey z(sec, key);
	if (z.m_sec && z.m_key) {
		HKEY hkey = GetSectionKey(z.m_sec);
		if (hkey) {
			SHDeleteKey(hkey, z.m_key);
			RegCloseKey(hkey);
		}
	}
}
