////////////////////////////////////////////////////////////////
// drgdrp.h

#pragma once
#ifndef __DRGDRP_H__
#define __DRGDRP_H__

////////////////////////////////////////////////////////////////
// CDataObject

class CDataObject : public IDataObject
{
protected:
	ULONG m_cRef;
	CString m_strFile;
	HGLOBAL m_hmemDesc;
	HGLOBAL m_hmemData;
	UINT m_sizeData;
	static const FORMATETC m_formatetc[];

public:
	CDataObject(LPCTSTR pszFile, void* dataptr, UINT size)
		: m_cRef(1)
		, m_strFile(pszFile)
		, m_hmemDesc(NULL)
		, m_hmemData(NULL)
		, m_sizeData(0)
	{
		if (dataptr && size) {
			m_hmemData = GlobalAlloc(GHND | GMEM_SHARE, size);
			void* pv = GlobalLock(m_hmemData);
			memcpy(pv, dataptr, size);
			GlobalUnlock(m_hmemData);
		}
	}
	~CDataObject()
	{
		if (m_hmemDesc) { GlobalFree(m_hmemDesc); }
		if (m_hmemData) { GlobalFree(m_hmemData); }
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
private:
	void ParseData();
	HGLOBAL CreateFileGroupDescriptor();

};

#endif		// __DRGDRP_H__
