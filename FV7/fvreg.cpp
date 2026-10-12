///////////////////////////////////////////////////////////////////////
// fvreg.cpp

#include "stdafx.h"
#include "resource.h"
#include "fv7.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
//@ #define new DEBUG_NEW
#endif

////////////////////////////////////////////////////////////////
//

static LPTSTR _loadstring(UINT id)
{
	LPTSTR pstr = NULL;
	if (id) {
		for (int len = 256; ; len *= 2) {
			pstr = new TCHAR[len];
			if (pstr == NULL) return NULL;

			int res = ::LoadString(RESINST, id, pstr, len);
			if (res < len - 1) break;

			delete [] pstr;
			pstr = NULL;
		}
	}
	return pstr;
}

class CResStr
{
private:
	LPTSTR m_strAlloc;
public:
	LPCTSTR m_str;

public:
	CResStr(_U_STRINGorID res)
	{
		if (IS_INTRESOURCE(res.m_lpstr)) {
			m_strAlloc = _loadstring(LOWORD(res.m_lpstr));
			m_str = m_strAlloc;
		}
		else {
			m_strAlloc = NULL;
			m_str = res.m_lpstr;
		}
	}

	~CResStr()
	{
		if (m_strAlloc) delete [] m_strAlloc;
	}

	LPCTSTR str() const { return m_str; }
};

class CSecKey
{
private:
	LPTSTR m_secAlloc;
	LPTSTR m_keyAlloc;
public:
	LPCTSTR m_sec;
	LPCTSTR m_key;

public:
	CSecKey(_U_STRINGorID sec, _U_STRINGorID key)
	{
		if (IS_INTRESOURCE(sec.m_lpstr)) {
			m_secAlloc = _loadstring(LOWORD(sec.m_lpstr));
			m_sec = m_secAlloc;
		}
		else {
			m_secAlloc = NULL;
			m_sec = sec.m_lpstr;
		}

		if (IS_INTRESOURCE(key.m_lpstr)) {
			m_keyAlloc = _loadstring(LOWORD(key.m_lpstr));
			m_key = m_keyAlloc;
		}
		else {
			m_keyAlloc = NULL;
			m_key = key.m_lpstr;
		}
	}

	~CSecKey()
	{
		if (m_secAlloc) delete [] m_secAlloc;
		if (m_keyAlloc) delete [] m_keyAlloc;
	}

	LPCTSTR sec() const { return m_sec; }
	LPCTSTR key() const { return m_key; }
};

////////////////////////////////////////////////////////////////
//

void GetAppRootKeyName(CString& str)
{
	str = _T("Software\\LightShip\\");
	str += APP_PRODUCT;
}

bool GetRegApp(CRegKey& app, LPCTSTR pszApp)
{
	if (!pszApp || *pszApp == 0)
		pszApp = APP_PRODUCT;
	CRegKey soft;
	if (soft.Open(HKEY_CURRENT_USER, _T("Software")) == ERROR_SUCCESS) {
		// 通常は Software\LightShip、
		// x64上で32bitを動作させているときは Software\LightShip\x86
		CString root = APP_COMPANY;
/* まだしない
		if (fvApp.IsWow64())
			root += _T("\\x86");
*/
		CRegKey comp;
		if (comp.Create(soft, root) == ERROR_SUCCESS) {
			if (app.Create(comp, pszApp) == ERROR_SUCCESS) {
				return true;
			}
		}
	}
	return false;
}

bool GetRegSection(CRegKey& sec, LPCTSTR pszSection)
{
	CRegKey app;
	if (GetRegApp(app)) {
		if (sec.Create(app, pszSection) == ERROR_SUCCESS) {
			return true;
		}
	}
	return false;
}

//////////////////////////////////////////////////////////////////////
//

void DeleteRegSection(_U_STRINGorID sec, _U_STRINGorID key)
{
	CString strKey;
	RegKeyCombine(sec, key, strKey);
	SHDeleteKey(HKEY_CURRENT_USER, strKey);
}

BOOL RegSectionExists(_U_STRINGorID sec)
{
	BOOL ok = FALSE;
	CResStr s(sec);
	if (s.str()) {
		CRegKey app;
		if (GetRegApp(app)) {
			CRegKey reg;
			ok = (reg.Open(app, s.str(), KEY_READ) == ERROR_SUCCESS);
		}
	}
	return ok;
}

BOOL CreateRegSection(_U_STRINGorID sec)
{
	BOOL ok = FALSE;
	CResStr s(sec);
	if (s.str()) {
		CRegKey app;
		if (GetRegApp(app)) {
			CRegKey reg;
			ok = (reg.Create(app, s.str()) == ERROR_SUCCESS);
		}
	}
	return ok;
}

