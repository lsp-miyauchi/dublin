#include "stdafx.h"
#include "ip_doc.h"


//=========================================================================
//---- ip_parse.cpp キーワード解析
//
//		キーワード定義ファイルに従って、保持する文字列を
//		適切に切り分ける作業がここ。
//
//---- ip_text.cpp   文字列操作・他
//---- ip_wrap.cpp   折り返し
//---- ip_scroll.cpp スクロール
//---- ip_draw.cpp   描画・他
//---- ip_cursor.cpp カーソルコントロール
//=========================================================================



//=========================================================================
//
// 解析結果データ仕様
// これだけ色々姑息な手段を持ち込んで本当に
// 速くなっているのかどうかは不明…(^^;
//
// -----------------------------------------------
//
// Line::m_isLineHeadCommented
//    0: 行頭がブロックコメントの内部ではない
//    1: 行頭がブロックコメントの内部
//
// -----------------------------------------------
//
// Line::m_commentTransition
//   00: 行末は常にコメントの外
//   01: 行頭と行末はコメント状態が逆転
//   10: 行頭と行末はコメント状態が同じ
//   11: 行末は常にコメントの中
//
// -----------------------------------------------
//
// 以上二つのフラグを元に、前の行の情報から今の行の情報を
//   this.head = (prev.trans >> prev.head)&1;
// で順次計算していくことが出来る。
// この計算の際に内部バッファの状態まで書き換えるのは
// コストがでかすぎるので、次に示すフラグを見ながら
// 描画寸前に適宜調整する。
//
// -----------------------------------------------
//
// Line::commentBitReady_
//   コメントビットが調整済みかどうか
//
// -----------------------------------------------
//
// Line::m_str[]
//   UCS-2ベタで、文字列データがそのまま格納される。
//   ただし、パーサの高速化のために最終文字の後ろに
//   0x007fが付加される。
//
// -----------------------------------------------
//
// Line::m_flg
//   一文字毎に、下のような8bitのフラグを割り当てる
//   | aaabbbcd |
// 
// -----------------------------------------------
//
// aaa == "PosInToken"
//     0: トークンの途中
//   1-6: トークンの頭。次の頭は1-6文字先。
//     7: トークンの頭。次の頭は7文字以上先。
//
// -----------------------------------------------
//
// bbb == "TokenType"
//     0: TAB: タブ文字
//     1: WSP: ホワイトスペース
//     2: TXT: 普通の文字
//     3:  CE: コメント開始タグ
//     4:  CB: コメント終了タグ
//     5:  LB: 行コメント開始タグ
//     6:  Q1: '' 引用符1
//     7:  Q2: "" 引用符2
//
// -----------------------------------------------
//
//  c  == "isKeyword?"
//     0: キーワードではない
//     1: キーワード
//
// -----------------------------------------------
//
//  d  == "inComment?"
//     0: コメントの中ではない
//     1: コメントの中
//
// -----------------------------------------------



//-------------------------------------------------------------------------
// コメントの中なのか外なのか等を判定するためのオートマトン
//
// /* が出たらその後ろはコメントで */ が出たらその後ろはノーマルゾーン
// …という単純な規則では上手く行かない。例えば str"/*"str なんてものが
// 出現した場合に困ってしまう。そこで、
//   ・普通のテキスト
//   ・ブロックコメントの中
//   ・行コメントの中
//   ・一重引用符の中
//   ・二重引用符の中
// の５種類の状態に分けて、それぞれの場合について、どの記号が出たら
// 次にどの状態に移るのか…を処理する必要がある。その状態変化の規則を
// 5x5の２次元配列で与えて管理する。
//-------------------------------------------------------------------------

enum CommentDFASymbol{ sCB, sCE, sLB, sQTS, sQTD, sXXX };

struct CommentDFA {
	// <状態>
	// 最下位bitが、現在コメント内かどうかのフラグになります。
	// ブロックコメント中かどうかは (state >> 1) & (state) で。
	//   000: normal text        011: in BlockComment
	//   001: in LineComment     100: in Quote2
	//   010: in Quote1
	//
	// <シンボル>
	// C++で言うと下の通り
	// 値はTokenTypeフラグとシンクロするようになってます。
	//   000: CE */              011: QTS '
	//   001: CB /*              100: QTD "
	//   010: LB //

	// 初期状態を指定。コメント内かコメント外か
	CommentDFA( bool inComment ) : state( inComment ? 3 : 0 ) { }

