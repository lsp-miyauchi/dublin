// mainfrm.cpp : implementation of the CMainFrame class
//

#include "stdafx.h"
#include <process.h>
#include "fv7.h"
#include "shellexec.h"
#include "mainfrm.h"
#include "setupsheet.h"
#include "optionsetup.h"
#include "wildcard.h"
#include "keymap.h"
#include "show-keymap.h"
#include "shortcut-menu.h"
#include "..\lsshell\lsshell_i.h"
#include "regwnd.h"
#include "rebarsetting.h"
#include "tabclose.h"
#include "updir_confirm.h"

#include "atten.cxx"

#ifdef _DEBUG
//	#define DUMP_SHELLNOTIFY			// ShellNotifyダンプ
#endif

#ifndef RBS_EX_TRANSPARENT
	#define RBS_EX_TRANSPARENT			0x0001
	#define RBS_EX_SPLITTER				0x0002
#endif

#define SWITCH_COMMAND(id, func) \
	if (id == LOWORD(wParam)) { \
		func(); \
		bHandled = TRUE; \
		return TRUE; \
	}
#define SWITCH_COMMAND_CODE(id, code, func) \
	if (id == LOWORD(wParam) && code == HIWORD(wParam)) { \
		func(); \
		bHandled = TRUE; \
		return TRUE; \
	}
#define SWITCH_COMMAND_EX(id, func) \
	if (id == LOWORD(wParam)) { \
		func((int)HIWORD(wParam)); \
		bHandled = TRUE; \
		return TRUE; \
	}
#define SWITCH_COMMAND_RANGE(idFirst, idLast, func) \
	if(LOWORD(wParam) >= idFirst  && LOWORD(wParam) <= idLast) { \
		func(LOWORD(wParam), HIWORD(wParam), (HWND)lParam); \
		bHandled = TRUE; \
		return TRUE; \
	}

//////////////////////////////////////////////////////////////
//

typedef int (__cdecl * QSORTCOMPPROC)(const void*, const void*, const void*);
void __cdecl qsort_ex(void *base, size_t n, size_t size, QSORTCOMPPROC, const void* param);
int EnumTabGroup(CCmItemArray& array);
bool CreateJumpList();

//////////////////////////////////////////////////////////////
//

#define MAXTABCOMBINE				16
#define XML_TABRECENT				_T("filevisor7.recent")

static const TCHAR sszFSS[] = _T("*.fss");
static const TCHAR sszSessionGroup[] = _T("Session.tabgroup");
static bool sbForceExit = false;

//////////////////////////////////////////////////////////////
//

typedef struct tagTABCOMBINEINFO {
	ICONLISTSTYLE ls;
	DWORD idlsize;
	BYTE idl[1];
} TABCOMBINEINFO;


//////////////////////////////////////////////////////////////
//

CMainFrame* gpMainFrame = NULL;


//////////////////////////////////////////////////////////////
//

const GUID FID_null = { 0, 0, 0, { 0, 0, 0, 0, 0, 0, 0, 0 } };
const GUID FID_separator = { 1, 0, 0, { 0, 0, 0, 0, 0, 0, 0, 0 } };
const GUID FID_submenu = { 2, 0, 0, { 0, 0, 0, 0, 0, 0, 0, 0 } };
const GUID FID_Documents = { 0x450d8fba, 0xad25, 0x11d0, { 0x98, 0xa8, 0x08, 0x00, 0x36, 0x1b, 0x11, 0x03 } };

static KNFL stKnownFolder[] = {
	FID_submenu,					IDS_SPFL_USERSFILES,
		FOLDERID_UsersFiles,			0,			// ユーザー\<username>
		FOLDERID_Contacts,				0,			// アドレス帳
		FOLDERID_Pictures,				0,
		FOLDERID_Music,					0,
		FOLDERID_Videos,				0,
		FOLDERID_Favorites,				0,
		FOLDERID_Downloads,				0,
		FOLDERID_Links,					0,
		FID_separator,					0,
		FOLDERID_SendTo,				IDS_CSIDL_SENDTO,
		FOLDERID_Recent,				0,
		FOLDERID_Templates,				IDS_CSIDL_TEMPLATES,
		FID_separator,					0,
		FOLDERID_RoamingAppData,		0,
		FOLDERID_LocalAppData,			0,
		FOLDERID_LocalAppDataLow,		0,
		FOLDERID_CDBurning,				0,
		FID_separator,					0,
		FID_submenu,					IDS_SPFL_MENUS,
			FOLDERID_StartMenu,				0,
			FOLDERID_Startup,				0,
			FOLDERID_Programs,				0,
			FOLDERID_AdminTools,			0,
			FID_null,						0,
		FID_null,						0,
	FID_submenu,					IDS_SPFL_ALLUSER,
		FOLDERID_PublicDesktop,			0,
		FOLDERID_Public,				0,
		FOLDERID_PublicLibraries,		0,
		FOLDERID_PublicDocuments,		0,
		FOLDERID_PublicPictures,		0,
		FOLDERID_PublicMusic,			0,
		FOLDERID_PublicVideos,			0,
		FOLDERID_PublicDownloads,		0,
		FID_separator,					0,
		FID_submenu,					IDS_SPFL_MENUS,
			FOLDERID_CommonStartMenu,		0,
			FOLDERID_CommonStartup,			0,
			FOLDERID_CommonPrograms,		0,
			FOLDERID_CommonAdminTools,		0,
			FID_null,						0,
		FID_null,						0,
	FID_separator,					0,
	FID_submenu,					IDS_SPFL_SYSTEM,
		FOLDERID_Windows,				0,
		FOLDERID_System,				0,
		FOLDERID_Fonts,					IDS_CSIDL_FONTS,
#ifdef _WIN64
		FOLDERID_SystemX86,				0,
#endif
		FOLDERID_ProgramFiles,			0,
#ifdef _WIN64
		FOLDERID_ProgramFilesX86,		0,
#endif
		FOLDERID_ProgramData,			0,
		FOLDERID_ResourceDir,			0,
		FID_null,						0,
};


/////////////////////////////////////////////////////////////////////////////
// ファイルペイン用ショートカットメニュー

static UINT snFileBrowserShortcutMenu[] = {
	IDM_VIEW_SORT,
	IDM_VIEW_ICON,
	ID_SEPARATOR,
	IDM_VIEW_SAETDEFAULT,
	ID_SEPARATOR,
	IDM_FILE_PACK,
	IDM_FILE_UNPACK,
	ID_SEPARATOR,
	IDM_FILE_SELALL,
	IDM_VIEW_WILDCARD,
	IDM_FILE_MASKSELECT,
	ID_SEPARATOR,
	IDM_FILE_SHELLNEW,
	IDM_FILE_PROPERTY
};

static UINT snFolderTreeShortcutMenu[] = {
	IDM_FILE_MKDIR,
	ID_SEPARATOR,
	IDM_DIR_SEARCH,
	ID_SEPARATOR,
	IDM_DIR_SYNC,
	ID_SEPARATOR,
	IDM_FILE_PROPERTY
};


/////////////////////////////////////////////////////////////////////////////
//

void WriteCmItemCore(TiXmlElement* pElement, CCmItem* pci);
CCmItem* LoadCmItemCore(TiXmlElement* pElement);

static void xmlwrite_tabitem(TiXmlElement* pItem, CCmItem* pci, const CIconListStyle* pils, BOOL bLocked)
{
	WriteCmItemCore(pItem, pci);
	// iconstyle
	if (pils) {
		char* data = b64Encode((const BYTE*)pils, sizeof(CIconListStyle));
		pItem->SetAttribute( "liststyle", data );
		free(data);
	}
	// lock
	pItem->SetAttribute( "locked", bLocked ? 1 : 0 );
	// last access time
	int t = (int)pci->GetTime();
	pItem->SetAttribute( "time", t );
}

static CCmItem* xmlload_tabitem(TiXmlElement* pItem, CIconListStyle* pils, BOOL* pbLocked)
{
	CCmItem* pci = LoadCmItemCore(pItem);
	if (pci) {
		// iconstyle
		if (pils) {
			int size;
			BYTE* data = b64Decode(pItem->Attribute("liststyle"), &size);
			if (data) {
				*pils = (const ICONLISTSTYLE*)data;
				free(data);
			}
		}
		// lock
		if (pbLocked) {
			int locked; if (pItem->QueryIntAttribute("locked", &locked) != TIXML_SUCCESS) locked = 0;
			*pbLocked = (locked != 0);
		}
		// time（値取得OKの場合のみセット）
		int t; if (pItem->QueryIntAttribute("time", &t) == TIXML_SUCCESS) pci->SetTime(t);
	}
	return pci;
}

/////////////////////////////////////////////////////////////////////////////
//

static bool SaveTabReopenArray(CTabReopenArray& array, int maxEntry)
{
	char ssz[MAX_PATH * 2];
	// Save XML
	TiXmlDocument xmlDoc;
	xmlDoc.InsertEndChild( TiXmlDeclaration( "1.0", "UTF-8", "yes" ) );

	// --------------------------
	// XML directory header
	// --------------------------
	TiXmlNode* pDoc = xmlDoc.InsertEndChild( TiXmlElement( "recentitems" ) );
	TiXmlElement* pRoot = pDoc->ToElement();

	// XML item
	CString str;
	int cnt = array.GetItemCount();
	if (maxEntry > 0)
		cnt = min(cnt, maxEntry);

	CIconListStyle ils;
	CCmItem* pci;
	for (int i = 0; i < cnt; i++) {
		array.GetStyleAt(i, ils);		// iconstyle
		pci = array.GetItemAt(i);
		// 書き出し
		TiXmlNode* pItem = pRoot->InsertEndChild( TiXmlElement( "item" ) );
		TiXmlElement* pElement = pItem->ToElement();
		xmlwrite_tabitem(pElement, pci, &ils, FALSE);
	}
	// Save XML
	GetAppDataFolder(str, XML_TABRECENT);
	WideCharToMultiByte(CP_ACP, 0, str, -1, ssz, ARRAYSIZE(ssz), NULL, NULL);
	// 書き出し可能か？
	if (!CheckFileWritable(ssz)) {
		;	// とりあえずreadonly属性がついていれば解除される
	}
	return xmlDoc.SaveFile(ssz);
}

int LoadTabReopenArray(CTabReopenArray& array, int maxEntry)
{
	CString str;
	GetAppDataFolder(str, XML_TABRECENT);
	char xml[MAX_PATH * 2];
	WideCharToMultiByte(CP_ACP, 0, str, -1, xml, ARRAYSIZE(xml), NULL, NULL);

	TiXmlDocument xmlDoc;
	if (!xmlDoc.LoadFile( xml )) return -1;

	int cnt = 0;
	TiXmlNode* pDoc = xmlDoc.ToDocument();
	if (pDoc) {
		TiXmlElement* pRoot = pDoc->FirstChildElement("recentitems");
		if (pRoot) {
			TiXmlElement* pItem = pRoot->FirstChildElement("item");
			while (pItem) {
				if (maxEntry > 0 && cnt >= maxEntry)
					break;
				CIconListStyle ils;
				CCmItem* pci = xmlload_tabitem(pItem, &ils, NULL);
				if (pci) {
					pci->m_id = cnt++;
					array.Append(pci, ils);
				}
				pItem = pItem->NextSiblingElement("item");
			}
		}
	}
	return cnt;
}

/////////////////////////////////////////////////////////////////////////////
// CMainFrame construction/destruction

CMainFrame::CMainFrame()
		: m_pCurrentDocument(NULL)
		, m_timerid_combine(0)
		, m_wm_notifyicon(RegisterWindowMessage(_T("wm_fv7notifyicon")))
		, m_wm_frameisidle(RegisterWindowMessage(_T("wm_fv7isidle")))
		, m_bEnableAccel(true)
		, m_bInitCompleted(false)
		, m_bIgnoreFolderBrowsing(false)
{
}

CMainFrame::~CMainFrame()
{
}

void CMainFrame::EnableAccel(bool bEnable)
{
	m_bEnableAccel = bEnable;
}

BOOL CMainFrame::PreTranslateMessage(MSG* pMsg)
{
	if (!::IsWindow(pMsg->hwnd)) return FALSE;

	//---- 2004-09-09
// ヘルプを表示していてもアクセラレータキーが有効になってしまう問題
//	if (m_bEnableAccel) {
	if (m_bEnableAccel && GetActiveWindow() == m_hWnd) {
		// 仮想キーコードの処理
		if (pMsg->message == WM_KEYDOWN) {
			int vkeyShift = _ISKEYDOWN(VK_SHIFT);
			int vkeyCtrl = _ISKEYDOWN(VK_CONTROL);
			switch (pMsg->wParam) {
			case VK_TAB:					// フォーカスペインの変更
				if (!vkeyCtrl)
					fvApp.ActivateNextView(!vkeyShift);
				else
					PostMessage(WM_COMMAND, vkeyShift ? IDM_TAB_PREV : IDM_TAB_NEXT);
				return TRUE;
			case VK_BROWSER_BACK:			// ひとつ前のフォルダ
				PostMessage(WM_COMMAND, IDM_DIR_PREV);
				return TRUE;
			case VK_BROWSER_FORWARD:		// 次のフォルダ
				PostMessage(WM_COMMAND, IDM_DIR_NEXT);
				return TRUE;
			case VK_BROWSER_REFRESH:		// リフレッシュ
				PostMessage(WM_COMMAND, IDM_WINDOW_REFLESH);
				return TRUE;
			case VK_BROWSER_HOME:			// ホームを開く
				PostMessage(WM_COMMAND, IDM_DIR_HOME);
				return TRUE;
			case VK_BROWSER_SEARCH:			// 検索
				PostMessage(WM_COMMAND, IDM_DIR_SEARCH);
				return TRUE;
//@			case VK_BROWSER_FAVORITES:
			default:
				break;
			}
		}

		// ナビバー
		if (m_navibar.IsMyMessage(pMsg))
			return m_navibar.ProcessMessage(pMsg);
		// しおりパネル
		if (m_viewFavo.IsMyMessage(pMsg))
			return m_viewFavo.ProcessMessage(pMsg);
		// オブジェクトパネル
		if (m_appbar.IsMyMessage(pMsg))
			return m_appbar.ProcessMessage(pMsg);

		// ファイルペイン --- 主にインクリメンタルサーチの処理
		CFileBrowseView* fileview = (CFileBrowseView*)GetCurrentFileBrowser().GetClassBody();
		if (fileview) {
			// 検索バー
			if (fileview->m_finder.IsMyMessage(pMsg))
				return fileview->m_finder.ProcessMessage(pMsg);

			if (fileview->IsMyMessage(pMsg)) {
				if (fileview->ProcessMessage(pMsg))
					return TRUE;
			}
		}

		// ルート戻りキーのチェック
		if (pMsg->message == WM_KEYDOWN && pMsg->wParam == 0xdc) {
			PostMessage(WM_COMMAND, IDM_DIR_ROOT);
			return TRUE;
		}

		HACCEL hacc = fvApp.GetLocalAccelerator();
		if (GetFocus() != m_menubar && hacc) {
			if (::TranslateAccelerator(m_hWnd, hacc, pMsg))
				return TRUE;
		}
		return CFrameWindowImpl<CMainFrame>::PreTranslateMessage(pMsg);
	}
	return FALSE;
}

BOOL CMainFrame::OnIdle()
{
	UpdateUIAll();

	fvApp.OnIdleOperationThreadManager();
	return FALSE;
}

void CMainFrame::TaskbarButtonCreated(ITaskbarList3* pTaskbarList)
{
	if (pTaskbarList != NULL) {
/*
		HICON hIcon = LoadIcon(RESINST, MAKEINTRESOURCE(IDR_ARCVIEW));
		if (hIcon) {
			pTaskbarList->SetOverlayIcon(m_hWnd, hIcon, NULL);
			DestroyIcon(hIcon);
		}
*/
		if (fvApp.isAdmin()) {
			HICON hIcon = ::LoadIcon(NULL, IDI_SHIELD);
			if (hIcon) {
				pTaskbarList->SetOverlayIcon(m_hWnd, hIcon, NULL);
				DestroyIcon(hIcon);
			}
		}
	}
}

void CMainFrame::UpdateUIAll()
{
	UISetCheck(IDM_VIEW_DESKTOPTREE, !(gnSysStat & STS_DRIVEROOT));

	CFileBrowseWindow browser = GetCurrentFileBrowser();
	CIconListStyle ls;
	int indexListColumn = browser.GetListStyle(&ls);

	UIEnable(IDM_SORT, TRUE);
	if (ls.isSortNone()) {
		UISetCheck(IDM_SORT_UP, FALSE);
		UISetCheck(IDM_SORT_DOWN, FALSE);
		UIEnable(IDM_SORT_UP, FALSE);
		UIEnable(IDM_SORT_DOWN, FALSE);
	} else {
		UIEnable(IDM_SORT_UP, TRUE);
		UIEnable(IDM_SORT_DOWN, TRUE);
		UISetCheck(IDM_SORT_UP, ls.isSortUp());
		UISetCheck(IDM_SORT_DOWN, ls.isSortDown());
	}
	UISetCheck(IDM_SORT_NAME, ls.isSortName());
	UISetCheck(IDM_SORT_SIZE, ls.isSortSize());
	UISetCheck(IDM_SORT_TIME, ls.isSortTime());
	UISetCheck(IDM_SORT_TYPE, ls.isSortExt());
	UISetCheck(IDM_SORT_ATTR, ls.isSortAttr());
	UISetCheck(IDM_SORT_DOS,  ls.isSortDos());
	UISetCheck(IDM_SORT_PATH, ls.isSortPath());

	UISetCheck(IDM_SORT_COL0, indexListColumn == 0);
	UISetCheck(IDM_SORT_COL1, indexListColumn == 1);
	UISetCheck(IDM_SORT_COL2, indexListColumn == 2);
	UISetCheck(IDM_SORT_COL3, indexListColumn == 3);
	UISetCheck(IDM_SORT_COL4, indexListColumn == 4);
	UISetCheck(IDM_SORT_COL5, indexListColumn == 5);
	UISetCheck(IDM_SORT_COL6, indexListColumn == 6);
	UISetCheck(IDM_SORT_COL7, indexListColumn == 7);
	UISetCheck(IDM_SORT_COL8, indexListColumn == 8);

	UISetCheck(IDM_ICON_NORMAL, ls.isStyleNormal());
	UISetCheck(IDM_ICON_REPORT, ls.isStyleReport());
	UISetCheck(IDM_ICON_OUTLINE, ls.isStyleOutline());
	UISetCheck(IDM_ICON_IMAGE, ls.isStyleImage());
	UISetCheck(IDM_ICON_ICON, ls.isStyleIcon());
	UISetCheck(IDM_ICON_RELAX, (gnListStat & LTS_RELAX) != 0);

	BOOL b;
	if (ls.isStyleImage()) {
		UIEnable(IDM_ICON_ENLARGE, FALSE);
	} else {
		if      (ls.isStyleNormal())	b = ls.isEnlargeNormal();
		else if (ls.isStyleReport())	b = ls.isEnlargeReport();
		else if (ls.isStyleOutline())	b = ls.isEnlargeOutline();
		else							b = ls.isEnlargeIcon();
		UIEnable(IDM_ICON_ENLARGE, TRUE);
		UISetCheck(IDM_ICON_ENLARGE, b);
	}

	UINT id;
	switch (ls.size_thumb.cx) {
		case 72:	id = IDM_IMAGE_072; break;
		case 128:	id = IDM_IMAGE_128; break;
		case 160:	id = IDM_IMAGE_160; break;
		case 192:	id = IDM_IMAGE_192; break;
		case 96:
		default:	id = IDM_IMAGE_096; break;
	}
	UISetRadioMenuItem(id, IDM_IMAGE_072, IDM_IMAGE_192);

	b = (IsClipboardFormatAvailable(CF_HDROP) || IsClipboardFormatAvailable(nRCF_SHELLIDLIST));
	UIEnable(IDM_EDIT_PASTE, b);
	UIEnable(IDM_EDIT_PASTESHORTCUT, b);

	if (ls.isStyleNormal() || ls.isStyleIcon()) {
		UIEnable(IDM_VIEW_SIZE, TRUE);
		UIEnable(IDM_VIEW_DATE, TRUE);
		UIEnable(IDM_VIEW_ATTRIB, TRUE);
		UISetCheck(IDM_VIEW_SIZE, ls.isShowSize());
		UISetCheck(IDM_VIEW_DATE, ls.isShowDate());
		UISetCheck(IDM_VIEW_ATTRIB, ls.isShowAttr());
	} else if (ls.isStyleImage()) {
		UIEnable(IDM_VIEW_SIZE, TRUE);
		UIEnable(IDM_VIEW_DATE, TRUE);
		UIEnable(IDM_VIEW_ATTRIB, TRUE);
		UISetCheck(IDM_VIEW_SIZE, ls.isShowSize());
		UISetCheck(IDM_VIEW_DATE, ls.isShowDate());
		UISetCheck(IDM_VIEW_ATTRIB, ls.isShowAttr());
	} else {
		UISetCheck(IDM_VIEW_SIZE, FALSE);
		UISetCheck(IDM_VIEW_DATE, FALSE);
		UISetCheck(IDM_VIEW_ATTRIB, FALSE);
		UIEnable(IDM_VIEW_SIZE, FALSE);
		UIEnable(IDM_VIEW_DATE, FALSE);
		UIEnable(IDM_VIEW_ATTRIB, FALSE);
	}

	if (ls.isStyleNormal() || ls.isStyleReport()) {
		UIEnable(IDM_VIEW_NAMEELLIPSIS, TRUE);
		UISetCheck(IDM_VIEW_NAMEELLIPSIS, ls.isShowEllp());
	}
	else {
		UISetCheck(IDM_VIEW_NAMEELLIPSIS, FALSE);
		UIEnable(IDM_VIEW_NAMEELLIPSIS, FALSE);
	}

	UISetCheck(IDM_VIEW_DESKTOPTREE, !(gnSysStat & STS_DRIVEROOT));

	b = ((gnFileStat & FTS_SUPERHIDDEN) != 0);
	UISetCheck(IDM_TYPE_FILESUPERHIDDEN, b);
	b = ((gnFileStat & FTS_HIDDEN) != 0);
	UISetCheck(IDM_TYPE_FILEHIDDEN, b);
	UISetCheck(IDM_TYPE_EXCFOLDER, (gnFileStat & FTS_FOLDER) != 0);

	LPCITEMIDLIST pidl = m_drive.GetCurrentDrive();
	UIEnable(IDM_DISK_LABEL, pidl != NULL);

	b = ((GetSystemMetrics(SM_NETWORK) & 1) != 0);
	UIEnable(IDM_NET_CONNECT, b);
	UIEnable(IDM_NET_DISCONNECT, b);
	UIEnable(IDM_NET_ADDPLACE, b);

	// バーの表示状態
	UISetCheck(IDM_BAR_MENU, m_menubar.IsWindowVisible());
	UISetCheck(IDM_BAR_PROP, m_propbar.IsWindowVisible());
	UISetCheck(IDM_BAR_SORT, m_sortbar.IsWindowVisible());
	UISetCheck(IDM_BAR_APPBAR, m_appbar.IsWindowVisible());
	UISetCheck(IDM_BAR_SMART, m_smartbar.IsWindowVisible());
	UISetCheck(IDM_BAR_STAT, m_statbar.IsWindowVisible());
	UISetCheck(IDM_BAR_FUNCTION, m_functionbar.IsWindowVisible());
	UISetCheck(IDM_BAR_DRIVE, m_drive.IsWindowVisible());
	UISetCheck(IDM_BAR_FAVO, m_viewFavo.IsWindowVisible());
	UISetCheck(IDM_BAR_NAVI, m_navibar.IsWindowVisible());

	// ドライブバーの配置
	b = ((gnBarStat & BTS_VERTDRIVE) != 0);
	id = !b ? IDM_BAR_DRIVE_HORZ : ((gnBarStat & BTS_DRIVERIGHT) ? IDM_BAR_DRIVE_VERTRIGHT : IDM_BAR_DRIVE_VERTLEFT);
	UISetRadioMenuItem(id, IDM_BAR_DRIVE_HORZ, IDM_BAR_DRIVE_VERTRIGHT);
	// ツールバーボタンの大きさ
	UISetCheck(IDM_BAR_LARGEICON, ((gnBarStat & BTS_LARGEICON) != 0));

	// カスタムツールバー
	int cnt = m_arrToolbar.GetSize();
	if (cnt > 0) {
		for (int i = 0; i < cnt; i++) {
			CToolBarCtrl* pBar = m_arrToolbar[i];
			UISetCheck(IDW_TBFIRST + i, pBar->IsWindowVisible());
		}
	}

	// バーのロック
	if (gnBarStat & BTS_LOCKREBAR) {
		UIEnable(IDM_BAR_REPOSITION, FALSE);
		UISetCheck(IDM_BAR_LOCKREBAR, TRUE);
	}
	else {
		UIEnable(IDM_BAR_REPOSITION, TRUE);
		UISetCheck(IDM_BAR_LOCKREBAR, FALSE);
	}

	// 履歴
	CHistory* pHistory = GetHistory();
	ATLASSERT(pHistory != NULL);
	UIEnable(IDM_DIR_TRACE, pHistory->GetCount() > 1);
	UIEnable(IDM_DIR_HISTORY, pHistory->GetCount() > 1);
	UIEnable(IDM_DIR_PREV, pHistory->canBack());
	UIEnable(IDM_DIR_NEXT, pHistory->canForward());

	// タブ
	UpdateTabMenu(m_tabbar.GetCurSel());

	// ウィンドウ
	b = (GetWindowNumCount(APP_FRAMECLASS, NULL, FALSE) > 1);
	UIEnable(IDM_WINDOW_PREV, b);
	UIEnable(IDM_WINDOW_NEXT, b);

	UIUpdateToolBar();
	UpdateStatusBarText();
}

void CMainFrame::UpdateTabMenu(int indexTarget)
{
	int cnt = m_tabbar.GetCount();
	BOOL b = (cnt > 1);
	UIEnable(IDM_TAB_PREV, b);
	UIEnable(IDM_TAB_NEXT, b);
	UIEnable(IDM_TAB_CLOSE_ANYOTHER, b);
	UIEnable(IDM_TAB_SORTDEFAULT, b);
	UIEnable(IDM_TAB_SORTREFERENCE, b);
	b = (cnt > 1 && indexTarget > 0);
	UIEnable(IDM_TAB_CLOSE_LEFTSIDE, b);
	b = (cnt > 1 && indexTarget < cnt - 1);
	UIEnable(IDM_TAB_CLOSE_RIGHTSIDE, b);
	b = (m_arrTabReopen.GetItemCount() > 0);
	UIEnable(IDM_TAB_REOPEN_RECENT, b);
	b = m_tabbar.isItemLocked(indexTarget);
	UISetCheck(IDM_TAB_LOCK, b);
	UIEnable(IDM_TAB_CLOSE, !b);
	UIEnable(IDM_TAB_EXPORT, !b && (cnt > 1));
	CString str;
	str.LoadString(b ? IDS_TAB_UNLOCK : IDS_TAB_LOCK);
	UISetText(IDM_TAB_LOCK, str.GetString());
}

void CMainFrame::UpdateStatusBarText()
{
	CString str;
	int cchMax = MAX_PATH * 2;
	str.GetBuffer(cchMax);
	CFileBrowseWindow browser = GetCurrentFileBrowser();
	if (browser.IsWindow()) {
		// TOTALが先でないとだめ
		browser.GetTotalInformation(str.GetBuffer(), cchMax);
		m_statbar.SetStockText(ID_INDICATOR_TOTAL, str);
		browser.GetSelectInformation(str.GetBuffer(), cchMax);
		m_statbar.SetStockText(ID_DEFAULT_PANE, str);
	}

	DWORDLONG dlFree = 0, dlTotal = 0;
	if (GetFolderTree().GetCurrentPath(str.GetBuffer(), cchMax))
		_getdiskfreespace(str.GetBuffer(), dlFree, dlTotal);
	else {
		LPCITEMIDLIST pidlCur = GetFolderTree().GetCurrentIDList();
		if (pidlCur) {
			CComPtr<IShellFolder> psf;
			LPCITEMIDLIST pidl;
			if (::SHBindToParent(pidlCur, IID_PPV_ARGS(&psf), &pidl) == S_OK) {
				CComQIPtr<IShellFolder2, &IID_IShellFolder2> psf2(psf);
				if (psf2.p) {
					CComVariant vr;
					if (psf2->GetDetailsEx(pidl, &PKEY_Capacity, &vr) == S_OK) {
						dlTotal = vr.ullVal;
					}
					if (psf2->GetDetailsEx(pidl, &PKEY_FreeSpace, &vr) == S_OK) {
						dlFree = vr.ullVal;
					}
				}
			}
		}
	}
	if (dlTotal != 0) {
		TCHAR szFree[64], szTotal[64];
		StrFormatByteSize64(dlFree, szFree, 64);
		StrFormatByteSize64(dlTotal, szTotal, 64);
		str.Format(IDS_INDICATOR_LABEL, szFree, szTotal);
	}
	m_statbar.SetStockText(ID_INDICATOR_LABEL, str.GetString());

	CFilterSetting* filterInfo = GetDocument()->GetFilter();
	ATLASSERT(filterInfo != NULL);
	str = filterInfo->m_bUnmatchFilter ? _T("!") : _T("");
	str += filterInfo->m_strFilter.GetString();
	m_statbar.SetStockText(ID_INDICATOR_FILTER, str.GetString());

	CIconListStyle ls;
	browser.GetListStyle(&ls);
	UpdateStatusbarImageSize(ls);
}

