#include "stdafx.h"
#include <math.h>
#include "ip_view.h"



//=========================================================================
//---- ip_draw.cpp   描画・他
//
//		折り返しとか色とかを考慮しつつ、実際に描画処理を
//		行うのがここ。あとメッセージディスパッチャなども
//		ついでにこのファイルに。^^;
//
//---- ip_text.cpp   文字列操作・他
//---- ip_parse.cpp  キーワード解析
//---- ip_wrap.cpp   折り返し
//---- ip_scroll.cpp スクロール
//---- ip_cursor.cpp カーソルコントロール
//=========================================================================



//-------------------------------------------------------------------------
// Viewの初期化・解放
//-------------------------------------------------------------------------

CPadView::CPadView()
	: m_vlNum(0)
	, m_textCx(0)
	, m_canvas(NULL)
	, m_cursor(NULL)
{
/* @@
	static bool ClassRegistered = false;
	if (!ClassRegistered) {
		// 初回構築時のみ、クラス登録を行う
		ClassRegistered = true;
		WNDCLASSEX wc    = {0};
		wc.lpszClassName = className_;
		wc.style         = CS_DBLCLKS | CS_OWNDC;
		wc.hCursor       = app().LoadOemCursor( IDC_IBEAM );

		// GlobalIMEを有効にする
		ATOM a = WndImpl::Register( &wc );
		ime().FilterWindows( &a, 1 );
	}
@@ */

	// ドキュメント作成
	m_doc  = new CPadDocument;
}

CPadView::~CPadView()
{
	// ドキュメント破棄
	delete m_doc;
}

int CPadView::OnCreate(CREATESTRUCT* pcs)
{
	m_canvas = new CCanvas(this);
	m_cursor = new CPadCursor(m_hWnd, this, m_doc);

	// 適当に折り返し情報初期化
	InsertMulti(0, m_doc->tln() - 1);

	// 適当にスクロール情報初期化
	m_scrVert.cbSize = m_scrHorz.cbSize = sizeof(m_scrVert);
	m_scrVert.fMask  = m_scrHorz.fMask  = SIF_PAGE | SIF_POS | SIF_RANGE;
	m_scrVert.nMin   = m_scrHorz.nMin   = 0;
	m_scrVert.nPos   = m_scrHorz.nPos   = 0;
	m_scrVert.fMask |= SIF_DISABLENOSCROLL;
	m_top_tl = m_top_vrl = 0;
	m_cur_vl = 0;

	ReSetScrollInfo();

	m_doc->AddHandler(this);

	return 0;
}

void CPadView::OnDestroy()
{
	m_doc->RemoveHandler(this);

	if (m_cursor) {
		delete m_cursor;
		m_cursor = NULL;
	}
	if (m_canvas) {
		delete m_canvas;
		m_canvas = NULL;
	}
}



//-------------------------------------------------------------------------
// サブオブジェクトにそのまま回す
//-------------------------------------------------------------------------

void CPadView::OnDocEventKeywordChanged()
{
	Invalidate(FALSE);
}


static bool DrawClientEdge(HWND hWnd, HRGN hRgnUpdate, HBRUSH hBrush)
{
	CWindowDC dc(hWnd);
	if (dc.IsNull())
		return false;

	// Get border size
	int cxBorder = GetSystemMetrics(SM_CXBORDER);
	int cyBorder = GetSystemMetrics(SM_CYBORDER);

	RECT rect;
	::GetWindowRect(hWnd, &rect);

	// Remove the client edge from the update region
	int cxEdge = GetSystemMetrics(SM_CXEDGE);
	int cyEdge = GetSystemMetrics(SM_CYEDGE);
	::InflateRect(&rect, -cxEdge, -cyEdge);
	CRgn rgn;
	rgn.CreateRectRgnIndirect(&rect);
	if (rgn.IsNull())
		return false;

	if (hRgnUpdate != NULL)
		rgn.CombineRgn(hRgnUpdate, rgn, RGN_AND);

	::OffsetRect(&rect, -rect.left, -rect.top);

	::OffsetRect(&rect, cxEdge, cyEdge);
	dc.ExcludeClipRect(&rect);
	::InflateRect(&rect, cxEdge, cyEdge);

	if (hBrush == NULL) {
// need conditional code because types don't match in winuser.h
#ifdef _WIN64
		hBrush = (HBRUSH)::GetClassLongPtr(hWnd, GCLP_HBRBACKGROUND);
#else
		hBrush = (HBRUSH)UlongToPtr(::GetClassLongPtr(hWnd, GCLP_HBRBACKGROUND));
#endif
	}
	dc.FillRect(&rect, hBrush);

	// Use background brush too, since theme border might not cover everything
	if (cxBorder < cxEdge && cyBorder < cyEdge) {
		::InflateRect(&rect, cxBorder - cxEdge, cyBorder - cyEdge);
#ifdef _WIN64
		hBrush = (HBRUSH)::GetClassLongPtr(hWnd, GCLP_HBRBACKGROUND);
#else
		hBrush = (HBRUSH)UlongToPtr(::GetClassLongPtr(hWnd, GCLP_HBRBACKGROUND));
#endif
		dc.FillRect(&rect, hBrush);
	}

	::DefWindowProc(hWnd, WM_NCPAINT, (WPARAM)rgn.m_hRgn, 0L);
	return true;
}

