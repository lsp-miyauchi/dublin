// shlnew.cpp:  shellnew support

#include "stdafx.h"
#include "fopstd.h"

/////////////////////////////////////////////////////////////////////
//

static TCHAR sszShellNew[] = _T("ShellNew");


enum {
	SHLNT_ERROR = 0,
	SHLNT_NULLFILE,
	SHLNT_COMMAND,
	SHLNT_DATA,
	SHLNT_FILENAME,
};

/////////////////////////////////////////////////////////////////////////////
// ShellNewDlgFn

class CShellNewDlg : public CDialogImpl<CShellNewDlg>
{
public:
	CShellNewDlg() { m_path.Empty(); }
	~CShellNewDlg() { }

	enum { IDD = IDD_SHELLNEW };

BEGIN_MSG_MAP(CShellNewDlg)
	ON_WM_INITDIALOG()
	ON_BN_CLICKED(IDOK, OnOK)
	ON_BN_CLICKED(IDCANCEL, OnCancel)
END_MSG_MAP()

	BOOL OnInitDialog(LPARAM lParam);
	void OnOK();
	void OnCancel() { EndDialog(IDCANCEL); }

	CString m_path;
};

BOOL CShellNewDlg::OnInitDialog(LPARAM lParam)
{
	CenterWindow();

	TCHAR path[MAX_PATH * 2], name[MAX_PATH * 2];
	lstrcpy(name, PathFindFileName(m_path));
	PathGetParent(m_path, path);
	SetDlgItemText(IDC_PATH, path);
	SetDlgItemText(IDC_EDIT, name);
	return TRUE;
}

void CShellNewDlg::OnOK()
{
	TCHAR path[MAX_PATH * 2], name[MAX_PATH * 2];
	GetDlgItemText(IDC_PATH, path, ARRAYSIZE(path));

	if (GetDlgItemText(IDC_EDIT, name, ARRAYSIZE(name)) > 0 && validate_makefile(name)) {
		PathAppend(path, name);
		if (!_pathfileexists(path)) {
			m_path = path;
			EndDialog(IDOK);
			return;
		}
	}
	MessageBeep(0);
	GetDlgItem(IDC_EDIT).SetFocus();
}

/////////////////////////////////////////////////////////////////////
//

static int extract_resource_string(LPCTSTR name, CString& str)
{
	str.Empty();
	if (*name == _T('@')) {
		TCHAR buf[MAX_PATH * 2];
		lstrcpy(buf, name + 1);
		LPTSTR p = buf;
		LPTSTR q = StrChr(p, _T(','));
		if (q) {
			*q++ = 0;
			int id = abs(_ttoi(q));
			PathUnquoteSpaces(p);
			CString tmp = p;
			ExpandEnvironmentStrings(tmp.GetString(), buf, ARRAYSIZE(buf));
			CDll dll;
			if (dll.LoadLibrary(buf, LOAD_LIBRARY_AS_DATAFILE) && ::LoadString(dll, id, buf, ARRAYSIZE(buf)))
				str = buf;
		}
	}
	else
		str = name;
	return str.GetLength();
}


