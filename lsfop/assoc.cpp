// assoc.cpp : implementation file
//

#include "stdafx.h"
#include "fopstd.h"

static TCHAR sszShellOpenCommand[]  = _T("shell\\open\\command");

#define MAXCOLUMN		4

static RESLISTHEAD rcHead[MAXCOLUMN] = {
	_MKIR(IDS_ASSOC_PROGID),	LVCFMT_LEFT,	140,
	_MKIR(IDS_ASSOC_EXT), 		LVCFMT_LEFT,	140,
	_MKIR(IDS_ASSOC_TYPENAME),	LVCFMT_LEFT,	120,
	_MKIR(IDS_ASSOC_COMMAND),	LVCFMT_LEFT,	300
};

/////////////////////////////////////////////////////////////////////////////
//

class CNameEqualHelper
{
public:
	static bool IsEqual(const LPCTSTR k1, const LPCTSTR k2) { return (lstrcmpi(k1, k2) == 0); }
};

class CNameMapEqualHelper
{
public:
	static bool IsEqualKey(const LPCTSTR k1, const LPCTSTR k2) { return (lstrcmpi(k1, k2) == 0); }
	static bool IsEqualValue(const LPCTSTR k1, const LPCTSTR k2) { return (lstrcmpi(k1, k2) == 0); }
};

/////////////////////////////////////////////////////////////////////////////
//

class CRegItem
{
public:
	CRegItem() : m_bReg(true), m_iIcon(-1) { }

	CString m_strProgId;			// ProgID
	CString m_strName;				// タイプ名
	CString m_strCmd;				// コマンド
	CString m_strExec;				// 実行プログラム
	CString m_strIcon;				// 標準のアイコン
	CSimpleArray<CString> m_arrExt;
	int m_iIcon;					// アイコンインデクス
	bool m_bReg;					// レジストリ上に存在するデータか否か (true=レジストリ上にあり)
};

/////////////////////////////////////////////////////////////////////////////
//

static int RetrieveRegFileType(CSimpleArray<CRegItem*>& array)
{
	CWaitCursor wc;
	CSimpleMap<CString, CString, CNameMapEqualHelper> map;			// 拡張子とそれに該当するProgIdのマップ
	CSimpleMap<CString, CString, CNameMapEqualHelper> ignore;		// 無視するProgIdリスト
	CSimpleArray<CRegItem*> browse;

	CRegKey reg;
	CRegItem* item;
	TCHAR key[MAX_PATH], buf[MAX_PATH * 2];
	long lcb;
	DWORD cch;
	for (DWORD index = 0; RegEnumKey(HKEY_CLASSES_ROOT, index, key, ARRAYSIZE(key)) == ERROR_SUCCESS; index++) {
		if ((*key == '.') && (StrChr(key + 1, '.') == NULL)) {
			// 拡張子キーの場合
			// ProgID（"cppfile" など）を取得
			lcb = ARRAYSIZE(buf);
			if (RegQueryValue(HKEY_CLASSES_ROOT, key, buf, &lcb) == ERROR_SUCCESS && lstrlen(buf) > 0) {
				// ProgIdあり
				CharLower(key);
				map.Add(key, buf);
			}
			else {
				// ProgIdなし
				// 拡張子キーにそのまま shell/open/command が記録されている？
			}
		}
		else {
			// ProgIdの場合
			// CurVerキーがあればそちらは最終的に無視するので、ここでは無視リストに登録しておく
			lstrcpy(buf, key);
			PathAppend(buf, _T("CurVer"));
			if (reg.Open(HKEY_CLASSES_ROOT, buf, KEY_READ) == ERROR_SUCCESS) {
				ULONG len = ARRAYSIZE(buf);
				if (reg.QueryStringValue(NULL, buf, &len) == ERROR_SUCCESS && len > 1)
					ignore.Add(buf, key);
				reg.Close();
			}

			item = new CRegItem;
			item->m_strProgId = key;
			browse.Add(item);
		}
	}

	// 無視するProgIdを適用し、データを取得
	bool bDelete;
	int i, cnt = browse.GetSize();
	for (i = 0; i < cnt; i++) {
		bDelete = true;
		item = browse[i];
		if (ignore.FindKey(item->m_strProgId) < 0) {
			// コマンドを取得
			cch = ARRAYSIZE(buf);
			if (AssocQueryString(0, ASSOCSTR_COMMAND, item->m_strProgId, NULL, buf, &cch) == S_OK) {
				item->m_strCmd = buf;
				// 実行プログラムを取得
				cch = ARRAYSIZE(buf);
				if (AssocQueryString(0, ASSOCSTR_EXECUTABLE, item->m_strProgId, NULL, buf, &cch) == S_OK) {
					TCHAR* p = PathFindExtension(buf);
					// 拡張子があるもののみ有効
					if (p && *p == '.') {
						item->m_strExec = buf;
						// 文書名を取得
						cch = ARRAYSIZE(buf);
						if (AssocQueryString(0, ASSOCSTR_FRIENDLYDOCNAME, item->m_strProgId, NULL, buf, &cch) == S_OK)
							item->m_strName = buf;
						// アイコンを取得
						cch = ARRAYSIZE(buf);
						if (AssocQueryString(0, ASSOCSTR_DEFAULTICON, item->m_strProgId, NULL, buf, &cch) == S_OK)
							item->m_strIcon = buf;
						else if (!item->m_strExec.IsEmpty())
							item->m_strIcon.Format(_T("%s,0"), item->m_strExec);
						bDelete = false;
					}
				}
			}
		}

		if (bDelete)
			delete item;
		else
			array.Add(item);
	}

	// 拡張子とProgIdをマッチさせる
	cnt = array.GetSize();
	int nummap = map.GetSize();
	for (i = 0; i < nummap; i++) {
		CString progid = map.GetValueAt(i);
		int pos = ignore.FindKey(progid);
		if (pos >= 0)
			progid = ignore.GetValueAt(pos);
		for (int j = 0; j < cnt; j++) {
			item = array[j];
			if (!item->m_strProgId.CompareNoCase(progid)) {
				item->m_arrExt.Add(map.GetKeyAt(i));
				break;
			}
		}
	}
	return cnt;
}

