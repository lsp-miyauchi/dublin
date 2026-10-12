////////////////////////////////////////////////////////////////
// hotkeymenu.cpp
//
#include "stdafx.h"
#include "fv7.h"

////////////////////////////////////////////////////////////////

void BuildHotKeyFileName(CString& str)
{
	GetAppDataFolder(str, _T("filevisor7.hotkey"));
}

bool SaveHotKey(CHotKeyItem** pKeys, LPCTSTR pszHotKeyFileName)
{
	char ssz[MAX_PATH * 2];
	// Save XML
	TiXmlDocument xmlDoc;
	xmlDoc.InsertEndChild( TiXmlDeclaration( "1.0", "UTF-8", "yes" ) );

	// --------------------------
	// XML directory header
	// --------------------------
	TiXmlNode* pDoc = xmlDoc.InsertEndChild( TiXmlElement( "hotkey" ) );
	TiXmlElement* pRoot = pDoc->ToElement();

	// XML item
	TiXmlElement* pElement;
	for (int i = 0; i < 26; i++) {
		CHotKeyItem* pck = pKeys[i];
		if (pck) {
			TiXmlNode* pItem = pRoot->InsertEndChild( TiXmlElement( "item" ) );
			pElement = pItem->ToElement();
			// key
			pElement->SetAttribute( "key", pck->m_key );
			// hotkey-type
			pElement->SetAttribute( "type", pck->m_type );
			// name
			if (!pck->m_name.IsEmpty()) {
				WideCharToMultiByte(CP_UTF8, 0, pck->m_name, -1, ssz, ARRAYSIZE(ssz), NULL, NULL);
				pElement->SetAttribute( "name", ssz );
			}
			// command
			if (!pck->m_exec.IsEmpty()) {
				WideCharToMultiByte(CP_UTF8, 0, pck->m_exec, -1, ssz, ARRAYSIZE(ssz), NULL, NULL);
				pElement->SetAttribute( "command", ssz );
			}
			// param
			if (!pck->m_param.IsEmpty()) {
				WideCharToMultiByte(CP_UTF8, 0, pck->m_param, -1, ssz, ARRAYSIZE(ssz), NULL, NULL);
				pElement->SetAttribute( "param", ssz );
			}
			// workdir
			if (!pck->m_workdir.IsEmpty()) {
				WideCharToMultiByte(CP_UTF8, 0, pck->m_workdir, -1, ssz, ARRAYSIZE(ssz), NULL, NULL);
				pElement->SetAttribute( "workdir", ssz );
			}
			// asadmin
			pElement->SetAttribute( "asadmin", pck->m_bAsAdmin ? 1 : 0 );
			// editable
			pElement->SetAttribute( "editable", pck->m_bEdit ? 1 : 0 );
		}
	}
	// Save XML
	WideCharToMultiByte(CP_ACP, 0, pszHotKeyFileName, -1, ssz, ARRAYSIZE(ssz), NULL, NULL);
	// 書き出し可能か？
	if (!CheckFileWritable(ssz)) {
		;	// とりあえずreadonly属性がついていれば解除される
	}
	return xmlDoc.SaveFile(ssz);
}

