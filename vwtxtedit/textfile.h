////////////////////////////////////////////////////////////////////////
// textfile.h

#ifndef __TEXTFILE_H__
#define __TEXTFILE_H__


//========================================================================
//	利用可能文字コードリスト
//========================================================================

enum {
	T_EOF = 0,
	T_EOL = 1,
	T_EOB = 2
} state;

enum {
	CSF_SAVE = 1,
	CSF_LOAD = 2,
	CSF_BOTH = 3
};

typedef struct __CSINFO {
	int                 ID;
	const TCHAR*  longName;
	const TCHAR* shortName;
	int               type;
} CSINFO;

class CCharSetList : public CSimpleArray<CSINFO>
{
public:
	CCharSetList();
	void Enroll(int id, int index, int type);

	ULONG DefaultCharSetIndex() const;
	ULONG FindCharSetIndex( int cs ) const;
};


//=========================================================================
//	簡易ファイル読込
//	ファイルマッピングを用いるので、扱いが簡単でわりと高速です。
//	ただし困ったことに、4GBまでしか開けません。
//=========================================================================

class FileR
{
public:
	FileR()
		: m_hmap    ( NULL )
		, m_baseptr( NULL )
		, m_lasterror(0)
	{
	}

	~FileR()
	{
		Close();
	}

	//	開く
	//	@param fname ファイル名
	//	@return 開けたかどうか
	bool Open(const TCHAR* fname)
	{
		Close();

		// ファイルを読みとり専用で開く
		HANDLE handle = ::CreateFile(fname, GENERIC_READ, FILE_SHARE_READ|FILE_SHARE_WRITE,
								NULL, OPEN_EXISTING, FILE_FLAG_SEQUENTIAL_SCAN, NULL);
		m_lasterror = GetLastError();
		if (handle == INVALID_HANDLE_VALUE)
			return false;

		// サイズを取得
		m_size = ::GetFileSize(handle, NULL);

		if (m_size == 0) {
			// 0バイトのファイルはマッピング出来ないので適当に回避
			m_baseptr = &m_size;
		}
		else {
			// マッピングオブジェクトを作る
			m_hmap = ::CreateFileMapping(handle, NULL, PAGE_READONLY, 0, 0, NULL);
			m_lasterror = GetLastError();

			::CloseHandle(handle);
			if (m_hmap == NULL)
				return false;

			// ビュー
			m_baseptr = ::MapViewOfFile(m_hmap, FILE_MAP_READ, 0, 0, 0);
			m_lasterror = GetLastError();
			if (m_baseptr == NULL) {
				::CloseHandle(m_hmap);
				m_hmap = NULL;
				return false;
			}
		}
		return true;
	}

	//	閉じる
	void Close()
	{
		if (m_hmap) {
			::UnmapViewOfFile(m_baseptr);
			::CloseHandle(m_hmap);
			m_hmap = NULL;
		}
		m_baseptr = NULL;
	}

public:
	// ファイルサイズ
	inline ULONG size() const { return m_size; }

	// ファイル内容をマップしたアドレス取得
	inline const BYTE* base() const { return static_cast<const BYTE*>(m_baseptr); }

private:
	HANDLE      m_hmap;
	ULONG       m_size;
	const void* m_baseptr;
	DWORD		m_lasterror;
};

//=========================================================================
//	簡易ファイル書き込み
//	てきとーにバッファリングしつつ。
//=========================================================================

class FileW
{
public:

	FileW()
		: BUFSIZE( 65536 )
		, m_handle( INVALID_HANDLE_VALUE )
		, m_bufptr   ( new BYTE[BUFSIZE] )
		, m_lasterror(0)
	{
	}

	~FileW()
	{
		Close();
		delete [] m_bufptr;
	}

	// 開く
	bool Open( const TCHAR* fname, bool creat=true )
	{
		Close();

		// ファイルを書き込み専用で開く
		m_handle = ::CreateFile( fname, GENERIC_WRITE, FILE_SHARE_READ, NULL,
								creat ? CREATE_ALWAYS : OPEN_EXISTING,
								FILE_ATTRIBUTE_NORMAL | FILE_FLAG_SEQUENTIAL_SCAN, NULL );
		m_lasterror = GetLastError();
		if (m_handle == INVALID_HANDLE_VALUE) {
			m_lasterror = GetLastError();
			return false;
		}

		m_pos = 0;
		return true;
	}

	// 閉じる
	void Close()
	{
		if (m_handle != INVALID_HANDLE_VALUE) {
			Flush();
			::CloseHandle(m_handle);
			m_handle = INVALID_HANDLE_VALUE;
		}
	}

