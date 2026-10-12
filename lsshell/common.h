//////////////////////////////////////////////////////////////////////
// common.h

#pragma once
#ifndef __LSPSHELL_COMMON_H__
#define __LSPSHELL_COMMON_H__

//----------------------------------------------
// MessageBox ACTION
//
#define MB_INFORMATION	(MB_OK | MB_ICONINFORMATION)
#define MB_ATTENTION	(MB_OK | MB_ICONEXCLAMATION)
#define MB_QUESTION		(MB_YESNOCANCEL | MB_ICONQUESTION)
#define MB_QYESNO		(MB_YESNO | MB_ICONQUESTION)
#define MB_QOKCANCEL	(MB_OKCANCEL | MB_ICONEXCLAMATION)
#define MB_QABORT		(MB_ICONEXCLAMATION | MB_ABORTRETRYIGNORE)
#define MB_HAND			(MB_OKCANCEL | MB_ICONHAND)
#define MB_STOP			(MB_OK | MB_ICONSTOP)
#define MB_QAST			(MB_OKCANCEL | MB_ICONASTERISK)
#define MB_QEXCM		(MB_OKCANCEL | MB_ICONEXCLAMATION)
#define MB_QRETRY		(MB_RETRYCANCEL | MB_ICONEXCLAMATION)



#define _ISKEYDOWN(vkey)	((::GetKeyState((vkey)) & 0x8000) != 0)

enum {
	INSTALL_NOUSER = -1,
	INSTALL_UNREGIST,
	INSTALL_REGIST,
};

enum {
	USRL_NOCHECK = -1,
	USRL_NOUSER,
	USRL_VALID,
	USRL_TRIAL,
	USRL_EXPIRE,
};

#define MAXCMDRANGE			32

// アーカイバ書庫関連
#define SHLA_PACK			0x00000001
#define SHLA_CHECK			0x00000002
#define SHLA_UNPACK			0x00000004
#define SHLA_DROPPACK		0x00000010
#define SHLA_DROPUNPACK		0x00000020
#define SHLA_LZH			0x00000100
#define SHLA_ZIP			0x00000200
#define SHLA_CAB			0x00000400
#define SHLA_ARJ			0x00000800
#define SHLA_TAR			0x00001000
#define SHLA_ISH			0x00002000
#define SHLA_RAR			0x00004000
#define SHLA_BGA			0x00008000
#define SHLA_YZ1			0x00010000
#define SHLA_GCA			0x00020000
#define SHLA_7Z				0x00040000

// その他シェル関連
#define SHLE_SHORTCUT		0x00000001
#define SHLE_FV				0x00000002		// FileVisorの起動
#define SHLE_SYNC			0x00000004		// フォルダの同期
#define SHLE_ICONDUMP		0x00000008		// アイコン一覧
#define SHLE_BDUMP			0x00000010		// バイナリダンプ
#define SHLE_ATTR			0x00000020		// 属性の変更
#define SHLE_DIV			0x00000040		// ファイルの分割
#define SHLE_DFLUSH			0x00000080		// ディスクフラッシュ
#define SHLE_RENAME			0x00000100		// ファイル名の置換
#define SHLE_FILTERDEL		0x00000400		// 一括削除
#define SHLE_FSIZE			0x00000800		// ファイル容量の詳細
#define SHLE_DRVINFO		0x00001000		// ドライブ情報
#define SHLE_FILEINFO		0x00002000		// ファイル情報
#define SHLE_PATHCLIP		0x00004000		// パス名のコピー
#define SHLE_DROPREPARSE	0x00008000		// リパース ポイント作成
#define SHLE_DROPCOPYTO		0x00010000		// ここへFileVisorでコピー
#define SHLE_CRYPT			0x00020000		// 暗号・復号化
#define SHLE_DROPCRYPTTO	0x00040000		// ここへ暗号・復号化
#define SHLE_DROPSYMLINK	0x00080000		// ここシンボリック リンクを作成
#define SHLE_CALCHASH		0x00100000		// ハッシュを計算
#define SHLE_SHREDDER		0x00200000		// シュレッダ
#define SHLE_DROPICONPICK	0x00400000		// アイコン抽出
#define SHLE_PROPICON		0x00800000		// プロパティにアイコン一覧
#define SHLE_DROPSYSLINK	0x01000000		// ここシステム リンクを作成
#define SHLE_SUBMENU		0x80000000		// サブメニュー化

