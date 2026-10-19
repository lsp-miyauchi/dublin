#include "stdafx.h"
#include "ip_view.h"
#include "dataobject.h"



//=========================================================================
//---- ip_cursor.cpp カーソルコントロール
//
//		カレットを表示したりIMEに適当に対応したり色々。
//		ところで疑問なのだが Caret って「カレット」と
//		読むのか「キャレット」と読むのか？
//
//---- ip_text.cpp   文字列操作・他
//---- ip_parse.cpp  キーワード解析
//---- ip_wrap.cpp   折り返し
//---- ip_scroll.cpp スクロール
//---- ip_draw.cpp   描画・他
//=========================================================================



//-------------------------------------------------------------------------
// Caret制御用ラッパー
//-------------------------------------------------------------------------

class CCaret
{
public:
	CCaret( HWND hwnd, bool bBlockMode = true )
		: m_hWnd( hwnd )
		, m_height(0)
		, m_width(0)
		, m_bBlockMode( bBlockMode )
		, m_bCreated(false)
		, m_bInsert(true)
		, m_bSelected(false)
		, m_ptCur(CPoint(0, 0))
	{
	}

	~CCaret()
	{
		Destroy();
	}

	void Show()
	{
		if (m_bCreated)
			::ShowCaret(m_hWnd);
	}

	void Hide()
	{
		if (m_bCreated)
			::HideCaret(m_hWnd);
	}

	void Destroy()
	{
		if (m_bCreated) {
			::DestroyCaret();
			m_bCreated = false;
			m_ptCur = CPoint(0, 0);
		}
	}

	void SetPos(int x, int y, int width, bool bSelected)
	{
		if (m_bCreated) {
			if ((width != m_width) || (m_bSelected != bSelected))
				Create(m_height, width, m_bInsert, bSelected, NULL);

			int h = 0;
			if (m_bBlockMode) {
				if (!m_bSelected && m_bInsert)
					h = m_height >> 1;
			}
			::SetCaretPos(x, y + h);
			ime().SetPos(m_hWnd, x, y);
			m_ptCur = CPoint(x, y);
		}
	}

	void SetBlockMode(bool bCaretIsBlock)
	{
		// キャレットのブロックモード／ラインモードを切り替える
		if (m_bBlockMode != bCaretIsBlock) {
			m_bBlockMode = bCaretIsBlock;
			if (m_bCreated) {
				::DestroyCaret();
				Create(m_height, m_width, m_bInsert, m_bSelected, NULL);
			}
		}
	}

	CPoint GetPos() const
	{
		return m_ptCur;
	}

	void Create(int H, int W, bool bInsert, bool bSelected, const LOGFONT* plf)
	{
		if (m_bCreated)
			::DestroyCaret();

		m_height = H;
		m_width = W;
		m_bCreated = true;
		m_bInsert = bInsert;
		m_bSelected = bSelected;

		if (m_bSelected)
			W = 1;
		else {
			if (m_bInsert) {
				if (m_bBlockMode)
					H /= 2;
				else
					W = 2;
			}
		}
		::CreateCaret(m_hWnd, NULL, W, H);
		if (plf)
			ime().SetFont(m_hWnd, *plf);
		Show();
	}

	bool isAlive()
	{
		return m_bCreated;
	}

	ATL::CWindow wnd()
	{
		return ATL::CWindow(m_hWnd);
	}

private:
	const HWND m_hWnd;
	int m_height;
	int m_width;
	bool m_bBlockMode:1;
	bool m_bCreated:1;
	bool m_bInsert:1;
	bool m_bSelected:1;
	CPoint m_ptCur;
};



//-------------------------------------------------------------------------
// カーソル初期化
//-------------------------------------------------------------------------

CPadCursor::CPadCursor( HWND wnd, CPadView* vw, CPadDocument* dc )
	: m_view( vw )
	, m_doc( dc )
	, m_pEventHandler( &m_defaultHandler )
	, m_bIns( true )
	, m_bRO    ( false )
	, m_idTimer(0)
	, m_bLineSelectMode( false )
	, m_bDragging(false)
{
	m_caret = new CCaret(wnd);

	// てきとーに情報初期化
	::SystemParametersInfo( SPI_GETKEYBOARDSPEED, 0, &m_keyRepTime, 0 );
	m_cur.tl = m_cur.ad = m_cur.vl = m_cur.rl = 0;
	m_cur.vx = m_cur.rx = 0; m_sel = m_cur;
}

CPadCursor::~CPadCursor()
{
	delete m_caret;
}

