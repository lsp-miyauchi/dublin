#include "stdafx.h"
// Windows SDK内のICUヘッダーをインクルード
#include <icu.h>
#include "textfile.h"


//------------------------------------------------------------------------
// 文字コードリスト
//------------------------------------------------------------------------

CCharSetList::CCharSetList()
{
	// 適宜登録
	int idx = 0;
	                               Enroll(CS_AUTODETECT, idx++, CSF_LOAD);
	if( ::IsValidCodePage(932) ) {
	                               Enroll(CS_SJIS,       idx++, CSF_LOAD | CSF_SAVE);
	                               Enroll(CS_EUCJP,      idx++, CSF_LOAD | CSF_SAVE);
	                               Enroll(CS_ISOJP,      idx++, CSF_LOAD | CSF_SAVE);
	}
#ifdef SUPPORT_ALL_CHARSET_ENCODE
	                               Enroll(CS_UTF5,       idx++, CSF_LOAD | CSF_SAVE);
#endif
	                               Enroll(CS_UTF7,       idx++, CSF_LOAD | CSF_SAVE);
	                               Enroll(CS_UTF8,       idx++, CSF_LOAD | CSF_SAVE);
	                               Enroll(CS_UTF8N,      idx++, CSF_LOAD | CSF_SAVE);
	                               Enroll(CS_UTF16BEX,   idx++, CSF_LOAD | CSF_SAVE);
	                               Enroll(CS_UTF16LEX,   idx++, CSF_LOAD | CSF_SAVE);
	                               Enroll(CS_UTF16BE,    idx++, CSF_LOAD | CSF_SAVE);
	                               Enroll(CS_UTF16LE,    idx++, CSF_LOAD | CSF_SAVE);
#ifdef SUPPORT_ALL_CHARSET_ENCODE
	                               Enroll(CS_UTF32BEX,   idx++, CSF_LOAD | CSF_SAVE);
	                               Enroll(CS_UTF32LEX,   idx++, CSF_LOAD | CSF_SAVE);
	                               Enroll(CS_UTF32BE,    idx++, CSF_LOAD | CSF_SAVE);
	                               Enroll(CS_UTF32LE,    idx++, CSF_LOAD | CSF_SAVE);
#endif
	                               Enroll(CS_WESTERN,    idx++, CSF_LOAD | CSF_SAVE);
	if( ::IsValidCodePage(28592) ) Enroll(CS_CENTRAL,    idx++, CSF_LOAD | CSF_SAVE);
	if( ::IsValidCodePage(949) ) {
	                               Enroll(CS_UHC,        idx++, CSF_LOAD | CSF_SAVE);
#ifdef SUPPORT_ALL_CHARSET_ENCODE
	                               Enroll(CS_ISOKR,      idx++, CSF_LOAD | CSF_SAVE);
#endif
	}
	if( ::IsValidCodePage(1361) )  Enroll(CS_JOHAB,      idx++, CSF_LOAD | CSF_SAVE);
	if( ::IsValidCodePage(936) ) {
	                               Enroll(CS_GBK,        idx++, CSF_LOAD | CSF_SAVE);
#ifdef SUPPORT_ALL_CHARSET_ENCODE
	                               Enroll(CS_ISOCN,      idx++, CSF_LOAD | CSF_SAVE);
	                               Enroll(CS_HZ   ,      idx++, CSF_LOAD | CSF_SAVE);
#endif
	}
	if( ::IsValidCodePage(950) )   Enroll(CS_BIG5 ,      idx++, CSF_LOAD | CSF_SAVE);
	if( ::IsValidCodePage(28595) ) Enroll(CS_CYRILLIC,   idx++, CSF_LOAD | CSF_SAVE);
#ifdef SUPPORT_ALL_CHARSET_ENCODE
	if( ::IsValidCodePage(20866) ) Enroll(CS_KOI8R,      idx++, CSF_LOAD | CSF_SAVE);
	if( ::IsValidCodePage(21866) ) Enroll(CS_KOI8U,      idx++, CSF_LOAD | CSF_SAVE);
#endif
	if( ::IsValidCodePage(874) )   Enroll(CS_THAI,       idx++, CSF_LOAD | CSF_SAVE);
	if( ::IsValidCodePage(1254) )  Enroll(CS_TURKISH,    idx++, CSF_LOAD | CSF_SAVE);
	if( ::IsValidCodePage(1257) )  Enroll(CS_BALTIC,     idx++, CSF_LOAD | CSF_SAVE);
	if( ::IsValidCodePage(1258) )  Enroll(CS_VIETNAMESE, idx++, CSF_LOAD | CSF_SAVE);
	if( ::IsValidCodePage(28597) ) Enroll(CS_GREEK,      idx++, CSF_LOAD | CSF_SAVE);
	                               Enroll(CS_DOSUS,      idx++, CSF_LOAD | CSF_SAVE);
}

void CCharSetList::Enroll(int id, int index, int type)
{
	static const TCHAR* const lnmJp[] = {
		_T("自動判定"),
		_T("日本語(ShiftJIS)"),
		_T("日本語(EUC)"),
		_T("日本語(ISO-2022-JP)"),
#ifdef SUPPORT_ALL_CHARSET_ENCODE
			_T("UTF-5"),
#endif
		_T("UTF-7"),
		_T("UTF-8"),
		_T("UTF-8N"),
		_T("UTF-16BE(BOM)"),
		_T("UTF-16LE(BOM)"),
		_T("UTF-16BE"),
		_T("UTF-16LE"),
#ifdef SUPPORT_ALL_CHARSET_ENCODE
			_T("UTF-32BE(BOM)"),
			_T("UTF-32LE(BOM)"),
			_T("UTF-32BE"),
			_T("UTF-32LE"),
#endif
		_T("欧米"),
		_T("中欧"),
		_T("韓国語(EUC-KR)"),
#ifdef SUPPORT_ALL_CHARSET_ENCODE
			_T("韓国語(ISO-2022-KR)"),
#endif
		_T("韓国語(Johab)"),
		_T("簡体字中国語(GB2312)"),
#ifdef SUPPORT_ALL_CHARSET_ENCODE
			_T("簡体字中国語(ISO-2022-CN)"),
			_T("簡体字中国語(HZ)"),
#endif
		_T("繁体字中国語(Big5)"),
		_T("キリル語(Windows)"),
#ifdef SUPPORT_ALL_CHARSET_ENCODE
			_T("キリル語(KOI8-R)"),
			_T("キリル語(KOI8-U)"),
#endif
		_T("タイ語"),
		_T("トルコ語"),
		_T("バルト語"),
		_T("ベトナム語"),
		_T("ギリシャ語"),
		_T("MSDOS(us)")
	};
	static const TCHAR* const snm[] = {
		_T(""),
		_T("SJIS"),
		_T("EUC"),
		_T("JIS"),
#ifdef SUPPORT_ALL_CHARSET_ENCODE
			_T("UTF5"),
#endif
		_T("UTF7"),
		_T("UTF8"),
		_T("UTF8"),
		_T("U16B"),
		_T("U16L"),
		_T("U16B"),
		_T("U16L"),
#ifdef SUPPORT_ALL_CHARSET_ENCODE
			_T("U32B"),
			_T("U32L"),
			_T("U32B"),
			_T("U32L"),
#endif
		_T("LTN1"),
		_T("LTN2"),
		_T("UHC"),
#ifdef SUPPORT_ALL_CHARSET_ENCODE
			_T("I2KR"),
#endif
		_T("Jhb"),
		_T("GBK"),
#ifdef SUPPORT_ALL_CHARSET_ENCODE
			_T("I2CN"),
			_T("HZ"),
#endif
		_T("BIG5"),
		_T("CYRL"),
#ifdef SUPPORT_ALL_CHARSET_ENCODE
			_T("KO8R"),
			_T("KO8U"),
#endif
		_T("THAI"),
		_T("TRK"),
		_T("BALT"),
		_T("VTNM"),
		_T("GRK"),
		_T("DOS")
	};

	CSINFO cs;
	cs.ID= id;
	cs.longName = lnmJp[index];
	cs.shortName = snm[index];
	cs.type = type;
	Add(cs);
}

