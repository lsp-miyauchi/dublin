////////////////////////////////////////////////////////////////////////
// app_registery.h

#pragma once
#ifndef __APP_REGISTERY_H__
#define __APP_REGISTERY_H__

////////////////////////////////////////////////////////////////////////
//

class CSidText
{
private:
	LPTSTR m_baseptr;
	LPCTSTR m_sidptr;

	LPTSTR _loadstring(UINT id)
	{
		LPTSTR pstr = NULL;
		for(int len = 256; ; len *= 2) {
			pstr = new TCHAR[len];
			if (pstr == NULL) return NULL;

			int res = ::LoadString(ATL::_AtlBaseModule.GetResourceInstance(), id, pstr, len);
			ATLASSERT(res > 0);
			if (res < len - 1) break;

			delete [] pstr;
			pstr = NULL;
		}
		return pstr;
	}

public:
	CSidText(_U_STRINGorID sid) : m_baseptr(NULL), m_sidptr(NULL)
	{
		if (IS_INTRESOURCE(sid.m_lpstr)) {
			m_sidptr = m_baseptr = _loadstring(LOWORD(sid.m_lpstr));
		}
		else {
			m_sidptr = sid.m_lpstr;
		}
	}
	~CSidText()
	{
		if (m_baseptr) delete [] m_baseptr;
	}
	operator LPCTSTR() const { return m_sidptr; }
	operator LPTSTR() { return (LPTSTR)m_sidptr; }
	bool isNull() { return (!m_sidptr || *m_sidptr == 0); }
};

////////////////////////////////////////////////////////////////////////
//

struct _APP_REGSITERY_INFO
{
	LPCTSTR m_pszCompany;
	LPCTSTR m_pszApplication;
};
typedef _APP_REGSITERY_INFO CAppRegisteryInfo;

#define DECLARE_APP_REGISTERY_INFO(CompanyName, AppName) \
static CAppRegisteryInfo& GetAppRegisteryInfo() \
{ \
	static CAppRegisteryInfo ai = { CompanyName, AppName }; \
	return ai; \
}

////////////////////////////////////////////////////////////////////////
//

template <class T>
class CAppRegisteryImpl
{
private:
	HKEY m_hAppKey;

public:
	DECLARE_APP_REGISTERY_INFO(NULL, NULL)

	CAppRegisteryImpl() : m_hAppKey(NULL) { }
	~CAppRegisteryImpl() { if (m_hAppKey) RegCloseKey(m_hAppKey); }

	CString GetRootName()
	{
		CString str; str.Format(_T("software\\%s\\%s"), T::GetAppRegisteryInfo().m_pszCompany, T::GetAppRegisteryInfo().m_pszApplication);
		return str;
	}

	HKEY GetAppKey()
	{
		if (!m_hAppKey) {
			HKEY keySoft;
			if (RegOpenKeyEx(HKEY_CURRENT_USER, _T("software"), 0, KEY_WRITE | KEY_READ, &keySoft) == ERROR_SUCCESS) {
				DWORD dw;
				HKEY keyCompany = NULL;
				if (RegCreateKeyEx(keySoft, T::GetAppRegisteryInfo().m_pszCompany, 0, REG_NONE, REG_OPTION_NON_VOLATILE, KEY_WRITE | KEY_READ, NULL, &keyCompany, &dw) == ERROR_SUCCESS) {
					HKEY keyApp;
					if (RegCreateKeyEx(keyCompany, T::GetAppRegisteryInfo().m_pszApplication, 0, REG_NONE, REG_OPTION_NON_VOLATILE, KEY_WRITE|KEY_READ, NULL, &keyApp, &dw) == ERROR_SUCCESS) {
						m_hAppKey = keyApp;
					}
					RegCloseKey(keyCompany);
				}
				RegCloseKey(keySoft);
			}
		}
		return m_hAppKey;
	}

	HKEY GetKey(LPCTSTR pszSection)
	{
		HKEY keySec = NULL;
		if (GetAppKey()) {
			DWORD dw;
			RegCreateKeyEx(m_hAppKey, pszSection, 0, REG_NONE, REG_OPTION_NON_VOLATILE, KEY_WRITE | KEY_READ, NULL, &keySec, &dw);
		}
		return keySec;
	}