	// 入力符号を与えて状態遷移
	void transit( int sym ) { state = tr_table[state][sym]; }

	// 現在の状態
	int state;

	// 状態遷移テーブル
	static const int tr_table[5][5];
};

const int CommentDFA::tr_table[5][5] = {
	{0,3,1,2,4},
	{1,1,1,1,1},
	{2,2,2,0,2},
	{0,3,3,3,3},
	{4,4,4,4,0},
};



//-------------------------------------------------------------------------
// 単純な、キーワード格納構造体。
// ChainHashの要素にするためnextポインタがつけてあります。
//-------------------------------------------------------------------------

struct Keyword {
	WCHAR* str;
	const ULONG len;
	Keyword* next;

	Keyword( const WCHAR* s, ULONG l )
		: str( new WCHAR[l + 1] )
		, len( l )
		, next( NULL )
	{
		memcpy(str, s, l * sizeof(WCHAR));
		str[l] = 0;
	}

	~Keyword() { delete [] str; }
};


//-------------------------------------------------------------------------
// サポート関数。Unicodeテキスト同士の比較
//-------------------------------------------------------------------------

static bool compare_s(const WCHAR* a,const WCHAR* b,ULONG l)
{
	// 大文字小文字を区別
	while( l-- )
		if( *a++ != *b++ )
			return false;
	return true;
}

static bool compare_i(const WCHAR* a,const WCHAR* b,ULONG l)
{
	// 大文字小文字を区別しない（雑）
	while( l-- )
		if( ((*a++) ^ (*b++)) & 0xdf )
			return false;
	return true;
}



//-------------------------------------------------------------------------
// 与えられた記号文字列から、コメント開始等の意味のあるトークンを
// 切り出してくるための構造。
//-------------------------------------------------------------------------

class CTagMap
{
	Keyword* m_tag[3]; // 0:CE 1:CB 2:LB
	bool m_esc, m_q1, m_q2, m_map[128];

public:

	CTagMap(const WCHAR* cb, ULONG cblen,
		    const WCHAR* ce, ULONG celen,
		    const WCHAR* lb, ULONG lblen,
		    bool q1, bool q2, bool esc )
		: m_q1 ( q1 )
		, m_q2 ( q2 )
		, m_esc( esc )
	{
		// '/' で始まる記号は使われているか…？
		// みたいな、１文字目のみのチェックに使う表を作成
		m_tag[0] = m_tag[1] = m_tag[2] = NULL;
		memset(m_map, 0, sizeof(m_map));
		m_map[L'\''] = q1;
		m_map[L'\"'] = q2;
		m_map[L'\\'] = esc;
		if (celen) {
			m_map[*ce] = true;
			m_tag[0] = new Keyword(ce, celen);
		}
		if (cblen) {
			m_map[*cb] = true;
			m_tag[1] = new Keyword(cb, cblen);
		}
		if (lblen) {
			m_map[*lb] = true;
			m_tag[2] = new Keyword(lb, lblen);
		}
	}

	~CTagMap()
	{
		// キーワード解放
		delete m_tag[0];
		delete m_tag[1];
		delete m_tag[2];
	}

	bool does_esc()
	{
		// \ によるエスケープをするかどうか
		return m_esc;
	}

	ULONG SymbolLoop(const WCHAR* str, ULONG len, ULONG& mlen, int& sym )
	{
		// 有意味な記号にマッチするまでループ
		// 返値に、マッチするまでに飛ばした文字数、
		// mlen, symに、マッチした記号の情報を返す

		int i;
		ULONG ans=0;
		for (sym = sXXX, mlen = 1; ans < len; ++ans) {
			if (m_map[str[ans]]) {
				for (i = 2; i >= 0; --i) {
					if (m_tag[i] != NULL &&
						m_tag[i]->len <= len - ans &&
						compare_s(m_tag[i]->str, str + ans, m_tag[i]->len )) {
						sym  = i;
						mlen = m_tag[i]->len;
						goto symbolfound;
					}
				}

				if (str[ans] == L'\'') {		// 一重引用符
					if (m_q1) {
						sym  = sQTS;
						goto symbolfound;
					}
				}
				else if (str[ans] == L'\"') {	// 二重引用符
					if (m_q2) {
						sym  = sQTD;
						goto symbolfound;
					}
				}
				else if (str[ans] == L'\\') {	// \ の後の文字はSkip
					if (m_esc && ans + 1 < len)
						++ans;
				}
			}
		}
	symbolfound:
		return ans;
	}
};