void CPadCursor::AddHandler( CCursorEventHandler* ev )
{
	m_pEventHandler = ev;
	if (ev != &m_defaultHandler) {
		m_pEventHandler->OnCursorEventMove(m_cur, m_sel);
		m_pEventHandler->OnCursorEventMode(CEM_INSERTMODE, m_bIns);
		m_pEventHandler->OnCursorEventMode(CEM_READONLYMODE, m_bRO);
	}
}

void CPadCursor::RemoveHandler( CCursorEventHandler* ev )
{
	if (ev == m_pEventHandler)
		m_pEventHandler = &m_defaultHandler;
}



//-------------------------------------------------------------------------
// ヘルパー関数群
//-------------------------------------------------------------------------

void CPadCursor::UpdateCaretPos()
{
	const CPainter* painter = m_view->painter();
	ATLASSERT(painter != NULL);

	// メンバ変数の値を元に、実際にCaretを動かす処理
	int x, y;
	m_view->GetOrigin(&x, &y);
	x += m_cur.vx;
	y += m_cur.vl * painter->H();

	// 行番号ゾーンにCaretがあっても困るので左に追いやる
	if (0 < x && x < m_view->left())
		x = -m_view->left();

	// 文字幅
	const WCHAR* str = m_doc->tl(m_cur.tl);
	WCHAR ch = str[m_cur.ad];
	int width = painter->W((ch >= 0x20) ? ch : L'x');

	// セット
	m_caret->SetPos(x, y, width, (m_cur != m_sel));
	// viewで現在行を記録
	m_view->SetCurrentLine(m_cur.vl);

	m_pEventHandler->OnCursorEventMove(m_cur, m_sel);
}

void CPadCursor::Redraw( const VPos& s, const VPos& e )
{
	int x, y; // 原点
	m_view->GetOrigin( &x, &y );

	const CPainter* painter = m_view->painter();
	ATLASSERT(painter != NULL);

	CPoint sp(x + s.vx, y + s.vl * painter->H());
	CPoint ep(x + e.vx, y + e.vl * painter->H());
	if (s > e) {
		// Swap
		sp.x ^= ep.x;
		ep.x ^= sp.x;
		sp.x ^= ep.x;
		sp.y ^= ep.y;
		ep.y ^= sp.y;
		sp.y ^= ep.y;
	}
	ep.x += 2;

	// 手抜き16bitチェック入り…
	const long LFT = m_view->left();
	const long RHT = m_view->right();
	const long TOP = 0;
	const int  BTM = m_view->bottom();

	if (sp.y == ep.y) {
		CRect rc( Max(LFT,sp.x), sp.y, Min(RHT,ep.x), sp.y + painter->H() );
		m_caret->wnd().InvalidateRect( &rc, FALSE );
	}
	else {
		CRect rc( Max(LFT,sp.x), Max(TOP,sp.y), RHT, Min<int>(BTM, sp.y + painter->H()) );
		m_caret->wnd().InvalidateRect( &rc, FALSE );
		CRect re( LFT, Max(TOP,ep.y), Min(RHT,ep.x), Min<int>(BTM, ep.y + painter->H()) );
		m_caret->wnd().InvalidateRect( &re, FALSE );
		CRect rd( LFT, Max(TOP,rc.bottom), RHT, Min<int>((long)BTM,re.top) );
		m_caret->wnd().InvalidateRect( &rd, FALSE );
	}
}

void CPadCursor::RedrawLine( const VPos& s )
{
	int x, y; // 原点
	m_view->GetOrigin( &x, &y );

	const CPainter* painter = m_view->painter();
	ATLASSERT(painter != NULL);

	CRect rc(0, y, m_view->right(), y + painter->H());
	rc.OffsetRect(0, s.vl * painter->H());
	m_caret->wnd().InvalidateRect( &rc, FALSE );
}

const VPos& CPadCursor::GetCurPos()
{
	return m_cur;
}

bool CPadCursor::GetCurSelRangePos( const VPos** start, const VPos** end ) const
{
	*start = *end = &m_cur;
	if (m_cur == m_sel)		//|| !m_caret->isAlive() )
		return false;
	if (m_cur < m_sel)
		*end = &m_sel;
	else
		*start = &m_sel;
	return true;
}

bool CPadCursor::isInSelRange( const VPos& s ) const
{
	// 指定の位置は選択範囲内かどうかを返す
	if (m_cur < m_sel)
		return (m_cur <= s && s <= m_sel);
	else if (m_cur > m_sel)
		return (m_sel <= s && s <= m_cur);
	return false;
}


//-------------------------------------------------------------------------
// Viewからの指令を処理
//-------------------------------------------------------------------------

void CPadCursor::SetFocus()
{
	const CPainter* painter = m_view->painter();
	ATLASSERT(painter !=NULL);

	m_caret->Create(painter->H(), painter->W(), m_bIns, (m_cur != m_sel), &painter->LogFont());
	UpdateCaretPos();
}

