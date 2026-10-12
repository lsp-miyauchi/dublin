///////////////////////////////////////////////////////
// dllmain.cpp

#include "stdafx.h"


///////////////////////////////////////////////////////
//

HINSTANCE ghInst = NULL;

///////////////////////////////////////////////////////
// Initialize Library


BOOL APIENTRY DllMain(HINSTANCE hDLL, DWORD dwReason, LPVOID lpReserved)
{
	switch (dwReason)
	{
		case DLL_PROCESS_ATTACH:
			ghInst = hDLL;
			DisableThreadLibraryCalls(hDLL);
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

