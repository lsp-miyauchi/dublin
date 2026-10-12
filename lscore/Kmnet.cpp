#include "stdafx.h"
#include <lm.h>
#include "resource.h"
#include "lscore.h"
#include "fnc.h"

///////////////////////////////////////////////////////
//

#define MB_STOP			(MB_OK | MB_ICONSTOP)

typedef struct tagNETCONNINFO {
	LPTSTR lpUser;
	LPTSTR lpLocal;
	LPTSTR lpRemote;
	LPTSTR lpProvider;
	DWORD dwConnection;
} NETCONNINFO, *LPNETCONNINFO;

///////////////////////////////////////////////////////
//

extern "C"
BOOL WINAPI lccGetNetworkPath(int nDrive, LPTSTR pszName)
{
	// ネットワークドライブ名の取得
	*pszName = '\0';
	TCHAR buf[32];
	PathBuildRoot(buf, nDrive);
	if (GetDriveType(buf) == DRIVE_REMOTE) {
		DWORD len = MAX_PATH;
		buf[2] = '\0';
		if (WNetGetConnection(buf, pszName, &len) == NO_ERROR)
			return TRUE;
	}
	return FALSE;
}

////////////////////////////////////////////////////////////////////////
// 以下、未使用

#if 0

extern "C"
BOOL WINAPI lccLogonNetwork(HWND hwndOwner, LPTSTR lpLocal, LPTSTR lpRemote, DWORD dwConn)
{
	// このAPIは実際には使われていない

	if (PathIsUNC(lpRemote)) return TRUE;

	CWaitCursor wc;
	NETRESOURCE nr;
	memset(&nr, 0, sizeof(nr));
	nr.dwType = RESOURCETYPE_DISK;
	nr.lpLocalName = lpLocal;
	nr.lpRemoteName = lpRemote;

	DWORD dw = WNetAddConnection2(&nr, NULL, NULL, dwConn);

	if (dw == NO_ERROR || dw == ERROR_ALREADY_ASSIGNED)
		return TRUE;
	else if (dw == ERROR_INVALID_PASSWORD || dw == ERROR_LOGON_FAILURE) {
		NETCONNINFO nci;
		nci.lpUser = NULL;
		nci.lpLocal = lpLocal;
		nci.lpRemote = lpRemote;
		nci.lpProvider = NULL;
		nci.dwConnection = dwConn;
		if (DialogBoxParam(RESINST, _MKIR(IDD_NEEDPASS), hwndOwner, PasswdDlgFn, (LPARAM)&nci) == IDOK)
			return TRUE;
	}

	SetLastError(dw);
	return FALSE;
}

BOOL CALLBACK PasswdDlgFn(HWND hdlg, UINT msg, WPARAM wParam, LPARAM lParam);
static TCHAR szProp[] = _T("NetConnInfo");
static BOOL ps_OnInitDialog(HWND hdlg, LPARAM dwParam);
static void ps_OnOK(HWND hdlg);

BOOL CALLBACK PasswdDlgFn(HWND hdlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch (msg) {
		case WM_INITDIALOG:
			return ps_OnInitDialog(hdlg, lParam);

		case WM_COMMAND:
			switch (LOWORD(wParam)) {
				case IDOK:
					ps_OnOK(hdlg);
					return TRUE;

				case IDCANCEL:
					EndDialog(hdlg, IDCANCEL);
					return TRUE;

				default:
					break;
			}
			break;

		case WM_DESTROY:
			RemoveProp(hdlg, szProp);
			break;

		default:
			break;

	}
	return FALSE;
}

static BOOL ps_OnInitDialog(HWND hdlg, LPARAM dwParam)
{
	lccCenterWindow(hdlg, GetParent(hdlg));
	NETCONNINFO* pnci = (NETCONNINFO*)dwParam;
	SetDlgItemText(hdlg, IDC_USER, pnci->lpUser);
	SetDlgItemText(hdlg, IDC_PATH, pnci->lpRemote);
	SetProp(hdlg, szProp, (HANDLE)pnci);

	return TRUE;
}

