////////////////////////////////////////////////////////////////////////
// ip_doc.h

#pragma once

#ifndef _IP_DOC_H_
#define _IP_DOC_H_
#include "ip_common.h"

class CParser;
class CPadDocument;
class CPadCommand;
class CCmdInsert;
class CCmdDelete;
class CCmdReplace;




//=========================================================================
//	行バッファ構造体
//
//	UCS-2ベタの形式でテキストデータを保持する。またそれと同時に、
//	キーワードファイルによって指定された強調語を区別するための
//	解析処理結果用バッファも管理する。文字データに終端NULは
//	付けないが、解析作業の高速化のため、終端 U+007f が入る。
//=========================================================================

class CTextLine
{
public:

	CTextLine()
		: m_len(0)
		, m_alen(0)
		, m_str(NULL)
		, m_flg(NULL)
		, m_ktp(NULL)
		, m_bCommentBitReady(false)
		, m_isLineHeadCommented(0)
	{
		m_len = 0;
		m_alen = 10;
		m_str = (WCHAR*)malloc((m_alen + 1) * 2 + m_alen * 2);
		m_flg = (BYTE*)(m_str + m_alen + 1);
		m_ktp = m_flg + m_alen;
		m_str[0] = 0x007f;
	}

	// 指定テキストで初期化
	CTextLine( const WCHAR* str, ULONG len )
		: m_bCommentBitReady(false)
		, m_isLineHeadCommented(0)
	{
		m_len = len;
		m_alen = (10 > len) ? 10 : len;
		m_str = (WCHAR*)malloc((m_alen + 1) * 2 + m_alen * 2);
		m_flg = (BYTE*)(m_str + m_alen + 1);
		m_ktp = m_flg + m_alen;
		memcpy(m_str, str, len * 2);
		m_str[len] = 0x007f;
	}

	~CTextLine()
	{
		free(m_str);
	}

	// テキスト挿入(指定位置に指定サイズ)
	void InsertAt(ULONG at, const WCHAR* buf, ULONG siz)
	{
		if (m_len + siz > m_alen) {
			// バッファ拡張
			ULONG psiz = (m_alen + 1) * 2 + m_alen;
			m_alen = Max(m_alen << 1, m_len + siz );
			WCHAR* tmpS = (WCHAR*)malloc((m_alen + 1) * 2 + m_alen * 2);
			BYTE* tmpF = (BYTE*)(tmpS + m_alen + 1);
			BYTE* tmpK = tmpF + m_alen;
			// コピー
			memcpy(tmpS,            m_str,      at * 2);
			memcpy(tmpS + at + siz, m_str + at, (m_len - at + 1) * 2);
			memcpy(tmpF,            m_flg,      at);
			memcpy(tmpK,            m_ktp,      at);
			// 古いのを削除
			free(m_str);
			m_str = tmpS;
			m_flg = tmpF;
			m_ktp = tmpK;
		}
		else {
			memmove(m_str + at + siz, m_str + at, (m_len - at + 1) * 2);
			memmove(m_flg + at + siz, m_flg + at, (m_len - at));
			memmove(m_ktp + at + siz, m_ktp + at, (m_len - at));
		}
		memcpy(m_str + at, buf, siz * sizeof(WCHAR));
		m_len += siz;
	}

	// テキスト挿入(末尾に)
	inline void Append(const WCHAR* buf, ULONG siz) { InsertAt(m_len, buf, siz); }

	// テキスト削除(指定位置から指定サイズ)
	void RemoveAt(ULONG at, ULONG siz)
	{
		memmove(m_str + at, m_str + at + siz, (m_len - siz - at + 1) * 2);
		memmove(m_flg + at, m_flg + at + siz, (m_len - siz - at));
		memmove(m_ktp + at, m_ktp + at + siz, (m_len - siz - at));
		m_len -= siz;
	}

	// テキスト削除(指定位置から末尾まで)
	void Truncate(ULONG at)
	{
		if (at < m_len) {
			m_len = at;
			m_str[m_len] = 0x007f;
		}
	}

	// バッファにコピー(指定位置から指定サイズ)
	ULONG GetAt(ULONG at, ULONG siz, WCHAR* buf)
	{
		ATLASSERT(IsBadWritePtr(buf, siz * sizeof(WCHAR)) == FALSE);
		memcpy(buf, m_str + at, siz * sizeof(WCHAR));
		return siz;
	}

