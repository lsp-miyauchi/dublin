#pragma once

#include <iostream>
#include <string>
#include <vector>
#include <memory>
#include <format>
#include <array>
#include <sstream>
#include <iterator>

#if !defined(ISARC_FUNCTION_START)
#define ISARC_FUNCTION_START        0
#define ISARC							          0	/* Unlha */
#define ISARC_GET_VERSION				    1	/* UnlhaGetVersion */
#define ISARC_GET_CURSOR_INTERVAL		2	/* UnlhaGetCursorInterval */
#define ISARC_SET_CURSOR_INTERVAL		3	/* UnlhaSetCursorInterval */
#define ISARC_GET_BACK_GROUND_MODE	4	/* UnlhaGetBackGroundMode */
#define ISARC_SET_BACK_GROUND_MODE	5	/* UnlhaSetBackGroundMode */
#define ISARC_GET_CURSOR_MODE			  6	/* UnlhaGetCursorMode */
#define ISARC_SET_CURSOR_MODE       7	/* UnlhaSetCursorMode */
#define ISARC_GET_RUNNING           8	/* UnlhaGetRunning */

#define ISARC_CHECK_ARCHIVE				  16	/* UnlhaCheckArchive */
#define ISARC_CONFIG_DIALOG				  17	/* UnlhaConfigDialog */
#define ISARC_GET_FILE_COUNT        18	/* UnlhaGetFileCount */
#define ISARC_QUERY_FUNCTION_LIST   19	/* UnlhaQueryFunctionList */
#define ISARC_HOUT                  20	/* (UnlhaHOut) */
#define ISARC_STRUCTOUT             21	/* (UnlhaStructOut) */
#define ISARC_GET_ARC_FILE_INFO     22	/* UnlhaGetArcFileInfo */

#define ISARC_OPEN_ARCHIVE          23	/* UnlhaOpenArchive */
#define ISARC_CLOSE_ARCHIVE         24	/* UnlhaCloseArchive */
#define ISARC_FIND_FIRST            25	/* UnlhaFindFirst */
#define ISARC_FIND_NEXT             26	/* UnlhaFindNext */
#define ISARC_EXTRACT               27	/* (UnlhaExtract) */
#define ISARC_ADD                   28	/* (UnlhaAdd) */
#define ISARC_MOVE                  29	/* (UnlhaMove) */
#define ISARC_DELETE                30	/* (UnlhaDelete) */
#define ISARC_SETOWNERWINDOW        31	/* UnlhaSetOwnerWindow */
#define ISARC_CLEAROWNERWINDOW      32	/* UnlhaClearOwnerWindow */
#define ISARC_SETOWNERWINDOWEX      33	/* UnlhaSetOwnerWindowEx */
#define ISARC_KILLOWNERWINDOWEX     34	/* UnlhaKillOwnerWindowEx */

#define ISARC_GET_ARC_FILE_NAME     40	/* UnlhaGetArcFileName */
#define ISARC_GET_ARC_FILE_SIZE     41	/* UnlhaGetArcFileSize */
#define ISARC_GET_ARC_ORIGINAL_SIZE 42	/* UnlhaArcOriginalSize */
#define ISARC_GET_ARC_COMPRESSED_SIZE 43	/* UnlhaGetArcCompressedSize */
#define ISARC_GET_ARC_RATIO         44	/* UnlhaGetArcRatio */
#define ISARC_GET_ARC_DATE          45	/* UnlhaGetArcDate */
#define ISARC_GET_ARC_TIME          46	/* UnlhaGetArcTime */
#define ISARC_GET_ARC_OS_TYPE       47	/* UnlhaGetArcOSType */
#define ISARC_GET_ARC_IS_SFX_FILE   48	/* UnlhaGetArcIsSFXFile */
#define ISARC_GET_ARC_WRITE_TIME_EX 49	/* UnlhaGetArcWriteTimeEx */
#define ISARC_GET_ARC_CREATE_TIME_EX 50	/* UnlhaGetArcCreateTimeEx */
#define ISARC_GET_ARC_ACCESS_TIME_EX 51	/* UnlhaGetArcAccessTimeEx */
#define ISARC_GET_ARC_CREATE_TIME_EX2 52	/* UnlhaGetArcCreateTimeEx2 */
#define ISARC_GET_FILE_NAME         57	/* UnlhaGetFileName */
#define ISARC_GET_ORIGINAL_SIZE     58	/* UnlhaGetOriginalSize */
#define ISARC_GET_COMPRESSED_SIZE   59	/* UnlhaGetCompressedSize */
#define ISARC_GET_RATIO             60	/* UnlhaGetRatio */
#define ISARC_GET_DATE              61	/* UnlhaGetDate */
#define ISARC_GET_TIME              62	/* UnlhaGetTime */
#define ISARC_GET_CRC               63	/* UnlhaGetCRC */
#define ISARC_GET_ATTRIBUTE         64	/* UnlhaGetAttribute */
#define ISARC_GET_OS_TYPE           65	/* UnlhaGetOSType */
#define ISARC_GET_METHOD            66	/* UnlhaGetMethod */
#define ISARC_GET_WRITE_TIME        67	/* UnlhaGetWriteTime */
#define ISARC_GET_CREATE_TIME       68	/* UnlhaGetCreateTime */
#define ISARC_GET_ACCESS_TIME       69	/* UnlhaGetAccessTime */
#define ISARC_GET_WRITE_TIME_EX     70	/* UnlhaGetWriteTimeEx */
#define ISARC_GET_CREATE_TIME_EX    71	/* UnlhaGetCreateTimeEx */
#define ISARC_GET_ACCESS_TIME_EX    72	/* UnlhaGetAccessTimeEx */
#define ISARC_SET_ENUM_MEMBERS_PROC 80  /* UnlhaSetEnumMembersProc */
#define ISARC_CLEAR_ENUM_MEMBERS_PROC 81	/* UnlhaClearEnumMembersProc */

