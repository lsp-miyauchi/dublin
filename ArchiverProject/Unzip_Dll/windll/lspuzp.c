#ifndef WIN32   /* this code is currently only tested for 32-bit console */
	#define WIN32
#endif

#if defined(__WIN32__) && !defined(WIN32)
	#define WIN32
#endif

#include "windll.h"
#include <windowsx.h>
#include <shlwapi.h>
#include <commctrl.h>
#ifdef __RSXNT__
	#include "win32/rsxntwin.h"
#endif
#include <sys/types.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <string.h>
#include <mbctype.h>
#include <mbstring.h>
#ifndef __RSXNT__
	#include <winver.h>
#else
	#include <winversi.h>
#endif
#include "resource.h"
#include "unzver.h"

#pragma comment(lib, "shlwapi.lib")


void __cdecl dbgPrintf(const char* pszFormat, ...);


extern BOOL gbTryLastPassword;


#if !defined(ISARC_FUNCTION_START)
#define ISARC_FUNCTION_START			0
#define ISARC							0	/* Unlha */
#define ISARC_GET_VERSION				1	/* UnlhaGetVersion */
#define ISARC_GET_CURSOR_INTERVAL		2	/* UnlhaGetCursorInterval */
#define ISARC_SET_CURSOR_INTERVAL		3	/* UnlhaSetCursorInterval */
#define ISARC_GET_BACK_GROUND_MODE		4	/* UnlhaGetBackGroundMode */
#define ISARC_SET_BACK_GROUND_MODE		5	/* UnlhaSetBackGroundMode */
#define ISARC_GET_CURSOR_MODE			6	/* UnlhaGetCursorMode */
#define ISARC_SET_CURSOR_MODE			7	/* UnlhaSetCursorMode */
#define ISARC_GET_RUNNING				8	/* UnlhaGetRunning */

#define ISARC_CHECK_ARCHIVE				16	/* UnlhaCheckArchive */
#define ISARC_CONFIG_DIALOG				17	/* UnlhaConfigDialog */
#define ISARC_GET_FILE_COUNT			18	/* UnlhaGetFileCount */
#define ISARC_QUERY_FUNCTION_LIST		19	/* UnlhaQueryFunctionList */
#define ISARC_HOUT						20	/* (UnlhaHOut) */
#define ISARC_STRUCTOUT					21	/* (UnlhaStructOut) */
#define ISARC_GET_ARC_FILE_INFO			22	/* UnlhaGetArcFileInfo */

#define ISARC_OPEN_ARCHIVE				23	/* UnlhaOpenArchive */
#define ISARC_CLOSE_ARCHIVE				24	/* UnlhaCloseArchive */
#define ISARC_FIND_FIRST				25	/* UnlhaFindFirst */
#define ISARC_FIND_NEXT					26	/* UnlhaFindNext */
#define ISARC_EXTRACT					27	/* (UnlhaExtract) */
#define ISARC_ADD						28	/* (UnlhaAdd) */
#define ISARC_MOVE						29	/* (UnlhaMove) */
#define ISARC_DELETE					30	/* (UnlhaDelete) */
#define ISARC_SETOWNERWINDOW			31	/* UnlhaSetOwnerWindow */
#define ISARC_CLEAROWNERWINDOW			32	/* UnlhaClearOwnerWindow */
#define ISARC_SETOWNERWINDOWEX			33	/* UnlhaSetOwnerWindowEx */
#define ISARC_KILLOWNERWINDOWEX			34	/* UnlhaKillOwnerWindowEx */

#define ISARC_GET_ARC_FILE_NAME			40	/* UnlhaGetArcFileName */
#define ISARC_GET_ARC_FILE_SIZE			41	/* UnlhaGetArcFileSize */
#define ISARC_GET_ARC_ORIGINAL_SIZE		42	/* UnlhaArcOriginalSize */
#define ISARC_GET_ARC_COMPRESSED_SIZE	43	/* UnlhaGetArcCompressedSize */
#define ISARC_GET_ARC_RATIO				44	/* UnlhaGetArcRatio */
#define ISARC_GET_ARC_DATE				45	/* UnlhaGetArcDate */
#define ISARC_GET_ARC_TIME				46	/* UnlhaGetArcTime */
#define ISARC_GET_ARC_OS_TYPE			47	/* UnlhaGetArcOSType */
#define ISARC_GET_ARC_IS_SFX_FILE		48	/* UnlhaGetArcIsSFXFile */
#define ISARC_GET_ARC_WRITE_TIME_EX		49	/* UnlhaGetArcWriteTimeEx */
#define ISARC_GET_ARC_CREATE_TIME_EX	50	/* UnlhaGetArcCreateTimeEx */
#define	ISARC_GET_ARC_ACCESS_TIME_EX	51	/* UnlhaGetArcAccessTimeEx */
#define	ISARC_GET_ARC_CREATE_TIME_EX2	52	/* UnlhaGetArcCreateTimeEx2 */
#define ISARC_GET_FILE_NAME				57	/* UnlhaGetFileName */
#define ISARC_GET_ORIGINAL_SIZE			58	/* UnlhaGetOriginalSize */
#define ISARC_GET_COMPRESSED_SIZE		59	/* UnlhaGetCompressedSize */
#define ISARC_GET_RATIO					60	/* UnlhaGetRatio */
#define ISARC_GET_DATE					61	/* UnlhaGetDate */
#define ISARC_GET_TIME					62	/* UnlhaGetTime */
#define ISARC_GET_CRC					63	/* UnlhaGetCRC */
#define ISARC_GET_ATTRIBUTE				64	/* UnlhaGetAttribute */
#define ISARC_GET_OS_TYPE				65	/* UnlhaGetOSType */
#define ISARC_GET_METHOD				66	/* UnlhaGetMethod */
#define ISARC_GET_WRITE_TIME			67	/* UnlhaGetWriteTime */
#define ISARC_GET_CREATE_TIME			68	/* UnlhaGetCreateTime */
#define ISARC_GET_ACCESS_TIME			69	/* UnlhaGetAccessTime */
#define ISARC_GET_WRITE_TIME_EX			70	/* UnlhaGetWriteTimeEx */
#define ISARC_GET_CREATE_TIME_EX		71	/* UnlhaGetCreateTimeEx */
#define ISARC_GET_ACCESS_TIME_EX		72	/* UnlhaGetAccessTimeEx */
#define ISARC_SET_ENUM_MEMBERS_PROC		80  /* UnlhaSetEnumMembersProc */
#define ISARC_CLEAR_ENUM_MEMBERS_PROC	81	/* UnlhaClearEnumMembersProc */

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