int GetRegText(_U_STRINGorID sec, _U_STRINGorID key, CString& str)
{
	str.Empty();
	CSecKey z(sec, key);
	if (z.sec() && z.key()) {
		CRegKey reg;
		if (GetRegSection(reg, z.sec())) {
			DWORD type;
			ULONG size;
			// データ長を取得
			if (reg.QueryValue(z.key(), &type, NULL, &size) == ERROR_SUCCESS && type == REG_SZ && size > 0) {
				LPTSTR p = str.GetBuffer((size + 16) * sizeof(TCHAR));
				LSTATUS ok = reg.QueryStringValue(z.key(), p, &size);
				str.ReleaseBuffer();
				if (ok != ERROR_SUCCESS)
					str.Empty();
			}
		}
	}
	return lstrlen(str);
}

BOOL SetRegText(_U_STRINGorID sec, _U_STRINGorID key, LPCTSTR pszText)
{
	BOOL ok = FALSE;
	CSecKey z(sec, key);
	if (z.sec() && z.key()) {
		CRegKey reg;
		if (GetRegSection(reg, z.sec())) {
			if (pszText == NULL)
				ok = (reg.DeleteValue(z.key()) == ERROR_SUCCESS);
			else
				ok = (reg.SetStringValue(z.key(), pszText) == ERROR_SUCCESS);
		}
	}
	return ok;
}

int GetRegTextSeq(_U_STRINGorID sec, _U_STRINGorID key, CString& str, int nEntry)
{
	CResStr s(key);
	if (!s.str()) return 0;

	CString fmt;
	fmt.Format(s.str(), nEntry);
	return GetRegText(sec, (LPCTSTR)fmt, str);
}

BOOL SetRegTextSeq(_U_STRINGorID sec, _U_STRINGorID key, LPCTSTR pszText, int nEntry)
{
	CResStr s(key);
	if (!s.str()) return FALSE;

	TCHAR fmt[128];
	wsprintf(fmt, s.str(), nEntry);
	return SetRegText(sec, fmt, pszText);
}

int GetRegValue(_U_STRINGorID sec, _U_STRINGorID key, int nDefValue)
{
	int val = nDefValue;
	CSecKey z(sec, key);
	if (z.sec() && z.key()) {
		CRegKey reg;
		if (GetRegSection(reg, z.sec())) {
			DWORD dw = 0;
			if (reg.QueryDWORDValue(z.key(), dw) == ERROR_SUCCESS)
				val = dw;
		}
	}
	return val;
}

BOOL SetRegValue(_U_STRINGorID sec, _U_STRINGorID key, int nValue)
{
	BOOL ok = FALSE;
	CSecKey z(sec, key);
	if (z.sec() && z.key()) {
		CRegKey reg;
		if (GetRegSection(reg, z.sec())) {
			ok = (reg.SetDWORDValue(z.key(), (DWORD)nValue) == ERROR_SUCCESS);
		}
	}
	return ok;
}

BOOL DelRegValue(_U_STRINGorID sec, _U_STRINGorID key)
{
	BOOL ok = FALSE;
	CSecKey z(sec, key);
	if (z.sec() && z.key()) {
		CRegKey reg;
		if (GetRegSection(reg, z.sec())) {
			ok = (reg.DeleteValue(z.key()) == ERROR_SUCCESS);
		}
	}
	return ok;
}

int GetRegBinary(_U_STRINGorID sec, _U_STRINGorID key, LPVOID pcbData, int nLen)
{
	int reslen = 0;
	CSecKey z(sec, key);
	if (z.sec() && z.key()) {
		CRegKey reg;
		if (GetRegSection(reg, z.sec())) {
			ULONG len = nLen;
			if (reg.QueryBinaryValue(z.key(), pcbData, &len) == ERROR_SUCCESS) {
				reslen = (int)len;
			}
		}
	}
	return reslen;
}

LPBYTE GetRegBinaryEx(_U_STRINGorID sec, _U_STRINGorID key, LPINT lpnLen)
{
	LPBYTE pd = NULL;
	int len = GetRegBinary(sec, key, NULL, 4);
	if (len > 0) {
		pd = (LPBYTE)malloc(len);
		GetRegBinary(sec, key, pd, len);
	}
	if (lpnLen)
		*lpnLen = len;
	return pd;
}

LPITEMIDLIST GetRegPidl(_U_STRINGorID sec, _U_STRINGorID key)
{
	LPITEMIDLIST pidl = NULL;
	int len;
	BYTE* pd = GetRegBinaryEx(sec, key, &len);
	if (pd) {
		pidl = ILClone((LPITEMIDLIST)pd);
		free(pd);
	}
	return pidl;
}