	// バッファにコピー(指定位置から末尾まで)
	inline ULONG GetToTail( ULONG at, WCHAR* buf ) { return GetAt( at, m_len-at, buf ); }

	// 長さ
	inline ULONG size() const { return m_len; }

	// テキスト
	inline WCHAR* str() { return m_str; }

	// テキスト(const)
	inline const WCHAR* str() const { return m_str; }

	// 解析結果
	inline BYTE* flg() { return m_flg; }

	// 解析結果(const)
	inline const BYTE* flg() const { return m_flg; }

	// キーワードタイプ解析結果
	inline BYTE* ktp() { return m_ktp; }

	// キーワードタイプ解析結果(const)
	inline const BYTE* ktp() const { return m_ktp; }

	// ask
	inline bool isCmtBitReady() const { return m_bCommentBitReady; }
	inline BYTE isLineHeadCmt() const { return m_isLineHeadCommented; }
	// for doc
	BYTE TransitCmt( BYTE start )
	{
		m_isLineHeadCommented = start;
		m_bCommentBitReady = false;
		return (m_commentTransition >> start) & 1;
	}
	// for parser
	inline void SetTransitFlag( BYTE flag ) { m_commentTransition = flag; }
	inline void CommentBitUpdated() { m_bCommentBitReady = true; }

private:
	ULONG m_alen;
	ULONG m_len;
	WCHAR* m_str;
	BYTE* m_flg;
	BYTE* m_ktp;

	BYTE m_isLineHeadCommented;
	BYTE m_commentTransition;
	bool m_bCommentBitReady;
};



//=========================================================================
//	Unicodeテキスト切り分け君
//
//	行単位で処理を行うことが多いので、その行毎に分ける処理を
//	切り出した。getLine() するたびに、指定したポインタと整数変数へ
//	先頭から順に行のデータを格納して行く。
//=========================================================================

class CUnicodeReader
{
public:

	// 読みとり元バッファを与えて初期化
	CUnicodeReader(const WCHAR* str, ULONG len)
		: m_eof( false )
	{
		LPTSTR p = m_str.GetBuffer(len + 1);
		memcpy(p, str, sizeof(WCHAR) * len);
		m_str.ReleaseBuffer(len);

		m_ptr = m_str.GetString();
		m_end = m_ptr + len;
	}

	// 読み終わったかどうかチェック
	bool Eof() { return m_eof; }

	// 一行取得
	int GetLine(LPCTSTR* ppsz)
	{
		// 次の改行の位置を取得
		const WCHAR *p = m_ptr, *e = m_end;
		for (; p < e; p++) {
			if (*p == L'\r' || *p == L'\n')
				break;
		}

		// 記録
		*ppsz = m_ptr;
		int len = int(p - m_ptr);

		// 改行コードスキップ
		if (p == e)
			m_eof = true;
		else {
			if (*(p++) == L'\r' && p < e && *p == L'\n')
				p++;
		}
		m_ptr  = p;

		return len;
	}

private:
	bool  m_eof;
	CString m_str;
	const WCHAR *m_ptr, *m_end;
};



//=========================================================================
//	Undo/Redo用に、CPadCommandオブジェクトを保存しておくクラス
//=========================================================================

class CUnReDoChain
{
public:

	// コンストラクタ
	CUnReDoChain();

	// デストラクタ
	~CUnReDoChain();

	// Undo実行
	void Undo( CPadDocument* doc );

	// Redo実行
	void Redo( CPadDocument* doc );

	// 新しいコマンドを実行
	void NewlyExec( const CPadCommand& cmd, CPadDocument* doc );

	// 初期状態に戻す
	void Clear();

	// 保存位置フラグを現在位置にセット
	inline void SavedHere() { m_savedPos = m_lastOp; }

public:
	// Undo操作が可能か？
	inline bool CanUndo() const { return (m_lastOp != &m_headTail); }

	// Redo操作が可能か？
	inline bool CanRedo() const { return (m_lastOp->m_next != &m_headTail); }

	// 保存後、変更されているか？
	inline bool IsModified() const { return (m_lastOp != m_savedPos); }

private:

	struct Node
	{
		Node();
		Node( CPadCommand*, Node*, Node* );
		~Node();
		void  ResetCommand( CPadCommand* cmd );
		ULONG ChainDelete(Node*& m_savedPosref);
		Node    *m_next, *m_prev;
		CPadCommand *m_cmd;
	};

private:

	Node  m_headTail;
	Node* m_savedPos;
	Node* m_lastOp;
	ULONG m_num;
};