#define TPM_BOTTOMCENTERANIMATION	(TPM_CENTERALIGN | TPM_BOTTOMALIGN | TPM_VERNEGANIMATION)

void CMainFrame::OnStatusbarNotifyClick(NMHDR* pNmhdr, LRESULT* pResult)
{
	NMMOUSE* pm = (NMMOUSE*)pNmhdr;
	// 項目の矩形を取得しておく
	CRect rc;
	m_statbar.GetPaneRect((int)pm->dwItemData, &rc);
	m_statbar.ClientToScreen(&rc);

	if (pm->dwItemData == ID_INDICATOR_FILTER) {
		CFilterSetting* filterInfo = GetDocument()->GetFilter();
		CSimpleArray<CFilterSetting> list;
		int cnt = GetFilterHistory(list);

		CMenu pop;
		pop.CreatePopupMenu();
		int maxlist = IDM_FILTERHISTORY_LAST - IDM_FILTERHISTORY_FIRST + 1;
		cnt = min(maxlist, cnt);
		int pos = 0;
		for (int i = 0; i < cnt; i++) {
			UINT flag = MF_BYPOSITION;
			if (list[i].isFilterMatched(*filterInfo)) {
				flag |= MF_CHECKED;
			}
			pop.InsertMenu(pos++, flag, IDM_FILTERHISTORY_FIRST + i, list[i].menuLabel());
		}
		if (cnt == 0) {
			pop.InsertMenu(pos++, MF_BYPOSITION | MF_GRAYED, (UINT_PTR)0, L"（なし）");
		}
		else {
			pop.InsertMenu(pos++, MF_BYPOSITION | MF_SEPARATOR, (UINT_PTR)0, (LPCTSTR)NULL);
			pop.InsertMenu(pos++, MF_BYPOSITION, IDM_FILTERHISTORY_FIRST - 2, L"履歴をリセット(&R)");
			if (!filterInfo->isDefault()) {
				pop.InsertMenu(pos++, MF_BYPOSITION, IDM_FILTERHISTORY_FIRST - 1, L"フィルタを解除(&F)");
			}
		}
		int cmd = (int)pop.TrackPopupMenu(TPM_BOTTOMCENTERANIMATION | TPM_RETURNCMD, (rc.left + rc.right) / 2, rc.top, m_hWnd);
		if (cmd > 0) {
			cmd -= IDM_FILTERHISTORY_FIRST;
			if (cmd == -2) {
				// 履歴リセット
				ResetFilterHistory();
			}
			else {
				SetFileFilter(cmd >= 0 ? &list[cmd] : NULL);
				if (cmd >= 0) {
					CFilterSetting fs = list[cmd];
					list.RemoveAt(cmd);
					list.Add(fs);
				}
			}
		}
	}
	else if (pm->dwItemData == ID_INDICATOR_IMAGESIZE) {
		CMenuHandle pop = GetSubSequenceMenu(3, 9);
		pop.TrackPopupMenu(TPM_BOTTOMCENTERANIMATION, (rc.left + rc.right) / 2, rc.top, m_hWnd);
	}
	*pResult = TRUE;
}

void CMainFrame::OnMenubarTakeFocus(WPARAM wParam, LPARAM /*lParam*/)
{
	if (wParam) {
		if (!m_menubar.IsWindowVisible())
			ShowBar(m_menubar, TRUE);
	}
	else {
		if (m_menubar.IsWindowVisible())
			ShowBar(m_menubar, FALSE);
	}
}

LRESULT CMainFrame::OnCopyData(WPARAM /*wParam*/, LPARAM lParam)
{
	BOOL bLocked = m_tabbar.isItemLocked(m_tabbar.GetCurSel());

	COPYDATASTRUCT* pcds = (COPYDATASTRUCT*)lParam;
	if (pcds) {
		TCHAR buf[MAX_PATH * 2];
		CIDListPtr idl;
		switch (pcds->dwData & CPD_COMMAND_MASK) {
		case CPD_JUMP_PATHW:
			idl.CreateFromPath((LPCTSTR)pcds->lpData);
			if (bLocked)
				TabNew(idl);
			else
				OpenFolder(idl);
			break;

		case CPD_JUMP_FILEW:
			idl.CreateFromPath((LPCTSTR)pcds->lpData);
			if (bLocked) {
				CIDListPtr idlParent(idl_GetParent(idl));
				TabNew(idlParent);
			}
			SelectFolderItem(idl);
			break;

		case CPD_JUMP_IDLIST:
			if (idl.Clone((LPITEMIDLIST)pcds->lpData)) {
				SFGAOF attr = idl.GetAttributes(SFGAO_FOLDER);
				if (attr & SFGAO_FOLDER) {
					if (bLocked || pcds->dwData & CPD_NEW_TAB)
						TabNew(idl);
					else
						OpenFolder(idl);
				}
				else {
					if (bLocked || pcds->dwData & CPD_NEW_TAB) {
						CIDListPtr idlParent(idl_GetParent(idl));
						TabNew(idlParent);
					}
					SelectFolderItem(idl);
				}
			}
			break;

		case CPD_EXEC_FILEW:
		case CPD_VIEW_FILEW:
			lstrcpy(buf, (LPTSTR)pcds->lpData);
			if (_pathisdirectory(buf)) {
				idl.CreateFromPath(buf);
				if (bLocked)
					TabNew(idl);
				else
					OpenFolder(idl);
			}
			else if (_pathfileexists(buf)) {
				if (!lstrcmpi(PathFindExtension(buf), gszTabGroup))
					TabGroupLoad(buf);
				else if (!ExecAddin(m_hWnd, buf))
					AddinViewer(m_hWnd, buf);
			}
			break;

		case CPD_MACROW:
			lstrcpy(buf, (LPTSTR)pcds->lpData);
			SetInitialFolder(buf);
			break;
		}
	}
	return 0;
}

/////////////////////////////////////////////////////////////////////////////
// CMainFrame message handlers

int CMainFrame::OnCreate(LPCREATESTRUCT pcs)
{
ImmAssociateContext(m_hWnd, NULL);

	fvApp.wndMain = m_hWnd;
	ModifyStyleEx(0, WS_EX_ACCEPTFILES);
	// アプリケーションマーカ （シェル用に）
	::SetProp(m_hWnd, APP_PRODUCT, (void*)REGPREFIX);
	// ウィンドウ位置
	if (fvApp.isStartPrimaryMonitor()) {
		CRect rc;
		int sib = GetWindowNumCount(APP_FRAMECLASS, NULL, FALSE);
		WINDOWPLACEMENT wpm;
		if (GetRegBinarySeq(IDS_SEC_SETTINGS, IDS_KEY_FRAMEPOS, sib - 1, (BYTE*)&wpm, sizeof(wpm)) == sizeof(wpm)) {
			rc = wpm.rcNormalPosition;
			lccMapRectToProgmanPane(&rc);
			SetWindowPos(NULL, &rc, SWP_NOZORDER | SWP_NOREDRAW);
		}
	}

	// フォルダのリストスタイル情報
	m_mapListStyle.Load();

	// リバー
#ifdef CUSTOM_REBER_USING
	CreateSimpleReBar(ATL_SIMPLE_REBAR_NOBORDER_STYLE | RBS_DBLCLKTOGGLE);
	m_rebar.SubclassWindow(m_hWndToolBar);
#else
	CreateSimpleReBar(ATL_SIMPLE_REBAR_NOBORDER_STYLE | RBS_DBLCLKTOGGLE);
	m_rebar = m_hWndToolBar;
#endif

	// 縦置きリバー
	DWORD style = ATL_SIMPLE_REBAR_NOBORDER_STYLE | CCS_VERT | CCS_NOPARENTALIGN;
#ifdef CUSTOM_REBER_USING
	HWND hbar = CreateSimpleReBarCtrl(m_hWnd, style, IDW_VERTREBAR);
	m_vtrebar.SubclassWindow(hbar);
#else
	m_vtrebar = CreateSimpleReBarCtrl(m_hWnd, style, IDW_VERTREBAR);
#endif

	// メニューバー
	m_menubar.Create(m_hWnd, rcDefault, NULL, ATL_SIMPLE_CMDBAR_PANE_STYLE, 0, IDW_MENUBAR);
	m_menubar.AttachMenu(GetMenu());
	m_menubar.SetImageMaskColor(RGB(255, 0, 255));
	m_menubar.LoadMultipleImages(IDB_SORTBAR);
	m_menubar.LoadMultipleImages(IDB_PROPBAR);
	m_menubar.LoadMultipleImages(IDB_TOOLBAR);
	m_menubar.LoadMultipleImages(IDB_APPICON);
	SetMenu(NULL);
#ifdef CUSTOM_REBER_USING
	m_rebar.SetMenuBar(m_menubar);						// メニューバーを明示的に登録
#endif
	AddSimpleReBarBandCtrl(m_rebar, m_menubar, IDW_MENUBAR, NULL, FALSE, 0, FALSE);

	if (!CreatePropBar() ||
		!CreateSortBar() ||
		!CreateNaviBar() ||
		!CreateSmartBar() ||
		!CreateToolBar() ||
		!CreateFunctionBar() ||
		!CreateStatusBar() ||
		!CreateDriveBar())
		return -1;

	// 機能キーバー
	if (!(gnBarStat & BTS_FUNCTION))
		m_functionbar.ShowWindow(SW_HIDE);
	else
		m_functionbar.EnableTimer(TRUE);
	// ステータスバー
	if (!(gnBarStat & BTS_STATUS))
		m_statbar.ShowWindow(SW_HIDE);

	// プロパティバー
	CString str;
	str.LoadString(IDW_PROPBAR);
	m_propbar.SetWindowText(str);
	AddSimpleReBarBandCtrl(m_rebar, m_propbar, IDW_PROPBAR, NULL, TRUE, 0, FALSE);
	UIAddToolBar(m_propbar);

	// ソートバー
	str.LoadString(IDW_SORTBAR);
	m_sortbar.SetWindowText(str);
	AddSimpleReBarBandCtrl(m_rebar, m_sortbar, IDW_SORTBAR, NULL, FALSE, 0, FALSE);
	UIAddToolBar(m_sortbar);

	// ツールバー
	CToolBarCtrl* pBar;
	int num = m_arrToolbar.GetSize();
	for (int i = 0; i < num; i++) {
		pBar = m_arrToolbar[i];
		AddSimpleReBarBandCtrl(m_rebar, *pBar, pBar->GetDlgCtrlID(), NULL, FALSE, 0, FALSE);
		UIAddToolBar(*pBar);
	}

	// ドライブバーを格納
	str.LoadString(IDW_DRIVEBAR);
	m_drive.SetWindowText(str);
	if (m_drive.isVertical()) {
		AddSimpleReBarBandCtrl(m_vtrebar, m_drive, IDW_DRIVEBAR, NULL, TRUE, 0, FALSE);
		RecalcDriveBar();
	} else {
		AddSimpleReBarBandCtrl(m_rebar, m_drive, IDW_DRIVEBAR, NULL, TRUE, 0, FALSE);
	}
	// スマートバー
	str.LoadString(IDW_SMARTBAR);
	m_smartbar.SetWindowText(str);
	AddSimpleReBarBandCtrl(m_rebar, m_smartbar, IDW_SMARTBAR, NULL, m_drive.isVertical(), 0, FALSE);
	UIAddToolBar(m_smartbar);

	// ナビバー
	str.LoadString(IDW_NAVIGATIONBAR);
	m_navibar.SetWindowText(str);
	AddSimpleReBarBandCtrl(m_rebar, m_navibar, IDW_NAVIGATIONBAR, NULL, TRUE, 0, FALSE);
	UIAddToolBar(m_navibar);

	// 親リバーに対する初期設定
	m_navibar.ApplyRebarInitialSetting(m_rebar, m_rebar.GetBandCount() - 1);
	AddCustomDrawBar(m_navibar);

	// タブバー
	m_tabbar.Create(m_hWnd, IDW_TABBAR);
	HIMAGELIST himL, himS;
	Shell_GetImageLists(&himL, &himS);
	m_tabbar.SetImageList(himS);													// アイコンイメージリスト
//	HBITMAP hbm = lccLoadImage(RESINST, RT_RCDATA, MAKEINTRESOURCE(IDB_TABBKGND));	// 背景ビットマップ
//	m_tabbar.GetTabCtrl().SetBackgroundImage(hbm);
	COLORREF tabclr = RGB(232, 232, 232); // old GetSysColor(COLOR_ACTIVECAPTION)
	m_tabbar.GetTabCtrl().SetPaneColor(tabclr);			// 背景色
	m_tabbar.GetTabCtrl().SetMinTabWidth(LOWORD(GetDialogBaseUnits()) * 12);
	if (!(gnTabStat & TAB_BARSHOWANY))			// タブバーが「常に表示」でなければ隠す
		m_tabbar.ShowWindow(SW_HIDE);

	// 以前のリバーバンド状態を復帰
	CRebarSettings rs;
	if (rs.Load(_T("RebarSettings"), _T("Toolbar")))
		rs.ApplyTo(m_rebar);
	else {
		// 各バンドの幅を調整
		SizeSimpleReBarBands();
	}

	// リバーロック
	LockBars((gnBarStat & BTS_LOCKREBAR) != 0);

	if (!CreateClient())
		return -1;

	// アクティブフォーカス切り替え用配列への登録
	m_ViewSwitch.Add(m_appbar);
	m_ViewSwitch.Add(GetFolderTree());
	m_ViewSwitch.Add(m_viewFavo);
	m_ViewSwitch.Add(m_navibar);

	CFileBrowseWindow browser = GetCurrentFileBrowser();
	m_ViewSwitch.Add(browser);

	// トレイ格納
	fvApp.ResetTSR();

	// ショートカットメニュー
	m_scutFile.SetDefault(ARRAYSIZE(snFileBrowserShortcutMenu), snFileBrowserShortcutMenu);
	m_scutFile.Initial(IDS_SEC_OPERATIONS, IDS_KEY_MENUFILE);
	m_scutTree.SetDefault(ARRAYSIZE(snFolderTreeShortcutMenu), snFolderTreeShortcutMenu);
	m_scutTree.Initial(IDS_SEC_OPERATIONS, IDS_KEY_MENUTREE);

	// アクティブウィンドウをセット
	m_ViewSwitch.SetActiveView(browser);

	// ネットワーク配下のリストアップが遅いをの補う
	m_ShellItemSearch.Start(m_hWnd, FOLDERID_NetworkFolder);

	// 試用チェック
	RegistChecker();

	// register object for message filtering and idle updates
	CMessageLoop* pLoop = _Module.GetMessageLoop();
	ATLASSERT(pLoop != NULL);
	pLoop->AddMessageFilter(this);
	pLoop->AddIdleHandler(this);

	return 0;
}

BOOL CMainFrame::CreateClient()
{
	CRect rc;
	GetClientRect(&rc);

	// オブジェクトパネルは右置きかどうか
	bool bAppBarRightSide = (gnBarStat & BTS_APPRIGHT) != 0;

	// （（ツリー＋しおり）＋ファイルペイン）とオブジェクトパネルを分けるスプリタ
	m_splitApp.Create(m_hWnd, rc, NULL, WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS | WS_CLIPCHILDREN, 0, IDW_SPLITTER_APP);
	DWORD exStyle = (bAppBarRightSide) ? SPLIT_RIGHTALIGNED : 0;
	m_splitApp.SetSplitterExtendedStyle(exStyle);
	m_splitApp.GetSystemSettings(false);
	m_splitApp.m_cxyMinBottomRight = _GCDATA.m_IconMetrics.iHorzSpacing;

	int cx = GetRegValue(IDS_SEC_OBJECTBAR, IDS_KEY_WINDOWWIDTH, m_splitApp.m_cxyMinBottomRight + GetSystemMetrics(SM_CXVSCROLL) + 4);
	m_splitApp.SetSplitterPos(bAppBarRightSide ? rc.Width() - (cx + m_splitApp.m_cxySplitBar) : cx);

	// （ツリー＋しおり）とファイルペインを分けるスプリタ
	if (bAppBarRightSide)	rc.right -= cx;
	else					rc.left += cx + m_splitApp.m_cxySplitBar;
	m_splitView.Create(m_splitApp, rc, NULL, WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS | WS_CLIPCHILDREN, 0, IDW_SPLITTER_VIEW);
	m_splitView.SetSplitterExtendedStyle(0, SPLIT_RIGHTALIGNED | SPLIT_PROPORTIONAL);
	m_splitView.GetSystemSettings(false);

	// オブジェクトパネル作成
	m_appbar.Create(m_splitApp, 0, NULL, WS_CHILD | WS_VISIBLE, 0, IDW_APPBAR);
	if (bAppBarRightSide)
		m_splitApp.SetSplitterPanes(m_splitView, m_appbar);
	else
		m_splitApp.SetSplitterPanes(m_appbar, m_splitView);

	cx = GetRegValue(IDS_SEC_SETTINGS, IDS_KEY_TREEWIDTH, 220);
	m_splitView.SetSplitterPos(cx);

	// ツリーとしおりを分けるスプリタ
	m_splitTree.Create(m_splitView, 0, NULL, WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS | WS_CLIPCHILDREN, 0, IDW_SPLITTER_TREE);

	// ドキュメントペイン作成
	CDocument* pDoc = new CDocument;
	CFileBrowseWindow browser = pDoc->CreateBrowser(m_splitView, WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS | WS_CLIPCHILDREN, 0, IDW_FILEBROWSER);
	m_tabbar.AddItem(_T(""), 0, (LPARAM)pDoc);
	m_splitView.SetSplitterPanes(m_splitTree, browser);

	m_splitTree.SetSplitterExtendedStyle(SPLIT_BOTTOMALIGNED);
	m_splitTree.GetSystemSettings(false);
	m_baseTree.Create(m_splitTree, 0, NULL, WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS, 0, IDW_FOLDERTREE, NULL);
	m_baseTree.SetOwner(m_hWnd);

	// しおりパネル作成
	CString str; str.LoadString(IDW_FAVORITE);
	m_viewFavo.Create(m_splitTree, (LPCTSTR)str, IDW_FAVORITE);
	m_splitTree.SetSplitterPanes(m_baseTree, m_viewFavo);
	// しおりパネルの最少高はヘッダの高さ
	m_splitTree.m_cxyMinBottomRight = m_viewFavo.GetHeaderHeight();

	cx = GetRegValue(IDS_SEC_SETTINGS, IDS_KEY_FAVOHEIGHT, 90);
	m_splitTree.SetSplitterPos(rc.Height() - cx);

	// オブジェクトパネルが非表示の場合
	if (!(gnBarStat & BTS_APPBAR))
		m_splitApp.SetSinglePaneMode(bAppBarRightSide ? SPLIT_PANE_LEFT : SPLIT_PANE_RIGHT);
	// しおりパネルが非表示の場合
	if (!(gnBarStat & BTS_FAVO))
		m_splitTree.SetSinglePaneMode(SPLIT_PANE_TOP);

	// フレームクライアント登録
	m_hWndClient = m_splitApp;

	// シェル更新追跡機能の初期化
	StartShellChangeNotify((PIDLIST_ABSOLUTE)NULL,
#ifdef _DEBUG
#ifdef DUMP_SHELLNOTIFY
						SHCNE_DRIVEADDGUI |
						SHCNE_FREESPACE |
						SHCNE_EXTENDED_EVENT |
						SHCNE_SERVERDISCONNECT |
						SHCNE_RENAMEITEM |
						SHCNE_ATTRIBUTES |
#endif		// DUMP_SHELLNOTIFY
#endif		// _DEBUG
						SHCNE_CREATE |
						SHCNE_DELETE |
						SHCNE_MKDIR |
						SHCNE_RMDIR |
						SHCNE_MEDIAINSERTED |
						SHCNE_MEDIAREMOVED |
						SHCNE_DRIVEREMOVED |
						SHCNE_DRIVEADD |
						SHCNE_NETSHARE |
						SHCNE_NETUNSHARE |
						SHCNE_UPDATEDIR |
						SHCNE_UPDATEITEM |
						SHCNE_UPDATEIMAGE |
						SHCNE_RENAMEFOLDER |
						SHCNE_ASSOCCHANGED,
						TRUE);

	return TRUE;
}

/////////////////////////////////////////////////////////////////////////////
// arrays of IDs used to initialize control bars

static CLspStatusBarCtrl::_ColorPair sbclrNormal = { RGB(255, 255, 255), RGB(56, 128, 216) };
static CLspStatusBarCtrl::_ColorPair sbclrAdmin = { RGB(255, 255, 255), RGB(153, 51, 102) };

static _StatusBarPaneItem statbarIndicators[] = {
	ID_DEFAULT_PANE, 0, -1, {0},
	ID_INDICATOR_IMAGESIZE, SBPIF_HIDDEN | SBPIF_AUTOWIDTH | SBPIF_BUTTON | SBPIF_PULLUP, 0, {0},
	ID_INDICATOR_FILTER, SBPIF_AUTOWIDTH | SBPIF_BUTTON | SBPIF_PULLUP, 1, {0},
	ID_INDICATOR_TOTAL, 0, 2, {0},
	ID_INDICATOR_LABEL, 0, 3, {0}
};

BOOL CMainFrame::CreateStatusBar()
{
	// statusbar
	m_hWndStatusBar = m_statbar.Create(m_hWnd, (LPCTSTR)NULL);
	// statusbar imagelist
	m_statbarImageList = lccLoadAlphaImageList(RESINST, RT_RCDATA, MAKEINTRESOURCE(IDB_STATUSBAR), 16, FALSE);
	m_statbar.SetImageList(m_statbarImageList);
	// set bar color
	if (fvApp.isAdmin())
		m_statbar.SetColorTheme(sbclrAdmin.text, sbclrAdmin.bkgnd);
	else
		m_statbar.SetColorTheme(sbclrNormal.text, sbclrNormal.bkgnd);
	// add indicators
	m_statbar.SetIndicators(statbarIndicators, ARRAYSIZE(statbarIndicators));
	return TRUE;
}

BOOL CMainFrame::CreateFunctionBar()
{
	m_functionbar.CreateBar(m_hWnd, IDW_FUNCTIONBAR);
	return TRUE;
}

void CMainFrame::InitialUpdate()
{
	// フレームウィンドウ作成完了後に呼び出される

	// アクセラレータをメニューに適用
	if (gnListStat & LTS_MENUACCEL)
		UpdateMenuAccelerator();
	// 閉じたタブのリストを読み込み
	LoadTabReopenArray(m_arrTabReopen, 6);
}

typedef struct tagKNOWNFOLDER_PAIR {
	LPCWSTR name;
	GUID guid;
} KNOWNFOLDER_PAIR;

static KNOWNFOLDER_PAIR sstKnownFolderPair[] = {
	{ L"desktop",		FOLDERID_Desktop },
	{ L"documents",		FOLDERID_Documents },
	{ L"computer",		FOLDERID_ComputerFolder },
	{ L"network",		FOLDERID_NetworkFolder },
	{ L"video",			FOLDERID_Videos },
	{ L"music",			FOLDERID_Music },
	{ L"pictures",		FOLDERID_Pictures },
	{ L"cpanel",		FOLDERID_ControlPanelFolder },
	{ L"contacts",		FOLDERID_Contacts },
	{ L"profile",		FOLDERID_Profile },
	{ L"recycle",		FOLDERID_RecycleBinFolder },
	{ L"cdburn",		FOLDERID_CDBurning },
	{ L"connections",	FOLDERID_ConnectionsFolder },
	{ L"downloads",		FOLDERID_Downloads },
	{ L"favoites",		FOLDERID_Favorites },
	{ L"photos",		FOLDERID_PhotoAlbums },
};

static LPCITEMIDLIST Name2KnownFolderIDList(LPCTSTR name)
{
	CIDListPtr idl;
	for (int i = 0; i < ARRAYSIZE(sstKnownFolderPair); i++) {
		if (!StrCmpNI(sstKnownFolderPair[i].name, name, 5)) {
			idl.GetKnownFolder(sstKnownFolderPair[i].guid);
			break;
		}
	}
	return idl.Detach();
}

void CMainFrame::SetInitialFolder(LPCTSTR pszInitialFolder)
{
	// 初期表示フォルダの決定
	TCHAR buf[MAX_PATH * 2];
	lstrcpy(buf, pszInitialFolder);

	CIDListPtr idl;
	if (buf[0]) {
		if (buf[0] == '"')
			PathUnquoteSpaces(buf);

		if (buf[0] == _T('/')) {
			LPTSTR p = buf + 1;
			idl.Attach(Name2KnownFolderIDList(p));
			if (idl.isNull()) {
				if (!StrCmpNI(p, _T("bookmark"), 8)) {
					// しおりのエントリー番号
					p += 8;
					if (*p++ == ':') {
						GUID guid;
						CLSIDFromString((OLECHAR*)p, &guid);

						CFavoriteItemArray arrItems;
						if (arrItems.Load() > 0) {
							CCmItem* pci = arrItems.LookupGuid(guid);
							if (pci) {
								if (!pci->isFolder()) {
									// タブ グループ
									CString str;
									if (pci->GetParseName(str) && TabGroupLoad(str) > 0)
										return;
								}
								idl.Clone(pci->m_idl);
							}
						}
					}
				} else if (!StrCmpNI(p, _T("recent"), 6)) {
					// 最近閉じたタブのエントリー番号
					p += 6;
					if (*p++ == ':') {
						GUID guid;
						CLSIDFromString((OLECHAR*)p, &guid);

						CTabReopenArray array;
						if (LoadTabReopenArray(array, 0) > 0) {
							CCmItem* pci = array.LookupGuid(guid);
							if (pci)
								idl.Clone(pci->m_idl);
						}
					}
				} else if (!StrCmpNI(p, _T("guid"), 4)) {
					while (*p && *p != _T(' ')) p++;
					while (*p == _T(' ')) p++;
					int len;
					void* data = RegLoadGuidOperation(p, &len);
					if (data) {
						idl.Clone((LPCITEMIDLIST)data);
						free(data);

						if ((idl.GetAttributes(SFGAO_FOLDER) & SFGAO_FOLDER) == 0 && idl.GetPath(buf)) {
							// タブ グループ ファイルが指定された場合
							if (_pathfileexists(buf) && !StrCmpI(PathFindExtension(buf), gszTabGroup)) {
								if (TabGroupLoad(buf) > 0)
									return;
							}
							// そうでなければパス名だけを取り出す
							idl.RemoveLastID();
						}
					}
				}
			}
		} else if (PathIsUNC(buf) || _pathisdirectory(buf))
			idl.CreateFromPath(buf);
		else if (_pathfileexists(buf)) {
			// タブ グループ ファイルが指定された場合
			if (!StrCmpI(PathFindExtension(buf), gszTabGroup)) {
				if (TabGroupLoad(buf) > 0)
					return;
			}

			// そうでなければパス名だけを取り出す
			PathRemoveFileSpec(buf);
			idl.CreateFromPath(buf);
		}
	} else {
		CString str;
		int type = gnSysStat & STS_STARTDIR_MASK;
		if (type == STS_HOMEDIR) {
			// ホームフォルダが指定されている
			idl = GetRegPidl(IDS_SEC_FOLDER, IDS_KEY_HOME);
		} else if (type == STS_LASTDIR) {
			// 前回終了時のディレクトリが有効
			int sib = GetWindowNumCount(APP_FRAMECLASS, NULL, FALSE);
			int len;
			BYTE* pd = GetRegBinarySeqEx(IDS_SEC_FOLDER, IDS_KEY_LASTACCESS, sib - 1, &len);
			if (pd) {
				if (idl_IDListIsValid((LPITEMIDLIST)pd))
					idl.Clone((LPITEMIDLIST)pd);
				else
					idl.Free();
				free(pd);
			}
		} else if (type == STS_LASTTABGROUP) {
			// 前回終了時のタブ グループ
			if (GetLocalAppDataFolder(str, sszSessionGroup)) {
				if (TabGroupLoad(str) > 0)
					return;
			}
		} else if (type == STS_SPECIFICTABGROUP) {
			// 指定のタブ グループ
			if (GetRegText(IDS_SEC_SETTINGS, IDS_KEY_STARTUP_TABGROUP, str) > 0 && _pathfileexists(str)) {
				if (TabGroupLoad(str) > 0)
					return;
			}
		}
	}

	// 起動フォルダの存在確認
	if (!idl.isNull() && !idl.isDesktop()) {
		int descid = idl_GetDescriptionId(idl);
		if (descid == 0 ||
			descid == SHDID_COMPUTER_OTHER ||
			descid == SHDID_NET_OTHER ||
			descid == SHDID_FS_OTHER) {
			idl.Free();
		}
	}

	if (idl.isNull())
		idl.GetFolderLocation(NULL, CSIDL_DRIVES);

	OpenFolder(idl);
}

