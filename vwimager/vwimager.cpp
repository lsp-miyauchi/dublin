// vwimg.cpp : DLL アプリケーションのエントリ ポイントを定義します。
//

#include "stdafx.h"
#include "resource.h"
#include "common.h"
#include "cdll.h"

void InitApplication(HINSTANCE hInst);
void ExitApplication();

void InitApplication(HINSTANCE hInst)
{
	::CoInitializeEx(NULL, COINIT_MULTITHREADED);
	::DefWindowProc(NULL, 0, 0, 0L);
	AtlInitCommonControls(ICC_COOL_CLASSES | ICC_BAR_CLASSES);
	_Module.Init(NULL, hInst);
	imgSetVersion(0x700);
}

void ExitApplication()
{
	_Module.Term();
	::CoUninitialize();
}
