#include "stdafx.h"
#include "resource.h"
#include "common.h"

#include "..\lsimg\imgspecify.cpp"


///////////////////////////////////////////////////////////////////////
// グローバルデータ

COSPlatform gosp;
TCHAR gszAst[] = _T("*.*");
TCHAR gszCrLf[] = _T("\x0d\x0a");
TCHAR gszDateForm[] = _T("9999/99/99 99:99:99");
TCHAR gszShell32[] = _T("shell32.dll");
CColorTheme gColors;

enum {
	FT_NORMAL,
	FT_PROGRAM,
	FT_ASSOCIATE,
	FT_ARC,
	FT_DIR,
};

///////////////////////////////////////////////////////////////////////
// for HtmlHelp


class CHtmlHelp {
private:
	HMODULE m_hmod;
	HWND (WINAPI* m_proc)(HWND, LPCWSTR, UINT, DWORD_PTR);
	DWORD m_cookie;
	TCHAR m_helpFile[MAX_PATH * 2];


public:
	CHtmlHelp() {
		m_cookie = 0;
		m_hmod = NULL;
		m_proc = NULL;
	}
	~CHtmlHelp() {
		if (m_hmod) {
			FreeLibrary(m_hmod);
		}
	}
	void Initialize() {
		m_hmod = LoadLibrary(_T("htmlhelp.dll"));
		if (m_hmod) {
			(FARPROC&)m_proc = GetProcAddress(m_hmod, "HtmlHelpW");
			if (m_proc) {
				HWND hwndCapture = GetCapture();
				if (hwndCapture != NULL)
					SendMessage(hwndCapture, WM_CANCELMODE, 0, 0);

				if (!m_cookie) {
					GetModuleFileName(MODINST, m_helpFile, ARRAYSIZE(m_helpFile));
					PathRenameExtension(m_helpFile, _T(".chm"));
					Dispatch(NULL, HH_INITIALIZE, (DWORD_PTR)&m_cookie);
				}
			} else {
				FreeLibrary(m_hmod);
				m_hmod = NULL;
			}
		}
	}
	HWND Dispatch(HWND hwnd, UINT command, DWORD_PTR data)
	{
		if (!m_cookie) {
			Initialize();
		}
		return m_proc ? (*m_proc)(hwnd, m_helpFile, command, data) : NULL;
	}

};


static CHtmlHelp sHtmlHelp;


void __fastcall ShowHtmlHelp(HWND hwnd, UINT idDlg)
{
	sHtmlHelp.Dispatch(hwnd, HH_HELP_CONTEXT, idDlg + HID_BASE_RESOURCE);
}

void __fastcall ResHelp(HWND hwnd, UINT idResource)
{
	TCHAR buf[MAX_PATH * 2];
	TCHAR dll[MAX_PATH];
	GetModuleFileName(MODINST, dll, _COUNTOF(dll));
	GetShortPathName(dll, dll, _COUNTOF(dll));
	wsprintf(buf, _T("res://%s/%d"), dll, idResource);
	TCHAR* p = PathFindFileName(dll);
	*p = '\0';
	lccExecApp(hwnd, buf, dll, SW_SHOWNORMAL);
}

///////////////////////////////////////////////////////////////////////
// COMMON FUNCTIONS
//

int AppMessage(HWND hwnd, UINT idMsg, LPCTSTR pszInsert /*=NULL*/, UINT nType /*=MB_STOP*/)
{
	CString msg;
	msg.LoadString(idMsg);
	if (pszInsert && *pszInsert)
		msg.Replace(_T("%1"), pszInsert);

	if (!hwnd || !IsWindow(hwnd)) hwnd = ::GetActiveWindow();

	CString tit;
	tit.LoadString(IDS_APP_TITLE);
	return ::MessageBox(hwnd, msg, tit, nType);
}

int LastError(LPCTSTR pszTitle, UINT nType /*=MB_STOP*/)
{
	int nError = GetLastError();
	CString cs;
	cs.LoadString(IDS_APP_TITLE);
	return lccErrorBox(::GetFocus(), nError, cs, pszTitle, nType);
}

