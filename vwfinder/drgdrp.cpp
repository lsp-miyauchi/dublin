////////////////////////////////////////////////////////////////
// drgdrp.cpp

#include "stdafx.h"
#include "drgdrp.h"
#include "resource.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
//@ #define new DEBUG_NEW
#endif

////////////////////////////////////////////////////////
//

HRESULT __fastcall DragStart(HDROP hdrop, LPDWORD pdwEffect)
{
	HRESULT res = E_OUTOFMEMORY;
	int cnt = DragQueryFile(hdrop, 0xFFFFFFFF, NULL, 0);
	if (cnt > 0) {
		CDataObject* pmd = new CDataObject(hdrop);
		res = DragStart(pmd, pdwEffect);
		pmd->Release();
	}
	return res;
}

HRESULT __fastcall DragStart(IDataObject* pdo, LPDWORD pdwEffect)
{
	CDropSource* pds = new CDropSource();
	HRESULT res = DoDragDrop(pdo, pds, *pdwEffect, pdwEffect);
	pds->Release();
	return res;
}

////////////////////////////////////////////////////////////////
// CDropSource Implementation
//

CDropSource::CDropSource()
{
	m_refs = 1;
}

////////////////////////////////////////////////////////////////
// CDropSource::IUnknown Methods
//

STDMETHODIMP
CDropSource::QueryInterface(REFIID iid, void** ppv)
{
	if (iid == IID_IUnknown || iid == IID_IDropSource) {
		*ppv = this;
		AddRef();
		return NOERROR;
	}
	*ppv = NULL;
	return ResultFromScode(E_NOINTERFACE);
}

STDMETHODIMP_(ULONG)
CDropSource::AddRef(void)
{
	return ++m_refs;
}

STDMETHODIMP_(ULONG)
CDropSource::Release(void)
{
	if (--m_refs == 0) {
		delete this;
		return 0;
	}
	return m_refs;
}

////////////////////////////////////////////////////////////////
// CDropSource::IDropSource Methods
//

STDMETHODIMP
CDropSource::QueryContinueDrag(BOOL fEsc, DWORD keyState)
{
	if (fEsc) return ResultFromScode(DRAGDROP_S_CANCEL);

	if (!(keyState & (MK_LBUTTON | MK_RBUTTON)))
		return ResultFromScode(DRAGDROP_S_DROP);

	return NOERROR;
}

STDMETHODIMP
CDropSource::GiveFeedback(DWORD eff)
{
	HINSTANCE hInst = ATL::_AtlBaseModule.GetResourceInstance();
	if (eff & DROPEFFECT_COPY)
		SetCursor(LoadCursor(hInst, MAKEINTRESOURCE(IC_DDCOPY)));
	else if (eff & DROPEFFECT_MOVE)
		SetCursor(LoadCursor(hInst, MAKEINTRESOURCE(IC_DDMOVE)));
	else if (eff & DROPEFFECT_LINK)
		SetCursor(LoadCursor(hInst, MAKEINTRESOURCE(IC_DDLINK)));
	else
		SetCursor(LoadCursor(hInst, MAKEINTRESOURCE(IC_DDNONE)));
	return ResultFromScode(S_OK);
}
