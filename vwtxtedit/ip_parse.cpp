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
	std::wstring str;
	const ULONG len;
	Keyword* next;

	Keyword(const wchar_t* s)
    : str(s)
		, len(static_cast<ULONG>(str.size()))
		, next(nullptr)
	{
	}

	~Keyword() {}
};


//-------------------------------------------------------------------------
// サポート関数。Unicodeテキスト同士の比較
//-------------------------------------------------------------------------

static bool compare_s(const wchar_t* a, const wchar_t* b, unsigned long l)
{
	// 大文字小文字を区別
	// 先頭から5文字の string_view を作って比較
	std::wstring_view sv1(a, l);
	std::wstring_view sv2(b, l);
	return (sv1 == sv2);
}

static bool compare_i(const wchar_t* a, const wchar_t* b, unsigned long l)
{
	// 大文字小文字を区別しない（雑）
	std::wstring_view sv1(a, l);
	std::wstring_view sv2(b, l);
	return (_wcsnicmp(sv1.data(), sv2.data(), l) == 0);
}

//-------------------------------------------------------------------------
// 与えられた記号文字列から、コメント開始等の意味のあるトークンを
// 切り出してくるための構造。
//-------------------------------------------------------------------------

class CTagMap
{
	std::array<std::unique_ptr<Keyword>, 3> m_tag; // 0:CE 1:CB 2:LB
	bool m_esc;
	bool m_q1;
	bool m_q2;
	std::array<BYTE, 128> m_map;

public:

	CTagMap(const wchar_t* cb,
		    const wchar_t* ce,
		    const wchar_t* lb,
		    bool q1, bool q2, bool esc )
		: m_esc(esc)
		, m_q1(q1)
		, m_q2(q2)
	{
		// '/' で始まる記号は使われているか…？
		// みたいな、１文字目のみのチェックに使う表を作成
		m_map.fill(0);
		m_map[L'\''] = q1;
		m_map[L'\"'] = q2;
		m_map[L'\\'] = esc;
		if (ce && *ce) {
			m_map[*ce] = true;
			m_tag[0] = std::make_unique<Keyword>(ce);
		}
		if (cb && *cb) {
			m_map[*cb] = true;
			m_tag[1] = std::make_unique<Keyword>(cb);
		}
		if (lb && *lb) {
			m_map[*lb] = true;
			m_tag[2] = std::make_unique<Keyword>(lb);
		}
	}

	~CTagMap() = default;

	// \ によるエスケープをするかどうか
	inline bool does_esc() const { return m_esc; }

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
					if (m_tag[i] != nullptr &&
						m_tag[i]->len <= len - ans &&
						compare_s(m_tag[i]->str.c_str(), str + ans, m_tag[i]->len )) {
						sym  = i;
						mlen = m_tag[i]->len;
						return ans;
					}
				}

				if (str[ans] == L'\'' && m_q1) {			// 一重引用符
					sym  = sQTS;
					return ans;
				}
				else if (str[ans] == L'\"' && m_q2) {	// 二重引用符
					sym  = sQTD;
					return ans;
				}
				else if (str[ans] == L'\\' && (m_esc && ans + 1 < len)) {	// \ の後の文字はSkip
					++ans;
				}
			}
		}
		return ans;
	}
};



//-------------------------------------------------------------------------
// 与えられた文字列がキーワードかどうか高速判定するためのハッシュ表
//-------------------------------------------------------------------------

class CKeywordMap : public std::array<Keyword*, 4096>
{
	std::map<Keyword*, ULONG> m_kwdmap;
	bool (*m_compare)(const wchar_t*,const wchar_t*, ULONG);

public:

	CKeywordMap( bool bCaseSensitive )
	{
		m_compare = bCaseSensitive ? compare_s : compare_i;
		// ハッシュ表初期化
		fill(nullptr);
	}

	~CKeywordMap()
	{
		DeleteAll();
	}

	void DeleteAll()
	{
		// 解放
		for (auto& kwptr : *this) {
			if (kwptr != nullptr)
				delete kwptr;
		}
		fill(nullptr);
		m_kwdmap.clear();
	}

