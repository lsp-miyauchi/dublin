////////////////////////////////////////////////////////////////////////
// wrapper.cpp

#include "stdafx.h"

////////////////////////////////////////////////////////////////////////
//

static HRESULT (WINAPI* _fSHGetImageList)(int, REFIID, void**) = NULL;


////////////////////////////////////////////////////////////////////////
//

void InitShell()
{
	CDllHandle dll = GetModuleHandle(_T("shell32.dll"));
	(FARPROC&)_fSHGetImageList = dll.FindFunction((LPCSTR)727);
}

HRESULT rSHGetImageList(int iImageList, REFIID riid, void** ppv)
{
	if (_fSHGetImageList) {
		return _fSHGetImageList(iImageList, riid, ppv);
	}
	return E_NOTIMPL;
}
