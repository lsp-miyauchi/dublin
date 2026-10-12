///////////////////////////////////////////////////////
// ctxwnd.cpp

#include "stdafx.h"
#include "lscore.h"
#include "idlist.h"

class CCtxWindow : public CWindowImpl<CCtxWindow>
{
public:
	LPCONTEXTMENU2 m_pcm2;
	LPCONTEXTMENU3 m_pcm3;

	CCtxWindow() : m_pcm2(NULL), m_pcm3(NULL)
	{
	}

	~CCtxWindow()
	{
	}

BEGIN_MSG_MAP(CCtxWindow)
	ON_WM_CREATE()
	MESSAGE_HANDLER(WM_INITMENUPOPUP, OnHandleMenuMsg)
	MESSAGE_HANDLER(WM_MEASUREITEM, OnHandleMenuMsg)
	MESSAGE_HANDLER(WM_DRAWITEM, OnHandleMenuMsg)
	MESSAGE_HANDLER(WM_MENUCHAR, OnHandleMenuMsg2)
	ON_WM_DESTROY()
END_MSG_MAP()

	int OnCreate(LPCREATESTRUCT pcs)
	{
		LPCONTEXTMENU pcm = (LPCONTEXTMENU)pcs->lpCreateParams;
		if (pcm) {
			if (pcm->QueryInterface(IID_IContextMenu2, (LPVOID*)&m_pcm2) == NOERROR) {
				if (m_pcm2->QueryInterface(IID_IContextMenu3, (LPVOID*)&m_pcm3) != NOERROR)
					m_pcm3 = NULL;
			}
			else
				m_pcm2 = NULL;
		}
		return 0;
	}

	LRESULT OnHandleMenuMsg(UINT msg, WPARAM wp, LPARAM lp, BOOL& bHandled)
	{
		if (m_pcm2 && m_pcm2->HandleMenuMsg(msg, wp, lp) == NOERROR)
			return 0;
		bHandled = FALSE;
		return 0;
	}

	LRESULT OnHandleMenuMsg2(UINT msg, WPARAM wp, LPARAM lp, BOOL& bHandled)
	{
		LRESULT lr;
		if (m_pcm3 && m_pcm3->HandleMenuMsg2(msg, wp, lp, &lr) == NOERROR)
			return lr;
		bHandled = FALSE;
		return 0;
	}

	void OnDestroy()
	{
		if (m_pcm3) {
			m_pcm3->Release();
			m_pcm3 = NULL;
		}
		if (m_pcm2) {
			m_pcm2->Release();
			m_pcm2 = NULL;
		}
	}
};

static BOOL track_ctxmenu(HWND hParent, POINT pt, LPCONTEXTMENU pcm, UINT uFlags)
{
	BOOL bOK = FALSE;
	HMENU hmenu = CreatePopupMenu();
	HRESULT hr = pcm->QueryContextMenu(hmenu, 0, 0x0001, 0x7fff, uFlags);
	if (SUCCEEDED(hr)) {
		CCtxWindow ctx;
		ctx.Create(hParent, 0, NULL, WS_CHILD, 0, 0U, (void*)pcm);
		if (ctx.IsWindow()) {
			int idCmd = TrackPopupMenu(hmenu,
							TPM_LEFTALIGN | TPM_RETURNCMD | TPM_RIGHTBUTTON,
							pt.x, pt.y, 0, ctx, NULL);
			ctx.DestroyWindow();
			if (idCmd > 0) {
/* @@@
				CMINVOKECOMMANDINFO cmi;
				memset(&cmi, 0, sizeof(cmi));
				cmi.cbSize = sizeof(cmi);
				cmi.hwnd   = hParent;
				cmi.lpVerb = (LPCSTR)(idCmd - 1);
				cmi.nShow  = SW_SHOWNORMAL;
				pcm->InvokeCommand(&cmi);
@@@ */
				CMINVOKECOMMANDINFOEX cmi;
				memset(&cmi, 0, sizeof(cmi));
				cmi.cbSize = sizeof(cmi);
				cmi.fMask = CMIC_MASK_UNICODE;
				if (GetKeyState(VK_SHIFT) < 0)
					cmi.fMask |= CMIC_MASK_SHIFT_DOWN;
				if (GetKeyState(VK_CONTROL) < 0)
					cmi.fMask |= CMIC_MASK_CONTROL_DOWN;

				cmi.hwnd   = hParent;
				cmi.lpVerb = (LPCSTR)(ULONG_PTR)(idCmd - 1);
				cmi.lpVerbW = (LPWSTR)(ULONG_PTR)(idCmd - 1);
				cmi.nShow  = SW_SHOWNORMAL;
				pcm->InvokeCommand((CMINVOKECOMMANDINFO*)&cmi);
				bOK = TRUE;
			}
		}
	}
	DestroyMenu(hmenu);
	return bOK;
}

////////////////////////////////////////////////////////////////////////
//

extern "C"
BOOL WINAPI lccTrackContextMenu(HWND hwnd, IContextMenu* pcm, POINT* ppt, UINT flag)
{
	BOOL bOK = FALSE;
	if (pcm)
		bOK = track_ctxmenu(hwnd, *ppt, pcm, flag);
	return bOK;
}

extern "C"
BOOL WINAPI lccContextMenuByIDList(HWND hwnd, LPITEMIDLIST pidl, LPPOINT ppt)
{
	BOOL bOK = FALSE;
	IContextMenu* pcm;
	pcm = (IContextMenu*)idl_GetUIObjectOf(hwnd, pidl, IID_IContextMenu);
	if (pcm) {
		bOK = lccTrackContextMenu(hwnd, pcm, ppt, CMF_EXPLORE);
		pcm->Release();
	}
	return bOK;
}

extern "C"
BOOL WINAPI lccContextMenuByFile(HWND hwnd, LPCTSTR pszFile, LPPOINT ppt)
{
	BOOL bOK = FALSE;
	CPidl idl;
	if (idl.CreateFromPath((LPTSTR)pszFile)) {
		bOK = lccContextMenuByIDList(hwnd, idl, ppt);
	}
	return bOK;
}

extern "C"
BOOL WINAPI lccContextMenuByShellItem(HWND hwnd, IShellItem* psi, LPPOINT ppt)
{
	BOOL bOK = FALSE;
	IContextMenu* pcm = NULL;
	if (psi) {
		if (psi->BindToHandler(NULL, BHID_SFUIObject, IID_IContextMenu, (void**)&pcm) == S_OK) {
			bOK = lccTrackContextMenu(hwnd, pcm, ppt, CMF_EXPLORE);
			pcm->Release();
		}
	}
	return bOK;
}
