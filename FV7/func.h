//
// FUNCTION PROTOTYPE DEFINITION
//

#pragma once


///////////////////////////////////////////////////////////////////////
// common.cpp
//
int __fastcall CheckExecType(LPCTSTR pszFileName);
void GetErrorString(DWORD code, CString& str);
int AppMessage(UINT nID, LPCTSTR pszInsert = NULL, UINT nType = MB_ICONSTOP | MB_OK);
LPTSTR str_clone(LPCTSTR pszText);
void str_free(LPCTSTR* ppsz);
void StrFormatByteSize(DWORDLONG dl, CString& str);
BOOL __fastcall GetFileInfo(LPCTSTR pszName, WIN32_FIND_DATA& wfd);
BOOL FileBrowse(BOOL bRead, HWND hwnd, _U_STRINGorID filter, CString& name, LPCTSTR pszInitialDir = NULL);
BOOL LoadFontInfo(LPLOGFONT lplf, UINT nSecID, UINT nKeyID);
void SaveFontInfo(LPLOGFONT lplf, UINT nSecID, UINT nKeyID);
int GetSiblings(HWND hwndOwn, CSimpleArray<HWND>& array, bool bVisibleOnly = false);
void __fastcall DrawItemDriveStandard(CDC *pdc, LPDRAWITEMSTRUCT lpdi);
void __fastcall DrawDrive(CDC *pdc, int x, int y, UINT nDriveID);
void __fastcall AddNormalTab(HWND hwnd, LPCTSTR lpTitle, HIMAGELIST hImageList);
void PopupHint(HWND hwnd, UINT nID);
void __fastcall ValidateCurrentFolder(LPTSTR lpPath);
void __fastcall DrawUnderline(CDC* pdc, LPRECT prc, COLORREF rgb);
void __fastcall WebCall(HWND hwnd, UINT nIDResource);
BOOL DumpMemory(HGLOBAL hmem, LPCTSTR pszDumpFile);
BOOL DumpMemory(LPBYTE lpb, DWORD size, LPCTSTR pszDumpFile);
int SeparateString(LPCTSTR pszText, TCHAR delim, CSimpleArray<CString>& array);
int SplitString(LPCTSTR pszText, LPCTSTR delim, CSimpleArray<CString>& array, bool trim = false);
CSimpleArray<CString> SplitString(LPCTSTR pszText, LPCTSTR delim, bool trim);
int LoadTokenizedString(UINT id, LPCTSTR delim, CSimpleArray<CString>& array);
CString JoinArray(CSimpleArray<CString>& array, LPCTSTR join);
//@ void RegUpdateStringHistory(UINT nIDSec, UINT nIDKey, LPCTSTR pszText, int nMax);
CString GetCommonParentDir(HDROP hDrop);
BOOL __fastcall ExtractSubString(LPCTSTR psz, int index, _STRorCH sc, LPTSTR sub, int cchMax);
BOOL __fastcall checktoken(LPCTSTR pList, LPCTSTR pToken, _STRorCH sc);
LPTSTR EnumDropItemAll(HDROP hDrop, BOOL bQuote);
// void dump_imagelist(HIMAGELIST him);
size_t __fastcall _wcs2mbs(char* mbstr, const wchar_t* wcstr, size_t cnt);
size_t __fastcall _mbs2wcs(wchar_t* wcstr, const char* mbstr, size_t cnt);
BOOL __fastcall PathGetUNCServerShare(LPCTSTR pPath, LPTSTR pShare);
LPTSTR __fastcall PathGetRootFromPath(LPCTSTR pszPath, LPTSTR pszRoot);
BOOL getlinkinfo(LPCTSTR pszName, CString& target);
int __fastcall MakeEnumStrFromHdrop(HDROP hdrop, CString& csFiles);
int __fastcall MakeEnumStrFromDataObject(IDataObject* pdo, CString& str);
void __fastcall CalcTextRectDimension(HDC hdc, LPCTSTR psz, int len, UINT format, int maxwidth, SIZE* size);
void __fastcall CalcTextDimension(HDC hdc, LPCTSTR psz, int len, SIZE* size);
LPCTSTR __fastcall num_form(DWORDLONG dlNum, CString& str);
int __fastcall FormatFileSize(DWORDLONG size, CString& str, BOOL bFmtByte);
int __fastcall FormatSystemTime(SYSTEMTIME& st, CString& str, BOOL b2col, BOOL bRelative = FALSE);
int __fastcall FormatFileTime(FILETIME* pft, CString& str, BOOL b2col = FALSE, BOOL bRelative = FALSE);
BOOL __fastcall WriteDataToFile(LPCTSTR pszFile, void* pData, int len);
void* __fastcall ReadDataFromFile(LPCTSTR lpFileName);
BOOL _openclipboardbyeditor(HWND hwnd);
void __fastcall MakeShortcutObject(HWND hwnd, IShellFolder* psf, LPCITEMIDLIST pidl, LPCTSTR pszDisplayName);
void __fastcall PasteShortcutFromClipboard(HWND hwnd, LPCITEMIDLIST pidlTarget);
BOOL __fastcall GetFileData(LPCTSTR pszName, WIN32_FIND_DATA* pwfd);
BOOL __fastcall _overridefileattributes(LPCTSTR pszName, WIN32_FIND_DATA* pwfd);
BOOL __fastcall _pathfileexists(LPCTSTR pszName);
BOOL __fastcall _pathisfile(LPCTSTR pszName);
BOOL __fastcall _pathisdirectory(LPCTSTR pszName);
BOOL __fastcall _pathisdirectoryUI(HWND hwnd, LPCTSTR pszPath);
BOOL __fastcall _pathispathspec(LPCTSTR pszPath);
BOOL __fastcall CheckDirectoryNameUI(HWND hwnd, LPCTSTR pszPath);
BOOL __fastcall CheckServerUI(HWND hwnd, LPCTSTR pszFolder);
int __fastcall GetWindowNumCount(LPCTSTR pszClass, HWND* phWnd, BOOL bVisibleOnly);
void __fastcall build_unicode_path(LPCTSTR pSrc, LPTSTR pDst, int cchMax);
HANDLE __fastcall _createfile(LPCTSTR pName, DWORD fdwAccess, DWORD fdwShareMode, LPSECURITY_ATTRIBUTES psa, DWORD fdwCreate, DWORD fdwAttrsAndFlags, HANDLE hTemplateFile);
void* __fastcall _get_rcdata(UINT idResorce, DWORD* psize);

