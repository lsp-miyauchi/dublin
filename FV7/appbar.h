///////////////////////////////////////////////////////////////////
// CAppBar Class
//

#pragma once
#ifndef __APPBAR_H__
#define __APPBAR_H__

#include "dropper.h"
#include "mousewheeler.h"

#define IDW_APPBAR_RAPPER			100
#define IDW_APPBAR_PANE				101

///////////////////////////////////////////////////////////////////
// �댯�Ȓ�`

#define SFGAO_SPECIALFOLDER			0x00800000L

BOOL __fastcall InvokeIDList(HWND hParent, LPCITEMIDLIST pidl, LPCTSTR lpVerb);


///////////////////////////////////////////////////////////////////
//

class CEntering
{
public:
	bool m_bEntered;
	CEntering() : m_bEntered(false) { }
	__inline void Enter() { m_bEntered = true; }
	__inline void Leave() { m_bEntered = false; }
	__inline bool IsEntered() { return m_bEntered; }
};

///////////////////////////////////////////////////////////////////
// CAppItem

class CAppItem
{
public:
	CIDListPtr m_idl;
	SFGAOF m_attr;
	int m_iIcon;
	CString m_strDisplayName;
	CString m_strIconLocation;
	CString m_strInfoTip;
	CString m_strParams;
	GUID m_guid;

public:
	CAppItem();
	CAppItem(LPCITEMIDLIST pidl);
	CAppItem(LPCTSTR pszFile);

// operation
	void InitParams();
	void ParseInfo();
	void ShowProperty(HWND hwnd) const;
	int GetIconIndex();
	LPCTSTR GetInfoTip();
	void SetIconPath(LPCTSTR pszIconPath, int iconIndex);

	BOOL Invoke() const;
	BOOL Open() const;
	inline void* GetUIObjectOf(HWND hwnd, REFIID riid) const { return m_idl.GetUIObjectOf(hwnd, riid); }
	inline IDataObject* GetDataObject(HWND hwnd) const { return (IDataObject*)GetUIObjectOf(hwnd, IID_IDataObject); }
	inline IContextMenu* GetContextMenuObject(HWND hwnd) const { return (IContextMenu*)GetUIObjectOf(hwnd, IID_IContextMenu); }
	inline IDropTarget* GetDropTarget(HWND hwnd) const { return (IDropTarget*)GetUIObjectOf(hwnd, IID_IDropTarget); }
	inline bool isFolder() const { return ((m_attr & SFGAO_FOLDER) == SFGAO_FOLDER); }
	inline bool isFileSystem() const { return ((m_attr & SFGAO_FILESYSTEM) == SFGAO_FILESYSTEM); }
	inline bool isDropTarget() const { return ((m_attr & SFGAO_DROPTARGET) == SFGAO_DROPTARGET); }
	bool GetGuid(CString& str);
	bool SetGuid(LPCTSTR pszGuidString);
};


///////////////////////////////////////////////////////////////////
// CAppGroup

#define GIT_LARGE			0
#define GIT_SMALL			1
#define GIT_EXTRALARGE		2
#define GIT_LIST			3

class CAppGroup
{
public:
	CAppGroup();
	CAppGroup(LPCTSTR pszGroupName);
	~CAppGroup();

	void Wep();

	CRect m_rc;
	CAtlArray<CAppItem*> m_array;
	CString m_strGroupName;
	int m_nFocused;
	int m_typeIcon;						// GIT_LARGE, GIT_SMALL, GIT_EXTRALARGE, ...
	COLORREF m_clrText;
	COLORREF m_clrBack;
};

/////////////////////////////////////////////////////////////////////////////
// CAppBar window


