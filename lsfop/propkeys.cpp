#include "stdafx.h"
#include "fopstd.h"

///////////////////////////////////////////////////////////////////////
//

bool GetItemProperty(LPCTSTR name, const SHCOLUMNID* pscid, VARIANT* pvr)
{
	bool ok = false;
	IShellFolder2* psf2;
	LPITEMIDLIST pidlFq = ILCreateFromPath(name);
	if (pidlFq) {
		LPCITEMIDLIST pidl;
		if (SHBindToParent(pidlFq, IID_IShellFolder2, (void**)&psf2, &pidl) == S_OK) {
			ok = (psf2->GetDetailsEx(pidl, pscid, pvr) == S_OK);
			psf2->Release();
		}
		CoTaskMemFree(pidlFq);
	}
	return ok;
}

static void vt2FileTime(DOUBLE vtime, FILETIME* pft, bool bLocalTime)
{
	SYSTEMTIME st;
	VariantTimeToSystemTime(vtime, &st);
	FILETIME ft;
	SystemTimeToFileTime(&st, &ft);
	if (bLocalTime) {
		FileTimeToLocalFileTime(&ft, pft);
	}
	else {
		*pft = ft;
	}
}
	
void VariantTimeToFileTime(VARIANT* pvr, FILETIME* pft, bool bLocalTime)
{
	vt2FileTime(pvr->date, pft, bLocalTime);
}

void PropVariantTimeToFileTime(PROPVARIANT* ppvr, FILETIME* pft, bool bLocalTime)
{
	vt2FileTime(ppvr->date, pft, bLocalTime);
}
