////////////////////////////////////////////////////////////////////////
// folderview.h

#pragma once

#include "dropper.h"
#include "mousewheeler.h"
#include "view_wrapper.h"

/////////////////////////////////////////////////////////////////////////////
// CFolderTreeView �r���[

class CFolderTreeView : public CWindowImpl<CFolderTreeView>, public TDropperImpl<CFolderTreeView>
{
private:
	CWindow m_owner;
	CMWTreeViewCtrl m_tree;
	DWORD m_dwExpandTimer;
	HTREEITEM m_hDraggedItem;
	UINT_PTR m_timerid;
	HFONT m_hFont;
	CSize m_sizeIcon;
	HIMAGELIST m_himTree;
	HIMAGELIST m_himSystem;
	bool m_bInitializeCompleted:1;
	bool m_bCollapsing:1;


public:
	DECLARE_WND_CLASS_EX(WNDCLASS_FOLDERTREE, 0, COLOR_WINDOW)

	CFolderTreeView()
		: m_hDraggedItem(NULL)
		, m_timerid(0)
		, m_hFont(NULL)
		, m_himTree(NULL)
		, m_dwExpandTimer(0)
		, m_bInitializeCompleted(false)
		, m_bCollapsing(false)
	{
	}
	~CFolderTreeView()
	{
		if (m_hFont) DeleteObject(m_hFont);
		if (m_himTree) ImageList_Destroy(m_himTree);
	}

BEGIN_MSG_MAP(CFolderTreeView)
	CHAIN_MSG_MAP(TDropperImpl<CFolderTreeView>)
	ON_WM_CREATE()
	ON_WM_SIZE()
	ON_WM_DESTROY()
	ON_WM_MOUSEACTIVATE()
	ON_WM_ERASEBKGND()
	ON_WM_TIMER()
	ON_WM_SETFOCUS()
	ON_NOTIFY(TVN_ITEMEXPANDING, IDW_TREE, OnNotifyItemExpanding)
	ON_NOTIFY(TVN_ITEMEXPANDED, IDW_TREE, OnNotifyItemExpanded)
	ON_NOTIFY(TVN_DELETEITEM, IDW_TREE, OnNotifyDeleteItem)
	ON_NOTIFY(TVN_GETDISPINFO, IDW_TREE, OnNotifyGetDispInfo)
	ON_NOTIFY(NM_CUSTOMDRAW, IDW_TREE, OnNotifyCustomDraw)
	ON_NOTIFY(NM_CLICK, IDW_TREE, OnNotifyClick)
	ON_NOTIFY(NM_RCLICK, IDW_TREE, OnNotifyRclick)
	ON_NOTIFY(TVN_SELCHANGED, IDW_TREE, OnNotifySelChanged)
	ON_NOTIFY(TVN_KEYDOWN, IDW_TREE, OnNotifyKeyDown)
	ON_NOTIFY(TVN_BEGINDRAG, IDW_TREE, OnNotifyBeginDrag)
	ON_NOTIFY(TVN_BEGINRDRAG, IDW_TREE, OnNotifyBeginDrag)
	ON_COMMAND(IDM_FILE_COPY, OnFileCopy)
	ON_COMMAND(IDM_FILE_MOVE, OnFileMove)
	ON_COMMAND(IDM_FILE_RENAME, OnFileRename)
	ON_COMMAND(IDM_FILE_DELETE, OnFileDelete)
	ON_COMMAND(IDM_FILE_PROPERTY, OnFileProperty)
	ON_COMMAND(IDM_FILE_ATTRIB, OnFileAttrib)
	ON_COMMAND(IDM_FILE_PACK, OnFilePack)
	ON_COMMAND(IDM_FILE_FILTERDEL, OnFileFilterDelete)
	ON_COMMAND(IDM_FILE_SENDTO, OnFileSendTo)
	ON_COMMAND(IDM_FILE_CONTEXTMENU, OnFileContextMenu)
	ON_COMMAND(IDM_FILE_SHORTCUT, OnFileShortcut)
	ON_COMMAND(IDM_FILE_ASSOCIATE, OnFileAssociate)
	ON_COMMAND(IDM_FILE_EXECUTE, OnFileExecute)
	ON_COMMAND(IDM_EDIT_COPY, OnEditCopy)
	ON_COMMAND(IDM_EDIT_CUT, OnEditCut)
	ON_COMMAND(IDM_COPY_NAMEONLY, OnCopyNameOnly)
	ON_COMMAND(IDM_COPY_FULLNAME, OnCopyFullName)
	ON_COMMAND(IDM_COPY_DETAIL, OnCopyDetails)
	ON_COMMAND(IDM_COPY_SHORTNAME, OnCopyShortName)
	ON_COMMAND(IDM_COPY_UNCNAME, OnCopyUncName)
	ON_COMMAND(IDM_VIEW_FILEINFO, OnViewFileInfo)
	ON_COMMAND(IDM_VIEW_FILESIZE, OnViewFileSize)
	ON_COMMAND(IDM_TAB_NEW, OnTabNew)
	ON_MESSAGE_VOID(TFM_UPDATEVIEW, OnUpdateView)
	ON_MESSAGE(TFM_SELECTPARENT, OnSelectParent)
	ON_MESSAGE(TFM_SELECTCHILD, OnSelectChild)
	ON_MESSAGE(TFM_SELECTFOLDER, OnSelectFolder)
	ON_MESSAGE(TFM_HASPARENT, OnHasParent)
	ON_MESSAGE(TFM_ADDFOLDER, OnAddFolder)
	ON_MESSAGE(TFM_DELETEFOLDER, OnDeleteFolder)
	ON_MESSAGE(TFM_UPDATEFOLDER, OnUpdateFolder)
	ON_MESSAGE(TFM_ENUMFOLDER, OnEnumFolder)
	ON_MESSAGE_VOID(TFM_GETSELECTIONCENTERPOINT, OnGetSelectionCenterPoint)
	ON_MESSAGE_VOID(TFM_REFRESHTREE, OnRefreshTree)
	ON_MESSAGE(TFM_GETCURRENTIDLIST, OnGetCurrentIDList)
	ON_MESSAGE(TFM_GETCURRENTPATH, OnGetCurrentPath)
	ON_MESSAGE(TFM_GETCLASSBODY, OnHandlerGetClassBody)
	ON_MESSAGE(TFM_GETROOT, OnHandlerGetRoot)
	ON_MESSAGE_VOID(TFM_SETROOT, OnHandlerSetRoot)
	ON_MESSAGE(TFM_SETOWNER, OnHandlerSetOwner)
END_MSG_MAP()

