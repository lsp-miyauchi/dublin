#include "stdafx.h"
#include "arcitem.h"
#include "drgdrp.h"


////////////////////////////////////////////////////////////////
// CDataObject::IDataObject Methods
//

const FORMATETC CDataObject::m_formatetc[] =
{
	{ CF_HDROP, nullptr, DVASPECT_CONTENT, -1, TYMED_HGLOBAL }
};


STDMETHODIMP
CDataObject::GetData(FORMATETC* pFmtc, STGMEDIUM* pStdm)
{
	if (pFmtc->dwAspect == DVASPECT_CONTENT && pFmtc->cfFormat == CF_HDROP) {
		HGLOBAL hmem = CreateDropFiles();
		if (!hmem)
			return E_OUTOFMEMORY;

		pStdm->tymed = TYMED_HGLOBAL;
		pStdm->hGlobal = hmem;
		pStdm->pUnkForRelease = NULL;
		return S_OK;
	}
	return DATA_E_FORMATETC;
}

STDMETHODIMP_(HGLOBAL)
CDataObject::CreateDropFiles()
{
	HDROP hDrop = NULL;
	if (!m_hdrop) {
		int cnt = m_pArray->GetSize();
		if (m_pArray && cnt > 0) {
			POINT pt;
			pt.x = pt.y = 0;
			hDrop = fopDragInitBlock(pt, FALSE);
			if (hDrop) {
				for (int i = 0; i < cnt; i++) {
					CArcItem* pai = (CArcItem*)(*m_pArray)[i];
					if (pai->m_strRealName.IsEmpty())
						continue;

					CPath path;
					path.Combine(m_strTempFolder, pai->m_strRealName);
					path.m_strPath.Replace(_T('/'), _T('\\'));
					path.RemoveBackslash();

					HDROP hw = fopDragAppendFile(hDrop, path);
					if (!hw) {
						DragFinish(hDrop);
						return NULL;
					}
					hDrop = hw;
				}
				m_hdrop = hDrop;
			}
		}
	}

	if (m_hdrop) {
		SIZE_T size = GlobalSize(m_hdrop);
		hDrop = (HDROP)GlobalAlloc(GHND | GMEM_SHARE, size);
		if (hDrop) {
			void* ps = GlobalLock(m_hdrop);
			void* pt = GlobalLock(hDrop);
			memcpy(pt, ps, size);
			GlobalUnlock(m_hdrop);
			GlobalUnlock(hDrop);
		}
	}
	return hDrop;
}

STDMETHODIMP_(HGLOBAL) CDataObject::CreateFileGroupDescriptor() { return NULL; }
STDMETHODIMP_(HGLOBAL) CDataObject::CreateFileContents(long /*lindex*/) { return NULL; }

STDMETHODIMP_(BOOL)
CDataObject::Extract()
{
	// コマンドラインの設定
	bool bReplaceSlash = true;
	LPCTSTR pszFormat = NULL;
	switch (m_nArchiveType) {
	case ARC_LZH:
		// 解凍状況を抑止する "-n"
		pszFormat = _T("e -c1r2x1a2m0p1s0n1y1 -jf0jp1 -jyc1 -+ %s %s @%s");
		break;
	case ARC_ZIP:
		// 解凍状況を抑止（--i）し、常に上書き（-o）する
		bReplaceSlash = false;
		pszFormat = _T("-x --i -o %s %s @%s");
		break;
	case ARC_CAB:
		// 解凍状況を抑止する "-i"
		bReplaceSlash = false;
		pszFormat = _T("-x -i -o %s %s @%s");
		break;
	case ARC_TAR:
		// 解凍状況を抑止 "--display-dialog=0"
		bReplaceSlash = false;
		pszFormat = _T("--use-directory=0 --display-dialog=0 -xf %s -o %s @%s");
		break;
	case ARC_RAR:
		bReplaceSlash = false;
// for unrar32.dll		pszFormat = _T("e -o --i %s %s @%s");
		pszFormat = _T("x -o -i -j %s %s @%s");
		break;
	case ARC_7ZIP:
	case ARC_ISO:
	case ARC_RPM:
		// 解凍状況抑止 "-hide" 上書き "-aoa" 出力先 "-o<path>"
		pszFormat = _T("x %s -hide -aoa -o%s @%s");
		break;
	case ARC_ARJ:
		// 解凍状況抑止 "-i"   すべての質問にyesと仮定 "-y"
		pszFormat = _T("x -hk -i -y %s %s @%s");
		break;
	case ARC_BGA:
		// 解凍状況を抑止する "-i"
		pszFormat = _T("x -i -j -a -o %s %s @%s");
		break;
	case ARC_GCA:
		// 解凍状況を抑止する "-i"
		pszFormat = _T("-e -i -j %s %s @%s");
		break;
	case ARC_YZ1:
		// 解凍状況を抑止する "-i"
		pszFormat = _T("x -i %s %s @%s");
		break;
	default:
		break;
	}

	BOOL bOK = TRUE;
	if (pszFormat) {
		CPath tmp = m_strTempFolder;
		tmp.AddBackslash();

		CString cmd;
		cmd.Format(pszFormat, (LPCTSTR)m_strArchive, (LPCTSTR)tmp, (LPCTSTR)m_strResponse);
		if (bReplaceSlash)
			cmd.Replace('/', '\\');

		CComPtr<IArcManager> pam;
		HRESULT hr = pam.CoCreateInstance(__uuidof(ArcManager));
		if (SUCCEEDED(hr))
			hr = pam->ArchiveW(NULL, m_nArchiveType, cmd, NULL, 0, NULL);
		bOK = (SUCCEEDED(hr) ? TRUE : FALSE);
	}
	return bOK;
}

STDMETHODIMP
CDataObject::QueryGetData(FORMATETC* pFmtc)
{
	if (pFmtc->dwAspect == DVASPECT_CONTENT && pFmtc->cfFormat == CF_HDROP && pFmtc->tymed == TYMED_HGLOBAL) {
		return S_OK;
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

////////////////////////////////////////////////////////////////
// CDropSource Implementation
//

STDMETHODIMP CDropSource::QueryContinueDrag(BOOL fEsc, DWORD grfKeyState)
{
	if (fEsc)
		return ResultFromScode(DRAGDROP_S_CANCEL);

	if (grfKeyState & (MK_LBUTTON | MK_RBUTTON))
		return S_OK;

	if (m_dropEffect != DROPEFFECT_NONE) {
		if (m_pDataObject->Extract())
			return DRAGDROP_S_DROP;
	}
	return DRAGDROP_S_CANCEL;
}

STDMETHODIMP CDropSource::GiveFeedback(DWORD dwEffect)
{
	m_dropEffect = dwEffect;
	return ResultFromScode(DRAGDROP_S_USEDEFAULTCURSORS);
}
