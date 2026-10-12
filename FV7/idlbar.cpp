#include "stdafx.h"
#include "fv7.h"
#include "idl_func.h"
#include "trackmenuwnd.h"

#ifndef __IDLBAR_H__
	#include "idlbar.h"
#endif


void XmlWriteIDListCore(TiXmlElement* pElement, LPCITEMIDLIST pidl, LPCTSTR pszName);
LPITEMIDLIST XmlLoadIDListCore(TiXmlElement* pElement, CString& str);

/////////////
//

static const TCHAR sszListFileName[] = _T("smartbar.itemlist");

typedef struct tagIDLITEM {
	DWORD dwOpenMode;
	TCHAR szIconLocation[MAX_PATH];
	DWORD dwItemSize;
	LPITEMIDLIST GetIDListPtr() { return (LPITEMIDLIST)((BYTE*)this + sizeof(IDLITEM)); }
} IDLITEM;

#define IDLMAGIC				0x8888FFFF

typedef struct tagIDLREG {
	DWORD magic;
	DWORD dwTotalSize;
	DWORD dwEntry;
	IDLITEM ii;
} IDLREG;

#define COMMANDID_BASE			IDM_SMARTBAR_IDSTART
#define MAX_SMART_ITEMS			(IDM_SMARTBAR_IDEND - IDM_SMARTBAR_IDSTART + 1)

const int cxyBtnMargin = 7;

/////////////////////////////////////////////////////////////////////////////
//


CIdlPane::CIdlPane()
{
	m_iIcon = -1;
	m_attr = 0;
	m_opmode = IDL_OPEN;
}

CIdlPane::CIdlPane(LPCITEMIDLIST pidl, DWORD opmode)
{
	m_iIcon = -1;
	m_idl.Clone(pidl);
	m_attr = m_idl.GetAttributes(SFGAO_FOLDER | SFGAO_CAPABILITYMASK);
	m_opmode = opmode;
}

CIdlPane::CIdlPane(LPCTSTR path, DWORD opmode)
{
	m_iIcon = -1;
	m_idl.CreateFromPath(path);
	m_attr = m_idl.GetAttributes(SFGAO_FOLDER | SFGAO_CAPABILITYMASK);
	m_opmode = opmode;
}

void CIdlPane::ParseIcon()
{
	m_iIcon = -1;
	if (!m_strIconLocation.IsEmpty()) {
		TCHAR buf[MAX_PATH * 2];
		lstrcpy(buf, m_strIconLocation);
		int index = PathParseIconLocation(buf);
		m_iIcon = Shell_GetCachedImageIndex(buf, index, FALSE);
	}
	else
		m_iIcon = idl_GetSysIconIndex(m_idl);
}

LPCTSTR CIdlPane::GetDisplayName()
{
	if (m_strDisplayName.IsEmpty()) {
		TCHAR buf[MAX_PATH];
		if (m_idl.GetPath(buf)) {
			CFileVersion ver;
			if (ver.Open(buf)) {
				ver.GetFileDescription(m_strDisplayName);
				ver.Close();
			}
		}
		if (m_strDisplayName.IsEmpty()) {
			SHFILEINFO sfi;
			m_idl.GetFileInfo(0, &sfi, sizeof(sfi), SHGFI_DISPLAYNAME);
			m_strDisplayName = sfi.szDisplayName;
		}
	}
	return m_strDisplayName.GetString();
}

/////////////////////////////////////////////////////////////////////////////
// CIdlBar attribute access

CIdlPane* CIdlBar::GetItemData(INT_PTR index)
{
	int cnt = (int)m_array.GetCount();
	if (cnt > 0 && index >= 0 && index < cnt)
		return m_array[index];
	return NULL;
}

void CIdlBar::WepAllItems()
{
	int cnt = (int)m_array.GetCount();
	for (int i = 0; i < cnt; i++) {
		CIdlPane* pip = m_array[i];
		if (pip) delete pip;
	}
	m_array.RemoveAll();
}

int CIdlBar::OnCreate(LPCREATESTRUCT lpCreateStruct) 
{
	if (DefWindowProc() == -1)
		return -1;

	SetExtendedStyle(TBSTYLE_EX_DRAWDDARROWS);
	SetNotifyWnd(m_hWnd);
	InitButtonImages();

	return 0;
}

