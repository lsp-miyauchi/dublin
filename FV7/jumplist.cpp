////////////////////////////////////////////////////////////////////////
// jumplist.cpp

#include "stdafx.h"
#include <propkey.h>
#include <propvarutil.h>
#include "fv7.h"


int EnumTabGroup(CCmItemArray& array);
int LoadBookmark(CCmItemArray& array);
int LoadTabReopenArray(CTabReopenArray& array, int maxEntry);

/////////////////////////////////////////////////////////////////////////////
// Windows7 ジャンプリスト

static HRESULT GetIconOf(CIDListPtr& idl, CString& str, int& iImage)
{
	LPCITEMIDLIST pidlSelf;
	CComPtr<IShellFolder> psf = idl.BindToParent(&pidlSelf);
	if (psf) {
		CComPtr<IExtractIcon> pei;
		if (psf->GetUIObjectOf(NULL, 1, &pidlSelf, IID_IExtractIcon, NULL, (void**)&pei) == S_OK) {
			TCHAR buf[MAX_PATH * 2];
			UINT flags;
			if (pei->GetIconLocation(GIL_FORSHELL, buf, ARRAYSIZE(buf), &iImage, &flags) == S_OK) {
				str = buf;
				return S_OK;
			}
		}
	}
	return E_FAIL;
}

static bool AddJumpListTask(IObjectCollection* pObjColl, LPCWSTR pszArgs, CCmItem* pci, LPCTSTR szExePath, LPCTSTR pszIconPath, int iconIndex )
{
	// Create a shell link COM object.
	HRESULT hr;
	CComPtr<IShellLink> pLink;
	hr = pLink.CoCreateInstance(CLSID_ShellLink, NULL, CLSCTX_INPROC_SERVER);
	if (FAILED(hr)) return false;

	// Set the executable path
	hr = pLink->SetPath( szExePath );
	if (FAILED(hr)) return false;

    // Set the arguments
	if (pszArgs && *pszArgs) {
		hr = pLink->SetArguments( pszArgs );
		if (FAILED(hr)) return false;
	}

    // Set the icon location
	CString str;
	int iImage;
	if (pszIconPath && *pszIconPath) {
		str = pszIconPath;
		iImage = iconIndex;
	}
	else {
		if (FAILED(GetIconOf(pci->m_idl, str, iImage))) {
			str = szExePath;
			iImage = -IDI_STAR;
		}
	}
	hr = pLink->SetIconLocation(str.GetString(), iImage);
	if (FAILED(hr)) return false;

    // Set the link description (tooltip on the jump list item)
	hr = pLink->SetDescription( pci->m_name );
	if (FAILED(hr)) return false;

	// Set the link title (the text of the jump list item). This is kept in the
	// object's property store, so QI for that interface.
	CComQIPtr<IPropertyStore> pPropStore = pLink;
	PROPVARIANT pv;
    if ( !pPropStore )
        return false;

    hr = InitPropVariantFromString( CT2CW(pci->m_name.GetString()), &pv );
	if (FAILED(hr)) return false;

	// Set the title property.
	hr = pPropStore->SetValue( PKEY_Title, pv );
	PropVariantClear( &pv );
	if (FAILED(hr)) return false;

	// Save the property changes.
	hr = pPropStore->Commit();
	if (FAILED(hr)) return false;

	// Add this shell link to the object collection.
	hr = pObjColl->AddObject( pLink );
	return SUCCEEDED(hr);
}

