// folderview.cpp : インプリメンテーション ファイル
//

#include "stdafx.h"
#include "fv7.h"
#include "mainfrm.h"
#include "shellexec.h"
#include "execdlg.h"


#define TIMERID_DELAY				1
#define TIMER_SELDELAY				800
#define VK_LINEFEED					0x0A

/////////////////////////////////////////////////////////////////////////////
// CFolderTreeView

void CFolderTreeView::UpdateView(WPARAM wHint, LPARAM lHint)
{
  switch (wHint) {
    case DCM_CHANGEFORMAT:
      SetFolderViewColor();
      ResetTreeStyle();
      break;
    case DCM_UPDATEIMAGE:
      UpdateImage((int)lHint);
      break;
    default:
      break;
  }
}

/////////////////////////////////////////////////////////////////////////////
// CFolderTreeView メッセージ ハンドラ

int CFolderTreeView::OnCreate(LPCREATESTRUCT lpCreateStruct) 
{
#ifdef _DEBUG
    CString tit = APP_PRODUCT;
    tit += _T(" - FolderTree");
    SetWindowText(tit);
#else
    SetWindowText(APP_PRODUCT);
#endif // _DEBUG

    m_owner = lpCreateStruct->hwndParent;

    // TreeCtrl作成
    CRect rc;
    GetClientRect(&rc);

    DWORD style = WS_CHILD | WS_VISIBLE | TVS_HASBUTTONS | TVS_SHOWSELALWAYS;
    if (!(gnListStat & LTS_TREENONELINE))
    style |= TVS_HASLINES;
    m_tree.Create(m_hWnd, &rc, NULL, style, 0, IDW_TREE);
    ATLASSERT(m_tree.IsWindow());

    // ツリー拡張スタイル
    if (gnListStat & LTS_TREENONELINE) {
        DWORD exStyle = TVS_EX_DOUBLEBUFFER | TVS_EX_FADEINOUTEXPANDOS;
        TreeView_SetExtendedStyle(m_tree, exStyle, exStyle);
    }

    // ツリーで利用するイメージリストの作成と設定
    HIMAGELIST dumm;
    Shell_GetImageLists(&dumm, &m_himSystem);

    int cx, cy;
    if (!ImageList_GetIconSize(m_himSystem, &cx, &cy)) {
        cx = cy = 16;
    }
    m_sizeIcon.cx = cx;
    m_sizeIcon.cy = cy;
    m_himTree = ImageList_Create(m_sizeIcon.cx, m_sizeIcon.cy, ILC_COLOR32, 16, 1);
    ImageList_SetBkColor(m_himTree, CLR_NONE);
    m_tree.SetImageList(m_himTree, TVSIL_NORMAL);

    SetFolderViewColor();
    ResetTreeStyle();

    // IMEを無効に
    ImmAssociateContext(m_hWnd, NULL);
    ImmAssociateContext(m_tree, NULL);
    // ツリー初期化
    InitializeTree(NULL);
    return 0;
}

void CFolderTreeView::OnSize(UINT nType, int cx, int cy)
{
  if (nType != SIZE_MINIMIZED) {
    m_tree.SetWindowPos(NULL, 0, 0, cx, cy, SWP_NOACTIVATE | SWP_NOZORDER);
  }
}

BOOL CFolderTreeView::OnEraseBkgnd(HDC hdc)
{
  if (!m_tree.IsWindow()) {
    CRect rc;
    GetClientRect(&rc);
    CDCHandle dc = hdc;
    dc.FillSolidRect(&rc, gColors[colBack]);
  }
  return TRUE;
}

void CFolderTreeView::OnDestroy() 
{
  if (m_timerid)
    KillTimer(m_timerid);
  m_tree.DeleteAllItems();
}

int CFolderTreeView::OnMouseActivate(HWND hDesktopWnd, UINT nHitTest, UINT message)
{
  LRESULT lr = DefWindowProc();
  if (lr != MA_NOACTIVATE && lr != MA_NOACTIVATEANDEAT) {		// frame does not want to activate
    if (nHitTest == HTCLIENT)
      fvApp.SetActiveView(m_hWnd);
  }
  return (int)lr;
}

void CFolderTreeView::SetFolderViewColor()
{
  m_tree.SetBkColor(gColors[colBack]);
  m_tree.SetTextColor(gColors[colLine]);
  m_tree.SetLineColor(gColors[colLine]);

  m_tree.EnableHotTrack((gnMosStat & MOS_HOTTRACK) != 0);
}

void CFolderTreeView::ResetTreeStyle()
{
    CSetRedraw sr(m_tree);
    // フォント
    HFONT hTemp = m_hFont;
    LOGFONT lf;
    if (!LoadFontInfo(&lf, IDS_SEC_SETTINGS, IDS_KEY_TREEFONT)) {
        _GCDATA.GetIconTitleLogFont(&lf);
    }
    m_hFont = CreateFontIndirect(&lf);
    m_tree.SetFont(m_hFont, FALSE);
    if (hTemp) {
        DeleteObject(hTemp);
    }
    m_tree.SetRedraw(TRUE);
    // 高さ
    int h;
    if (gnListStat & LTS_RELAX) {
        h = m_tree.GetItemHeight();
        h += (h / 4);
    } else {
        h = -1;   // 既定の高さ
    }
    m_tree.SetItemHeight(h);

    // ツリー線の表示／非表示
    m_tree.ModifyStyle(TVS_HASLINES, (gnListStat & LTS_TREENONELINE) ? 0 : TVS_HASLINES, 0);
    // ツリー拡張スタイル
    DWORD mask = TVS_EX_DOUBLEBUFFER | TVS_EX_FADEINOUTEXPANDOS;
    DWORD exStyle = (gnListStat & LTS_TREENONELINE) ? mask : 0;
    m_tree.SetExtendedStyle(exStyle, mask);
}

void CFolderTreeView::RefreshTree(PCIDLIST_ABSOLUTE pidlRoot)
{
  CSetRedraw sr(m_tree);

  CIDListPtr idl;
  if (idl.Clone(GetCurrentIDList())) {
/*
    CIDListPtr idlRoot;
    if (gnSysStat & STS_DRIVEROOT) {
      idlRoot.Clone(idl);
      idlRoot.MakeRoot();
    }
*/
    CIDListPtr idlRoot;
    idlRoot.Clone((pidlRoot != NULL)? pidlRoot : GetRoot());
    InitializeTree(idlRoot);
    SelectFolder(idl);
  }
}

bool CFolderTreeView::InitializeTree(PCIDLIST_ABSOLUTE pidlRoot)
{
    if (m_bInitializeCompleted) {
        // 既に初期化済みならばエントリを削除
        m_tree.DeleteAllItems();
    }

    IShellFolder* psf = NULL;
    LPCITEMIDLIST pidlFullyQual = NULL;
    LPCITEMIDLIST pidlSelf = NULL;

    if (pidlRoot && !idl_isDesktop(pidlRoot)) {
        CIDListPtr idl;
        idl.Clone(pidlRoot);
        psf = idl.BindToParent(&pidlSelf);
        if (!psf) {
            return false;
        }
        pidlFullyQual = idl.Detach();
    } else {
        SHGetKnownFolderIDList(FOLDERID_Desktop, 0, NULL, (LPITEMIDLIST*)&pidlFullyQual);
        pidlSelf = pidlFullyQual;
    }

    CFolderItem* pf = new CFolderItem;
    pf->m_pidlFq = pidlFullyQual;
    pf->m_pidl = pidlSelf;
    pf->m_psf = psf;
    pf->GetAttributes();
    HTREEITEM hItem = InsertTreeItem(NULL, TVI_LAST, pf);
    if (m_bInitializeCompleted) {
       m_tree.SelectItem(hItem);
    }
    m_tree.Expand(hItem, TVE_EXPAND);

    m_bInitializeCompleted = true;
    return true;
}

