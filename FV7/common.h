////////////////////////////////////////////////////////////////////
// common.h

#pragma once
#ifndef __COMMON_H__
#define __COMMON_H__


// vwfndでも同じものを使う
enum {
	CPD_UNKNOWN = 0,
	CPD_JUMP_PATHA = 1,
	CPD_JUMP_PATHW,
	CPD_JUMP_IDLIST,
	CPD_JUMP_FILEA,
	CPD_JUMP_FILEW,
	CPD_EXEC_IDLIST = 11,
	CPD_EXEC_FILEA,
	CPD_EXEC_FILEW,
	CPD_VIEW_IDLIST = 21,
	CPD_VIEW_FILEA,
	CPD_VIEW_FILEW,
	CPD_MACROA = 31,
	CPD_MACROW
};

#define CPD_COMMAND_MASK		0x0000ffff
#define CPD_NEW_TAB				0x80000000

////////////////////////////////////////////////////////////////////
// Help ID bases

#define HID_BASE_COMMAND		0x00010000UL        // ID and IDM
#define HID_BASE_RESOURCE		0x00020000UL        // IDR and IDD
#define HID_BASE_PROMPT			0x00030000UL        // IDP
#define HID_BASE_NCAREAS		0x00040000UL
#define HID_BASE_CONTROL		0x00050000UL        // IDC
#define HID_BASE_DISPATCH		0x00060000UL        // IDispatch help codes

////////////////////////////////////////////////////////////////////
// MAXIMUM NUMBER
//
#define MAXACCESS				20
#define MAXHISTORY				20


////////////////////////////////////////////////////////////////////
// char search

#define _CHECKDOT(a)	((a)[0]=='.' && ((a)[1]==0 || ((a)[1]=='.' && (a)[2]==0)))
#define _ISNETPATH(a)	(lstrlen((a)) > 3 && *(a) == '\\' && *((a) + 1) == '\\' && *((a) + 2) != '?')

//----------------------------------------------
// COPY / MOVE CONFIRMATION DEFINITIONS
//
#define C_QUERY					0x00000000
#define C_NEWONLY				0x00000001
#define C_OVERWRITE				0x00000002
#define C_RENAME				0x00000003
#define C_IGNORE				0x00000004
#define C_MERGE					0x00000005
#define C_YESTOALL				0x00010000
#define C_MULTIDEST				0x00020000


//----------------------------------------------
// CHECK FILE CREATE
//
#define CC_ERROR				-1
#define CC_EXIST				0
#define CC_NEW					1


//----------------------------------------------
// RELAY DEFINITION
//
enum RelayMode {
	RELAY_COPY,
	RELAY_MOVE,
	RELAY_MERGE,
	RELAY_DIVIDE
};

enum {
	addinMsdos,
	addinEditor,
	addinViewer,
	addinBmpViewer,
	addinBDump
};

//----------------------------------------------
// BITMAP SIZE DEFINITION
//
#define ABCBMPX						16
#define ABCBMPY						16
#define FNKEYBMPX					16
#define FNKEYBMPY					16
#define KEYBD_X						20
#define KEYBD_Y						20


//----------------------------------------------
// BIT STATUS DEFINITIONS
//

// システム動作系列
#define STS_MAXIMIZE				0x00000002		// 最大化して起動
#define STS_NOSPLASH				0x00000008		// スプラッシュ画面の禁止
#define STS_MIN1ST					0x00000020		// 初回はアイコン起動
#define STS_SWITCHEXEC				0x00000100		// スイッチバック
#define STS_DRIVEROOT				0x00000400		// ツリーはドライブルート
#define STS_STARTDIR_MASK			0xF0000000
#define STS_HOMEDIR					0x00000000		// ホームディレクトリ
#define STS_LASTDIR					0x10000000		// 前回終了時のフォルダ
#define STS_LASTTABGROUP			0x20000000		// 前回終了時のタブ グループ
#define STS_SPECIFICTABGROUP		0x30000000		// 指定のタブ グループ

// 常駐動作（Terminate and Stay Resident）
#define TSS_TSR						0x00000001		// Memory Resident mode
#define TSS_NEWWINDOW				0x00000002		// トレイアイコンメニューで選択したフォルダをどう開くか
#define TSS_MINHIDE					0x00000004		// 常駐時に最小化した際にウィンドウをタスクバーから消す

