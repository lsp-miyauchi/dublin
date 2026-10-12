// lspftree.cpp
//

#include "stdafx.h"
#include "resource.h"
#include "lscore.h"
#include "common.h"
#include "fnc.h"
#include "droptarget.h"
#include "shellnotify.h"
#include "folderitem.h"
#include "foldertree.h"


#define TIMERID_DELAY				1
#define TIMER_SELDELAY				600


////////////////////////////////////////////////////////////////
//

static HIMAGELIST CreateDummyImageList(int cx, int cy)
{
	const COLORREF clrMask = RGB(255, 0, 255);

	// ダミービットマップ作成
	HIMAGELIST him = ImageList_Create(cx, cy, ILC_COLOR24 | ILC_MASK, 1, 0);
	if (him) {
		CClientDC dc(NULL);
		CBitmap bmp;
		bmp.CreateCompatibleBitmap(dc, cx, cy);
		CDC dcw;
		dcw.CreateCompatibleDC(dc);
		HBITMAP hbmOld = dcw.SelectBitmap(bmp);
		dcw.FillSolidRect(0, 0, cx, cy, clrMask);
		dcw.SelectBitmap(hbmOld);
		ImageList_AddMasked(him, bmp, clrMask);
		ImageList_SetBkColor(him, CLR_NONE);
	}
	return him;
}

////////////////////////////////////////////////////////////////
//

int CShellTreeCtrl::OnCreate(LPCREATESTRUCT pcs)
{
	m_owner = pcs->hwndParent;
	LONG style = WS_CHILD | WS_VISIBLE | WS_TABSTOP | TVS_HASBUTTONS | TVS_SHOWSELALWAYS;
	if (!(GetStyle() & FTWS_NOFUSE))
		style |= TVS_HASLINES;

	if (m_tree.Create(m_hWnd, NULL, NULL, style, 0, IDC_FOLDERTREE, NULL) == NULL)
		return -1;

	if (OsVISTA()) {
		DWORD mask = TVS_EX_DOUBLEBUFFER;
		DWORD exStyle = TVS_EX_DOUBLEBUFFER;
		TreeView_SetExtendedStyle(m_tree, mask, exStyle);
	}

	// 配色
	SetupTreeColors();

	// イメージリスト
	HIMAGELIST himL;
	Shell_GetImageLists(&himL, &m_himSystem);
	int cx, cy;
	ImageList_GetIconSize(m_himSystem, &cx, &cy);
	m_sizeIcon.SetSize(cx, cy);
	m_himTree = ImageList_Create(m_sizeIcon.cx, m_sizeIcon.cy, IsRGBA() ? ILC_COLOR32 : (ILC_COLORDDB | ILC_MASK), 16, 1);
	ImageList_SetBkColor(m_himTree, IsRGBA() ? CLR_NONE : GetSysColor(COLOR_WINDOW));

	m_tree.SetImageList(m_himTree, TVSIL_NORMAL);
	m_tree.SetScrollTime(100);

	int h = m_tree.GetItemHeight();
	m_tree.SetItemHeight(h);

	ImmAssociateContext(m_hWnd, NULL);
	ImmAssociateContext(m_tree, NULL);

	m_pShellNotify = new CShellNotify();
	m_pShellNotify->Start(m_hWnd,
					SHCNE_UPDATEDIR |
					SHCNE_NETSHARE | SHCNE_NETUNSHARE |
					SHCNE_MEDIAINSERTED | SHCNE_MEDIAREMOVED |
					SHCNE_DRIVEREMOVED | SHCNE_DRIVEADD |
					SHCNE_MKDIR | SHCNE_RMDIR | SHCNE_RENAMEFOLDER |
					SHCNE_UPDATEITEM | SHCNE_UPDATEIMAGE);

	if (!InitializeTree())
		return -1;
	return 0;
}

void CShellTreeCtrl::OnDestroy()
{
	if (m_pShellNotify) {
		delete m_pShellNotify;
		m_pShellNotify = NULL;
	}

	if (m_timerid)
		KillTimer(m_timerid);

	m_tree.DeleteAllItems();

	if (m_himTree)
		ImageList_Destroy(m_himTree);
}

void CShellTreeCtrl::SetupTreeColors()
{
	m_tree.SetTextColor(m_colors[colNorm]);
	m_tree.SetBkColor(m_colors[colBack]);
	m_tree.SetLineColor(m_colors[colLine]);
}