void __fastcall tchar_set(LPTSTR psz, TCHAR ch, size_t size);
LPTSTR __fastcall tchar_malloc(size_t size);
LPTSTR __fastcall tchar_realloc(LPTSTR p, size_t size);

BYTE __fastcall byte_rangerot_left(BYTE w, int range, int rot);
BYTE __fastcall byte_rangerot_right(BYTE w, int range, int rot);
WORD __fastcall word_rangerot_left(WORD w, int range, int rot);
WORD __fastcall word_rangerot_right(WORD w, int range, int rot);
DWORD __fastcall dword_rangerot_left(DWORD w, int range, int rot);
DWORD __fastcall dword_rangerot_right(DWORD w, int range, int rot);

HDROP __fastcall MakeDropHandleFromPath(LPCTSTR pszFile);
void __fastcall ViewFileInformation(HWND hWnd, LPCTSTR lpFile);

LPCTSTR __fastcall gtime2jtime(SYSTEMTIME* pst, CString& str, bool bWidthTime);
BOOL __fastcall GetToolBarButtonPoint(const MSG* pmsg, UINT nIDCommand, LPPOINT lppt);
void __fastcall ShadeRect(HDC hdc, const CRect& rc, COLORREF crFrom, COLORREF crTo, bool bVirtical = true);
COLORREF __fastcall PixelAlpha(COLORREF srcPixel, int percent);

void __fastcall CannotCreateWindow(HWND hwnd, LPCTSTR pszText);
void __fastcall InitDlgCtrl(HWND hctl, ATL::_U_STRINGorID res);


DWORDLONG MakeQWORDHash(const BYTE* pcb, int len);

void ShowCtrlHelp(HWND hwndCtrl, UINT idCtrl);
bool UnicodeToUTF8(LPCWSTR pwszUnicode, CStringA& strUtf);

