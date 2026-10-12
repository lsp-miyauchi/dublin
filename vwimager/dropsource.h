////////////////////////////////////////////////////////////////////////
// CDropSource クラス

#pragma once
#ifndef __VWI_DROPSOURCE_H__
#define __VWI_DROPSOURCE_H__

#include "resource.h"

typedef struct tagDRAGIMAGEBITSSTRUCT {
	DWORD width;									// イメージの幅
	DWORD height;									// イメージの高さ
	DWORD xoffset;									// X カーソルオフセット
	DWORD yoffset;									// Y カーソルオフセット
	DWORD handle;									//
	DWORD clrkey;									// カラーキー
	BYTE data[1];
} DRAGIMAGEBITSSTRUCT;


class CDropSource : public IDropSource
{
private:
	ULONG m_refs;
	IDragSourceHelper* m_pHelper;
	IDataObject* m_pdo;
	CSimpleArray<HCURSOR> m_arrCursor;

public:
	CDropSource(HWND hwnd, IDataObject* pdo)
			: m_pHelper(NULL)
			, m_pdo(NULL)
	{
		// データオブジェクトの参照カウント増
		m_pdo = pdo;
		m_pdo->AddRef();

		// フィードバックカーソル作成
		HIMAGELIST him = lccLoadImageList(RESINST, RT_RCDATA, MAKEINTRESOURCE(IDB_DRAGDROP_CURSOR), 32, RGB(255, 0, 255));
		if (him) {
			for (int i = 0; i < 4; i++) {
				HCURSOR hcur = NULL;
				HICON hic = ImageList_GetIcon(him, i, ILD_TRANSPARENT);
				if (hic) {
					ICONINFO ii;
					GetIconInfo(hic, &ii);
					ii.fIcon = FALSE;
					ii.xHotspot = 0;
					ii.yHotspot = 0;
					hcur = (HCURSOR)CreateIconIndirect(&ii);
					if (ii.hbmColor) ::DeleteObject(ii.hbmColor);
					if (ii.hbmMask) ::DeleteObject(ii.hbmMask);

					DestroyIcon(hic);
				}
				m_arrCursor.Add(hcur);
			}
			ImageList_Destroy(him);
		}

		m_refs = 1;
		HRESULT hr = CoCreateInstance(CLSID_DragDropHelper, NULL, CLSCTX_INPROC_SERVER, IID_IDragSourceHelper, (void**)&m_pHelper);
		if (hr == S_OK) {
			if (IsWindow(hwnd)) {
				CPoint pt(0, 0);
				m_pHelper->InitializeFromWindow(hwnd, &pt, pdo);
			}
		}
	}

	~CDropSource()
	{
		if (m_pHelper) m_pHelper->Release();

		int cnt = m_arrCursor.GetSize();
		for (int i = 0; i < cnt; i++) {
			HCURSOR hcur = m_arrCursor[i];
			DestroyCursor(hcur);
		}

		// データオブジェクトの参照カウント減
		if (m_pdo) m_pdo->Release();
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
	STDMETHODIMP QueryContinueDrag(BOOL fEsc, DWORD keyState)
	{
		HRESULT hr = NOERROR;
		if (fEsc)
			hr = ResultFromScode(DRAGDROP_S_CANCEL);
		else if (!(keyState & (MK_LBUTTON | MK_RBUTTON)))
			hr = ResultFromScode(DRAGDROP_S_DROP);
		return hr;
	}

	STDMETHODIMP GiveFeedback(DWORD eff)
	{
		int index = -1;						// カーソルインデクス
		if (index < 0) {
			if      (eff & DROPEFFECT_COPY)	index = 1;
			else if (eff & DROPEFFECT_MOVE)	index = 2;
			else if (eff & DROPEFFECT_LINK)	index = 3;
			else                           	index = 0;
		}

		::SetCursor(m_arrCursor[index]);
		return ResultFromScode(S_OK);
	}
};


#endif			// __VWI_DROPSOURCE_H__
