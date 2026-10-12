// linkinfo.cpp : implementation file
//

#include "stdafx.h"
#include "fopstd.h"

////////////////////////////////////////////////////////////////////////
//

static BOOL lckGetInfo(IShellLink* ps, IPersistFile* pp, LPCTSTR pszName, LPFSLINKINFO pli)
{
	if (FAILED(pp->Load(pszName, STGM_READ)))
		return FALSE;

	if (FAILED(ps->GetPath(pli->szTargetName, ARRAYSIZE(pli->szTargetName), &pli->wfd, 0)))
		pli->szTargetName[0] = '\0';

	if (FAILED(ps->GetWorkingDirectory(pli->szWorkDir, ARRAYSIZE(pli->szWorkDir))))
		pli->szWorkDir[0] = '\0';

	if (FAILED(ps->GetDescription(pli->szDescription, ARRAYSIZE(pli->szDescription))))
		pli->szDescription[0] = '\0';

	return TRUE;
}

////////////////////////////////////////////////////////////////////////
//

extern "C"
BOOL WINAPI fopParseLinkInfo(LPCTSTR pszName, FSLINKINFO* pli)
{
	BOOL bOK = FALSE;
	IShellLink* psl;
	HRESULT hr = CoCreateInstance(CLSID_ShellLink, NULL, CLSCTX_INPROC_SERVER, IID_IShellLink, (LPVOID*)&psl);
	if (SUCCEEDED(hr)) {
		IPersistFile* ppf;
		hr = psl->QueryInterface(IID_IPersistFile, (LPVOID*)&ppf);
		if (SUCCEEDED(hr)) {
			bOK = lckGetInfo(psl, ppf, pszName, pli);
			ppf->Release();
		}
		psl->Release();
	}
	return bOK;
}
