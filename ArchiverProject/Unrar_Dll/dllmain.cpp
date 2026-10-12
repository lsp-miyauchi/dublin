// dllmain.cpp : DLL アプリケーションのエントリ ポイントを定義します。
#include "stdafx.h"

CUnrarModule _G;

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
	// このバージョンは 95/98/ME では動作しません
//	if (!_G.OsNT()) return FALSE;

	::DefWindowProc(NULL, 0, 0, 0L);
	_G.Init(NULL, hModule);

	return TRUE;
}

static void ExitInstance()
{
	_G.Term();
}
