//////////////////////////////////////////////////////////////////
// func.h

#define _ISNETPATHW(a)	(wcslen((a)) > 3 && *(a) == L'\\' && *((a) + 1) == L'\\' && *((a) + 2) != L'?')
#define _ISNETPATHA(a)	(strlen((a)) > 3 && *(a) == '\\' && *((a) + 1) == '\\' && *((a) + 2) != '?')

int ReplaceString(LPWSTR pszTarget, LPCWSTR pszOld, LPCWSTR pszNew);
int ReplaceChar(LPWSTR pszTarget, const WCHAR chOld, const WCHAR chNew);

LPWSTR alloc_wcscpy(LPWSTR* ppszDst, LPCWSTR pszSrc);
int AppMessage(UINT idResource, LPCWSTR pszInsert = NULL, UINT nType = MB_OK | MB_ICONSTOP);

BOOL GetExtractPath(CSfxInfo* ps);
int GetCopyAction(LPCWSTR psz, int defaction, bool& yesall);

BOOL fdiExtract(HWND hwnd, LPBYTE pbCabData, CSfxInfo* ps);

int xLoadString(HINSTANCE hInst, UINT id, LPWSTR psz, int cch);
int xMessageBox(HWND hwnd, LPCWSTR pszText, LPCWSTR pszTitle, UINT style);
BOOL xCreateDirectory(LPCWSTR psz, LPSECURITY_ATTRIBUTES psa);
HANDLE xCreateFile(LPCWSTR psz, DWORD fAccess, DWORD fShareMode, LPSECURITY_ATTRIBUTES psa, DWORD fCreate, DWORD fAttrs, HANDLE hTemp);
BOOL xRemoveDirectory(LPCWSTR psz);
BOOL xDeleteFile(LPCWSTR psz);

BOOL xSetFileAttributes(LPCWSTR psz, DWORD attr);
BOOL xCreateProcess(LPWSTR pszCommandLine, LPCWSTR pszCurDir, PROCESS_INFORMATION* ppiProcInfo);
HINSTANCE xShellExecute(HWND hwnd, LPCWSTR pszCmd, LPCWSTR pszDir);
BOOL xSetDlgItemText(HWND hwnd, int idCtrl, LPCWSTR psz);
UINT xGetDlgItemText(HWND hwnd, int idCtrl, LPWSTR psz, int cchMax);
BOOL xSetWindowText(HWND hwnd, LPCWSTR psz);

UINT xGetWindowsDirectory(LPWSTR psz, UINT cch);
UINT xGetSystemDirectory(LPWSTR psz, UINT cch);
DWORD xGetCurrentDirectory(DWORD cch, LPWSTR psz);
DWORD xSetCurrentDirectory(LPCWSTR psz);
DWORD xGetFileAttributes(LPCWSTR psz);
void xPathChangeNotify(LONG idEvent, LPCWSTR psz);
BOOL xPathMakeTree(LPCWSTR pszPath);
BOOL xPathGetSpecialFilesDir(LPCWSTR pszName, LPWSTR pszDir, int cchMax);

LPWSTR xStrChrW(LPCWSTR psz, WCHAR ch);
LPSTR xStrChrA(LPCSTR psz, char ch);
#ifdef UNICODE
#define xStrChr						xStrChrW
#else
#define xStrChr						xStrChrA
#endif // !UNICODE

LPWSTR xStrRChrW(LPCWSTR psz, WCHAR ch);
LPSTR xStrRChrA(LPCSTR psz, char ch);
#ifdef UNICODE
#define xStrRChr					xStrRChrW
#else
#define xStrRChr					xStrRChrA
#endif // !UNICODE

int xStrCmpNW(LPCWSTR pszStr, LPCWSTR pszComp, int iLen);
int xStrCmpNA(LPCSTR pszStr, LPCSTR pszComp, int iLen);
#ifdef UNICODE
#define xStrCmpN					xStrCmpNW
#else
#define xStrCmpN					xStrCmpNA
#endif // !UNICODE

int xStrCmpNIW(LPCWSTR pszStr, LPCWSTR pszComp, int iLen);
int xStrCmpNIA(LPCSTR pszStr, LPCSTR pszComp, int iLen);
#ifdef UNICODE
#define xStrCmpNI					xStrCmpNIW
#else
#define xStrCmpNI					xStrCmpNIA
#endif // !UNICODE

LPWSTR xPathFindFileNameW(LPCWSTR psz);
LPSTR xPathFindFileNameA(LPCSTR psz);
#ifdef UNICODE
#define xPathFindFileName			xPathFindFileNameW
#else
#define xPathFindFileName			xPathFindFileNameA
#endif // !UNICODE

