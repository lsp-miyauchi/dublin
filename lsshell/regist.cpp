////////////////////////////////////////////////////////////////////////
// regist.cpp

#include "stdafx.h"
#include "mainmodule.h"
#include "resource.h"


////////////////////////////////////////////////////////////////////////
// 書庫の関連付けは HKEY_CURRENT_USER\SOftware\Classes 配下へおこなう

static const TCHAR nil[]			= _T("\0\0");
static const TCHAR strStatus[]		= _T("Status");
static const TCHAR strShellArc[]	= _T("ShellArc");
static const TCHAR strShellEtc[]	= _T("ShellEtc");
static const TCHAR strShellImg[]	= _T("ShellImg");
static const TCHAR strDirectory[]	= _T("Directory");
static const TCHAR strDrive[]		= _T("Drive");
static const TCHAR strShell[]		= _T("shell");
static const TCHAR strCommand[]		= _T("command");
static const TCHAR strArcProgid[]	= _T("vwarc.document");		// InstallShiledでのvwarc.dllコンポーネントの
																// レジストリ設定の名前に注意
typedef struct tagEXTARCS {
	int type;
	LPCTSTR ext;
} EXTARCS;

static EXTARCS stExtArcs[] = {
	{ SHLA_ZIP,	_T(".zip") },
	{ SHLA_ZIP,	_T(".jar") },
	{ SHLA_ZIP,	_T(".war") },
	{ SHLA_ZIP,	_T(".xpi") },
	{ SHLA_CAB,	_T(".cab") },
	{ SHLA_RAR,	_T(".rar") },
	{ SHLA_TAR,	_T(".tar") },
	{ SHLA_TAR,	_T(".tgz") },
	{ SHLA_TAR,	_T(".gz") },
	{ SHLA_TAR,	_T(".bz2") },
	{ SHLA_TAR,	_T(".lzma") },
	{ SHLA_TAR,	_T(".xz") },
	{ SHLA_TAR,	_T(".z") },
	{ SHLA_TAR,	_T(".deb") },
	{ SHLA_7Z,	_T(".7z") },
#ifndef _WIN64
	{ SHLA_LZH,	_T(".lzh") },
	{ SHLA_ARJ,	_T(".arj") },
	{ SHLA_BGA,	_T(".bza") },
	{ SHLA_BGA,	_T(".gza") },
	{ SHLA_YZ1,	_T(".yz1") },
	{ SHLA_GCA,	_T(".gca") },
#endif
};

////////////////////////////////////////////////////////////////////////
//

static BOOL QueryAppPath(LPCTSTR pszExeName, LPTSTR pszPath, int cchMax)
{
	if (!pszExeName || *pszExeName == 0 || !pszPath || cchMax < 1) return FALSE;

	BOOL bOK = FALSE;
	CRegKey reg;
	if (reg.Open(HKEY_LOCAL_MACHINE, _T("SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\App Paths"), KEY_READ) == ERROR_SUCCESS) {
		CRegKey sec;
		if (sec.Open(reg, pszExeName, KEY_READ) == ERROR_SUCCESS) {
			TCHAR buf[MAX_PATH * 2];
			ULONG len = ARRAYSIZE(buf);
			if (sec.QueryStringValue(NULL, buf, &len) == ERROR_SUCCESS && len > 1) {
				TCHAR tmp[MAX_PATH * 2];
				if (ExpandEnvironmentStrings(buf, tmp, ARRAYSIZE(tmp)) > 0)
					lstrcpy(buf, tmp);
				lstrcpyn(pszPath, buf, cchMax);
				bOK = TRUE;
			}
			sec.Close();
		}
		reg.Close();
	}
	return bOK;
}

static HRESULT SetAppAsDefault(LPCWSTR pwzAssocName, LPCWSTR pwzExt)
{
	CComPtr<IApplicationAssociationRegistration> paar;
	HRESULT hr = paar.CoCreateInstance(CLSID_ApplicationAssociationRegistration);
	if (SUCCEEDED(hr)) {
		hr = paar->SetAppAsDefault(pwzAssocName, pwzExt, AT_FILEEXTENSION);
	}
	return hr;
}

static HRESULT QueryAppIsDefault(LPCWSTR pwzAssocName, LPCWSTR pwzExt, BOOL* pfHas)
{
	CComPtr<IApplicationAssociationRegistration> paar;
	HRESULT hr = paar.CoCreateInstance(CLSID_ApplicationAssociationRegistration);
	if (SUCCEEDED(hr)) {
		hr = paar->QueryAppIsDefault(pwzExt, AT_FILEEXTENSION, AL_EFFECTIVE, pwzAssocName, pfHas);
	}
	return hr;
}

static HRESULT QueryCurrentDefault(LPCWSTR pwzExt, LPWSTR pwzResult, int cchMax)
{
	CComPtr<IApplicationAssociationRegistration> paar;
	HRESULT hr = paar.CoCreateInstance(CLSID_ApplicationAssociationRegistration);
	if (SUCCEEDED(hr)) {
		LPWSTR pwzStr;
		hr = paar->QueryCurrentDefault(pwzExt, AT_FILEEXTENSION, AL_EFFECTIVE, &pwzStr);
		if (SUCCEEDED(hr)) {
			*pwzResult = 0;
			if (pwzStr) {
				lstrcpynW(pwzResult, pwzStr, cchMax);
				CoTaskMemFree(pwzStr);
			}
		}
	}
	return hr;
}