/////////////////////////////////////////////////////////////////////////////
// CAssocDlg dialog

class CAssocDlg : public CDialogImpl<CAssocDlg>, public CDialogResize<CAssocDlg>
{
private:
	CListViewCtrl m_list;
public:
	CAssocDlg() { }
	~CAssocDlg() { }

	enum { IDD = IDD_ASSOCIATE };

BEGIN_MSG_MAP(CAssocDlg)
	ON_WM_INITDIALOG()
	CHAIN_MSG_MAP(CDialogResize<CAssocDlg>)
	ON_WM_HELPINFO()
	ON_NOTIFY(LVN_GETDISPINFO, IDC_LIST, OnGetDispInfo)
	ON_NOTIFY(LVN_ITEMCHANGED, IDC_LIST, OnItemChanged)
	ON_NOTIFY(LVN_DELETEITEM, IDC_LIST, OnDeleteItem)
	ON_BN_CLICKED(IDOK, OnOK)
	ON_BN_CLICKED(IDCANCEL, OnCancel)
	ON_NOTIFY(SEN_GETTEXT, IDC_APP, OnNotifyBrowse)
	ON_BN_CLICKED(IDC_HELPCALL, OnHelpCall)
END_MSG_MAP()

BEGIN_DLGRESIZE_MAP(CAssocDlg)
	DLGRESIZE_CONTROL(IDOK,				DLSZ_MOVE_Y)
	DLGRESIZE_CONTROL(IDCANCEL,			DLSZ_MOVE_Y)
	DLGRESIZE_CONTROL(IDC_HELPCALL,		DLSZ_MOVE_X | DLSZ_MOVE_Y)
	DLGRESIZE_CONTROL(IDC_TITLE,		DLSZ_SIZE_X)
	DLGRESIZE_CONTROL(IDC_APP,			DLSZ_SIZE_X)
	DLGRESIZE_CONTROL(IDC_LIST,			DLSZ_SIZE_X | DLSZ_SIZE_Y)
END_DLGRESIZE_MAP()

	BOOL OnInitDialog(LPARAM lParam);
	BOOL OnHelpInfo(HELPINFO* pInfo);
	void OnGetDispInfo(LPNMHDR pnmh, LRESULT* pResult);
	void OnItemChanged(LPNMHDR pnmh, LRESULT* pResult);
	void OnDeleteItem(LPNMHDR pnmh, LRESULT* pResult);
	void OnOK();
	void OnCancel() { EndDialog(IDCANCEL); }
	void OnNotifyBrowse(NMHDR* pNmhdr, LRESULT* pResult);
	void OnHelpCall() { ShowHtmlHelp(m_hWnd, IDD); }

	bool InitRegItem(CRegItem* item, LPCTSTR ext, LPCTSTR exec);
	int FindItemIndex(LPCTSTR pszName);
	void ListupRegDataBase(LPCTSTR pszDefaultExtension);
};