LRESULT CPadView::OnHandlerNcPaint(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	LRESULT lRet = 0;
	bHandled = FALSE;

	HRGN hRgn = (wParam != 1) ? (HRGN)wParam : NULL;

	// WS_BORDER または WS_EX_CLIENTEDGE スタイルを独自に描く
	if (DrawClientEdge(m_hWnd, hRgn, GetSysColorBrush(COLOR_3DSHADOW)))
		bHandled = TRUE;
	return lRet;
}

void CPadView::OnPaint()
{
	CPaintDC dc(m_hWnd);
	OnDraw(dc.m_ps);
}

BOOL CPadView::OnEraseBkgnd(HDC hdc)
{
	return TRUE;
}

void CPadView::OnSize(UINT type, int cx, int cy)
{
	ATLASSERT(m_canvas != NULL);
	if (type != SIZE_MINIMIZED)
		DoResize(m_canvas->ResizeView(cx, cy));
}

void CPadView::OnSetFocus(HWND hwndOld)
{
	ATLASSERT(m_cursor != NULL);
	m_cursor->SetFocus();
}

void CPadView::OnKillFocus(HWND hwndOld)
{
	ATLASSERT(m_cursor != NULL);
	m_cursor->KillFocus();
}

void CPadView::OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags)
{
	ATLASSERT(m_cursor != NULL);
	m_cursor->KeyDown(nChar, nRepCnt, nFlags);
}

void CPadView::OnChar(UINT nChar, UINT nRepCnt, UINT nFlags)
{
	ATLASSERT(m_cursor != NULL);
	m_cursor->Char(nChar, nRepCnt, nFlags);
}

void CPadView::OnLButtonDown(UINT nFlags, CPoint point)
{
	ATLASSERT(m_cursor != NULL);
	m_cursor->LButtonDown(point.x, point.y, (nFlags & MK_SHIFT) != 0);
}

void CPadView::OnLButtonUp(UINT nFlags, CPoint point)
{
	ATLASSERT(m_cursor != NULL);
	m_cursor->LButtonUp(point.x, point.y);
}

void CPadView::OnLButtonDblClk(UINT nFlags, CPoint point)
{
	ATLASSERT(m_cursor != NULL);
	m_cursor->LButtonDblClk(point.x, point.y);
}

void CPadView::OnMouseMove(UINT nFlags, CPoint point)
{
	ATLASSERT(m_cursor != NULL);
	m_cursor->MouseMove(point.x, point.y);
}

LRESULT CPadView::OnHandlerSetCursor(UINT msg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	HWND hwnd  = (HWND)wParam;
	UINT nHitTest = (UINT)LOWORD(lParam);
	if (nHitTest == HTCLIENT && hwnd == m_hWnd) {
		CPoint point(GetMessagePos());
		ScreenToClient(&point);
		if (point.x >= lna()) {
			::SetCursor(::LoadCursor(NULL, IDC_IBEAM));
			return TRUE;
		}
	}
	bHandled = FALSE;
	return FALSE;
}

void CPadView::OnTimer(UINT_PTR nIDEvent)
{
	ATLASSERT(m_cursor != NULL);
	m_cursor->Timer();
}