//////////////////////////////////////////////////////////////////////
// ERROR

#define ERROR_MAKEDIRECTORY       0x8012
#define ERROR_CANNOT_WRITE        0x8013
#define ERROR_HUFFMAN_CODE        0x8014
#define ERROR_COMMENT_HEADER      0x8015
#define ERROR_HEADER_CRC          0x8016
#define ERROR_HEADER_BROKEN       0x8017
#define ERROR_ARC_FILE_OPEN       0x8018
#define ERROR_NOT_ARC_FILE        0x8019
#define ERROR_CANNOT_READ         0x801A
#define ERROR_FILE_STYLE          0x801B
#define ERROR_COMMAND_NAME        0x801C
#define ERROR_MORE_HEAP_MEMORY    0x801D
#define ERROR_ENOUGH_MEMORY       0x801E
#define ERROR_ALREADY_RUNNING     0x801F
#define ERROR_USER_CANCEL         0x8020
#define ERROR_HARC_ISNOT_OPENED   0x8021
#define ERROR_NOT_SEARCH_MODE     0x8022
#define ERROR_NOT_SUPPORT         0x8023
#define ERROR_TIME_STAMP          0x8024
#define ERROR_TMP_OPEN            0x8025
#define ERROR_LONG_FILE_NAME      0x8026
#define ERROR_ARC_READ_ONLY       0x8027
#define ERROR_SAME_NAME_FILE      0x8028
#define ERROR_NOT_FIND_ARC_FILE   0x8029
#define ERROR_RESPONSE_READ       0x802A
#define ERROR_NOT_FILENAME        0x802B
#define ERROR_TMP_COPY            0x802C
#define ERROR_END                 ERROR_TMP_COPY

#endif	/* ISARC_FUNCTION_START */

#ifndef FNAME_MAX32
#define FNAME_MAX32		512
#endif

#pragma pack(1)
struct INDIVIDUALINFOA {
  DWORD 		dwOriginalSize;		/* ファイルのサイズ */
  DWORD 		dwCompressedSize;	/* 圧縮後のサイズ */
  DWORD			dwCRC;				/* 格納ファイルのチェックサム */
  /* @@@@
    UINT			uFlag;				// 処理結果
  @@@@ */
  DWORD			fCrypt : 1;			/* 暗号化 */
  DWORD			fDummy : 31;
  UINT			uOSType;			/* 書庫作成に使われたＯＳ */
  WORD			wRatio;				/* 圧縮率 */
  WORD			wDate;				/* 格納ファイルの日付(DOS 形式) */
  WORD 			wTime;				/* 格納ファイルの時刻(〃) */
  char			szFileName[FNAME_MAX32 + 1];	/* 書庫名 */
  char			dummy1[3];
  char			szAttribute[8];		/* 格納ファイルの属性(書庫固有) */
  char			szMode[8];			/* 格納ファイルの格納モード(〃) */
};