HTREEITEM CFolderTreeView::InsertTreeItem(HTREEITEM hParent, HTREEITEM hInsertAfter, CFolderItem* pf)
{
  TVITEM item = {0};
  item.mask = TVIF_PARAM | TVIF_TEXT | TVIF_IMAGE | TVIF_SELECTEDIMAGE | TVIF_CHILDREN;
  item.pszText = LPSTR_TEXTCALLBACK;
  item.iImage = item.iSelectedImage = I_IMAGECALLBACK;
  item.lParam= (LPARAM)pf;
  if (pf->isDesktop()) {
    item.cChildren = 1;
  } else {
    // サブフォルダの存在チェック
    SFGAOF attr = pf->GetAttributes();
    item.cChildren = I_CHILDRENCALLBACK;

    // 属性の取得
    WIN32_FIND_DATA wfd;
    if (SHGetDataFromIDList(pf->m_psf, pf->m_pidl, SHGDFIL_FINDDATA, (void*)&wfd, sizeof(wfd)) == S_OK) {
      pf->m_fileattr = wfd.dwFileAttributes;
    } else {
      pf->m_fileattr = 0;
      if (attr & SFGAO_HIDDEN) pf->m_fileattr |= FILE_ATTRIBUTE_HIDDEN;
      if (attr & SFGAO_SYSTEM) pf->m_fileattr |= FILE_ATTRIBUTE_SYSTEM;
    }
  }

  TVINSERTSTRUCT ins = {0};
  ins.item = item;
  ins.hInsertAfter = hInsertAfter;
  ins.hParent = hParent;
  HTREEITEM hItem = m_tree.InsertItem(&ins);

  item.hItem = hParent;
  item.mask = TVIF_CHILDREN;
  item.cChildren = 1;
  m_tree.SetItem(&item);

  return hItem;
}

HWND CFolderTreeView::SetOwner(HWND hwndOwner)
{
  HWND hwndPrevOwner = m_owner;
  m_owner = hwndOwner;
  return hwndPrevOwner;
}

PCIDLIST_ABSOLUTE CFolderTreeView::GetRoot()
{
  CFolderItem* pf = (CFolderItem*)m_tree.GetItemData(m_tree.GetRootItem());
  return (pf != NULL) ? pf->m_pidlFq : NULL;
}

BOOL CFolderTreeView::SetRoot(PCIDLIST_ABSOLUTE pidlRoot)
{
  CFolderItem* pf = (CFolderItem*)m_tree.GetItemData(m_tree.GetRootItem());
  if (pf && pf->isEqual(pidlRoot))
    return TRUE;
  return InitializeTree(pidlRoot);
}

BOOL CFolderTreeView::EnumFolder(PCIDLIST_ABSOLUTE pidl, BOOL bNeedUI)
{
  CSetRedraw sr(m_tree);

  HTREEITEM hItem;
  if (FindFolder(pidl, &hItem, bNeedUI != FALSE)) {
    CFolderItem* pf = (CFolderItem*)m_tree.GetItemData(hItem);
    CComPtr<IShellFolder> psf;
    if (pf->BindToShellFolder(&psf) == S_OK) {
      EnumFolderObjects(hItem, psf, pf->m_pidlFq);
      return TRUE;
    }
  }
  return FALSE;
}

bool CFolderTreeView::EnumFolderObjects(HTREEITEM hParentItem, IShellFolder* psf, PCIDLIST_ABSOLUTE pidlParent)
{
  CFolderItem* pf;
  CComPtr<IEnumIDList> pe;
//@ SHCONTF_FASTITEMSを指定するとftpサイトのデータが取得できない
//@	HRESULT hr = psf->EnumObjects(NULL, SHCONTF_FOLDERS | SHCONTF_INCLUDEHIDDEN | SHCONTF_INCLUDESUPERHIDDEN | SHCONTF_FASTITEMS, &pe);
  if (psf->EnumObjects(NULL, SHCONTF_FOLDERS | SHCONTF_INCLUDEHIDDEN | SHCONTF_INCLUDESUPERHIDDEN, &pe) == S_OK) {
    // このフォルダに既にリスト済みのものをチェック
    CSimpleArray<PCIDLIST_ABSOLUTE> stack;
    HTREEITEM hItem = m_tree.GetChildItem(hParentItem);
    while (hItem) {
      pf = (CFolderItem*)m_tree.GetItemData(hItem);
      HTREEITEM hItemNext = m_tree.GetNextSiblingItem(hItem);

      // 実在するかチェック
      SFGAOF attr = SFGAO_VALIDATE;
      if (FAILED(psf->GetAttributesOf(1, &pf->m_pidl, &attr)))
        m_tree.DeleteItem(hItem);
      else
        stack.Add(pf->m_pidlFq);
      hItem = hItemNext;
    }

    int i, cnt, numAdd = 0;
    LPITEMIDLIST pidl;
    DWORD celt;
    while (SUCCEEDED(pe->Next(1, &pidl, &celt)) && celt) {
        ATLASSERT(pidl != NULL);			// 何故かコントロールパネルのツリーを開くときに起きることがある
        if (!pidl) break;

        pf = new CFolderItem(psf, pidlParent, pidl);
        CoTaskMemFree(pidl);

        // 属性を取得
        SFGAOF attr = pf->GetAttributes();
        if (!(attr & SFGAO_FOLDER) ||
            ((gnFileStat & FTS_HIDDEN) && (attr & SFGAO_HIDDEN)) ||
            ((gnFileStat & FTS_SUPERHIDDEN) && (attr & SFGAO_HIDDEN) && (attr & SFGAO_SYSTEM))) {
            // フォルダ以外のもの、隠し属性を除外
            delete pf;
            continue;
        }
        if ((gnListStat & LTS_NOARCTREE) && (attr & (SFGAO_STREAM | SFGAO_LINK)) == SFGAO_STREAM) {
            // 書庫?
            delete pf;
            continue;
        }

        // 既存のものと一致するか検証
        cnt = stack.GetSize();
        for (i = 0; i < cnt; i++) {
            if (::ILIsEqual(pf->m_pidlFq, stack[i])) {
                // 一致したものはリストから除外する
                stack.RemoveAt(i);
                delete pf;
                pf = NULL;
                break;
            }
        }
        if (pf) {
            InsertTreeItem(hParentItem, TVI_LAST, pf);
            numAdd++;
        }
    }

    if (numAdd > 0) {
      SortChildren(hParentItem);
    } else if (m_tree.GetChildItem(hParentItem) == NULL) {
      // サブフォルダが無いので[+]マークを除去
      TVITEM tvi;
      tvi.mask = TVIF_CHILDREN;
      tvi.hItem = hParentItem;
      m_tree.GetItem(&tvi);
      if (tvi.cChildren) {
        tvi.cChildren = 0;
        m_tree.SetItem(&tvi);
      }
    }
    return true;
  }
  return false;
}

BOOL CFolderTreeView::FindFolder(PCIDLIST_ABSOLUTE pidlSelection, HTREEITEM* pItem, bool bExplore)
{
  if (pItem)
    *pItem = NULL;

  // Move pidlCurrent behind the SHITEMIDs in pidlSelection, which are the root of
  // the sub-tree currently displayed.
  HTREEITEM hItem = m_tree.GetRootItem();
  if (!hItem)
    return FALSE;

  // ルートと一致
  CFolderItem* pf = (CFolderItem*)m_tree.GetItemData(hItem);
  if (pf->isEqual(pidlSelection)) {
    if (pItem)
      *pItem = hItem;
    return TRUE;
  }

  if (!pf->isParent(pidlSelection, FALSE)) {
    // ルートとは無関係
    return FALSE;
  }

  // ルート配下のサブフォルダをチェック
  HTREEITEM hParent = hItem;
  hItem = m_tree.GetChildItem(hParent);
  while (hItem) {
    pf = (CFolderItem*)m_tree.GetItemData(hItem);
    if (pf->isEqual(pidlSelection)) {
      // フォルダ一致
      if (pItem)
        *pItem = hItem;
      return TRUE;
    }
    if (pf->isParent(pidlSelection, FALSE)) {
      // pidlSelection は このフォルダの子孫
      if (bExplore)
        m_tree.Expand(hItem, TVE_EXPAND);
      hParent = hItem;
      hItem = m_tree.GetChildItem(hParent);
    }
    else {
      // 兄弟を検索
      hItem = m_tree.GetNextSiblingItem(hItem);
    }
  }

  if (bExplore) {
    // hParent はトレース最後の親を指している
    pf = (CFolderItem*)m_tree.GetItemData(hParent);
    // 配下に強制追加する
    CSimpleArray<LPITEMIDLIST> arr;
    // 親へさかのぼって階層を列挙
    LPITEMIDLIST pidl = ILClone(pidlSelection);
    while (pidl && !pf->isEqual(pidl)) {
      arr.Add(pidl);
      pidl = idl_GetParent(pidl);
    }
    if (pidl)
      CoTaskMemFree(pidl);
    // 親から順に追加
    int cnt = arr.GetSize();
    if (cnt > 0) {
      for (int i = 0; i < cnt; i++) {
        pidl = arr[cnt - i - 1];
        pf = new CFolderItem(pidl);
        CoTaskMemFree(pidl);
        if (!pf->isValid()) {
          delete pf;
          break;
        }
        hItem = InsertTreeItem(hParent, TVI_LAST, pf);
        hParent = hItem;
      }
      if (pItem)
        *pItem = hItem;
      return TRUE;
    }
  }
  return FALSE;
}