void CPadCursor::KillFocus()
{
	m_caret->Destroy();
	Redraw(m_cur, m_sel);
}

void CPadCursor::ScrollBegin()
{
	m_caret->Hide();
}

void CPadCursor::ScrollEnd()
{
	UpdateCaretPos();
	m_caret->Show();
}

void CPadCursor::ResetPos()
{
	// 設定変更などに対応
	m_view->ConvDPosToVPos( m_cur, &m_cur );
	m_view->ConvDPosToVPos( m_sel, &m_sel );
	UpdateCaretPos();
	if (m_caret->isAlive())
		m_view->ScrollTo(m_cur);
}

void CPadCursor::TextUpdate(const DPos& s, const DPos& e, const DPos& e2, bool mCur)
{
	VPos* search_base  = NULL;

	VPos old = m_cur;
	if (mCur && s == m_cur && e == m_sel) {
		search_base = &m_cur;
	}
	else if (mCur && s == m_sel && e == m_cur) {
		search_base = &m_sel;
	}
	else {
		Redraw( m_cur, m_sel );
		if (mCur && m_caret->isAlive()) {
			if( m_cur <= s )
				search_base = &m_cur;
		}
		else {
			if (s < m_cur) {
				if( m_cur <= e )
					m_cur = e2;
				else if( m_cur.tl == e.tl )
					m_cur.tl=e2.tl, m_cur.ad=e2.ad+m_cur.ad-e.ad;
				else
					m_cur.tl=e2.tl-e.tl;
				m_view->ConvDPosToVPos( m_cur, &m_cur );
			}
			if( s < m_sel )
				m_sel = m_cur;
		}
	}

	if (mCur) {
		m_view->ConvDPosToVPos( e2, &m_cur, search_base );
		m_sel = m_cur;
		if( m_caret->isAlive() )
			m_view->ScrollTo( m_cur );
	}
	UpdateCaretPos();

	if (old != m_cur) {
		// カーソル行再描画
		RedrawLine(old);
		RedrawLine(m_cur);
	}
}



//-------------------------------------------------------------------------
// キー入力への対応
//-------------------------------------------------------------------------

void CCursorEventHandler::OnCursorEventChar( CPadCursor* cur, WCHAR wch )
{
	cur->InputChar(wch);
}

void CCursorEventHandler::OnCursorEventIME( CPadCursor* cur, const WCHAR* str, ULONG len )
{
	cur->Input(str, len);
}

void CCursorEventHandler::OnCursorEventKey( CPadCursor* cur, int vk, bool sft, bool ctl )
{
	switch (vk) {
	case VK_HOME:	cur->Home(ctl, sft);	break;
	case VK_END:	cur->End(ctl, sft);		break;
	case VK_RIGHT:	cur->Right(ctl, sft);	break;
	case VK_LEFT:	cur->Left(ctl, sft);	break;
	case VK_UP:		cur->Up(ctl, sft);		break;
	case VK_DOWN:	cur->Down(ctl, sft);	break;
	case VK_PRIOR:	cur->PageUp(sft);		break;
	case VK_NEXT:	cur->PageDown(sft);		break;
	case VK_DELETE:	cur->Del();				break;
	case VK_BACK:	cur->DelBack();			break;
	case VK_INSERT: cur->SetInsMode(!cur->isInsMode()); break;
	}
}

void CPadCursor::Char(UINT nChar, UINT nRepCnt, UINT nFlags)
{
	if (!isROMode() && nChar != 0x7f && (nChar >= 0x20 || nChar == TEXT('\r') || nChar == TEXT('\t'))) {
		m_pEventHandler->OnCursorEventChar(this, nChar);
	}
}

void CPadCursor::ImeComposition( LPARAM lp )
{
	m_view->ScrollTo(m_cur);

	if (!isROMode() && (lp & GCS_RESULTSTR)) {
		CString str;
		int len = ime().GetString(m_caret->wnd(), str);
		if (len > 0)
			m_pEventHandler->OnCursorEventIME(this, str, len);
	}
}

void CPadCursor::KeyDown(UINT nChar, UINT nRepCnt, UINT nFlags)
{
	bool sft = (::GetKeyState(VK_SHIFT) < 0);
	bool ctl = (::GetKeyState(VK_CONTROL) < 0);
	m_pEventHandler->OnCursorEventKey(this, nChar, sft, ctl);
}



//-------------------------------------------------------------------------
// モード切替
//-------------------------------------------------------------------------

