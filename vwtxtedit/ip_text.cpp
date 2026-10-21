#include "stdafx.h"
#include "ip_doc.h"



//=========================================================================
//---- ip_text.cpp   文字列操作・他
//
//		文字列を挿入したり削除したり…という辺りの処理が
//		このファイルにまとめてある。外向けのインターフェイスの
//		実装もついでにここで。
//
//---- ip_parse.cpp  キーワード解析
//---- ip_wrap.cpp   折り返し
//---- ip_scroll.cpp スクロール
//---- ip_draw.cpp   描画・他
//---- ip_cursor.cpp カーソルコントロール
//=========================================================================



//-------------------------------------------------------------------------
// イベントハンドラ処理
//-------------------------------------------------------------------------

void CPadDocument::AddHandler( CDocumentEventHandler* eh ) 
{
	// ハンドラ追加
	m_arrDocEvHandler.Add( eh );
}

void CPadDocument::RemoveHandler( CDocumentEventHandler* eh )
{
	// 後ろから見て行って…
	const int last = m_arrDocEvHandler.size() - 1;

	// …見つけたら削除
	for( int i=last; i>=0; --i )
		if( m_arrDocEvHandler[i] == eh )
		{
			m_arrDocEvHandler[i] = m_arrDocEvHandler[last];
			m_arrDocEvHandler.ForceSize( last );
			break;
		}
}

void CPadDocument::Fire_TEXTUPDATE(const DPos& s, const DPos& e, const DPos& e2, bool reparsed, bool nmlcmd )
{
//@	AutoLock lk(this);

	// 最後に編集したところ
	m_dpLastEdit = s;

	// 全部にイベント通知
	for (ULONG i = 0, ie = m_arrDocEvHandler.size(); i < ie; i++)
		m_arrDocEvHandler[i]->OnDocEventTextUpdate(s, e, e2, reparsed, nmlcmd );
}

void CPadDocument::Fire_KEYWORDCHANGE()
{
//@	AutoLock lk(this);

	// 全部にイベント通知
	for( ULONG i=0, ie=m_arrDocEvHandler.size(); i<ie; ++i )
		m_arrDocEvHandler[i]->OnDocEventKeywordChanged();
}

void CPadDocument::Fire_MODIFYFLAGCHANGE()
{
//@	AutoLock lk(this);

	// 全部にイベント通知
	bool b = m_urdo.IsModified();
	for( ULONG i=0, ie=m_arrDocEvHandler.size(); i<ie; ++i )
		m_arrDocEvHandler[i]->OnDocumentEventDirtyFlagChanged( b );
}



//-------------------------------------------------------------------------
// UnDo,ReDo 処理
//-------------------------------------------------------------------------

CUnReDoChain::Node::Node( CPadCommand* c, Node* p, Node* n )
	: m_cmd ( c )
	, m_prev( p )
	, m_next( n )
{
}

CUnReDoChain::Node::Node()
{
	m_next = m_prev = this;
	m_cmd  = NULL;
}

CUnReDoChain::Node::~Node()
{
	if (m_cmd)
		delete m_cmd;
}

void CUnReDoChain::Node::ResetCommand( CPadCommand* cmd )
{
	delete m_cmd;
	m_cmd = cmd;
}

CUnReDoChain::CUnReDoChain()
	: m_lastOp  ( &m_headTail )
	, m_savedPos( &m_headTail )
	, m_num     ( 0 )
{
}

CUnReDoChain::~CUnReDoChain()
{
	Clear();
}

ULONG CUnReDoChain::Node::ChainDelete(Node*& m_savedPosref)
{
	if (m_cmd == nullptr)
		return 0;
	if (m_savedPosref == this)
		m_savedPosref = nullptr;

	Node* d = new Node(*this);
	ULONG res = 1 + m_next->ChainDelete(m_savedPosref);
	delete d;
	return res;
}

void CUnReDoChain::Clear()
{
	m_headTail.m_next->ChainDelete(m_savedPos);
	m_headTail.m_next = m_headTail.m_prev = m_lastOp  = m_savedPos = &m_headTail;
	m_num = 0;
}