void* __fastcall fvreg_initkey();
int __fastcall fvreg_getjum(char* pStr, int cchMax);

void CMainFrame::RegistChecker()
{
	// 試用チェック構造体初期化
	CRegWnd* pRegWnd = new CRegWnd;

	// レジストリから取り込んだ登録キーの設定
	char reg_code[MAX_PATH];
	int reg_code_len = fvreg_getjum(reg_code, ARRAYSIZE(reg_code));
	if (reg_code_len > 0)
		memcpy(pRegWnd->m_pui->code, reg_code, reg_code_len + 1);

	// 試用チェック
	CRect rc;
	pRegWnd->Create(m_hWnd, rc, NULL, WS_CHILD);
	pRegWnd->PostMessage(nRM_REGCHECK1ST, 0);
}

void CMainFrame::RecalcDriveBar()
{
	CReBarCtrl& rebar = m_drive.isVertical() ? m_vtrebar : m_rebar;
	// ドライブバーのバンドインデクスを取得
	int index = rebar.IdToIndex(IDW_DRIVEBAR);
	REBARBANDINFO rbbi = { RunTimeHelper::SizeOf_REBARBANDINFO(), RBBIM_SIZE | RBBIM_IDEALSIZE | RBBIM_CHILDSIZE };
	rebar.GetBandInfo(0, &rbbi);

	CRect rc;
	int cnt = m_drive.GetButtonCount();
	if (cnt > 0) {
		if (m_drive.isVertical()) {
			int maxWidth = 0;
			for (int i = 0; i < cnt; i++) {
				m_drive.GetItemRect(i, &rc);
				maxWidth = max(maxWidth, rc.Width());
			}
			rbbi.cx = rc.bottom;					// rcは最後のボタン矩形を持っている
			rbbi.cyMinChild = maxWidth;
			rbbi.cxMinChild = 0;
		}
		else {
			m_drive.GetItemRect(0, &rc);
			rbbi.cxMinChild = rc.right;	
			m_drive.GetItemRect(cnt - 1, &rc);
			rbbi.cx = rc.right;
			rbbi.cyMinChild = rc.Height();
		}
		rbbi.cxIdeal = rbbi.cx;
		rebar.SetBandInfo(index, &rbbi);
	}
}

void CMainFrame::RecalcSmartBar()
{
	// ドライブバーのバンドインデクスを取得
	int index = m_rebar.IdToIndex(IDW_SMARTBAR);
	REBARBANDINFO rbbi = { RunTimeHelper::SizeOf_REBARBANDINFO(), RBBIM_SIZE | RBBIM_IDEALSIZE | RBBIM_CHILDSIZE };
	m_rebar.GetBandInfo(0, &rbbi);

	CRect rc;
	int cnt = m_smartbar.GetButtonCount();
	if (cnt > 0) {
		m_smartbar.GetItemRect(0, &rc);
		rbbi.cxMinChild = rc.right;	
		m_smartbar.GetItemRect(cnt - 1, &rc);
		rbbi.cx = rc.right;
		rbbi.cyMinChild = rc.Height();
		rbbi.cxIdeal = rbbi.cx;
		m_rebar.SetBandInfo(index, &rbbi);
	}
}

void CMainFrame::SetupDriveBar()
{
	CIDListPtr idl;
	idl.Clone(m_drive.GetCurrentDrive());

	int oldDrives = m_drive.GetDriveItemCount();
	m_drive.ResetDrives();

	if (!idl.isNull())
		m_drive.SelectDrive(idl);

	// ドライブアイテム数が変わってなければここで処理を終了
	if (oldDrives == m_drive.GetDriveItemCount()) return;

	if (m_drive.isVertical())
		RecalcDriveBar();
	else {
		// ドライブバーのバンドインデクスを取得
		int index = m_rebar.IdToIndex(IDW_DRIVEBAR);
		if (index < 0) return;

		// ドライブバーの長さを最適化する
		CRect rc;
		int nBtnCount = m_drive.GetButtonCount();
		m_drive.GetItemRect(nBtnCount - 1, &rc);

		REBARBANDINFO rbbi = { RunTimeHelper::SizeOf_REBARBANDINFO(), RBBIM_IDEALSIZE };
		rbbi.cxIdeal = rc.right;
		m_rebar.GetBandBorders(index, &rc);

		rbbi.cx = rbbi.cxIdeal + rc.left + rc.right;
		rbbi.fMask = RBBIM_IDEALSIZE | RBBIM_SIZE;
		m_rebar.SetBandInfo(index, &rbbi);

		// ドライブバーが非表示であればここで処理終了
		if (m_drive.IsWindowVisible()) return;

		// ロックしていればここで処理終了
		if (gnBarStat & BTS_LOCKREBAR) return;

		// ドライブバー以降のバンドサイズ（同じ行内のみ）を再計算
		int cnt = m_rebar.GetBandCount();
		for (int i = index + 1; i < cnt; i++) {
			rbbi.cbSize = RunTimeHelper::SizeOf_REBARBANDINFO();
			rbbi.fMask = RBBIM_IDEALSIZE | RBBIM_CHILD | RBBIM_STYLE;
			m_rebar.GetBandInfo(i, &rbbi);
			if (rbbi.fStyle & RBBS_BREAK) {
				// このインデクスがドライブバーのインデクスではなくて、且つ
				// 新しい行になれば、以降は処理せず
				break;
			}

			if (rbbi.hwndChild != m_navibar) {
				rc.SetRect( 0, 0, 0, 0);
				m_rebar.GetBandBorders(i, &rc);

				rbbi.fMask = RBBIM_SIZE;
				rbbi.cx = rbbi.cxIdeal + rc.left + rc.right;
				m_rebar.SetBandInfo(i, &rbbi);
			}
		}
	}
}

DWORDLONG CMainFrame::GetProcessCreationTime()
{
	HANDLE th = GetCurrentProcess();
	FILETIME ftc, fte, ftk, ftu;
	GetProcessTimes(th, &ftc, &fte, &ftk, &ftu);
	return *((DWORDLONG*)&ftc);
}

DWORDLONG CMainFrame::GetProcessActiveTime()
{
	FILETIME ft;
	GetSystemTimeAsFileTime(&ft);
	DWORDLONG delta = *((DWORDLONG*)&ft) - GetProcessCreationTime();
	return delta / 10000000;
}

LRESULT CMainFrame::OnMySize(UINT /*uMsg*/, WPARAM wParam, LPARAM /*lParam*/, BOOL& bHandled)
{
	if (wParam == SIZE_MINIMIZED) {
		// 未登録または使用期限経過後で起動後2時間以上経過した場合
		if (!glFVValid && glFVAttension) {
			DWORDLONG delta = GetProcessActiveTime();
			if (delta > 60 * 60 * 2) {
				sbForceExit = true;
				PostMessage(WM_CLOSE);
				return 0;
			}
		}

		// トレイ格納時は、設定によりアイコン非表示に
		if (::FindWindow(APP_TRAYCLASS, NULL) && (gnTsrStat & TSS_MINHIDE)) ShowWindow(SW_HIDE);
	}
	bHandled = FALSE;
	return 0;
}

void CMainFrame::UpdateLayout(BOOL bResizeBars)
{
	CRect rc;
	GetClientRect(&rc);

	// position bars and offset their dimensions
	UpdateBarsPosition(rc, bResizeBars);

	// タブバーの処理
	if (m_tabbar.IsWindow() && m_tabbar.IsWindowVisible()) {
		int height = m_tabbar.GetHeight();
		m_tabbar.SetWindowPos(NULL, rc.left, rc.top, rc.Width(), height, SWP_NOZORDER | SWP_NOACTIVATE);
		rc.top += height;
	}

	// 機能キーバーの処理
	if (m_functionbar.IsWindow() && m_functionbar.IsWindowVisible()) {
		int height = m_functionbar.GetBarHeight();
		m_functionbar.SetWindowPos(NULL, rc.left, rc.bottom - height, rc.Width(), height, SWP_NOZORDER | SWP_NOACTIVATE);
		rc.bottom -= height;
	}

	// 縦置きドライブバーの処理
	CRect rcb;
	if (m_drive.IsWindow() && m_drive.isVertical() && m_drive.IsWindowVisible()) {
		// resize vert rebar
		m_vtrebar.GetWindowRect(&rcb);
		int width = rcb.Width();
		rcb = rc;
		if (gnBarStat & BTS_DRIVERIGHT) {
			rcb.left = rcb.right - width;
			rc.right -= width;
		}
		else {
			rcb.right = rcb.left + width;
			rc.left += width;
		}
		// SWP_FRAMECHANGEDを指定しないとWindows2000で初期アイコン起動設定の場合、
		// ウィンドウリストアすると縦置きリバーの描画が不十分になる
		m_vtrebar.SetWindowPos(NULL, rcb.left, rcb.top, rcb.Width(), rcb.Height(), SWP_NOZORDER | SWP_NOACTIVATE | SWP_NOCOPYBITS | SWP_FRAMECHANGED);
	}

	// resize client window
	if (::IsWindow(m_hWndClient))
		::SetWindowPos(m_hWndClient, NULL, rc.left, rc.top, rc.Width(), rc.Height(), SWP_NOZORDER | SWP_NOACTIVATE);
}

BOOL CMainFrame::OnAppCommand(HWND hwnd, int appCommand, UINT uDevice, UINT uKeyState)
{
	UINT idCommand = 0;
	switch (appCommand) {
	case APPCOMMAND_BROWSER_BACKWARD:	idCommand = IDM_DIR_PREV;		break;
	case APPCOMMAND_BROWSER_FORWARD:	idCommand = IDM_DIR_NEXT;		break;
	case APPCOMMAND_BROWSER_HOME:		idCommand = IDM_DIR_HOME;		break;
	case APPCOMMAND_BROWSER_REFRESH:	idCommand = IDM_WINDOW_REFLESH;	break;
	case APPCOMMAND_FIND:
	case APPCOMMAND_BROWSER_SEARCH:		idCommand = IDM_DIR_SEARCH;		break;
	case APPCOMMAND_CLOSE:				idCommand = ID_APP_EXIT;		break;
	}
	if (idCommand) {
		PostMessage(WM_COMMAND, idCommand, 0);
		return TRUE;
	}
	return FALSE;
}

static bool _make_current_path_pidl(CIDListPtr& idl, LPCTSTR path)
{
	TCHAR buf[MAX_PATH * 2];
	lstrcpy(buf, path);
	int dnum = PathGetDriveNumber(path);
	if (dnum >= 0) {
		if (!fopGetDirRemind(dnum, buf) || !_pathfileexists(buf)) {
			fopSetDirRemind(dnum, NULL);
			PathBuildRoot(buf, dnum);
		}
	}
	return (idl.CreateFromPath(buf) != NULL);
}

void CMainFrame::OnDrivebarLButtonDown(NMHDR* pNmhdr, LRESULT* /*pResult*/)
{
	BOOL bLocked = m_tabbar.isItemLocked(m_tabbar.GetCurSel());

	DRIVENOTIFY* pdn = (DRIVENOTIFY*)pNmhdr;
	CIDListPtr idl;
	if (pdn && _make_current_path_pidl(idl, pdn->path)) {
		if (bLocked || pdn->flags & MK_CONTROL)
			TabNew(idl);				// Ctrlキー併用の場合には新しいタブで開く
		else
			OpenFolder(idl);
	}
}

void CMainFrame::OnDrivebarNewWindow(NMHDR* pNmhdr, LRESULT* /*pResult*/)
{
	BOOL bLocked = m_tabbar.isItemLocked(m_tabbar.GetCurSel());

	DRIVENOTIFY* pdn = (DRIVENOTIFY*)pNmhdr;
	CIDListPtr idl;
	if (pdn && _make_current_path_pidl(idl, pdn->path)) {
		if (bLocked)
			TabNew(idl);
		else
			ExecNewFrame(idl, _T("/force"));
	}
}

void CMainFrame::OnDrivebarNewTab(NMHDR* pNmhdr, LRESULT* /*pResult*/)
{
	DRIVENOTIFY* pdn = (DRIVENOTIFY*)pNmhdr;
	CIDListPtr idl;
	if (pdn && _make_current_path_pidl(idl, pdn->path)) {
		TabNew(idl);
	}
}

void CMainFrame::OnDrivebarAddFavorite(NMHDR* pNmhdr, LRESULT* /*pResult*/)
{
	DRIVENOTIFY* pdn = (DRIVENOTIFY*)pNmhdr;
	CIDListPtr idl;
	if (pdn && _make_current_path_pidl(idl, pdn->path)) {
		m_viewFavo.InsertFavoriteItem(idl);
	}
}

LRESULT CMainFrame::RerootCurrentMessage(HWND hwndTo, BOOL bUseSendMessage)
{
	LRESULT lr = 0;
	if (::IsWindow(hwndTo)) {
		const _ATL_MSG* msg = GetCurrentMessage();
		if (bUseSendMessage)
			lr = ::SendMessage(hwndTo, msg->message, msg->wParam, msg->lParam);
		else
			lr = ::PostMessage(hwndTo, msg->message, msg->wParam, msg->lParam);
	}
	return lr;
}

LRESULT CMainFrame::OnHandlerNcPaint(UINT msg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	bHandled = FALSE;
	return 0;
}

LRESULT CMainFrame::OnHandlerCommand(UINT msg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	SWITCH_COMMAND(IDM_EDIT_PASTE, OnEditPaste)
	SWITCH_COMMAND(IDM_EDIT_PASTESHORTCUT, OnEditPasteShortcut)
	SWITCH_COMMAND(IDM_VIEW_DRIVEINFO, OnViewDriveInfo)
	SWITCH_COMMAND(IDM_VIEW_ICON, OnViewIcon)
	SWITCH_COMMAND(IDM_VIEW_SIZE, OnViewSize)
	SWITCH_COMMAND(IDM_VIEW_DATE, OnViewDate)
	SWITCH_COMMAND(IDM_VIEW_ATTRIB, OnViewAttrib)
	SWITCH_COMMAND(IDM_VIEW_NAMEELLIPSIS, OnViewNameEllipsis)
	SWITCH_COMMAND(IDM_VIEW_SORT, OnViewSort)
	SWITCH_COMMAND(IDM_VIEW_DESKTOPTREE, OnViewDesktopTree)
	SWITCH_COMMAND(IDM_VIEW_SAETDEFAULT, OnViewSetDefaultStyle)
	SWITCH_COMMAND(IDM_TYPE_FILESUPERHIDDEN, OnTypeFileSuperHidden)
	SWITCH_COMMAND(IDM_TYPE_FILEHIDDEN, OnTypeFileHidden)
	SWITCH_COMMAND(IDM_TYPE_EXCFOLDER, OnTypeFolder)
	SWITCH_COMMAND(IDM_VIEW_WILDCARD, OnSetFilter)
	SWITCH_COMMAND(IDM_VIEW_WILDCARD_LIST, OnFileFilterList)
	SWITCH_COMMAND(IDM_VIEW_RELEASEFILTER, OnReleaseFileFilter)
	SWITCH_COMMAND(IDM_VIEW_KEYMAP, OnViewKeyMap)
	SWITCH_COMMAND(IDM_OPTION_SHORTCUT, OnShortcutSetup)
	SWITCH_COMMAND(IDM_TOOL_MSDOS, OnExecMsdos)
	SWITCH_COMMAND(IDM_TOOL_SHELL, OnExecShell)
	SWITCH_COMMAND(IDM_EXEC_OWN, OnExecOwn)
#ifdef SUPPORT_FINDURL
	SWITCH_COMMAND(IDM_TOOL_FINDURL, OnToolFindUrl)
#endif
//@	SWITCH_COMMAND(IDM_DISK_COPY, OnDiskCopy)
//@ SWITCH_COMMAND(IDM_DISK_FORMAT, OnDiskFormat)
//@	SWITCH_COMMAND(IDM_DISK_LABEL, OnDiskLabel)
	SWITCH_COMMAND(IDM_FILE_MKDIR, OnMakeDir)
	SWITCH_COMMAND(IDM_WINDOW_MAXIMIZE, OnWindowMaxmize)
	SWITCH_COMMAND(IDM_WINDOW_MINIMIZE, OnWindowMinimize)
	SWITCH_COMMAND(IDM_WINDOW_REFLESH, OnRefresh)
	SWITCH_COMMAND(IDM_WINDOW_COMBINESAVE, OnWindowCombineSave)
	SWITCH_COMMAND(IDM_WINDOW_COMBINELOAD, OnWindowCombineRestore)
	SWITCH_COMMAND(IDM_WINDOW_NEXT, OnNextWindow)
	SWITCH_COMMAND(IDM_WINDOW_PREV, OnPrevWindow)
	SWITCH_COMMAND(IDM_TAB_NEW, OnTabNew)
	SWITCH_COMMAND(IDM_TAB_PREV, OnTabPrev)
	SWITCH_COMMAND(IDM_TAB_NEXT, OnTabNext)
	SWITCH_COMMAND(IDM_TAB_CLOSE, OnTabClose)
	SWITCH_COMMAND(IDM_TAB_CLOSE_ANYOTHER, OnTabCloseAnyOther)
	SWITCH_COMMAND(IDM_TAB_CLOSE_LEFTSIDE, OnTabCloseLeftSide)
	SWITCH_COMMAND(IDM_TAB_CLOSE_RIGHTSIDE, OnTabCloseRightSide)
	SWITCH_COMMAND(IDM_TAB_EXPORT, OnTabExport)
	SWITCH_COMMAND(IDM_TAB_LOCK, OnTabLock)
	SWITCH_COMMAND(IDM_TAB_SAVEALL, OnTabGroupSave)
	SWITCH_COMMAND(IDM_TAB_RESTOREALL, OnTabGroupLoad)
	SWITCH_COMMAND_RANGE(IDM_TAB_SORTDEFAULT, IDM_TAB_SORTREFERENCE, OnTabSortRange)
	SWITCH_COMMAND(ID_WINDOW_CASCADE, OnWindowCascade)
	SWITCH_COMMAND(ID_WINDOW_TILE_HORZ, OnWindowTileHorz)
	SWITCH_COMMAND(ID_WINDOW_TILE_VERT, OnWindowTileVert)
	SWITCH_COMMAND(ID_APP_EXIT, OnAppExit)
	SWITCH_COMMAND(ID_APP_EXITALL, OnAppExitAll)
//@	SWITCH_COMMAND(IDM_DISK_MAKEIMAGE, OnDiskMakeImage)
//@	SWITCH_COMMAND(IDM_DISK_RESTOREIMAGE, OnDiskRestoreImage)
	SWITCH_COMMAND(IDM_DIR_PREV, OnHistoryBack)
	SWITCH_COMMAND(IDM_DIR_NEXT, OnHistoryForward)
	SWITCH_COMMAND(IDM_DIR_SELECT, OnDiskSelect)
//@	SWITCH_COMMAND(IDM_DIR_OPENTHISFOLDER, OnDirOpenAll)
	SWITCH_COMMAND(IDM_DIR_QUICKACCESS, OnQuickAccess)
	SWITCH_COMMAND(IDM_DIR_ADDFAVORITE, OnDirAddFavorite)
	SWITCH_COMMAND(IDM_NET_CONNECT, OnNetConnect)
	SWITCH_COMMAND(IDM_NET_DISCONNECT, OnNetDisconnect)
	SWITCH_COMMAND(IDM_NET_ADDPLACE, OnNetAddPlace)
	SWITCH_COMMAND(IDM_BAR_MENU, OnBarMenu)
	SWITCH_COMMAND(IDM_BAR_PROP, OnBarProp)
	SWITCH_COMMAND(IDM_BAR_SORT, OnBarSort)
	SWITCH_COMMAND(IDM_BAR_SMART, OnBarSmart)
	SWITCH_COMMAND(IDM_BAR_APPBAR, OnBarApp)
	SWITCH_COMMAND(IDM_BAR_APPBAR_SIDECHANGE, OnBarAppSideChange)
	SWITCH_COMMAND(IDM_BAR_STAT, OnBarStat)
	SWITCH_COMMAND(IDM_BAR_FUNCTION, OnBarFunction)
	SWITCH_COMMAND(IDM_BAR_DRIVE, OnBarDrive)
	SWITCH_COMMAND_RANGE(IDM_BAR_DRIVE_HORZ, IDM_BAR_DRIVE_VERTRIGHT, OnBarDriveVertRange)
	SWITCH_COMMAND(IDM_BAR_FAVO, OnBarFavo)
	SWITCH_COMMAND(IDM_BAR_NAVI, OnBarNavi)
	SWITCH_COMMAND(IDM_BAR_LARGEICON, OnBarLargeIcon)
	SWITCH_COMMAND(ID_PANE_CLOSE, OnBarFavo)
	SWITCH_COMMAND(IDM_BAR_REPOSITION, OnBarAutoReposition)
	SWITCH_COMMAND(IDM_BAR_LOCKREBAR, OnBarLock)
	SWITCH_COMMAND(IDM_BAR_SETTINGS, OnBarSettings)
	SWITCH_COMMAND(IDM_FILE_SHELLNEW, OnFileShellNew)
	SWITCH_COMMAND(IDM_FILE_NEWFILE, OnFileNewFile)
	SWITCH_COMMAND(IDM_FILE_SELALL, OnSetSelect)
	SWITCH_COMMAND(IDM_FILE_SELINVERT, OnRevSelect)
	SWITCH_COMMAND(IDM_FILE_MASKSELECT, OnMaskSelect)
	SWITCH_COMMAND(IDM_FILE_MASKSELECT_LIST, OnMaskSelectList)
	SWITCH_COMMAND(IDM_FILE_RENEXT, OnFileRenExt)
	SWITCH_COMMAND(IDM_DIR_UPDIR, OnDirUp)
	SWITCH_COMMAND(IDM_DIR_SCRIPTSYNC, OnDirScriptSync)
	SWITCH_COMMAND(IDM_DIR_OPENSYNCFOLDER, OnOpenFolderSyncDataFolder)
	SWITCH_COMMAND(IDM_OPTION_SHELLEXT, OnOptionShellExt)
	SWITCH_COMMAND(IDM_OPTION_MISC, OnOptionEnv)
	SWITCH_COMMAND(IDM_DIR_TRACE, OnHistoryTrace)
	SWITCH_COMMAND(IDM_DIR_HISTORYCLEAR, OnHistoryClear)
	SWITCH_COMMAND(IDM_DIR_HOME, OnDirHome)
	SWITCH_COMMAND(IDM_DIR_EMPTYRECYCLEBIN, OnDirEmptyRecycleBin)
	SWITCH_COMMAND(IDM_DIR_MAKEBRIEFCASE, OnDirMakeBriefcase)
	SWITCH_COMMAND(IDM_DIR_ROOT, OnDirRoot)
	SWITCH_COMMAND(IDM_DIR_SYNC, OnFolderSync)
	SWITCH_COMMAND(IDM_DIR_SEARCH, OnFileSearch)
	SWITCH_COMMAND(IDM_DIR_GOTO, OnDirGoto)
//@	SWITCH_COMMAND(IDM_DISK_CHKDSK, OnDiskCheck)
	SWITCH_COMMAND(IDM_HELP_SHAREWARE, OnHelpShareware)
	SWITCH_COMMAND(IDM_TOOL_GREP, OnToolGrep)
//@	SWITCH_COMMAND(IDM_TOOL_SLIDESHOW, OnToolSlideShow)
	SWITCH_COMMAND(IDM_WEB_FEEDBACK, OnWebFeedBack)
	SWITCH_COMMAND(IDM_WEB_DOWNLOAD, OnWebDownload)
	SWITCH_COMMAND(IDM_WEB_HOMEPAGE, OnWebHomePage)
	SWITCH_COMMAND(ID_APP_ABOUT, OnAppAbout)
	SWITCH_COMMAND(IDM_VIEW_HOTKEYMENU, OnPopupHotkeyMenu)
	SWITCH_COMMAND(IDM_OPTION_KEYMAP, OnEditKeyMap)
//@	SWITCH_COMMAND(ID_HELP, CFrameWnd::OnHelp)
	SWITCH_COMMAND(ID_HELP_FINDER, fvApp.WinHelpFinder)
//@	SWITCH_COMMAND(ID_CONTEXT_HELP, CFrameWnd::OnContextHelp)
//@	SWITCH_COMMAND(ID_DEFAULT_HELP, CFrameWnd::OnHelpFinder)
	SWITCH_COMMAND(IDM_ICON_ENLARGE, OnIconEnlarge)
	SWITCH_COMMAND(IDM_TAB_REOPEN_RECENT, OnCommandTabReopenRecent)
	SWITCH_COMMAND_RANGE(IDM_ICON_NORMAL, IDM_ICON_ICON, OnIconModeRange)
	SWITCH_COMMAND_RANGE(IDM_HOT_ICON_NORMAL, IDM_HOT_ICON_SWITCH, OnHotIconModeRange)
	SWITCH_COMMAND_RANGE(IDM_SORT_FIRST, IDM_SORT_LAST, OnSortRange)
	SWITCH_COMMAND_RANGE(IDW_TBFIRST, IDW_TBLAST, OnToolBarMenu)
	SWITCH_COMMAND_RANGE(IDM_SSYNC_START, IDM_SSYNC_END, OnDoScriptSync)
	SWITCH_COMMAND_RANGE(IDM_HOTKEY_FIRST, IDM_HOTKEY_LAST, OnHotKeyRange)
	SWITCH_COMMAND_RANGE(IDM_SPFL_START, IDM_SPFL_END, OnCommandSpecialFolder)
	SWITCH_COMMAND_RANGE(IDM_SHLNEW_START, IDM_SHLNEW_END, OnCommandShellNew)
	SWITCH_COMMAND_RANGE(IDM_DRVACCESS_A, IDM_DRVACCESS_Z, OnCommandDriveAccess)
	SWITCH_COMMAND_RANGE(IDM_TAB_REOPEN_FIRST, IDM_TAB_REOPEN_LAST, OnCommandTabReopenRange)
	SWITCH_COMMAND_RANGE(IDM_TAB_GROUP_FIRST, IDM_TAB_GROUP_LAST, OnCommandTabGroupRange)
	SWITCH_COMMAND_RANGE(IDM_IMAGE_072, IDM_IMAGE_192, OnThumbnailSizeRange)
	SWITCH_COMMAND(IDM_IMAGE_ZOOMIN, OnThumbnailZoomIn)
	SWITCH_COMMAND(IDM_IMAGE_ZOOMOUT, OnThumbnailZoomOut)
	// ドライブバーのシェブロンメニューで選択された場合の処理
	SWITCH_COMMAND_RANGE(IDM_DRIVEBAR_IDSTART, IDM_DRIVEBAR_IDEND, OnDriveBarCommandRange)
	// ナビバー
	SWITCH_COMMAND(IDM_HOT_NAVIBAR, OnNaviBarClicked)
	SWITCH_COMMAND_CODE(IDW_NAVIGATIONBAR, CBN_DROPDOWN, OnNaviBarClicked)
	SWITCH_COMMAND_CODE(IDW_NAVIGATIONBAR, CBN_SETFOCUS, OnNaviBarSetFocus)
	SWITCH_COMMAND(IDM_ICON_RELAX, OnIconRelax)

	CWindow view = m_ViewSwitch.GetCurrentView();
	if (view.IsWindow() &&
		view.IsWindowVisible() &&
		(view == GetCurrentFileBrowser() || view == GetFolderTree()) &&
		view.SendMessage(WM_COMMAND, wParam, lParam))
	{
		return TRUE;
	}
	bHandled = FALSE;
	return FALSE;
}

LRESULT CMainFrame::OnHandlerMenuSelect(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& bHandled)
{
	return 0;
}

BOOL CMainFrame::CloseApp()
{
	if (fvApp.m_nProcessCounter > 0 || fvApp.GetOperationThreadCount() > 0) {
		AppMessage(IDS_FILEOP_UNDERPROCESSING, NULL, MB_STOP);
		return FALSE;
	}

	// 複数のタブがある場合、タブを閉じるか確認する
	if (!sbForceExit && !(gnTabStat & TAB_CLOSENOCONFIRM) && (m_tabbar.GetCount() > 1)) {
		CTabCloseAllDlg dlg;
		if (dlg.DoModal() != IDOK) {
			return FALSE;
		}
		if (!dlg.m_bConfirmAnyTime)
			gnTabStat |= TAB_CLOSENOCONFIRM;
	}

	// プロファイル保管
	SaveProfile();
	return TRUE;
}