#endif	/* ISARC_FUNCTION_START */

typedef struct tagPASSENTER {
	LPCSTR pszFile;
	LPSTR pszCode;
} PASSENTER;

typedef struct tagMETERINFO {
	DWORD dwTotal;
	DWORD dwDone;
	BOOL bCanceled;
} METERINFO, *LPMETERINFO;

typedef struct tagREPLACEINFO {
	LPCSTR pszName;
	DWORD dwReplace;
} REPLACEINFO, *LPREPLACEINFO;

#define TICKS			100

////////////////////////////////////////////////////////////
// Forward References

extern HANDLE hInst;
extern DWORD gdwLocked;

int WINAPI LuzSingleEntryUnzip(int, char**, int, char**, LPDCL, LPUSERFUNCTIONS);

int WINAPI LuzDisplay(void*, LPSTR, ULONG);
int WINAPI LuzReplace(void* pInfo, char *filename);
int WINAPI LuzPassword(char *, int, const char *);
void WINAPI LuzMessage(void*, ULONG, ULONG, unsigned,
    SYSTEMTIME*, BOOL, LPSTR, LPSTR, ULONG, BOOL);
int WINAPI LuzReport(void*, UINT, LPCSTR, LPCSTR, ULONG);

BOOL WINAPI PasswordDlgFn(HWND hdlg, UINT msg, WPARAM wParam, LPARAM lParam);
BOOL WINAPI MeterDlgFn(HWND hdlg, UINT msg, WPARAM wParam, LPARAM lParam);
BOOL WINAPI ReplaceDlgFn(HWND hdlg, UINT msg, WPARAM wParam, LPARAM lParam);
int DlgModal(HWND hwnd, DLGPROC lpFn, int nID, void* pvParam);
HWND DlgModeless(HWND hwnd, DLGPROC lpFn, int nID, void* pvParam);


////////////////////////////////////////////////////////////
//

void ReplaceChar(LPSTR pszText, int nSrcChar, int nDstChar)
{
	register char *p = pszText;
	while (p) {
		if (p = (char*)_mbschr((BYTE*)p, nSrcChar))
			*p++ = nDstChar;
	}
}

char* SkipTabSpace(char* p)
{
	if (p) {
		while (*p == ' ' || *p == '\t')
			p++;
	}
	return p;
}

int SeparateString(char* pszText, char** argv, int maxentry)
{
	int i;
	char *p, *q;

	i = 0;
	p = SkipTabSpace(pszText);
	while (p && *p && i < maxentry) {
		if (*p == '"') {
			q = (char*)_mbschr((BYTE*)(++p), '"');
			if (q)
				*q++ = '\0';
			else
				break;
		}
		else {
			q = (char*)_mbschr((BYTE*)(p + 1), ' ');
			if (q)
				*q++ = '\0';
		}
		argv[i++] = p;
		p = SkipTabSpace(q);
	}
	return i;
}

BOOL setswitchs(LPDCL pDcl, LPCSTR psw)
{
	while (psw && *psw) {
		switch (*psw) {
			case 'a':
				pDcl->fConvCR = TRUE;		// Convert CR to CRLF
				break;
			case '^':
				pDcl->fInsideVerb = TRUE;
			case 'v':
				pDcl->fVerbose = TRUE;
				break;
			case 't':
				pDcl->fTest = TRUE;
				break;
			case 'z':
				pDcl->fComment = TRUE;
				break;
			case 'j':						// junk paths
				pDcl->fMkdir = FALSE;
				break;
			case 'o':						// Over-write all files
				pDcl->fOverwrite = TRUE;
				break;
			case 'q':
				pDcl->fQuiet = 2;
				break;
			case 'u':
				pDcl->fUpdate = TRUE;
				break;
			case 'f':
				pDcl->fOnlyNewer = TRUE;
				break;
			case 'x':
				break;
			case '-':
				psw++;
				if (*psw == 'i')
					pDcl->fProgress = FALSE;
				break;
			case 'P':						// コマンドラインパスワード
				psw++;
				if (*psw) {
					pDcl->lpszPassword = (LPSTR)psw;
					return TRUE;
				}
				return FALSE;
			default:
				break;
		}
		psw++;
	}
	return TRUE;
}

static BOOL PerseResponseFile(LPDCL pDcl, LPCSTR pszResp)
{
	int len;
	char *argv[128], szText[MAX_PATH * 4];
	FILE* fp;

	fp = fopen(pszResp, "r");
	if (fp == NULL)
		return FALSE;

	while (fgets(szText, MAX_PATH * 4, fp)) {
		len = lstrlen(szText);
		if (len > 0 && szText[len - 1] == '\n')
			szText[len - 1] = '\0';

		len = SeparateString(szText, argv, 128);
		if (len > 0) {
			int i;
			for (i = 0; i < len; i++) {
				if (pDcl->iMaxFilters < pDcl->iNumFilters + 1) {
					char** ppw = (char**)realloc(pDcl->ppFilters, sizeof(char*) * (pDcl->iMaxFilters + 256));
					if (!ppw)
						break;

					pDcl->ppFilters = ppw;
					pDcl->iMaxFilters += 256;
				}
				pDcl->ppFilters[pDcl->iNumFilters++] = _strdup(argv[i]);
			}
		}
	}
	fclose(fp);
	return TRUE;
}

