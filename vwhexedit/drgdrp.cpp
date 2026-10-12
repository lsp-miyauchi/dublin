#include "stdafx.h"
#include "drgdrp.h"

UINT nRCF_FILEDESCRIPTOR = ::RegisterClipboardFormat(CFSTR_FILEDESCRIPTOR);
UINT nRCF_FILECONTENTS = ::RegisterClipboardFormat(CFSTR_FILECONTENTS);

////////////////////////////////////////////////////////////////
// CDataObject::IDataObject Methods
//

const FORMATETC CDataObject::m_formatetc[] =
{
	{ nRCF_FILEDESCRIPTOR, nullptr, DVASPECT_CONTENT, -1, TYMED_HGLOBAL },
	{ nRCF_FILECONTENTS, nullptr, DVASPECT_CONTENT, -1, TYMED_ISTREAM }
};

STDMETHODIMP
CDataObject::GetData(FORMATETC* pFmtc, STGMEDIUM* pStdm)
{
	pStdm->tymed = NULL;
	pStdm->pUnkForRelease = NULL;

	if (pFmtc->dwAspect != DVASPECT_CONTENT)
		return DATA_E_FORMATETC;

	// FILEGROUPDESCRIPTOR
	if (pFmtc->cfFormat == nRCF_FILEDESCRIPTOR) {
		HGLOBAL hmem = CreateFileGroupDescriptor();
		if (!hmem)
			return E_OUTOFMEMORY;

		pStdm->tymed = TYMED_HGLOBAL;
		pStdm->hGlobal = hmem;
		return S_OK;
	}

	// FILECONTENTS
	if (pFmtc->cfFormat == nRCF_FILECONTENTS) {
		HGLOBAL hmem = lccDuplicateMemoryHandle(m_hmemData, GHND | GMEM_SHARE);
		if (!hmem)
			return E_OUTOFMEMORY;
		pStdm->tymed = TYMED_ISTREAM;
		CreateStreamOnHGlobal(hmem, TRUE, &pStdm->pstm);
		return S_OK;
	}

	return DATA_E_FORMATETC;
}

HGLOBAL CDataObject::CreateFileGroupDescriptor()
{
	if (!m_hmemDesc) {
		HGLOBAL hmem = GlobalAlloc(GHND | GMEM_SHARE, sizeof(FILEGROUPDESCRIPTOR));
		FILEGROUPDESCRIPTOR* pFileGroupDescriptor = (FILEGROUPDESCRIPTOR*)GlobalLock(hmem);
		pFileGroupDescriptor->cItems = 1;

		FILEDESCRIPTOR* pFileDesc = &pFileGroupDescriptor->fgd[0];
		memset(pFileDesc, 0, sizeof(FILEDESCRIPTOR));
		pFileDesc->dwFlags = FD_PROGRESSUI | FD_FILESIZE | FD_UNICODE;
		lstrcpy(pFileDesc->cFileName, PathFindFileName(m_strFile.GetString()));
		lstrcat(pFileDesc->cFileName, _T(".part.bin"));
		pFileDesc->nFileSizeLow = m_sizeData;
		m_hmemDesc = hmem;
	}
	return lccDuplicateMemoryHandle(m_hmemDesc, GHND | GMEM_SHARE);
}

STDMETHODIMP
CDataObject::QueryGetData(FORMATETC* pFmtc)
{
	if (pFmtc->dwAspect == DVASPECT_CONTENT) {
		if ((pFmtc->cfFormat == nRCF_FILEDESCRIPTOR && pFmtc->tymed == TYMED_HGLOBAL) ||
			(pFmtc->cfFormat == nRCF_FILECONTENTS && pFmtc->tymed == TYMED_ISTREAM) ||
			(pFmtc->cfFormat == CF_BITMAP && pFmtc->tymed == TYMED_GDI)) {
			return S_OK;
		}
	}
	return E_INVALIDARG;
}

STDMETHODIMP
CDataObject::EnumFormatEtc(DWORD dwDirection, IEnumFORMATETC** ppenumFmtc)
{
	switch (dwDirection) {
	case DATADIR_GET:
		return CreateFormatEnumerator(ARRAYSIZE(m_formatetc), const_cast<FORMATETC*>(m_formatetc), ppenumFmtc);
	case DATADIR_SET:
		return E_NOTIMPL;
	default:
		break;
	}
	return E_INVALIDARG;
}

