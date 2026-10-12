// mainfrm_tree.cpp : インプリメンテーション ファイル
//

#include "stdafx.h"
#include "common.h"
#include "mainfrm.h"
#include "drgdrp.h"

#define TIMERID_DELAY				1
#define TIMER_SELDELAY				600

class CFolderItem
{
public:
	CFolderItem() : m_key(0) {};
	~CFolderItem() {};

	DWORD_PTR m_key;
	CString m_path;
	CString m_name;
};


/////////////////////////////////////////////////////////////////////////////
// CMainFrame メッセージ ハンドラ

void CMainFrame::InitializeTreeView()
{
	// イメージリストの作成
	// ルートアイコン
	HIMAGELIST hl, hs;
	Shell_GetImageLists(&hl, &hs);
	m_himTree = ImageList_Duplicate(hs);
	int n = ImageList_GetImageCount(m_himTree);
	while (n-- > 0)
		ImageList_Remove(m_himTree, 0);

	HICON hic = (HICON)LoadImage(RESINST, _MKIR(IDI_NEWARC), IMAGE_ICON, 16, 16, 0);
	ImageList_AddIcon(m_himTree, hic);
	DestroyIcon(hic);

	// フォルダアイコン（通常）
	TCHAR buf[MAX_PATH * 2];
	GetWindowsDirectory(buf, ARRAYSIZE(buf));
	SHFILEINFO sfi;
	SHGetFileInfo(buf, 0, &sfi, sizeof(sfi), SHGFI_ICON | SHGFI_SMALLICON);
	ImageList_AddIcon(m_himTree, sfi.hIcon);
	DestroyIcon(hic);
	m_iImage = 1;

	// フォルダアイコン（オープン状態）
	SHGetFileInfo(buf, 0, &sfi, sizeof(sfi), SHGFI_ICON | SHGFI_OPENICON | SHGFI_SMALLICON);
	ImageList_AddIcon(m_himTree, sfi.hIcon);
	DestroyIcon(hic);
	m_iSelImage = 2;

	m_tree.SetBkColor(gColors[colBack]);
	ImageList_SetBkColor(m_himTree, gColors[colBack]);
	m_tree.SetImageList(m_himTree, TVSIL_NORMAL);
}

void CMainFrame::InitFolder(LPCTSTR pszRoot)
{
	m_tree.DeleteAllItems();
	if (pszRoot) {
		HTREEITEM hItem = m_tree.InsertItem(pszRoot, 0, 0, TVI_ROOT, TVI_ROOT);

		CFolderItem* pt = new CFolderItem();
		pt->m_key = 0;
		m_tree.SetItemData(hItem, (DWORD_PTR)pt);
	}
}

void CMainFrame::InitFolderComplete()
{
	// ルート配下１階層まで開く
	HTREEITEM hRootItem = m_tree.GetRootItem();
	m_tree.Expand(hRootItem, TVE_EXPAND);
	HTREEITEM hItem = m_tree.GetNextItem(hRootItem, TVGN_CHILD);
	while (hItem != NULL) {
		m_tree.Expand(hItem, TVE_EXPAND);
		hItem = m_tree.GetNextItem(hItem, TVGN_NEXT);
	}
	m_tree.SelectItem(hRootItem);
}

void CMainFrame::TreeSettingChange()
{
	ImageList_SetBkColor(m_himTree, gColors[colBack]);
	m_tree.SetBkColor(gColors[colBack]);
	m_tree.Invalidate();
}

HTREEITEM CMainFrame::FindTree(HTREEITEM hStart, LPCTSTR pszDir)
{
	if (m_tree.GetCount() < 1)
		return NULL;

	HTREEITEM hItem = (hStart != NULL) ? hStart : m_tree.GetRootItem();
	do {
		CFolderItem* pt = (CFolderItem*)m_tree.GetItemData(hItem);
		if (!compare_path(pt->m_path, pszDir))
			return hItem;

		if (!ispathchild(pt->m_path, pszDir)) {
			HTREEITEM hChild = m_tree.GetChildItem(hItem);
			if (hChild) {
				hChild = FindTree(hChild, pszDir);
				if (hChild)
					return hChild;
			}
		}
		hItem = m_tree.GetNextSiblingItem(hItem);
	} while (hItem);
	return NULL;
}