void CPadView::OnHandlerCopy(WPARAM, LPARAM)
{
	ATLASSERT(m_cursor != NULL);
	m_cursor->Copy();
}

void CPadView::OnHandlerCut(WPARAM, LPARAM)
{
	ATLASSERT(m_cursor != NULL);
	m_cursor->Cut();
}

void CPadView::OnHandlerPaste(WPARAM, LPARAM)
{
	ATLASSERT(m_cursor != NULL);
	m_cursor->Paste();
}

LRESULT CPadView::OnHandlerUndo(WPARAM, LPARAM)
{
	ATLASSERT(m_doc != NULL);
	m_doc->Undo();
	return TRUE;
}

LRESULT CPadView::OnHandlerCanUndo(WPARAM, LPARAM)
{
	ATLASSERT(m_doc != NULL);
	return (LRESULT)m_doc->CanUndo();
}

LRESULT CPadView::OnHandlerSetReadOnly(WPARAM wParam, LPARAM)
{
	ATLASSERT(m_cursor != NULL);
	m_cursor->SetROMode(wParam != FALSE);
	return TRUE;
}

LRESULT CPadView::OnHandlerImeRequest(WPARAM wParam, LPARAM lParam)
{
	ATLASSERT(m_cursor != NULL);
	switch (wParam) {
	case IMR_RECONVERTSTRING:
		return m_cursor->ImeReconvertString(reinterpret_cast<RECONVERTSTRING*>(lParam) );
	case IMR_CONFIRMRECONVERTSTRING:
		return m_cursor->ImeConfirmReconvertString(reinterpret_cast<RECONVERTSTRING*>(lParam) );
	}
	return 0;
}

LRESULT CPadView::OnHandlerImeStartComposition(UINT msg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	ATLASSERT(m_cursor != NULL);
	m_cursor->ImeComposition(0);
	bHandled = FALSE;
	return 1;
}

LRESULT CPadView::OnHandlerImeComposition(UINT msg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	ATLASSERT(m_cursor != NULL);
	m_cursor->ImeComposition(lParam);
	if (lParam & GCS_RESULTSTR)
		return 0;

	bHandled = FALSE;
	return 1;
}

//-------------------------------------------------------------------------
// 線を引くとか四角く塗るとか、そーいう基本的な処理
//-------------------------------------------------------------------------

#define ROUND_INT(a)		(int)floor(0.5F+(a))

CPainter::CPainter( HDC hdc, const VConfig& vc )
	: m_dc(hdc)
	, m_gr(hdc)
	, m_hfont(::CreateFontIndirect(&vc.m_logfont))
	, m_widthTable(new int[65536])
	, m_hbmCR(NULL)
	, m_hbmTB(NULL)
	, m_hbmZS(NULL)
	, m_hbmHS(NULL)
{
	// 制御文字を描画するか否か？のフラグを記憶
	int i;
	for (i = 0; i < ARRAYSIZE(m_scDraw); ++i)
		m_scDraw[i] = vc.m_sc[i];

	// 文字色を記憶
	for (i = 0; i < ARRAYSIZE(m_colorTable); i++) {
		m_colorTable[i] = vc.m_color[i];
	}

	// DCにセット
	m_dc.SelectFont(m_hfont);
	m_dc.SetBkMode(TRANSPARENT);
	m_dc.SetMapMode(MM_TEXT);

	// 文字幅テーブル初期化（ASCII範囲の文字以外は遅延処理）
	memset(m_widthTable, -1, 65536 * sizeof(int));
	int firstChar = L' ';
	int lastChar = L'~';
	int count = lastChar - firstChar + 1;
	ABC* abcs = new ABC[count];

	m_dc.GetCharABCWidths(firstChar, lastChar, abcs);
	for (int i = 0; i < count; ++i) {
		m_widthTable[firstChar++] = abcs[i].abcA + abcs[i].abcB + abcs[i].abcC;
	}
	m_widthTable[L'\t'] = W() * Max(1, vc.m_tabstep);
  delete[] abcs;

	// 数字の最大幅を計算
	m_figWidth = 0;
	for (WCHAR ch = L'0'; ch <= L'9'; ch++) {
		if (m_figWidth < m_widthTable[ch])
			m_figWidth = m_widthTable[ch];
	}

	// 高さの情報
	TEXTMETRIC tm;
	m_dc.GetTextMetrics(&tm);
	m_height = tm.tmHeight + tm.tmExternalLeading;
	m_margin_v = 0;
	if (vc.m_linedenom > 0)
		m_margin_v = ROUND_INT((float)m_height / (float)vc.m_linedenom);		// 行間を空けるためのマージン
	m_height += m_margin_v;

	// LOGFONT
	::GetObject(m_hfont, sizeof(LOGFONT), &m_logfont );

	// 制御コードのビットマップなど
	CreateSymbolBitmaps();
}

