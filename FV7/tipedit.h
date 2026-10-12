/////////////////////////////////////////////////////////////////////////////
// tipedit.h
//

#pragma once
#ifndef __TIPEDIT_H__
#define __TIPEDIT_H__

#define TED_ENDEDIT					512
#define TED_CANCELEDIT				513
#define IDW_INNER_EDIT				100
#define EN_ENDEDIT					0x1000
#define EN_CANCELEDIT				0x1001


class CTipEdit : public CWindowImpl<CTipEdit>
{
public:
	DECLARE_WND_CLASS_EX(_T("TipEdit"), CS_GLOBALCLASS, COLOR_WINDOW)

	CTipEdit() { m_strText.Empty(); }

BEGIN_MSG_MAP(CTipEdit)
	ON_WM_CREATE()
	ON_WM_SIZE()
	ON_WM_SETFOCUS()
	ON_WM_DESTROY()
	ON_CONTROL(EN_ENDEDIT, IDW_INNER_EDIT, OnEndEdit)
	ON_CONTROL(EN_CANCELEDIT, IDW_INNER_EDIT, OnCancelEdit)
END_MSG_MAP()

	int OnCreate(LPCREATESTRUCT pcs)
	{
		CRect rc;
		GetClientRect(&rc);
		m_edit.Create(m_hWnd, rc, m_strText, WS_VISIBLE | WS_CHILD | WS_BORDER | ES_AUTOHSCROLL | ES_LEFT, 0, IDW_INNER_EDIT);
		m_edit.SetFont((HFONT)GetStockObject(DEFAULT_GUI_FONT));
		m_edit.SetSel(0, -1);
		m_pfn = (WNDPROC)::GetWindowLongPtr(m_edit, GWLP_WNDPROC);
		::SetWindowLongPtr(m_hWnd, GWLP_USERDATA, (LONG_PTR)m_pfn);
		::SetWindowLongPtr(m_edit, GWLP_WNDPROC, (LONG_PTR)HookEditProc);
		return 0;
	}

	void OnSize(UINT nType, int cx, int cy)
	{
		m_edit.MoveWindow(0, 0, cx, cy, TRUE);
	}

	void OnSetFocus(HWND hOldWnd)
	{
		m_edit.SetFocus();
	}

	void OnDestroy()
	{
		WNDPROC pfn = (WNDPROC)::GetWindowLongPtr(m_hWnd, GWLP_USERDATA);
		::SetWindowLongPtr(m_edit, GWLP_WNDPROC, (LONG_PTR)pfn);
		m_edit.DestroyWindow();
	}

	static LRESULT CALLBACK HookEditProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
	{
		HWND hParent = ::GetParent(hwnd);
		int id = ::GetDlgCtrlID(hwnd);
		switch (msg) {
		case WM_KEYDOWN:
//dbg.printf(L"key=%02X\n", wParam);
			switch (wParam) {
			case VK_RETURN:
				::PostMessage(hParent, WM_COMMAND, MAKELONG(id, EN_ENDEDIT), (LPARAM)hwnd);
				return 0;
			case VK_ESCAPE:
				::PostMessage(hParent, WM_COMMAND, MAKELONG(id, EN_CANCELEDIT), (LPARAM)hwnd);
				return 0;
			}
			break;

		case WM_KILLFOCUS:
			::PostMessage(hParent, WM_COMMAND, MAKELONG(id, EN_CANCELEDIT), (LPARAM)hwnd);
			break;
		}

		WNDPROC pfn = (WNDPROC)::GetWindowLongPtr(hParent, GWLP_USERDATA);
		return ::CallWindowProc(pfn, hwnd, msg, wParam, lParam);
	}

	void OnEndEdit()
	{
//dbg.printf(L"end edit\n");
		NotifyEndEdit(false);
	}

	void OnCancelEdit()
	{
		NotifyEndEdit(true);
	}

	void NotifyEndEdit(bool bCancel)
	{
//dbg.printf(L"end edit\n");
		ATLASSERT(m_owner.IsWindow());
		if (m_owner.IsWindow()) {
			NMHDR hdr;
			hdr.hwndFrom = m_hWnd;
			hdr.idFrom = GetDlgCtrlID();
			hdr.code = bCancel ? TED_CANCELEDIT : TED_ENDEDIT;
			m_owner.SendMessage(WM_NOTIFY, (WPARAM)hdr.idFrom, (LPARAM)&hdr);
		}
		DestroyWindow();
	}

	void SetEditText(LPCTSTR pszText)
	{
		m_strText = pszText;
		if (m_edit.IsWindow()) {
			m_edit.SetWindowText(m_strText);
			m_edit.SetSel(0, -1);
		}
	}

	int GetEditText(LPTSTR pszText, int cchMax)
	{
		int len = m_strText.GetLength();
		if (pszText == NULL && cchMax == 0)
			return len;
		ATLASSERT(pszText != NULL);
		lstrcpyn(pszText, m_strText, cchMax);
		return lstrlen(pszText);
	}

	void SetOwner(HWND hOwner)
	{
		ATLASSERT(::IsWindow(hOwner));
		m_owner = hOwner;
	}

protected:
	CWindow m_owner;
	CEdit m_edit;
	CString m_strText;
	WNDPROC m_pfn;
};


#endif					// __TIPEDIT_H__