void CMainFrame::OnClose(BOOL& bHandled)
{
	if (!CloseApp())
		bHandled = TRUE;
}

void CMainFrame::ClearDocumentHistoryAll()
{
	// 各タブのフォルダ履歴を消去
	int cnt = m_tabbar.GetCount();
	for (int i = 0; i < cnt; i++) {
		CDocument* pDoc = (CDocument*)m_tabbar.GetItemData(i);
		if (pDoc) {
			CHistory* pHistory = pDoc->GetHistory();
			if (pHistory)
				pHistory->Reset();
		}
	}

	// 閉じたタブの履歴を消去
	m_arrTabReopen.DeleteAll();
	// XML-DB更新
	SaveTabReopenArray(m_arrTabReopen, 0);
	CreateJumpList();
}

void CMainFrame::ResetFolderSettings()
{
	// フォルダの設定を消去
	m_mapListStyle.DiscardAllSettings();
	m_mapListStyle.Save();
}

void CMainFrame::CleanUp()
{
	m_arrSysFolder.DeleteAll();
	m_arrSSync.DeleteAll();
	m_arrTabGroup.DeleteAll();
	m_arrTabReopen.DeleteAll();

	int cnt = m_arrShellNew.GetSize();
	int i;
	for (i = 0; i < cnt; i++)
		delete m_arrShellNew[i];
	m_arrShellNew.RemoveAll();

	// 動的に作成したツールバーの削除
	cnt = m_arrToolbar.GetSize();
	for (i = 0; i < cnt; i++)
		delete m_arrToolbar[i];
	m_arrToolbar.RemoveAll();

	::RemoveProp(m_hWnd, APP_PRODUCT);

	// タブバー
	cnt = m_tabbar.GetCount();
	for (i = 0; i < cnt; i++) {
		CDocument* pDoc = (CDocument*)m_tabbar.GetItemData(i);
		if (pDoc) {
			m_tabbar.SetItemData(i, NULL);
			delete pDoc;
		}
	}
	m_tabbar.DeleteAllItems();

	// タイマー関連
	if (m_timerid_combine) {
		KillTimer(m_timerid_combine);
		m_timerid_combine = 0;
	}
}

void CMainFrame::OnEndSession(BOOL bEnding)
{
	if (bEnding) {
		SaveProfile();
		CleanUp();
	}
	DefWindowProc();
}

void CMainFrame::OnDestroy()
{
	CleanUp();
}

void CMainFrame::OnDropFiles(HDROP hdrop)
{
	if (hdrop) {
		DragFinish(hdrop);
	}
}

void CMainFrame::SaveProfile()
{
	// 終了時のタブ構成を保存
	CString str;
	if (GetLocalAppDataFolder(str, sszSessionGroup))
		TabGroupSave(str);

	// ステータス保存
	fvApp.SaveStates();

	// ソート情報等
	CFileBrowseWindow browser = GetCurrentFileBrowser();
	browser.SaveSettings();

	// フォルダのリストスタイル情報
	m_mapListStyle.Save();

	// 最終アクセスパス
	CIDListPtrHandle idlCur = browser.GetCurrentFolder();
	if (!idlCur.isNull()) {
		bool bRemote = false;

		// 現在のフォルダが実フォルダでないか、実フォルダでも固定ディスク
		// の場合には情報を保存
		if (idlCur.GetPath(str))
			bRemote = (PathIsUNC(str) || DriveType(PathGetDriveNumber(str)) != DRIVE_FIXED);
		else {
			int descid = idl_GetDescriptionId(idlCur);
			if (descid != 0 && (SHDID_NET_DOMAIN < descid && descid < SHDID_NET_OTHER))
				bRemote = true;
		}

		// ネットワーク配下は保存しない
		LPITEMIDLIST pidlNetwork;
		BOOL onnet = FALSE;
		if (SHGetFolderLocation(NULL, CSIDL_NETWORK, NULL, 0, &pidlNetwork) == S_OK) {
			onnet = idlCur.isEqual(pidlNetwork);
			if (!onnet)
				onnet = idlCur.isParent(pidlNetwork);
			CoTaskMemFree(pidlNetwork);
		}
		if (!onnet) {
			int num = GetWindowNumCount(APP_FRAMECLASS, NULL, FALSE);
			SetRegBinarySeq(IDS_SEC_FOLDER, IDS_KEY_LASTACCESS, num - 1, (BYTE*)idlCur.GetData(), idlCur.GetSize());
		}
	}

	// オブジェクトパネルの状態を保存
	m_appbar.SaveState();
	// リバーバンドとツールバーの状態を保存
	SaveRebarSettings();

	// 現在のウィンドウ位置情報を保存
	CRect rc;
	if ((GetStyle() & (WS_VISIBLE | WS_MINIMIZE | WS_MAXIMIZE)) == WS_VISIBLE) {
		WINDOWPLACEMENT wpm;
		wpm.length = sizeof(wpm);
		GetWindowPlacement(&wpm);

		int sib = GetWindowNumCount(APP_FRAMECLASS, NULL, FALSE);
		SetRegBinarySeq(IDS_SEC_SETTINGS, IDS_KEY_FRAMEPOS, sib - 1, &wpm, sizeof(wpm));
	}

	// フォルダビュー分割位置記憶
	CWindow wnd = m_splitView.GetSplitterPane(SPLIT_PANE_LEFT);
	wnd.GetWindowRect(&rc);
	SetRegValue(IDS_SEC_SETTINGS, IDS_KEY_TREEWIDTH, rc.Width());

	// お気に入りビュー分割位置記憶
	if (m_viewFavo.IsWindowVisible()) {
		int pos = m_splitTree.GetSplitterPos();
		m_splitTree.GetWindowRect(&rc);
		SetRegValue(IDS_SEC_SETTINGS, IDS_KEY_FAVOHEIGHT, rc.Height() - pos);
	}

	// オブジェクトバー分割位置記憶
	if (m_appbar.IsWindowVisible()) {
		m_appbar.GetWindowRect(&rc);
		SetRegValue(IDS_SEC_OBJECTBAR, IDS_KEY_WINDOWWIDTH, rc.Width());
	}
}

void CMainFrame::GetBrowserPaneRect(RECT* prc)
{
	CWindow wnd = m_splitView.GetSplitterPane(SPLIT_PANE_RIGHT);
	wnd.GetClientRect(prc);
}

void CMainFrame::SaveRebarSettings()
{
	// ツールバーの状態を保存
	SaveToolBarStateAll();
	// リバーバンド状態を保存
	CRebarSettings rs;
	rs.GetFrom(m_rebar);
	rs.Save(_T("RebarSettings"), _T("Toolbar"));
}

int CMainFrame::ListupShellNewItems()
{
	int cnt = m_arrShellNew.GetSize();
	if (cnt > 0) return cnt;

	cnt = 0;
	TCHAR* exts = new TCHAR[4096];
	if (shnEnumExtension(exts) > 0) {
		LPTSTR pw = exts;
		CShellNewItem* psi;
		while (*pw != NULL) {
			psi = new CShellNewItem();
			psi->Initialize(pw);
			m_arrShellNew.Add(psi);
			pw += lstrlen(pw) + 1;
		}
	}
	delete [] exts;

	return m_arrShellNew.GetSize();
}

int CMainFrame::InitShellNewMenu(CMenuHandle pop)
{
	bool bInitDone = true;
	int i, cnt = pop.GetMenuItemCount();
	for (i = 0; i < cnt; i++) {
		if (IDM_SHLNEW_EMPTY == pop.GetMenuItemID(i)) {
			pop.DeleteMenu(IDM_SHLNEW_EMPTY, MF_BYCOMMAND);
			bInitDone = false;
			break;
		}
	}

	if (!bInitDone) {
		cnt = ListupShellNewItems();
		if (cnt > 0) {
			if (pop.GetMenuItemCount() > 0)
				pop.AppendMenu(MF_SEPARATOR);
			for (i = 0; i < cnt; i++) {
				CShellNewItem* psi = m_arrShellNew[i];
				CString s = psi->FormatNameExtension();
				pop.AppendMenu(MF_STRING, i + IDM_SHLNEW_START, s);
				psi->GetImageIndex(true);
				if (!psi->m_bmIcon.IsNull())
					SetBitmapToMenuItem(pop, i + IDM_SHLNEW_START, psi->m_bmIcon);
			}
		}
	}
	return pop.GetMenuItemCount();
}

LRESULT CMainFrame::OnInitMenuPopup(UINT msg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	if (!HIWORD(lParam)) {
		int i, n, posIns, cnt;
		CString str;
		TCHAR buf[MAX_PATH];
		CMenuHandle pop = (HMENU)wParam;
		// -------------------------
		// 新規作成
		// -------------------------
		if (GetSubSequenceMenu(0, 3) == pop) {
			InitShellNewMenu(pop);
		} else if (m_scutTree.GetShortcutMenu() == pop || m_scutFile.GetShortcutMenu() == pop) {
			cnt = pop.GetMenuItemCount();
			for (i = 0; i < cnt; i++) {
				if (IDM_FILE_SHELLNEW == pop.GetMenuItemID(i)) {
					CString s;
					pop.GetMenuString(IDM_FILE_SHELLNEW, s, MF_BYCOMMAND);
					CMenuHandle sub = ::LoadMenu(RESINST, MAKEINTRESOURCE(ID_MENU_SHELLNEW));
					pop.InsertMenu(IDM_FILE_SHELLNEW, MF_BYCOMMAND, sub, s.GetString());
					pop.DeleteMenu(IDM_FILE_SHELLNEW, MF_BYCOMMAND);
					InitShellNewMenu(sub);
					break;
				}
			}
		}
		// -------------------------
		// システムフォルダ
		// -------------------------
		else if (GetSubSequenceMenu(2, 5) == pop) {
			if (m_arrSysFolder.GetSize() < 1) {
				CSimpleArray<HMENU> stack;
				stack.Add(pop.m_hMenu);
				CMenuHandle popCur = pop.m_hMenu;

				cnt = ARRAYSIZE(stKnownFolder);
				CIDListPtr idl;
				for (i = 0; i < cnt; i++) {
					if (stKnownFolder[i].guid == FID_separator) {
						popCur.AppendMenu(MF_SEPARATOR | MF_BYPOSITION, (UINT_PTR)0, (LPCTSTR)NULL);
					} else if (stKnownFolder[i].guid == FID_submenu) {
						HMENU hmenu = CreatePopupMenu();
						MENUINFO mnfo;
						mnfo.cbSize = sizeof(mnfo);
						mnfo.fMask = MIM_STYLE;
						mnfo.dwStyle = MNS_CHECKORBMP;
						SetMenuInfo(hmenu, &mnfo);
						str.LoadString(stKnownFolder[i].idString);
						popCur.AppendMenu(MF_POPUP | MF_BYPOSITION, (UINT_PTR)hmenu, str);
						stack.Add(hmenu);
						popCur = hmenu;
					} else if (stKnownFolder[i].guid == FID_null) {
						int m = stack.GetSize();
						if (m > 1) {
							stack.RemoveAt(--m);
							popCur = stack[m - 1];
						}
					} else if (idl.GetKnownFolder(stKnownFolder[i].guid) == S_OK) {
						CCmItem* pci = new CCmItem(idl);
						idl.Free();

						if (stKnownFolder[i].idString)
							pci->m_name.LoadString(stKnownFolder[i].idString);

						m_arrSysFolder.Add(pci);

						popCur.AppendMenu(MF_STRING, IDM_SPFL_START + i, pci->m_name);
						pci->GetImageIndex(true);
						if (!pci->m_bmIcon.IsNull())
							SetBitmapToMenuItem(popCur, IDM_SPFL_START + i, pci->m_bmIcon);
					}
				}
				::DeleteMenu((HMENU)wParam, 0, MF_BYPOSITION);
			}
		}
		// -------------------------
		// 同期スクリプト
		// -------------------------
		else if (GetSubSequenceMenu(2, 10) == pop) {
			n = pop.GetMenuItemCount();
			for (posIns = 0; posIns < n; posIns++) {
				if (IDM_SSYNC_START == pop.GetMenuItemID(posIns))
					break;
			}

			if (posIns < n) {
				for (i = IDM_SSYNC_START; i <= IDM_SSYNC_END; i++) {
					if (!pop.DeleteMenu(i, MF_BYCOMMAND))
						break;
				}
			}

			m_arrSSync.DeleteAll();
			TCHAR dir[MAX_PATH];
			fopGetSyncDataFolder(dir);
			PathCombine(buf, dir, sszFSS);

			i = 0;
			CFileFinder ff;
			if (ff.BeginFind(buf)) {
				do {
					PathCombine(buf, dir, ff.cFileName);
					CIDListPtr idl;
					if (idl.CreateFromPath(buf)) {
						CCmItem* pci = new CCmItem(idl);
						idl.Free();

						m_arrSSync.Add(pci);
						pop.AppendMenu(MF_STRING, i + IDM_SSYNC_START, pci->m_name);
						pci->GetImageIndex(true);
						if (!pci->m_bmIcon.IsNull())
							SetBitmapToMenuItem(pop, IDM_SSYNC_START + i, pci->m_bmIcon);
						i++;
					}
				} while (ff.NextFind());
				ff.EndFind();
			}

			if (i == 0) {
				str.LoadString(IDS_UNDEFINED);
				pop.AppendMenu(MF_BYPOSITION | MF_GRAYED, IDM_SSYNC_START, str);
			}
		}
		// -------------------------
		// カスタムツールバー
		// -------------------------
		else if (GetSubSequenceMenu(3, 4) == pop) {
			n = pop.GetMenuItemCount();
			for (posIns = 0; posIns < n; posIns++) {
				if (IDW_TBFIRST == pop.GetMenuItemID(posIns))
					break;
			}

			if (posIns < n) {
				for (i = IDW_TBFIRST; i <= IDW_TBLAST; i++) {
					// keep the first one as an insertion point
					if (i != IDW_TBFIRST)
						pop.DeleteMenu(i, MF_BYCOMMAND);
				}
			}
			else
				posIns = 0;

			cnt = m_arrToolbar.GetSize();
			if (cnt < 1) {
				str.LoadString(IDS_UNDEFINED);
				pop.InsertMenu(posIns, MF_BYPOSITION | MF_GRAYED, IDW_TBFIRST, str);
			} else {
				CToolBarCtrl* pBar;
				UINT flg;
				for (i = 0; i < cnt; i++) {
					pBar = m_arrToolbar[i];
					pBar->GetWindowText(buf, ARRAYSIZE(buf));
					flg = MF_BYPOSITION;
					if (pBar->IsWindowVisible())
						flg |= MF_CHECKED;
					pop.InsertMenu(posIns++, flg, i + IDW_TBFIRST, buf);
				}
			}
			pop.DeleteMenu(posIns, MF_BYPOSITION);
		}
		// -------------------------
		// ソートメニュー
		// -------------------------
		else if (GetSubSequenceMenu(3, 7) == pop) {
			GetCurrentFileBrowser().BuildSortMenu(pop.m_hMenu);
			__UpdateMenuAccelerator(pop.m_hMenu, true);
		}
		// -------------------------
		// 最近閉じたタブ
		// -------------------------
		else if (GetSubSequenceMenu(5, 2) == pop) {
			n = pop.GetMenuItemCount();
			for (posIns = 0; posIns < n; posIns++) {
				if (IDM_TAB_REOPEN_FIRST == pop.GetMenuItemID(posIns))
					break;
			}

			if (posIns < n) {
				for (i = IDM_TAB_REOPEN_FIRST; i <= IDM_TAB_REOPEN_LAST; i++)
					pop.DeleteMenu(i, MF_BYCOMMAND);
			}

			cnt = m_arrTabReopen.GetItemCount();
			if (cnt > 0) {
				for (i = 0; i < cnt; i++) {
					CCmItem* pci = m_arrTabReopen.GetItemAt(i);
					pop.AppendMenu(MF_STRING, i + IDM_TAB_REOPEN_FIRST, pci->m_name);
					pci->GetImageIndex(true);
					if (!pci->m_bmIcon.IsNull())
						SetBitmapToMenuItem(pop, i + IDM_TAB_REOPEN_FIRST, pci->m_bmIcon);
				}
			} else {
				str.LoadString(IDS_UNDEFINED);
				pop.AppendMenu(MF_BYPOSITION | MF_GRAYED, IDM_TAB_REOPEN_FIRST, str);
			}
		}
		// -------------------------
		// タブ グループ
		// -------------------------
		else if (GetSubSequenceMenu(5, 3) == pop) {
			cnt = pop.GetMenuItemCount();
			for (i = IDM_TAB_GROUP_FIRST; i <= IDM_TAB_GROUP_LAST; i++) {
				if (pop.GetMenuState(i, MF_BYCOMMAND) != 0xFFFFFFFF)
					pop.DeleteMenu(i, MF_BYCOMMAND);
			}

			// タブ グループ ファイルの列挙
			int numGroups = EnumTabGroup(m_arrTabGroup);
			for (i = 0; i < numGroups; i++) {
				CCmItem* pci = m_arrTabGroup[i];
				lstrcpy(buf, pci->m_name);
				PathRemoveExtension(buf);
				pop.AppendMenu(MF_STRING, i + IDM_TAB_GROUP_FIRST, buf);
				pci->GetImageIndex(true);
				if (!pci->m_bmIcon.IsNull())
					SetBitmapToMenuItem(pop, i + IDM_TAB_GROUP_FIRST, pci->m_bmIcon);
			}

			if (i == 0) {
				str.LoadString(IDS_UNDEFINED);
				pop.AppendMenu(MF_BYPOSITION | MF_GRAYED, IDM_TAB_GROUP_FIRST, str);
			}
		}
		// -------------------------
		// それ以外のメニュー
		// -------------------------
		else {
			cnt = pop.GetMenuItemCount();
			for (i = 0; i < cnt; i++) {
				UINT id = pop.GetMenuItemID(i);
				CMenuItemInfo mii;
				mii.fMask = MIIM_BITMAP;
				pop.GetMenuItemInfo(i, TRUE, &mii);
				if (!mii.hbmpItem) {
					if (_INRANGE(id, IDM_DRIVEBAR_IDSTART, IDM_DRIVEBAR_IDEND)) {
						mii.hbmpItem = m_drive.GetMenuImageBitmap(id);
					} else if (_INRANGE(id, IDM_SMARTBAR_IDSTART, IDM_SMARTBAR_IDEND)) {
						mii.hbmpItem = m_smartbar.GetMenuImageBitmap(id);
					} else if (_INRANGE(id, IDM_HISTORY_START, IDM_HISTORY_END)) {
						CHistory* pHistory = GetHistory();
						CCmItem* pci = pHistory->FindBy(FB_ID, id);
						if (pci) {
							pci->GetImageIndex(true);
							mii.hbmpItem = pci->m_bmIcon;
						}
					}
					if (mii.hbmpItem) {
						pop.SetMenuItemInfo(i, TRUE, &mii);
					}
				}
			}
		}
	}
	bHandled = FALSE;
	return 0;
}

void CMainFrame::OnSetFocus(HWND hOldWnd)
{
	m_ViewSwitch.GiveFocus();
}

void CMainFrame::OnDrawItem(int idCtrl, LPDRAWITEMSTRUCT pdi)
{
	if (idCtrl == ATL_IDW_STATUS_BAR) {
		m_statbar.DrawItem(pdi);
	}
}

void CMainFrame::GetSelectionCenterPoint(LPPOINT ppt)
{
	CFileBrowseWindow browser = GetCurrentFileBrowser();
	CFolderTreeWindow tree = GetFolderTree();

	// ポップアップメニューを表示する位置を求める
	// マウスカーソルがフォーカスを持つウィンドウにあれば、マウスカーソル位置に、
	// そうでなければ、フォーカスを持つウィンドウの選択項目の中心を座標として求める
	GetCursorPos(ppt);
	CPoint pt = *ppt;
	bool bInView;
	bool bInApp = ((int)SendMessage(WM_NCHITTEST, 0, MAKELPARAM(pt.x, pt.y)) > 0);
	HWND hwnd = m_ViewSwitch.GetCurrentView();
	if (hwnd == tree) {
		bInView = ((int)tree.SendMessage(WM_NCHITTEST, 0, MAKELPARAM(pt.x, pt.y)) > 0);
		if (!bInApp || !bInView)
			tree.GetSelectionCenterPoint(ppt);
	}
	else if (hwnd == browser) {
		bInView = ((int)browser.SendMessage(WM_NCHITTEST, 0, MAKELPARAM(pt.x, pt.y)) > 0);
		if (!bInApp || !bInView)
			browser.GetSelectionCenterPoint(ppt);
	}
	else {
		bInView = ((int)::SendMessage(hwnd, WM_NCHITTEST, 0, MAKELPARAM(pt.x, pt.y)) > 0);
		if (!bInApp || !bInView) {
			CRect rc;
			::GetWindowRect(hwnd, &rc);
			ppt->x = (rc.left + rc.right) / 2;
			ppt->y = (rc.top + rc.bottom) / 2;
		}
	}
}

void CMainFrame::OnTimer(UINT_PTR idEvent)
{
	if (idEvent == m_timerid_combine) {
		CSimpleArray<HWND> arrHwnd;
		int cnt = GetRegValue(IDS_SEC_COMBINE, IDS_KEY_ENTRY, 0);
		if (GetSiblings(NULL, arrHwnd, true) < cnt)
			return;

		KillTimer(m_timerid_combine);
		m_timerid_combine = 0;

		CRect rc;
		CSimpleArray<CString> arr;
		CString str;
		HDWP hdwp = ::BeginDeferWindowPos(cnt);
		for (int i = 0; i < cnt; i++) {
			arr.RemoveAll();
			int len = GetRegTextSeq(IDS_SEC_COMBINE, IDS_KEY_FRAMEPOS, str, i);
			if (len < 1 || SeparateString(str, _T(','), arr) != 4)
				break;

			CWindow wnd = arrHwnd[i];
			if (wnd.IsZoomed()) {
				wnd.ShowWindow(SW_RESTORE);
				wnd.UpdateWindow();
			}
			HDWP htmp = wnd.DeferWindowPos(hdwp, NULL, StrToInt(arr[0]), StrToInt(arr[1]), StrToInt(arr[2]), StrToInt(arr[3]), SWP_NOZORDER | SWP_NOACTIVATE);
			if (!htmp)
				break;
			hdwp = htmp;
		}
		EndDeferWindowPos(hdwp);
	}
}

void CMainFrame::OnHotKeyRange(UINT nID, UINT code, HWND hwnd)
{
	RerootCurrentMessage(GetCurrentFileBrowser());
}

void CMainFrame::OnPopupHotkeyMenu()
{
	if (fvApp.menuHotKey.IsMenuEnabled()) {
		CPoint pt;
//@ --- ツールバーボタンをドロップダウン形式にしたので不要
//@		if (!GetToolBarButtonPoint(GetCurrentMessage(), IDM_VIEW_HOTKEYMENU, &pt))
//@			GetCursorPos(&pt);
			GetSelectionCenterPoint(&pt);
		CHotKeyTrackWnd wnd;
		wnd.DoTrack(m_hWnd, &fvApp.menuHotKey, pt);
	}
	else if (AppMessage(IDS_HOTKEY_ENTER_EDIT, NULL, MB_QYESNO) == IDYES) {
		OnOptionEnv(CHotkeyPageDlg::IDD);
	}
}

BOOL CMainFrame::SelectFolderItem(LPCITEMIDLIST pidlFullyQual)
{
	if (!pidlFullyQual)
		return FALSE;

	CIDListPtr idl;
	idl.Clone(pidlFullyQual);

	BOOL bOK = FALSE;
	if (idl.isDesktop())
		bOK = OpenFolder(idl);
	else {
		CIDListPtr idlSelf;
		if (idlSelf.Clone(idl.FindLastID())) {
			idl.RemoveLastID();

			CFileBrowseWindow browser = GetCurrentFileBrowser();
			CIDListPtrHandle idlCur = browser.GetCurrentFolder();
			if (idl.isEqual(idlCur) || OpenFolder(idl)) {
				browser.SelectItem(idlSelf);
				// ファイルペインをアクティブに
				m_ViewSwitch.SetActiveView(browser);
				bOK = TRUE;
			}
		}
	}
	return bOK;
}

LRESULT CMainFrame::OnJump(WPARAM /*wParam*/, LPARAM lParam)
{
	if (lParam) {
		CIDListPtr idl;
		if (idl.CreateFromPath((LPCTSTR)lParam))
			SelectFolderItem(idl);
	}
	return 0;
}

LRESULT CMainFrame::OnOpenTabFolder(WPARAM wParam, LPARAM lParam)
{
	CIDListPtr idl;
	if (idl.Clone((LPITEMIDLIST)lParam))
		TabNew(idl, (wParam != 0));
	return TRUE;
}

LRESULT CMainFrame::OnOpenFolder(WPARAM, LPARAM lParam)
{
	CIDListPtr idl;
	if (idl.Clone((LPITEMIDLIST)lParam)) {
		if (m_tabbar.isItemLocked(m_tabbar.GetCurSel()))
			return TabNew(idl);
		else
			return OpenFolder(idl);
	}
	return FALSE;
}

BOOL CMainFrame::ChangeFolder(LPCITEMIDLIST pidlFq)
{
	if (!pidlFq) return FALSE;

	// 指定のフォルダでEnumObjectsできるかチェック
	CIDListPtrHandle phw(pidlFq);
	if (!phw.isBrowsable(m_hWnd)) {
		return FALSE;
	}

	// 履歴リスト取得
	CHistory* pHistory = GetHistory();
	ATLASSERT(pHistory != NULL);

	// 現在の情報を保存
	CFileBrowseWindow browser = GetCurrentFileBrowser();
	CIconListStyle ls;
	browser.GetListStyle(&ls);

	CCmItem* pci = pHistory->GetCurrentItem();
	if (pci && !pci->m_idl.isEqual(pidlFq)) {
		// 履歴の前・次でフォルダを変更すると、履歴ポインタの指す位置が問題となる
		// このためILIsEqualでチェックを追加している
		m_mapListStyle.SetItemStyle(pci->m_idl, ls);
	}

	// （もし情報が記録されていれば）リスト状態を復元
	if ((gnListStat & LTS_EACH_ICONSTYLE) && !m_mapListStyle.GetItemStyle(pidlFq, ls))
		ls = gDefaultIconListStyle;
	browser.SetListStyle(&ls);
	// 履歴の更新
	pHistory->PushItem(pidlFq);

	// ドライブバーの選択状態更新
	m_drive.SelectDrive(pidlFq);

	// フォルダをブラウズ
	if (!m_bIgnoreFolderBrowsing) {
		if (gnFopStat & FOS_RESETFILTER) {
			// フォルダを変更したらフィルタをリセット
			CFilterSetting* wildcard = GetDocument()->GetFilter();
			wildcard->Clear();
		}
		browser.Browse(pidlFq);
	}
	// タイトル更新
	UpdateTitle();
	return TRUE;
}

LRESULT CMainFrame::OnShellItemSearch(WPARAM wParam, LPARAM lParam)
{
	gnNetSearchResult = (int)wParam;
	return 0;
}

BOOL CMainFrame::OpenFolder(LPCITEMIDLIST pidl, LPCITEMIDLIST pidlRoot, bool bSelectTreeNode)
{
	if (!ChangeFolder(pidl)) return FALSE;

	// ナビバーがアクティブであればフォーカスを変更する
	if (m_navibar == m_ViewSwitch.GetCurrentView())
		m_ViewSwitch.SetActiveView(GetCurrentFileBrowser());

	if (!bSelectTreeNode) return TRUE;

	// ルート選択
	CIDListPtr idl;
	if (gnSysStat & STS_DRIVEROOT) {
		// 現在のツリールート
		CIDListPtr idlTreeRoot;
		idlTreeRoot.Clone(GetFolderTree().GetRoot());

		if (pidlRoot) {
			idl.Clone(pidlRoot);
			if (idl != idlTreeRoot)
				GetFolderTree().SetRoot(idl);
		}
		else {
			idl.Clone(m_drive.GetCurrentDrive());
			if (!idl.isNull()) {
				GetFolderTree().SetRoot(idl);
			}
			else {
				if (!idlTreeRoot.isChild(pidl)) {
					idl.Clone(pidl);
					idl.MakeRoot();
					GetFolderTree().SetRoot(idl);
				}
				else
					idl.Free();
			}
		}
		// ドキュメントのルートフォルダを保存
		GetDocument()->SetRootFolder(idl);
		// カレントフォルダをツリーで選択
		return GetFolderTree().SelectFolder(pidl);
	}
	else {
		CIDListPtrHandle idlTemp = GetDocument()->GetRootFolder();
		if (!idlTemp.isNull() && !idlTemp.isChild(pidl)) {
			// ドキュメントのルートフォルダをクリア
			GetDocument()->SetRootFolder(NULL);
		}
	}

	// ツリーでフォルダを選択
	if (gnNetSearchResult < 1) {
		// まだネット探索が完了していない or ネット探索できなければ
		idl.GetFolderLocation(NULL, CSIDL_NETWORK);
		if (idl.isChild(pidl))
			return GetFolderTree().SelectFolder(idl);
	}
	return GetFolderTree().SelectFolder(pidl);
}

