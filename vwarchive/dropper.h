//////////////////////////////////////////////////////////////////////
// dropper.h

#pragma once
#ifndef __DROPPER_H__
#define __DROPPER_H__

#include "drgdrp.h"

//////////////////////////////////////////////////////////////////////
//

template <class T>
class CDropperImpl : public CDropTargetHandler
{
private:
	UINT m_nRM_GETDRAGIMAGE;
	bool m_bDppDragEnter:1;
	bool m_bDppDragMyself:1;
	bool m_bDppWaitingForDestroyWindow:1;
	bool m_bDppHDROP:1;
	DWORD m_uDppScrollTimer;
	DWORD m_uDppScrollTimerLimit;
	IDropTarget* m_pDppDropTarget;
	IDropTargetHelper* m_pDppTargetHelper;
	IDataObject* m_pDppCurrentDataObject;
	CDropSource* m_pDppDropSource;

public:
	CDropperImpl()
		: m_nRM_GETDRAGIMAGE(::RegisterWindowMessage(DI_GETDRAGIMAGE))
		, m_pDppDropTarget(NULL)
		, m_bDppDragEnter(false)
		, m_bDppDragMyself(false)
		, m_bDppWaitingForDestroyWindow(false)
		, m_bDppHDROP(false)
		, m_uDppScrollTimer(0)
		, m_uDppScrollTimerLimit(0)
		, m_pDppTargetHelper(NULL)
		, m_pDppCurrentDataObject(NULL)
		, m_pDppDropSource(NULL)
	{
	}

	// --------------------------------------------
	// CDropTargetHandler - OnTargetDragEnter
	// --------------------------------------------
	DWORD OnTargetDragEnter(IDataObject* pDataObject, DWORD keyState, DWORD dropEffect, POINT point)
	{
		// コピーして参照カウントをアップ
		m_pDppCurrentDataObject = pDataObject;
		m_pDppCurrentDataObject->AddRef();

		// 利用可能なフォーマットを取得
		m_bDppHDROP = QueryDataObjectFormat(CF_HDROP);

		T* pT = static_cast<T*>(this);
		if (S_OK == CoCreateInstance(CLSID_DragDropHelper, NULL, CLSCTX_INPROC_SERVER, IID_IDropTargetHelper, (void**)&m_pDppTargetHelper))
			m_pDppTargetHelper->DragEnter(pT->m_hWnd, pDataObject, &point, dropEffect);
		else
			m_pDppTargetHelper = NULL;

		ResetDragScrollTimer();
		m_bDppDragEnter = true;

		return pT->OnDragEnter(pDataObject, keyState, dropEffect, point);
	}

	// --------------------------------------------
	// CDropTargetHandler - OnTargetDragOver
	// --------------------------------------------
	DWORD OnTargetDragOver(IDataObject* pDataObject, DWORD keyState, DWORD dropEffect, POINT point)
	{
		T* pT = static_cast<T*>(this);
		if (m_pDppTargetHelper)
			m_pDppTargetHelper->DragOver(&point, dropEffect);

		return pT->OnDragOver(pDataObject, keyState, dropEffect, point);
	}

	// --------------------------------------------
	// CDropTargetHandler - OnTargetDragLeave
	// --------------------------------------------
	void OnTargetDragLeave()
	{
		if (m_pDppTargetHelper)
			m_pDppTargetHelper->DragLeave();

		T* pT = static_cast<T*>(this);
		pT->OnDragLeave();
		m_bDppDragEnter = false;
		pT->OnDropExit();
	}

	// --------------------------------------------
	// CDropTargetHandler - OnTargetDrop
	// --------------------------------------------
	DWORD OnTargetDrop(IDataObject* pDataObject, DWORD keyState, DWORD dropEffect, POINT point)
	{
		T* pT = static_cast<T*>(this);
		if (m_pDppTargetHelper)
			m_pDppTargetHelper->Drop(pDataObject, &point, dropEffect);

		DWORD eff = pT->OnDrop(pDataObject, keyState, dropEffect, point);
		m_bDppDragEnter = false;
		pT->OnDropExit();

		if (m_bDppWaitingForDestroyWindow) {
			m_bDppWaitingForDestroyWindow = false;
			if (pT->IsWindow())
				pT->PostMessage(WM_CLOSE);
		}

		return eff;
	}

BEGIN_MSG_MAP(CDropperImpl)
	MESSAGE_HANDLER(WM_CREATE, OnDppCreate)
	MESSAGE_HANDLER(WM_DESTROY, OnDppDestroy)
	ON_REGISTERD_MESSAGE(m_nRM_GETDRAGIMAGE, OnDppGetDragImage)
END_MSG_MAP()