void CIdlBar::OnRButtonDown(UINT flags, CPoint point)
{
	int index = HitTest(&point);
	CIdlPane* pip = GetItemData(index);
	if (!pip) return;

	SetDropDownProcessing(true);

	CRect rc;
	GetItemRect(index, &rc);

	CMenu menu;
	menu.LoadMenu(ID_MENU_SMARTBAR);
	CMenuHandle pop = menu.GetSubMenu(0);

	// オープンモード
	UINT idCommand = 0;
	switch (pip->OpenMode()) {
		case IDL_OPEN:			idCommand = IDM_SMART_OPEN; break;
		case IDL_OPENBYFV:		idCommand = IDM_SMART_OPENBYFV; break;
		case IDL_OPENTAB:		idCommand = IDM_SMART_OPENTAB; break;
		case IDL_OPENNEWFRAME:	idCommand = IDM_SMART_OPENNEWFRAME; break;
	}
	if (idCommand) {
		pop.CheckMenuItem(idCommand, MF_BYCOMMAND | MF_CHECKED);
	}

	// フォルダモード
	if (pip->isFolderMenu()) {
		pop.CheckMenuItem(IDM_SMART_MENU, MF_BYCOMMAND | MF_CHECKED);
	}
	if (pip->isFolderRetrieve()) {
		pop.CheckMenuItem(IDM_SMART_TREEMENU, MF_BYCOMMAND | MF_CHECKED);
	}
	// フォルダ以外はメニューを調整
	if (!(pip->m_attr & SFGAO_FOLDER)) {
		pop.EnableMenuItem(IDM_SMART_OPENBYFV, MF_GRAYED);
		pop.EnableMenuItem(IDM_SMART_OPENTAB, MF_GRAYED);
		pop.EnableMenuItem(IDM_SMART_OPENNEWFRAME, MF_GRAYED);
		pop.EnableMenuItem(IDM_SMART_MENU, MF_GRAYED);
		pop.EnableMenuItem(IDM_SMART_TREEMENU, MF_GRAYED);
	}

	ClientToScreen(&rc);

	TBBUTTON tbb;
	UINT id = (GetButton(index, &tbb) ? tbb.idCommand : 0);
	PressButton(id, TRUE);
	CTrackMenuWnd wnd;
	int cmd = wnd.DoTrack(m_hWnd, pop, CPoint(rc.left, rc.bottom));
	PressButton(id, FALSE);

	menu.DestroyMenu();

	if (cmd > 0) {
		TBBUTTONINFO tbbi;
		tbbi.cbSize = sizeof(tbbi);

		if (cmd == IDM_SMART_PROPERTY)
			InvokeIDList(fvApp.wndMain, pip->m_idl, gszVerbProp);
		else if (cmd == IDM_SMART_DELETE) {
			if (AppMessage(IDS_QUERY_DELSMARTITEM, pip->GetDisplayName(), MB_QYESNO) == IDYES) {
				m_array.RemoveAt(index);
				delete pip;
				BuildButtons(true);
			}
		}
		else if (cmd == IDM_SMART_EDIT) {
			Customize();
		}
		else if (cmd == IDM_SMART_CHANGEICON) {
			// アイコンの変更
			TCHAR name[MAX_PATH * 2];
			lstrcpy(name, pip->m_strIconLocation);
			int iImage = PathParseIconLocation(name);
			if (fopGetIconPath(fvApp.wndMain, name, &iImage)) {
				if (name[0] && iImage >= 0)
					pip->m_strIconLocation.Format(_T("\"%s\",%d"), name, iImage);
				else
					pip->m_strIconLocation.Empty();
				pip->ParseIcon();

				tbbi.dwMask = TBIF_IMAGE;
				tbbi.iImage = pip->m_iIcon;
				SetButtonInfo(id, &tbbi);

				SaveItems();		// 保存
			}
		}
		else if (cmd == IDM_SMART_HELP) {
			fvApp.HelpTopic(NULL, HB_CTL, IDW_SMARTBAR);
		}
		else {
			switch (cmd) {
			case IDM_SMART_OPEN:
				pip->m_opmode &= ~IDL_OPENMASK;
				break;
			case IDM_SMART_OPENBYFV:
				pip->m_opmode &= ~IDL_OPENMASK;
				pip->m_opmode |= IDL_OPENBYFV;
				break;
			case IDM_SMART_OPENTAB:
				pip->m_opmode &= ~IDL_OPENMASK;
				pip->m_opmode |= IDL_OPENTAB;
				break;
			case IDM_SMART_OPENNEWFRAME:
				pip->m_opmode &= ~IDL_OPENMASK;
				pip->m_opmode |= IDL_OPENNEWFRAME;
				break;
			case IDM_SMART_MENU:
				if (pip->isFolderMenu())
					pip->m_opmode &= ~IDL_FOLDERMENU;
				else
					pip->m_opmode = (pip->m_opmode & ~IDL_FOLDERRETRIEVE) | IDL_FOLDERMENU;
				break;
			case IDM_SMART_TREEMENU:
				if (pip->isFolderRetrieve())
					pip->m_opmode &= ~IDL_FOLDERRETRIEVE;
				else
					pip->m_opmode = (pip->m_opmode & ~IDL_FOLDERMENU) | IDL_FOLDERRETRIEVE;
				break;
			}

			tbbi.dwMask = TBIF_STYLE;
			tbbi.fsStyle = BTNS_AUTOSIZE | (pip->isTreatMenu() ? BTNS_DROPDOWN : BTNS_BUTTON);
			SetButtonInfo(id, &tbbi);
			SaveItems();		// 保存
		}
	}

	SetDropDownProcessing(false);
}

