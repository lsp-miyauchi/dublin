////////////////////////////////////////////////////////////////////////////
// keydata.h
//

#pragma once
#ifndef __KEYDATA_H__
#define __KEYDATA_H__

static UINT keymapDefFnc[48] = {
	IDM_VIEW_KEYMAP,				// F1
	IDM_FILE_RENAME,				// F2
	IDM_DIR_SEARCH,					// F3
	IDM_DIR_SELECT,					// F4
	IDM_WINDOW_REFLESH,				// F5
	IDM_WINDOW_NEXT,				// F6
	0,								// F7
	0,								// F8
	IDM_BAR_MENU,					// F9
	0,								// F10
	IDM_HOT_ICON_SWITCH,			// F11
	IDM_HOT_ICON_IMAGE,				// F12
	IDM_VIEW_RELEASEFILTER,			// Shift + F1
	0,								// Shift + F2
	0,								// Shift + F3
	0,								// Shift + F4
	0,								// Shift + F5
	IDM_WINDOW_PREV,				// Shift + F6
	0,								// Shift + F7
	0,								// Shift + F8
	0,								// Shift + F9
	IDM_FILE_PROPERTY,				// Shift + F10
	0,								// Shift + F11
	0,								// Shift + F12
	0,								// Ctrl + F1
	0,								// Ctrl + F2
	0,								// Ctrl + F3
	IDM_TAB_CLOSE,					// Ctrl + F4
	0,								// Ctrl + F5
	0,								// Ctrl + F6
  IDM_TOOL_SHELL,		// Ctrl + F7
	0,								// Ctrl + F8
	0,								// Ctrl + F9
  IDM_TOOL_MSDOS,					// Ctrl + F10
	IDM_TOOL_GREP,					// Ctrl + F11
	IDM_DIR_SYNC,					// Ctrl + F12
	0,								// Shift + Ctrl + F1
	0,								// Shift + Ctrl + F2
	0,								// Shift + Ctrl + F3
	0,								// Shift + Ctrl + F4
	0,								// Shift + Ctrl + F5
	0,								// Shift + Ctrl + F6
	0,								// Shift + Ctrl + F7
	0,								// Shift + Ctrl + F8
	0,								// Shift + Ctrl + F9
	0,								// Shift + Ctrl + F10
	0,								// Shift + Ctrl + F11
	IDM_BAR_REPOSITION				// Shift + Ctrl + F12
};

static UINT keymapDefAbc[78] = {
	IDM_FILE_ATTRIB,				// A ファイルATTRIB
	IDM_TOOL_BDUMP,					// B バイナリダンプ
	IDM_FILE_COPY,					// C ファイルコピー
	IDM_FILE_DELETE,				// D ファイル削除
	IDM_TOOL_EDITOR,				// E エディタの起動
	IDM_DIR_SEARCH,					// F ファイル検索
	IDM_DIR_HISTORY,				// G 履歴
	IDM_FILE_MERGE,					// H ファイル連結
	IDM_VIEW_FILEINFO,				// I ファイル情報
	0,								// J 未定義
	IDM_FILE_MKDIR,					// K ディレクトリ作成
	IDM_DIR_SELECT,					// L ディレクトリ移動
	IDM_FILE_MOVE,					// M ファイルの移動
	IDM_FILE_DIVIDE,				// N ファイル分割
	0,								// O 未定義
	IDM_FILE_PACK,					// P ファイルの圧縮
	IDM_VIEW_HOTKEYMENU,			// Q ホットキーメニュー
	IDM_FILE_RENAME,				// R リネーム
	IDM_VIEW_SORT,					// S ファイルソート
	0,								// T 未定義
	IDM_FILE_UNPACK,				// U 圧縮ファイル解凍
	IDM_TOOL_VIEWER,				// V ビュアーの起動
	IDM_VIEW_WILDCARD,				// W ワイルドカード
	IDM_FILE_EXECUTE,				// X ファイル実行
	IDM_FILE_ASSOCIATE,				// Y 拡張子関連付け
	IDM_FILE_MASKSELECT,			// Z マスク選択
	IDM_FILE_SELALL,				// Ctrl + A 全選択
	IDM_FILE_SELINVERT,				// Ctrl + B 選択の反転
	IDM_EDIT_COPY,					// Ctrl + C オブジェクトのコピー
	0,								// Ctrl + D
	0,								// Ctrl + E
	0,								// Ctrl + F
	0,								// Ctrl + G
	0,								// Ctrl + H
	0,								// Ctrl + I
	0,								// Ctrl + J
	0,								// Ctrl + K
	IDM_TAB_LOCK,					// Ctrl + L タブをロック
	0,								// Ctrl + M
	IDM_EXEC_OWN,					// Ctrl + N 新しいウィンドウを開く
	0,								// Ctrl + O
	0,								// Ctrl + P
	ID_APP_EXIT,					// Ctrl + Q 終了
	0,								// Ctrl + R
	0,								// Ctrl + S
	IDM_TAB_NEW,					// Ctrl + T 新しいタブ
	0,								// Ctrl + U
	IDM_EDIT_PASTE,					// Ctrl + V オブジェクトの貼付け
	IDM_TAB_CLOSE,					// Ctrl + W タブを閉じる
	IDM_EDIT_CUT,					// Ctrl + X オブジェクトの切り取り
	0,								// Ctrl + Y
	0,								// Ctrl + Z
	0,								// Shift + Ctrl + A
	0,								// Shift + Ctrl + B
	0,								// Shift + Ctrl + C
	0,								// Shift + Ctrl + D
	0,								// Shift + Ctrl + E
	0,								// Shift + Ctrl + F
	0,								// Shift + Ctrl + G
	0,								// Shift + Ctrl + H
	0,								// Shift + Ctrl + I
	0,								// Shift + Ctrl + J
	0,								// Shift + Ctrl + K
	0,								// Shift + Ctrl + L
	0,								// Shift + Ctrl + M
	0,								// Shift + Ctrl + N
	0,								// Shift + Ctrl + O
	0,								// Shift + Ctrl + P
	0,								// Shift + Ctrl + Q
	0,								// Shift + Ctrl + R
	0,								// Shift + Ctrl + S
	IDM_TAB_REOPEN_RECENT,			// Shift + Ctrl + T
	0,								// Shift + Ctrl + U
	0,								// Shift + Ctrl + V
	IDM_TAB_CLOSE_ANYOTHER,			// Shift + Ctrl + W 他のタブをすべて閉じる
	0,								// Shift + Ctrl + X
	0,								// Shift + Ctrl + Y
	0,								// Shift + Ctrl + Z
};

static UINT keymapDefNum[20] = {
	0,								// 0
	0,								// 1
	0,								// 2
	0,								// 3
	0,								// 4
	0,								// 5
	0,								// 6
	0,								// 7
	0,								// 8
	0,								// 9
	0,								// Ctrl + 0
	0,								// Ctrl + 1
	0,								// Ctrl + 2
	0,								// Ctrl + 3
	0,								// Ctrl + 4
	0,								// Ctrl + 5
	0,								// Ctrl + 6
	0,								// Ctrl + 7
	0,								// Ctrl + 8
	0,								// Ctrl + 9
};

#endif				// __KEYDATA_H__