//-------------------------------------------------------------------------
// 与えられた文字列がキーワードかどうか高速判定するためのハッシュ表
//-------------------------------------------------------------------------

class CKeywordMap : public CSimpleArray<Keyword*>
{
	CSimpleMap<Keyword*, ULONG> m_kwdmap;
	bool (*m_compare)(const WCHAR*,const WCHAR*,ULONG);

public:

	CKeywordMap( bool bCaseSensitive )
	{
		m_compare = bCaseSensitive ? compare_s : compare_i;

		// ハッシュ表初期化
		for (int i = 0; i < 4096; i++)
			Add(NULL);
	}

	~CKeywordMap()
	{
		DeleteAll();
	}

	void DeleteAll()
	{
		// 解放
		int cnt = GetSize();
		for (int i = 0; i < cnt; i++)
			delete (*this)[i];
		RemoveAll();
		m_kwdmap.RemoveAll();
	}

	void AddKeyword( const WCHAR* str, ULONG len, ULONG keywordtype)
	{
		// データ登録
		Keyword* x = new Keyword(str,len);
		int h = hash(str,len);

		Keyword* q = (*this)[h];
		if (!q) {
			// ハッシュテーブルが空の場合
			SetAtIndex(h, x);
		}
		else {
			// チェイン末尾に繋ぐ場合
			Keyword* p = q->next;

			while (p != NULL) {
				q = p;
				p = p->next;
			}
			q->next = x;
		}
		m_kwdmap.Add(x, keywordtype);
	}

	ULONG isKeyword( const WCHAR* str, ULONG len )
	{
		// 登録されているキーワードと一致するか？
		Keyword* p = (*this)[hash(str, len)];
		while (p) {
			if (p->len == len && m_compare(p->str, str, len))
				return m_kwdmap.Lookup(p);
			p = p->next;
		}
		return 0;
	}

private:

	static int hash( const WCHAR* a, ULONG al )
	{
		// 12bitに潰すめっちゃ雑なハッシュ関数
		// ルーチン分けるの面倒なので、大文字小文字は常に区別されない。(^^;
		int h = 0, i = 0;
		while (al--) {
			h ^= ((*(a++) & 0xdf) << i);
			i = (i + 5) & 7;
		}
		return h & 4095;
	}
};



//-------------------------------------------------------------------------
// 以上の道具立てでもって、テキストの解析を行うParser
//-------------------------------------------------------------------------

class CParser
{
	CKeywordMap m_kwd;
	CTagMap m_tag;

public:
	// 初期化１
	CParser(
		const WCHAR* cb, ULONG cblen,
		const WCHAR* ce, ULONG celen,
		const WCHAR* lb, ULONG lblen,
		bool q1, bool q2, bool esc,
		bool casesensitive
	)
		: m_tag(cb, cblen, ce, celen, lb, lblen, q1, q2, esc)
		, m_kwd(casesensitive)
	{
	}

	// 初期化２：キーワード追加
	void AddKeyword( const WCHAR* str, ULONG len, ULONG keywordtype )
	{
		m_kwd.AddKeyword( str, len, keywordtype );
	}

	// 全角タイプ判定
	int CheckKanjiType(WCHAR a)
	{
		if (a < 0x80)	return KTYPE_INVALID;
		if (KSPACE(a))	return KTYPE_SPACE;
		if (KDIGIT(a))	return KTYPE_DIGIT;
		if (KALPHA(a))	return KTYPE_ALPHA;
		if (KHIRA(a))	return KTYPE_HIRA;
		if (KKATA(a))	return KTYPE_KATA;
		if (KKANJI(a))	return KTYPE_KANJI;
		if (KKEISEN(a))	return KTYPE_KEISEN;
		if (KHAN(a))	return KTYPE_HAN;
		return KTYPE_UNKNOWN;
	}