CPainter::~CPainter()
{
	// 適当な別オブジェクトをくっつけて自分を解放する
	m_dc.SelectFont(AtlGetStockFont(OEM_FIXED_FONT));
	::DeleteObject(m_hfont);

	if (m_hbmCR) ::DeleteObject(m_hbmCR);
	if (m_hbmTB) ::DeleteObject(m_hbmTB);
	if (m_hbmZS) ::DeleteObject(m_hbmZS);
	if (m_hbmHS) ::DeleteObject(m_hbmHS);

	delete [] m_widthTable;
}

////////////////////////////////////////////////////////////////////////
//

/* ==================

----@---		0xf7		0x08
----@---		0xf7		0x08
----@---		0xf7		0x08
----@---		0xf7		0x08
----@---		0xf7		0x08
--@-@-@-		0xd5		0x2a
---@@@--		0xe3		0x1c
----@---		0xf7		0x08


---@----		0xef
----@---		0xf7
-----@--		0xfb
----@---		0xf7
---@----		0xef

================== */

void CPainter::CreateSymbolBitmaps()
{
	if (m_hbmCR) DeleteObject(m_hbmCR);
	if (m_hbmTB) DeleteObject(m_hbmTB);

	int i, j;
	int dx = W();
	int dy = H();			// 行の高さ
	TEXTMETRIC tm;
	HFONT hfont = m_dc.SelectFont(m_hfont);
	m_dc.GetTextMetrics(&tm);
	int ty = tm.tmAscent;		// 文字のみの高さ（ベースラインより上）
	m_dc.SelectFont(hfont);

	m_hbmCR = CreateBitmap(dx,       dy, 1, 1, NULL);
	m_hbmTB = CreateBitmap(dx,       dy, 1, 1, NULL);
	m_hbmZS = CreateBitmap(W(L'　'), dy, 1, 1, NULL);
	m_hbmHS = CreateBitmap(dx,       dy, 1, 1, NULL);

	// 作業用DC作成
	CDC dc1, dc2;
	dc1.CreateCompatibleDC(m_dc);
	dc2.CreateCompatibleDC(m_dc);

	// 改行マーク --- 文字の高さを計算して作成
	HBITMAP hbm;
	int cy;

	CSimpleArray<short> bittbl;
	bittbl.Add(0xff);
	bittbl.Add(0xff);
	for (i = 0; i < ty - 5; i++)
		bittbl.Add(0xf7);
	bittbl.Add(0xd5);
	bittbl.Add(0xe3);
	bittbl.Add(0xf7);
	cy = bittbl.GetSize();
	hbm = CreateBitmap(8, cy, 1, 1, bittbl.GetData());
	HBITMAP hbm1 = dc1.SelectBitmap(m_hbmCR);
	HBITMAP hbm2 = dc2.SelectBitmap(hbm);
	dc1.PatBlt(0, 0, dx, dy, WHITENESS);
	dc1.BitBlt((dx - 8) / 2, (dy - tm.tmHeight) / 2, 8, cy, dc2, 0, 0, SRCCOPY);
	dc1.SelectBitmap(hbm1);
	::DeleteObject(dc2.SelectBitmap(hbm2));

	// TAB記号
	bittbl.RemoveAll();
	bittbl.Add(0xef);
	bittbl.Add(0xf7);
	bittbl.Add(0xfb);
	bittbl.Add(0xf7);
	bittbl.Add(0xef);
	cy = bittbl.GetSize();
	hbm = CreateBitmap(8, cy, 1, 1, bittbl.GetData());
	hbm1 = dc1.SelectBitmap(m_hbmTB);
	hbm2 = dc2.SelectBitmap(hbm);
	dc1.PatBlt(0, 0, dx, dy, WHITENESS);
	dc1.BitBlt((dx - 8) / 2, (dy - cy) / 2, 8, cy, dc2, 0, 0, SRCCOPY);
	dc1.SelectBitmap(hbm1);
	::DeleteObject(dc2.SelectBitmap(hbm2));

	// 全角スペース
	hbm1 = dc1.SelectBitmap(m_hbmZS);
	CRect rc(0, 0, W(L'　'), ty);
	dc1.PatBlt(0, 0, rc.Width(), dy, WHITENESS);
	rc.InflateRect(-2, 0);
	rc.top += 2;
	for (i = rc.top; i < rc.bottom; i++) {
		for (j = rc.left; j < rc.right; j++) {
			if (!(i & 1) && !(j & 1))
				dc1.SetPixel(j, i, RGB(0, 0, 0));
			else if ((i & 1) && (j & 1))
				dc1.SetPixel(j, i, RGB(0, 0, 0));
		}
	}
	rc.InflateRect(-1, -1);
	dc1.PatBlt(rc.left, rc.top, rc.Width(), rc.Height(), WHITENESS);
	dc1.SelectBitmap(hbm1);

	// 半角スペース（ホチキスの芯型）
	hbm1 = dc1.SelectBitmap(m_hbmHS);
	dc1.PatBlt(0, 0, dx, dy, WHITENESS);
	HPEN hpen = dc1.SelectPen(AtlGetStockPen(BLACK_PEN));
	dc1.MoveTo(0     , ty - 2, NULL);
	dc1.LineTo(0     , ty);
	dc1.LineTo(dx - 2, ty);
	dc1.LineTo(dx - 2, ty - 3);
	dc1.SelectPen(hpen);
	dc1.SelectBitmap(hbm1);
}

