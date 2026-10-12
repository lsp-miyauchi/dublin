// dllmain.cpp : DllMain の実装

#include "stdafx.h"
#include "resource.h"
#include "lsftc_i.h"
#include "dllmain.h"
#include "dlldatax.h"

ClsftcModule _AtlModule;

void InitFunctions();

// DLL エントリ ポイント
extern "C" BOOL WINAPI DllMain(HINSTANCE hInstance, DWORD dwReason, LPVOID lpReserved)
{
#ifdef _MERGE_PROXYSTUB
	if (!PrxDllMain(hInstance, dwReason, lpReserved))
		return FALSE;
#endif
	hInstance;
	BOOL ok = _AtlModule.DllMain(dwReason, lpReserved);
	if (dwReason == DLL_PROCESS_ATTACH)
		InitFunctions();
	return ok;
}