LPWSTR xPathFindExtensionW(LPCWSTR psz);
LPSTR xPathFindExtensionA(LPCSTR psz);
#ifdef UNICODE
#define xPathFindExtension			xPathFindExtensionW
#else
#define xPathFindExtension			xPathFindExtensionA
#endif // !UNICODE

BOOL xPathRemoveFileSpecW(LPCWSTR psz);
BOOL xPathRemoveFileSpecA(LPCSTR psz);
#ifdef UNICODE
#define xPathRemoveFileSpec			xPathRemoveFileSpecW
#else
#define xPathRemoveFileSpec			xPathRemoveFileSpecA
#endif // !UNICODE

void xPathRemoveExtensionW(LPCWSTR psz);
void xPathRemoveExtensionA(LPCSTR psz);
#ifdef UNICODE
#define xPathRemoveExtension		xPathRemoveExtensionW
#else
#define xPathRemoveExtension		xPathRemoveExtensionA
#endif // !UNICODE

BOOL xPathRenameExtensionW(LPWSTR psz, LPCWSTR ext);
BOOL xPathRenameExtensionA(LPSTR psz, LPCSTR ext);
#ifdef UNICODE
#define xPathRenameExtension		xPathRenameExtensionW
#else
#define xPathRenameExtension		xPathRenameExtensionA
#endif // !UNICODE

BOOL xPathGetParentW(LPCWSTR psz, LPWSTR parent);
BOOL xPathGetParentA(LPCSTR psz, LPSTR parent);
#ifdef UNICODE
#define xPathGetParent				xPathGetParentW
#else
#define xPathGetParent				xPathGetParentA
#endif // !UNICODE

LPWSTR xPathAddBackslashW(LPWSTR psz);
LPSTR xPathAddBackslashA(LPSTR psz);
#ifdef UNICODE
#define xPathAddBackslash			xPathAddBackslashW
#else
#define xPathAddBackslash			xPathAddBackslashA
#endif // !UNICODE

LPWSTR xPathRemoveBackslash(LPWSTR psz);

LPWSTR xPathCombineW(LPWSTR pszDst, LPCWSTR pszDir, LPCWSTR pszFile);
LPSTR xPathCombineA(LPSTR pszDst, LPCSTR pszDir, LPCSTR pszFile);
#ifdef UNICODE
#define xPathCombine				xPathCombineW
#else
#define xPathCombine				xPathCombineA
#endif // !UNICODE

LPWSTR xPathAppendW(LPWSTR pszPath, LPCWSTR pszMore);
LPSTR xPathAppendA(LPSTR pszPath, LPCSTR pszMore);
#ifdef UNICODE
#define xPathAppend					xPathAppendW
#else
#define xPathAppend					xPathAppendA
#endif // !UNICODE

void xPathQuoteSpacesW(LPWSTR pszName);
void xPathQuoteSpacesA(LPSTR pszName);
#ifdef UNICODE
#define xPathQuoteSpaces			xPathQuoteSpacesW
#else
#define xPathQuoteSpaces			xPathQuoteSpacesA
#endif // !UNICODE

void xPathUnquoteSpacesW(LPWSTR pszName);
void xPathUnquoteSpacesA(LPSTR pszName);
#ifdef UNICODE
#define xPathUnquoteSpaces			xPathUnquoteSpacesW
#else
#define xPathUnquoteSpaces			xPathUnquoteSpacesA
#endif // !UNICODE

BOOL xPathIsRoot(LPCWSTR pszPath);
BOOL xPathIsDirectory(LPCWSTR pszName);
BOOL xPathFileExists(LPCWSTR pszName);
BOOL xPathIsUNC(LPCWSTR pszPath);
BOOL xPathIsUNCServer(LPCWSTR pszPath);
BOOL xPathIsUNCServerShare(LPCWSTR psz);
int xPathGetDriveNumber(LPCWSTR psz);

int EncodeWStrToUTF(const WCHAR* widestr, int widelen, BYTE* utfstr, int utflen);
int EncodeStrToUTF(const char* str, int len, BYTE* utfstr, int utflen);
int DecodeUTFToWStr(const BYTE* utfstr, int utflen, WCHAR* widestr, int widelen);
int DecodeUTFToStr(const BYTE* utfstr, int utflen, char* str, int len);
BOOL IsUTFEncoded(const BYTE* utfstr);

BOOL CenterWindow(HWND hWnd, HWND hWndCenter);