void CPadCursor::SetInsMode( bool bIns )
{
	m_bIns = bIns;
	SetFocus();
	m_pEventHandler->OnCursorEventMode(CEM_INSERTMODE, m_bIns);
}

void CPadCursor::SetROMode( bool bRO )
{
	m_bRO = bRO;
	SetFocus();
	m_pEventHandler->OnCursorEventMode(CEM_READONLYMODE, m_bRO);
}

//-------------------------------------------------------------------------
// 文字入力・削除
//-------------------------------------------------------------------------

void CPadCursor::InputChar(WCHAR ch)
{
	if (isROMode()) return;

	// 「上書モード ＆ 選択状態でない ＆ 行末でない」なら右一文字選択
	if (!m_bIns && m_cur == m_sel && m_doc->len(m_cur.tl) != m_cur.ad)
		Right(false, true);

	// 入力
	Input( &ch, 1 );
}

void CPadCursor::Input(const WCHAR* str, ULONG len)
{
	if (isROMode()) return;

	if (m_cur == m_sel)
		m_doc->Execute(CCmdInsert(m_cur, str, len));
	else
		m_doc->Execute(CCmdReplace(m_cur, m_sel, str, len));
}

void CPadCursor::Input( const char* str, ULONG len )
{
	if (isROMode()) return;

	WCHAR* ustr = new WCHAR[ len*4 ];
	len = ::MultiByteToWideChar( CP_ACP, 0, str, len, ustr, len*4 );
	Input( ustr, len );
	delete [] ustr;
}

void CPadCursor::DupLine()
{
	if (isROMode()) return;

	// 現在行を複製
	Home(false, false);

	DPos e = m_cur;
	if (e.tl < m_doc->tln() - 1) {
		e.tl++;
		e.ad = 0;
	}
	else
		e.ad = m_doc->len(e.tl);

	CString str;
	int len = m_doc->GetRangeLength(m_cur, e);
	LPTSTR p = str.GetBuffer((len + 1) * sizeof(TCHAR));
	m_doc->GetText(p, m_cur, e);
	str.ReleaseBuffer();

	m_doc->Execute(CCmdInsert(m_cur, str, len));
}

void CPadCursor::DelBack()
{
	if (isROMode()) return;

	// 選択状態なら BackSpace == Delete
	// でなければ、 BackSpace == Left + Delete (手抜き
	if (m_cur == m_sel) {
		if (m_cur.tl == 0 && m_cur.ad == 0)
			return;
		Left(false, false);
	}
	Del();
}

void CPadCursor::Del()
{
	if (isROMode()) return;

	// 選択状態なら m_cur ～ m_sel を削除
	// でなければ、 m_cur ～ rightOf(m_cur) を削除
	DPos dp = (m_cur == m_sel ? m_doc->rightOf(m_cur) : (DPos)m_sel);
	if (m_cur != dp)
		m_doc->Execute(CCmdDelete(m_cur, dp));
}

void CPadCursor::DelLine()
{
	if (isROMode()) return;

	// 現在行1行削除
	DPos s = m_cur;
	s.ad = 0;
	DPos e = m_cur;
	if (e.tl < m_doc->tln() - 1) {
		e.tl++;
		e.ad = 0;
	}
	else
		e.ad = m_doc->len(e.tl);

	m_doc->Execute(CCmdDelete(s, e));
}

void CPadCursor::DelLeft()
{
	if (isROMode()) return;

	// カーソルの左側削除
	if (m_cur.ad > 0) {
		DPos s = m_cur;
		s.ad = 0;
		DPos e = m_cur;
		m_doc->Execute(CCmdDelete(s, e));
	}
}

void CPadCursor::DelRight()
{
	if (isROMode()) return;

	// カーソルの右側削除
	if (m_cur.ad < m_doc->len(m_cur.tl)) {
		DPos s = m_cur;
		DPos e = m_cur;
		e.ad = m_doc->len(e.tl);
		m_doc->Execute(CCmdDelete(s, e));
	}
}

//-------------------------------------------------------------------------
// 文字・テキスト取得
//-------------------------------------------------------------------------

int CPadCursor::GetSelectedStr(CString& str)
{
	str.Empty();

	DPos s = m_cur, e = m_sel;
	if (m_cur > m_sel) {
		s = m_sel;
		e = m_cur;
	}

	// テキスト取得
	int len = m_doc->GetRangeLength(s, e);
	WCHAR* p = str.GetBuffer(len + 1);
	m_doc->GetText(p, s, e);
	str.ReleaseBuffer();
	return len;
}

WCHAR CPadCursor::GetCurrentChar()
{
	return m_doc->GetChar(m_cur);
}

