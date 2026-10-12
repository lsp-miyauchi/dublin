////////////////////////////////////////////////////////////
// CommonArchiver.h      Kuniaki Miyauchi

#pragma once
#ifndef __COMMONARCHIVER_H
#define __COMMONARCHIVER_H

#ifndef FNAME_MAX32
#define FNAME_MAX32		512
#endif

typedef	HGLOBAL			HARCHIVER;

#pragma pack(1)

typedef struct tagINDIVIDUALINFO {
	DWORD dwOriginalSize;					// ファイルのサイズ
 	DWORD dwCompressedSize;					// 圧縮後のサイズ
	DWORD dwCRC;							// 格納ファイルのチェックサム
	UINT uFlag;								// 処理結果
	UINT uOSType;							// 書庫作成に使われたＯＳ
	WORD wRatio;							// 圧縮率
	WORD wDate;								// 格納ファイルの日付(DOS 形式)
	WORD wTime;								// 格納ファイルの時刻(〃)
	char szFileName[FNAME_MAX32+1];			// 書庫名
	char dummy1[3];
	char szAttribute[8];					// 格納ファイルの属性(書庫固有)
	char szMode[8];							// 格納ファイルの格納モード(〃) 
} INDIVIDUALINFO, *LPINDIVIDUALINFO;

typedef struct {
	DWORD dwOriginalSize;					// ファイルのサイズ
 	DWORD dwCompressedSize;					// 圧縮後のサイズ
	DWORD dwCRC;							// 格納ファイルのチェックサム
	UINT uFlag;								// 処理結果
											// Status flag
	UINT uOSType;							// 書庫作成に使われた OS
	WORD wRatio;							// 圧縮率
	WORD wDate;								// 格納ファイルの日付(DOS 形式)
	WORD wTime;								// 格納ファイルの時刻(〃)
	WCHAR szFileName[FNAME_MAX32+1];		// 格納ファイル名
	WCHAR dummy1[3];
	WCHAR szAttribute[8];					// 格納ファイルの属性(書庫固有)
	WCHAR szMode[8];						// 格納ファイルの格納モード(〃)
}	INDIVIDUALINFOW, *LPINDIVIDUALINFOW;

typedef struct tagEXTRACTINGINFO {
	DWORD dwFileSize;						// 格納ファイルのサイズ
	DWORD dwWriteSize;						// 書き込みサイズ
	char szSourceFileName[FNAME_MAX32+1];	// 格納ファイル名
	char dummy1[3];
	char szDestFileName[FNAME_MAX32+1];		// 解凍先または圧縮元パス名
	char dummy[3];
} EXTRACTINGINFO, *LPEXTRACTINGINFO;

typedef struct tagEXTRACTINGINFOEX {
	EXTRACTINGINFO exinfo;
	DWORD	dwCompressedSize;
	DWORD	dwCRC;
	UINT	uOSType;
	WORD	wRatio;
	WORD	wDate;
	WORD	wTime;
	char	szAttribute[8];
	char	szMode[8];
} EXTRACTINGINFOEX, *LPEXTRACTINGINFOEX;

#pragma pack()


/////////////////////////////////////////////////////////////////////
// 『統合アーカイバ API』共通の API

#define	CHECKARCHIVE_RAPID				0	// 簡易(最初の３ヘッダまで)
#define	CHECKARCHIVE_BASIC				1	// 標準(全てのヘッダ)
#define	CHECKARCHIVE_FULLCRC			2	// 完全(格納ファイルの CRC チェック)

// 以下のフラグは上記と組み合わせて使用
#define CHECKARCHIVE_RECOVERY			4	// 破損ヘッダを読み飛ばして処理
#define CHECKARCHIVE_SFX				8	// SFX かどうかを返す
#define CHECKARCHIVE_ALL				16	// ファイルの最後まで検索する

#define	UNPACK_CONFIG_MODE				1	// 解凍系の設定
#define PACK_CONFIG_MODE				2	// 圧縮系の設定

