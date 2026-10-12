//////////////////////////////////////////////////////////////////////
// enumfe.h


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
	ULONG m_ref;					// Object reference count
	ULONG m_index;					// Current element
	ULONG m_cfe;					// Number of FORMATETCs in us
	LPFORMATETC m_prgfe;			// Source of FORMATETCs

public:
	CEnumFormatEtc(ULONG cFE, LPFORMATETC prgFE)
	{
		m_ref=0;
		m_index = 0;
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
		if (IsEqualIID(riid, IID_IUnknown) || IsEqualIID(riid, IID_IEnumFORMATETC))
			*ppv = (LPVOID)this;
		if (NULL != *ppv) {
			((LPUNKNOWN)*ppv)->AddRef();
			return NOERROR;
		}
		return ResultFromScode(E_NOINTERFACE);
	}

	STDMETHODIMP_(ULONG) AddRef(void)
	{
		++m_ref;
		return m_ref;
	}

	STDMETHODIMP_(ULONG) Release(void)
	{
		ULONG cT = --m_ref;
		if (0L == m_ref)
			delete this;
		return cT;
	}

	// IEnumFORMATETC members
	STDMETHODIMP Next(ULONG cFE, LPFORMATETC pFE, ULONG FAR * pulFE)
	{
		ULONG celt = 0L;
		if (NULL == m_prgfe)
			return ResultFromScode(S_FALSE);

		if (NULL == pulFE) {
			if (1L != cFE)
				return ResultFromScode(E_POINTER);
		} else {
			*pulFE = 0L;
		}
		if (NULL == pFE || m_index >= m_cfe)
			return ResultFromScode(S_FALSE);

		while (m_index < m_cfe && cFE > 0) {
			*pFE++ = m_prgfe[m_index++];
			celt++;
			cFE--;
		}
		if (NULL != pulFE)
			*pulFE = celt;
		return NOERROR;
	}

	STDMETHODIMP Skip(ULONG cSkip)
	{
		if (((m_index+cSkip) >= m_cfe) || NULL == m_prgfe)
			return ResultFromScode(S_FALSE);

		m_index += cSkip;
		return NOERROR;
	}

	STDMETHODIMP Reset()
	{
		m_index = 0;
		return NOERROR;
	}

	STDMETHODIMP Clone(LPENUMFORMATETC FAR *ppEnum)
	{
		LPCEnumFormatEtc pNew;
		*ppEnum = NULL;

		// Create the clone
		pNew = new CEnumFormatEtc(m_cfe, m_prgfe);

		if (NULL == pNew)
			return ResultFromScode(E_OUTOFMEMORY);

		pNew->AddRef();
		pNew->m_index = m_index;

		*ppEnum = pNew;
		return NOERROR;
	}
};
