#include "stdafx.h"
#include "sys\stat.h"
#include "fv7.h"
#include "unlha32.h"
#include "tiphelp.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
//@ #define new DEBUG_NEW
#endif

///////////////////////////////////////////////////////////////////////
// 

static TCHAR* sszExecType[] = {
	_T("EXE"),
	_T("PIF"),
	_T("COM"),
	_T("BAT"),
	_T("CMD"),
	_T("LNK")
};

int __fastcall CheckExecType(LPCTSTR pszFileName)
{
	if (!pszFileName || *pszFileName == 0)
		return -1;

	LPTSTR p = PathFindExtension(pszFileName);
	if (*p == '.') {
		p++;
		for (int i = CE_EXE; i <= CE_LNK; i++) {
			if (!StrCmpI(sszExecType[i], p))
				return i;
		}
	}
	return -1;
}

void GetErrorString(DWORD code, CString& str)
{
	str.Empty();
	CMemLocal lp;
	FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
					NULL, code, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPTSTR)&lp, 0, NULL);
	str = lp;
}

int AppMessage(UINT id, LPCTSTR pszInsert /*=NULL*/, UINT nType /*=MB_STOP*/)
{
	CString msg;
	if (pszInsert && *pszInsert)
		msg.Format(id, pszInsert);
	else
		msg.LoadString(id);
	return AtlMessageBox(::GetActiveWindow(), (LPCTSTR)msg, APP_PRODUCT, nType);
}

LPTSTR str_clone(LPCTSTR psz)
{
	if (psz && *psz) {
		LPTSTR p = (LPTSTR)malloc(sizeof(TCHAR) * (lstrlen(psz) + 1));
		if (p)
			lstrcpy(p, psz);
		return p;
	}
	return NULL;
}

void str_free(LPCTSTR* ppsz)
{
	if (ppsz && *ppsz) {
		free((void*)*ppsz);
		*ppsz = NULL;
	}
}

BOOL __fastcall GetFileInfo(LPCTSTR pszName, WIN32_FIND_DATA& wfd)
{
	CFileFinder ff;
	if (ff.BeginFind(pszName)) {
		wfd = ff;
		FileTimeToLocalFileTime(&ff.ftCreationTime, &wfd.ftCreationTime);
		FileTimeToLocalFileTime(&ff.ftLastWriteTime, &wfd.ftLastWriteTime);
		FileTimeToLocalFileTime(&ff.ftLastAccessTime, &wfd.ftLastAccessTime);
		return TRUE;
	}
	return FALSE;
}

BOOL FileBrowse(BOOL bRead, HWND hwnd, _U_STRINGorID filter, CString& name, LPCTSTR pszInitialDir)
{
	TCHAR szFileName[MAX_PATH * 2], szFileTitle[MAX_PATH];
	szFileName[0] = szFileTitle[0] = 0;

	OPENFILENAME ofn;
	memset(&ofn, 0, sizeof(ofn));

	ofn.lStructSize = sizeof(ofn);
	ofn.hwndOwner = hwnd;
	ofn.lpstrFile = szFileName;
	ofn.nMaxFile = ARRAYSIZE(szFileName);
	ofn.lpstrFileTitle = szFileTitle;
	ofn.nMaxFileTitle = ARRAYSIZE(szFileTitle);
	ofn.Flags = OFN_HIDEREADONLY | OFN_EXPLORER;
	if (pszInitialDir && PathIsDirectory(pszInitialDir))
		ofn.lpstrInitialDir = pszInitialDir;

	CString flt(filter.m_lpstr);
	LPTSTR p = (LPTSTR)(LPCTSTR)flt;
	while (*p) {
		if (*p == _T('|')) *p = 0;
		p++;
	}
	ofn.lpstrFilter = flt;

	CEnterFolder ef;
	int nResult;
	if (bRead)
		nResult = GetOpenFileName(&ofn);
	else
		nResult = GetSaveFileName(&ofn);
	if (nResult)
		name = ofn.lpstrFile;
	return nResult;
}

void SaveFontInfo(LOGFONT* plf, UINT idSec, UINT idKey)
{
	if (plf) {
		LOGFONT lf = *plf;
		lf.lfWidth = 0;
		SetRegBinary(idSec, idKey, (BYTE*)&lf, sizeof(lf));
	}
}

BOOL LoadFontInfo(LOGFONT* plf, UINT idSec, UINT idKey)
{
	if (GetRegBinary(idSec, idKey, (BYTE*)plf, sizeof(LOGFONT)) == sizeof(LOGFONT)) {
		plf->lfWidth = 0;
		return TRUE;
	}
	return FALSE;
}

int GetSiblings(HWND hwndOwn, CSimpleArray<HWND>& array, bool bVisibleOnly)
{
	HWND hDesktop = ::GetDesktopWindow();
	TCHAR buf[256];
	for (HWND hwnd = ::GetTopWindow(hDesktop); hwnd != NULL; hwnd = ::GetNextWindow(hwnd, GW_HWNDNEXT)) {
		if (hwnd == hwndOwn)
			continue;
		if (bVisibleOnly && (!::IsWindowVisible(hwnd) || ::IsIconic(hwnd)))
			continue;
		if (::GetClassName(hwnd, buf, ARRAYSIZE(buf)) && !StrCmp(APP_FRAMECLASS, buf)) {
			array.Add(hwnd);
		}
	}
	return (int)array.GetSize();
}

void __fastcall AddNormalTab(HWND hwnd, LPCTSTR pszTitle, HIMAGELIST hImageList)
{
	if (hImageList)
		::SendMessage(hwnd, TCM_SETIMAGELIST, 0, (LPARAM)hImageList);

	CMemLocal str((void*)StrDup(pszTitle));
	LPTSTR p = str;
	int index = 0;
	TC_ITEM it;
	while (p && *p) {
		LPTSTR q = StrChr(p, '|');
		if (q)
			*q++ = '\0';

		memset(&it, 0, sizeof(it));
		it.mask = TCIF_TEXT;
		it.iImage = -1;
		if (hImageList) {
			it.mask |= TCIF_IMAGE;
			it.iImage = index;
		}
		it.pszText = p;
		it.cchTextMax = lstrlen(p);
		::SendMessage(hwnd, TCM_INSERTITEM, index++, (LPARAM)(const TC_ITEM*)(&it));
		p = q;
	}
}

void PopupHint(HWND hwnd, UINT nID)
{
	CString str;
	str.LoadString(nID);
	lccPopView(hwnd, NULL, str, PVS_CAPTION);
}

void __fastcall ValidateCurrentFolder(LPTSTR lpPath)
{
	// カレントディレクトリが有効でないならばディレクトリを変更
	TCHAR buf[MAX_PATH * 2];
	lstrcpy(buf, lpPath);

	while (!SetCurrentDirectory(buf)) {
		if (PathIsRoot(buf)) {
			lstrcpy(buf, _GCDATA.m_strWinDir);
			break;
		}
		PathRemoveFileSpec(buf);
	}
	lstrcpy(lpPath, buf);
}

//////////////////////////////////////////////////////////////////////
// debug only

BOOL DumpMemory(HGLOBAL hmem, LPCTSTR pszDumpFile)
{
	SIZE_T size = GlobalSize(hmem);
	LPBYTE lpb = (LPBYTE)GlobalLock(hmem);
	BOOL bOK = DumpMemory(lpb, (DWORD)size, pszDumpFile);
	GlobalUnlock(hmem);
	return bOK;
}

