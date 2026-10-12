#include <windows.h>
#include <windowsx.h>
#include <stdio.h>
#include <io.h>

void jp2_init();

BOOL APIENTRY DllMain(HINSTANCE hinstDll, DWORD fdwReason, LPVOID plvReserved)
{
	if (fdwReason == DLL_PROCESS_ATTACH)
	{
		DisableThreadLibraryCalls(hinstDll);
		jp2_init();
	}
	return TRUE;
}
