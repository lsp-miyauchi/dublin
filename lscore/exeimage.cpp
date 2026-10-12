#include "stdafx.h"
#include <imagehlp.h>
#include "sys\stat.h"
#include "lscore.h"
#include "fnc.h"

////////////////////////////////////////////////////////////////////
//

#define MakePtr(cast, base, ofs)	((cast)((base) + (ofs)))

typedef struct _IMAGE_EXE_HEADERS {
    WORD Signature;
} IMAGE_EXE_HEADERS, *PIMAGE_EXE_HEADERS;



////////////////////////////////////////////////////////////////////
//


static BOOL __fastcall pe_dump(LPCTSTR lpName, LPLONG lpType)
{
	HANDLE hf = CreateFile(lpName, GENERIC_READ, FILE_SHARE_READ, NULL,
						OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, 0);
	if (hf == INVALID_HANDLE_VALUE )
		return FALSE;

	HANDLE hfMapping = CreateFileMapping(hf, NULL, PAGE_READONLY, 0, 0, NULL);
	if (hfMapping == 0)
	{
		CloseHandle(hf);
		return FALSE;
	}

	LPVOID lpFileBase = MapViewOfFile(hfMapping, FILE_MAP_READ, 0, 0, 0);
	if (lpFileBase == 0)
	{
		CloseHandle(hfMapping);
		CloseHandle(hf);
		return FALSE;
	}

	BOOL bOK = FALSE;
	PIMAGE_DOS_HEADER dosHeader = (PIMAGE_DOS_HEADER)lpFileBase;
	if (dosHeader->e_magic == IMAGE_DOS_SIGNATURE)
	{
		*lpType = SCS_DOS_BINARY;
		if (dosHeader->e_lfanew)
		{
			LPLONG lp = MakePtr(LPLONG, (DWORD)dosHeader, dosHeader->e_lfanew);
			if (!IsBadReadPtr(lp, sizeof(LONG)))
			{
				if (*lp == IMAGE_NT_SIGNATURE)
					*lpType = SCS_32BIT_BINARY;
				else if (LOWORD(*lp) == IMAGE_OS2_SIGNATURE)
					*lpType = SCS_WOW_BINARY;
			}
		}
		bOK = TRUE;
	}
	UnmapViewOfFile(lpFileBase);
	CloseHandle(hfMapping);
	CloseHandle(hf);

	return bOK;
}

extern "C"
BOOL WINAPI lccGetBinaryType(LPCTSTR lpName, LPLONG lpType)
{
	BOOL bOK = FALSE;
	bOK = GetBinaryType(lpName, (ULONG*)lpType);
	if (!bOK)
		bOK = pe_dump(lpName, lpType);
	return bOK;
}
