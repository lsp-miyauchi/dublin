// dllmain.cpp : DLL アプリケーション用のエントリ ポイントを定義します。
//

#include "stdafx.h"


HINSTANCE ghInst = NULL;

BOOL APIENTRY DllMain( HINSTANCE hModule, 
                       DWORD  reason, 
                       LPVOID lpReserved
					 )
{
	if (reason == DLL_PROCESS_ATTACH)
	{
		DisableThreadLibraryCalls(hModule);
		ghInst = hModule;
	}
    return TRUE;
}