BOOL CMainFrame::OpenChildFolder(LPCITEMIDLIST pidl)
{
	CIDListPtr idl; idl.Clone(pidl);
	if (m_tabbar.isItemLocked(m_tabbar.GetCurSel()))
		return TabNew(idl);
	return OpenFolder(idl);
}

HMENU CMainFrame::GetSubSequenceMenu(int iMain, int iSub)
{
	HMENU hmenu = m_menubar.GetMenu();
	if (hmenu) {
		if (iMain < 0) return hmenu;
		hmenu = ::GetSubMenu(hmenu, iMain);
		if (hmenu) {
			if (iSub < 0) return hmenu;
			return ::GetSubMenu(hmenu, iSub);
		}
	}
	return NULL;
}

void CMainFrame::TrackMenuForFileBrowser(CPoint pt)
{
	HMENU hmenu = m_scutFile.CreateShortcutMenu();
	if (IsMenu(hmenu)) {
		TrackMenu(hmenu, TPM_RIGHTBUTTON | TPM_VERTICAL | TPM_LEFTALIGN | TPM_TOPALIGN, pt);
		m_scutFile.DestroyShortcutMenu();
	}
}

void CMainFrame::TrackMenuForFolderTree(CPoint pt)
{
	HMENU hmenu = m_scutTree.CreateShortcutMenu();
	if (IsMenu(hmenu)) {
		TrackMenu(hmenu, TPM_RIGHTBUTTON | TPM_VERTICAL | TPM_LEFTALIGN | TPM_TOPALIGN, pt);
		m_scutTree.DestroyShortcutMenu();
	}
}

BOOL CMainFrame::TrackMenu(HMENU hmenu, UINT fuFlags, CPoint pt, bool bApplyAccel, TRACKMENUCUSTOMPROC pfn)
{
	if (::IsMenu(hmenu)) {
		if (pfn)
			pfn(hmenu);
		if (bApplyAccel)
			__UpdateMenuAccelerator(hmenu, true);
		return m_menubar.TrackPopupMenu(hmenu, fuFlags, pt.x, pt.y);
	}
	return FALSE;
}

BOOL CMainFrame::TrackMenu(UINT idPopup, UINT fuFlags, CPoint pt, bool bApplyAccel, TRACKMENUCUSTOMPROC pfn)
{
	CMenu menu;
	menu.LoadMenu(idPopup);
	return TrackMenu(menu.GetSubMenu(0), fuFlags, pt, bApplyAccel, pfn);
}

void CMainFrame::OnContextMenu(HWND hWnd, CPoint point)
{
	if (SendMessage(WM_NCHITTEST, 0, MAKELPARAM(point.x, point.y)) == HTCLIENT) {
		CRect rc;
		if (m_tabbar.IsWindowVisible()) {
			int indexCur = m_tabbar.GetCurSel();
			m_tabbar.GetWindowRect(&rc);
			if (rc.PtInRect(point)) {
				// クリック位置のインデクスを取得
				CLccTabExCtrl tab = m_tabbar.GetTabCtrl();
				TC_HITTESTINFO hit;
				hit.pt = point;
				tab.ScreenToClient(&hit.pt);
				int index = tab.HitTest(&hit);
				if (index < 0)
					index = indexCur;

				m_tabbar.SetHilightItem(index);
				UpdateTabMenu(index);		// タブメニューの属性を更新
				int cmd = (int)TrackMenu(ID_MENU_TABBAR, TPM_RETURNCMD | TPM_RIGHTBUTTON | TPM_VERTICAL | TPM_LEFTALIGN | TPM_TOPALIGN, point);
				m_tabbar.ResetHilightItem();

				if (cmd > 0) {
					switch (cmd) {
						case IDM_TAB_REFRESH:			TabRefresh(index); 	break;
						case IDM_TAB_CLOSE_ANYOTHER:	TabCloseAnyOther(index); break;
						case IDM_TAB_CLOSE_LEFTSIDE:	TabCloseLeftSide(index); break;
						case IDM_TAB_CLOSE_RIGHTSIDE:	TabCloseRightSide(index); break;
						case IDM_TAB_CLOSE:				TabClose(index); break;
						case IDM_TAB_EXPORT:			TabExport(index); break;
						case IDM_TAB_LOCK:				ToggleTabLock(index); break;
						default:						PostMessage(WM_COMMAND, cmd, NULL); break;
					}
				}
				return;
			}
		}

		BarSettings(point);
	}
}

void CMainFrame::OnExecMsdos()
{
	bool bAsAdmin = _ISKEYDOWN(VK_CONTROL);

	TCHAR dir[MAX_PATH * 2];
	if (!GetFolderTree().GetCurrentPath(dir, ARRAYSIZE(dir)) || PathIsUNC(dir))
		PathBuildRoot(dir, PathGetDriveNumber(_GCDATA.m_strWinDir));

	CString str;
	if (!GetAddin(addinMsdos, str)) {
		TCHAR cmd[MAX_PATH * 2];
		if (GetEnvironmentVariable(_T("COMSPEC"), cmd, ARRAYSIZE(cmd)) > 1) {
			str = cmd;
/*
			STARTUPINFO si;
			memset(&si, 0, sizeof(si));
			si.cb = sizeof(si);
			si.lpTitle = PathFindFileName(cmd);
			PROCESS_INFORMATION pi;
			if (CreateProcess(cmd, NULL, NULL, NULL, FALSE,
						CREATE_DEFAULT_ERROR_MODE | CREATE_NEW_CONSOLE |
						CREATE_NEW_PROCESS_GROUP | NORMAL_PRIORITY_CLASS,
						NULL, dir, &si, &pi))
			{
				CloseHandle(pi.hThread);
				CloseHandle(pi.hProcess);
			}
*/
		}
	}
	if (!str.IsEmpty()) {
		if (str[0] == _T('@')) {
			bAsAdmin = true;
			str.Delete(0);
		}
		ExecuteAsAdmin(m_hWnd, str, dir, SW_SHOWNORMAL, bAsAdmin);
	}
}

void CMainFrame::OnExecShell()
{
  const int buflen = MAX_PATH * 2;
  bool bAsAdmin = _ISKEYDOWN(VK_CONTROL);
  TCHAR dir[buflen];
  if (!GetFolderTree().GetCurrentPath(dir, ARRAYSIZE(dir)) || PathIsUNC(dir)) {
    PathBuildRoot(dir, PathGetDriveNumber(_GCDATA.m_strWinDir));
  } else if (PathIsRoot(dir) && dir[1] == _T(':')) {
	// PowerShell のSet-Location で\付きのドライブルートを指定するとエラー
    dir[2] = 0;
  }

  // 1st:pwsh.exe, 2nd:windows terminal, 3rd:pwershell
  TCHAR app[buflen];
  CString cmd;
  if (SearchPath(NULL, _T("pwsh"), _T(".exe"), buflen, app, NULL) > 0) {
    // PowerShell ver7 Core
	cmd.Format(_T("\"%s\" -WorkingDirectory \"%s\""), app, dir);
  } else if (SearchPath(NULL, _T("wt"), _T(".exe"), buflen, app, NULL) > 0) {
	cmd.Format(_T("\"%s\" -d \"%s\""), app, dir);
  } else if (SearchPath(NULL, _T("powershell"), _T(".exe"), buflen, app, NULL)) {
	cmd.Format(_T("\"%s\" -NoExit -WorkingDirectory \"%s\""), app, dir);
  }
  else {
    return;
  }
  ExecuteAsAdmin(m_hWnd, cmd, dir, SW_SHOWNORMAL, bAsAdmin);
}

void CMainFrame::OnExecOwn()
{
	bool bAsAdmin = _ISKEYDOWN(VK_CONTROL);

	CIDListPtr idl;
	if (idl.Clone(GetCurrentFileBrowser().GetCurrentFolder()))
		ExecNewFrame(idl, _T("/force"), bAsAdmin);
}

void CMainFrame::OnWinIniChange(LPCTSTR lpszSection)
{
	if (!lpszSection || !StrCmpI(lpszSection, _T("Extensions")))
		UpdateFileView(NULL);

	// ウィンドウメトリクスの再取得
	_GCDATA.UpdateWindowsSettings();
}

void CMainFrame::OnWindowMaxmize()
{
	WPARAM wp = !IsZoomed() ? SC_MAXIMIZE : SC_RESTORE;
	SendMessage(WM_SYSCOMMAND, wp, 0L);
}

void CMainFrame::OnWindowMinimize()
{
	WPARAM wp = !IsIconic() ? SC_MINIMIZE : SC_RESTORE;
	SendMessage(WM_SYSCOMMAND, wp, 0L);
}

LRESULT CMainFrame::OnSaveAppProp(WPARAM wParam, LPARAM lParam)
{
	CString str;
	GetAppDataFolder(str, _T("combination\\frame"));
	str.AppendFormat(_T("_%d.tabgroup"), wParam);
	return (LRESULT)TabGroupSave(str);
}

void CMainFrame::OnWindowCombineSave()
{
	CSimpleArray<HWND> array;
	int cnt = GetSiblings(NULL, array, true);
	SetRegValue(IDS_SEC_COMBINE, IDS_KEY_ENTRY, cnt);

	CString str;
	CRect rc;
	int index = 0;
	for (int i = 0; i < cnt; i++) {
		if (::SendMessage(array[i], nRM_SAVEAPPPROP, (WPARAM)index, 0)) {
			::GetWindowRect(array[i], &rc);
			str.Format(_T("%d,%d,%d,%d"), rc.left, rc.top, rc.Width(), rc.Height());
			SetRegTextSeq(IDS_SEC_COMBINE, IDS_KEY_FRAMEPOS, str.GetString(), index);
			index++;
		}
	}
}

void CMainFrame::OnWindowCombineRestore()
{
	CSimpleArray<HWND> array;
	int sib = GetSiblings(NULL, array, true);
	int cnt = GetRegValue(IDS_SEC_COMBINE, IDS_KEY_ENTRY, 0);
	if (cnt == 0)
		return;

	int i;
	for (i = 0; i < cnt; i++) {
		CString str;
		GetAppDataFolder(str, _T("combination\\frame"));
		str.AppendFormat(_T("_%d.tabgroup"), i);
		if (i >= sib)
			ExecNewFrame(str.GetString(), _T("/force"));
		else {
			COPYDATASTRUCT cds;
			cds.dwData = CPD_EXEC_FILEW;
			cds.cbData = sizeof(TCHAR) * (str.GetLength() + 1);
			cds.lpData = (void*)str.GetString();
			::SendMessage(array[i], WM_COPYDATA, 0, (LPARAM)&cds);
		}
	}
	m_timerid_combine = SetTimer(TIMERID_COMBINE, TIM_COMBINEWAIT, NULL);
}

void CMainFrame::OnWindowCascade()
{
	CascadeWindows();
}

void CMainFrame::CascadeWindows()
{
	TileWindows(0);
}

void CMainFrame::OnWindowTileHorz()
{
	TileWindows(1);
}

void CMainFrame::OnWindowTileVert()
{
	TileWindows(2);
}

void CMainFrame::TileWindows(WORD wMode)
{
	CSimpleArray<HWND> array;
	if (GetSiblings(m_hWnd, array, true) < 1) {
		// もうひとつWindowを開いて並べるか問い合わせる
		if (AppMessage(IDS_TILEWINDOW_WITH_NEWFRAME, NULL, MB_QYESNO) != IDYES)
			return;

		int len;
		BYTE* pd = GetRegBinarySeqEx(IDS_SEC_FOLDER, IDS_KEY_LASTACCESS, 1, &len);
		if (pd) {
			if (!idl_IDListIsValid((LPITEMIDLIST)pd)) {
				free(pd);
				pd = NULL;
			}
		}
		ExecNewFrame((LPITEMIDLIST)pd, _T("/force"));
		if (pd)
			free(pd);

		DWORD limit = GetTickCount() + 4000;
		MSG msg;
		while (1) {
			if (PeekMessage(&msg, NULL, NULL, NULL, PM_REMOVE)) {
				TranslateMessage(&msg);
				DispatchMessage(&msg);
			}

			if (GetTickCount() > limit)
				return;

			array.RemoveAll();
			if (GetSiblings(m_hWnd, array, true) > 0) {
				if (::SendMessage(array[0], m_wm_frameisidle, 0, 0))
					break;
			}
		}
	}

	// Windows XPになって、現在のアクティブウィンドウから呼び出すと
	// そのアクティブウィンドウが無視されるようになってしまったので
	// RunDLLで他のプロセスからコールさせる形式とする
	CString str; str.Format(_T("%d %s"), wMode, APP_FRAMECLASS);
	lccRunDll(m_hWnd, _T("lscore.dll"), _T("TileCascadeWindows"), str);
}

/* windows10以降では diskcopy.dll が無い
void CMainFrame::OnDiskCopy()
{
	TCHAR dir[MAX_PATH];
	GetWindowsDirectory(dir, ARRAYSIZE(dir));
	// DiskCopyRunDll に引数 0 を加えないとVISTAではコマンドが実行できない。 0=A: 1=B: ...
	ShellExecute(m_hWnd, NULL, gszRunDLL, _T("diskcopy.dll,DiskCopyRunDll 0"), dir, SW_SHOWNORMAL);
}
*/
/*
void CMainFrame::OnDiskFormat()
{
	fopFormatDrive(m_hWnd);
}
*/
/*
void CMainFrame::OnDiskLabel()
{
	// ドライブのプロパティを表示させる
	LPCITEMIDLIST pidl = m_drive.GetCurrentDrive();
	if (pidl) InvokeIDList(m_hWnd, pidl, gszVerbProp);
	return;
}
*/
void CMainFrame::OnDiskSelect()
{
	CIDListPtr idlCur;
	idlCur.Clone(GetCurrentFileBrowser().GetCurrentFolder());

	int openMode = 0;
	CIDListPtr idl = LogDisk(m_hWnd, idlCur, openMode);
	if (!idl.isNull()) {
		if (openMode == 2) {
			// new window
			ExecNewFrame(idl, _T("/force"), false);
		} else {
			BOOL bLocked = m_tabbar.isItemLocked(m_tabbar.GetCurSel());
			if (bLocked || openMode == 1)
				TabNew(idl);
			else
				OpenFolder(idl);
		}
	}
}

void CMainFrame::OnDirAddFavorite()
{
	CIDListPtr idlCur;
	if (idlCur.Clone(GetCurrentFileBrowser().GetCurrentFolder())) {
		CString str;
		idl_GetName(idlCur, SHGDN_NORMAL, str);
		if (AppMessage(IDS_QUERY_ADDFAVOITEM, str, MB_QYESNO) == IDYES)
			m_viewFavo.InsertFavoriteItem(idlCur);
	}
}
void CMainFrame::OnMakeDir()
{
	CIDListPtr idl;
	idl.Clone(GetCurrentFileBrowser().GetCurrentFolder());
	TCHAR dir[MAX_PATH * 2];
	BOOL ok = idl.GetPath(dir);
	if (!ok) {
		// 物理パスが直接取得できない場合
		CString str;
		if (idl_GetName(idl, SHGDN_FORPARSING, str)) {
			// ライブラリ配下ならフォルダ作成が可能なので
			CComPtr<IShellLibrary> psl;
			if (SHLoadLibraryFromParsingName(str, STGM_READWRITE, IID_PPV_ARGS(&psl)) == S_OK) {
				CComPtr<IShellItem> psi;
				if (psl->GetDefaultSaveFolder(DSFT_DETECT, IID_PPV_ARGS(&psi)) == S_OK) {
					idl.GetFromShellItem(psi);
					ok = idl.GetPath(dir);
				}
			}
		}
	}

	if (ok &&
		fopCreateNewFolder(m_hWnd, dir) &&
		(gnTabStat & TAB_MKDIR) &&
		idl.CreateFromPath(dir)) {
		TabNew(idl);
	}
}

void CMainFrame::OnNextWindow()
{
	RepositionFrames(TRUE);
}

void CMainFrame::OnPrevWindow()
{
	RepositionFrames(FALSE);
}

void CMainFrame::OnFolderTreeNotifySelChanged(NMHDR* pNmhdr, LRESULT* pResult)
{
	// フォルダツリーで選択が変更されたので、ツリーから現在のフォルダIDLISTを取得
	CIDListPtr idl;
	if (idl.Clone(GetFolderTree().GetCurrentIDList())) {
		if (m_tabbar.isItemLocked(m_tabbar.GetCurSel()))
			TabNew(idl);
		else
			OpenFolder(idl, NULL, false);
	}
}

void CMainFrame::OnFolderTreeNotifyEnterFileOperation(NMHDR* pNmhdr, LRESULT* pResult)
{
	// ディレクトリパースを中止する
	CFileBrowseWindow browser = GetCurrentFileBrowser();
	if (browser.IsWindow())
		browser.StopDirectoryParsing();
}

void CMainFrame::OnTabbarNotifyGetInfotip(NMHDR* pNmhdr, LRESULT* pResult)
{
	*pResult = 0;
	if (GetCapture()) return;

	NMTCGETINFOTIP* ptt = (NMTCGETINFOTIP*)pNmhdr;
	CDocument* pDoc = (CDocument*)ptt->lParam;
	if (pDoc) {
		CFileBrowseWindow browser = pDoc->GetFileBrowser();
		if (browser.IsWindow()) {
			CHistory* pHistory = pDoc->GetHistory();
			CCmItem* pci = pHistory->GetCurrentItem();
			CString str = pci->m_name.GetString();
			TCHAR buf[MAX_PATH];
			browser.GetTotalInformation(buf, ARRAYSIZE(buf));
			str.AppendFormat(_T("\n%s"), buf);
			lstrcpyn(ptt->pszText, str, ptt->cchTextMax);
		}
	}
}

void CMainFrame::OnTabbarNotifySelChange(NMHDR* pNmhdr, LRESULT* pResult)
{
	*pResult = 0;
	NMTCSELCHANGE* psc = (NMTCSELCHANGE*)pNmhdr;
	SwitchDocument(psc->iItemOld, psc->iItemNew);
}

void CMainFrame::SwitchDocument(int indexOld, int indexNew)
{
	if (indexOld != indexNew) {

		m_bIgnoreFolderBrowsing = true;				// ドキュメントを切り替えるだけなので、ブラウズ更新は無用

		CDocument* pDocOld = (CDocument*)m_tabbar.GetItemData(indexOld);
		ATLASSERT(pDocOld != NULL);
		CDocument* pDocNew = (CDocument*)m_tabbar.GetItemData(indexNew);
		ATLASSERT(pDocNew != NULL);
		if (pDocNew) {
			m_pCurrentDocument = pDocNew;
			if (pDocNew != pDocOld) {
				// ドキュメントに紐付いているフォルダを取得
				CIDListPtr idl;
				idl.Clone(pDocNew->GetDocumentFolder());
				// （もし必要なら）ルートフォルダを指定してフォルダ変更
				OpenFolder(idl, (gnSysStat & STS_DRIVEROOT) ? pDocNew->GetRootFolder() : NULL);
				// ブラウザを切り替え
				CFileBrowseWindow oldBrowser = pDocOld->GetFileBrowser();
				CFileBrowseWindow newBrowser = pDocNew->GetFileBrowser();
				m_splitView.SetSplitterPane(SPLIT_PANE_RIGHT, newBrowser, true);
				m_ViewSwitch.ReplaceView(oldBrowser, newBrowser);
				m_ViewSwitch.SetActiveView(newBrowser);
				// アクセス時刻の記録
				pDocNew->SetTimeReference();
			}
		}
		m_bIgnoreFolderBrowsing = false;
	}
}

int CMainFrame::GetDocumentIndex(CDocument* pdoc)
{
	int numDocument = m_tabbar.GetCount();
	for (int i = 0; i < numDocument; i++) {
		if (pdoc == (CDocument*)m_tabbar.GetItemData(i))
			return i;
	}
	return -1;
}

void CMainFrame::SelectTab(int indexNew)
{
	int indexOld = m_tabbar.SetCurSel(indexNew);
	if (indexOld != indexNew)
		SwitchDocument(indexOld, indexNew);
}

void CMainFrame::OnTabbarNotifyBeginDrag(NMHDR* pNmhdr, LRESULT* pResult)
{
	*pResult = TRUE;
}

void CMainFrame::OnTabbarNotifyCloseButtonClicked(NMHDR* pNmhdr, LRESULT* pResult)
{
	NMTAB* ptab = (NMTAB*)pNmhdr;
	TabClose(ptab->iItem);
}

void CMainFrame::OnTabbarNotifyAddButtonClicked(NMHDR* pNmhdr, LRESULT* pResult)
{
	OnTabNew();
}

void CMainFrame::OnTabbarNotifyDeleteItem(NMHDR* pNmhdr, LRESULT* pResult)
{
	NMTAB* ptab = (NMTAB*)pNmhdr;
	CDocument* pDoc = (CDocument*)ptab->lParam;
	if (pDoc)
		delete pDoc;
}

void CMainFrame::OnTabbarNotifyDblClk(NMHDR* pNmhdr, LRESULT* pResult)
{
	if (gnTabStat & TAB_DBLCLK_LOCK) {
		NMTAB* ptab = (NMTAB*)pNmhdr;
		ToggleTabLock(ptab->iItem);
	}
}

bool CMainFrame::TabNew(LPITEMIDLIST pidl, bool bActivate, bool bAppend, bool bForce, ICONLISTSTYLE* pStyle)
{
	if (!pidl) return false;

	CIDListPtrHandle idl = pidl;
	CFileBrowseWindow browser;
	if ((gnTabStat & TAB_NODUPLICATE) && !bForce) {
		// 既に開いているタブに一致するフォルダがあれば、それをアクティブにする
		int numDocument = m_tabbar.GetCount();
		for (int i = 0; i < numDocument; i++) {
			CIDListPtrHandle idlTab = GetTabFolder(i, browser);
			if (idlTab == idl) {
				SelectTab(i);
				return TRUE;
			}
		}
	}
	CString str;
	if ((gnTabStat & TAB_FULLPATH) && !idl.isDesktop() && idl.GetPath(str))
		;
	else
		idl_GetName(idl, SHGDN_INFOLDER, str);
	int iImage = idl_GetSysIconIndex(idl);

	// 新しいタブを開く
	CDocument* pDoc = new CDocument;
	browser = pDoc->CreateBrowser(m_splitView, WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS | WS_CLIPCHILDREN, 0, IDW_FILEBROWSER);

	// 履歴に追加
	CHistory* pHistory = pDoc->GetHistory();
	pHistory->PushItem(pidl);

	int insertAfter = bAppend ? m_tabbar.GetCount() : m_tabbar.GetCurSel() + 1;
	int index = m_tabbar.InsertItem(insertAfter, str, iImage, (LPARAM)pDoc);

	if (pStyle == NULL)
		pStyle = &gDefaultIconListStyle;
	browser.SetListStyle(pStyle);
	m_mapListStyle.SetItemStyle(idl, *pStyle);
	browser.Browse(idl);

	if (bActivate)
		SelectTab(index);

	if (m_tabbar.GetCount() > 1 && !m_tabbar.IsWindowVisible()) {
		// タブの数が複数であればタブを可視化
		m_tabbar.ShowWindow(SW_SHOWNOACTIVATE);
		UpdateLayout();
	}
	return true;
}

LPCITEMIDLIST CMainFrame::GetTabFolder(int index, CFileBrowseWindow& browser)
{
	CDocument* pDoc = (CDocument*)m_tabbar.GetItemData(index);
	ATLASSERT(pDoc != NULL);
	if (pDoc) {
		browser = pDoc->GetFileBrowser();
		return browser.GetCurrentFolder();
	}
	return NULL;
}

void CMainFrame::TabClose(int index)
{
	int numDocument = m_tabbar.GetCount();
	if (numDocument == 1)
		OnAppExit();
	else if (numDocument > 1) {
		// 閉じるタブの現在のIDListを取得して閉じたタブリストを更新
		int i;
		CFileBrowseWindow browser;
		LPCITEMIDLIST pidlFolder = GetTabFolder(index, browser);
		if (pidlFolder) {
			CIconListStyle ls;
			browser.GetListStyle(&ls);

			CCmItem* pci = new CCmItem(pidlFolder);
			// 既に登録されていればそれを削除
			int cnt = (int)m_arrTabReopen.GetItemCount();
			for (i = 0; i < cnt; i++) {
				CCmItem* pciTemp = m_arrTabReopen.GetItemAt(i);
				if (pciTemp && pciTemp == pci) {
					delete pciTemp;
					m_arrTabReopen.RemoveAt(i);
					break;
				}
			}
			// 閉じた時刻を記録
			pci->SetTime();
			m_arrTabReopen.Push(pci, ls);
			// XML-DB更新
			SaveTabReopenArray(m_arrTabReopen, 6);
			CreateJumpList();
		}

		int indexCur = m_tabbar.GetCurSel();
		if (indexCur == index) {
			// 次のドキュメントを選択してから削除
			int indexActivate = (index < numDocument - 1) ? index + 1 : index - 1;
			// 閉じるタブ以外の最後にアクティブだったタブを選択
			if (gnTabStat & TAB_SELECT_LASTACTIVE) {
				DWORD ref = 0;
				for (i = 0; i < numDocument; i++) {
					if (i != indexCur) {
						CDocument* pdoc = (CDocument*)m_tabbar.GetItemData(i);
						if (pdoc->GetTimeReference() > ref) {
							ref = pdoc->GetTimeReference();
							indexActivate = i;
						}
					}
				}
			}
			SelectTab(indexActivate);
		}

		m_ViewSwitch.RemoveView(browser);
		m_tabbar.DeleteItem(index);

		if (--numDocument == 1) {
			if (!(gnTabStat & TAB_BARSHOWANY)) {
				// タブが複数でなくなればタブバーを非表示に
				m_tabbar.ShowWindow(SW_HIDE);
				UpdateLayout();
			}
		}
	}
}

void CMainFrame::TabValidate(LPCITEMIDLIST pidlSource)
{
	CSimpleArray<int> array;
	int sel = m_tabbar.GetCurSel();
	int i, cnt = m_tabbar.GetCount();
	for (i = 0; i < cnt; i++) {
		if (i != sel) {
			CDocument* pDoc = (CDocument*)m_tabbar.GetItemData(i);
			ATLASSERT(pDoc != NULL);
			LPCITEMIDLIST pidlTarget = pDoc->GetDocumentFolder();
			if (pidlTarget) {
				if (ILIsEqual(pidlSource, pidlTarget))
					array.Add(i);
				else if (ILIsParent(pidlSource, pidlTarget, FALSE))
					array.Add(i);
			}
		}
	}

	// 最後のタブから削除していく
	cnt = array.GetSize();
	if (cnt > 0) {
		for (i = cnt - 1; i >= 0; i--) {
			TabClose(array[i]);
		}
	}
}

void CMainFrame::OnTabNew()
{
	CIDListPtr idl = GetRegPidl(IDS_SEC_FOLDER, IDS_KEY_HOME);
	if (idl.isNull()) {
		if (gnSysStat & STS_DRIVEROOT) {
			TCHAR buf[MAX_PATH];
			PathBuildRoot(buf, PathGetDriveNumber(_GCDATA.m_strWinDir));
			idl.CreateFromPath(buf);
		}
		else
			idl.GetMyDocument();
	}
	TabNew(idl, true, true, true);			// リスト最後に強制追加してアクティブ化
}

void CMainFrame::OnTabPrev()
{
	int numDocument = m_tabbar.GetCount();
	if (numDocument > 1) {
		int index = m_tabbar.GetCurSel();
		if (index > 0)
			index--;
		else
			index = numDocument - 1;
		SelectTab(index);
	}
}

void CMainFrame::OnTabNext()
{
	int numDocument = m_tabbar.GetCount();
	if (numDocument > 1) {
		int index = m_tabbar.GetCurSel();
		if (index < numDocument - 1)
			index++;
		else
			index = 0;
		SelectTab(index);
	}
}

void CMainFrame::OnTabClose()
{
	TabClose(m_tabbar.GetCurSel());
}

