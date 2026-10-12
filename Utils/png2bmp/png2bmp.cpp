// png2bmp.cpp : コンソール アプリケーションのエントリ ポイントを定義します。
//

#include "stdafx.h"
#include <shlwapi.h>

#pragma comment(lib, "shlwapi.lib")

void png2bmp(LPCTSTR pszPng, LPTSTR pszBmp);
void bmp2png(LPCTSTR pszBmp, LPTSTR pszPng);

void usage()
{
	_tprintf(_T("png2bmp:\n"));
	_tprintf(_T("   png -> bmp  <[in]png_file> [<[out]bmp_file>]\n"));
	_tprintf(_T("   bmp -> png  <[in]bmp_file> [<[out]png_file>]\n"));
}

void _tmain(int argc, _TCHAR* argv[])
{
	if (argc < 2) {
		usage();
		return;
	}

	TCHAR* p = PathFindExtension(argv[1]);
	if (!p) {
		usage();
		return;
	}

	TCHAR png[MAX_PATH * 2], bmp[MAX_PATH * 2];
	if (!lstrcmpi(p, _T(".png"))) {
		lstrcpy(png, argv[1]);
		if (argc > 2)
			lstrcpy(bmp, argv[2]);
		else {
			lstrcpy(bmp, argv[1]);
//@			lstrcat(bmp, _T(".bmp"));
			PathRenameExtension(bmp, _T(".bmp"));
		}
		_tprintf(_T("png -> bmp\n"));
		png2bmp(png, bmp);
	} else if (!lstrcmpi(p, _T(".bmp"))) {
		lstrcpy(bmp, argv[1]);
		if (argc > 2)
			lstrcpy(png, argv[2]);
		else {
			lstrcpy(png, argv[1]);
			lstrcat(png, _T(".png"));
		}
		_tprintf(_T("bmp -> png\n"));
		bmp2png(bmp, png);
	} else {
		usage();
	}
	return;
}