HRESULT CoCreateInstanceAsAdmin(HWND hwnd, BOOL bAdmin, REFCLSID rclsid, REFIID riid, __out void ** ppv);
BOOL isMediaInserted(int drive);
bool CheckFileWritable(const char* pszFile);
int __fastcall CompareFileName(LPCTSTR src, LPCTSTR dst);
const KNOWNFOLDERID& LookupKnownFolder(LPCTSTR keyword);
LPCTSTR PathAnalyze(LPTSTR path, int cchMax);



#define SIMPLE_TOOLBAR_STYLE	(WS_CHILD | WS_VISIBLE | WS_CLIPCHILDREN | WS_CLIPSIBLINGS | TBSTYLE_TOOLTIPS)
#ifdef __AFX_H__
	#define DEF_IDW_TOOLBAR		AFX_IDW_TOOLBAR
#else
	#define DEF_IDW_TOOLBAR		ATL_IDW_TOOLBAR
#endif


///////////////////////////////////////////////////////////////////////
// exec.cpp
//
BOOL AddinEditor(HWND hParent, LPCTSTR pszEditFile, LPCTSTR pszDefaultDir);
BOOL AddinViewer(HWND hParent, LPCTSTR pszViewFile);
BOOL AddinHexEditor(HWND hParent, LPCTSTR pszViewFile);
BOOL AddinImageViewer(HWND hParent, LPCTSTR pszViewFile, bool bRunAsViewer = false);
BOOL DoAddin(HWND hParent, LPCTSTR pExe, LPCTSTR pFile, LPCTSTR pszDefaultDir);
BOOL ExecAddin(HWND hParent, LPCTSTR pszName, LPCITEMIDLIST pidlFq = NULL);
void ExecNewFrame(LPCITEMIDLIST pidl, LPCTSTR pszParam = NULL, bool bAsAdmin = false);
void ExecNewFrame(LPCTSTR pszFile, LPCTSTR pszParam = NULL, bool bAsAdmin = false);
BOOL _getdiskfreespace(LPCTSTR pszRoot, DWORDLONG& sfree, DWORDLONG& stotal);
BOOL GetAddin(int nEntry, CString& str);
void SetAddin(int nEntry, LPCTSTR pszAddin);
BOOL ExecuteAsAdmin(HWND hwnd, LPCTSTR pszCmdLine, LPCTSTR pszDir, UINT nShowCmd, bool bAsAdmin);


///////////////////////////////////////////////////////////////////////
// assoc.cpp
//
void DoAssociate(LPCTSTR pszFileName);


///////////////////////////////////////////////////////////////////////
// keymap.cpp
//
int GetCommandString(UINT nKey, CString& name);
int GetCommandLabel(UINT nKey, CString& name);




///////////////////////////////////////////////////////////////////////
// maptype.cpp
//
void __fastcall InitMapType();
void __fastcall WepMapType();
LPCTSTR __fastcall GetMapType(LPCTSTR pszExt);
LPCTSTR __fastcall AddMapType(LPCTSTR pszExt, LPCTSTR pszType);


///////////////////////////////////////////////////////////////////////
// match.cpp

int __fastcall match(LPCTSTR string, LPCTSTR pattern);
int __fastcall match_ex(LPCTSTR string, LPCTSTR pattern);


///////////////////////////////////////////////////////////////////////
// renext.cpp
int RenameExtension(LPCTSTR pszPath, LPCTSTR pszSrcExt, LPCTSTR pszDstExt, BOOL bIncFolder);


///////////////////////////////////////////////////////////////////////
// queryfolder.cpp
BOOL QueryFolder(HWND hwnd, LPCTSTR pszRoot, LPTSTR pszPath, int cchMax, BOOL& bTreaded, BOOL& bAsAdmin);


///////////////////////////////////////////////////////////////////////
// fvshell.cpp
void InitShell();
IAssocHandler* GetRecommendedAssocHandler(LPCWSTR pszExtra);
int GetIDListsFromDataObject(IDataObject* pdo, CSimpleArray<PCIDLIST_ABSOLUTE>& array);