inline void CUnReDoChain::Undo( CPadDocument* doc )
{
	m_lastOp->ResetCommand( (*m_lastOp->m_cmd)(doc) );
	m_lastOp = m_lastOp->m_prev;
}

inline void CUnReDoChain::Redo( CPadDocument* doc )
{
	m_lastOp = m_lastOp->m_next;
	m_lastOp->ResetCommand( (*m_lastOp->m_cmd)(doc) );
}

void CUnReDoChain::NewlyExec( const CPadCommand& cmd, CPadDocument* doc )
{
	CPadCommand* nCmd = cmd(doc);
	if (nCmd != NULL) {
		m_num   -= (m_lastOp->m_next->ChainDelete(m_savedPos) - 1);
		m_lastOp = m_lastOp->m_next = new Node(nCmd, m_lastOp,&m_headTail);
	}
}



bool CPadDocument::CanUndo() const { return m_urdo.CanUndo(); }

bool CPadDocument::CanRedo() const { return m_urdo.CanRedo(); }

bool CPadDocument::IsModified() const { return m_urdo.IsModified(); }

bool CPadDocument::IsEmpty() const { return ((tln() == 1) && (len(0) == 0)); }

void CPadDocument::ClearModifyFlag()
{
	bool b = m_urdo.IsModified();
	m_urdo.SavedHere();
	if( b != m_urdo.IsModified() )
		Fire_MODIFYFLAGCHANGE();
}

void CPadDocument::Undo()
{
	if (m_urdo.CanUndo()) {
		bool b = m_urdo.IsModified();
		m_urdo.Undo(this);
		if (b != m_urdo.IsModified())
			Fire_MODIFYFLAGCHANGE();
	}
}

void CPadDocument::Redo()
{
	if (m_urdo.CanRedo()) {
		bool b = m_urdo.IsModified();
		m_urdo.Redo(this);
		if (b != m_urdo.IsModified())
			Fire_MODIFYFLAGCHANGE();
	}
}

void CPadDocument::Execute(const CPadCommand& cmd)
{
	bool b = m_urdo.IsModified();
	m_urdo.NewlyExec(cmd, this);
	if (b != m_urdo.IsModified())
		Fire_MODIFYFLAGCHANGE();
}



//-------------------------------------------------------------------------
// カーソル移動ヘルパー
//-------------------------------------------------------------------------

DPos CPadDocument::leftOf( const DPos& dp, bool wide ) const
{
	if (dp.ad == 0) {
		// 行の先頭だが、ファイルの先頭ではない場合
		// 一つ前の行の行末へ
		if (dp.tl > 0)
			return DPos(dp.tl - 1, len(dp.tl - 1));
		return dp;
	}
	else if (!wide) {
		// 行の途中で、普通に１文字戻る場合
		return DPos(dp.tl, dp.ad - 1);
	}
	else {
		// 行の途中で、１単語分戻る場合
		const BYTE* flg_ptr = pl(dp.tl);
		ULONG s = dp.ad - 1;
		while ((flg_ptr[s] >> 5) == 0 && 0 <= s)
			--s;
		return DPos(dp.tl, s);
	}
}

DPos CPadDocument::rightOf( const DPos& dp, bool wide ) const
{
	if (dp.ad == len(dp.tl)) {
		// 行末だが、ファイルの終わりではない場合
		// 一つ後の行の先頭へ
		if (dp.tl < tln() - 1)
			return DPos(dp.tl + 1, 0);
		return dp;
	}
	else if (!wide) {
		// 行の途中で、普通に１文字進む場合
		return DPos(dp.tl, dp.ad + 1);
	}
	else {
		// 行の途中で、普通に１単語進む場合
		const BYTE* flg_ptr = pl(dp.tl);
		const ULONG e = len(dp.tl);
			  ULONG s = dp.ad;
		const ULONG t = (flg_ptr[s] >> 5);
		s += t;
		if (s >= e)
			s = e;
		else if (t == 7 || t == 0)
			while ((flg_ptr[s] >> 5) == 0 && s < e)
				++s;
		return DPos(dp.tl, s);
	}
}