int CPadCursor::GetCurrentWord(CString& str, bool bFullWord)
{
	WCHAR* p = str.GetBuffer(512);
	m_doc->GetWord(p, m_cur, bFullWord);
	str.ReleaseBuffer();
	return str.GetLength();
}

const WCHAR* CPadCursor::GetCurrentText()
{
	return m_doc->tl(m_cur.tl);
}

void CPadCursor::ResetSelection()
{
	if (m_cur != m_sel) {
		// 選択状態をクリア
		VPos vp = GetCurPos();
		MoveTo(vp, false, true);
	}
}

//-------------------------------------------------------------------------
// クリップボード処理
//-------------------------------------------------------------------------

void CPadCursor::Cut(bool bAppend)
{
	if (isROMode()) return;

	if (m_cur != m_sel) {
		// コピーして削除
		Copy(false, bAppend);
		Del();
	}
}

void CPadCursor::Copy(bool bFollow, bool bAppend)
{
	static TCHAR sszFollow[] = _T("> ");

	// bFollow = 引用付き
	// bAppned = 追加

	CClipboard clp(m_caret->wnd(), true);
	if (m_cur == m_sel || !clp.isOpened())
		return;

	// 既存のクリップボードデータ
	CString strBase;
	if (bAppend)
		clp.GetUnicodeText(strBase);
	clp.Empty();

	DPos s = m_cur, e = m_sel;
	if (m_cur > m_sel) {
		s = m_sel;
		e = m_cur;
	}

	LPCTSTR pszFollow = bFollow ? sszFollow : NULL;

	int len = m_doc->GetRangeLength(s, e, pszFollow);
	if (!strBase.IsEmpty())
		len += strBase.GetLength();

	WCHAR* p = (WCHAR*)malloc((len + 1) * sizeof(WCHAR));
	if (!strBase.IsEmpty()) {
		lstrcpyW(p, strBase);
		p += lstrlenW(p);
	}
	m_doc->GetText(p, s, e, pszFollow);
	clp.SetData(CF_UNICODETEXT, p);
}

bool CPadCursor::CanPaste()
{
	CClipboard clp(m_caret->wnd(), true);
	if (clp.isOpened()) {
		CSimpleArray<UINT> arr;
		arr.Add(CF_TEXT);
		arr.Add(CF_UNICODETEXT);
		return clp.IsAvail(arr.GetData(), arr.GetSize());
	}
	return false;
}

void CPadCursor::Paste()
{
	if (isROMode()) return;

	CClipboard clp(m_caret->wnd(), true);
	if (clp.isOpened()) {
		CString str;
		int len = clp.GetUnicodeText(str);
		if (len > 0)
			m_doc->Execute(CCmdReplace(m_cur, m_sel, (LPCTSTR)str, len));
	}
}

void CPadCursor::ReplaceSel(LPCTSTR pszReplace)
{
	if (isROMode()) return;

	if (pszReplace)
		m_doc->Execute(CCmdReplace(m_cur, m_sel, pszReplace, lstrlen(pszReplace)));
}

//-------------------------------------------------------------------------
// カーソル移動
//-------------------------------------------------------------------------

void CPadCursor::MoveCur(int ad, int line, bool select)
{
	VPos vp;
	m_view->ConvDPosToVPos(DPos(line, ad), &vp);
	MoveTo(vp, select);
}

void CPadCursor::MoveCur(const DPos& dp, bool select)
{
	VPos vp;
	m_view->ConvDPosToVPos( dp, &vp );
	MoveTo(vp, select);
}

void CPadCursor::MoveTo(const VPos& vp, bool sel, bool bKeepVertOffset)
{
	bool bSelected = (m_cur != m_sel);
	if (sel) {
		// 選択状態が変わる範囲を再描画
		if (!bSelected)
			RedrawLine(m_cur);
		Redraw(vp, m_cur);
	}
	else {
		// 選択解除される範囲を再描画
		if (bSelected) {
			Redraw(m_cur, m_sel);
			RedrawLine(vp);
		}
		m_sel = vp;
	}

	bool bLineChanged = (m_cur.vl != vp.vl);
	m_view->ScrollTo(vp, bKeepVertOffset);

	VPos old = m_cur;
	m_cur = vp;
	UpdateCaretPos();

	if (bLineChanged) {
		RedrawLine(old);
		RedrawLine(m_cur);
	}
}

void CPadCursor::LastEditPos()
{
	DPos dp = m_doc->lastEditOf();
	VPos vp;
	m_view->ConvDPosToVPos(dp, &vp);
	MoveTo(vp, false);
}

void CPadCursor::Home( bool wide, bool select )
{
	VPos np;
	np.ad = np.vx = np.rx = np.rl = 0;
	if( wide ) // 文書の頭へ
		np.tl = np.vl = 0;
	else // 行の頭へ
		np.tl = m_cur.tl, np.vl = m_cur.vl-m_cur.rl;
	MoveTo( np, select );
}

