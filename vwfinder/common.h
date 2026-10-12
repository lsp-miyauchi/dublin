////////////////////////////////////////////////////////////////////
// common.h

#pragma once

extern int (STDAPICALLTYPE* rStringCompare)(LPCWSTR, LPCWSTR);

extern TCHAR gszAst[];
extern TCHAR gszCrLf[];
extern TCHAR gszDateForm[];
extern TCHAR gszShell32[];
extern CColorTheme gColors;

#ifndef HID_BASE_COMMAND

// Help ID bases
#define HID_BASE_COMMAND    0x00010000UL        // ID and IDM
#define HID_BASE_RESOURCE   0x00020000UL        // IDR and IDD
#define HID_BASE_PROMPT     0x00030000UL        // IDP
#define HID_BASE_NCAREAS    0x00040000UL
#define HID_BASE_CONTROL    0x00050000UL        // IDC
#define HID_BASE_DISPATCH   0x00060000UL        // IDispatch help codes

#endif


////////////////////////////////////////////////////////////////////
// FILE ATTRIBUTES
//


#define FAT_SUBDIR				FILE_ATTRIBUTE_DIRECTORY
#define FAT_SYSTEM				FILE_ATTRIBUTE_SYSTEM
#define FAT_HIDDEN				FILE_ATTRIBUTE_HIDDEN
#define FAT_RDONLY				FILE_ATTRIBUTE_READONLY
#define FAT_ARCH				FILE_ATTRIBUTE_ARCHIVE
#define FAT_NORMAL				FILE_ATTRIBUTE_NORMAL
#define FAT_TEMP				FILE_ATTRIBUTE_TEMPORARY
#define FAT_CMPRSD				FILE_ATTRIBUTE_COMPRESSED
#define FAT_ENCRYPT				FILE_ATTRIBUTE_ENCRYPTED
#define FAT_ATOMIC				FILE_ATTRIBUTE_ATOMIC_WRITE
#define FAT_SPARSE				FILE_ATTRIBUTE_SPARSE_FILE
#define FAT_OFFLINE				FILE_ATTRIBUTE_OFFLINE
#define FAT_TEMP				FILE_ATTRIBUTE_TEMPORARY
#define FAT_REPARSE				FILE_ATTRIBUTE_REPARSE_POINT


//----------------------------------------------
// SORT MODE DEFINITION
//
enum SortType {
	SORT_NONE = 0,
	SORT_NAME,
	SORT_EXT,
	SORT_SIZE,
	SORT_TIME,
	SORT_ATTR,
	SORT_TYPE,
	SORT_PATH,
};

enum SortSeq {
	SORT_UP = 0,
	SORT_DOWN
};

// リスト操作系列
#define LTS_FMTDATE			0x00000002			// 2001年 / 01年
#define LTS_LOWERCASE		0x00000004			// Lower case
#define LTS_ALIGNEXT		0x00000008			// 拡張子を揃える
#define LTS_FMTSIZE			0x00000010			// 1.23KB / 23.4MB 等の表示
#define LTS_INFOTIP			0x00000020			// チップ情報
#define LTS_TBSMALL			0x00000040
#define LTS_REPCHECKER		0x00000080			// 1行おきに背景色を替える
#define LTS_RELATIVEDATE	0x00000100			// 昨日・今日

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

////////////////////////////////////////////////

#define LODWORD(a)				(*((DWORD*)&(a)))
#define HIDWORD(a)				(*(((DWORD*)&(a)) + 1))
#define SETDWORDLONG(a,b,c)		(*((DWORD*)&(a))=(DWORD)(b),*(((DWORD*)&(a))+1)=(DWORD)(c))


//////////////////////////////////////////////////////////////////////
//

class CToday
{
private:
	DWORDLONG m_dayToday;
public:
	CToday() { UpdateToday(); }
	void UpdateToday()
	{
		SYSTEMTIME st;
		GetLocalTime(&st);
		st.wHour = st.wMinute = st.wSecond = st.wMilliseconds = 0;
		SystemTimeToFileTime(&st, (FILETIME*)&m_dayToday);
	}
	inline bool isToday(DWORDLONG day) { return (m_dayToday <= day && day < (m_dayToday + 864000000000)); }
	inline bool isYesterday(DWORDLONG day) { return ((m_dayToday - 864000000000) <= day && day < m_dayToday); }
};

//////////////////////////////////////////////////
//

_inline TCHAR ToUpper(TCHAR ch)
	{ return ('a' <= ch && ch <= 'z') ? (ch + ('a' - 'A')) : ch; }
_inline LPTSTR __fastcall tchar_malloc(size_t size)
	{ return (LPTSTR)malloc(size * sizeof(TCHAR)); }
_inline LPTSTR __fastcall tchar_realloc(LPTSTR p, size_t size)
	{ return (LPTSTR)realloc(p, size * sizeof(TCHAR)); }


//////////////////////////////////////////////////
//

void __fastcall ShowHtmlHelp(HWND hwnd, UINT idDlg);
void __fastcall ResHelp(HWND hwnd, UINT idResource);
int AppMessage(HWND hwnd, UINT nID, LPCTSTR pszInsert = NULL, UINT nType = MB_STOP);
int LastError(LPCTSTR pszTitle, UINT nType = MB_STOP);
TCHAR* __fastcall SkipTabSpace(TCHAR* p);
int SeparateString(LPCTSTR pszText, int delim, CSimpleArray<CString>& array);
LPTSTR __fastcall num_form(DWORDLONG dlNum, LPTSTR pszFormat);
int __fastcall FormatFileSize(DWORDLONG size, LPTSTR pszSize, BOOL bFmtByte);
int __fastcall FormatFileTime(FILETIME* pft, LPTSTR pszDate, BOOL b2col, BOOL bRelative);
void __fastcall FormatText(LPCTSTR lpszSrc, LPTSTR lpszDst, int nTabConv, int nMax);
void __fastcall AddNormalTab(HWND hwnd, LPCTSTR lpTitle, HIMAGELIST hImageList);
void __fastcall tchar_set(LPTSTR psz, TCHAR ch, size_t size);

void PopupHint(HWND hwnd, UINT nID, DWORD style = PVS_CAPTION);
int __fastcall match(LPCTSTR string, LPCTSTR pattern);

BOOL RunBinDump(HWND hWnd, LPCTSTR pszName);
void ViewFile(HWND hWnd, LPCTSTR pName, LPCTSTR pDir);
void myShellExecute(HWND hWnd, LPCTSTR pName, LPCTSTR pDir);
void __fastcall ResHelp(HWND hwnd, UINT idResource);

void InitDlgCtrl(HWND hctl, UINT idResource);

void ReadStringFromStream(IStream* pstrm, CString& str);
void WriteStringToStream(IStream* pstrm, CString& str);
