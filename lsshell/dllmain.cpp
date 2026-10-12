// dllmain.cpp : DllMain の実装

#include "stdafx.h"
#include "resource.h"
#include "lsshell_i.h"
#include "mainmodule.h"
#include "dlldatax.h"

// DLL エントリ ポイント
extern "C" BOOL WINAPI DllMain(HINSTANCE hInstance, DWORD dwReason, LPVOID lpReserved)
{
#ifdef _MERGE_PROXYSTUB
	if (!PrxDllMain(hInstance, dwReason, lpReserved))
		return FALSE;
#endif
	hInstance;

	BOOL res = _AtlModule.DllMain(dwReason, lpReserved);
	switch (dwReason) {
	case DLL_PROCESS_ATTACH:
		_AtlModule.InitInstance();
		AddMyPathEnvironment();
		break;
	case DLL_PROCESS_DETACH:
		_AtlModule.ExitInstance();
		break;
	}
	return res;


}