void CMainFrame::OnTabCloseAnyOther()
{
	TabCloseAnyOther(m_tabbar.GetCurSel());
}

void CMainFrame::OnTabCloseLeftSide()
{
	TabCloseLeftSide(m_tabbar.GetCurSel());
}

void CMainFrame::OnTabCloseRightSide()
{
	TabCloseRightSide(m_tabbar.GetCurSel());
}

void CMainFrame::TabCloseAnyOther(int index)
{
	int numTabs = m_tabbar.GetCount();
	if (numTabs > 2) {
		if (!(gnTabStat & TAB_CLOSENOCONFIRM)) {
			CTabCloseAllDlg dlg;
			if (dlg.DoModal() != IDOK)
				return;
			if (!dlg.m_bConfirmAnyTime) {
				gnTabStat |= TAB_CLOSENOCONFIRM;
				fvApp.SaveStates();
			}
		}
	}

	if (index != m_tabbar.GetCurSel())
		SelectTab(index);
	// index以外のタブを後ろから閉じる
	for (int i = numTabs - 1; i >= 0; i--) {
		if (i != index && !m_tabbar.isItemLocked(i))
			TabClose(i);
	}
}

void CMainFrame::TabCloseLeftSide(int index)
{
	int numTabs = m_tabbar.GetCount();
	if (numTabs > 1 && index > 0) {
		if (index > 1 && !(gnTabStat & TAB_CLOSENOCONFIRM)) {
			CTabCloseAllDlg dlg;
			if (dlg.DoModal() != IDOK)
				return;
			if (!dlg.m_bConfirmAnyTime) {
				gnTabStat |= TAB_CLOSENOCONFIRM;
				fvApp.SaveStates();
			}
		}

		if (index != m_tabbar.GetCurSel())
			SelectTab(index);
		// index以外のタブを後ろから閉じる
		for (int i = index - 1; i >= 0; i--) {
			if (!m_tabbar.isItemLocked(i))
				TabClose(i);
		}
	}
}

void CMainFrame::TabCloseRightSide(int index)
{
	int numTabs = m_tabbar.GetCount();
	if (numTabs > 1 && index < numTabs - 1) {
		if (index < numTabs - 2 && !(gnTabStat & TAB_CLOSENOCONFIRM)) {
			CTabCloseAllDlg dlg;
			if (dlg.DoModal() != IDOK)
				return;
			if (!dlg.m_bConfirmAnyTime) {
				gnTabStat |= TAB_CLOSENOCONFIRM;
				fvApp.SaveStates();
			}
		}

		if (index != m_tabbar.GetCurSel())
			SelectTab(index);
		// index以外のタブを後ろから閉じる
		for (int i = numTabs - 1; i > index; i--) {
			if (!m_tabbar.isItemLocked(i))
				TabClose(i);
		}
	}
}

void CMainFrame::TabRefresh(int index)
{
	int numDocument = m_tabbar.GetCount();
	if (numDocument > 0 && 0 <= index && index < numDocument) {
		CDocument* pDoc = (CDocument*)m_tabbar.GetItemData(index);
		ATLASSERT(pDoc != NULL);
		pDoc->GetFileBrowser().Browse(NULL);
	}
}

void CMainFrame::OnTabExport()
{
	if (m_tabbar.GetCount() > 1)
		TabExport(m_tabbar.GetCurSel());
}

void CMainFrame::TabExport(int index)
{
	bool bAsAdmin = _ISKEYDOWN(VK_CONTROL);
	// タブを新しいウィンドウで開く。元のタブは閉じる
	int numDocument = m_tabbar.GetCount();
	// タブが複数無い場合には何もしない
	if (numDocument < 2 || index < 0 || index >= numDocument) return;

	CDocument* pDoc = (CDocument*)m_tabbar.GetItemData(index);
	ATLASSERT(pDoc != NULL);
	LPCITEMIDLIST pidl = pDoc->GetDocumentFolder();
	if (pidl) {
		ExecNewFrame(pidl, _T("/force"), bAsAdmin);
		TabClose(index);
	}
}

void CMainFrame::OnTabLock()
{
	ToggleTabLock(m_tabbar.GetCurSel());
}

void CMainFrame::ToggleTabLock(int index)
{
	BOOL bLocked = m_tabbar.isItemLocked(index);
	m_tabbar.LockItem(index, !bLocked);
}

class CDocCompare
{
public:
	CMainFrame* m_pFrame;
	int m_sortMode;
	CDocCompare(CMainFrame* pFrame, int sortMode) : m_pFrame(pFrame), m_sortMode(sortMode) { }
};

int __cdecl CMainFrame::__doc_sort(const void* arg1, const void* arg2, const void* param)
{
	CDocument* p1 = *(CDocument**)arg1;
	CDocument* p2 = *(CDocument**)arg2;
	CDocCompare* pdc = (CDocCompare*)param;

	DWORD v1 = (pdc->m_sortMode == 0) ? p1->m_serial : p1->m_lastTimeReference;
	DWORD v2 = (pdc->m_sortMode == 0) ? p2->m_serial : p2->m_lastTimeReference;
	return ((v1 == v2) ? 0 : ((v1 > v2) ? 1 : -1));
}

void CMainFrame::OnTabSortRange(UINT id, UINT code, HWND hwnd)
{
	int numDocument = m_tabbar.GetCount();
	if (numDocument > 1) {
		CSimpleArray<CDocument*> arr;
		for (int i = 0; i < numDocument; i++)
			arr.Add((CDocument*)m_tabbar.GetItemData(i));
		CDocCompare dc(this, (id == IDM_TAB_SORTDEFAULT) ? 0 : 1);
		qsort_ex(arr.GetData(), numDocument, sizeof(CDocument*), __doc_sort, &dc);
		for (int i = 0; i < numDocument; i++) {
			int index = GetDocumentIndex(arr[i]);
			if (i != index)
				m_tabbar.ExchangeItem(i, index);
		}
	}
}

bool CMainFrame::TabGroupSave(LPCTSTR pszTabGroupName)
{
	char ssz[MAX_PATH * 2];

	// Save XML
	TiXmlDocument xmlDoc;
	xmlDoc.InsertEndChild( TiXmlDeclaration( "1.0", "UTF-8", "yes" ) );

	// --------------------------
	// XML directory header
	// --------------------------
	TiXmlNode* pDoc = xmlDoc.InsertEndChild( TiXmlElement( "tabgroup" ) );
	TiXmlElement* pRoot = pDoc->ToElement();
	// アクティブインデクス書き込み
	pRoot->SetAttribute( "active", (int)m_tabbar.GetCurSel());
	// XML item
	CString str;
	CIconListStyle ils;
	CCmItem* pci;
	int numDocument = m_tabbar.GetCount();
	for (int i = 0; i < numDocument; i++) {
		CFileBrowseWindow browser;
		LPCITEMIDLIST pidlTarget = GetTabFolder(i, browser);
		if (pidlTarget) {
			m_tabbar.GetItemText(i, str);		// 表示名
			browser.GetListStyle(&ils);			// iconstyle
			pci = new CCmItem(pidlTarget, str);
			// 書き出し
			TiXmlNode* pItem = pRoot->InsertEndChild( TiXmlElement( "item" ) );
			xmlwrite_tabitem(pItem->ToElement(), pci, &ils, m_tabbar.isItemLocked(i));
			delete pci;
		}
	}
	// Save XML
	WideCharToMultiByte(CP_ACP, 0, pszTabGroupName, -1, ssz, ARRAYSIZE(ssz), NULL, NULL);
	// 書き出し可能か？
	if (!CheckFileWritable(ssz)) {
		;	// とりあえずreadonly属性がついていれば解除される
	}
	return xmlDoc.SaveFile(ssz);
}

int CMainFrame::TabGroupLoad(LPCTSTR pszTabGroupName)
{
	char xml[MAX_PATH * 2];
	WideCharToMultiByte(CP_ACP, 0, pszTabGroupName, -1, xml, ARRAYSIZE(xml), NULL, NULL);

	// 現在のタブ数
	int numDocument = m_tabbar.GetCount();

	TiXmlDocument xmlDoc;
	if (!xmlDoc.LoadFile( xml )) return 0;

	int add = 0;
	int sel = 0;
	TiXmlNode* pDoc = xmlDoc.ToDocument();
	if (pDoc) {
		TiXmlElement* pRoot = pDoc->FirstChildElement("tabgroup");
		if (pRoot) {
			const char* p = pRoot->Attribute("active");
			sel = p ? atoi(p) : 0;

			TiXmlElement* pItem = pRoot->FirstChildElement("item");
			while (pItem) {
				CIconListStyle ils;
				BOOL bLocked;
				CCmItem* pci = xmlload_tabitem(pItem, &ils, &bLocked);
				if (pci) {
					CComPtr<IShellFolder> psf = pci->m_idl.GetShellFolder();
					if (psf.p != NULL) {
						CComPtr<IEnumIDList> pe;
						HRESULT hr = psf->EnumObjects(NULL, SHCONTF_FOLDERS | SHCONTF_FASTITEMS, &pe);
						if (hr == S_OK) {
							TabNew(pci->m_idl, false, true, true, &ils);
							add++;
							// ロック状態を復帰
							if (bLocked)
								m_tabbar.LockItem(m_tabbar.GetCount() - 1, TRUE);
						}
						else {
							TCHAR name[MAX_PATH * 2];
							SHGetPathFromIDList(pci->m_idl, name);
							lccErrorBox(m_hWnd, hr, APP_PRODUCT, name, MB_STOP);
						}
					}
					delete pci;
				}
				pItem = pItem->NextSiblingElement("item");
			}
		}
	}

	// 既存のタブがあれば、それらは閉じる
	if (add > 0) {
		sel = numDocument + min(add - 1, max(0, sel));
		// タブをアクティブ化
		SelectTab(sel);

		// 以前の不要なタブを先頭から閉じる
		while (numDocument > 0) {
			TabClose(0);
			--numDocument;
		}
	}
	return add;
}

void CMainFrame::OnTabGroupSave()
{
	CString dir;
	if (GetAppDataFolder(dir)) {
		CString str;
		if (FileBrowse(FALSE, m_hWnd, IDS_FILTER_TABGROUP, str, dir)) {
			if (StrCmpI(PathFindExtension(str), gszTabGroup))
				str += gszTabGroup;
			if (_pathfileexists(str)) {
				if (AppMessage(IDS_ALREADY_FILE_EXISTS, PathFindFileName(str), MB_QYESNO) != IDYES)
					return;
			}
			if (TabGroupSave(str) && (gnTabStat & TAB_SAVEGROUPTOBOOKMARK)) {
				CIDListPtr idl(str.GetString());
				m_viewFavo.InsertFavoriteItem(idl);
			}
			CreateJumpList();
		}
	}
}

void CMainFrame::OnTabGroupLoad()
{
	CString dir;
	if (GetAppDataFolder(dir)) {
		CString str;
		if (FileBrowse(TRUE, m_hWnd, IDS_FILTER_TABGROUP, str, dir)) {
			if (_pathfileexists(str))
				TabGroupLoad(str);
		}
	}
}

LRESULT CMainFrame::OnGetTabsFolderIDList(WPARAM wParam, LPARAM lParam)
{
	int index = (int)wParam;
	int numTabs = m_tabbar.GetCount();
	if (0 <= index && index < numTabs) {
		CDocument* pDoc = (CDocument*)m_tabbar.GetItemData(index);
		ATLASSERT(pDoc != NULL);
		LPCITEMIDLIST pidlTarget = pDoc->GetDocumentFolder();
		if (pidlTarget)
			return (LRESULT)ILClone(pidlTarget);
	}
	return 0;
}

void CMainFrame::OnFavoviewNotifyDblClk(NMHDR* pNmhdr, LRESULT* pResult)
{
	if (m_splitTree.IsPaneMinimized(SPLIT_PANE_BOTTOM))
		m_splitTree.RestoreSplitterPos();
	else {
		m_splitTree.MinimizePane(SPLIT_PANE_BOTTOM);
		if (m_viewFavo == m_ViewSwitch.GetCurrentView())
			m_ViewSwitch.ActivateNextView(FALSE);
	}
}

void CMainFrame::RepositionFrames(BOOL bNext)
{
	TCHAR buf[MAX_PATH];
	CSimpleArray<HWND> arr;
	arr.Add(m_hWnd);
	HWND hwnd = ::GetTopWindow(::GetDesktopWindow());
	while ((hwnd = ::GetNextWindow(hwnd, GW_HWNDNEXT)) != NULL) {
		if (hwnd != m_hWnd &&
			::GetClassName(hwnd, buf, 128) &&
			!StrCmp(APP_FRAMECLASS, buf) &&
			!::IsIconic(hwnd))
			arr.Add(hwnd);
	}

	int cnt = arr.GetSize();
	if (cnt > 1) {
		HDWP hdwp = ::BeginDeferWindowPos(2);
		if (bNext) {
			hwnd = arr[1];
			hdwp = ::DeferWindowPos(hdwp, hwnd, HWND_TOP, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);
			hdwp = ::DeferWindowPos(hdwp, m_hWnd, arr[cnt - 1], 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);
		}
		else {
			hwnd = arr[cnt - 1];
			hdwp = ::DeferWindowPos(hdwp, hwnd, HWND_TOP, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);
			hdwp = ::DeferWindowPos(hdwp, m_hWnd, hwnd, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);
		}
		::EndDeferWindowPos(hdwp);
		::BringWindowToTop(hwnd);
	}
}

HWND CMainFrame::SearchSiblings(BOOL bBound)
{
	TCHAR buf[128];
	UINT nFlag = bBound ? GW_HWNDNEXT : GW_HWNDPREV;
	HWND hwnd = m_hWnd;
	while (hwnd) {
		hwnd = ::GetNextWindow(hwnd, nFlag);
		if (::GetClassName(hwnd, buf, ARRAYSIZE(buf)) && !StrCmp(APP_FRAMECLASS, buf))
			return hwnd;
	}

	nFlag = bBound ? GW_HWNDPREV : GW_HWNDNEXT;
	hwnd = m_hWnd;
	while (hwnd) {
		hwnd = ::GetNextWindow(hwnd, nFlag);
		if (::GetClassName(hwnd, buf, ARRAYSIZE(buf)) && !StrCmp(APP_FRAMECLASS, buf))
			return hwnd;
	}

	return NULL;
}

/* @@@
int CMainFrame::ListupOther(CSimpleArray<HWND>& array)
{
	HWND hDesktop = ::GetDesktopWindow();
	HWND hwnd = ::GetWindow(hDesktop, GW_CHILD);

	TCHAR buf[MAX_PATH];
	DWORD style;
	while (hwnd) {
		::GetClassName(hwnd, buf, ARRAYSIZE(buf));
		style = ::GetWindowLong(hwnd, GWL_STYLE);
		if (StrCmp(APP_FRAMECLASS, buf) &&
			style & WS_VISIBLE &&
			style & WS_DLGFRAME &&
			!(style & WS_MINIMIZE))
		{
			array.Add(hwnd);
		}
		hwnd = ::GetNextWindow(hwnd, GW_HWNDNEXT);
	}
    return (int)array.GetSize();
}
@@@ */

void CMainFrame::ShowApp(CSimpleArray<HWND>& array, UINT nCmdShow)
{
	int cnt = array.GetSize();
	if (cnt > 0) {
		UINT flags = SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER;
		if (nCmdShow == SW_SHOWNA)
			flags |= SWP_SHOWWINDOW | SWP_NOACTIVATE;
		else
			flags |= SWP_HIDEWINDOW;

		HDWP hdwp = ::BeginDeferWindowPos(cnt);
		for (int i = 0; i < cnt; i++)
			hdwp = ::DeferWindowPos(hdwp, array[i], NULL, 0, 0, 0, 0, flags);
		::EndDeferWindowPos(hdwp);
	}
}

void CMainFrame::OnAppExit()
{
	PostMessage(WM_CLOSE);
}

void CMainFrame::OnAppExitAll()
{
	CSimpleArray<HWND> array;
	int cnt = GetSiblings(m_hWnd, array);
	for (int i = 0; i < cnt; i++)
		::SendMessage(array[i], WM_CLOSE, 0, 0L);
	PostMessage(WM_CLOSE);
}

void CMainFrame::OnRefresh()
{
	GetFolderTree().RefreshTree();
	UpdateFileView(NULL);

	SetupDriveBar();
	m_appbar.Refresh();
}

void CMainFrame::OnEditPaste()
{
	CIDListPtr idl;
	if (idl.Clone(GetCurrentFileBrowser().GetCurrentFolder())) {
		CWaitCursor wc;
		InvokeIDList(m_hWnd, idl, gszCmdPaste);
	}
}

void CMainFrame::OnEditPasteShortcut()
{
	CIDListPtr idl;
	if (idl.Clone(GetCurrentFileBrowser().GetCurrentFolder()))
		PasteShortcutFromClipboard(m_hWnd, idl);
}

void CMainFrame::OnViewIcon()
{
	CMenuHandle pop = GetSubSequenceMenu(3, 9);
	if (pop.IsMenu()) {
		CPoint pt;
		GetSelectionCenterPoint(&pt);
		TrackMenu(pop, TPM_LEFTALIGN | TPM_RIGHTBUTTON, pt);
	}
}

void CMainFrame::OnViewSize()
{
	CFileBrowseWindow browser = GetCurrentFileBrowser();
	CIconListStyle ls;
	browser.GetListStyle(&ls);
	ls.ToggleSize();
	UpdateFileBrowserListStyle(ls);
}

void CMainFrame::OnViewDate()
{
	CFileBrowseWindow browser = GetCurrentFileBrowser();
	CIconListStyle ls;
	browser.GetListStyle(&ls);
	ls.ToggleDate();
	UpdateFileBrowserListStyle(ls);
}

void CMainFrame::OnViewAttrib()
{
	CFileBrowseWindow browser = GetCurrentFileBrowser();
	CIconListStyle ls;
	browser.GetListStyle(&ls);
	ls.ToggleAttr();
	UpdateFileBrowserListStyle(ls);
}

void CMainFrame::OnViewNameEllipsis()
{
	CFileBrowseWindow browser = GetCurrentFileBrowser();
	CIconListStyle ls;
	browser.GetListStyle(&ls);
	ls.ToggleEllp();
	UpdateFileBrowserListStyle(ls);
}

void CMainFrame::OnIconRelax()
{
	gnListStat ^= LTS_RELAX;
	fvApp.SaveStates();
	// 全てのビューを更新（最初に非表示のビュー、最後に現在のビュー）
	int cnt = m_tabbar.GetCount();
	for (int i = 0; i < cnt; i++) {
		CDocument* pDoc = (CDocument*)m_tabbar.GetItemData(i);
		if (pDoc && pDoc != GetDocument())
			pDoc->GetFileBrowser().UpdateView(DCM_CHANGEFORMAT);
	}
	GetCurrentFileBrowser().UpdateView(DCM_CHANGEFORMAT);
	GetFolderTree().UpdateView(DCM_CHANGEFORMAT);
}

void CMainFrame::OnViewSetDefaultStyle()
{
	// 現在の表示スタイルをデフォルトとして記録
	CFileBrowseWindow browser = GetCurrentFileBrowser();
	CIconListStyle ls;
	browser.GetListStyle(&ls);

	CString strOld; gDefaultIconListStyle.toString(strOld);
	CString strNew; ls.toString(strNew);
	CString str; str.Format(IDS_OLDNEW_ICONLISTSTYLE, strOld.GetString(), strNew.GetString());
	if (AppMessage(IDS_CHANGE_DEFAULTICONLISTSTYLE, str.GetString(), MB_QINFO) == IDOK) {
		gDefaultIconListStyle = ls;
		SetRegBinary(IDS_SEC_OPERATIONS, IDS_KEY_DEFAULTICONLISTSTYLE, &gDefaultIconListStyle, sizeof(gDefaultIconListStyle));
	}
}

void CMainFrame::OnViewDriveInfo()
{
	int drive;
	TCHAR dir[MAX_PATH * 2];
	if (GetFolderTree().GetCurrentPath(dir, ARRAYSIZE(dir)))
		drive = PathGetDriveNumber(dir);
	else
		drive = PathGetDriveNumber(_GCDATA.m_strWinDir);
	fopDriveInfo(m_hWnd, drive);
}

void CMainFrame::OnViewSort()
{
/* @@
	CMenuHandle pop = GetSubSequenceMenu(3, 3);
	if (pop.IsMenu()) {
		CPoint pt;
		GetSelectionCenterPoint(&pt);
		TrackMenu(pop, TPM_LEFTALIGN | TPM_RIGHTBUTTON, pt);
	}
@@ */

	CFileBrowseWindow browser = GetCurrentFileBrowser();
	browser.QuerySort();
}

void CMainFrame::OnViewDesktopTree()
{
	gnSysStat = TOGGLEFLAG(gnSysStat, STS_DRIVEROOT);
	SetRegValue(IDS_SEC_OPERATIONS, IDS_KEY_SYSSTATUS, gnSysStat);

	CIDListPtr idlRoot;
	if (gnSysStat & STS_DRIVEROOT) {
		// 保持しているルート情報を取得
		idlRoot.Clone(GetDocument()->GetRootFolder());
		if (idlRoot.isNull()) {
			// ファイルペインから現在のフォルダを取得してルートを類推
			idlRoot.Clone(GetCurrentFileBrowser().GetCurrentFolder());
			idlRoot.MakeRoot();
		}
	}
	else
		SHGetSpecialFolderLocation(NULL, CSIDL_DESKTOP, (LPITEMIDLIST*)&idlRoot.m_pidl);
	GetFolderTree().RefreshTree(idlRoot);
}

void CMainFrame::OnSortRange(UINT id, UINT code, HWND hwnd)
{
	CFileBrowseWindow browser = GetCurrentFileBrowser();
	// ソート情報を更新
	browser.Sort(id, 0);
	// 現在のブラウザのソート情報を取得して履歴に記録
	CIconListStyle ls;
	browser.GetListStyle(&ls);
	m_mapListStyle.SetItemStyle(browser.GetCurrentFolder(), ls);
}

void CMainFrame::OnTypeFileHidden()
{
	gnFileStat = RESETFLAG(gnFileStat, FTS_SUPERHIDDEN);
	gnFileStat = TOGGLEFLAG(gnFileStat, FTS_HIDDEN);
	GetFolderTree().RefreshTree();
	UpdateFileView(NULL);
}

void CMainFrame::OnTypeFileSuperHidden()
{
	gnFileStat = RESETFLAG(gnFileStat, FTS_HIDDEN);
	gnFileStat = TOGGLEFLAG(gnFileStat, FTS_SUPERHIDDEN);
	GetFolderTree().RefreshTree();
	UpdateFileView(NULL);
}

void CMainFrame::OnTypeFolder()
{
	gnFileStat = TOGGLEFLAG(gnFileStat, FTS_FOLDER);
	UpdateFileView(NULL);
}

void CMainFrame::OnShortcutSetup()
{
	CSetupSheet sheet(IDS_CAPTION_SHORTCUTMENU);

	CShortcutMenuPage tree;
	CShortcutMenuPage file;

	tree.SetCommands(m_scutTree);
	file.SetCommands(m_scutFile);
	sheet.AddPage(tree);
	sheet.AddPage(file);

	if (sheet.DoModal() == IDOK) {
		if (tree.m_update)	tree.GetCommands(m_scutTree);
		if (file.m_update)	file.GetCommands(m_scutFile);
	}
}

void CMainFrame::OnViewKeyMap()
{
	CShowKeymapWnd wnd;
	int nCmd = wnd.TrackKeymapMenu(m_hWnd);
	if (nCmd > 0)
		PostMessage(WM_COMMAND, (WPARAM)nCmd, 0);
}

/*
void CMainFrame::OnDiskMakeImage() 
{
	CDll dll;
	dll.LoadLibrary(DLL_MEDIA);
	BOOL (WINAPI *pfn)(HWND);
	(FARPROC&)pfn = dll.FindFunction("fmMakeImageUI");
	if (pfn)
		(*pfn)(m_hWnd);
}
*/
/*
void CMainFrame::OnDiskRestoreImage() 
{
	CDll dll;
	dll.LoadLibrary(DLL_MEDIA);
	BOOL (WINAPI *pfn)(HWND);
	(FARPROC&)pfn = dll.FindFunction("fmRestoreImageUI");
	if (pfn)
		(*pfn)(m_hWnd);
}
*/
/*
void CMainFrame::OnDiskCheck()
{
	TCHAR dir[MAX_PATH * 2];
	if (!GetFolderTree().GetCurrentPath(dir, ARRAYSIZE(dir)))
		PathBuildRoot(dir, PathGetDriveNumber(_GCDATA.m_strWinDir));
	lccRunDll(m_hWnd, DLL_MEDIA, _T("RunCheckDisk"), dir);
}
*/
void CMainFrame::OnDirUp()
{
	// ------------------------------------------
	// 現在のフォルダの親フォルダに移動する
	// ------------------------------------------

	// 現在のフォルダを取得して保持しておく
	CIDListPtrHandle idl = GetDocument()->GetDocumentFolder();
	if (idl.isDesktop())
		return;

	CIDListPtr idlRoot;
	idlRoot.Clone(GetDocument()->GetRootFolder());
	if (idl.isEqual(idlRoot)) {
		if (!(gnConfStat & COS_UPDIR_MASK)) {
			MessageBeep(0);
			return;
		}
		if ((gnConfStat & COS_UPDIR_MASK) == COS_UPDIR_CONFIRM) {
			CUpdirConfirmDlg dlg;
			if (dlg.DoModal() != IDOK)
				return;
			if (!dlg.m_bConfirmAnyTime)
				fvApp.SetConfStat((gnSysStat & ~COS_UPDIR_MASK) | COS_UPDIR_ENABLE);
		}
	}

	BOOL bLocked = m_tabbar.isItemLocked(m_tabbar.GetCurSel());
	if (bLocked) {
		idlRoot.RemoveLastID();
		GetFolderTree().RefreshTree(idlRoot);
		TabNew(idlRoot);
	}
	else {
		idlRoot.RemoveLastID();
		GetFolderTree().RefreshTree(idlRoot);
	}
	SelectFolderItem(idl);
}

void CMainFrame::OnDirScriptSync()
{
	fopScriptSyncNew(m_hWnd);
}

void CMainFrame::OnOpenFolderSyncDataFolder()
{
	TCHAR dir[MAX_PATH];
	fopGetSyncDataFolder(dir);

	CIDListPtr idl;
	idl.CreateFromPath(dir);
	BOOL bLocked = m_tabbar.isItemLocked(m_tabbar.GetCurSel());
	if (bLocked)
		TabNew(idl);
	else
		OpenFolder(idl);
}

void CMainFrame::OnHistoryBack()
{
	CHistory* pHistory = GetHistory();
	ATLASSERT(pHistory != NULL);

	CIDListPtr idl;
	if (idl.Clone(pHistory->Back())) {
		pHistory->DisableUpdate(true);
		OpenFolder(idl);
	}
}

void CMainFrame::OnHistoryForward()
{
	CHistory* pHistory = GetHistory();
	ATLASSERT(pHistory != NULL);
	CIDListPtr idl;
	if (idl.Clone(pHistory->Forward())) {
		pHistory->DisableUpdate(true);
		OpenFolder(idl);
	}
}

void CMainFrame::OnHistoryTrace()
{
	CPoint pt;
	if (!GetToolBarButtonPoint(GetCurrentMessage(), IDM_DIR_TRACE, &pt))
		GetCursorPos(&pt);

	CHistory* pHistory = GetHistory();
	ATLASSERT(pHistory != NULL);
	CIDListPtr idl;
	if (idl.Clone(pHistory->HistoryMenu(m_hWnd, pt))) {
		pHistory->DisableUpdate(true);
		if (!OpenFolder(idl)) {
			// 履歴から一致するすべてを除外する
			pHistory->RemoveItemAll(idl);
		}
	}
}

void CMainFrame::OnDirHistory()
{
	CPoint pt;
//@ --- ツールバーから削除されたので不要
//@	if (!GetToolBarButtonPoint(GetCurrentMessage(), IDM_DIR_HISTORY, &pt))
//@		GetCursorPos(&pt);
		GetSelectionCenterPoint(&pt);

	CHistory* pHistory = GetHistory();
	ATLASSERT(pHistory != NULL);
	CIDListPtr idl;
	if (idl.Clone(pHistory->HistoryMenu(m_hWnd, pt))) {
		pHistory->DisableUpdate(true);
		OpenFolder(idl);
	}
}

void CMainFrame::OnHistoryClear()
{
	// 履歴のクリア
	CHistory* pHistory = GetHistory();
	ATLASSERT(pHistory != NULL);
	pHistory->Clear();

	pHistory->PushItem(GetCurrentFileBrowser().GetCurrentFolder());

	// ナビバーの更新
	m_navibar.UpdateList(pHistory);
}