struct INDIVIDUALINFOW {
  DWORD 		dwOriginalSize;		/* ファイルのサイズ */
  DWORD 		dwCompressedSize;	/* 圧縮後のサイズ */
  DWORD			dwCRC;				/* 格納ファイルのチェックサム */
  /* @@@@
    UINT			uFlag;				// 処理結果
  @@@@ */
  DWORD			fCrypt : 1;			/* 暗号化 */
  DWORD			fDummy : 31;
  UINT			uOSType;			/* 書庫作成に使われたＯＳ */
  WORD			wRatio;				/* 圧縮率 */
  WORD			wDate;				/* 格納ファイルの日付(DOS 形式) */
  WORD 			wTime;				/* 格納ファイルの時刻(〃) */
  wchar_t		szFileName[FNAME_MAX32 + 1];	/* 書庫名 */
  wchar_t		dummy1[3];
  wchar_t		szAttribute[8];		/* 格納ファイルの属性(書庫固有) */
  wchar_t		szMode[8];			/* 格納ファイルの格納モード(〃) */
};

struct INDIVIDUALINFO64A {
  ULONGLONG ullOriginalSize;    /* ファイルのサイズ */
  ULONGLONG ullCompressedSize;  /* 圧縮後のサイズ */
  DWORD			dwCRC;              /* 格納ファイルのチェックサム */
  /* @@@@
    UINT			uFlag;				// 処理結果
  @@@@ */
  DWORD			fCrypt : 1;					/* 暗号化 */
  DWORD			fDummy : 31;
  UINT			uOSType;						/* 書庫作成に使われたＯＳ */
  WORD			wRatio;				      /* 圧縮率 */
  WORD			wDate;				/* 格納ファイルの日付(DOS 形式) */
  WORD 			wTime;				/* 格納ファイルの時刻(〃) */
  char			szFileName[FNAME_MAX32 + 1];	/* 書庫名 */
  char			dummy1[3];
  char			szAttribute[8];		/* 格納ファイルの属性(書庫固有) */
  char			szMode[8];			/* 格納ファイルの格納モード(〃) */
};

struct INDIVIDUALINFO64W {
  ULONGLONG ullOriginalSize;       /* ファイルのサイズ */
  ULONGLONG ullCompressedSize;     /* 圧縮後のサイズ */
  DWORD			dwCRC;              /* 格納ファイルのチェックサム */
  /* @@@@
    UINT			uFlag;				// 処理結果
  @@@@ */
  DWORD			fCrypt : 1;					/* 暗号化 */
  DWORD			fDummy : 31;
  UINT			uOSType;						/* 書庫作成に使われたＯＳ */
  WORD			wRatio;				      /* 圧縮率 */
  WORD			wDate;				/* 格納ファイルの日付(DOS 形式) */
  WORD 			wTime;				/* 格納ファイルの時刻(〃) */
  wchar_t		szFileName[FNAME_MAX32 + 1];	/* 書庫名 */
  wchar_t		dummy1[3];
  wchar_t		szAttribute[8];		/* 格納ファイルの属性(書庫固有) */
  wchar_t		szMode[8];			/* 格納ファイルの格納モード(〃) */
};

#ifndef __PACKED_FILEITEM_DEFINED
#define __PACKED_FILEITEM_DEFINED

struct PACKED_FILEITEM {
  ULONGLONG originalSize;
  ULONGLONG compressedSize;
  unsigned cfactor;
  unsigned ratio;
  SYSTEMTIME fstime;
  unsigned attributs;
  char filename[MAX_PATH * 2];
  char method[8];
  ULONG crc;
  BOOL crypt;
  char lcflag;
};

#endif  /* __ZIPPED_FILEITEM_DEFINED */


#pragma pack()

#ifdef UNICODE
typedef struct INDIVIDUALINFOW			INDIVIDUALINFO;
typedef struct INDIVIDUALINFO64W		INDIVIDUALINFO64;
#else
typedef struct INDIVIDUALINFOA			INDIVIDUALINFO;
typedef struct INDIVIDUALINFO64A		INDIVIDUALINFO64;
#endif	// UNICODE

struct PASSENTER {
  LPCSTR pszFile;
  LPSTR pszCode;
};

struct PROGRESSINFO {
  BOOL bCanceled;
};

struct REPLACEINFO {
  LPCSTR pszName;
  DWORD dwReplace;
};
