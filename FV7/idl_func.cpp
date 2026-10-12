////////////////////////////////////////////////////////////////
// idl_func.cpp

#include "stdafx.h"
#include <stgprop.h>
#include <thumbcache.h>
#include "data.h"
#include "idl_func.h"

#define MAX_SHELLCOLUMNS		16


BOOL getlinkinfo(LPCTSTR pszName, CString& target);


////////////////////////////////////////////////////////////////
//

LPITEMIDLIST __fastcall idl_Create(UINT cbSize)
{
	LPITEMIDLIST pidl = NULL;
	pidl = (LPITEMIDLIST)CoTaskMemAlloc(cbSize);
	if (pidl)
		memset(pidl, 0, cbSize);			// zero-init for external task alloc
	ATLASSERT(pidl != NULL);
	return pidl;
}

UINT __fastcall idl_GetNumIds(LPCITEMIDLIST pidl)
{
	UINT cIds = 0;
	while (!idl_isEmpty(pidl)) {
		cIds++;
		pidl = ILGetNext(pidl);
	}
	return cIds;
}

int idl_GetInfoTip(HWND hwnd, IShellFolder* psf, LPCITEMIDLIST pidl, CString& strTip)
{
	int len = 0;
	CComPtr<IQueryInfo> pqi;
	if (psf &&
		pidl &&
		psf->GetUIObjectOf(hwnd, 1, &pidl, IID_IQueryInfo, NULL, (void**)&pqi) == NOERROR) {
		CMemTask pw;
		if (pqi->GetInfoTip(QITIPF_DEFAULT, (PWSTR*)&pw) == NOERROR) {
			strTip = pw;
			len = strTip.GetLength();
		}
	}
/* --- こっちのやり方だと情報を取得できないこともある ---
	CComQIPtr<IShellFolder2, &IID_IShellFolder2> psf2(psf);
	if (psf2.p != NULL) {
		VARIANT vr;
		SHCOLUMNID scid = PKEY_InfoTipText;
		if (psf2->GetDetailsEx(pidl, &scid, &vr) == S_OK) {
			strTip = vr.bstrVal;
			VariantClear(&vr);
		}
	}
------------------------------------------------------ */
	return len;
}

int idl_GetInfoTip(HWND hwnd, LPCITEMIDLIST pidlFq, CString& strTip)
{
	CComPtr<IShellFolder> psf;
	LPCITEMIDLIST pidl;
	if (!pidlFq && SHBindToParent(pidlFq, IID_PPV_ARGS(&psf), &pidl) != S_OK) {
		return idl_GetInfoTip(hwnd, psf, pidl, strTip);
	}
	return 0;
}

int idl_GetInfoTip(IShellItem* psi, CString& strTip)
{
	int len = 0;
	CComPtr<IQueryInfo> pqi;
	if (psi && psi->BindToHandler(NULL, BHID_SFUIObject, IID_PPV_ARGS(&pqi)) == S_OK) {
		CMemTask pw;
		if (pqi->GetInfoTip(QITIPF_DEFAULT, (PWSTR*)&pw) == S_OK) {
			strTip = pw;
			len = strTip.GetLength();
		}
	}
	return len;
}

BOOL idl_GetName(IShellFolder* psf, LPCITEMIDLIST pidl, DWORD flags, CString& strName)
{
	BOOL bOK = FALSE;
	STRRET str;
	if (psf && pidl && NOERROR == psf->GetDisplayNameOf(pidl, flags, &str)) {
		CMemTask ptr;
		if (StrRetToStr(&str, pidl, (PWSTR*)&ptr) == S_OK) {
			strName = ptr;
			bOK = TRUE;
		}
	}
	return bOK;
}

BOOL idl_GetName(LPCITEMIDLIST pidlFq, DWORD flag, CString& str)
{
	BOOL bOK = FALSE;
	if (pidlFq) {
		CComPtr<IShellFolder> psf;
		LPCITEMIDLIST pidl;
		if (SUCCEEDED(SHBindToParent(pidlFq, IID_PPV_ARGS(&psf), &pidl))) {
			bOK = idl_GetName(psf, pidl, flag, str);
		}
	}
	return bOK;
}

BOOL idl_GetPathFromIDList(LPCITEMIDLIST pidlFq, CString& strPath)
{
	BOOL ok = FALSE;
	strPath.Empty();
	if (pidlFq) {
		ok = idl_GetName(pidlFq, SHGDN_FORPARSING, strPath);
		if (ok) {
			SHFILEINFO sfi;
			sfi.dwAttributes = SFGAO_FOLDER | SFGAO_LINK;
			SHGetFileInfo((LPCTSTR)pidlFq, 0, &sfi, sizeof(sfi), SHGFI_PIDL | SHGFI_ATTR_SPECIFIED | SHGFI_ATTRIBUTES);
			if ((sfi.dwAttributes & SFGAO_LINK) && (sfi.dwAttributes & SFGAO_FOLDER)) {
				CString tmp;
				if (getlinkinfo(strPath, tmp)) {
					if (!PathIsURL(tmp)) {
						strPath = tmp;
					}
				}
			}

			WIN32_FILE_ATTRIBUTE_DATA wfad;
			if (!GetFileAttributesEx(strPath, GetFileExInfoStandard, &wfad))
				ok = FALSE;
		}
	}
	return ok;
}

LPITEMIDLIST __fastcall idl_ParseDisplayName(LPCTSTR pszFile, ULONG* pattr)
{
	LPITEMIDLIST pidlFq = NULL;
	CComPtr<IShellFolder> psf;
	if (SHGetDesktopFolder(&psf) == S_OK) {
		ULONG eat;
		if (psf->ParseDisplayName(0, 0, (LPWSTR)pszFile, &eat, &pidlFq, pattr) != S_OK)
			pidlFq = NULL;
	}
	return pidlFq;
}

LPITEMIDLIST __fastcall idl_GetFullyQualPidl(IShellFolder* psf, LPCITEMIDLIST pidl)
{
	LPITEMIDLIST pidlFq = NULL;
	if (psf && pidl) {
		CComPtr<IShellItem> psi;
		if (SHCreateShellItem(NULL, psf, pidl, &psi) == S_OK) {
			if (SHGetIDListFromObject(psi, &pidlFq) != S_OK)
				pidlFq = NULL;
		}
		if (pidlFq == NULL) {
			STRRET str;
			if (S_OK == psf->GetDisplayNameOf(pidl, SHGDN_FORPARSING, &str)) {
				CMemTask ptr;
				if (StrRetToStr(&str, pidl, (PWSTR*)&ptr) == S_OK) {
					pidlFq = idl_ParseDisplayName(ptr, NULL);
				}
			}
		}
	}
	return pidlFq;
}



