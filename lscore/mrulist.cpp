//////////////////////////////////////////////////////////////////////
// Comctl32's MRU List Function wrapper
//

#include "stdafx.h"
#include "lscore.h"
#include "mrulist.h"

extern "C"
HANDLE WINAPI lccMRUCreateList(HKEY hKey, LPCTSTR pszSubKey, DWORD dwMaxItem)
{
	class CMRUProc* mp = new CMRUProc;
	if (!mp->Create(hKey, pszSubKey, dwMaxItem)) {
		delete mp;
		return NULL;
	}
	return (HANDLE)mp;
}

extern "C"
void WINAPI lccMRUFreeList(HANDLE hList)
{
	class CMRUProc* mp = (CMRUProc*)hList;
	if (mp)
		delete mp;
}

extern "C"
int WINAPI lccMRUAddString(HANDLE hList, LPCTSTR pszString)
{
	class CMRUProc* mp = (CMRUProc*)hList;
	if (mp)
		return mp->Add(pszString);
	return -1;
}

extern "C"
BOOL WINAPI lccMRUDelString(HANDLE hList, int nItemPos)
{
	// ���T�|�[�g
	return FALSE;
}

extern "C"
int WINAPI lccMRUEnumList(HANDLE hList, int nItemPos, LPVOID pBuffer, DWORD nSize)
{
	class CMRUProc* mp = (CMRUProc*)hList;
	if (mp)
		return mp->Enum(nItemPos, pBuffer, nSize);
	return -1;
}

extern "C"
int WINAPI lccMRUFindString(HANDLE hList, LPCTSTR pszString, LPINT pRegNum)
{
	class CMRUProc* mp = (CMRUProc*)hList;
	if (mp)
		return mp->Find(pszString, pRegNum);
	return -1;
}

extern "C"
int WINAPI lccMRUGetCount(HANDLE hList)
{
	class CMRUProc* mp = (CMRUProc*)hList;
	if (mp)
		return mp->Enum(-1, NULL, 0);
	return 0;
}
