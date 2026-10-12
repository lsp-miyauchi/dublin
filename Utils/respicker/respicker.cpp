// これは、アプリケーション ウィザードを使って生成された、VC++ アプリケーション プロジェクトの 
// メイン プロジェクト ファイルです。

#include "stdafx.h"

#pragma comment(lib, "shlwapi.lib")

#define IS_RESID(a)			(HIWORD((a)) == 0)
#define WIDTHBYTES(bits)    (((bits) + 31) / 32 * 4)
#define DIB_HEADER_MARKER   ((WORD)('M' << 8) | 'B')


BOOL ExportIconFromFile(LPCTSTR pszName, LPCTSTR pszTo);



BOOL CALLBACK EnumResTypeProc(HMODULE hmod, LPTSTR pszType, LONG_PTR lParam)
{
	CSimpleArray<LPTSTR>* pa = (CSimpleArray<LPTSTR>*)lParam;
	if (IS_RESID(pszType))
		pa->Add(pszType);
	else
		pa->Add(StrDup(pszType));
	return TRUE;
}

int dib_numcolors(void* pv)
{
	int colors = ((LPBITMAPINFOHEADER)pv)->biClrUsed;
	if (!colors) {
		int bits = ((LPBITMAPINFOHEADER)pv)->biBitCount;
		colors = (bits <= 8) ? (1 << bits) : 0;
	}
	return colors;
}

int dib_palsize(void* pdib)
{
	int size = dib_numcolors(pdib) * sizeof(RGBQUAD);
	if (((BITMAPINFOHEADER*)pdib)->biCompression == BI_BITFIELDS)
		size += sizeof(DWORD) * 3;
	return size;
}

void* get_resdata_ptr(HMODULE hmod, LPCTSTR pszType, LPCTSTR pszName, DWORD* pdwSize)
{
	HRSRC hres = FindResource(hmod, pszName, pszType);
	if (hres) {
		HGLOBAL hmem = LoadResource(hmod, hres);
		if (hmem) {
			*pdwSize = SizeofResource(hmod, hres);
			return LockResource(hmem);
		}
	}
	return NULL;
}

BOOL CALLBACK EnumResNameProc(HMODULE hmod, LPCTSTR pszType, LPTSTR pszName, LONG_PTR lParam)
{
	bool dumpIt = false;
	CString s;
	if (pszType == RT_CURSOR) { s = _T("CURSOR"); }
	else if (pszType == RT_ICON) { s = _T("ICON"); }
	else if (pszType == RT_MENU) { s = _T("MENU"); }
	else if (pszType == RT_DIALOG) { s = _T("DIALOG"); }
	else if (pszType == RT_STRING) { s = _T("STRING"); }
	else if (pszType == RT_FONTDIR) { s = _T("FONTDIR"); }
	else if (pszType == RT_FONT) { s = _T("FONT"); }
	else if (pszType == RT_ACCELERATOR) { s = _T("ACCELERATOR"); }
	else if (pszType == RT_MESSAGETABLE) { s = _T("MESSAGETABLE"); }
	else if (pszType == RT_VERSION) { s = _T("VERSION"); }
	else if (pszType == RT_DLGINCLUDE) { s = _T("DLGINCLUDE"); }
	else if (pszType == RT_PLUGPLAY) { s = _T("PLUGPLAY"); }
	else if (pszType == RT_VXD) { s = _T("VXD"); }
	else if (pszType == RT_MANIFEST) { s = _T("MANIFEST"); }
	else if (pszType == RT_GROUP_CURSOR) { s = _T("GROUP_CURSOR"); }
	else if (pszType == RT_GROUP_ICON) { s = _T("GROUP_ICON"); }
	else if (pszType == RT_ANICURSOR) { s = _T("ANIMATION"); }
	else if (pszType == RT_ANIICON) { s = _T("ANIICON"); }
	else if (pszType == RT_HTML) { s = _T("HTML"); }
	else if (pszType == RT_MANIFEST) { s = _T("MANIFEST"); }
	else if (pszType == RT_RCDATA) { s = _T("RCDATA"); dumpIt = true;  }
	else if (pszType == RT_BITMAP) { s = _T("BITMAP");  dumpIt = true; }
	else { s.Format(_T("** UNKNOWNTYPE: %d **"), pszType);  dumpIt = true; }

	if (!dumpIt) {
		_tprintf(_T("skip, resource type = %s\n"), s.GetString());
		return TRUE;
	}

	TCHAR buf[MAX_PATH * 2];
	if (IS_RESID(pszName))
		_stprintf(buf, _T("%s_(%d)"), lParam, pszName);
	else
		_stprintf(buf, _T("%s_%s"), lParam, pszName);

_tprintf(_T("%s\n"), buf);

	HANDLE hf;
	DWORD resSize, tmp;
	void* pv = get_resdata_ptr(hmod, pszType, pszName, &resSize);
	if (!pv || resSize < 1)
		return TRUE;

	if (pszType == RT_BITMAP) {
		lstrcat(buf, _T(".bmp"));
		hf = CreateFile(buf, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_ARCHIVE, NULL);
		if (hf != INVALID_HANDLE_VALUE) {
			DWORD hdrSize;

			BITMAPINFOHEADER bi;
			memcpy(&bi, pv, sizeof(bi));

			BITMAPFILEHEADER header;
			memset(&header, 0, sizeof(header));

			header.bfType = DIB_HEADER_MARKER;
			int palSize = dib_palsize(&bi);
			hdrSize = bi.biSize + palSize;
			if (bi.biCompression == BI_RLE8 || bi.biCompression == BI_RLE4)
				hdrSize += bi.biSizeImage;
			else {
				tmp = WIDTHBYTES(bi.biWidth * bi.biBitCount) * bi.biHeight;
				hdrSize += tmp;
				bi.biSizeImage = tmp;
			}

			header.bfSize = hdrSize + sizeof(BITMAPFILEHEADER);
			header.bfOffBits = sizeof(BITMAPFILEHEADER) + bi.biSize + palSize;

			WriteFile(hf, &header, sizeof(header), &tmp, NULL);
			WriteFile(hf, &bi, sizeof(bi), &tmp, NULL);
			WriteFile(hf, (BYTE*)pv + sizeof(bi), resSize - sizeof(bi), &tmp, NULL);
			SetEndOfFile(hf);
			CloseHandle(hf);
		}
	}
	else {
		if ((*((DWORD*)pv) & 0xF0FFFFFF) == 0xE0FFD8FF)
			lstrcat(buf, _T(".jpg"));
		else if (*((DWORD*)pv) == 0x38464947 && (((BYTE*)pv)[4] == '7' || ((BYTE*)pv)[4] == '9'))
			lstrcat(buf, _T(".gif"));
		else if (*((DWORD*)pv) == 0x474E5089 && *((DWORD*)((BYTE*)pv + 4)) == 0x0A1A0A0D)
			lstrcat(buf, _T(".png"));
		else if (((BITMAPFILEHEADER*)pv)->bfType == 0x4D42 &&
				 ((BITMAPFILEHEADER*)pv)->bfSize != 0 &&
				 ((BITMAPFILEHEADER*)pv)->bfReserved1 == 0 &&
				 ((BITMAPFILEHEADER*)pv)->bfReserved2 == 0)
			lstrcat(buf, _T(".bmp"));
		else if (*((DWORD*)pv) == 0x46464952)
			lstrcat(buf, _T(".avi"));
		else {
			int len = lstrlen(buf);
			if (IS_RESID(pszType))
				_stprintf(&buf[len], _T("_%d.bin"), pszType);
			else
				_stprintf(&buf[len], _T("_%s.bin"), pszType);
		}

		hf = CreateFile(buf, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_ARCHIVE, NULL);
		if (hf != INVALID_HANDLE_VALUE) {
			WriteFile(hf, pv, resSize, &tmp, NULL);
			SetEndOfFile(hf);
			CloseHandle(hf);
		}
	}
	return TRUE;
}