//=========================================================================
//	イベントハンドラインターフェイス
//
//	ドキュメントから発生するイベント（挿入/削除などなど…）を
//	受け取りたい場合は、このインターフェイスを継承し、適宜ハンドラを
//	書くこと。Viewの再描画処理などもこれを通じて実行されている。
//=========================================================================

class CDocumentEventHandler
{
public:
	//	テキスト内容が変更されたときに発生
	//	@param s        変更範囲の先頭
	//	@param e        変更範囲の終端(前)
	//	@param e2       変更範囲の終端(後)
	//	@param reparsed e2より後ろのコメントアウト状態が変化していたらtrue
	//	@param nmlcmd   挿入/削除/置換ならtrue、ファイル開き/全置換ならfalse
	virtual void OnDocEventTextUpdate( const DPos& s, const DPos& e, const DPos& e2, bool reparsed, bool nmlcmd ) {}

	//	キーワードが変更されたときに発生
	virtual void OnDocEventKeywordChanged() {}

	//	ダーティフラグが変更されたときに発生
	virtual void OnDocumentEventDirtyFlagChanged(bool dirty) {}
};


//=========================================================================
//	CPadDocumentクラスの実装部分
//=========================================================================

class CPadDocument
{
public:

	CPadDocument();
	~CPadDocument();

	// 操作コマンド実行
	void Execute( const CPadCommand& cmd );

	// キーワード定義切り替え
	void SetKeyword( const WCHAR* defbuf, ULONG siz );

	// イベントハンドラ登録
	void AddHandler( CDocumentEventHandler* eh );

	// イベントハンドラ解除
	void RemoveHandler( CDocumentEventHandler* eh );

	// ファイルを開く
	void OpenFile( TextFileR* tf );

	// ファイルを保存
	void SaveFile( TextFileW* tf );

	// 内容破棄
	void ClearAll(bool bExecUndo = true);

	// アンドゥ
	void Undo();

	// リドゥ
	void Redo();

	// 変更フラグをクリア
	void ClearModifyFlag();

public:

	// 行数
	inline ULONG tln() const { return m_text.size(); }

	// 行バッファ
	const WCHAR* tl( ULONG i ) const;

	// 行解析結果バッファ
	inline const BYTE* pl(ULONG i) const
	{
		const CTextLine& x = m_text[i];
		if (!x.isCmtBitReady())
			SetCommentBit(x);
		return x.flg();
	}

	// キーワード解析結果バッファ
	inline const BYTE* kl(ULONG i) const
	{
		const CTextLine& x = m_text[i];
//@		if (!x.isCmtBitReady())
//@			SetCommentBit(x);
		return x.ktp();
	}

	// 行文字数
	inline ULONG len( ULONG i ) const { return m_text[i].size(); }

	// 指定範囲のテキストの長さ
	ULONG GetRangeLength(const DPos& stt, const DPos& end, LPCTSTR pszFollow = NULL);

	// 指定範囲のテキスト
	void GetText(WCHAR* buf, const DPos& stt, const DPos& end, LPCTSTR pszFollow = NULL);

	// 指定位置からの単語
	void GetWord(WCHAR* buf, const DPos& dp, bool bFullWord = false);

	// 指定位置の文字
	WCHAR GetChar(const DPos& s);

	// 指定位置の単語の先頭を取得
	DPos wordStartOf( const DPos& dp ) const;

	// 指定位置の一つ左の位置を取得
	DPos leftOf( const DPos& dp, bool wide=false ) const;

	// 指定位置の一つ右の位置を取得
	DPos rightOf( const DPos& dp, bool wide=false ) const;

	// 最終変更位置
	DPos lastEditOf() const;

	// アンドゥ可能？
	bool CanUndo() const;

	// リドゥ可能？
	bool CanRedo() const;

	// 変更済み？
	bool IsModified() const;

	// 空
	bool IsEmpty() const;

	//@{ ビジーフラグ（マクロコマンド実行中のみ成立） //@}
	void setBusyFlag( bool b=true ) { m_busy = b; }
	bool isBusy() const { return m_busy; }

private:
	std::unique_ptr<CParser> m_parser;			// 文字列解析役
	CGapBufObj<CTextLine> m_text;						// テキストデータ
	mutable CStorage<CDocumentEventHandler*> m_arrDocEvHandler; // イベント通知先
	CUnReDoChain m_urdo;								// アンドゥリドゥ
	DPos m_dpLastEdit;									// 最終変更位置

private:
	// 変更通知
	void Fire_KEYWORDCHANGE();
	void Fire_MODIFYFLAGCHANGE();
	void Fire_TEXTUPDATE( const DPos& s, const DPos& e, const DPos& e2, bool reparsed, bool nmlcmd );

