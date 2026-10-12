#include "stdafx.h"
#include "fopstd.h"

///////////////////////////////////////////////////////////////
//

static TCHAR szCmnSet[]      = _T("CommonSetting");
static TCHAR szSecFolder[]   = _T("Folder");
static TCHAR szKeyPersonal[] = _T("Personal");
static TCHAR szKeyAppData[]  = _T("AppData");


///////////////////////////////////////////////////////////////
//

int WINAPIV key_combine(LPTSTR dst, LPCTSTR psz, ... )
{
	LPCTSTR p = psz;
	*dst = 0;
	va_list marker;
	va_start(marker, psz);
	while (p && *p) {
		if (*dst == 0)
			lstrcpy(dst, p);
		else
			PathAppend(dst, p);
		p = va_arg(marker, LPCTSTR);
	}
	va_end(marker);
	return lstrlen(dst);
}

bool GetAppRegSection(CRegKey& reg, LPCTSTR sec)
{
	CString strSec;
	if (IS_INTRESOURCE(sec)) { strSec.LoadString(PtrToUint(sec)); sec = strSec; }

	TCHAR buf[MAX_PATH * 2];
	key_combine(buf, gszRegRoot, gszApp, NULL);

	bool ok = false;
	CRegKey app;
	if (app.Create(HKEY_CURRENT_USER, buf) == ERROR_SUCCESS) {
		if (reg.Create(app, sec) == ERROR_SUCCESS)
			ok = true;
		app.Close();
	}
	return ok;
}

ULONG GetRegText(LPCTSTR sec, LPCTSTR vname, LPTSTR psz, ULONG cch)
{
	if (!psz || cch < 1) return 0;
	*psz = 0;
	ULONG len = cch;
	cch = 0;
	CRegKey reg;
	if (GetAppRegSection(reg, sec)) {
		CString strName;
		if (IS_INTRESOURCE(vname)) { strName.LoadString(PtrToUint(vname)); vname = strName; }

		if (reg.QueryStringValue(vname, psz, &len) == ERROR_SUCCESS)
			cch = lstrlen(psz);
		reg.Close();
	}
	return cch;
}

bool SetRegText(LPCTSTR sec, LPCTSTR vname, LPCTSTR psz)
{
	bool ok = false;
	CRegKey reg;
	if (GetAppRegSection(reg, sec)) {
		CString strName;
		if (IS_INTRESOURCE(vname)) { strName.LoadString(PtrToUint(vname)); vname = strName; }

		if (psz == NULL)
			ok = (reg.DeleteValue(vname) == ERROR_SUCCESS);
		else
			ok = (reg.SetStringValue(vname, psz) == ERROR_SUCCESS);
		reg.Close();
	}
	return ok;
}

int GetRegValue(LPCTSTR sec, LPCTSTR vname, int defval)
{
	DWORD val = (DWORD)defval;
	CRegKey reg;
	if (GetAppRegSection(reg, sec)) {
		CString strName;
		if (IS_INTRESOURCE(vname)) { strName.LoadString(PtrToUint(vname)); vname = strName; }

		if (reg.QueryDWORDValue(vname, val) != ERROR_SUCCESS)
			val = (DWORD)defval;
		reg.Close();
	}
	return (int)val;
}

bool SetRegValue(LPCTSTR sec, LPCTSTR vname, int val)
{
	bool ok = false;
	CRegKey reg;
	if (GetAppRegSection(reg, sec)) {
		CString strName;
		if (IS_INTRESOURCE(vname)) { strName.LoadString(PtrToUint(vname)); vname = strName; }

		ok = (reg.SetDWORDValue(vname, (DWORD)val) == ERROR_SUCCESS);
		reg.Close();
	}
	return ok;
}

bool DelRegValue(LPCTSTR sec, LPCTSTR vname)
{
	bool ok = false;
	CRegKey reg;
	if (GetAppRegSection(reg, sec)) {
		CString strName;
		if (IS_INTRESOURCE(vname)) { strName.LoadString(PtrToUint(vname)); vname = strName; }

		ok = (reg.DeleteValue(vname) == ERROR_SUCCESS);
		reg.Close();
	}
	return ok;
}

int GetRegBinary(LPCTSTR sec, LPCTSTR vname, void* data, int size)
{
	int n = 0;
	CRegKey reg;
	if (GetAppRegSection(reg, sec)) {
		CString strName;
		if (IS_INTRESOURCE(vname)) { strName.LoadString(PtrToUint(vname)); vname = strName; }

		ULONG len = (ULONG)size;
		if (reg.QueryBinaryValue(vname, data, &len) == ERROR_SUCCESS)
			n = (int)len;
		reg.Close();
	}
	return n;
}

