// lsmedia.cpp : DLL アプリケーションのエントリ ポイントを定義します。
//

#include "stdafx.h"
#include "lsmedia.h"

CAppModule _Module;
DWORD gdwComCtlVersion = 0;

BOOL APIENTRY DllMain(HMODULE hDLL, DWORD  ul_reason_for_call,  LPVOID lpReserved)
{
	switch (ul_reason_for_call) {
	case DLL_PROCESS_ATTACH:
		ghInst = hDLL;
		::DefWindowProc(NULL, 0, 0, 0L);
		_Module.Init(NULL, hDLL);
		{
			// comctl32.dllのバージョン
			DWORD major, minor;
			if (FAILED(AtlGetCommCtrlVersion(&major, &minor))) {
				major = 4;
				minor = 0;
			}
			gdwComCtlVersion = MAKELONG(minor, major);
		}
		lccGetVersion();
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
