#include "stdafx.h"
#include "ip_view.h"



//=========================================================================
//---- ip_wrap.cpp   折り返し
//
//		Documentで文字列データが更新されるのを受けて
//		Viewでは折り返し位置情報を更新する。その処理がココ。
//
//---- ip_text.cpp   文字列操作・他
//---- ip_parse.cpp  キーワード解析
//---- ip_scroll.cpp スクロール
//---- ip_draw.cpp   描画・他
//---- ip_cursor.cpp カーソルコントロール
//=========================================================================


//-------------------------------------------------------------------------
// 状態変更への対応
//-------------------------------------------------------------------------

void CPadView::DoResize( bool wrapWidthChanged )
{
	// 折り返し位置再計算
	if( wrapWidthChanged )
	{
		ReWrapAll();
		UpdateTextCx();
	}

	// スクロール情報変更
	ReSetScrollInfo();
	if (wrapWidthChanged)
		ForceScrollTo(m_top_tl);

	// 再描画
	ReDraw( ALL );
	m_cursor->ResetPos();
}

void CPadView::DoConfigChange()
{
	// 折り返し位置再計算
	ReWrapAll();
	UpdateTextCx();

	// スクロール情報変更
	ReSetScrollInfo();
	ForceScrollTo(m_top_tl);

	// 再描画
	ReDraw( ALL );
	m_cursor->ResetPos();
}

void CPadView::OnDocEventTextUpdate(const DPos& s, const DPos& e, const DPos& e2, bool bAft, bool mCur)
{
	// まず、折り返し位置再計算

	// 置換範囲の先頭行を調整
	int r3 = 0, r2 = 1, r1 = ReWrapSingle( s );

	// 残りを調整
	if( s.tl != e.tl )
		r2 = DeleteMulti( s.tl+1,  e.tl );
	if( s.tl != e2.tl )
		r3 = InsertMulti( s.tl+1, e2.tl );

	// この変更で横幅が…
	// if( "長くなったなてはいない" AND "短くなっちゃった可能性あり" )
	//     横幅再計算();
	if( !(r1==2 || r3==1) && (r1==0 || r2==0) )
		UpdateTextCx();

	// スクロールバー修正
	ReDrawType t = TextUpdate_ScrollBar( s, e, e2 );
	bool doResize = false;

	// 行数に変化があって、行番号表示域の幅を変えなきゃならん時
	if (e.tl != e2.tl && m_canvas->TotalLineNumberChange(m_doc->tln())) {
		doResize = true;
	}
	else if (bAft && t != ALL) {
		t = AFTER;
	}

	// カーソル移動
	m_cursor->TextUpdate( s, e, e2, mCur );

	// 再描画
	if( doResize )
		DoResize( true );
	else {
		if ( e.tl != e2.tl ) // 行番号領域再描画の必要があるとき
			ReDraw( ALL );
		else
			ReDraw( t, &s );
	}
}



//-------------------------------------------------------------------------
// 折り返し位置計算補助ルーチン
//-------------------------------------------------------------------------

void CPadView::UpdateTextCx()
{
	if (m_canvas->wrapType() == WRAP_NONE) {
		// 折り返しなしなら、数えてみないと横幅はわからない
		ULONG cx=0;
		for (ULONG i = 0, ie = m_doc->tln(); i < ie; ++i )
			if( cx < m_wraplines[i].width() )
				cx = m_wraplines[i].width();
		m_textCx = cx;
	}
	else {
		// 折り返しありなら、横幅:=折り返し幅とする
		m_textCx = m_canvas->wrapWidth();
	}
}

ULONG CPadView::CalcLineWidth( const WCHAR* txt, ULONG len ) const
{
	// 行を折り返さずに書いたときの横幅を計算する
	// ほとんどの行が折り返し無しで表示されるテキストの場合、
	// この値を計算しておくことで、処理の高速化が可能。
	const CPainter* p = m_canvas->GetPainter();

	ULONG w=0;
	for( ULONG i=0; i<len; ++i )
		if( txt[i] == L'\t' )
			w = p->nextTab(w);
		else
			w += p->W( txt[i] );
	return w;
}

