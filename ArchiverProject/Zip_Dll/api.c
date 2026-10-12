/*---------------------------------------------------------------------------

  api.c

  This module supplies a Zip dll engine for use directly from C/C++
  programs.

  The entry points are:

    ZpVer *ZpVersion(void);

  This module is currently only used by the Windows dll, and is not used at
  all by any of the other platforms, although it should be easy enough to
  implement this on most platforms.

  ---------------------------------------------------------------------------*/

#include <direct.h>
#include <ctype.h>
#include "zip.h"
#include "crypt.h"
#include "revision.h"
#include "resource.h"
#include <commctrl.h>
#include <shlwapi.h>
#ifdef USE_ZLIB
	#include "zlib.h"
#endif

extern HINSTANCE ghInst;
extern int  zipmain OF((ZIPINFO*, int, char **));

BOOL WINAPI PasswordDlgFn(HWND hdlg, UINT msg, WPARAM wParam, LPARAM lParam);
BOOL WINAPI MeterDlgFn(HWND hdlg, UINT msg, WPARAM wParam, LPARAM lParam);

typedef struct tagARGS {
	int argc;
	char** argv;
	int maxargc;
} ARGS;

typedef struct tagPASSENTER {
	int mode;
	LPCSTR pszFile;
	LPSTR pszCode;
} PASSENTER;

typedef struct tagMETERINFO {
	DWORD dwTotal;
	DWORD dwDone;
	BOOL bCanceled;
} METERINFO, *LPMETERINFO;

#define TICKS		100

//////////////////////////////////////////////////////////////////////
//

void replacechar(LPSTR pszText, int nSrcChar, int nDstChar)
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

static int arginit(ARGS* pag)
{
	// malloc additional 256 to allow for additional command line arguments
	pag->argc = 0;
	if ((pag->argv = (char**)malloc(256 * sizeof(char*))) == NULL)
		return ZE_MEM;
	pag->maxargc = 256;
	return ZE_OK;
}

static void argfree(ARGS* pag)
{
	register int i;
	for (i = 0; i < pag->argc; i++)
		free(pag->argv[i]);
	pag->argc = 0;
	free(pag->argv);
}

static int argalloc(ARGS* pag, char* cmd)
{
	if (pag->maxargc < pag->argc + 1) {
		char** ppw = (char**)realloc(pag->argv, sizeof(char*) * (pag->maxargc + 256));
		if (!ppw) {
			argfree(pag);
			fprintf(stdout, "Unable to allocate memory\n");
			return ZE_MEM;
		}
		pag->argv = ppw;
		pag->maxargc += 256;
	}

	if ((pag->argv[pag->argc] = _strdup(cmd)) == NULL) {
		argfree(pag);
		fprintf(stdout, "Unable to allocate memory\n");
		return ZE_MEM;
	}
	pag->argc++;
	pag->argv[pag->argc] = NULL;
	return ZE_OK;
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
		} else {
			q = (char*)_mbschr((BYTE*)(p + 1), ' ');
			if (q)
				*q++ = '\0';
		}
		argv[i++] = p;
		p = SkipTabSpace(q);
	}
	return i;
}


//////////////////////////////////////////////////////////////////////
//

int WINAPI lzpReport(void* pInfo, int mode, int flag, LPCSTR pszZip, LPCSTR pszName, DWORD size)
{
	ZCL* pzcl = (ZCL*)pInfo;
	if (IsWindow(pzcl->hMeter)) {
		MSG	msg;
		HWND hctl;
		METERINFO* pmi = (METERINFO*)GetWindowLongPtr(pzcl->hMeter, DWLP_USER);
		if (pmi->bCanceled)
			return 1;

		hctl = GetDlgItem(pzcl->hMeter, IDC_METER);
		if (mode == ZP_ST_START_COMPRESS) {
			PathSetDlgItemPath(pzcl->hMeter, IDC_FROM, pszZip);
			PathSetDlgItemPath(pzcl->hMeter, IDC_FILE, pszName);
			pzcl->dlTotal = size;
			pzcl->dlDone = 0;
			SendMessage(hctl, PBM_SETRANGE32, 0, (int)size);
			SendMessage(hctl, PBM_SETPOS, 0, 0);
			UpdateWindow(hctl);
		} else if (mode == ZP_ST_IN_PROGRESS) {
			int per;
			pzcl->dlDone += size;
//			per= percent(pzcl->dlTotal, pzcl->dlDone);
//			SendMessage(hctl, PBM_SETPOS, per, 0);
			SendMessage(hctl, PBM_DELTAPOS, size, 0L);
			UpdateWindow(hctl);
		}

		while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) {
			if (!IsDialogMessage(pzcl->hMeter, &msg)) {
				TranslateMessage(&msg);
				DispatchMessage(&msg);
			}
		}
	}
	return ZP_ST_CONTINUE;
}