void CIdlBar::OnToolTipText(int id, NMHDR* pNMHDR, LRESULT* pResult)
{
	*pResult = 0;
	if (GetCapture()) return;

	CIdlPane* pip = GetItemData(pNMHDR->idFrom - COMMANDID_BASE);
	if (pip) {
		if (pip->m_strInfoTip.IsEmpty()) {
			CString it;
			pip->m_strInfoTip = pip->GetDisplayName();
			if (idl_GetInfoTip(m_hWnd, pip->m_idl, it) > 0)
				pip->m_strInfoTip.AppendFormat(_T("\n%s"), it.GetString());
		}

		NMTTDISPINFOW* ptdi = (NMTTDISPINFO*)pNMHDR;
		if (pip->m_strInfoTip.GetLength() < ARRAYSIZE(ptdi->szText))
			lstrcpy(ptdi->szText, pip->m_strInfoTip);
		else
			ptdi->lpszText = (LPTSTR)pip->m_strInfoTip.GetString();

		// bring the tooltip window above other popup windows
		CToolTipCtrl tip = pNMHDR->hwndFrom;
		tip.SetMaxTipWidth(MY_INFOTIP_LEN);
	}
}


LRESULT CIdlBar::OnHandlerTBGetButtonInfo(UINT msg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	// シェブロンメニュー対応：altframe.h内でメニュー構築の際に呼び出される
	// ボタン名称のみを取得し、それ以外の情報は元TOOLBARTCTRLで処理させる。
	// そのため、ボタン名称取得後にTBIF_TEXTフラグを落とし、bHandledをFALSEにして戻る。
	TBBUTTONINFO* ptbbi = (TBBUTTONINFO*)lParam;
	if (ptbbi && ptbbi->dwMask & TBIF_TEXT) {
		CIdlPane* pip = GetItemData(wParam - COMMANDID_BASE);
		if (pip) {
			lstrcpyn(ptbbi->pszText, pip->GetDisplayName(), ptbbi->cchText);
			ptbbi->dwMask &= ~TBIF_TEXT;
		}
	}
	bHandled = FALSE;
	return 0;
}

void CIdlBar::OnNotifyQueryInsert(NMHDR* pNmhdr, LRESULT* pResult)
{
	*pResult = TRUE;
}

void CIdlBar::OnNotifyQueryDelete(NMHDR* pNmhdr, LRESULT* pResult)
{
	*pResult = TRUE;
}