	LRESULT OnDppCreate(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
	{
		BeginDropTarget();
		bHandled = FALSE;
		return 0;
	}

	LRESULT OnDppDestroy(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
	{
		EndDropTarget();
		bHandled = FALSE;

		return 0;
	}

	LRESULT OnDppGetDragImage(WPARAM, LPARAM lParam)
	{
		T* pT = static_cast<T*>(this);
		return (LRESULT)pT->OnGetDragImage((SHDRAGIMAGE*)lParam);
	}

	// overridable
	virtual BOOL OnGetDragImage(SHDRAGIMAGE* psdi)
	{
		return FALSE;
	}

	// overridable
	virtual DWORD OnDragEnter(IDataObject* pdo, DWORD keyState, DWORD dropEffect, POINT point)
	{
		T* pT = static_cast<T*>(this);
		return pT->OnDragOver(pdo, keyState, dropEffect, point);
	}

	// overridable
	virtual DWORD OnDragOver(IDataObject* pdo, DWORD keyState, DWORD dropEffect, POINT point)
	{
		return DROPEFFECT_NONE;
	}

	// overridable
	virtual void OnDragLeave()
	{
	}

	// overridable
	virtual DWORD OnDrop(IDataObject* pdo, DWORD keyState, DWORD dropEffect, POINT point)
	{
		return DROPEFFECT_NONE;
	}

	// overridable
	void OnDropExit()
	{
		if (m_pDppTargetHelper) {
			m_pDppTargetHelper->Release();
			m_pDppTargetHelper = NULL;
		}

		if (m_pDppCurrentDataObject) {
			m_pDppCurrentDataObject->Release();
			m_pDppCurrentDataObject = NULL;
		}
	}

	inline bool IsDragging() { return m_bDppDragEnter; }
	inline bool IsDragMyself() { return m_bDppDragMyself; }
	inline bool IsDropFormatsHDROP() { return m_bDppHDROP; }
	inline void SetDropWaitingForDestroyWindow(bool bSet) { m_bDppWaitingForDestroyWindow = bSet; }


	void SetDragScrollTimer(DWORD timer)
	{
		m_uDppScrollTimer = timer;
		ResetDragScrollTimer();
	}

	inline void CancelDragScrollTimer() { SetDragScrollTimer(0); }

	void ResetDragScrollTimer()
	{
		m_uDppScrollTimerLimit = (m_uDppScrollTimer != 0) ? GetTickCount() + m_uDppScrollTimer : 0;
	}

	bool IsTimeToDragScroll()
	{
		bool b = (m_uDppScrollTimer && (GetTickCount() > m_uDppScrollTimerLimit)) ? true : false;
		if (b) ResetDragScrollTimer();
		return b;
	}

	void BeginDropTarget()
	{
		T* pT = static_cast<T*>(this);
		LPDROPTARGET pDropTarget = (LPDROPTARGET)new CDropTarget(this);
		if (CoLockObjectExternal(pDropTarget, TRUE, FALSE) == S_OK) {
			if (RegisterDragDrop(pT->m_hWnd, pDropTarget) != S_OK) {
				CoLockObjectExternal(pDropTarget, FALSE, FALSE);
				delete pDropTarget;
				pDropTarget = NULL;
			}
		}
		else {
			delete pDropTarget;
			pDropTarget = NULL;
		}
		m_pDppDropTarget = pDropTarget;
	}

	void EndDropTarget()
	{
		T* pT = static_cast<T*>(this);
		if (m_pDppDropTarget) {
			RevokeDragDrop(pT->m_hWnd);
			CoLockObjectExternal(m_pDppDropTarget, FALSE, TRUE);
			m_pDppDropTarget->Release();
			m_pDppDropTarget = NULL;
		}
	}

	BOOL ShowDropHelper(BOOL fShow)
	{
		if (m_pDppTargetHelper)
			m_pDppTargetHelper->Show(fShow);
		return fShow;
	}

	HRESULT DragStart(HWND hwndDragImage, CDataObject* pdo, LPDWORD peff)
	{
		HRESULT hr = E_INVALIDARG;
		if (pdo) {
			m_bDppDragMyself = true;
			m_pDppDropSource = new CDropSource(hwndDragImage, pdo);

			// フレームウィンドウへのD&Dを無効にする
			CWindow wnd = ::GetActiveWindow();
			CWindow top = wnd.GetTopLevelParent();

			top.ModifyStyleEx(WS_EX_ACCEPTFILES, 0);

			// SHDoDragDrop（XP sp2以降）を使って IDropSource にNULLを指定すると、
			// Windows側で勝手にインタフェースを作成してくれる → VISTAでDROPDESCRIPTIONが可能
/* @@
			HRESULT (WINAPI* _fSHDoDragDrop)(HWND, IDataObject*, IDropSource*, DWORD, DWORD*) = 0;
			CDllHandle dll = GetModuleHandle(gszShell32);
			(FARPROC&)_fSHDoDragDrop = dll.FindFunction("SHDoDragDrop");
			if (OsVISTA() && _fSHDoDragDrop)
				hr = _fSHDoDragDrop(hwndDragImage, pdo, NULL, *peff, peff);
			else
@@ */
				hr = DoDragDrop(pdo, m_pDppDropSource, *peff, peff);
			top.ModifyStyleEx(0, WS_EX_ACCEPTFILES);

			m_pDppDropSource->Release();
			m_pDppDropSource = NULL;
			m_bDppDragMyself = false;
		}
		return hr;
	}

	bool QueryDataObjectFormat(CLIPFORMAT cf)
	{
		FORMATETC fe = { cf, nullptr, DVASPECT_CONTENT, -1, TYMED_HGLOBAL };
		return (m_pDppCurrentDataObject->QueryGetData(&fe) == S_OK);
	}

	bool QueryDataObjectFormat(LPCTSTR pszClipFormatString)
	{
		UINT cf = RegisterClipboardFormat(pszClipFormatString);
		return QueryDataObjectFormat((CLIPFORMAT)cf);
	}
};

#endif			// __DROPPER_H__
