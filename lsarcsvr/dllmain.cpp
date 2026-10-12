// dllmain.cpp : DllMain の実装

#include "stdafx.h"
#include "resource.h"
#include "lsarcsvr_i.h"
#include "dllmain.h"
#include "dlldatax.h"
#include "common.h"

COSPlatform gosp;
ClsarcsvrModule _AtlModule;

// DLL エントリ ポイント
extern "C" BOOL WINAPI DllMain(HINSTANCE hInstance, DWORD dwReason, LPVOID lpReserved)
{
#ifdef _MERGE_PROXYSTUB
	if (!PrxDllMain(hInstance, dwReason, lpReserved))
		return FALSE;
#endif
	hInstance;
	if (dwReason == DLL_PROCESS_ATTACH)
		AddMyPathEnvironment();
	return _AtlModule.DllMain(dwReason, lpReserved); 
}