BOOL CAssocDlg::OnInitDialog(LPARAM lParam)
{
	DlgResize_Init();
	CenterWindow();

	lccDisableCharInput(GetDlgItem(IDC_TITLE));

	LPCTSTR pExt = (LPCTSTR)lParam;
	if (pExt) SetDlgItemText(IDC_EXT, pExt);

	m_list = GetDlgItem(IDC_LIST);
	DWORD style = m_list.GetExtendedListViewStyle() | LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES;
	m_list.SetExtendedListViewStyle(style);

	for (int i = 0; i < MAXCOLUMN; i++) {
		CString str(rcHead[i].label);
		m_list.InsertColumn(i, str, rcHead[i].fmt, rcHead[i].cx, i);
	}

	HIMAGELIST himL, himS;
	Shell_GetImageLists(&himL, &himS);
	m_list.SetImageList(himS, LVSIL_SMALL);

	// 登録情報データベースのエントリをリストアップ
	ListupRegDataBase(pExt);
	return TRUE;
}

BOOL CAssocDlg::OnHelpInfo(HELPINFO* pInfo)
{
	if (pInfo->iContextType == HELPINFO_WINDOW) {
		ShowHtmlHelp(m_hWnd, IDD);
		return TRUE;
	}
	return FALSE;
}

int CAssocDlg::FindItemIndex(LPCTSTR pszName)
{
	if (!pszName || *pszName == 0)
		return -1;

	int cnt = m_list.GetItemCount();
	for (int i = 1; i < cnt; i++) {
		CRegItem* item = (CRegItem*)m_list.GetItemData(i);
		if (item && !item->m_strProgId.CompareNoCase(pszName))
			return i;
	}
	return -1;
}

void CAssocDlg::OnOK()
{
	// 拡張子を取得
	TCHAR ext[MAX_PATH], old[MAX_PATH];
	bool fAssocChanged = false;

	ext[0] = '.';
	if (GetDlgItemText(IDC_EXT, &ext[1], ARRAYSIZE(ext) - 1) < 1) {
		MessageBeep(0);
		GetDlgItem(IDC_EXT).SetFocus();
		return;
	}
	CharLower(ext);

	// 標準の関連付けの修正
	if (!StrCmpI(ext, _T(".exe")) ||
		!StrCmpI(ext, _T(".com")) ||
		!StrCmpI(ext, _T(".lnk")) ||
		!StrCmpI(ext, _T(".dll")) ||
		!StrCmpI(ext, _T(".cpl")) ||
		!StrCmpI(ext, _T(".ini")) ||
		!StrCmpI(ext, _T(".sys")) ||
		!StrCmpI(ext, _T(".drv")) ||
		!StrCmpI(ext, _T(".ocx")) ||
		!StrCmpI(ext, _T(".pif")) ||
		!StrCmpI(ext, _T(".reg"))) {
		AppMessage(IDS_TITLE_ASSOCIATE, IDS_EXEASSOC_ERROR, NULL, MB_ATTENTION);
		return;
	}

	// 指定拡張子の既存するProgIDを取得
	long lch = ARRAYSIZE(old);
	if (RegQueryValue(HKEY_CLASSES_ROOT, ext, old, &lch) != ERROR_SUCCESS)
		old[0] = 0;

	CString progid;
	if (GetDlgItemText(IDC_APP, progid) > 0) {
		CRegItem* item = NULL;
		int index = m_list.GetNextItem(-1, LVNI_ALL | LVNI_SELECTED);
		if (index > 0)
			item = (CRegItem*)m_list.GetItemData(index);

		if (!item || progid.CompareNoCase(item->m_strProgId)) {
			// 入力されたProgIdが選択中のものとは異なる場合
			item = NULL;
			index = FindItemIndex(progid);
			if (index > 0)
				item = (CRegItem*)m_list.GetItemData(index);
		}

		if (item && item->m_bReg) {
			// この項目は最初からレジストリに存在しているProgId
			if (old[0]) {
				if (!item->m_strProgId.CompareNoCase(old)) {
					// ProgIdが一致するので何もしない
					return;
				}
				else if (AppMessage(IDS_TITLE_ASSOCIATE, IDS_QUERY_ASSOC, ext, MB_QYESNO) == IDNO) {
					// ProgIDがレジストリの情報と異なれば上書き確認をおこなう
					return;
				}
			}

			// 指定の拡張子に新しいProgIDをセット
			RegSetValue(HKEY_CLASSES_ROOT, ext, REG_SZ, item->m_strProgId, item->m_strProgId.GetLength());
			fAssocChanged = true;
		}
		else {
			CRegItem ri;
			if (!item) {
				if (!InitRegItem(&ri, ext, progid)) {
					AppMessage(IDS_TITLE_ASSOCIATE, IDS_EXEC_OR_PROGID, progid, MB_STOP);
					return;
				}
				item = &ri;
			}

			// ProgIDが拡張子.XXXに登録されていなければ
			if (StrCmpI(old, item->m_strProgId)) {
				if (AppMessage(IDS_TITLE_ASSOCIATE, IDS_QUERY_ASSOC, ext, MB_QYESNO) == IDNO)
					return;
				RegSetValue(HKEY_CLASSES_ROOT, ext, REG_SZ, item->m_strProgId, lstrlen(item->m_strProgId));
			}

			// タイプエントリ "xxxfile\shell\open\command" を作成
			TCHAR sk[MAX_PATH];
			PathCombine(sk, item->m_strProgId, sszShellOpenCommand);
			CRegKey reg;
			if (reg.Create(HKEY_CLASSES_ROOT, sk) == ERROR_SUCCESS) {
				reg.SetStringValue(NULL, item->m_strCmd);
				reg.Close();
				fAssocChanged = true;
			}
		}
	}
	else {
		// レジストリに情報が登録されていれば、ProgIdを削除
		if (old[0]) {
			if (AppMessage(IDS_TITLE_ASSOCIATE, IDS_QUERY_ASSOC, ext, MB_QYESNO) == IDNO)
				return;

			CRegKey reg;
			if (reg.Open(HKEY_CLASSES_ROOT, ext, KEY_ALL_ACCESS) == ERROR_SUCCESS) {
				reg.SetStringValue(NULL, _T(""));
				reg.Close();
				fAssocChanged = true;
			}
		}
	}

	if (fAssocChanged)
		SHChangeNotify(SHCNE_ASSOCCHANGED, SHCNF_FLUSHNOWAIT, 0, 0);

	EndDialog(IDOK);
}