////////////////////////////////////////////////////////////////
//  Takes a fully qualified pidl and returns the the relative pidl
//
//	IN: pidlFQ   - Pointer to the fully qualified ITEMIDLIST that needs to be
//				   parsed.
//	RET:         - Points to the pidl that will contain the root after parsing.
//
LPITEMIDLIST __fastcall idl_GetParent(LPCITEMIDLIST pidlFullyQual)
{
	if (!pidlFullyQual) return NULL;

	LPITEMIDLIST pidlParent = ILClone(pidlFullyQual);
	if (pidlParent) {
		if (ILRemoveLastID(pidlParent))
			return pidlParent;
		CoTaskMemFree(pidlParent);
	}
	return NULL;
}

////////////////////////////////////////////////////////////////
//  This routine takes a fully qualified pidl for a folder and returns 
//	the IShellFolder pointer for that pidl
//
//	IN: pidl - Pointer to a fully qualified ITEMIDLIST for the folder
//

IShellFolder* __fastcall idl_GetShellFolder(LPCITEMIDLIST pidl)
{
	if (!pidl) return NULL;

	IShellFolder* psf = NULL;
	IShellFolder* psfDesk;
	if (SHGetDesktopFolder(&psfDesk) == NOERROR) {
		if (idl_isDesktop(pidl))
			return psfDesk;

		if (psfDesk->BindToObject(pidl, NULL, IID_IShellFolder, (LPVOID*)&psf) != NOERROR)
			psf = NULL;
		psfDesk->Release();
	}
	if (!psf) {
		IShellFolder* psfParent;
		LPCITEMIDLIST pidlSingle;
		if (SHBindToParent(pidl, IID_PPV_ARGS(&psfParent), &pidlSingle) == S_OK) {
			if (psfParent->BindToObject(pidlSingle, 0, IID_PPV_ARGS(&psf)) != S_OK)
				psf = NULL;
			psfParent->Release();
		}
	}
	return psf;
}

IShellFolder* __fastcall idl_BindToShellFolder(IShellFolder* psf, LPCITEMIDLIST pidl)
{
	IShellFolder* psfNew;
	HRESULT hr;
	if (psf)
		hr = psf->BindToObject(pidl, 0, IID_PPV_ARGS(&psfNew));
	else
		hr = SHGetDesktopFolder(&psfNew);
	if (SUCCEEDED(hr))
		return psfNew;
	return NULL;
}

LPVOID __fastcall idl_GetUIObjectOf(HWND hwnd, LPCITEMIDLIST pidlFq, REFIID riid)
{
	void* pv = NULL;
	CComPtr<IShellFolder> psf;
	LPCITEMIDLIST pidl;
	if (SHBindToParent(pidlFq, IID_PPV_ARGS(&psf), &pidl) == S_OK) {
		if (S_OK != psf->GetUIObjectOf(hwnd, 1, &pidl, riid, 0, &pv))
			pv = NULL;
	}
	return pv;
}

LPVOID __fastcall idl_GetUIObjectOf(HWND hwnd, IShellFolder* psf, LPCITEMIDLIST pidl, REFIID riid)
{
	if (psf)
		psf->AddRef();
	else
		SHGetDesktopFolder(&psf);

	LPVOID pv = NULL;
	if (NOERROR != psf->GetUIObjectOf(hwnd, 1, (LPCITEMIDLIST*)&pidl, riid, 0, &pv))
		pv = NULL;

	psf->Release();
	return pv;
}

DROPEFFECT __fastcall idl_GetDropEffect(HWND hwnd, LPCITEMIDLIST pidl, IDataObject* pdo, DWORD keyState, DWORD effSupp)
{
	// 指定のITEMIDLISTに、DATAOBJECTがドロップされた場合の動作を得る

	DROPEFFECT eff = DROPEFFECT_NONE;
	CComPtr<IDropTarget> pdt = (IDropTarget*)idl_GetUIObjectOf(hwnd, pidl, IID_IDropTarget);
	if (pdt.p != NULL) {
		POINTL ptl;
		ptl.x = ptl.y = 0;
		eff = effSupp;
		if (pdt->DragEnter(pdo, keyState, ptl, &eff) == S_OK)
			pdt->DragLeave();
		else
			eff = DROPEFFECT_NONE;
	}
	return eff;
}

DROPEFFECT __fastcall idl_GetDropEffect(HWND hwnd, IShellFolder* psf, LPCITEMIDLIST pidl, IDataObject* pdo, DWORD keyState, DWORD effSupp)
{
	// 指定のITEMIDLISTに、DATAOBJECTがドロップされた場合の動作を得る

	DROPEFFECT eff = DROPEFFECT_NONE;
	CComPtr<IDropTarget> pdt = (IDropTarget*)idl_GetUIObjectOf(hwnd, psf, pidl, IID_IDropTarget);
	if (pdt.p != NULL) {
		POINTL ptl;
		ptl.x = ptl.y = 0;
		eff = effSupp;
		if (pdt->DragEnter(pdo, keyState, ptl, &eff) == S_OK)
			pdt->DragLeave();
		else
			eff = DROPEFFECT_NONE;
	}
	return eff;
}

DROPEFFECT __fastcall idl_GetDropEffect(HWND hwnd, IShellFolder* psf, IDataObject* pdo, DWORD keyState, DWORD effSupp)
{
	// 指定のShellFolderに、DATAOBJECTがドロップされた場合の動作を得る
	DROPEFFECT eff = DROPEFFECT_NONE;
	if (psf) {
		CComPtr<IDropTarget> pdt;
		HRESULT hr = psf->CreateViewObject(hwnd, IID_PPV_ARGS(&pdt));
		if (SUCCEEDED(hr)) {
			POINTL ptl;
			ptl.x = ptl.y = 0;
			eff = effSupp;
			if (pdt->DragEnter(pdo, keyState, ptl, &eff) == S_OK)
				pdt->DragLeave();
			else
				eff = DROPEFFECT_NONE;
		}
	}
	return eff;
}

DROPEFFECT __fastcall idl_Execute(HWND hwnd, LPCITEMIDLIST pidl, IDataObject* pdo, DWORD keyState, DROPEFFECT dropEffect, POINT pt)
{
	DROPEFFECT eff = DROPEFFECT_NONE;
	CComPtr<IDropTarget> pdt = (IDropTarget*)idl_GetUIObjectOf(hwnd, pidl, IID_IDropTarget);
	if (pdt.p != NULL) {
		POINTL ptl;
		ptl.x = pt.x;
		ptl.y = pt.y;
		eff = dropEffect;
		if (pdt->DragEnter(pdo, keyState, ptl, &eff) == S_OK) {
			// 何らかのアクションが指定されていた場合
			if (eff & (DROPEFFECT_COPY | DROPEFFECT_MOVE | DROPEFFECT_LINK)) {
				eff = dropEffect;
				pdt->Drop(pdo, keyState, ptl, &eff);
			}
			else
				pdt->DragLeave();
		}
		else
			eff = DROPEFFECT_NONE;
	}
	return eff;
}