	void AddKeyword(const wchar_t* str, ULONG keywordtype)
	{
		// データ登録
		auto x = new Keyword(str);
		int h = hash(str, wcslen(str));

		Keyword* q = at(h);
		if (q == nullptr) {
			// ハッシュテーブルが空の場合
			at(h) = x;
		}
		else {
			// チェイン末尾に繋ぐ場合
			Keyword* p = q->next;

			while (p != nullptr) {
				q = p;
				p = p->next;
			}
			q->next = x;
		}
		m_kwdmap.insert(std::make_pair(x, keywordtype));
	}

	ULONG isKeyword( const WCHAR* str, ULONG len )
	{
		// 登録されているキーワードと一致するか？
		Keyword* p = at(hash(str, len));
		while (p) {
			if (p->len == len && m_compare(p->str.c_str(), str, len)) {
				auto it = m_kwdmap.find(p);
        return (it != m_kwdmap.end()) ? it->second : 0;
			}
			p = p->next;
		}
		return 0;
	}

private:

	static int hash( const WCHAR* a, size_t al )
	{
		// 12bitに潰すめっちゃ雑なハッシュ関数
		// ルーチン分けるの面倒なので、大文字小文字は常に区別されない。(^^;
		int h = 0;
		int i = 0;
		while (al--) {
			h ^= ((*(a++) & 0xdf) << i);
			i = (i + 5) & 7;
		}
		return h & 4095;
	}
};



/*-------------------------------------------------------------------------
 * 以上の道具立てでもって、テキストの解析を行うParser
 *-----------------------------------------------------------------------*/

class CParser
{
	CKeywordMap m_kwd;
	CTagMap m_tag;

public:
	// 初期化１
	CParser(
		const wchar_t* cb,
		const wchar_t* ce,
		const wchar_t* lb,
		bool q1, bool q2, bool esc,
		bool casesensitive
	)
		: m_tag(cb, ce, lb, q1, q2, esc)
		, m_kwd(casesensitive)
	{
	}

	// 初期化２：キーワード追加
	void AddKeyword( const WCHAR* str, ULONG keywordtype )
	{
		m_kwd.AddKeyword( str, keywordtype );
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
		// シフト無しでTokenTypeに流用出来るようにするため、値が４飛び
    // T: タブ文字, W: 半角空白, A: アルファベット, S: 記号, O: その他
		enum { T=0, W=4, A=8, S=12, O=0 };
		static const std::array<BYTE, 128> letter_type {
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
		std::array<CommentDFA, 2> dfa {
			CommentDFA(false),
			CommentDFA(true)
		};
		int& cmtState  = dfa[line.isLineHeadCmt()].state;
		BYTE commentbit = cmtState & 1;

		// 作業領域
		int sym;
		ULONG j, k, um, m;
		BYTE t, f;

		// ループ～
		const wchar_t* str = line.str();
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

				// タブ・制御文字 + 半角空白
				case T:
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
		BYTE commentbit = dfa.state & 1;

		// ループ～
		BYTE* flg_ptr = line.flg();
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
	, m_parser(nullptr)
	, m_busy(false)
{
	m_text.Add(new CTextLine(L"", 0));		// 最初は一行だけ
	KEYWORD_DICT dummy{ L"",{L"",L"",L""},{L""} };
	SetKeyword(dummy);										// キーワード無し
}

CPadDocument::~CPadDocument()
{
}

void CPadDocument::SetKeyword(const KEYWORD_DICT& keywords)
{
	// フラグ case q1 q2 esc
	//

	std::array<bool, 4> flags = { 0,0,0,0 };
	int i = 0;
	for (const auto& ch : keywords.flags) {
		flags[i++] = (ch == L'1');
	}

	// ブロコメ開始記号、ブロコメ終了記号、行コメ記号
	std::array<const wchar_t*, 4> tags;
	i = 0;
	for (const auto& str : keywords.comments) {
    tags[i++] = (str.size() > 0) ? str.c_str() : nullptr;
	}

	// パーサー作成
	m_parser = std::make_unique<CParser>(
						tags[0], tags[1], tags[2],
						flags[1], flags[2], flags[3], flags[0] );

	// ５行目以降：キーワードリスト
	for (const auto& word : keywords.keywords) {
		if (word.size() > 0) {
      const wchar_t* s = word.c_str();
			ULONG type = 0;
			if ('1' <= *s && *s <= '9' && *(s + 1) == ',') {
				type = *s - '1';
				s += 2;
			}
			m_parser->AddKeyword(s, type + CLR_KWD1);
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
