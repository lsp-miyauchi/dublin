//////////////////////////////////////////////////////////////////////
// taskbarlist.h

#pragma once
#ifndef __TASKBARLIST_H__
#define __TASKBARLIST_H__

///////////////////////////////////////////////////////////////////////
//

template <class T>
class CTaskbarListImpl
{
private:
	ITaskbarList3* m_pTaskbarList;
	UINT m_nRM_INITTASKBUTTON;

public:
	CTaskbarListImpl() : m_pTaskbarList(NULL)
	{
		m_nRM_INITTASKBUTTON = RegisterWindowMessage(_T("TaskbarButtonCreated"));
	}

BEGIN_MSG_MAP(CTaskbarListImpl)
	MESSAGE_HANDLER(WM_NCCREATE, OnHandlerTBLNcCreate)
	MESSAGE_HANDLER(WM_DESTROY, OnHandlerTBLDestory)
	ON_REGISTERD_MESSAGE(m_nRM_INITTASKBUTTON, OnHandlerTBLTaskbarButtonCreated)
END_MSG_MAP()

	LRESULT OnHandlerTBLNcCreate(UINT msg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
	{
		HMODULE hmod = GetModuleHandle(_T("user32.dll"));
		if (hmod) {
			BOOL (WINAPI* _fChangeWindowMessageFilter)(UINT, DWORD);
			(FARPROC&)_fChangeWindowMessageFilter = ::GetProcAddress(hmod, "ChangeWindowMessageFilter");
			if (_fChangeWindowMessageFilter)
				_fChangeWindowMessageFilter(m_nRM_INITTASKBUTTON, MSGFLT_ADD);
		}
		bHandled = FALSE;
		return 0;
	}

	LRESULT OnHandlerTBLDestory(UINT msg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
	{
		if (m_pTaskbarList) {
			m_pTaskbarList->Release();
			m_pTaskbarList = NULL;
		}
		bHandled = FALSE;
		return 0;
	}

	LRESULT OnHandlerTBLTaskbarButtonCreated(WPARAM, LPARAM)
	{
		if (m_pTaskbarList == NULL) {
			HRESULT hr = CoCreateInstance(CLSID_TaskbarList, NULL, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&m_pTaskbarList));
			if (SUCCEEDED(hr)) {
				hr = m_pTaskbarList->HrInit();
				if (FAILED(hr)) {
					m_pTaskbarList->Release();
					m_pTaskbarList = NULL;
				}
				else {
					T* pT = static_cast<T*>(this);
					pT->TaskbarButtonCreated(m_pTaskbarList);
				}
			}
		}
		return 0;
	}

	virtual void TaskbarButtonCreated(ITaskbarList3*) { }

	virtual HRESULT TaskbarList_SetProgressState(TBPFLAG tbpFlags)
	{
		HRESULT hr = E_FAIL;
		T* pT= (T*)this;
		if (m_pTaskbarList != NULL && pT->IsWindow()) {
			hr = m_pTaskbarList->SetProgressState(pT->m_hWnd, tbpFlags);
		}
		return hr;
	}

	virtual HRESULT TaskbarList_SetProgressValue(ULONGLONG ullCompleted, ULONGLONG ullTotal)
	{
		HRESULT hr = E_FAIL;
		T* pT= (T*)this;
		if (m_pTaskbarList != NULL && pT->IsWindow()) {
			hr = m_pTaskbarList->SetProgressValue(pT->m_hWnd, ullCompleted, ullTotal);
		}
		return hr;
	}
};


#endif		// __TASKBARLIST_H__
