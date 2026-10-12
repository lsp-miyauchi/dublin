#include "stdafx.h"
#include "resource.h"
#include "common.h"


UINT nRM_OPEN      = RegisterWindowMessage(_T("wm_textpad_open"));
UINT nRM_JUMPTO    = RegisterWindowMessage(_T("wm_textpad_jumpto"));
UINT nRM_FINDNOTIFY = RegisterWindowMessage(FINDMSGSTRING);

COSPlatform gosp;

///////////////////////////////////////////////////////////////////////
// COMMON FUNCTIONS
//

int AppMessage(UINT nID, LPCTSTR pszInsert /*=NULL*/, UINT nType /*=MB_STOP*/)
{
	HINSTANCE hInst = ATL::_AtlBaseModule.GetResourceInstance();
	CString cs;
	cs.LoadString(hInst, nID);
	if (pszInsert && lstrlen(pszInsert))
		cs.Replace(_T("%1"), pszInsert);

	CString tit;
	tit.LoadString(hInst, IDS_APP_TITLE);
	return MessageBox(GetActiveWindow(), cs, tit, nType);
}

int LastError(LPCTSTR pszTitle, UINT nType /*=MB_STOP*/)
{
	int nError = GetLastError();
	CString cs;
	cs.LoadString(ATL::_AtlBaseModule.GetResourceInstance(), IDS_APP_TITLE);
	return lccErrorBox(::GetFocus(), nError, cs, pszTitle, nType);
}

TCHAR* __fastcall SkipTabSpace(TCHAR* p)
{
	if (p) {
		while (*p == ' ' || *p == '\t')
			p++;
	}
	return p;
}

int SeparateString(LPCTSTR pszText, int delim, CSimpleArray<CString>& arr)
{
	arr.RemoveAll();
	TCHAR* buffer = new TCHAR[lstrlen(pszText) + 2];
	lstrcpy(buffer, pszText);

	TCHAR *p = SkipTabSpace(buffer);
	TCHAR *q;
	while (p && *p) {
		if (*p == '"') {
			q = StrChr(++p, _T('"'));
			if (q)
				*q++ = 0;
			else
				break;
		}
		else {
			q = StrChr(p + 1, delim);
			if (q)
				*q++ = 0;
		}
		arr.Add(p);
		p = SkipTabSpace(q);
	}
	return arr.GetSize();
}

void __fastcall FormatText(LPCTSTR lpszSrc, LPTSTR lpszDst, int nTabConv, int nMax)
{
	int len = lstrlen(lpszSrc);
	register int i, j;
	for (i = j = 0; i < len; i++)
	{
		if (j >= nMax - 1)
			break;

		if (lpszSrc[i] == '\t')
		{
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

void PopupHint(HWND hwnd, UINT nID, DWORD style)
{
	WCHAR buf[MAX_PATH];
	LoadStringW(ATL::_AtlBaseModule.GetResourceInstance(), nID, buf, ARRAYSIZE(buf));
	lccPopView(hwnd, NULL, buf, style);
}


static DWORD dwCookie = 0;
static TCHAR sszHtmlHelp[MAX_PATH];

void InitHtmlHelp()
{
/*
	HWND hwndCapture = GetCapture();
	if (hwndCapture != NULL)
		SendMessage(hwndCapture, WM_CANCELMODE, 0, 0);

	if (!dwCookie) {
		GetModuleFileName(ATL::_AtlBaseModule.GetModuleInstance(), sszHtmlHelp, ARRAYSIZE(sszHtmlHelp));
		PathRenameExtension(sszHtmlHelp, _T(".chm"));
		HtmlHelp(NULL, NULL, HH_INITIALIZE, (DWORD_PTR)&dwCookie);
	}
*/
}

void __fastcall ShowHtmlHelp(HWND hwnd, UINT idDlg)
{
/*
	InitHtmlHelp();
	HtmlHelp(hwnd, sszHtmlHelp, HH_HELP_CONTEXT, idDlg + HID_BASE_RESOURCE);
*/
}

void __fastcall ResHelp(HWND hwnd, UINT idResource)
{
	TCHAR buf[MAX_PATH * 2];
	TCHAR dll[MAX_PATH];
	GetModuleFileName(ATL::_AtlBaseModule.GetModuleInstance(), dll, ARRAYSIZE(dll));
	GetShortPathName(dll, dll, ARRAYSIZE(dll));
	wsprintf(buf, _T("res://%s/%d"), dll, idResource);
	TCHAR* p = PathFindFileName(dll);
	*p = '\0';
	lccExecApp(hwnd, buf, dll, SW_SHOWNORMAL);
}

void InitDlgCtrl(HWND hctl, UINT idResource)
{
	TCHAR buf[MAX_PATH * 2];
	if (GetClassName(hctl, buf, ARRAYSIZE(buf)) < 1) return;
	CString str;
	if (str.LoadString(idResource) < 1) return;

	int type = -1;
	if (!StrCmpI(buf, WC_COMBOBOX))
		type = 1;
	else if (!StrCmpI(buf, WC_COMBOBOXEX))
		type = 2;
	else if (!StrCmpI(buf, WC_LISTBOX))
		type = 3;

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
		case 3:
			::SendMessage(hctl, LB_ADDSTRING, 0, (LPARAM)p);
			break;
		}
		p = q;
	}
	::SendMessage(hctl, (type < 3) ? CB_SETCURSEL : LB_SETCURSEL, 0, 0);
}