bool CShellTreeCtrl::InitializeTree(PCIDLIST_ABSOLUTE pidlRoot)
{
	if (m_bInitializeCompleted) {
		// 既に初期化済みならばエントリを削除
		CSetRedraw sr(m_tree);
		m_tree.DeleteAllItems();
	}

	// 隠し属性フォルダの取り扱いを問い合わせ
	m_treatHidden = (int)SendSimpleNotify(FTWN_QUERYHIDDEN);

	IShellFolder* psf = NULL;
	LPITEMIDLIST pidlFullyQual = NULL;
	LPCITEMIDLIST pidlSelf = NULL;

	if (pidlRoot) {
		CPidl idl;
		idl.Clone(pidlRoot);
		if (idl.isDesktop()) {
			// デスクトップなら特別扱い
			SHGetSpecialFolderLocation(NULL, CSIDL_DESKTOP, &pidlFullyQual);
			pidlSelf = pidlFullyQual;
		}
		else {
			psf = idl.BindToParent(&pidlSelf);
			if (!psf)
				return false;
			pidlFullyQual = idl.Detach();
		}
	}
	else {
		SHGetSpecialFolderLocation(NULL, CSIDL_DESKTOP, &pidlFullyQual);
		pidlSelf = pidlFullyQual;
	}

	CFolderItem* pf = new CFolderItem;
	pf->m_pidlFq = pidlFullyQual;
	pf->m_pidl = pidlSelf;
	pf->m_psf = psf;
	pf->GetAttributes();
	HTREEITEM hItem = InsertTreeItem(NULL, TVI_LAST, pf);
	if (m_bInitializeCompleted)
		m_tree.SelectItem(hItem);
	m_tree.Expand(hItem, TVE_EXPAND);

	m_bInitializeCompleted = true;
	return true;
}

void CShellTreeCtrl::OnNotifyGetDispInfo(NMHDR* pNmhdr, LRESULT* pResult)
{
	NMTVDISPINFO* ptvdi = (NMTVDISPINFO*)pNmhdr;
	CFolderItem* pf = (CFolderItem*)ptvdi->item.lParam;

	if (ptvdi->item.mask & TVIF_TEXT) {
		CString str;
		pf->GetDisplayName(str);
		lstrcpyn(ptvdi->item.pszText, str, ptvdi->item.cchTextMax);
		ptvdi->item.mask |= TVIF_DI_SETITEM;
	}
	if (ptvdi->item.mask & TVIF_IMAGE) {
		UINT style = ILD_NORMAL;
		int iImage = pf->GetOverlayImageIndex();
		if (iImage > 0)
			style |= INDEXTOOVERLAYMASK(iImage);

		// Sysytem Imagelist からアイコンを取得して、自分のイメージリストにコピー
		iImage = pf->GetImageIndex();
		HICON hic = ImageList_GetIcon(m_himSystem, iImage, style);
		ptvdi->item.iImage = ImageList_ReplaceIcon(m_himTree, -1, hic);
		DestroyIcon(hic);
		ptvdi->item.mask |= TVIF_DI_SETITEM;
	}
	if (ptvdi->item.mask & TVIF_SELECTEDIMAGE) {
		UINT style = ILD_NORMAL;
		int iImage = pf->GetOverlayImageIndex();
		if (iImage > 0)
			style |= INDEXTOOVERLAYMASK(iImage);

		// Sysytem Imagelist からアイコンを取得して、自分のイメージリストにコピー
		iImage = pf->GetSelectedImageIndex();
		HICON hic = ImageList_GetIcon(m_himSystem, iImage, style);
		ptvdi->item.iSelectedImage = ImageList_ReplaceIcon(m_himTree, -1, hic);
		DestroyIcon(hic);
		ptvdi->item.mask |= TVIF_DI_SETITEM;
	}
	if (ptvdi->item.mask & TVIF_CHILDREN) {
		ptvdi->item.cChildren = pf->hasSubFolder() ? 1 : 0;
		ptvdi->item.mask |= TVIF_DI_SETITEM;
	}
}

void CShellTreeCtrl::OnNotifySelChanged(NMHDR* pNMHDR, LRESULT* pResult)
{
	*pResult = 0;
	NMTREEVIEW* pnmtv = (NMTREEVIEW*)pNMHDR;
	if (m_timerid) {
		KillTimer(m_timerid);
		m_timerid = 0;
	}

	if (!(GetStyle() & FTWS_NOBOLDSELECT)) {
		m_tree.SetItemState(pnmtv->itemOld.hItem, 0, TVIS_BOLD);
		m_tree.SetItemState(pnmtv->itemNew.hItem, TVIS_BOLD, TVIS_BOLD);
	}

	if (pnmtv->action == TVC_BYKEYBOARD)		// キー入力によって選択が変わった
		m_timerid = SetTimer(TIMERID_DELAY, TIMER_SELDELAY);
	else {
		SendSimpleNotify(FTWN_SELCHANGED);
		m_owner.SendMessage(WM_COMMAND, MAKEWPARAM(GetDlgCtrlID(), FTWN_SELCHANGED), (LPARAM)m_hWnd);
	}
}

