//////////////////////////////////////////////////////////////////////
// regapp.h

#pragma once
#ifndef __REGAPP_H__
#define __REGAPP_H__

#include "crctable.h"


//////////////////////////////////////////////////////////////////////
//

#pragma pack(1)

typedef struct tagJUMSTRING {
	WORD crc;
	BYTE data[1];
} JUMSTRING;

#pragma pack()


class CRegApp
{
private:
	HKEY m_hkeyRoot;				// HKCU or HKLM
	CString m_strCompany;
	CString m_strApplication;

public:
	CRegApp() : m_hkeyRoot(HKEY_CURRENT_USER)
	{
	}

	void SetApplication(LPCTSTR pszCompany, LPCTSTR pszApplication)
	{
		m_strCompany = pszCompany;
		m_strApplication = pszApplication;
	}

	HKEY GetApplicationKey(bool bForRead)
	{
		CString str = _T("Software");
		str += _T('\\');
		str += m_strCompany;
		str += _T('\\');
		str += m_strApplication;

		CRegKey app;
		if (bForRead) {
			if (app.Open(m_hkeyRoot, str, KEY_READ) != ERROR_SUCCESS)
				return NULL;
		}
		else {
			if (app.Create(m_hkeyRoot, str) != ERROR_SUCCESS)
				return NULL;
		}
		return app.Detach();
	}

	HKEY GetSectionKey(LPCTSTR pszSection, bool bForRead)
	{
		HKEY happ = GetApplicationKey(bForRead);
		if (!happ) return NULL;

		CRegKey app(happ);
		CRegKey sec;
		if (bForRead) {
			if (sec.Open(app, pszSection, KEY_READ) != ERROR_SUCCESS)
				return NULL;
		}
		else {
			if (sec.Create(app, pszSection) != ERROR_SUCCESS)
				return NULL;
		}
		return sec.Detach();
	}

	ULONG GetBinary(LPCTSTR pszSection, LPCTSTR pszValueName, void* pvData, ULONG cbMax)
	{
		HKEY hkey = GetSectionKey(pszSection, true);
		if (hkey) {
			CRegKey reg(hkey);
			if (reg.QueryBinaryValue(pszValueName, pvData, &cbMax) == ERROR_SUCCESS)
				return cbMax;
		}
		return 0;
	}

	void* GetBinaryEx(LPCTSTR pszSection, LPCTSTR pszValueName, ULONG* pnData)
	{
		void* buffer = NULL;
		ULONG len = GetBinary(pszSection, pszValueName, NULL, 4);
		if (len > 0) {
			buffer = malloc(len);
			len = GetBinary(pszSection, pszValueName, buffer, len);
		}
		if (pnData)
			*pnData = len;
		return buffer;
	}

	BOOL SetBinary(LPCTSTR pszSection, LPCTSTR pszValueName, void* pvData, ULONG nData)
	{
		HKEY hkey = GetSectionKey(pszSection, false);
		if (!hkey) return FALSE;

		CRegKey reg(hkey);
		reg.SetBinaryValue(pszValueName, pvData, nData);
		return TRUE;
	}

	ULONG GetString(LPCTSTR pszSection, LPCTSTR pszValueName, LPTSTR pszString, ULONG cchMax)
	{
		*pszString = 0;
		HKEY hkey = GetSectionKey(pszSection, true);
		if (!hkey) return 0;

		CRegKey reg(hkey);
		if (reg.QueryStringValue(pszValueName, pszString, &cchMax) == ERROR_SUCCESS) {
			--cchMax;
			pszString[cchMax] = 0;
			return cchMax;
		}
		return 0;
	}

	BOOL SetString(LPCTSTR pszSection, LPCTSTR pszValueName, LPCTSTR pszString)
	{
		HKEY hkey = GetSectionKey(pszSection, false);
		if (!hkey) return FALSE;

		CRegKey reg(hkey);
		if (!pszString)
			reg.DeleteValue(pszValueName);
		else {
			if (reg.SetStringValue(pszValueName, pszString) != ERROR_SUCCESS)
				return FALSE;
		}
		return TRUE;
	}

