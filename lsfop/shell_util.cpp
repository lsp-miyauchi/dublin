///////////////////////////////////////////////////////////////////////////
// shell_util.cpp

#include "stdafx.h"
#include "fopstd.h"

#define _GetPIDLFolder(pida)				(LPCITEMIDLIST)(((LPBYTE)pida)+(pida)->aoffset[0])
#define _GetPIDLItem(pida,i)				(LPCITEMIDLIST)(((LPBYTE)pida)+(pida)->aoffset[i+1])
#define SETFormatEtc(fe, cf, med, li)\
		{\
			(fe).cfFormat = cf;\
			(fe).dwAspect = DVASPECT_CONTENT;\
			(fe).ptd = NULL;\
			(fe).tymed = med;\
			(fe).lindex = li;\
		};

HGLOBAL GetDataFromDataObject(IDataObject* pdo, CLIPFORMAT cf)
{
	HGLOBAL hmem = NULL;
	FORMATETC fe;
	SETFormatEtc(fe, cf, TYMED_HGLOBAL, -1);
	if (pdo && pdo->QueryGetData(&fe) == S_OK) {
		STGMEDIUM sm;
		if (pdo->GetData(&fe, &sm) == S_OK) {
			hmem = lccDuplicateMemoryHandle(sm.hGlobal, GMEM_SHARE | GMEM_MOVEABLE);
			ReleaseStgMedium(&sm);
		}
	}
	return hmem;
}

int GetIDListsFromDataObject(IDataObject* pdo, CSimpleIDListArray& array)
{
	PIDLIST_ABSOLUTE pidl;
	int cnt = 0;

	IShellItemArray* psia;
	if (SHCreateShellItemArrayFromDataObject(pdo, IID_PPV_ARGS(&psia)) == S_OK) {
		IEnumShellItems* pe;
		if (psia->EnumItems(&pe) == S_OK) {
			IShellItem* psi;
			ULONG celt;
			while (pe->Next(1, &psi, &celt) == S_OK && celt) {
				if (SHGetIDListFromObject(psi, &pidl) == S_OK) {
					cnt++;
					array.Add(pidl);
				}
				psi->Release();
			}
			pe->Release();
		}
		psia->Release();
		return cnt;
	}

	HGLOBAL hmem = GetDataFromDataObject(pdo, (CLIPFORMAT)RegisterClipboardFormat(CFSTR_SHELLIDLIST));
	if (hmem) {
		// IDList
		CIDA* pida = (CIDA*)GlobalLock(hmem);
		LPCITEMIDLIST pidlRoot = _GetPIDLFolder(pida);
		for (UINT u = 0; u < pida->cidl; u++) {
			LPCITEMIDLIST pidl = _GetPIDLItem(pida, u);
			LPCITEMIDLIST pidlFq;
			if (pidlRoot != NULL)	pidlFq = ::ILCombine(pidlRoot, pidl);
			else					pidlFq = ::ILClone(pidl);

			if (pidlFq != NULL) {
				array.Add(pidlFq);
				cnt++;
			}
		}
		GlobalUnlock(hmem);
		GlobalFree(hmem);
	}
	else {
		// HDROP
		HANDLE hmem = GetDataFromDataObject(pdo, CF_HDROP);
		if (hmem) {
			int num = DragQueryFile((HDROP)hmem, 0xFFFFFFFF, NULL, 0);
			TCHAR buf[MAX_PATH * 2];
			for (int i = 1; i < num; i++) {
				DragQueryFile((HDROP)hmem, i, buf, ARRAYSIZE(buf));
				pidl = ::ILCreateFromPath(buf);
				if (pidl) {
					array.Add(pidl);
					cnt++;
				}
			}
			DragFinish((HDROP)hmem);
		}
	}
	return cnt;
}
