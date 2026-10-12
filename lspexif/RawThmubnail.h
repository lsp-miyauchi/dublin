// RawThmubnail.h : CRawThmubnail の宣言

#pragma once

#include <shlguid.h>
#include <thumbcache.h>
#include "..\lscore\gdiplus_sup.h"

#include "resource.h"       // メイン シンボル
#include "lspexifapi.h"


#if defined(_WIN32_WCE) && !defined(_CE_DCOM) && !defined(_CE_ALLOW_SINGLE_THREADED_OBJECTS_IN_MTA)
#error "DCOM の完全サポートを含んでいない Windows Mobile プラットフォームのような Windows CE プラットフォームでは、単一スレッド COM オブジェクトは正しくサポートされていません。ATL が単一スレッド COM オブジェクトの作成をサポートすること、およびその単一スレッド COM オブジェクトの実装の使用を許可することを強制するには、_CE_ALLOW_SINGLE_THREADED_OBJECTS_IN_MTA を定義してください。ご使用の rgs ファイルのスレッド モデルは 'Free' に設定されており、DCOM Windows CE 以外のプラットフォームでサポートされる唯一のスレッド モデルと設定されていました。"
#endif



// CRawThmubnail

class ATL_NO_VTABLE CRawThmubnail :
	public CComObjectRootEx<CComSingleThreadModel>,
	public CComCoClass<CRawThmubnail, &CLSID_RawThmubnail>,
	public IDispatchImpl<IRawThmubnail, &IID_IRawThmubnail, &LIBID_LSExifLib, /*wMajor =*/ 1, /*wMinor =*/ 0>,
	public IInitializeWithItem,
	public IThumbnailProvider
{
private:
    IShellItem* m_pShellItem;
	gdiplus_using m_gu;

public:
	CRawThmubnail()
		: m_pShellItem(NULL)
	{
	}

DECLARE_REGISTRY_RESOURCEID(IDR_RAWTHMUBNAIL)


BEGIN_COM_MAP(CRawThmubnail)
	COM_INTERFACE_ENTRY(IRawThmubnail)
	COM_INTERFACE_ENTRY(IDispatch)
END_COM_MAP()


	DECLARE_PROTECT_FINAL_CONSTRUCT()

	HRESULT FinalConstruct()
	{
		return S_OK;
	}

	void FinalRelease()
	{
		if (m_pShellItem != NULL)
			m_pShellItem->Release();
	}

public:

	// IInitializeWithStream.Initialize method
	HRESULT STDMETHODCALLTYPE Initialize(IShellItem* pShellItem, DWORD grfMode);

	// IThumbnailProvider.GetThumbnail method. Gets the thumbnail for this item.
	IFACEMETHODIMP GetThumbnail(UINT cx, HBITMAP* phbmp, WTS_ALPHATYPE* pdwAlpha);
};

OBJECT_ENTRY_AUTO(__uuidof(RawThmubnail), CRawThmubnail)
