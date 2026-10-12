// cabstub.cpp : アプリケーションのエントリ ポイントを定義します。
//

#include "stdafx.h"
#include "cabstub.h"

HINSTANCE ghInst = NULL;

static void ProcessExecuting(CSfxInfo* ps);
static BOOL DeleteTempFile(LPCWSTR pszName);

int APIENTRY _tWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPTSTR lpCmdLine, int nCmdShow)
{
	UNREFERENCED_PARAMETER(hPrevInstance);
	UNREFERENCED_PARAMETER(lpCmdLine);
	UNREFERENCED_PARAMETER(nCmdShow);

	ghInst = hInstance;

	// this resolves ATL window thunking problem when Microsoft Layer for Unicode (MSLU) is used
	::DefWindowProc(NULL, 0, 0, 0L);
	InitCommonControls();

	HRSRC hr = FindResource(hInstance, MAKEINTRESOURCE(ID_SFX_INFO), RT_RCDATA);
	if (hr) {
		HGLOBAL hmem = LoadResource(hInstance, hr);
		if (hmem) {
			SFX* sfx = (SFX*)LockResource(hmem);
			if (sfx) {
				if (!IsBadReadPtr(sfx, sfx->wSize)) {
					CSfxInfo si;
					si.m_lTotalUncompressedSize = sfx->lTotalUncompressedSize;
					si.m_lCabDataSize = sfx->lCabDataSize;
					si.m_wStat = sfx->wStat;
					LPWSTR p = sfx->wszString;
					if (sfx->wStat & SFX_TITLE) {
						alloc_wcscpy(&si.m_strTitle, p);
						p += wcslen(p) + 1;
					}
					if (sfx->wStat & SFX_MESSAGE) {
						alloc_wcscpy(&si.m_strMessage, p);
						p += wcslen(p) + 1;
					}
					if (sfx->wStat & SFX_PATH) {
						alloc_wcscpy(&si.m_strPath, p);
						p += wcslen(p) + 1;
					}
					if (sfx->wStat & SFX_COMMAND) {
						alloc_wcscpy(&si.m_strCommand, p);
						p += wcslen(p) + 1;
					}
					si.m_filearray.RemoveAll();

					BOOL bOK = FALSE;
					if (GetExtractPath(&si)) {
						hr = FindResource(hInstance, MAKEINTRESOURCE(ID_SFX_CABDATA), RT_RCDATA);
						if (hr) {
							hmem = LoadResource(hInstance, hr);
							if (hmem) {
								void* dataptr = LockResource(hmem);
								if (dataptr && !IsBadReadPtr(dataptr, sfx->lCabDataSize))
									bOK = fdiExtract(NULL, (BYTE*)dataptr, &si);
							}
						}
					}

					if (bOK) {
						ProcessExecuting(&si);
						if (si.m_wStat & SFX_AUTODELETE) {
							int cnt = si.m_filearray.GetSize();
							for (int i = cnt - 1; i >= 0; i--) {
								// エントリの最後から順に削除
								LPCWSTR pszFile = si.m_filearray[i];
								DeleteTempFile(pszFile);
							}
						}
					}
					si.m_filearray.RemoveAll();
				}
			}
		}
	}
	return 0;
}

static void ProcessExecuting(CSfxInfo* psi)
{
	if (psi->m_wStat & SFX_COMMAND) {
		WCHAR curpath[MAX_PATH], cmd[MAX_PATH];
		GetCurrentDirectoryW(ARRAYSIZE(curpath), curpath);

		if (psi->m_wStat & SFX_PATH) {
			SetCurrentDirectoryW(psi->m_strPath);
			xPathCombineW(cmd, psi->m_strPath, psi->m_strCommand);
		}
		else
			wcscpy(cmd, psi->m_strCommand);

		if (psi->m_wStat & SFX_AUTODELETE) {
			PROCESS_INFORMATION pi;
			if (xCreateProcess(cmd, psi->m_strPath, &pi)) {
				WaitForSingleObject(pi.hProcess, INFINITE);
				CloseHandle(pi.hThread);
				CloseHandle(pi.hProcess);
			}
		}
		else
			ShellExecuteW(NULL, NULL, cmd, NULL, psi->m_strPath, SW_SHOWNORMAL);
		SetCurrentDirectoryW(curpath);
	}
}

static BOOL DeleteTempFile(LPCWSTR pszName)
{
	BOOL bOK = FALSE;
	if (xPathFileExists(pszName)) {
		SetFileAttributesW(pszName, FILE_ATTRIBUTE_NORMAL);
		if (xPathIsDirectory(pszName))
			bOK = RemoveDirectoryW(pszName);
		else
			bOK = DeleteFileW(pszName);
	}
	return bOK;
}
