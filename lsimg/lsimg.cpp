// lsimg.cpp : DLL アプリケーションのエントリ ポイントを定義します。
//

#include "stdafx.h"
#include <ole2.h>
#include "lsimg.h"


///////////////////////////////////////////////////////
//

HINSTANCE ghInst = NULL;
PROGRESSCALLBACK gpfnCallback = NULL;
BOOL gbEnableCallback = TRUE;

static BOOL InitControls();
static DWORD sdwVersion = 0x600;

///////////////////////////////////////////////////////
// Initialize Library


BOOL APIENTRY DllMain(HINSTANCE hDLL, DWORD dwReason, LPVOID lpReserved)
{
	switch (dwReason) {
		case DLL_PROCESS_ATTACH:
			ghInst = hDLL;
			DisableThreadLibraryCalls(hDLL);
//@			OleInitialize(NULL);
			InitControls();
			break;

		case DLL_THREAD_ATTACH:
			break;

		case DLL_THREAD_DETACH:
			break;

		case DLL_PROCESS_DETACH:
//@			OleUninitialize();
			break;
	}
	return TRUE;
}

BOOL DoProgressCallback(int msg, int param)
{
	if (gpfnCallback && gbEnableCallback)
		return (*gpfnCallback)(msg, param);
	return TRUE;
}

extern "C"
void WINAPI imgSetCallback(PROGRESSCALLBACK pfn)
{
	if (!pfn || IsBadCodePtr((FARPROC)pfn))
		gpfnCallback = NULL;
	else
		gpfnCallback = pfn;
}

extern "C"
void WINAPI imgEnableCallback(BOOL bEnable)
{
	gbEnableCallback = bEnable;
}

DWORD imgGetVersion()
{
	return sdwVersion;
}

extern "C"
void WINAPI imgSetVersion(DWORD version)
{
	sdwVersion = version;
}


LRESULT CALLBACK MifViewWndFn(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
LRESULT CALLBACK PreViewWndFn(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
LRESULT CALLBACK ShadeWndFn(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
LRESULT CALLBACK GraphWndFn(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
LRESULT CALLBACK GradationWndFn(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
LRESULT CALLBACK ViewWndFn(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);

static BOOL InitControls()
{
	WNDCLASS wc;
	memset(&wc, 0, sizeof(WNDCLASS));

	wc.hInstance        = ghInst;
	wc.hCursor          = LoadCursor(0, IDC_ARROW);
	wc.hIcon            = NULL;
	wc.lpszMenuName     = NULL;
	wc.lpszClassName 	= WC_PREVIEW;
	wc.lpfnWndProc   	= PreViewWndFn;
	wc.hbrBackground 	= (HBRUSH)(COLOR_WINDOW + 1);
	wc.style         	= CS_HREDRAW | CS_VREDRAW | CS_GLOBALCLASS | CS_DBLCLKS;
	wc.cbClsExtra    	= 0;
	wc.cbWndExtra    	= 0;
	if (!RegisterClass(&wc))
		return FALSE;

	wc.lpszClassName	= WC_MIFGRAPH;
	wc.style            = CS_HREDRAW | CS_VREDRAW | CS_GLOBALCLASS | CS_OWNDC;
	wc.lpfnWndProc		= GraphWndFn;
	wc.hbrBackground	= GetSysColorBrush(COLOR_3DFACE);
	wc.cbWndExtra		= 16;
	if (!RegisterClass(&wc))
		return FALSE;

	return TRUE;
}

