#include "stdafx.h"
#include "ip_view.h"



//=========================================================================
//---- ip_scroll.cpp スクロール
//
//		ウインドウサイズはスクロールバーの位置によって
//		描画位置を適当に更新していく処理がここ。
//
//---- ip_text.cpp   文字列操作・他
//---- ip_parse.cpp  キーワード解析
//---- ip_wrap.cpp   折り返し
//---- ip_draw.cpp   描画・他
//---- ip_cursor.cpp カーソルコントロール
//=========================================================================



//-------------------------------------------------------------------------
// 描画領域サイズ管理
//-------------------------------------------------------------------------

CCanvas::CCanvas(const CPadView* view)
	: m_wrapType (WRAP_NONE)
	, m_bShowLineNumber(false)
	, m_wrapWidth(0xffffffff)
	, m_numFigColumns(4)
{
	VConfig vc(AtlGetStockFont(OEM_FIXED_FONT));
	m_painter = new CPainter(::GetDC(view->m_hWnd), vc);
	view->GetClientRect(&m_rcTextZone);
}

CCanvas::~CCanvas()
{
	delete m_painter;
}

bool CCanvas::CalcLNAreaWidth()
{
	const int prev = m_rcTextZone.left;
	if (m_bShowLineNumber) {
		m_rcTextZone.left  = (1 + m_numFigColumns) * m_painter->F();
		if (m_rcTextZone.left + m_painter->W() >= m_rcTextZone.right)
			m_rcTextZone.left = 0; // 行番号ゾーンがデカすぎるときは表示しない
	}
	else {
		m_rcTextZone.left = 0;
	}

	return (prev != m_rcTextZone.left);
}

void CCanvas::CalcWrapWidth()
{
	switch (m_wrapType) {
	case WRAP_NONE:
		m_wrapWidth = 0xffffffff;
		break;
	case WRAP_WINDOWEDGE:
		m_wrapWidth = m_rcTextZone.right - m_rcTextZone.left - 3;
		break; //Caretの分-3補正
	default:
		m_wrapWidth = m_wrapType * m_painter->W();
		break;
	}
}

bool CCanvas::ResizeView( int cx, int cy )
{
	m_rcTextZone.right  = cx;
	m_rcTextZone.bottom = cy;

	CalcLNAreaWidth();
	if (m_wrapType == WRAP_WINDOWEDGE) {
		CalcWrapWidth();
		return true;
	}
	return false;
}

void CCanvas::SetConfig( const VConfig& vc )
{
	m_bShowLineNumber = vc.m_sc[scNUM];			// 行番号
	m_wrapType = vc.m_wraptype;					// 折り返し方式

	HDC hdc = m_painter->GetDC();
	delete m_painter;
	m_painter = new CPainter(hdc, vc);

	CalcLNAreaWidth();
	CalcWrapWidth();
}

bool CCanvas::TotalLineNumberChange( ULONG tln )
{
	// 行番号桁数計算
	int col = 4;
	ULONG lim = 10000;
	while ((lim - 1) < tln && col < 10) {
		lim *= 10;
		col++;
	}
	m_numFigColumns = col;

	if (CalcLNAreaWidth()) {
		if (m_wrapType == WRAP_WINDOWEDGE)
			CalcWrapWidth();
		return true;
	}
	return false;
}



//-------------------------------------------------------------------------
// スクロールバー計算ルーチン
//-------------------------------------------------------------------------
// rl (横スクロール情報)
// max:  view.txt.txtwidth()
// page: view.cx()
// pos:  0～max-page

// ud (縦スクロール情報)
// max:   view.txt.vln() + page - 1
// page:  view.cy() / view.fnt.H()
// delta: 0～view.fnt.H()
// pos:   0～max-page (topの行番号)

bool CPadView::ReSetScrollInfo()
{
	const int prevRlPos = m_scrHorz.nPos;
	const ULONG cx = m_canvas->zone().Width();
	const ULONG cy = m_canvas->zone().bottom;

	// 横は変な値にならないよう補正するだけでよい
//	m_scrHorz.nPage = cx + 1;
//	m_scrHorz.nMax  = Max( textCx_, cx );
//	m_scrHorz.nPos  = Min<int>( m_scrHorz.nPos, m_scrHorz.nMax-m_scrHorz.nPage+1 );
	m_scrHorz.nPage = cx + 1;
	m_scrHorz.nMax  = Max( m_textCx + 3, cx );
	m_scrHorz.nPos  = Min<int>(m_scrHorz.nPos, m_scrHorz.nMax - m_scrHorz.nPage + 1);

	// 縦はnPageとnMaxはとりあえず補正
	// nPosは場合によって直し方が異なるので別ルーチンにて
	m_scrVert.nPage = cy / m_canvas->GetPainter()->H() + 1;
	m_scrVert.nMax  = vln() + m_scrVert.nPage - 2;

	// 横スクロールが起きたらtrue
	return (prevRlPos != m_scrHorz.nPos);
}