// イメージツール
#define SHLI_COPY           0x00000001		// クリップボードヘコピー
#define SHLI_CONVERT        0x00000002		// イメージ変換
#define SHLI_ALBUM          0x00000004		// WEBアルバム
#define SHLI_RENAME         0x00000008		// 撮影日時でリネーム
#define SHLI_THUMBOUT       0x00000010		// サムネイル抽出
#define SHLI_SLIDESHOW      0x00000020		// スライドショウ
#define SHLI_JPEG2PDF       0x00000040		// Jpeg2PDF
#define SHLI_ORIENTATION    0x00000080		// Exifオリエンテーション
#define SHLI_MYTOOLS        0x00000100		// イメージ分割（私用ですわな）
#define SHLI_DROPTHUMBOUT   0x00001000		// ここにサムネイルを抽出
#define SHLI_CROPIMAGE		0x00010000		// 四辺をクロップ
#define SHLI_PROPEXIF       0x01000000		// プロパティにEXIF情報


#define MF_SHELLMENU		(MF_STRING | MF_BYPOSITION | MF_OWNERDRAW)

/////////////////////////////////////////////////////////////////////
//
class _RES_STRING
{
private:
	LPTSTR m_strAlloc;
public:
	LPCTSTR m_str;

public:
	_RES_STRING(LPCTSTR pstr)
	{
		m_strAlloc = NULL;
		m_str = pstr;
	}

	_RES_STRING(UINT id)
	{
		m_strAlloc = _loadstring(id);
		m_str = m_strAlloc;
	}

	~_RES_STRING()
	{
		if (m_strAlloc) delete [] m_strAlloc;
	}

	LPTSTR _loadstring(UINT id)
	{
		LPTSTR pstr = NULL;
		HINSTANCE hInst = RESINST;
		for(int len = 256; ; len *= 2) {
			pstr = new TCHAR[len];
			if (pstr == NULL) return NULL;

			int res = ::LoadString(hInst, id, pstr, len);
			if (res < len - 1) break;

			delete [] pstr;
			pstr = NULL;
		}
		return pstr;
	}

	operator LPCTSTR() const { return m_str; }
};

/////////////////////////////////////////////////////////////////////
// CDirStack

class CDirStack
{
private:
	TCHAR m_szPath[MAX_PATH * 2];
public:
	CDirStack() { GetCurrentDirectory(ARRAYSIZE(m_szPath), m_szPath); }
	~CDirStack() { SetCurrentDirectory(m_szPath); }
};

/////////////////////////////////////////////////////////////////////
// CEnterFolder

class CEnterFolder
{
private:
	CDirStack m_stack;
public:
	CEnterFolder() { }
	~CEnterFolder() { }
	__inline BOOL Enter(LPCTSTR pszPath) { return SetCurrentDirectory(pszPath); }
};

//////////////////////////////////////////////////////////////////////
// メモリアロケート用

template <class T>
class CEasyArray
{
private:
	T* m_aT;
	int m_size;
public:
	__inline CEasyArray(int size) { m_size = size; m_aT = (T*)malloc(sizeof(T) * size); }
	__inline ~CEasyArray() { if (m_aT) free(m_aT); }
	__inline operator T*() const { return m_aT; }
	bool SetAt(int index, T t)
	{
		ATLASSERT(index >= 0 && index < m_size);
		if (0 <= index && index < m_size) {
			m_aT[index] = t;
			return true;
		}
		_AtlRaiseException(EXCEPTION_ARRAY_BOUNDS_EXCEEDED);
		return false;
	}
	const T& operator[] (int index) const
	{
		ATLASSERT(index >= 0 && index < m_size);
		if (0 <= index && index < m_size)
			return m_aT[index];

		_AtlRaiseException(EXCEPTION_ARRAY_BOUNDS_EXCEEDED);
	}
	T& operator[] (int index)
	{
		ATLASSERT(index >= 0 && index < m_size);
		if (0 <= index && index < m_size)
			return m_aT[index];

		_AtlRaiseException(EXCEPTION_ARRAY_BOUNDS_EXCEEDED);
	}
};