int LoadHotKey(CHotKeyItem** pKeys, LPCTSTR pszHotKeyFileName)
{
	char xml[MAX_PATH * 2];
	WideCharToMultiByte(CP_ACP, 0, pszHotKeyFileName, -1, xml, ARRAYSIZE(xml), NULL, NULL);

	TiXmlDocument xmlDoc;
	if (!xmlDoc.LoadFile( xml )) return 0;

	int entries = 0;
	WCHAR wsz[MAX_PATH * 2];
	int val;
	const char* psz;
	TiXmlNode* pDoc = xmlDoc.ToDocument();
	if (pDoc) {
		TiXmlElement* pRoot = pDoc->FirstChildElement("hotkey");
		if (pRoot) {
			TiXmlElement* pItem = pRoot->FirstChildElement("item");
			while (pItem) {
				// key
				if (pItem->Attribute("key", &val) != NULL) {
					CHotKeyItem* pck = new CHotKeyItem;
					pKeys[val] = pck;
					pck->m_key = val;
					// hotkey-type
					if (pItem->Attribute("type", &val) == NULL)
						val = HOTKEY_UNKNOWN;
					pck->m_type = val;
					// name
					psz = pItem->Attribute("name");
					if (psz) {
						MultiByteToWideChar(CP_UTF8, 0, psz, -1, wsz, ARRAYSIZE(wsz));
						pck->m_name = wsz;
					}
					// command
					psz = pItem->Attribute("command");
					if (psz) {
						MultiByteToWideChar(CP_UTF8, 0, psz, -1, wsz, ARRAYSIZE(wsz));
						pck->m_exec = wsz;
					}
					// param
					psz = pItem->Attribute("param");
					if (psz) {
						MultiByteToWideChar(CP_UTF8, 0, psz, -1, wsz, ARRAYSIZE(wsz));
						pck->m_param = wsz;
					}
					// workdir
					psz = pItem->Attribute("workdir");
					if (psz) {
						MultiByteToWideChar(CP_UTF8, 0, psz, -1, wsz, ARRAYSIZE(wsz));
						pck->m_workdir = wsz;
					}
					// asadmin
					if (pItem->Attribute("asadmin", &val) == NULL)
						val = 0;
					pck->m_bAsAdmin = (val != 0);
					// editable
					if (pItem->Attribute("editable", &val) == NULL)
						val = 0;
					pck->m_bEdit = (val != 0);
					entries++;
				}
				pItem = pItem->NextSiblingElement("item");
			}
		}
	}
	return entries;
}

int RestoreHotKey(CHotKeyItem** pKeys)
{
	// 外部xmlファイルから読み込み
	CString str;
	BuildHotKeyFileName(str);
	if (_pathfileexists(str))
		return LoadHotKey(pKeys, str);

	// ファイルが無ければレジストリ
	int cnt = 0;
	CString key;
	for (int i = 0; i < 26; i++) {
		key.Format(_T("%c"), 'A' + i);
		if (GetRegText(IDS_SEC_HOTKEY, (LPCTSTR)key, str) > 0) {
			CHotKeyItem* pck = new CHotKeyItem;
			if (pck->Initialize(i, str)) {
				pKeys[i] = pck;
				cnt++;
			}
			else
				delete pck;
		}
	}
	return cnt;
}


////////////////////////////////////////////////////////////////
// class CHotKeyMenu
//

int CHotKeyMenu::GetPositionFromChar(UINT nChar)
{
	int index = ToUpper((TCHAR)nChar) - 'A';
	if (!m_pKey[index])
		return -1;

	int pos = 0;
	for (int i = 0; i < 26; i++) {
		if (i == index)
			return pos;
		if (m_pKey[i])
			pos++;
	}
	return -1;
}

BOOL CHotKeyMenu::Restore()
{
	Clear();
	m_nEntries = RestoreHotKey(m_pKey);
	return (m_nEntries > 0);
}

bool CHotKeyMenu::Save(LPCTSTR pszHotKeyFileName)
{
	return SaveHotKey(m_pKey, pszHotKeyFileName);
}

bool CHotKeyMenu::Load(LPCTSTR pszHotKeyFileName)
{
	Clear();
	m_nEntries = LoadHotKey(m_pKey, pszHotKeyFileName);
	return (m_nEntries > 0);
}

int CHotKeyMenu::DoTrack(HWND hWnd, CPoint pt)
{
	int nCmd = -1;
	if (m_nEntries > 0 && CreatePopupMenu()) {
		CHotKeyItem* pck;
		int cnt = 0;
		CImageList il = lccLoadImageList(RESINST, RT_RCDATA, _MKIR(IDB_KEYBOARD), KEYBD_X, RGB(255, 0, 255));
		CSimpleArray<HBITMAP> arrbm;		// delete after menu used
		for (int i = 0; i < 26; i++) {
			pck = m_pKey[i];
			if (pck) {
				UINT flag = MF_BYPOSITION;
				if (cnt == 13)
					flag |= MF_MENUBARBREAK;
				InsertMenu(cnt, flag, IDM_HOTKEY_FIRST + i, pck->m_name);
				HBITMAP hbm = CreateBitmapFromImageList(il, i + 10);
				if (hbm) {
					SetBitmapToMenuItem(m_hMenu, IDM_HOTKEY_FIRST + i, hbm);
					arrbm.Add(hbm);
				}
				cnt++;
			}
		}
		if (cnt > 0) {
			nCmd = ::TrackPopupMenu(m_hMenu, TPM_LEFTALIGN | TPM_RETURNCMD | TPM_LEFTBUTTON, pt.x, pt.y, 0, hWnd, NULL);
		}
		for (int i = 0; i < arrbm.GetSize(); i++) {
			::DeleteObject(arrbm[i]);
		}
		arrbm.RemoveAll();

		DestroyMenu();
	}
	return nCmd;
}