void CPadView::ForceScrollTo(ULONG tl)
{
	m_scrVert.nPos = tl2vl(tl);
	m_top_tl   = tl;
	m_top_vrl  = 0;
}

ULONG CPadView::tl2vl( ULONG tl ) const
{
	if (vln() == m_doc->tln())
		return tl;

	ULONG vl = 0;
	for (ULONG i = 0; i < tl; ++i)
		vl += rln(i);
	return vl;
}

ULONG CPadView::topvl()
{
	return tl2vl(m_top_tl);
}

void CPadView::UpdateScrollBar()
{
	::SetScrollInfo(m_hWnd, SB_HORZ, &m_scrHorz, TRUE );
	::SetScrollInfo(m_hWnd, SB_VERT, &m_scrVert, TRUE );
}

ReDrawType CPadView::TextUpdate_ScrollBar(const DPos& s, const DPos& e, const DPos& e2)
{
	const ULONG prevUdMax = m_scrVert.nMax;
	const bool rlScrolled = ReSetScrollInfo();
	const long vl_dif = (m_scrVert.nMax - prevUdMax);
	ReDrawType ans = (vl_dif != 0 || s.tl != e2.tl ? AFTER : LINE);

	if( m_top_tl < s.tl ) {
		// パターン１：現在の画面上端より下で更新された場合
		// スクロールしない
	}
	else if( m_top_tl == s.tl ) {
		// パターン２：現在の画面上端と同じ行で更新された場合
		// 出来るだけ同じ位置を表示し続けようと試みる。

		if( static_cast<ULONG>(m_scrVert.nPos) >= vln() ) {
			// パターン2-1：しかしそこはすでにEOFよりも下だ！
			// しゃーないので一番下の行を表示
			m_scrVert.nPos = vln() - 1;
			m_top_tl   = m_doc->tln() - 1;
			m_top_vrl  = rln(m_top_tl) - 1;
			ans = ALL;
		}
		else {
			// パターン2-2：
			// スクロール無し
			while (m_top_vrl >= rln(m_top_tl)) {
				m_top_vrl -= rln(m_top_tl);
				m_top_tl++;
			}
		}
	}
	else {
		// パターン３：現在の画面上端より上で更新された場合
		// 表示内容を変えないように頑張る

		if (e.tl < m_top_tl) {
			// パターン3-1：変更範囲の終端も、現在行より上の場合
			// 行番号は変わるが表示内容は変わらないで済む
			m_scrVert.nPos += vl_dif;
			m_top_tl   += (e2.tl - e.tl);
			ans = LNAREA;
		}
		else {
			// パターン3-2：
			// どうしよーもないので適当な位置にスクロール
			ForceScrollTo( e2.tl );
			ans = ALL;
		}
	}

	// どんな再描画をすればよいか返す
	return (rlScrolled ? ALL : ans);
}

void CPadView::ScrollTo(const VPos& vp, bool bKeepVertOffset)
{
	// 横フォーカス
	int dx = 0;
	if (vp.vx < (signed)m_scrHorz.nPos) {
		dx = vp.vx - m_scrHorz.nPos;
	}
	else {
		const int W = m_canvas->GetPainter()->W();
		if (m_scrHorz.nPos + (signed)(m_scrHorz.nPage - W) <= vp.vx)
			dx = vp.vx - (m_scrHorz.nPos + m_scrHorz.nPage) + W;
	}

	// 縦フォーカス
	int dy = 0;
	if (bKeepVertOffset) {
		VPos cur = m_cursor->GetCurPos();
		dy = vp.vl - cur.vl;
	}
	else {
		if (vp.vl <= (unsigned)m_scrVert.nPos)
			dy = vp.vl - m_scrVert.nPos - 1;
		else if (m_scrVert.nPos + (m_scrVert.nPage - 1) <= vp.vl)
			dy = vp.vl - (m_scrVert.nPos + m_scrVert.nPage) + 2;
	}

	// スクロール
	if (dy) UpDown(dy, dx == 0);
	if (dx) ScrollView(dx, 0, true);
}