void CPadCursor::End(bool wide, bool select)
{
	VPos np;
	if (wide) {	// 文書の末尾へ
		np.tl = m_doc->tln() - 1;
		np.vl = m_view->vln() - 1;
	}
	else {		// 行の末尾へ
		np.tl = m_cur.tl;
		np.vl = m_cur.vl + m_view->rln(np.tl) - 1 - m_cur.rl;
	}
	np.ad = m_doc->len(np.tl);
	np.rl = m_view->rln(np.tl)-1;
	np.rx = np.vx = m_view->GetLastWidth( np.tl );

	MoveTo( np, select );
}

VPos CPadCursor::CalcUpDown(int dy) const
{
	const CPainter* painter = m_view->painter();
	ATLASSERT(painter != NULL);

	// はみ出す場合は、先頭行/終端行で止まるように制限
	VPos np = m_cur;
	if ((signed)np.vl + dy < 0)
		dy = -(signed)np.vl;
	else if (np.vl + dy >= m_view->vln())
		dy = m_view->vln() - np.vl - 1;

	np.vl += dy;
	np.rl += dy;
	if (dy < 0) {		// 上へ戻る場合
		// ジャンプ先論理行の行頭へDash!
		while ((signed)np.rl < 0)
			np.rl += m_view->rln(--np.tl);
	}
	else if (dy > 0) {	// 下へ進む場合
		// ジャンプ先論理行の行頭へDash!
		while ((signed)np.rl > 0)
			np.rl -= m_view->rln(np.tl++);
		if ((signed)np.rl < 0)
			np.rl += m_view->rln(--np.tl); //行き過ぎ修正～
	}

	// x座標決定にかかる
	const WCHAR* str = m_doc->tl(np.tl);

	// 右寄せになってる。不自然？
	np.ad = (np.rl == 0 ? 0 : m_view->rlend(np.tl, np.rl - 1) + 1);
	np.vx = (np.rl == 0 ? 0 : painter->W(str[np.ad - 1]));
	while (np.vx < np.rx && np.ad < m_view->rlend(np.tl, np.rl)) {
		// 左寄せにしてみた。
		ULONG newvx;
		if (str[np.ad] == L'\t')
			newvx = painter->nextTab(np.vx);
		else
			newvx = np.vx + painter->W(str[np.ad]);

		if (newvx > ULONG(np.rx))
			break;

		np.vx = newvx;
		++np.ad;
	}

	return np;
}

void CPadCursor::Up(bool wide, bool select)
{
	int dy = wide ? -3 : -1;
	if (!wide && m_cur.vl < m_view->topvl() + 2)
		dy = -2;
	VPos np = CalcUpDown(dy);
	MoveTo(np, select);
}

void CPadCursor::Down(bool wide, bool select)
{
	int dy = wide ? 3 : 1;
	if (!wide && m_cur.vl > m_view->topvl() + (m_view->cy() / m_view->painter()->H()) - 3)
		dy = 2;
	VPos np = CalcUpDown(dy);
	MoveTo(np, select);
}

void CPadCursor::PageUp( bool select )
{
	bool bKeepOffset = (m_cur.vl > m_view->topvl() + 2) && (m_cur.vl < m_view->topvl() + (m_view->cy() / m_view->painter()->H()) - 3);
	VPos np = CalcUpDown(-m_view->cy() / m_view->painter()->H());
	MoveTo(np, select, bKeepOffset);
}

void CPadCursor::PageHalfUp( bool select )
{
	VPos np = CalcUpDown((-m_view->cy() / m_view->painter()->H()) / 2);
	MoveTo(np, select, true);
}

void CPadCursor::PageDown( bool select )
{
	bool bKeepOffset = (m_cur.vl > m_view->topvl() + 2) && (m_cur.vl < m_view->topvl() + (m_view->cy() / m_view->painter()->H()) - 3);
	VPos np = CalcUpDown(m_view->cy() / m_view->painter()->H());
	MoveTo(np, select, bKeepOffset);
}

void CPadCursor::PageHalfDown( bool select )
{
	VPos np = CalcUpDown((m_view->cy() / m_view->painter()->H()) / 2);
	MoveTo(np, select, true);
}

void CPadCursor::Left(bool wide, bool select)
{
	VPos vp;
#ifdef NOTEPAD_LIKE
	if (m_cur != m_sel && !select) {
		vp = Min(m_cur, m_sel);
		vp.rx = vp.vx;
	}
	else
#endif
	{
		DPos dp = m_doc->leftOf(m_cur, wide);
		m_view->ConvDPosToVPos(dp, &vp, &m_cur);
	}
	MoveTo(vp, select);
}