void CShellTreeCtrl::OnNotifyItemExpanding(NMHDR* pNMHDR, LRESULT* pResult)
{
	NMTREEVIEW* pnmtv = (NMTREEVIEW*)pNMHDR;

	if (pnmtv->action == TVE_COLLAPSE)
		m_tree.Expand(pnmtv->itemNew.hItem, TVE_COLLAPSE | TVE_COLLAPSERESET);
	else if (pnmtv->action == TVE_EXPAND) {
		CFolderItem* pf = (CFolderItem*)m_tree.GetItemData(pnmtv->itemNew.hItem);
		if (pf) {
			HCURSOR hCursor = SetCursor(LoadCursor(NULL, IDC_WAIT));
			IShellFolder* psf;
			if (SUCCEEDED(pf->BindToShellFolder(&psf))) {
				EnumFolderObjects(pnmtv->itemNew.hItem, psf, pf->m_pidlFq);
				psf->Release();
			}
			SetCursor(hCursor);
		}
	}
}

void CShellTreeCtrl::OnNotifyKeyDown(NMHDR* pNMHDR, LRESULT* pResult) 
{
	TV_KEYDOWN* ptvkd = (TV_KEYDOWN*)pNMHDR;
	*pResult = TRUE;

	switch (ptvkd->wVKey) {
	case VK_TAB:
		SendSimpleNotify(FTWN_TABPRESSED);
		m_owner.SendMessage(WM_COMMAND, MAKEWPARAM(GetDlgCtrlID(), FTWN_TABPRESSED), (LPARAM)m_hWnd);
		break;
	case VK_RETURN: {
		HTREEITEM hItem = m_tree.GetSelectedItem();
		if (hItem)
			m_tree.Expand(hItem, TVE_TOGGLE);
		break;
	}
	default:
		*pResult = FALSE;
		break;
	}
}

void CShellTreeCtrl::OnNotifyDeleteItem(NMHDR* pNMHDR, LRESULT* pResult)
{
	NMTREEVIEW* pnmtv = (NMTREEVIEW*)pNMHDR;
	CFolderItem* pf = (CFolderItem*)pnmtv->itemOld.lParam;
	if (pf)
		delete pf;
}

void CShellTreeCtrl::OnNotifyRClick(NMHDR* pNMHDR, LRESULT* pResult)
{
	*pResult = TRUE;
	CPoint point(GetMessagePos());

	TV_HITTESTINFO tvhti;
	tvhti.pt = point;
	m_tree.ScreenToClient(&tvhti.pt);
	HTREEITEM hItem = m_tree.HitTest(&tvhti);
	if (hItem) {
		if (!(GetStyle() & FTWS_NOCONTEXTMENU)) {
			ContextMenu(hItem, &point);
			return;
		}
		HTREEITEM hSel = m_tree.GetSelectedItem();
		if (hSel != hItem)
			m_tree.SelectItem(hItem);
	}
}

COLORREF CShellTreeCtrl::GetAttributeColor(CFolderItem* pf, bool bHilited)
{
	COLORREF clr;
	if (bHilited) {
		if      (pf->m_fileattr & FILE_ATTRIBUTE_SYSTEM)		clr = m_colors[colSystem];
		else if (pf->m_fileattr & FILE_ATTRIBUTE_HIDDEN)		clr = m_colors[colHidden];
		else if (pf->m_fileattr & FILE_ATTRIBUTE_READONLY)		clr = m_colors[colRdonly];
		else if (pf->m_fileattr & FILE_ATTRIBUTE_COMPRESSED)	clr = m_colors[colCmprs];
		else if (pf->m_fileattr & FILE_ATTRIBUTE_ENCRYPTED)		clr = m_colors[colEncr];
		else if (pf->m_fileattr & FILE_ATTRIBUTE_REPARSE_POINT)	clr = m_colors[colRepp];
		else 													clr = m_colors[colSelBack];
	}
	else {
		if      (pf->m_fileattr & FILE_ATTRIBUTE_SYSTEM)		clr = m_colors[colSystem];
		else if (pf->m_fileattr & FILE_ATTRIBUTE_HIDDEN)		clr = m_colors[colHidden];
		else if (pf->m_fileattr & FILE_ATTRIBUTE_READONLY)		clr = m_colors[colRdonly];
		else if (pf->m_fileattr & FILE_ATTRIBUTE_COMPRESSED)	clr = m_colors[colCmprs];
		else if (pf->m_fileattr & FILE_ATTRIBUTE_ENCRYPTED)		clr = m_colors[colEncr];
		else if (pf->m_fileattr & FILE_ATTRIBUTE_REPARSE_POINT)	clr = m_colors[colRepp];
		else													clr = m_colors[colNorm];
	}
	return clr;
}

