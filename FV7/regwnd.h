////////////////////////////////////////////////////////////////////////
// regwnd.h : ヘッダー ファイル
//

#pragma once
#ifndef __REGWND_H__
#define __REGWND_H__

#include "reguserinfo.h"


HRESULT NotifyUser(UINT idIcon, UINT idTitle, UINT idCaption, UINT idText);


/////////////////////////////////////////////////////////////////////////////
// CRegWnd ウィンドウ

class CRegWnd : public CWindowImpl<CRegWnd>
{
private:
	CSimpleArray<HANDLE> m_threadarray;

public:
	CRegWnd();
	~CRegWnd();

	USRINFO* m_pui;
	BOOL (*m_pfnRegist)(const char*);
	BOOL m_bRegistResult;

BEGIN_MSG_MAP(CRegWnd)
	ON_WM_CREATE()
	ON_WM_DESTROY()
	ON_REGISTERD_MESSAGE(nRM_REGCHECK1ST, OnRegCheck1st)
	ON_REGISTERD_MESSAGE(nRM_REGDIALOG, OnRegDialog)
	ON_REGISTERD_MESSAGE(nRM_REGADDTHREAD, OnRegAddThread)
END_MSG_MAP()

	int OnCreate(LPCREATESTRUCT lpCreateStruct);
	void OnDestroy() { m_bRegistResult = (*m_pfnRegist)(m_pui->code); }
	LRESULT OnRegCheck1st(WPARAM wParam, LPARAM lParam);
	LRESULT OnRegDialog(WPARAM wParam, LPARAM lParam);
	LRESULT OnRegAddThread(WPARAM wParam, LPARAM lParam);

	virtual void OnFinalMessage(HWND /*hWnd*/)
	{
		int cnt = (int)m_threadarray.GetSize();
		if (cnt > 0) {
			for (int i = 0; i < cnt; i++) {
				HANDLE thread = m_threadarray[i];
				if (thread) {
					CloseHandle(thread);
				}
			}
		}
		delete this;
	}
};


#endif			// __REGWND_H__