BOOL CHotKeyMenu::GetCommandLine(UINT nMenuID, CHotKeyItem& item)
{
	if (nMenuID < IDM_HOTKEY_FIRST || nMenuID > IDM_HOTKEY_LAST) return FALSE;

	CHotKeyItem* pck = m_pKey[nMenuID - IDM_HOTKEY_FIRST];
	if (!pck) return FALSE;

	item = *pck;
	return TRUE;
}

/////////////////////////////////////////////////////////////////////////////
// CHotKeyTrackWnd

void CHotKeyTrackWnd::DoTrack(HWND hwnd, CHotKeyMenu* pm, CPoint pt)
{
	m_pm = pm;
	Create(hwnd, 0, NULL, WS_CHILD);
	if (IsWindow()) {
		int cmd = m_pm->DoTrack(m_hWnd, pt);
		DestroyWindow();

		if (cmd > 0) ::PostMessage(hwnd, WM_COMMAND, cmd, 0);
	}
}

void CHotKeyTrackWnd::OnInitMenuPopup(HMENU hPopupMenu, UINT nIndex, BOOL bSysMenu)
{
	if (!bSysMenu) {
		MENUINFO mi;
		mi.cbSize = sizeof(mi);
		mi.fMask = MIM_BACKGROUND;
		mi.hbrBack = _GCDATA.GetMenuBackgroundBrush();
		::SetMenuInfo(hPopupMenu, &mi);
	}
}

LRESULT CHotKeyTrackWnd::OnMenuChar(UINT nChar, UINT nFlags, HMENU hMenu)
{
	if (ISALPHA(nChar)) {
		int id = m_pm->GetPositionFromChar(nChar);
		if (id >= 0)
			return MAKELPARAM(id, 2);
	}
	return DefWindowProc();
}

/////////////////////////////////////////////////////////////////////////
//

BOOL CHotKeyExecPage::OnInit()
{
	// 管理者
	GetDlgItem(IDC_ASADMIN).EnableWindow(TRUE);
	SetDlgItemText(IDC_KEYNAME, m_itemptr->m_name);

	if (m_itemptr->m_type != HOTKEY_EXEC)
		return TRUE;

	SetDlgItemText(IDC_PROGRAM, m_itemptr->m_exec);
	SetDlgItemText(IDC_COMMAND, m_itemptr->m_param);
	SetDlgItemText(IDC_DEFDIR, m_itemptr->m_workdir);

	CheckDlgButton(IDC_CHECK, m_itemptr->m_bEdit);
	CheckDlgButton(IDC_ASADMIN, m_itemptr->m_bAsAdmin);
	return TRUE;
}

void CHotKeyExecPage::OnNotifyExeBrowse(NMHDR* phdr, LRESULT* pResult)
{
	NMSEGETTEXT* psgt = (NMSEGETTEXT*)phdr;

	CMemTask path;
	SHGetKnownFolderPath(FOLDERID_ProgramFiles, 0, NULL, (PWSTR*)&path);
	CString str;
	if (FileBrowse(TRUE, m_hWnd, IDS_FILTER_EXECUTE, str, path)) {
		CFileVersion ver;
		if (ver.Open(str)) {
			TCHAR buf[MAX_PATH];
			if (ver.GetFileDescription(buf, ARRAYSIZE(buf)))
				SetDlgItemText(IDC_KEYNAME, buf);
			ver.Close();
		}
		StrCpyN(psgt->pszText, str, psgt->cchTextMax);
		*pResult = TRUE;
	}
}