	int OnCreate(LPCREATESTRUCT lpCreateStruct);
	void OnSize(UINT nType, int cx, int cy);
	void OnDestroy();
	void OnSetFocus(HWND hOldWnd) { m_tree.SetFocus(); }
	int OnMouseActivate(HWND hDesktopWnd, UINT nHitTest, UINT message);
	BOOL OnEraseBkgnd(HDC hdc);
	void OnTimer(UINT_PTR nIDEvent);
	void OnNotifyItemExpanding(NMHDR* pNMHDR, LRESULT* pResult);
	void OnNotifyItemExpanded(NMHDR* pNMHDR, LRESULT* pResult);
	void OnNotifyDeleteItem(NMHDR* pNMHDR, LRESULT* pResult);
	void OnNotifyGetDispInfo(NMHDR* pNMHDR, LRESULT* pResult);
	void OnNotifyClick(NMHDR* pNMHDR, LRESULT* pResult);
	void OnNotifyRclick(NMHDR* pNMHDR, LRESULT* pResult);
	void OnNotifyBeginDrag(NMHDR* pNMHDR, LRESULT* pResult);
	void OnNotifyCustomDraw(NMHDR* pNMHDR, LRESULT* pResult);
	void OnNotifySelChanged(NMHDR* pNMHDR, LRESULT* pResult);
	void OnNotifyKeyDown(NMHDR* pNMHDR, LRESULT* pResult);
	void OnFileCopy();
	void OnFileMove();
	void OnFileRename();
	void OnFileDelete();
	void OnFileProperty();
	void OnFileAttrib();
	void OnFilePack();
	void OnFileFilterDelete();
	void OnFileSendTo();
	void OnFileContextMenu();
	void OnFileShortcut();
	void OnFileAssociate();
	void OnFileExecute();
	void OnEditCopy();
	void OnEditCut();
	void OnCopyNameOnly();
	void OnCopyFullName();
	void OnCopyShortName();
	void OnCopyDetails();
	void OnCopyUncName();
	void OnViewFileInfo();
	void OnViewFileSize();
	void OnTabNew();
	__inline void OnUpdateView(WPARAM wParam, LPARAM lParam) { UpdateView(wParam, lParam); }
	__inline LRESULT OnSelectParent(WPARAM, LPARAM) { return (LRESULT)SelectParent(); }
	__inline LRESULT OnSelectChild(WPARAM, LPARAM lParam) { return (LRESULT)SelectChild((LPITEMIDLIST)lParam); }
	__inline LRESULT OnSelectFolder(WPARAM, LPARAM lParam)
	{
		CSetRedraw cr(m_tree);
		return (LRESULT)SelectFolder((LPITEMIDLIST)lParam);
	}
	__inline LRESULT OnHasParent(WPARAM, LPARAM lParam) { return (LRESULT)HasParent((HTREEITEM)lParam); }
	__inline LRESULT OnAddFolder(WPARAM, LPARAM lParam) { return (LRESULT)AddFolder((LPITEMIDLIST)lParam); }
	__inline LRESULT OnDeleteFolder(WPARAM, LPARAM lParam) { return (LRESULT)DeleteFolder((LPITEMIDLIST)lParam); }
	__inline LRESULT OnUpdateFolder(WPARAM wParam, LPARAM lParam) { return (LRESULT)UpdateFolder((LPITEMIDLIST)lParam, (BOOL)wParam); }
	__inline LRESULT OnEnumFolder(WPARAM wParam, LPARAM lParam) { return (LRESULT)EnumFolder((LPITEMIDLIST)lParam, (BOOL)wParam); }
	__inline void OnGetSelectionCenterPoint(WPARAM, LPARAM lParam) { GetSelectionCenterPoint((POINT*)lParam); }
	__inline void OnRefreshTree(WPARAM, LPARAM lParam) { RefreshTree((PCIDLIST_ABSOLUTE)lParam); }
	__inline LRESULT OnGetCurrentIDList(WPARAM, LPARAM) { return (LRESULT)GetCurrentIDList(); }
	LRESULT OnGetCurrentPath(WPARAM wParam, LPARAM lParam)
	{
		int len = 0;
		if (lParam && wParam) {
			CFolderItem* pf = GetCurrentFolder();
			CString str;
			if (pf && pf->GetPathName(str)) {
				lstrcpyn((LPTSTR)lParam, str, (int)wParam);
				len = lstrlen((LPTSTR)lParam);
			}
		}
		return (LRESULT)len;
	}
	__inline LRESULT OnHandlerGetClassBody(WPARAM wParam, LPARAM lParam) { return (LRESULT)this; }
	__inline LRESULT OnHandlerGetRoot(WPARAM wParam, LPARAM lParam) { return (LRESULT)GetRoot(); }
	__inline void OnHandlerSetRoot(WPARAM wParam, LPARAM lParam) { SetRoot((LPITEMIDLIST)lParam); }
	__inline LRESULT OnHandlerSetOwner(WPARAM wParam, LPARAM lParam) { return (LRESULT)SetOwner((HWND)lParam); }