	// 書く
	void Write( const void* dat, ULONG siz )
	{
		if (siz == 0) return;

		const BYTE* udat = static_cast<const BYTE*>(dat);
		while ((BUFSIZE - m_pos) <= siz) {
			memmove(m_bufptr + m_pos, udat, BUFSIZE - m_pos);
			siz  -= (BUFSIZE - m_pos);
			udat += (BUFSIZE - m_pos);
			m_pos = BUFSIZE;
			Flush();
		}

		memmove(m_bufptr + m_pos, udat, siz);
		m_pos += siz;
	}

	// 一文字書く
	void WriteC( BYTE ch )
	{
		if ((BUFSIZE - m_pos) <= 1)
			Flush();

		m_bufptr[m_pos++] = ch;
	}

public:
	inline void Flush()
	{
		DWORD dummy;
		::WriteFile( m_handle, m_bufptr, m_pos, &dummy, NULL );
		m_lasterror = GetLastError();
		m_pos = 0;
	}

private:

	const int    BUFSIZE;
	HANDLE       m_handle;
	BYTE* const  m_bufptr;
	ULONG        m_pos;
	DWORD        m_lasterror;
};

//=========================================================================
//	利用可能コードセット
//
//	ただし、ここでリストアップされたもののうち、Windowsにちゃんと
//	言語サポートがインストールされているものだけが実際には対応可能。
//	値が-100より小さいコードは、そのすぐ上にあるコードページの言語
//	サポートを利用して変換を行うため、それに依存する。
//=========================================================================

enum charset {
	CS_UNKNOWN    = -1,
	CS_AUTODETECT = 0, 		// 自動判定
							// CS_SJIS/CS_EUCJP/CS_ISOJP/CS_ISOKR/CS_ISOCN
							// CS_UTF5/CS_UTF8/CS_UTF8N/CS_UTF16BEX/CS_UTF16LEX/CS_UTF32BEX/CS_UTF32LEX
							// を判定する。他は知らない。(^^;

	CS_WESTERN    = 1252,	// 欧米      (Windows1252 >> ISO-8859-1)
	CS_TURKISH    = 1254,	// トルコ語  (Windows1254 >> ISO-8859-9)
	CS_HEBREW     = 1255,	// ヘブライ語(Windows1255 >> ISO-8859-8)
	CS_ARABIC     = 1256,	// アラビア語(Windows1256 ～ ISO-8859-6)
	CS_BALTIC     = 1257,	// バルト語  (Windows1257 >> ISO-8859-13)
	CS_VIETNAMESE = 1258,	// ベトナム語(Windows1258 != VISCII)
	CS_CENTRAL    = 1250,	// 中央ﾖｰﾛｯﾊﾟ(Windows1250 ～ ISO-8859-2)
	CS_GREEK      = 1253,	// ギリシャ語(Windows1253 ～ ISO-8859-7)
	CS_THAI       = 874,	// タイ語

	CS_CYRILLIC   = 1251,	// キリル語(Windows1251 != ISO-8859-5)
#ifdef SUPPORT_ALL_CHARSET_ENCODE
	CS_KOI8R      = 20866,	// キリル語(KOI8-R)
	CS_KOI8U      = 21866,	// キリル語(KOI8-U ウクライナ系)
#endif

	CS_UHC        = 949,	// 韓国語１ (Unified Hangle Code >> EUC-KR)
#ifdef SUPPORT_ALL_CHARSET_ENCODE
	CS_ISOKR      = -950,	// 韓国語２ (ISO-2022-KR)
#endif
	CS_JOHAB      = 1361,	// 韓国語３ (CS_JOHAB)

	CS_GBK        = 936,	// 中国語１ (簡体字 CS_GBK >> EUC-CN)
#ifdef SUPPORT_ALL_CHARSET_ENCODE
	CS_ISOCN      = -936,	// 中国語２ (簡体字 ISO-2022-CN)
	CS_HZ         = -937,	// 中国語３ (簡体字 HZ-GB2312)
#endif
	CS_BIG5       = 950,	// 中国語４ (繁体字 CS_BIG5)

	CS_SJIS       = 932,	// 日本語１ (Shift_JIS)
	CS_EUCJP      = -932,	// 日本語２ (日本語EUC)
	CS_ISOJP      = -933,	// 日本語３ (ISO-2022-JP)

