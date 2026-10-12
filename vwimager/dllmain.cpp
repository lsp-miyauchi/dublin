////////////////////////////////////////////////////////////////////////
// dllmain.cpp

#include "stdafx.h"

void InitApplication(HINSTANCE hInst);
void ExitApplication();

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
