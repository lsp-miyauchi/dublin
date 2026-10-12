////////////////////////////////////////////////////////////
// cab.h

#pragma once
#ifndef __CAB_H__
#define __CAB_H__

#include <io.h>
#include <fcntl.h>
#include <time.h>
#include <fdi.h>
#include <fci.h>

#include "strseparator.h"
#include "match_exp.h"
#include "fileitem.h"
#include "cabapi.h"
#include "cabenv.h"
#include "cabfoldr.h"
#include "resource.h"

#define _SNDMSG(a,b,c,d)	(::SendMessage((HWND)(a),(UINT)(b),(WPARAM)(c),(LPARAM)(d)))

///////////////////////////////////////////////////////////
//

extern WCHAR gszAst[];
extern LPARCHIVERPROC gpfnProc;
extern UINT nRM_ARCEXTRACT;
extern void (WINAPI* _fSHChangeNotify)(LONG, UINT, LPCVOID, LPCVOID);


////////////////////////////////////////////////////////////////////
// TIME ACCESS DTE_(EXPAND)
//
#define DTE_SEC(d)		(((d) & 0x001f) << 1)
#define DTE_MIN(d)		(((d) & 0x07e0) >> 5)
#define DTE_HOUR(d)		(((d) & 0xf800) >> 11)
#define DTE_DAY(d)		(((d) & 0x001f))
#define DTE_MON(d)		(((d) & 0x01e0) >> 5)
#define DTE_YEAR(d)		(((d) & 0xfe00) >> 9)


////////////////////////////////////////////////////////////////////
// TIME ACCESS DTC_(CONSTRUCT)
//
#define DTC_SEC(d)		(((d) >> 1) & 0x001f)
#define DTC_MIN(d)		(((d) << 5) & 0x07e0)
#define DTC_HOUR(d)		(((d) << 11) & 0xf800)
#define DTC_DAY(d)		(((d) & 0x001f))
#define DTC_MON(d)		(((d) << 5) & 0x01e0)
#define DTC_YEAR(d)		(((d) << 9) & 0xfe00)


///////////////////////////////////////////////////////////
//

enum {
	CABR_NOERROR,
	CABR_PARTIALFILE,
	CABR_USERABORTED,
	CABR_CABNOTFOUND
};

///////////////////////////////////////////////////////////
//

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

///////////////////////////////////////////////////////////
//

typedef struct tagREPLDATA {
	LPCWSTR lpSrc;
	LPCWSTR lpDst;
	WIN32_FIND_DATA* pwfdSrc;
	WIN32_FIND_DATA* pwfdDst;
	DWORD nAction;
	BOOL bMultiDest;
	BOOL bYesToAll;
} REPLDATA;
typedef REPLDATA* LPREPLDATA;

enum {
	C_QUERY,
	C_NEWONLY,
	C_OVERWRITE,
	C_IGNORE
};

class CRunning
{
public:
	CRunning() { _G.AddModuleRef(); }
	~CRunning() { _G.DecModuleRef(); }
};

///////////////////////////////////////////////////////////
//

enum {
	ARCMES_START_FILE = 0,				// 該当ファイルの処理を開始することを表します。
	ARCMES_PROCESSING_FILE = 1,			// 該当ファイルの処理中であることを表します。
	ARCMES_EXIT_ARCHIVE = 2,			// 処理を終え，lpEis のメモリを開放しようとしていることを表します。
	ARCMES_START_ARCHIVE = 3,			// 該当書庫の処理を開始することを表します。szSourceFileName に書庫名が格納されます。
	ARCMES_ENBODY_TEMPFILE = 4,			// ワークファイルの書き戻しを開始することを表します。szSourceFileName にワークファイル名が格納されます。
	ARCMES_SEARCH_FILES = 5,			// ファイルをサーチ中であることを表します。検索にマッチしたファイルのファイル名が得られます。
	ARCMES_STORE_FILE_COMPLETED = 6,	// 該当ファイルの格納を終えたことを表します。圧縮サイズ等はこのメッセージがきた時点で有効となります。
	ARCMES_CHECK_ARCHIVE = 7,			// 書庫のチェックを開始することを表します。szSourceFileName にワークファイル名が格納されます。
};

