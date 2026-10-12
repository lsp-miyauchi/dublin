// ExifParser.h : CExifParser の宣言

#pragma once
#include "resource.h"       // メイン シンボル
#include "lspexif.h"
#include "lspexifapi.h"

#if defined(_WIN32_WCE) && !defined(_CE_DCOM) && !defined(_CE_ALLOW_SINGLE_THREADED_OBJECTS_IN_MTA)
#error "DCOM の完全サポートを含んでいない Windows Mobile プラットフォームのような Windows CE プラットフォームでは、単一スレッド COM オブジェクトは正しくサポートされていません。ATL が単一スレッド COM オブジェクトの作成をサポートすること、およびその単一スレッド COM オブジェクトの実装の使用を許可することを強制するには、_CE_ALLOW_SINGLE_THREADED_OBJECTS_IN_MTA を定義してください。ご使用の rgs ファイルのスレッド モデルは 'Free' に設定されており、DCOM Windows CE 以外のプラットフォームでサポートされる唯一のスレッド モデルと設定されていました。"
#endif


///////////////////////////////////////////////////////////////////////////
// CExifParser

class ATL_NO_VTABLE CExifParser :
	public CComObjectRootEx<CComSingleThreadModel>,
	public CComCoClass<CExifParser, &CLSID_ExifParser>,
	public IDispatchImpl<IExifParser, &IID_IExifParser, &LIBID_LSExifLib, /*wMajor =*/ 1, /*wMinor =*/ 0>
{
public:
	CExifParser()
	{
	}

DECLARE_REGISTRY_RESOURCEID(IDR_EXIFPARSER)


BEGIN_COM_MAP(CExifParser)
	COM_INTERFACE_ENTRY(IExifParser)
	COM_INTERFACE_ENTRY(IDispatch)
END_COM_MAP()


	DECLARE_PROTECT_FINAL_CONSTRUCT()

	HRESULT FinalConstruct()
	{
		return S_OK;
	}

	void FinalRelease()
	{
	}

public:

	STDMETHOD(IsExifFile)(LPCWSTR pszFileName);
	STDMETHOD(GetExifInfo)(LPCWSTR pszFileName, IStream* pStream, int* pnOrientation, BOOL bLargeImage, BYTE* pInfo);
	STDMETHOD(GetThumbsnail)(LPCWSTR pszFileName, IStream* pStream, int* pnOrientation, BOOL bLargeImage, LPWSTR pszTips, int cchMax);
	STDMETHOD(GetInfoTip)(LPCWSTR pszFileName, LPWSTR pszTips, int cchMax);
	STDMETHOD(GetDateTimeString)(LPCWSTR pszFileName, LPWSTR pszDate, int cchMax);
	STDMETHOD(EnumExifInfo)(LPCWSTR pszFileName, UINT_PTR pfnCallback, LPARAM param);
};

OBJECT_ENTRY_AUTO(__uuidof(ExifParser), CExifParser)
