#ifndef __structs_h
#define __structs_h

#ifndef Far
#  define Far far
#endif

/* Porting definitions between Win 3.1x and Win32 */
#ifdef WIN32
#  define far
#  define _far
#  define __far
#  define near
#  define _near
#  define __near
#  ifndef FAR
#    define FAR
#  endif
#endif

#ifndef PATH_MAX
#  define PATH_MAX 260            /* max total file or directory name path */
#endif

#ifndef DEFINED_ONCE
#define DEFINED_ONCE

typedef struct {
	int fOnlyNewer;
	int fSpc2Undscr;
	int fAskOverwrite;
	int fQuiet;								// 0 = all messages
											// 1 = fewer messages
											// 2 = no messages
	int fConsole;							// Write to stdout if TRUE
	int fTest;								// test zip file if TRUE
	int fVerbose;							// give a verbose listing if TRUE
	int fUpdate;							// Do not extract only newer
	int fComment;							// display a zip file comment if TRUE
	int fMkdir;								// Recreate directories if TRUE
	int fOverwrite;							// Over-write all files if TRUE
	int fConvCR;							// Do not convert CR to CRLF
	int fInfo;								// Get ZIP Info
	int fCase;								// be case insensitive if TRUE
	int fPrivilege;
	int fProgress;							// 解凍進捗表示
	int fInsideVerb;
	LPSTR lpszZipFN;						// The archive name
	LPSTR lpszExtractDir;					// The directory to extract to.
											// This is set to NULL if you are
											// extracting to the current directory.
	LPSTR lpszPassword;						// コマンドライン パスワード
	int iNumFilters;
	int iMaxFilters;
	char** ppFilters;
} DCL, far * LPDCL;

typedef int (WINAPI DLLPRNT) (void*, LPSTR, ULONG);
typedef int (WINAPI DLLPASSWORD) (LPSTR, int, LPCSTR);
typedef int (WINAPI DLLREPORT)(void*, UINT type, LPCSTR, LPCSTR, ULONG);
#endif
typedef int (WINAPI DLLREPLACE)(void*, LPSTR);
typedef void (WINAPI DLLMESSAGE)(void*, ULONG, ULONG, unsigned,
   SYSTEMTIME*, BOOL, LPSTR, LPSTR, ULONG, BOOL);

typedef struct {
	DLLPRNT *print;
	DLLREPLACE *replace;
	DLLPASSWORD *password;
	DLLMESSAGE *message;
	DLLREPORT *report;
	DWORD dwTotalSizeComp;
	DWORD dwTotalSize;
	int iCompFactor;
	UINT iNumMembers;
	WORD cchComment;
// -------------
	HWND hwndParent;
	LPSTR pResult;
	int nResultLength;
	HWND hMeter;
	LPDCL pDcl;
	int nReplace;							// 0 = query
											// 1 = all
											// 2 = ignore
	DWORD dwMeterSize;
	DWORD dwMeterDone;
} USERFUNCTIONS, far * LPUSERFUNCTIONS;

/////////////////////////////////////////////////////////////////////
// for OpenArchive

#define _A_CRPT			0x4000

typedef struct tagITEMINFO {
	char szItem[MAX_PATH * 2];
	DWORD dwComp;
	DWORD dwOrig;
	DWORD dwAttr;
	char szMethod[8];
	DWORD dwCrc;
	FILETIME filetime;
	WORD wOs;
	unsigned fCrypt : 1;
	unsigned fLowerCase : 1;
} ITEMINFO, *LPITEMINFO;


#define OAMAGIC			0x9999

typedef struct tagOAINFO {
	WORD magic;
	HWND hwndOwner;
	char szZip[MAX_PATH * 2];
	char szFilter[MAX_PATH * 2];
	int iNumMembers;
	DWORD dwTotal;
	DWORD dwCompTotal;
	ITEMINFO** pArray;
	int iArraySize;
	int iFindStart;
	ITEMINFO* pii;
} OAINFO, *LPOAINFO;

#ifndef FNAME_MAX32
#define FNAME_MAX32		512
#endif

#pragma pack(1)
typedef struct tagINDIVIDUALINFO {
	DWORD 			dwOriginalSize;		/* ファイルのサイズ */
 	DWORD 			dwCompressedSize;	/* 圧縮後のサイズ */
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
} INDIVIDUALINFO, *LPINDIVIDUALINFO;
#pragma pack()


#endif /* __structs_h */