ULONG CCharSetList::DefaultCharSetIndex() const
{
	return FindCharSetIndex(::GetACP());
}

ULONG CCharSetList::FindCharSetIndex( int cs ) const
{
	ULONG cnt = GetSize();
	for (ULONG i = 0; i < cnt; i++)
		if ((*this)[i].ID == cs)
			return i;
	return 0xffffffff;
}

//=========================================================================
// テキストファイル読み出し共通インターフェイス
//=========================================================================

struct TextFileRPimpl
{
	inline TextFileRPimpl()
		: state(T_EOL) {}

	virtual size_t ReadLine( WCHAR* buf, ULONG siz )
		= 0;

	int state;
};



//-------------------------------------------------------------------------
// Unicode系用のベースクラス
//	UTF-8以外はそんなに出会わないだろうから遅くてもよしとする。
//-------------------------------------------------------------------------

struct rBasicUTF : public TextFileRPimpl {
	virtual WCHAR PeekC() = 0;
	virtual WCHAR GetC() { WCHAR ch = PeekC(); Skip(); return ch; }
	virtual void Skip() = 0;
	virtual bool Eof() = 0;

	size_t ReadLine( WCHAR* buf, ULONG siz )
	{
		state = T_EOF;

		// 改行が出るまで読む
		WCHAR *w=buf, *e=buf+siz;
		while (!Eof()) {
			*w = GetC();
			if (*w == L'\r' || *w == L'\n') {
				state = T_EOL;
				break;
			}
			else if (*w != 0xfeff && ++w == e) {
				state = T_EOB;
				break;
			}
		}

		// 改行コードスキップ処理
		if (state == T_EOL) {
			if (*w == L'\r' && !Eof() && PeekC() == L'\n')
				Skip();
		}

		// 読んだ文字数
		return w - buf;
	}
};



//-------------------------------------------------------------------------
// UCS2ベタ/UCS4ベタ。それぞれUTF16, UTF32の代わりとして使う。
// ついでに同じtemplateで、ISO-8859-1も処理してしまう。^^;
//-------------------------------------------------------------------------

template<typename T>
struct rUCS : public rBasicUTF
{
	rUCS( const BYTE* b, ULONG s, bool bigendian )
		: fb( reinterpret_cast<const T*>(b) )
		, fe( reinterpret_cast<const T*>(b+(s/sizeof(T))*sizeof(T)) )
		, be( bigendian ) {}

	const T *fb, *fe;
	const bool be;

	// エンディアン変換
	inline  BYTE swap(  BYTE val ) { return val; }
	inline  WORD swap(  WORD val ) { return (val<<8) |(val>>8); }
	inline DWORD swap( DWORD val ) { return ((val>>24)|(val>>8)); }

	void Skip() { ++fb; }
	bool Eof() { return fb==fe; }
	WCHAR PeekC() { return (WCHAR)(be ? swap(*fb) : *fb); }
};

typedef rUCS<BYTE> rWest;
typedef rUCS<WORD> rUtf16;
typedef rUCS<DWORD> rUtf32;

//-------------------------------------------------------------------------
// UTF-5
//     0-  F : 1bbbb
//    10- FF : 1bbbb 0bbbb
//   100-FFF : 1bbbb 0bbbb 0bbbb
// というように、16進での一桁を一文字で表していくフォーマット。
// 各 0bbbb は '0', '1', ... '9', 'A', ... 'F'
// 各 1bbbb は 'G', 'H', ... 'P', 'Q', ... 'V' の字で表現。
//-------------------------------------------------------------------------

struct rUtf5 : public rBasicUTF
{
	rUtf5( const BYTE* b, ULONG s )
		: fb( b )
		, fe( b+s ) {}

	const BYTE *fb, *fe;

	// 16進文字から整数値へ変換
	inline byte conv( BYTE x )
	{
		if( '0'<=x && x<='9' ) return x-'0';
		else                   return x-'A'+0x0A;
	}

	void Skip() { do ++fb; while( fb<fe && *fb<'G' ); }
	bool Eof() { return fb==fe; }
	WCHAR PeekC()
	{
		WCHAR ch = (*fb-'G');
		for( const BYTE* p=fb+1; p<fe && *p<'G'; ++p )
			ch = (ch<<4)|conv(*p);
		return ch;
	}
};



//-------------------------------------------------------------------------
// UTF-7
//   ASCII範囲の字はそのまま。それ以外はUTF-16の値をbase64エンコード
//   して出力。エンコードされた部分は + と - で挟まれる。また '+' と
//   いう字自体を表現するために "+-" という形式を用いる。
//-------------------------------------------------------------------------

static const BYTE u7c[128]={
	0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
	0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
	0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0x3e,0xff,0xff,0xff,0x3f,
	0x34,0x35,0x36,0x37,0x38,0x39,0x3a,0x3b,0x3c,0x3d,0xff,0xff,0xff,0xff,0xff,0xff,
	0xff,0x00,0x01,0x02,0x03,0x04,0x05,0x06,0x07,0x08,0x09,0x0a,0x0b,0x0c,0x0d,0x0e,
	0x0f,0x10,0x11,0x12,0x13,0x14,0x15,0x16,0x17,0x18,0x19,0xff,0xff,0xff,0xff,0xff,
	0xff,0x1a,0x1b,0x1c,0x1d,0x1e,0x1f,0x20,0x21,0x22,0x23,0x24,0x25,0x26,0x27,0x28,
	0x29,0x2a,0x2b,0x2c,0x2d,0x2e,0x2f,0x30,0x31,0x32,0x33,0xff,0xff,0xff,0xff,0xff
};

struct rUtf7 : public rBasicUTF
{
	rUtf7( const BYTE* b, ULONG s )
		: fb( b )
		, fe( b+s )
		, rest( -1 )
		, inB64(false) { fillbuf(); }