void CIdlBar::OnNotifyGetButtonInfo(NMHDR* pNmhdr, LRESULT* pResult)
{
	*pResult = FALSE;
	NMTOOLBAR* ptb = (NMTOOLBAR*)pNmhdr;
	int cnt = (int)m_array.GetCount();
	if (ptb->iItem < cnt) {
		CIdlPane* pip = m_array[ptb->iItem];
		memset(&ptb->tbButton, 0, sizeof(ptb->tbButton));
		ptb->tbButton.fsState = TBSTATE_ENABLED;
		ptb->tbButton.iString = -1;
		if (pip) {
			ptb->tbButton.iBitmap = pip->m_iIcon;
			ptb->tbButton.idCommand = ptb->iItem + COMMANDID_BASE;
			ptb->tbButton.fsStyle = BTNS_BUTTON;
			if (pip->isTreatMenu()) {
				ptb->tbButton.fsStyle |= BTNS_DROPDOWN;
			}
			lstrcpyn(ptb->pszText, pip->GetDisplayName(), ptb->cchText - 1);
			ptb->pszText[ptb->cchText - 1] = 0;
		}
		else {
			ptb->tbButton.idCommand = ID_SEPARATOR;
			ptb->tbButton.fsStyle = TBSTYLE_SEP;
		}
		*pResult = TRUE;
	}
}

void CIdlBar::OnNotifyEndAdjust(NMHDR* pNmhdr, LRESULT* pResult)
{
	CAtlArray<CIdlPane*> array;

	int cnt = GetButtonCount();
	for (int i = 0; i < cnt; i++) {
		TBBUTTON tbb;
		GetButton(i, &tbb);
		if (tbb.fsStyle & TBSTYLE_SEP)
			array.Add(NULL);
		else {
			CIdlPane* pip = GetItemData(tbb.idCommand - COMMANDID_BASE);
			if (pip) {
				array.Add(pip);
				m_array.SetAt(tbb.idCommand - COMMANDID_BASE, NULL);
			}
		}
	}

	WepAllItems();
	m_array.Append(array);
	BuildButtons(true);
}

int CIdlBar::DoToolBarDropDown(TBNOTIFY* ptbn)
{
	int index = ptbn->iItem - COMMANDID_BASE;
	CIdlPane* pip = GetItemData(index);
	if (pip) {
		CPoint pt(ptbn->rcButton.left, ptbn->rcButton.bottom - 1);
		ClientToScreen(&pt);

		CMenu pop;
		pop.CreatePopupMenu();
		CCmWnd wnd(pop, pip->m_idl, pip->isFolderRetrieve());
		int idCmd = wnd.DoTrack(m_hWnd, pop, pt);
		if (idCmd > 0) {
			CCmItem* pci = wnd.FindItem(idCmd);
			if (pci) {
				int mode = (pci->GetAttributesOf() & SFGAO_FOLDER) ? pip->OpenMode() : IDL_OPEN;
				OpenItem(pci->m_idl, mode);
			}
		}
	}
	return TBDDRET_DEFAULT;
}

void CIdlBar::OpenItem(LPITEMIDLIST pidl, int mode)
{
	switch (mode) {
	case IDL_OPENBYFV:
		fvApp.OpenFolder(pidl);
		break;
	case IDL_OPENTAB:
		fvApp.OpenTabFolder(pidl, !_ISKEYDOWN(VK_CONTROL));
		break;
	case IDL_OPENNEWFRAME:
		ExecNewFrame(pidl, _T("/force"));
		break;
	default:
		InvokeIDList(fvApp.wndMain, pidl);
		break;
	}
}

BOOL CIdlBar::OnCommand(UINT id, UINT code, HWND hwnd)
{
	int index = id - COMMANDID_BASE;
	CIdlPane* pip = GetItemData(index);
	if (pip)
		OpenItem(pip->m_idl, pip->OpenMode());
	return TRUE;
}