///////////////////////////////////////////////////////////
// common.cpp

int WStrToStr(LPCWSTR pszWide, CStringA& str);
int StrToWStr(LPCSTR psz, CStringW& str);
bool IsLosslessUnicodeName(LPCWSTR pwzName);

int AppMessage(
			UINT idText,
			LPCWSTR pszInsert = NULL,
			UINT nType = MB_OK | MB_ICONSTOP
	);

BOOL GetFileData(
			LPCWSTR pszName,
			WIN32_FIND_DATA* pwfd
	);

BOOL MakeTree(
			LPCWSTR pszPath
	);

BOOL SetFileDateTime(
			LPCWSTR pszName,
			FILETIME* pftCreate,
			FILETIME* pftUpdate
	);

LPWSTR FormatDosDate(
				WORD wDate,
				LPWSTR pszFormat
	);

LPWSTR FormatDosTime(
				WORD wTime,
				LPWSTR pszFormat
	);


BOOL RemoveFileNoConfirm(LPCWSTR pszName);

void AttrToString(
				LPWSTR pszAttr,
				DWORD dwAttr
	);

DWORD GetModuleVersion();
BOOL NotifyOwnerMessage(
				int mode,
				EXTRACTINGINFOEX* peie
	);

void rSHChangeNotifyMakeDir(
			LPCWSTR pszPath
	);


///////////////////////////////////////////////////////////
// utfconvert.cpp

int EncodeWStrToUTF(const WCHAR* widestr, int widelen, BYTE* utfstr, int utflen);
int EncodeStrToUTF(const char* str, int len, BYTE* utfstr, int utflen);
int DecodeUTFToWStr(const BYTE* utfstr, int utflen, WCHAR* widestr, int widelen);
int DecodeUTFToStr(const BYTE* utfstr, int utflen, char* str, int len);
BOOL IsUTFEncoded(const BYTE* bytestr);

///////////////////////////////////////////////////////////
// nextcab.cpp

BOOL FindNextCab(HWND hwnd, LPWSTR pszCabFile);

///////////////////////////////////////////////////////////
//

BOOL FDIGetCabinetInfo(LPCWSTR pszCab, FDICABINETINFO* pinfo);
CCabFolder* FDIOpenCabinet(HWND hwndOwner, LPCWSTR pszCabinetFile, DWORD mode);

///////////////////////////////////////////////////////////
//

BOOL QueryReplace(HWND hwnd, LPREPLDATA lprd);

///////////////////////////////////////////////////////////
// reg.cpp

int GetRegBinary(LPCWSTR pSec, LPCWSTR pKey, LPBYTE pData, int nLen);
BYTE* GetRegBinaryEx(LPCWSTR pSec, LPCWSTR pKey, LPINT pnLen);
BOOL SetRegBinary(LPCWSTR pSec, LPCWSTR pKey, LPBYTE pData, int nLen);
int GetRegString(LPCWSTR pSec, LPCWSTR pKey, LPWSTR pStr, int cchMax);
BOOL SetRegString(LPCWSTR pSec, LPCWSTR pKey, LPCWSTR pStr);
int GetRegInt(LPCWSTR pSec, LPCWSTR pKey, int nDefValue);
BOOL SetRegInt(LPCWSTR pSec, LPCWSTR pKey, int nData);

///////////////////////////////////////////////////////////
// testcab.cpp

BOOL CabCheckInside(CCabEnv* pev, LPBYTE pBase);
BOOL CabGetCompressionType(LPCWSTR pszName, CSimpleArray<WORD>& array);



#endif		// __CAB_H__