bool CAssocDlg::InitRegItem(CRegItem* item, LPCTSTR ext, LPCTSTR exec)
{
	if (!PathFileExists(exec) || PathIsDirectory(exec))
		return false;

	item->m_bReg = false;				// レジストリ上のキーではない
	item->m_arrExt.Add(ext);

	// 実行ファイル名からProgIDを作成
	TCHAR buf[MAX_PATH * 2];
	LPTSTR p = PathFindFileName(exec);
	lstrcpy(buf, p);
	p = PathFindExtension(buf);
	*p = 0;
	item->m_strProgId.Format(_T("%s.Document"), buf);

	// タイプ名
	CFileVersion ver;
	if (ver.Open(exec)) {
		buf[0] = 0;
		if (ver.GetFileDescription(buf, ARRAYSIZE(buf)))
			item->m_strName = buf;
		ver.Close();
	}

	// アイコン
	SHFILEINFO sfi;
	SHGetFileInfo(exec, 0, &sfi, sizeof(sfi), SHGFI_SYSICONINDEX | SHGFI_SMALLICON);
	item->m_iIcon = sfi.iIcon;

	// コマンド
	lstrcpy(buf, exec);
	PathQuoteSpaces(buf);
	item->m_strCmd = buf;
	item->m_strCmd += _T(" \"%1\"");
	return true;
}


void CAssocDlg::OnNotifyBrowse(NMHDR* pNmhdr, LRESULT* pResult)
{
	NMSEGETTEXT* psgt = (NMSEGETTEXT*)pNmhdr;

	CString ext;
	if (GetDlgItemText(IDC_EXT, ext) < 1) {
		GetDlgItem(IDC_EXT).SetFocus();
		MessageBeep(0);
		return;
	}

	if (ext[0] != _T('.'))
		ext.Insert(0, _T('.'));

	TCHAR exe[MAX_PATH * 2], curdir[MAX_PATH * 2];
	GetCurrentDirectory(ARRAYSIZE(curdir), curdir);
	if (FileBrowse(TRUE, m_hWnd, IDS_CAPTION_BROWSE, IDS_FILTER_EXECUTE, curdir, exe)) {
		CRegItem* item = new CRegItem;
		InitRegItem(item, ext, exe);
		int index = m_list.InsertItem(m_list.GetItemCount(), item->m_strProgId, item->m_iIcon);
		m_list.SetItemText(index, 1, item->m_arrExt[0]);
		m_list.SetItemText(index, 2, item->m_strName);
		m_list.SetItemText(index, 3, item->m_strCmd);
		m_list.SetItemData(index, (DWORD_PTR)item);

		m_list.SetItemState(index, LVIS_SELECTED | LVIS_FOCUSED, LVIS_SELECTED | LVIS_FOCUSED);
		m_list.EnsureVisible(index, FALSE);

		lstrcpyn(psgt->pszText, item->m_strProgId, psgt->cchTextMax);
		*pResult = TRUE;
	}
}