static void ps_OnOK(HWND hdlg)
{
	CWindow dlg = hdlg;

	TCHAR path[MAX_PATH], user[MAX_PATH], password[MAX_PATH];
	NETCONNINFO* pnci = (NETCONNINFO*)dlg.GetProp(szProp);

	lstrcpy(path, pnci->lpRemote);
	PathRemoveBackslash(path);

	CString user;
	if (dlg.GetDlgItemText(IDC_USER, user) < 1)
		user.Empty();
	if (dlg.GetDlgItemText(IDC_PASSWORD, password) < 1)
		password.Empty();

	NETRESOURCE nr;
	memset(&nr, 0, sizeof(nr));
	nr.dwType = RESOURCETYPE_DISK;
	nr.lpLocalName = pnci->lpLocal;
	nr.lpRemoteName = (LPTSTR)(LPCTSTR)path;
	nr.lpProvider = pnci->lpProvider;

	HCURSOR hcur = SetCursor(LoadCursor(NULL, IDC_WAIT));
	DWORD dw = WNetAddConnection2(&nr, password, user, pnci->dwConnection);
	SetCursor(hcur);

	if (dw == NO_ERROR)
		dlg.EndDialog(IDOK);
	else {
		user.LoadString(IDS_CONNECT_NETWORK);
		lccErrorBox(GetFocus(), dw, user, path, MB_STOP);
	}
}

extern "C"
BOOL WINAPI lccRestoreConnection(int nDrive)
{
	// このAPIは実際には使われていない

	BOOL bOK = FALSE;
	TCHAR path[MAX_PATH], user[MAX_PATH];

	CString str;
	str.Format(_T("%c:"), nDrive + 'A');

	LPTSTR lpRemote = NULL, lpUser = NULL;
	DWORD dwl = MAX_PATH;
	WNetGetConnection(buf, path, &dwl);

	dwl = MAX_PATH;
	WNetGetUser(str, user, &dwl);
	lpUser = (lstrlen(user) < 1) ? NULL : user;

	NETRESOURCE nr;
	memset(&nr, 0, sizeof(nr));
	nr.dwType = RESOURCETYPE_DISK;
	nr.lpLocalName = (LPTSTR)(LPCTSTR)str;
	nr.lpRemoteName = path;

	HCURSOR hcur = SetCursor(LoadCursor(NULL, IDC_WAIT));
	DWORD dw = WNetAddConnection2(&nr, NULL, lpUser, 0);
	SetCursor(hcur);

	if (dw == NO_ERROR || dw == ERROR_ALREADY_ASSIGNED)
		bOK = TRUE;
	else if (dw == ERROR_INVALID_PASSWORD || dw == ERROR_LOGON_FAILURE) {
		NETCONNINFO nci;
		nci.lpUser = user;
		nci.lpLocal = (LPTSTR)(LPCTSTR)str;
		nci.lpRemote = path;
		nci.dwConnection = CONNECT_UPDATE_PROFILE;
		if (DialogBoxParam(RESINST, MAKEINTRESOURCE(IDD_NEEDPASS), GetActiveWindow(), PasswdDlgFn, (LPARAM)&nci) == IDOK)
			bOK = TRUE;
	}
	else {
		str.LoadString(IDS_CONNECT_NETWORK);
		lccErrorBox(GetFocus(), dw, str, lpRemote, MB_STOP);
	}
	return bOK;
}

static int TopOfFreeDrive()
{
	register DWORD dwDrives = GetLogicalDrives();
	for (int i = 0; i < 26; i++) {
		if (i > 2 && !(dwDrives & 1))
			return i;
		dwDrives = dwDrives >> 1;
	}
	return -1;
}

extern "C"
int WINAPI lccAssignDrive(HWND hwnd, LPTSTR lpLocal, LPTSTR lpRemote, BOOL fProf)
{
	if (!lpRemote || lstrlen(lpRemote) == 0)
		return -1;

	int nDrive = lccGetLocalDrive(lpRemote);
	if (nDrive >= 0)
		return nDrive;

	TCHAR buf[MAX_PATH];
	if (!lpLocal || lstrlen(lpLocal) == 0)
	{
		if ((nDrive = TopOfFreeDrive()) < 0)
			return -1;
	}
	else
		nDrive = (int)(ToUpper(lpLocal[0]) - 'A');
	wsprintf(buf, _T("%c:"), nDrive + 'A');

	NETRESOURCE nr;
	nr.dwType = RESOURCETYPE_DISK;
	nr.lpLocalName = buf;
	nr.lpRemoteName = lpRemote;
	nr.lpProvider = NULL;

	DWORD dwRes = WNetAddConnection2(&nr,
									NULL,
									NULL,
									fProf ? CONNECT_UPDATE_PROFILE : 0);
	if (dwRes == NO_ERROR)
		return nDrive;
	return -1;
}