BOOL CFolderTreeView::IsRootDescendant(PCIDLIST_ABSOLUTE pidl)
{
  // ルートの子孫かどうかをチェック
  BOOL bDescendant = FALSE;
  if (pidl) {
    CFolderItem* pf = (CFolderItem*)m_tree.GetItemData(m_tree.GetRootItem());
    if (pf)
      bDescendant = (pf->isParent(pidl, FALSE) || pf->isEqual(pidl));
  }
  return bDescendant;
}

BOOL CFolderTreeView::SelectFolder(PCIDLIST_ABSOLUTE pidlTarget)
{
  BOOL bOK = FALSE;
  CIDListPtrHandle idlTarget = pidlTarget;
  if (!idlTarget.isNull() && IsRootDescendant(idlTarget)) {
    HTREEITEM hItem = NULL;
    bOK = FindFolder(idlTarget, &hItem, true);
    if (bOK) {
      m_tree.SelectItem(hItem);
      m_tree.EnsureVisible(hItem);
    }
  }
  return bOK;
}

int CALLBACK CFolderTreeView::CompareFolder(LPARAM lp1, LPARAM lp2, LPARAM lps)
{
  CFolderItem* p1 = (CFolderItem*)lp1;
  CFolderItem* p2 = (CFolderItem*)lp2;
  HRESULT hr = p1->CompareIDs(p2->m_pidl);
  return (short)SCODE_CODE(GetScode(hr));
}

void CFolderTreeView::SortChildren(HTREEITEM hParentItem)
{
  TV_SORTCB tvscb;
  tvscb.hParent = hParentItem;
  tvscb.lpfnCompare = CompareFolder;
  tvscb.lParam = 0;
  m_tree.SortChildrenCB(&tvscb);
}
int CFolderTreeView::ListupAllItems(HTREEITEM hItem, CSimpleArray<HTREEITEM>& array)
{
  int cnt = 0;
  if (!hItem) hItem = m_tree.GetRootItem();

  do {
    array.Add(hItem);
    cnt++;
    HTREEITEM hChild = m_tree.GetChildItem(hItem);
    if (hChild)
      cnt += ListupAllItems(hChild, array);
    hItem = m_tree.GetNextSiblingItem(hItem);
  } while (hItem);
  return cnt;
}

void CFolderTreeView::UpdateImage(int iChangeImageIndex)
{
  CSimpleArray<HTREEITEM> array;
  ListupAllItems(NULL, array);
  int cnt = array.GetSize();
  for (int i = 0; i < cnt; i++) {
    HTREEITEM hItem = array[i];
    CFolderItem* pf = (CFolderItem*)m_tree.GetItemData(hItem);
    if (pf->GetImageIndex() == iChangeImageIndex) {
      // 次回に再度取得する
      pf->ResetImageIndex();
      m_tree.SetItemImage(hItem, I_IMAGECALLBACK, I_IMAGECALLBACK);
    }
  }
  array.RemoveAll();
}

void CFolderTreeView::RedrawItem(HTREEITEM hItem)
{
  CRect rc;
  if (hItem && m_tree.GetItemRect(hItem, &rc, FALSE))
    RedrawWindow(&rc, NULL, RDW_INVALIDATE | RDW_UPDATENOW);
}

HTREEITEM CFolderTreeView::GetLastVisibleItem()
{
  HTREEITEM hItem = m_tree.GetFirstVisibleItem();
  if (hItem) {
    CRect rc;
    m_tree.GetItemRect(hItem, &rc, FALSE);
    int dy = rc.Height();
    m_tree.GetClientRect(&rc);

    TV_HITTESTINFO tvhti;
    tvhti.pt.x = rc.Width() / 2;
    tvhti.pt.y = rc.bottom - dy;

    hItem = m_tree.HitTest(&tvhti);
  }
  return hItem;
}

BOOL CFolderTreeView::ItemIsParent(HTREEITEM hParent, HTREEITEM hItem)
{
  hItem = m_tree.GetParentItem(hItem);
  while (hItem) {
    if (hItem == hParent)
      return TRUE;
    hItem = m_tree.GetParentItem(hItem);
  }
  return FALSE;
}

BOOL CFolderTreeView::UpdateFolder(PCIDLIST_ABSOLUTE pidl, BOOL bSweepSubFolder)
{
  HTREEITEM hItem = m_tree.GetRootItem();
  CFolderItem* pf = (CFolderItem*)m_tree.GetItemData(hItem);
  if (!pf->IsDesktop()) {
    if (!pf->isEqual(pidl)) {
      // 配下のフォルダじゃなければ何もしない
      if (!pf->isParent(pidl, FALSE))
        return FALSE;
      hItem = NULL;
    }
  }
  else
    hItem = NULL;

  if (hItem || FindFolder(pidl, &hItem, false)) {
    pf = (CFolderItem*)m_tree.GetItemData(hItem);
    pf->ValidateCacheFolderInformation();

    pf->m_attr = 0;
    pf->GetAttributes();
    // ファイル属性の取得
    WIN32_FIND_DATA wfd;
    if (SHGetDataFromIDList(pf->m_psf, pf->m_pidl, SHGDFIL_FINDDATA, (void*)&wfd, sizeof(wfd)) == NOERROR) {
      pf->m_fileattr = wfd.dwFileAttributes;
    }
    // イメージリセット
    pf->ResetImageIndex();
    // イメージ、表示名、サブフォルダの有無を再取得させる
    TVITEM item = {0};
    item.mask = TVIF_TEXT | TVIF_IMAGE | TVIF_SELECTEDIMAGE | TVIF_CHILDREN;
    item.hItem = hItem;
    item.pszText = LPSTR_TEXTCALLBACK;
    item.iImage = item.iSelectedImage = I_IMAGECALLBACK;
    item.cChildren = I_CHILDRENCALLBACK;
    bool bSelItemDeleted = false;
    if (bSweepSubFolder) {
      // 現在選択中の項目
      HTREEITEM hSelItem = m_tree.GetSelectedItem();
      // 配下のツリーを削除
      HTREEITEM hChild = m_tree.GetChildItem(hItem);
      while (hChild != NULL) {
        HTREEITEM hTemp = m_tree.GetNextSiblingItem(hChild);
        if (!bSelItemDeleted && (hChild == hSelItem || ItemIsParent(hChild, hSelItem)))
          bSelItemDeleted;
        m_tree.DeleteItem(hChild);
        hChild = hTemp;
      }
      item.cChildren = 0;
    }
    m_tree.SetItem(&item);

    if (bSelItemDeleted) {
      m_tree.SelectItem(hItem);
      NotifyOwner(TFN_SELCHANGED);
    }
    return TRUE;
  }
  return FALSE;
}

BOOL CFolderTreeView::AddFolder(PCIDLIST_ABSOLUTE pidl)
{
  BOOL bOK = FALSE;
  CIDListPtr idlParent = idl_GetParent(pidl);
  if (idlParent.isNull())
    return FALSE;

  CSetRedraw sr(m_tree);

  // 挿入するフォルダの親フォルダを検索
  HTREEITEM hParent;
  if (FindFolder(idlParent, &hParent, false)) {
    CFolderItem* pf = (CFolderItem*)m_tree.GetItemData(hParent);

    // 過去に展開済みでなければフラグ操作のみ
    if (!m_tree.GetItemState(hParent, TVIS_EXPANDEDONCE)) {
      TVITEM tvi;
      tvi.hItem = hParent;
      tvi.mask = TVIF_CHILDREN;
      tvi.cChildren = 1;
      m_tree.SetItem(&tvi);
    }
    else {
      // 展開済みなので再リストアップ
      CComPtr<IShellFolder> psf;
      if (pf->BindToShellFolder(&psf) == S_OK) {
        EnumFolderObjects(hParent, psf, pf->m_pidlFq);
      }
    }
    bOK = TRUE;
  }
  return bOK;
}

void CFolderTreeView::DeleteTreeItem(HTREEITEM hItem)
{
  HTREEITEM hSelItem = m_tree.GetSelectedItem();
  // 現在選択中の項目を削除するか、選択項目の親を削除するのであれば
  // 削除後に選択項目を変更する
  if ((hItem == hSelItem) || ItemIsParent(hItem, hSelItem))
    hSelItem = m_tree.GetParentItem(hItem);
  else
    hSelItem = NULL;

  // 削除の実行
  m_tree.DeleteItem(hItem);

  if (hSelItem) {
    if (!m_tree.GetChildItem(hSelItem)) {
      // 配下にサブフォルダが無くなれば、[+]記号を消去
      TVITEM tvi;
      tvi.hItem = hSelItem;
      tvi.mask = TVIF_CHILDREN;
      tvi.cChildren = 0;
      m_tree.SetItem(&tvi);
    }
    m_tree.SelectItem(hSelItem);
    NotifyOwner(TFN_SELCHANGED);
  }
}