DROPEFFECT __fastcall idl_ExecuteDirect(HWND hwnd, LPCITEMIDLIST pidl, IDataObject* pdo, DWORD keyState, DROPEFFECT dropEffect, POINT pt)
{
	DROPEFFECT eff = DROPEFFECT_NONE;
	CComPtr<IDropTarget> pdt = (IDropTarget*)idl_GetUIObjectOf(hwnd, pidl, IID_IDropTarget);
	if (pdt) {
		POINTL ptl;
		ptl.x = pt.x;
		ptl.y = pt.y;
		eff = dropEffect;
		pdt->Drop(pdo, keyState, ptl, &eff);
	}
	return eff;
}

IShellFolder* __fastcall idl_GetSpecialFolder(UINT id, LPITEMIDLIST* ppidl)
{
	if (ppidl)
		*ppidl = NULL;

	IShellFolder* psf = NULL;
	IShellFolder* psfDesktop;
	SHGetDesktopFolder(&psfDesktop);
	if (psfDesktop) {
		LPITEMIDLIST pidl;
		SHGetFolderLocation(NULL, id, NULL, 0, &pidl);
		if (id == CSIDL_DESKTOP) {
			if (ppidl)
				*ppidl = pidl;
			else
				CoTaskMemFree(pidl);
			return psfDesktop;
		}

		if (pidl) {
			psfDesktop->BindToObject(pidl, NULL, IID_IShellFolder, (LPVOID*)&psf);
			if (ppidl)
				*ppidl = pidl;
			else
				CoTaskMemFree(pidl);
		}
		psfDesktop->Release();
	}
	return psf;
}

SFGAOF __fastcall idl_GetAttributes(LPCITEMIDLIST pidlFullyQual, SFGAOF mask)
{
	if (!pidlFullyQual || !mask)
		return 0;

	SHFILEINFO sfi;
	memset(&sfi, 0, sizeof(sfi));
	sfi.dwAttributes = mask;
	SHGetFileInfo((LPCTSTR)pidlFullyQual, 0, &sfi, sizeof(sfi), SHGFI_PIDL | SHGFI_ATTR_SPECIFIED | SHGFI_ATTRIBUTES);
	return (SFGAOF)sfi.dwAttributes;
}

void __fastcall idl_ValidateCacheFolderInformation(IShellFolder* psf)
{
	if (psf) {
		SFGAOF attr = SFGAO_VALIDATE;
		psf->GetAttributesOf(0, NULL, &attr);
	}
}

void __fastcall idl_ValidateDriveInformation()
{
	LPITEMIDLIST pidlDrives;
	SHGetFolderLocation(NULL, CSIDL_DRIVES, NULL, 0, &pidlDrives);
	CComPtr<IShellFolder> psf = idl_GetShellFolder(pidlDrives);
	if (psf.p != NULL) {
		idl_ValidateCacheFolderInformation(psf);
	}
	CoTaskMemFree(pidlDrives);
}

int __fastcall idl_GetSysIconIndex(LPCITEMIDLIST pidlFullyQual, BOOL bDirty)
{
	if (pidlFullyQual) {
		SHFILEINFO sfi;
		if (bDirty) {
			TCHAR name[MAX_PATH * 2];
			if (SHGetPathFromIDList(pidlFullyQual, name) && PathIsUNC(name)) {
				SHGetFileInfo(name, FILE_ATTRIBUTE_DIRECTORY, &sfi, sizeof(sfi), SHGFI_USEFILEATTRIBUTES | SHGFI_SYSICONINDEX);
				return sfi.iIcon;
			}
		}
		SHGetFileInfo((LPCTSTR)pidlFullyQual, 0, &sfi, sizeof(sfi), SHGFI_PIDL | SHGFI_SYSICONINDEX);
		return sfi.iIcon;
	}
	return -1;
}

int __fastcall idl_GetShellColumnInfo(IShellFolder* psf, CSimpleArray<SHELLCOLUMNINFO>& arr, ULONG* pSortDefault, ULONG* pDispDefault)
{
	arr.RemoveAll();
	if (psf) {
		SHELLDETAILS sdt;
		// IShellFolder2インタフェースで試行
		CComQIPtr<IShellFolder2> psf2 = psf;
		if (psf2.p != NULL) {
			// デフォルトカラムの取得
			if (pSortDefault) *pSortDefault = 0;
			if (pDispDefault) *pDispDefault = 0;
			ULONG sort = 0;
			ULONG disp = 0;
			if (psf2->GetDefaultColumn(0, &sort, &disp) == S_OK) {
				if (pSortDefault) *pSortDefault = sort;
				if (pDispDefault) *pDispDefault = disp;
			}

			// カラム詳細を取得
			SHELLCOLUMNINFO info;
			int i = 0;
			while (i < MAX_SHELLCOLUMNS) {
				if (psf2->GetDetailsOf(NULL, i, &sdt) != S_OK)
					break;

				memset(&info, 0, sizeof(info));
				info.iCol = i;
				info.fmt = sdt.fmt;
				info.cxChar = sdt.cxChar;
				CMemTask ptr;
				if (StrRetToStr(&sdt.str, NULL, (PWSTR*)&ptr) == S_OK) {
					StrCpyN(info.title, ptr, ARRAYSIZE(info.title));
				}

				if (psf2->GetDefaultColumnState(i, &info.state) != S_OK)
					info.state = SHCOLSTATE_ONBYDEFAULT | SHCOLSTATE_TYPE_STR;
				psf2->MapColumnToSCID(i, &info.scid);
				arr.Add(info);
				i++;
			}
		}
	}
	return arr.GetSize();
}

int __fastcall FormatSystemTime(SYSTEMTIME& st, CString& str, BOOL b2col, BOOL bRelative = FALSE);

static void BSTRArray2String(VARIANT& vr, CString& str)
{
	str.Empty();
	// Get the lower and upper bound
	long ls, le;
	HRESULT hr = SafeArrayGetLBound( vr.parray, 1, &ls );
	if (FAILED(hr)) return;

	hr = SafeArrayGetUBound( vr.parray, 1, &le );
	if (FAILED(hr)) return;

	// loop
	BSTR* pbstr;
	hr = SafeArrayAccessData(vr.parray, (void HUGEP**)&pbstr);
	if (SUCCEEDED(hr)) {
		for (long i = ls; i <= le; i++) {
			if (!str.IsEmpty())
				str += L";";
			str = pbstr[i];
		}
		hr = SafeArrayUnaccessData(vr.parray);	
	}
}

