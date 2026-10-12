////////////////////////////////////////////////////////////////
// drgdrp.h

#pragma once
#ifndef __DRGDRP_H__
#define __DRGDRP_H__

////////////////////////////////////////////////////////////////
//

inline HGLOBAL GetDataFromDataObject(IDataObject* pdo, CLIPFORMAT cf)
{
	HGLOBAL hmem = NULL;
	FORMATETC fe = { cf, nullptr, DVASPECT_CONTENT, -1, TYMED_HGLOBAL };

	if (pdo->QueryGetData(&fe) == S_OK) {
		STGMEDIUM sm;
		if (pdo->GetData(&fe, &sm) == S_OK) {
			hmem = lccDuplicateMemoryHandle(sm.hGlobal, GMEM_SHARE | GMEM_MOVEABLE);
			ReleaseStgMedium(&sm);
		}
	}
	return hmem;
}

////////////////////////////////////////////////////////////////
// CDataObject

class CDataObject : public IDataObject
{
protected:
	ULONG m_cRef;
	CSimpleArray<void*>* m_pArray;

	CString m_strArchive;
	CString m_strTempFolder;
	CString m_strResponse;
	int m_nArchiveType;
	HDROP m_hdrop;
	static const FORMATETC m_formatetc[];

public:
	CDataObject(LPCTSTR pszArchive, int nType, CSimpleArray<void*>* pArray, LPCTSTR pszTempFolder, LPCTSTR pszResponse)
	{
		m_cRef = 1;

		// 短い名前で
		TCHAR buf[MAX_PATH * 2];
		GetShortPathName(pszArchive, buf, ARRAYSIZE(buf));
		m_strArchive = buf;
		GetShortPathName(pszTempFolder, buf, ARRAYSIZE(buf));
		m_strTempFolder = buf;

		// D&D時の一時解凍用レスポンスファイル
		m_strResponse = pszResponse;

		m_nArchiveType = nType;
		m_pArray = pArray;
		m_hdrop = NULL;
	}

	~CDataObject()
	{
		if (m_hdrop) DragFinish(m_hdrop);
	}

	// IDataObject interface members
	STDMETHODIMP QueryInterface(REFIID riid, void** ppv)
	{
		if (riid == IID_IUnknown || riid == IID_IDataObject) {
			*ppv = this;
			AddRef();
			return NOERROR;
		}
		*ppv = NULL;
		return ResultFromScode(E_NOINTERFACE);
	}
	STDMETHODIMP_(ULONG) AddRef()
	{
		return ++m_cRef;
	}
	STDMETHODIMP_(ULONG) Release()
	{
		if (--m_cRef == 0) {
			delete this;
			return 0;
		}
		return m_cRef;
	}
	STDMETHODIMP GetData(FORMATETC* pFmtc, STGMEDIUM* pStdm);
	STDMETHODIMP_(HGLOBAL) CreateDropFiles();
	STDMETHODIMP_(HGLOBAL) CreateFileGroupDescriptor();
	STDMETHODIMP_(HGLOBAL) CreateFileContents(long lindex);
	STDMETHODIMP_(BOOL) Extract();
	__inline STDMETHODIMP GetDataHere(FORMATETC* pFmtc, STGMEDIUM* pStdm)
	{
		return ResultFromScode(E_NOTIMPL);
	}
	STDMETHODIMP QueryGetData(FORMATETC* pFmtc);
	__inline STDMETHODIMP GetCanonicalFormatEtc(FORMATETC* pFmtcIn, FORMATETC* pFmtcOut)
	{
		pFmtcOut->ptd = NULL;
		return ResultFromScode(E_NOTIMPL);
	}
	__inline STDMETHODIMP SetData(FORMATETC* pFmtc, STGMEDIUM* pStdm, BOOL fRelease)
	{
		return ResultFromScode(E_NOTIMPL);
	}
	STDMETHODIMP EnumFormatEtc(DWORD dwDirection, IEnumFORMATETC** ppenumFmtc);
	__inline STDMETHODIMP DAdvise(FORMATETC* pFmtc, DWORD advf, IAdviseSink* pAdvSink, DWORD* pdwConnection)
	{
		return ResultFromScode(OLE_E_ADVISENOTSUPPORTED);
	}
	__inline STDMETHODIMP DUnadvise(DWORD dwConnection)
	{
		return ResultFromScode(OLE_E_ADVISENOTSUPPORTED);
	}
	__inline STDMETHODIMP EnumDAdvise(IEnumSTATDATA** ppenumAdvise)
	{
		return ResultFromScode(OLE_E_ADVISENOTSUPPORTED);
	}
};


//////////////////////////////////////////////////////////////////////
//

class CDropTargetHandler
{
public:
	virtual DWORD OnTargetDragEnter(IDataObject* pDataObject, DWORD keyState, DWORD dropEffect, POINT point)
	{
		return DROPEFFECT_NONE;
	}
	virtual DWORD OnTargetDragOver(IDataObject* pDataObject, DWORD keyState, DWORD dropEffect, POINT point)
	{
		return DROPEFFECT_NONE;
	}
	virtual void OnTargetDragLeave()
	{
	}
	virtual DWORD OnTargetDrop(IDataObject* pDataObject, DWORD keyState, DWORD dropEffect, POINT point)
	{
		return DROPEFFECT_NONE;
	}
};

////////////////////////////////////////////////////////////////
// CDropTarget