	const BYTE *fb, *fe;
	WCHAR buf[3]; // b64を８文字毎に読んでバッファに溜めておく
	int rest;       // バッファの空き
	bool inB64;     // base64エリア内ならtrue

	void Skip() { if(--rest==0) fillbuf(); }
	bool Eof() { return fb==fe && rest==0; }
	WCHAR PeekC() { return buf[rest-1]; }

	void fillbuf()
	{
		if( fb<fe )
			if( !inB64 )
				if( *fb=='+' )
					if( fb+1<fe && fb[1]=='-' )
						rest=1, buf[0]=L'+', fb+=2;  // +-
					else
						++fb, inB64=true, fillbuf(); // 単独 +
				else
					rest=1, buf[0]=*fb++;            // 普通の字
			else
			{
				// 何文字デコードできるか数える
				int N=0, E=max(int(fb - fe), 8);
				while( N<E && fb[N]<0x80 && u7c[fb[N]]!=0xff )
					++N;

				// デコード
				buf[0]=buf[1]=buf[2]=0;
				switch( N )
				{
				case 8: buf[2]|= u7c[fb[7]];
				case 7: buf[2]|=(u7c[fb[6]]<< 6);
				case 6: buf[2]|=(u7c[fb[5]]<<12), buf[1]|=(u7c[fb[5]]>>4);
				case 5: buf[1]|=(u7c[fb[4]]<< 2);
				case 4: buf[1]|=(u7c[fb[3]]<< 8);
				case 3: buf[1]|=(u7c[fb[2]]<<14), buf[0]|=(u7c[fb[2]]>>2);
				case 2: buf[0]|=(u7c[fb[1]]<< 4);
				case 1: buf[0]|=(u7c[fb[0]]<<10);
					WCHAR t;
					rest = 1;
					if( N==8 )
						rest=3, t=buf[0], buf[0]=buf[2], buf[2]=t;
					else if( N>=6 )
						rest=2, t=buf[0], buf[0]=buf[1], buf[1]=t;
				}

				// 使った分進む
				if( N<E )
				{
					inB64=false;
					if( fb[N]=='-' )
						++fb;
				}
				fb += N;
				if( N==0 )
					fillbuf();
			}
	}
};



//-------------------------------------------------------------------------
// UTF8/MBCS
//  CR,LFが１バイト文字としてきちんと出てくるので、
//  切り分けが簡単な形式をここでまとめて扱う。UTF8以外の変換は
//  Windowsに全て任せている。
//-------------------------------------------------------------------------

typedef char* (WINAPI * uNextFunc)(WORD,const char*,DWORD);
typedef int (WINAPI * uConvFunc)(UINT,DWORD,const char*,int,wchar_t*,int);

static const byte mask[] = { 0, 0xff, 0x1f, 0x0f, 0x07, 0x03, 0x01 };
static inline int GetMaskIndex(BYTE n)
{
	if( BYTE(n+2) < 0xc2 ) return 1; // 00～10111111, fe, ff
	if( n          < 0xe0 ) return 2; // 110xxxxx
	if( n          < 0xf0 ) return 3; // 1110xxxx
	if( n          < 0xf8 ) return 4; // 11110xxx
	if( n          < 0xfc ) return 5; // 111110xx
	                        return 6; // 1111110x
}

static char* WINAPI CharNextUtf8( WORD, const char* p, DWORD )
{
	return const_cast<char*>( p+GetMaskIndex(BYTE(*p)) );
}


struct rMBCS : public TextFileRPimpl
{
	// ファイルポインタ＆コードページ
	const char* fb;
	const char* fe;
	const int   cp;
	uNextFunc next;
	uConvFunc conv;

	// 初期設定
	rMBCS( const BYTE* b, ULONG s, int c )
		: fb( reinterpret_cast<const char*>(b) )
		, fe( reinterpret_cast<const char*>(b+s) )
		, cp( c==CS_UTF8 ? CS_UTF8N : c )
		, next( cp==CS_UTF8N ?   CharNextUtf8 : CharNextExA )
		, conv( MultiByteToWideChar )
	{
		if( cp==CS_UTF8N && fe-fb>=3
		 && b[0]==0xef && b[1]==0xbb && b[2]==0xbf )
			fb += 3; // BOMスキップ
	}

	size_t ReadLine( WCHAR* buf, ULONG siz )
	{
		// バッファの終端か、ファイルの終端の近い方まで読み込む
		const char *p, *end = Min( fb+siz/2, fe );
		state = (end==fe ? T_EOF : T_EOB);

		// 改行が出るまで進む
		for( p=fb; p<end; )
			if( *p=='\r' || *p=='\n' )
			{
				state = T_EOL;
				break;
			}
			else if( (*p) & 0x80 )
			{
				p = next(cp,p,0);
			}
			else
			{
				++p;
			}

		// Unicodeへ変換
		ULONG len = ::MultiByteToWideChar( cp, 0, fb, int(p-fb), buf, siz );

		// 改行コードスキップ処理
		if( state == T_EOL )
			if( *(p++)=='\r' && p<fe && *p=='\n' )
				++p;
		fb = p;

		// 終了
		return len;
	}
};



//-------------------------------------------------------------------------
// ISO-2022 の適当な実装
//
//	コードページとして、G0, G1, G2, G3 の四面を持つ。
//	それぞれ決まったエスケープシーケンスによって、
//	さまざまな文字集合を各面に呼び出すことが出来る。
//	とりあえず現在のバージョンで対応しているふりなのは
//	次の通り。()内に、そのいい加減っぷりを示す。
//
//		<<いつでも>>
//		1B 28 42    : G0へ ASCII
//		1B 28 4A    : G0へ JIS X 0201 ローマ字 (のかわりにASCII)
//		1B 29 4A    : G1へ JIS X 0201 ローマ字 (のかわりにASCII)
//		1B 2A 4A    : G2へ JIS X 0201 ローマ字 (のかわりにASCII)
//		1B 3B 4A    : G3へ JIS X 0201 ローマ字 (のかわりにASCII)
//		1B 2E 41    : G2へ ISO-8859-1
//		<<CP932が有効な場合>>
//		1B 28 49    : G0へ JIS X 0201 カナ
//		1B 29 49    : G1へ JIS X 0201 カナ
//		1B 2A 49    : G2へ JIS X 0201 カナ
//		1B 2B 49    : G3へ JIS X 0201 カナ
//		1B 24 40    : G0へ JIS X 0208(1978)
//		1B 24 42    : G0へ JIS X 0208(1983)    (年度は区別しない)
//		<<CP936が有効な場合>>
//		1B 24 41    : G0へ GB 2312
//		1B 24 29 41 : G1へ GB 2312
//		1B 24 2A 41 : G2へ GB 2312
//		1B 24 2B 41 : G3へ GB 2312
//		<<CP949が有効な場合>>
//		1B 24 28 43 : G0へ KS X 1001
//		1B 24 29 43 : G1へ KS X 1001
//		1B 24 2A 43 : G2へ KS X 1001
//		1B 24 2B 43 : G3へ KS X 1001
//
//	各面に呼び出した文字集合は、
//		GL (0x21～0xfe) GR (0xa0～0xff)
//	のどちらかへマップすることで、実際のバイト値となる。
//	マップ命令となるバイト列は、次の通り
//
//		0F    : GL        へG0を呼び出し
//		0E    : GL        へG1を呼び出し
//		1B 7E : GR        へG1を呼び出し
//		8E    : GL/GR両方 へG2を一瞬だけ呼び出し。1B 4E も同義
//		8F    : GL/GR両方 へG3を一瞬だけ呼び出し。1B 4F も同義
//
//-------------------------------------------------------------------------