BOOL CFolderTreeView::DeleteFolder(PCIDLIST_ABSOLUTE pidl)
{
  HTREEITEM hItem = NULL;
  CFolderItem* pf = (CFolderItem*)m_tree.GetItemData(m_tree.GetRootItem());
  if (pf->isEqual(pidl)) {
    // ルートは削除できませんがな
    return FALSE;
  }
  if (!pf->isParent(pidl, FALSE)) {
    // 配下のフォルダじゃなければ何もしない
    return FALSE;
  }
  // ツリーの中に見つかれば削除
  if (FindFolder(pidl, &hItem, false)) {
    DeleteTreeItem(hItem);
    return TRUE;
  }
  return FALSE;
}

CFolderItem* CFolderTreeView::GetCurrentFolder()
{
  HTREEITEM hItem = m_tree.GetSelectedItem();
  if (hItem)
    return (CFolderItem*)m_tree.GetItemData(hItem);
  return NULL;
}

BOOL CFolderTreeView::HasParent(HTREEITEM hItem)
{
  if (hItem == NULL) hItem = m_tree.GetSelectedItem();
  return (hItem != NULL) ? (m_tree.GetParentItem(hItem) != NULL) : FALSE;
}

BOOL CFolderTreeView::SelectParent()
{
  HTREEITEM hItem = m_tree.GetParentItem(m_tree.GetSelectedItem());
  return (hItem != NULL) ? m_tree.SelectItem(hItem) : FALSE;
}

HTREEITEM CFolderTreeView::FindIDList(HTREEITEM hParent, PCUIDLIST_RELATIVE pidl)
{
  HTREEITEM hItem = m_tree.GetChildItem(hParent);
  while (hItem) {
    CFolderItem* pf = (CFolderItem*)m_tree.GetItemData(hItem);
    HRESULT hr = pf->CompareIDs(pidl);
    if ((short)SCODE_CODE(GetScode(hr)) == 0)
      break;
    hItem = m_tree.GetNextSiblingItem(hItem);
  }
  return hItem;
}

BOOL CFolderTreeView::SelectChild(PCUIDLIST_RELATIVE pidlSingle)
{
  HTREEITEM hItem = m_tree.GetSelectedItem();
  m_tree.Expand(hItem, TVE_EXPAND);

  BOOL bOK = FALSE;
  HTREEITEM hChild = FindIDList(hItem, pidlSingle);
  if (!hChild) {
    CFolderItem* pf = (CFolderItem*)m_tree.GetItemData(hItem);
    ATLASSERT(pf != NULL);
    CComPtr<IShellFolder> psf;
    if (pf && SUCCEEDED(pf->BindToShellFolder(&psf))) {
      ULONG ula = SFGAO_FOLDER | SFGAO_CAPABILITYMASK;
      if (psf->GetAttributesOf(1, (LPCITEMIDLIST*)&pidlSingle, &ula) == S_OK) {
        // 新規に追加
        CFolderItem* pNewItemInfo = new CFolderItem(psf, pf->m_pidlFq, pidlSingle);
        pNewItemInfo->m_psf = psf.Detach();
        pNewItemInfo->m_pidlFq = ILCombine(pf->m_pidlFq, pidlSingle);
        pNewItemInfo->m_pidl = ILFindLastID(pNewItemInfo->m_pidlFq);
        hChild = InsertTreeItem(hItem, TVI_LAST, pNewItemInfo);
        SortChildren(hItem);

        TVITEM tvi;
        tvi.mask = TVIF_CHILDREN;
        tvi.hItem = hItem;
        tvi.cChildren = 1;
        m_tree.SetItem(&tvi);
        m_tree.Expand(hItem, TVE_EXPAND);
        bOK = TRUE;
      }
    }
  }
  if (hChild)
    m_tree.SelectItem(hChild);
  return bOK;
}

PCIDLIST_ABSOLUTE CFolderTreeView::GetCurrentIDList()
{
  CFolderItem* pf = GetCurrentFolder();
  return (pf ? pf->m_pidlFq : NULL);
}

void CFolderTreeView::OnNotifyItemExpanding(NMHDR* pNMHDR, LRESULT* pResult) 
{
  NMTREEVIEW* pnmtv = (NMTREEVIEW*)pNMHDR;
  if (pnmtv->action == TVE_EXPAND) {
    // まだ開かれたことがなければフォルダをトレース
    UINT state = m_tree.GetItemState(pnmtv->itemNew.hItem, TVIS_EXPANDEDONCE);
    if (!(state & TVIS_EXPANDEDONCE)) {
      CFolderItem* pf = (CFolderItem*)m_tree.GetItemData(pnmtv->itemNew.hItem);
      if (pf) {
        CWaitCursor wc;
        CComPtr<IShellFolder> psf;
        if (pf->BindToShellFolder(&psf) == S_OK) {
          EnumFolderObjects(pnmtv->itemNew.hItem, psf, pf->m_pidlFq);
        }
      }
    }
  }
  else if (pnmtv->action == TVE_COLLAPSE) {
    m_bCollapsing = true;
  }
}

void CFolderTreeView::OnNotifyItemExpanded(NMHDR* pNMHDR, LRESULT* pResult) 
{
  NMTREEVIEW* pnmtv = (NMTREEVIEW*)pNMHDR;
  if (pnmtv->action == TVE_COLLAPSE) {
    m_bCollapsing = false;
  }
}

void CFolderTreeView::OnNotifyDeleteItem(NMHDR* pNMHDR, LRESULT* pResult) 
{
  NM_TREEVIEW* pnmtv = (NM_TREEVIEW*)pNMHDR;
  CFolderItem* pf = (CFolderItem*)pnmtv->itemOld.lParam;
  if (pf) delete pf;

  *pResult = 0;
}

void CFolderTreeView::OnNotifyGetDispInfo(NMHDR* pNMHDR, LRESULT* pResult) 
{
  TV_DISPINFO* ptvdi  = (TV_DISPINFO*)pNMHDR;
  if (ptvdi) {
    CFolderItem* pf = (CFolderItem*)ptvdi->item.lParam;
    if (pf) {
      if (ptvdi->item.mask & TVIF_TEXT) {
        // 表示項目がルートアイテムであれば pidlFqを使って表示名を取得する
        // （\\Server\Share などは 「Share - Server」形式の名前が取得できるため）
        CString str;
        bool bUseFullQualPidl = (m_tree.GetRootItem() == ptvdi->item.hItem);
        pf->GetDisplayName(str, bUseFullQualPidl);
        if (gnListStat & LTS_LOWERCASE)
          str.MakeLower();
//@str.AppendFormat(L"(%08X)", pf->GetAttributes());
        StrCpyN(ptvdi->item.pszText, str, ptvdi->item.cchTextMax);
        ptvdi->item.mask |= TVIF_DI_SETITEM;
      }
      if (ptvdi->item.mask & TVIF_IMAGE) {
        // Sysytem Imagelist からアイコンを取得して、自分のイメージリストにコピー
        CIcon ic = ImageList_GetIcon(m_himSystem, pf->GetImageIndex(), ILD_NORMAL | pf->OverlayImageMask());
        ptvdi->item.iImage = ImageList_ReplaceIcon(m_himTree, -1, ic);
        ptvdi->item.mask |= TVIF_DI_SETITEM;
      }
      if (ptvdi->item.mask & TVIF_SELECTEDIMAGE) {
        // Sysytem Imagelist からアイコンを取得して、自分のイメージリストにコピー
        CIcon ic = ImageList_GetIcon(m_himSystem, pf->GetSelectedImageIndex(), ILD_NORMAL | pf->OverlayImageMask());
        ptvdi->item.iSelectedImage = ImageList_ReplaceIcon(m_himTree, -1, ic);
        ptvdi->item.mask |= TVIF_DI_SETITEM;
      }
      if (ptvdi->item.mask & TVIF_CHILDREN) {
        ptvdi->item.cChildren = pf->hasSubFolder() ? 1 : 0;
        ptvdi->item.mask |= TVIF_DI_SETITEM;
      }
    }
  }
  *pResult = 0;
}

DWORD CFolderTreeView::OnDragEnter(IDataObject* pdo, DWORD keyState, DWORD dropEffect, POINT point)
{
  SetDragScrollTimer(80);
  m_dwExpandTimer = 0;
  return OnDragOver(pdo, keyState, dropEffect, point);
}