BOOL WINAPI DllGetFunctionPrefix(LPSTR pszPrefix, LONG cchMax)
{
	if (pszPrefix) {
		lstrcpyn(pszPrefix, "UnZip", cchMax);
		return TRUE;
	}
	return FALSE;
}

BOOL WINAPI UnZipLocked(DWORD dw)
{
	gdwLocked = dw;
	return (gdwLocked != 0);
}

int WINAPI UnZip(HWND hwnd, LPCSTR pszCmd, LPSTR pszResult, DWORD dwSize)
{
	int i, argc, result, exfc;
	char *p, *cmd, *argv[256], **exfv, dir[MAX_PATH * 2];
	LPUSERFUNCTIONS pUserFn;
	LPDCL pDcl;

	if (!gdwLocked)
		return -1;

	pDcl = GlobalAllocPtr(GHND, (DWORD)sizeof(DCL));
	if (!pDcl)
		return -1;

	pUserFn = GlobalAllocPtr(GHND, (DWORD)sizeof(USERFUNCTIONS));
	if (!pUserFn) {
		GlobalFreePtr(pDcl);
		return -1;
	}

	cmd = _strdup(pszCmd);
	argc = SeparateString(cmd, argv, 256);
	if (argc < 1) {
		free(cmd);
		return -1;
	}

	// コールバック関数の設定
	pUserFn->password = LuzPassword;
	pUserFn->print = LuzDisplay;
	pUserFn->replace = LuzReplace;
	pUserFn->message = LuzMessage;
	pUserFn->report = LuzReport;
	pUserFn->hwndParent = hwnd;
	pUserFn->pResult = pszResult;

	if (pszResult && dwSize > 0) {
		*pszResult = '\0';
		pUserFn->pResult = pszResult;
		pUserFn->nResultLength = dwSize;
	}
	else
		pUserFn->nResultLength = 0;

	pUserFn->hMeter = NULL;
	pUserFn->pDcl = pDcl;
	pUserFn->nReplace = 0;

	// デフォルト処理フラグの設定
	pDcl->fOnlyNewer = FALSE;
	pDcl->fAskOverwrite = TRUE;			// true if prompt to overwrite is wanted
	pDcl->fSpc2Undscr = FALSE;			// true if convert space to underscore
	pDcl->fConsole = FALSE;				// Write to stdout if true
	pDcl->fQuiet = FALSE;				// 0 = all messages
										// 1 = fewer messages
										// 2 = no messages
	pDcl->fTest = FALSE;				// test zip file if true
	pDcl->fVerbose = FALSE;				// give a verbose listing if true
	pDcl->fUpdate = FALSE;				// Do not extract only newer
	pDcl->fComment = FALSE;				// display a zip file comment if true
	pDcl->fMkdir = TRUE;				// Recreate directories if true
	pDcl->fOverwrite = FALSE;			// Over-write all files if true
	pDcl->fConvCR = FALSE;				// Do not convert CR to CRLF
	pDcl->lpszZipFN = NULL;				// The archive name
	pDcl->lpszExtractDir = NULL;		// The directory to extract to. This is set
										// to NULL if you are extracting to the
										// current directory.
	pDcl->lpszPassword = NULL;			// パスワード
	pDcl->fCase = FALSE;				// ignore case if TRUE
	pDcl->fProgress = TRUE;

	// コマンドスイッチ
	pDcl->iNumFilters = 0;
	pDcl->ppFilters = (char**)malloc(sizeof(char*) * 256);
	pDcl->iMaxFilters = 256;
	for (i = 0; i < argc; i++) {
		p = argv[i];
		if (*p == '-')					// コマンドスイッチ
			setswitchs(pDcl, p + 1);
		else if (*p == '@')				// ファイルリスト
			PerseResponseFile(pDcl, ++p);
		else {
			int len;
			len = lstrlen(p) - 1;
			if (p[len] == '\\' && _mbsbtype((BYTE*)p, len) == _MBC_SINGLE) {
				// 解凍先フォルダ
				pDcl->lpszExtractDir = p;
			}
			else if (!pDcl->lpszZipFN) {
				// ZIP書庫名
				pDcl->lpszZipFN = p;
			}
			else {
				// 解凍ファイル
				if (pDcl->iMaxFilters < pDcl->iNumFilters + 1) {
					char** ppw = (char**)realloc(pDcl->ppFilters, sizeof(char*) * (pDcl->iMaxFilters + 256));
					if (!ppw)
						break;

					pDcl->ppFilters = ppw;
					pDcl->iMaxFilters += 256;
				}
				pDcl->ppFilters[pDcl->iNumFilters++] = _strdup(p);
			}
		}
	}

	// 解凍除外ファイルはここでは無視！
	exfc = 0;
	exfv = NULL;

	if (!pDcl->fTest && !pDcl->fVerbose) {
		if (!pDcl->lpszExtractDir) {
			GetCurrentDirectory(MAX_PATH * 2, dir);
			pDcl->lpszExtractDir = dir;
		}

		if (pDcl->fProgress) {
			pUserFn->hMeter = DlgModeless(hwnd, (DLGPROC)MeterDlgFn, IDD_RELAY, 0);
			if (pUserFn->hMeter)
				PathSetDlgItemPath(pUserFn->hMeter, IDC_FROM, pDcl->lpszZipFN);
			ShowWindow(pUserFn->hMeter, SW_SHOW);
			UpdateWindow(pUserFn->hMeter);
		}
	}

	result = LuzSingleEntryUnzip(pDcl->iNumFilters, pDcl->ppFilters, exfc, exfv, pDcl, pUserFn);

	if (pUserFn->hMeter)
		DestroyWindow(pUserFn->hMeter);

	if (pDcl->iNumFilters > 0) {
		for (i = 0; i < pDcl->iNumFilters; i++)
			free(pDcl->ppFilters[i]);
	}
	free(pDcl->ppFilters);

	free(cmd);
	GlobalFreePtr(pDcl);
	GlobalFreePtr(pUserFn);

	if (result != 0) {
		switch (result) {
			case IZ_CTRLC: result = ERROR_USER_CANCEL; break;
			case PK_WARN: result = 0; break;		// 警告はno-error
			case PK_MEM:
			case PK_MEM2:
			case PK_MEM3:
			case PK_MEM4:
			case PK_MEM5: result = ERROR_ENOUGH_MEMORY; break;
			case PK_NOZIP: result = ERROR_NOT_ARC_FILE; break;
			case PK_PARAM: result = ERROR_COMMAND_NAME; break;
			case PK_FIND: result = ERROR_NOT_FILENAME; break;
			case PK_DISK: result = ERROR_CANNOT_WRITE; break;
			default: result = -1; break;
		}
	}
	return result;
}