	// CCustomDraw overrides
	DWORD OnCustomDrawItemPrePaint(LPNMTVCUSTOMDRAW ptvcd);
/* ----------
	DWORD OnCustomDrawItemPostPaint(LPNMTVCUSTOMDRAW ptvcd);
----------- */


	void SetFolderViewColor();
	bool InitializeTree(PCIDLIST_ABSOLUTE pidlRoot);
	static int CALLBACK CompareFolder(LPARAM lp1, LPARAM lp2, LPARAM lps);
	void SortChildren(HTREEITEM hParentItem);
	int ListupAllItems(HTREEITEM hItem, CSimpleArray<HTREEITEM>& array);
	HTREEITEM GetLastVisibleItem();
	HTREEITEM InsertTreeItem(HTREEITEM hParent, HTREEITEM hInsertAfter, CFolderItem* pf);
	HWND SetOwner(HWND hwndOwner);
	PCIDLIST_ABSOLUTE GetRoot();
	BOOL SetRoot(PCIDLIST_ABSOLUTE pidlRoot);
	BOOL EnumFolder(PCIDLIST_ABSOLUTE pidl, BOOL bNeedUI);
	bool EnumFolderObjects(HTREEITEM hParentItem, IShellFolder* psf, PCIDLIST_ABSOLUTE pidlParent);
	BOOL FindFolder(PCIDLIST_ABSOLUTE pidlSelection, HTREEITEM* pItem, bool bExplore);
	BOOL IsRootDescendant(PCIDLIST_ABSOLUTE pidl);
	BOOL SelectFolder(PCIDLIST_ABSOLUTE pidlSelection);
	HTREEITEM FindIDList(HTREEITEM hParent, PCUIDLIST_RELATIVE pidl);
	void RedrawItem(HTREEITEM hItem);
	BOOL UpdateFileView(HTREEITEM hItem);
	LRESULT NotifyOwner(UINT code);
	PCIDLIST_ABSOLUTE GetCurrentIDList();

