// lspexif.cpp : DLL エクスポートの実装です。


#include "stdafx.h"
#include "resource.h"
#include "lspexif.h"
#include "RawThmubnail.h"

////////////////////////////////////////////////////////////////////////
//

class CLSExifModule : public CAtlDllModuleT< CLSExifModule >
{
public :
	DECLARE_LIBID(LIBID_LSExifLib)
	DECLARE_REGISTRY_APPID_RESOURCEID(IDR_LSEXIF, "{03AED78B-5501-4C47-9D2B-2CBB0C5E0EE1}")
};

CLSExifModule _AtlModule;


////////////////////////////////////////////////////////////////////////
//

COSPlatform gosp;


#ifdef _MANAGED
#pragma managed(push, off)
#endif

////////////////////////////////////////////////////////////////////////
// DLL エントリ ポイント
extern "C" BOOL WINAPI DllMain(HINSTANCE hInstance, DWORD dwReason, LPVOID lpReserved)
{
	hInstance;
    return _AtlModule.DllMain(dwReason, lpReserved); 
}

#ifdef _MANAGED
#pragma managed(pop)
#endif


////////////////////////////////////////////////////////////////////////
// DLL を OLE によってアンロードできるようにするかどうかを指定します。
STDAPI DllCanUnloadNow(void)
{
    return _AtlModule.DllCanUnloadNow();
}


////////////////////////////////////////////////////////////////////////
// 要求された型のオブジェクトを作成するクラス ファクトリを返します。
STDAPI DllGetClassObject(REFCLSID rclsid, REFIID riid, LPVOID* ppv)
{
    return _AtlModule.DllGetClassObject(rclsid, riid, ppv);
}


////////////////////////////////////////////////////////////////////////
//

typedef struct __REGISTRY_ENTRY {
	HKEY    hkeyRoot;
	LPCTSTR pszKey;
	LPCTSTR pszValue;
	LPCTSTR pszData;
} REGISTRY_ENTRY;

static HRESULT CreateRegKeyAndSetValue(REGISTRY_ENTRY* pre)
{
	HRESULT hr = E_INVALIDARG;
	if (pre) {
		HKEY hKey;
		LONG lRet = RegCreateKeyEx(pre->hkeyRoot, pre->pszKey, 0, NULL, REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, NULL, &hKey, NULL);
		if (lRet != ERROR_SUCCESS) {
			hr = HRESULT_FROM_WIN32(lRet);
		}
		else {
			lRet = RegSetValueEx(hKey, pre->pszValue, 0, REG_SZ, (BYTE*)pre->pszData, ((DWORD)lstrlen(pre->pszData) + 1) * sizeof(TCHAR));
			hr = HRESULT_FROM_WIN32(lRet);
			RegCloseKey(hKey);
		}
	}
	return hr;
}

#define SZ_RAWTHUMBNAIL					L"RawThmubnail Class"
#define SZ_CLSID_RAWTHUMBNAIL			L"{12524D4C-6E12-4615-987D-94BB7E1C1D8B}"
#define SZ_SHELLEXTAPPROVEDPATH			L"Software\\Microsoft\\Windows\\CurrentVersion\\Shell Extensions\\Approved"

////////////////////////////////////////////////////////////////////////
// DllRegisterServer - エントリをシステム レジストリに追加します。
STDAPI DllRegisterServer(void)
{
	// ThumbnailHandler : HKCR\lspexif.DigitalImage\shellex\<IID_IThumbnailProvider> = {12524D4C-6E12-4615-987D-94BB7E1C1D8B}
	WCHAR wsz[MAX_PATH];
	if (StringFromGUID2(IID_IThumbnailProvider, wsz, ARRAYSIZE(wsz))) {
		TCHAR sz[MAX_PATH] = {0};
		REGISTRY_ENTRY table[] = {
			// RootKey             KeyName                                                    ValueName               Data
			// ------------------- ---------------------------------------------------------- ----------------------- ------------------------
			{  HKEY_CLASSES_ROOT,  PathCombine(sz, _T(".lspexif.DigitalImage\\ShellEx"), wsz), NULL,                  SZ_CLSID_RAWTHUMBNAIL },
			{  HKEY_LOCAL_MACHINE, SZ_SHELLEXTAPPROVEDPATH,                                    SZ_CLSID_RAWTHUMBNAIL, SZ_RAWTHUMBNAIL },
		};

		for (int i = 0; i < ARRAYSIZE(table); i++)
			CreateRegKeyAndSetValue(&table[i]);
	}

    // タイプライブラリ以外を登録する（FALSE）
    HRESULT hr = _AtlModule.DllRegisterServer(FALSE);
	return hr;
}


////////////////////////////////////////////////////////////////////////
// DllUnregisterServer - エントリをレジストリから削除します。
STDAPI DllUnregisterServer(void)
{
	SHDeleteKeyW(HKEY_CLASSES_ROOT, _T(".lspexif.DigitalImage"));
	SHDeleteValueW(HKEY_LOCAL_MACHINE, SZ_SHELLEXTAPPROVEDPATH, SZ_CLSID_RAWTHUMBNAIL);

	HRESULT hr = _AtlModule.DllUnregisterServer(FALSE);
	return hr;
}


////////////////////////////////////////////////////////////////////////
// DllInstall - ユーザーおよびコンピュータごとのシステム レジストリ エントリを
//              追加または削除します。	
STDAPI DllInstall(BOOL bInstall, LPCWSTR pszCmdLine)
{
	HRESULT hr = E_FAIL;
	static const wchar_t szUserSwitch[] = _T("user");

	if (pszCmdLine != NULL) {
		if (_wcsnicmp(pszCmdLine, szUserSwitch, _countof(szUserSwitch)) == 0) {
			AtlSetPerUserRegistration(true);
		}
	}

    if (bInstall) {
    	hr = DllRegisterServer();
    	if (FAILED(hr)) {
    		DllUnregisterServer();
    	}
	}
    else {
		hr = DllUnregisterServer();
	}
	return hr;
}