void CMainFrame::OnDirHome()
{
	CIDListPtr idl = GetRegPidl(IDS_SEC_FOLDER, IDS_KEY_HOME);
	if (idl.isNull())
		idl.GetFolderLocation(NULL, CSIDL_DRIVES);

	if (!idl.isNull())
		OpenFolder(idl);
}

void CMainFrame::OnDirEmptyRecycleBin()
{
	SHEmptyRecycleBin(m_hWnd, _T(""), 0);
}

void CMainFrame::OnDirMakeBriefcase()
{
	TCHAR dir[MAX_PATH * 2];
	if (GetFolderTree().GetCurrentPath(dir, ARRAYSIZE(dir)))
		fopMakeBriefcase(m_hWnd, dir);
}

/* @@
void CMainFrame::OnDirLinkCheck()
{
	TCHAR dir[MAX_PATH * 2];
	if (!GetFolderTree().GetCurrentPath(dir, ARRAYSIZE(dir)))
		PathBuildRoot(dir, PathGetDriveNumber(_GCDATA.m_strWinDir));
	fopValidateLinks(m_hWnd, dir);
}
@@ */

void CMainFrame::OnDoScriptSync(UINT nID, UINT code, HWND hwnd)
{
	int index = nID - IDM_SSYNC_START;
	int cnt = m_arrSSync.GetSize();
	if (cnt > 0 && index >= 0 && index < cnt) {
		CCmItem* pci = m_arrSSync[index];
		pci->Invoke(m_hWnd, NULL);
	}
}

void CMainFrame::OnDirRoot()
{
	// ルートに移動する。ただし、ここでいうルートとはツリーのルートではなく
	// 現在パスに対するルートパスのことである
	CIDListPtr idl;
	idl.Clone(GetCurrentFileBrowser().GetCurrentFolder());
	if (!idl.isNull()) {
		// 現在パスのルートを求める
		CIDListPtr idlRoot;
		idlRoot.Clone(idl);
		idlRoot.MakeRoot();
		if (idlRoot != idl) {
			// ルートが異なるので、ルートに移動
			BOOL bLocked = m_tabbar.isItemLocked(m_tabbar.GetCurSel());
			if (bLocked)
				TabNew(idlRoot);
			else
				OpenFolder(idlRoot);
			return;
		}
	}
	// ひとつ上へ移動してみる
	OnDirUp();
}

void CMainFrame::OnDirGoto()
{
	m_navibar.GotoFolder();
}

void CMainFrame::OnFolderSync()
{
	TCHAR dir[MAX_PATH * 2];
	LPCTSTR pszDir = dir;
	if (!GetFolderTree().GetCurrentPath(dir, ARRAYSIZE(dir)))
		pszDir = NULL;

	__attension_app();

//@	lccRunDll(m_hWnd, DLL_FOPSRV, _T("FolderSyncManager"), pszDir);
	TCHAR exec[MAX_PATH * 2];
	if (lccRegGetAppPath(_T("syncman.exe"), exec, ARRAYSIZE(exec))) {
		InvokeFile(m_hWnd, exec, pszDir);
	}
}

void CMainFrame::OnFileSearch()
{
	TCHAR dir[MAX_PATH * 2];
	if (!GetFolderTree().GetCurrentPath(dir, ARRAYSIZE(dir)))
		PathBuildRoot(dir, PathGetDriveNumber(_GCDATA.m_strWinDir));
	lccDllExecute(m_hWnd, DLL_FINDER, _T("finderRun"), dir, _T("/search"), FALSE);
}

BOOL CMainFrame::OnDeviceChange(UINT nEventType, DWORD_PTR dwData)
{
/* @@@
	if (nEventType == DBT_DEVICEARRIVAL ||
		nEventType == DBT_DEVICEREMOVECOMPLETE)
	{
		DEV_BROADCAST_HDR* pdbhdr = (DEV_BROADCAST_HDR*)dwData;
		if (pdbhdr->dbch_devicetype == DBT_DEVTYP_VOLUME)
			SetupDriveBar();
	}
@@@ */
	return TRUE;
}

void CMainFrame::RefuseToFolder(LPITEMIDLIST pidlTarget, LPITEMIDLIST pidlRefuse)
{
	// pidlTargetと現在のフォルダを比較して、双方が一致または
	// pidlTargetが現在のフォルダの親フォルダであれば、指定されたcsidlへ移動する

	LPCITEMIDLIST pidlCur = GetCurrentFileBrowser().GetCurrentFolder();
	// 現在のフォルダか？
	BOOL bMatch = ::ILIsEqual(pidlCur, pidlTarget);
	// でなければ現在のフォルダの親フォルダか？
	if (!bMatch)
		bMatch = ::ILIsParent(pidlTarget, pidlCur, FALSE);
	// そうであれば指定のフォルダに移動
	if (bMatch) {
		CIDListPtr idl;
		if (IS_INTPIDL(pidlRefuse)) {
			int csidl = PtrToInt(pidlRefuse);
			idl.GetFolderLocation(m_hWnd, csidl);
		}
		else {
			if (pidlRefuse == (LPITEMIDLIST)-1)
				idl = idl_GetParent(pidlTarget);
			else
				idl.Clone(pidlRefuse);
		}

		if (!idl.isNull()) {
			// 現在のタブがロック中であれば強制解除
			int index = m_tabbar.GetCurSel();
			if (m_tabbar.isItemLocked(index))
				m_tabbar.LockItem(index, FALSE);
			OpenFolder(idl);
		}
	}
}

#ifdef _DEBUG
#ifdef DUMP_SHELLNOTIFY
static void dumpShellNotify(long lEvent, PIDLIST_ABSOLUTE* pidls)
{
	int num = 1;
	switch(lEvent) {
	case SHCNE_DRIVEADDGUI: dbg.printf(L"SHCNE_DRIVEADDGUI\n"); break;
	case SHCNE_FREESPACE: dbg.printf(L"SHCNE_FREESPACE\n"); break;
	case SHCNE_EXTENDED_EVENT: dbg.printf(L"SHCNE_EXTENDED_EVENT\n"); break;
	case SHCNE_SERVERDISCONNECT: dbg.printf(L"SHCNE_SERVERDISCONNECT\n"); break;
	case SHCNE_CREATE: dbg.printf(L"SHCNE_CREATE\n"); break;
	case SHCNE_DELETE: dbg.printf(L"SHCNE_DELETE\n"); break;
	case SHCNE_RENAMEITEM: dbg.printf(L"SHCNE_RENAMEITEM\n"); num = 2; break;
	case SHCNE_ATTRIBUTES: dbg.printf(L"SHCNE_ATTRIBUTES\n"); break;
	case SHCNE_MKDIR: dbg.printf(L"SHCNE_MKDIR\n"); break;
	case SHCNE_RMDIR: dbg.printf(L"SHCNE_RMDIR\n"); break;
	case SHCNE_MEDIAINSERTED: dbg.printf(L"SHCNE_MEDIAINSERTED\n"); break;
	case SHCNE_MEDIAREMOVED: dbg.printf(L"SHCNE_MEDIAREMOVED\n"); break;
	case SHCNE_DRIVEREMOVED: dbg.printf(L"SHCNE_DRIVEREMOVED\n"); break;
	case SHCNE_DRIVEADD: dbg.printf(L"SHCNE_DRIVEADD\n"); break;
	case SHCNE_NETSHARE: dbg.printf(L"SHCNE_NETSHARE\n"); break;
	case SHCNE_NETUNSHARE: dbg.printf(L"SHCNE_NETUNSHARE\n"); break;
	case SHCNE_UPDATEDIR: dbg.printf(L"SHCNE_UPDATEDIR\n"); break;
	case SHCNE_UPDATEITEM: dbg.printf(L"SHCNE_UPDATEITEM\n"); break;
	case SHCNE_UPDATEIMAGE: dbg.printf(L"SHCNE_UPDATEIMAGE\n"); num = 0; break;
	case SHCNE_RENAMEFOLDER: dbg.printf(L"SHCNE_RENAMEFOLDER\n"); num = 2; break;
	case SHCNE_ASSOCCHANGED: dbg.printf(L"SHCNE_ASSOCCHANGED\n"); break;
	}

	CString str;
	while (num > 0) {
		idl_GetName(*pidls, SHGDN_FORPARSING, str);
		int descid = idl_GetDescriptionId(*pidls);
		dbg.printf(L"      (%d)%s\n", descid, str.GetString());
		pidls++;
		--num;
	}
}
#endif		// DUMP_SHELLNOTIFY
#endif		// _DEBUG

void CMainFrame::OnShellChangeNotify(long lEvent, PIDLIST_ABSOLUTE* pidls)
{
#ifdef _DEBUG
#ifdef DUMP_SHELLNOTIFY
dumpShellNotify(lEvent, pidls);
#endif		// DUMP_SHELLNOTIFY
#endif		// _DEBUG

	TCHAR buf[MAX_PATH * 2];
	CString str;

	CFolderTreeWindow tree = GetFolderTree();
	CFileBrowseWindow browser = GetCurrentFileBrowser();
	CIDListPtrHandle idlCurFolder = browser.GetCurrentFolder();

	switch (lEvent) {
	//------------------------------
	// ネットワークの共有・解除
	//------------------------------
	case SHCNE_NETUNSHARE:
	case SHCNE_NETSHARE:
		m_drive.UpdateItem(pidls[0]);
		tree.UpdateFolder(pidls[0]);
		if (lEvent == SHCNE_NETUNSHARE)
			TabValidate(pidls[0]);
		break;
	//------------------------------
	// メディアの挿入・取り出し
	//------------------------------
	case SHCNE_MEDIAREMOVED:
	case SHCNE_MEDIAINSERTED:
		m_drive.UpdateItem(pidls[0]);
		if (SHGetPathFromIDList(pidls[0], buf))
			fopSetDirRemind(PathGetDriveNumber(buf), buf);
		else
			buf[0] = 0;

		if (lEvent == SHCNE_MEDIAREMOVED)
			RefuseToFolder(pidls[0], MAKEINTPIDL(CSIDL_DRIVES));
		tree.UpdateFolder(pidls[0], (lEvent == SHCNE_MEDIAREMOVED));
		if (lEvent == SHCNE_MEDIAREMOVED)
			TabValidate(pidls[0]);
		else if (gnTabStat & TAB_MEDIAINSERTED) {
			CEnterFolder ef;
			if (ef.Enter(buf)) {
				ef.Leave();
				// メディアを新しいタブで非アクティブ状態で開く
				TabNew(pidls[0], false);
			}
		}
		break;
	//------------------------------
	// ドライブの追加
	//------------------------------
	case SHCNE_DRIVEADD:
		buf[0] = 0;
		if (SHGetPathFromIDList(pidls[0], buf))
			fopSetDirRemind(PathGetDriveNumber(buf), buf);
		tree.AddFolder(pidls[0]);
		SetupDriveBar();

		if (buf[0] && (gnTabStat & TAB_DRIVEADD)) {
			CEnterFolder ef;
			if (ef.Enter(buf)) {
				// ドライブを新しいタブで非アクティブ状態で開く
				ef.Leave();
				TabNew(pidls[0], false);
			}
		}
		break;
	//------------------------------
	// ドライブの削除
	//------------------------------
	case SHCNE_DRIVEREMOVED:
		if (SHGetPathFromIDList(pidls[0], buf))
			fopSetDirRemind(PathGetDriveNumber(buf), buf);
		RefuseToFolder(pidls[0], MAKEINTPIDL(CSIDL_DRIVES));
		tree.DeleteFolder(pidls[0]);
		TabValidate(pidls[0]);
		SetupDriveBar();
		break;
	//------------------------------
	// オブジェクト作成
	//------------------------------
	case SHCNE_CREATE:
	{
/*
		SFGAOF attr = SFGAO_FOLDER;
		if (!idl_GetAttributesOf(pidls[0], &attr) || ((attr & SFGAO_FOLDER) == 0))
*/
		SFGAOF attr;
		if (!idl_GetShellAttributes(pidls[0], SFGAO_FOLDER, &attr) || ((attr & SFGAO_FOLDER) == 0))
			break;
	}
	//------------------------------
	// フォルダの作成
	//------------------------------
	case SHCNE_MKDIR:
		tree.AddFolder(pidls[0]);

		if (SHGetPathFromIDList(pidls[0], buf)) {
			if (idlCurFolder.isChild(pidls[0], TRUE)) {
				browser.PostMessage(WM_IDLISTNOTIFY, FILE_ACTION_ADDED, (LPARAM)ILClone(pidls[0]));
			}
		}
		break;
	//------------------------------
	// フォルダのリネーム
	//------------------------------
	case SHCNE_RENAMEFOLDER:
		if (idl_GetPathFromIDList(pidls[0], str) && PathIsRoot(str))
			break;
		tree.DeleteFolder(pidls[0]);
		tree.AddFolder(pidls[1]);
		TabValidate(pidls[0]);
		break;
	//------------------------------
	// アイテムの更新
	//------------------------------
	case SHCNE_UPDATEITEM:
		tree.UpdateFolder(pidls[0]);
		if (idl_GetPathFromIDList(pidls[0], str) && PathIsRoot(str))
			m_drive.UpdateItem(pidls[0]);
		if (idlCurFolder.isChild(pidls[0], TRUE))
			browser.PostMessage(WM_IDLISTNOTIFY, FILE_ACTION_MODIFIED, (LPARAM)ILClone(pidls[0]));
		break;
	//------------------------------
	// フォルダの更新
	//------------------------------
	case SHCNE_UPDATEDIR:
		if (!idl_GetPathFromIDList(pidls[0], str)) {
			// デジカメ・スキャナ等？
			tree.EnumFolder(pidls[0], FALSE);
			// コンピューター配下？
			CIDListPtr drives;
			drives.GetFolderLocation(NULL, CSIDL_DRIVES);
			if (drives.isEqual(pidls[0]))
				SetupDriveBar();
			break;
		}
		if (_pathisdirectory(str)) break;
		// fall down
	//------------------------------
	// オブジェクト削除
	//------------------------------
	case SHCNE_DELETE:
	{
/*
		SFGAOF attr = SFGAO_FOLDER;
		if (!idl_GetAttributesOf(pidls[0], &attr) || ((attr & SFGAO_FOLDER) == 0))
*/
		SFGAOF attr;
		if (!idl_GetShellAttributes(pidls[0], SFGAO_FOLDER, &attr) || ((attr & SFGAO_FOLDER) == 0))
			break;
	}
	//------------------------------
	// フォルダの削除
	//------------------------------
	case SHCNE_RMDIR:
		RefuseToFolder(pidls[0], (LPITEMIDLIST)-1);
		tree.DeleteFolder(pidls[0]);
		TabValidate(pidls[0]);

		if (SHGetPathFromIDList(pidls[0], buf)) {
			if (idlCurFolder.isChild(pidls[0], TRUE)) {
				browser.PostMessage(WM_IDLISTNOTIFY, FILE_ACTION_REMOVED, (LPARAM)ILClone(pidls[0]));
			}
		}
		break;
	//------------------------------
	// シェルイメージの更新
	//------------------------------
	case SHCNE_UPDATEIMAGE:
	{
		int iImage;
		if (pidls[1])
			iImage = SHHandleUpdateImage(pidls[1]);
		else
			iImage = *(int UNALIGNED *)((BYTE*)pidls + 2);

		if (iImage != -1) {
			GetFolderTree().UpdateView(DCM_UPDATEIMAGE, iImage);
			m_viewFavo.UpdateView(DCM_UPDATEIMAGE, iImage);
			if (m_appbar.IsWindowVisible()) m_appbar.Refresh();
		}
		break;
	}
	//------------------------------
	// 関連付けの変更
	//------------------------------
	case SHCNE_ASSOCCHANGED:
		UpdateFileView(NULL);
		break;

	default:
		break;
	}
}

void CMainFrame::OnNetConnect()
{
	// ネットワークドライブの接続
	WNetConnectionDialog(m_hWnd, RESOURCETYPE_DISK);
}

void CMainFrame::OnNetDisconnect()
{
	// ネットワークドライブの切断
	WNetDisconnectDialog(m_hWnd, RESOURCETYPE_DISK);
}

void CMainFrame::OnNetAddPlace()
{
	// ネットワークの場所の追加
	ShellExecute(m_hWnd, NULL, gszRunDLL, _T("shwebsvc.dll,AddNetPlaceRunDll"), NULL, SW_SHOWNORMAL);
}

void CMainFrame::ShowBar(HWND hwndBar, BOOL bShow)
{
	CToolBarCtrl bar = hwndBar;
	UINT id = bar.GetDlgCtrlID();
	int index = m_rebar.IdToIndex(id);
	if (index >= 0)
		m_rebar.ShowBand(index, bShow);
	else {
		index = m_vtrebar.IdToIndex(id);
		if (index >= 0)
			m_vtrebar.ShowBand(index, bShow);
	}
}

void CMainFrame::OnBarMenu()
{
	ShowBar(m_menubar, !m_menubar.IsWindowVisible());
}

void CMainFrame::OnBarProp()
{
	ShowBar(m_propbar, !m_propbar.IsWindowVisible());
}

void CMainFrame::OnBarSort()
{
	ShowBar(m_sortbar, !m_sortbar.IsWindowVisible());
}

void CMainFrame::OnBarStat()
{
	if (m_statbar.IsWindowVisible()) {
		m_statbar.ShowWindow(SW_HIDE);
		gnBarStat &= ~BTS_STATUS;
	}
	else {
		m_statbar.ShowWindow(SW_SHOWNOACTIVATE);
		gnBarStat |= BTS_STATUS;
	}
	UpdateLayout();
}

void CMainFrame::OnBarFunction()
{
	bool bEnable = false;
	if (m_functionbar.IsWindowVisible()) {
		m_functionbar.ShowWindow(SW_HIDE);
		gnBarStat &= ~BTS_FUNCTION;
	}
	else {
		m_functionbar.ShowWindow(SW_SHOWNOACTIVATE);
		gnBarStat |= BTS_FUNCTION;
		bEnable = true;
	}
	m_functionbar.EnableTimer(bEnable);
	UpdateLayout();
}

void CMainFrame::OnBarApp()
{
	if (m_appbar.IsWindowVisible()) {
		HWND hwndView = m_ViewSwitch.GetCurrentView();
		if (hwndView == m_appbar)
			m_ViewSwitch.ActivateNextView(FALSE);
		m_splitApp.SetSinglePaneMode((gnBarStat & BTS_APPRIGHT) ? SPLIT_PANE_LEFT : SPLIT_PANE_RIGHT);
		gnBarStat &= ~BTS_APPBAR;
	}
	else {
		m_splitApp.SetSinglePaneMode(SPLIT_PANE_NONE);
		gnBarStat |= BTS_APPBAR;
	}
}

void CMainFrame::OnBarAppSideChange()
{
	// オブジェクトパネルが非表示の状態でここを実行すると
	// 不具合が発生する

	if (!m_appbar.IsWindowVisible()) {
		m_splitApp.SetSinglePaneMode(SPLIT_PANE_NONE);
		gnBarStat |= BTS_APPBAR;
	}

	CRect rc;
	m_splitApp.GetWindowRect(&rc);
	int pos = m_splitApp.GetSplitterPos() + m_splitApp.m_cxySplitBar;

	HWND hwndL = m_splitApp.GetSplitterPane(SPLIT_PANE_LEFT);
	HWND hwndR = m_splitApp.GetSplitterPane(SPLIT_PANE_RIGHT);

	DWORD style = (gnBarStat & BTS_APPRIGHT) ? 0 : SPLIT_RIGHTALIGNED;
	m_splitApp.SetSplitterExtendedStyle(style, SPLIT_RIGHTALIGNED);

	m_splitApp.SetSplitterPanes(hwndR, hwndL);
	m_splitApp.SetSplitterPos(rc.Width() - pos);
	gnBarStat = TOGGLEFLAG(gnBarStat, BTS_APPRIGHT);
}

void CMainFrame::OnBarFavo()
{
	if (m_viewFavo.IsWindowVisible()) {
		HWND hwndView = m_ViewSwitch.GetCurrentView();
		if (hwndView == m_viewFavo)
			m_ViewSwitch.ActivateNextView(FALSE);
		m_splitTree.SetSinglePaneMode(SPLIT_PANE_TOP);
		gnBarStat &= ~BTS_FAVO;
	}
	else {
		m_splitTree.SetSinglePaneMode(SPLIT_PANE_NONE);
		gnBarStat |= BTS_FAVO;

		// しおりをアクティブに
		m_ViewSwitch.SetActiveView(m_viewFavo, true);
	}
}

void CMainFrame::OnBarDrive()
{
	ShowBar(m_drive, !m_drive.IsWindowVisible());
}

void CMainFrame::OnBarDriveVertRange(UINT id, UINT code, HWND hwnd)
{
	// ドライブバーの配置、縦置き（左右）、横置き

	if (id == IDM_BAR_DRIVE_VERTLEFT)
		gnBarStat &= ~BTS_DRIVERIGHT;
	else if (id == IDM_BAR_DRIVE_VERTRIGHT)
		gnBarStat |= BTS_DRIVERIGHT;

	if (id != IDM_BAR_DRIVE_HORZ && gnBarStat & BTS_VERTDRIVE) {
		UpdateLayout(FALSE);
		return;
	}

	CReBarCtrl& rsbar = m_drive.isVertical() ? m_vtrebar : m_rebar;
	CReBarCtrl& rdbar = m_drive.isVertical() ? m_rebar : m_vtrebar;
	int idCtrl = m_drive.GetDlgCtrlID();
	int index = rsbar.IdToIndex(idCtrl);
	rsbar.ShowBand(index, FALSE);			// ドライブバーを非表示
	rsbar.DeleteBand(index);				// ドライブバーをバンドから削除

	gnBarStat = TOGGLEFLAG(gnBarStat, BTS_VERTDRIVE);
	m_drive.SetVertical((gnBarStat & BTS_VERTDRIVE) != 0);
	AddSimpleReBarBandCtrl(rdbar, m_drive, idCtrl, NULL, TRUE, 0, FALSE);
	if (gnBarStat & BTS_VERTDRIVE)
		RecalcDriveBar();

	// レジストリ更新
	SetRegValue(IDS_SEC_OPERATIONS, IDS_KEY_BARSTATUS, gnBarStat);
}

void CMainFrame::OnBarSmart()
{
	ShowBar(m_smartbar, !m_smartbar.IsWindowVisible());
}

void CMainFrame::OnBarNavi()
{
	ShowBar(m_navibar, !m_navibar.IsWindowVisible());
}

void CMainFrame::OnBarLargeIcon()
{
	gnBarStat = TOGGLEFLAG(gnBarStat, BTS_LARGEICON);
	ResetToolbarImage();

	CSimpleArray<CToolBarCtrl*> arr;
	arr.Add(&m_propbar);
	arr.Add(&m_sortbar);
	arr.Add(&m_navibar);
	int i, num = m_arrToolbar.GetSize();
	for (i = 0; i < num; i++) {
		arr.Add(m_arrToolbar[i]);
	}

	num = arr.GetSize();
	for (i = 0; i < num; i++) {
		CToolBarCtrl* pBar = arr[i];
		int index = m_rebar.IdToIndex(::GetWindowLong(pBar->m_hWnd, GWL_ID));
		REBARBANDINFO rbbi = { RunTimeHelper::SizeOf_REBARBANDINFO(), RBBIM_SIZE | RBBIM_IDEALSIZE | RBBIM_CHILDSIZE };
		m_rebar.GetBandInfo(0, &rbbi);

		CRect rc;
		int cnt = pBar->GetButtonCount();
		if (cnt > 0) {
			pBar->GetItemRect(0, &rc);
			rbbi.cxMinChild = rc.right;	
			pBar->GetItemRect(cnt - 1, &rc);
			rbbi.cx = rc.right;
			rbbi.cyMinChild = rc.Height();
			rbbi.cxIdeal = rbbi.cx;
			m_rebar.SetBandInfo(index, &rbbi);
		}
	}
	// smart bar 
	m_smartbar.ChangeButtonImageSize();
	RecalcSmartBar();
	// drive bar
	m_drive.ChangeButtonImageSize();
	RecalcDriveBar();

	fvApp.SaveStates();
	OnBarAutoReposition();
}

void CMainFrame::OnBarAutoReposition()
{
	if (gnBarStat & BTS_LOCKREBAR) return;

	int cnt = m_rebar.GetBandCount();
	for (int i = 0; i < cnt; i++) {
		REBARBANDINFO rbbi = { RunTimeHelper::SizeOf_REBARBANDINFO(), RBBIM_SIZE | RBBIM_IDEALSIZE | RBBIM_CHILD | RBBIM_STYLE };
		m_rebar.GetBandInfo(i, &rbbi);
		CRect rc( 0, 0, 0, 0);
		m_rebar.GetBandBorders(i, &rc);

		if (rbbi.hwndChild != m_navibar) {
			rbbi.fMask = RBBIM_SIZE;
			rbbi.cx = rbbi.cxIdeal + rc.left + rc.right;
			m_rebar.SetBandInfo(i, &rbbi);
		}
	}
}

void CMainFrame::LockBars(bool bLock)
{
	// CRebarCtrlの LockBands は利用しない

	int nBandCount = m_rebar.GetBandCount();
	for (int i = 0; i < nBandCount; i++) {
		REBARBANDINFO rbbi = { RunTimeHelper::SizeOf_REBARBANDINFO(), RBBIM_STYLE };
		BOOL bRet = m_rebar.GetBandInfo(i, &rbbi);
		ATLASSERT(bRet);

		if ((rbbi.fStyle & RBBS_GRIPPERALWAYS) == 0) {
			rbbi.fStyle |= RBBS_GRIPPERALWAYS;
			bRet = m_rebar.SetBandInfo(i, &rbbi);
			ATLASSERT(bRet);
			rbbi.fStyle &= ~RBBS_GRIPPERALWAYS;
		}

		rbbi.fMask |= RBBIM_TEXT;
		rbbi.fMask |= RBBIM_HEADERSIZE;
		TCHAR title[4];
		if (bLock) {
			rbbi.fStyle |= RBBS_NOGRIPPER;
			title[0] = 0;
			title[1] = 0;
			rbbi.lpText = title;
			rbbi.cch = 1;
			rbbi.cxHeader = 24;
		}
		else {
			rbbi.fStyle &= ~RBBS_NOGRIPPER;
			title[0] = 0;
			rbbi.lpText = title;
			rbbi.cch = 0;
			rbbi.cxHeader = UINT_MAX;
		}
		bRet = m_rebar.SetBandInfo(i, &rbbi);
		ATLASSERT(bRet);
	}
}

void CMainFrame::OnBarLock()
{
	gnBarStat = TOGGLEFLAG(gnBarStat, BTS_LOCKREBAR);
	// リバーロック
//@	m_rebar.LockBands((gnBarStat & BTS_LOCKREBAR) != 0);
	LockBars((gnBarStat & BTS_LOCKREBAR) != 0);
}

void CMainFrame::BarSettings(POINT point)
{
	// 2004/09/01
	// メインメニューを表示していない場合のみ許可
	if (!m_menubar.m_bMenuActive) {
		CMenuHandle pop = GetSubSequenceMenu(3, 4);
		TrackMenu(pop, TPM_LEFTALIGN | TPM_RIGHTBUTTON, point);
	}
}


void CMainFrame::OnBarSettings()
{
	CRect rc;
	GetWindowRect(&rc);
	CPoint point;
	GetCursorPos(&point);

	if (!rc.PtInRect(point)) {
		GetClientRect(&rc);
		ClientToScreen(&rc);
		point = rc.TopLeft();
	}
	BarSettings(point);
}

void CMainFrame::TabbarShowAny()
{
	if (gnTabStat & TAB_BARSHOWANY) {
		if ((m_tabbar.GetCount() < 2) && !m_tabbar.IsWindowVisible()) {
			m_tabbar.ShowWindow(SW_SHOWNOACTIVATE);
			UpdateLayout();
		}
	}
	else {
		if ((m_tabbar.GetCount() < 2) && m_tabbar.IsWindowVisible()) {
			m_tabbar.ShowWindow(SW_HIDE);
			UpdateLayout();
		}
	}
	m_tabbar.SetExStyle(TCS_EX_HOVERSELECT, (gnTabStat & TAB_HOVERSELECT) ? TCS_EX_HOVERSELECT : 0);
}

void CMainFrame::OnToolBarMenu(UINT nID, UINT code, HWND hwnd)
{
	int pos = nID - IDW_TBFIRST;
	int cnt = m_arrToolbar.GetSize();
	if (0 <= pos && pos < cnt) {
		CToolBarCtrl* pBar = m_arrToolbar[pos];
		int index = m_rebar.IdToIndex(pBar->GetDlgCtrlID());
		m_rebar.ShowBand(index, !pBar->IsWindowVisible());
	}
}

