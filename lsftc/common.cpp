// lstus.cpp : DLL エクスポートの実装です。

#include "stdafx.h"
#include "resource.h"
#include <math.h>

BOOL (WINAPI* __FileTimeToDosDateTime)(CONST FILETIME*, LPWORD, LPWORD);
BOOL (WINAPI* __DosDateTimeToFileTime)(WORD, WORD, FILETIME*);
LONG (WINAPI* __CompareFileTime)(CONST FILETIME*, CONST FILETIME*);
HRESULT (WINAPI* __StringFromCLSID)(REFCLSID, OLECHAR**);
HRESULT (WINAPI* __CLSIDFromString)(OLECHAR*, CLSID*);
HRESULT (WINAPI* __CoCreateGuid)(GUID* pguid);

const char rszFileTimeToDosDateTime[] = "emiTetaDsoDoTemiTeliF";
const char rszDosDateTimeToFileTime[] = "emiTeliFoTemiTetaDsoD";
const char rszCompareFileTime[] = "emiTeliFerapmoC";
const char rszStringFromCLSID[] = "DISLCmorFgnirtS";
const char rszCLSIDFromString[] = "gnirtSmorFDISLC";
const char rszCoCreateGuid[] = "diuGetaerCoC";


////////////////////////////////////////////////////////////////////////
//

void InitFunctions()
{
	HMODULE hmod = GetModuleHandle(_T("kernel32.dll"));
	CStringA str = rszFileTimeToDosDateTime; str.MakeReverse();
	(FARPROC&)__FileTimeToDosDateTime = GetProcAddress(hmod, str);
	str = rszDosDateTimeToFileTime; str.MakeReverse();
	(FARPROC&)__DosDateTimeToFileTime = GetProcAddress(hmod, str);
	str = rszCompareFileTime; str.MakeReverse();
	(FARPROC&)__CompareFileTime = GetProcAddress(hmod, str);

	hmod = GetModuleHandle(_T("ole32.dll"));
	str = rszStringFromCLSID; str.MakeReverse();
	(FARPROC&)__StringFromCLSID = GetProcAddress(hmod, str);
	str = rszCLSIDFromString; str.MakeReverse();
	(FARPROC&)__CLSIDFromString = GetProcAddress(hmod, str);
	str = rszCoCreateGuid; str.MakeReverse();
	(FARPROC&)__CoCreateGuid = GetProcAddress(hmod, str);
}

////////////////////////////////////////////////////////////////////////
//

bool MakeRegSecString(REFCLSID rc, CString& sec)
{
	OLECHAR* clsid;
	if (__StringFromCLSID(rc, &clsid) == S_OK) {
		sec.Format(_T("Software\\Classes\\CLSID\\%s\\Categories"), clsid);
		CoTaskMemFree(clsid);
		return true;
	}
	return false;
}

void BuildRegSecData(GUID g, DWORD dt, CString& str)
{
	g.Data1 = dt;
	OLECHAR* clsid;
	if (__StringFromCLSID(g, &clsid) == S_OK) {
		str = clsid;
		CoTaskMemFree(clsid);
	}
}

DWORD PickRegSecData(LPCTSTR psz)
{
	DWORD dt = 0;
	CLSID clsid;
	if (__CLSIDFromString((OLECHAR*)psz, &clsid) == NOERROR)
		dt = clsid.Data1;
	return dt;
}

bool GetCurrentFileTime(FILETIME* pft)
{
	TCHAR path[MAX_PATH], temp[MAX_PATH];
	GetTempPath(MAX_PATH, path);
	if (GetTempFileName(path, _T("tus"), 0, temp)) {
		HANDLE hf = CreateFile(temp, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_TEMPORARY | FILE_FLAG_DELETE_ON_CLOSE, NULL);
		if (hf != INVALID_HANDLE_VALUE) {
			GetFileTime(hf, pft, NULL, NULL);
			CloseHandle(hf);
			return true;
		}
	}
	return false;
}

long GetDeltaDate(const FILETIME* ps, const FILETIME* pt)
{
	ULARGE_INTEGER s, t;
	s.LowPart = ps->dwLowDateTime;
	s.HighPart = ps->dwHighDateTime;
	t.LowPart = pt->dwLowDateTime;
	t.HighPart = pt->dwHighDateTime;
	ULONGLONG delta = t.QuadPart - s.QuadPart;
	delta /= 10000000;
	return (long)floor((double)(long)delta / 86400.0);
}
