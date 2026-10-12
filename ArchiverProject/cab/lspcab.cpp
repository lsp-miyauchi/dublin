///////////////////////////////////////////////////////
// dllmain.cpp

#include "stdafx.h"
#include "cab.h"

CCabModule _G;

WCHAR gszAst[] = L"*.*";
LPARCHIVERPROC gpfnProc = NULL;
UINT nRM_ARCEXTRACT = 0;
void (WINAPI* _fSHChangeNotify)(LONG, UINT, LPCVOID, LPCVOID);


///////////////////////////////////////////////////////
// Initialize Library

static BOOL InitInstance(HMODULE hModule);
static void ExitInstance();

BOOL APIENTRY DllMain( HMODULE hModule,
                       DWORD  ul_reason_for_call,
                       LPVOID /*lpReserved*/
					 )
{
	switch (ul_reason_for_call) {
	case DLL_PROCESS_ATTACH:
		return InitInstance(hModule);
	case DLL_THREAD_ATTACH:
		break;
	case DLL_THREAD_DETACH:
		break;
	case DLL_PROCESS_DETACH:
		ExitInstance();
		break;
	}
	return TRUE;
}

static BOOL InitInstance(HMODULE hModule)
{
	::DefWindowProc(NULL, 0, 0, 0L);
	_G.Init(NULL, hModule);

	nRM_ARCEXTRACT = RegisterWindowMessage(WM_ARCEXTRACT);

	HMODULE hmod = GetModuleHandle(L"shell32.dll");
	_fSHChangeNotify = NULL;
	if (hmod)
		(FARPROC&)_fSHChangeNotify = GetProcAddress(hmod, "SHChangeNotify");

	return TRUE;
}

static void ExitInstance()
{
	_G.Term();
}
