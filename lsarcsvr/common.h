//////////////////////////////////////////////
// common.h

#pragma once
#include "comarcdef.h"
#include "reg.h"



//////////////////////////////////////////////////////////////////////
// グローバル変数

extern COSPlatform gosp;

//////////////////////////////////////////////////////////////////////
// MessageBox

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

//////////////////////////////////////////////////////////////////////
//

enum {
	CR_NO = 0,
	CR_YES,
	CR_YESALL,
};

enum {
	BASEDIR_NOPATH = 0,
	BASEDIR_RELATIVE = 1,
	BASEDIR_ABSOLUTE = 2,
};

//////////////////////////////////////////////////////////////////////
// shortcut info

typedef struct tagLINKINFO {
	WIN32_FIND_DATA wfd;
	TCHAR szName[MAX_PATH * 2];
	TCHAR szTargetName[MAX_PATH * 2];
	TCHAR szWorkDir[MAX_PATH * 2];
	TCHAR szArguments[MAX_PATH * 2];
	TCHAR szDescription[MAX_PATH * 2];
	TCHAR szIconPath[MAX_PATH * 2];
	int nIconIndex;
} LINKINFO, *LPLINKINFO;


/////////////////////////////////////////////////////////////////////
// CWaitCursor

class CWaitCursor
{
private:
	HCURSOR m_hcur;
public:
	CWaitCursor() { m_hcur = SetCursor(LoadCursor(NULL, IDC_WAIT)); }
	~CWaitCursor() { SetCursor(m_hcur); }
};

/////////////////////////////////////////////////////////////////////
// CEnterFolder

class CEnterFolder
{
private:
	CString m_strBasePath;

public:
	CEnterFolder()
	{
		LPTSTR p = m_strBasePath.GetBuffer(MAX_PATH * 2);
		GetCurrentDirectory(MAX_PATH * 2, p);
		m_strBasePath.ReleaseBuffer();
	}
	~CEnterFolder()
	{
		Leave();
	}
	BOOL Enter(LPCTSTR pszNewPath)
	{
		return SetCurrentDirectory(pszNewPath);
	}
	void Leave()
	{
		if (!m_strBasePath.IsEmpty()) {
			SetCurrentDirectory(m_strBasePath);
			m_strBasePath.Empty();
		}
	}
};


//////////////////////////////////////////////////////////////////////
// DateTime control helper

_inline BOOL DateTime_SetTime(HWND hwnd, SYSTEMTIME* pTime)
	{ WPARAM wParam = (!pTime) ? GDT_NONE : GDT_VALID;
	  return (BOOL)SendMessage(hwnd, DTM_SETSYSTEMTIME, wParam, (LPARAM)pTime); }
_inline DWORD DateTime_GetTime(HWND hwnd, SYSTEMTIME* pTime)
	{ return (DWORD)SendMessage(hwnd, DTM_GETSYSTEMTIME, 0, (LPARAM)pTime); }

#define SETFormatEtc(fe, cf, med, li)\
		{\
			(fe).cfFormat = cf;\
			(fe).dwAspect = DVASPECT_CONTENT;\
			(fe).ptd = NULL;\
			(fe).tymed = med;\
			(fe).lindex = li;\
		};

int SpecifyArchiveType(LPCTSTR lpszName);
int SpecifyArchiveTypeByFileHeader(LPCTSTR pszName);

void __fastcall ShowHtmlHelp(HWND hwnd, UINT idDlg);
void __fastcall ShowCtrlHelp(HWND hwndCtrl, UINT idCtrl);
int WStrToStr(LPCWSTR pszWide, CStringA& str);
int StrToWStr(LPCSTR psz, CStringW& str);
int GetCheckedRadioButton(HWND hwnd, int nIDFirst, int nIDLast);
HGLOBAL GetDataFromDataObject(IDataObject* pdo, CLIPFORMAT type);
bool MakeTempFile(HDROP hDrop, LPTSTR pszPath, int pathMode, bool bDos, int nType, bool bUnicode = false, bool bUtf8 = false, LPCTSTR pszBasePath = NULL);
int AppMessage(UINT id, LPCTSTR pszInsert=NULL, UINT nType=MB_STOP);
int LastError(DWORD code, LPCTSTR pszInsert=NULL, UINT nTypeAction=MB_STOP);
BOOL __iszipfile(LPCTSTR pszName);
BOOL __fastcall FileBrowse(BOOL bRead, HWND hwnd, UINT nIDCaption, UINT nIDFilter, LPTSTR pszName);
int __fastcall BrowseForFolder(HWND hwnd, LPTSTR pszPath);
BOOL __fastcall ParseLinkInfo(LPCTSTR pszName, LINKINFO* pli);
BOOL __fastcall GetPathCommonPrefix(HDROP hdrop, LPTSTR pszPrefix);
LPTSTR __fastcall _strcombine(LPTSTR pszDst, LPCTSTR pszSrc1, LPCTSTR pszSrc2);
LPTSTR __fastcall _addchar(LPTSTR psz, TCHAR ch);
int ComparePath(LPCTSTR pszSrc, LPCTSTR pszDst);
BOOL PathBuild(LPCTSTR pszSrc, LPCTSTR pszBase, LPTSTR pszDst);
FARPROC rGetProcAddress(HMODULE hmod, LPCTSTR pszProc);
BOOL GetLinkTarget(LPCTSTR pszLinkFile, LPTSTR pszTarget, int cchMax);
BOOL exec_console_app(LPCTSTR pszCmdLine, LPCTSTR pszDir, LPCTSTR pszConsoleTitle);
BOOL __GetArchiverTypeInfo(ULONG type, LPWSTR pszPrefix, LPWSTR pszDll, BOOL& rbUnicodeSupported, BOOL& bUtf8Supported);
void __fastcall InitDlgCtrl(HWND hctl, UINT idResource);
void __fastcall SetDialogIcon(HWND hdlg, UINT idIcon);
void __fastcall RemoveDialogIcon(HWND hdlg);
void AddMyPathEnvironment();
BOOL CopyTextToClipboard(HWND hwnd, LPCTSTR pszText);
LPTSTR FormatFileDateTime(FILETIME* pft, LPTSTR psz, int cchMax);

void iiw2iia(INDIVIDUALINFOW* iiw, INDIVIDUALINFO* iia);
void iia2iiw(INDIVIDUALINFO* iia, INDIVIDUALINFOW* iiw, bool utf8 = false);

//------ utfconvert.cpp
int EncodeWStrToUTF(const WCHAR* widestr, int widelen, BYTE* utfstr, int utflen);
int DecodeUTFToWStr(const BYTE* utfstr, int utflen, WCHAR* widestr, int widelen);

//------ stringutil.cpp
CSimpleArray<CString> Split(LPCTSTR target, LPCTSTR delim);