DWORD CIdlBar::OnDragOver(IDataObject* pdo, DWORD keyState, DWORD dropEffect, POINT point)
{
	ScreenToClient(&point);

	// ドロップ可能パスを特定する
	CRect rc;
	int posNew = HitTest(&point);
	if (posNew >= 0)
		GetItemRect(posNew, &rc);

	int indexHot = GetHotItem();
	TBINSERTMARK im;
	GetInsertMark(&im);

	CString str;
	// ALTキーまたは(Ctrl+Shift)キーが押されていれば、追加モード
	bool bAppend = _ISKEYDOWN(VK_MENU) || (_ISKEYDOWN(VK_CONTROL) && _ISKEYDOWN(VK_SHIFT));
	if (bAppend) {
		// 旧位置のドロップ待機状態を解除
		if (indexHot >= 0) {
			SetHotItem(-1);
		}

		im.dwFlags = 0;
		if (posNew < 0) {
			posNew = min(-posNew, GetButtonCount() - 1);
			im.dwFlags = TBIMHT_AFTER;
		}
		im.iButton = posNew;
		SetInsertMark(&im);

		str.LoadString(IDW_SMARTBAR);
		SetDropTargetDescription(pdo, str, DROPEFFECT_LINK);
		return DROPEFFECT_LINK;
	}

	im.iButton = -1;
	SetInsertMark(&im);

	if (posNew > 0) {
		if (GetItemData(posNew) == NULL)
			posNew = -1;
	}

	// ドロップ位置に変化がある場合
	if (posNew != indexHot) {
		// 新しいドロップ位置を設定
		SetHotItem(posNew);
	}

	CIdlPane* pip = GetItemData(posNew);
	if (pip) {
		DWORD eff = idl_GetDropEffect(m_hWnd, pip->m_idl, pdo, keyState, dropEffect);
		SetDropTargetDescription(pdo, pip->GetDisplayName(), eff);
		return eff;
	}
	return DROPEFFECT_NONE;
}

void CIdlBar::OnDragLeave()
{
	int indexHot = GetHotItem();
	TBINSERTMARK im;
	GetInsertMark(&im);

	// ドロップ待機状態になっているパスを元に戻す
	if (indexHot >= 0)
		SetHotItem(-1);
	if (im.iButton >= 0) {
		im.iButton = -1;
		SetInsertMark(&im);
	}
}

DWORD CIdlBar::OnDrop(IDataObject* pdo, DWORD keyState, DWORD dropEffect, POINT point)
{
	int indexHot = GetHotItem();
	TBINSERTMARK im;
	GetInsertMark(&im);

	fvApp.wndMain.SetWindowPos(HWND_TOP, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);
	::SetForegroundWindow(fvApp.wndMain);

	DROPEFFECT eff = DROPEFFECT_NONE;
	CIdlPane* pip;
	if (im.iButton >= 0) {
		CSimpleArray<PCIDLIST_ABSOLUTE> array;
		int cnt = GetIDListsFromDataObject(pdo, array);
		if (cnt > 0) {
			// ShellIDList形式のデータ
			for (int i = 0; i < cnt; i++) {
				PCIDLIST_ABSOLUTE pidl = array[i];
				pip = new CIdlPane(pidl);
				if (pip->isFolder())
					pip->m_opmode = IDL_OPENBYFV;
				m_array.InsertAt(im.iButton + ((im.dwFlags == TBIMHT_AFTER) ? 1 : 0), pip);
				CoTaskMemFree((void*)pidl);
			}
			BuildButtons(true);
		}
	}
	else if (indexHot >= 0) {
		pip = GetItemData(indexHot);
		eff = idl_Execute(m_hWnd, pip->m_idl, pdo, keyState, dropEffect, point);
	}
	return eff;
}

static const GUID ssgSysFolders[] = {
	FOLDERID_DocumentsLibrary,
	FOLDERID_Downloads,
	FOLDERID_Pictures,
	FOLDERID_Music,
	FOLDERID_Videos,
	GUID_NULL
};
static const TCHAR* sszSysTools[] = {
	_T("calc.exe"),
	_T("mspaint.exe"),
	_T("notepad.exe"),
	_T("wmplayer.exe"),
	NULL
};

void CIdlBar::Refresh()
{
	int cnt = (int)m_array.GetCount();
	for (int i = 0; i < cnt; i++) {
		CIdlPane* pip = m_array[i];
		if (pip) pip->ParseIcon();
	}
	Invalidate(FALSE);
}

HBITMAP CIdlBar::GetMenuImageBitmap(UINT id)
{
	CIdlPane* pip = m_array.GetAt(id - COMMANDID_BASE);
	if (pip->m_iIcon >= 0 && pip->m_bmIcon.IsNull()) {
		HIMAGELIST himL, himS;
		Shell_GetImageLists(&himL, &himS);
		pip->m_bmIcon.Attach(CreateBitmapFromImageList(himS, pip->m_iIcon));
	}
	return (pip != NULL) ? pip->m_bmIcon.m_hBitmap : NULL;
}

