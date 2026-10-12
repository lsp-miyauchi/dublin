////////////////////////////////////////////////////////////
// unrarapi.h      Kuniaki Miyauchi

#ifndef __UNRAR_API_H__
#define __UNRAR_API_H__

#define UNRARAPI_VERSION	10

#ifndef FNAME_MAX32
#define FNAME_MAX32			512
#endif

#pragma pack(1)

#ifndef ARC_DECSTRACT
#define ARC_DECSTRACT

typedef	HGLOBAL				HARC;

#ifndef _A_ENCRYPTED
#define _A_ENCRYPTED		0x4000						/* 暗号化 */
#endif

typedef struct {
	DWORD 			dwOriginalSize;						/* ファイルのサイズ */
 	DWORD 			dwCompressedSize;					/* 圧縮後のサイズ */
	DWORD			dwCRC;								/* 格納ファイルのチェックサム */
	UINT			uFlag;								/* 処理結果 */
	UINT			uOSType;							/* 書庫作成に使われたＯＳ */
	WORD			wRatio;								/* 圧縮率 */
	WORD			wDate;								/* 格納ファイルの日付(DOS 形式) */
	WORD 			wTime;								/* 格納ファイルの時刻(〃) */
	char			szFileName[FNAME_MAX32 + 1];		/* 書庫名 */
	char			dummy1[3];
	char			szAttribute[8];						/* 格納ファイルの属性(書庫固有) */
	char			szMode[8];							/* 格納ファイルの格納モード(〃) */
} INDIVIDUALINFOA, *LPINDIVIDUALINFOA;

typedef struct {
	DWORD 			dwOriginalSize;						/* ファイルのサイズ */
 	DWORD 			dwCompressedSize;					/* 圧縮後のサイズ */
	DWORD			dwCRC;								/* 格納ファイルのチェックサム */
	UINT			uFlag;								/* 処理結果 */
	UINT			uOSType;							/* 書庫作成に使われたＯＳ */
	WORD			wRatio;								/* 圧縮率 */
	WORD			wDate;								/* 格納ファイルの日付(DOS 形式) */
	WORD 			wTime;								/* 格納ファイルの時刻(〃) */
	WCHAR			szFileName[FNAME_MAX32 + 1];		/* 書庫名 */
	WCHAR			dummy1[3];
	WCHAR			szAttribute[8];						/* 格納ファイルの属性(書庫固有) */
	WCHAR			szMode[8];							/* 格納ファイルの格納モード(〃) */
} INDIVIDUALINFOW, *LPINDIVIDUALINFOW;

#if !defined(UNICODE)
typedef INDIVIDUALINFOA   INDIVIDUALINFO;
typedef LPINDIVIDUALINFOA LPINDIVIDUALINFO;
#else
typedef INDIVIDUALINFOW   INDIVIDUALINFO;
typedef LPINDIVIDUALINFOW LPINDIVIDUALINFO;
#endif

typedef struct {
	DWORD 			dwFileSize;							/* 格納ファイルのサイズ */
	DWORD			dwWriteSize;						/* 書き込みサイズ */
	char			szSourceFileName[FNAME_MAX32 + 1];	/* 格納ファイル名 */
	char			dummy1[3];
	char			szDestFileName[FNAME_MAX32 + 1];	/* 解凍先または圧縮元パス名 */
	char			dummy[3];
} EXTRACTINGINFOA, *LPEXTRACTINGINFOA;

typedef struct {
	DWORD 			dwFileSize;							/* 格納ファイルのサイズ */
	DWORD			dwWriteSize;						/* 書き込みサイズ */
	WCHAR			szSourceFileName[FNAME_MAX32 + 1];	/* 格納ファイル名 */
	WCHAR			dummy1[3];
	WCHAR			szDestFileName[FNAME_MAX32 + 1];	/* 解凍先または圧縮元パス名 */
	WCHAR			dummy[3];
} EXTRACTINGINFOW, *LPEXTRACTINGINFOW;

#if !defined(UNICODE)
typedef EXTRACTINGINFOA   EXTRACTINGINFO;
typedef LPEXTRACTINGINFOA LPEXTRACTINGINFO;
#else
typedef EXTRACTINGINFOW   EXTRACTINGINFO;
typedef LPEXTRACTINGINFOW LPEXTRACTINGINFO;
#endif