TCHAR* __fastcall SkipTabSpace(TCHAR* p)
{
	if (p) {
		while (*p == ' ' || *p == '\t') p++;
	}
	return p;
}

int SeparateString(LPCTSTR pszText, int delim, CSimpleArray<CString>& array)
{
	array.RemoveAll();

	LPTSTR base = StrDup(pszText);
	if (base) {
		TCHAR *p = SkipTabSpace(base);
		TCHAR *q;
		while (p && *p) {
			if (*p == '"') {
				q = StrChr(++p, '"');
				if (q)
					*q++ = '\0';
				else
					break;
			}
			else {
				q = StrChr(p + 1, delim);
				if (q)
					*q++ = '\0';
			}
			array.Add(p);
			p = SkipTabSpace(q);
		}
		LocalFree(base);
	}
	return array.GetSize();
}

LPTSTR __fastcall num_form(DWORDLONG dlNum, LPTSTR pszFormat)
{
	DWORD t0 = (DWORD)(dlNum / (DWORDLONG)1000000000);
	dlNum = dlNum % (DWORDLONG)1000000000;
	DWORD t1 = (DWORD)(dlNum / (DWORDLONG)1000000);
	dlNum = dlNum % (DWORDLONG)1000000;
	DWORD t2 = (DWORD)(dlNum / (DWORDLONG)1000);
	DWORD t3 = (DWORD)(dlNum % (DWORDLONG)1000);

	TCHAR buf[32];
	wsprintf(buf, _T("%03u,%03u,%03u,%03u"), t0, t1, t2, t3);
	register int i;
	for (i = 0; buf[i] != 0; i++) {
		if (buf[i] != '0' && buf[i] != ',')
			break;
	}

	if (!buf[i]) {
		buf[i]     = '0';
		buf[i + 1] = '\0';
	}
	lstrcpy(pszFormat, &buf[i]);
	return pszFormat;
}

int __fastcall FormatFileSize(DWORDLONG size, LPTSTR pszSize, BOOL bFmtByte)
{
	if (bFmtByte) {
		if (size < 1024)
			size = (size == 0) ? 0 : 1;
		else
			size >>= 10;
		lstrcat(num_form(size, pszSize), _T(" KB"));
	}
	else
		num_form(size, pszSize);
	return lstrlen(pszSize);
}

int __fastcall FormatFileTime(FILETIME* pft, LPTSTR pszDate, BOOL b2col, BOOL bRelative)
{
	SYSTEMTIME st;
	FileTimeToSystemTime(pft, &st);

	CString str;
	str.Format(_T("%04u/%02u/%02u"), st.wYear, st.wMonth, st.wDay);
	if (b2col)
		str.Delete(0, 2);
	if (bRelative) {
		CToday today;
		if      (today.isToday(*(DWORDLONG*)pft))     str = _T("今日");
		else if (today.isYesterday(*(DWORDLONG*)pft)) str = _T("昨日");
	}
	str.AppendFormat(_T(" %02u:%02u:%02u"), st.wHour, st.wMinute, st.wSecond);
	lstrcpy(pszDate, str.GetString());
	return lstrlen(pszDate);
}

void __fastcall FormatText(LPCTSTR lpszSrc, LPTSTR lpszDst, int nTabConv, int nMax)
{
	int len = lstrlen(lpszSrc);
	register int i, j;
	for (i = j = 0; i < len; i++) {
		if (j >= nMax - 1)
			break;

		if (lpszSrc[i] == '\t') {
			for (int k = j % nTabConv; k < nTabConv; k++)
				lpszDst[j++] = ' ';
		}
		else
			lpszDst[j++] = lpszSrc[i];
	}
	lpszDst[j] = '\0';

	if (lpszDst[j - 1] == '\n')
		lpszDst[j - 1] = '\0';
}