extern "C"
int WINAPI shnEnumExtension(LPTSTR pszExt)
{
	// .ext[\tmenu-text]\0.ext[\tmenu-text]...\0.ext[\tmenu-text]\0\0
	LPTSTR pw = pszExt;
	*pw = '\0';

	int cnt = 0;
	DWORD i = 0;
	TCHAR ext[MAX_PATH * 2], buf[MAX_PATH * 2], progid[MAX_PATH];
	long len;
	while (RegEnumKey(HKEY_CLASSES_ROOT, i++, ext, ARRAYSIZE(ext)) == ERROR_SUCCESS) {
		if (ext[0] != '.')
			continue;

		if (!StrCmpI(ext, gszLNK) ||
			!StrCmpI(ext, _T(".bfc")))
			continue;

		// ProgIdを取得
		len = ARRAYSIZE(progid);
		if (RegQueryValue(HKEY_CLASSES_ROOT, ext, progid, &len) != ERROR_SUCCESS || lstrlen(progid) < 1)
			continue;

		// ProgIdがキーとして存在しなければ、列挙から除外しなければ
		// ならないようだ
		CRegKey reg;
		if (reg.Open(HKEY_CLASSES_ROOT, progid, KEY_READ) != ERROR_SUCCESS)
			continue;
		reg.Close();

		// まず ".ext\progid\ShellNew" をチェック
		key_combine(buf, ext, progid, sszShellNew, NULL);
		if (reg.Open(HKEY_CLASSES_ROOT, buf, KEY_READ) != ERROR_SUCCESS) {
			// だめなら ".ext\ShellNew" をチェック
			key_combine(buf, ext, sszShellNew, NULL);
			if (reg.Open(HKEY_CLASSES_ROOT, buf, KEY_READ) != ERROR_SUCCESS)
				continue;
		}

		// 配下にサブキーがあればリストに加える
		DWORD j = 0;
		len = ARRAYSIZE(buf);
		while (RegEnumValue(reg, j++, buf, (DWORD*)&len, 0, 0, 0, 0) == ERROR_SUCCESS) {
			if (len > 0) {
				// メニューテキスト
				len = ARRAYSIZE(buf);
				if (reg.QueryStringValue(_T("MenuText"), buf, (ULONG*)&len) == ERROR_SUCCESS && len > 0) {
					CString str;
					if (extract_resource_string(buf, str) > 0)
						wsprintf(&ext[lstrlen(ext)], _T("\t%s"), str.GetString());
				}
				lstrcpy(pw, ext);
				pw += lstrlen(ext) + 1;
				cnt++;
				break;
			}
		}
		reg.Close();
	}
	*pw = 0;
	return cnt;
}

static BOOL GetExtTypeName(LPCTSTR pszExt, LPTSTR pszTypeName)
{
	*pszTypeName = '\0';

/* @@
	TCHAR buf[MAX_PATH], name[MAX_PATH];
	long lcb = ARRAYSIZE(buf);
	if (RegQueryValue(HKEY_CLASSES_ROOT, pszExt, buf, &lcb) == ERROR_SUCCESS) {
		lcb = ARRAYSIZE(name);
		if (RegQueryValue(HKEY_CLASSES_ROOT, buf, name, &lcb) == ERROR_SUCCESS) {
			if (lstrlen(name) > 0) {
				lstrcpy(pszTypeName, name);
				return TRUE;
			}
		}

		if (lstrlen(buf) > 0) {
			lstrcpy(pszTypeName, buf);
			return TRUE;
		}
	}
@@ */
	TCHAR buf[MAX_PATH];
	DWORD cch = ARRAYSIZE(buf);
	if (AssocQueryString(0, ASSOCSTR_FRIENDLYDOCNAME, pszExt, NULL, buf, &cch) == S_OK) {
		lstrcpy(pszTypeName, buf);
		return TRUE;
	}
	return FALSE;
}

static int get_shellnew_data(CRegKey& reg, ULONG* pnLen, void** ppVal)
{
	*pnLen = 0;
	*ppVal = NULL;

	TCHAR buf[MAX_PATH * 2];
	ULONG len = ARRAYSIZE(buf);
	if (reg.QueryStringValue(_T("NullFile"), buf, &len) == ERROR_SUCCESS)
		return 1;

	// "FileName"
	len = ARRAYSIZE(buf);
	if (reg.QueryStringValue(_T("Filename"), buf, &len) == ERROR_SUCCESS) {
		TCHAR* p = (TCHAR*)malloc((len + 1) * sizeof(TCHAR));
		lstrcpy(p, buf);
		*ppVal = p;
		*pnLen = len;
		return SHLNT_FILENAME;
	}

	// "Data" タイプはバイナリと文字列と２種類ある
	len = 0;
	if (reg.QueryStringValue(_T("Data"), NULL, &len) == ERROR_SUCCESS) {
		len++;
		TCHAR* p = (TCHAR*)malloc(len * sizeof(TCHAR));
		reg.QueryStringValue(_T("Data"), p, &len);
		*ppVal = p;
		*pnLen = lstrlen(p) * sizeof(TCHAR);
		return SHLNT_DATA;
	}
	len = 0;
	if (reg.QueryBinaryValue(_T("Data"), NULL, &len) == ERROR_SUCCESS) {
		void* p = malloc(len);
		reg.QueryBinaryValue(_T("Data"), p, &len);
		*ppVal = p;
		*pnLen = len;
		return SHLNT_DATA;
	}

	// "Command" の場合は、環境変数で構成されているものもある
	len = ARRAYSIZE(buf);
	if (reg.QueryStringValue(_T("Command"), buf, &len) == ERROR_SUCCESS) {
		TCHAR* p = (TCHAR*)malloc(MAX_PATH * 3 * sizeof(TCHAR));
		ExpandEnvironmentStrings(buf, p, MAX_PATH * 3);
		*ppVal = p;
		*pnLen = lstrlen(p);
		return SHLNT_COMMAND;
	}

	return SHLNT_ERROR;
}