// リスト操作系列
#define LTS_LOWERCASE				0x00000004		// Lower case
#define LTS_ALIGNEXT				0x00000008		// 拡張子を揃える
#define LTS_FMTSIZE					0x00000010		// 1.23KB / 23.4MB 等の表示
#define LTS_INFOTIP					0x00000020		// チップ情報
#define LTS_EXTRIGHT				0x00000040		// 拡張子を右寄せする
#define LTS_FMTDATE					0x00000080		// 2001年 / 01年
#define LTS_CMPUNDERSCORE			0x00000100		// ファイル比較時、ハイフンとアンダースコアを区別しない
#define LTS_UPDIR					0x00000200		// 「ひとつ上」をエミュレート
#define LTS_UPDIR_NOROOT			0x00000400		// ドライブルートには「ひとつ上」を表示しない
#define LTS_RELATIVEDATE			0x00000800		// 相対日時（昨日、今日）
#define LTS_NOARCTREE		0x00001000		// 書庫をフォルダツリーから除外
#define LTS_LOGICALCMP				0x00002000		// ファイル名中の数値の大小で比較
#define LTS_AUTOADJUST				0x00004000		// 一覧表示で自動調整
#define LTS_FULLPATH				0x00008000		// タイトルバーにフルパス表示
#define LTS_PATH1ST					0x00010000		// タイトルバーにパス名を先に表示
#define LTS_SELTREELINE				0x00040000		// 現在のツリーを行選択
#define LTS_REPCHECKER				0x00080000		// 詳細表示で市松模様
#define LTS_BOLDNAME				0x00100000		// 縮小・アイコン表示でファイル名を強調表示する
#define LTS_MENUACCEL				0x00200000		// メニューにショートカットキーを表示
#define LTS_NODOSNAME				0x00400000		// 詳細表示ではDOS名を省略する
#define LTS_EACH_ICONSTYLE			0x00800000		// アイコンリスト表示スタイルをフォルダごとに保存
#define LTS_TREENONELINE			0x01000000		// ツリー線無し
#define LTS_WRAPCURSOR				0x02000000		// カーソルラップアラウンド
#define LTS_BOLDSELTREE				0x04000000		// ツリーで太字表示する
#define LTS_RELAX					0x08000000		// ゆとりを持って表示
#define LTS_DSZ_FIXED				0x10000000		// HDDフォルダサイズの計算
#define LTS_DSZ_CDROM				0x20000000		// CDROMフォルダサイズの計算
#define LTS_DSZ_REMOTE				0x40000000		// ネットワークフォルダサイズの計算
#define LTS_DSZ_REMOVABLE			0x80000000		// 取外し可能メディアフォルダサイズの計算
#define LTS_DSZ_MASK				0xF0000000		// フォルダサイズの計算マスク

// ファイル表示系列
#define FTS_HIDDEN					0x00000001		// すべての隠しファイル非表示
#define FTS_SYSDB					0x00000002		// thumbs.db, desktop.ini を表示しない
#define FTS_SUPERHIDDEN				0x00000004		// 隠しシステムファイル非表示
#define FTS_THUMBNAILMASK			0x00000FF0		// サムネイル関連のマスク
#define FTS_NOTHUMBFOLDER			0x00000010		// フォルダのサムネイル表示はしない
#define FTS_EXIF					0x00000020		// EXIF情報
#define FTS_THUMBCACHE_MASK			0x00000F00		// 
#define FTS_THUMBCACHE_NOUSE		0x00000000		// 
#define FTS_THUMBCACHE_READONLY		0x00000100		// 
#define FTS_THUMBCACHE_UPDATE		0x00000200		// 
#define FTS_FOLDER					0x00008000		// フォルダ非表示
#define MAKETHUMBCACHECODE(a)		(((a) & FTS_THUMBCACHE_MASK) >> 8)

// ツールバー系列
#define BTS_STATUS					0x00000004
#define BTS_FAVO					0x00000008
#define BTS_APPBAR					0x00000010
#define BTS_APPRIGHT				0x00000080		// アプリバー右置き
#define BTS_DRIVE					0x00000100
#define BTS_FUNCTION				0x00000200
#define BTS_NAVI					0x00000400
#define BTS_SMART					0x00000800		// スマートバー
#define BTS_LARGEICON				0x00004000
#define BTS_DRIVERIGHT				0x20000000		// ドライブバー縦置きであれば右側に配置
#define BTS_VERTDRIVE				0x40000000		// ドライブバーを縦置き
#define BTS_LOCKREBAR				0x80000000		// ツールバーを固定する

