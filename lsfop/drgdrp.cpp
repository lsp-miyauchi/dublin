#include "stdafx.h"
#include "fopstd.h"
#include "iconres.h"
#include "drgdrp.h"

UINT nRCF_FILEDESCRIPTOR = ::RegisterClipboardFormat(CFSTR_FILEDESCRIPTOR);
UINT nRCF_FILECONTENTS = ::RegisterClipboardFormat(CFSTR_FILECONTENTS);

////////////////////////////////////////////////////////////////
// CDataObject::IDataObject Methods
//

const FORMATETC CDataObject::m_formatetc[] =
{
	{ nRCF_FILEDESCRIPTOR, nullptr, DVASPECT_CONTENT, -1, TYMED_HGLOBAL },
	{ nRCF_FILECONTENTS, nullptr, DVASPECT_CONTENT, -1, TYMED_ISTREAM },
	{ CF_BITMAP, nullptr, DVASPECT_CONTENT, -1, TYMED_GDI }
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
		HGLOBAL hmem = CreateFileContents(pFmtc->lindex);
		if (!hmem)
			return E_OUTOFMEMORY;
		pStdm->tymed = TYMED_ISTREAM;
		CreateStreamOnHGlobal(hmem, TRUE, &pStdm->pstm);
		return S_OK;
	}

	// BITMAP
	if (pFmtc->cfFormat == CF_BITMAP) {
		HBITMAP hbm = CreateIconBitmap(pFmtc->lindex);
		if (!hbm)
			return E_OUTOFMEMORY;
		pStdm->tymed = TYMED_GDI;
		pStdm->hBitmap = hbm;
		return S_OK;
	}
	return DATA_E_FORMATETC;
}

void CDataObject::ParseData()
{
	if (!m_hFileContents || !m_hFileGroupDescriptor) {
		ICONDATAOBJECT ido;
		if (MakeIconDataObject(m_strFile.GetString(), *m_selection, &ido) > 0) {
			m_hFileGroupDescriptor = ido.hFileGroupDescriptor;
			m_hFileContents = ido.hFileContents;
		}
	}
}

STDMETHODIMP_(HGLOBAL)
CDataObject::CreateFileGroupDescriptor()
{
	ParseData();
	return lccDuplicateMemoryHandle(m_hFileGroupDescriptor, GHND | GMEM_SHARE);
}

STDMETHODIMP_(HGLOBAL)
CDataObject::CreateFileContents(long lindex)
{
	ParseData();

	HGLOBAL hmem = NULL;
	FILEGROUPDESCRIPTOR* pFileGroupDesc = (FILEGROUPDESCRIPTOR*)GlobalLock(m_hFileGroupDescriptor);
	if (lindex >= 0 && lindex < (int)pFileGroupDesc->cItems) {
		BYTE* pContents = (BYTE*)GlobalLock(m_hFileContents);
		for (int i = 0; i < lindex; i++) {
			pContents += pFileGroupDesc->fgd[i].nFileSizeLow;
		}

		hmem = GlobalAlloc(GHND | GMEM_SHARE, pFileGroupDesc->fgd[lindex].nFileSizeLow);
		if (hmem) {
			void* pv = GlobalLock(hmem);
			memcpy(pv, pContents, pFileGroupDesc->fgd[lindex].nFileSizeLow);
			GlobalUnlock(hmem);
		}
	}
	return hmem;
}

STDMETHODIMP_(HBITMAP)
CDataObject::CreateIconBitmap(long lindex)
{
	HBITMAP hbm = NULL;
	if (lindex >= 0 && lindex < m_selection->GetSize()) {
		HICON hic = (*m_selection)[lindex]->GetIcon(m_cxImage);
		if (hic) {
			BITMAP bm;
			memset(&bm, 0, sizeof(bm));
			ICONINFO ii;
			if (GetIconInfo(hic, &ii)) {
				GetObject(ii.hbmColor, sizeof(bm), &bm);
				if (ii.hbmColor) ::DeleteObject(ii.hbmColor);
				if (ii.hbmMask) ::DeleteObject(ii.hbmMask);
			}

			DWORD size = sizeof(BITMAPINFOHEADER) + WIDTHBYTES(bm.bmBitsPixel * bm.bmWidth) * bm.bmHeight;
			BITMAPINFOHEADER* pbh = (BITMAPINFOHEADER*)malloc(size);
			pbh->biSize = sizeof(BITMAPINFOHEADER);
			pbh->biWidth = bm.bmWidth;
			pbh->biHeight = bm.bmHeight;
			pbh->biPlanes = 1;
			pbh->biBitCount = bm.bmBitsPixel;
			pbh->biCompression = BI_RGB;
			void* pv = (void*)((BYTE*)pbh + sizeof(BITMAPINFOHEADER));

			CClientDC dc(NULL);
			HBITMAP hbm = ::CreateDIBitmap(dc, pbh, CBM_INIT, pv, (BITMAPINFO*)pbh, DIB_RGB_COLORS);
			CDC dcw;
			dcw.CreateCompatibleDC(dc);

			HBITMAP hbmOld = dcw.SelectBitmap(hbm);
			dcw.DrawIconEx(0, 0, hic, bm.bmWidth, bm.bmHeight, 0, NULL, DI_NORMAL);
			dcw.SelectBitmap(hbmOld);
			free((void*)pbh);
		}
	}
	return hbm;
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


extern "C"
HRESULT WINAPI fopIcmCreateDataObject(HANDLE h, const int* pIndex, int entries, IDataObject** ppdo)
{
	CIconListManager* pilm = (CIconListManager*)h;

	CSimpleArray<CIconItem*> arr;
	for (int i = 0; i < entries; i++) {
		arr.Add(pilm->m_arrIcon[pIndex[i]]);
	}
	*ppdo = new CDataObject(pilm->m_name, &arr);
	return S_OK;
}
