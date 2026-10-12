// txtinput.cpp : DLL アプリケーションのエントリ ポイントを定義します。
//

#include "stdafx.h"

CAppModule _Module;
CConsole dbg;

BOOL APIENTRY DllMain( HANDLE hModule, 
                       DWORD  ul_reason_for_call, 
                       LPVOID lpReserved
					 )
{
	switch (ul_reason_for_call)
	{
	case DLL_PROCESS_ATTACH:
		lccGetVersion();
		_Module.Init(NULL, (HINSTANCE)hModule);
		break;
	case DLL_THREAD_ATTACH:
		break;
	case DLL_THREAD_DETACH:
		break;
	case DLL_PROCESS_DETACH:
		_Module.Term();
		break;
	}
    return TRUE;
}