WORD WINAPI UnZipGetVersion()
{
	return 90;
}

BOOL WINAPI UnZipGetRunning()
{
	return TRUE;
}

BOOL WINAPI UnZipGetBackGroundMode()
{
	return FALSE;
}

BOOL WINAPI UnZipSetBackGroundMode(BOOL bBkgndMode)
{
	return TRUE;
}

BOOL WINAPI UnZipGetCursorMode()
{
	return TRUE;
}

BOOL WINAPI UnZipSetCursorMode(BOOL bCursorMode)
{
	return TRUE;
}

WORD WINAPI UnZipGetCursorInterval()
{
	return 0;
}

BOOL WINAPI UnZipSetCursorInterval(WORD wInterval)
{
	return TRUE;
}

BOOL WINAPI UnZipCheckArchive(LPCSTR pszName, int iMode)
{
	char cmd[MAX_PATH * 3];
	wsprintf(cmd, "-t \"%s\"", pszName);
	return (UnZip(NULL, cmd, NULL, 0) == 0) ? TRUE : FALSE;
}

BOOL WINAPI UnZipQueryFunctionList(int iFunction)
{
	switch (iFunction) {
		case ISARC_GET_VERSION:
		case ISARC_CHECK_ARCHIVE:
		case ISARC_GET_FILE_COUNT:
		case ISARC_QUERY_FUNCTION_LIST:
		case ISARC_OPEN_ARCHIVE:
		case ISARC_CLOSE_ARCHIVE:
		case ISARC_FIND_FIRST:
		case ISARC_FIND_NEXT:
		case ISARC_GET_CURSOR_INTERVAL:
		case ISARC_SET_CURSOR_INTERVAL:
		case ISARC_GET_BACK_GROUND_MODE:
		case ISARC_SET_BACK_GROUND_MODE:
		case ISARC_GET_CURSOR_MODE:
		case ISARC_SET_CURSOR_MODE:
			return TRUE;

		case ISARC:
		case ISARC_GET_RUNNING:
		case ISARC_GET_ARC_FILE_NAME:
		case ISARC_GET_ARC_FILE_SIZE:
		case ISARC_GET_ARC_ORIGINAL_SIZE:
		case ISARC_GET_ARC_DATE:
		case ISARC_GET_ARC_TIME:
		case ISARC_GET_FILE_NAME:
		case ISARC_GET_ORIGINAL_SIZE:
		case ISARC_GET_DATE:
		case ISARC_GET_TIME:
		case ISARC_CONFIG_DIALOG:
		case ISARC_HOUT:
		case ISARC_STRUCTOUT:
		case ISARC_GET_ARC_FILE_INFO:
		case ISARC_EXTRACT:
		case ISARC_ADD:
		case ISARC_MOVE:
		case ISARC_DELETE:
		case ISARC_GET_ARC_COMPRESSED_SIZE:
		case ISARC_GET_ARC_RATIO:
		case ISARC_GET_ARC_OS_TYPE:
		case ISARC_GET_ARC_IS_SFX_FILE:
		case ISARC_GET_COMPRESSED_SIZE:
		case ISARC_GET_RATIO:
		case ISARC_GET_CRC:							// CabGetCRC
		case ISARC_GET_ATTRIBUTE:					// CabGetAttribute
		case ISARC_GET_OS_TYPE:						// CabGetOSType
		case ISARC_GET_METHOD:						// CabGetMethod
		case ISARC_GET_WRITE_TIME:					// CabGetWriteTime
		case ISARC_GET_CREATE_TIME:					// CabGetCreateTime
		case ISARC_GET_ACCESS_TIME:					// CabGetAccessTime
		default:
			break;
	}
	return FALSE;
}

static BOOL EnumItem(LPSTR pString, ITEMINFO* pii)
{
	char *p, *argv[10];
	int year, mon, day, hour, min, sec;
	SYSTEMTIME st;

	if (!lstrlen(pString))
		return FALSE;

	p = SkipTabSpace(pString);
	if (SeparateString(p, argv, 9) != 9)
		return FALSE;

	memset(pii, 0, sizeof(ITEMINFO));
	pii->dwComp = atol(argv[1]);
	pii->dwOrig = atol(argv[0]);

	sscanf(argv[4], "%d/%d/%d", &year, &mon, &day);
	sscanf(argv[5], "%d:%d:%d", &hour, &min, &sec);
	memset(&st, 0, sizeof(st));
	st.wYear = (WORD)year;
	st.wMonth = (WORD)mon;
	st.wDay = (WORD)day;
	st.wHour = (WORD)hour;
	st.wMinute = (WORD)min;
	st.wSecond = (WORD)sec;
	SystemTimeToFileTime(&st, &pii->filetime);

	sscanf(argv[6], "%x", &pii->dwCrc);

	pii->fLowerCase = (argv[7][0] != '-') ? 1 : 0;
	pii->fCrypt = (argv[7][1] != '-') ?  1 : 0;

	lstrcpy(pii->szItem, argv[8]);
	lstrcpy(pii->szMethod, argv[2]);
	return TRUE;
}

