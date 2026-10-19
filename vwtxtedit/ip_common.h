////////////////////////////////////////////////////////////////////////
// ewcommon.h

#pragma once

// #define CURSOR_UNDERLINE


#ifndef _PAD_COMMON_H_
#define _PAD_COMMON_H_


//=========================================================================
//	テキスト中の位置情報
//=========================================================================

struct DPos
{
	//@{ バッファ中のアドレス (0～ ) //@}
	ULONG ad;

	//@{ 論理行番号 (0～ ) //@}
	ULONG tl;

	bool operator == ( const DPos& r ) const { return (tl==r.tl && ad==r.ad); }
	bool operator != ( const DPos& r ) const { return (tl!=r.tl || ad!=r.ad); }
	bool operator <  ( const DPos& r ) const { return (tl<r.tl || (tl==r.tl && ad<r.ad)); }
	bool operator >  ( const DPos& r ) const { return (tl>r.tl || (tl==r.tl && ad>r.ad)); }
	bool operator <= ( const DPos& r ) const { return (tl<r.tl || (tl==r.tl && ad<=r.ad)); }
	bool operator >= ( const DPos& r ) const { return (tl>r.tl || (tl==r.tl && ad>=r.ad)); }

	DPos( ULONG t, ULONG a ) : tl(t), ad(a) {}
	DPos() {}
};

//=========================================================================
//	表示位置情報まで含めたDPos
//=========================================================================

struct VPos : public DPos
{
	ULONG vl;		// ドキュメント全体の先頭からの論理行番号
	ULONG rl;		// 実際の１行中の何番目の折返し行かを示す行番号
	int   vx;		// スクロールを考慮しない仮想スクリーン上のx座標(pixel) 
	int   rx;		// 文字の並びに左右されてないx座標(pixel)
					//   == 長い行のしっぽから短い行に [↑] で移動して
					//   == その後 [↓] で戻れるようなアレです。
	void operator=( const DPos& dp ) { tl=dp.tl, ad=dp.ad; }

	VPos(bool) : DPos(0,0),vl(0),rl(0),vx(0),rx(0) {}
	VPos() {}
};


//=========================================================================
//	特殊文字を表す定数値
//=========================================================================

enum SpecialChars
{
	scEOF = 0,		// EOF
	scEOL = 1,		// 改行
	scTAB = 2,		// タブ
	scHSP = 3,		// 半角スペース
	scZSP = 4,		// 全角スペース
	scNUM = 5,		// 行番号
};



//=========================================================================
//	単語の種類を表す定数値
//=========================================================================

enum TokenType
{
	TOKEN_TAB = 0x00,			// Tab
	TOKEN_WSP = 0x04,			// 半角スペース
	TOKEN_ALP = 0x08,			// 普通の字
	TOKEN_CE  = 0x0c,			// コメント終了タグ
	TOKEN_CB  = 0x10,			// コメント開始タグ
	TOKEN_LB  = 0x14,			// 行コメント開始タグ
	TOKEN_QTS = 0x18,			// 単一引用符
	TOKEN_QTD = 0x1c			// 二重引用符
};

// 半角日本語
#define KHAN(a)			(0xff61 <= (a) && (a) <= 0xff9f)
// 全角数字
#define KDIGIT(a)		(0xff10 <= (a) && (a) <= 0xff19)
// 全角英字
#define KALPHA(a)		((0xff21 <= (a) && (a) <= 0xff3a) || (0xff41 <= (a) && (a) <= 0xff5a))
// 全角ひらがな
#define KHIRA(a)		(0x3041 <= (a) && (a) <= 0x3093)
// 全角カタカナ
#define KKATA(a)		(0x30a1 <= (a) && (a) <= 0x30fe)
// 全角漢字
#define KKANJI(a)		((0x4e00 <= (a) && (a) <= 0x9fa5) || (0xf929 <= (a) && (a) <= 0xfa2d))
// 全角罫線
#define KKEISEN(a)		(0x2500 <= (a) && (a) <= 0x256c)
// 全角空白
#define KSPACE(a)		((a) == L'　')

enum {
	KTYPE_INVALID = -1,
	KTYPE_UNKNOWN,
	KTYPE_HAN,
	KTYPE_DIGIT,
	KTYPE_ALPHA,
	KTYPE_HIRA,
	KTYPE_KATA,
	KTYPE_KANJI,
	KTYPE_KEISEN,
	KTYPE_SPACE,
};

//=========================================================================
//@{
//	色指定箇所を表す定数値
//@}
//=========================================================================

enum ColorType
{
/* ---
	TXT = 0,	// 文字色
	CMT = 1,	// コメント文字色
	KWD = 2,	// キーワード文字色
	//  = 3,	// ( コメントアウトされたキーワード文字色 )
	CTL = 4,	// 特殊文字色
	BG  = 5,	// 背景色
	LN  = 6,	// 行番号
	EOL = 7		// 改行

	CLR_TXT = 0,			// 文字色
	CLR_CMT = 1,			// コメント文字色
	CLR_KWD = 2,			// キーワード文字色
	CLR_CMTKWD = 3,			// ( コメントアウトされたキーワード文字色 )
	CLR_CTL = 4,			// 特殊文字色
	CLR_BG = 5,				// 背景色
	CLR_NUM = 6,			// 行番号
	CLR_EOL = 7,			// 改行
	CLR_RTX = 8,			// 選択時文字色
	CLR_RBG = 9,			// 選択時背景色
--- */
	CLR_TXT = 0,			// 文字色
	CLR_BG = 1,				// 背景色
	CLR_NUM = 2,			// 行番号
	CLR_EOL = 3,			// 改行
	CLR_CTL = 4,			// TAB/全角空白
	CLR_CUR = 5,			// カーソル行
	CLR_RTXT = 6,			// 選択時文字色
	CLR_RBG = 7,			// 選択時背景色
	CLR_DUM1 = 8,				// ダミー1
	CLR_DUM2 = 9,				// ダミー2
	CLR_CMT = 10,			// コメント
	CLR_KWD1 = 11,			// キーワード1
	CLR_KWD2 = 12,			// キーワード2
	CLR_KWD3 = 13,			// キーワード3
	CLR_KWD4 = 14,			// キーワード4
};


