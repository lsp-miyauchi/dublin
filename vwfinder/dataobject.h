////////////////////////////////////////////////////////////////
// dataobject.h

#define GMSHARE		(GMEM_SHARE | GMEM_ZEROINIT)
#define SETFormatEtc(fe, cf, med, li) {\
			(fe).cfFormat = cf;\
			(fe).dwAspect = DVASPECT_CONTENT;\
			(fe).ptd = NULL;\
			(fe).tymed = med;\
			(fe).lindex = li; };

//////////////////////////////////////////////////////////////////////
//

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

	~CEnumFormatEtc(void)
	{
		if (NULL != m_prgfe) delete [] m_prgfe;
	}

	// IUnknown members that delegate to m_pUnkOuter.
	STDMETHODIMP QueryInterface(REFIID riid, LPVOID FAR *ppv)
	{
		*ppv = NULL;
		if (IsEqualIID(riid, IID_IUnknown) || IsEqualIID(riid, IID_IEnumFORMATETC))
			*ppv=(LPVOID)this;

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
		if (0L == m_cRef) delete this;
		return cRefT;
	}

	// IEnumFORMATETC members
	STDMETHODIMP Next(ULONG cFE, LPFORMATETC pFE, ULONG FAR * pulFE)
	{
		ULONG cReturn = 0L;
		if (NULL == m_prgfe) return ResultFromScode(E_INVALIDARG);

		if (NULL == pulFE) {
			if (1L != cFE) return ResultFromScode(E_POINTER);
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
		if (NULL != pulFE) *pulFE=cReturn;
		return NOERROR;
	}

	STDMETHODIMP Skip(ULONG cSkip)
	{
		if (((m_iCur+cSkip) >= m_cfe) || NULL == m_prgfe) return ResultFromScode(S_FALSE);

		m_iCur += cSkip;
		return NOERROR;
	}

	STDMETHODIMP Reset(void)
	{
		m_iCur = 0;
		return NOERROR;
	}

	STDMETHODIMP Clone(LPENUMFORMATETC FAR *ppEnum)
	{
		*ppEnum = NULL;
		LPCEnumFormatEtc pNew = new CEnumFormatEtc(m_cfe, m_prgfe);

		if (NULL == pNew) return ResultFromScode(E_OUTOFMEMORY);

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
protected:
	HDROP m_hdrop;
	ULONG m_cRef;
	UINT m_CF_PREFERREDDROPEFFECT;
	DWORD m_effPreferred;
public:
	CDataObject(HDROP hdrop, DWORD effPreferred = DROPEFFECT_COPY)
	{
		m_cRef = 1;
		m_hdrop = hdrop;
		m_CF_PREFERREDDROPEFFECT = ::RegisterClipboardFormat(CFSTR_PREFERREDDROPEFFECT);
		m_effPreferred = effPreferred;
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

	STDMETHODIMP GetData(FORMATETC* pFmtc, STGMEDIUM* pStdm)
	{
		pStdm->tymed = NULL;
		pStdm->pUnkForRelease = NULL;
		pStdm->hGlobal = NULL;

		if (pFmtc->tymed == TYMED_HGLOBAL && pFmtc->dwAspect == DVASPECT_CONTENT) {
			HGLOBAL hmem;
			if (pFmtc->cfFormat == CF_HDROP)
				hmem = lccDuplicateMemoryHandle(m_hdrop, GMSHARE);
			else if (pFmtc->cfFormat == m_CF_PREFERREDDROPEFFECT) {
				hmem = GlobalAlloc(GMSHARE, sizeof(DWORD));
				if (hmem) {
					DWORD* pdw = (DWORD*)GlobalLock(hmem);
					*pdw = m_effPreferred;
					GlobalUnlock(hmem);
				}
			}
			else
				return ResultFromScode(DATA_E_FORMATETC);

			if (hmem) {
				pStdm->tymed = TYMED_HGLOBAL;
				pStdm->hGlobal = hmem;
				return ResultFromScode(S_OK);
			}
			return ResultFromScode(E_OUTOFMEMORY);
		}
		return ResultFromScode(DATA_E_FORMATETC);
	}

	STDMETHODIMP GetDataHere(FORMATETC* pFmtc, STGMEDIUM* pStdm)
	{
		return ResultFromScode(E_NOTIMPL);
	}

	STDMETHODIMP QueryGetData(FORMATETC* pFmtc)
	{
		if (pFmtc->dwAspect == DVASPECT_CONTENT &&
			pFmtc->tymed & TYMED_HGLOBAL &&
			pFmtc->lindex == -1) {
			if (pFmtc->cfFormat == CF_HDROP || pFmtc->cfFormat == m_CF_PREFERREDDROPEFFECT)
				return ResultFromScode(S_OK);
		}
		return ResultFromScode(DATA_E_FORMATETC);
	}

	STDMETHODIMP GetCanonicalFormatEtc(FORMATETC* pFmtcIn, FORMATETC* pFmtcOut)
	{
		pFmtcOut->ptd = NULL;
		return ResultFromScode(E_NOTIMPL);
	}

	STDMETHODIMP SetData(FORMATETC* pFmtc, STGMEDIUM* pStdm, BOOL fRelease)
	{
		return ResultFromScode(E_NOTIMPL);
	}

	STDMETHODIMP EnumFormatEtc(DWORD dwDirection, IEnumFORMATETC** ppenumFmtc)
	{
		FORMATETC fe[2];
		SETFormatEtc(fe[0], CF_HDROP, TYMED_HGLOBAL, -1);
		SETFormatEtc(fe[1], m_CF_PREFERREDDROPEFFECT, TYMED_HGLOBAL, -1);
		SCODE sc = S_OK;
		if (dwDirection == DATADIR_GET) {
			*ppenumFmtc = (IEnumFORMATETC*)new CEnumFormatEtc(2, fe);
			if (*ppenumFmtc == NULL) sc = E_OUTOFMEMORY;
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

};