void __fastcall AddNormalTab(HWND hwnd, LPCTSTR lpTitle, HIMAGELIST hImageList)
{
	CTabCtrl tab = hwnd;

	TCHAR buf[512];
	if (hImageList) tab.SetImageList(hImageList);

	lstrcpy(buf, lpTitle);
	LPTSTR p = buf;
	TC_ITEM it;
	int i = 0;
	while (p && *p) {
		LPTSTR q = StrChr(p, '|');
		if (q) *q++ = '\0';

		ZeroMemory(&it, sizeof(it));
		it.mask = TCIF_TEXT;
		it.iImage = -1;
		if (hImageList) {
			it.mask |= TCIF_IMAGE;
			it.iImage = i;
		}
		it.pszText = p;
		it.cchTextMax = lstrlen(p);
		tab.InsertItem(i, &it);
		i++;
		p = q;
	}
}

void __fastcall tchar_set(LPTSTR psz, TCHAR ch, size_t size)
{
	for (size_t i = 0; i < size; i++)
		*psz++ = ch;
}

void PopupHint(HWND hwnd, UINT nID, DWORD style)
{
	CString str;
	str.LoadString(nID);
	lccPopView(hwnd, NULL, str, style);
}

static TCHAR sszShellOpenCommand[] = _T("shell\\open\\command");

BOOL RegGetShellCommand(LPCTSTR pszExt, LPTSTR pszCommand, int cchMax)
{
	if (!pszExt || *pszExt == 0) return 0;

	TCHAR buf[MAX_PATH * 2], name[MAX_PATH * 2];
	LONG size = sizeof(name);
	if (RegQueryValue(HKEY_CLASSES_ROOT, pszExt, name, &size) == ERROR_SUCCESS && size > 0) {
		PathCombine(buf, name, sszShellOpenCommand);
		size = sizeof(name);
		if (RegQueryValue(HKEY_CLASSES_ROOT, buf, name, &size) == ERROR_SUCCESS && size > 0) {
			if (pszCommand && cchMax > 0)
				lstrcpyn(pszCommand, name, cchMax);
			return TRUE;
		}
	}
	else {
		PathCombine(buf, pszExt, sszShellOpenCommand);
		size = sizeof(name);
		if (RegQueryValue(HKEY_CLASSES_ROOT, buf, name, &size) == ERROR_SUCCESS && size > 0) {
			if (pszCommand && cchMax > 0)
				lstrcpyn(pszCommand, name, cchMax);
			return TRUE;
		}
	}
	return FALSE;
}

static int CheckFileType(LPCTSTR pszFileName)
{
	if (!pszFileName || *pszFileName == 0)
		return FT_NORMAL;

	TCHAR* p = PathFindExtension(pszFileName);
	if (*p != '.')
		return FT_NORMAL;

	TCHAR ext[MAX_PATH * 2];
	lstrcpy(ext, p);
	CharLower(ext);

	// 実行モジュールか否かを検証
	TCHAR szExec[MAX_PATH * 2];
	lstrcpy(szExec, _T("exe com pif bat cmd lnk"));
	if (lccPresent(szExec, ext + 1))
		return FT_PROGRAM;

	// 関連付けされているか否かを検証
	if (RegGetShellCommand(ext, NULL, 0))
		return FT_ASSOCIATE;

	// 圧縮ファイル
	if (SpecifyArchiveType(pszFileName) != ARC_ERROR)
		return FT_ARC;

	return FT_NORMAL;
}

static BOOL __fastcall checktoken(LPCTSTR pList, LPCTSTR pToken, TCHAR dlm)
{
	BOOL bFound = FALSE;
	TCHAR* pw = StrDup(pList);
	if (pw)
	{
		TCHAR* p = pw;
		TCHAR* q;
		do {
			q = StrChr(p, dlm);
			if (q)
				*q++ = '\0';

			if (!StrCmpI(p, pToken))
				bFound = TRUE;
			p = q;
		} while (p);
		LocalFree(pw);
	}
	return bFound;
}