///////////////////////////////////////////////////////////////////////
// logdisk.cpp

LPCITEMIDLIST __fastcall LogDisk(HWND hWnd, LPITEMIDLIST pidlBase, int& mode);

///////////////////////////////////////////////////////////////////////
// contextmenu_sup.cpp

int TrackContextMenu(HWND hwnd, IContextMenu* pcm, POINT* ppt, SFGAOF attr = 0);
int TrackContextMenuByIDList(HWND hwnd, LPCITEMIDLIST pidl, LPPOINT ppt);
int TrackContextMenuByFile(HWND hwnd, LPCTSTR pszFile, LPPOINT ppt);

///////////////////////////////////////////////////////////////////////
// base64 encode/decode

BYTE* b64Decode(const char* inptr, int* plen);
char* b64Encode(const BYTE* inptr, int size);

///////////////////////////////////////////////////////////////////////
// resource_util.cpp

int GetToolbarCommandFromResource(UINT idToolbar, CSimpleArray<UINT>& array);
BOOL SetBitmapToMenuItem(HMENU hmenu, UINT id, HBITMAP hbm);
HBITMAP CreateBitmapFromImageList(HIMAGELIST him, int iImage);
HCURSOR CreateCursorFromIcon(HICON hic, const POINT* pSpot = NULL);
HCURSOR ImageList_GetCursor(HIMAGELIST him, int index, UINT flags, const POINT* pSpot = NULL);
void __fastcall SetDialogIcon(HWND hdlg, UINT idIcon);
void __fastcall RemoveDialogIcon(HWND hdlg);
HIMAGELIST __fastcall MakeDummyImageList(int cx, int cy);

///////////////////////////////////////////////////////////////////////
// inline function
//

__inline BOOL _getfileinfo(LPCTSTR pszName, WIN32_FIND_DATA* pwfd)
	{ HANDLE hf = FindFirstFile(pszName, pwfd);
	  DWORD dw = GetLastError();
	  if (hf != INVALID_HANDLE_VALUE)
		  FindClose(hf);
	  return (hf != INVALID_HANDLE_VALUE || dw == ERROR_MORE_DATA); }

__inline bool IsCommandTranslateAccel(const _ATL_MSG* msg)
	{ return (msg->message == WM_COMMAND && HIWORD(msg->wParam) == 1); }

__inline COLORREF mix_color(COLORREF c1, COLORREF c2, int t)
	{
		return RGB(	(t * GetRValue(c1) + (100 - t) * GetRValue(c2)) / 100,
					(t * GetGValue(c1) + (100 - t) * GetGValue(c2)) / 100,
					(t * GetBValue(c1) + (100 - t) * GetBValue(c2)) / 100	);
	}

__inline bool isColorDarken(COLORREF clr)
	{ return ((clr >> 16) + ((clr >> 8) & 0x00ff) + (clr & 0x0000ff) < 500); }


__inline DWORD SystemTimeToDWord(SYSTEMTIME* pft = NULL)
{
	SYSTEMTIME st;
	if (!pft) {
		GetSystemTime(&st);
		pft = &st;
	}
	return (DWORD)(
		(DWORD)pft->wSecond +							// use 6bit 0-59
		((DWORD)pft->wMinute << 6) + 					// use 6bit 0-59
		((DWORD)pft->wHour << 12) +						// use 5bit 0-11
		((DWORD)pft->wDay << 17) +						// use 5bit 1-31
		((DWORD)pft->wMonth << 22) +					// use 4bit 1-12
		((DWORD)(pft->wYear - 2000) << 26));			// use 6bit 0-63
}

__inline void DWordToSystemTime(DWORD tm, SYSTEMTIME* pft)
{
	memset(pft, 0, sizeof(SYSTEMTIME));
	pft->wSecond = (WORD)(tm & 0x3f);
	pft->wMinute = (WORD)((tm >> 6) & 0x3f);
	pft->wHour = (WORD)((tm >> 12) & 0x1f);
	pft->wDay = (WORD)((tm >> 17) & 0x1f);
	pft->wMonth = (WORD)((tm >> 22) & 0xf);
	pft->wYear = (WORD)((tm >> 26) & 0x3f) + 2000;
}