	// ヘルパー関数
	bool ReParse( ULONG s, ULONG e );
	void SetCommentBit( const CTextLine& x ) const;
	void CorrectPos( DPos& pos );
	void CorrectPos( DPos& stt, DPos& end );

	// 挿入・削除作業
	bool InsertingOperation(DPos& stt, const WCHAR* str, ULONG len, DPos& undoend );
	bool DeletingOperation(DPos& stt, DPos& end, WCHAR*& undobuf, ULONG& undosiz );

public:
	// アンドゥを記録せずに文字列を末尾に追加
	bool AppendString(const WCHAR* str);

private:
	// パラレルリード
	virtual void StartThread();

private:
	bool m_busy;

	friend class CCmdInsert;
	friend class CCmdDelete;
	friend class CCmdReplace;
};


//=========================================================================
//	操作コマンドインターフェイス
//
//	ドキュメントは、Command から派生したクラスのインスタンスの
//	operator() を呼び出すことで、色々な操作を実行する。とりあえず
//	具体的には Insert/Delete/Replace の３つだけ。あとでマクロコマンド用
//	クラスも作るつもりだけど、とりあえずは保留。
//=========================================================================

class CPadCommand
{
protected:
	friend class CUnReDoChain;
	friend class CMacroCommand;
	virtual CPadCommand* operator()( CPadDocument* doc ) const = 0;
};



//=========================================================================
//	挿入コマンド
//=========================================================================

class CCmdInsert : public CPadCommand
{
public:

	//	@param s 挿入位置
	//	@param str 挿入文字列
	//	@param len 文字列の長さ
	//	@param del コマンド終了時にdelete [] strしてよいか？
	CCmdInsert( const DPos& s, const WCHAR* str, ULONG len, bool del=false );
	~CCmdInsert();

private:

	CPadCommand* operator()( CPadDocument* doc ) const;
	DPos m_stt;
	const WCHAR* m_buf;
	ULONG m_len;
	bool m_del;
};



//=========================================================================
//	削除コマンド
//=========================================================================

class CCmdDelete : public CPadCommand
{
public:

	//	@param s 開始位置
	//	@param e 終端位置
	CCmdDelete( const DPos& s, const DPos& e );

private:

	CPadCommand* operator()( CPadDocument* doc ) const;
	DPos m_stt;
	DPos m_end;
};




//=========================================================================
//	置換コマンド
//=========================================================================

class CCmdReplace : public CPadCommand
{
public:

	//	@param s 開始位置
	//	@param e 終端位置
	//	@param str 挿入文字列
	//	@param len 文字列の長さ
	//	@param del コマンド終了時にdelete [] strしてよいか？
	CCmdReplace( const DPos& s, const DPos& e, const WCHAR* str, ULONG len, bool del = false);
	~CCmdReplace();

private:

	CPadCommand* operator()( CPadDocument* doc ) const;
	DPos           m_stt;
	DPos           m_end;
	const WCHAR* m_buf;
	ULONG          m_len;
	bool           m_del;
};



//=========================================================================
//	マクロコマンド
//
//	複数のコマンドを一つのコマンドとして連続実行する。
//	ただし、Insert/Delete/Replaceを一回行うたびに当然
//	文字列の位置は変化するのだが、それに関する変換処理は
//	行わない。すなわち、Insert->Delete->Insert みたいな
//	連続処理を書くときは、行数や文字数の変化を考慮しながら
//	値を定めていくことが必要になる。ので、あんまり使えない(^^;
//=========================================================================

class CMacroCommand : public CPadCommand
{
public:
	//@{ コマンドの追加 //@}
	void Add(CPadCommand* cmd) { m_arr.Add(cmd); }

	//@{ コマンド数 //@}
	ULONG size() const { return m_arr.size(); }

	//@ デストラクタ //@}
	~CMacroCommand()
	{
		for (ULONG i = 0, e = m_arr.size(); i < e; ++i)
			delete m_arr[i];
	}

private:
	CPadCommand* operator()( CPadDocument* doc ) const;
	CStorage<CPadCommand*> m_arr;
};


#endif // _IP_DOC_H_
