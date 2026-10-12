// lsarcsvr.cpp : DLL エクスポートの実装です。


#include "stdafx.h"
#include "resource.h"
#include "lsarcsvr_i.h"
#include "dllmain.h"
#include "dlldatax.h"

////////////////////////////////////////////////////////////////////////
//

static TCHAR strShellApproved[]   = _T("SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Shell Extensions\\Approved");


////////////////////////////////////////////////////////////////////////
// DLL を OLE によってアンロードできるようにするかどうかを指定します。
STDAPI DllCanUnloadNow(void)
{
#ifdef _MERGE_PROXYSTUB
    HRESULT hr = PrxDllCanUnloadNow();
    if (hr != S_OK)
        return hr;
#endif
    return _AtlModule.DllCanUnloadNow();
}


////////////////////////////////////////////////////////////////////////
// 要求された型のオブジェクトを作成するクラス ファクトリを返します。
STDAPI DllGetClassObject(REFCLSID rclsid, REFIID riid, LPVOID* ppv)
{
#ifdef _MERGE_PROXYSTUB
    if (PrxDllGetClassObject(rclsid, riid, ppv) == S_OK)
        return S_OK;
#endif
    return _AtlModule.DllGetClassObject(rclsid, riid, ppv);
}


////////////////////////////////////////////////////////////////////////
// DllRegisterServer - エントリをシステム レジストリに追加します。
STDAPI DllRegisterServer(void)
{
    // オブジェクトを登録します
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
		LPOLESTR pole;
		StringFromCLSID(CLSID_CommArchiver, &pole); reg.SetStringValue(pole, _T("lsarcsvr CommArchiver")); CoTaskMemFree(pole);
		StringFromCLSID(CLSID_ArcManager, &pole); reg.SetStringValue(pole, _T("lsarcsvr ArcManager")); CoTaskMemFree(pole);
		reg.Close();
	}
	return hr;
}


////////////////////////////////////////////////////////////////////////
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
		LPOLESTR pole;
		StringFromCLSID(CLSID_CommArchiver, &pole); reg.DeleteValue(pole); CoTaskMemFree(pole);
		StringFromCLSID(CLSID_ArcManager, &pole); reg.DeleteValue(pole); CoTaskMemFree(pole);
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