	int GetText(_U_STRINGorID sec, _U_STRINGorID key, LPTSTR pszText, int cchMax)
	{
		int reslen = 0;
		CSidText s(sec); CSidText k(key);
		if (!s.isNull() && !k.isNull() && pszText) {
			HKEY keySec = GetKey(s);
			if (keySec) {
				DWORD type;
				DWORD len = (DWORD)cchMax;
				if (RegQueryValueEx(keySec, k, NULL, &type, (LPBYTE)pszText, &len) == ERROR_SUCCESS)
					reslen = lstrlen(pszText);
				RegCloseKey(keySec);
			}
		}
		return reslen;
	}

	int GetText(_U_STRINGorID sec, _U_STRINGorID key, CString& str)
	{
		LPTSTR p = str.GetBuffer(512);
		int len = GetText(sec, key, p, 512);
		str.ReleaseBuffer();
		return len;
	}

	bool SetText(_U_STRINGorID sec, _U_STRINGorID key, LPCTSTR pszText)
	{
		CSidText s(sec); CSidText k(key);
		if (!s.isNull() && !k.isNull()) {
			HKEY keySec = GetKey(s);
			if (keySec) {
				LONG res;
				if (pszText == NULL)
					res = RegDeleteValue(keySec, k);
				else
					res = RegSetValueEx(keySec, k, NULL, REG_SZ, (LPBYTE)pszText, (lstrlen(pszText) + 1) * sizeof(TCHAR));
				RegCloseKey(keySec);
				return (res == ERROR_SUCCESS);
			}
		}
		return false;
	}

	int GetTextSeq(_U_STRINGorID sec, _U_STRINGorID key, CString& str, int nEntry)
	{
		CSidText s(sec);
		if (!s.isNull()) return 0;

		CString fmt; fmt.Format(s, nEntry);
		return GetText(sec, (LPCTSTR)fmt, str);
	}

	bool SetTextSeq(_U_STRINGorID sec, _U_STRINGorID key, LPCTSTR pszText, int nEntry)
	{
		CSidText s(sec);
		if (!s.isNull()) return false;

		CString fmt; fmt.Format(s, nEntry);
		return SetText(sec, fmt, pszText);
	}

	int GetValue(_U_STRINGorID sec, _U_STRINGorID key, int defValue)
	{
		int value = defValue;
		CSidText s(sec); CSidText k(key);
		if (!s.isNull() && !k.isNull()) {
			HKEY keySec = GetKey(s);
			if (keySec) {
				DWORD type;
				DWORD count = sizeof(DWORD);
				if (RegQueryValueEx(keySec, k, NULL, &type, (LPBYTE)&value, &count) != ERROR_SUCCESS)
					value = defValue;
				RegCloseKey(keySec);
			}
		}
		return value;
	}

	bool SetValue(_U_STRINGorID sec, _U_STRINGorID key, int value)
	{
		CSidText s(sec); CSidText k(key);
		if (!s.isNull() && !k.isNull()) {
			HKEY keySec = GetKey(s);
			if (keySec) {
				LONG res = RegSetValueEx(keySec, k, NULL, REG_DWORD, (LPBYTE)&value, sizeof(value));
				RegCloseKey(keySec);
				return (res == ERROR_SUCCESS);
			}
		}
		return false;
	}

	bool DeleteValue(_U_STRINGorID sec, _U_STRINGorID key)
	{
		CSidText s(sec); CSidText k(key);
		if (!s.isNull() && k.isNull()) {
			HKEY keySec = GetKey(s);
			if (keySec) {
				LONG res = RegDeleteValue(keySec, k);
				RegCloseKey(keySec);
				return (res == ERROR_SUCCESS);
			}
		}
		return false;
	}

	int GetBinary(_U_STRINGorID sec, _U_STRINGorID key, void* pData, int nLen)
	{
		CSidText s(sec); CSidText k(key);
		if (!s.isNull() && !k.isNull()) {
			HKEY keySec = GetKey(s);
			if (keySec) {
				DWORD len = nLen;
				DWORD type = REG_BINARY;
				LONG res = RegQueryValueEx(keySec, k, 0, &type, (BYTE*)pData, &len);
				RegCloseKey(keySec);
				if (res == ERROR_SUCCESS) return len;
			}
		}
		return 0;
	}