HANDLE WINAPI UnZipOpenArchive(HWND hwnd, LPCSTR pszZip, DWORD dwMode)
{
	OAINFO* poa;
	if (!gdwLocked)
		return NULL;

	poa = (OAINFO*)malloc(sizeof(OAINFO));
	if (poa) {
		memset(poa, 0, sizeof(OAINFO));
		poa->magic = OAMAGIC;
		poa->hwndOwner = hwnd;
		poa->pArray = (ITEMINFO**)malloc(sizeof(ITEMINFO*) * 256);
		if (!poa->pArray) {
			free(poa);
			poa = NULL;
		}
		else
			poa->iArraySize = 256;
		lstrcpy(poa->szZip, pszZip);
	}
	return (HANDLE)poa;
}

int WINAPI UnZipCloseArchive(HANDLE harc)
{
	OAINFO* poa = (OAINFO*)harc;
	if (!poa || poa->magic != OAMAGIC)
		return -1;

	if (poa->pArray) {
		int i;
		for (i = 0; i < poa->iNumMembers; i++)
			free(poa->pArray[i]);
	}
	free(poa);
	return 0;
}

static void attr2string(DWORD attr, char* psz)
{
	if (attr & FILE_ATTRIBUTE_DIRECTORY)	*psz++ = 'd';
	if (attr & FILE_ATTRIBUTE_SYSTEM)		*psz++ = 's';
	if (attr & FILE_ATTRIBUTE_HIDDEN)		*psz++ = 'h';
	if (attr & FILE_ATTRIBUTE_READONLY)		*psz++ = 'r';
	if (attr & FILE_ATTRIBUTE_ARCHIVE)		*psz++ = 'a';
	*psz = '\0';
}

int WINAPI UnZipFindNext(HANDLE harc, void* pidi)
{
	INDIVIDUALINFO* pdv;
	ITEMINFO* pii;
	OAINFO* poa = (OAINFO*)harc;
	if (!pidi || !poa || poa->magic != OAMAGIC)
		return -1;

	if (poa->iFindStart >= poa->iNumMembers) {
		poa->pii = NULL;
		return -1;
	}

	pii = poa->pArray[poa->iFindStart];
	if (!pii)
		return -1;
	poa->pii = pii;
	poa->iFindStart++;

	pdv = (INDIVIDUALINFO*)pidi;
	memset(pdv, 0, sizeof(INDIVIDUALINFO));
	// ファイル名
	lstrcpy(pdv->szFileName, pii->szItem);
	// オリジナルサイズ
	pdv->dwOriginalSize = pii->dwOrig;
	// 圧縮サイズ
	pdv->dwCompressedSize = pii->dwComp;
	// 圧縮比率
	if (pii->dwOrig)
		pdv->wRatio = (WORD)(pii->dwComp * 1000 / pii->dwOrig);
	else
		pdv->wRatio = 1000;
	// 日付時刻
	FileTimeToDosDateTime(&pii->filetime, &pdv->wDate, &pdv->wTime);
	// CRC
	pdv->dwCRC = pii->dwCrc;
	// 暗号化
	pdv->fCrypt = pii->fCrypt;
	// メソッド
	lstrcpy(pdv->szMode, pii->szMethod);
	// 属性文字列変換
	attr2string(pii->dwAttr, pdv->szAttribute);

	return 0;
}

int WINAPI UnZipFindFirst(HANDLE harc, LPCSTR pszFilter, void* pidi)
{
	OAINFO* poa;
	int i;
	char cmd[MAX_PATH * 3];

	poa = (OAINFO*)harc;
	if (!poa || poa->magic != OAMAGIC)
		return -1;

	// 既存のアイテムリストを削除
	for (i = 0; i < poa->iNumMembers; i++)
		free(poa->pArray[i]);
	poa->iNumMembers = 0;
	poa->iFindStart = 0;
	poa->dwTotal = 0;
	poa->dwCompTotal = 0;
	if (pszFilter && lstrcmp(pszFilter, "*.*"))
		lstrcpy(poa->szFilter, pszFilter);
	else
		lstrcpy(poa->szFilter, "*");

	wsprintf(cmd, "-^ \"%s\" %s", poa->szZip, poa->szFilter);
	if (UnZip(poa->hwndOwner, cmd, (LPSTR)poa, 0) == 0)
		return UnZipFindNext(harc, pidi);
	return -1;
}

WORD WINAPI UnZipGetDate(HANDLE harc)
{
	OAINFO* poa = (OAINFO*)harc;
	if (poa && poa->magic == OAMAGIC && poa->pii) {
		WORD dt, tm;
		FileTimeToDosDateTime(&poa->pii->filetime, &dt, &tm);
		return dt;
	}
	return (WORD)-1;
}

WORD WINAPI UnZipGetTime(HANDLE harc)
{
	OAINFO* poa = (OAINFO*)harc;
	if (poa && poa->magic == OAMAGIC && poa->pii) {
		WORD dt, tm;
		FileTimeToDosDateTime(&poa->pii->filetime, &dt, &tm);
		return tm;
	}
	return (WORD)-1;
}

DWORD WINAPI UnZipGetCRC(HANDLE harc)
{
	OAINFO* poa = (OAINFO*)harc;
	if (poa && poa->magic == OAMAGIC && poa->pii)
		return poa->pii->dwCrc;
	return (DWORD)-1;
}

int WINAPI UnZipGetAttribute(HANDLE harc)
{
	OAINFO* poa = (OAINFO*)harc;
	if (poa && poa->magic == OAMAGIC && poa->pii)
		return (int)(poa->pii->dwAttr & 0xffff);
	return -1;
}

DWORD WINAPI UnZipGetOriginalSize(HANDLE harc)
{
	OAINFO* poa = (OAINFO*)harc;
	if (poa && poa->magic == OAMAGIC && poa->pii)
		return poa->pii->dwOrig;
	return (DWORD)-1;
}

DWORD WINAPI UnZipGetCompressedSize(HANDLE harc)
{
	OAINFO* poa = (OAINFO*)harc;
	if (poa && poa->magic == OAMAGIC && poa->pii)
		return poa->pii->dwComp;
	return (DWORD)-1;
}