	CS_UTF7       = 65000,	// Unicode  (UTF-7)   : BOM無し
	CS_UTF8       =-65001,	// Unicode  (UTF-8)   : BOM有り
	CS_UTF8N      = 65001,	// Unicode  (UTF-8N)  : BOM無し
	CS_UTF16BEX   = -3,		// Unicode  (UTF-16)  : BOM有り BE
	CS_UTF16LEX   = -4,		// Unicode  (UTF-16)  : BOM有り LE
	CS_UTF16BE    = -5,		// Unicode  (UTF-16BE): BOM無し
	CS_UTF16LE    = -6,		// Unicode  (UTF-16LE): BOM無し
#ifdef SUPPORT_ALL_CHARSET_ENCODE
	CS_UTF5       = -2,		// Unicode  (UTF-5)   : BOM無し
	CS_UTF32BEX   = -7,		// Unicode  (UTF-32)  : BOM有り BE
	CS_UTF32LEX   = -8,		// Unicode  (UTF-32)  : BOM有り LE
	CS_UTF32BE    = -9,		// Unicode  (UTF-32BE): BOM無し
	CS_UTF32LE    = -10,	// Unicode  (UTF-32LE): BOM無し
#endif
	CS_DOSUS      = 437,	// DOSLatinUS (CP437)
};

//=========================================================================
//	改行コード
//=========================================================================

enum lbcode {
	CR   = 0,
	LF   = 1,
	CRLF = 2
};

struct TextFileRPimpl;


//=========================================================================
//	テキストファイル読込
//
//	ファイルを指定された文字コードで解釈し、Unicode文字列として
//	一行毎に返す。文字コードや改行コードの自動判定も可能。
//=========================================================================

class TextFileR
{
public:

	// コンストラクタ（コード指定）
	TextFileR(int charset = CS_AUTODETECT);

	~TextFileR();

	// 開く
	bool Open( const TCHAR* fname );

	// 閉じる
	void Close();

	//	読み込み (読んだ長さを返す)
	//	少なくとも20くらいのサイズを確保したバッファを指定してください。
	size_t ReadLine( WCHAR* buf, ULONG siz );

public:
	// 読んでるファイルのコードページ
	inline int codepage() const { return m_cs; }

	// 改行コード (0:CR, 1:LF, 2:CRLF)
	inline int linebreak() const { return m_lb; }

	// 読み込み状況 (0:EOF, 1:EOL, 2:EOB)
	int state() const;

	// ファイルサイズ
	inline ULONG size() const { return m_fp.size(); }

	// 改行が一個も見つからなかったフラグ
	inline bool nolb_found() const { return m_bNoLBFound; }

private:
	TextFileRPimpl* m_impl;
	FileR m_fp;
	int m_cs;
	int m_lb;
	bool m_bNoLBFound;

private:
	int AutoDetection( int cs, const BYTE* ptr, ULONG siz );
};


//=========================================================================
// テキストファイル出力共通インターフェイス
//=========================================================================

struct TextFileWPimpl
{
	virtual void WriteLine( const WCHAR* buf, ULONG siz )
	{
		if (siz > 0) {
			while (siz--)
				WriteChar(*buf++);
		}
	}

	virtual void WriteLB( const WCHAR* buf, ULONG siz )
	{
		WriteLine(buf, siz);
	}

	virtual void WriteChar( WCHAR ch )
	{}

	~TextFileWPimpl()
	{
		delete [] m_bufptr;
	}

protected:

	TextFileWPimpl( FileW& w )
		: m_fp   (w)
		, m_bufsize (65536)
		, m_bufptr  (new char[m_bufsize]) {}

	void ReserveMoreBuffer()
	{
		char* nBuf = new char[m_bufsize <<= 1];
		delete [] m_bufptr;
		m_bufptr = nBuf;
	}

	FileW& m_fp;
	ULONG  m_bufsize;
	char*  m_bufptr;
};


//=========================================================================
//	テキストファイル書込
//	Unicode文字列を受け取り、指定された文字コードに変換しながら出力する。
//=========================================================================

class TextFileW
{
public:
	// コンストラクタ（文字,改行コード指定）
	TextFileW(int charset, int linebreak)
		: m_cs(charset)
		, m_lb(linebreak)
		, m_impl(NULL)
	{
	}

	~TextFileW()
	{
		Close();
	}

	// 開く
	bool Open( const TCHAR* fname );

	// 閉じる
	void Close()
	{
		if (m_impl) {
			delete m_impl;
			m_impl = NULL;
		}
		m_fp.Close();
	}

	// 一行書き出し
	void WriteLine( const WCHAR* buf, ULONG siz, bool lastline )
	{
		if (siz > 0)
			m_impl->WriteLine( buf, siz );

		if (!lastline) {
			static const ULONG lbLst[] = {0x0D, 0x0A, 0x000A000D};
			static const ULONG lbLen[] = {   1,    1,          2};
			m_impl->WriteLB(reinterpret_cast<const WCHAR*>(&lbLst[m_lb]), lbLen[m_lb]);
		}
	}

private:
	TextFileWPimpl* m_impl;
	FileW           m_fp;
	const int       m_cs;
	const int       m_lb;
};


#endif		// __TEXTFILE_H__