int WINAPI lzpPrint(void* pInfo, char* buf, unsigned long size)
{
	ZCL* pzcl = (ZCL*)pInfo;
	if (pzcl->nResultLength > (int)size) {
		lstrcpy(pzcl->pResult, buf);
		pzcl->pResult += size;
		pzcl->nResultLength -= size;
	}
	return (int)size;
}

char* WINAPI lzpComment(char* szBuf)
{
	szBuf[0] = '\0';
	return szBuf;
}

/////////////////////////////////////////////////////////////////////
// Add, update, freshen, or delete zip entries in a zip file.
// See the command help in help() zip.c */

static BOOL PerseResponseFile(ARGS* pag, LPCSTR pszResp)
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
				if (argalloc(pag, argv[i]) != ZE_OK)
					return FALSE;
			}
		}
	}
	fclose(fp);
	return TRUE;
}

typedef struct tagZQ {
	ZIPINFO zi;
	ZCL zcl;
} ZQ;

BOOL WINAPI DllGetFunctionPrefix(LPSTR pszPrefix, LONG cchMax)
{
	if (pszPrefix)
	{
		lstrcpyn(pszPrefix, "Zip", cchMax);
		return TRUE;
	}
	return FALSE;
}

BOOL WINAPI ZipLocked(DWORD dw)
{
	gdwLocked = dw;
	return (gdwLocked != 0);
}

int WINAPI Zip(HWND hwnd, LPCSTR pszCmd, LPSTR pszResult, DWORD dwResultLength)
{
	int i, j, m, result;
	char *p, *cmd, *argv[256], olddir[MAX_PATH * 2];
	ARGS ag;
	ZQ* pzq;
	BOOL bProgress;

	if (!gdwLocked)
		return -1;

	pzq = (ZQ*)GlobalAllocPtr(GHND, sizeof(ZQ));
	if (!pzq)
		return -1;

	memset(pzq, 0, sizeof(ZQ));
	pzq->zi.pzcl = &pzq->zcl;
	pzq->zcl.hwndParent = hwnd;
	pzq->zcl.pResult = pszResult;
	pzq->zcl.nResultLength = dwResultLength;
	pzq->zcl.pfnReport = lzpReport;
	pzq->zcl.pfnPrint = lzpPrint;
	pzq->zcl.pfnComment = lzpComment;

	cmd = _strdup(pszCmd);
	m = SeparateString(cmd, argv, 256);
	if (m < 1) {
		GlobalFreePtr(pzq);
		free(cmd);
		return -1;
	}

	// malloc additional 26 to allow for additional command line arguments
	if (arginit(&ag) != ZE_OK) {
		GlobalFreePtr(pzq);
		free(cmd);
		return -1;
	}

	if (argalloc(&ag, "~.exe") != ZE_OK) {
		free(cmd);
		return -1;
	}

	bProgress = TRUE;
	for (i = 0; i < m; i++) {
		p = argv[i];
		if (*p == '@') {				// ファイルリスト
			if (!PerseResponseFile(&ag, ++p)) {
				GlobalFreePtr(pzq);
				free(cmd);
				return -1;
			}
		} else {
			if (*p != '-') {
				j = lstrlen(p) - 1;
				if (p[j] == '\\' && _mbsbtype((BYTE*)p, j) == _MBC_SINGLE) {
					// 基準フォルダ
					pzq->zcl.pszBaseDir = p;
					SetCurrentDirectory(p);
					continue;
				}

				if (!pzq->zcl.pszZipName)
					pzq->zcl.pszBaseDir = p;
			} else if (*(p + 1) == '-' && *(p + 2) == 'i') {
				bProgress = FALSE;
				continue;
			}

			if (argalloc(&ag, p) != ZE_OK) {
				GlobalFreePtr(pzq);
				free(cmd);
				return -1;
			}
		}
	}

	if (bProgress) {
		pzq->zcl.hMeter = DlgModeless(hwnd, (DLGPROC)MeterDlgFn, IDD_RELAY, 0);
		if (pzq->zcl.hMeter) {
			ShowWindow(pzq->zcl.hMeter, SW_SHOW);
			UpdateWindow(pzq->zcl.hMeter);
		}
	}

	GetCurrentDirectory(MAX_PATH * 2, olddir);
	pZipInfo = &pzq->zi;
	result = zipmain(&pzq->zi, ag.argc, ag.argv);
	pZipInfo = NULL;
	SetCurrentDirectory(olddir);

	if (pzq->zcl.hMeter)
		DestroyWindow(pzq->zcl.hMeter);

	argfree(&ag);
	GlobalFreePtr(pzq);
	free(cmd);
	return (result == ZE_OK) ? 0 : -1;
}