DPos CPadDocument::wordStartOf( const DPos& dp ) const
{
	if (dp.ad == 0) {
		// 行の先頭
		return dp;
	}
	else {
		// 行の途中
		const BYTE* flg_ptr = pl(dp.tl);
		ULONG s = dp.ad;
		while ((flg_ptr[s] >> 5) == 0 && 0 <= s)
			--s;
		return DPos(dp.tl, s);
	}
}

DPos CPadDocument::lastEditOf() const
{
	return m_dpLastEdit;
}

//-------------------------------------------------------------------------
// 挿入・削除等の作業用関数群
//-------------------------------------------------------------------------

ULONG CPadDocument::GetRangeLength(const DPos& s, const DPos& e, LPCTSTR pszFollow)
{
	// とりあえず全部足す
	ULONG ans = 0, tl = s.tl, te = e.tl;
	for (; tl <= te; tl++)
		ans += len(tl);

	// 先頭行の分を引く
	ans -= s.ad;
	// 最終行の分を引く
	ans -= len(te) - e.ad;
	// 改行コード(CRLF)の分を加える
	ans += (e.tl - s.tl) * 2;
	// 引用符分を追加
	if (pszFollow && *pszFollow)
		ans += (e.tl - s.tl + 1) * lstrlen(pszFollow);

	return ans;
}

void CPadDocument::GetText(WCHAR* buf, const DPos& s, const DPos& e, LPCTSTR pszFollow)
{
	int len = (pszFollow && *pszFollow) ? lstrlen(pszFollow) : 0;
	if (len) {
		lstrcpy(buf, pszFollow);
		buf += len;
	}

	if (s.tl == e.tl) {
		// 一行だけの場合
		m_text[s.tl].GetAt(s.ad, e.ad - s.ad, buf);
		buf[e.ad - s.ad] = L'\0';
	}
	else {
		// 先頭行の後ろをコピー
		buf += m_text[s.tl].GetToTail(s.ad, buf);
		*buf++ = L'\r';
		*buf++ = L'\n';
		// 途中をコピー
		for (ULONG i = s.tl + 1; i < e.tl; i++) {
			if (len > 0) {
				lstrcpy(buf, pszFollow);
				buf += len;
			}
			buf += m_text[i].GetToTail( 0, buf );
			*buf++ = L'\r';
			*buf++ = L'\n';
		}
		// 終了行の先頭をコピー
		if (len > 0) {
			lstrcpy(buf, pszFollow);
			buf += len;
		}
		buf += m_text[e.tl].GetAt(0, e.ad, buf);
		*buf = L'\0';
	}
}

void CPadDocument::GetWord(WCHAR* buf, const DPos& dp, bool bFullWord)
{
	// 指定位置の単語を取得
	DPos s = dp;
	if (bFullWord == true)
		s = wordStartOf(dp);

	const BYTE* flg_ptr = pl(s.tl);
	ULONG l = len(s.tl);
	ULONG ad = s.ad + 1;		// 次の文字からチェックしていく
	while ((flg_ptr[ad] >> 5) == 0 && ad < l)
			ad++;
	GetText(buf, s, DPos(s.tl, ad), NULL);
}

const WCHAR* CPadDocument::tl(ULONG line_number) const
{
	// 指定行の文字列ポインタを取得
	return ((line_number < tln()) ? m_text[line_number].str() : NULL);
}

WCHAR CPadDocument::GetChar(const DPos& s)
{
	WCHAR ch = 0;
	if (s.tl < tln() && s.ad < len(s.tl))
		m_text[s.tl].GetAt(s.ad, 1, &ch);
	return ch;
}

void CPadDocument::CorrectPos( DPos& pos )
{
	// 正常範囲に収まるように修正
	pos.tl = Min( pos.tl, tln()-1 );
	pos.ad = Min( pos.ad, len(pos.tl) );
}

