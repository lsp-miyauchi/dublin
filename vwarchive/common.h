////////////////////////////////////////////////////////////////////
// common.h

extern COLORREF gColors[];
extern TCHAR gszAst[];
extern COSPlatform gosp;
extern TCHAR gszLzhExt[];
extern TCHAR gszZipExt[];

#ifndef MAX_URL_LENGTH
	#define MAX_URL_LENGTH		(2048 + 32 + 8)
#endif

////////////////////////////////////////////////////////////////////
//

#define VERSION_XP		MAKELONG(0, 6)

enum {
	colText,
	colBack,
	colRText,
	colRBack,
	colInact
};

/////////////////////////////////////////////////////////////////////////////
//

#define ATS_LOWER		0x0001					// 小文字で表示
#define ATS_GRID		0x0002					// グリッドを表示
#define ATS_SIZEKB		0x0004					// サイズをKBで表示
#define ATS_HOTTRACK	0x0008					// ホットトラッキング
#define ATS_TBSMALL		0x0010					// 小さなアイコンのツールバー
#define ATS_REPCHECKER	0x0020					// 詳細表示で市松模様
#define ATS_PLACE		0x1000					// 解凍先の指定


////////////////////////////////////////////////////////////////////
// MAXIMUM NUMBER
//
#define MAXHIAR 	16


//----------------------------------------------
// MessageBox ACTION
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


enum {
	CE_NORMAL,
	CE_PROGRAM,
	CE_ASSOCIATE,
	CE_ARC,
	CE_DIR,
};

///////////////////////////////////////////////////////////////////
//

BOOL get_fileinfo(LPCTSTR pszName, WIN32_FIND_DATA* pwfd);
BOOL ChangeDriveDir(LPCTSTR pszPath);
int AppMessage(HWND hwnd, UINT nID, LPCTSTR pszInsert = NULL, UINT nType = MB_OK | MB_ICONSTOP);
int LastError(LPCTSTR pszTitle, UINT nType = MB_OK | MB_ICONSTOP);
BOOL SetStringToClipboard(HWND hWnd, LPCTSTR pszText);
BOOL FileBrowse(BOOL bOpen, HWND hwnd, UINT nIDCaption, UINT nIDFilter, LPTSTR pszName);
int QueryDefaultIcon(LPCTSTR lpName, LPTSTR lpIconPath, LPINT pnIconIndex);
void myShellExecute(HWND hWnd, LPCTSTR lpName, LPCTSTR lpDir);
void ViewFile(HWND hWnd, LPCTSTR lpName, LPCTSTR lpDir);
void RunHexEditor(HWND hWnd, LPCTSTR pszFile);
void RunTextEditor(HWND hwnd, LPCTSTR pszFile);
void RunArchiver(HWND hWnd, LPCTSTR pszName);
void RunImager(HWND hWnd, LPCTSTR pszName);
int ispathchild(LPCTSTR pszSrc, LPCTSTR pszDst);
int compare_path(LPCTSTR pszSrc, LPCTSTR pszDst);
int compare_fullname(LPCTSTR pszSrc, LPCTSTR pszDst);
void add_slash(LPTSTR pszPath);
void del_slash(LPTSTR pszPath);
int split_pathcomponent(LPCTSTR pszPath, CSimpleArray<CString>& arr);
int replace_char(LPTSTR psz, TCHAR src, TCHAR dst);
void add_char(LPTSTR psz, TCHAR add);
int CheckFileType(LPCTSTR pszFileName);
BOOL user_ristriction();
__inline bool OsVISTA() { return gosp.OsVISTA(); }