/////////////////////////////////////////////////////////////////////
//

int AppMessage(ATL::_U_STRINGorID title, ATL::_U_STRINGorID mes, LPCTSTR pszInsert=NULL, UINT nType=MB_STOP);
BOOL FileBrowse(BOOL bRead, HWND hwnd, _RES_STRING caption, _RES_STRING filter, LPTSTR pszInitialDir, LPTSTR pszName);
int PathComparePath(LPCTSTR pathParent, LPCTSTR pathChild);
BOOL CompressFiles(HWND hParent, LPCTSTR pszPath, HDROP hDrop);
BOOL ExtractFiles(HWND hParent, LPCTSTR pszPath, HDROP hDrop);
BOOL PackAddFiles(HWND hParent, LPCTSTR pszArc, LPCTSTR pszBase, HDROP hdrop);
BOOL MakeSFX(HWND hParent, LPCTSTR pszArc, LPCTSTR pszPath);
BOOL CheckArchive(HWND hParent, HDROP hdrop);
int _check_user();
void _fillrect(HDC hdc, LPRECT prc, COLORREF rgb);
LPITEMIDLIST getlinktarget(LPCTSTR pszName);
int notify_message(UINT nTitleID, UINT nTextID, LPCTSTR pszInsert, UINT nType);
BOOL dobj_getfilename(IDataObject* pdo, LPTSTR pszName);
void replace_str(LPTSTR pText, LPCTSTR pSrc, LPCTSTR pDst);
BOOL checkmedia(LPCTSTR pszDriveName);
HRESULT dll_getversion(LPCTSTR pszDll, LPDWORD pdwVersion);
void ShowCtrlHelp(HWND hwnd, UINT idResource);
LPTSTR MakeFileNames(HDROP hdrop);
BOOL IsValidExecuteModule();
int WINAPIV key_combine(LPTSTR dst, LPCTSTR psz, ... );
void GetMyModulePath(LPTSTR psz, int cchMax);
void GetMyModulePath(CString& path);
void ZapMessage();
HMODULE LoadMyLibrary(LPCTSTR pszDll);
void AddMyPathEnvironment();
BOOL RunDll(HWND hwnd, LPCTSTR pszDll, LPCTSTR pszFunc, LPCTSTR pszArg);
BOOL GuidRunDll(HWND hwnd, DWORD effect, LPCTSTR path, HDROP hdrop, LPCTSTR dll, LPCTSTR func);
HGLOBAL DuplicateMemoryHandle(HGLOBAL hmem, UINT flag);
BOOL CopyTextToClipboard(HWND hwnd, LPCTSTR pszText, bool bAppend = false);
int GetArchiveType(LPCTSTR pszName);
HBITMAP CreateVistaBitmapFromImageList(HIMAGELIST him, int index);
HIMAGELIST CreateBitmapMapArrayFromImageList(UINT idResource, CSimpleMap<UINT, HBITMAP>& arr, int btnWidth);
HBITMAP CreateOemBitmap(int idResource);
HRESULT CoCreateInstanceAsAdmin(HWND hwnd, bool bAdmin, REFCLSID rclsid, REFIID riid, __out void ** ppv);
LPTSTR __fastcall SkipTabSpace(LPTSTR p);
int SeparateString(LPCTSTR pszText, int delim, CSimpleArray<CString>& array);
void __fastcall InitDlgCtrl(HWND hctl, LPCTSTR pszResource);
HANDLE __fastcall _createfile(LPCTSTR pName, DWORD fdwAccess, DWORD fdwShareMode, LPSECURITY_ATTRIBUTES psa, DWORD fdwCreate, DWORD fdwAttrsAndFlags, HANDLE hTemplateFile);
HIMAGELIST __fastcall MakeDummyImageList(int cx, int cy = 0);
void SetBitmapToMenuItem(HMENU hmenu, UINT idMenu, UINT idBitmap);



#endif			// __LSPSHELL_COMMON_H__