// ファイル操作系列
#define FOS_RECYCLE					0x00000002		// ごみ箱を利用してファイル削除
#define FOS_NOWATCHRECYCLEFOLDER	0x00000004		// ごみ箱表示中はファイル更新を監視しない
#define FOS_DDNOCONFIRM				0x00001000		// Ｄ＆Ｄでドロップされた際のコピー移動実行確認
#define FOS_DDALTOVERWRITE			0x00002000		// Ｄ＆Ｄでドロップされた際のCtrl+Alt、Shift+Alt押下で強制上書きモード
#define FOS_RUNCOPY					0x00004000		// ファイルのコピー・移動はバックグラウンド処理がデフォルト
#define FOS_COPYSD					0x00008000		// コピー時、セキュリティ情報もコピー
#define FOS_REXFINDER				0x00010000		// サーチファインダーでは正規表現を使う
#define FOS_SHIFTSEARCH				0x00020000		// Shift+アルファベットでサーチファインダーを開く
#define FOS_RESETFILTER				0x00040000		// フォルダを変更したらフィルタをリセット
#define FOS_PREFIX_MATCH			0x00080000		// 先頭一致で検索（通常のワイルドカード検索時）
#define FOS_EXEC_MASK				0xF0000000		// 実行モードマスク
#define FOS_EXEC_VIEWER				0x00000000		// Shift+Enterでビュア実行
#define FOS_EXEC_EDITOR				0x10000000		// Shift+Enterでエディタ実行
#define FOS_EXEC_OPEN				0x20000000		// Shift+Enterで標準の動作で開く

// ユーザー確認系
#define COS_UPDIR_MASK				0x0F000000
#define COS_UPDIR_DISABLE			0x00000000		// ドライブルート時ひとつ上NG
#define COS_UPDIR_ENABLE			0x01000000		// ドライブルート時ひとつ上OK
#define COS_UPDIR_CONFIRM			0x02000000		// ドライブルート時ひとつ上確認してOK

// マウス操作
#define MOS_SPC_DBLCLK_UPDIR		0x00000001		// 空きペインクリックでひとつ上へ
#define MOS_SPC_FOLDER_CTXMENU		0x00000002		// 空きペイン右クリックでフォルダのコンテキストメニューを表示
#define MOS_HOTTRACK				0x00000004		// ホットトラッキング
#define MOS_FAVO_ONECLICK			0x00000010		// しおりでは1クリックでフォルダ選択
#define MOS_LBTN_MASK				0x0000F000
#define MOS_LBTN_DEFAULT			0x00000000		// クリックして選択し、ダブルクリックで開く
#define MOS_LBTN_ONECLICK			0x00001000		// ポイントして選択し、クリックして開く
#define MOS_RBTN_MASK				0x000F0000
#define MOS_RBTN_CONTEXTMENU		0x00000000		// コンテキストメニューを表示
#define MOS_RBTN_SHORTCUTMENU		0x00010000		// ショートカットメニューを表示
#define MOS_RBTN_SINGLE_SEL			0x00020000		// 個別にオブジェクトを選択
#define MOS_CBTN_MASK				0x00F00000
#define MOS_CBTN_DEFAULT			0x00000000		// 何もしない
#define MOS_CBTN_SINGLE_SEL			0x00100000		// 個別にオブジェクトを選択
#define MOS_CBTN_SHORTCUTMENU		0x00200000		// ショートカットメニューを表示

// ドライブバー
#define DRS_MOBILEDEVICE			0x00000001		// モバイルデバイスを表示
#define DRS_NETDEVICE				0x00000002		// ネットワークデバイスを表示
#define DRS_MBTN_MASK				0x0F000000
#define DRS_MBTN_NEWTAB				0x01000000		// ドライブアイコン MBUTTON ダブルクリックで新しいタブを開く
#define DRS_MBTN_NEWWINDOW			0x02000000		// ドライブアイコン MBUTTON ダブルクリックで新しいウィンドウを開く
#define DRS_LBTN_MASK				0xF0000000
#define DRS_LBTN_NEWTAB				0x10000000		// ドライブアイコンダブルクリックで新しいタブを開く
#define DRS_LBTN_NEWWINDOW			0x20000000		// ドライブアイコンダブルクリックで新しいウィンドウを開く