typedef struct {
	EXTRACTINGINFOA	exinfo;
	DWORD			dwCompressedSize;
	DWORD			dwCRC;
	UINT			uOSType;
	WORD			wRatio;
	WORD			wDate;
	WORD			wTime;
	char			szAttribute[8];
	char			szMode[8];
} EXTRACTINGINFOEXA, *LPEXTRACTINGINFOEXA;

typedef struct {
	EXTRACTINGINFOW	exinfo;
	DWORD			dwCompressedSize;
	DWORD			dwCRC;
	UINT			uOSType;
	WORD			wRatio;
	WORD			wDate;
	WORD			wTime;
	WCHAR			szAttribute[8];
	WCHAR			szMode[8];
} EXTRACTINGINFOEXW, *LPEXTRACTINGINFOEXW;

#if !defined(UNICODE)
typedef EXTRACTINGINFOEXA   EXTRACTINGINFOEX;
typedef LPEXTRACTINGINFOEXA LPEXTRACTINGINFOEX;
#else
typedef EXTRACTINGINFOEXW   EXTRACTINGINFOEX;
typedef LPEXTRACTINGINFOEXW LPEXTRACTINGINFOEX;
#endif

#endif		/* ARC_DECSTRACT */

#pragma pack()

#define	_export

#ifdef __cplusplus
extern "C" {
#endif

/* ### LHA.DLL Ver 1.1 と互換性のある API です。### */
WORD WINAPI UnrarGetVersion(
		VOID
	);

BOOL WINAPI UnrarGetRunning(
		VOID
	);

BOOL WINAPI UnrarGetBackGroundMode(
		VOID
	);

BOOL WINAPI UnrarSetBackGroundMode(
		BOOL bBkgndMode
	);

BOOL WINAPI UnrarGetCursorMode(
		VOID
	);

BOOL WINAPI UnrarSetCursorMode(
		BOOL bCursorMode
	);

WORD WINAPI UnrarGetCursorInterval(
		VOID
	);

BOOL WINAPI UnrarSetCursorInterval(
		WORD wInterval
	);

int WINAPI Unrar(
		HWND hwnd,
		LPCSTR pszCmdLine,
		LPSTR pszOutput,
		DWORD dwSize
	);
int WINAPI UnrarA(
		HWND hwnd,
		LPCSTR pszCmdLine,
		LPSTR pszOutput,
		DWORD dwSize
	);
int WINAPI UnrarW(
		HWND hwnd,
		LPCWSTR pszCmdLine,
		LPWSTR pszOutput,
		DWORD dwSize
	);

/* ###『統合アーカイバ API』共通の API です。### */
#if !defined(CHECKARCHIVE_RAPID)
#define	CHECKARCHIVE_RAPID		0	/* 簡易(最初の３ヘッダまで) */
#define	CHECKARCHIVE_BASIC		1	/* 標準(全てのヘッダ) */
#define	CHECKARCHIVE_FULLCRC	2	/* 完全(格納ファイルの CRC チェック) */

	/* 以下のフラグは上記と組み合わせて使用。*/
#define CHECKARCHIVE_RECOVERY	4   /* 破損ヘッダを読み飛ばして処理 */
#define CHECKARCHIVE_SFX		8	/* SFX かどうかを返す */
#define CHECKARCHIVE_ALL		16	/* ファイルの最後まで検索する */
#endif

BOOL WINAPI UnrarCheckArchive(
		LPCSTR pszArchive,
		int iMode
	);
BOOL WINAPI UnrarCheckArchiveA(
		LPCSTR pszArchive,
		int iMode
	);
BOOL WINAPI UnrarCheckArchiveW(
		LPCWSTR pszArchive,
		int iMode
	);

int WINAPI UnrarGetFileCount(
		LPCSTR pszArchive
	);
int WINAPI UnrarGetFileCountA(
		LPCSTR pszArchive
	);
int WINAPI UnrarGetFileCountW(
		LPCWSTR pszArchive
	);

#if !defined(UNPACK_CONFIG_MODE)
#define	UNPACK_CONFIG_MODE		1	/* 解凍系の設定 */
#define PACK_CONFIG_MODE		2	/* 圧縮系の設定 */
#endif

BOOL WINAPI UnrarConfigDialog(
		HWND hwnd,
		LPSTR pszComBuffer,
		int iMode
	);
BOOL WINAPI UnrarConfigDialogA(
		HWND hwnd,
		LPSTR pszComBuffer,
		int iMode
	);
BOOL WINAPI UnrarConfigDialogW(
		HWND hwnd,
		LPWSTR pszComBuffer,
		int iMode
	);

#if !defined(ISARC_FUNCTION_START)
#define ISARC_FUNCTION_START			0
#define ISARC							0	/* Unrar */
#define ISARC_GET_VERSION				1	/* UnrarGetVersion */
#define ISARC_GET_CURSOR_INTERVAL		2	/* UnrarGetCursorInterval */
#define ISARC_SET_CURSOR_INTERVAL		3	/* UnrarSetCursorInterval */
#define ISARC_GET_BACK_GROUND_MODE		4	/* UnrarGetBackGroundMode */
#define ISARC_SET_BACK_GROUND_MODE		5	/* UnrarSetBackGroundMode */
#define ISARC_GET_CURSOR_MODE			6	/* UnrarGetCursorMode */
#define ISARC_SET_CURSOR_MODE			7	/* UnrarSetCursorMode */
#define ISARC_GET_RUNNING				8	/* UnrarGetRunning */

#define ISARC_CHECK_ARCHIVE				16	/* UnrarCheckArchive */
#define ISARC_CONFIG_DIALOG				17	/* UnrarConfigDialog */
#define ISARC_GET_FILE_COUNT			18	/* UnrarGetFileCount */
#define ISARC_QUERY_FUNCTION_LIST		19	/* UnrarQueryFunctionList */
#define ISARC_HOUT						20	/* (UnrarHOut) */
#define ISARC_STRUCTOUT					21	/* (UnrarStructOut) */
#define ISARC_GET_ARC_FILE_INFO			22	/* UnrarGetArcFileInfo */

#define ISARC_OPEN_ARCHIVE				23	/* UnrarOpenArchive */
#define ISARC_CLOSE_ARCHIVE				24	/* UnrarCloseArchive */
#define ISARC_FIND_FIRST				25	/* UnrarFindFirst */
#define ISARC_FIND_NEXT					26	/* UnrarFindNext */
#define ISARC_EXTRACT					27	/* (UnrarExtract) */
#define ISARC_ADD						28	/* (UnrarAdd) */
#define ISARC_MOVE						29	/* (UnrarMove) */
#define ISARC_DELETE					30	/* (UnrarDelete) */

#define ISARC_GET_ARC_FILE_NAME			40	/* UnrarGetArcFileName */
#define ISARC_GET_ARC_FILE_SIZE			41	/* UnrarGetArcFileSize */
#define ISARC_GET_ARC_ORIGINAL_SIZE		42	/* UnrarArcOriginalSize */
#define ISARC_GET_ARC_COMPRESSED_SIZE	43	/* UnrarGetArcCompressedSize */
#define ISARC_GET_ARC_RATIO				44	/* UnrarGetArcRatio */
#define ISARC_GET_ARC_DATE				45	/* UnrarGetArcDate */
#define ISARC_GET_ARC_TIME				46	/* UnrarGetArcTime */
#define ISARC_GET_ARC_OS_TYPE			47	/* UnrarGetArcOSType */
#define ISARC_GET_ARC_IS_SFX_FILE		48	/* UnrarGetArcIsSFXFile */
#define ISARC_GET_FILE_NAME				57	/* UnrarGetFileName */
#define ISARC_GET_ORIGINAL_SIZE			58	/* UnrarGetOriginalSize */
#define ISARC_GET_COMPRESSED_SIZE		59	/* UnrarGetCompressedSize */
#define ISARC_GET_RATIO					60	/* UnrarGetRatio */
#define ISARC_GET_DATE					61	/* UnrarGetDate */
#define ISARC_GET_TIME					62	/* UnrarGetTime */
#define ISARC_GET_CRC					63	/* UnrarGetCRC */
#define ISARC_GET_ATTRIBUTE				64	/* UnrarGetAttribute */
#define ISARC_GET_OS_TYPE				65	/* UnrarGetOSType */
#define ISARC_GET_METHOD				66	/* UnrarGetMethod */
#define ISARC_GET_WRITE_TIME			67	/* UnrarGetWriteTime */
#define ISARC_GET_CREATE_TIME			68	/* UnrarGetCreateTime */
#define ISARC_GET_ACCESS_TIME			69	/* UnrarGetAccessTime */

#define ISARC_FUNCTION_END				69
#endif	/* ISARC_FUNCTION_START */

BOOL WINAPI UnrarQueryFunctionList(const int iFunction);

#ifndef WM_ARCEXTRACT
#define	WM_ARCEXTRACT					L"wm_arcextract"
#define	ARCEXTRACT_BEGIN				0	/* 該当ファイルの処理の開始 */
#define	ARCEXTRACT_INPROCESS			1	/* 該当ファイルの展開中 */
#define	ARCEXTRACT_END					2	/* 処理終了、関連メモリを開放 */
#define ARCEXTRACT_OPEN					3	/* 該当書庫の処理の開始 */
#define ARCEXTRACT_COPY					4	/* ワークファイルの書き戻し */

int WINAPI UnrarSetOwnerWindow(const HWND hwnd);
BOOL WINAPI UnrarClearOwnerWindow(VOID);

typedef BOOL CALLBACK ARCHIVERPROC(HWND hwnd, UINT uMsg, UINT nState, LPEXTRACTINGINFOEX lpEis);
typedef ARCHIVERPROC *LPARCHIVERPROC;

BOOL WINAPI UnrarSetOwnerWindowEx(
		HWND hwnd,
		LPARCHIVERPROC lpArcProc
	);

BOOL WINAPI UnrarKillOwnerWindowEx(
		HWND hwnd
	);


#endif		/* WM_ARCEXTRACT */

/* OpenArchive 系 API です。 */

#if !defined(EXTRACT_FOUND_FILE)
/* MODE (for UnarjOpenArchive) */
#define M_INIT_FILE_USE					0x00000001L	/* レジストリの設定を使用 */
#define M_REGARDLESS_INIT_FILE			0x00000002L	/* 〃 を使用しない */
#define M_CHECK_ALL_PATH				0x00000100L	/* 厳格なファイル名サーチ */
#define M_CHECK_FILENAME_ONLY			0x00000200L	/* 〃を行わない */
#define M_USE_DRIVE_LETTER				0x00001000L	/* ドライブ名から格納 */
#define M_NOT_USE_DRIVE_LETTER			0x00002000L	/* 〃 を格納しない */
#define M_ERROR_MESSAGE_ON				0x00400000L	/* エラーメッセージを表示 */
#define M_ERROR_MESSAGE_OFF				0x00800000L	/* 〃を表示しない */
#define M_RECOVERY_ON					0x08000000L /* 破損ヘッダの読み飛ばし */

#define EXTRACT_FOUND_FILE				0x40000000L	/* 検索されたファイルを解凍 */
#define EXTRACT_NAMED_FILE				0x80000000L	/* 指定したファイルを解凍 */
#endif /* EXTRACT_FOUND_FILE */

HARC WINAPI UnrarOpenArchive(
		HWND hwnd,
		LPCSTR pszArchive,
		DWORD dwMode
	);
HARC WINAPI UnrarOpenArchiveA(
		HWND hwnd,
		LPCSTR pszArchive,
		DWORD dwMode
	);
HARC WINAPI UnrarOpenArchiveW(
		HWND hwnd,
		LPCWSTR pszArchive,
		DWORD dwMode
	);

int WINAPI UnrarCloseArchive(
		HARC harc
	);

int WINAPI UnrarFindFirst(
		HARC harc,
		LPCSTR pszWildName,
		INDIVIDUALINFOA* lpSubInfo
	);
int WINAPI UnrarFindFirstA(
		HARC harc,
		LPCSTR pszWildName,
		INDIVIDUALINFOA* lpSubInfo
	);
int WINAPI UnrarFindFirstW(
		HARC harc,
		LPCWSTR pszWildName,
		INDIVIDUALINFOW* lpSubInfo
	);

int WINAPI UnrarFindNext(
		HARC harc,
		INDIVIDUALINFOA* lpSubInfo
	);
int WINAPI UnrarFindNextA(
		HARC harc,
		INDIVIDUALINFOA* lpSubInfo
	);
int WINAPI UnrarFindNextW(
		HARC harc,
		INDIVIDUALINFOW* lpSubInfo
	);

int WINAPI UnrarGetArcFileName(HARC harc,
		LPSTR lpBuffer,
		int cchMax
	);
int WINAPI UnrarGetArcFileNameA(HARC harc,
		LPSTR lpBuffer,
		int cchMax
	);
int WINAPI UnrarGetArcFileNameW(HARC harc,
		LPWSTR lpBuffer,
		int cchMax
	);

DWORD WINAPI UnrarGetArcFileSize(
		HARC harc
	);

DWORD WINAPI UnrarGetArcOriginalSize(
		HARC harc
	);
DWORD WINAPI UnrarGetArcCompressedSize(
		HARC harc
	);
WORD WINAPI UnrarGetArcRatio(
		HARC harc
	);
WORD WINAPI UnrarGetArcDate(
		HARC harc
	);
WORD WINAPI UnrarGetArcTime(
		HARC harc
	);
UINT WINAPI UnrarGetArcOSType(
		HARC harc
	);
int WINAPI UnrarIsSFXFile(
		HARC harc
	);
int WINAPI UnrarGetFileName(
		HARC harc,
		LPSTR lpBuffer,
		const int cchMax
	);
int WINAPI UnrarGetFileNameA(
		HARC harc,
		LPSTR lpBuffer,
		int cchMax
	);
int WINAPI UnrarGetFileNameW(
		HARC harc,
		LPWSTR lpBuffer,
		int cchMax
	);

int WINAPI UnrarGetMethod(
		HARC harc,
		LPSTR lpBuffer,
		int cchMax
	);
int WINAPI UnrarGetMethodA(
		HARC harc,
		LPSTR lpBuffer,
		int cchMax
	);
int WINAPI UnrarGetMethodW(
		HARC harc,
		LPWSTR lpBuffer,
		int cchMax
	);

DWORD WINAPI UnrarGetOriginalSize(
		HARC harc
	);

DWORD WINAPI UnrarGetCompressedSize(
		HARC harc
	);

WORD WINAPI UnrarGetRatio(
		HARC harc
	);

WORD WINAPI UnrarGetDate(
		HARC harc
	);

WORD WINAPI UnrarGetTime(
		HARC harc
	);

DWORD WINAPI UnrarGetWriteTime(
		HARC harc
	);

DWORD WINAPI UnrarGetAccessTime(
		HARC harc
	);

DWORD WINAPI UnrarGetCreateTime(
		HARC harc
	);

DWORD WINAPI UnrarGetCRC(
		HARC harc
	);

int WINAPI UnrarGetAttribute(
		HARC harc
	);

UINT WINAPI UnrarGetOSType(
		HARC harc
	);

int WINAPI UnrarCompressMem(
		HWND hwnd,
		LPCSTR pszCmd,
		const BYTE* pszBuffer,
		DWORD dwSize,
		const time_t* pTime,
		const WORD* pwAttributes,
		DWORD* pdwWriteSize
	);
int WINAPI UnrarCompressMemA(
		HWND hwnd,
		LPCSTR pszCmd,
		const BYTE* pszBuffer,
		DWORD dwSize,
		const time_t* pTime,
		const WORD* pwAttributes,
		DWORD* pdwWriteSize
	);
int WINAPI UnrarCompressMemW(
		HWND hwnd,
		LPCWSTR pszCmd,
		const BYTE* pszBuffer,
		DWORD dwSize,
		const time_t* pTime,
		const WORD* pwAttributes,
		DWORD* pdwWriteSize
	);



DWORD WINAPI UnrarGetUncompressedTotalSize(
		LPCSTR pszUnrar
	);
DWORD WINAPI UnrarGetUncompressedTotalSizeA(
		LPCSTR pszUnrar
	);
DWORD WINAPI UnrarGetUncompressedTotalSizeW(
		LPCWSTR pszUnrar
	);

#ifdef __cplusplus
}
#endif

#ifndef FA_RDONLY
/* Attribute */
#define FA_RDONLY						0x01		/* 書き込み保護属性 */
#define FA_HIDDEN						0x02		/* 隠し属性 */
#define FA_SYSTEM						0x04		/* システム属性 */
#define FA_LABEL						0x08		/* ボリューム・ラベル */
#define FA_DIREC						0x10		/* ディレクトリ */
#define FA_ARCH							0x20		/* アーカイブ属性 */
#endif

#ifndef ERROR_ARC_FILE_OPEN
/* WARNING */
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

/* ERROR */
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
#if !defined(ERROR_ALREADY_RUNNING)
#define ERROR_ALREADY_RUNNING			0x801F
#endif
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
#endif /* ERROR_ARC_FILE_OPEN */


#endif	// __UNRAR_API_H__