void CPainter::Highlight(const CRect& rc)
{
	// Create a SolidBrush object.
	COLORREF clr = GetSysColor(COLOR_HIGHLIGHT);
	SolidBrush brush(Color(128, GetRValue(clr), GetGValue(clr), GetBValue(clr)));
	m_gr.FillRectangle(&brush, rc.left, rc.top, rc.Width(), rc.Height());
}

void CPainter::DrawSymbolBitmap( HBITMAP hbm, int x, int y, int times, int indexFore, int indexBack )
{
	BITMAP bm;
	if (::GetObject(hbm, sizeof(bm), &bm) < 1) return;

	CDC dcw; dcw.CreateCompatibleDC(m_dc);
	HBITMAP hbmw = dcw.SelectBitmap(hbm);
	COLORREF clrFore = m_dc.SetTextColor(m_colorTable[indexFore]);
	COLORREF clrBack = m_dc.SetBkColor(m_colorTable[indexBack]);
	while (times--) {
		m_dc.BitBlt(x, y, bm.bmWidth, bm.bmHeight, dcw, 0, 0, SRCCOPY);
		x += bm.bmWidth;
	}
	dcw.SelectBitmap(hbmw);

	m_dc.SetTextColor(clrFore);
	m_dc.SetBkColor(clrBack);
}

//-------------------------------------------------------------------------
// 再描画したい範囲を Invalidate する。
//-------------------------------------------------------------------------

void CPadView::ReDraw( ReDrawType r, const DPos* s )
{
	// まずスクロールバーを更新
	UpdateScrollBar();

	switch (r) {
	case ALL: // 全画面
		Invalidate(FALSE);
		break;

	case LNAREA: // 行番号表示域のみ
		if (lna() > 0) {
			CRect rc(0, 0, lna(), bottom());
			InvalidateRect(&rc, FALSE);
		}
		break;
	case LINE: // 指定した行の後半
	case AFTER: // 指定した行以下全部
		{
			DPos st = ( s->ad==0 ? *s : m_doc->leftOf(*s,true) );
			InvalidateView( st, r==AFTER );
		}
	}
}


//-------------------------------------------------------------------------
// 描画処理
//-------------------------------------------------------------------------

void CPadView::OnDraw(const PAINTSTRUCT& ps)
{
	// 描画範囲の情報を詳しく取得

	VDrawInfo v( ps.rcPaint );
	GetDrawPosInfo( v );

	CPainter* painter = m_canvas->GetPainter();
	ATLASSERT(painter != NULL);
	DrawTXT(v);
}