BOOL SetRegBinary(_U_STRINGorID sec, _U_STRINGorID key, LPVOID pcbData, int nLen)
{
	BOOL ok = FALSE;
	CSecKey z(sec, key);
	if (z.sec() && z.key()) {
		CRegKey reg;
		if (GetRegSection(reg, z.sec())) {
			if (pcbData == NULL || nLen < 1)
				ok = (reg.DeleteValue(z.key()) == ERROR_SUCCESS);
			else
				ok = (reg.SetBinaryValue(z.key(), pcbData, nLen) == ERROR_SUCCESS);
		}
	}
	return ok;
}

int GetRegBinarySeq(_U_STRINGorID sec, _U_STRINGorID key, int nEntry, LPBYTE pd, int nLen)
{
	CResStr s(key);
	if (s.str() && pd && nLen > 0) {
		TCHAR fmt[128];
		wsprintf(fmt, s.str(), nEntry);
		return GetRegBinary(sec, fmt, pd, nLen);
	}
	return 0;
}

LPBYTE GetRegBinarySeqEx(_U_STRINGorID sec, _U_STRINGorID key, int nEntry, LPINT pnLen)
{
	CResStr s(key);
	if (s.str()) {
		TCHAR k[MAX_PATH];
		wsprintf(k, s.str(), nEntry);
		return GetRegBinaryEx(sec, k, pnLen);
	}
	return NULL;
}

BOOL SetRegBinarySeq(_U_STRINGorID sec, _U_STRINGorID key, int nEntry, void* pd, int nLen)
{
	CResStr s(key);
	if (s.str()) {
		TCHAR fmt[MAX_PATH];
		wsprintf(fmt, s.str(), nEntry);
		return SetRegBinary(sec, fmt, pd, nLen);
	}
	return FALSE;
}

IStream* RegOpenStream(_U_STRINGorID sec, _U_STRINGorID key, DWORD grfMode)
{
	IStream* pStrm = NULL;
	CSecKey z(sec, key);
	if (z.sec() && z.key()) {
		CRegKey app;
		if (GetRegApp(app)) {
			pStrm = OpenRegStream(app, z.sec(), z.key(), grfMode);
		}
	}
	return pStrm;
}

int RegLoadHistory(CSimpleArray<CString>& rArray, _U_STRINGorID sec, _U_STRINGorID key, int maxEntry)
{
	CString strKey;
	RegKeyCombine(sec, key, strKey);

	int numItems = 0;
	CMRUProc mru;
	if (mru.Create(HKEY_CURRENT_USER, strKey, maxEntry)) {
		int n = mru.GetCount();
		TCHAR buf[MAX_PATH * 2];
		for (int i = 0; i < n; i++) {
			if (mru.Enum(i, buf, ARRAYSIZE(buf)) > 0) {
				rArray.Add(buf);
				numItems++;
			}
		}
		mru.Close();
	}
	return numItems;
}

void RegLoadHistoryEx(HWND hCombo, _U_STRINGorID sec, _U_STRINGorID key, int maxEntry)
{
	CComboBox combo = hCombo;
	if (!combo.IsWindow()) return;

	CSimpleArray<CString> array;
	int cnt = RegLoadHistory(array, sec, key, maxEntry);
	for (int i = 0; i < cnt; i++)
		combo.AddString(array[i]);
}

BOOL RegSaveHistory(_U_STRINGorID sec, _U_STRINGorID key, int maxEntry, LPCTSTR pszString)
{
	CString strKey;
	RegKeyCombine(sec, key, strKey);

	CMRUProc mru;
	if (mru.Create(HKEY_CURRENT_USER, strKey, maxEntry)) {
		mru.Add(pszString);
		mru.Close();
	}
	return TRUE;
}

void RegClearHistoryEx(HWND hCombo, _U_STRINGorID sec, _U_STRINGorID key)
{
	CComboBox combo = hCombo;
	if (combo.IsWindow())
		combo.ResetContent();

	DeleteRegSection(sec, key);
}

int EnumRegValue(HKEY hkey, CSimpleArray<CString>& arr)
{
	int numValue = 0;
	DWORD len, type;
	TCHAR buf[MAX_PATH];
	for (DWORD dw = 0; ; dw++) {
		len = ARRAYSIZE(buf);
		if (RegEnumValue(hkey, dw, buf, &len, NULL, &type, NULL, NULL) == ERROR_NO_MORE_ITEMS)
			break;
		arr.Add(buf);
		numValue++;
	}
	return numValue;
}

int GetRegEnumValue(_U_STRINGorID sec, CSimpleArray<CString>& arr)
{
	int numValue = 0;
	CResStr s(sec);
	if (s.str()) {
		CRegKey reg;
		if (GetRegSection(reg, s.str()))
			numValue = EnumRegValue(reg, arr);
	}
	return numValue;
}

BOOL __fastcall _pathisdirectory(LPCTSTR pszName);