static bool getshellnew_name(LPCTSTR ext, CString& str)
{
	str.Empty();

	TCHAR progid[MAX_PATH];
	long len = ARRAYSIZE(progid);
	if (RegQueryValue(HKEY_CLASSES_ROOT, ext, progid, &len) == ERROR_SUCCESS) {
		// まず ".ext\progid\ShellNew" をチェック
		TCHAR buf[MAX_PATH * 2];
		key_combine(buf, ext, progid, sszShellNew, NULL);
		CRegKey reg;
		if (reg.Open(HKEY_CLASSES_ROOT, buf, KEY_READ) != ERROR_SUCCESS) {
			// だめなら ".ext\ShellNew" をチェック
			key_combine(buf, ext, sszShellNew, NULL);
			if (reg.Open(HKEY_CLASSES_ROOT, buf, KEY_READ) != ERROR_SUCCESS)
				return false;
		}

		TCHAR name[MAX_PATH * 2];
		len = ARRAYSIZE(name);
		if (reg.QueryStringValue(_T("ItemName"), name, (ULONG*)&len) == ERROR_SUCCESS && len > 0)
			extract_resource_string(name, str);
		reg.Close();
	}
	return (!str.IsEmpty());
}

extern "C"
BOOL WINAPI shnCreateShellNew(LPCTSTR pszExt, LPCTSTR pszName, LPCTSTR pszDir)
{
	TCHAR buf[MAX_PATH * 2];

	CRegKey reg;
	long lcb = sizeof(buf);
	if (RegQueryValue(HKEY_CLASSES_ROOT, pszExt, buf, &lcb) == ERROR_SUCCESS && lcb > 0) {
		TCHAR tmp[MAX_PATH * 2];
		key_combine(tmp, pszExt, buf, sszShellNew, NULL);
		if (reg.Open(HKEY_CLASSES_ROOT, tmp, KEY_READ) != ERROR_SUCCESS) {
			key_combine(tmp, pszExt, sszShellNew, NULL);
			if (reg.Open(HKEY_CLASSES_ROOT, tmp, KEY_READ) != ERROR_SUCCESS)
				return FALSE;
		}
	}

	ULONG len;
	void* pv;
	int type = get_shellnew_data(reg, &len, &pv);
	if (type == SHLNT_ERROR) {
		reg.Close();
		return FALSE;
	}

	// 作成するドキュメントオブジェクト名の取得
	CString baseName;
	len = ARRAYSIZE(buf);
	if (reg.QueryStringValue(_T("ItemName"), buf, &len) == ERROR_SUCCESS) {
		if (buf[0] == _T('@')) {
			LPTSTR pp = buf + 1;
			LPTSTR pq = StrChr(pp, _T(','));
			if (pq) {
				*pq++ = 0;
				int id = abs(_ttoi(pq));
				PathUnquoteSpaces(pp);
				CString tmp = pp;
				ExpandEnvironmentStrings(tmp.GetString(), buf, ARRAYSIZE(buf));
				CDll dll;
				if (dll.LoadLibrary(buf, LOAD_LIBRARY_AS_DATAFILE) && ::LoadString(dll, id, buf, ARRAYSIZE(buf)))
					baseName = buf;
			}
		}
		else
			baseName = buf;
	}
	reg.Close();

	if (baseName.IsEmpty()) {
		GetExtTypeName(pszExt, buf);
		baseName.Format(_T("新しい%s"), buf);
	}
	baseName += pszExt;

	TCHAR fname[MAX_PATH * 2];
	PathCombine(fname, pszDir, baseName);

	PathSetUnique(fname, 0);

	CShellNewDlg dlg;
	BOOL bOK = FALSE;
	CFileHandle file;
	switch (type) {
		case SHLNT_NULLFILE:	// NullFile
		case SHLNT_DATA:		// Data
			dlg.m_path = fname;
			if (dlg.DoModal() != IDOK)
				break;

			lstrcpy(fname, dlg.m_path);
			file.CreateFile(fname, GENERIC_WRITE, 0, CREATE_NEW, FILE_ATTRIBUTE_NORMAL);
			if (!file.IsInvalid()) {
				bOK = TRUE;
				if (type == SHLNT_DATA && pv && len) {
					DWORD written;
					file.WriteFile(pv, len, &written);
				}
				file.CloseHandle();
				SHChangeNotify(SHCNE_CREATE, SHCNF_PATH, fname, NULL);
			}
			break;

		case SHLNT_COMMAND:		// Command
			if (pv) {
				CString str = (LPCTSTR)pv;
				str.Replace(_T("%1"), pszDir);
				bOK = lccExecApp(NULL, str, pszDir, SW_SHOWNORMAL);
			}
			break;

		case SHLNT_FILENAME:	// Filename
		{
			dlg.m_path = fname;
			if (dlg.DoModal() != IDOK)
				break;

			lstrcpy(fname, dlg.m_path);
			if (pv) {
				BOOL bFound = FALSE;
				// Windowsフォルダ配下のShellNewをチェック
				GetWindowsDirectory(buf, ARRAYSIZE(buf));
				PathAppend(buf, sszShellNew);
				PathAppend(buf, (LPCTSTR)pv);
				bFound = _pathfileexists(buf);
				// CSIDL_TEMPLATES
				if (!bFound && SHGetFolderPath(NULL, CSIDL_TEMPLATES, NULL, SHGFP_TYPE_CURRENT, buf) == S_OK) {
					PathAppend(buf, (LPCTSTR)pv);
					bFound = _pathfileexists(buf);
				}
				// CSIDL_COMMON_TEMPLATES
				if (!bFound && SHGetFolderPath(NULL, CSIDL_COMMON_TEMPLATES, NULL, SHGFP_TYPE_CURRENT, buf) == S_OK) {
					PathAppend(buf, (LPCTSTR)pv);
					bFound = _pathfileexists(buf);
				}
				if (bFound) {
					bOK = (CopyFileSilent(buf, fname) == NOERROR);
				}
				else {
					// サイズ０の空ファイルを作成
					file.CreateFile(fname, GENERIC_WRITE, 0, CREATE_NEW, FILE_ATTRIBUTE_ARCHIVE);
					bOK = !file.IsInvalid();
					if (bOK) {
						file.CloseHandle();
						SHChangeNotify(SHCNE_CREATE, SHCNF_PATH, fname, NULL);
					}
				}
			}
			break;
		}

		default:
			break;
	}

	if (pv)
		free(pv);
	return bOK;
}

extern "C"
BOOL WINAPI shnCreateNewFile(LPCTSTR pszExt, LPCTSTR pszDir)
{
	TCHAR name[MAX_PATH];

	CString baseName;
	if (!getshellnew_name(pszExt, baseName))
		baseName = _T("新しいファイル");

	// 作成するドキュメントオブジェクト名の取得
	CString str;
	str.Format(_T("%s%s"), baseName.GetString(), pszExt);
	lstrcpy(name, pszDir);
	PathAppend(name, str);
	// ユニークなファイルを探索
	PathSetUnique(name, 0);

	CShellNewDlg dlg;
	dlg.m_path = name;
	if (dlg.DoModal() != IDOK)
		return FALSE;

	CFileHandle file;
	file.CreateFile(dlg.m_path, GENERIC_WRITE, 0, CREATE_NEW, FILE_ATTRIBUTE_NORMAL);
	BOOL bOK = !file.IsInvalid();
	if (bOK)
		SHChangeNotify(SHCNE_CREATE, SHCNF_PATH, dlg.m_path.GetString(), NULL);
	return bOK;
}