bool CreateJumpList()
{
	HRESULT hr;
	CComPtr<ICustomDestinationList> pDestList;
	// Create a jump list COM object.
	hr = pDestList.CoCreateInstance(CLSID_DestinationList, NULL, CLSCTX_INPROC_SERVER);
	if (FAILED(hr)) return false;

	// Tell the jump list our AppID.
//	hr = pDestList->SetAppID(APP_APPID);
//	if ( FAILED(hr) ) return false;

	// まず削除
	pDestList->DeleteList(NULL);

	// モジュールパス
	TCHAR szExePath[MAX_PATH];
	GetModuleFileName(NULL, szExePath, ARRAYSIZE(szExePath));

	// Create a new, empty jump list. We aren't adding custom destinations,
	// so slots and pRemovedItems aren't used.
	UINT slots;
	CComPtr<IObjectArray> pRemovedItems;
	hr = pDestList->BeginList(&slots, IID_PPV_ARGS(&pRemovedItems));
	if (FAILED(hr)) return false;

	// ----------------------------------------------
	// しおり用のオブジェクトコレクションを作成
	// ----------------------------------------------
	CCmItem* pci;
	CString str;
	int i, numAdd;
	CCmItemArray array;
	int cnt = LoadBookmark(array);
	if (cnt > 0) {
		CComPtr<IObjectCollection> pBookmark;
		hr = pBookmark.CoCreateInstance( CLSID_EnumerableObjectCollection, NULL, CLSCTX_INPROC_SERVER );
		if (SUCCEEDED(hr)) {
			// しおりの個別データをコレクションに追加
			numAdd = 0;
		    for (i = 0; i < cnt; i++) {
				pci = array[i];
				if (pci->isFolder()) {
					// Add the next task to the object collection.
					CString guidstr;
					pci->GetGuid(guidstr);
					str.Format(_T("/bookmark:%s"), guidstr.GetString());
					if (AddJumpListTask( pBookmark, str, pci, szExePath, NULL, 0 ))
						numAdd++;
				}
				else {
					// タブ グループ
					pci->GetParseName(str);
					if (AddJumpListTask( pBookmark, NULL, pci, str, szExePath, -IDI_TABGROUP))
						numAdd++;
				}
			}

			if (numAdd > 0) {
				CComQIPtr<IObjectArray> pTasksArray = pBookmark;
				if (pTasksArray) {
					// 「しおり」カテゴリを作成して追加
					CString s; s.LoadString(IDW_FAVORITE);
					hr = pDestList->AppendCategory(s.GetString(), pTasksArray);
				}
			}
		}
		array.DeleteAll();
	}

	// ----------------------------------------------
	// タブグループ用のオブジェクトコレクションを作成
	// ----------------------------------------------
	cnt = EnumTabGroup(array);
	if (cnt > 0) {
		CComPtr<IObjectCollection> pTabgroup;
		hr = pTabgroup.CoCreateInstance( CLSID_EnumerableObjectCollection, NULL, CLSCTX_INPROC_SERVER );
		if (SUCCEEDED(hr)) {
			numAdd = 0;
			for (i = 0; i < cnt; i++) {
				pci = array[i];
				// Add the next task to the object collection.
				pci->GetParseName(str);
				if (AddJumpListTask( pTabgroup, NULL, pci, str, szExePath, -IDI_TABGROUP))
					numAdd++;
			}

			if (numAdd > 0) {
				CComQIPtr<IObjectArray> pTasksArray = pTabgroup;
				if (pTasksArray) {
					// 「タブ グループ」カテゴリを作成して追加
					str.LoadString(IDS_TABGROUP);
					hr = pDestList->AppendCategory(str.GetString(), pTasksArray);
				}
			}
		}
		array.DeleteAll();
	}

	// ----------------------------------------------
	// 最近閉じたタブ用のオブジェクトコレクションを作成
	// ----------------------------------------------
	CTabReopenArray tra;
	cnt = LoadTabReopenArray(tra, 3);
	if (cnt > 0) {
		CComPtr<IObjectCollection> pTabrecent;
		hr = pTabrecent.CoCreateInstance( CLSID_EnumerableObjectCollection, NULL, CLSCTX_INPROC_SERVER );
		if (SUCCEEDED(hr)) {
			numAdd = 0;
			for (i = 0; i < cnt; i++) {
				// Add the next task to the object collection.
				pci = tra.GetItemAt(i);
				CString guidstr;
				pci->GetGuid(guidstr);
				str.Format(_T("/recent:%s"), guidstr.GetString());
				if (AddJumpListTask( pTabrecent, str, pci, szExePath, NULL, 0))
					numAdd++;
			}

			if (numAdd > 0) {
				CComQIPtr<IObjectArray> pTasksArray = pTabrecent;
				if (pTasksArray) {
					// 「最近閉じたタブ」カテゴリを作成して追加
					str.LoadString(IDS_TABRECENT);
					hr = pDestList->AppendCategory(str.GetString(), pTasksArray);
				}
			}
		}
		array.DeleteAll();
	}

	// Save the jump list.
	hr = pDestList->CommitList();
	return SUCCEEDED(hr);
}

// Removes that existing custom Jump List for this application.
void DeleteJumpList()
{
	HRESULT hr;
	CComPtr<ICustomDestinationList> pJumpList;
	// Create a jump list COM object.
	hr = pJumpList.CoCreateInstance ( CLSID_DestinationList, NULL, CLSCTX_INPROC_SERVER );
	if (SUCCEEDED(hr)) {
		// Tell the jump list our AppID.
//		hr = pDestList->SetAppID(APP_APPID);
//		if ( SUCCEEDED(hr) )
			pJumpList->DeleteList(NULL);
	}
}