	// 行データ解析
	BYTE Parse( CTextLine& line, BYTE cmst )
	{
		line.TransitCmt( cmst );

		// ASCII振り分けテーブル。
		// シフト無しでTokenTypeに流用出来るようにするため、
		// 値が４飛びになってます
		enum { T=0, W=4, A=8, S=12, O=0 };
		static const BYTE letter_type[128] = {
			O,O,O,O,O,O,O,O,O,T,O,O,O,O,O,O,
			O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,
			W,S,S,S,S,S,S,S,S,S,S,S,S,S,S,S,
			A,A,A,A,A,A,A,A,A,A,S,S,S,S,S,S,
			S,A,A,A,A,A,A,A,A,A,A,A,A,A,A,A,
			A,A,A,A,A,A,A,A,A,A,A,S,S,S,S,A,
			S,A,A,A,A,A,A,A,A,A,A,A,A,A,A,A,
			A,A,A,A,A,A,A,A,A,A,A,S,S,S,S,O,
		};

		// PosInToken算出用の距離エンコーダ( 5bitシフト済み )
		//  ( _d>7 ? 7<<5 : _d<<5 )
		#define tkenc(_d) ( (_d)>7 ? 0xe0 : (_d)<<5 )

		// コメント状態遷移追跡用オートマトン
		CommentDFA dfa[2] = { CommentDFA(false), CommentDFA(true) };
		int& cmtState  = dfa[line.isLineHeadCmt()].state;
		int commentbit = cmtState & 1;

		// 作業領域
		int sym;
		ULONG j, k, um, m;
		BYTE t, f;

		// ループ～
		const WCHAR* str = line.str();
		BYTE* flg_ptr = line.flg();
		BYTE* ktp_ptr = line.ktp();
		ULONG end = line.size();
		for (ULONG i = 0; i < end; i = j) {
			j = i;

			if (str[i] >= 0x007f) {
				// ASCII文字でない場合
				f = (TOKEN_ALP | commentbit);
				int kt = CheckKanjiType(str[i]);
				while (++j < end && CheckKanjiType(str[j]) == kt) {
					flg_ptr[j] = f;
					ktp_ptr[j] = commentbit ? 1 : 0;
				}
				flg_ptr[i] = static_cast<BYTE>(tkenc(j - i) | f);
				ktp_ptr[i] = commentbit ? 1 : 0;
			}
			else {
				// ASCII文字の場合??
				t = letter_type[str[i]];
				if (t == S && m_tag.does_esc()) {
					do {
						if (str[j] == L'\\' && j + 1 < end)
							j++;
					} while (++j < end && str[j] < 0x7f && S == letter_type[str[j]]);
				}
				else {
					while (++j < end && str[j] < 0x7f && t == letter_type[str[j]])
						;
				}

				f = (t | commentbit);

				BYTE kwd_index = 0;
				switch (t) {
				// アルファベット＆数字
				case A:
					kwd_index = (BYTE)m_kwd.isKeyword(str + i, j - i);
					f |= (kwd_index > 0) ? 2 : 0;
					// fall...

				// タブ・制御文字
				case T:
					// fall...

				// 半角空白
				case W:
					for (k = i + 1; k < j; k++) {
						flg_ptr[k] = f;
						ktp_ptr[k] = commentbit ? CLR_CMT : kwd_index;
					}
					flg_ptr[i] = (BYTE)(tkenc(j - i) | f);
					ktp_ptr[i] = commentbit ? 1 : kwd_index;
					break;

				// 記号
				case S:
					k = i;
					while (k < j) {
						// マッチしなかった部分
						um = m_tag.SymbolLoop( str + k, j - k, m, sym );
						f = (0x20 | TOKEN_ALP | commentbit);
						while ( um-- ) {
							flg_ptr[k] = f;
							ktp_ptr[k] = commentbit ? CLR_CMT : 0;
							k++;
						}
						if (k >= j)
							break;

						// マッチした部分
						f = (TOKEN_CE | commentbit);
						dfa[0].transit( sym );
						dfa[1].transit( sym );
						commentbit = cmtState & 1;
						if (sym != 0)			// 0:comment end
							f = (((sym + 3) << 2) | commentbit);

						flg_ptr[k] = (BYTE)(tkenc(m) | f);
						ktp_ptr[k] = commentbit ? CLR_CMT : 0;
						k++;

						while (--m) {
							flg_ptr[k] = f;
							ktp_ptr[k] = commentbit ? CLR_CMT : 0;
							k++;
						}
					}
					break;
				}
			}
		}

		// transitフラグ更新
		line.SetTransitFlag(
			(dfa[1].state & (dfa[1].state << 1)) | ((dfa[0].state >> 1) & dfa[0].state)
		);
		line.CommentBitUpdated();
		return line.TransitCmt( cmst );
	}