extern "C"
int WINAPI lccGetLocalDrive(LPTSTR lpRemote)
{
	if (!lpRemote || lstrlen(lpRemote) == 0)
		return -1;

	TCHAR buf[16], szRemote[MAX_PATH];
	DWORD dwDrives = GetLogicalDrives();
	for (int i = 0; i < 26; i++)
	{
		wsprintf(buf, _T("%c:\\"), i + 'A');
		if (dwDrives & 1 && GetDriveType(buf) == DRIVE_REMOTE)
		{
			buf[2] = '\0';
			DWORD dw = MAX_PATH;
			if (WNetGetConnection(buf, szRemote, &dw) == NO_ERROR)
			{
				if (!lstrcmpi(lpRemote, szRemote))
					return i;
			}
		}
		dwDrives = dwDrives >> 1;
	}
	return -1;
}

extern "C"
BOOL WINAPI lccGetDriveDisplayName(int nDrive, LPTSTR pszName)
{
	TCHAR buf[MAX_PATH * 2];
	UINT nType = GetDriveType(PathBuildRoot(buf, nDrive));
	if (nType == DRIVE_REMOTE)
	{
		TCHAR root[16];
		root[0] = (TCHAR)(nDrive + 'A');
		root[1] = ':';
		root[2] = '\0';

		DWORD len = _COUNTOF(buf);
		buf[0] = '\0';
		if (WNetGetConnection(root, buf, &len) == NO_ERROR)
		{
			// サーバ名の探索
			LPTSTR p1 = buf;
			while (*p1 == '\\')
				p1++;

			// シェア名の探索
			LPTSTR p2 = StrChr(p1, '\\');
			if (p2)
			{
				*p2++ = '\0';
				LPTSTR p3 = StrChr(p2, '\\');
				if (p3)
					*p3 = '\0';

				if (!IsDBCSLeadByte((BYTE)p1[0]))
				{
					CharLower(p1);
					CharUpperBuff(p1, 1);
				}
				if (!IsDBCSLeadByte((BYTE)p2[0]))
				{
					CharLower(p2);
					CharUpperBuff(p2, 1);
				}
				wsprintf(pszName, _T("'%s' の %s (%c:)"), p1, p2, nDrive + 'A');
			}
			else
				wsprintf(pszName, _T("'%s' (%c:)"), p1, nDrive + 'A');
		}
		else
			wsprintf(pszName, _T(" (%c:)"), nDrive + 'A');
		return TRUE;
	}

	if (nType == DRIVE_CDROM)
	{
		TCHAR vol[MAX_PATH];
		DWORD len, flg;
		if (GetVolumeInformation(buf, vol, MAX_PATH, NULL, &len, &flg, NULL, 0) &&
			lstrlen(vol) > 0 &&
			!IsDBCSLeadByte((BYTE)vol[0]))
		{
			CharLower(vol);
			CharUpperBuff(vol, 1);
			wsprintf(pszName, _T("%s (%c:)"), vol, nDrive + 'A');
		}
		else
			wsprintf(pszName, _T("(%c:)"), nDrive + 'A');
	}
	else
	{
		SHFILEINFO sfi;
		SHGetFileInfo(buf, 0, &sfi, sizeof(sfi), SHGFI_DISPLAYNAME);
		lstrcpy(pszName, sfi.szDisplayName);
	}
	return TRUE;
}

static BOOL __fastcall get_uncservershare(LPCTSTR pPath, LPTSTR pShare)
{
	if (pPath && *pPath == '\\' && *(pPath + 1) == '\\')
	{
		lstrcpy(pShare, pPath);
		register LPTSTR p = pShare;

		p = StrChr(p + 2, '\\');
		if (p && *(p + 1) != '\0')
		{
			p = StrChr(p + 1, '\\');
			if (p)
				*p = '\0';
			return TRUE;
		}
	}
	return FALSE;
}

extern "C"
BOOL WINAPI lccGetSharedPointDisplayName(LPCTSTR pszSharedPoint, LPTSTR pszName)
{
	TCHAR buf[MAX_PATH];
	if (get_uncservershare(pszSharedPoint, buf))
	{
		LPTSTR p1 = buf + 2;
		LPTSTR p2 = StrChr(p1, '\\');
		*p2++ = '\0';

		if (!IsDBCSLeadByte((BYTE)p1[0]))
		{
			CharLower(p1);
			CharUpperBuff(p1, 1);
		}
		if (!IsDBCSLeadByte((BYTE)p2[0]))
		{
			CharLower(p2);
			CharUpperBuff(p2, 1);
		}
		wsprintf(pszName, _T("'%s' の %s"), p1, p2);
		return TRUE;
	}
	return FALSE;
}

#endif