void CMainFrame::OnToolGrep()
{
	TCHAR dir[MAX_PATH * 2];
	if (!GetFolderTree().GetCurrentPath(dir, ARRAYSIZE(dir)) || PathIsUNC(dir))
		PathBuildRoot(dir, PathGetDriveNumber(_GCDATA.m_strWinDir));
	lccDllExecute(m_hWnd, DLL_MIL, MIL_GREP, dir, _T("/edit"), FALSE);
}

/*
void CMainFrame::OnToolSlideShow()
{
	TCHAR dir[MAX_PATH * 2];
	if (GetFolderTree().GetCurrentPath(dir, ARRAYSIZE(dir)))
		lccDllExecute(m_hWnd, DLL_GMIL, _T("slideRun"), dir, _T("/image"), FALSE);
}
*/

void CMainFrame::OnSetSelect()
{
	CFileBrowseWindow browser = GetCurrentFileBrowser();

	m_ViewSwitch.SetActiveView(browser);
	browser.SelectAll(FILESEL_ALL);
}

void CMainFrame::OnRevSelect()
{
	CFileBrowseWindow browser = GetCurrentFileBrowser();

	m_ViewSwitch.SetActiveView(browser);
	browser.SelectAll(FILESEL_REV);
}

void CMainFrame::FileFilterList(const CPoint& pt)
{
	CFilterSetting* filterInfo = GetDocument()->GetFilter();
	ATLASSERT(filterInfo != NULL);

	CFilterSetting fs;
	if (SelectFilterSetting(m_hWnd, true, fs, pt) && fs != *filterInfo) {
		*filterInfo = fs;
		UpdateFileView(NULL);
		UpdateTitle();
	}
}

void CMainFrame::OnFileFilterList()
{
	CPoint pt;
	GetSelectionCenterPoint(&pt);
	FileFilterList(pt);
}

void CMainFrame::OnSetFilter()
{
	TCHAR buf[MAX_PATH * 2];
	GetCurrentFileBrowser().GetSelectionFilter(buf, (gnFopStat & FOS_REXFINDER) != 0);

	CFilterSetting* filterInfo = GetDocument()->GetFilter();
	ATLASSERT(filterInfo != NULL);

	CFilterSetting fs = *filterInfo;
	fs.m_bUseRegExp = ((gnFopStat & FOS_REXFINDER) != 0);
	fs.m_strFilter = buf;
	if (FileFilterDialog(m_hWnd, &fs, filterInfo->m_strFilter) == IDOK && fs != *filterInfo) {
		SetFileFilter(&fs);
	}
}

void CMainFrame::OnReleaseFileFilter()
{
	SetFileFilter(NULL);
}

void CMainFrame::SetFileFilter(const CFilterSetting* pfs)
{
	CFilterSetting* filterInfo = GetDocument()->GetFilter();
	*filterInfo = pfs ? *pfs : CFilterSetting(gnFopStat & FOS_REXFINDER);
	UpdateFileView(NULL);
	UpdateTitle();

	m_statbar.CheckPaneItem(ID_INDICATOR_FILTER, !filterInfo->isDefault());
	if (!filterInfo->isDefault()) {
		UpdateFilterHistory(*filterInfo);
	}
}

void CMainFrame::MaskSelectList(const CPoint& pt)
{
	CFilterSetting fs;
	if (SelectFilterSetting(m_hWnd, false, fs, pt)) {
		CFileBrowseWindow browser = GetCurrentFileBrowser();
		m_ViewSwitch.SetActiveView(browser);
		browser.FilterSelect(fs.m_strFilter, fs.m_bUnmatchFilter, fs.m_bUseRegExp, fs.m_bCaseSense);
	}
}

void CMainFrame::OnMaskSelectList()
{
	CPoint pt;
	GetSelectionCenterPoint(&pt);
	MaskSelectList(pt);
}

int CALLBACK CMainFrame::ApplyMaskFilter(const CFilterSetting* pfs)
{
	CFileBrowseWindow browser = gpMainFrame->GetCurrentFileBrowser();
	if (pfs)
		browser.FilterMark(pfs->m_strFilter, pfs->m_bUnmatchFilter, pfs->m_bUseRegExp, pfs->m_bCaseSense);
	else
		browser.FilterMark(NULL);
	return 0;
}

void CMainFrame::OnMaskSelect()
{
	CFileBrowseWindow browser = GetCurrentFileBrowser();

	TCHAR buf[MAX_PATH * 2];
	browser.GetSelectionFilter(buf, (gnFopStat & FOS_REXFINDER) != 0);

	browser.SaveSelection();
	CFilterSetting fs; fs.Set(NULL, buf, FALSE, ((gnFopStat & FOS_REXFINDER) != 0), FALSE, TRUE);
	if (DoFilterSelect(m_hWnd, &fs, ApplyMaskFilter) == IDOK) {
		m_ViewSwitch.SetActiveView(browser);
		browser.FilterMark(NULL);
		browser.FilterSelect(fs.m_strFilter, fs.m_bUnmatchFilter, fs.m_bUseRegExp, fs.m_bCaseSense);
	}
	else {
		browser.FilterMark(NULL);
		browser.RestoreSelection();
	}
}

void CMainFrame::OnFileRenExt()
{
	CFileBrowseWindow browser = GetCurrentFileBrowser();

	m_ViewSwitch.SetActiveView(browser);
//@	browser.RenameExtAll();
	RerootCurrentMessage(browser);
}

void CMainFrame::OnFileShellNew()
{
	CMenuHandle pop = GetSubSequenceMenu(0, 3);
	if (pop.IsMenu()) {
		CPoint pt;
//@ --- ツールバーから削除されたので不要
//@		if (!GetToolBarButtonPoint(GetCurrentMessage(), IDM_FILE_SHELLNEW, &pt))
//@			GetCursorPos(&pt);
			GetSelectionCenterPoint(&pt);
		TrackMenu(pop, TPM_LEFTALIGN | TPM_RIGHTBUTTON, pt);
	}
}

void CMainFrame::OnCommandSpecialFolder(UINT id, UINT code, HWND hwnd)
{
	CIDListPtr idl;
	idl.GetKnownFolder(stKnownFolder[id - IDM_SPFL_START].guid);
	if (!idl.isNull()) {
//@		if (_ISKEYDOWN(VK_MENU))
//@			InvokeIDList(m_hWnd, idl);
		if (_ISKEYDOWN(VK_SHIFT))
			ExecNewFrame(idl, _T("/force"), _ISKEYDOWN(VK_CONTROL));
		else if (_ISKEYDOWN(VK_CONTROL))
			TabNew(idl);
		else
			OpenFolder(idl);
	}
}

void CMainFrame::OnCommandShellNew(UINT nID, UINT code, HWND hwnd)
{
	TCHAR dir[MAX_PATH * 2];
	if (!GetFolderTree().GetCurrentPath(dir, ARRAYSIZE(dir)))
		SHGetFolderPath(m_hWnd, CSIDL_DESKTOP, NULL, SHGFP_TYPE_CURRENT, dir);
	CShellNewItem* psi = m_arrShellNew[nID - IDM_SHLNEW_START];
	shnCreateShellNew(psi->m_ext, psi->m_name, dir);
}

void CMainFrame::OnCommandTabReopenRecent()
{
	OnCommandTabReopenRange(IDM_TAB_REOPEN_FIRST, 0, NULL);
}

void CMainFrame::OnCommandTabReopenRange(UINT nID, UINT code, HWND hwnd)
{
	int index = nID - IDM_TAB_REOPEN_FIRST;
	int cnt = m_arrTabReopen.GetItemCount();
	if (cnt > 0 && index >= 0 && index < cnt) {
		CIconListStyle ls;
		CCmItem* pci = m_arrTabReopen.Pop(index, ls);
		TabNew(pci->m_idl, true, false, true, &ls);
		delete pci;
	}
}

void CMainFrame::OnCommandTabGroupRange(UINT nID, UINT code, HWND hwnd)
{
	int index = nID - IDM_TAB_GROUP_FIRST;
	int cnt = m_arrTabGroup.GetSize();
	if (cnt > 0 && index >= 0 && index < cnt) {
		CCmItem* pci = m_arrTabGroup[index];
		CString str;
		if (pci->GetParseName(str) && _pathfileexists(str)) {
			if (TabGroupLoad(str) < 1) {
				if (AppMessage(IDS_INVALID_TABGROUP, pci->m_name, MB_QYESNO) == IDYES) {
					fopRemoveFileNoConfirm(str);
				}
			}
		}
	}
}

void CMainFrame::OnCommandDriveAccess(UINT nID, UINT code, HWND hwnd)
{
	TCHAR path[MAX_PATH];
	nID -= IDM_DRVACCESS_A;
	if (!fopGetDirRemind(nID, path) || !_pathfileexists(path)) {
		fopSetDirRemind(nID, NULL);
		PathBuildRoot(path, nID);
	}

	CIDListPtr idl;
	idl.CreateFromPath(path);
	BOOL bLocked = m_tabbar.isItemLocked(m_tabbar.GetCurSel());
	if (bLocked)
		TabNew(idl);
	else
		OpenFolder(idl);
}

void CMainFrame::OnDriveBarCommandRange(UINT id, UINT code, HWND hwnd)
{
	// シェブロンメニューで選択された場合の処理
	m_drive.PostMessage(WM_COMMAND, MAKEWPARAM(id, code), (LPARAM)hwnd);
}

void CMainFrame::OnFileNewFile()
{
	TCHAR dir[MAX_PATH * 2];
	if (!GetFolderTree().GetCurrentPath(dir, ARRAYSIZE(dir)))
		SHGetFolderPath(m_hWnd, CSIDL_DESKTOP, NULL, SHGFP_TYPE_CURRENT, dir);
	shnCreateNewFile(_T(".tmp"), dir);
}

void CMainFrame::OnOptionShellExt()
{
	CComPtr<ICtx> pc;
	HRESULT hr = pc.CoCreateInstance(__uuidof(Ctx));
	if (SUCCEEDED(hr)) {
		pc->Setup(m_hWnd);
	}
}

void CMainFrame::QuickAccessMenu(const CPoint& pt)
{
	int openMode;
	CIDListPtr idl = TrackFavoriteMenu(m_hWnd, false, true, pt, openMode);
	if (!idl.isNull()) {
		if      (openMode == FavoOpenByNewWindow)   ExecNewFrame(idl, _T("/force"));
		else if (openMode == FavoOpenByNewTab)      TabNew(idl);
		else                                        OpenFolder(idl);
	}
}

void CMainFrame::OnQuickAccess()
{
	CPoint pt;
	GetSelectionCenterPoint(&pt);
	QuickAccessMenu(pt);
}

void CMainFrame::OnOptionEnv(UINT idd)
{
	// 常駐していれば HOTKEY を一時停止
	HWND hwndTsr = ::FindWindow(APP_TRAYCLASS, NULL);
	if (hwndTsr)
		::SendMessage(hwndTsr, nRM_TRAYSERVICE, 101, 0);

	COptionSetupDlg dlg;
	if (dlg.DoModal(m_hWnd, idd) == IDOK) {
		// プロファイル保管
		SaveProfile();

		// アクセラレータ関連
		if (dlg.m_bAccelSetting) {
			fvApp.menuHotKey.Restore();
			fvApp.MakeLocalAccelTable();
			UpdateMenuAccelerator();
		}

		if (dlg.m_bDisplaySetting) {
			fvApp.LoadIconMap();
			// 全てのビューを更新（最初に非表示のビュー、最後に現在のビュー）
			int cnt = m_tabbar.GetCount();
			for (int i = 0; i < cnt; i++) {
				CDocument* pDoc = (CDocument*)m_tabbar.GetItemData(i);
				if (pDoc && pDoc != GetDocument())
					pDoc->GetFileBrowser().UpdateView(DCM_CHANGEFORMAT);
			}
			GetCurrentFileBrowser().UpdateView(DCM_CHANGEFORMAT);
			GetFolderTree().UpdateView(DCM_CHANGEFORMAT);
			m_viewFavo.UpdateView(DCM_CHANGEFORMAT);
			m_appbar.UpdateProperty();
			m_functionbar.UpdateProperty(DCM_CHANGEFORMAT);
			UpdateTitle();
		}

		// ドライブバー関連
		if (dlg.m_bDriveSetting)
			SetupDriveBar();

		// タブ関連
		if (dlg.m_bTabSetting)
			TabbarShowAny();
	}

	// 常駐 HOTKEY 再開
	if (hwndTsr)
		::SendMessage(hwndTsr, nRM_TRAYSERVICE, 100, 0);
}

void CMainFrame::UpdateTitle()
{
	// ナビバー
	CHistory* pHistory = GetHistory();
	ATLASSERT(pHistory != NULL);
	m_navibar.UpdateList(pHistory);

	CCmItem* pci = pHistory->GetCurrentItem();
	ATLASSERT(pci != NULL);

	// タブバー
	CString name;
	idl_GetName(pci->m_idl, SHGDN_INFOLDER, name);
	int sel = m_tabbar.GetCurSel();
	if (sel >= 0) {
		m_tabbar.SetItem(sel, (gnTabStat & TAB_FULLPATH) ? pci->m_name : name, pci->GetImageIndex());
	}

	// タイトルの設定
	if (gnListStat & LTS_FULLPATH)
		name = pci->m_name;
	CString app;
#ifdef _DEBUG
	app = _T("(DEBUG)");
#endif
	if (fvApp.isAdmin())
		app.AppendFormat(IDS_IAMADMIN);
	if (app.GetLength() > 0)
		app += L": ";
	app += name;
	SetWindowText(app);
}

void CMainFrame::RecordAccessFolder(LPCITEMIDLIST pidlTarget)
{
	if (pidlTarget)
		m_arrAccessHistory.AddItem(pidlTarget);
}

void CMainFrame::UpdateFileBrowserListStyle(const CIconListStyle& ls)
{
	CFileBrowseWindow browser = GetCurrentFileBrowser();
	browser.SetListStyle(&ls);
	m_mapListStyle.SetItemStyle(browser.GetCurrentFolder(), ls);
}

/*
 * ファイル一覧のスタイルが変更されたら CFileBrowseView より呼び出される
 */
void CMainFrame::BrowserListStyleChanged(const ICONLISTSTYLE* pls)
{
	CIconListStyle ls = *pls;
	m_statbar.HidePaneItem(ID_INDICATOR_IMAGESIZE, !ls.isStyleImage());
	if (ls.isStyleImage()) {
		UpdateStatusbarImageSize(ls);
	}
}

void CMainFrame::UpdateStatusbarImageSize(const CIconListStyle& ls)
{
	CString str;
	if (ls.isStyleImage())
		str.Format(L"%d x %d", ls.size_thumb.cx, ls.size_thumb.cy);
	else
		str = L" ";
	m_statbar.SetStockText(ID_INDICATOR_IMAGESIZE, str.GetString());
}

void CMainFrame::OnIconEnlarge()
{
	CFileBrowseWindow browser = GetCurrentFileBrowser();
	CIconListStyle ls;
	browser.GetListStyle(&ls);

	if      (ls.isStyleNormal())	ls.ToggleEnlargeNormal();
	else if (ls.isStyleReport())	ls.ToggleEnlargeReport();
	else if (ls.isStyleOutline())	ls.ToggleEnlargeOutline();
	else if (ls.isStyleIcon())		ls.ToggleEnlargeIcon();
	else return;

	UpdateFileBrowserListStyle(ls);
}

void CMainFrame::OnHotIconModeRange(UINT id, UINT code, HWND hwnd)
{
	// ショートカットキー用
	UINT n;
	switch (id) {
		case IDM_HOT_ICON_NORMAL: n = IDM_ICON_NORMAL; break;
		case IDM_HOT_ICON_REPORT: n = IDM_ICON_REPORT; break;
		case IDM_HOT_ICON_OUTLINE: n = IDM_ICON_OUTLINE; break;
		case IDM_HOT_ICON_IMAGE: n = IDM_ICON_IMAGE;  break;
		case IDM_HOT_ICON_ICON: n = IDM_ICON_ICON;  break;
		case IDM_HOT_ICON_SWITCH: {
			CIconListStyle ls;
			GetCurrentFileBrowser().GetListStyle(&ls);
			if      (ls.isStyleOutline())	n = IDM_ICON_NORMAL;
			else if (ls.isStyleReport())	n = IDM_ICON_IMAGE;
			else if (ls.isStyleImage())		n = IDM_ICON_ICON;
			else if (ls.isStyleIcon())		n = IDM_ICON_OUTLINE;
			else                       		n = IDM_ICON_REPORT;
			break;
		}
		default: return;
	}
	OnIconModeRange(n, 0, NULL);
}

void CMainFrame::OnIconModeRange(UINT id, UINT code, HWND hwnd)
{
	int mode;
	switch (id) {
		case IDM_ICON_NORMAL:	mode = ICON_NORMAL; break;
		case IDM_ICON_REPORT:	mode = ICON_REPORT; break;
		case IDM_ICON_OUTLINE:	mode = ICON_OUTLINE; break;
		case IDM_ICON_IMAGE:	mode = ICON_IMAGE;  break;
		case IDM_ICON_ICON:		mode = ICON_ICON;  break;
		default: return;
	}

	CFileBrowseWindow browser = GetCurrentFileBrowser();
	CIconListStyle ls;
	browser.GetListStyle(&ls);
	if (!ls.isStyle(mode)) {
		ls.SetStyle(mode);
		UpdateFileBrowserListStyle(ls);
	} else if (mode == ICON_IMAGE) {
		// クリックごとにイメージサイズを変更する
		UINT cmd;
		switch (ls.size_thumb.cx) {
			case  72: cmd = IDM_IMAGE_096; break;
			case  96: cmd = IDM_IMAGE_128; break;
			case 128: cmd = IDM_IMAGE_160; break;
			case 160: cmd = IDM_IMAGE_192; break;
			case 192: cmd = IDM_IMAGE_072; break;
			default:  cmd = IDM_IMAGE_128; break;
		}
		OnThumbnailSizeRange(cmd, 0, NULL);
	}
}

void CMainFrame::OnThumbnailSizeRange(UINT id, UINT code, HWND hwnd)
{
	CFileBrowseWindow browser = GetCurrentFileBrowser();

	int cx = 96;
	switch (id) {
		case IDM_IMAGE_072: cx = 72; break;
		case IDM_IMAGE_096: cx = 96; break;
		case IDM_IMAGE_128: cx = 128; break;
		case IDM_IMAGE_160: cx = 160; break;
		case IDM_IMAGE_192: cx = 192; break;
	}

	CIconListStyle ls;
	browser.GetListStyle(&ls);
	if (cx != ls.size_thumb.cx) {
		ls.size_thumb.cx = cx;
		ls.size_thumb.cy = cx;
		ls.SetStyle(ICON_IMAGE);
		// 画面更新
		browser.SetListStyle(&ls);
		// 属性を記録
		m_mapListStyle.SetItemStyle(browser.GetCurrentFolder(), ls);
	}
}

void CMainFrame::OnThumbnailZoomIn()
{
	CFileBrowseWindow browser = GetCurrentFileBrowser();

	bool bUpdateView = false;
	CIconListStyle ls;
	browser.GetListStyle(&ls);
	if (ls.isStyleImage()) {
		int cx = 0;
		switch (ls.size_thumb.cx) {
			case 96:	cx = 128; break;
			case 128:	cx = 160; break;
			case 160:	cx = 192; break;
			case 192:	cx = 96; break;
			default:	cx = 128; break;
		}
		if (cx) {
			ls.size_thumb.cx = cx;
			ls.size_thumb.cy = cx;
			browser.SetListStyle(&ls);
		}
	}
	else if (ls.isStyleNormal()) {
		if (!ls.isEnlargeNormal()) {
			ls.SetEnlargeNormal();
			bUpdateView = true;
		}
	}
	else if (ls.isStyleReport()) {
		if (!ls.isEnlargeReport()) {
			ls.SetEnlargeReport();
			bUpdateView = true;
		}
	}
	else if (ls.isStyleOutline()) {
		if (!ls.isEnlargeOutline()) {
			ls.SetEnlargeOutline();
			bUpdateView = true;
		}
	}
	else if (ls.isStyleIcon()) {
		if (!ls.isEnlargeIcon()) {
			ls.SetEnlargeIcon();
			bUpdateView = true;
		}
	}
	if (bUpdateView)
		UpdateFileBrowserListStyle(ls);
}

void CMainFrame::OnThumbnailZoomOut()
{
	CFileBrowseWindow browser = GetCurrentFileBrowser();

	bool bUpdateView = false;
	CIconListStyle ls;
	browser.GetListStyle(&ls);
	if (ls.isStyleImage()) {
		int cx = 0;
		switch (ls.size_thumb.cx) {
			case 96:	cx = 192; break;
			case 128:	cx = 96; break;
			case 160:	cx = 128; break;
			case 192:	cx = 160; break;
			default:	cx = 128; break;
		}
		if (cx) {
			ls.size_thumb.cx = cx;
			ls.size_thumb.cy = cx;
			browser.SetListStyle(&ls);
		}
	}
	else if (ls.isStyleNormal()) {
		if (ls.isEnlargeNormal()) {
			ls.ResetEnlargeNormal();
			bUpdateView = true;
		}
	}
	else if (ls.isStyleReport()) {
		if (ls.isEnlargeReport()) {
			ls.ResetEnlargeReport();
			bUpdateView = true;
		}
	}
	else if (ls.isStyleOutline()) {
		if (ls.isEnlargeOutline()) {
			ls.ResetEnlargeOutline();
			bUpdateView = true;
		}
	}
	else if (ls.isStyleIcon()) {
		if (ls.isEnlargeIcon()) {
			ls.ResetEnlargeIcon();
			bUpdateView = true;
		}
	}
	if (bUpdateView)
		UpdateFileBrowserListStyle(ls);
}

void CMainFrame::UpdateFileView(LPCITEMIDLIST pidlFolder)
{
	CFileBrowseWindow browser = GetCurrentFileBrowser();

	if (browser.IsWindow()) {
		fvApp.LoadIconMap();
		browser.Browse(pidlFolder);
	}
}

void CMainFrame::OnEditKeyMap()
{
	HIMAGELIST himKey = lccLoadImageList(RESINST, RT_RCDATA, _MKIR(IDB_KEYBOARD), 20, CLR_NONE);

	CKeyPage page_abc;
	page_abc.SetImage(himKey, 10, 26);
	page_abc.m_bUseShiftCtrl = true;
	page_abc.SetCommandList(fvApp.keymapAbc, ARRAYSIZE(fvApp.keymapAbc));

	CKeyPage page_123;
	page_123.SetImage(himKey, 0, 10);
	page_123.SetCommandList(fvApp.keymapNum, ARRAYSIZE(fvApp.keymapNum));

	CKeyPage page_fnc;
	page_fnc.SetImage(himKey, 36, 12);
	page_fnc.SetCommandList(fvApp.keymapFnc, ARRAYSIZE(fvApp.keymapFnc));
	page_fnc.m_bUseShift = true;

	CString str;
	GetCommandLabel(IDM_OPTION_KEYMAP, str);
	CKeySheet sheet((LPCTSTR)str);
	sheet.AddPage(page_abc);
	sheet.AddPage(page_123);
	sheet.AddPage(page_fnc);

	if (sheet.DoModal() == IDOK) {
		if (page_abc.m_bUpdate) {
			page_abc.GetCommandList(fvApp.keymapAbc);
		}
		if (page_123.m_bUpdate) {
			page_abc.GetCommandList(fvApp.keymapNum);
		}
		if (page_fnc.m_bUpdate) {
			page_fnc.GetCommandList(fvApp.keymapFnc);
			m_functionbar.ResetButtons();
		}
		if (page_abc.m_bUpdate || page_123.m_bUpdate || page_fnc.m_bUpdate) {
			fvApp.SaveKeyMap();
			fvApp.MakeLocalAccelTable();
			if (gnListStat & LTS_MENUACCEL)
				UpdateMenuAccelerator();
		}
	}

	ImageList_Destroy(himKey);
}

void CMainFrame::OnAppAbout()
{
	APPABOUTINFO_V2 ai = { 0 };
	ai.magic = MAGIC_APPABOUTINFO_V2;
	if (glFVValid) {
		CString str;
		if (GetRegText(IDS_SEC_USER, IDS_KEY_USERNAME, str) > 0) lstrcpyn(ai.szUser, str, ARRAYSIZE(ai.szUser));
		if (GetRegText(IDS_SEC_USER, IDS_KEY_COMPANY, str) > 0)  lstrcpyn(ai.szCompany, str, ARRAYSIZE(ai.szCompany));
	}
	::LoadString(RESINST, IDS_WEB_HOMEPAGE, ai.szUrl, ARRAYSIZE(ai.szUrl));
	lccShowAppAbout(m_hWnd, (LPAPPABOUTINFO)&ai);
}

LRESULT CMainFrame::OnUpdateFavorite(WPARAM wParam, LPARAM lParam)
{
	if ((HWND)lParam != m_viewFavo) {
		m_viewFavo.LoadList();
	}
	return 0;
}

void CMainFrame::OnNaviBarClicked()
{
	if (!m_navibar.IsWindowVisible()) return;

	if (m_ViewSwitch.GetCurrentView() != m_navibar) {
		// フォーカス設定をおこなってアクティブとしてセット
		m_ViewSwitch.SetActiveView(m_navibar);
	}
}

void CMainFrame::OnNaviBarSetFocus()
{
	if (m_ViewSwitch.GetCurrentView() != m_navibar) {
		// フォーカス設定をおこなわず(false)にアクティブとしてセット
		m_ViewSwitch.SetActiveView(m_navibar, false);
	}
}

void CMainFrame::__UpdateMenuAccelerator(HMENU hmenu, bool bForce)
{
	if (!IsMenu(hmenu)) return;

	int cnt, numAccl = 0;
	ACCEL* pa = NULL;
	if (m_hAccel) {
		cnt = CopyAcceleratorTable(m_hAccel, NULL, 0);
		pa = (ACCEL*)malloc(cnt * sizeof(ACCEL));
		numAccl = CopyAcceleratorTable(m_hAccel, pa, cnt);
	}

	HACCEL hacc = fvApp.GetLocalAccelerator();
	if ((gnListStat & LTS_MENUACCEL) && hacc) {
		cnt = CopyAcceleratorTable(hacc, NULL, 0);
		if (cnt > 0) {
			ACCEL* paw = (ACCEL*)malloc(cnt * sizeof(ACCEL));
			cnt = CopyAcceleratorTable(hacc, paw, cnt);
			if (cnt > 0) {
				if (!pa)
					pa = paw;
				else {
					pa = (ACCEL*)realloc(pa, (cnt + numAccl) * sizeof(ACCEL));
					memcpy(pa + numAccl, paw, cnt * sizeof(ACCEL));
					free(paw);
				}
				numAccl += cnt;
			}
			else
				free(paw);
		}
	}

	if (pa) {
		// サブメニューを変換
		int numItems = ::GetMenuItemCount(hmenu);
		for (int i = 0; i < numItems; i++) {
			HMENU hSubMenu = GetSubMenu(hmenu, i);
			if (hSubMenu)
				lccApplyAccelMenu(hSubMenu, pa, numAccl);
		}

		// メインメニューを変換
		lccApplyAccelMenu(hmenu, pa, numAccl);
		delete [] pa;
	}
}

void CMainFrame::UpdateMenuAccelerator(bool bForce)
{
	__UpdateMenuAccelerator(m_menubar.GetMenu(), bForce);
}

LRESULT CMainFrame::OnQueryImageList(WPARAM wParam, LPARAM lParam)
{
	int id = (int)wParam;
	HIMAGELIST him = NULL;
	if (_INRANGE(id, IDM_DRIVEBAR_IDSTART, IDM_DRIVEBAR_IDEND)) {
		him = (HIMAGELIST)m_drive.SendMessage(WM_QUERYIMAGELIST, wParam, lParam);
	}
	return (LRESULT)him;
}

#ifdef SUPPORT_FINDURL
void CMainFrame::OnToolFindUrl()
{
	TCHAR buf[MAX_PATH];
	if (fopGetUrlHistoryFolder(buf))
		fopFindUrl(m_hWnd);
}
#endif

LRESULT CMainFrame::OnRegThanks(WPARAM /*wParam*/, LPARAM /*lParam*/)
{
	NotifyUser(IDI_STAR, IDS_REGTHANKS_TITLE, IDS_REGTHANKS_CAPTION, IDS_REGTHANKS_TEXT);
	return 0;
}

LRESULT CMainFrame::OnRegOK(WPARAM /*wParam*/, LPARAM /*lParam*/)
{
	CMenuHandle menu = GetSubSequenceMenu(6, -1);
	if (menu.IsMenu())
		menu.DeleteMenu(IDM_HELP_SHAREWARE, MF_BYCOMMAND);
	return 0;
}