//-------------------------------------------------------------------------
// テキスト描画
//-------------------------------------------------------------------------

inline void CPadView::Invert( int y, int xBegin, int xEnd)
{
	CPainter* painter = m_canvas->GetPainter();
	ATLASSERT(painter != NULL);

	CRect rc(
		Max( left(),  xBegin ), y,
		Min( right(), xEnd ), y + painter->H()
	);

//@	painter->Highlight(rc);
	painter->Invert(rc);
}

void CPadView::DrawTXT( const VDrawInfo vdi)
{
	CPainter* painter = m_canvas->GetPainter();
	ATLASSERT(painter != NULL);

	// 行番号境界線のペン
	LOGBRUSH lb = { BS_SOLID, painter->GetColor(CLR_NUM), 0 };
	HPEN hpen = painter->SelectPen(ExtCreatePen(PS_COSMETIC | PS_ALTERNATE, 1, &lb, 0, NULL));

	// 定数１
	const int yChar = painter->H();
	const ULONG TLM = m_doc->tln() - 1;
	const int yPad = painter->LM() >> 1;		// 行間マージン

	// 作業用変数１
	CRect rc(0, vdi.yMin, 0, vdi.yMin + painter->H());
	int clrIndex = -1;
	register int x = 0, x2;
	register ULONG i = 0, i2;

	// 論理行単位のLoop
	for (ULONG tl = vdi.tlMin; rc.top < vdi.yMax; tl++) {
		// 定数２
		const WCHAR* str = m_doc->tl(tl);
		const BYTE* flg_ptr = m_doc->pl(tl);
		const BYTE* ktp_ptr = m_doc->kl(tl);
		const int rYMAX = Min<int>( vdi.yMax, rc.top + rln(tl) * yChar);

		// 作業用変数２
		ULONG stt = 0, end, t, n;

		// 表示行単位のLoop
		bool bCurLine = false;
		for (ULONG rl = 0; rc.top < rYMAX; rl++, rc.OffsetRect(0, yChar), stt = end) {
			// 作業用変数３
			end = rlend(tl, rl);
			if (rc.bottom <= vdi.yMin)
				continue;

			// 背景塗りつぶし：選択中でなくかつ現在行か否か
			bCurLine = !m_cursor->isSelected() && (tl2vl(tl) + rl == m_cursor->GetCurPos().vl);
			rc.left   = 0;
			rc.right  = vdi.xBase + vdi.xMax;
#ifndef CURSOR_UNDERLINE
			painter->Fill(rc, bCurLine ? CLR_CUR : CLR_BG);
#else
			painter->Fill(rc, CLR_BG);
      if (bCurLine) {
        CRect rc2(rc);
        rc2.top = rc.bottom - 1;
        rc2.right = vdi.xBase + vdi.xMax;
				painter->Fill(rc2, CLR_CUR);
			}
#endif
			// 行番号表示
			if (vdi.rc.left < lna()) {
				// 境界線表示
				painter->DrawLine(lna() - painter->F() / 2, rc.top, lna() - painter->F() / 2, rc.bottom);

				if (rl == 0) {
					rc.left = 0;
					rc.right = lna() - painter->F();
					CString str; str.Format(_T("%d"), tl + 1);
					clrIndex = CLR_NUM;
					painter->SetTextColor(clrIndex);
					painter->DrawString(str, str.GetLength(), &rc, DT_VCENTER | DT_RIGHT | DT_SINGLELINE | DT_NOCLIP);
				}
			}

			// 不要な描画はスキップする
			if (vdi.rc.right <= lna())
				continue;

			for (x2 = x = 0, i2 = i = stt; x <= vdi.xMax && i < end; x = x2, i = i2) {
				// n := 次のTokenの頭
				t = (flg_ptr[i] >> 5);
				n = i + t;
				if (n >= end)
					n = end;
				else if (t == 7 || t == 0) {
					while ((flg_ptr[n] >> 5) == 0 && n < end)
						++n;
				}

				// x2, i2 := このTokenの右端
				i2++;
				x2 = (str[i] == L'\t' ? painter->nextTab(x2) : x2 + painter->W(str[i]));
				while (i2 < n && x2 <= vdi.xMax)
					x2 += painter->W(str[i2++]);

				// 再描画すべき範囲と重なっていない
				if (x2 <= vdi.xMin)
					continue;

				// x, i := このトークンの左端
				if (x < vdi.xMin) {
					// tabの分が戻りすぎ？
					x = x2, i = i2;
					while (vdi.xMin < x)
						x -= painter->W(str[--i]);
				}

				// 描画
				switch (str[i]) {
				case L'\t':
					// TAB
					if (painter->sc(scTAB)) {
						for (; i < i2; i++) {
#ifndef CURSOR_UNDERLINE
							painter->DrawTAB(x + vdi.xBase, rc.top + yPad, CLR_CTL, bCurLine ? CLR_CUR : CLR_BG);
#else
							painter->DrawTAB(x + vdi.xBase, rc.top + yPad, CLR_CTL, CLR_BG);
#endif
							x = painter->nextTab(x);
						}
					}
					break;
				case L' ':
					// 半角空白 L'　':
					if (painter->sc(scHSP)) {
#ifndef CURSOR_UNDERLINE
						painter->DrawHSP(x + vdi.xBase, rc.top + yPad, i2 - i, CLR_CTL, bCurLine ? CLR_CUR : CLR_BG);
#else
						painter->DrawHSP(x + vdi.xBase, rc.top + yPad, i2 - i, CLR_CTL, CLR_BG);
#endif
					}
					break;
				case 0x3000:
					// 全角空白 L'　':
					if (painter->sc(scZSP)) {
#ifndef CURSOR_UNDERLINE
						painter->DrawZSP(x + vdi.xBase, rc.top + yPad, i2 - i, CLR_CTL, bCurLine ? CLR_CUR : CLR_BG);
#else
						painter->DrawZSP(x + vdi.xBase, rc.top + yPad, i2 - i, CLR_CTL, CLR_BG);
#endif
					}
					break;
				default:
					{
					// 文字色
					int idx = CLR_TXT;
					if (flg_ptr[i] & 3) {
						if (flg_ptr[i] & 1)
							idx = CLR_CMT;
						else
							idx = ktp_ptr[i];
					}
					if (clrIndex != idx) {
						clrIndex = idx;
						painter->SetTextColor(clrIndex);
					}
// ----- 文字列一括で描画しない ------------
//					painter->StringOut(str + i, i2 - i, x + vdi.xBase, rc.top + yPad);
					for (; i < i2; i++) {
						painter->CharOut(str[i], x + vdi.xBase, rc.top + yPad);
						x += painter->W(str[i]);
					}
					break;
					}
				}
			}

			// 選択範囲だったら反転
			if (vdi.ySelBegin <= rc.top && rc.top <= vdi.ySelEnd) {
				Invert(rc.top,
					(rc.top == vdi.ySelBegin) ? vdi.xSelBegin : vdi.xBase,
					(rc.top == vdi.ySelEnd)   ? vdi.xSelEnd   : (vdi.xBase + x));
			}
		}

		// 行末記号描画反転
		SpecialChars sc = (tl == TLM ? scEOF : scEOL);
		if (i == m_doc->len(tl) && -32768 < x + vdi.xBase) {
			if (painter->sc(sc)) {
				clrIndex = CLR_EOL;
				painter->SetTextColor(clrIndex);
				if (sc == scEOL) {
					bool bInvert = (vdi.ySelBegin < rc.top && rc.top <= vdi.ySelEnd);
					int indexFore = bInvert ? CLR_CUR : CLR_EOL;
#ifndef CURSOR_UNDERLINE
					int indexBack = bInvert ? CLR_EOL : (bCurLine ? CLR_CUR : CLR_BG);
#else
					int indexBack = bInvert ? CLR_EOL : CLR_BG;
#endif
					painter->DrawEOL(x + vdi.xBase, rc.top - yChar, indexFore, indexBack);
				}
				else
					painter->StringOut(L"[EOF]", 5, x + vdi.xBase, rc.top - yChar + yPad);
			}
		}
	}

	// EOF後余白を背景色塗
	if (rc.top < vdi.rc.bottom) {
		rc.left   = vdi.rc.left;
		rc.right  = vdi.rc.right;
		rc.bottom = vdi.rc.bottom;
		painter->Fill(rc);
	}

	::DeleteObject(painter->SelectPen(hpen));
}