void CPadCursor::Right( bool wide, bool select )
{
	VPos vp;
#ifdef NOTEPAD_LIKE
	if (m_cur != m_sel && !select) {
		vp = Max(m_cur, m_sel);
		vp.rx = vp.vx;
	}
	else
#endif
	{
		DPos dp = m_doc->rightOf(m_cur, wide);
		m_view->ConvDPosToVPos(dp, &vp, &m_cur);
	}
	MoveTo(vp, select);
}



//-------------------------------------------------------------------------
// マウス入力への対応
//-------------------------------------------------------------------------

// http url の正規表現（簡易版）
static const TCHAR sszHttpURL[] = _T("s?https?://[-_.!~*'()a-zA-Z0-9;/?:@&=+$,%#]+");

// メールアドレス検索の正規表現（簡易版）
static const TCHAR sszMailAddress[] = _T("\\a+[a-zA-Z0-9_\\-\\.]+\\@[a-zA-Z0-9_\\-\\.]+\\.\\w+");


/* @@@ 関数検索の正規表現 ---------------------------------

^[_a-zA-Z][^\(=;:]* [_a-zA-Z][_a-zA-Z0-9]*\([^;]*$
^[_a-zA-Z][^\(=;:]* operator[^\(;]+\([^;]*$
^[_a-zA-Z][^\(=;:]*::[^\(;]+\([^;]*$
^[_a-zA-Z][_a-zA-Z0-9]*[a-z]+[_a-zA-Z0-9]* *\([^;]*$

---------------------------------------------------- @@@ */

void CPadCursor::LButtonDblClk(long x, long y)
{
	// 行番号ゾーンの場合は特に何もしない
	if (m_view->lna() - m_view->painter()->F() < x) {
		// 行末の場合も特に何もしない
		ULONG len = m_doc->len(m_cur.tl);
		if (m_cur.ad != len) {
			// 行をコピー
			CString str;
			TCHAR* p = str.GetBuffer(len + 1);
			memcpy(p, m_doc->tl(m_cur.tl), len * sizeof(WCHAR));
			p[len] = 0;
			str.ReleaseBuffer();

			CAtlRegExp<> jre;
			CAtlREMatchContext<> mc;

			// url チェック
			jre.Parse(sszHttpURL);
			BOOL bMatch = jre.Match(str, &mc);
			if (!bMatch) {
				// メールアドレス チェック
				jre.Parse(sszMailAddress);
				bMatch = jre.Match(str, &mc);
			}

			if (bMatch) {
				DPos s = m_cur, e = m_cur;
				s.ad = (ULONG)(mc.m_Match.szStart - (LPCTSTR)str);
				e.ad = (ULONG)(mc.m_Match.szEnd - (LPCTSTR)str);
				if (s.ad <= m_cur.ad && m_cur.ad <= e.ad) {
					VPos vs, ve;
					m_view->ConvDPosToVPos(s, &vs, &m_cur);
					m_view->ConvDPosToVPos(e, &ve, &m_cur);
					MoveTo(vs, false);
					MoveTo(ve, true);
				}
				else
					bMatch = FALSE;
			}

			if (!bMatch) {
				VPos np;
				m_view->ConvDPosToVPos(m_doc->wordStartOf(m_cur), &np, &m_cur );
				MoveTo( np, false );
				Right( true, true );
			}

			// ドラッグ開始位置を保存
			m_view->ConvDPosToVPos(m_sel, &m_dragS, &m_sel);
			m_view->ConvDPosToVPos(m_cur, &m_dragE, &m_sel);

			// マウス位置の追跡開始
			m_caret->wnd().SetCapture();
			m_idTimer = m_caret->wnd().SetTimer( 178116, m_keyRepTime, NULL);
		}
	}
}

