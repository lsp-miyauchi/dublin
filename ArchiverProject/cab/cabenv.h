////////////////////////////////////////////////////////////
// cabenv.h

#pragma once
#ifndef __CABENV_H__
#define __CABENV_H__

#include "sfx.h"

#pragma pack(1)
typedef struct tagHEADEREX {
	WORD stat;
	WORD type;
	DWORD crc;
} HEADEREX;
#pragma pack()

#define CABSETID				0x1128
#define CB_MAX_DISK_NEW			0x7fffffffL		// 2,147,486,647


///////////////////////////////////////////////////////////
// Cab()へ通知可能なコマンド・オプションフラグ

#define CABCMD_NONE				0				// 命令未指定
#define	CABCMD_PACK				('a')			// a キャビネット作成
#define	CABCMD_MOVE				('m')			// m キャビネット作成
#define CABCMD_UNPACK			('x')			// x キャビネット解凍
#define CABCMD_LIST				('l')			// l キャビネット一覧
#define CABCMD_CHECK			('c')			// c キャビネットチェック
#define CABCMD_TEST				('t')			// t キャビネット整合性テスト
#define CABCMD_SFX				('f')			// f 自己解凍形式書庫
#define CABCMD_FROMSFX			('b')			// b 自己解凍形式書庫からキャビネットを抽出

#define CABOPT_IGNOREDIR		0x00000001		// -j フォルダを無視
#define CABOPT_SILENT			0x00000002		// -i サイレントモード
#define CABOPT_NEWONLY			0x00000004		// -n 新しいファイルのみ
#define CABOPT_OVERWRITE		0x00000008		// -o 無条件に上書き解凍
#define CABOPT_MULTIVOL			0x00000010		// -v:xxx マルチボリューム
#define CABOPT_RECURSEDIR		0x00000020		// -r 配下のフォルダも検索
#define CABOPT_KEEPNEXT			0x00000040		// -k 連続するキャビネットを解凍
#define CABOPT_REGEXP			0x00000080		// -w 正規表現を利用
#define CABOPT_MASK_COMPRESS	0x00000F00		// 圧縮モードのマスク
#define CABOPT_LZX				0x00000100		// -ml:xxx     LZX圧縮
#define CABOPT_QUANTUM			0x00000200		// -mq:xxx,yyy QUANTUM圧縮
#define CABOPT_MSZIP			0x00000400		// -mz         MSZIP圧縮
#define CABOPT_STORE			0x00000800		// -ms         無圧縮
#define CABOPT_DOSNAME			0x00001000		// -q 8+3のDOS名で処理
#define CABOPT_DTSTART			0x00002000		// 開始日時
#define CABOPT_DTEND			0x00004000		// 終了日時
#define CABOPT_EXC_SYSHIDDEN	0x00008000		// -s System/Hiddenを格納しない
#define CABOPT_FULLPATHMATCH	0x00010000		// 全パスで一致
#define CABOPT_DEBUG			0x01000000		// -~ デバッグ表示
#define CABOPT_FILTERPRESENTS	0x10000000		// フィルタ指定有り
												// -s:xxxx セットID
#define CABOPT_UTFENCODE		0x20000000		// -utf 格納ファイル名をUTFでエンコード

enum {
	CAB_COMPRESSION_LZX,
	CAB_COMPRESSION_MSZIP,
	CAB_COMPRESSION_QUANTUM,
	CAB_COMPRESSION_STORE
};

enum {
	BRT_QUERY = 0,				// 都度問い合わせ
	BRT_ERROR,					// エラー扱い
	BRT_EXTRACT,				// そのまま展開
	BRT_IGNORE,					// 無視
	BRT_STRIPPATH,				// パスを削除
	BRT_RELATIVE,				// 相対パスへ変換
	BRT_TYPEMASK = 0xFFFF,
	BRT_YESALL = 0x10000,
};


///////////////////////////////////////////////////////////
//

class CCabEnv
{
public:
	CCabEnv();
	~CCabEnv();

	int Execute(HWND hwnd, LPCWSTR pszCmd);
	BOOL PerseResponseFile(LPCWSTR pFileList);
	BOOL PerseResponseFileW(LPCWSTR pFileList);
	BOOL PerseSwitch(LPCWSTR pszSwitch);
	BOOL PerseCommand(LPCWSTR pszCmd);
	int CmdList();
	int CmdTest();
	int CmdCheck();
#ifdef _CABSFX_SUPPORT
	int CmdMakeSfx();
	BOOL MakeSfxFile(LPCWSTR pszSfx, void* pvStubData, DWORD sizeStub, SFX* info);
#endif
	BOOL FciCompress();
	BOOL FdiExtract();

	void msgout(LPCWSTR fmt, ...);

	BOOL GetWorkingPath(LPWSTR pszPath, int cchMax);
	int BuildTempPathName(LPCWSTR pszFile);

	HWND m_hwndOwner;
	WCHAR m_chCmd;
	DWORD m_cmdOption;
	long m_nVolumeSize;
	CStringW m_strDefaultPath;
	CStringW m_strCabName;
	CStringW m_strBasePath;
	CStringW m_strWorkPath;
	WCHAR m_szTemp[MAX_PATH * 2];
	short m_sSetID;
	CSimpleArray<CStringW> m_arrMask;
	int m_nCompressionLevel;				// for LZW or QUANTUM compression
	int m_nMemoryLevel;						// for QUANTUM compression
	int m_nCompressionMode;
	int m_nNumOfFiles;
	CStringW m_strStored;					// 圧縮しないファイルのフィルタ
	CStringW m_strExcludes;					// 格納しないファイルのフィルタ
	FILETIME m_ftStart;
	FILETIME m_ftEnd;
	CStringW m_strResult;
	int m_brittle;							// ファイル展開方法

	CStringW m_strMask;						// 解凍時のマスクリスト
};


///////////////////////////////////////////////////////////
//

class CEnterFolder
{
public:
	CEnterFolder()
	{
		m_szOldPath[0] = 0;
	}
	~CEnterFolder()
	{
		if (m_szOldPath[0])
			SetCurrentDirectory(m_szOldPath);
	}

	BOOL enter(LPCWSTR pszNewPath)
	{
		if (m_szOldPath[0] == 0)
			GetCurrentDirectory(ARRAYSIZE(m_szOldPath), m_szOldPath);
		if (pszNewPath && *pszNewPath)
			return SetCurrentDirectory(pszNewPath);
		return FALSE;
	}

	WCHAR m_szOldPath[MAX_PATH * 2];
};

///////////////////////////////////////////////////////////
//

class CWaitingCursor
{
public:
	HCURSOR m_hcur;
	CWaitingCursor() { m_hcur = ::SetCursor(::LoadCursor(NULL, IDC_WAIT)); }
	~CWaitingCursor() { if (m_hcur) ::SetCursor(m_hcur); }
};


#endif			// __CABENV_H__
