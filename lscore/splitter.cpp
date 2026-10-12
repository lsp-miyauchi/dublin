///////////////////////////////////////////////////////
// SPLITTER control implementation

#include "stdafx.h"
#include <atlsplit.h>
#include "lscore.h"

template <class T, bool t_bVertical = true, class TBase = CWindow, class TWinTraits = CControlWinTraits>
class ATL_NO_VTABLE CSplitterCtrlImpl : public CWindowImpl< T, TBase, TWinTraits >, public CSplitterImpl< T >
{
public:
	typedef CSplitterCtrlImpl< T , t_bVertical, TBase, TWinTraits >	thisClass;
	typedef CSplitterImpl< T >				baseClass;
	BEGIN_MSG_MAP(thisClass)
		ON_MESSAGE(WM_ERASEBKGND, OnEraseBackground)
		MESSAGE_HANDLER(WM_SIZE, OnSize)
		ON_MESSAGE_VOID(SPM_SETSPLITTERRECT, OnHandlerSetSplitterRect)
		ON_MESSAGE_VOID(SPM_GETSPLITTERRECT, OnHandlerGetSplitterRect)
		ON_MESSAGE(SPM_SETSPLITTERPOS, OnHandlerSetSplitterPos)
		ON_MESSAGE(SPM_GETSPLITTERPOS, OnHandlerGetSplitterPos)
		ON_MESSAGE(SPM_SETSINGLEPANEMODE, OnHandlerSetSinglePaneMode)
		ON_MESSAGE(SPM_GETSINGLEPANEMODE, OnHandlerGetSinglePaneMode)
		ON_MESSAGE(SPM_GETEXTENDEDSTYLE, OnHandlerGetSplitterExtendedStyle)
		ON_MESSAGE(SPM_SETEXTENDEDSTYLE, OnHandlerSetSplitterExtendedStyle)
		ON_MESSAGE_VOID(SPM_SETPANES,OnHandlerSetSplitterPanes)
		ON_MESSAGE(SPM_SETSPLITTERPANE, OnHandlerSetSplitterPane)
		ON_MESSAGE(SPM_GETSPLITTERPANE, OnHandlerGetSplitterPane)
		ON_MESSAGE(SPM_SETACTIVEPANE, OnHandlerSetActivePane)
		ON_MESSAGE(SPM_GETACTIVEPANE, OnHandlerGetActivePane)
		ON_MESSAGE(SPM_ACTIVATENEXTPANE, OnHandlerActivateNextPane)
		ON_MESSAGE(SPM_SETDEFAULTACTIVEPANE, OnHandlerSetDefaultActivePane)
		ON_MESSAGE(SPM_GETDEFAULTACTIVEPANE, OnHandlerGetDefaultActivePane)
		ON_MESSAGE_VOID(SPM_SETSPLITTERMINSIZE, OnHandlerSetSplitterMinSize)
		ON_MESSAGE(SPM_GETSPLITTERMINSIZE, OnHandlerGetSplitterMinSize)
		ON_MESSAGE_VOID(SPM_SETSPLITTERDRAGMODE, OnHandlerSetSplitterDragMode)
		ON_MESSAGE(SPM_GETSPLITTERDRAGMODE, OnHandlerGetSplitterDragMode)


		CHAIN_MSG_MAP(baseClass)
		FORWARD_NOTIFICATIONS()
	END_MSG_MAP()

	LRESULT OnEraseBackground(WPARAM wParam, LPARAM lParam)
	{
		return 1;
	}

	LRESULT OnSize(UINT /*uMsg*/, WPARAM wParam, LPARAM /*lParam*/, BOOL& bHandled)
	{
		if(wParam != SIZE_MINIMIZED)
			SetSplitterRect();
		bHandled = FALSE;
		return 1;
	}

	void OnHandlerSetSplitterRect(WPARAM wParam, LPARAM lParam)
	{
		SetSplitterRect((LPRECT)lParam, (wParam != 0));
	}

	void OnHandlerGetSplitterRect(WPARAM wParam, LPARAM lParam)
	{
		GetSplitterRect((LPRECT)lParam);
	}

	LRESULT OnHandlerSetSplitterPos(WPARAM wParam, LPARAM lParam)
	{
		return (LRESULT)SetSplitterPos((int)lParam, (wParam != 0));
	}

