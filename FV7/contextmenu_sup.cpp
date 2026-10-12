///////////////////////////////////////////////////////
// contextmenu_sup.cpp

#include "stdafx.h"
#include "resource.h"
#include "idlist.h"

class CCtxHandleWindow : public CWindowImpl<CCtxHandleWindow>
{
public:
	CComQIPtr<IContextMenu2> m_pcm2;
	CComQIPtr<IContextMenu3> m_pcm3;

	CCtxHandleWindow(IContextMenu* pcm)
	{
		if (pcm) {
			m_pcm2 = pcm;
			if (m_pcm2.p != NULL) {
				m_pcm3 = m_pcm2;
			}
		}
	}

	~CCtxHandleWindow()
	{}

BEGIN_MSG_MAP(CCtxHandleWindow)
	MESSAGE_HANDLER(WM_INITMENUPOPUP, OnHandleMenuMsg)
	MESSAGE_HANDLER(WM_MEASUREITEM, OnHandleMenuMsg)
	MESSAGE_HANDLER(WM_DRAWITEM, OnHandleMenuMsg)
	MESSAGE_HANDLER(WM_MENUCHAR, OnHandleMenuMsg2)
END_MSG_MAP()

	LRESULT OnHandleMenuMsg(UINT msg, WPARAM wp, LPARAM lp, BOOL& bHandled)
	{
		if (m_pcm2.p != NULL && m_pcm2->HandleMenuMsg(msg, wp, lp) == NOERROR)
			return 0;
		bHandled = FALSE;
		return 0;
	}

	LRESULT OnHandleMenuMsg2(UINT msg, WPARAM wp, LPARAM lp, BOOL& bHandled)
	{
		LRESULT lr;
		if (m_pcm3.p != NULL && m_pcm3->HandleMenuMsg2(msg, wp, lp, &lr) == NOERROR)
			return lr;
		bHandled = FALSE;
		return 0;
	}
};

typedef ULONG CCMIF;
#define CCMI_FOLDER			0x00000001
#define CCMI_UNCPATH		0x00000002

// カスタムメニュー
typedef struct tagCUSTOMCTXMENU {
	UINT res;
	UINT cmd;
	CCMIF mask;
} CUSTOMCTXMENU;

static CUSTOMCTXMENU stCustomCtxMenus[] = {
	{ IDS_OPEN_BY_NEWTAB,	IDM_TAB_NEW,			CCMI_FOLDER },
	{ IDS_ADD_FAVORITE,		IDM_DIR_ADDFAVORITE,	CCMI_FOLDER },
//	{ IDS_ADD_DRIVEBAR,		IDM_NET_ADDDRIVEBAR,	CCMI_FOLDER | CCMI_UNCPATH },
};

