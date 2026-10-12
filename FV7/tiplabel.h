////////////////////////////////////////////////////////////////////////
// tiplabel.h

#pragma once
#ifndef __TIPLABEL_H__
#define __TIPLABEL_H__

#define TLM_BASE					(WM_USER + 0x1128)
#define TLM_SETLABEL				(TLM_BASE + 1)				// wParam = DROPIMAGE_xxxx, lParam = LPCTSTR
#define TLM_SHOW					(TLM_BASE + 2)				// wParam = BOOL
#define TLM_SETDROPDESCRIPTION		(TLM_BASE + 3)				// lParam = DROPDESCRIPTION*

class CTipLabelWnd : public CWindowImpl<CTipLabelWnd>
{
private:
	CString m_strLabel;
	CFont m_FontDefault;
	CFontHandle m_Font;
	HRGN m_hrgn;
	DROPDESCRIPTION m_desc;

public:
	DECLARE_WND_CLASS_EX(_T("TipLabel"), CS_HREDRAW | CS_VREDRAW | CS_SAVEBITS, COLOR_WINDOW)

public:
	CTipLabelWnd()
		: m_hrgn(NULL)
	{
		memset(&m_desc, 0, sizeof(m_desc));
		LOGFONT logfont;
		SystemParametersInfo(SPI_GETICONTITLELOGFONT, sizeof(LOGFONT), &logfont, FALSE);
		m_FontDefault.CreateFontIndirect(&logfont);
		m_Font = m_FontDefault;
	}

	HWND Create()
	{
		HWND hwnd = CWindowImpl::Create(NULL, CWindow::rcDefault, NULL, WS_POPUP, WS_EX_TOOLWINDOW | WS_EX_TOPMOST | WS_EX_LAYERED);
		ATLASSERT(hwnd != NULL);
		::SetLayeredWindowAttributes(hwnd, 0, 224, LWA_ALPHA);
		return hwnd;
	}

	BEGIN_MSG_MAP(CTipLabelWnd)
		ON_WM_CREATE()
		ON_WM_DESTROY()
		ON_WM_PAINT()
		ON_WM_PRINTCLIENT()
		ON_WM_ERASEBKGND()
		ON_WM_SIZE()
		MESSAGE_HANDLER(WM_SETFONT, OnHandlerSetFont)
		ON_MESSAGE_VOID(TLM_SETLABEL, OnSetLabel)
		ON_MESSAGE_VOID(TLM_SETDROPDESCRIPTION, OnSetDropDescription)
	END_MSG_MAP()

private:
	int OnCreate(CREATESTRUCT* pcs)
	{
		SetFont(m_FontDefault);
		OnSetLabel((WPARAM)DROPIMAGE_INVALID, 0);
		return 0;
	}

	void OnDestroy()
	{
		if (m_hrgn)
			::DeleteObject(m_hrgn);
	}

	void OnPaint()
	{
		CPaintDC dc(m_hWnd);
		OnDraw(dc);
	}

	void OnPrintClient(HDC hdc, DWORD flags)
	{
		if (flags & (PRF_CHECKVISIBLE | PRF_CLIENT | PRF_OWNED))
			OnDraw(hdc);
	}

	void OnDraw(HDC hdc)
	{
		if ((m_desc.type == DROPIMAGE_INVALID) || (m_desc.szMessage[0] == 0))
			return;

		CDCHandle dc = hdc;
		HFONT hfOld = dc.SelectFont(m_Font);

		TEXTMETRIC tm;
		dc.GetTextMetrics(&tm);

		CRect rc;
		GetClientRect(&rc);
		rc.InflateRect(-3, -2);
		rc.left += 3 + GetSystemMetrics(SM_CXSMICON);

		// ドラッグイメージのタイプにより色分け
		COLORREF clrM, clrI;
		if (m_desc.type == DROPIMAGE_NONE) {
			clrM = RGB(192, 0, 0);
			clrI = RGB(128, 0, 0);
		}
		else {
			clrM = RGB(0, 0, 192);
			clrI = RGB(0, 0, 0);
		}

		// 文字列描画設定
		UINT align = dc.SetTextAlign(TA_LEFT | TA_TOP | TA_UPDATECP);
		int bkMode = dc.SetBkMode(TRANSPARENT);
		COLORREF clr = dc.SetTextColor(clrM);

		// 描画開始位置へ移動
		dc.MoveTo(rc.left, rc.top + (rc.Height() - (tm.tmHeight + tm.tmExternalLeading)) / 2);
		CString str = m_desc.szMessage;

		// 置換因子位置取得
		LPTSTR p = StrStr(str, _T("%1"));
		if (p) {
			*p = 0;
			p += 2;
		}

		// 出力
		dc.TextOut(0, 0, str, lstrlen(str));
		if (m_desc.szInsert[0]) {
			dc.SetTextColor(clrI);
			dc.TextOut(0, 0, m_desc.szInsert, lstrlen(m_desc.szInsert));
			dc.SetTextColor(clrM);
		}
		dc.TextOut(0, 0, p, lstrlen(p));

		dc.SetTextColor(clr);
		dc.SetBkMode(bkMode);
		dc.SetTextAlign(align);

		dc.SelectFont(hfOld);
	}