enum CodeSet { CD_ASCII, CD_LATIN, CD_KANA, CD_JIS, CD_KSX, CD_GB };

struct rIso2022 : public TextFileRPimpl
{
	// Helper: JIS X 0208 => SJIS
	void jis2sjis( BYTE k, BYTE t, char* s )
	{
		if(k>=0x3f)	s[0] = (char)(((k+1)>>1)+0xc0);
		else		s[0] = (char)(((k+1)>>1)+0x80);
		if( k&1 )	s[1] = (char)((t>>6) ? t+0x40 : t+0x3f);
		else		s[1] = (char)(t+0x9e);
	}

	// ファイルポインタ
	const BYTE* fb;
	const BYTE* fe;
	bool      fixed; // ESCによる切り替えを行わないならtrue
	bool    mode_hz; // CS_HZの場合。

	// 作業変数
	CodeSet *GL, *GR, G[4];
	int gWhat; // 次の字は 1:GL/GR 2:G2 3:G3 で出力
	ULONG len;

	// 初期化
	rIso2022( const BYTE* b, ULONG s, bool f, bool hz,
		CodeSet g0, CodeSet g1, CodeSet g2=CD_ASCII, CodeSet g3=CD_ASCII )
		: fb( b )
		, fe( b+s )
		, fixed( f )
		, mode_hz( hz )
		, GL( &G[0] )
		, GR( &G[1] )
		, gWhat( 1 )
	{
		G[0]=g0, G[1]=g1, G[2]=g2, G[3]=g3;
	}

	void DoSwitching( const BYTE*& p )
	{
		if( fixed )
		{
			if( p[0]==0x24 && p[1]!=0x40 && p[1]!=0x41 && p[1]!=0x42
			 && p+2 < fe && (p[2]==0x41 || p[2]==0x43) )
				++p;
		}
		else
		{
			if( p[1]==0x4A )
				G[ (p[0]-0x28)%4 ] = CD_ASCII;         // 1B [28-2B] 4A
			else if( p[1]==0x49 )
				G[ (p[0]-0x28)%4 ] = CD_KANA;          // 1B [28-2B] 49
			else if( *reinterpret_cast<const WORD*>(p)==0x4228 )
				G[ 0 ] = CD_ASCII;                     // 1B 28 42
			else if( *reinterpret_cast<const WORD*>(p)==0x412E )
				G[ 2 ] = CD_LATIN;                     // 1B 2E 41
			else if( p[0]==0x24 )
				if( p[1]==0x40 || p[1]==0x42 )
					G[ 0 ] = CD_JIS;                   // 1B 24 [40|42]
				else if( p[1]==0x41 )
					G[ 0 ] = CD_GB;                    // 1B 24 41
				else if( p+2 < fe )
					if( p[2]==0x41 )
						G[ ((*++p)-0x28)%4 ] = CD_GB;  // 1B 24 [28-2B] 41
					else if( p[2]==0x43 )
						G[ ((*++p)-0x28)%4 ] = CD_KSX; // 1B 24 [28-2B] 43
		}
		++p;
	}

	void DoOutput( WCHAR*& buf, const BYTE*& p )
	{
		// 文字集合取り出し
		CodeSet cs =
			(gWhat==2 ? G[2] : 
			(gWhat==3 ? G[3] :
			(*p&0x80  ? *GR  : *GL)));

		char c[2];
		ULONG wt=1;
		switch( cs )
		{
		case CD_ASCII:
			*buf = (*p)&0x7f;
			break;
		case CD_LATIN:
			*buf = (*p)|0x80;
			break;
		case CD_KANA:
			c[0] = (*p)|0x80;
			wt = ::MultiByteToWideChar(
				932, MB_PRECOMPOSED, c, 1, buf, 2 );
			break;
		case CD_GB:
		case CD_KSX:
			c[0] = (*  p)|0x80;
			c[1] = (*++p)|0x80;
			wt = ::MultiByteToWideChar(
				(cs==CD_GB?936:949), MB_PRECOMPOSED, c, 2, buf, 2 );
			break;
		case CD_JIS:
			jis2sjis( (p[0]&0x7f)-0x20, (p[1]&0x7f)-0x20, c );
			++p;
			wt = ::MultiByteToWideChar(
				932, MB_PRECOMPOSED, c, 2, buf, 2 );
			break;
		}
		buf+=wt;
		len+=wt;
	}

	size_t ReadLine( WCHAR* buf, ULONG siz )
	{
		len=0;

		// バッファの終端か、ファイルの終端の近い方まで読み込む
		const BYTE *p, *end = Min( fb+siz/2, fe );
		state = (end==fe ? T_EOF : T_EOB);

		// 改行が出るまで進む
		for( p=fb; p<end; ++p )
			switch( *p )
			{
			case '\r':
			case '\n': state =   T_EOL; goto outofloop;
			case 0x0F:    GL = &G[0]; break;
			case 0x0E:    GL = &G[1]; break;
			case 0x8E: gWhat =     2; break;
			case 0x8F: gWhat =     3; break;
			case 0x1B:
				if( p+1<fe ) {
					++p; if( *p==0x7E )    GR = &G[1];
					else if( *p==0x4E ) gWhat =  2;
					else if( *p==0x4F ) gWhat =  3;
					else if( p+1<fe )   DoSwitching(p);
				}break;
			case 0x7E: if( mode_hz && p+1<fe ) {
					++p; if( *p==0x7D ){ GL = &G[0]; break; }
					else if( *p==0x7B ){ GL = &G[1]; break; }
				} // fall through...
			default:   DoOutput( buf, p ); gWhat=1; break;
			}
		outofloop:

		// 改行コードスキップ処理
		if( state == T_EOL )
			if( *(p++)=='\r' && p<fe && *p=='\n' )
				++p;
		fb = p;

		// 終了
		return len;
	}
};



//-------------------------------------------------------------------------
// 自動判定などなど
//-------------------------------------------------------------------------

TextFileR::TextFileR( int charset )
	: m_cs( charset )
	, m_bNoLBFound(true)
	, m_impl(NULL)
{
}

TextFileR::~TextFileR()
{
	Close();

	if (m_impl) delete m_impl;
}

size_t TextFileR::ReadLine( WCHAR* buf, ULONG siz )
{
	return m_impl->ReadLine( buf, siz );
}