void CPadView::CalcEveryLineWidth()
{
	// 全ての行に対してCalcLineWidthを実行
	// …するだけ。
	for (ULONG i = 0, ie = m_doc->tln(); i < ie; ++i )
		m_wraplines[i].width() = CalcLineWidth( m_doc->tl(i), m_doc->len(i) );
}

void CPadView::ModifyWrapInfo(const WCHAR* txt, ULONG len, WLine& wl, ULONG stt)
{
	// 設定幅での折り返しを実行する。
	// 行の途中からの変更の場合、sttが開始addressを指している
	const CPainter* p = m_canvas->GetPainter();
	ATLASSERT(p != NULL);

	const ULONG ww = m_canvas->wrapWidth();

	while (stt < len) {
		ULONG i, w;
		for (w = 0,i = stt; i < len; ++i) {
			if( txt[i] == L'\t' )
				w = p->nextTab(w);
			else
				w += p->W( txt[i] );

			if( w>ww )
				break; // 幅が設定値を超えた所でおしまい
		}
		wl.Add( stt = (i==stt?i+1:i) );
	}
}

int CPadView::GetLastWidth( ULONG tl ) const
{
	if( rln(tl)==1 )
		return m_wraplines[tl][0];

	ULONG beg = rlend(tl,rln(tl)-2);
	return CalcLineWidth(m_doc->tl(tl) + beg, m_doc->len(tl) - beg);
}

void CPadView::ReWrapAll()
{
	// 折り返し幅に変更があった場合に、全ての行の
	// 折り返し位置情報を変更する。
	const ULONG ww = m_canvas->wrapWidth();

	ULONG vln=0;
	for (ULONG i = 0, ie = m_doc->tln(); i < ie; ++i) {
		WLine& wl = m_wraplines[i];
		wl.ForceSize(1);

		if (wl.width() < ww) {
			// 設定した折り返し幅より短い場合は一行で済む。
			wl.Add(m_doc->len(i));
			++vln;
		}
		else {
			// 複数行になる場合
			ModifyWrapInfo(m_doc->tl(i), m_doc->len(i), wl, 0);
			vln += wl.rln();
		}
	}
	m_vlNum = vln;
}



//-------------------------------------------------------------------------
// 折り返し位置計算メインルーチン
//-------------------------------------------------------------------------

int CPadView::ReWrapSingle( const DPos& s )
{
	// 指定した一行のみ折り返しを修正。
	//
	// 返値は
	//   2: "折り返しあり" or "この行が横に一番長くなった"
	//   1: "この行以外のどこかが最長"
	//   0: "さっきまでこの行は最長だったが短くなっちゃった"
	// で、上位ルーチンにm_TextCx修正の必要性を伝える。
	//
	// 昔は再描画範囲の計算のために、表示行数の変化を返していたが、
	// これは上位ルーチン側で vln() を比較すれば済むし、
	// むしろその方が効率的であるため廃止した。


	// 旧情報保存
	WLine& wl            = m_wraplines[s.tl];
	const ULONG oldVRNum = wl.rln();
	const ULONG oldWidth = wl.width();

	// 横幅更新
	wl.width() = CalcLineWidth(m_doc->tl(s.tl), m_doc->len(s.tl));

	if (wl.width() < m_canvas->wrapWidth()) {
		// 設定した折り返し幅より短い場合は一行で済む。
		wl[1] = m_doc->len(s.tl);
		wl.ForceSize( 2 );
	}
	else {
		// 複数行になる場合
		ULONG vr = 1, stt = 0;
		while (wl[vr] < s.ad) // while( vr行目は変更箇所より手前 )
			stt = wl[ vr++ ];  // stt = 次の行の行頭のアドレス

		// 変更箇所以降のみ修正
		wl.ForceSize( vr );
		ModifyWrapInfo(m_doc->tl(s.tl), m_doc->len(s.tl), wl, stt );
	}

	// 表示行の総数を修正
	m_vlNum += ( wl.rln() - oldVRNum );

	// 折り返しなしだと総横幅の更新が必要
	if (m_canvas->wrapType() == WRAP_NONE)
		if( m_textCx <= wl.width() )
		{
			m_textCx = wl.width();
			return 2;
		}
		else if( m_textCx == oldWidth )
		{
			return 0;
		}
		else
		{
			return 1;
		}
	return 2;
}