static int I4Vector2Array(VARIANT& vr, CSimpleArray<long>& array)
{
	array.RemoveAll();
	// Get the lower and upper bound
	long ls, le;
	HRESULT hr = SafeArrayGetLBound( vr.parray, 1, &ls );
	if (SUCCEEDED(hr)) {
		hr = SafeArrayGetUBound( vr.parray, 1, &le );
		if (SUCCEEDED(hr)) {
			// loop
			LONG* pval;
			hr = SafeArrayAccessData(vr.parray, (void HUGEP**)&pval);
			if (SUCCEEDED(hr)) {
				for (long i = ls; i <= le; i++) {
					array.Add(pval[i]);
				}
				hr = SafeArrayUnaccessData(vr.parray);
			}
		}
	}
	return array.GetSize();
}

static int R8Vector2Array(VARIANT& vr, CSimpleArray<double>& array)
{
	array.RemoveAll();
	// Get the lower and upper bound
	long ls, le;
	HRESULT hr = SafeArrayGetLBound( vr.parray, 1, &ls );
	if (SUCCEEDED(hr)) {
		hr = SafeArrayGetUBound( vr.parray, 1, &le );
		if (SUCCEEDED(hr)) {
			// loop
			double* pval;
			hr = SafeArrayAccessData(vr.parray, (void HUGEP**)&pval);
			if (SUCCEEDED(hr)) {
				for (long i = ls; i <= le; i++) {
					array.Add(pval[i]);
				}
				hr = SafeArrayUnaccessData(vr.parray);
			}
		}
	}
	return array.GetSize();
}

static HRESULT PropVariantFromI4Array(VARIANT& vr, PROPVARIANT& pvr)
{
	ULONG count = VariantGetElementCount(vr);
	LONG* plval;
	HRESULT hr = SafeArrayAccessData(vr.parray, (void HUGEP**)&plval);
	if (SUCCEEDED(hr))
		hr = InitPropVariantFromInt32Vector(plval, count, &pvr);
	return hr;
}

static const TCHAR* sszControlPanelCategories[] = {
	_T("一般"),
	_T("デスクトップのカスタマイズ"),
	_T("ハードウェア"),
	_T("ネットワークとインターネット"),
	NULL,
	_T("システムとセキュリティ"),
	_T("時計・言語および地域"),
	_T("コンピューターの簡単操作"),
	_T("プログラム"),
	_T("ユーザー アカウント"),
};

BOOL __fastcall idl_GetPropertyStoreText(LPCITEMIDLIST pidlFq, const SHCOLUMNID& scid, CString& str, DWORDLONG* pdwl)
{
	// ---------------
	// DWORDLONGにセットする値を使ってオブジェクトのソートをおこなう
	// ---------------
	BOOL ok = FALSE;
	str.Empty();
	CComPtr<IPropertyStore> pps;
	if (pidlFq && SHGetPropertyStoreFromIDList(pidlFq, GPS_DEFAULT, IID_PPV_ARGS(&pps)) == S_OK) {
		PROPVARIANT prop = { 0 };
		if (pps->GetValue(scid, &prop) == S_OK) {
			CComPtr<IPropertyDescription> ppd;
			if (PSGetPropertyDescription(scid, IID_PPV_ARGS(&ppd)) == S_OK) {
				CMemTask ptr;
				if (ppd->FormatForDisplay(prop, PDFF_DEFAULT, (PWSTR*)&ptr) == S_OK) {
					str = ptr;
					ok = TRUE;

					if (pdwl) {
						VARTYPE vt;
						*pdwl = 0;
						if (ppd->GetPropertyType(&vt) == S_OK) {
							switch (vt &= VT_TYPEMASK) {
							case VT_BOOL: *pdwl = prop.boolVal; break;
							case VT_UI1:  *pdwl = prop.bVal; break;
							case VT_I2:   *pdwl = prop.iVal; break;
							case VT_UI2:  *pdwl = prop.uiVal; break;
							case VT_I4:   *pdwl = prop.lVal; break;
							case VT_UI4:  *pdwl = prop.ulVal;  break;
							case VT_I8:   *pdwl = prop.hVal.QuadPart;  break;
							case VT_UI8:  *pdwl = prop.uhVal.QuadPart; break;
							case VT_R8:   *pdwl = (DWORDLONG)prop.dblVal; break;
							case VT_FILETIME: *pdwl = *((DWORDLONG*)&prop.filetime); break;
							}
						}
					}
				}
			}
			PropVariantClear(&prop);
		}
	}
	return ok;
}

