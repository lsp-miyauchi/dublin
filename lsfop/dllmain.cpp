// dllmain.cpp : DllMain の実装

#include "stdafx.h"
#include "resource.h"
#include "lsfop_i.h"
#include "dllmain.h"
#include "dlldatax.h"
#include "fopstd.h"

CLsFopModule _AtlModule;

////////////////////////////////////////////////////////////////////////
// DLL エントリ ポイント

extern "C" BOOL WINAPI DllMain(HINSTANCE hInstance, DWORD dwReason, LPVOID lpReserved)
{
#ifdef _MERGE_PROXYSTUB
	if (!PrxDllMain(hInstance, dwReason, lpReserved))
		return FALSE;
#endif
	hInstance;
	BOOL bOK = _AtlModule.DllMain(dwReason, lpReserved);
	if (dwReason == DLL_PROCESS_ATTACH) {
		DisableThreadLibraryCalls(hInstance);
		AtlInitCommonControls(	ICC_WIN95_CLASSES |
							ICC_DATE_CLASSES |
							ICC_USEREX_CLASSES |
							ICC_COOL_CLASSES	);
	}
	return bOK;
}