void CHotKeyExecPage::OnNotifyWorkingPathBrowse(NMHDR* phdr, LRESULT* pResult)
{
	NMSEGETTEXT* psgt = (NMSEGETTEXT*)phdr;

	CString str = psgt->pszText;

	CBrowseFolder bf;
	if (bf.BrowsePath(m_hWnd, str)) {
		lstrcpyn(psgt->pszText, str, psgt->cchTextMax);
		*pResult = TRUE;
	}
}

void CHotKeyExecPage::OnOK()
{
	CString tit, exec, cmd, dir;
	if (GetDlgItemText(IDC_KEYNAME, tit) < 1) { MessageBeep(0); GetDlgItem(IDC_KEYNAME).SetFocus(); return; }
	if (GetDlgItemText(IDC_PROGRAM, exec) < 1) { MessageBeep(0); GetDlgItem(IDC_PROGRAM).SetFocus(); return; }
	if (exec[0] == '"' && exec.Find(' ') >= 0) {
		exec.Trim(' ');
		exec.Insert(0, '"');
		exec += '"';
	}

	m_itemptr->m_type = HOTKEY_EXEC;
	m_itemptr->m_name = tit;
	m_itemptr->m_exec = exec;
	GetDlgItemText(IDC_COMMAND, m_itemptr->m_param);
	GetDlgItemText(IDC_DEFDIR, m_itemptr->m_workdir);
	m_itemptr->m_bEdit = (IsDlgButtonChecked(IDC_CHECK) != FALSE);
	m_itemptr->m_bAsAdmin = (IsDlgButtonChecked(IDC_ASADMIN) != FALSE);
}

/////////////////////////////////////////////////////////////////////////
//

BOOL CHotKeyFolderPage::OnInit()
{
	// 管理者
	GetDlgItem(IDC_ASADMIN).EnableWindow(TRUE);
	SetDlgItemText(IDC_KEYNAME, m_itemptr->m_name);

	if (m_itemptr->m_type != HOTKEY_FOLDER) {
		CheckDlgButton(IDC_JUMP, TRUE);
		return TRUE;
	}

	SetDlgItemText(IDC_FOLDER, m_itemptr->m_param);

	if (!m_itemptr->m_exec.CompareNoCase(_T("%newtab%")))
		CheckDlgButton(IDC_NEWTAB, TRUE);
	else if (!m_itemptr->m_exec.CompareNoCase(_T("%jump%")))
		CheckDlgButton(IDC_JUMP, TRUE);
	else
		CheckDlgButton(IDC_NEWWINDOW, TRUE);

	CheckDlgButton(IDC_ASADMIN, m_itemptr->m_bAsAdmin);
	return TRUE;
}

void CHotKeyFolderPage::OnNotifyPathBrowse(NMHDR* phdr, LRESULT* pResult)
{
	NMSEGETTEXT* psgt = (NMSEGETTEXT*)phdr;

	CString str = psgt->pszText;

	CBrowseFolder bf;
	if (bf.BrowsePath(m_hWnd, str)) {
		lstrcpyn(psgt->pszText, str, psgt->cchTextMax);
		*pResult = TRUE;
	}
}

void CHotKeyFolderPage::OnOK()
{
	CString tit, dir;
	if (GetDlgItemText(IDC_KEYNAME, tit) < 1) { MessageBeep(0); GetDlgItem(IDC_KEYNAME).SetFocus(); return; }
	if (GetDlgItemText(IDC_FOLDER, dir) < 1) { MessageBeep(0); GetDlgItem(IDC_FOLDER).SetFocus(); return; }

	m_itemptr->m_type = HOTKEY_FOLDER;
	m_itemptr->m_name = tit;
	m_itemptr->m_param = dir;

	if (IsDlgButtonChecked(IDC_NEWTAB))
		m_itemptr->m_exec = _T("%newtab%");
	else if (IsDlgButtonChecked(IDC_NEWWINDOW))
		m_itemptr->m_exec = _T("%newwindow%");
	else
		m_itemptr->m_exec = _T("%jump%");

	m_itemptr->m_workdir.Empty();
	m_itemptr->m_bEdit = FALSE;
	m_itemptr->m_bAsAdmin = (IsDlgButtonChecked(IDC_ASADMIN) != FALSE);
}


/////////////////////////////////////////////////////////////////////////
//

