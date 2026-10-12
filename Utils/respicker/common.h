#pragma once

#include "fileitem.h"

DWORD __fastcall _getfileinfo(LPCTSTR pszName, WIN32_FIND_DATA* pwfd);
BOOL __fastcall GetFileData(LPCTSTR pszName, WIN32_FIND_DATA* pwfd);
BOOL __fastcall GetFileInfo(LPCTSTR pszName, CFileInfo& fi);
BOOL __fastcall validate_makefile(LPTSTR pszName);
HANDLE __fastcall _createfile(LPCTSTR pName, DWORD fdwAccess, DWORD fdwShareMode, LPSECURITY_ATTRIBUTES psa, DWORD fdwCreate, DWORD fdwAttrsAndFlags, HANDLE hTemplateFile);
BOOL __fastcall _createdirectory(LPCTSTR pName, LPSECURITY_ATTRIBUTES psa);
BOOL __fastcall _removedirectory(LPCTSTR pszPath);
BOOL __fastcall _movefile(LPCTSTR pszExist, LPCTSTR pszNew);
void __fastcall ShowError(int nErrorCode);
_inline BOOL PathGetParent(LPCWSTR src, LPWSTR dst)
{
	lstrcpyW(dst, src);
	return PathRemoveFileSpecW(dst);
}

