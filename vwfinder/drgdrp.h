////////////////////////////////////////////////////////////////
// drgdrp.h

#pragma once
#ifndef __DRGDRP_H__
#define __DRGDRP_H__

#include "dataobject.h"

////////////////////////////////////////////////////////////////
//

HRESULT __fastcall DragStart(HDROP hdrop, LPDWORD pdwEffect);
HRESULT __fastcall DragStart(IDataObject* pdo, LPDWORD pdwEffect);

////////////////////////////////////////////////////////////////
// CDropSource

class CDropSource : public IDropSource
{
public:
	CDropSource();

	// IUnknown methods
	STDMETHODIMP QueryInterface(REFIID riid, void FAR* FAR* ppvObj);
	STDMETHODIMP_(ULONG) AddRef(void);
	STDMETHODIMP_(ULONG) Release(void);

	// IDropSource methods
	STDMETHODIMP QueryContinueDrag(BOOL fEsc, DWORD keyState);
	STDMETHODIMP GiveFeedback(DWORD dwEffect);

private:
	ULONG m_refs;
};


#endif			// __DRGDRP_H__