// タブ操作系列
#define TAB_CLOSENOCONFIRM			0x00000001		// 終了時、複数タブを閉じる確認をおこなわない
#define TAB_MKDIR					0x00000002		// フォルダを作成したら新しいタブで開く
#define TAB_BARSHOWANY				0x00000004		// タブバーを常に表示
#define TAB_DRIVEADD				0x00000008		// ドライブが追加されたらタブで開く
#define TAB_MEDIAINSERTED			0x00000010		// メディアが挿入されたらタブで開く
#define TAB_HOVERSELECT				0x00000020		// ホバーセレクト
#define TAB_DBLCLK_LOCK				0x00000040		// タブのダブルクリックでロック・アンロック
#define TAB_FULLPATH				0x00000080		// タブのパス名はフルパス
#define TAB_SAVEGROUPTOBOOKMARK		0x00000100		// タブグループ保存時、しおりにも登録する
#define TAB_NODUPLICATE				0x00000200		// 既に開いているフォルダを新しいタブで二重に開かない
#define TAB_SELECT_LASTACTIVE		0x00000400		// 最後にアクティブだったタブを選択
#define TAB_DROPOPEN				0x00000800		// オブジェクトドロップの処理後そのタブを開く

//////////////////////////////////////////////////////////////////////
// SpecialFolder structure def

typedef struct tagSPFL {
	int nID;
	UINT idString;
} SPFL;

typedef struct tagKNFL {
	GUID guid;
	UINT idString;
} KNFL;


/////////////////////////////////////////////////////////////////////////////
//

class CSetRedraw
{
private:
	bool m_redraw;
	HWND m_hwnd;
public:
	CSetRedraw(HWND hwnd)
	{
		m_redraw = false;
		m_hwnd = NULL;
		if (hwnd && ::IsWindow(hwnd)) {
			m_hwnd = hwnd;
			m_redraw = true;
			::SendMessage(m_hwnd, WM_SETREDRAW, FALSE, 0);
		}
	}

	~CSetRedraw()
	{
		if (m_redraw && ::IsWindow(m_hwnd)) {
			::SendMessage(m_hwnd, WM_SETREDRAW, TRUE, 0);
			::InvalidateRect(m_hwnd, NULL, TRUE);
		}
	}
};

//////////////////////////////////////////////////////////////////////
//

class CToday
{
private:
	DWORDLONG m_dayToday;
	SYSTEMTIME m_timeToday;
public:
	CToday() { UpdateToday(); }
	void UpdateToday()
	{
		GetLocalTime(&m_timeToday);
		m_timeToday.wHour = m_timeToday.wMinute = m_timeToday.wSecond = m_timeToday.wMilliseconds = 0;
		SystemTimeToFileTime(&m_timeToday, (FILETIME*)&m_dayToday);
	}
	inline bool isToday(DWORDLONG day) { return (m_dayToday <= day && day < (m_dayToday + 864000000000)); }
	inline bool isYesterday(DWORDLONG day) { return ((m_dayToday - 864000000000) <= day && day < m_dayToday); }
	inline bool is2daysAgo(DWORDLONG day) { return ((m_dayToday - 1728000000000) <= day && day < m_dayToday - 864000000000); }
	inline bool is3daysAgo(DWORDLONG day) { return ((m_dayToday -  2592000000000) <= day && day < m_dayToday - 1728000000000); }
	bool isThisMonth(DWORDLONG day)
	{
		SYSTEMTIME st;
		FileTimeToSystemTime((const FILETIME*)&day, &st);
		return (st.wMonth == m_timeToday.wMonth);
	}
	bool isThisYear(DWORDLONG day)
	{
		SYSTEMTIME st;
		FileTimeToSystemTime((const FILETIME*)&day, &st);
		return (st.wYear == m_timeToday.wYear);
	}
};

////////////////////////////////////////////////////////////////////////
//

class CStringArray : public CSimpleArray<CString>
{
public:
	CString Join(LPCTSTR joiner)
	{
		CString dst;
		int cnt = GetSize();
		if (cnt > 0) {
			dst = (*this)[0];
			for (int i = 1; i < cnt; i++) {
				if (joiner && *joiner) {
					dst += joiner;
				}
				dst += (*this)[i];
			}
		}
		return dst;
	}
	CString Join(TCHAR joiner)
	{
		CString jstr(joiner);
		return Join(jstr.GetString());
	}
};

//////////////////////////////////////////////////////////////////////
// メモリアロケート用

template <class T>
class CTypeAlloc
{
private:
	T* m_aT;
	int m_size;
public:
	__inline CTypeAlloc(int size) { m_size = size; m_aT = (T*)malloc(sizeof(T) * size); }
	__inline ~CTypeAlloc() { if (m_aT) free(m_aT); }
	__inline operator T*() const { return m_aT; }
#ifdef DEBUG
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
#else
	__inline bool SetAt(int index, T t) { m_aT[index] = t; return true; }
	__inline const T& operator[] (int index) const { return m_aT[index]; }
	__inline T& operator[] (int index) { return m_aT[index]; }
#endif
};



#endif				// __COMMON_H__