#ifdef _DEBUG
	static const TCHAR sszShellFile[] = _T("e:\\sample\\shell32.dll");
#endif

int _tmain(int argc, TCHAR** argv)
{
	bool bExtractIcon = false;
	LPCTSTR pName = NULL;
	if (argc < 2) {
#ifndef _DEBUG
		_tprintf(_T("respick <target_exe_dll_file>\n"));
		return -1;
#else
		pName = sszShellFile;
#endif
	} else {
		for (int n = 1; n < argc; n++) {
			CString s = argv[n];
			if (s[0] == _T('/')) {
				if (s == _T("/icon")) {
					bExtractIcon = true;
				}
			} else {
				pName = argv[1];
			}
		}
	}

	if (!PathFileExists(pName)) {
		_tprintf(_T("file not found: %s\n"), pName);
		return -1;
	}

	HMODULE hmod = LoadLibraryEx(pName, NULL, LOAD_LIBRARY_AS_DATAFILE);
	if (!hmod) {
		hmod = LoadLibraryEx(pName, NULL, DONT_RESOLVE_DLL_REFERENCES);
		if (!hmod) {
			_tprintf(_T("file cannot loaded: %s\n"), pName);
			ShowError(GetLastError());
			return -1;
		}
	}

	TCHAR ini[MAX_PATH * 2];
	GetModuleFileName(NULL, ini, ARRAYSIZE(ini));
	PathRenameExtension(ini, _T(".ini"));

	TCHAR path[MAX_PATH];
	if (GetPrivateProfileString(_T("Setting"), _T("OutPath"), _T(""), path, ARRAYSIZE(path), ini) < 1)
		lstrcpy(path, pName);
	else
		PathAppend(path, PathFindFileName(pName));

	if (!PathIsDirectory(path) && !CreateDirectory(path, NULL)) {
		_tprintf(_T("cannot create directory: %s\n"), path);
		ShowError(GetLastError());
		FreeLibrary(hmod);
		return -1;
	}

	if (bExtractIcon) {
		ExportIconFromFile(pName, path);
	} else {
		// リソースタイプを列挙
		CSimpleArray<LPTSTR> array;
		if (EnumResourceTypes(hmod, EnumResTypeProc, (LONG_PTR)&array)) {
			for (int i = 0; i < array.GetSize(); i++) {
				LPTSTR pszType = array[i];
				if (pszType) {
					EnumResourceNames(hmod, pszType, EnumResNameProc, (LONG_PTR)path);
					if (!IS_RESID(pszType))
						LocalFree(pszType);
				}
			}
		} else {
			ShowError(GetLastError());
		}
	}

	FreeLibrary(hmod);

MessageBox(NULL, _T("completed."), _T("respick"), MB_OK);

	return 0;
}
