// lsshell.cpp : DLL エクスポートの実装です。


#include "stdafx.h"
#include "resource.h"
#include "lsshell_i.h"
#include "mainmodule.h"
#include "setup.h"
#include "dlldatax.h"

#include <delayimp.h>


////////////////////////////////////////////////////////////////////////
//

static TCHAR strShellApproved[]   = _T("SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Shell Extensions\\Approved");

typedef struct tagREGAPPROVED {
	CLSID clsid;
	TCHAR* title;
} REGAPPROVED;

static REGAPPROVED tblClsIds[] = {
	{ CLSID_Ctx,		_T("lsshell Ctx") },
	{ CLSID_AddPack,	_T("lsshell AddPack") },
	{ CLSID_UnpackTo,	_T("lsshell UnpackTo") },
	{ CLSID_PackTo,		_T("lsshell PackTo") },
	{ CLSID_JunctionTo,	_T("lsshell JunctionTo") },
	{ CLSID_CopyTo,		_T("lsshell CopyTo") },
	{ CLSID_ThumbOutTo,	_T("lsshell ThumbOutTo") },
	{ CLSID_ExifProp,	_T("lsshell ExifProp") },
	{ CLSID_ImageTools,	_T("lsshell ImageTools") },
	{ CLSID_NULL,		NULL },
};

////////////////////////////////////////////////////////////////////////
//

// DLL を OLE によってアンロードできるようにするかどうかを指定します。
STDAPI DllCanUnloadNow(void)
{
	__FUnloadDelayLoadedDLL2("lscore.dll");

#ifdef _MERGE_PROXYSTUB
    HRESULT hr = PrxDllCanUnloadNow();
    if (hr != S_OK)
        return hr;
#endif
    return _AtlModule.DllCanUnloadNow();
}


// 要求された型のオブジェクトを作成するクラス ファクトリを返します。
STDAPI DllGetClassObject(REFCLSID rclsid, REFIID riid, LPVOID* ppv)
{
#ifdef _MERGE_PROXYSTUB
    if (PrxDllGetClassObject(rclsid, riid, ppv) == S_OK)
        return S_OK;
#endif
    return _AtlModule.DllGetClassObject(rclsid, riid, ppv);
}


// DllRegisterServer - エントリをシステム レジストリに追加します。
STDAPI DllRegisterServer(void)
{
    // オブジェクト、タイプ ライブラリおよびタイプ ライブラリ内のすべてのインターフェイスを登録します
    HRESULT hr = _AtlModule.DllRegisterServer(FALSE);
#ifdef _MERGE_PROXYSTUB
    if (FAILED(hr))
        return hr;
    hr = PrxDllRegisterServer();
#endif

	// インストール単位の取得
	bool bPerUserRegistration = false;
	AtlGetPerUserRegistration(&bPerUserRegistration);
	HKEY hRootKey = bPerUserRegistration ? HKEY_CURRENT_USER : HKEY_LOCAL_MACHINE;

	// シェル拡張の登録
	CRegKey reg;
	if (reg.Create(hRootKey, strShellApproved) == ERROR_SUCCESS) {
		int i = 0;
		while (tblClsIds[i].clsid != CLSID_NULL) {
			LPOLESTR pole;
			StringFromCLSID(tblClsIds[i].clsid, &pole);
			reg.SetStringValue(pole, tblClsIds[i].title);
			CoTaskMemFree(pole);
			i++;
		}
		reg.Close();
	}
	return hr;
}


// DllUnregisterServer - エントリをレジストリから削除します。
STDAPI DllUnregisterServer(void)
{
	// インストール単位の取得
	bool bPerUserRegistration = false;
	AtlGetPerUserRegistration(&bPerUserRegistration);
	HKEY hRootKey = bPerUserRegistration ? HKEY_CURRENT_USER : HKEY_LOCAL_MACHINE;

	// シェル拡張の解除
	CRegKey reg;
	if (reg.Open(hRootKey, strShellApproved) == ERROR_SUCCESS) {
		int i = 0;
		while (tblClsIds[i].clsid != CLSID_NULL) {
			LPOLESTR pole;
			StringFromCLSID(tblClsIds[i].clsid, &pole);
			reg.DeleteValue(pole);
			CoTaskMemFree(pole);
			i++;
		}
		reg.Close();
	}

	HRESULT hr = _AtlModule.DllUnregisterServer(FALSE);
#ifdef _MERGE_PROXYSTUB
    if (FAILED(hr))
        return hr;
    hr = PrxDllRegisterServer();
    if (FAILED(hr))
        return hr;
    hr = PrxDllUnregisterServer();
#endif
	return hr;
}

// DllInstall - ユーザーおよびコンピュータごとのシステム レジストリ エントリを
//              追加または削除します。	
STDAPI DllInstall(BOOL bInstall, LPCWSTR pszCmdLine)
{
    HRESULT hr = E_FAIL;
    static const wchar_t szUserSwitch[] = _T("user");

    if (pszCmdLine != NULL)
    {
    	if (_wcsnicmp(pszCmdLine, szUserSwitch, _countof(szUserSwitch)) == 0)
    	{
    		AtlSetPerUserRegistration(true);
    	}
    }

    if (bInstall)
    {	
    	hr = DllRegisterServer();
    	if (FAILED(hr))
    	{	
    		DllUnregisterServer();
    	}
    }
    else
    {
    	hr = DllUnregisterServer();
    }

    return hr;
}

STDAPI_(BOOL) DllGetVersion(DLLVERSIONINFO* pdvi)
{
	if (pdvi && pdvi->cbSize >= sizeof(DLLVERSIONINFO)) {
		pdvi->dwMajorVersion = 2;
		pdvi->dwMinorVersion = 0;
		pdvi->dwBuildNumber = 0;
		pdvi->dwPlatformID = DLLVER_PLATFORM_NT;
		return TRUE;
	}
	return FALSE;
}

// fv5.exe 用に残してある
STDAPI_(void) DllSetupServer(HWND hwnd, HINSTANCE hInst, LPTSTR pCmdLine, int nShow)
{
	DoSetup(hwnd);
}