BOOL __fastcall idl_GetShellPropertyText(IShellFolder2* psf2, LPCITEMIDLIST pidl, const SHCOLUMNID& scid, CString& str, DWORDLONG* pdwl)
{
	// ---------------
	// DWORDLONGにセットする値を使ってオブジェクトのソートをおこなう
	// ---------------
	str.Empty();
	VARIANT vr;
	if (psf2->GetDetailsEx(pidl, &scid, &vr) != S_OK) return FALSE;

	BOOL ok = TRUE;
	*pdwl = 0;
	if (!(vr.vt & VT_ARRAY) && ((vr.vt & VT_TYPEMASK) == VT_BSTR)) {
		// 文字列
		str = vr.bstrVal;
		if (scid == PKEY_ItemType) {
			PROPVARIANT prop;
			if (SUCCEEDED(InitPropVariantFromString(str.GetString(), &prop))) {
				WCHAR wsz[256];
				if (SUCCEEDED(PSFormatForDisplay(scid, prop, PDFF_DEFAULT, wsz, ARRAYSIZE(wsz))))
					str = wsz;
				PropVariantClear(&prop);
			}
		}
	}
	else if (scid == PKEY_FreeSpace ||
			 scid == PKEY_Capacity ||
			 scid == PKEY_Size ||
			 scid == PKEY_FileAllocationSize) {
		if (vr.vt != VT_UI8)
			VariantChangeType(&vr, &vr, 0, VT_UI8);
		StrFormatByteSize64(vr.ullVal, str.GetBuffer(128), 128);
		str.ReleaseBuffer();
		*pdwl = vr.ullVal;
	}
	else if (scid == PKEY_DateModified ||
			 scid == PKEY_DateCreated ||
			 scid == PKEY_DateAccessed ||
			 scid == PKEY_DateAcquired ||
			 scid == PKEY_DateArchived ||
			 scid == PKEY_DateCompleted ||
			 scid == PKEY_DateImported ||
			 scid == PKEY_ItemDate ||
			 scid == PKEY_Document_DateCreated ||
			 scid == PKEY_Document_DatePrinted ||
			 scid == PKEY_Document_DateSaved ||
			 scid == PKEY_DueDate ||
			 scid == PKEY_EndDate) {
		SYSTEMTIME st, lt;
		VariantTimeToSystemTime(vr.date, &st);
		SystemTimeToLocalTime(&st, &lt);
		FormatSystemTime(lt, str, FALSE);
		SystemTimeToFileTime(&st, (FILETIME*)pdwl);
	}
//	else if (scid == PKEY_Category ||
//			 scid == PKEY_Devices_CategoryGroup_Desc) {
//		BSTRArray2String(vr, str);
//	}
	else if (scid == PKEY_FileAttributes) {
		lccFileAttrToStr(vr.ulVal, str.GetBuffer(64));
		str.ReleaseBuffer();
		*pdwl = vr.ulVal;
	}
	else if (scid == PKEY_ControlPanel) {
		CSimpleArray<long> arr;
		int cnt = I4Vector2Array(vr, arr);
		if (cnt > 0) {
			for (int i = 0; i < cnt; i++) {
				if (0 <= arr[i] && arr[i] < ARRAYSIZE(sszControlPanelCategories)) {
					if (!str.IsEmpty())
						str += L",";
					str += sszControlPanelCategories[arr[i]];
				}
			}
		}
	}
	else if ((vr.vt & VT_ARRAY) && (vr.vt & VT_TYPEMASK) == VT_BSTR)
		BSTRArray2String(vr, str);
/*
		else if (vr.vt & (VT_I4 | VT_UI4)) {
			CSimpleArray<long> arr;
			int cnt = I4Vector2Array(vr, arr);
			if (cnt > 0) {
				for (int i = 0; i < cnt; i++) {
					if (!str.IsEmpty())
						str += L";";
					str.AppendFormat(_T("%d"), arr[i]);
				}
			}
		}
*/
	else
		ok = FALSE;
	VariantClear(&vr);
	return ok;
}

BOOL __fastcall idl_GetShellColumnText(IShellFolder2* psf2, LPCITEMIDLIST pidl, int col, CString& str)
{
	// 旧方式で取得
	SHELLDETAILS sdt;
	if (psf2->GetDetailsOf(pidl, col, &sdt) == S_OK) {
		CMemTask ptr;
		if (StrRetToStr(&sdt.str, pidl, (PWSTR*)&ptr) == S_OK) {
			str = ptr;
			return TRUE;
		}
	}
	return FALSE;
}


#pragma pack(1)
typedef struct tagVIDLIST {
	WORD size;
	BYTE mediatype;			// MTYPE_???
	BYTE cb2;
	DWORD filesize;
	WORD dosdate;
	WORD dostime;
	WORD fileattr;
	BYTE filename[1];
} VIDLIST;
#pragma pack()

/*************************************
  シングルIDLISTの構成解析

  mediatype:
  0x20～0x2F ドライブ関連
  0x30～0x3F フォルダ・ファイル関連
  0x40～0x4F ネットワーク関連

#define MTYPE_ROOT_REGITEM		0x1F		// マイドキュメントやマイコンピュータ
#define MTYPE_DRIVE_REMOVABLE	0x22
#define MTYPE_DRIVE_FIXED		0x23
#define MTYPE_DRIVE_OTHER		0x24
#define MTYPE_DRIVE_CDROM		0x25
#define MTYPE_DRIVE_RAMDISK		0x26
#define MTYPE_DRIVE_OTHER		0x27
#define MTYPE_DRIVE_FDD525		0x28
#define MTYPE_DRIVE_FDD35		0x29
#define MTYPE_DRIVE_NETDRIVE	0x2A
#define MTYPE_FSYS_CONTROLS		0x2E		// コントロールパネル
#define MTYPE_FS_FOLDER			0x31
#define MTYPE_FS_FILE			0x32
#define MTYPE_FS_UNICODEFOLDER	0x35		// UNICODE フォルダ
#define MTYPE_FS_UNICODEFILE	0x36		// UNICODE ファイル
#define MTYPE_FSYS_A002			0x3E		// 
#define MTYPE_NET_OTHER			0x41		// ワークグループ
#define MTYPE_NET_SERVER		0x42		// サーバー
#define MTYPE_FSYS_A003			0x45		// コントロールパネルのアプレット等
#define MTYPE_NET_DOMAIN		0x46		// ネットワークドメイン
#define MTYPE_NET_RESTOFNET		0x47		// ネットワーク全体
#define MTYPE_FSYS_A005			0x4D		// ネットワークプレースの追加
#define MTYPE_REMOTE_PRINTER	0x4E		// プリンタとFAX／タスク
#define MTYPE_FSYS_A006			0x71		// フォルダ?
#define MTYPE_FSYS_A007			0xB1		// お気に入りや履歴フォルダ等
#define MTYPE_FSYS_A008			0xB5		// ブリーフケース等
#define MTYPE_NET_SHARE			0xC3		// サーバシェア
*************************************/