#if CRYPT
int encr_passwd(int modeflag, char *pwbuf, int size, const char *zfn)
{
	PASSENTER pe;
	pe.mode = modeflag;
	pe.pszFile = zfn;
	pe.pszCode = pwbuf;

	if (DlgModal(GetFocus(), (DLGPROC)PasswordDlgFn, IDD_PASSWORD, &pe) == IDOK)
		return IZ_PW_ENTERED;
	return 1;
}
#endif /* CRYPT */

void EXPENTRY ZpVersion(ZpVer* p)   /* should be pointer to const struct */
{
    p->structlen = ZPVER_LEN;

    p->flag = 0;
    lstrcpy(p->betalevel, Z_BETALEVEL);
    lstrcpy(p->date, REVDATE);

#ifdef ZLIB_VERSION
    lstrcpy(p->zlib_version, ZLIB_VERSION);
    p->flag |= 2;
#else
    p->zlib_version[0] = '\0';
#endif

    p->zip.major = Z_MAJORVER;
    p->zip.minor = Z_MINORVER;
    p->zip.patchlevel = Z_PATCHLEVEL;


    p->os2dll.major = D2_MAJORVER;
    p->os2dll.minor = D2_MINORVER;
    p->os2dll.patchlevel = D2_PATCHLEVEL;


    p->windll.major = DW_MAJORVER;
    p->windll.minor = DW_MINORVER;
    p->windll.patchlevel = DW_PATCHLEVEL;
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

	if (hwndParent)
	{
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
	else
	{
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
	hrsrc = FindResource(ghInst, MAKEINTRESOURCE(nID), RT_DIALOG);
	if (hrsrc)
	{
		HGLOBAL hmem;
		hmem = LoadResource(ghInst, hrsrc);
		if (hmem)
			nRes = DialogBoxIndirectParam(ghInst, (LPDLGTEMPLATE)hmem,
										hwnd, lpFn, (LPARAM)pvParam);
	}
	return nRes;
}

HWND DlgModeless(HWND hwnd, DLGPROC lpFn, int nID, void* pvParam)
{
	HRSRC hrsrc = FindResource(ghInst, MAKEINTRESOURCE(nID), RT_DIALOG);
	if (hrsrc)
	{
		HGLOBAL hmem = LoadResource(ghInst, hrsrc);
		if (hmem)
		{
			HWND hwndModeless = CreateDialogIndirectParam(
										ghInst, (LPDLGTEMPLATE)hmem,
										hwnd, lpFn, (LPARAM)pvParam);
			FreeResource(hmem);
			return hwndModeless;
		}
	}
	return NULL;
}


BOOL WINAPI PasswordDlgFn(HWND hdlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
	PASSENTER* ppe;
	switch (msg)
	{
		case WM_INITDIALOG:
			CenterPopup(hdlg, GetParent(hdlg));

			SetWindowLongPtr(hdlg, DWLP_USER, lParam);
			ppe = (PASSENTER*)lParam;
			if (ppe->pszFile)
				SetDlgItemText(hdlg, IDC_TEXT, ppe->pszFile);

			if (ppe->mode == ZP_PW_VERIFY)
			{
				char buf[MAX_PATH * 2];
				LoadString(ghInst, IDS_REENTER_PASSWORD, buf, MAX_PATH * 2);
				SetDlgItemText(hdlg, IDC_TITLE, buf);
			}
			SetForegroundWindow(hdlg);
			return TRUE;

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
	switch (msg) {
	case WM_INITDIALOG:
		CenterPopup(hdlg, GetParent(hdlg));
		pmi = (METERINFO*)malloc(sizeof(METERINFO));
		memset(pmi, 0, sizeof(METERINFO));
		SetWindowLongPtr(hdlg, DWLP_USER, (LONG_PTR)pmi);
		SendDlgItemMessage(hdlg, IDC_METER, PBM_SETRANGE, 0, MAKELPARAM(0, TICKS));
		return TRUE;

	case WM_COMMAND:
		switch (LOWORD(wParam)) {
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