	// コメントビットを正しく調整
	void SetCommentBit( CTextLine& line )
	{
		CommentDFA dfa( line.isLineHeadCmt()==1 );
		ULONG commentbit = dfa.state & 1;

		// ループ～
		const WCHAR* str = line.str();
		BYTE* flg_ptr = line.flg();
		BYTE* ktp_ptr = line.ktp();
		ULONG j, k, end = line.size();
		for (ULONG i = 0; i < end; i = j) {
			// Tokenの終端を得る
			k = (flg_ptr[i] >> 5);
			j = i + k;
			if (j >= end)
				j = end;
			else if (k == 7) {		// || k==0 )
				while ((flg_ptr[j] >> 5) == 0 && j < end)
					j++;
			}

			k = (flg_ptr[i] & 0x1c);
			if (k <= TOKEN_CE) {
				for (;i < j; i++)
					flg_ptr[i] = (BYTE)((flg_ptr[i] & 0xfe) | commentbit);
			}
			if (k >= TOKEN_CE) {
				dfa.transit((k >> 2) - 3);
				commentbit = dfa.state & 1;
				if (k != TOKEN_CE) {
					for( ; i < j; i++)
						flg_ptr[i] = (BYTE)((flg_ptr[i] & 0xfe) | commentbit);
				}
			}
		}

		line.CommentBitUpdated();
	}
};



//-------------------------------------------------------------------------
// 定義ファイル読みとり処理とか
//-------------------------------------------------------------------------

CPadDocument::CPadDocument()
	: m_arrDocEvHandler(2)
	, m_parser(NULL)
	, m_busy(false)
{
	m_text.Add(new CTextLine(L"", 0));		// 最初は一行だけ
	SetKeyword(NULL, 0);					// キーワード無し
}

CPadDocument::~CPadDocument()
{
	if (m_parser)
		delete m_parser;
}

void CPadDocument::SetKeyword(const WCHAR* textptr, ULONG siz)
{
	// BOMがあったらスキップ
	if (siz != 0 && *textptr == 0xfeff) {
		textptr++;
		siz--;
	}

	// 読み込み準備
	CUnicodeReader r(textptr, siz);
	bool flags[] = { false,false,false,false };
	const WCHAR* tags[] = { NULL, NULL, NULL };
	ULONG taglen[] = { 0, 0, 0 };

	const WCHAR* str;
	int len;
	if (siz != 0) {
		// １行目:フラグ
		//   case? q1? q2? esc?
		len = r.GetLine(&str);
		len = Min<int>(len, 4);
		for (int i = 0; i < len; i++)
			flags[i] = (str[i] == L'1');

		// ２～４行目
		//   ブロコメ開始記号、ブロコメ終了記号、行コメ記号
		for (int j = 0; j < 3; j++) {
			len = r.GetLine(&str);
			if (len > 0) {
				tags[j] = str;
				taglen[j] = len;
			}
		}
	}

	// パーサー作成
	if (m_parser) {
		delete m_parser;
		m_parser = NULL;
	}
	m_parser = new CParser(
						tags[0], taglen[0], tags[1], taglen[1], tags[2], taglen[2],
						flags[1], flags[2], flags[3], flags[0] );

	// ５行目以降：キーワードリスト
	while (!r.Eof()) {
		len = r.GetLine(&str);
		if (len != 0) {
			ULONG type = 0;
			if ('1' <= *str && *str <= '9' && *(str + 1) == ',') {
				type = *str - '1';
				len -= 2;
				str += 2;
			}
			m_parser->AddKeyword(str, len, type + CLR_KWD1);
		}
	}

	// 全行解析し直し
	ReParse(0, tln() - 1);

	// 変更通知
	Fire_KEYWORDCHANGE();
}

bool CPadDocument::ReParse( ULONG s, ULONG e )
{
	ULONG i;
	BYTE cmt = m_text[s].isLineHeadCmt();

	// まずは変更範囲を再解析
	for (i = s; i <= e; i++)
		cmt = m_parser->Parse(m_text[i], cmt);

	// コメントアウト状態に変化がなかったらここでお終い。
	if (i == tln() || m_text[i].isLineHeadCmt() == cmt)
		return false;

	// 例えば、/* が入力された場合などは、下の方の行まで
	// コメントアウト状態の変化を伝達する必要がある。
	do {
		cmt = m_text[i++].TransitCmt(cmt);
	} while (i < tln() && m_text[i].isLineHeadCmt() != cmt);
	return true;
}

void CPadDocument::SetCommentBit( const CTextLine& x ) const
{
	m_parser->SetCommentBit( const_cast<CTextLine&>(x) );
}