static BOOL __AppKnownFolder(const KNOWNFOLDERID kfid, CString& strFolder, LPCTSTR pszAppend)
{
	CMemTask path;
	SHGetKnownFolderPath(kfid, 0, NULL, (PWSTR*)&path);
	TCHAR buf[MAX_PATH * 4];
	PathCombine(buf, path, APP_COMPANY);
	PathAppend(buf, APP_PRODUCT);
	if (pszAppend && *pszAppend)
		PathAppend(buf, pszAppend);

	// フォルダ作成
	TCHAR dir[MAX_PATH * 2];
	PathGetParent(buf, dir);
	if (!_pathisdirectory(dir) && SHCreateDirectoryEx(NULL, dir, NULL) != ERROR_SUCCESS)
		return FALSE;

	strFolder = buf;
	return TRUE;
}

BOOL GetAppDataFolder(CString& strFolder, LPCTSTR pszAppend)
{
	return __AppKnownFolder(FOLDERID_RoamingAppData, strFolder, pszAppend);
}

BOOL GetLocalAppDataFolder(CString& strFolder, LPCTSTR pszAppend)
{
	return __AppKnownFolder(FOLDERID_LocalAppData, strFolder, pszAppend);
}

void RegLoadWindowPlacement(HWND hwnd, UINT idResource)
{
	CString str;
	str.LoadString(idResource);
	if (str.IsEmpty())
		str.Format(_T("%d"), idResource);

	WINDOWPLACEMENT wpm;
	if (GetRegBinary(IDS_SEC_SETTINGS, (LPCTSTR)str, &wpm, sizeof(wpm)) == sizeof(wpm)) {
		lccMapRectToProgmanPane(&wpm.rcNormalPosition);
		wpm.showCmd = SW_SHOWNORMAL;
		::SetWindowPlacement(hwnd, &wpm);
	}
}

void RegSaveWindowPlacement(HWND hwnd, UINT idResource)
{
	CString str;
	str.LoadString(idResource);
	if (str.IsEmpty())
		str.Format(_T("%d"), idResource);

	WINDOWPLACEMENT wpm;
	wpm.length = sizeof(wpm);
	::GetWindowPlacement(hwnd, &wpm);
	lccUnmapRectFromProgmanPane(&wpm.rcNormalPosition);
	SetRegBinary(IDS_SEC_SETTINGS, (LPCTSTR)str, (BYTE*)&wpm, sizeof(wpm));
}

int RegKeyCombine(_U_STRINGorID sec, _U_STRINGorID key, CString& strKey)
{
	CSecKey z(sec, key);
	if (!z.sec()) return -1;

	CString strApp;
	GetAppRootKeyName(strApp);

	strKey.Format(_T("%s\\%s"), (LPCTSTR)strApp, z.sec());
	if (z.key()) {
		strKey += _T('\\');
		strKey += z.key();
	}
	return strKey.GetLength();
}

static const TCHAR sszRegGuidOperation[] = _T("GuidOperation");

BOOL RegSaveGuidOperation(const void* dataptr, int len, LPTSTR guidstr)
{
	if (!dataptr || len < 1 || !guidstr) return FALSE;

	BOOL bOK = FALSE;
	CRegKey app;
	if (GetRegApp(app)) {
		CRegKey sec;
		if (sec.Create(app, sszRegGuidOperation, REG_NONE, REG_OPTION_VOLATILE, KEY_WRITE) == ERROR_SUCCESS) {
			GUID g;
			if (CoCreateGuid(&g) == S_OK) {
				OLECHAR oz[128];
				StringFromGUID2(g, oz, ARRAYSIZE(oz));
				sec.SetBinaryValue(oz, dataptr, (ULONG)len);
				lstrcpy(guidstr, oz);
				bOK = TRUE;
			}
			sec.Close();
		}
		app.Close();
	}
	return bOK;
}

void* RegLoadGuidOperation(LPCTSTR guidstr, int* lenptr)
{
	if (!guidstr || !lenptr) return NULL;

	*lenptr = 0;
	void* dataptr = NULL;
	CRegKey app;
	if (GetRegApp(app)) {
		CRegKey sec;
		if (sec.Open(app, sszRegGuidOperation, KEY_READ) == ERROR_SUCCESS) {
			ULONG len = 0;
			if (sec.QueryBinaryValue(guidstr, NULL, &len) == ERROR_SUCCESS) {
				dataptr = malloc(len);
				if (dataptr) {
					if (sec.QueryBinaryValue(guidstr, dataptr, &len) == ERROR_SUCCESS)
						*lenptr = (int)len;
				}
				sec.DeleteValue(guidstr);
			}
			sec.Close();
		}
		app.Close();
	}
	return dataptr;
}
