////////////////////////////////////////////////////////////
// unrarenv.h

#pragma once
#ifndef __UNRARENV_H__
#define __UNRARENV_H__

///////////////////////////////////////////////////////////
//

enum {
	RARF_VOLUME = 1,					// Volume attribute (archive volume)
	RARF_COMMENT = 2,					// Archive comment present
	RARF_LOCKED = 4,					// Archive lock attribute
	RARF_SOLID = 8,						// Solid attribute (solid archive)
	RARF_NEW_NAMING = 16,				// New volume naming scheme ('volname.partN.rar')
	RARF_AUTHENTICITY = 32,				// Authenticity information present
	RARF_RECOVERY = 64,					// Recovery record present
	RARF_ENCRYPT_HEADERS = 128,			// Block headers are encrypted
	RARF_FIRST_VOLUME = 256				// First volume (set only by RAR 3.0 and later)
};

class CRarArchive
{
public:
	HANDLE m_harcdata;
	CStringA m_aname;
	CStringW m_wname;
	CStringA m_comment;
	unsigned int m_flags;
	CMatchSpec m_filter;
	struct RARHeaderDataEx m_curhdr;

	CRarArchive()
		: m_harcdata(NULL)
		, m_flags(0)
	{
	}
	~CRarArchive()
	{
		if (m_harcdata)
			_G.RARCloseArchive(m_harcdata);
	}
	void set(struct RAROpenArchiveDataEx& data)
	{
		m_wname = data.ArcNameW;
		m_flags = data.Flags;
		if (data.CmtState == 1)
			m_comment = data.CmtBuf;
	}
};

///////////////////////////////////////////////////////////
// Unrar()へ通知可能なコマンド・オプションフラグ

#define RARCMD_NONE				0				// 命令未指定
#define RARCMD_UNPACK			('x')			// x キャビネット解凍
#define RARCMD_LIST				('l')			// l キャビネット一覧
#define RARCMD_CHECK			('c')			// c キャビネットチェック
#define RARCMD_TEST				('t')			// t キャビネット整合性テスト

#define RAROPT_IGNOREDIR		0x00000001		// -j フォルダを無視
#define RAROPT_SILENT			0x00000002		// -i サイレントモード
#define RAROPT_NEWONLY			0x00000004		// -n 新しいファイルのみ
#define RAROPT_OVERWRITE		0x00000008		// -o 無条件に上書き解凍
#define RAROPT_REGEXP			0x00000080		// -w 正規表現を利用
#define RAROPT_DTSTART			0x00002000		// 開始日時
#define RAROPT_DTEND			0x00004000		// 終了日時
#define RAROPT_DEBUG			0x01000000		// -~ デバッグ表示
#define RAROPT_FILTERPRESENTS	0x10000000		// フィルタ指定有り

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

class CUnrarEnv
{
public:
	HWND m_hwndOwner;
	WCHAR m_chCmd;
	DWORD m_cmdOption;
	CStringW m_strDefaultPath;
	CStringW m_strRarName;
	CStringW m_strBasePath;
	CStringW m_strWorkPath;
	CStringW m_strTemp;
	FILETIME m_ftStart;
	FILETIME m_ftEnd;
	CStringW m_strResult;
	CSimpleArray<CStringW> m_arrMask;
	int m_brittle;							// ファイル展開方法
	int m_nNumOfFiles;

	CUnrarEnv();
	~CUnrarEnv();

	inline bool isOption(DWORD flag) { return ((m_cmdOption & flag) != 0); }
	int Execute(HWND hwnd, LPCWSTR pszCmd);
	BOOL PerseResponseFile(LPCWSTR pFileList);
	BOOL PerseResponseFileW(LPCWSTR pFileList);
	BOOL PerseSwitch(LPCWSTR pszSwitch);
	BOOL PerseCommand(LPCWSTR pszCmd);
	void ErrorReport(struct RARHeaderDataEx& hdr, int code);
	int CmdUnpack();
	int CmdList();
	int CmdTest();
	int CmdCheck();

	void msgout(LPCWSTR fmt, ...);

	BOOL GetWorkingPath(LPWSTR pszPath, int cchMax);
	int BuildTempPathName(LPCWSTR pszFile);
};

#endif			// __UNRARENV_H__
