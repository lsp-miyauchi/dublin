// lsptwain.cpp : DLL アプリケーション用のエントリ ポイントを定義します。
//

#include "stdafx.h"

HINSTANCE ghInst;

BOOL APIENTRY DllMain( HANDLE hModule, 
                       DWORD  ul_reason_for_call, 
                       LPVOID lpReserved
					 )
{
	if (ul_reason_for_call == DLL_PROCESS_ATTACH)
	{
		ghInst = (HINSTANCE)hModule;
	}
    return TRUE;
}