	BOOL OnEraseBkgnd(HDC hdc)
	{
		CDCHandle dc = hdc;
		CRect rc;
		GetClientRect(&rc);

		// 中身（グラデーション）
		Graphics g(dc);
		Rect r(rc.left, rc.top, rc.Width(), rc.Height());
		Color cF(255, 255, 255, 255);
		Color cT(255, 228, 228, 240);
		LinearGradientBrush lbr(r, cF, cT, LinearGradientModeVertical);

		REAL fac[3] = {0.0f, 0.9f, 1.0f};
		REAL pos[3] = {0.0f, 0.7f, 1.0f};
		lbr.SetBlend(fac, pos, 3);
		g.FillRectangle(&lbr, r);

		// 枠
		rc.right--;
		rc.bottom--;
		CPen pen;
		pen.CreatePen(PS_SOLID, 0, GetSysColor(COLOR_3DDKSHADOW));
		HPEN hpen = dc.SelectPen(pen);
		HBRUSH hbrush = dc.SelectBrush((HBRUSH)GetStockObject(NULL_BRUSH));
		dc.RoundRect(&rc, CPoint(5, 5));
		dc.SelectBrush(hbrush);
		dc.SelectPen(hpen);

		return TRUE;
	}

	void OnSize(UINT flag, int cx, int cy)
	{
		if (m_hrgn) {
			::DeleteObject(m_hrgn);
			m_hrgn = NULL;
		}

		m_hrgn = ::CreateRoundRectRgn(0, 0, cx, cy, 5, 5);
		SetWindowRgn(m_hrgn, TRUE);
	}

	LRESULT OnHandlerSetFont(UINT msg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
	{
		m_Font = (HFONT)wParam;
		UpdateLabel();
		if (lParam)
			Invalidate(TRUE);
		return 0;
	}

	void OnSetDropDescription(WPARAM wParam, LPARAM lParam)
	{
		DROPDESCRIPTION* pdd = (DROPDESCRIPTION*)lParam;
		if (!pdd || IsBadReadPtr(pdd, sizeof(DROPDESCRIPTION))) return;

		memcpy(&m_desc, pdd, sizeof(m_desc));
		CString str = m_desc.szMessage;
		if (!str.IsEmpty() && str.Replace(_T("%%"), _T("%")) > 0)
			lstrcpyn(m_desc.szMessage, str, ARRAYSIZE(m_desc.szMessage));

		// 更新が必要か確認して更新
		ConfirmUpdate();
	}

	void OnSetLabel(WPARAM wParam, LPARAM lParam)
	{
		memset(&m_desc, 0, sizeof(m_desc));
		m_desc.type = (DROPIMAGETYPE)wParam;
		if (lParam)
			lstrcpyn(m_desc.szMessage, (LPCTSTR)lParam, ARRAYSIZE(m_desc.szMessage));
		// 更新が必要か確認して更新
		ConfirmUpdate();
	}

	void BuildLabel()
	{
		m_strLabel.Empty();
		if (m_desc.type != DROPIMAGE_INVALID) {
			m_strLabel = m_desc.szMessage;
			if (!m_strLabel.IsEmpty())
				m_strLabel.Replace(_T("%1"), m_desc.szInsert);
		}
	}

	void ConfirmUpdate()
	{
		// 更新が必要か確認して更新する
		CString str = m_strLabel.GetString();	// 直前のラベルを退避
		BuildLabel();							// 新しいラベルを構築

		bool bNeedUpdate = false;
		if (!m_strLabel.IsEmpty()) {
			if (str.IsEmpty() || m_strLabel.Compare(str))
				bNeedUpdate = true;
		}
		else {
			if (!str.IsEmpty())
				bNeedUpdate = true;
		}

		if (bNeedUpdate)
			UpdateLabel();

		if (m_strLabel.IsEmpty() && IsWindowVisible())
			ShowWindow(SW_HIDE);
		else if (!m_strLabel.IsEmpty() && !IsWindowVisible())
			ShowWindow(SW_SHOWNOACTIVATE);
	}

	void UpdateLabel()
	{
		CClientDC dc(m_hWnd);
		HFONT hfont = dc.SelectFont(m_Font);
		CRect rc(0, 0, 0, 0);
		dc.DrawText(m_strLabel, -1, &rc, DT_NOPREFIX | DT_CALCRECT | DT_SINGLELINE);
		dc.SelectFont(hfont);

		int cxIcon = GetSystemMetrics(SM_CXSMICON);
		int cyIcon = GetSystemMetrics(SM_CYSMICON);

		rc.bottom = max(cyIcon, rc.bottom) + 4;						// 上下それぞれ=2
		rc.right += cxIcon + 9;										// アイコン左側=3, アイコンとテキストの間隔=3, テキスト右側=3

		DWORD style = GetStyle();
		if (!(style & WS_BORDER)) {
			rc.right += 2;
			rc.bottom += 2;
		}

		// クライアント矩形からウィンドウ矩形を計算
		AdjustWindowRectEx(&rc, style, FALSE, GetExStyle());

		CRect rcw;
		GetWindowRect(&rcw);
		SetWindowPos(NULL, rcw.left, rcw.top, rc.Width(), rc.Height(), SWP_NOMOVE | SWP_NOZORDER | SWP_NOACTIVATE);
	}

	//------------------------
	// 以降外部からの呼び出し
	//------------------------
public:
	void SetLabel(DROPIMAGETYPE type, LPCTSTR pszLabel)
	{
		if (::IsWindow(m_hWnd))
			::SendMessage(m_hWnd, TLM_SETLABEL, (WPARAM)type, (LPARAM)pszLabel);
	}

	void SetDropDescription(DROPDESCRIPTION* pdd)
	{
		if (::IsWindow(m_hWnd))
			::SendMessage(m_hWnd, TLM_SETDROPDESCRIPTION, 0, (LPARAM)pdd);
	}
};




#endif		// __TIPLABEL_H__