class CAppBar
		: public CWindowImpl<CAppBar>
		, public TDropperImpl<CAppBar>
		, public CMouseTrackerImpl<CAppBar>
{
private:
	CMWListViewCtrl m_list;
	CAtlArray<CAppGroup*> m_page;
	HIMAGELIST m_himSmall;
	HIMAGELIST m_himLarge;
	HIMAGELIST m_himExtra;

	int m_cyHeader;
	int m_cySpacer;
	float m_cyChar;
	int m_cxMaxTextTab;
	int m_nDragItem;
	CString m_strAppGroupFile;
	int m_nHilitedPage;
	int m_nHotPage;
	int m_nCurPage;
	bool m_bCapture:1;
	bool m_bItemLabelEditing:1;
	Font* m_pLabelFont;
	CFont m_font;

public:
	DECLARE_WND_CLASS(_T("AppBar"))

	CAppBar();

BEGIN_MSG_MAP(CAppBar)
	CHAIN_MSG_MAP(CMouseTrackerImpl<CAppBar>)
	CHAIN_MSG_MAP(TDropperImpl<CAppBar>)
	ON_WM_CREATE()
	ON_WM_SIZE()
	ON_WM_DESTROY()
	ON_WM_PAINT()
	ON_WM_LBUTTONDOWN()
	ON_WM_RBUTTONDOWN()
	ON_WM_CANCELMODE()
	ON_WM_PARENTNOTIFY()
	ON_WM_CONTEXTMENU()
	ON_WM_INITMENUPOPUP()
	ON_WM_ERASEBKGND()
	ON_WM_SETFOCUS()
	ON_WM_MOUSEACTIVATE()
	ON_NOTIFY(NM_DBLCLK, IDW_APPBAR_PANE, OnNotifyDblClk)
	ON_NOTIFY(NM_RCLICK, IDW_APPBAR_PANE, OnNotifyRClick)
	ON_NOTIFY(NM_SETFOCUS, IDW_APPBAR_PANE, OnNotifySetFocus)
	ON_NOTIFY(LVN_KEYDOWN, IDW_APPBAR_PANE, OnNotifyKeyDown)
	ON_NOTIFY(LVN_BEGINLABELEDIT, IDW_APPBAR_PANE, OnNotifyBeginLabelEdit)
	ON_NOTIFY(LVN_ENDLABELEDIT, IDW_APPBAR_PANE, OnNotifyEndLabelEdit)
	ON_NOTIFY(LVN_BEGINDRAG, IDW_APPBAR_PANE, OnNotifyBeginDrag)
	ON_NOTIFY(LVN_ITEMCHANGED, IDW_APPBAR_PANE, OnNotifyItemChanged)
	ON_NOTIFY(LVN_GETINFOTIP, IDW_APPBAR_PANE, OnNotifyItemGetInfoTip)
	ON_NOTIFY(LVN_GETDISPINFO, IDW_APPBAR_PANE, OnNotifyItemGetDispInfo)
	ON_COMMAND(IDM_APPPAGE_EXTRAICON, OnPageExtraIcon)
	ON_COMMAND(IDM_APPPAGE_LARGEICON, OnPageLargeIcon)
	ON_COMMAND(IDM_APPPAGE_SMALLICON, OnPageReportSmallIcon)
	ON_COMMAND(IDM_APPPAGE_LARGEICONLIST, OnPageReportLargeIcon)
	ON_COMMAND(IDM_APPPAGE_HIDEBAR, OnPageHideBar)
	ON_COMMAND(IDM_APPPAGE_RENAME, OnPageRename)
	ON_COMMAND(IDM_APPPAGE_DELGROUP, OnPageDelete)
	ON_COMMAND(IDM_APPPAGE_NEWGROUP, OnPageAdd)
	ON_COMMAND(IDM_APPPAGE_BKGND, OnPageBkgnd)
	ON_COMMAND(IDM_APPPAGE_IMPORT, OnPageImport)
	ON_COMMAND(IDM_APPPAGE_SORT, OnPageSort)
	ON_COMMAND(IDM_APPPAGE_PAGEUP, OnGroupMoveUp)
	ON_COMMAND(IDM_APPPAGE_PAGEDOWN, OnGroupMoveDown)
	ON_COMMAND(IDM_APPPAGE_LEFT, OnBarLeft)
	ON_COMMAND(IDM_APPPAGE_RIGHT, OnBarRight)
	ON_COMMAND(IDM_PAGEITEM_OPEN, OnItemOpen)
	ON_COMMAND(IDM_PAGEITEM_DELETE, OnItemDelete)
	ON_COMMAND(IDM_PAGEITEM_RENAME, OnItemRename)
	ON_COMMAND(IDM_PAGEITEM_ICON, OnItemIcon)
	ON_COMMAND(IDM_PAGEITEM_PROPERTY, OnItemProperty)
	ON_COMMAND(IDM_PAGEITEM_OPENSRCDIR, OnItemOpenSrcDir)
	ON_COMMAND(IDM_PAGEITEM_MOVEUP, OnItemMoveUp)
	ON_COMMAND(IDM_PAGEITEM_MOVEDOWN, OnItemMoveDown)
	ON_COMMAND(IDM_PAGEITEM_OPENNEWTAB, OnItemOpenNewTab)
	ON_COMMAND(IDM_PAGEITEM_OPENNEWWINDOW, OnItemOpenNewWindow)
	ON_MESSAGE(WM_HELPHITTEST, OnHelpHitTest)
END_MSG_MAP()

	int OnCreate(LPCREATESTRUCT lpCreateStruct);
	void OnSize(UINT nType, int cx, int cy);
	__inline BOOL OnEraseBkgnd(HDC hdc) { return TRUE; }
	void OnDestroy();
	void OnPaint();
	void OnLButtonDown(UINT nFlags, CPoint point);
	void OnRButtonDown(UINT nFlags, CPoint point);
	void OnCancelMode();
	void OnParentNotify(UINT message, LPARAM lParam);
	void OnContextMenu(HWND hwnd, CPoint point);
	void OnSetFocus(HWND hOldWnd);
	int OnMouseActivate(HWND hDesktopWnd, UINT nHitTest, UINT message);
	void OnNotifyDblClk(NMHDR* pNmhdr, LRESULT* pResult);
	void OnNotifyRClick(NMHDR* pNmhdr, LRESULT* pResult);
	void OnNotifyKeyDown(NMHDR* pNmhdr, LRESULT* pResult);
	void OnNotifyBeginLabelEdit(NMHDR* pNmhdr, LRESULT* pResult);
	void OnNotifyEndLabelEdit(NMHDR* pNmhdr, LRESULT* pResult);
	void OnNotifyBeginDrag(NMHDR* pNmhdr, LRESULT* pResult);
	void OnNotifySetFocus(NMHDR* pNmhdr, LRESULT* pResult);
	void OnNotifyItemChanged(NMHDR* pNmhdr, LRESULT* pResult);
	void OnNotifyItemGetInfoTip(NMHDR* pNmhdr, LRESULT* pResult);
	void OnNotifyItemGetDispInfo(NMHDR* pNmhdr, LRESULT* pResult);
	void OnPageExtraIcon();
	void OnPageLargeIcon();
	void OnPageReportSmallIcon();
	void OnPageReportLargeIcon();
	void OnPageReportIcon(bool bSmall);
	void OnPageHideBar();
	void OnPageRename();
	void OnPageDelete();
	void OnPageAdd();
	void OnPageBkgnd();
	void OnPageImport();
	void OnPageSort();
	void OnBarLeft();
	void OnBarRight();
	void OnItemOpen();
	void OnItemDelete();
	void OnItemRename();
	void OnItemIcon();
	void OnItemProperty();
	void OnItemOpenSrcDir();
	void OnInitMenuPopup(HMENU hMenu, UINT nIndex, BOOL bSysMenu);
//	void OnUIPageActive(CCmdUI* pCmdUI);
//	void OnUIPageLeft(CCmdUI* pCmdUI);
//	void OnUIPageRight(CCmdUI* pCmdUI);
//	void OnUIPageLargeIcon(CCmdUI* pCmdUI);
//	void OnUIPageSmallIcon(CCmdUI* pCmdUI);
	LRESULT OnHelpHitTest(WPARAM wParam, LPARAM lParam);
	void OnPageMenu(CPoint pt);

public:
	inline BOOL IsMyMessage(const MSG* pMsg) const { return (pMsg->hwnd == m_list); }
	BOOL ProcessMessage(const MSG* pMsg)
	{
		switch (pMsg->message) {
		case WM_CHAR:
			switch (pMsg->wParam) {
			case VK_RETURN:
				return TRUE;
			}
			break;
		}
		return FALSE;
	}
	void Refresh();

	CAppItem* AddItem(LPCTSTR pFile, int nInsBefore = -1, LPCTSTR pName = NULL, int iSysImageIndex = I_IMAGECALLBACK);
	CAppItem* AddItem(LPCITEMIDLIST pidl, int nInsBefore = -1, LPCTSTR pName = NULL, int iSysImageIndex = I_IMAGECALLBACK);
	void SelectPage(int nPage);
	CAppGroup* GetPage(int index);
	__inline CAppGroup* GetCurrentPage() { return GetPage(m_nCurPage); }
	int GetPageIndexFromPoint(CPoint pt);
	BOOL GetPageRect(int index, CRect& rc);

	void LoadDefaultState();
	BOOL LoadState();
	BOOL SaveState();
	BOOL LoadLegacyAppGroup(HANDLE hf);

	void UpdateProperty();

	// ---------------------------
	// TDropperImple overrides
	// ---------------------------
	DWORD OnDragEnter(IDataObject* pdo, DWORD keyState, DWORD dropEffect, CPoint point);
	DWORD OnDragOver(IDataObject* pdo, DWORD keyState, DWORD dropEffect, CPoint point);
	DWORD OnDrop(IDataObject* pdo, DWORD keyState, DWORD dropEffect, CPoint point);
	void OnDragLeave();

	// ---------------------------
	// CMouseTrackerImple overrides
	// ---------------------------
	void MouseMove(POINT pt);
	void MouseLeave();

private:
	void TrackItemContextMenu(CAppItem* pai, const POINT& pt);

	bool SavePanelGroups();
	int LoadPanelGroups();

	BOOL OnTrackMouse(int nIndex);
	void ResizeList();

	void _drawbutton(HDC hdc, int index, CRect& rc, BOOL bPush);
	void DrawHeader(HDC hdc, int index, bool bPush = false, bool bSelect = false);
	CAppGroup* AddPage(LPCTSTR pszTitle);
	void RenamePage(CAppGroup* pag);

	void OnGroupMoveUp();
	void OnGroupMoveDown();
	void OnItemMoveUp();
	void OnItemMoveDown();
	void OnItemOpenNewTab();
	void OnItemOpenNewWindow();

	CAppItem* GetSelectedItem();

};

#endif

