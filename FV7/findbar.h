////////////////////////////////////////////////////////////////////////
// findbar.h

#pragma once
#ifndef __FINBAR_H__
#define __FINBAR_H__

////////////////////////////////////////////////////////////////////////
//

class CFindBar : public CWindowImpl<CFindBar, CToolBarCtrl>
{
private:
	UINT m_idCtrl;
	CEdit m_edit;
	HACCEL m_haccel;

public:
	CFindBar()
		: m_idCtrl(0)
		, m_haccel(NULL)
	{
	}

	BOOL ProcessMessage(const MSG* pMsg)
	{
		if (m_haccel && ::TranslateAccelerator(GetParent(), m_haccel, (MSG*)pMsg)) {
			return TRUE;
		}
		return FALSE;
	}

	BOOL OnIdle()
	{
		if (m_edit.IsWindow() && m_edit.IsWindowVisible()) {
			BOOL bEnable = m_edit.GetWindowTextLength() > 0;
			EnableButton(IDM_FINDER_NEXT, bEnable);
			EnableButton(IDM_FINDER_PREV, bEnable);
			EnableButton(IDM_FINDER_SELECT, bEnable);
		}
		// チェックボタン
		CheckButton(IDM_FINDER_REGEXP, (gnFopStat & FOS_REXFINDER) != 0);
		return FALSE;
	}

protected:

BEGIN_MSG_MAP(CFindBar)
	MESSAGE_HANDLER(WM_SETFOCUS, OnHandlerSetFocus)
	ON_WM_MOUSEACTIVATE()
	ON_WM_DESTROY()
	ON_EN_CHANGE(m_idCtrl, OnEditChanged)
END_MSG_MAP()

	LRESULT OnHandlerSetFocus(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
	{
		if (m_edit.IsWindow()) {
			m_edit.SetFocus();
		}
		return 0;
	}

	int OnMouseActivate(HWND hDesktopWnd, UINT nHitTest, UINT message)
	{
		LRESULT lr = DefWindowProc();
		if (lr != MA_NOACTIVATE && lr != MA_NOACTIVATEANDEAT) {		// frame does not want to activate
			if (nHitTest == HTCLIENT)
				fvApp.SetActiveView(m_hWnd);
		}
		return (int)lr;
	}

	void OnDestroy()
	{
		if (m_haccel) {
			DestroyAcceleratorTable(m_haccel);
			m_haccel = NULL;
		}
	}

	inline void OnEditChanged() { GetParent().SendMessage(WM_COMMAND, MAKEWPARAM(IDM_FINDER_NEXT, EN_CHANGE)); }

public:
	HWND Initialize(int index, UINT idCtrl)
	{
		if (m_edit.IsWindow()) return NULL;

		// 文字列ありボタン
		SetExtendedStyle(TBSTYLE_EX_MIXEDBUTTONS);

		static UINT sids[] = { IDM_FINDER_NEXT, IDM_FINDER_PREV, IDM_FINDER_SELECT, IDM_FINDER_REGEXP };
		for (int i = 0; i < ARRAYSIZE(sids); i++) {
			TBBUTTONINFO tbi;
			memset(&tbi, 0, sizeof(tbi));
			tbi.cbSize = sizeof(tbi);
			tbi.dwMask = TBIF_STYLE;
			if (sids[i]) {
				tbi.dwMask |= TBIF_TEXT;
				CString str; str.LoadString(sids[i]);
				tbi.pszText = (LPTSTR)str.GetString();
				tbi.fsStyle = BTNS_SHOWTEXT;
				if (sids[i] == IDM_FINDER_REGEXP)
					tbi.fsStyle |= BTNS_CHECK;
			}
			SetButtonInfo(sids[i], &tbi);
		}

		// Editコントロール分をインデントして確保
		SetIndent(2 + 192 + 2);

		// ツールバーの高さを調整
		CRect rc;
		GetWindowRect(&rc);
		CRect rcb;
		GetItemRect(0, &rcb);
		SetWindowPos(NULL, 0, 0, rc.Width(), rcb.Height(), SWP_NOZORDER | SWP_NOMOVE);

		// Editコントロール作成
		DWORD style = WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS | WS_CLIPCHILDREN | ES_LEFT | ES_AUTOHSCROLL;
		if (m_edit.Create(m_hWnd, NULL, NULL, style, WS_EX_CLIENTEDGE, idCtrl, NULL)) {
			m_idCtrl = idCtrl;
			// 文字数制限
			m_edit.SetLimitText(MAX_PATH);

			// フォント設定
			HFONT hfont = GetFont();
			m_edit.SetFont(hfont, TRUE);

			// フォントの高さを元に、Editコントロールの高さを調整
			CClientDC dc(NULL);
			HFONT htmp = dc.SelectFont(hfont);
			TEXTMETRIC tm;
			dc.GetTextMetrics(&tm);
			dc.SelectFont(htmp);

			int height = tm.tmHeight + tm.tmExternalLeading + 2 + GetSystemMetrics(SM_CXEDGE) * 2;
			m_edit.MoveWindow(2, rcb.top + (rcb.Height() - height) / 2, 192, height);

			// アクセラレータ
			m_haccel = ::LoadAccelerators(RESINST, MAKEINTRESOURCE(IDW_FINDBAR));
		}

		return m_edit.m_hWnd;
	}

	inline bool IsMyMessage(const MSG* pMsg) const { return (m_edit.IsWindow() && ((pMsg->hwnd == m_edit) || (pMsg->message == WM_KEYDOWN && pMsg->wParam == VK_ESCAPE))); }
	inline int GetText(LPTSTR psz, int cchMax) { return m_edit.GetWindowText(psz, cchMax); }
	inline void SetText(LPCTSTR psz) { m_edit.SetWindowText(psz); m_edit.SetSel(lstrlen(psz), lstrlen(psz)); }
	inline bool hasFocus() { return (::GetFocus() == m_edit); }
};


#endif		// __NAVIBAR_H__