HTREEITEM CMainFrame::FindChild(HTREEITEM hParent, LPCTSTR pszDir)
{
	HTREEITEM hItem = m_tree.GetChildItem(hParent);
	while (hItem) {
		CFolderItem* pt = (CFolderItem*)m_tree.GetItemData(hItem);
		if (!compare_path(pt->m_name, pszDir))
			return hItem;
		hItem = m_tree.GetNextSiblingItem(hItem);
	}
	return NULL;
}

static int CALLBACK _comp(LPARAM lParam1, LPARAM lParam2, LPARAM /*lParamSort*/)
{
	CFolderItem* p1 = (CFolderItem*)lParam1;
	CFolderItem* p2 = (CFolderItem*)lParam2;
	return StrCmpLogicalW(p1->m_name, p2->m_name);
}

void CMainFrame::AddFolder(LPCTSTR pszDir, DWORD_PTR key)
{
	CString str = pszDir;
	str.Replace('\\', '/');

	CFolderItem* pt;
	CSimpleArray<CString> arr;
	int argc = split_pathcomponent(str, arr);
	if (argc > 0) {
		HTREEITEM hItem = m_tree.GetRootItem();

		CString tmp;
		for (int i = 0; i < argc; i++) {
			tmp = arr[i];
			if (tmp != _T("/"))
				tmp += '/';

			HTREEITEM hChild = FindTree(hItem, tmp);
			if (!hChild) {
				pt = new CFolderItem();
				pt->m_path = tmp;

				LPCTSTR p = StrRChr(arr[i], NULL, '/');
				if (!p)
					p = arr[i];
				else
					p++;
				pt->m_name = p;

				hChild = m_tree.InsertItem(p, m_iImage, m_iSelImage, hItem, TVI_LAST);
				m_tree.SetItemData(hChild, (DWORD_PTR)pt);

				TVSORTCB tsc;
				tsc.hParent = hItem;
				tsc.lpfnCompare = _comp;
				tsc.lParam = 0;
				m_tree.SortChildrenCB(&tsc);
			}
			hItem = hChild;

			if (i == argc - 1) {
				pt = (CFolderItem*)m_tree.GetItemData(hChild);
				pt->m_key = key;
			}
		}
	}
}

HTREEITEM CMainFrame::SelectFolder(LPCTSTR pszDir)
{
	TCHAR buf[MAX_PATH * 2];
	lstrcpy(buf, pszDir);
	replace_char(buf, '\\', '/');
	add_slash(buf);
	HTREEITEM hItem = FindTree(NULL, buf);
	if (hItem)
		m_tree.SelectItem(hItem);
	return hItem;
}

HTREEITEM CMainFrame::SelectParent()
{
	HTREEITEM hItem = m_tree.GetParentItem(m_tree.GetSelectedItem());
	if (hItem)
		m_tree.SelectItem(hItem);
	return hItem;
}

LPCTSTR CMainFrame::GetCurrentFolderPath()
{
	HTREEITEM hItem = m_tree.GetSelectedItem();
	if (hItem) {
		CFolderItem* pt = (CFolderItem*)m_tree.GetItemData(hItem);
		return pt->m_path;
	}
	return NULL;
}

DWORD_PTR CMainFrame::GetCurrentFolderID()
{
	HTREEITEM hItem = m_tree.GetSelectedItem();
	if (hItem) {
		CFolderItem* pt = (CFolderItem*)m_tree.GetItemData(hItem);
		return pt->m_key;
	}
	return (DWORD_PTR)-1;
}

void CMainFrame::OnTVDeleteItem(NMHDR* pNMHDR, LRESULT* pResult) 
{
	NM_TREEVIEW* pnmtv = (NM_TREEVIEW*)pNMHDR;
	CFolderItem* pt = (CFolderItem*)pnmtv->itemOld.lParam;
	if (pt)
		delete pt;
	*pResult = 0;
}