DWORD CFolderTreeView::OnDragOver(IDataObject* pdo, DWORD keyState, DWORD dropEffect, POINT point)
{
  ScreenToClient(&point);

  TV_HITTESTINFO tvhti;
  tvhti.pt = point;
  tvhti.flags = TVHT_ONITEM;
  HTREEITEM hItem = m_tree.HitTest(&tvhti);
  HTREEITEM hDropItem = m_tree.GetDropHilightItem();

  CFolderItem* pf;

  // 階層下を開く処理
  if (hItem &&
    m_tree.ItemHasChildren(hItem) &&
    (m_tree.GetItemState(hItem, TVIS_EXPANDED) & TVIS_EXPANDED) == 0)
  {
    if (hDropItem != hItem)
      m_dwExpandTimer = GetTickCount() + 1400;
    else if (m_dwExpandTimer != 0 && m_dwExpandTimer < GetTickCount()) {
      ShowDropHelper(FALSE);
      m_tree.Expand(hItem, TVE_EXPAND);
    }
  }

  if (hItem != hDropItem) {
    ShowDropHelper(FALSE);
    m_tree.SelectDropTarget(hItem);
  }

  // スクロール
  HTREEITEM hLast = GetLastVisibleItem();
  if (hLast && m_tree.GetNextVisibleItem(hLast) == NULL)
    hLast = NULL;
  HTREEITEM hFirst = m_tree.GetFirstVisibleItem();
  if (m_tree.GetRootItem() == hFirst)
    hFirst = NULL;
  if (IsTimeToDragScroll() && (hItem == hFirst || hItem == hLast)) {
    ShowDropHelper(FALSE);
    m_tree.SendMessage(WM_VSCROLL, (hFirst == hItem) ? SB_LINEUP : SB_LINEDOWN, 0);
  }

  // VISTAのエアロD&Dテキスト
  CString strTargetDisplayName;
  DROPEFFECT eff = DROPEFFECT_NONE;
  if (hItem) {
    m_tree.GetItemText(hItem, strTargetDisplayName);
    if (m_hDraggedItem != hItem) {
      pf = (CFolderItem*)m_tree.GetItemData(hItem);
      if (pf) {
        eff = GetDropEffect(dropEffect, keyState);
        if (hItem != m_tree.GetRootItem()) {
          // デスクトップ以外
          DROPEFFECT e = idl_GetDropEffect(m_hWnd, pf->m_psf, pf->m_pidl, pdo, keyState, dropEffect);
          if (IsDropFormatsSupported(SDO_HDROP) && !e && pf->m_fileattr)
            ;	// 何もせず
          else
            eff = e;
        }
      }
    }
  }
  ShowDropHelper(TRUE);

  SetDropTargetDescription(pdo, strTargetDisplayName, eff);

  return eff;
}

void CFolderTreeView::OnDragLeave()
{
  // ドロップ待機状態になっているパスを元に戻す
  m_tree.SelectDropTarget(NULL);
}

DWORD CFolderTreeView::OnDrop(IDataObject* pdo, DWORD keyState, DWORD dropEffect, POINT point)
{
  DROPEFFECT eff = DROPEFFECT_NONE;
  HTREEITEM hDropItem = m_tree.GetDropHilightItem();
  if (hDropItem) {
    CFolderItem* pf = (CFolderItem*)m_tree.GetItemData(hDropItem);
    if (pf) {
      CString dir;
      if ((IsDropFormatsSupported(SDO_HDROP|SDO_FILENAMEMAPA|SDO_FILENAMEMAPW) == SDO_HDROP) &&
        pf->GetPathName(dir) &&
        (keyState & MK_LBUTTON)) {
        HDROP hdrop = (HDROP)GetDataFromDataObject(pdo, CF_HDROP);
        if (hdrop) {
          BOOL bThreaded = FALSE;
          BOOL bAsAdmin = FALSE;
          BOOL bForceOverwrite = FALSE;
          eff = dropEffect;
          CString str;
          if (QueryDrop(m_hWnd, pf->GetDisplayName(str), eff, bThreaded, bAsAdmin, bForceOverwrite)) {
            AcceptFiles(hdrop, eff, dir, bThreaded, bAsAdmin, bForceOverwrite);
          }
          DragFinish(hdrop);
          return eff;
        }
      }

      eff = AcceptDataObject(m_hWnd, pf->m_pidlFq, pdo, keyState, dropEffect, point);

      CComPtr<IShellFolder> psf;
      if (pf->BindToShellFolder(&psf) == S_OK) {
        EnumFolderObjects(hDropItem, psf, pf->m_pidlFq);
      }
    }
  }
  return eff;
}

void CFolderTreeView::OnNotifyRclick(NMHDR* /*pNMHDR*/, LRESULT* pResult) 
{
  *pResult = TRUE;

  CPoint point(GetMessagePos());

  TV_HITTESTINFO tvhti;
  tvhti.pt = point;
  m_tree.ScreenToClient(&tvhti.pt);
  HTREEITEM hItem = m_tree.HitTest(&tvhti);
  if (hItem && (tvhti.flags & TVHT_ONITEM) != 0) {
    // アイテム本体がクリックされた
    if ((gnMosStat & MOS_RBTN_MASK) == MOS_RBTN_CONTEXTMENU) {
      CFolderItem* pf = (CFolderItem*)m_tree.GetItemData(hItem);
      pf->ContextMenu(m_hWnd, &point);
      return;
    }
    HTREEITEM hSel = m_tree.GetSelectedItem();
    if (hSel != hItem)
      m_tree.SelectItem(hItem);
  }

  gpMainFrame->TrackMenuForFolderTree(point);
}

void CFolderTreeView::OnNotifyClick(NMHDR* /*pNMHDR*/, LRESULT* pResult) 
{
  *pResult = FALSE;

  CPoint point(GetMessagePos());

  TV_HITTESTINFO tvhti;
  tvhti.pt = point;
  m_tree.ScreenToClient(&tvhti.pt);
  HTREEITEM hItem = m_tree.HitTest(&tvhti);
  if (hItem && (tvhti.flags & TVHT_ONITEM) != 0) {
    // アイテム本体がクリックされた
    if (m_tree.GetSelectedItem() == hItem) {
      // 選択中のアイテムとクリックされたアイテムが同一本体がクリックされた
      CFolderItem* pf = (CFolderItem*)m_tree.GetItemData(hItem);
      int descid = pf->GetDescriptionId();
      switch (descid) {
      case SHDID_COMPUTER_DRIVE35:
      case SHDID_COMPUTER_DRIVE525:
      case SHDID_COMPUTER_REMOVABLE:
      case SHDID_COMPUTER_NETDRIVE:
      case SHDID_COMPUTER_CDROM:
      case SHDID_COMPUTER_RAMDISK:
      case SHDID_COMPUTER_OTHER:
      case SHDID_COMPUTER_SHAREDDOCS:				// XP
      case SHDID_MOBILE_DEVICE:					// Vista
      case SHDID_COMPUTER_LIBRARIES:				// Vista
      case SHDID_SHAREPOINT_SITES:				// Vista
        {
          CFileBrowseWindow browser = gpMainFrame->GetCurrentFileBrowser();
          if (browser.IsWindow()) {
            CIDListPtr idl;
            if (idl.Clone(pf->m_pidlFq))
              browser.Browse(idl);
          }
          break;
        }
      }
    }
  }
}

BOOL CFolderTreeView::OnGetDragImage(SHDRAGIMAGE* image)
{
  memset(image, 0, sizeof(SHDRAGIMAGE));
  if (m_hDraggedItem) {
    CFolderItem* pf = (CFolderItem*)m_tree.GetItemData(m_hDraggedItem);
    if (pf) {
      CClientDC dc(NULL);
      CDC dcw;
      dcw.CreateCompatibleDC(dc);
      HFONT hfont = dcw.SelectFont(m_hFont);

      // ドラッグイメージのマウスポインタ位置
      CRect rc;
      m_tree.GetItemRect(m_hDraggedItem, &rc, TRUE);
      CPoint pt;
      GetCursorPos(&pt);
      m_tree.ScreenToClient(&pt);
      pt.x -= rc.left - (m_sizeIcon.cx + 2);
      pt.y -= rc.top;

      // 文字列サイズを取得
      UINT state = m_tree.GetItemState(m_hDraggedItem, TVIS_SELECTED);

      CString str;
      pf->GetDisplayName(str);
      rc.SetRectEmpty();
      dcw.DrawText(str, -1, &rc, DT_CALCRECT | DT_SINGLELINE | DT_NOPREFIX);
      rc.right += 6;
      rc.bottom += 2;

      image->crColorKey = RGB(255, 255, 255);
      image->ptOffset = pt;
      image->sizeDragImage.cx = m_sizeIcon.cx + rc.Width();
      image->sizeDragImage.cy = max(m_sizeIcon.cy, rc.Height());

      rc.right = image->sizeDragImage.cx;
      rc.bottom = image->sizeDragImage.cy;

      HBITMAP hbm = CreateCompatibleBitmap(dc, image->sizeDragImage.cx, image->sizeDragImage.cy);
      if (hbm) {
        hbm = dcw.SelectBitmap(hbm);
        dcw.FillSolidRect(0, 0, image->sizeDragImage.cx, image->sizeDragImage.cy, image->crColorKey);

        int iImage = (state & TVIS_SELECTED) ? pf->GetSelectedImageIndex() : pf->GetImageIndex();
        ImageList_Draw(m_himSystem, iImage, dcw, 0, (rc.Height() - m_sizeIcon.cy) / 2, ILD_TRANSPARENT | pf->OverlayImageMask());

        COLORREF clrText = dcw.SetTextColor(gColors[colSelect]);
        COLORREF clrBack = GetAttribColor(pf->m_fileattr, true);
        dcw.SetBkMode(TRANSPARENT);

        rc.left += (m_sizeIcon.cx + 2);
        dcw.FillSolidRect(&rc, clrBack);
        dcw.DrawText(str, -1, &rc, DT_SINGLELINE | DT_NOPREFIX | DT_VCENTER | DT_CENTER);

        dcw.SetTextColor(clrText);

        image->hbmpDragImage = dcw.SelectBitmap(hbm);
      }
      dcw.SelectFont(hfont);
    }
  }
  return (image->hbmpDragImage != NULL);
}