void CShellTreeCtrl::OnNotifyCustomDraw(NMHDR* pHdr, LRESULT* pResult)
{
	LPNMTVCUSTOMDRAW ptvcd = (LPNMTVCUSTOMDRAW)pHdr;

	CFolderItem* pf;
	HTREEITEM hItem;

	switch (ptvcd->nmcd.dwDrawStage) {
	case CDDS_PREPAINT:
		*pResult = CDRF_NOTIFYITEMDRAW;
		break;
	case CDDS_ITEMPREPAINT:
	{
		pf = (CFolderItem*)ptvcd->nmcd.lItemlParam;
		if (!pf) {
			*pResult = CDRF_DODEFAULT;
			break;
		}

		hItem = (HTREEITEM)ptvcd->nmcd.dwItemSpec;
		bool bHot = (m_tree.GetHotItem() == hItem);
		bool bHilited = (m_tree.GetDropHilightItem() == hItem);
		bool bSelected = ((ptvcd->nmcd.uItemState & CDIS_SELECTED) != 0);
		bool bFocused = ((ptvcd->nmcd.uItemState & CDIS_FOCUS) != 0);

		COLORREF clrText, clrBack, clrLine;
		if (bSelected || bHot || bHilited) {
			clrText = m_colors[colSelect];
			clrBack = GetAttributeColor(pf, true);
			clrLine = clrBack;
			if (bHot)
				clrBack = mix_color(clrBack, m_colors[colBack], 50);

			if (bSelected && !bHot && !bHilited && !bFocused) {
				clrText = GetAttributeColor(pf, false);
				clrBack = GetSysColor(COLOR_3DFACE);
			}
		}
		else {
			clrText = GetAttributeColor(pf, false);
			clrLine = clrText;
			clrBack = m_colors[colBack];
		}
		ptvcd->clrText   = clrText;
		ptvcd->clrTextBk = clrBack;

		if (GetStyle() & FTWS_FULLROWSELECT) {
			// 1行選択
			CRect rc;
			m_tree.GetItemRect(hItem, &rc, FALSE);
			CDCHandle dc = ptvcd->nmcd.hdc;
			dc.FillSolidRect(&rc, bSelected ? mix_color(clrLine, m_colors[colBack], 10) : GetSysColor(COLOR_WINDOW));
		}
		*pResult = CDRF_DODEFAULT;
		break;
	}
	default:
		*pResult = 0;
		break;
	}
}

void CShellTreeCtrl::OnTimer(UINT_PTR id)
{
	if (id == m_timerid) {
		KillTimer(m_timerid);
		m_timerid = 0;

		HTREEITEM hItem = m_tree.GetSelectedItem();
		if (hItem) {
			SendSimpleNotify(FTWN_SELCHANGED);
			m_owner.SendMessage(WM_COMMAND, MAKEWPARAM(GetDlgCtrlID(), FTWN_SELCHANGED), (LPARAM)m_hWnd);
		}
	}
}

int CALLBACK CShellTreeCtrl::CompareFolder(LPARAM lp1, LPARAM lp2, LPARAM lps)
{
	CFolderItem* p1 = (CFolderItem*)lp1;
	CFolderItem* p2 = (CFolderItem*)lp2;
	HRESULT hr = p1->CompareIDs(p2->m_pidl);
	return (short)SCODE_CODE(GetScode(hr));
}

void CShellTreeCtrl::SortChildren(HTREEITEM hParentItem)
{
	TV_SORTCB tvscb;
	tvscb.hParent = hParentItem;
	tvscb.lpfnCompare = CompareFolder;
	tvscb.lParam = 0;
	m_tree.SortChildrenCB(&tvscb);
}

HTREEITEM CShellTreeCtrl::InsertTreeItem(HTREEITEM hParent, HTREEITEM hInsertAfter, CFolderItem* pf)
{
	TVITEM item = {0};
	item.mask = TVIF_PARAM | TVIF_TEXT | TVIF_IMAGE | TVIF_SELECTEDIMAGE | TVIF_CHILDREN;
	item.pszText = LPSTR_TEXTCALLBACK;
	item.iImage = item.iSelectedImage = I_IMAGECALLBACK;
	item.lParam= (LPARAM)pf;
	if (pf->isDesktop()) {
		// デスクトップ
		item.cChildren = 1;
		pf->m_attr = SFGAO_FOLDER | SFGAO_FILESYSTEM | SFGAO_DROPTARGET | SFGAO_DROPTARGET;
	}
	else {
		// サブフォルダの存在チェック
		pf->GetAttributes();
		item.cChildren = I_CHILDRENCALLBACK;

		// 属性の取得
		WIN32_FIND_DATA wfd;
		if (SHGetDataFromIDList(pf->m_psf, pf->m_pidl, SHGDFIL_FINDDATA, (void*)&wfd, sizeof(wfd)) == NOERROR) {
			pf->m_fileattr = wfd.dwFileAttributes;
		}
	}

	TVINSERTSTRUCT ins = {0};
	ins.item = item;
	ins.hInsertAfter = hInsertAfter;
	ins.hParent = hParent;

	HTREEITEM hItem = m_tree.InsertItem(&ins);
	return hItem;
}