	BOOL MakeDragImage(HTREEITEM hItem, SHDRAGIMAGE* image);

	BOOL OnGetDragImage(SHDRAGIMAGE* psdi);

	DWORD OnDragEnter(IDataObject* pdo, DWORD keyState, DWORD dropEffect, POINT point);
	DWORD OnDragOver(IDataObject* pdo, DWORD keyState, DWORD dropEffect, POINT point);
	DWORD OnDrop(IDataObject* pdo, DWORD keyState, DWORD dropEffect, POINT point);
	void OnDragLeave();

protected:
	void UpdateView(WPARAM wHint = 0, LPARAM lHint = 0);
	CFolderItem* GetCurrentFolder();
	BOOL SelectParent();
	BOOL SelectChild(PCUIDLIST_RELATIVE pidl);
	BOOL HasParent(HTREEITEM hItem = NULL);
	BOOL ItemIsParent(HTREEITEM hParent, HTREEITEM hItem);
	BOOL AddFolder(PCIDLIST_ABSOLUTE pidl);
	void DeleteTreeItem(HTREEITEM hItem);
	BOOL DeleteFolder(PCIDLIST_ABSOLUTE pidl);
	BOOL UpdateFolder(PCIDLIST_ABSOLUTE pidl, BOOL bSweepSubFolder);
	void UpdateImage(int iChangeImageIndex);
	void GetSelectionCenterPoint(LPPOINT ppt);
	void ResetTreeStyle();
	void RefreshTree(PCIDLIST_ABSOLUTE pidlRoot = NULL);

	void SaveToXML(LPCTSTR pszXml);
	void SaveXMLNode(HTREEITEM hItem, int indent, TiXmlNode* pNode);
	void DrawItemToBuffer(HTREEITEM hItem, int indent, CString& cs);
	void GetPopupMenuPoint(UINT nIDCommand, LPPOINT lppt);
	void DoCopyMove(BOOL bCopy);
	void ExpandFolderAll(HTREEITEM hItem);
};