void CFolderTreeView::OnNotifyBeginDrag(NMHDR* pNMHDR, LRESULT* /*pResult*/) 
{
  NM_TREEVIEW* pnmtv = (NM_TREEVIEW*)pNMHDR;

  CFolderItem* pf = (CFolderItem*)pnmtv->itemNew.lParam;
  if (pf) {
    DWORD eff = pf->m_attr & (SFGAO_CANCOPY | SFGAO_CANMOVE | SFGAO_CANLINK);
    if (eff) {
      CComPtr<IDataObject> pdo = (IDataObject*)pf->GetUIObjectOf(m_hWnd, IID_IDataObject);
      if (pdo.p != NULL) {
        m_hDraggedItem = pnmtv->itemNew.hItem;
        DragStart(m_hWnd, pdo, 1, &eff);
        m_hDraggedItem = NULL;
        RedrawItem(pnmtv->itemNew.hItem);
      }
    }
  }
}

void CFolderTreeView::OnNotifyCustomDraw(NMHDR* pnmh, LRESULT* pResult) 
{
  LPNMTVCUSTOMDRAW ptvcd = (LPNMTVCUSTOMDRAW)pnmh;
  switch (ptvcd->nmcd.dwDrawStage) {
  case CDDS_PREPAINT:
    *pResult = CDRF_NOTIFYITEMDRAW;
    break;
  case CDDS_ITEMPREPAINT:
    *pResult = OnCustomDrawItemPrePaint(ptvcd);
    break;
/* ----------
  case CDDS_ITEMPOSTPAINT:
    *pResult = OnCustomDrawItemPostPaint(ptvcd);
    break;
----------- */
  default:
    *pResult = CDRF_DODEFAULT;
    break;
  }
}

DWORD CFolderTreeView::OnCustomDrawItemPrePaint(LPNMTVCUSTOMDRAW ptvcd)
{
  CFolderItem* pf = (CFolderItem*)ptvcd->nmcd.lItemlParam;
  if (!pf) return CDRF_DODEFAULT;

  CDCHandle dc = ptvcd->nmcd.hdc;
  HTREEITEM hItem = (HTREEITEM)ptvcd->nmcd.dwItemSpec;
  COLORREF clr = GetAttribColor(pf->m_fileattr, true);

  if (ptvcd->nmcd.uItemState & CDIS_FOCUS ||
    m_tree.GetItemState(hItem, TVIS_DROPHILITED) & TVIS_DROPHILITED ||
    hItem == m_hDraggedItem)
  {
    // 対象アイテムがフォーカスを持っているか　あるいは
    // ドロップアイテムか　あるいは
    // 対象アイテムがマークアイテムの場合
    ptvcd->clrTextBk = clr;
    ptvcd->clrText = gColors[colSelect];
  }
  else if (hItem == m_tree.GetHotItem()) {
    // CMWTreeViewCtrlの機能
    ptvcd->clrTextBk = mix_color(clr, gColors[colBack], 50);
    ptvcd->clrText = gColors[colSelect];
  }
  else {
    ptvcd->clrText = GetAttribColor(pf->m_fileattr, false);
    if (ptvcd->nmcd.uItemState & CDIS_SELECTED)
      ptvcd->clrTextBk = GetSysColor(COLOR_3DFACE);
    else
      ptvcd->clrTextBk = gColors[colBack];
  }

  CRect rc;
  if (gnListStat & LTS_SELTREELINE) {
    m_tree.GetItemRect(hItem, &rc, FALSE);
    // 1行選択
    if (ptvcd->nmcd.uItemState & CDIS_SELECTED) {
      clr = mix_color(clr, gColors[colBack], 10);
      dc.FillSolidRect(&rc, clr);
    }
  }
  return CDRF_DODEFAULT;
}

/* ----------
DWORD CFolderTreeView::OnCustomDrawItemPostPaint(LPNMTVCUSTOMDRAW ptvcd)
{
  CFolderItem* pf = (CFolderItem*)ptvcd->nmcd.lItemlParam;
  if (!pf)
    return CDRF_DODEFAULT;

  HTREEITEM hItem = (HTREEITEM)ptvcd->nmcd.dwItemSpec;
  CRect rc;
  m_tree.GetItemRect(hItem, &rc, TRUE);
  int iImage = (ptvcd->nmcd.uItemState & CDIS_SELECTED) ? pf->GetSelectedImageIndex() : pf->GetImageIndex();
  ImageList_Draw(m_himSystem, iImage, ptvcd->nmcd.hdc,
      rc.left - (m_sizeIcon.cx + 3), rc.top + (rc.Height() - m_sizeIcon.cy) / 2, ILD_TRANSPARENT | pf->OverlayImageMask());
  return CDRF_DODEFAULT;
}
---------- */

void CFolderTreeView::OnNotifyKeyDown(NMHDR* pNMHDR, LRESULT* pResult) 
{
  TV_KEYDOWN* ptvkd = (TV_KEYDOWN*)pNMHDR;
  *pResult = TRUE;

  int vkeyCtrl = _ISKEYDOWN(VK_CONTROL);
  int vkeyShift = _ISKEYDOWN(VK_SHIFT);
  int vkeyMenu = _ISKEYDOWN(VK_MENU);
  UINT nID = 0;
  switch (ptvkd->wVKey) {
    case VK_DELETE:					// ファイル削除
      nID = vkeyCtrl ? IDM_EDIT_CUT : IDM_FILE_DELETE;
      break;

    case VK_APPS:
      nID = IDM_FILE_CONTEXTMENU;
      break;

    case VK_INSERT:
      if (vkeyCtrl)
        nID = vkeyShift ? IDM_EDIT_PASTESHORTCUT : IDM_EDIT_COPY;
      else if (vkeyShift)
        nID = IDM_EDIT_PASTE;
      break;

    case VK_RETURN:
      if (vkeyShift)
        nID = IDM_FILE_SENDTO;
      else if (vkeyCtrl)
        nID = IDM_TAB_NEW;
      else if (vkeyMenu)
        nID = IDM_FILE_PROPERTY;
      else {
        HTREEITEM hItem = m_tree.GetSelectedItem();
        if (hItem) {
          if (m_tree.GetItemState(hItem, TVIS_EXPANDED) & TVIS_EXPANDED) {
            m_tree.Expand(hItem, TVE_COLLAPSERESET | TVE_COLLAPSE);
            m_tree.SetItemState(hItem, 0, TVIS_EXPANDEDONCE);
          }
          else
            m_tree.Expand(hItem, TVE_EXPAND);
        }
      }
      break;

    default:
      *pResult = FALSE;
      break;
  }

  if (nID)
    PostMessage(WM_COMMAND, (WPARAM)nID);
}

LRESULT CFolderTreeView::NotifyOwner(UINT code)
{
  NMHDR hdr;
  hdr.hwndFrom = m_hWnd;
  hdr.idFrom = GetDlgCtrlID();
  hdr.code = code;
  return m_owner.SendMessage(WM_NOTIFY, hdr.idFrom, (LPARAM)&hdr);
}

void CFolderTreeView::OnTimer(UINT_PTR nIDEvent) 
{
  if (nIDEvent == m_timerid) {
    KillTimer(m_timerid);
    m_timerid = 0;

    HTREEITEM hItem = m_tree.GetSelectedItem();
    if (hItem)
      NotifyOwner(TFN_SELCHANGED);
  }
}

