////////////////////////////////////////////////////////////////////////
// FVSHELL.CPP
#include "stdafx.h"
#include "fv7.h"
#include "mainfrm.h"


////////////////////////////////////////////////////////////////////////
//

void InitShell()
{
	// AppID
	SetCurrentProcessExplicitAppUserModelID(APP_APPID);
}


////////////////////////////////////////////////////////////////////////
//

IAssocHandler* GetRecommendedAssocHandler(LPCWSTR pszExtra)
{
	CComPtr<IEnumAssocHandlers> pe;
	if (SHAssocEnumHandlers(pszExtra, ASSOC_FILTER_RECOMMENDED, &pe) == S_OK) {
		ULONG celt;
		IAssocHandler* pah;
		if (pe->Next(1, &pah, &celt) == S_OK)
			return pah;
	}
	return NULL;
}

////////////////////////////////////////////////////////////////////////
//

int GetIDListsFromDataObject(IDataObject* pdo, CSimpleArray<PCIDLIST_ABSOLUTE>& array)
{
	PIDLIST_ABSOLUTE pidl;
	int cnt = 0;
	CComPtr<IShellItemArray> psia;
	if (SHCreateShellItemArrayFromDataObject(pdo, IID_PPV_ARGS(&psia)) == S_OK) {
		CComPtr<IEnumShellItems> pe;
		if (psia->EnumItems(&pe) == S_OK) {
			CComPtr<IShellItem> psi;
			ULONG celt;
			while (pe->Next(1, &psi, &celt) == S_OK && celt) {
				if (SHGetIDListFromObject(psi, &pidl) == S_OK) {
					cnt++;
					array.Add(pidl);
				}
				psi.Release();
			}
		}
		return cnt;
	}

	HGLOBAL hmem = GetDataFromDataObject(pdo, nRCF_SHELLIDLIST);
	if (hmem) {
		// IDList
		CIDA* pida = (CIDA*)GlobalLock(hmem);
		CIDListPtrHandle idlRoot = _GetPIDLFolder(pida);
		for (UINT u = 0; u < pida->cidl; u++) {
			CIDListPtrHandle idl = _GetPIDLItem(pida, u);
			CIDListPtr idlFq;
			if (!idlRoot.isNull())	idlFq.Combine(idlRoot, idl);
			else					idlFq.Clone(idl);

			if (!idlFq.isNull()) {
				array.Add(idlFq.Detach());
				cnt++;
			}
		}
		GlobalUnlock(hmem);
		GlobalFree(hmem);
	} else {
		// HDROP
		HDROP hdrop = (HDROP)GetDataFromDataObject(pdo, CF_HDROP);
		if (hdrop) {
			int num = DragQueryFile(hdrop, 0xFFFFFFFF, NULL, 0);
			TCHAR buf[MAX_PATH * 2];
			for (int i = 1; i < num; i++) {
				DragQueryFile(hdrop, i, buf, ARRAYSIZE(buf));
				pidl = ILCreateFromPath(buf);
				if (pidl) {
					array.Add(pidl);
					cnt++;
				}
			}
			DragFinish(hdrop);
		}
	}
	return cnt;
}