#define ISARC							0	// Xxx
#define ISARC_GET_VERSION				1	// XxxGetVersion
#define ISARC_GET_CURSOR_INTERVAL		2	// XxxGetCursorInterval
#define ISARC_SET_CURSOR_INTERVAL		3	// XxxSetCursorInterval
#define ISARC_GET_BACK_GROUND_MODE		4	// XxxGetBackGroundMode
#define ISARC_SET_BACK_GROUND_MODE		5	// XxxSetBackGroundMode
#define ISARC_GET_CURSOR_MODE			6	// XxxGetCursorMode
#define ISARC_SET_CURSOR_MODE			7	// XxxSetCursorMode
#define ISARC_GET_RUNNING				8	// XxxGetRunning

#define ISARC_CHECK_ARCHIVE				16	// XxxCheckArchive
#define ISARC_CONFIG_DIALOG				17	// XxxConfigDialog
#define ISARC_GET_FILE_COUNT			18	// XxxGetFileCount
#define ISARC_QUERY_FUNCTION_LIST		19	// XxxQueryFunctionList
#define ISARC_HOUT						20	// (XxxHOut)
#define ISARC_STRUCTOUT					21	// (XxxStructOut)
#define ISARC_GET_ARC_FILE_INFO			22	// XxxGetArcFileInfo

#define ISARC_OPEN_ARCHIVE				23	// XxxOpenArchive
#define ISARC_CLOSE_ARCHIVE				24	// XxxCloseArchive
#define ISARC_FIND_FIRST				25	// XxxFindFirst
#define ISARC_FIND_NEXT					26	// XxxFindNext
#define ISARC_EXTRACT					27	// (XxxExtract)
#define ISARC_ADD						28	// (XxxAdd)
#define ISARC_MOVE						29	// (XxxMove)
#define ISARC_DELETE					30	// (XxxDelete)

#define ISARC_GET_ARC_FILE_NAME			40	// XxxGetArcFileName
#define ISARC_GET_ARC_FILE_SIZE			41	// XxxGetArcFileSize
#define ISARC_GET_ARC_ORIGINAL_SIZE		42	// XxxArcOriginalSize
#define ISARC_GET_ARC_COMPRESSED_SIZE	43	// XxxGetArcCompressedSize
#define ISARC_GET_ARC_RATIO				44	// XxxGetArcRatio
#define ISARC_GET_ARC_DATE				45	// XxxGetArcDate
#define ISARC_GET_ARC_TIME				46	// XxxGetArcTime
#define ISARC_GET_ARC_OS_TYPE			47	// XxxGetArcOSType
#define ISARC_GET_ARC_IS_SFX_FILE		48	// XxxGetArcIsSFXFile
#define ISARC_GET_FILE_NAME				57	// XxxGetFileName
#define ISARC_GET_ORIGINAL_SIZE			58	// XxxGetOriginalSize
#define ISARC_GET_COMPRESSED_SIZE		59	// XxxGetCompressedSize
#define ISARC_GET_RATIO					60	// XxxGetRatio
#define ISARC_GET_DATE					61	// XxxGetDate
#define ISARC_GET_TIME					62	// XxxGetTime
#define ISARC_GET_CRC					63	// XxxGetCRC
#define ISARC_GET_ATTRIBUTE				64	// XxxGetAttribute
#define ISARC_GET_OS_TYPE				65	// XxxGetOSType
#define ISARC_GET_METHOD				66	// XxxGetMethod
#define ISARC_GET_WRITE_TIME			67	// XxxGetWriteTime
#define ISARC_GET_CREATE_TIME			68	// XxxGetCreateTime
#define ISARC_GET_ACCESS_TIME			69	// XxxGetAccessTime

#define ISARC_FUNCTION_END				69


//////////////////////////////////////////////////////////////////////
// コールバック

#ifdef UNICODE
	#define	WM_ARCEXTRACT				L"wm_arcextract"
#else
	#define	WM_ARCEXTRACT				"wm_arcextract"
#endif		// UNICODE

#define	ARCEXTRACT_BEGIN				0	// 該当ファイルの処理の開始
#define	ARCEXTRACT_INPROCESS			1	// 該当ファイルの展開中
#define	ARCEXTRACT_END					2	// 処理終了、関連メモリを開放
#define ARCEXTRACT_OPEN					3	// 該当書庫の処理の開始
#define ARCEXTRACT_COPY					4	// ワークファイルの書き戻し