void CFolderTreeView::OnNotifySelChanged(NMHDR* pNMHDR, LRESULT* pResult)
{
  *pResult = 0;
  NMTREEVIEW* pnmtv = (NMTREEVIEW*)pNMHDR;
  if (m_timerid) {
    KillTimer(m_timerid);
    m_timerid = 0;
  }

  if (pnmtv->itemOld.hItem) m_tree.SetItemState(pnmtv->itemOld.hItem, 0, TVIS_BOLD);
  if ((gnListStat & LTS_BOLDSELTREE) &&
    pnmtv->itemNew.hItem)
    m_tree.SetItemState(pnmtv->itemNew.hItem, TVIS_BOLD, TVIS_BOLD);

  if (pnmtv->action == TVC_BYKEYBOARD) {
    // キー入力によって選択が変わった
    m_timerid = SetTimer(TIMERID_DELAY, TIMER_SELDELAY, NULL);
  }
  else if (pnmtv->action == TVC_BYMOUSE || m_bCollapsing) {
    // マウスで選択が変わったか、ツリー閉で選択が変わった場合
    NotifyOwner(TFN_SELCHANGED);
  }
}

void CFolderTreeView::DoCopyMove(BOOL bCopy)
{
  CFolderItem* pf = GetCurrentFolder();
  if ((pf->m_attr & SFGAO_CANCOPY) && (pf->GetDescriptionId() == SHDID_FS_DIRECTORY)) {
    HDROP hdrop = pf->MakeDropFiles();
    if (hdrop) {
      BOOL bTabOpen = FALSE;
      BOOL bThreaded = FALSE;
      BOOL bAsAdmin = FALSE;
      TCHAR dst[MAX_PATH * 2];
      if (QueryCopy2(m_hWnd, hdrop, pf->m_pidlFq, dst, bCopy, bTabOpen, bThreaded, bAsAdmin) == IDOK) {
        if (!_pathisdirectory(dst)) {
          if (AppMessage(IDS_QUERY_MKDIR, dst, MB_QYESNO) != IDYES ||
            !fopMakeTree(dst)) {
            DragFinish(hdrop);
            return;
          }
        }
        AcceptFiles(hdrop, bCopy ? DROPEFFECT_COPY : DROPEFFECT_MOVE, dst, bThreaded);
        if (bTabOpen) {
          fvApp.OpenTabFolder(dst, TRUE);
        }
      }
      DragFinish(hdrop);
    }
  }
}

void CFolderTreeView::OnFileCopy()
{
  DoCopyMove(TRUE);
}

void CFolderTreeView::OnFileMove()
{
  NotifyOwner(TFN_ENTERFILEOPERATION);
  DoCopyMove(FALSE);
}

void CFolderTreeView::OnFileRename()
{
  CFolderItem* pf = GetCurrentFolder();
  CString name;
  if (pf && pf->m_fileattr && pf->GetPathName(name)) {
    NotifyOwner(TFN_ENTERFILEOPERATION);
    TCHAR buf[MAX_PATH * 2];
    lstrcpy(buf, name);
    fopRename(GetTopLevelParent(), buf);
  }
}

void CFolderTreeView::OnFileDelete()
{
  CWindow wndTop = GetTopLevelParent();
  CFolderItem* pf = GetCurrentFolder();
  if (pf->m_attr & SFGAO_CANDELETE) {
    if (pf->GetDescriptionId() == SHDID_FS_DIRECTORY) {
      NotifyOwner(TFN_ENTERFILEOPERATION);

      if (gnFopStat & FOS_RECYCLE || _ISKEYDOWN(VK_SHIFT)) {
        TCHAR buf[MAX_PATH * 2];
        fopTrashFile(wndTop, pf->MakeFileNames(buf, ARRAYSIZE(buf)), (GetAsyncKeyState(VK_SHIFT) >= 0) ? FOF_ALLOWUNDO : 0);
      } else {
        HDROP hDrop = pf->MakeDropFiles();
        if (hDrop) {
          fopDeleteFile(wndTop, hDrop);
          DragFinish(hDrop);
        }
      }
    } else {
      InvokeIDList(wndTop, pf->m_pidlFq, _T("delete"));
    }
  }
}

void CFolderTreeView::OnFileProperty()
{
  CFolderItem* pf = GetCurrentFolder();
  if (pf->m_attr & SFGAO_HASPROPSHEET)
    InvokeIDList(GetTopLevelParent(), pf->m_pidlFq, gszVerbProp);
}

void CFolderTreeView::OnFileAttrib()
{
  CFolderItem* pf = GetCurrentFolder();
  if (pf->GetDescriptionId() == SHDID_FS_DIRECTORY) {
    HDROP hDrop = pf->MakeDropFiles();
    if (hDrop && fopFileAttributes(GetTopLevelParent(), hDrop)) {
      CSimpleArray<HTREEITEM> array;
      ListupAllItems(m_tree.GetSelectedItem(), array);
      for (int i = 0; i < array.GetSize(); i++) {
        HTREEITEM hItem = array[i];
        pf = (CFolderItem*)m_tree.GetItemData(hItem);

        CString dir;
        if (pf->GetPathName(dir)) {
          DWORD attr = GetFileAttributes(dir);
          if (attr == 0xFFFFFFFF)
            attr = 0;
          if (attr != pf->m_fileattr) {
            pf->m_fileattr = attr;
            RedrawItem(hItem);
          }
        }
      }
    }
    DragFinish(hDrop);
  }
}

void CFolderTreeView::OnFilePack()
{
  CFolderItem* pf = GetCurrentFolder();
  if (pf->GetDescriptionId() == SHDID_FS_DIRECTORY) {
    HDROP hdrop = pf->MakeDropFiles();
    if (hdrop) {
      TCHAR dir[MAX_PATH * 2], base[MAX_PATH * 2];
      DragQueryFile(hdrop, 0, dir, ARRAYSIZE(dir));
      PathGetParent(dir, base);
      ArcCompress(m_hWnd, base, hdrop);
      DragFinish(hdrop);
    }
  }
}

void CFolderTreeView::OnFileFilterDelete()
{
  CFolderItem* pf = GetCurrentFolder();
  CString dir;
  if (pf->GetPathName(dir))
    fopFilterDelete(GetTopLevelParent(), dir);
}

void CFolderTreeView::OnFileSendTo()
{
  CFolderItem* pf = GetCurrentFolder();
  if (pf->GetDescriptionId() == SHDID_FS_DIRECTORY) {
    CComPtr<IDataObject> pdo = (IDataObject*)pf->GetUIObjectOf(m_hWnd, IID_IDataObject);
    if (pdo.p != NULL) {
      CPoint pt;
      GetPopupMenuPoint(IDM_FILE_SENDTO, &pt);
      SendTo(m_hWnd, pdo, pt);
    }
  }
}

void CFolderTreeView::OnFileContextMenu() 
{
  CPoint pt;
  GetPopupMenuPoint(IDM_FILE_CONTEXTMENU, &pt);
  CFolderItem* pf = GetCurrentFolder();
  pf->ContextMenu(m_hWnd, &pt);
}

void CFolderTreeView::OnFileShortcut() 
{
  HTREEITEM hItem = m_tree.GetSelectedItem();
  if (hItem) {
    CFolderItem* pf = (CFolderItem*)m_tree.GetItemData(hItem);
    if (pf && pf->m_psf && pf->m_attr & SFGAO_CANLINK) {
      CString str;
      if (m_tree.GetItemText(hItem, str))
        MakeShortcutObject(m_hWnd, pf->m_psf, pf->m_pidl, str);
    }
  }
}

void CFolderTreeView::OnFileAssociate()
{
  fopAssociateExtension(GetTopLevelParent(), NULL);
}

void CFolderTreeView::OnFileExecute()
{
  CFolderItem* pf = GetCurrentFolder();

  CString dir;
  if (pf && pf->GetPathName(dir)) {
    CExecuteInfo ei;
    ei.m_strCommand = dir;

    CExecDlg dlg;
    dlg.m_pei = &ei;
    if (dlg.DoModal() == IDOK) {
      if (PathIsUNC(ei.m_strCommand) || _pathisdirectory(ei.m_strCommand)) {
        ExecNewFrame(ei.m_strCommand, _T("/force"));
        return;
      }
      ExecuteAsAdmin(m_hWnd, ei.m_strCommand, dir, SW_SHOWNORMAL, ei.m_bAsAdmin);
    }
  }
}


void CFolderTreeView::GetSelectionCenterPoint(LPPOINT ppt)
{
  HTREEITEM hItem = m_tree.GetSelectedItem();
  CRect rc;
  m_tree.GetItemRect(hItem, &rc, TRUE);
  ppt->x = rc.left + rc.Width() / 2;
  ppt->y = rc.top + rc.Height() / 2;
  m_tree.ClientToScreen(ppt);
}