void CAssocDlg::ListupRegDataBase(LPCTSTR pszDefaultExtension)
{
	// (なし) を挿入
	CString str;
	str.LoadString(IDS_UNDEFINED);
	m_list.InsertItem(0, str, -1);
	m_list.SetItemData(0, 0);

	int sel = -1;
	CSimpleArray<CRegItem*> array;
	if (RetrieveRegFileType(array) > 0) {
		CRegItem* item;
		int index;
		for (int i = 0; i < array.GetSize(); i++) {
			item = array[i];
			index = m_list.InsertItem(LVIF_IMAGE | LVIF_PARAM | LVIF_TEXT, i + 1, item->m_strProgId, 0, 0, I_IMAGECALLBACK, (LPARAM)item);
			int cnt = item->m_arrExt.GetSize();
			if (cnt > 0) {
				CString ext;
				for (int j = 0; j < cnt; j++) {
					ext.AppendFormat(_T("%s, "), (LPCTSTR)item->m_arrExt[j]);
					if (pszDefaultExtension && *pszDefaultExtension && !StrCmpI(pszDefaultExtension, (LPCTSTR)item->m_arrExt[j] + 1))
						sel = index;
				}
				ext.TrimRight(_T(", "));
				m_list.SetItemText(index, 1, ext);
			}
			m_list.SetItemText(index, 2, item->m_strName);
			m_list.SetItemText(index, 3, item->m_strCmd);
		}
		array.RemoveAll();
	}

	if (sel >= 0) {
		m_list.SetItemState(sel, LVIS_SELECTED | LVIS_FOCUSED, LVIS_SELECTED | LVIS_FOCUSED);
		m_list.EnsureVisible(sel, FALSE);
		TCHAR buf[MAX_PATH];
		m_list.GetItemText(sel, 0, buf, ARRAYSIZE(buf));
		SetDlgItemText(IDC_TITLE, buf);
	}
}

void CAssocDlg::OnGetDispInfo(LPNMHDR pnmh, LRESULT* pResult)
{
	LV_DISPINFO* plvdi  = (LV_DISPINFO*)pnmh;
	if (plvdi && plvdi->item.mask & LVIF_IMAGE) {
		plvdi->item.iImage = -1;
		CRegItem* item = (CRegItem*)plvdi->item.lParam;
		if (item) {
			if (!item->m_strIcon.IsEmpty()) {
				TCHAR buf[MAX_PATH * 2];
				lstrcpy(buf,item->m_strIcon);
				int index = PathParseIconLocation(buf);

				int (WINAPI* pfn)(LPCWSTR, int, UINT);
				#ifdef Shell_GetCachedImageIndex
				#undef Shell_GetCachedImageIndex
				#endif
				pfn = Shell_GetCachedImageIndex;
				plvdi->item.iImage = item->m_iIcon = (*pfn)(buf, index, FALSE);
			}
		}
		plvdi->item.mask |= LVIF_DI_SETITEM;
	}
}

void CAssocDlg::OnItemChanged(LPNMHDR pnmh, LRESULT* pResult)
{
	NM_LISTVIEW* pnmlv = (NM_LISTVIEW*)pnmh;
	CRegItem* item = (CRegItem*)m_list.GetItemData(pnmlv->iItem);
	if (item)
		SetDlgItemText(IDC_APP, item->m_strProgId);
	else
		SetDlgItemText(IDC_APP, NULL);
}

void CAssocDlg::OnDeleteItem(LPNMHDR pnmh, LRESULT* pResult)
{
	NM_LISTVIEW* pnmlv = (NM_LISTVIEW*)pnmh;
	CRegItem* item = (CRegItem*)m_list.GetItemData(pnmlv->iItem);
	if (item)
		delete item;
}

//////////////////////////////////////////////////////////////////////
//

extern "C"
void WINAPI fopAssociateExtension(HWND hwnd, LPCTSTR pszFileName)
{
	LPTSTR p = NULL;
	if (pszFileName && *pszFileName) {
		p = PathFindExtension(pszFileName);
		if (*p == '.')
			p++;
		else
			p = NULL;
	}
	CAssocDlg assoc;
	assoc.DoModal(hwnd, (LPARAM)p);
}
