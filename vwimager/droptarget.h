////////////////////////////////////////////////////////////////////////
// droptarget.h

#pragma once
#ifndef _VWI_DROPTARGET_H__
#define _VWI_DROPTARGET_H__

////////////////////////////////////////////////////////////////////////
//

class CDropTargetHandler
{
public:
	virtual DWORD OnTargetDragEnter(IDataObject* pDataObject, DWORD keyState, DWORD dropEffect, POINT point)
	{
		return DROPEFFECT_NONE;
	}
	virtual DWORD OnTargetDragOver(IDataObject* pDataObject, DWORD keyState, DWORD dropEffect, POINT point)
	{
		return DROPEFFECT_NONE;
	}
	virtual void OnTargetDragLeave()
	{
	}
	virtual DWORD OnTargetDrop(IDataObject* pDataObject, DWORD keyState, DROPEFFECT dropEffect, POINT point)
	{
		return DROPEFFECT_NONE;
	}
};


////////////////////////////////////////////////////////////////
// CDropTarget

class CDropTarget : public IDropTarget
{
private:
	IDataObject* m_pDataObject;
	ULONG m_refs;
	CDropTargetHandler* m_pTargetHandler;
	DWORD m_keyState;
	DWORD m_dropEffect;

public:
	CDropTarget(CDropTargetHandler* pHandler)
		: m_refs(1)
		, m_pTargetHandler(pHandler)
		, m_keyState(0)
		, m_dropEffect(0)
	{
	}

	// -------------------------------
	// IUnknown methods
	// -------------------------------
	STDMETHODIMP QueryInterface(REFIID riid, void FAR* FAR* ppvObj)
	{
		if (riid == IID_IUnknown || riid == IID_IDropTarget) {
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

	// -------------------------------
	// IDropTarget methods
	// -------------------------------
	STDMETHODIMP DragEnter(LPDATAOBJECT pDataObj, DWORD keyState, POINTL pt, LPDWORD pdwEffect)
	{
		m_pDataObject = pDataObj;
		m_keyState = keyState;

		POINT point;
		point.x = pt.x;
		point.y = pt.y;
		m_dropEffect = m_pTargetHandler->OnTargetDragEnter(m_pDataObject, keyState, *pdwEffect, point);
		*pdwEffect = m_dropEffect;
		return S_OK;
	}

	STDMETHODIMP DragOver(DWORD keyState, POINTL pt, LPDWORD pdwEffect)
	{
		m_keyState = keyState;

		POINT point;
		point.x = pt.x;
		point.y = pt.y;
		m_dropEffect = m_pTargetHandler->OnTargetDragOver(m_pDataObject, keyState, *pdwEffect, point);
		*pdwEffect = m_dropEffect;
		return S_OK;
	}

	STDMETHODIMP DragLeave()
	{
		m_pTargetHandler->OnTargetDragLeave();
		return S_OK;
	}

	STDMETHODIMP Drop(LPDATAOBJECT pdo, DWORD keyState, POINTL pt, LPDWORD pdwEffect)
	{
		if (*pdwEffect != DROPEFFECT_NONE) {
			// 左ボタンが押されていれば、DragOver時に取得した効果を設定
			if (m_keyState & MK_LBUTTON)
				*pdwEffect = m_dropEffect;

			POINT point;
			point.x = pt.x;
			point.y = pt.y;
			*pdwEffect = m_pTargetHandler->OnTargetDrop(pdo, m_keyState, *pdwEffect, point);
		}
		else
			*pdwEffect = DROPEFFECT_NONE;

		DragLeave();
		return S_OK;
	}
};


#endif		// _VWI_DROPTARGET_H__