WORD WINAPI UnZipGetRatio(HANDLE harc)
{
	OAINFO* poa = (OAINFO*)harc;
	if (poa && poa->magic == OAMAGIC && poa->pii) {
		WORD w;
		w = (poa->pii->dwOrig != 0 ) ?
						(WORD)(poa->pii->dwComp * 100 / poa->pii->dwOrig) : 0;
		return w;
	}
	return (WORD)-1;
}

static time_t FileTimeToUnixTime(FILETIME* pft)
{
	SYSTEMTIME st;
	struct tm t;

	FileTimeToSystemTime(pft, &st);
	t.tm_sec = (int)st.wSecond;
	t.tm_min = (int)st.wMinute;
	t.tm_hour = (int)st.wHour;
	t.tm_mday = (int)st.wDay;
	t.tm_mon = (int)st.wMonth - 1;			// tm_mon is 0 based
	t.tm_year = (int)st.wYear - 1900;		// tm_year is 1900 based
	t.tm_isdst = 0;
	return mktime(&t);
}

DWORD WINAPI UnZipGetCreateTime(HANDLE harc)
{
	OAINFO* poa = (OAINFO*)harc;
	if (poa && poa->magic == OAMAGIC && poa->pii)
		return (DWORD)FileTimeToUnixTime(&poa->pii->filetime);
	return (DWORD)-1;
}

DWORD WINAPI UnZipGetAccessTime(HANDLE harc)
{
	OAINFO* poa = (OAINFO*)harc;
	if (poa && poa->magic == OAMAGIC && poa->pii)
		return (DWORD)FileTimeToUnixTime(&poa->pii->filetime);
	return (DWORD)-1;
}

DWORD WINAPI UnZipGetWriteTime(HANDLE harc)
{
	OAINFO* poa = (OAINFO*)harc;
	if (poa && poa->magic == OAMAGIC && poa->pii)
		return (DWORD)FileTimeToUnixTime(&poa->pii->filetime);
	return (DWORD)-1;
}

BOOL WINAPI UnZipGetCreateTimeEx(HANDLE harc, FILETIME* pft)
{
	OAINFO* poa = (OAINFO*)harc;
	if (poa && poa->magic == OAMAGIC && poa->pii) {
		*pft = poa->pii->filetime;
		return TRUE;
	}
	return FALSE;
}

BOOL WINAPI UnZipGetAccessTimeEx(HANDLE harc, FILETIME* pft)
{
	OAINFO* poa = (OAINFO*)harc;
	if (poa && poa->magic == OAMAGIC && poa->pii) {
		*pft = poa->pii->filetime;
		return TRUE;
	}
	return FALSE;
}

BOOL WINAPI UnZipGetWriteTimeEx(HANDLE harc, FILETIME* pft)
{
	OAINFO* poa = (OAINFO*)harc;
	if (poa && poa->magic == OAMAGIC && poa->pii) {
		*pft = poa->pii->filetime;
		return TRUE;
	}
	return FALSE;
}

int WINAPI LuzReplace(void* pInfo, char *filename)
{
	LPUSERFUNCTIONS pUserFn = (LPUSERFUNCTIONS)pInfo;
	REPLACEINFO ri;
	ri.pszName = filename;
	ri.dwReplace = pUserFn->nReplace;
	if (DlgModal(GetFocus(), (DLGPROC)ReplaceDlgFn, IDD_REPLACE, &ri) == IDOK) {
		pUserFn->nReplace = ri.dwReplace;
		return pUserFn->nReplace;
	}
	return 1;
}

// This is a very stripped down version of what is done in Wiz. Essentially
// what this function is for is to do a listing of an archive contents. It
// is actually never called in this example, but a dummy procedure had to
// be put in, so this was used.

void WINAPI LuzMessage(void* pInfo, ULONG dwOriginalSize,
	ULONG dwCompressedSize, unsigned cfactor,
	SYSTEMTIME* pst, BOOL fLowerCase, LPSTR filename, LPSTR methbuf,
    ULONG crc, BOOL fCrypt)
{
	LPUSERFUNCTIONS pUserFn = (LPUSERFUNCTIONS)pInfo;

	if (pUserFn->pDcl->fInsideVerb) {
		OAINFO* poa;
		ITEMINFO* pii;
		int len;

		poa = (OAINFO*)pUserFn->pResult;
		pii = (ITEMINFO*)malloc(sizeof(ITEMINFO));
		if (!pii) return;

		memset(pii, 0, sizeof(ITEMINFO));
		lstrcpy(pii->szItem, filename);
		pii->dwComp = dwCompressedSize;
		pii->dwOrig = dwOriginalSize;
		lstrcpy(pii->szMethod, methbuf);
		SystemTimeToFileTime(pst, &pii->filetime);
		pii->dwCrc = crc;
		pii->fCrypt = (fCrypt != 0) ? 1 : 0;
		pii->dwAttr = cfactor;
		if (pii->fCrypt)
			pii->dwAttr |= _A_CRPT;
		pii->fLowerCase = (fLowerCase != 0) ? 1 : 0;

		// ファイル名が '/' で終わっていればディレクトリと見なす
		len = lstrlen(pii->szItem) - 1;
		if (pii->szItem[len] == '/' && _mbsbtype((BYTE*)pii->szItem, len) == _MBC_SINGLE) {
			pii->dwAttr |= FILE_ATTRIBUTE_DIRECTORY;
		}

		if (poa->iNumMembers + 1 > poa->iArraySize) {
			ITEMINFO** pw;
			pw = (ITEMINFO**)realloc(poa->pArray, sizeof(ITEMINFO*) * (poa->iArraySize + 256));
			if (!pw) {
				free(pii);
				return;
			}
			poa->pArray = pw;
			poa->iArraySize += 256;
		}
		poa->pArray[poa->iNumMembers++] = pii;
		poa->dwTotal += pii->dwOrig;
		poa->dwCompTotal += pii->dwComp;
	}
	else {
		char buf[MAX_PATH * 3];
		int len, per;

		if (dwOriginalSize)
			per = (dwCompressedSize * 100) / dwOriginalSize;
		else
			per = 100;
		len = wsprintf(buf,
				"%7d %7d %-8s %3d %04u/%02u/%02u %02u:%02u:%02u %08x %c%c %s\n",
				dwOriginalSize, dwCompressedSize, methbuf, per,
				pst->wYear, pst->wMonth, pst->wDay,
				pst->wHour, pst->wMinute, pst->wSecond,
				crc,
				fLowerCase ? 'c' : '-',
				fCrypt ? 'e' : '-',
				filename);

		if (pUserFn->nResultLength > len) {
			lstrcpy(pUserFn->pResult, buf);
			pUserFn->pResult += len;
			pUserFn->nResultLength -= len;
		}
	}
}