int CPadView::InsertMulti( ULONG ti_s, ULONG ti_e )
{
	// 指定した分だけ新しく行情報を追加。
	// ＆折り返し情報もきちんと計算
	//
	// 返値は
	//   1: "折り返しあり" or "この行が横に一番長くなった"
	//   0: "この行以外のどこかが最長"
	// 詳しくは ReWrapSingle() を見よ。

	ULONG dy=0, cx=0;
	for (ULONG i = ti_s; i <= ti_e; ++i) {
		WLine* pwl = new WLine;
		pwl->Add( CalcLineWidth(m_doc->tl(i), m_doc->len(i)));

		if (pwl->width() < m_canvas->wrapWidth()) {
			// 設定した折り返し幅より短い場合は一行で済む。
			pwl->Add(m_doc->len(i));
			dy++;
			if (cx < pwl->width())
				cx = pwl->width();
		}
		else {
			// 複数行になる場合
			ModifyWrapInfo(m_doc->tl(i), m_doc->len(i), *pwl, 0);
			dy += pwl->rln();
		}

		m_wraplines.InsertAt( i, pwl );
	}

	// 表示行の総数を修正
	m_vlNum += dy;

	// 折り返しなしだと総横幅の更新が必要
	if (m_canvas->wrapType() == WRAP_NONE) {
		if (m_textCx <= cx) {
			m_textCx = cx;
			return 1;
		}
		return 0;
	}
	return 1;
}

int CPadView::DeleteMulti( ULONG ti_s, ULONG ti_e )
{
	// 指定した範囲の行情報を削除
	//
	// 返値は
	//   1: "折り返しあり" or "この行以外のどこかが最長"
	//   0: "さっきまでこの行は最長だったが短くなっちゃった"
	// 詳しくは ReWrapSingle() を見よ。

	bool  widthChanged = false;
	ULONG dy = 0;

	// 情報収集しながら削除
	for( ULONG cx=m_textCx, i=ti_s; i<=ti_e; ++i )
	{
		WLine& wl = m_wraplines[i];
		dy += wl.rln();
		if( cx == wl.width() )
			widthChanged = true;
	}
	m_wraplines.RemoveAt(ti_s, (ti_e - ti_s + 1));

	// 表示行の総数を修正
	m_vlNum -= dy;

	// 折り返しなしだと総横幅の更新が必要
	return ( m_canvas->wrapType() == WRAP_NONE && widthChanged ) ? 0 : 1;
}



//-------------------------------------------------------------------------
// 座標値変換
//-------------------------------------------------------------------------

void CPadView::ConvDPosToVPos(DPos dp, VPos* vp, const VPos* base) const
{
	// 補正
	dp.tl = Min(dp.tl, m_doc->tln() - 1);
	dp.ad = Min(dp.ad, m_doc->len(dp.tl));

	// 変換の基準点が指定されていなければ、原点を基準とする
	VPos topPos(false); // ０クリア
	if (base == NULL)
		base = &topPos;

	// とりあえずbase行頭の値を入れておく
	ULONG vl = base->vl - base->rl;
	ULONG rl = 0;
	int vx;

	// 同じ行内だった場合
	//if (dp.tl == base->tl) {
	//	例えば [→] を押したときなど、右隣の文字の横幅を
	//	足すだけで次の位置は算出できる。これを使って普通の
	//	カーソル移動はずっと高速化できるはずであるが、
	//	とりあえず面倒くさいので、今のところ略。
	//}

	// 違う行だった場合
	//else
	{
		// vlを合わせる
		ULONG tl = base->tl;
		if (tl > dp.tl) {
			// 目的地が基準より上にある場合
			do {
				vl -= rln(--tl);
			} while (tl > dp.tl);
		}
		else if (tl < dp.tl) {
			// 目的地が基準より下にある場合
			do {
				vl += rln(tl++);
			} while (tl < dp.tl);
		}
		// rlを合わせる
		ULONG stt = 0;
/* -------------------------------
// K.Miyauchi:
// 折り返しのある行の場合、物理行の終端ではない論理行終端へカーソルを進めた際に
// カーソル位置が現実的でない
   		while (m_wraplines[tl][rl + 1] < dp.ad) {
			stt = m_wraplines[tl][++rl];
		}
------------------------------- */
		int i, cnt = m_wraplines[tl].size();
		for (i = 1; i < cnt; i++) {
			ULONG n = m_wraplines[tl][i];
			if (i < cnt - 1) {
				// １行を折り返しした論理行の最終行ではない場合
				if (n > dp.ad)
					break;
			}
			else {
				// １行を折り返しした論理行の最終行の場合
				if (n >= dp.ad)
					break;
			}
			stt = n;
			rl++;
		}
// <----------- ここまで
		vl += rl;

		// x座標計算
		vx = CalcLineWidth(m_doc->tl(tl) + stt, dp.ad - stt);
	}

	vp->tl = dp.tl;
	vp->ad = dp.ad;
	vp->vl = vl;
	vp->rl = rl;
	vp->rx = vp->vx = vx;
}