	bool SetBinary(_U_STRINGorID sec, _U_STRINGorID key, void* pData, int nLen)
	{
		CSidText s(sec); CSidText k(key);
		if (!s.isNull() && !k.isNull()) {
			HKEY keySec = GetKey(s);
			if (keySec) {
				LONG res = RegSetValueEx(keySec, k, 0, REG_BINARY, (BYTE*)pData, nLen);
				RegCloseKey(keySec);
				return (res == ERROR_SUCCESS);
			}
		}
		return false;
	}

	BYTE* GetBinaryEx(_U_STRINGorID sec, _U_STRINGorID key, int* pnLen)
	{
		BYTE* pd = NULL;
		int len = GetBinary(sec, key, NULL, 0);
		if (len > 0) {
			pd = (BYTE*)malloc(len);
			if (pd) {
				len = GetBinary(sec, key, pd, len);
				if (pnLen)
					*pnLen = len;
			}
		}
		return pd;
	}

	BYTE* GetBinarySeq(_U_STRINGorID sec, _U_STRINGorID key, int index, LPINT pnLen)
	{
		CSidText k(key);
		if (!k.IsEmpty()) {
			CString str; str.Format(_T("%s%d"), k.ptr(), index);
			return GetBinaryEx(sec, str, pnLen);
		}
		return NULL;
	}

	bool SetBinarySeq(_U_STRINGorID sec, _U_STRINGorID key, int index, void* pd, int nLen)
	{
		bool ok = false;
		CSidText k(key);
		if (!k.IsEmpty()) {
			CString str; str.Format(_T("%s%d"), k.ptr(), index);
			ok = SetBinary(s, str, pd, nLen);
		}
		return ok;
	}

	void LoadComboHistory(HWND hCombo, _U_STRINGorID sec, _U_STRINGorID key, int nMaxEntry)
	{
		CComboBox combo = hCombo;
		if (!combo.IsWindow()) return;

		int len;
		BYTE* pd = GetBinaryEx(sec, key, &len);
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

	BOOL SaveComboHistory(HWND hCombo, _U_STRINGorID sec, _U_STRINGorID key, int nMaxEntry, LPCTSTR pszString)
	{
		CComboBox combo = hCombo;
		if (!combo.IsWindow()) return FALSE;

		int cnt = min(combo.GetCount(), nMaxEntry);

		// ヒストリの保存
		// 現在の文字列を最初に設定
		TCHAR* pd = new TCHAR[(cnt + 1) * (MAX_PATH + 1) + 1];
		TCHAR* psz = (TCHAR*)pd;
		lstrcpy(psz, pszString);
		psz += lstrlen(pszString) + 1;

		// リスト内のファイル名を追加
		if (cnt > 0) {
			for (int i = 0; i < cnt; i++) {
				int len = combo.GetLBText(i, psz) + 1;
				if (!lstrcmpi(pszString, psz))
					continue;

				psz += len;
			}
		}

		// \0\0で終端を確定して保存
		*psz++ = 0;
#ifndef _WIN64
		SetBinary(sec, key, pd, PtrToInt((BYTE*)psz - (BYTE*)pd));
#else
		SetBinary(sec, key, pd, (int)((BYTE*)psz - (BYTE*)pd));
#endif // !_WIN64
		delete [] pd;
		return TRUE;
	}

	IStream* OpenStream(_U_STRINGorID sec, _U_STRINGorID key, DWORD grfMode)
	{
		IStream* pstrm = NULL;
		CSidText s(sec); CSidText k(key);
		if (!s.isNull() && !k.isNull()) {
			// 無ければいったん作成してキーを閉じる
			HKEY keySec = GetKey(s);
			if (keySec) {
				RegCloseKey(keySec);
				pstrm = OpenRegStream(m_hAppKey, s, k, grfMode);
			}
		}
		return pstrm;
	}
};





#endif		// __APP_REGISTERY_H__