void CPadView::GetDrawPosInfo(VDrawInfo& vdi) const
{
	const int yChar = m_canvas->GetPainter()->H();

	long most_under = (vln() - m_scrVert.nPos) * yChar;
	if (most_under <= vdi.rc.top) {
		vdi.yMin  = vdi.rc.top;
		vdi.yMax  = most_under;
	}
	else {
		int    y = -(signed)m_top_vrl;
		ULONG tl = m_top_tl;
		int  top = vdi.rc.top / yChar;
		while (y + (signed)rln(tl) <= top)
			y += rln( tl++ );

		// 縦座標
		vdi.yMin  = y * yChar;
		vdi.yMax  = Min(vdi.rc.bottom, most_under);
		vdi.tlMin = tl;

		// 横座標
		vdi.xBase = left() - m_scrHorz.nPos;
		vdi.xMin  = vdi.rc.left  - vdi.xBase;
		vdi.xMax  = vdi.rc.right - vdi.xBase;

		// 選択範囲
		vdi.xSelBegin = vdi.xSelEnd = vdi.ySelBegin = vdi.ySelEnd = 0x7fffffff;

		const VPos *bg, *ed;
		if (m_cursor->GetCurSelRangePos(&bg, &ed)) {
			vdi.xSelBegin = bg->vx - m_scrHorz.nPos + left();
			vdi.xSelEnd   = ed->vx - m_scrHorz.nPos + left();
			vdi.ySelBegin = (bg->vl - m_scrVert.nPos) * yChar;
			vdi.ySelEnd   = (ed->vl - m_scrVert.nPos) * yChar;
		}
	}
}

void CPadView::ScrollView(int dx, int dy, bool update)
{
	// スクロール開始通知
	m_cursor->ScrollBegin();

	const RECT* clip = (dy == 0 ? &m_canvas->zone() : NULL);
	const int H = m_canvas->GetPainter()->H();

	// スクロールバー更新
	if (dx) {
		// 範囲チェック
		if (m_scrHorz.nPos+dx < 0)
			dx = -m_scrHorz.nPos;
		else if (m_scrHorz.nMax - (signed)m_scrHorz.nPage < m_scrHorz.nPos + dx) 
			dx = m_scrHorz.nMax - m_scrHorz.nPage - m_scrHorz.nPos + 1;

		m_scrHorz.nPos += dx;
		SetScrollInfo(SB_HORZ, &m_scrHorz, TRUE);
		dx = -dx;
	}
	if (dy) {
		// 範囲チェック…は前処理で終わってる。

		m_scrVert.nPos += dy;
		SetScrollInfo(SB_VERT, &m_scrVert, TRUE );
		dy *= -H;
	}
	if (dx || dy) {
		if (-dx >= right() || dx >= right() || -dy >= bottom() || dy >= bottom()) {
			// 全画面再描画
			// ちょうど65536の倍数くらいスクロールしたときに、
			// ScrollWindowEx on Win9x だと再描画が変なのを回避。
			Invalidate(FALSE);
		}
		else {
			// 再描画の不要な領域をスクロール
			ScrollWindowEx(dx, dy, NULL, clip, NULL, NULL, SW_INVALIDATE);

			// 即時再描画？
			if (update) {
				// 縦スクロールは高速化したいので一工夫
				if (dy) {
					// 再描画の必要な領域を自分で計算
					CRect rc(0, 0, right(), bottom());
					if (dy < 0) rc.top  = rc.bottom + dy;
					else        rc.bottom = dy;

					// インテリマウスの中ボタンクリックによる
					// オートスクロール用カーソルの下の部分を先に描く
					// ２回に分けることで、小さな矩形部分二つで済むので高速
					ValidateRect(&rc);
					UpdateWindow();
					InvalidateRect(&rc, FALSE);
				}
				UpdateWindow();
			}
		}
	}

	// スクロール終了通知
	m_cursor->ScrollEnd();
}

void CPadView::OnHScroll(UINT code, UINT nPos, HWND hwndScrollBar)
{
	const CPainter* painter = m_canvas->GetPainter();
	ATLASSERT(painter != NULL);

	// 変化量を計算
	int dx;
	switch (code) {
	case SB_LINELEFT:		dx = -painter->W(); break;
	case SB_LINERIGHT:	dx = +painter->W(); break;
	case SB_PAGELEFT:		dx = -(cx() >> 1); break;
	case SB_PAGERIGHT:	dx = +(cx() >> 1); break;
	case SB_THUMBTRACK: {
		SCROLLINFO si = { sizeof(SCROLLINFO), SIF_TRACKPOS };
		GetScrollInfo(SB_HORZ, &si);
		dx = si.nTrackPos - m_scrHorz.nPos;
		break;
	}
	case SB_LEFT:				dx = -m_scrHorz.nPos; break;
	case SB_RIGHT:			dx = m_scrHorz.nMax+1-(signed)m_scrHorz.nPage-m_scrHorz.nPos; break;
	default:						return;
	}

	// スクロール
	ScrollView(dx, 0, code!=SB_THUMBTRACK);
}