typedef BOOL CALLBACK ARCHIVERPROC(HWND hwnd, UINT uMsg, UINT nState, LPEXTRACTINGINFOEX lpEis);
typedef ARCHIVERPROC *LPARCHIVERPROC;

/////////////////////////////////////////////////////////////////////
// OpenArchive 系 API

#define M_INIT_FILE_USE					0x00000001L	// レジストリの設定を使用
#define M_REGARDLESS_INIT_FILE			0x00000002L	// 〃 を使用しない
#define M_CHECK_ALL_PATH				0x00000100L	// 厳格なファイル名サーチ
#define M_CHECK_FILENAME_ONLY			0x00000200L	// 〃を行わない
#define M_USE_DRIVE_LETTER				0x00001000L	// ドライブ名から格納
#define M_NOT_USE_DRIVE_LETTER			0x00002000L	// 〃 を格納しない
#define M_ERROR_MESSAGE_ON				0x00400000L	// エラーメッセージを表示
#define M_ERROR_MESSAGE_OFF				0x00800000L	// 〃を表示しない
#define M_RECOVERY_ON					0x08000000L // 破損ヘッダの読み飛ばし
#define EXTRACT_FOUND_FILE				0x40000000L	// 検索されたファイルを解凍
#define EXTRACT_NAMED_FILE				0x80000000L	// 指定したファイルを解凍


//////////////////////////////////////////////////////////////////////
// 格納ファイル属性

#define FAA_RDONLY						0x01			// 書き込み保護属性
#define FAA_HIDDEN						0x02			// 隠し属性
#define FAA_SYSTEM						0x04			// システム属性
#define FAA_LABEL						0x08			// ボリューム・ラベル
#define FAA_DIR							0x10			// ディレクトリ
#define FAA_ARCH						0x20			// アーカイブ属性
#define FAA_CRYPT						0x80			// 暗号化

//////////////////////////////////////////////////////////////////////
// WARNING

#define ERROR_DISK_SPACE				0x8005
#define ERROR_READ_ONLY					0x8006
#define ERROR_USER_SKIP					0x8007
#define ERROR_UNKNOWN_TYPE				0x8008
#define ERROR_METHOD					0x8009
#define ERROR_PASSWORD_FILE				0x800A
#define ERROR_VERSION					0x800B
#define ERROR_FILE_CRC					0x800C
#define ERROR_FILE_OPEN					0x800D
#define ERROR_MORE_FRESH				0x800E
#define ERROR_NOT_EXIST					0x800F
#define ERROR_ALREADY_EXIST				0x8010
#define ERROR_TOO_MANY_FILES			0x8011

//////////////////////////////////////////////////////////////////////
// ERROR

#define ERROR_MAKEDIRECTORY				0x8012
#define ERROR_CANNOT_WRITE				0x8013
#define ERROR_HUFFMAN_CODE				0x8014
#define ERROR_COMMENT_HEADER			0x8015
#define ERROR_HEADER_CRC				0x8016
#define ERROR_HEADER_BROKEN				0x8017
#define ERROR_ARC_FILE_OPEN				0x8018
#define ERROR_NOT_ARC_FILE				0x8019
#define ERROR_CANNOT_READ				0x801A
#define ERROR_FILE_STYLE				0x801B
#define ERROR_COMMAND_NAME				0x801C
#define ERROR_MORE_HEAP_MEMORY			0x801D
#define ERROR_ENOUGH_MEMORY				0x801E
#define ERROR_ALREADY_RUNNING			0x801F
#define ERROR_USER_CANCEL				0x8020
#define ERROR_HARC_ISNOT_OPENED			0x8021
#define ERROR_NOT_SEARCH_MODE			0x8022
#define ERROR_NOT_SUPPORT				0x8023
#define ERROR_TIME_STAMP				0x8024
#define ERROR_TMP_OPEN					0x8025
#define ERROR_LONG_FILE_NAME			0x8026
#define ERROR_ARC_READ_ONLY				0x8027
#define ERROR_SAME_NAME_FILE			0x8028
#define ERROR_NOT_FIND_ARC_FILE 		0x8029
#define ERROR_RESPONSE_READ				0x802A
#define ERROR_NOT_FILENAME				0x802B
#define ERROR_TMP_COPY					0x802C
#define ERROR_END						ERROR_TMP_COPY


#endif				// __COMMONARCHIVER_H
