////////////////////////////////////////////////////////////////////////
// convert_setting.cpp

#include "stdafx.h"
#include "fv7.h"
#include "..\lsftc\lsftc_i.h"

//////////////////////////////////////////////////////////////////////////
//

void CheckAndConvertReg()
{
	// インストール後、最初の起動時
	if (!RegSectionExists(IDS_SEC_SETTINGS)) {
		// 試用期限
		CComPtr<IFileXferComponent> pt;
		HRESULT hr = pt.CoCreateInstance(__uuidof(FileXferComponent));
		if (SUCCEEDED(hr)) {
			pt->InitInstance();
			CreateRegSection(IDS_SEC_SETTINGS);
		}
	}
}

/////////////////////////////////////////////////////////////////////////
//

void ResetApplicationEnvironment()
{
	TCHAR buf[MAX_PATH * 2];
	CSimpleArray<CString> arrKey;
	DWORD len, index;
	FILETIME ft;

	CRegKey app;
	// Software\LightShip\FileVisor
	if (GetRegApp(app)) {
		for (index = 0; ;index++) {
			len = ARRAYSIZE(buf);
			if (app.EnumKey(index, buf, &len, &ft) == ERROR_SUCCESS)
				break;
			if (StrCmpI(buf, _T("UserInfo")))
				arrKey.Add(buf);
		}
		app.Close();
	}
	// Software\LightShip\CommonSetting
	if (GetRegApp(app, _T("CommonSetting"))) {
		for (index = 0; ;index++) {
			len = ARRAYSIZE(buf);
			if (app.EnumKey(index, buf, &len, &ft) == ERROR_SUCCESS)
				break;
			arrKey.Add(buf);
		}
		app.Close();
	}
	// Software\LightShip\FopSetting
	if (GetRegApp(app, _T("FopSetting"))) {
		for (index = 0; ;index++) {
			len = ARRAYSIZE(buf);
			if (app.EnumKey(index, buf, &len, &ft) == ERROR_SUCCESS)
				break;
			arrKey.Add(buf);
		}
		app.Close();
	}
	int cnt = arrKey.GetSize();
	for (int i = 0; i < cnt; i++)
		SHDeleteKey(app, arrKey[i]);

	// app用データの消去
	CString str;
	GetAppDataFolder(str);
	PathCombine(buf, str.GetString(), _T("*.*"));
	WIN32_FIND_DATA wfd;
	HANDLE hFind = ::FindFirstFile(buf, &wfd);
	if (hFind != INVALID_HANDLE_VALUE) {
		do {
			PathCombine(buf, str.GetString(), wfd.cFileName);
			fopRemoveFileNoConfirm(str);
		} while (::FindNextFile(hFind, &wfd));
		FindClose(hFind);
	}
}
