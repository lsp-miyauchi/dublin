#include <windows.h>
#include <windowsx.h>
#include <commctrl.h>
#include <shlwapi.h>
#include "lspuzp.h"
#include "resource.h"

extern BOOL gbTryLastPassword;
extern HMODULE hUzInstance;

static const int TICKS = 100;

enum {
	REPLACE_STOP	= 0,
	REPLACE_NO		= 1,
	REPLACE_YES		= 2,
	REPLACE_ALL		= 3,
	REPLACE_NONE	= 4,
	REPLACE_RENAME	= 5
};

static void CenterPopup(HWND hwnd, HWND hwndParent)
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
		y = rc.top + ((cyParent - cy) / 2);

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

/*
 * modal dialog box
 */
int DlgModal(HWND hwnd, DLGPROC lpFn, int nID, void* pvParam)
{
	int nRes;
	HRSRC hrsrc;

	nRes = 0;
	hrsrc = FindResource(hUzInstance, MAKEINTRESOURCE(nID), RT_DIALOG);
	if (hrsrc) {
		HGLOBAL hmem;
		hmem = LoadResource(hUzInstance, hrsrc);
		if (hmem)
			nRes = (int)DialogBoxIndirectParam(hUzInstance, (DLGTEMPLATE*)hmem, hwnd, lpFn, (LPARAM)pvParam);
	}
	return nRes;
}

/*
 * modeless dialog box
 */
HWND DlgModeless(HWND hwnd, DLGPROC lpFn, int nID, void* pvParam)
{
	HRSRC hrsrc = FindResource(hUzInstance, MAKEINTRESOURCE(nID), RT_DIALOG);
	if (hrsrc) {
		HGLOBAL hmem = LoadResource(hUzInstance, hrsrc);
		if (hmem) {
			HWND hwndModeless = CreateDialogIndirectParam(hUzInstance, (LPDLGTEMPLATE)hmem, hwnd, lpFn, (LPARAM)pvParam);
			FreeResource(hmem);
			return hwndModeless;
		}
	}
	return NULL;
}

/*
 * password required
 */
BOOL WINAPI PasswordDlgFn(HWND hdlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
	PASSENTER* ppe;
  HWND hctl;
	switch (msg) {
	case WM_INITDIALOG:
		if (gbTryLastPassword) {
			hctl = GetDlgItem(hdlg, IDC_EDIT);
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
			if (hctl) {
				SetWindowFont(hctl, GetWindowFont(hdlg), FALSE);
			}
		}

		MessageBeep(0);
		CenterPopup(hdlg, GetParent(hdlg));

		SetWindowLongPtr(hdlg, DWLP_USER, lParam);
		ppe = (PASSENTER*)lParam;
		if (ppe->pszFile)
			SetDlgItemText(hdlg, IDC_TEXT, ppe->pszFile);

		hctl = GetDlgItem(hdlg, IDC_EDIT);
		if (ppe->pszCode) {
			Edit_SetText(hctl, ppe->pszCode);
			Edit_SetSel(hctl, 0, -1);
		}

		SetForegroundWindow(hdlg);
		SetFocus(hctl);
		return FALSE;

	case WM_COMMAND:
		switch (LOWORD(wParam)) {
		case IDCANCEL:
			EndDialog(hdlg, IDCANCEL);
			return TRUE;

		case IDOK:
		{
			hctl = GetDlgItem(hdlg, IDC_EDIT);
			ppe = (PASSENTER*)GetWindowLongPtr(hdlg, DWLP_USER);
			if (!Edit_GetText(hctl, ppe->pszCode, MAX_PATH)) {
				MessageBeep(0);
				SetFocus(hctl);
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

/*
 * extracting progress meter
 */
BOOL WINAPI ProgressDlgFn(HWND hdlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
	PROGRESSINFO* pi;
	switch (msg) {
	case WM_INITDIALOG:
		CenterPopup(hdlg, GetParent(hdlg));

		pi = (PROGRESSINFO*)malloc(sizeof(PROGRESSINFO));
		if (pi) {
			memset(pi, 0, sizeof(PROGRESSINFO));
			SetWindowLongPtr(hdlg, DWLP_USER, (LONG_PTR)pi);
		}
		SendDlgItemMessage(hdlg, IDC_METER, PBM_SETRANGE, 0, MAKELPARAM(0, TICKS));
		return TRUE;

	case WM_COMMAND:
		switch (LOWORD(wParam)) {
		case IDCANCEL:
			pi = (PROGRESSINFO*)GetWindowLongPtr(hdlg, DWLP_USER);
      if (pi) {
        pi->bCanceled = TRUE;
				ShowWindow(hdlg, SW_HIDE);
			}
			return TRUE;

		default:
			break;
		}
		break;

	case WM_DESTROY:
		pi = (PROGRESSINFO*)GetWindowLongPtr(hdlg, DWLP_USER);
		if (pi)
			free(pi);
		break;

	default:
		break;
	}
	return FALSE;
}

/*
 * replace file dialog box
 */
BOOL WINAPI ReplaceDlgFn(HWND hdlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
	REPLACEINFO* pri;
	switch (msg) {
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
		switch (LOWORD(wParam)) {
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