static HDROP make_hdrop(LPCTSTR pszName)
{
	POINT pt;
	pt.x = pt.y = 0;
	HDROP hdrop = fopDragInitBlock(pt, FALSE);
	HDROP hw;
	hw = fopDragAppendFile(hdrop, pszName);
	if (!hw) 	{
		DragFinish(hdrop);
		return NULL;
	}
	return hw;
}

BOOL RunTxed(HWND hParent, LPCTSTR pszFile)
{
	TCHAR buf[MAX_PATH * 2];
	lstrcpy(buf, _T("/r "));
	LPTSTR p = buf + lstrlen(buf);
	lstrcpy(p, pszFile);
	PathQuoteSpaces(p);
	return lccDllExecute(hParent, DLL_MIL, MIL_EDIT, buf, _T("/edit"), FALSE);
}

BOOL RunBinDump(HWND hParent, LPCTSTR pszFile)
{
	return lccDllExecute(hParent, DLL_BED, BED_EDIT, pszFile, _T("/bdump"), FALSE);
}

void ViewFile(HWND hWnd, LPCTSTR pName, LPCTSTR pDir)
{
	TCHAR buf[MAX_PATH * 2];
	PathCombine(buf, pDir, pName);
	TCHAR* p = PathFindExtension(buf);
	if (*p == '.') {
		p++;
		if (!lstrcmpi(p, _T("WAV")))
			sndPlaySound(buf, SND_SYNC);
		else if (imgSpecifyImageType(buf))
			lccDllExecute(hWnd, DLL_GMIL, _T("imgRun"), buf, _T("/image"), FALSE);
		else if (SpecifyArchiveType(buf) != ARC_ERROR)
			lccDllExecute(hWnd, DLL_ARC, ARC_VIEW, buf, _T("/archive"), FALSE);
		else if (checktoken(_T("exe dll drv w95 cpl 386 qtc vbx vxd ocx"), p, ' ')) {
			HDROP hdrop = make_hdrop(buf);
			if (hdrop) {
				fopShowFileInfo(hWnd, hdrop);
				DragFinish(hdrop);
			}
		}
		else
			RunTxed(hWnd, buf);
	}
	else
		RunTxed(hWnd, buf);
}

void myShellExecute(HWND hWnd, LPCTSTR pName, LPCTSTR pDir)
{
	TCHAR buf[MAX_PATH * 2];
	PathCombine(buf, pDir, pName);

	int type = CheckFileType(buf);
	if (type == FT_PROGRAM || type == FT_ASSOCIATE)
		ShellExecute(hWnd, NULL, pName, NULL, pDir, SW_NORMAL);
	else
		ViewFile(hWnd, pName, pDir);
}

void InitDlgCtrl(HWND hctl, UINT idResource)
{
	TCHAR buf[MAX_PATH * 2];
	if (GetClassName(hctl, buf, _COUNTOF(buf)) < 1) return;
	CString str;
	if (str.LoadString(idResource) < 1) return;

	int type = -1;
	if (!StrCmpI(buf, WC_COMBOBOX))
		type = 1;
	else if (!StrCmpI(buf, WC_COMBOBOXEX))
		type = 2;

	lstrcpy(buf, str);
	LPTSTR p = buf;
	COMBOBOXEXITEM item;
	while (p && *p) {
		LPTSTR q = StrChr(p, ';');
		if (q) *q++ = 0;

		switch (type) {
		case 1:
			::SendMessage(hctl, CB_ADDSTRING, 0, (LPARAM)p);
			break;
		case 2:
			item.mask = CBEIF_TEXT;
			item.iItem = -1;
			item.pszText = p;
			::SendMessage(hctl, CBEM_INSERTITEM, 0, (LPARAM)&item);
			break;
		}
		p = q;
	}

	::SendMessage(hctl, CB_SETCURSEL, 0, 0);
}