void dump_descid(int descid)
{
#ifdef _DEBUG
	switch (descid) {
	case SHDID_ROOT_REGITEM:		dbg.printf(L"id=SHDID_ROOT_REGITEM\n"); break;
	case SHDID_FS_FILE:				dbg.printf(L"id=SHDID_FS_FILE\n"); break;
	case SHDID_FS_DIRECTORY:		dbg.printf(L"id=SHDID_FS_DIRECTORY\n"); break;
	case SHDID_FS_OTHER:			dbg.printf(L"id=SHDID_FS_OTHER\n"); break;
	case SHDID_COMPUTER_DRIVE35:	dbg.printf(L"id=SHDID_COMPUTER_DRIVE35\n"); break;
	case SHDID_COMPUTER_DRIVE525:	dbg.printf(L"id=SHDID_COMPUTER_DRIVE525\n"); break;
	case SHDID_COMPUTER_REMOVABLE:	dbg.printf(L"id=SHDID_COMPUTER_REMOVABLE\n"); break;
	case SHDID_COMPUTER_FIXED:		dbg.printf(L"id=SHDID_COMPUTER_FIXED\n"); break;
	case SHDID_COMPUTER_NETDRIVE:	dbg.printf(L"id=SHDID_COMPUTER_NETDRIVE\n"); break;
	case SHDID_COMPUTER_CDROM:		dbg.printf(L"id=SHDID_COMPUTER_CDROM\n"); break;
	case SHDID_COMPUTER_RAMDISK:	dbg.printf(L"id=SHDID_COMPUTER_RAMDISK\n"); break;
	case SHDID_COMPUTER_OTHER:		dbg.printf(L"id=SHDID_COMPUTER_OTHER\n"); break;
	case SHDID_NET_DOMAIN:			dbg.printf(L"id=SHDID_NET_DOMAIN\n"); break;
	case SHDID_NET_SERVER:			dbg.printf(L"id=SHDID_NET_SERVER\n"); break;
	case SHDID_NET_SHARE:			dbg.printf(L"id=SHDID_NET_SHARE\n"); break;
	case SHDID_NET_RESTOFNET:		dbg.printf(L"id=SHDID_NET_RESTOFNET\n"); break;
	case SHDID_NET_OTHER:			dbg.printf(L"id=SHDID_NET_OTHER\n"); break;
	case SHDID_COMPUTER_IMAGING:	dbg.printf(L"id=SHDID_COMPUTER_IMAGING\n"); break;
	case SHDID_COMPUTER_AUDIO:		dbg.printf(L"id=SHDID_COMPUTER_AUDIO\n"); break;
	case SHDID_COMPUTER_SHAREDDOCS:	dbg.printf(L"id=SHDID_COMPUTER_SHAREDDOCS\n"); break;
	case SHDID_MOBILE_DEVICE:		dbg.printf(L"id=SHDID_MOBILE_DEVICE\n"); break;
//@	case SHDID_COMPUTER_LIBRARIES:	dbg.printf(L"id=SHDID_COMPUTER_LIBRARIES\n"); break;
//@	case SHDID_SHAREPOINT_SITES:	dbg.printf(L"id=SHDID_SHAREPOINT_SITES\n"); break;
	default:						dbg.printf(L"id=** unknown ** (%d)\n", descid); break;
	}
#endif
}

int __fastcall idl_GetDescriptionId(IShellFolder* psf, LPCITEMIDLIST pidl)
{
	int descid = 0;
	if (psf && pidl) {
		SHDESCRIPTIONID sdi;
		if (SHGetDataFromIDList(psf, pidl, SHGDFIL_DESCRIPTIONID, &sdi, sizeof(sdi)) == NOERROR)
			descid = sdi.dwDescriptionId;
		else {
			VIDLIST* pvd = (VIDLIST*)pidl;
			switch (pvd->mediatype) {
				case 0x22: descid = SHDID_COMPUTER_REMOVABLE; break;
				case 0x23: descid = SHDID_COMPUTER_FIXED; break;
				case 0x24: descid = SHDID_COMPUTER_OTHER; break;
				case 0x25: descid = SHDID_COMPUTER_CDROM; break;
				case 0x26: descid = SHDID_COMPUTER_RAMDISK; break;
				case 0x28: descid = SHDID_COMPUTER_DRIVE525; break;
				case 0x27: descid = SHDID_COMPUTER_OTHER; break;
				case 0x29: descid = SHDID_COMPUTER_DRIVE35; break;
				case 0x2A: descid = SHDID_COMPUTER_NETDRIVE; break;
				case 0x2E: descid = SHDID_ROOT_REGITEM; break;
				case 0x31: descid = SHDID_FS_DIRECTORY; break;
				case 0x32: descid = SHDID_FS_FILE; break;
				case 0x3A: descid = SHDID_FS_FILE; break;
						// ↑Win98デスクトップ上のOutLookExpressリンクファイル
				case 0xC3: descid = SHDID_NET_SHARE; break;
				case 0x42: descid = SHDID_NET_SERVER; break;
				case 0x46: descid = SHDID_NET_DOMAIN; break;
				case 0x47: descid = SHDID_NET_RESTOFNET; break;
				default: break;
			}
		}
	}
	return descid;
}

int __fastcall idl_GetDescriptionId(LPCITEMIDLIST pidlFq)
{
	int descid = 0;
	CComPtr<IShellFolder> psf;
	LPCITEMIDLIST pidl;
	if (SHBindToParent(pidlFq, IID_PPV_ARGS(&psf), &pidl) == S_OK) {
		descid = idl_GetDescriptionId(psf, pidl);
	}
	return descid;
}

BOOL __fastcall idl_GetAttributesOf(LPCITEMIDLIST pidlFq, SFGAOF* pattr)
{
	/* @@@ あまり信用できない
		IShellFolder* psf;
		SHGetDesktopFolder(&psf);
		HRESULT hr = psf->GetAttributesOf(1, (LPCITEMIDLIST*)&pidlFq, pattr);
		psf->Release();
		return (hr == NOERROR) ? TRUE : FALSE;
	@@@ */

	SHFILEINFO sfi;
	sfi.dwAttributes = *pattr;
	BOOL bOK = (BOOL)SHGetFileInfo((LPCTSTR)pidlFq, 0, &sfi, sizeof(sfi), SHGFI_PIDL | SHGFI_ATTR_SPECIFIED | SHGFI_ATTRIBUTES);
	if (bOK) *pattr = sfi.dwAttributes;
	return bOK;
}

BOOL __fastcall idl_GetShellAttributes(LPCITEMIDLIST pidlFq, SFGAOF mask, SFGAOF* pattr)
{
	CComPtr<IShellItem> psi;
	return (SHCreateItemFromIDList(pidlFq, IID_PPV_ARGS(&psi)) == S_OK &&
		psi->GetAttributes(mask, pattr) == S_OK);
}

HBITMAP __fastcall idl_ExtractImage(IShellFolder* psf, LPCITEMIDLIST pidl, SIZE* prgSize, DWORD depth)
{
	if (!psf || !pidl || !prgSize) return NULL;

	HBITMAP hbm = NULL;
	CComPtr<IThumbnailProvider> ptp;
	if (psf->GetUIObjectOf(NULL, 1, (LPCITEMIDLIST*)&pidl, IID_IThumbnailProvider, NULL, (void **)&ptp) == S_OK) {
		WTS_ALPHATYPE alphaType;
		if (ptp->GetThumbnail(prgSize->cx, &hbm, &alphaType) != S_OK)
			hbm = NULL;
	}
	CComPtr<IExtractImage> pie;
	if (!hbm && psf->GetUIObjectOf(NULL, 1, (LPCITEMIDLIST*)&pidl, IID_IExtractImage, NULL, (void**)&pie) == S_OK) {
		OLECHAR* wsz = new OLECHAR[MAX_URL_LENGTH];
		DWORD prior = 0;			// IEI_PRIORITY_NORMAL is defined nowhere!
		DWORD flags = IEIFLAG_SCREEN | IEIFLAG_OFFLINE | IEIFLAG_CACHE;
		HRESULT hr = pie->GetLocation(wsz, MAX_URL_LENGTH, &prior, prgSize, depth, &flags);
		if (hr == NOERROR || hr == E_PENDING) {
			// even if we've got shell v4.70+, not all files support thumbnails 
			if (pie->Extract(&hbm) != NOERROR)
				hbm = NULL;
		}
		delete [] wsz;
	}
	return hbm;
}