/* @@@ ���g�p
void* GetRegBinaryEx(LPCTSTR sec, LPCTSTR vname, int* size)
{
	*size = 0;
	void* data = NULL;
	CRegKey reg;
	if (GetAppRegSection(reg, sec)) {
		CString strName;
		if (IS_INTRESOURCE(vname)) { strName.LoadString(PtrToUint(vname)); vname = strName; }

		ULONG len = 0;
		if (reg.QueryBinaryValue(vname, NULL, &len) == ERROR_SUCCESS) {
			data = malloc(len);
			if (data) {
				if (reg.QueryBinaryValue(vname, data, &len) != ERROR_SUCCESS) {
					free(data);
					data = NULL;
				}
				else
					*size = (int)len;
			}
		}
		reg.Close();
	}
	return data;
}
@@@ */

bool SetRegBinary(LPCTSTR sec, LPCTSTR vname, const void* data, int size)
{
	bool ok = false;
	CRegKey reg;
	if (GetAppRegSection(reg, sec)) {
		CString strName;
		if (IS_INTRESOURCE(vname)) { strName.LoadString(PtrToUint(vname)); vname = strName; }

		if (data == NULL)
			ok = (reg.DeleteValue(vname) == ERROR_SUCCESS);
		else
			ok = (reg.SetBinaryValue(vname, data, (ULONG)size) == ERROR_SUCCESS);
		reg.Close();
	}
	return ok;
}

///////////////////////////////////////////////////////////////
//

static void _build_dr(LPTSTR psz, int drive)
{
	*psz++ = _T('=');
	*psz++ = drive + _T('A');
	*psz++ = _T(':');
	*psz = 0;
}

extern "C"
BOOL WINAPI fopGetDirRemind(int nDrive, LPTSTR pszDir)
{
	if (0 > nDrive || nDrive > 25) return FALSE;

	TCHAR dr[8];
	 _build_dr(dr, nDrive);

	TCHAR buf[MAX_PATH * 2];
	key_combine(buf, gszRegRoot, szCmnSet, szSecFolder, NULL);

	BOOL bOK = FALSE;
	CRegKey reg;
	if (reg.Open(HKEY_CURRENT_USER, buf, KEY_READ) == ERROR_SUCCESS) {
		buf[0] = 0;
		ULONG len = ARRAYSIZE(buf);
		if (reg.QueryStringValue(dr, buf, &len) == ERROR_SUCCESS && len > 1 &&
			nDrive == PathGetDriveNumber(buf)) {
			lstrcpy(pszDir, buf);
			bOK = TRUE;
		}
	}
	return bOK;
}

extern "C"
void WINAPI fopSetDirRemind(int nDrive, LPCTSTR pszDir)
{
	if (nDrive < 0 || 25 < nDrive) return;

	TCHAR buf[MAX_PATH * 2];
	key_combine(buf, gszRegRoot, szCmnSet, szSecFolder, NULL);
	CRegKey reg;
	if (reg.Create(HKEY_CURRENT_USER, buf) == ERROR_SUCCESS) {
		TCHAR dr[8];
		 _build_dr(dr, nDrive);
		if (!pszDir || *pszDir == 0)
			reg.DeleteValue(dr);
		else if (nDrive == PathGetDriveNumber(pszDir))
			reg.SetStringValue(dr, pszDir);
	}
}

/* ---- ���g�p -------------
extern "C"
BOOL WINAPI fopGetAppDataFolder(LPCTSTR pszProduct, LPTSTR pszFolder)
{
	TCHAR buf[MAX_PATH * 2];
	key_combine(buf, gszRegRoot, pszProduct, szSecFolder, NULL);

	BOOL bOK = FALSE;
	CRegKey reg;
	if (reg.Create(HKEY_CURRENT_USER, buf) == ERROR_SUCCESS) {
		ULONG len = ARRAYSIZE(buf);
		if (reg.QueryStringValue(szKeyAppData, buf, &len) != ERROR_SUCCESS || len < 1) {
			SHGetFolderPath(GetFocus(), CSIDL_APPDATA, NULL, SHGFP_TYPE_CURRENT, buf);
			PathCombine(pszFolder, buf, pszProduct);
			if (!_pathisdirectory(pszFolder))
				fopMakeTree(pszFolder);
			reg.SetStringValue(szKeyAppData, pszFolder);
		}
		else
			lstrcpy(pszFolder, buf);
		reg.Close();
		bOK = TRUE;
	}
	return bOK;
}
------------------------ */
