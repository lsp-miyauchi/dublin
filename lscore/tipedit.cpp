//////////////////////////////////////////////////////////////////////
// tipedit.cpp

#include "stdafx.h"
#include <limits.h>
#include "resource.h"
#include "lscore.h"
#include "fnc.h"
#include "common.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


#define IDW_INNER_EDIT				100
#define EN_ENDEDIT					0x1000
#define EN_CANCELEDIT				0x1001


class CTipEditInfo
{
public:
	CString m_csText;
	WNDPROC m_pfnEdit;
	BOOL m_bCompleted;
	CWindow m_edit;
	CRect m_rcInit;

	CTipEditInfo()
	{
		m_csText = _T("");
		m_pfnEdit = NULL;
		m_bCompleted = FALSE;
	}

	~CTipEditInfo()
	{
	}
};

////////////////////////////////////////////////////////////////////////////////
//

extern "C"
BOOL WINAPI lccTipEdit(HWND hParent, LPRECT prc, LPTSTR pszText)
{
	CTipEditInfo tei;
	tei.m_csText = pszText;

	tei.m_rcInit = *prc;
	HWND hwnd = CreateWindowEx(WS_EX_TOPMOST, WC_LSC_TIPEDIT, NULL, WS_VISIBLE | WS_POPUP | WS_BORDER,
							prc->left, prc->top, prc->right - prc->left, prc->bottom - prc->top,
							hParent, 0, MODINST, &tei);
	if (hwnd) {
		while (IsWindow(hwnd)) {
			MSG msg;
			if (!PeekMessage(&msg, NULL, NULL, NULL, PM_NOREMOVE)) {
				if (hParent && !(GetWindowStyle(hParent) & DS_NOIDLEMSG))
					SendMessage(hParent, WM_ENTERIDLE, MSGF_DIALOGBOX, (LPARAM)hwnd);
			}

			while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) {
				TranslateMessage(&msg);
				DispatchMessage(&msg);
			}
		}
	}

	if (tei.m_bCompleted) {
		lstrcpy(pszText, tei.m_csText);
		return TRUE;
	}
	return FALSE;
}

////////////////////////////////////////////////////////////////////////////////
// Edit Hook procedure

static LRESULT CALLBACK HookEditProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	HWND hParent = GetParent(hwnd);
	int id = GetDlgCtrlID(hwnd);

	switch (msg) {
	case WM_KEYDOWN:
		switch (wParam) {
		case VK_RETURN:
			PostMessage(hParent, WM_COMMAND, MAKELONG(id, EN_ENDEDIT), (LPARAM)hwnd);
			return 0;
		case VK_ESCAPE:
			PostMessage(hParent, WM_COMMAND, MAKELONG(id, EN_CANCELEDIT), (LPARAM)hwnd);
			return 0;
		}
		break;

	case WM_KILLFOCUS:
		PostMessage(hParent, WM_COMMAND, MAKELONG(id, EN_CANCELEDIT), (LPARAM)hwnd);
		break;
	}

	CTipEditInfo* ptei = (CTipEditInfo*)GetWindowLongPtr(hParent, GWLP_USERDATA);
	return CallWindowProc(ptei->m_pfnEdit, hwnd, msg, wParam, lParam);
}

////////////////////////////////////////////////////////////////////////////////
//

extern "C"
LRESULT CALLBACK TipEditWndFn(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	CTipEditInfo* ptei = (CTipEditInfo*)GetWindowLongPtr(hwnd, GWLP_USERDATA);
	TCHAR buf[MAX_PATH * 2];
	int i;
	switch (msg) {
	case WM_CREATE:
		ptei = (CTipEditInfo*)((CREATESTRUCT*)lParam)->lpCreateParams;
		SetWindowLongPtr(hwnd, GWLP_USERDATA, (LONG_PTR)ptei);

		ptei->m_edit = CreateWindow(_T("Edit"), ptei->m_csText,
								WS_CHILD | WS_VISIBLE |  ES_AUTOHSCROLL | ES_LEFT,
								0, 0, 10, 10, hwnd, (HMENU)IDW_INNER_EDIT, MODINST, NULL);
		ptei->m_edit.SetFont((HFONT)GetStockObject(DEFAULT_GUI_FONT));
		ptei->m_pfnEdit = (WNDPROC)::GetWindowLongPtr(ptei->m_edit, GWLP_WNDPROC);
		SetWindowLongPtr(ptei->m_edit, GWLP_WNDPROC, (LONG_PTR)HookEditProc);
		return 0;

	case WM_COMMAND:
		switch (LOWORD(wParam)) {
		case IDW_INNER_EDIT:
			switch (HIWORD(wParam)) {
			case EN_ENDEDIT:
				ptei->m_bCompleted = FALSE;
				if (ptei->m_edit.GetWindowText(buf, ARRAYSIZE(buf)) > 0) {
					if (lstrcmp(buf, ptei->m_csText)) {
						ptei->m_bCompleted = TRUE;
						ptei->m_csText = buf;
					}
				}
				DestroyWindow(hwnd);
				return 0;

			case EN_UPDATE:
				i = ptei->m_edit.GetWindowText(buf, ARRAYSIZE(buf));
				if (i > 0) {
					CClientDC dc(ptei->m_edit);
					HFONT hfont = dc.SelectFont((HFONT)GetStockObject(DEFAULT_GUI_FONT));
					CSize size;
					dc.GetTextExtent(buf, i, &size);
					dc.SelectFont(hfont);

					CRect rc = ptei->m_rcInit;
					rc.right = rc.left + size.cx + GetSystemMetrics(SM_CXBORDER) * 2 + 2;
					if (rc.Width() > ptei->m_rcInit.Width()) {
						SetWindowPos(hwnd, NULL, 0, 0, rc.Width(), rc.Height(), SWP_NOMOVE | SWP_NOZORDER);
					}
				}
				return 0;

			case EN_CANCELEDIT:
				ptei->m_bCompleted = FALSE;
				DestroyWindow(hwnd);
				return 0;
			}
			break;
		}
		break;

	case WM_ERASEBKGND:
		return TRUE;

	case WM_SIZE:
		ptei->m_edit.MoveWindow(0, 0, LOWORD(lParam), HIWORD(lParam), TRUE);
		return 0;

	case WM_SETFOCUS:
		ptei->m_edit.SetFocus();
		return 0;

	case WM_DESTROY:
		if (ptei->m_edit.IsWindow()) {
			SetWindowLongPtr(ptei->m_edit, GWLP_WNDPROC, (LONG_PTR)ptei->m_pfnEdit);
			ptei->m_edit.DestroyWindow();
		}
		break;
	}
	return DefWindowProc(hwnd, msg, wParam, lParam);
}