UINT_PTR __ReadStringLength(IStream* pstrm, int& nCharSize)
{
	nCharSize = sizeof(char);

	// 1st, try to read a one-byte length
	ULONG len;
	BYTE bLength;
	pstrm->Read(&bLength, sizeof(bLength), &len);
	if (bLength < 0xff)
		return bLength;

	// 2nd, try a two-byte length
	WORD wLength;
	pstrm->Read(&wLength, sizeof(wLength), &len);
	if (wLength == 0xfffe) {
		// Unicode string.  Start over at 1-byte length
		nCharSize = sizeof(WCHAR);

		pstrm->Read(&bLength, sizeof(bLength), &len);
		if (bLength < 0xff)
			return bLength;

		// Two-byte length
		pstrm->Read(&wLength, sizeof(wLength), &len);
		// Fall through to continue on same branch as ANSI string
	}
	if (wLength < 0xffff)
		return wLength;

	// 3rd, 4-byte length
	DWORD dwLength;
	pstrm->Read(&dwLength, sizeof(dwLength), &len);
	if (dwLength < 0xffffffff)
		return dwLength;

	// 8-byte length
	ULONGLONG qwLength;
	pstrm->Read(&qwLength, sizeof(qwLength), &len);

	return (UINT_PTR)qwLength;
}

void ReadStringFromStream(IStream* pstrm, CString& str)
{
	str.Empty();

	ULONG len;

	int nCharSize;  // 1 = char, 2 = wchar_t
	UINT nLength = UINT(__ReadStringLength(pstrm, nCharSize) );
	if (nLength > 0) {
		if (nCharSize == sizeof(char)) {
			ATL::CTempBuffer< char > pszBufferA(nLength + 1);
			pstrm->Read(pszBufferA, nLength * sizeof(char), &len);
			pszBufferA[len / sizeof(char)] = 0;
			LPTSTR p = str.GetBuffer(len * 3);
			MultiByteToWideChar(CP_ACP, 0, pszBufferA, len, p, len * 3);
			str.ReleaseBuffer();
		}
		else {
			ATL::CTempBuffer<WCHAR> pszBufferW(nLength);
			pstrm->Read(pszBufferW, nLength * sizeof(WCHAR), &len);
			pszBufferW[len / sizeof(WCHAR)] = 0;
			str = pszBufferW;
		}
	}
}

void __WriteStringLength(IStream* pstrm, UINT_PTR nLength, BOOL bUnicode)
{
	ULONG len;
	BYTE b;
	WORD w;
	DWORD d;
	ULONGLONG u;

	if (bUnicode) {
		// Tag Unicode strings
		b = 0xff;
		w = 0xfffe;
		pstrm->Write(&b, sizeof(b), &len);
		pstrm->Write(&w, sizeof(w), &len);
	}

	if (nLength < 255) {
		b = (BYTE)nLength;
		pstrm->Write(&b, sizeof(b), &len);
	}
	else if (nLength < 0xfffe) {
		b = (BYTE)0xff;
		w = (WORD)nLength;
		pstrm->Write(&b, sizeof(b), &len);
		pstrm->Write(&w, sizeof(w), &len);

	}
	else if (nLength < 0xffffffff) {
		b = (BYTE)0xff;
		w = (WORD)0xffff;
		d = (WORD)nLength;
		pstrm->Write(&b, sizeof(b), &len);
		pstrm->Write(&w, sizeof(w), &len);
		pstrm->Write(&d, sizeof(d), &len);
	}
	else {
		b = (BYTE)0xff;
		w = (WORD)0xffff;
		d = (WORD)nLength;
		u = (ULONGLONG)nLength;
		pstrm->Write(&b, sizeof(b), &len);
		pstrm->Write(&w, sizeof(w), &len);
		pstrm->Write(&d, sizeof(d), &len);
		pstrm->Write(&u, sizeof(u), &len);
	}
}

void WriteStringToStream(IStream* pstrm, CString& str)
{
	UINT_PTR nLength = (UINT_PTR)str.GetLength();
	__WriteStringLength(pstrm, nLength, sizeof(TCHAR) == sizeof(WCHAR));
	if (nLength > 0) {
		ULONG len;
		pstrm->Write((LPCTSTR)str, str.GetLength() * sizeof(TCHAR), &len);
	}
}