int TextFileR::state() const
{
	return m_impl->state;
}

void TextFileR::Close()
{
	m_fp.Close();
}

bool TextFileR::Open( const TCHAR* fname )
{
	// ファイルを開く
	if (!m_fp.Open(fname)) return false;

	if (m_impl) {
		delete m_impl;
		m_impl = NULL;
	}

	const BYTE* buf = m_fp.base();
	const ULONG  siz = m_fp.size();

	// 必要なら自動判定
	m_cs = AutoDetection( m_cs, buf, Min<ULONG>(siz,16<<10) ); // 先頭16KB

	// 対応するデコーダを作成
	switch( m_cs ) {
	case CS_WESTERN:	m_impl = new rWest(buf, siz, true); break;
	case CS_UTF16BEX:
	case CS_UTF16BE:	m_impl = new rUtf16(buf, siz, true); break;
	case CS_UTF16LEX:
	case CS_UTF16LE:	m_impl = new rUtf16(buf, siz, false); break;
#ifdef SUPPORT_ALL_CHARSET_ENCODE
	case CS_UTF5:		m_impl = new rUtf5(buf, siz); break;
	case CS_UTF32BEX:
	case CS_UTF32BE:	m_impl = new rUtf32(buf, siz, true); break;
	case CS_UTF32LEX:
	case CS_UTF32LE:	m_impl = new rUtf32(buf, siz, false); break;
#endif
	case CS_UTF7:		m_impl = new rUtf7(buf, siz); break;
	case CS_EUCJP:		m_impl = new rIso2022(buf, siz, true, false, CD_ASCII, CD_JIS, CD_KANA); break;
	case CS_ISOJP:		m_impl = new rIso2022(buf, siz, false, false, CD_ASCII, CD_KANA); break;
#ifdef SUPPORT_ALL_CHARSET_ENCODE
	case CS_ISOKR:		m_impl = new rIso2022(buf, siz, true, false, CD_ASCII, CD_KSX); break;
	case CS_ISOCN:		m_impl = new rIso2022(buf, siz, true, false, CD_ASCII, CD_GB); break;
	case CS_HZ:			m_impl = new rIso2022(buf, siz, true, true,  CD_ASCII, CD_GB); break;
#endif
	default:			m_impl = new rMBCS(buf, siz, m_cs); break;
	}

	return true;
}