void CPadDocument::CorrectPos( DPos& s, DPos& e )
{
	// 必ずs<=eになるように修正
	if (s > e) {
		ULONG t;
		t=s.ad, s.ad=e.ad, e.ad=t;
		t=s.tl, s.tl=e.tl, e.tl=t;
	}
}

bool CPadDocument::DeletingOperation(DPos& s, DPos& e, WCHAR*& undobuf, ULONG& undosiz)
{
//@	AutoLock lk( this );

	// 位置補正
	CorrectPos( s );
	CorrectPos( e );
	CorrectPos( s, e );

	// 削除される量をカウント
	undosiz = GetRangeLength(s, e);

	// Undo操作用バッファ確保
	undobuf = new WCHAR[undosiz + 1];
	GetText(undobuf, s, e);

	// 削除る
	if (s.tl == e.tl) {
		// 一行内削除
		m_text[s.tl].RemoveAt(s.ad, e.ad - s.ad);
	}
	else {
		// 先頭行の後ろを削除
		m_text[s.tl].Truncate(s.ad);
		// 終了行の残り部分をくっつける
		m_text[s.tl].Append(tl(e.tl) + e.ad, len(e.tl) - e.ad);
		// いらん行を削除
		m_text.RemoveAt( s.tl+1, e.tl-s.tl );
	}

	// 再解析
	return ReParse( s.tl, s.tl );
}

bool CPadDocument::InsertingOperation( DPos& s, const WCHAR* str, ULONG len, DPos& e )
{
//@	AutoLock lk( this );

	// 位置補正
	CorrectPos( s );

	// よーい、どん！
	e.ad = s.ad;
	e.tl = s.tl;

	// 指定文字列を改行で切り分ける準備
	CUnicodeReader r(str, len);

	const WCHAR* lineStr;
	ULONG lineLen;

	// 一行目…
	lineLen = r.GetLine(&lineStr);
	m_text[e.tl].InsertAt(e.ad, lineStr, lineLen);
	e.ad += lineLen;

	if (!r.Eof()) {
		// 二行目～最終行
		do {
			lineLen = r.GetLine(&lineStr);
			m_text.InsertAt( ++e.tl, new CTextLine(lineStr, lineLen) );
		} while (!r.Eof());

		// 一行目の最後尾に残ってた文字列を最終行へ
		CTextLine& fl = m_text[s.tl];
		CTextLine& ed = m_text[e.tl];
		const ULONG ln = fl.size()-e.ad;
		if (ln) {
			ed.Append(fl.str() + e.ad, ln);
			fl.Truncate(e.ad);
		}

		// 終了位置記録
		e.ad = lineLen;
	}

	// 再解析
	return ReParse( s.tl, e.tl );
}

bool CPadDocument::AppendString(const WCHAR* str)
{
	// アンドゥを記録せずに文字列を末尾に追加する
	DPos dp(tln() - 1, 0xffffffff), e;
	bool bOK = InsertingOperation(dp, str, lstrlen(str), e);

	// イベント発火
	Fire_TEXTUPDATE(dp, dp, e, true, false);
	return bOK;
}

//-------------------------------------------------------------------------
// 挿入コマンド
//-------------------------------------------------------------------------

CCmdInsert::CCmdInsert( const DPos& s, const WCHAR* str, ULONG len, bool del )
	: m_stt( s )
	, m_buf( str )
	, m_len( len )
	, m_del( del )
{
}

CCmdInsert::~CCmdInsert()
{
	if (m_del)
		delete const_cast<WCHAR*>(m_buf);
}

CPadCommand* CCmdInsert::operator()( CPadDocument* d ) const
{
	// 挿入
	DPos s = m_stt, e;
	bool aa = d->InsertingOperation( s, m_buf, m_len, e );

	// イベント発火
	d->Fire_TEXTUPDATE( s, s, e, aa, true );

	// 逆操作オブジェクトを返す
	return new CCmdDelete( s, e );
}

//-------------------------------------------------------------------------
// 削除コマンド
//-------------------------------------------------------------------------

