///////////////////////////////////////////////////////
// dllmain.cpp

#include <windows.h>
#include "debug.h"

///////////////////////////////////////////////////////
// Initialize Library

void initcrc16tbl();
void initcrc32tbl();

CConsole dbg;

extern "C"
BOOL WINAPI DllMain(HINSTANCE hDLL, DWORD dwReason, LPVOID lpReserved)
{
	switch (dwReason) {
	case DLL_PROCESS_ATTACH:
		initcrc16tbl();
		initcrc32tbl();
		break;
	case DLL_THREAD_ATTACH:
		break;
	case DLL_THREAD_DETACH:
		break;
	case DLL_PROCESS_DETACH:
		break;
	}
	return TRUE;
}
