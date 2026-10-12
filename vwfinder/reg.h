/////////////////////////////////////////////////////////////////////
// reg.h

#pragma once
#include "..\lscore\app_registery.h"

class CAppReg : public CAppRegisteryImpl<CAppReg>
{
public:
	DECLARE_APP_REGISTERY_INFO(_T("LightShip"), _T("Applets\\vwfinder"))

	CString KeyCombine(_U_STRINGorID sec, _U_STRINGorID key)
	{
		CSidText s(sec); CSidText k(key);
		if (s.isNull() || k.isNull()) return _T("");

		CString str = GetRootName();
		str.AppendFormat(_T("\\%s\\%s"), (LPCTSTR)s, (LPCTSTR)k);
		return str;
	}

	void LoadMruHistory(HWND hCombo, _U_STRINGorID sec, _U_STRINGorID key, int nMaxEntry)
	{
		CComboBox combo = hCombo;
		if (!combo.IsWindow()) return;

		CString str = KeyCombine(sec, key);
		CMRUProc mru;
		if (mru.Create(HKEY_CURRENT_USER, str, nMaxEntry)) {
			int n = mru.GetCount();
			TCHAR buf[MAX_PATH * 2];
			for (int i = 0; i < n; i++) {
				if (mru.Enum(i, buf, _COUNTOF(buf)) > 0)
					combo.AddString(buf);
			}
			mru.Close();
		}
	}

	void SaveMruHistory(_U_STRINGorID sec, _U_STRINGorID key, int nMaxEntry, LPCTSTR pszString)
	{
		CString str = KeyCombine(sec, key);
		CMRUProc mru;
		if (mru.Create(HKEY_CURRENT_USER, str, nMaxEntry)) {
			mru.Add(pszString);
			mru.Close();
		}
	}

	void ClearMruHistory(HWND hCombo, _U_STRINGorID sec, _U_STRINGorID key)
	{
		CComboBox combo = hCombo;
		if (combo.IsWindow())
			combo.ResetContent();

		CString str = KeyCombine(sec, key);
		SHDeleteKey(HKEY_CURRENT_USER, str);
	}
};