void CIdlBar::ChangeButtonImageSize()
{
	InitButtonImages();
	BuildButtons();
}

void CIdlBar::InitButtonImages()
{
	RemoveCustomDrawBar(m_hWnd);

	// ボタンサイズなど
	SetButtonStructSize(sizeof(TBBUTTON));
	// 全ボタン削除
	DeleteAllButtons();

	HIMAGELIST hims, himl;
	Shell_GetImageLists(&himl, &hims);
	m_himSystem = (gnBarStat & BTS_LARGEICON) ? himl : hims;
	int ix, iy;
	if (!ImageList_GetIconSize(m_himSystem, &ix, &iy)) {
		ix = iy = (gnBarStat & BTS_LARGEICON) ? 32 : 16;
	}
	m_sizeIcon = CSize(ix, iy);
	SetBitmapSize(ix, iy);
	SetButtonSize(ix + cxyBtnMargin, iy + cxyBtnMargin);
	SetImageList(m_himSystem);

	AddCustomDrawBar(m_hWnd);
}

void CIdlBar::DeleteAllButtons()
{
	// 全ボタン削除
	int cnt = GetButtonCount();
	while (cnt-- > 0)
		DeleteButton(0);
}

void CIdlBar::BuildButtons(bool needSerialize)
{
	// 全ボタン削除
	DeleteAllButtons();
	// ツールバーへ追加する
	int cnt = (int)m_array.GetCount();
	if (cnt > 0) {
		cnt = min(cnt, MAX_SMART_ITEMS);
		TBBUTTON* tbb = new TBBUTTON[cnt];
		memset(tbb, 0, sizeof(TBBUTTON) * cnt);

		bool bStyleList = ((GetStyle() & TBSTYLE_LIST) != 0);

		TBBUTTON* pt = tbb;
		int i;
		for (i = 0; i < cnt; i++) {
			pt->fsState = TBSTATE_ENABLED;
			CIdlPane* pip = m_array[i];
			if (pip) {
				if (pip->m_iIcon < 0)
					pip->ParseIcon();

				pt->idCommand = i + COMMANDID_BASE;
				pt->iBitmap = pip->m_iIcon;
				pt->fsStyle = BTNS_BUTTON;
				if (pip->isTreatMenu())
					pt->fsStyle |= BTNS_DROPDOWN;
				if (bStyleList) {
					pt->fsStyle = BTNS_AUTOSIZE;
					if (pip->isShowLabel())
						pt->iString = (INT_PTR)pip->GetDisplayName();
				}
			}
			else {
				pt->idCommand = ID_SEPARATOR;
				pt->iBitmap = 6;
				pt->fsStyle = BTNS_SEP;
			}
			pt++;
		}
		AddButtons(cnt, tbb);
		delete [] tbb;
	}
	// 保存
	if (needSerialize)
		SaveItems();
}

bool CIdlBar::SaveItems()
{
	CString dbname;
	GetAppDataFolder(dbname, sszListFileName);

	char ssz[MAX_PATH * 2];
	// Save XML
	TiXmlDocument xmlDoc;
	xmlDoc.InsertEndChild( TiXmlDeclaration( "1.0", "UTF-8", "yes" ) );

	// --------------------------
	// XML directory header
	// --------------------------
	TiXmlNode* pDoc = xmlDoc.InsertEndChild( TiXmlElement( "smartitems" ) );
	TiXmlElement* pRoot = pDoc->ToElement();

	// XML item
	TiXmlElement* pElement;
	int cnt = (int)m_array.GetCount();
	for (int i = 0; i < cnt; i++) {
		CIdlPane* pip = m_array.GetAt(i);
		TiXmlNode* pItem = pRoot->InsertEndChild( TiXmlElement( "item" ) );
		pElement = pItem->ToElement();
		if (pip != NULL) {
			XmlWriteIDListCore(pElement, pip->m_idl, pip->GetDisplayName());
			// アイコンパス
			if (!pip->m_strIconLocation.IsEmpty()) {
				// utf-8で記録
				WideCharToMultiByte(CP_UTF8, 0, pip->m_strIconLocation, -1, ssz, ARRAYSIZE(ssz), NULL, NULL);
				pElement->SetAttribute( "iconPath", ssz );
			}
			// 操作モード
			pElement->SetAttribute( "opmode", pip->m_opmode );
		}
		else {
			// セパレータ
			pElement->SetAttribute( "name", "separator" );
		}
	}
	// Save XML
	WideCharToMultiByte(CP_ACP, 0, dbname, -1, ssz, ARRAYSIZE(ssz), NULL, NULL);
	// 書き出し可能か？
	if (!CheckFileWritable(ssz)) {
		;	// とりあえずreadonly属性がついていれば解除される
	}
	return xmlDoc.SaveFile(ssz);
}