BOOL __fastcall idl_IDListIsValid(LPCITEMIDLIST pidlFullyQual)
{
	BOOL bOK = FALSE;
	CComPtr<IShellFolder> psf;
	LPCITEMIDLIST pidl;
	if (SHBindToParent(pidlFullyQual, IID_PPV_ARGS(&psf), &pidl) == S_OK) {
		SFGAOF ula = SFGAO_VALIDATE;
		if (psf->GetAttributesOf(1, &pidl, &ula) == NOERROR)
			bOK = TRUE;
	}
	else
		bOK = idl_isDesktop(pidlFullyQual);
	return bOK;
}

BOOL __fastcall idl_SendTo(HWND hwnd, LPCITEMIDLIST pidlFullyQual, IDataObject* pdo)
{
	BOOL bOK = FALSE;
	CComPtr<IDropTarget> pdt = (IDropTarget*)idl_GetUIObjectOf(hwnd, pidlFullyQual, IID_IDropTarget);
	if (pdt.p != NULL) {
		POINTL pt;
		pt.x = pt.y = 0;
		DWORD eff = DROPEFFECT_COPY | DROPEFFECT_MOVE | DROPEFFECT_LINK;

		DWORD keyState = MK_LBUTTON;
		if (GetKeyState(VK_CONTROL) < 0)	keyState |= MK_CONTROL;
		if (GetKeyState(VK_SHIFT) < 0)		keyState |= MK_SHIFT;
		if (pdt->DragEnter(pdo, keyState, pt, &eff) == S_OK &&
			pdt->Drop(pdo, keyState, pt, &eff) == S_OK) {
			bOK = TRUE;
		}
	}
	return bOK;
}

BOOL __fastcall idl_CopyTo(HWND hwnd, LPCITEMIDLIST pidlFullyQual, IDataObject* pdo, BOOL bCopy)
{
	BOOL bOK = FALSE;
	CComPtr<IDropTarget> pdt = (IDropTarget*)idl_GetUIObjectOf(hwnd, pidlFullyQual, IID_IDropTarget);
	if (pdt.p != NULL) {
		POINTL pt;
		pt.x = pt.y = 0;
		DWORD eff = DROPEFFECT_COPY;
		DWORD keyState = MK_LBUTTON | (bCopy ? MK_CONTROL : MK_SHIFT);
		if (pdt->DragEnter(pdo, keyState, pt, &eff) == S_OK &&
			pdt->Drop(pdo, keyState, pt, &eff) == S_OK) {
			bOK = TRUE;
		}
	}
	return bOK;
}

///////////////////////////////
// 正常に動作するか不明
//
BOOL __fastcall idl_IsValidPidl(LPCITEMIDLIST pidlFullyQual)
{
	CComPtr<IShellFolder> psf;
	if (pidlFullyQual && SHGetDesktopFolder(&psf) == S_OK) {
		STRRET str;
		if (S_OK == psf->GetDisplayNameOf(pidlFullyQual, SHGDN_FORPARSING, &str)) {
			CMemTask psz;
			if (StrRetToStr(&str, pidlFullyQual, (PWSTR*)&psz) == S_OK) {
				ULONG eat, attr = SFGAO_VALIDATE;
				CMemTask pidl;
				return (psf->ParseDisplayName(0, 0, psz, &eat, (LPITEMIDLIST*)&pidl, &attr) == S_OK);
			}
		}
	}
	return FALSE;
}


////////////////////////////////////////////////////////////////////////
//

DWORDLONG idl_HashPidl(LPITEMIDLIST pidl)
{
	DWORDLONG dl = 0;
	if (pidl) {
		int len = ILGetSize(pidl);
		DWORD hash = 0;
		WORD sum = 0;
		BYTE* pb = (BYTE*)pidl;
		for (int i = 0; i < len; i++) {
			hash = (hash << 5) + hash + *pb;
			sum = sum + *pb;
			pb++;
		}

		DWORD tmp = MAKELONG(len, sum);
		SETDWORDLONG(dl, hash, tmp);
	}
	return dl;
}

////////////////////////////////////////////////////////////////////////
//


#define PT_CPLAPPLET	0x00
#define PT_GUID			0x1F
#define PT_DRIVE		0x23
#define PT_DRIVE2		0x25
#define PT_DRIVE3		0x29
#define PT_SHELLEXT		0x2E
#define PT_DRIVE1		0x2F
#define PT_FOLDER1		0x30
#define PT_FOLDER		0x31
#define PT_VALUE		0x32
#define PT_VALUEW       0x34
#define PT_WORKGRP		0x41
#define PT_COMP			0x42
#define PT_NETPROVIDER  0x46
#define PT_NETWORK		0x47
#define PT_IESPECIAL1	0x61
#define PT_YAGUID		0x70 /* yet another guid.. */
#define PT_IESPECIAL2	0xb1
#define PT_SHARE		0xc3

typedef BYTE PIDLTYPE;

#pragma pack(1)

typedef struct tagPIDLCPanelStruct { 
	BYTE dummy;					/*01 is 0x00 */
	DWORD iconIdx;				/*02 negative icon ID */
	WORD offsDispName;			/*06*/
	WORD offsComment;			/*08*/
	CHAR szName[1];				/*10*/ /* terminated by 0x00, followed by display name and comment string */
} PIDLCPanelStruct;

typedef struct tagGUIDStruct {
	BYTE dummy;					/* offset 01 is unknown */
	GUID guid;					/* offset 02 */
} GUIDStruct;

typedef struct tagDriveStruct {
	CHAR szDriveName[20];		/*01*/
	WORD unknown;				/*21*/
} DriveStruct;

typedef struct tagFileStruct {
	BYTE dummy;					/*01 is 0x00 for files or dirs */
	DWORD dwFileSize;			/*02*/
	WORD uFileDate;				/*06*/
	WORD uFileTime;				/*08*/
	WORD uFileAttribs;			/*10*/
	CHAR szNames[1];			/*12*/
	/* Here are coming two strings. The first is the long name.
	   The second the dos name when needed or just 0x00 */
} FileStruct;

typedef struct tagValueW {
	WCHAR name[1];
} ValueWStruct;