int TextFileR::AutoDetection( int cs, const BYTE* ptr, ULONG siz )
{
// chardet.lib

//	if (cs == CS_AUTODETECT)
//		cs = cdlDetectCharset((const char*)ptr, (int)siz);

//-- まず、文字の出現回数の統計を取る

	ULONG i;
	int  freq[256];
	bool bit8 = false;
	memset(freq, 0, sizeof(freq) );
	for(i=0; i<siz; ++i ) {
		if( ptr[i] >= 0x80 )
			bit8 = true;
		++freq[ ptr[i] ];
	}

//-- 改行コード決定 (UTF16/32/7のとき問題あり。CS_UTF5に至っては判定不可…)

	     if( freq['\r'] > freq['\n']*2 ) m_lb = CR;
	else if( freq['\n'] > freq['\r']*2 ) m_lb = LF;
	else                                 m_lb = CRLF;
	m_bNoLBFound = freq['\r']==0 && freq['\n']==0;

//-- デフォルトコード

	int defCs = ::GetACP();

//-- 小さすぎる場合はここで終了

	if( siz <= 4 )
		return cs==CS_AUTODETECT ? defCs : cs;

//-- 明示指定がある場合はここで終了

	ULONG bom4 = (ptr[0]<<24) + (ptr[1]<<16) + (ptr[2]<<8) + (ptr[3]);
	ULONG bom2 = (ptr[0]<<8)  + (ptr[1]);

	if( cs==CS_UTF8 || cs==CS_UTF8N )
		cs = (bom4>>8==0xefbbbf ? CS_UTF8 : CS_UTF8N);
#ifdef SUPPORT_ALL_CHARSET_ENCODE
	else if( cs==CS_UTF32BEX || cs==CS_UTF32BE )
		cs = (bom4==0x0000feff ? CS_UTF32BEX : CS_UTF32BE);
	else if( cs==CS_UTF32LEX || cs==CS_UTF32LE )
		cs = (bom4==0xfffe0000 ? CS_UTF32LEX : CS_UTF32LE);
#endif
	else if( cs==CS_UTF16BEX || cs==CS_UTF16BE )
		cs = (bom2==0xfeff ? CS_UTF16BEX : CS_UTF16BE);
	else if( cs==CS_UTF16LEX || cs==CS_UTF16LE )
		cs = (bom2==0xfffe ? CS_UTF16LEX : CS_UTF16LE);

	if( cs != CS_AUTODETECT )
		return cs;

//-- BOMチェック・7bitチェック

	bool Jp = ::IsValidCodePage(932)!=FALSE;

	if( (bom4>>8) == 0xefbbbf )   cs = CS_UTF8;
#ifdef SUPPORT_ALL_CHARSET_ENCODE
	else if( bom4 == 0x0000feff ) cs = CS_UTF32BEX;
	else if( bom4 == 0xfffe0000 ) cs = CS_UTF32LEX;
#endif
	else if( bom2 == 0xfeff )     cs = CS_UTF16BEX;
	else if( bom2 == 0xfffe )     cs = CS_UTF16LEX;
#ifdef SUPPORT_ALL_CHARSET_ENCODE
	else if( bom4 == 0x1b242943 && ::IsValidCodePage(949) ) cs = CS_ISOKR;
	else if( bom4 == 0x1b242941 && ::IsValidCodePage(936) ) cs = CS_ISOCN;
#endif
	else if( Jp && !bit8 && freq[0x1b]>0 )                  cs = CS_ISOJP;

	if( cs != CS_AUTODETECT )
		return cs;

//-- UTF-5 チェック
#ifdef SUPPORT_ALL_CHARSET_ENCODE
	ULONG u5sum = 0;
	for( BYTE c='0'; c<='9'; ++c ) u5sum += freq[c];
	for( BYTE c='A'; c<='V'; ++c ) u5sum += freq[c];
	if( siz == u5sum )
		return CS_UTF5;
#endif

//-- 暫定版 UTF-8 / 日本語EUC チェック

	cs = defCs;

	// 改行コードがLFか、ある程度の大きさか、でないと
	// 無条件で ANSI-CP と見なしてしまう。
	if( bit8 && (siz>4096 || m_lb==1
	 || freq[0xfd]>0 || freq[0xfe]>0 || freq[0xff]>0 || freq[0x80]>0) )
	{
		// CS_UHCやCS_GBKはEUC-JPと非常に混同しやすいので、そっちがデフォルトの場合は
		// EUC-JP自動判定を切る
		if( Jp && ::GetACP()!=CS_UHC && ::GetACP()!=CS_GBK && ::GetACP()!=CS_BIG5 )
		{
			// EUCとしておかしい値が無いかチェック
			bool be=true;
			for( int k=0x90; k<=0xa0; ++k )if( freq[k]>0 ){be=false;break;}
			for( int k=0x7f; k<=0x8d; ++k )if( freq[k]>0 ){be=false;break;}
			if( be )
				return CS_EUCJP;
		}
		{
			// CS_UTF8として読めるかどうかチェック
			bool b8=true;
			int mi=1;
			for(i=0; i<siz && b8; ++i )
				if( --mi )
				{
					if( ptr[i]<0x80 || ptr[i]>=0xc0 )
						b8 = false;
				}
				else
				{
					mi = 1;
					if( ptr[i] > 0x7f )
					{
						mi = GetMaskIndex( ptr[i] );
						if( mi == 1 )//ptr[i] >= 0xfe )
							b8 = false;
					}
				}
			if( b8 )
				return CS_UTF8N;
		}
	}

//-- UTF-7 チェック
	static const char sszBase64[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

	const BYTE* bufptr = ptr;
	int level64 = 0;
	int nFit = 0;
	for (i = 0; i < siz; i++) {
		// +～-まではBASE64ENCODE
		if (*bufptr == _T('+')) {
			if (!level64) {
				level64++;
			}
			nFit++;
		}
		else {
			if (level64) {
				if (*bufptr == _T('-')) {
					nFit++;
					level64 = 0;
				}
				else if (StrChrA(sszBase64, *bufptr))
					nFit++;
				else {
					nFit = 0;
					break;
				}
			}
		}
		bufptr++;
	}

	if (nFit > 0)
		return CS_UTF7;

//-- 判定結果

	return cs;
}



//-------------------------------------------------------------------------
// Unicodeテキスト
//-------------------------------------------------------------------------

struct wUtf16LE : public TextFileWPimpl
{
	wUtf16LE( FileW& w, bool bom ) : TextFileWPimpl(w)
		{ if(bom){ WCHAR ch=0xfeff; m_fp.Write(&ch,2); } }
	void WriteLine( const WCHAR* buf, ULONG siz ) {m_fp.Write(buf,siz*2);}
};

struct wUtf16BE : public TextFileWPimpl
{
	wUtf16BE( FileW& w, bool bom ) : TextFileWPimpl(w)
		{ if(bom) WriteChar(0xfeff); }
	void WriteChar( WCHAR ch ) { m_fp.WriteC(ch>>8), m_fp.WriteC(ch&0xff); }
};

struct wUtf32LE : public TextFileWPimpl
{
	wUtf32LE( FileW& w, bool bom ) : TextFileWPimpl(w)
		{ if(bom) WriteChar(0xfeff); }
	void WriteChar( WCHAR ch )
		{ m_fp.WriteC(ch&0xff), m_fp.WriteC(ch>>8), m_fp.WriteC(0), m_fp.WriteC(0); }
};

struct wUtf32BE : public TextFileWPimpl
{
	wUtf32BE( FileW& w, bool bom ) : TextFileWPimpl(w)
		{ if(bom) WriteChar(0xfeff); }
	void WriteChar( WCHAR ch )
		{ m_fp.WriteC(0), m_fp.WriteC(0), m_fp.WriteC(ch>>8), m_fp.WriteC(ch&0xff); }
};

struct wWest : public TextFileWPimpl
{
	wWest( FileW& w ) : TextFileWPimpl(w) {}
	void WriteChar( WCHAR ch ) { m_fp.WriteC(ch>0xff ? '?' : (BYTE)ch); }
};

struct wUtf5 : public TextFileWPimpl
{
	wUtf5( FileW& w ) : TextFileWPimpl(w) {}
	void WriteChar( WCHAR ch )
	{
		static const char conv[] = { '0','1','2','3','4','5','6','7', '8','9','A','B','C','D','E','F' };
		if (ch < 0x10) {
			m_fp.WriteC(ch+'G');
		}
		else if (ch < 0x100) {
			m_fp.WriteC((ch>>4)+'G');
			m_fp.WriteC(conv[ch&0xf]);
		}
		else if (ch < 0x1000) {
			m_fp.WriteC((ch>>8)+'G');
			m_fp.WriteC(conv[(ch>>4)&0xf]);
			m_fp.WriteC(conv[ch&0xf]);
		}
		else {
			m_fp.WriteC((ch>>12)+'G');
			m_fp.WriteC(conv[(ch>>8)&0xf]);
			m_fp.WriteC(conv[(ch>>4)&0xf]);
			m_fp.WriteC(conv[ch&0xf]);
		}
	}
};

//-------------------------------------------------------------------------
// Windows頼りの変換
//-------------------------------------------------------------------------

struct wMBCS : public TextFileWPimpl
{
	wMBCS( FileW& w, int cp )
		: TextFileWPimpl(w), cp_(cp)
	{
		if( cp == CS_UTF8 )
		{
			// BOM書き込み
			cp_ = CS_UTF8N;
			m_fp.Write( "\xEF\xBB\xBF", 3 );
		}
	}

	void WriteLine( const WCHAR* str, ULONG len )
	{
		// WideCharToMultiByte API を利用した変換
		int r;
		while(
			0==(r=::WideCharToMultiByte(cp_,0,str,len,m_bufptr,m_bufsize,NULL,NULL))
		 && ::GetLastError()==ERROR_INSUFFICIENT_BUFFER )
			ReserveMoreBuffer();

		// ファイルへ書き込み
		m_fp.Write( m_bufptr, r );
	}

	int cp_;
};



//-------------------------------------------------------------------------
// ISO-2022 サブセットその１。
// ASCIIともう一つしか文字集合を使わないもの
//-------------------------------------------------------------------------

#ifdef SUPPORT_ALL_CHARSET_ENCODE

struct wIso2022 : public TextFileWPimpl
{
	wIso2022( FileW& w, int cp )
		: TextFileWPimpl(w), hz_(cp==CS_HZ)
	{
		switch( cp )
		{
		case CS_ISOKR:
			m_fp.Write( "\x1B\x24\x29\x43", 4 );
			cp_ = CS_UHC;
			break;

		case CS_ISOCN:
			m_fp.Write( "\x1B\x24\x29\x41", 4 );
			// fall through...

		default:
			cp_ = CS_GBK;
			break;
		}
	}

	void WriteLine( const WCHAR* str, ULONG len )
	{
		// まず WideCharToMultiByte API を利用して変換
		int r;
		while(
			0==(r=::WideCharToMultiByte(cp_,0,str,len,m_bufptr,m_bufsize,NULL,NULL))
		 && ::GetLastError()==ERROR_INSUFFICIENT_BUFFER )
			ReserveMoreBuffer();

		bool ascii = true;
		for( int i=0; i<r; ++i )
			if( m_bufptr[i] & 0x80 )
			{
				// 非ASCII部分は最上位ビットを落としてから出力
				if( ascii )
				{
					if( hz_ )
						m_fp.WriteC( 0x7E ), m_fp.WriteC( 0x7B );
					else
						m_fp.WriteC( 0x0E );
					ascii = false;
				}
				m_fp.WriteC( m_bufptr[i++] & 0x7F );
				m_fp.WriteC( m_bufptr[i]   & 0x7F );
			}
			else
			{
				// ASCII部分はそのまま出力
				if( !ascii )
				{
					if( hz_ )
						m_fp.WriteC( 0x7E ), m_fp.WriteC( 0x7D );
					else
						m_fp.WriteC( 0x0F );
					ascii = true;
				}
				m_fp.WriteC( m_bufptr[i] );

				// ただしCS_HZの場合、0x7E は 0x7E 0x7E と表す
				if( hz_ && m_bufptr[i]==0x7E )
					m_fp.WriteC( 0x7E );
			}

		// 最後は確実にASCIIに戻す
		if( !ascii )
			if( hz_ )
				m_fp.WriteC( 0x7E ), m_fp.WriteC( 0x7D );
			else
				m_fp.WriteC( 0x0F );
	}

	int  cp_;
	bool hz_;
};

#endif



//-------------------------------------------------------------------------
// ISO-2022 サブセットその２。日本語EUC
//-------------------------------------------------------------------------

// Helper: SJIS ==> JIS X 0208
static void sjis2jis( BYTE s1, BYTE s2, char* k )
{
	if( ((s1==0xfa || s1==0xfb) && s2>=0x40) || (s1==0xfc && (s2&0xf0)==0x40) )
	{
		// IBM外字のマッピング
static const WORD IBM_sjis2kuten[] = {
/*fa40*/0x5c51,0x5c52,0x5c53,0x5c54,0x5c55,0x5c56,0x5c57,0x5c58,0x5c59,0x5c5a,0x0d15,0x0d16,0x0d17,0x0d18,0x0d19,0x0d1a,
/*fa50*/0x0d1b,0x0d1c,0x0d1d,0x0d1e,0x022c,0x5c5c,0x5c5d,0x5c5e,0x0d4a,0x0d42,0x0d44,0x0248,0x5901,0x5902,0x5903,0x5904,
/*fa60*/0x5905,0x5906,0x5907,0x5908,0x5909,0x590a,0x590b,0x590c,0x590d,0x590e,0x590f,0x5910,0x5911,0x5912,0x5913,0x5914,
/*fa70*/0x5915,0x5916,0x5917,0x5918,0x5919,0x591a,0x591b,0x591c,0x591d,0x591e,0x591f,0x5920,0x5921,0x5922,0x5923,/**/0x0109,
/*fa80*/0x5924,0x5925,0x5926,0x5927,0x5928,0x5929,0x592a,0x592b,0x592c,0x592d,0x592e,0x592f,0x5930,0x5931,0x5932,0x5933,
/*fa90*/0x5934,0x5935,0x5936,0x5937,0x5938,0x5939,0x593a,0x593b,0x593c,0x593d,0x593e,0x593f,0x5940,0x5941,0x5942,0x5943,
/*faa0*/0x5944,0x5945,0x5946,0x5947,0x5948,0x5949,0x594a,0x594b,0x594c,0x594d,0x594e,0x594f,0x5950,0x5951,0x5952,0x5953,
/*fab0*/0x5954,0x5955,0x5956,0x5957,0x5958,0x5959,0x595a,0x595b,0x595c,0x595d,0x595e,0x5a01,0x5a02,0x5a03,0x5a04,0x5a05,
/*fac0*/0x5a06,0x5a07,0x5a08,0x5a09,0x5a0a,0x5a0b,0x5a0c,0x5a0d,0x5a0e,0x5a0f,0x5a10,0x5a11,0x5a12,0x5a13,0x5a14,0x5a15,
/*fad0*/0x5a16,0x5a17,0x5a18,0x5a19,0x5a1a,0x5a1b,0x5a1c,0x5a1d,0x5a1e,0x5a1f,0x5a20,0x5a21,0x5a22,0x5a23,0x5a24,0x5a25,
/*fae0*/0x5a26,0x5a27,0x5a28,0x5a29,0x5a2a,0x5a2b,0x5a2c,0x5a2d,0x5a2e,0x5a2f,0x5a30,0x5a31,0x5a32,0x5a33,0x5a34,0x5a35,
/*faf0*/0x5a36,0x5a37,0x5a38,0x5a39,0x5a3a,0x5a3b,0x5a3c,0x5a3d,0x5a3e,0x5a3f,0x5a40,0x5a41,0x5a42,/**/0x0109,0x0109,0x0109,
/*fb40*/0x5a43,0x5a44,0x5a45,0x5a46,0x5a47,0x5a48,0x5a49,0x5a4a,0x5a4b,0x5a4c,0x5a4d,0x5a4e,0x5a4f,0x5a50,0x5a51,0x5a52,
/*fb50*/0x5a53,0x5a54,0x5a55,0x5a56,0x5a57,0x5a58,0x5a59,0x5a5a,0x5a5b,0x5a5c,0x5a5d,0x5a5e,0x5b01,0x5b02,0x5b03,0x5b04,
/*fb60*/0x5b05,0x5b06,0x5b07,0x5b08,0x5b09,0x5b0a,0x5b0b,0x5b0c,0x5b0d,0x5b0e,0x5b0f,0x5b10,0x5b11,0x5b12,0x5b13,0x5b14,
/*fb70*/0x5b15,0x5b16,0x5b17,0x5b18,0x5b19,0x5b1a,0x5b1b,0x5b1c,0x5b1d,0x5b1e,0x5b1f,0x5b20,0x5b21,0x5b22,0x5b23,/**/0x0109,
/*fb80*/0x5b24,0x5b25,0x5b26,0x5b27,0x5b28,0x5b29,0x5b2a,0x5b2b,0x5b2c,0x5b2d,0x5b2e,0x5b2f,0x5b30,0x5b31,0x5b32,0x5b33,
/*fb90*/0x5b34,0x5b35,0x5b36,0x5b37,0x5b38,0x5b39,0x5b3a,0x5b3b,0x5b3c,0x5b3d,0x5b3e,0x5b3f,0x5b40,0x5b41,0x5b42,0x5b43,
/*fba0*/0x5b44,0x5b45,0x5b46,0x5b47,0x5b48,0x5b49,0x5b4a,0x5b4b,0x5b4c,0x5b4d,0x5b4e,0x5b4f,0x5b50,0x5b51,0x5b52,0x5b53,
/*fbb0*/0x5b54,0x5b55,0x5b56,0x5b57,0x5b58,0x5b59,0x5b5a,0x5b5b,0x5b5c,0x5b5d,0x5b5e,0x5c01,0x5c02,0x5c03,0x5c04,0x5c05,
/*fbc0*/0x5c06,0x5c07,0x5c08,0x5c09,0x5c0a,0x5c0b,0x5c0c,0x5c0d,0x5c0e,0x5c0f,0x5c10,0x5c11,0x5c12,0x5c13,0x5c14,0x5c15,
/*fbd0*/0x5c16,0x5c17,0x5c18,0x5c19,0x5c1a,0x5c1b,0x5c1c,0x5c1d,0x5c1e,0x5c1f,0x5c20,0x5c21,0x5c22,0x5c23,0x5c24,0x5c25,
/*fbe0*/0x5c26,0x5c27,0x5c28,0x5c29,0x5c2a,0x5c2b,0x5c2c,0x5c2d,0x5c2e,0x5c2f,0x5c30,0x5c31,0x5c32,0x5c33,0x5c34,0x5c35,
/*fbf0*/0x5c36,0x5c37,0x5c38,0x5c39,0x5c3a,0x5c3b,0x5c3c,0x5c3d,0x5c3e,0x5c3f,0x5c40,0x5c41,0x5c42,/**/0x0109,0x0109,0x0109,
/*fc40*/0x5c43,0x5c44,0x5c45,0x5c46,0x5c47,0x5c48,0x5c49,0x5c4a,0x5c4b,0x5c4c,0x5c4d,0x5c4e,/**/0x0109,0x0109,0x0109,0x0109,
};
		k[0] = IBM_sjis2kuten[ (s1-0xfa)*12*16 + (s2-0x40) ]>>8;
		k[1] = IBM_sjis2kuten[ (s1-0xfa)*12*16 + (s2-0x40) ]&0xff;
	}
	else
	{
		// その他
		if( s2>=0x9f )
		{
			if( s1>=0xe0 ) k[0] = ((s1-0xc0)<<1);
			else           k[0] = ((s1-0x80)<<1);
			k[1] = s2-0x9e;
		}
		else
		{
			if( s1>=0xe0 ) k[0] = ((s1-0xc0)<<1)-1;
			else           k[0] = ((s1-0x80)<<1)-1;

			if( s2 & 0x80 )	k[1] = s2-0x40;
			else			k[1] = s2-0x3f;
		}
	}

	k[0] += 0x20;
	k[1] += 0x20;
}

struct wEucJp : public TextFileWPimpl
{
	wEucJp( FileW& w )
		: TextFileWPimpl(w) {}

	void WriteLine( const WCHAR* str, ULONG len )
	{
		// まず WideCharToMultiByte API を利用して変換
		int r;
		while(
			0==(r=::WideCharToMultiByte(932,0,str,len,m_bufptr,m_bufsize,NULL,NULL))
		 && ::GetLastError()==ERROR_INSUFFICIENT_BUFFER )
			ReserveMoreBuffer();

		for( int i=0; i<r; ++i )
			if( m_bufptr[i] & 0x80 )
			{
				if( 0xA1<=(BYTE)m_bufptr[i] && (BYTE)m_bufptr[i]<=0xDF )
				{
					// カナ
					m_fp.WriteC( 0x8E );
					m_fp.WriteC( m_bufptr[i] );
				}
				else
				{
					// JIS X 0208
					sjis2jis( m_bufptr[i], m_bufptr[i+1], m_bufptr+i );
					m_fp.WriteC( m_bufptr[i++] | 0x80 );
					m_fp.WriteC( m_bufptr[i]   | 0x80 );
				}
			}
			else
			{
				// ASCII部分はそのまま出力
				m_fp.WriteC( m_bufptr[i] );
			}
	}
};



//-------------------------------------------------------------------------
// ISO-2022 サブセットその３。ISO-2022-JP
//-------------------------------------------------------------------------

struct wIsoJp : public TextFileWPimpl
{
	wIsoJp( FileW& w )
		: TextFileWPimpl(w)
	{
		m_fp.Write( "\x1b\x28\x42", 3 );
	}

	void WriteLine( const WCHAR* str, ULONG len )
	{
		// まず WideCharToMultiByte API を利用して変換
		int r;
		while(
			0==(r=::WideCharToMultiByte(932,0,str,len,m_bufptr,m_bufsize,NULL,NULL))
		 && ::GetLastError()==ERROR_INSUFFICIENT_BUFFER )
			ReserveMoreBuffer();

		enum { ROMA, KANJI, CD_KANA } state = ROMA;
		for( int i=0; i<r; ++i )
			if( m_bufptr[i] & 0x80 )
			{
				if( 0xA1<=(BYTE)m_bufptr[i] && (BYTE)m_bufptr[i]<=0xDF )
				{
					// カナ
					if( state != CD_KANA )
						m_fp.Write( "\x1b\x28\x49", 3 ), state = CD_KANA;
					m_fp.WriteC( m_bufptr[i] & 0x7f );
				}
				else
				{
					// JIS X 0208
					if( state != KANJI )
						m_fp.Write( "\x1b\x24\x42", 3 ), state = KANJI;
					sjis2jis( m_bufptr[i], m_bufptr[i+1], m_bufptr+i );
					m_fp.WriteC( m_bufptr[i++] );
					m_fp.WriteC( m_bufptr[i]   );
				}
			}
			else
			{
				// ASCII部分はそのまま出力
				if( state != ROMA )
					m_fp.Write( "\x1b\x28\x42", 3 ), state = ROMA;
				m_fp.WriteC( m_bufptr[i] );
			}

		if( state != ROMA )
			m_fp.Write( "\x1b\x28\x42", 3 ), state = ROMA;
	}
};


////////////////////////////////////////////////////////////////////////
// TextFileW メンバー

bool TextFileW::Open( const TCHAR* fname )
{
	if (!m_fp.Open(fname, true))
		return false;

	if (m_impl) {
		delete m_impl;
		m_impl = NULL;
	}
	switch( m_cs ) {
	case CS_WESTERN:	m_impl = new wWest( m_fp ); break;
	case CS_UTF16LEX:
	case CS_UTF16LE:	m_impl = new wUtf16LE( m_fp, m_cs==CS_UTF16LEX ); break;
	case CS_UTF16BEX:
	case CS_UTF16BE:	m_impl = new wUtf16BE( m_fp, m_cs==CS_UTF16BEX ); break;
#ifdef SUPPORT_ALL_CHARSET_ENCODE
	case CS_UTF5:		m_impl = new wUtf5( m_fp ); break;
	case CS_UTF32LEX:
	case CS_UTF32LE:	m_impl = new wUtf32LE( m_fp, m_cs==CS_UTF32LEX ); break;
	case CS_UTF32BEX:
	case CS_UTF32BE:	m_impl = new wUtf32BE( m_fp, m_cs==CS_UTF32BEX ); break;
#endif
	case CS_EUCJP:		m_impl = new wEucJp( m_fp ); break;
	case CS_ISOJP:		m_impl = new wIsoJp( m_fp ); break;
#ifdef SUPPORT_ALL_CHARSET_ENCODE
	case CS_ISOKR:		m_impl = new wIso2022( m_fp, m_cs ); break;
	case CS_ISOCN:		m_impl = new wIso2022( m_fp, m_cs ); break;
	case CS_HZ:			m_impl = new wIso2022( m_fp, m_cs ); break;
#endif
	case CS_UTF8:
	case CS_UTF8N:
	default:			m_impl = new wMBCS( m_fp, m_cs ); break;
	}
	return true;
}