void CIdlBar::LoadItems()
{
	CString dbname;
	GetAppDataFolder(dbname, sszListFileName);

	char xml[MAX_PATH * 2];
	WideCharToMultiByte(CP_ACP, 0, dbname, -1, xml, ARRAYSIZE(xml), NULL, NULL);

	CIdlPane* pip;
	TiXmlDocument xmlDoc;
	if (xmlDoc.LoadFile( xml )) {
		const char* psz;
		WCHAR wsz[MAX_PATH * 2];
		TiXmlNode* pDoc = xmlDoc.ToDocument();
		if (pDoc) {
			TiXmlElement* pRoot = pDoc->FirstChildElement("smartitems");
			if (pRoot) {
				TiXmlElement* pItem = pRoot->FirstChildElement("item");
				while (pItem) {
					CString str;
					LPITEMIDLIST pidl = XmlLoadIDListCore(pItem, str);
					if (pidl) {
						// 操作モード
						int mode;
						if (pItem->QueryIntAttribute("opmode", &mode) != TIXML_SUCCESS) mode = IDL_OPEN;
						pip = new CIdlPane(pidl, mode);
						// 名前
						pip->m_strDisplayName = str;
						// アイコンパス
						psz = pItem->Attribute("iconPath");
						if (psz) {
							// utf-8を戻す
							MultiByteToWideChar(CP_UTF8, 0, psz, -1, wsz, ARRAYSIZE(wsz));
							pip->m_strIconLocation = wsz;
						}
						m_array.Add(pip);
						::CoTaskMemFree(pidl);
					}
					else {
						// セパレータ
						m_array.Add(NULL);
					}
					pItem = pItem->NextSiblingElement("item");
				}
			}
		}
	}
	else {				// 対応するファイルがない == 初めて起動
		CIDListPtr idl;
		const GUID* guids = ssgSysFolders;
		while (*guids != GUID_NULL) {
			if (idl.GetKnownFolder(*guids) == S_OK) {
				pip = new CIdlPane(idl, IDL_OPENBYFV);
				m_array.Add(pip);
				idl.Free();
			}
			guids++;
		}
		// セパレータ
		if (m_array.GetCount() > 0) m_array.Add(NULL);

		CString strSysDir;
		GetSystemDirectory(strSysDir.GetBuffer(MAX_PATH), MAX_PATH);
		strSysDir.ReleaseBuffer();

		TCHAR buf[MAX_PATH * 2];
		LPCTSTR* tools = sszSysTools;
		while (*tools) {
			// windirをチェック
			PathCombine(buf, _GCDATA.m_strWinDir, *tools);
			if (!PathFileExists(buf)) {
				// system32をチェック
				PathCombine(buf, strSysDir, *tools);
				if (!PathFileExists(buf)) {
					// レジストリのApp Pathsをチェック
					if (!lccRegGetAppPath(*tools, buf, ARRAYSIZE(buf)))
						lstrcpy(buf, *tools);
				}
			}
			pip = new CIdlPane(buf);
			if (!pip->isNull())
				m_array.Add(pip);
			else
				delete pip;
			tools++;
		}
/*
		// セパレータ
		m_array.Add(NULL);
		// スマートバー説明
		LoadString(RESINST, IDS_SMARTBAR_HELP, buf, ARRAYSIZE(buf));
		CString name;
		fvApp.MakeHelpLink(buf, _T("/operation/panel/smartbar.htm"), name.GetBuffer(512), 512);
		pip = new CIdlPane(name.GetString());
		if (!pip->isNull())
			m_array.Add(pip);
		else
			delete pip;
*/
	}
	BuildButtons();
}