BOOL DumpMemory(LPBYTE lpb, DWORD size, LPCTSTR pszDumpFile)
{
	BOOL bOK = FALSE;
	HANDLE hf = _createfile(pszDumpFile, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
	if (hf != INVALID_HANDLE_VALUE)
	{
		DWORD s;
		bOK = WriteFile(hf, (LPCVOID)lpb, size, &s, NULL);
		if (!bOK || s != size)
			bOK = FALSE;
		CloseHandle(hf);
		if (!bOK)
			DeleteFile(pszDumpFile);
	}
	return bOK;
}

void __fastcall DrawUnderline(CDC* pdc, LPRECT prc, COLORREF rgb)
{
	lccPatB(pdc->m_hDC, prc->left, prc->bottom - 1,
					prc->right - prc->left, 1, rgb);
}

void __fastcall WebCall(HWND hwnd, UINT idResource)
{
	CString url;
	url.LoadString(idResource);

	SHELLEXECUTEINFO sei;
	memset(&sei, 0, sizeof(sei));
	sei.cbSize = sizeof(SHELLEXECUTEINFO);
	sei.fMask = SEE_MASK_UNICODE;
	sei.hwnd = hwnd;
	sei.nShow = SW_SHOWNORMAL;
	sei.lpVerb = _T("open");
	sei.lpFile = url;
	ShellExecuteEx(&sei);
}

LPTSTR __fastcall SkipTabSpace(LPTSTR p)
{
	if (p) {
		while (*p == ' ' || *p == '\t')
			p++;
	}
	return p;
}

int SeparateString(LPCTSTR pszText, TCHAR delim, CSimpleArray<CString>& array)
{
	array.RemoveAll();
	CMemLocal base((void*)StrDup(pszText));
	LPTSTR p = SkipTabSpace(base);
	LPTSTR q;
	while (p && *p) {
		if (*p == '"') {
			q = StrChr(++p, '"');
			if (q)
				*q++ = 0;
			else
				break;
		} else {
			q = StrChr(p + 1, delim);
			if (q)
				*q++ = 0;
		}
		array.Add(p);
		p = SkipTabSpace(q);
	}
	return array.GetSize();
}

/**
 *  
 */
int SplitString(LPCTSTR pszText, LPCTSTR delim, CSimpleArray<CString>& array, bool trim)
{
	array.RemoveAll();
	CString keywords = pszText;
	trim && keywords.Trim();
	int pos = 0;
	CString token = keywords.Tokenize(delim, pos);
	while (token != _T("")) {
		trim && token.Trim();
		array.Add(token);
		token = keywords.Tokenize(delim, pos);
	}
	return array.GetSize();
}

CSimpleArray<CString> SplitString(LPCTSTR pszText, LPCTSTR delim, bool trim)
{
	CSimpleArray<CString> arr;
	CString keywords = pszText;
	trim && keywords.Trim();
	int pos = 0;
	CString token = keywords.Tokenize(delim, pos);
	while (token != _T("")) {
		trim && token.Trim();
		arr.Add(token);
		token = keywords.Tokenize(delim, pos);
	}
	return arr;
}

int LoadTokenizedString(UINT id, LPCTSTR delim, CSimpleArray<CString>& array)
{
	CString s(MAKEINTRESOURCE(id));
	return SplitString(s.GetString(), delim, array);
}

CString JoinArray(CSimpleArray<CString>& array, LPCTSTR join)
{
	int cnt = array.GetSize();
	CString str;
	if (cnt > 0) {
		str = array[0];
		for (int i = 1; i < cnt; i++) {
			str.AppendFormat(_T("%s%s"), join, array[i].GetString());
		}
	}
	return str;
}

/* @@@ 未使用
void RegUpdateStringHistory(UINT idSec, UINT idKey, LPCTSTR pszText, int nMaxEntry)
{
	if (!pszText || *pszText == '\0') return;

	CSimpleArray<CString> array;
	array.Add(pszText);
	int nSize = lstrlen(pszText) + 1;
	register LPTSTR p;

	int len;
	BYTE* pd = GetRegBinaryEx(idSec, idKey, &len);
	if (pd) {
		p = (LPTSTR)pd;
		while (p && *p && nMaxEntry > array.GetSize()) {
			if (StrCmpI(pszText, p)) {
				nSize += lstrlen(p) + 1;
				array.Add(p);
			}
			p += lstrlen(p) + 1;
		}
		free(pd);
	}

	// リスト内のテキストを追加
	pd = new BYTE[(nSize + 16) * sizeof(TCHAR)];
	register LPTSTR psz = (LPTSTR)pd;
	for (int i = 0; i < array.GetSize(); i++) {
		lstrcpy(psz, array[i]);
		psz += lstrlen(array[i]) + 1;
	}

	// \0\0で終端を確定して保存
	*psz = '\0';
	SetRegBinary(idSec, idKey, (BYTE*)pd, (nSize + 1) * sizeof(TCHAR));
	delete [] pd;
}
@@@ */

CString GetCommonParentDir(HDROP hDrop)
{
	TCHAR name[MAX_PATH * 2];
	CString cc;
	int cnt = DragQueryFile(hDrop, 0xFFFFFFFF, NULL, 0);
	for (int i = 0; i < cnt; i++) {
		DragQueryFile(hDrop, i, name, ARRAYSIZE(name));
		PathRemoveFileSpec(name);
		if (cc.IsEmpty()) {
			cc = name;
		} else if (lccComparePath(cc, name)) {
			cc.Empty();
			break;
		}
	}
	return cc;
}

BOOL __fastcall ExtractSubString(LPCTSTR psz, int index, _STRorCH sc, LPTSTR sub, int cch)
{
	LPCTSTR sep = (IS_INTRESOURCE(sc.m_pstr)) ? (LPCTSTR)&sc.m_pstr : sc.m_pstr;
	CSimpleArray<CString> arr;
	int cnt = SplitString(psz, sep ,arr);
	if (0 <= index && index < cnt) {
		_tcscpy_s(sub, cch, arr[index].GetString());
		return TRUE;
	}
	return FALSE;
}

HDROP __fastcall MakeDropHandleFromPath(LPCTSTR pszFile)
{
	TDropFiles df;
	df.DragAppendFile(pszFile);
	return df.Detach();
}

void __fastcall ViewFileInformation(HWND hWnd, LPCTSTR lpFile)
{
	HDROP hdrop = MakeDropHandleFromPath(lpFile);
	if (hdrop) {
		fopShowFileInfo(hWnd, hdrop);
		DragFinish(hdrop);
	}
}

BOOL __fastcall checktoken(LPCTSTR pList, LPCTSTR pToken, _STRorCH sc)
{
	CString str(pList);
	LPCTSTR dlm = (IS_INTRESOURCE(sc.m_pstr)) ? (LPCTSTR)&sc.m_pstr : sc.m_pstr;
	CString token;
	int pos = 0;
	token= str.Tokenize(dlm, pos);
	while (token != _T("")) {
		if (!token.CompareNoCase(pToken)) { return TRUE; }
		token = str.Tokenize(dlm, pos);
	}
	return FALSE;
}

LPTSTR EnumDropItemAll(HDROP hDrop, BOOL bQuote)
{
	// ドロップされるファイル数を取得
	int num = (int)DragQueryFile(hDrop, 0xFFFFFFFF, NULL, 0);

	LPTSTR pEnum = tchar_malloc(num * (MAX_PATH + 3));
	if (pEnum) {
		TCHAR buf[MAX_PATH * 2 + 3];
		LPTSTR p = pEnum;
		int nEnum = 0;
		for (int i = 0; i < num; i++) {
			// ファイル名を取得
			DragQueryFile(hDrop, i, buf, ARRAYSIZE(buf));
			if (bQuote)
				PathQuoteSpaces(buf);

			if (nEnum) {
				lccAddChar(p, ' ');
				p++;
			}
			lstrcpy(p, buf);
			p += lstrlen(buf);
			nEnum++;
		}
	}
	return pEnum;
}

size_t __fastcall _wcs2mbs(char* mbstr, const wchar_t* wcstr, size_t cnt)
{
	if (cnt == 0 && mbstr != NULL)
		return 0;

	size_t res = ::WideCharToMultiByte(CP_ACP, 0, wcstr, -1, mbstr, (int)cnt, NULL, NULL);
	if (res > 0)
		mbstr[res - 1] = 0;
	return res;
}

size_t __fastcall _mbs2wcs(wchar_t* wcstr, const char* mbstr, size_t cnt)
{
	if (cnt == 0 && wcstr != NULL)
		return 0;

	size_t res = ::MultiByteToWideChar(CP_ACP, 0, mbstr, -1, wcstr, (int)cnt);
	if (res > 0)
		wcstr[res - 1] = 0;
	return res;
}

BOOL __fastcall PathGetUNCServerShare(LPCTSTR pPath, LPTSTR pShare)
{
	if (pPath && *pPath == '\\' && *(pPath + 1) == '\\') {
		lstrcpy(pShare, pPath);
		register LPTSTR p = pShare;

		p = StrChr(p + 2, '\\');
		if (p && *(p + 1) != '\0') {
			p = StrChr(p + 1, '\\');
			if (p)
				*p = '\0';
			return TRUE;
		}
	}
	return FALSE;
}

LPTSTR __fastcall PathGetRootFromPath(LPCTSTR pszPath, LPTSTR pszRoot)
{
	if (PathIsUNC(pszPath))
		return (PathGetUNCServerShare(pszPath, pszRoot) ? pszRoot : NULL);
	else if (ISALPHA(*pszPath) && *(pszPath + 1) == ':')
		return PathBuildRoot(pszRoot, ToUpper(*pszPath) - 'A');
	return NULL;
}

BOOL getlinkinfo(LPCTSTR pszName, CString& target)
{
	BOOL bOK = FALSE;
	CComPtr<IShellLink> psl;
	if (psl.CoCreateInstance(CLSID_ShellLink) == S_OK) {
		CComQIPtr<IPersistFile> ppf = psl;
		if (ppf.p != NULL) {
			if (ppf->Load(pszName, STGM_READ) == S_OK) {
				LPITEMIDLIST pidl;
				if (NOERROR == psl->GetIDList(&pidl)) {
					TCHAR buf[MAX_PATH * 2];
					if (SHGetPathFromIDList(pidl, buf)) {
						target = buf;
						bOK = TRUE;
					}
					else
						bOK = idl_GetName(pidl, SHGDN_FORPARSING | SHGDN_NORMAL, target);
					CoTaskMemFree(pidl);
				}
			}
		}
	}
	return bOK;
}

void __fastcall CalcTextRectDimension(HDC hdc, LPCTSTR psz, int len, UINT format, int maxwidth, SIZE* size)
{
	CRect rc(0, 0, maxwidth, 0);
	::DrawText(hdc, psz, len, &rc, DT_CALCRECT | format);
	size->cx = rc.Width();
	size->cy = rc.Height();
}

void __fastcall CalcTextDimension(HDC hdc, LPCTSTR psz, int len, SIZE* size)
{
	return CalcTextRectDimension(hdc, psz, len, DT_SINGLELINE | DT_NOPREFIX, 0, size);
}

LPCTSTR __fastcall num_form(DWORDLONG dlNum, CString& str)
{
	DWORD t0 = (DWORD)(dlNum / (DWORDLONG)1000000000);
	dlNum = dlNum % (DWORDLONG)1000000000;
	DWORD t1 = (DWORD)(dlNum / (DWORDLONG)1000000);
	dlNum = dlNum % (DWORDLONG)1000000;
	DWORD t2 = (DWORD)(dlNum / (DWORDLONG)1000);
	DWORD t3 = (DWORD)(dlNum % (DWORDLONG)1000);

	TCHAR buf[64];
	wsprintf(buf, _T("%03u,%03u,%03u,%03u"), t0, t1, t2, t3);

	register LPTSTR p = buf;
	while (*p == '0' || *p == ',')
		p++;

	if (*p == 0)
		str = _T("0");
	else
		str = p;
	return (LPCTSTR)str;
}

int __fastcall FormatFileSize(DWORDLONG size, CString& str, BOOL bFmtByte)
{
	if (bFmtByte) {
		if (size < 1024) {
			size = (size == 0) ? 0 : 1;
			num_form(size, str);
			str += _T(" KB");
		}
		else {
			StrFormatByteSize64(size, str.GetBuffer(64), 64);
			str.ReleaseBuffer();
		}
	}
	else
		num_form(size, str);
	return str.GetLength();
}

LPCTSTR __fastcall gtime2jtime(SYSTEMTIME* pst, CString& str, bool bWidthTime)
{
	// 西暦→和暦文字列
	str.Empty();
	int year = pst->wYear;
	if (pst->wYear < 1864)		{ /* 何もしない */; }
	else if (pst->wYear < 1911) { str = _T("明治"); year = pst->wYear - 1864; }
	else if (pst->wYear < 1925) { str = _T("大正"); year = pst->wYear - 1911; }
	else if (pst->wYear < 1988) { str = _T("昭和"); year = pst->wYear - 1925; }
	else						{ str = _T("平成"); year = pst->wYear - 1988; }

	CString date;
	date.Format(_T("%02d年%02d月%02d日"), year, pst->wMonth, pst->wDay);
	str += date;
	if (bWidthTime) {
		date.Format(_T(" %02d時%02d分%02d秒"), pst->wHour, pst->wMinute, pst->wSecond);
		str += date;
	}
	return (LPCTSTR)str;
}

int __fastcall FormatSystemTime(SYSTEMTIME& st, CString& str, BOOL b2col, BOOL bRelative)
{
//	gtime2jtime(&st, str, true);		// 和暦表示
	str.Format(_T("%04u/%02u/%02u"), st.wYear, st.wMonth, st.wDay);
	if (b2col)
		str.Delete(0, 2);
	if (bRelative) {
		DWORDLONG dl;
		SystemTimeToFileTime(&st, (FILETIME*)&dl);

		CToday today;
		if      (today.isToday(dl))     str = _T("今日");
		else if (today.isYesterday(dl)) str = _T("昨日");
		else if (today.is2daysAgo(dl))  str = _T("2日前");
//@		else if (today.isThisMonth(dl)) str.Format(_T("今月%d日"), st.wDay);
		str.AppendFormat(_T(" %02u:%02u"), st.wHour, st.wMinute);
	}
	else {
		str.AppendFormat(_T(" %02u:%02u:%02u"), st.wHour, st.wMinute, st.wSecond);
	}
	return str.GetLength();
}

int __fastcall FormatFileTime(FILETIME* pft, CString& str, BOOL b2col, BOOL bRelative)
{
	SYSTEMTIME st;
	FileTimeToSystemTime(pft, &st);
	return FormatSystemTime(st, str, b2col, bRelative);
}

int __fastcall MakeEnumStrFromHdrop(HDROP hdrop, CString& str)
{
	// ファイルがひとつであればその名前を、
	// 複数であれば 選択された<nn>個のファイル

	str.Empty();
	int entries = 0;
	if (hdrop) {
		TCHAR buf[MAX_PATH * 2];
		int num = DragQueryFile(hdrop, 0xFFFFFFFF, NULL, 0);
		if (num == 1) {
			DragQueryFile(hdrop, 0, buf, ARRAYSIZE(buf));
			str = PathFindFileName(buf);
		}
		else {
			WIN32_FILE_ATTRIBUTE_DATA ad;
			int files = 0;
			int folders = 0;
			for (int i = 0; i < num; i++) {
				DragQueryFile(hdrop, i, buf, ARRAYSIZE(buf));
				if (!GetFileAttributesEx(buf, GetFileExInfoStandard, &ad)) continue;
				if (ad.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) folders++;
				else files++;
			}

			str.Format(IDS_FORMAT_ENUMFILES, files, folders);
			lstrcpy(buf, str);

			int pos = str.Find(_T(','));
			if (files > 0 && folders < 1)
				str.Delete(pos, str.GetLength() - pos);
			else if (files < 1)
				str.Delete(0, pos + 1);
			str.Trim();
		}
	}
	return entries;
}

int __fastcall MakeEnumStrFromDataObject(IDataObject* pdo, CString& str)
{
	// オブジェクトがひとつであればその名前を、
	// 複数であれば 選択された<nn>個のファイル
	str.Empty();
	CSimpleArray<PCIDLIST_ABSOLUTE> array;
	int entries = GetIDListsFromDataObject(pdo, array);
	if (entries > 0) {
		TCHAR buf[MAX_PATH * 2];
		if (entries == 1) {
			idl_GetName(array[0], SHGDN_INFOLDER | SHGDN_FOREDITING, str);
		}
		else {
			int files = 0;
			int folders = 0;
			for (int i = 0; i < entries; i++) {
				SFGAOF attr = idl_GetAttributes(array[i], SFGAO_FOLDER | SFGAO_CAPABILITYMASK);
				if (attr != 0) {
					if (attr & SFGAO_FOLDER) folders++;
					else files++;
				}
			}

			str.Format(IDS_FORMAT_ENUMFILES, files, folders);
			lstrcpy(buf, str);

			int pos = str.Find(_T(','));
			if (files > 0 && folders < 1)
				str.Delete(pos, str.GetLength() - pos);
			else if (files < 1)
				str.Delete(0, pos + 1);
			str.Trim();
		}
	}
	return entries;
}

BOOL __fastcall WriteDataToFile(LPCTSTR pszFile, void* pData, int len)
{
	HANDLE hf = _createfile(pszFile, GENERIC_READ | GENERIC_WRITE, 0,
						NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_ARCHIVE, NULL);
	if (hf == INVALID_HANDLE_VALUE)
		return FALSE;

	DWORD dw;
	BOOL bOK = WriteFile(hf, pData, len, &dw, NULL);
	CloseHandle(hf);
	if (!bOK)
	{
		DeleteFile(pszFile);
		return FALSE;
	}
	return TRUE;
}

void* __fastcall ReadDataFromFile(LPCTSTR lpFileName)
{
	HANDLE hf = _createfile(lpFileName, GENERIC_READ,
							FILE_SHARE_READ | FILE_SHARE_WRITE,
							NULL, OPEN_EXISTING, 0, NULL);
	if (hf == INVALID_HANDLE_VALUE)
		return NULL;

	DWORD dwSize = GetFileSize(hf, NULL);
	void* pvBuffer = malloc(dwSize);
	if (!pvBuffer)
	{
		CloseHandle(hf);
		return NULL;
	}
	DWORD dwRead;
	BOOL bOK = ReadFile(hf, pvBuffer, dwSize, &dwRead, NULL);
	if (!bOK || dwRead != dwSize)
		bOK = FALSE;
	CloseHandle(hf);

	if (!bOK)
	{
		free(pvBuffer);
		return NULL;
	}
	return pvBuffer;
}

BOOL _openclipboardbyeditor(HWND hwnd)
{
	return lccDllExecute(hwnd, DLL_MIL, MIL_EDIT, _T("#clip"), _T("/edit"), FALSE);
}

void __fastcall MakeShortcutObject(HWND hwnd, IShellFolder* psf, LPCITEMIDLIST pidl, LPCTSTR pszDisplayName)
{
/////// 参考
// Create New Shortcut Wizard:
//    'puts the new shortcut in the location specified by %1
//    rundll32.exe AppWiz.Cpl,NewLinkHere %1
///////////////////////////////////

	CComPtr<IDataObject> pdo = (IDataObject*)idl_GetUIObjectOf(hwnd, psf, pidl, IID_IDataObject);
	if (pdo.p != NULL) {
		CString cs;
		cs.Format(IDS_TITLE_CREATELINK, pszDisplayName);

		TCHAR name[MAX_PATH];
		BROWSEINFO bi;
		memset(&bi, 0, sizeof(bi));
		bi.hwndOwner = hwnd;
		bi.pidlRoot = NULL;
		bi.pszDisplayName = name;
		bi.lpszTitle = cs;
		bi.ulFlags = BIF_RETURNONLYFSDIRS  | BIF_USENEWUI;
		CIDListPtr idlTarget = SHBrowseForFolder(&bi);
		if (!idlTarget.isNull()) {
			// デスクトップを選択した場合には、ショートカットを作成
			// できないための対処
			CString str;
			if (idlTarget.GetPath(str)) {
				idlTarget.Free();
				idlTarget.CreateFromPath(str);
			}
		}

		if (!idlTarget.isNull())
			idl_Execute(hwnd, idlTarget, pdo, MK_LBUTTON | MK_SHIFT | MK_CONTROL, DROPEFFECT_LINK, CPoint(0,0));
	}
}

void __fastcall PasteShortcutFromClipboard(HWND hwnd, LPCITEMIDLIST pidlTarget)
{
	CComPtr<IDataObject> pdo;
	if (OleGetClipboard(&pdo) == S_OK) {
		if (AppMessage(IDS_CLIPBOARD_PASTESHORTCUT, NULL, MB_QYESNO) == IDYES)
			idl_Execute(hwnd, pidlTarget, pdo, MK_LBUTTON | MK_SHIFT | MK_CONTROL, DROPEFFECT_LINK, CPoint(0,0));
	}
}

BOOL _getdiskfreespace(LPCTSTR pszRoot, DWORDLONG& sfree, DWORDLONG& stotal)
{
	sfree = 0;
	stotal = 0;
/* @@@ ストレージがフォルダにマウントされていると正確な情報が取得できない
	TCHAR buf[MAX_PATH];
	if (_ISNETPATH(pszRoot)) {
		// "\\Server\Share\" の形式を作成する
		if (PathGetUNCServerShare(pszRoot, buf))
			PathAddBackslash(buf);
		else
			lstrcpy(buf, pszRoot);
	}
	else
		PathBuildRoot(buf, ToUpper(*pszRoot) - 'A');

	ULARGE_INTEGER t, f, tf;
	if (GetDiskFreeSpaceEx(buf, &f, &t, &tf)) {
		SETDWORDLONG(stotal, t.LowPart, t.HighPart);
		SETDWORDLONG(sfree, f.LowPart, f.HighPart);
		return TRUE;
	}
	return FALSE;
@@@ */
	ULARGE_INTEGER fc, t, f;
	if (SHGetDiskFreeSpace(pszRoot, &fc, &t, &f)) {
		SETDWORDLONG(stotal, t.LowPart, t.HighPart);
		SETDWORDLONG(sfree, fc.LowPart, fc.HighPart);
		return TRUE;
	}
	return FALSE;
}

BOOL __fastcall GetFileData(LPCTSTR pszName, WIN32_FIND_DATA* pwfd)
{
	if (_getfileinfo(pszName, pwfd)) {
		// ローカル時間に変換
		FILETIME ft;
		FileTimeToLocalFileTime(&pwfd->ftCreationTime, &ft);
		pwfd->ftCreationTime = ft;
		FileTimeToLocalFileTime(&pwfd->ftLastWriteTime, &ft);
		pwfd->ftLastWriteTime = ft;
		return TRUE;
	}
	return FALSE;
}

BOOL __fastcall _overridefileattributes(LPCTSTR pszName, WIN32_FIND_DATA* pwfd)
{
/* @@@
	WIN32_FILE_ATTRIBUTE_DATA wfad;
	if (GetFileAttributesEx(pszName, GetFileExInfoStandard, &wfad)) {
		pwfd->dwFileAttributes = wfad.dwFileAttributes;
		pwfd->ftCreationTime = wfad.ftCreationTime;
		pwfd->ftLastAccessTime = wfad.ftLastAccessTime;
		pwfd->ftLastWriteTime = wfad.ftLastWriteTime;
		pwfd->nFileSizeLow = wfad.nFileSizeLow;
		pwfd->nFileSizeHigh = wfad.nFileSizeHigh;
		return TRUE;
	}
	return FALSE;
@@@ */
	return GetFileAttributesEx(pszName, GetFileExInfoStandard, (WIN32_FILE_ATTRIBUTE_DATA*)pwfd);
}

BOOL __fastcall _pathfileexists(LPCTSTR pszName)
{
	if (!pszName || *pszName == 0) return FALSE;

	WIN32_FILE_ATTRIBUTE_DATA wfad;
	return GetFileAttributesEx(pszName, GetFileExInfoStandard, &wfad);
}

BOOL __fastcall _pathisfile(LPCTSTR pszName)
{
	if (!pszName || *pszName == 0) return FALSE;

	WIN32_FILE_ATTRIBUTE_DATA wfad;
	return (GetFileAttributesEx(pszName, GetFileExInfoStandard, &wfad) &&
			!(wfad.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY));

}

BOOL __fastcall _pathisdirectory(LPCTSTR pszName)
{
	BOOL ok = FALSE;
	WIN32_FILE_ATTRIBUTE_DATA wfad;
	if (pszName && *pszName) {
		ok = GetFileAttributesEx(pszName, GetFileExInfoStandard, &wfad);
		if (ok) {
			if (!(wfad.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)) {
				ok = FALSE;
				SetLastError(ERROR_PATH_NOT_FOUND);
			}
		}
	}
	else
		SetLastError(ERROR_BAD_ARGUMENTS);
	return ok;
}

BOOL __fastcall _pathisdirectoryUI(HWND hwnd, LPCTSTR pszPath)
{
	// ディレクトリの検査…名前不正やネットワークリソースエラーのチェック
	if (!PathIsUNCServer(pszPath) && !_pathisdirectory(pszPath)) {
		DWORD err = GetLastError();
		if (err == ERROR_FILE_NOT_FOUND) err = ERROR_BAD_PATHNAME;

		lccErrorBox(hwnd, err, APP_PRODUCT, pszPath, MB_STOP);
		return FALSE;
	}
	return TRUE;
}

BOOL __fastcall _pathispathspec(LPCTSTR pszPath)
{
	// x: 形式とUNCパス形式の超手抜きチェック
	return (pszPath && ((IsCharAlpha(pszPath[0]) && pszPath[1] == _T(':')) || PathIsUNC(pszPath)));
}

BOOL __fastcall CheckDirectoryNameUI(HWND hwnd, LPCTSTR pszPath)
{
	// ディレクトリの検査…名前不正やネットワークリソースエラーのチェック
	// 対象のディレクトリが存在しなくても、エラーコードにより許容の是非を確認する
	if (!_pathisdirectory(pszPath)) {
		DWORD err = GetLastError();
		switch (err) {
		case ERROR_BAD_NETPATH:
		case ERROR_INVALID_NAME:
		case ERROR_NO_NETWORK:
			lccErrorBox(hwnd, err, APP_PRODUCT, pszPath, MB_STOP);
			return FALSE;
		}
	}
	return TRUE;
}

BOOL __fastcall CheckServerUI(HWND hwnd, LPCTSTR pszFolder)
{
	if (PathIsUNC(pszFolder)) {
		// サーバの接続を確認
		TCHAR path[MAX_PATH * 2];
		lstrcpy(path, pszFolder);
		while (PathIsUNC(path) && !PathIsUNCServer(path) && !PathIsUNCServerShare(path))
			PathRemoveFileSpec(path);

		if (!SHValidateUNC(hwnd, path, 0)) {
			lccErrorBox(hwnd, GetLastError(), APP_PRODUCT, pszFolder, MB_STOP);
			return FALSE;
		}

		if (lstrcmp(pszFolder, path)) {
			// Sharedフォルダの接続を確認
			lstrcpy(path, pszFolder);
			return SHValidateUNC(hwnd, path, 0);
		}
	}
	return TRUE;
}

int __fastcall GetWindowNumCount(LPCTSTR pszClass, HWND* phWnd, BOOL bVisibleOnly)
{
	int cnt = 0;
	HWND hwnd = GetWindow(GetDesktopWindow(), GW_CHILD);

	TCHAR buf[MAX_PATH];
	DWORD style;
	while (hwnd) {
		::GetClassName(hwnd, buf, MAX_PATH);
		style = GetWindowLong(hwnd, GWL_STYLE);
		if (!StrCmp(pszClass, buf)) {
			if (bVisibleOnly) {
				if (style & WS_VISIBLE && !(style & WS_MINIMIZE)) {
					cnt++;
					if (phWnd)
						*phWnd++ = hwnd;
				}
			}
			else {
				if (phWnd)
					*phWnd++ = hwnd;
				cnt++;
			}
		}
		hwnd = ::GetNextWindow(hwnd, GW_HWNDNEXT);
	}
    return cnt;
}

void __fastcall build_unicode_path(LPCTSTR pSrc, LPTSTR pDst, int /*cchMax*/)
{
	lstrcpy(pDst, _T("\\\\?\\"));
	LPCTSTR p = pSrc;
	if (PathIsUNC(pSrc)) {
		p = PathFindNextComponent(pSrc);
		lstrcat(pDst, _T("UNC\\"));
	}
	lstrcat(pDst, p);
}

HANDLE __fastcall _createfile(LPCTSTR pName, DWORD fdwAccess, DWORD fdwShareMode, LPSECURITY_ATTRIBUTES psa, DWORD fdwCreate, DWORD fdwAttrsAndFlags, HANDLE hTemplateFile)
{
	WCHAR wsz[MAX_PATH * 2];
	build_unicode_path(pName, wsz, ARRAYSIZE(wsz));
	return CreateFile(wsz, fdwAccess, fdwShareMode, psa, fdwCreate, fdwAttrsAndFlags, hTemplateFile);
}

void* __fastcall _get_rcdata(UINT idResource, DWORD* psize)
{
	void* pData = NULL;
	HRSRC hrsrc = FindResource(RESINST, _MKIR(idResource), RT_RCDATA);
	if (hrsrc) {
		if (psize)
			*psize = SizeofResource(RESINST, hrsrc);
		HGLOBAL hglb = LoadResource(RESINST, hrsrc);
		if (hglb)
			pData = LockResource(hglb);
	}
	return pData;
}

void __fastcall tchar_set(LPTSTR psz, TCHAR ch, size_t size)
{
	for (size_t i = 0; i < size; i++)
		*psz++ = ch;
}

LPTSTR __fastcall tchar_malloc(size_t size)
{
	return (LPTSTR)malloc(size * sizeof(TCHAR));
}

LPTSTR __fastcall tchar_realloc(LPTSTR p, size_t size)
{
	return (LPTSTR)realloc(p, size * sizeof(TCHAR));
}

BYTE __fastcall byte_rangerot_left(BYTE w, int range, int rot)
{
	BYTE chkbit = 1;
	BYTE mask = 1;
	while (--range > 0)
	{
		chkbit <<= 1;
		mask = (mask << 1) | 1;
	}

	BYTE save = w & (0xff^mask);
	w &= mask;
	while (rot > 0)
	{
		BOOL flg = (w & chkbit);
		w <<= 1;
		if (flg)
			w |= 1;
		rot--;
	}
	return (save | (w & mask));
}

BYTE __fastcall byte_rangerot_right(BYTE w, int range, int rot)
{
	BYTE chkbit = 1;
	BYTE mask = 1;
	while (--range > 0)
	{
		chkbit <<= 1;
		mask = (mask << 1) | 1;
	}

	BYTE save = w & (0xff^mask);
	w &= mask;
	while (rot > 0)
	{
		BOOL flg = (w & 1);
		w >>= 1;
		if (flg)
			w |= chkbit;
		rot--;
	}
	return (save | (w & mask));
}

WORD __fastcall word_rangerot_left(WORD w, int range, int rot)
{
	WORD chkbit = 1;
	WORD mask = 1;
	while (--range > 0)
	{
		chkbit <<= 1;
		mask = (mask << 1) | 1;
	}

	WORD save = w & (0xffff^mask);
	w &= mask;
	while (rot > 0)
	{
		BOOL flg = (w & chkbit);
		w <<= 1;
		if (flg)
			w |= 1;
		rot--;
	}
	return (save | (w & mask));
}

WORD __fastcall word_rangerot_right(WORD w, int range, int rot)
{
	WORD chkbit = 1;
	WORD mask = 1;
	while (--range > 0)
	{
		chkbit <<= 1;
		mask = (mask << 1) | 1;
	}

	WORD save = w & (0xffff^mask);
	w &= mask;
	while (rot > 0)
	{
		BOOL flg = (w & 1);
		w >>= 1;
		if (flg)
			w |= chkbit;
		rot--;
	}
	return (save | (w & mask));
}

DWORD __fastcall dword_rangerot_left(DWORD w, int range, int rot)
{
	DWORD chkbit = 1;
	DWORD mask = 1;
	while (--range > 0)
	{
		chkbit <<= 1;
		mask = (mask << 1) | 1;
	}

	DWORD save = w & (0xffffffff^mask);
	w &= mask;
	while (rot > 0)
	{
		BOOL flg = (w & chkbit);
		w <<= 1;
		if (flg)
			w |= 1;
		rot--;
	}
	return (save | (w & mask));
}

DWORD __fastcall dword_rangerot_right(DWORD w, int range, int rot)
{
	DWORD chkbit = 1;
	DWORD mask = 1;
	while (--range > 0)
	{
		chkbit <<= 1;
		mask = (mask << 1) | 1;
	}

	DWORD save = w & (0xffffffff^mask);
	w &= mask;
	while (rot > 0)
	{
		BOOL flg = (w & 1);
		w >>= 1;
		if (flg)
			w |= chkbit;
		rot--;
	}
	return (save | (w & mask));
}

BOOL __fastcall GetToolBarButtonPoint(const MSG* pmsg, UINT nIDCommand, LPPOINT lppt)
{
	if (pmsg && pmsg->lParam) {
		CToolBarCtrl bar = (HWND)pmsg->lParam;
		if (bar.IsWindow()) {
			int cmd = bar.CommandToIndex(nIDCommand);
			if (cmd >= 0) {
				CRect rc;
				bar.GetItemRect(cmd, &rc);
				bar.MapWindowPoints(::GetDesktopWindow(), &rc);
				lppt->x = rc.left;
				lppt->y = rc.bottom;
				return TRUE;
			}
		}
	}
	return FALSE;
}

#pragma comment(lib, "msimg32.lib")

void __fastcall ShadeRect(HDC hdc, const CRect& rc, COLORREF crFrom, COLORREF crTo, bool bVirtical)
{
	ATLASSERT(hdc != NULL);
	TRIVERTEX vert[2];
	GRADIENT_RECT grc;

	vert[0].x     = rc.left;
	vert[0].y     = rc.top;
	vert[0].Red   = GetRValue(crFrom) << 8;
	vert[0].Green = GetGValue(crFrom) << 8;
	vert[0].Blue  = GetBValue(crFrom) << 8;
	vert[0].Alpha = 0x0000;

	vert[1].x     = rc.right;
	vert[1].y     = rc.bottom;
	vert[1].Red   = GetRValue(crTo) << 8;
	vert[1].Green = GetGValue(crTo) << 8;
	vert[1].Blue  = GetBValue(crTo) << 8;
	vert[1].Alpha = 0x0000;

	grc.UpperLeft  = 0;
	grc.LowerRight = 1;
	GradientFill(hdc, vert, 2, &grc, 1, bVirtical ? GRADIENT_FILL_RECT_V : GRADIENT_FILL_RECT_H);
}

COLORREF __fastcall PixelAlpha(COLORREF srcPixel, int percent)
{
	// My formula for calculating the transpareny is as
	// follows (for each single color):
	//
	//							   percent
	// destPixel = sourcePixel * ( ------- )
	//                               100
	//
	// This is not real alpha blending, as it only modifies the brightness,
	// but not the color (a real alpha blending had to mix the source and
	// destination pixels, e.g. mixing green and red makes yellow).
	// For our nice "menu" shadows its good enough.
	int r = GetRValue(srcPixel);
	if (r == 0) r++;
	int g = GetGValue(srcPixel);
	if (g == 0) g++;
	int b = GetBValue(srcPixel);
	if (b == 0) b++;

	return RGB ( min (255, (r * percent) / 100), 
				 min (255, (g * percent) / 100), 
				 min (255, (b * percent) / 100));
}

void __fastcall CannotCreateWindow(HWND hwnd, LPCTSTR pszText)
{
	MessageBox(hwnd, _T("Window creation error"), pszText, MB_STOP);
}

void __fastcall InitDlgCtrl(HWND hctl, ATL::_U_STRINGorID res)
{
	CString s;
	if (GetClassName(hctl, s.GetBuffer(256), 256) < 1) return;
	s.ReleaseBuffer();
	int type = !s.CompareNoCase(WC_COMBOBOX) ? 1 : (!s.CompareNoCase(WC_COMBOBOXEX) ? 2 : 0);

	CSimpleArray<CString> arr;
	int cnt = IS_INTRESOURCE(res.m_lpstr) ?
				LoadTokenizedString(PtrToUint(res.m_lpstr), _T(";"), arr) :
				SplitString(res.m_lpstr, _T(";") ,arr);
	COMBOBOXEXITEM item;
	for (int i = 0; i < cnt; i++) {
		switch (type) {
		case 1:
			::SendMessage(hctl, CB_ADDSTRING, 0, (LPARAM)arr[i].GetString());
			break;
		case 2:
			item.mask = CBEIF_TEXT;
			item.iItem = -1;
			item.pszText = (LPTSTR)arr[i].GetString();
			::SendMessage(hctl, CBEM_INSERTITEM, 0, (LPARAM)&item);
			break;
		}
	}
}

DWORD __fastcall HashBin(const BYTE* pcb, int len)
{
	DWORD nHash = 0;
	if (pcb && len > 0) {
		while (len-- > 0) {
			nHash = (nHash<<5)+nHash+(*pcb);
			pcb++;
		}
	}
	return nHash;
}

DWORD __fastcall HashBinR(const BYTE* pcb, int len)
{
	DWORD nHash = 0;
	if (pcb && len > 0) {
		pcb += len;
		while (len-- > 0) {
			nHash = (nHash<<5)+nHash+(*pcb);
			pcb--;
		}
	}
	return nHash;
}

DWORDLONG MakeQWORDHash(const BYTE* pcb, int len)
{
	DWORD lo = HashBin(pcb, len);
	DWORD hi = HashBinR(pcb, len);
	DWORDLONG val;
	SETDWORDLONG(val, lo, hi);
	return val;
}

void ShowCtrlHelp(HWND hwndCtrl, UINT idCtrl)
{
	CTipHelp tiphelp;
	tiphelp.ShowTips(hwndCtrl, idCtrl);
}

//
// UNICODE文字列からUTF8文字列に変換
//

#include <mlang.h>

bool xxUnicodeToUTF8(LPCWSTR pwszUnicode, CStringA& strUtf)
{
	strUtf.Empty();

	CComPtr<IMultiLanguage2> pcv;
	if (pcv.CoCreateInstance(CLSID_CMultiLanguage) != S_OK)
		return false;

	UINT srcLen = (lstrlenW(pwszUnicode) + 1) * sizeof(WCHAR);
	UINT dstLen = 0;
	DWORD mode = 0;

	// バッファサイズ取得
	HRESULT	hr = pcv->ConvertString(&mode, 1200, 65001, (BYTE*)pwszUnicode, &srcLen, NULL, &dstLen);
	if (FAILED(hr))
		return false;

	// 変換
	strUtf.GetBufferSetLength(dstLen);
	hr = pcv->ConvertString(&mode, 1200, 65001, (BYTE*)pwszUnicode, &srcLen, (BYTE*)strUtf.GetBuffer(dstLen), &dstLen);
	if (FAILED(hr)) {
		strUtf.Empty();
		return	false;
	}
	return	true;
}

bool UnicodeToUTF8(LPCWSTR pwszUnicode, CStringA& strUtf)
{
	strUtf.Empty();

	// 最大3倍になる
	int size = lstrlen(pwszUnicode) * 3 + 1;
/*
	BYTE* q = (BYTE*)strUtf.GetBuffer(size);
	LPCWSTR p = pwszUnicode;
	while (*p) {
		if (*p <= 0x7f)
			*q++ = static_cast<BYTE>(*p);
		else if (*p <= 0x7ff) {
			*q++ = 0xc0 | static_cast<BYTE>(*p >> 6);
			*q++ = 0x80 | static_cast<BYTE>(*p & 0x3f);
		}
		else {
			*q++ = 0xe0 | static_cast<BYTE>(*p >> 12);
			*q++ = 0x80 | static_cast<BYTE>((*p >> 6 ) & 0x3f);
			*q++ = 0x80 | static_cast<BYTE>(*p &0x3f);
		}
		++p;
	}
	*q = 0;
*/
	char* q = strUtf.GetBuffer(size);
	WideCharToMultiByte(CP_UTF8, 0, pwszUnicode, -1, q, size, NULL, NULL);
	strUtf.ReleaseBuffer();
	return true;
}

HRESULT CoCreateInstanceAsAdmin(HWND hwnd, BOOL bAdmin, REFCLSID rclsid, REFIID riid, __out void ** ppv)
{
	if (bAdmin) {
		WCHAR  wszCLSID[128];
		StringFromGUID2(rclsid, wszCLSID, ARRAYSIZE(wszCLSID)); 
		CString moniker;
		moniker.Format(_T("Elevation:Administrator!new:%s"), wszCLSID);


		BIND_OPTS3 bo;
		memset(&bo, 0, sizeof(bo));
		bo.cbStruct = sizeof(bo);
		bo.hwnd = hwnd;
		bo.dwClassContext = CLSCTX_LOCAL_SERVER;
		return CoGetObject(moniker, &bo, riid, ppv);
	}
	else
		return CoCreateInstance(rclsid, NULL, CLSCTX_INPROC, riid, ppv);
}

BOOL isMediaInserted(int drive)
{
	BOOL ok = FALSE;
	CString str;
	str.Format(_T("\\\\.\\%c:"), drive + 'A');
	HANDLE hf = CreateFile(str.GetString(), GENERIC_READ, FILE_SHARE_WRITE, NULL, OPEN_EXISTING, 0, NULL);
	if (hf != INVALID_HANDLE_VALUE) {
		DWORD numBytes;
		ok = DeviceIoControl(hf, IOCTL_STORAGE_CHECK_VERIFY, NULL, 0, NULL, 0, &numBytes, NULL);
		CloseHandle(hf);
	}
	return ok;
}

bool CheckFileWritable(const char* pszFile)
{
	CStringW fileW = pszFile;
	CWin32FindData fd;
	if (!_getfileinfo(fileW, &fd))
		return true;

	if (fd.isReadOnly())
		SetFileAttributes(fileW, (fd.dwFileAttributes & 0xff) & ~FILE_ATTRIBUTE_READONLY);

	HANDLE hf = CreateFile(fileW, GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ, NULL, OPEN_EXISTING, 0, NULL);
	if (hf != INVALID_HANDLE_VALUE) {
		CloseHandle(hf);
		return true;
	}
	return false;
}

static void __fastcall fnametmp(register LPTSTR p)
{
	while (p && *p) {
		if (*p == _T('_') || *p == _T('-')) { *p = 0x20; }
		p++;
	}
}

/**
 * ハイフンやアンダースコアをスペースと同じと見なして文字列比較する
 * 通常比較                   見なし比較
 * -----------------------   -----------------------
 * 1: filename 02-34.ext     1: filename_01-24.ext
 * 2: filename_01_25.ext     2: filename_01_25.ext
 * 3: filename_01-24.ext     3: filename 02-34.ext
 */
int __fastcall CompareFileName(LPCTSTR src, LPCTSTR dst)
{
	int r = 0;
	if (gnListStat & LTS_CMPUNDERSCORE) {
		LPTSTR s = _tcsdup(src);
		LPTSTR t = _tcsdup(dst);
		fnametmp(s);
		fnametmp(t);
		r = _fCompareFileName(s, t);
		free(s);
		free(t);
	}
	return r ? r : _fCompareFileName(src, dst);
}


typedef struct tagKNOWNFOLDERMAP {
	LPCTSTR name;
	GUID guid;
} KNOWNFOLDERMAP;

static KNOWNFOLDERMAP knownFolderMap[] = {
// 省略版
  _T("~"), FOLDERID_Profile,
  _T("desk"), FOLDERID_Desktop,
  _T("pic"), FOLDERID_Pictures,
  _T("vid"), FOLDERID_Videos,
  _T("doc"), FOLDERID_Documents,
  _T("mus"), FOLDERID_Music,
  _T("cpl"), FOLDERID_ControlPanelFolder,
  _T("down"),FOLDERID_Downloads,
  // 通常
  _T("Network"), FOLDERID_NetworkFolder,
  _T("Drives"), FOLDERID_ComputerFolder,
  _T("Computer"), FOLDERID_ComputerFolder,
  _T("Internet"), FOLDERID_InternetFolder,
  _T("ControlPanel"), FOLDERID_ControlPanelFolder,
  _T("Printer"), FOLDERID_PrintersFolder,
  _T("SyncManagerFolder"), FOLDERID_SyncManagerFolder,
  _T("SyncSetupFolder"), FOLDERID_SyncSetupFolder,
  _T("ConflictFolder"), FOLDERID_ConflictFolder,
  _T("SyncResultsFolder"), FOLDERID_SyncResultsFolder,
  _T("RecycleBinFolder"), FOLDERID_RecycleBinFolder,
  _T("ConnectionsFolder"), FOLDERID_ConnectionsFolder,
  _T("Fonts"), FOLDERID_Fonts,                                    // <Windir>\Fonts
  _T("Desktop"), FOLDERID_Desktop,                                // <Home>\Desktop
  _T("Startup"), FOLDERID_Startup,                                // <MyAppData>\Roaming\Microsoft\Windows\Start Menu\Programs\Startup
  _T("Programs"), FOLDERID_Programs,                              // <MyAppData>\Roaming\Microsoft\Windows\Start Menu\Programs
  _T("StartMenu"), FOLDERID_StartMenu,                            // <MyAppData>\Roaming\Microsoft\Windows\Start Menu
  _T("Recent"), FOLDERID_Recent,                                  // <MyAppData>\Roaming\Microsoft\Windows\Recent
  _T("SendTo"), FOLDERID_SendTo,                                  // <MyAppData>\Roaming\Microsoft\Windows\SendTo
  _T("Documents"), FOLDERID_Documents,                            // <Home>\Documents
  _T("Favorites"), FOLDERID_Favorites,                            // <Home>\Favorites
  _T("NetHood"), FOLDERID_NetHood,                                // <MyAppData>\Roaming\Microsoft\Windows\Network Shortcuts
  _T("PrintHood"), FOLDERID_PrintHood,                            // <MyAppData>\Roaming\Microsoft\Windows\Printer Shortcuts
  _T("Templates"), FOLDERID_Templates,                            // <MyAppData>\Roaming\Microsoft\Windows\Templates
  _T("CommonStartup"), FOLDERID_CommonStartup,                    // <ProgData>\Microsoft\Windows\Start Menu\Programs\Startup
  _T("CommonPrograms"), FOLDERID_CommonPrograms,                  // <ProgData>\Microsoft\Windows\Start Menu\Programs
  _T("CommonStartMenu"), FOLDERID_CommonStartMenu,                // <ProgData>\Microsoft\Windows\Start Menu
  _T("PublicDesktop"), FOLDERID_PublicDesktop,                    // <UserPublic>\Desktop
  _T("ProgramData"), FOLDERID_ProgramData,                        // <ProgData>
  _T("CommonTemplates"), FOLDERID_CommonTemplates,                // <ProgData>\Microsoft\Windows\Templates
  _T("PublicDocuments"), FOLDERID_PublicDocuments,                // <UserPublic>\Documents
  _T("RoamingAppData"), FOLDERID_RoamingAppData,                  // <MyAppData>\Roaming
  _T("LocalAppData"), FOLDERID_LocalAppData,                      // <MyAppData>\Local
  _T("LocalAppDataLow"), FOLDERID_LocalAppDataLow,                // <MyAppData>\LocalLow
  _T("InternetCache"), FOLDERID_InternetCache,                    // <MyAppData>\Local\Microsoft\Windows\INetCache
  _T("Cookies"), FOLDERID_Cookies,                                // <MyAppData>\Local\Microsoft\Windows\INetCookies
  _T("History"), FOLDERID_History,                                // <MyAppData>\Local\Microsoft\Windows\History
  _T("System"), FOLDERID_System,                                  // <Windir>\System32
  _T("SystemX86"), FOLDERID_SystemX86,                            // <Windir>\SysWOW64
  _T("Windows"), FOLDERID_Windows,                                // <Windir>
  _T("Profile"), FOLDERID_Profile,                                // <UserProfile>
  _T("Pictures"), FOLDERID_Pictures,                              // <Home>\Pictures
  _T("ProgramFilesX86"), FOLDERID_ProgramFilesX86,                // <ProgramFilesX86>
  _T("ProgramFilesCommonX86"), FOLDERID_ProgramFilesCommonX86,    // <CommonFIlesX86>
  _T("ProgramFilesX64"), FOLDERID_ProgramFilesX64,                // <ProgramFiles>
  _T("ProgramFilesCommonX64"), FOLDERID_ProgramFilesCommonX64,    // <CommonFiles>
  _T("ProgramFiles"), FOLDERID_ProgramFiles,                      // <ProgramFiles>
  _T("ProgramFilesCommon"), FOLDERID_ProgramFilesCommon,          // <CommonFiles>
  _T("UserProgramFiles"), FOLDERID_UserProgramFiles,              // <MyAppData>\Local\Programs
  _T("UserProgramFilesCommon"), FOLDERID_UserProgramFilesCommon,  // <MyAppData>\Local\Programs\Common
  _T("AdminTools"), FOLDERID_AdminTools,                          // <MyAppData>\Roaming\Microsoft\Windows\Start Menu\Programs\Administrative Tools
  _T("CommonAdminTools"), FOLDERID_CommonAdminTools,              // <ProgData>\Microsoft\Windows\Start Menu\Programs\Administrative Tools
  _T("Music"), FOLDERID_Music,                                    // <Home>\Music
  _T("Videos"), FOLDERID_Videos,                                  // <Home>\Videos
  _T("Ringtones"), FOLDERID_Ringtones,                            // <MyAppData>\Local\Microsoft\Windows\Ringtones
  _T("PublicPictures"), FOLDERID_PublicPictures,                  // <UserPublic>\Pictures
  _T("PublicMusic"), FOLDERID_PublicMusic,                        // <UserPublic>\Music
  _T("PublicVideos"), FOLDERID_PublicVideos,                      // <UserPublic>\Videos
  _T("PublicRingtones"), FOLDERID_PublicRingtones,                // <ProgData>\Microsoft\Windows\Ringtones
  _T("ResourceDir"), FOLDERID_ResourceDir,                        // <Windir>\Resources
  _T("LocalizedResourcesDir"), FOLDERID_LocalizedResourcesDir,    // 
  _T("CommonOEMLinks"), FOLDERID_CommonOEMLinks,                  // 
  _T("CDBurning"), FOLDERID_CDBurning,                            // <MyAppData>\Local\Microsoft\Windows\Burn\Burn
  _T("UserProfiles"), FOLDERID_UserProfiles,                      // X:\Users
  _T("Playlists"), FOLDERID_Playlists,                            // 
  _T("SamplePlaylists"), FOLDERID_SamplePlaylists,                // 
  _T("SampleMusic"), FOLDERID_SampleMusic,                        // 
  _T("SamplePictures"), FOLDERID_SamplePictures,                  // 
  _T("SampleVideos"), FOLDERID_SampleVideos,                      // 
  _T("PhotoAlbums"), FOLDERID_PhotoAlbums,                        // 
  _T("Public"), FOLDERID_Public,                                  // <UserPublic>
  _T("ChangeRemovePrograms"), FOLDERID_ChangeRemovePrograms,      // N/A
  _T("AppUpdates"), FOLDERID_AppUpdates,                          // N/A
  _T("AddNewPrograms"), FOLDERID_AddNewPrograms,                  // N/A
  _T("Downloads"), FOLDERID_Downloads,                            // <Home>\Downloads
  _T("PublicDownloads"), FOLDERID_PublicDownloads,                // <UserPublic>\Downloads
  _T("SavedSearches"), FOLDERID_SavedSearches,                    // <Home>\Searches
  _T("QuickLaunch"), FOLDERID_QuickLaunch,                        // <MyAppData>\Roaming\Microsoft\Internet Explorer\Quick Launch
  _T("Contacts"), FOLDERID_Contacts,                              // <Home>\Contacts
  _T("SidebarParts"), FOLDERID_SidebarParts,                      // 
  _T("SidebarDefaultParts"), FOLDERID_SidebarDefaultParts,        // 
  _T("PublicGameTasks"), FOLDERID_PublicGameTasks,                // <ProgData>\Microsoft\Windows\GameExplorer
  _T("GameTasks"), FOLDERID_GameTasks,                            // <MyAppData>\Local\Microsoft\Windows\GameExplorer
  _T("SavedGames"), FOLDERID_SavedGames,                          // <Home>\Saved Games
  _T("Games"), FOLDERID_Games,                                    // 
  _T("SEARCH_MAPI"), FOLDERID_SEARCH_MAPI,                        // N/A
  _T("SEARCH_CSC"), FOLDERID_SEARCH_CSC,                          // N/A
  _T("Links"), FOLDERID_Links,                                    // <Home>\Links
  _T("UsersFiles"), FOLDERID_UsersFiles,                          // <Home>
  _T("UsersLibraries"), FOLDERID_UsersLibraries,                  // N/A
  _T("SearchHome"), FOLDERID_SearchHome,                          // N/A
  _T("OriginalImages"), FOLDERID_OriginalImages,                  // 
  _T("DocumentsLibrary"), FOLDERID_DocumentsLibrary,              // N/A
  _T("MusicLibrary"), FOLDERID_MusicLibrary,                      // N/A
  _T("PicturesLibrary"), FOLDERID_PicturesLibrary,                // N/A
  _T("VideosLibrary"), FOLDERID_VideosLibrary,                    // N/A
  _T("RecordedTVLibrary"), FOLDERID_RecordedTVLibrary,            // <UserPublic>\Libraries\RecordedTV.library-ms
  _T("HomeGroup"), FOLDERID_HomeGroup,                            // 
  _T("HomeGroupCurrentUser"), FOLDERID_HomeGroupCurrentUser,      // 
  _T("DeviceMetadataStore"), FOLDERID_DeviceMetadataStore,        // <ProgData>\Microsoft\Windows\DeviceMetadataStore
  _T("Libraries"), FOLDERID_Libraries,                            // N/A
  _T("PublicLibraries"), FOLDERID_PublicLibraries,                // <UserPublic>\Libraries
  _T("AccountPictures"), FOLDERID_AccountPictures,                // <MyAppData>\Roaming\Microsoft\Windows\AccountPictures
  _T("PublicUserTiles"), FOLDERID_PublicUserTiles,                // <UserPublic>\AccountPictures
  _T("AppsFolder"), FOLDERID_AppsFolder,                          // N/A
  _T("StartMenuAllPrograms"), FOLDERID_StartMenuAllPrograms,      // 
  _T("CommonStartMenuPlaces"), FOLDERID_CommonStartMenuPlaces,    // 
  _T("ApplicationShortcuts"), FOLDERID_ApplicationShortcuts,      // <MyAppData>\Local\Microsoft\Windows\Application Shortcuts
  _T("RoamingTiles"), FOLDERID_RoamingTiles,                      // <MyAppData>\Local\Microsoft\Windows\RoamingTiles
  _T("RoamedTileImages"), FOLDERID_RoamedTileImages,              // 
  _T("Screenshots"), FOLDERID_Screenshots,                        // <Home>\Pictures\Screenshots
  _T("CameraRoll"), FOLDERID_CameraRoll,                          // <Home>\Pictures\Camera Roll
  _T("SkyDrive"), FOLDERID_SkyDrive,                              // <Home>\OneDrive
  _T("OneDrive"), FOLDERID_OneDrive,                              // <Home>\OneDrive
  _T("SkyDriveDocuments"), FOLDERID_SkyDriveDocuments,            // <Home>\OneDrive\ドキュメント
  _T("SkyDrivePictures"), FOLDERID_SkyDrivePictures,              // <Home>\OneDrive\画像
  _T("SkyDriveMusic"), FOLDERID_SkyDriveMusic,                    // 
  _T("SkyDriveCameraRoll"), FOLDERID_SkyDriveCameraRoll,          // <Home>\OneDrive\画像\カメラ ロール
  _T("SearchHistory"), FOLDERID_SearchHistory,                    // 
  _T("SearchTemplates"), FOLDERID_SearchTemplates,                // 
  _T("CameraRollLibrary"), FOLDERID_CameraRollLibrary,            // N/A
  _T("SavedPictures"), FOLDERID_SavedPictures,                    // <Home>\Pictures\Saved Pictures
  _T("SavedPicturesLibrary"), FOLDERID_SavedPicturesLibrary,      // N/A
  _T("RetailDemo"), FOLDERID_RetailDemo,                          // <ProgData>\Microsoft\Windows\RetailDemo
  _T("Device"), FOLDERID_Device,                                  // ::{F8278C54-A712-415B-B593-B77A2BE0DDA9}
  _T("DevelopmentFiles"), FOLDERID_DevelopmentFiles,              // 
  _T("Objects3D"), FOLDERID_Objects3D,                            // 
  _T("AppCaptures"), FOLDERID_AppCaptures,                        // <Home>\Videos\Captures
  _T("LocalDocuments"), FOLDERID_LocalDocuments,                  // <Home>\Documents
  _T("LocalPictures"), FOLDERID_LocalPictures,                    // <Home>\Pictures
  _T("LocalVideos"), FOLDERID_LocalVideos,                        // <Home>\Videos
  _T("LocalMusic"), FOLDERID_LocalMusic,                          // <Home>\Music
  _T("LocalDownloads"), FOLDERID_LocalDownloads,                  // <Home>\Downloads
  _T("RecordedCalls"), FOLDERID_RecordedCalls,                    // 
  _T("AllAppMods"), FOLDERID_AllAppMods,                          // 
  _T("CurrentAppMods"), FOLDERID_CurrentAppMods,                  // 
  _T("AppDataDesktop"), FOLDERID_AppDataDesktop,                  // 
  _T("AppDataDocuments"), FOLDERID_AppDataDocuments,              // 
  _T("AppDataFavorites"), FOLDERID_AppDataFavorites,              // 
  _T("AppDataProgramData"), FOLDERID_AppDataProgramData,          // 
  _T("LocalStorage"), FOLDERID_LocalStorage,                      // 
  _T(""), GUID_NULL, NULL
};

const KNOWNFOLDERID& LookupKnownFolder(LPCTSTR keyword)
{
	const KNOWNFOLDERMAP* p = knownFolderMap;
	while (p->name) {
		if (!StrCmpI(p->name, keyword)) {
			return p->guid;
		}
		p++;
	}
	return GUID_NULL;
}

LPCTSTR PathAnalyze(LPTSTR path, int cchMax)
{
	CString s(path);
	s.Replace(_T('/'), _T('\\'));
	StrCpyN(path, s, cchMax);

	s.GetBuffer(MAX_PATH * 2);
	CMemTask tm;
	if (*path == _T('~') && SHGetKnownFolderPath(FOLDERID_Profile, 0, NULL, (PWSTR*)&tm) == S_OK) {
		s = tm;
		PathAppend(s.GetBuffer(), path + 1);
		StrCpyN(path, s.GetString(), cchMax);
	} else if (*path == _T('@')) {
		KNOWNFOLDERID id = LookupKnownFolder(path + 1);
		if (SHGetKnownFolderPath(id, 0, NULL, (PWSTR*)&tm) == S_OK) {
			StrCpyN(path, tm, cchMax);
		}
	}
	return path;
}

/////////////////////////////////////////////////////////////////////////////////

/* @@@@    DLL遅延ロードサンプル

	BOOL bOK = FALSE;
	__try {
		bOK = amUnlhaProc(GetActiveWindow(), lpCmdLine, lpResult, dwSize);
	}
	__except (DelayLoadDllExceptionFilter(GetExceptionInformation())) {
		// nothing to do
	}
	return bOK;


extern "C"
long WINAPI DelayLoadDllExceptionFilter(PEXCEPTION_POINTERS pep)
{
	PDelayLoadInfo pdli = PDelayLoadInfo(pep->ExceptionRecord->ExceptionInformation[0]);
	TCHAR sz[500] = { 0 };

	long lDisposition = EXCEPTION_EXECUTE_HANDLER;
	switch (pep->ExceptionRecord->ExceptionCode)
	{
		case VcppException(ERROR_SEVERITY_ERROR, ERROR_MOD_NOT_FOUND):
			wsprintf(sz, _T("Dll not found: %s"), pdli->szDll);
			break;

		case VcppException(ERROR_SEVERITY_ERROR, ERROR_PROC_NOT_FOUND):
			if (pdli->dlp.fImportByName)
				wsprintf(sz, _T("Function %s was not found in %s"),
								pdli->dlp.szProcName, pdli->szDll);
			else
				wsprintf(sz, _T("Function ordinal %d was not found in %s"),
								pdli->dlp.dwOrdinal, pdli->szDll);
			break;

		default:
			lDisposition = EXCEPTION_CONTINUE_SEARCH;
			break;
	}

	if (lDisposition == EXCEPTION_EXECUTE_HANDLER)
		MessageBox(::GetActiveWindow(), sz, APP_PRODUCT, MB_STOP);
	return lDisposition;
}

@@@@ */