class CDropTarget : public IDropTarget
{
private:
	IDataObject* m_pDataObject;
	ULONG m_refs;
	CDropTargetHandler* m_pTargetHandler;
	DWORD m_keyState;
	DWORD m_dropEffect;

public:
	CDropTarget(CDropTargetHandler* pHandler)
		: m_refs(1)
		, m_pTargetHandler(pHandler)
		, m_keyState(0)
		, m_dropEffect(0)
	{
	}

	// -------------------------------
	// IUnknown methods
	// -------------------------------
	STDMETHODIMP QueryInterface(REFIID riid, void FAR* FAR* ppvObj)
	{
		if (riid == IID_IUnknown || riid == IID_IDropTarget) {
			*ppvObj = this;
			AddRef();
			return NOERROR;
		}
		*ppvObj = NULL;
		return ResultFromScode(E_NOINTERFACE);
	}

	STDMETHODIMP_(ULONG) AddRef(void)
	{
		return ++m_refs;
	}

	STDMETHODIMP_(ULONG) Release(void)
	{
		if (--m_refs == 0) {
			delete this;
			return 0;
		}
		return m_refs;
	}

	// -------------------------------
	// IDropTarget methods
	// -------------------------------
	STDMETHODIMP DragEnter(LPDATAOBJECT pDataObj, DWORD keyState, POINTL pt, LPDWORD pdwEffect)
	{
		m_pDataObject = pDataObj;

		// デフォルト動作をコピーにするため、コピー可であれば
		// ctrlキー押下を模倣
		if (!(keyState & (MK_CONTROL | MK_SHIFT | MK_ALT)) && *pdwEffect & DROPEFFECT_COPY)
			keyState |= MK_CONTROL;
		m_keyState = keyState;

		POINT point;
		point.x = pt.x;
		point.y = pt.y;
		m_dropEffect = m_pTargetHandler->OnTargetDragEnter(m_pDataObject, keyState, *pdwEffect, point);
		*pdwEffect = m_dropEffect;
		return S_OK;
	}

	STDMETHODIMP DragOver(DWORD keyState, POINTL pt, LPDWORD pdwEffect)
	{
		// デフォルト動作をコピーにするため、コピー可であれば
		// ctrlキー押下を模倣
		if (!(keyState & (MK_CONTROL | MK_SHIFT | MK_ALT)) && *pdwEffect & DROPEFFECT_COPY)
			keyState |= MK_CONTROL;
		m_keyState = keyState;

		POINT point;
		point.x = pt.x;
		point.y = pt.y;
		m_dropEffect = m_pTargetHandler->OnTargetDragOver(m_pDataObject, keyState, *pdwEffect, point);
		*pdwEffect = m_dropEffect;
		return S_OK;
	}

	STDMETHODIMP DragLeave()
	{
		m_pTargetHandler->OnTargetDragLeave();
		return S_OK;
	}

	STDMETHODIMP Drop(LPDATAOBJECT pdo, DWORD keyState, POINTL pt, LPDWORD pdwEffect)
	{
		if (*pdwEffect != DROPEFFECT_NONE) {
			// 左ボタンが押されていれば、DragOver時に取得した効果を設定
			if (m_keyState & MK_LBUTTON)
				*pdwEffect = m_dropEffect;

			POINT point;
			point.x = pt.x;
			point.y = pt.y;
			*pdwEffect = m_pTargetHandler->OnTargetDrop(pdo, m_keyState, *pdwEffect, point);
		}
		else
			*pdwEffect = DROPEFFECT_NONE;

		DragLeave();
		return S_OK;
	}
};


////////////////////////////////////////////////////////////////
// CDropSource

class CDropSource : public IDropSource
{
private:
	ULONG m_refs;
	HWND m_hWnd;
	DWORD m_dropEffect;
	CDataObject* m_pDataObject;
	IDragSourceHelper* m_pSourceHelper;

public:
	CDropSource(HWND hwnd, CDataObject* pdo)
	{
		m_refs = 1;
		m_hWnd = hwnd;
		m_pSourceHelper = NULL;

		// データオブジェクトの参照カウント増
		m_pDataObject = pdo;
		m_pDataObject->AddRef();

		HRESULT hr = CoCreateInstance(CLSID_DragDropHelper, NULL, CLSCTX_INPROC_SERVER, IID_IDragSourceHelper, (void**)&m_pSourceHelper);
		if (hr == S_OK) {
			if (::IsWindow(hwnd)) {
				CPoint pt(0, 0);
				m_pSourceHelper->InitializeFromWindow(hwnd, &pt, pdo);
			}
		}
	}

	~CDropSource()
	{
		if (m_pSourceHelper)
			m_pSourceHelper->Release();
		if (m_pDataObject)
			m_pDataObject->Release();
	}

	// IUnknown methods
	STDMETHODIMP QueryInterface(REFIID riid, void FAR* FAR* ppvObj)
	{
		if (riid == IID_IUnknown ||
			riid == IID_IDropSource) {
			*ppvObj = this;
			AddRef();
			return NOERROR;
		}
		*ppvObj = NULL;
		return ResultFromScode(E_NOINTERFACE);
	}

	STDMETHODIMP_(ULONG) AddRef(void)
	{
		return ++m_refs;
	}

	STDMETHODIMP_(ULONG) Release(void)
	{
		if (--m_refs == 0) {
			delete this;
			return 0;
		}
		return m_refs;
	}

	// IDropSource methods
	STDMETHODIMP QueryContinueDrag(BOOL fEsc, DWORD grfKeyState);
	STDMETHODIMP GiveFeedback(DWORD dwEffect);
};


#endif		// __DRGDRP_H__