//=========================================================================
//@{
//	折り返し位置を示す定数値
//@}
//=========================================================================

enum WrapType {
	WRAP_NONE = -1,					// 折り返し無し
	WRAP_WINDOWEDGE = 0,			// 右端
	WRAP_CHARSIZE = 1,				// 指定文字数
};



//=========================================================================
//	表示設定
//
//	フォント・色・タブ幅・特殊文字の表示、の情報を保持。
//	ただし、強調単語の指定は Document に対して行う。
//=========================================================================

typedef struct tagVIEWCONFIG {
	LOGFONT m_logfont;							// フォント
	int m_tabstep;								// タブ幅文字数
	int m_widescroll;							// 
	int m_wraptype;								// 行の折り返し
	COLORREF m_color[15];						// 色
	bool m_sc[6];								// 特殊文字表示
	int m_linedenom;							// 行間
	bool m_blockcursor;							// カーソル形状
} VIEWCONFIG;

class VConfig : public VIEWCONFIG
{
public:
	VConfig()
	{
		InitialSetup();
		GetObject(AtlGetStockFont(OEM_FIXED_FONT), sizeof(m_logfont), &m_logfont);
		CClientDC dc(NULL);
		m_logfont.lfHeight = -::MulDiv(10, dc.GetDeviceCaps(LOGPIXELSY), 72);
	}

	// フォント関係初期化
	VConfig(const LOGFONT* plf)
	{
		InitialSetup();
		SetFont(plf);
	}

	VConfig(HFONT hFont)
	{
		InitialSetup();
		SetFont(hFont);
	}

	VConfig(const VIEWCONFIG& src)
	{
		CopyFrom(src);
	}

	void InitialSetup()
	{
		m_tabstep = 4;
		m_widescroll = 3;
		m_wraptype = WRAP_WINDOWEDGE;
		m_color[CLR_TXT]	= RGB(0xdf,0xdf,0xdf);		// 文字色
		m_color[CLR_BG ]	= RGB(0x1e,0x1e,0x1e);		// 背景色
		m_color[CLR_NUM]	= RGB(0x88,0x88,0x88);		// 行番号
		m_color[CLR_EOL]	= RGB(0x33,0xcc,0x33);		// 改行・EOF
		m_color[CLR_CTL]	= RGB(0x66,0x66,0x66);		// TAB/全角空白
#ifndef CURSOR_UNDERLINE
		m_color[CLR_CUR]  = RGB(0x33, 0x33, 0x33);	// カーソル行
#else
		m_color[CLR_CUR]	= RGB(0xee,0xee,0x33);		// カーソル行
#endif
		m_color[CLR_RTXT]	= RGB(0xff,0xff,0xff);		// 選択時文字色
		m_color[CLR_RBG]	= RGB(0x33,0x66,0x99);		// 選択時背景色
		m_color[CLR_DUM1]	= RGB(0xff,0xff,0xff);		// ダミー1
		m_color[CLR_DUM2]	= RGB(0xff,0xff,0xff);		// ダミー2
		m_color[CLR_CMT]	= RGB(0x56,0x98,0x1b);		// コメント行
		m_color[CLR_KWD1]	= RGB(0x71,0xb8,0xff);		// キーワード1
		m_color[CLR_KWD2]	= RGB(0xf5,0xe6,0x5a);		// キーワード2
		m_color[CLR_KWD3]	= RGB(0x33,0xcc,0xcc);		// キーワード3
		m_color[CLR_KWD4]	= RGB(0xcc,0x33,0xcc);		// キーワード4
		m_sc[scEOF]		= true;
		m_sc[scEOL]		= true;
		m_sc[scTAB]		= true;
		m_sc[scHSP]		= false;
		m_sc[scZSP]		= true;
		m_sc[scNUM]		= true;
		m_linedenom		= 3;								// 行間 1/3行
		m_blockcursor	= true;							// true = block
	}

	void CopyFrom(const VIEWCONFIG& src)
	{
		if (&src != this) {
			m_logfont = src.m_logfont;
			m_tabstep = src.m_tabstep;
			m_widescroll = src.m_widescroll;
			m_wraptype = src.m_wraptype;
			memcpy(m_color, src.m_color, sizeof(m_color));
			memcpy(m_sc, src.m_sc, sizeof(m_sc));
			m_linedenom = src.m_linedenom;
			m_blockcursor = src.m_blockcursor;
		}
	}

	inline VConfig& operator=(const VIEWCONFIG& src)
	{
		CopyFrom(src);
		return *this;
	}

	// フォント関係設定
	inline void SetFont(const LOGFONT* plf) { m_logfont = *plf; }

	void SetFont(HFONT hFont)
	{
		LOGFONT lf;
		if (GetObject(hFont, sizeof(lf), &lf))
			m_logfont = lf;
	}

	// タブ幅設定
	inline void SetTabStep(int tab) { m_tabstep = max(1, tab); }

	inline LOGFONT* Font() { return &m_logfont; };

};


#endif // _PAD_COMMON_H_