CCmdDelete::CCmdDelete(const DPos& s, const DPos& e)
	: m_stt(s)
	, m_end(e)
{
}

CPadCommand* CCmdDelete::operator()( CPadDocument* d ) const
{
	// 削除
	WCHAR* buf;
	ULONG    siz;
	DPos s = m_stt, e = m_end;
	bool aa = d->DeletingOperation( s, e, buf, siz );

	// イベント発火
	d->Fire_TEXTUPDATE( s, e, s, aa, true );

	// 逆操作オブジェクトを返す
	return new CCmdInsert(s, buf, siz, true);
}



//-------------------------------------------------------------------------
// 置換コマンド
//-------------------------------------------------------------------------

CCmdReplace::CCmdReplace(const DPos& s, const DPos& e, const WCHAR* str, ULONG len, bool del)
	: m_stt( s )
	, m_end( e )
	, m_buf( str )
	, m_len( len )
	, m_del( del )
{
}

CCmdReplace::~CCmdReplace()
{
	if (m_del)
		delete const_cast<WCHAR*>(m_buf);
}

CPadCommand* CCmdReplace::operator()( CPadDocument* d ) const
{
	// 削除
	WCHAR* buf;
	ULONG    siz;
	DPos s = m_stt, e = m_end;
	bool aa = d->DeletingOperation( s, e, buf, siz );

	// 挿入
	DPos e2;
	aa = (d->InsertingOperation( s, m_buf, m_len, e2 ) || aa);

	// イベント発火
	d->Fire_TEXTUPDATE( s, e, e2, aa, true );

	// 逆操作オブジェクトを返す
	return new CCmdReplace( s, e2, buf, siz, true );
}



//-------------------------------------------------------------------------
// マクロコマンド
//-------------------------------------------------------------------------

CPadCommand* CMacroCommand::operator()( CPadDocument* d ) const
{
	d->setBusyFlag();

	CMacroCommand* undo = new CMacroCommand;
	undo->m_arr.ForceSize( size() );
	for (ULONG i = 0, e = m_arr.size(); i < e; ++i)
		undo->m_arr[e - i - 1] = (*m_arr[i])(d);

	d->setBusyFlag(false);
	return undo;
}



//-------------------------------------------------------------------------
// ファイルに保存
//-------------------------------------------------------------------------

void CPadDocument::SaveFile( TextFileW* tf )
{
	m_urdo.SavedHere();
	ULONG numLines = tln() - 1;
	for (ULONG i = 0; i <= numLines; i++)
		tf->WriteLine(tl(i), len(i), i == numLines);
}



//-------------------------------------------------------------------------
// バッファの内容を全部破棄（暫定）
//-------------------------------------------------------------------------

void CPadDocument::ClearAll(bool bExecUndo)
{
	if (bExecUndo) {
		// 全部削除
		Execute( CCmdDelete( DPos(0,0), DPos(0xffffffff,0xffffffff) ) );
	}

	// Undo-Redoチェインをクリア
	m_urdo.Clear();
	m_urdo.SavedHere();
	Fire_MODIFYFLAGCHANGE();
}



//-------------------------------------------------------------------------
// ファイルを開く（暫定）
//-------------------------------------------------------------------------

void CPadDocument::OpenFile(TextFileR* tf)
{
	// 挿入
	DPos e(0,0);

	ULONG size = 2048;
	WCHAR* bufptr = new WCHAR[size + 1];
	ULONG i = 0;
	while (tf->state()) {
		size_t L = tf->ReadLine(bufptr, size);
		if (L) {
			DPos dp(i, 0xffffffff);
			InsertingOperation(dp, bufptr, (ULONG)L, e);
			i = tln() - 1;
		}
		if (tf->state() == T_EOL) {
			DPos dp(i++, 0xffffffff);
			InsertingOperation(dp, L"\n", 1, e);
		}
	}
	delete [] bufptr;

	// イベント発火
	Fire_TEXTUPDATE( DPos(0,0), DPos(0,0), e, true, false );
}


void CPadDocument::StartThread()
{
}