void CMainFrame::OnTVKeyDown(NMHDR* pNMHDR, LRESULT* pResult) 
{
	TV_KEYDOWN* pkd = (TV_KEYDOWN*)pNMHDR;
	NMKEY nk;
	nk.hdr.hwndFrom = m_hWnd;
	nk.hdr.idFrom = GetDlgCtrlID();
	nk.hdr.code = NM_KEYDOWN;
	nk.nVKey = pkd->wVKey;
	nk.uFlags = pkd->flags;
	if (KeyTranslate(&nk)) {
		*pResult = TRUE;
		return;
	}

	*pResult = TRUE;
	switch (pkd->wVKey) {
		case VK_TAB:
			m_split.SetActivePane(1);
			break;

		case VK_DELETE:					// ファイル削除
			break;

		case VK_APPS:
			break;

		case VK_RETURN:
		{
			HTREEITEM hItem = m_tree.GetSelectedItem();
			if (hItem)
				m_tree.Expand(hItem, TVE_TOGGLE);
		}
		break;

		default:
			*pResult = FALSE;
			break;
	}
}

void CMainFrame::OnTVSelChanged(NMHDR* pNMHDR, LRESULT* pResult) 
{
	*pResult = 0;
	NMTREEVIEW* pnmtv = (NMTREEVIEW*)pNMHDR;
	if (m_timerid) {
		KillTimer(m_timerid);
		m_timerid = 0;
	}

	m_tree.SetItemState(pnmtv->itemOld.hItem, 0, TVIS_BOLD);
	m_tree.SetItemState(pnmtv->itemNew.hItem, TVIS_BOLD, TVIS_BOLD);

	if (pnmtv->action == TVC_BYKEYBOARD)		// キー入力によって選択が変わった
		m_timerid = SetTimer(TIMERID_DELAY, TIMER_SELDELAY, NULL);
	else {
		CFolderItem* pt = (CFolderItem*)m_tree.GetItemData(pnmtv->itemNew.hItem);
		SelectGroup(pt ? pt->m_key : 0xFFFFFFFF);
	}
}

void CMainFrame::OnTimer(UINT_PTR nIDEvent) 
{
	if (nIDEvent == m_timerid) {
		KillTimer(m_timerid);
		m_timerid = 0;

		HTREEITEM hItem = m_tree.GetSelectedItem();
		if (hItem) {
			CFolderItem* pt = (CFolderItem*)m_tree.GetItemData(hItem);
			SelectGroup(pt ? pt->m_key : 0xFFFFFFFF);
		}
	}
}

void CMainFrame::OnTVDeleteItem()
{
	HTREEITEM hItem = m_tree.GetSelectedItem();
	if (hItem && hItem != m_tree.GetRootItem()) {
		CFolderItem* pt = (CFolderItem*)m_tree.GetItemData(hItem);
		DeleteFolder(pt->m_path);
	}
}

void CMainFrame::OnTVCustomDraw(NMHDR* pHdr, LRESULT* pResult)
{
	LPNMTVCUSTOMDRAW ptvcd = (LPNMTVCUSTOMDRAW)pHdr;
	switch (ptvcd->nmcd.dwDrawStage) {
		case CDDS_PREPAINT:
			*pResult = CDRF_NOTIFYITEMDRAW;
			break;

		case CDDS_ITEMPREPAINT:
		{
			HTREEITEM hItem = (HTREEITEM)ptvcd->nmcd.dwItemSpec;
			if (ptvcd->nmcd.uItemState & CDIS_FOCUS ||
				m_tree.GetItemState(hItem, TVIS_DROPHILITED) & TVIS_DROPHILITED) {
				// 対象アイテムがフォーカスを持っているか　あるいは
				// ドロップアイテムか
				ptvcd->clrText = gColors[colRText];
				ptvcd->clrTextBk = gColors[colRBack];
			}
			else if (ptvcd->nmcd.uItemState & CDIS_SELECTED) {
				ptvcd->clrText = gColors[colText];
				ptvcd->clrTextBk = gColors[colInact];
			}
			else {
				ptvcd->clrText = gColors[colText];
				ptvcd->clrTextBk = gColors[colBack];
			}
			*pResult = CDRF_DODEFAULT;
			break;
		}

		default:
			*pResult = 0;
			break;
	}
}