typedef struct tagPIDLDATA {
	PIDLTYPE type;				/*00*/
	union {
		GUIDStruct guids;
		DriveStruct drive;
		FileStruct file;
		struct {
			WORD dummy;			/*01*/
			CHAR szNames[1];	/*03*/
		} network;
		struct {
			WORD dummy;			/*01*/
			DWORD dummy1;		/*02*/
			CHAR szName[1];		/*06*/ /* terminated by 0x00 0x00 */
		} htmlhelp;
		PIDLCPanelStruct cpanel;
		ValueWStruct valueW;
	} u;
} PIDLDATA, *LPPIDLDATA;

#pragma pack()


LPPIDLDATA idl_GetDataPointer(LPCITEMIDLIST pidl)
{
	if (pidl && pidl->mkid.cb != 0)
		return (LPPIDLDATA) &pidl->mkid.abID;
	return NULL;
}

bool idl_GetGUIDPointer(LPCITEMIDLIST pidl, IID* piid)
{
	LPPIDLDATA pdata = idl_GetDataPointer(pidl);
	if (pdata) {
	//	TRACE("pdata->type 0x%04x\n", pdata->type);
		switch (pdata->type) {
		case PT_SHELLEXT:
		case PT_GUID:
		case PT_YAGUID:
			*piid = pdata->u.guids.guid;
			return true;
		}
	//	TRACE("Unknown pidl type 0x%04x\n", pdata->type);
	}
	return false;
}

BOOL idl_IsMyComputer(LPCITEMIDLIST pidl)
{
	IID iid;
	if (idl_GetGUIDPointer(pidl, &iid))
		return IsEqualIID(iid, CLSID_MyComputer);
	return FALSE;
}

LPITEMIDLIST idl_GetMyDocument()
{
	LPITEMIDLIST pidl = NULL;
	CComPtr<IShellFolder> psf;
	if (SHGetDesktopFolder(&psf) == S_OK) {
		ULONG eat;
		psf->ParseDisplayName(0, 0, STR_MYDOCS_CLSID_W, &eat, &pidl, 0);
	}
	return pidl;
}



////////////////////////////////////////////////////////////////////////////
// 以下コードサンプル


HRESULT DisplayNameOf(IShellFolder *psf, LPCITEMIDLIST pidl, DWORD flags, PWSTR psz, UINT cch)
{
	*psz = 0;
	STRRET sr;
	HRESULT hr = psf->GetDisplayNameOf(pidl, flags, &sr);
	if (SUCCEEDED(hr)) {
		CMemTask poc;
		hr = StrRetToStr(&sr, pidl, (PWSTR*)&poc);
		if (SUCCEEDED(hr)) {
			StrCpyN(psz, poc, cch);
		}
	}
	return hr;
}

DWORD SHGetAttributes(IShellFolder* psf, LPCITEMIDLIST pidl, DWORD dwAttribs)
{
	// like SHBindToObject, if psf is NULL, use absolute pidl
	LPCITEMIDLIST pidlChild;
	if (!psf) {
		SHBindToParent(pidl, IID_PPV_ARGS(&psf), &pidlChild);
	}
	else {
		psf->AddRef();
		pidlChild = pidl;
	}

	DWORD dw = 0;
	if (psf) {
		dw = dwAttribs;
		dw = SUCCEEDED(psf->GetAttributesOf(1, (LPCITEMIDLIST *)&pidlChild, &dw)) ? (dwAttribs & dw) : 0;
		psf->Release();
	}
	return dw;
}

HRESULT SHUnknown_QueryService(IUnknown* punk, REFGUID guidService, REFIID riid, void **ppv)
{
	*ppv = NULL;
	CComQIPtr<IServiceProvider> psp = punk;
	return !psp ? E_FAIL : psp->QueryService(guidService, riid, ppv);
}

HRESULT SHGetIDListFromUnk(IUnknown *punk, LPITEMIDLIST *ppidl)
{
	*ppidl = NULL;
	CComQIPtr<IPersistFolder2> ppf = punk;
	return !ppf ? E_NOINTERFACE : ppf->GetCurFolder(ppidl);
}

HRESULT SHGetName(LPCITEMIDLIST pidl, DWORD gdnFlags, PTSTR psz, UINT cch)
{
	CComPtr<IShellFolder> psfDesktop;
	HRESULT hr = SHGetDesktopFolder(&psfDesktop);
	if (hr == S_OK) {
		CComPtr<IShellFolder> psf;
		LPCITEMIDLIST pidlChild;
		hr = SHBindToParent(pidl, IID_PPV_ARGS(&psf), &pidlChild);
		if (hr == S_OK) {
			hr = DisplayNameOf(psf, pidlChild, gdnFlags, psz, cch);
		}
	}
	return hr;
}

HRESULT ViewFromWebBrowserApp(IWebBrowserApp *pwba, REFIID riid, void **ppv)
{
	CComPtr<IShellBrowser> psb;
	HRESULT hr = SHUnknown_QueryService(pwba, SID_STopLevelBrowser, IID_PPV_ARGS(&psb));
	if (SUCCEEDED(hr)) {
		CComPtr<IShellView> psv;
		hr = psb->QueryActiveShellView(&psv);
		if (SUCCEEDED(hr)) {
			hr = psv->QueryInterface(riid, ppv);
		}
	}
	return hr;
}

HWND WindowFromUnk(IUnknown *punk)
{
	HWND hwnd = NULL;
	CComQIPtr<IOleWindow> pow = punk;
	if (pow.p != NULL) {
		pow->GetWindow(&hwnd);
	}
	return hwnd;
}

HRESULT FindWindowByHWND(HWND hwnd, REFIID riid, void **ppv)
{
	HRESULT hr = E_FAIL;
	*ppv = NULL;

	CComPtr<IShellWindows> psw;
	if (SUCCEEDED(CoCreateInstance(CLSID_ShellWindows, NULL, CLSCTX_ALL, IID_PPV_ARGS(&psw)))) {
		VARIANT v;
		v.vt = VT_I4;

		CComPtr<IDispatch> pdispWBA;
		for (v.intVal = 0; FAILED(hr) && S_OK == psw->Item(v, &pdispWBA); v.intVal++) {
			CComQIPtr<IWebBrowserApp> pwba = pdispWBA;
			if (pwba.p != NULL) {
				HWND hwndWBA;
				if (SUCCEEDED(pwba->get_HWND((LONG_PTR*)&hwndWBA)) && (hwnd == hwndWBA)) {
					hr = ViewFromWebBrowserApp(pwba, riid, ppv);
				}
			}
		}
	}
	return hr;
}


////////////////////////////////////////////////////////////////////////
// scid_supp.cpp

#include "scid_supp.cpp"