void CPadView::GetLineRect(CRect& rc, ULONG vl)
{
	rc.left = left() - lna();
	rc.top = (vl - m_scrVert.nPos) * painter()->H();
	rc.right = right();
	rc.bottom = rc.top + painter()->H();
}

void CPadView::SetCurrentLine(ULONG vl)
{
	if (m_cur_vl != vl) {
//@		CPainter* painter = m_canvas->GetPainter();
//@		CRect rc;
//@		GetLineRect(rc, m_cur_vl);
//@		painter->Invert(rc);

		m_cur_vl = vl;

//@		GetLineRect(rc, m_cur_vl);
//@		painter->Invert(rc);
	}
}

void CPadView::GetVPos( int x, int y, VPos* vp, bool linemode ) const
{
// x座標補正

	x = x - lna() + m_scrHorz.nPos;

// まず行番号計算

	int tl = m_top_tl;
	int vl = m_scrVert.nPos - m_top_vrl;
	int rl = y / painter()->H() + m_top_vrl;
	if( rl >= 0 ) // View上端より下の場合、下方向を調べる
		while (tl < (int)m_doc->tln() && (int)rln(tl) <= rl) {
			vl += rln(tl);
			rl -= rln(tl);
			++tl;
		}
	else           // View上端より上の場合、上方向を調べる
		while (0 <= tl && rl < 0) {
			vl -= rln(tl);
			rl += rln(tl);
			--tl;
		}

	if (tl == (int)m_doc->tln()) {	// EOFより下に行ってしまう場合の補正
		--tl, vl-=rln(tl), rl=rln(tl)-1;
		if( linemode )
			x = 0x4fffffff;
	}
	else if (tl == -1) { // ファイル頭より上に行ってしまう場合の補正
		tl = vl = rl = 0;
		if( linemode )
			x = 0;
	}
	else {
		if (linemode) { // 行選択モードの場合
			if (tl == (int)m_doc->tln() - 1)
				rl = rln(tl) - 1, x = 0x4fffffff;
			else
				vl += rln(tl), rl = 0, ++tl, x = 0;
		}
	}

	vp->tl = tl;
	vp->vl = vl + rl;
	vp->rl = rl;

// 次に、横位置を計算

	const WCHAR* str = m_doc->tl(tl);
	const ULONG adend = rlend(tl,rl);
	ULONG ad = (rl==0 ? 0 : rlend(tl,rl-1));
	int   vx = (rl==0 ? 0 : painter()->W(str[ad++]));

	while( ad<adend ) {
		int nvx = (str[ad] == L'\t' ? painter()->nextTab(vx) : vx + painter()->W(str[ad]));
		if( x+2 < nvx )
			break;
		vx = nvx;
		++ad;
	}

	vp->ad          = ad;
	vp->rx = vp->vx = vx;
}