// Password entry routine - see password.c in the wiz directory for how
// this is actually implemented in WiZ. If you have an encrypted file,
// this will probably give you great pain.
int WINAPI LuzPassword(char *ppass, int n, const char *name)
{
	PASSENTER pe;
	pe.pszFile = name;
	pe.pszCode = ppass;

	if (DlgModal(GetFocus(), (DLGPROC)PasswordDlgFn, IDD_PASSWORD, &pe) != IDOK)
		return IZ_PW_CANCELALL;
	return IZ_PW_ENTERED;
}

static void __ZapMessage(HWND hdlg)
{
	MSG	msg;
	while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) {
		if (IsWindow(hdlg) && !IsDialogMessage(hdlg, &msg)) {
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
	}
}

////////////////////////////////////////////////////////////////////////
// Return the percentage compression from n to m using only integer
// operations
static int percent(ulg n, ulg m)
/* n is the original size, m is the new size */
{
	if (n > 0xffffffL) {          // If n >= 16M
		n += 0x80;  n >>= 8;      //  then divide n and m by 256
		m += 0x80;  m >>= 8;
	}
	return n > m ? (int)(1 + (200 * (n - m)/n)) / 2 : 0;
}

int WINAPI LuzReport(void* pInfo, UINT type, LPCSTR pZip, LPCSTR pFile, ULONG data)
{
	LPUSERFUNCTIONS pUserFn = (LPUSERFUNCTIONS)pInfo;
	if (IsWindow(pUserFn->hMeter)) {
		HWND hctl;
		METERINFO* pmi = (METERINFO*)GetWindowLongPtr(pUserFn->hMeter, DWLP_USER);
		if (pmi->bCanceled)
			return 1;

		hctl = GetDlgItem(pUserFn->hMeter, IDC_METER);
		if (type == UZ_ST_START_EXTRACT) {
			char buf[MAX_PATH * 3];

			cdir_file_hdr* pcrec = (cdir_file_hdr*)data;
			lstrcpy(buf, pFile);
			ReplaceChar(buf, '/', '\\');
			PathSetDlgItemPath(pUserFn->hMeter, IDC_FILE, buf);

			pUserFn->dwMeterSize = pcrec->ucsize;
			if (!pUserFn->dwMeterSize)
				pUserFn->dwMeterSize = 1;
			pUserFn->dwMeterDone = 0;
			SendMessage(hctl, PBM_SETPOS, 0, 0);
			UpdateWindow(hctl);
		} else if (type == UZ_ST_IN_PROGRESS) {
			int per;
			pUserFn->dwMeterDone += data;
			per = percent(pUserFn->dwMeterSize, pUserFn->dwMeterDone);
//			per = (pUserFn->dwMeterDone * 100) / pUserFn->dwMeterSize;
			SendMessage(hctl, PBM_SETPOS, per, 0);
			UpdateWindow(hctl);
		}

		__ZapMessage(pUserFn->hMeter);
	}
	return 0;
}

// Dummy "print" routine that simply outputs what is sent from the dll
int WINAPI LuzDisplay(void* pInfo, LPSTR buf, ULONG size)
{
	LPUSERFUNCTIONS pUserFn = (LPUSERFUNCTIONS)pInfo;
	if (pUserFn->nResultLength > (int)size) {
		lstrcpy(pUserFn->pResult, buf);
		pUserFn->pResult += size;
		pUserFn->nResultLength -= size;
	}
	return (int)size;
}

void CenterPopup(HWND hwnd, HWND hwndParent)
{
	int x, y, cx, cy, cxScreen, cyScreen;
	RECT rc;
	GetWindowRect(hwnd, &rc);
	cxScreen = GetSystemMetrics(SM_CXSCREEN);
	cyScreen = GetSystemMetrics(SM_CYSCREEN);

	cx = rc.right - rc.left;
	cy = rc.bottom - rc.top;

	if (hwndParent && IsWindowVisible(hwndParent)) {
		int cxParent, cyParent;

		GetWindowRect(hwndParent, &rc);
		cxParent = rc.right - rc.top;
		cyParent = rc.bottom - rc.top;
		x = rc.left + ((cxParent - cx) / 2);
		y = rc.top +  ((cyParent - cy) / 2);

		if (x + cx > cxScreen)
			x = cxScreen - cx;
		if (y + cy > cyScreen)
			y = cyScreen - cy;
	}
	else {
		x = (cxScreen - cx) / 2;
		y = (cyScreen - cy) / 2;
	}
	MoveWindow(hwnd, (x > 0 ? x : 0), (y > 0 ? y : 0), cx, cy, TRUE);
}

int DlgModal(HWND hwnd, DLGPROC lpFn, int nID, void* pvParam)
{
	int nRes;
	HRSRC hrsrc;

	nRes = 0;
	hrsrc = FindResource(hInst, MAKEINTRESOURCE(nID), RT_DIALOG);
	if (hrsrc) {
		HGLOBAL hmem;
		hmem = LoadResource(hInst, hrsrc);
		if (hmem)
			nRes = (int)DialogBoxIndirectParam(hInst, (DLGTEMPLATE*)hmem, hwnd, lpFn, (LPARAM)pvParam);
	}
	return nRes;
}