	LRESULT OnHandlerGetSplitterPos(WPARAM wParam, LPARAM lParam)
	{
		return (LRESULT)GetSplitterPos();
	}

	LRESULT OnHandlerSetSinglePaneMode(WPARAM wParam, LPARAM lParam)
	{
		return (LRESULT)SetSinglePaneMode((int)wParam);
	}

	LRESULT OnHandlerGetSinglePaneMode(WPARAM wParam, LPARAM lParam)
	{
		return (LRESULT)GetSinglePaneMode();
	}

	LRESULT OnHandlerGetSplitterExtendedStyle(WPARAM wParam, LPARAM lParam)
	{
		return (LRESULT)GetSplitterExtendedStyle();
	}

	LRESULT OnHandlerSetSplitterExtendedStyle(WPARAM wParam, LPARAM lParam)
	{
		return (LRESULT)SetSplitterExtendedStyle((DWORD)lParam, (DWORD)wParam);
	}

	void OnHandlerSetSplitterPanes(WPARAM wParam, LPARAM lParam)
	{
		SetSplitterPanes(((SPLITTERPANEINFO*)lParam)->hWndLeftTop, ((SPLITTERPANEINFO*)lParam)->hWndRightBottom, (wParam != 0));
	}

	LRESULT OnHandlerSetSplitterPane(WPARAM wParam, LPARAM lParam)
	{
		return (LRESULT)SetSplitterPane((int)LOWORD(wParam), (HWND)lParam, (HIWORD(wParam) != 0));
	}

	LRESULT OnHandlerGetSplitterPane(WPARAM wParam, LPARAM lParam)
	{
		return (LRESULT)GetSplitterPane((int)wParam);
	}

	LRESULT OnHandlerSetActivePane(WPARAM wParam, LPARAM lParam)
	{
		return (LRESULT)SetActivePane((int)wParam);
	}

	LRESULT OnHandlerGetActivePane(WPARAM wParam, LPARAM lParam)
	{
		return (LRESULT)GetActivePane();
	}

	LRESULT OnHandlerActivateNextPane(WPARAM wParam, LPARAM lParam)
	{
		return (LRESULT)ActivateNextPane(wParam != 0);
	}

	LRESULT OnHandlerSetDefaultActivePane(WPARAM wParam, LPARAM lParam)
	{
		bool bOK;
		if (wParam)	bOK = SetDefaultActivePane((int)lParam);
		else		bOK = SetDefaultActivePane((HWND)lParam);
		return (LRESULT)bOK;
	}

	LRESULT OnHandlerGetDefaultActivePane(WPARAM wParam, LPARAM lParam)
	{
		return (LRESULT)GetDefaultActivePane();
	}

	void OnHandlerSetSplitterMinSize(WPARAM wParam, LPARAM lParam)
	{
		m_cxyMin = (int)wParam;
	}

	LRESULT OnHandlerGetSplitterMinSize(WPARAM wParam, LPARAM lParam)
	{
		return (LRESULT)m_cxyMin;
	}

	void OnHandlerSetSplitterDragMode(WPARAM wParam, LPARAM lParam)
	{
		m_bFullDrag = (wParam != 0);
	}

	LRESULT OnHandlerGetSplitterDragMode(WPARAM wParam, LPARAM lParam)
	{
		return (LRESULT)m_bFullDrag;
	}
};

template <bool t_bVertical = true>
class CSplitterCtrlT : public CSplitterCtrlImpl<CSplitterCtrlT<t_bVertical>, t_bVertical>
{
public:
	virtual void OnFinalMessage(HWND /*hWnd*/)
	{
		delete this;
	}
};

typedef CSplitterCtrlT<true>	CSplitterCtrl;
typedef CSplitterCtrlT<false>	CHorSplitterCtrl;


////////////////////////////////////////////////////////////////////////

extern "C"
LRESULT CALLBACK SplitterWndFn(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	LRESULT lr = DefWindowProc(hwnd, msg, wParam, lParam);
	if (msg == WM_NCCREATE) {
		if (((LPCREATESTRUCT)lParam)->style & SPS_SPLITTERHORIZONTAL) {
			CHorSplitterCtrl* pHorWnd = new CHorSplitterCtrl;
			pHorWnd->SubclassWindow(hwnd);
		}
		else {
			CSplitterCtrl* pWnd = new CSplitterCtrl;
			pWnd->SubclassWindow(hwnd);
		}
	}
	return lr;
}