void CFolderTreeView::GetPopupMenuPoint(UINT nIDCommand, LPPOINT ppt)
{
  const _ATL_MSG* msg = GetCurrentMessage();
  GetCursorPos(ppt);
  if (!IsCommandTranslateAccel(msg) || !GetToolBarButtonPoint(msg, nIDCommand, ppt))
    GetSelectionCenterPoint(ppt);
}

void CFolderTreeView::OnEditCopy()
{
  CFolderItem* pf = GetCurrentFolder();
  if (pf->m_attr & SFGAO_CANCOPY) {
    CComPtr<IContextMenu> pcm = (IContextMenu*)pf->GetUIObjectOf(m_hWnd, IID_IContextMenu); 
    if (pcm.p != NULL) {
      ctx_Execute(m_hWnd, pcm, _T("copy"));
    }
  }
}

void CFolderTreeView::OnEditCut()
{
  CFolderItem* pf = GetCurrentFolder();
  if (pf->m_attr & SFGAO_CANMOVE) {
    CComPtr<IContextMenu> pcm = (IContextMenu*)pf->GetUIObjectOf(m_hWnd, IID_IContextMenu);
    if (pcm.p != NULL) {
      ctx_Execute(m_hWnd, pcm, _T("cut"));
    }
  }
}

void CFolderTreeView::OnCopyNameOnly()
{
  CFolderItem* pf = GetCurrentFolder();

  CString str;
  pf->GetDisplayName(str);
  lccSetStringToClipboard(m_hWnd, str);
  if (_ISKEYDOWN(VK_SHIFT))
    _openclipboardbyeditor(GetTopLevelParent());
}

void CFolderTreeView::OnCopyFullName()
{
  CFolderItem* pf = GetCurrentFolder();
  CString str;
  if (pf->GetRealPathName(str))
    ;
  else
    pf->GetDisplayName(str);
  lccSetStringToClipboard(m_hWnd, str);
  if (_ISKEYDOWN(VK_SHIFT))
    _openclipboardbyeditor(GetTopLevelParent());
}

void CFolderTreeView::OnCopyShortName()
{
  CFolderItem* pf = GetCurrentFolder();
  CString str;
  if (pf->GetRealPathName(str)) {
    TCHAR buf[MAX_PATH * 2];
    GetShortPathName(str, buf, ARRAYSIZE(buf));
    str = buf;
  }
  else
    pf->GetDisplayName(str);
  lccSetStringToClipboard(m_hWnd, str);
  if (_ISKEYDOWN(VK_SHIFT))
    _openclipboardbyeditor(GetTopLevelParent());
}

void CFolderTreeView::OnCopyDetails()
{
  CString str;
  str.Empty();
  DrawItemToBuffer(m_tree.GetRootItem(), 0, str);
  lccSetStringToClipboard(m_hWnd, str);
  if (_ISKEYDOWN(VK_SHIFT))
    _openclipboardbyeditor(GetTopLevelParent());
}

void CFolderTreeView::OnCopyUncName()
{
  CFolderItem* pf = GetCurrentFolder();

  CString dir;
  if (pf->GetPathName(dir)) {
    TCHAR unc[MAX_PATH * 2];
    if (PathIsUNC(dir))
      lstrcpy(unc, dir);
    else {
      if (!lccGetNetworkPath(PathGetDriveNumber(dir), unc))
        return;
      LPTSTR p = PathFindNextComponent(dir);
      if (p)
        PathAppend(unc, p);
    }
    lccSetStringToClipboard(m_hWnd, unc);
    if (_ISKEYDOWN(VK_SHIFT))
      _openclipboardbyeditor(GetTopLevelParent());
  }
}

void CFolderTreeView::DrawItemToBuffer(HTREEITEM hItem, int indent, CString& str)
{
  CFolderItem* pf;
  int pos;
  do {
    pf = (CFolderItem*)m_tree.GetItemData(hItem);
    pos = indent * 5;

    CString line;
    LPTSTR p = line.GetBuffer(pos + 1);
    tchar_set(p, ' ', pos);
    p[pos] = 0;
    line.ReleaseBuffer();

    bool bWalkTree = false;
    if (m_tree.ItemHasChildren(hItem)) {
      if (m_tree.GetItemState(hItem, TVIS_EXPANDED) & TVIS_EXPANDED) {
        bWalkTree = true;
        line += _T("[-] ");
      }
      else
        line += _T("[+] ");
    }
    else
      line += _T("    ");

    CString name;
    pf->GetDisplayName(name);
    line.AppendFormat(_T("%s\x0d\x0a"), (LPCTSTR)name);
    str += line;

    if (bWalkTree) {
      HTREEITEM hChild = m_tree.GetChildItem(hItem);
      if (hChild)
        DrawItemToBuffer(hChild, indent + 1, str);
    }
    hItem = m_tree.GetNextSiblingItem(hItem);
  } while (hItem);
}

void CFolderTreeView::SaveXMLNode(HTREEITEM hItem, int indent, TiXmlNode* pNode)
{
  CFolderItem* pf;
  do {
    pf = (CFolderItem*)m_tree.GetItemData(hItem);

    bool bWalkTree = false;
    if (m_tree.ItemHasChildren(hItem)) {
      if (m_tree.GetItemState(hItem, TVIS_EXPANDED) & TVIS_EXPANDED) {
        bWalkTree = true;
      }
    }

    CString str;
    pf->GetDisplayName(str);

    CStringA utf;
    UnicodeToUTF8(str, utf);

    // Save current item
    TiXmlNode* pItems = pNode->InsertEndChild( TiXmlElement( "folder" ) );
    TiXmlElement* pElement = pItems->ToElement();
    pElement->SetAttribute("name", utf);

    if (!pf->GetPathName(str))
      str.Empty();
    UnicodeToUTF8(str, utf);
    pElement->SetAttribute("link", utf);

    if (bWalkTree) {
      HTREEITEM hChild = m_tree.GetChildItem(hItem);
      if (hChild) {
        SaveXMLNode(hChild, indent + 1, pItems);
      }
    }
    hItem = m_tree.GetNextSiblingItem(hItem);
  } while (hItem);
}

void CFolderTreeView::SaveToXML(LPCTSTR pszXml)
{
  // Save XML
  TiXmlDocument xmlDoc;
  xmlDoc.InsertEndChild( TiXmlDeclaration( "1.0", "UTF-8", "yes" ) );

  // XML doc header
  TiXmlNode* pDoc = xmlDoc.InsertEndChild( TiXmlElement( "doc" ) );
  TiXmlElement* pElement = pDoc->ToElement();

  CStringA utf;
  SYSTEMTIME st;
  TCHAR tmp[128];
  GetLocalTime(&st);
  wsprintf(tmp, _T("%04d/%02d/%02d %02d:%02d:%02d"), st.wYear, st.wMonth, st.wDay, st.wHour, st.wMinute, st.wSecond);
  UnicodeToUTF8(tmp, utf);
  pElement->SetAttribute( "generateddatetime", utf );

  UnicodeToUTF8(APP_PRODUCT, utf);
  pElement->SetAttribute( "generater", utf );

  // XML root
  TiXmlNode* pRoot = pDoc->InsertEndChild( TiXmlElement( "root" ) );

  // Save our tree control
  SaveXMLNode(m_tree.GetRootItem(), 0, pRoot);

  char ssz[MAX_PATH * 2];
  WideCharToMultiByte(CP_ACP, 0, pszXml, -1, ssz, ARRAYSIZE(ssz), NULL, NULL);
  // Save XML
  // 書き出し可能か？
  if (!CheckFileWritable(ssz)) {
    ;	// とりあえずreadonly属性がついていれば解除される
  }
  xmlDoc.SaveFile(ssz);
}

void CFolderTreeView::OnViewFileInfo()
{
  CFolderItem* pf = GetCurrentFolder();
  HDROP hdrop = pf->MakeDropFiles();
  if (hdrop) {
    fopShowFileInfo(GetTopLevelParent(), hdrop);
    DragFinish(hdrop);
  }
}

void CFolderTreeView::OnViewFileSize()
{
  CFolderItem* pf = GetCurrentFolder();
  HDROP hdrop = pf->MakeDropFiles();
  if (hdrop) {
    fopEnumFileSize(GetTopLevelParent(), hdrop);
    DragFinish(hdrop);
  }
}

void CFolderTreeView::OnTabNew()
{
  CFolderItem* pf = GetCurrentFolder();
  CIDListPtr idl;
  if (idl.Clone(pf->m_pidlFq)) {
    // アクティブで開く
    fvApp.OpenTabFolder(idl, TRUE);
  }
}

