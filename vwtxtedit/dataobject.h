////////////////////////////////////////////////////////////////
// dataobject.h

#pragma once
#ifndef __DATAOBJECT_H__
#define __DATAOBJECT_H__


#define SETFormatEtc(fe, cf, asp, td, med, li)\
		{\
			(fe).cfFormat = cf;\
			(fe).dwAspect = asp;\
			(fe).ptd = td;\
			(fe).tymed = med;\
			(fe).lindex = li;\
		};

#define SETDefFormatEtc(fe, cf, med)\
		{\
			(fe).cfFormat = cf;\
			(fe).dwAspect = DVASPECT_CONTENT;\
			(fe).ptd = NULL;\
			(fe).tymed = med;\
			(fe).lindex = -1;\
		};

//////////////////////////////////////////////////////////////////////
//
/*
 * IEnumFORMATETC object that is created from
 * IDataObject::EnumFormatEtc.  This object lives on its own,
 * that is, QueryInterface only knows IUnknown and IEnumFormatEtc,
 * nothing more.  We still use an outer unknown for reference
 * counting, because as long as this enumerator lives, the data
 * object should live, thereby keeping the application up.
 */

class CEnumFormatEtc;
typedef class CEnumFormatEtc* LPCEnumFormatEtc;

class CEnumFormatEtc : public IEnumFORMATETC
{
private:
	ULONG m_cRef;					// Object reference count
	ULONG m_iCur;					// Current element
	ULONG m_cfe;					// Number of FORMATETCs in us
	LPFORMATETC m_prgfe;			// Source of FORMATETCs

public:
	CEnumFormatEtc(ULONG cFE, LPFORMATETC prgFE)
	{
		m_cRef=0;
		m_iCur = 0;
		m_cfe = cFE;
		m_prgfe = new FORMATETC[(UINT)cFE];

		if (NULL != m_prgfe) {
			for (UINT i = 0; i < cFE; i++)
				m_prgfe[i] = prgFE[i];
		}
	}

	~CEnumFormatEtc()
	{
		if (NULL != m_prgfe)
			delete [] m_prgfe;
	}

	// IUnknown members that delegate to m_pUnkOuter.
	STDMETHODIMP QueryInterface(REFIID riid, LPVOID FAR *ppv)
	{
		*ppv = NULL;

		// Enumerators are separate objects, not the data object, so
		// we only need to support out IUnknown and IEnumFORMATETC
		// interfaces here with no concern for aggregation.

		if (IsEqualIID(riid, IID_IUnknown) || IsEqualIID(riid, IID_IEnumFORMATETC))
			*ppv=(LPVOID)this;

		// AddRef any interface we'll return.
		if (NULL != *ppv) {
			((LPUNKNOWN)*ppv)->AddRef();
			return NOERROR;
		}
		return ResultFromScode(E_NOINTERFACE);
	}

	STDMETHODIMP_(ULONG) AddRef(void)
	{
		++m_cRef;
		return m_cRef;
	}

	STDMETHODIMP_(ULONG) Release(void)
	{
		ULONG cRefT;
		cRefT = --m_cRef;
		if (0L == m_cRef)
			delete this;
		return cRefT;
	}

	// IEnumFORMATETC members
	STDMETHODIMP Next(ULONG cFE, LPFORMATETC pFE, ULONG FAR * pulFE)
	{
		ULONG cReturn = 0L;

		if (NULL == m_prgfe)
			return ResultFromScode(E_INVALIDARG);

		if (NULL == pulFE) {
			if (1L != cFE)
				return ResultFromScode(E_POINTER);
		}
		else
			*pulFE = 0L;

		if (NULL == pFE || m_iCur >= m_cfe)
			return ResultFromScode(E_INVALIDARG);

		while (m_iCur < m_cfe && cFE > 0) {
			*pFE++ = m_prgfe[m_iCur++];
			cReturn++;
			cFE--;
		}

		if (NULL != pulFE)
			*pulFE=cReturn;

		return NOERROR;
	}

	STDMETHODIMP Skip(ULONG cSkip)
	{
		if (((m_iCur+cSkip) >= m_cfe) || NULL == m_prgfe)
			return ResultFromScode(S_FALSE);

		m_iCur += cSkip;
		return NOERROR;
	}

	STDMETHODIMP Reset()
	{
		m_iCur = 0;
		return NOERROR;
	}

	STDMETHODIMP Clone(LPENUMFORMATETC FAR *ppEnum)
	{
		*ppEnum = NULL;

		// Create the clone
		LPCEnumFormatEtc pNew = new CEnumFormatEtc(m_cfe, m_prgfe);

		if (NULL == pNew)
			return ResultFromScode(E_OUTOFMEMORY);

		pNew->AddRef();
		pNew->m_iCur = m_iCur;

		*ppEnum = pNew;
		return NOERROR;
	}
};

////////////////////////////////////////////////////////////////
// CDataObject

class CDataObject : public IDataObject
{
public:
	CDataObject(LPCTSTR pText)
	{
		m_cRef = 1;

		int len = lstrlen(pText);
		m_hText = GlobalAlloc(GMEM_MOVEABLE, (len + 16) * sizeof(TCHAR));
		if (m_hText) {
			TCHAR* p = (TCHAR*)GlobalLock(m_hText);
			lstrcpy(p, pText);
			GlobalUnlock(m_hText);
		}
	}