////////////////////////////////////////////////////////////////////////
//

static void assoc_set(LPCTSTR ext, LPCTSTR progid)
{
	CUserClassKey ck;
	CRegKey reg;
	if (reg.Create(ck, ext) == ERROR_SUCCESS) {
		reg.SetStringValue(NULL, progid);
	}
}

static int assoc_get(LPCTSTR ext, LPTSTR value, ULONG cchMax)
{
	*value = 0;
	CUserClassKey ck(false);
	CRegKey reg;
	if (reg.Open(ck, ext, KEY_READ) == ERROR_SUCCESS) {
		ULONG len = cchMax;
		if (reg.QueryStringValue(NULL, value, &len) == ERROR_SUCCESS && len > 1)
			return (int)(len - 1);
	}
	return 0;
}

static void assoc_clear(LPCTSTR pszExt, LPCTSTR progid)
{
	TCHAR name[MAX_PATH * 2];
	if (assoc_get(pszExt, name, ARRAYSIZE(name)) > 0 && !lstrcmpi(name, progid)) {
		assoc_set(pszExt, nil);
	}
}

void GetShellSetting(DWORD& arc, DWORD& etc, DWORD& img)
{
	// キーが無い場合の初期値
	arc = SHLA_PACK | SHLA_DROPPACK;
	etc = SHLE_SYNC | SHLE_RENAME | SHLE_FV | SHLE_PATHCLIP | SHLE_ATTR | SHLE_DROPCOPYTO;
	img = SHLI_COPY | SHLI_CONVERT | SHLI_JPEG2PDF;
	CSettingKey sk(false);
	CRegKey reg;
	if (reg.Open(sk, strStatus, KEY_READ) == ERROR_SUCCESS) {
		reg.QueryDWORDValue(strShellArc, arc);
		reg.QueryDWORDValue(strShellEtc, etc);
		reg.QueryDWORDValue(strShellImg, img);
		reg.Close();
	}
}

void SetShellSetting(DWORD arc, DWORD etc, DWORD img)
{
	CSettingKey sk;
	CRegKey reg;
	if (reg.Create(sk, strStatus) == ERROR_SUCCESS) {
		reg.SetDWORDValue(strShellArc, arc);
		reg.SetDWORDValue(strShellEtc, etc);
		reg.SetDWORDValue(strShellImg, img);
		reg.Close();
	}
}

void ClearShellSetting()
{
	CSettingKey sk(false);
	CRegKey reg;
	if (reg.Open(sk, strStatus, KEY_READ) == ERROR_SUCCESS) {
		reg.Close();
		sk.Close();
		SHDeleteKey(HKEY_CURRENT_USER, sk.GetRoot());
	}
}

void RegisterInit(DWORD arc, DWORD etc, DWORD img)
{
	SetShellSetting(arc, etc, img);

	if (etc & SHLE_FV) {
		// FileVisorの起動
		TCHAR exe[MAX_PATH * 2];
		if (QueryAppPath(APP_EXENAME, exe, ARRAYSIZE(exe))) {
			PathQuoteSpaces(exe);
			lstrcat(exe, _T(" %1"));

			CUserClassKey ck;
			CRegKey reg;
			TCHAR buf[MAX_PATH * 2];

			// 「FileVisorで開く」
			CString str; str.LoadString(IDS_CTX_OPENBYFILEVISOR);

			// Directory\shell\\<app_name>\\command を作成
			key_combine(buf, strDirectory, strShell, APP_PRODUCT, NULL);
			if (reg.Create(ck, buf) == ERROR_SUCCESS) {
				reg.SetStringValue(NULL, str.GetString());
				CRegKey cmd;
				if (cmd.Create(reg, strCommand) == ERROR_SUCCESS) {
					cmd.SetStringValue(NULL, exe);
					cmd.Close();
				}
				reg.Close();
			}

			// Drive\shell\\<app_name>\\command を作成
			key_combine(buf, strDrive, strShell, APP_PRODUCT, NULL);
			if (reg.Create(ck, buf) == ERROR_SUCCESS) {
				reg.SetStringValue(NULL, str.GetString());
				CRegKey cmd;
				if (cmd.Create(reg, strCommand) == ERROR_SUCCESS) {
					cmd.SetStringValue(NULL, exe);
					cmd.Close();
				}
				reg.Close();
			}
		}
	}
}

void RegisterTerm()
{
	ClearShellSetting();

	EXTARCS* pea = stExtArcs;
	int cnt = ARRAYSIZE(stExtArcs);
	for (int i = 0; i < cnt; i++) {
		assoc_clear(pea->ext, strArcProgid);
		pea++;
	}

	// FileVisorの起動
	CUserClassKey ck;
	TCHAR buf[MAX_PATH * 2];

	// Directory\shell\\<app_name>以下を削除
	key_combine(buf, strDirectory, strShell, APP_PRODUCT, NULL);
	SHDeleteKey(ck, buf);

	// Drive\shell\\<app_name>以下を削除
	key_combine(buf, strDrive, strShell, APP_PRODUCT, NULL);
	SHDeleteKey(ck, buf);

}
