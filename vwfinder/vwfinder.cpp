// vwfinder.cpp : DLL アプリケーションのエントリ ポイントを定義します。
//

#include "stdafx.h"
#include "resource.h"
#include "common.h"
#include "cdll.h"

int (STDAPICALLTYPE* rStringCompare)(LPCWSTR, LPCWSTR);

static void InitApplication(HINSTANCE hInst);
static void ExitApplication();

BOOL APIENTRY DllMain( HANDLE hModule, DWORD  ul_reason_for_call,  LPVOID lpReserved)
{
	switch (ul_reason_for_call) {
	case DLL_PROCESS_ATTACH:
		InitApplication((HINSTANCE)hModule);
		break;

	case DLL_THREAD_ATTACH:
		break;

	case DLL_THREAD_DETACH:
		break;

	case DLL_PROCESS_DETACH:
		ExitApplication();
		break;
	}
	return TRUE;
}

static void InitApplication(HINSTANCE hInst)
{
	::CoInitializeEx(NULL, COINIT_MULTITHREADED);
	::DefWindowProc(NULL, 0, 0, 0L);
	AtlInitCommonControls(ICC_COOL_CLASSES | ICC_WIN95_CLASSES | ICC_DATE_CLASSES);
	_Module.Init(NULL, hInst);

	// shlwapi の StrCmpLogicalW
	(FARPROC&)rStringCompare = GetProcAddress(GetModuleHandle(_T("shlwapi.dll")), "StrCmpLogicalW");
	if (!rStringCompare)
		rStringCompare = StrCmpI;
}

static void ExitApplication()
{
	_Module.Term();
	::CoUninitialize();
}