bool CPadView::FindPrev(CFind* pFinder)
{
	const VPos* ps;
	const VPos* pe;
	bool bSelected = m_cursor->GetCurSelRangePos(&ps, &pe);

	int y = ps->tl;
	int xad = ps->ad;

	if (y == 0 && xad == 0) return false;

	LPTSTR textptr = (LPTSTR)m_doc->tl(y);
	ULONG length = m_doc->len(y);
	TCHAR ch;
	int from, to;
	BOOL bFound;
	if (xad > 0) {
		ch = textptr[length];
		textptr[length] = 0;
		bFound = pFinder->RFind(textptr, textptr + xad, from, to);
		textptr[length] = ch;

		if (bFound && from < xad) {
			m_cursor->MoveCur(to,   y, false);
			m_cursor->MoveCur(from, y, true);
			return true;
		}
	}

	y--;
	while (y >= 0) {
		textptr = (LPTSTR)m_doc->tl(y);
		length = m_doc->len(y);

		ch = textptr[length];
		textptr[length] = 0;
		bFound = pFinder->RFind(textptr, textptr + length, from, to);
		textptr[length] = ch;

		if (bFound) {
			m_cursor->MoveCur(to,   y, false);
			m_cursor->MoveCur(from, y, true);
			return true;
		}
		y--;
	}
	return false;
}

bool CPadView::FindNext(CFind* pFinder)
{
	const VPos* ps;
	const VPos* pe;
	bool bSelected = m_cursor->GetCurSelRangePos(&ps, &pe);

	int y = pe->tl;
	int xad = pe->ad;

	if (y == m_doc->tln() && xad == m_doc->len(y)) return false;

	LPTSTR textptr = (LPTSTR)m_doc->tl(y);
	ULONG length = m_doc->len(y);
	int from, to;
	TCHAR ch;
	BOOL bFound;
	if (xad < (int)length) {
		ch = textptr[length];
		textptr[length] = 0;
		bFound = pFinder->Find(textptr + xad, textptr + length, from, to);
		textptr[length] = ch;

		if (bFound) {
			m_cursor->MoveCur(xad + from, y, false);
			m_cursor->MoveCur(xad + to,   y, true);
			return true;
		}
	}

	y++;
	while (y < (int)m_doc->tln()) {
		textptr = (LPTSTR)m_doc->tl(y);
		length = m_doc->len(y);

		ch = textptr[length];
		textptr[length] = 0;
		bFound = pFinder->Find(textptr, textptr + length, from, to);
		textptr[length] = ch;

		if (bFound) {
			m_cursor->MoveCur(from, y, false);
			m_cursor->MoveCur(to,   y, true);
			return true;
		}
		y++;
	}
	return false;
}

int CPadView::FindAll(CSimpleMap<ULONG, CString>& array, CSimpleArray<CString>& finder)
{
	if (0 == m_doc->tln() && 0 == m_doc->len(0)) return 0;

	int numFinder = finder.GetSize();
	CSimpleArray<CAtlRegExp<>*> jre;
	int i;
	for (i = 0; i < numFinder; i++) {
		CAtlRegExp<>* re = new CAtlRegExp<>;
		if (REPARSE_ERROR_OK != re->Parse(finder[i])) {
			delete re;
			re = NULL;
		}
		jre.Add(re);
	}

	CAtlREMatchContext<> mc;
	LPTSTR textptr = NULL;
	ULONG length, maxLength = 0;
	ULONG y = 0;
	while (y < m_doc->tln()) {
		length = m_doc->len(y);
		if (length > 0) {
			if (maxLength < length) {
				textptr = (TCHAR*)realloc(textptr, (length + 2) * sizeof(TCHAR));
				maxLength = length;
			}
			memcpy(textptr, m_doc->tl(y), length * sizeof(TCHAR));
			textptr[length] = 0;

			for (i = 0; i < numFinder; i++) {
				if (jre[i] && jre[i]->Match(textptr, &mc)) {
					*((TCHAR*)mc.m_Match.szEnd) = 0;
					array.Add(y, mc.m_Match.szStart);
					break;
				}
			}
		}
		y++;
	}

	if (textptr) free(textptr);

	for (i = 0; i < numFinder; i++) {
		if (jre[i])
			delete jre[i];
	}

	return array.GetSize();
}