	~CDataObject()
	{
		if (m_hText)
			GlobalFree(m_hText);
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

	STDMETHODIMP GetData(FORMATETC* pFmtc, STGMEDIUM* pStdm)
	{
		pStdm->tymed = NULL;
		pStdm->pUnkForRelease = NULL;
		pStdm->hGlobal = NULL;

		// This method is called by the drag-drop target to obtain the text
		// that is being dragged.
		if ((pFmtc->cfFormat == CF_UNICODETEXT || pFmtc->cfFormat == CF_TEXT) &&
			pFmtc->dwAspect == DVASPECT_CONTENT &&
			pFmtc->tymed == TYMED_HGLOBAL) {
			SIZE_T size = GlobalSize(m_hText);
			HGLOBAL hmem = GlobalAlloc(GMEM_SHARE | GMEM_ZEROINIT, size);
			if (!hmem)
				return ResultFromScode(E_OUTOFMEMORY);

			void* pDst = GlobalLock(hmem);
			void* pSrc = GlobalLock(m_hText);
			if (pFmtc->cfFormat == CF_UNICODETEXT)
				memcpy(pDst, pSrc, size);
			else
				WideCharToMultiByte(CP_ACP, 0, (LPCWSTR)pSrc, -1, (LPSTR)pDst, (int)size, NULL, NULL);
			GlobalUnlock(hmem);
			GlobalUnlock(m_hText);

			pStdm->tymed = TYMED_HGLOBAL;
			pStdm->hGlobal = hmem;
			return ResultFromScode(S_OK);
		}
		return ResultFromScode(DATA_E_FORMATETC);
	}

	STDMETHODIMP GetDataHere(FORMATETC* pFmtc, STGMEDIUM* pStdm)
	{
		return ResultFromScode(E_NOTIMPL);
	}

	STDMETHODIMP QueryGetData(FORMATETC* pFmtc)
	{
		if ((pFmtc->cfFormat == CF_UNICODETEXT || pFmtc->cfFormat == CF_TEXT) &&
			pFmtc->dwAspect == DVASPECT_CONTENT &&
			pFmtc->tymed & TYMED_HGLOBAL)
			return ResultFromScode(S_OK);
		else
			return ResultFromScode(S_FALSE);
	}

	STDMETHODIMP GetCanonicalFormatEtc(FORMATETC* pFmtcIn, FORMATETC* pFmtcOut)
	{
		pFmtcOut->ptd = NULL; return ResultFromScode(E_NOTIMPL);
	}

	STDMETHODIMP SetData(FORMATETC* pFmtc, STGMEDIUM* pStdm, BOOL fRelease)
	{
		return ResultFromScode(E_NOTIMPL);
	}

	STDMETHODIMP EnumFormatEtc(DWORD dwDirection, IEnumFORMATETC** ppenumFmtc)
	{
		//	return ResultFromScode(E_NOTIMPL);

		// A standard implementation is provided by OleStdEnumFmtEtc_Create
		// which can be found in \ole2\samp\ole2ui\enumfetc.c in the OLE 2 SDK.
		// This code from ole2ui is copied to the enumfetc.c file in this sample.

		FORMATETC fe;
		SETDefFormatEtc(fe, CF_TEXT, TYMED_HGLOBAL);
		SCODE sc = S_OK;
		if (dwDirection == DATADIR_GET) {
			*ppenumFmtc = (IEnumFORMATETC*)new CEnumFormatEtc(1, &fe);
			if (*ppenumFmtc == NULL)
				sc = E_OUTOFMEMORY;
		}
		else if (dwDirection == DATADIR_SET)
			sc = E_NOTIMPL;
		else
			sc = E_INVALIDARG;

		return ResultFromScode(sc);
	}

	STDMETHODIMP DAdvise(FORMATETC* pFmtc, DWORD advf, IAdviseSink* pAdvSink, DWORD* pdwConnection)
	{
		return ResultFromScode(OLE_E_ADVISENOTSUPPORTED);
	}

	STDMETHODIMP DUnadvise(DWORD dwConnection)
	{
		return ResultFromScode(OLE_E_ADVISENOTSUPPORTED);
	}

	STDMETHODIMP EnumDAdvise(IEnumSTATDATA** ppenumAdvise)
	{
		return ResultFromScode(OLE_E_ADVISENOTSUPPORTED);
	}

protected:
	HGLOBAL m_hText;
	ULONG m_cRef;
};







////////////////////////////////////////////////////////////////
// CDropSource

class CDropSource : public IDropSource
{
public:
	CDropSource()
	{
		m_refs = 1;
		m_bAcceptFmt = FALSE;
	}

	// IUnknown methods
	STDMETHODIMP QueryInterface(REFIID iid, void** ppv)
	{
		if (iid == IID_IUnknown || iid == IID_IDropSource) {
			*ppv = this;
			AddRef();
			return NOERROR;
		}
		*ppv = NULL;
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
	STDMETHODIMP QueryContinueDrag(BOOL fEsc, DWORD grfKeyState)
	{
		if (fEsc)
			return ResultFromScode(DRAGDROP_S_CANCEL);

		if (!(grfKeyState & MK_LBUTTON))
			return ResultFromScode(DRAGDROP_S_DROP);
		return NOERROR;
	}

	STDMETHODIMP GiveFeedback(DWORD dwEffect)
	{
		return ResultFromScode(DRAGDROP_S_USEDEFAULTCURSORS);
	}

private:
	ULONG m_refs;
	BOOL m_bAcceptFmt;
};


#endif		// __DATAOBJECT_H__