void CPadView::OnVScroll(UINT code, UINT nPos, HWND hwndScrollBar)
{
	const CPainter* painter = m_canvas->GetPainter();
	ATLASSERT(painter != NULL);

	// 変化量を計算
	int dy;
	switch (code) {
	case SB_LINEUP:   dy= -1; break;
	case SB_LINEDOWN: dy= +1; break;
	case SB_PAGEUP:   dy= -(cy() / painter->H()); break;
	case SB_PAGEDOWN: dy= +(cy() / painter->H()); break;
	case SB_THUMBTRACK:
		{
			SCROLLINFO si = { sizeof(SCROLLINFO), SIF_TRACKPOS };
			GetScrollInfo(SB_VERT, &si);
			dy = si.nTrackPos - m_scrVert.nPos;
			break;
		}
	case SB_TOP:      dy = -m_scrVert.nPos; break;
	case SB_BOTTOM:   dy = m_scrVert.nMax+1-(signed)m_scrVert.nPage-m_scrVert.nPos; break;
	default:          return;
	}

	// スクロール
	UpDown( dy, code==SB_THUMBTRACK );
}

LRESULT CPadView::OnHandlerMouseWheel(UINT msg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	UINT nFlags = GET_KEYSTATE_WPARAM(wParam);
	short zDelta = GET_WHEEL_DELTA_WPARAM(wParam);
	CPoint pt(GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam));

	// スクロール
	UpDown( -zDelta / WHEEL_DELTA * 3, false );
	return 0;
}

void CPadView::UpDown(int dy, bool thumb)
{
	// １．m_scrVert.nPos + dy が正常範囲に収まるように補正
	if (m_scrVert.nPos + dy < 0)
		dy = -m_scrVert.nPos;
	else if (m_scrVert.nMax + 1 - (signed)m_scrVert.nPage < m_scrVert.nPos + dy)
		dy = m_scrVert.nMax + 1 - m_scrVert.nPage - m_scrVert.nPos;

	if (!dy) return;

	// ２－１．折り返し無しの場合は一気にジャンプ出来る
	if (!wrapexists()) {
		m_top_tl = m_scrVert.nPos + dy;
	}

	// ２－２．でなけりゃ、現在位置からの相対サーチ
	// ScrollBarを連続的にドラッグせず一度に一気に飛んだ場合は
	// 1行目や最終行からの相対サーチの方が有効な可能性があるが、
	// その場合は多少速度が遅くなっても描画が引っかかることはないのでＯＫ
	else {
		int rl = dy + m_top_vrl;
		ULONG tl = m_top_tl;

		if (dy < 0) {		// 上へ戻る場合
			// ジャンプ先論理行の行頭へDash!
			while (rl < 0)
				rl += rln(--tl);
		}
		else if (dy > 0) {	// 下へ進む場合
			// ジャンプ先論理行の行頭へDash!
			while (rl > 0)
				rl -= rln(tl++);
			if (rl < 0)
				rl += rln(--tl); //行き過ぎ修正
		}
		m_top_tl = tl;
		m_top_vrl= static_cast<ULONG>(rl);
	}

	// ４．画面をスクロール
	ScrollView(0, dy, !thumb);
}

void CPadView::InvalidateView( const DPos& dp, bool afterall )
{
	const int H = m_canvas->GetPainter()->H();

	// 表示域より上での更新
	if (dp.tl < m_top_tl) {
		if( afterall )
			Invalidate(FALSE);
		return;
	}

	// 開始y座標計算
	int r = 0, yb = -(signed)m_top_vrl;
	for (int t = m_top_tl, ybe = cy() / H; (unsigned)t < dp.tl; yb += rln(t++))
		if (yb >= ybe)
			return;

	for ( ; dp.ad>rlend(dp.tl,r); ++r,++yb );

	yb = H * Max( yb, -100 ); // 上にはみ出し過ぎないよう調整
	if (yb >= cy())
		return;

	// １行目を再描画
	int rb = (r == 0 ? 0 : rlend(dp.tl, r - 1));
	int xb = left() + Max(0UL, CalcLineWidth(m_doc->tl(dp.tl) + rb, dp.ad - rb) - m_scrHorz.nPos);
	if (xb < right()) {
		CRect rc(xb, yb, right(), yb + H);
		InvalidateRect(&rc, FALSE );
	}

	// 残り
	int ye;
	yb += H;
	if (afterall) {
		xb = 0;
		ye = cy();
	}
	else {
		xb =left();
		ye = Min(cy(), yb + (int)(H * (rln(dp.tl) - r - 1)));
	}

	if (yb < ye) {
		CRect rc(xb, yb, right(), ye);
		InvalidateRect(&rc, FALSE);
	}
}