HWND DlgModeless(HWND hwnd, DLGPROC lpFn, int nID, void* pvParam)
{
	HRSRC hrsrc = FindResource(hInst, MAKEINTRESOURCE(nID), RT_DIALOG);
	if (hrsrc) {
		HGLOBAL hmem = LoadResource(hInst, hrsrc);
		if (hmem) {
			HWND hwndModeless = CreateDialogIndirectParam(
										hInst, (LPDLGTEMPLATE)hmem,
										hwnd, lpFn, (DWORD)pvParam);
			FreeResource(hmem);
			return hwndModeless;
		}
	}
	return NULL;
}

BOOL WINAPI PasswordDlgFn(HWND hdlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
	PASSENTER* ppe;
	switch (msg) {
		case WM_INITDIALOG:
			if (gbTryLastPassword) {
				HWND hctl = GetDlgItem(hdlg, IDC_EDIT);
				RECT rc;
				GetWindowRect(hctl, &rc);
				DestroyWindow(hctl);

				ScreenToClient(hdlg, (LPPOINT)&rc.left);
				ScreenToClient(hdlg, (LPPOINT)&rc.right);
				hctl = CreateWindowEx(WS_EX_CLIENTEDGE, "edit", NULL,
								WS_VISIBLE | WS_CHILD | ES_AUTOHSCROLL,
								rc.left, rc.top,
								(rc.right - rc.left), (rc.bottom - rc.top),
								hdlg, (HMENU)IDC_EDIT,
								GetWindowInstance(hdlg), NULL);
				if (hctl)
				{
					HFONT hf = GetWindowFont(hdlg);
					SetWindowFont(hctl, hf, FALSE);
				}
			}

			MessageBeep(0);
			CenterPopup(hdlg, GetParent(hdlg));

			SetWindowLongPtr(hdlg, DWLP_USER, lParam);
			ppe = (PASSENTER*)lParam;
			if (ppe->pszFile)
				SetDlgItemText(hdlg, IDC_TEXT, ppe->pszFile);
			if (ppe->pszCode)
			{
				SetDlgItemText(hdlg, IDC_EDIT, ppe->pszCode);
				SendDlgItemMessage(hdlg, IDC_EDIT, EM_SETSEL, 0, (LPARAM)-1);
			}

			SetForegroundWindow(hdlg);
			SetFocus(GetDlgItem(hdlg, IDC_EDIT));
			return FALSE;

		case WM_COMMAND:
			switch (LOWORD(wParam))
			{
				case IDCANCEL:
					EndDialog(hdlg, IDCANCEL);
					return TRUE;

				case IDOK:
				{
					ppe = (PASSENTER*)GetWindowLongPtr(hdlg, DWLP_USER);
					if (!GetDlgItemText(hdlg, IDC_EDIT, ppe->pszCode, MAX_PATH))
					{
						MessageBeep(0);
						SetFocus(GetDlgItem(hdlg, IDC_EDIT));
					}
					else
						EndDialog(hdlg, IDOK);
					return TRUE;
				}

				default:
					break;
			}
			break;

		default:
			break;
	}
	return FALSE;
}

BOOL WINAPI MeterDlgFn(HWND hdlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
	METERINFO* pmi;
	switch (msg)
	{
		case WM_INITDIALOG:
			CenterPopup(hdlg, GetParent(hdlg));

			pmi = (METERINFO*)malloc(sizeof(METERINFO));
			memset(pmi, 0, sizeof(METERINFO));
			SetWindowLongPtr(hdlg, DWLP_USER, (LONG_PTR)pmi);
			SendDlgItemMessage(hdlg, IDC_METER, PBM_SETRANGE, 0, MAKELPARAM(0, TICKS));
			return TRUE;

		case WM_COMMAND:
			switch (LOWORD(wParam))
			{
				case IDCANCEL:
					pmi = (METERINFO*)GetWindowLongPtr(hdlg, DWLP_USER);
					pmi->bCanceled = TRUE;
					ShowWindow(hdlg, SW_HIDE);
					return TRUE;

				default:
					break;
			}
			break;

		case WM_DESTROY:
			pmi = (METERINFO*)GetWindowLongPtr(hdlg, DWLP_USER);
			free(pmi);
			break;

		default:
			break;
	}
	return FALSE;
}

BOOL WINAPI ReplaceDlgFn(HWND hdlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
	REPLACEINFO* pri;
	switch (msg)
	{
		case WM_INITDIALOG:
			CenterPopup(hdlg, GetParent(hdlg));

			pri = (REPLACEINFO*)lParam;
			SetWindowLongPtr(hdlg, DWLP_USER, (LONG_PTR)pri);
			PathSetDlgItemPath(hdlg, IDC_TEXT, pri->pszName);
			if (pri->dwReplace == REPLACE_NO)
				CheckDlgButton(hdlg, IDC_IGNORE, TRUE);
			else
				CheckDlgButton(hdlg, IDC_REPLACE, TRUE);
			SetForegroundWindow(hdlg);
			return TRUE;

		case WM_COMMAND:
			switch (LOWORD(wParam))
			{
				case IDOK:
					pri = (REPLACEINFO*)GetWindowLongPtr(hdlg, DWLP_USER);
					if (IsDlgButtonChecked(hdlg, IDC_REPLACE))
						pri->dwReplace = REPLACE_YES;
					else
						pri->dwReplace = REPLACE_NO;
					EndDialog(hdlg, IDOK);
					return TRUE;

				case IDC_OKALL:
					pri = (REPLACEINFO*)GetWindowLongPtr(hdlg, DWLP_USER);
					if (IsDlgButtonChecked(hdlg, IDC_REPLACE))
						pri->dwReplace = REPLACE_ALL;
					else
						pri->dwReplace = REPLACE_NONE;
					EndDialog(hdlg, IDOK);
					return TRUE;

				case IDCANCEL:
					EndDialog(hdlg, IDCANCEL);
					return TRUE;

				default:
					break;
			}
			break;

		default:
			break;
	}
	return FALSE;
}
