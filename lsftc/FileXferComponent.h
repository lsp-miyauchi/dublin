// FileXferComponent.h : CFileXferComponent ÇÃêÈåæ

#pragma once
#include "resource.h"       // ÉÅÉCÉì ÉVÉìÉ{Éã

#include "lsftc_i.h"

// CFileXferComponent

class ATL_NO_VTABLE CFileXferComponent :
	public CComObjectRootEx<CComSingleThreadModel>,
	public CComCoClass<CFileXferComponent, &CLSID_FileXferComponent>,
	public IDispatchImpl<IFileXferComponent, &IID_IFileXferComponent, &LIBID_lsftcLib, /*wMajor =*/ 1, /*wMinor =*/ 0>
{
private:
	FILETIME m_ftCurrent;

public:
	CFileXferComponent()
	{
	}

DECLARE_REGISTRY_RESOURCEID(IDR_FILEXFERCOMPONENT)


BEGIN_COM_MAP(CFileXferComponent)
	COM_INTERFACE_ENTRY(IFileXferComponent)
	COM_INTERFACE_ENTRY(IDispatch)
END_COM_MAP()



	DECLARE_PROTECT_FINAL_CONSTRUCT()

	HRESULT FinalConstruct();

	void FinalRelease()
	{
	}

public:

	STDMETHOD(GetInstance)(LONG* val);
	STDMETHOD(ResetInstance)(void);
	STDMETHOD(InitInstance)(void);
};

OBJECT_ENTRY_AUTO(__uuidof(FileXferComponent), CFileXferComponent)
