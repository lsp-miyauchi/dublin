////////////////////////////////////////////////////////////////
// droptarget.cpp

#include "stdafx.h"
#include "lscore.h"
#include "common.h"
#include "idlist.h"
#include "droptarget.h"

// ???????????????????????????????????????????????????????????
// IDropTargetHepler を有効にするには、以下の定義をオンにする
#define SUPPORT_DROPTARGETHELPER


////////////////////////////////////////////////////////
//


UINT nRM_DRAGENTER = RegisterWindowMessage(_T("lft_dragenter"));
UINT nRM_DRAGOVER = RegisterWindowMessage(_T("lft_dragover"));
UINT nRM_DRAGLEAVE = RegisterWindowMessage(_T("lft_dragleave"));
UINT nRM_DROP = RegisterWindowMessage(_T("lft_drop"));


////////////////////////////////////////////////////////
//

LPDROPTARGET __fastcall BeginDropTarget(HWND hWndOwner)
{
	LPDROPTARGET pDropTarget = (LPDROPTARGET)new CDropTarget(hWndOwner);
	if (CoLockObjectExternal(pDropTarget, TRUE, FALSE) == S_OK)
	{
		if (RegisterDragDrop(hWndOwner, pDropTarget) != S_OK)
		{
			CoLockObjectExternal(pDropTarget, FALSE, FALSE);
			delete pDropTarget;
			pDropTarget = NULL;
		}
		return pDropTarget;
	}
	delete pDropTarget;
	return NULL;
}

void __fastcall EndDropTarget(HWND hWndOwner, LPDROPTARGET pDropTarget)
{
	if (pDropTarget)
	{
		RevokeDragDrop(hWndOwner);
		CoLockObjectExternal(pDropTarget, FALSE, TRUE);
		pDropTarget->Release();
	}
}

DROPEFFECT __fastcall GetDropEffect(DWORD dwSupportEffect, DWORD keyState)
{
	dwSupportEffect &= (DROPEFFECT_COPY | DROPEFFECT_MOVE);
	if (dwSupportEffect == (DROPEFFECT_COPY | DROPEFFECT_MOVE))
	{
		if (keyState & MK_SHIFT)
			return DROPEFFECT_MOVE;
		return DROPEFFECT_COPY;
	}
	else if (dwSupportEffect == DROPEFFECT_COPY)
		return DROPEFFECT_COPY;
	else if (dwSupportEffect == DROPEFFECT_MOVE)
		return DROPEFFECT_MOVE;

	return DROPEFFECT_NONE;
}

////////////////////////////////////////////////////////////////
// CDropTaget Implementation
//

CDropTarget::CDropTarget(HWND hwnd)
{
	m_refs = 1;
	m_hWnd = hwnd;
#ifdef SUPPORT_DROPTARGETHELPER
	HRESULT hr = CoCreateInstance(CLSID_DragDropHelper, NULL, CLSCTX_INPROC_SERVER,
									IID_IDropTargetHelper, (void**)&m_pHelper);
	if (FAILED(hr))
		m_pHelper = NULL;
#else
	m_pHelper = NULL;
#endif
}

////////////////////////////////////////////////////////////////
// CDropTarget::IUnknown Methods
//

STDMETHODIMP
CDropTarget::QueryInterface(REFIID iid, void** ppv)
{
	if (iid == IID_IUnknown || iid == IID_IDropTarget)
	{
		*ppv = this;
		AddRef();
		return NOERROR;
	}
	*ppv = NULL;
	return ResultFromScode(E_NOINTERFACE);
}

STDMETHODIMP_(ULONG)
CDropTarget::AddRef(void)
{
	return ++m_refs;
}

STDMETHODIMP_(ULONG)
CDropTarget::Release(void)
{
	if (--m_refs == 0)
	{
#ifdef SUPPORT_DROPTARGETHELPER
		if (m_pHelper)
			m_pHelper->Release();
#endif
		delete this;
		return 0;
	}
	return m_refs;
}


////////////////////////////////////////////////////////////////
// CDropTarget::IDropTarget Methods
//

STDMETHODIMP
CDropTarget::DragEnter(LPDATAOBJECT pDataObj, DWORD keyState, POINTL pt, LPDWORD pdwEffect)
{
	m_pDataObject = pDataObj;
	m_dropEffect = DROPEFFECT_NONE;
	m_keyState = 0;

	if (m_pHelper)
		m_pHelper->DragEnter(m_hWnd, pDataObj, (POINT*)&pt, *pdwEffect);

	return DragOver(keyState, pt, pdwEffect);
}

STDMETHODIMP
CDropTarget::DragOver(DWORD keyState, POINTL pt, LPDWORD pdwEffect)
{
	LLDROPOBJECT lldo;
	memset(&lldo, 0, sizeof(lldo));
	lldo.pDataObject = m_pDataObject;
	lldo.keyState = keyState;
	lldo.dropEffect = *pdwEffect;
	lldo.point.x = pt.x;
	lldo.point.y = pt.y;
	lldo.pHelper = m_pHelper;
	*pdwEffect = (DWORD)SendMessage(m_hWnd, nRM_DRAGOVER, 0, (LPARAM)&lldo);
	m_dropEffect = *pdwEffect;
	m_keyState = keyState;		// Drop時にはマウスボタン押下の情報がないため

	if (m_pHelper)
		m_pHelper->DragOver((POINT*)&pt, *pdwEffect);
	return NOERROR;
}

STDMETHODIMP
CDropTarget::DragLeave()
{
	if (m_pHelper)
		m_pHelper->DragLeave();

	SendMessage(m_hWnd, nRM_DRAGLEAVE, 0, 0);
	return NOERROR;
}

STDMETHODIMP
CDropTarget::Drop(LPDATAOBJECT pDataObj, DWORD keyState, POINTL pt, LPDWORD pdwEffect)
{
	if (m_pHelper)
		m_pHelper->Drop(pDataObj, (POINT*)&pt, *pdwEffect);

	HRESULT hr = NOERROR;
	if (*pdwEffect != DROPEFFECT_NONE) {
		// 左ボタンが押されていれば、DragOver時に取得した効果を設定
		if (m_keyState & MK_LBUTTON)
			*pdwEffect = m_dropEffect;

		// CF_HDROP形式のデータ
		LLDROPOBJECT lldo;
		memset(&lldo, 0, sizeof(lldo));
		lldo.keyState = m_keyState;		// DragOver時のキー情報
		lldo.dropEffect = *pdwEffect;
		lldo.point.x = pt.x;
		lldo.point.y = pt.y;
		lldo.pDataObject = pDataObj;
		*pdwEffect = (DWORD)SendMessage(m_hWnd, nRM_DROP, 0, (LPARAM)&lldo);

		// 上位側で処理するから COPY で戻る
		*pdwEffect = DROPEFFECT_COPY;
		hr = NOERROR;
	}

	DragLeave();
	return hr;
}