	DWORD GetValue(LPCTSTR pszSection, LPCTSTR pszValueName, DWORD defValue)
	{
		HKEY hkey = GetSectionKey(pszSection, true);
		if (!hkey) return defValue;

		CRegKey reg(hkey);
		DWORD value;
		if (reg.QueryDWORDValue(pszValueName, value) != ERROR_SUCCESS)
			value = defValue;
		return value;
	}

	BOOL SetValue(LPCTSTR pszSection, LPCTSTR pszValueName, DWORD value)
	{
		HKEY hkey = GetSectionKey(pszSection, false);
		if (!hkey) return FALSE;

		CRegKey reg(hkey);
		return (reg.SetDWORDValue(pszValueName, value) == ERROR_SUCCESS);
	}

	BOOL DeleteValue(LPCTSTR pszSection, LPCTSTR pszValueName)
	{
		HKEY hkey = GetSectionKey(pszSection, false);
		if (!hkey) return FALSE;

		CRegKey reg(hkey);
		return (reg.DeleteValue(pszValueName) == ERROR_SUCCESS);
	}

	BOOL DeleteValueAll(LPCTSTR pszSection)
	{
		HKEY hkey = GetSectionKey(pszSection, false);
		if (!hkey) return FALSE;

		// 現在のキーにある値を列挙
		CSimpleArray<CString> array;
		DWORD n;
		DWORD cnt = 0;
		for (;;) {
			TCHAR name[MAX_PATH];
			DWORD dw = MAX_PATH;
			if (RegEnumValue(hkey, cnt, name, &n, 0, 0, 0, 0) != ERROR_SUCCESS)
				break;
			array.Add(name);
			cnt++;
		}

		int errors = 0;
		for (n = 0; n < cnt; n++) {
			// 値を削除
			if (RegDeleteValue(hkey, (LPTSTR)(LPCTSTR)array[n]) != ERROR_SUCCESS)
				errors++;
		}
		RegCloseKey(hkey);

		return (errors == 0);
	}

	int GetStringJum(LPCTSTR pszSection, LPCTSTR pszValueName, char* pszString)
	{
		*pszString = 0;
		ULONG len;
		JUMSTRING* jumptr = (JUMSTRING*)GetBinaryEx(pszSection, pszValueName, &len);
		if (jumptr) {
			if (len > 2) {
				len -= 2;
				BYTE* p = jumptr->data;
				BYTE* q = (BYTE*)pszString;
				ULONG i;
				for (i = 0; i < len; i++) {
					*q = *p + 0x1E;
					p++;
					q++;
				}
				*q = 0;

				CCrc16 cc;
				if (jumptr->crc != cc.Calc(0, (BYTE*)pszString, len))
					*pszString = 0;
			}
			free(jumptr);
		}
		return lstrlenA(pszString);
	}

	BOOL SetStringJum(LPCTSTR pszSection, LPCTSTR pszValueName, const char* pszString)
	{
		if (!pszString) return DeleteValue(pszSection, pszValueName);

		int len = lstrlenA(pszString);
		ULONG jumsize = sizeof(JUMSTRING) + len - 1;			// dec sizeof(JUMSTRING.data)
		JUMSTRING* jumptr = (JUMSTRING*)malloc(jumsize);

		CCrc16 cc;
		jumptr->crc = cc.Calc(0, (BYTE*)pszString, len);
		BYTE* p = jumptr->data;
		BYTE* q = (BYTE*)pszString;

		while (len-- > 0) {
			*p = *q - 0x1E;
			p++;
			q++;
		}
		return SetBinary(pszSection, pszValueName, jumptr, jumsize);
	}
};


#endif		// __REGAPP_H__
