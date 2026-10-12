// Recent.cpp : implementation file
//

#include "stdafx.h"
#include "fopstd.h"

extern "C"
void WINAPI fopEmptyRecentFiles()
{
	TCHAR path[MAX_PATH * 2];
	if (SHGetFolderPath(GetFocus(), CSIDL_RECENT, NULL, SHGFP_TYPE_CURRENT, path) == S_OK) {
		// ファイルを検索
		CSimpleArray<CString> arrFile;
		CSimpleArray<CString> arrFolder;
		EnumFileDir(path, gszAst, arrFile, arrFolder);

		TCHAR buf[MAX_PATH * 2];
		int cnt = (int)arrFile.GetSize();
		for (int i = 0; i < cnt; i++) {
			PathCombine(buf, path, arrFile[i]);
			fopRemoveFileNoConfirm(buf);
		}
	}
}