void CPadCursor::LButtonDown(long x, long y, bool shift)
{
	VPos c;
	m_view->GetVPos(x, y, &c);

	if (!shift && CanCopy()) {
		// 選択範囲の中をクリックした？
		VPos s = m_sel;
		VPos e = m_cur;
		if (m_cur < m_sel) {
			s = m_cur;
			e = m_sel;
		}

		if (s <= c && c <= e) {
			// 選択範囲内の場合
			CPoint ptScr(x, y);
			m_view->ClientToScreen(&ptScr);
			CString str;
			GetSelectedStr(str);
			if (lccBeginTrack(m_view->m_hWnd, ptScr)) {
				m_caret->Hide();

				m_bDragging = true;
				CDataObject* pdo = new CDataObject(str);
				CDropSource* pds = new CDropSource();
				DWORD sup = DROPEFFECT_COPY;
				if (!m_bRO)
					sup |= DROPEFFECT_MOVE;
				DWORD eff = 0;
				if (DoDragDrop(pdo, pds, sup, &eff) == DRAGDROP_S_DROP) {
					if (eff == DROPEFFECT_MOVE)
						Cut();
				}
				pds->Release();
				pdo->Release();
				m_bDragging = false;

				m_caret->Show();
			}
			else {
				// これまでの選択範囲をクリア
				Redraw(m_cur, m_sel);
				// マウスがクリックされた位置にキャレットを移動
				MoveTo(c, false);
			}
			return;
		}
	}

	if (!shift) {
		// これまでの選択範囲をクリア
		Redraw(m_cur, m_sel);

		// 行番号ゾーンのクリックだったら、行選択モードに
		m_bLineSelectMode = (x < (m_view->lna() - m_view->painter()->F()));

		// 選択開始位置を調整
		m_view->GetVPos(x, y, &m_sel);
		if (m_bLineSelectMode)
			m_view->ConvDPosToVPos(DPos(m_sel.tl, 0), &m_sel, &m_sel);
	}
	MoveTo(c, shift);

	// ドラッグ開始位置を保存
	m_view->ConvDPosToVPos(m_cur, &m_dragS, &m_sel);
	m_dragE = m_dragS;

	// マウス位置の追跡開始
	m_caret->wnd().SetCapture();
	m_idTimer = m_caret->wnd().SetTimer( 178116, m_keyRepTime, NULL );
}

void CPadCursor::LButtonUp(long x, long y)
{
	// 追跡解除
	if (m_idTimer != 0) {
		m_caret->wnd().KillTimer(m_idTimer);
		m_idTimer = 0;
		::ReleaseCapture();
	}
}

void CPadCursor::MouseMove(long x, long y)
{
	if (m_idTimer != 0) {
		// View内部ならMouseMoveに反応
		CPoint pt(x, y);
		if (m_view->zone().PtInRect(pt))
			MoveByMouse(pt);
	}
}

void CPadCursor::Timer()
{
	// View外部ならTimerに反応
	CPoint pt;
	GetCursorPos(&pt);
	m_view->ScreenToClient(&pt);
	if (!m_view->zone().PtInRect(pt))
		MoveByMouse(pt);
}

void CPadCursor::MoveByMouse(const CPoint& pt)
{
	VPos vp;
	m_view->GetVPos(pt.x, pt.y, &vp, m_bLineSelectMode);

	if (m_dragS == m_dragE)
		MoveTo(vp, true);
	else {
		// ダブルクリックして単語選択し、引き続いてドラッグした場合の処理
		if (m_dragS > vp) {
			// 選択範囲より上
			m_view->ConvDPosToVPos(m_doc->leftOf(vp, true), &vp, &m_sel);	// ひと単語前まで選択
			m_sel = m_dragE;
			MoveTo(vp, true);
		}
		else if (m_dragE < vp) {
			// 選択範囲より下
			m_view->ConvDPosToVPos(m_doc->rightOf(vp, true), &vp, &m_sel);	// ひと単語後まで選択
			m_sel = m_dragS;
			MoveTo(vp, true);
		}
		else {
			// 選択範囲内
			if (m_sel != m_dragS || m_cur != m_dragE) {
				m_sel = m_dragS;
				MoveTo(m_dragE, true);
			}
		}
	}
}

//-------------------------------------------------------------------------
// 再変換
//-------------------------------------------------------------------------

int CPadCursor::ImeReconvertString(RECONVERTSTRING* rs)
{
	if (!isSelected() || m_cur.tl != m_sel.tl)
		return 0;

	CString str;
	const ULONG len = GetSelectedStr(str);
	if (rs != NULL) {
		rs->dwSize            = sizeof(RECONVERTSTRING) + len + 1;
		rs->dwVersion         = 0;
		rs->dwStrOffset       = sizeof(RECONVERTSTRING);
		rs->dwStrLen          = len;
		rs->dwCompStrOffset   = 0;
		rs->dwCompStrLen      = len;
		rs->dwTargetStrOffset = 0;
		rs->dwTargetStrLen    = len;
		memmove(((char*)rs) + rs->dwStrOffset, (LPCTSTR)str, (len + 1) * sizeof(WCHAR));

		if (m_sel < m_cur) {
			DPos rsel = m_sel;
			MoveCur(m_cur, false);
			MoveCur(rsel, true);
		}
	}
	return sizeof(RECONVERTSTRING) + len + 1;
}

bool CPadCursor::ImeConfirmReconvertString( RECONVERTSTRING* rs )
{
	return false;
}