BOOL CHotKeySortPage::OnInit()
{
	SetDlgItemText(IDC_KEYNAME, m_itemptr->m_name);
	CComboBox comboType = GetDlgItem(IDC_SORT_TYPE);
	InitDlgCtrl(comboType, IDS_SORT_TYPE);
	CComboBox comboDirection = GetDlgItem(IDC_SORT_DIRECTION);
	InitDlgCtrl(comboDirection, IDS_SORT_DIRECTION);

	if (m_itemptr->m_type != HOTKEY_SORT) {
		comboType.SetCurSel(0);
		comboDirection.SetCurSel(0);
		return TRUE;
	}

	CSimpleArray<CString> arr;
	int cnt = SplitString(m_itemptr->m_param, _T(","), arr);
	if (cnt < 1 || comboType.SelectString(-1, arr[0]) == CB_ERR)
		comboType.SetCurSel(0);
	if (cnt < 2 || comboDirection.SelectString(-1, arr[1]) == CB_ERR)
		comboDirection.SetCurSel(0);

	return TRUE;
}

static LPCTSTR ss_sort[] = { _T("name"), _T("ext"), _T("size"), _T("time"), _T("attr"), _T("dos"), _T("none") };
static LPCTSTR ss_dir[]  = { _T("up"), _T("down") };

void CHotKeySortPage::OnOK()
{
	CString tit, cmd;
	if (GetDlgItemText(IDC_KEYNAME, tit) < 1) { MessageBeep(0); GetDlgItem(IDC_KEYNAME).SetFocus(); return; }
	m_itemptr->m_type = HOTKEY_SORT;
	m_itemptr->m_name = tit;
	m_itemptr->m_exec = _T("%sort%");

	CComboBox combo;
	combo = GetDlgItem(IDC_SORT_TYPE);
	try { m_itemptr->m_param = ss_sort[combo.GetCurSel()]; } catch(...) {}
	m_itemptr->m_param += _T(',');
	combo = GetDlgItem(IDC_SORT_DIRECTION);
	try { m_itemptr->m_param += ss_dir[combo.GetCurSel()]; } catch(...) {}

	m_itemptr->m_workdir.Empty();
	m_itemptr->m_bEdit = FALSE;
	m_itemptr->m_bAsAdmin = FALSE;
}

/////////////////////////////////////////////////////////////////////////
//

BOOL CHotKeyFilterPage::OnInit()
{
	SetDlgItemText(IDC_KEYNAME, m_itemptr->m_name);

	if (m_itemptr->m_type != HOTKEY_FILTER) {
		CheckDlgButton(IDC_FILTER, TRUE);
		return TRUE;
	}

	SetDlgItemText(IDC_WILDCARD, m_itemptr->m_param);

	if (!StrCmpNI(m_itemptr->m_exec, _T("%filter"), 7))
		CheckDlgButton(IDC_FILTER, TRUE);
	else if (!StrCmpNI(m_itemptr->m_exec, _T("%select"), 7))
		CheckDlgButton(IDC_SELECT, TRUE);

	BOOL bRegExp = (StrStrI(m_itemptr->m_exec, _T("regexp")) != NULL);
	CheckDlgButton(IDC_REGEXP, bRegExp);

	return TRUE;
}

void CHotKeyFilterPage::OnOK()
{
	CString tit, cmd;
	if (GetDlgItemText(IDC_KEYNAME, tit) < 1) { MessageBeep(0); GetDlgItem(IDC_KEYNAME).SetFocus(); return; }
	if (GetDlgItemText(IDC_WILDCARD, cmd) < 1) { MessageBeep(0); GetDlgItem(IDC_WILDCARD).SetFocus(); return; }

	m_itemptr->m_type = HOTKEY_FILTER;
	m_itemptr->m_name = tit;
	m_itemptr->m_param = cmd;

	m_itemptr->m_exec = IsDlgButtonChecked(IDC_FILTER) ? _T("%filter") : _T("%select");
	if (IsDlgButtonChecked(IDC_REGEXP)) m_itemptr->m_exec += _T("regexp");
	m_itemptr->m_exec += _T('%');

	m_itemptr->m_workdir.Empty();
	m_itemptr->m_bEdit = FALSE;
	m_itemptr->m_bAsAdmin = FALSE;
}