bool CShellTreeCtrl::EnumFolderObjects(HTREEITEM hParentItem, IShellFolder* psf, PCIDLIST_ABSOLUTE pidlParent)
{
	CFolderItem* pf;
	IEnumIDList* pe = NULL;
	HRESULT hr = psf->EnumObjects(NULL, SHCONTF_FOLDERS | SHCONTF_INCLUDEHIDDEN | SHCONTF_INCLUDESUPERHIDDEN, &pe);
	if (SUCCEEDED(hr) && pe) {
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
		if (hr == S_OK && pe) {
			ITEMIDLIST* pidl;
			ULONG celt;
			while (SUCCEEDED(pe->Next(1, &pidl, &celt)) && celt) {
				ATLASSERT(pidl != NULL);			// 何故かコントロールパネルのツリーを開くときに起きることがある
				if (!pidl) break;

				pf = new CFolderItem(psf, pidlParent, pidl);
				CoTaskMemFree(pidl);

				SFGAOF attr = pf->GetAttributes();
				if ((attr & (SFGAO_STREAM | SFGAO_LINK)) == SFGAO_STREAM) {
					// 書庫?
					delete pf;
					pf = NULL;
				}
				else if ((m_treatHidden == FTH_HIDDEN && attr & SFGAO_HIDDEN) ||
						 (m_treatHidden == FTH_SYSHIDDEN && (attr & SFGAO_HIDDEN) && (attr & SFGAO_SYSTEM))) {
					delete pf;
					pf = NULL;
				}
				else {
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
				}
				if (pf) {
					InsertTreeItem(hParentItem, TVI_LAST, pf);
					numAdd++;
				}
			}
			pe->Release();
		}

		if (numAdd > 0)
			SortChildren(hParentItem);
		else if (m_tree.GetChildItem(hParentItem) == NULL) {
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

BOOL CShellTreeCtrl::ContextMenu(HTREEITEM hItem, LPPOINT ppt)
{
	m_tree.SelectDropTarget(hItem);
	BOOL bOK = FALSE;
	CFolderItem* pf = (CFolderItem*)m_tree.GetItemData(hItem);
	if (!pf->isDesktop()) {
		IContextMenu* pcm = (IContextMenu*)pf->GetUIObjectOf(m_hWnd, IID_IContextMenu);
		if (pcm) {
			UINT flags = CMF_EXPLORE;
			if (GetKeyState(VK_SHIFT) < 0)
				flags |= CMF_EXTENDEDVERBS;
			bOK = lccTrackContextMenu(m_hWnd, pcm, ppt, CMF_EXPLORE);
			pcm->Release();
		}
	}
	m_tree.SelectDropTarget(NULL);
	return bOK;
}

BOOL CShellTreeCtrl::SetRoot(UINT mode, void* pv)
{
	if (!pv) {
		// デスクトップをルート
		return InitializeTree(NULL);
	}

	CPidl idl;
	if (mode == FT_BYPATH) {
		// パス名で指定
		idl.CreateFromPath((LPCTSTR)pv);
	}
	else if (mode == FT_BYIDLIST) {
		// IDListで指定
		idl.Clone((LPCITEMIDLIST)pv);
	}
	else if (mode == FT_BYCSIDL) {
		// CSIDLで指定
		idl.GetFolderLocation(m_hWnd, PtrToInt(pv));
	}
	else if (mode == FT_BYFOLDERID) {
		// KNOWNFOLDERIDで指定
		idl.GetKnownFolder(*(KNOWNFOLDERID*)pv);
	}

	BOOL bOK = FALSE;
	if (!idl.isNull())
		bOK = InitializeTree(idl);
	return bOK;
}

BOOL CShellTreeCtrl::QueryDescendant(UINT mode, void* pv)
{
	if (!pv)
		return FALSE;

	CPidl idl;
	if (mode == FT_BYPATH) {
		// パス名で指定
		idl.CreateFromPath((LPCTSTR)pv);
	}
	else if (mode == FT_BYIDLIST) {
		// IDListで指定
		idl.Clone((LPCITEMIDLIST)pv);
	}
	else if (mode == FT_BYCSIDL) {
		// CSIDLで指定
		idl.GetFolderLocation(m_hWnd, PtrToInt(pv));
	}

	BOOL bDescendant = FALSE;
	if (!idl.isNull())
		bDescendant = IsRootDescendant(idl);
	return bDescendant;
}

BOOL CShellTreeCtrl::IsRootDescendant(PCIDLIST_ABSOLUTE pidl)
{
	// ルートの子孫かどうかをチェック
	BOOL bDescendant = FALSE;
	if (pidl) {
		CFolderItem* pf = (CFolderItem*)m_tree.GetItemData(m_tree.GetRootItem());
		if (pf)
			bDescendant = (ILIsParent(pf->m_pidlFq, pidl, FALSE) || ILIsEqual(pf->m_pidlFq, pidl));
	}
	return bDescendant;
}

HTREEITEM CShellTreeCtrl::FindIDList(HTREEITEM hParent, PCUIDLIST_RELATIVE pidl)
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

BOOL CShellTreeCtrl::SelectChild(PCUIDLIST_RELATIVE pidlSingle)
{
	HTREEITEM hItem = m_tree.GetSelectedItem();
	m_tree.Expand(hItem, TVE_EXPAND);

	BOOL bOK = FALSE;
	HTREEITEM hChild = FindIDList(hItem, pidlSingle);
	if (!hChild) {
		CFolderItem* pf = (CFolderItem*)m_tree.GetItemData(hItem);
		ATLASSERT(pf != NULL);
		IShellFolder* psf;
		if (pf && SUCCEEDED(pf->BindToShellFolder(&psf))) {
			ULONG ula = SFGAO_FOLDER | SFGAO_CAPABILITYMASK;
			if (psf->GetAttributesOf(1, (LPCITEMIDLIST*)&pidlSingle, &ula) == NOERROR) {
				// 新規に追加
				CFolderItem* pNewItemInfo = new CFolderItem;
				psf->AddRef();
				pNewItemInfo->m_psf = psf;
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
			psf->Release();
		}
	}
	if (hChild)
		m_tree.SelectItem(hChild);
	return bOK;
}

BOOL CShellTreeCtrl::SelectParent()
{
	HTREEITEM hItem = m_tree.GetParentItem(m_tree.GetSelectedItem());
	return (hItem != NULL) ? m_tree.SelectItem(hItem) : FALSE;
}

LRESULT CShellTreeCtrl::OnSetCurSel(WPARAM wParam, LPARAM lParam)
{
	CPidl idl;
	if (wParam == FT_BYPATH)
		idl.CreateFromPath((LPCTSTR)lParam);
	else if (wParam == FT_BYIDLIST)
		idl.Clone((LPCITEMIDLIST)lParam);
	else if (wParam == FT_BYCSIDL)
		idl.GetFolderLocation(m_hWnd, (UINT)lParam);

	BOOL bOK = FALSE;
	if (!idl.isNull()) {
		HTREEITEM hItem = NULL;
		bOK = FindFolder(idl, &hItem, true);
		if (bOK)
			m_tree.SelectItem(hItem);
	}
	return (LRESULT)bOK;
}

LRESULT CShellTreeCtrl::OnGetCurSel(WPARAM wParam, LPARAM lParam)
{
	HTREEITEM hItem = m_tree.GetSelectedItem();
	if (hItem) {
		CFolderItem* pf = (CFolderItem*)m_tree.GetItemData(hItem);
		if (lParam)
			*((DWORD*)lParam) = pf->m_attr;
		return (LRESULT)pf->m_pidlFq;
	}
	return 0;
}

void CShellTreeCtrl::RefreshTree()
{
	HIMAGELIST himL, himS;
	Shell_GetImageLists(&himL, &himS);
	m_tree.SetImageList(himS, TVSIL_NORMAL);

	HTREEITEM hNext, hItem = m_tree.GetRootItem();
	while (hItem) {
		// 表示されていないツリーアイテムの削除
		UINT state = m_tree.GetItemState(hItem, TVIS_EXPANDED | TVIS_EXPANDEDONCE);
		if (!(state & TVIS_EXPANDED) && state & TVIS_EXPANDEDONCE) {
			HTREEITEM hChild = m_tree.GetChildItem(hItem);
			while (hChild) {
				hNext = m_tree.GetNextSiblingItem(hChild);
				m_tree.DeleteItem(hChild);
				hChild = hNext;
			}
			m_tree.SetItemState(hItem, 0, TVIS_EXPANDEDONCE | TVIS_EXPANDED);
		}

		hNext = m_tree.GetNextVisibleItem(hItem);
		hItem = hNext;
	}
}

LRESULT CShellTreeCtrl::OnSetTreeCtrlStyle(WPARAM wParam, LPARAM lParam)
{
	DWORD oldStyle = m_tree.GetStyle();
	if (m_tree.ModifyStyle((DWORD)wParam, (DWORD)lParam)) {
		DWORD newStyle = m_tree.GetStyle();
		if (OsVISTA())
			TreeView_SetExtendedStyle(m_tree, TVS_EX_FADEINOUTEXPANDOS, (newStyle & TVS_HASLINES) ? TVS_EX_FADEINOUTEXPANDOS : 0);
		m_tree.Invalidate();

		ModifyStyle(FTWS_NOFUSE, (newStyle & TVS_HASLINES) ? FTWS_NOFUSE : 0);
	}
	return (LRESULT)m_tree.GetStyle();
}

void CShellTreeCtrl::OnSetCustomColors(WPARAM wParam, LPARAM lParam)
{
	if (wParam && lParam) {
		int cnt = min((int)wParam, colEndValue);
		COLORREF* clrs = (COLORREF*)lParam;
		for (int i = 0; i < cnt; i++) {
			if (*clrs != CLR_DEFAULT)
				m_colors[i] = *clrs;
			clrs++;
		}
		m_tree.Invalidate();
	}
}

HWND CShellTreeCtrl::SetOwner(HWND hwndOwner)
{
	HWND hwndT = NULL;
	if (::IsWindow(hwndOwner)) {
		hwndT = m_owner;
		m_owner = hwndOwner;
	}
	return hwndT;
}

BOOL CShellTreeCtrl::FindFolder(PCIDLIST_ABSOLUTE pidlSelection, HTREEITEM* pItem, bool bExplore)
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
	if (ILIsEqual(pf->m_pidlFq, pidlSelection)) {
		if (pItem)
			*pItem = hItem;
		return TRUE;
	}

	if (!ILIsParent(pf->m_pidlFq, pidlSelection, FALSE)) {
		// ルートとは無関係
		return FALSE;
	}

	// ルート配下のサブフォルダをチェック
	HTREEITEM hParent = hItem;
	hItem = m_tree.GetChildItem(hParent);
	while (hItem) {
		pf = (CFolderItem*)m_tree.GetItemData(hItem);
		if (ILIsEqual(pf->m_pidlFq, pidlSelection)) {
			// フォルダ一致
			if (pItem)
				*pItem = hItem;
			return TRUE;
		}
		if (ILIsParent(pf->m_pidlFq, pidlSelection, FALSE)) {
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
		while (pidl && !ILIsEqual(pidl, pf->m_pidlFq)) {
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

BOOL CShellTreeCtrl::UpdateFolder(PCIDLIST_ABSOLUTE pidl)
{
	CFolderItem* pf = (CFolderItem*)m_tree.GetItemData(m_tree.GetRootItem());
	if (!pf->IsDesktop()) {
		if (!ILIsParent(pf->m_pidlFq, pidl, FALSE)) {
			// 配下のフォルダじゃなければ何もしない
			return FALSE;
		}
		pidl = ILFindChild(pf->m_pidlFq, pidl);

		// 引数のPIDLが現在のフォルダだったらうまく動作しないゾ
	}

	HTREEITEM hItem;
	if (FindFolder(pidl, &hItem, false)) {
		pf = (CFolderItem*)m_tree.GetItemData(hItem);
		pf->ValidateCacheFolderInformation();

		pf->m_attr = 0;
		pf->GetAttributes();

		// 属性の取得
		WIN32_FIND_DATA wfd;
		if (SHGetDataFromIDList(pf->m_psf, pf->m_pidl, SHGDFIL_FINDDATA, (void*)&wfd, sizeof(wfd)) == NOERROR) {
			pf->m_fileattr = wfd.dwFileAttributes;
		}
		return TRUE;
	}
	return FALSE;
}

BOOL CShellTreeCtrl::AddFolder(PCIDLIST_ABSOLUTE pidl)
{
	BOOL bOK = FALSE;
	CPidl idlParent = idl_GetParent(pidl);
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
			IShellFolder* psf;
			if (SUCCEEDED(pf->BindToShellFolder(&psf))) {
				EnumFolderObjects(hParent, psf, pf->m_pidlFq);
				psf->Release();
			}
		}
		bOK = TRUE;
	}
	return bOK;
}

BOOL CShellTreeCtrl::ItemIsParent(HTREEITEM hParent, HTREEITEM hItem)
{
	hItem = m_tree.GetParentItem(hItem);
	while (hItem) {
		if (hItem == hParent)
			return TRUE;
		hItem = m_tree.GetParentItem(hItem);
	}
	return FALSE;
}

void CShellTreeCtrl::DeleteTreeItem(HTREEITEM hItem)
{
	HTREEITEM hSelItem = m_tree.GetSelectedItem();

	// 現在選択中の項目を削除するか、選択項目の親を削除するのであれば
	// 削除後に選択項目を変更する
	if ((hItem == hSelItem) || ItemIsParent(hSelItem, hItem))
		hSelItem = m_tree.GetParentItem(hItem);
	else
		hSelItem = NULL;

	// 削除の実行
	m_tree.DeleteItem(hItem);

	if (hSelItem) {
		m_tree.SelectItem(hSelItem);
		if (!m_tree.GetChildItem(hSelItem)) {
			// 配下にサブフォルダが無くなれば、[+]記号を消去
			TVITEM tvi;
			tvi.hItem = hSelItem;
			tvi.mask = TVIF_CHILDREN;
			tvi.cChildren = 0;
			m_tree.SetItem(&tvi);
		}
	}
	else {
		SendSimpleNotify(FTWN_SELCHANGED);
		m_owner.SendMessage(WM_COMMAND, MAKEWPARAM(GetDlgCtrlID(), FTWN_SELCHANGED), (LPARAM)m_hWnd);
	}
}

BOOL CShellTreeCtrl::DeleteFolder(PCIDLIST_ABSOLUTE pidl)
{
	HTREEITEM hItem = NULL;
	CFolderItem* pf = (CFolderItem*)m_tree.GetItemData(m_tree.GetRootItem());
	if (ILIsEqual(pf->m_pidlFq, pidl)) {
		// ルートは削除できませんがな
		return FALSE;
	}

	if (!ILIsParent(pf->m_pidlFq, pidl, FALSE)) {
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

int CShellTreeCtrl::ListupAllItems(HTREEITEM hItem, CSimpleArray<HTREEITEM>& array)
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

void CShellTreeCtrl::UpdateImage(int iChangeImageIndex)
{
	CSimpleArray<HTREEITEM> array;
	ListupAllItems(NULL, array);
	int cnt = array.GetSize();
	for (int i = 0; i < cnt; i++) {
		HTREEITEM hItem = array[i];
		CFolderItem* pf = (CFolderItem*)m_tree.GetItemData(hItem);

		if (pf->GetImageIndex() == iChangeImageIndex) {
			// 次回に再度取得する
			pf->m_iImage = -1;
			pf->m_iSelectedImage = -1;
			pf->m_iOverlayImage = -1;
		}
	}
	array.RemoveAll();
}

void CShellTreeCtrl::ProcessShellNotify(DWORD id, LPITEMIDLIST* pidls)
{
	switch (id) {
	case SHCNE_NETUNSHARE:
	case SHCNE_NETSHARE:
		UpdateFolder(pidls[0]);
		break;

	case SHCNE_MEDIAINSERTED:
	case SHCNE_MEDIAREMOVED:
		DeleteFolder(pidls[0]);
		AddFolder(pidls[0]);
		break;

	case SHCNE_MKDIR:
	case SHCNE_DRIVEADD:
		AddFolder(pidls[0]);
		break;

	case SHCNE_DRIVEREMOVED:
		DeleteFolder(pidls[0]);
		break;

	case SHCNE_RENAMEFOLDER: {
		TCHAR buf[MAX_PATH * 2];
		if (SHGetPathFromIDList(pidls[0], buf) && PathIsRoot(buf))
			break;
		// fall down ...
	}

	case SHCNE_RMDIR:
		DeleteFolder(pidls[0]);
		if (id == SHCNE_RENAMEFOLDER)
			AddFolder(pidls[1]);
		break;

	case SHCNE_UPDATEITEM:
	{
		SHFILEINFO sfi;
		sfi.dwAttributes = SFGAO_FOLDER;
		if (SHGetFileInfo((LPCTSTR)pidls[0], 0, &sfi, sizeof(sfi), SHGFI_PIDL | SHGFI_ATTRIBUTES | SHGFI_ATTR_SPECIFIED) && !(sfi.dwAttributes & SFGAO_FOLDER))
			UpdateFolder(pidls[0]);
		break;
	}
	case SHCNE_UPDATEIMAGE:
		UpdateImage(((WORD*)pidls[0])[1]);
		break;

	default:
		break;
	}
}

LRESULT CShellTreeCtrl::OnShellNotify(WPARAM wParam, LPARAM lParam)
{
	LPITEMIDLIST* pidls;
	LONG event_id;
	HANDLE h = m_pShellNotify->LockEvent((HANDLE)wParam, (DWORD)lParam, &pidls, &event_id);
	if (h) {
		ProcessShellNotify(event_id, pidls);
		m_pShellNotify->UnlockEvent(h);
	}
	return 0;
}

LRESULT CShellTreeCtrl::SendSimpleNotify(UINT code)
{
	NMHDR hdr = { 0 };
	hdr.hwndFrom = m_hWnd;
	hdr.idFrom = GetDlgCtrlID();
	hdr.code = code;
	return m_owner.SendMessage(WM_NOTIFY, (WPARAM)hdr.idFrom, (LPARAM)&hdr);
}


////////////////////////////////////////////////////////////////
//

extern "C"
LRESULT CALLBACK FolderTreeWnd(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	LRESULT lr = DefWindowProc(hwnd, msg, wParam, lParam);
	if (msg == WM_NCCREATE) {
		CShellTreeCtrl* pWnd = new CShellTreeCtrl;
		pWnd->SubclassWindow(hwnd);
	}
	return lr;
}

