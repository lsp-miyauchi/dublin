////////////////////////////////////////////////////////////////////////
// configmanager.h

#pragma once

#ifndef __CONFIG_MANAGER_H__
#define __CONFIG_MANAGER_H__


class ConfigManager
{
public:

	ConfigManager();
	~ConfigManager();

	// 指定した名前のファイル用の文書タイプをロード
	int SetDocType(LPCTSTR pszTypeFile);

	// 指定した名前の文書タイプをロード
	void SetDocTypeByName(LPCTSTR pszTypeName);

public:
	// 文字数のカウント方法
	inline bool countByUnicode() const { return m_bCountbyUnicode; }

	// 開く/保存ダイアログに出すフィルタの設定
	inline const ki::String& txtFileFilter() const { return m_strFilter; }

	// 文字数指定時の折り返し文字数
	inline int wrapWidth() const { return m_curDt->wrapWidth; }

	// 折り返し方法
	inline int wrapType() const { return (m_curDt->wrapType > 0) ? wrapWidth() : m_curDt->wrapType; }

	// 行番号表示する？
	inline bool showLN() const { return m_curDt->showLN; }

	// 表示色・フォントなど
	inline const VConfig& vConfig() const { return m_curDt->vc; }

	// キーワードファイル名(フルパス)
	inline ki::Path kwdFile() const { return ki::Path(ki::Path::Exe) + TEXT("type\\") + m_curDt->kwdfile; }

	// Grep用外部実行ファイル名
	inline const ki::Path& grepExe() const { return m_strGrepExe; }


	// 同じウインドウで開くモード
	inline bool openSame() const { return m_bOpenSame; }

	// ステータスバー表示
	inline bool showStatusBar() const { return m_bShowStatusBar; }

	inline void ShowStatusBarSwitch() { m_bShowStatusBar = !m_bShowStatusBar; SaveIni(); }

public:
	// 新規ファイルの文字コードindex
	inline int GetNewFileCharSetIndex() const { return m_CharSets.findCsi( m_NewFileCharset ); }

	// 新規ファイルの改行コード
	inline ki::lbcode GetNewFileLineBreak() const { return m_NewFileLineBreak; }

public:
	//@{ [最近使ったファイル]へ追加 //@}
	void AddMRU( const ki::Path& fname );

	//@{ [最近使ったファイル]メニューの構築 //@}
	void SetUpMRUMenu( HMENU m, UINT id );

	//@{ [最近使ったファイル]取得 //@}
	ki::Path GetMRU( int no ) const;

	// 対応文字セットリスト取得
	inline CharSetList& GetCharSetList() { return m_CharSets; }

public:
	// ウインドウ位置・サイズ復元処理
	inline int GetWndX() const { return m_rememberWindowPlace ? m_xWindowPos : CW_USEDEFAULT; }
	inline int GetWndY() const { return m_rememberWindowPlace ? m_yWindowPos : CW_USEDEFAULT; }
	inline int GetWndW() const { return m_rememberWindowSize ? m_cxWindow : CW_USEDEFAULT; }
	inline int GetWndH() const { return m_rememberWindowSize ? m_cyWindow : CW_USEDEFAULT; }
	inline bool GetWndM() const { return m_rememberWindowSize & m_bWindowMaximized; }
	void RememberWnd( ki::Window* wnd );

private:

	ki::IniFile m_IniFile;
	bool        m_bSharedConfigMode;
	CharSetList m_CharSets;

	// 全体的な設定
	int        m_nUndoLimit;
	ki::String m_strFilter;
	ki::Path   m_strGrepExe;
	bool       m_bOpenSame;
	bool       m_bCountbyUnicode;
	bool       m_bShowStatusBar;
	bool       m_rememberWindowSize;
	bool       m_rememberWindowPlace;

	// ウインドウサイズ記憶
	bool m_bWindowMaximized; // maximized?
	int  m_xWindowPos, m_yWindowPos, m_cxWindow, m_cyWindow;

	// 文書タイプのリスト
	struct DocType {
		// 定義ファイル名など
		ki::String        name;
		ki::String        pattern;
		ki::String        kwdfile;
		ki::String        layfile;

		// 設定項目
		VConfig vc;
		int               wrapType;
		int               wrapWidth;
		bool              showLN;
	};
	typedef ki::olist<DocType> DtList;

	DtList           m_dtList;
	DtList::iterator m_curDt;

	// 最近使ったファイルのリスト
	int m_numMrus;
	ki::Path m_mruList[20];

	// 新規ファイル関係
	int        m_NewFileCharset;
	ki::String m_strNewFileDocType;
	ki::lbcode m_NewFileLineBreak;

private:

	void LoadIni();
	void SaveIni();
	void LoadLayout( DocType* dt );
	bool MatchDocType( const unicode* fname, const unicode* pat );

private:

	friend struct ConfigDlg;
	NOCOPY(ConfigManager);
};



#endif		// __CONFIG_MANAGER_H__
