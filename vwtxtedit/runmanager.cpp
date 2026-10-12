// runmanager.cpp : main source file for vwtxtedit.dll
//

#include "stdafx.h"
#include "resource.h"
#include "padview.h"
#include "common.h"
#include "mainfrm.h"
#include "cdll.h"

CAppModule _Module;

int Run(LPTSTR pstrCmdLine, int nCmdShow = SW_SHOWDEFAULT)
{
	gdiplus_using gu;

	CMessageLoop theLoop;
	_Module.AddMessageLoop(&theLoop);

	CMainFrame wndMain;

	if (wndMain.CreateEx() == NULL) {
		ATLTRACE(_T("Main window creation failed!\n"));
		return 0;
	}

	// 初期セットアップ
	wndMain.InitialUpdate();

	// コマンドライン解析
	int optionLineNumber = -1;
	int optionCharSet = CS_AUTODETECT;
	bool optionROM = false;

	CString strFile;
	if (pstrCmdLine && *pstrCmdLine) {
		LPTSTR p = SkipTabSpace(pstrCmdLine);
		if (*p == _T('/') || *p == _T('-')) {
			CSimpleArray<CString> arr;
			int cnt = SeparateString(p, _T(' '), arr);
			for (int i = 0; i < cnt; i++) {
				CString str = arr[i];
				LPCTSTR ptr = str;
				if (*ptr == _T('/') || *ptr == _T('-')) {
					ptr++;
					switch (*ptr) {
					case _T('c'):
					case _T('C'):
						optionCharSet = StrToInt(++ptr);
						break;
					case _T('l'):
					case _T('L'):
						optionLineNumber = StrToInt(++ptr);
						break;
					case _T('r'):
					case _T('R'):
						optionROM = true;
						break;
					}
				}
				else {
					TCHAR* tmp = new TCHAR[lstrlen(ptr) + 1];
					lstrcpy(tmp, ptr);
					PathUnquoteSpaces(tmp);
					strFile = tmp;
					// オプションスイッチ以外が現れたらそこでお終い。
					break;
				}
			}
		}
		else {
			TCHAR* tmp = new TCHAR[lstrlen(p) + 1];
			lstrcpy(tmp, p);
			PathUnquoteSpaces(tmp);
			strFile = tmp;
		}
	}

	InitializeImeManager();
	wndMain.ShowWindow(nCmdShow);
	wndMain.UpdateWindow();
	wndMain.StartEdit(strFile, optionCharSet, optionLineNumber, optionROM);

	int nRet = theLoop.Run();

	TerminateImeManager();

	_Module.RemoveMessageLoop();
	return nRet;
}

/////////////////////////////////////////////////////////////////////
//

#ifdef _USRDLL

extern "C"
void WINAPI RunEditW(HWND hwnd, HINSTANCE hInst, LPWSTR lpCmdLine, int nCmdShow)
{
	Run((LPTSTR)lpCmdLine, nCmdShow);
}

extern "C"
void WINAPI RunEdit(HWND hwnd, HINSTANCE hInst, LPSTR lpCmdLine, int nCmdShow)
{
	WCHAR wz[MAX_PATH * 2];
	MultiByteToWideChar(CP_ACP, 0, lpCmdLine, -1, wz, ARRAYSIZE(wz));
	RunEditW(hwnd, hInst, wz, nCmdShow);
}

#endif	// _USRDLL