static int track_contextmenu(HWND hParent, POINT pt, IContextMenu* pcm, UINT shellFlags, CCMIF attr)
{
	static const int iStartCtx = 0x0010;
	int cmdBack = -1;
	if (pcm) {
		pcm->AddRef();
		HMENU hmenu = ::CreatePopupMenu();
		if (hmenu) {
			CString str;
			int iInsertAfter = 0;
			for (int i = 0; i < ARRAYSIZE(stCustomCtxMenus); i++) {
				if ((attr & stCustomCtxMenus[i].mask) == stCustomCtxMenus[i].mask) {
					str.LoadString(stCustomCtxMenus[i].res);
					::AppendMenu(hmenu, MF_STRING, i + 1, str);
					iInsertAfter++;
				}
			}
			if (iInsertAfter > 0) {
				::AppendMenu(hmenu, MF_SEPARATOR, 0, NULL);
				iInsertAfter++;
			}
			HRESULT hr = pcm->QueryContextMenu(hmenu, iInsertAfter, iStartCtx, 0x7fff, shellFlags);
			if (SUCCEEDED(hr)) {
				CCtxHandleWindow ctx(pcm);
				ctx.Create(hParent, 0, NULL, WS_CHILD);
				if (ctx.IsWindow()) {
					int idCmd = TrackPopupMenu(hmenu,
									TPM_LEFTALIGN | TPM_RETURNCMD | TPM_RIGHTBUTTON,
									pt.x, pt.y, 0, ctx, NULL);
					ctx.DestroyWindow();
					if (idCmd > 0 && idCmd <= ARRAYSIZE(stCustomCtxMenus)) {
						cmdBack = stCustomCtxMenus[idCmd - 1].cmd;
					} else if (idCmd >= iStartCtx) {
#ifdef USE_ANSI_CONTEXTMENU
						CMINVOKECOMMANDINFO cmi;
						memset(&cmi, 0, sizeof(cmi));
						cmi.cbSize = sizeof(cmi);
						cmi.hwnd   = hParent;
						cmi.lpVerb = (LPCSTR)(idCmd - iStartCtx);
						cmi.nShow  = SW_SHOWNORMAL;
						pcm->InvokeCommand(&cmi);
#else
						CMINVOKECOMMANDINFOEX cmi;
						memset(&cmi, 0, sizeof(cmi));
						cmi.cbSize = sizeof(cmi);
						cmi.fMask = CMIC_MASK_UNICODE;
						if (GetKeyState(VK_SHIFT) < 0)
							cmi.fMask |= CMIC_MASK_SHIFT_DOWN;
						if (GetKeyState(VK_CONTROL) < 0)
							cmi.fMask |= CMIC_MASK_CONTROL_DOWN;

						cmi.hwnd   = hParent;
						cmi.lpVerb = (LPCSTR)(ULONG_PTR)(idCmd - iStartCtx);
						cmi.lpVerbW = (LPWSTR)(ULONG_PTR)(idCmd - iStartCtx);
						cmi.nShow  = SW_SHOWNORMAL;
						pcm->InvokeCommand((CMINVOKECOMMANDINFO*)&cmi);
#endif	// USE_ANSI_CONTEXTMENU
						cmdBack = 0;
					}
				}
			}
			DestroyMenu(hmenu);
		}
		pcm->Release();
	}
	return cmdBack;
}

////////////////////////////////////////////////////////////////////////
//

int TrackContextMenu(HWND hwnd, IContextMenu* pcm, POINT* ppt, SFGAOF attr)
{
	UINT shf = CMF_EXPLORE;
	if (_ISKEYDOWN(VK_SHIFT))
		shf |= CMF_EXTENDEDVERBS;
	CCMIF f = (attr & SFGAO_FOLDER) ? CCMI_FOLDER : 0;
	return track_contextmenu(hwnd, *ppt, pcm, shf, f);
}

int TrackContextMenuByIDList(HWND hwnd, LPCITEMIDLIST pidl, LPPOINT ppt)
{
	CCMIF f = (idl_GetAttributes(pidl, SFGAO_FOLDER) != 0) ? CCMI_FOLDER : 0;
	TCHAR buf[MAX_PATH * 2];
	if (SHGetPathFromIDList(pidl, buf) && PathIsUNC(buf))
		f |= CCMI_UNCPATH;
	else {
		int descid = idl_GetDescriptionId(pidl);
		switch (descid) {
		case SHDID_NET_SHARE:
		case SHDID_NET_SERVER:
		case SHDID_NET_DOMAIN:
		case SHDID_NET_RESTOFNET:
			f |= CCMI_UNCPATH;
			break;
		}
	}
	int cmdBack = -1;
	CComPtr<IContextMenu> pcm = (IContextMenu*)idl_GetUIObjectOf(hwnd, pidl, IID_IContextMenu);
	if (pcm.p != NULL) {
		UINT shf = CMF_EXPLORE;
		if (_ISKEYDOWN(VK_SHIFT))
			shf |= CMF_EXTENDEDVERBS;
		cmdBack = track_contextmenu(hwnd, *ppt, pcm, shf, f);
	}
	return cmdBack;
}

int TrackContextMenuByFile(HWND hwnd, LPCTSTR pszFile, LPPOINT ppt)
{
	int cmdBack = -1;
	CIDListPtr idl;
	if (idl.CreateFromPath(pszFile))
		cmdBack = TrackContextMenuByIDList(hwnd, idl, ppt);
	return cmdBack;
}
