// quick.cpp : implementation file
//

#include "stdafx.h"
#include "fv7.h"

bool CreateJumpList();
void DeleteJumpList();

/////////////////////////////////////////////////////////////////////////////
//

void BuildBookmarkFileName(CString& str)
{
	GetAppDataFolder(str, _T("filevisor7.bookmark"));
}

int LoadBookmark(CCmItemArray& array)
{
	array.DeleteAll();

	CString str;
	BuildBookmarkFileName(str);
	if (_pathfileexists(str))
		return LoadCmItemArray(str, array);

	int cnt = -1;
	CCmItem* pci;
	int i, len;
	if (RegSectionExists(IDS_SEC_FAVORITE)) {
		i = 0;
		while (1) {
			BYTE* pd = GetRegBinarySeqEx(IDS_SEC_FAVORITE, IDS_KEY_SEQNUM, i, &len);
			if (!pd) break;

			QISTRUCT* ps = (QISTRUCT*)pd;
			if (ps->magic == QUICKMAGIC) {
				pci = new CCmItem(ps->GetIDListPtr(), ps->GetNamePtr());
				pci->m_id = i;
				array.Add(pci);
			}
			free(pd);
			i++;
		}
		cnt = array.GetSize();
	}
	return cnt;
}

bool SaveBookmark(CCmItemArray& array)
{
	CString str;
	BuildBookmarkFileName(str);
	bool ok = SaveCmItemArray(str, array);
	// ジャンプリスト更新
	CreateJumpList();
	return ok;
}

/////////////////////////////////////////////////////////////////////////////
// CFavoriteItemArray

int CFavoriteItemArray::Load()
{
	return LoadBookmark(*this);
}

void CFavoriteItemArray::Save()
{
	SaveBookmark(*this);
}

////////////////////////////////////////////////////////////////
//

CFavoWnd::CFavoWnd()
	: m_bShift(false)
	, m_bCtrl(false)
{
	CClientDC dc(NULL);
	HFONT hfont = dc.SelectFont(_GCDATA.GetMenuFont());
	TEXTMETRIC tm;
	dc.GetTextMetrics(&tm);
	m_cxChar = tm.tmMaxCharWidth;
	m_cyChar = tm.tmHeight + tm.tmExternalLeading;
	dc.SelectFont(hfont);
}

void CFavoWnd::MeasureItem(int nIDCtrl, LPMEASUREITEMSTRUCT pmi)
{
	CCmItem* pci = (CCmItem*)pmi->itemData;
	if (pci) {
		CClientDC dc(NULL);
		HFONT hfont = dc.SelectFont(_GCDATA.GetMenuFont());
		CSize csz;
		dc.GetTextExtent(pci->m_name, pci->m_name.GetLength(), &csz);
		dc.SelectFont(hfont);
		pmi->itemWidth  = csz.cx + m_cxChar + 16 + 12;
		pmi->itemHeight = max(_GCDATA.m_NcMetrics.iMenuHeight, MENU_OWNERDRAW_HEIGHT);
	}
}

void CFavoWnd::DrawItem(int nIDCtrl, LPDRAWITEMSTRUCT pdi)
{
	CDCHandle dc = pdi->hDC;
	if (pdi->itemAction & (ODA_DRAWENTIRE | ODA_SELECT)) {
		int iImage = -1;
		CString str;
		CCmItem* pci = (CCmItem*)pdi->itemData;
		if (pci) {
			str.Format(_T("%s\t&%C"), (LPCTSTR)pci->m_name, ((pci->m_id < 26) ? 'A' + pci->m_id : '0' + pci->m_id - 26));
			iImage = pci->GetImageIndex();
		}
		lccDrawCustomMenu(pdi, str, m_hImage, iImage, 0);
	}
	if (pdi->itemAction & ODA_FOCUS)
		dc.DrawFocusRect(&pdi->rcItem);
}

LRESULT CFavoWnd::MenuChar(UINT nChar, UINT nFlags, HMENU hMenu)
{
	if (0 < nChar && nChar <= 0x1a) {
		// Ctrl + Alphabet も対象
		nChar += '@';
	}

	int id;
	if (ISALPHA(nChar))
		id = ToUpper((TCHAR)nChar) - 'A';
	else if (isdigit(nChar))
		id = 26 + ToUpper((TCHAR)nChar) - '0';
	else
		return 0;

	int cnt = GetMenuItemCount(hMenu);

	for (int i = 0; i < cnt; i++) {
		CMenuItemInfo mii;
		mii.fMask = MIIM_DATA;
		GetMenuItemInfo(hMenu, i, TRUE, &mii);
		if (mii.dwItemData != NULL) {
			CCmItem* pci = (CCmItem*)mii.dwItemData;
			if (pci->m_id == id) {
				m_bShift = (GetKeyState(VK_SHIFT) < 0);
				m_bCtrl = (GetKeyState(VK_CONTROL) < 0);
				return MAKELPARAM(i, 2);
			}
		}
	}
	return 0;
}

LPITEMIDLIST TrackFavoriteMenu(HWND hOwner, bool bAddTabFolder, bool bAddCopyHistory, CPoint pt, int& openMode)
{
	CFavoriteItemArray arrItems;
	int num = arrItems.Load();

	CFavoWnd wnd;
	CMenu pop; pop.CreatePopupMenu();

	int i;
	CCmItem* pci;
	CString str;

	// しおりエントリをリストアップ
	CSimpleArray<HBITMAP> arrVistaBitmaps;
	num = min(26, num);
	for (i = 0; i < num; i++) {
		pci = arrItems[i];
		pci->m_id = i;
		if (!pci->isFolder())
			continue;
		str = pci->m_name;
		str.AppendFormat(_T("(&%C)"), i + 'A');
		pop.AppendMenu(MF_STRING, i + 1, str);
		HBITMAP hbm = CreateBitmapFromImageList(wnd.m_hImage, pci->GetImageIndex());
		if (hbm) {
			SetBitmapToMenuItem(pop, i + 1, hbm);
			arrVistaBitmaps.Add(hbm);
		}
	}

	// TABエントリをリストアップ
	if (bAddTabFolder) {
		int index = 0;
		while (1) {
			CIDListPtr idl = fvApp.GetTabsFolderIDList(index);
			if (idl.isNull())
				break;

			if (index == 0) {
				// セパレータ追加
				pop.AppendMenu(MF_SEPARATOR, (UINT_PTR)0, (LPCTSTR)NULL);
			}

			pci = new CCmItem(idl, NULL);
			pci->m_id = 26 + index;
			idl.Free();

			CString s = pci->m_name.GetString();
			s.AppendFormat(_T("(&%C)"), index + '0');
			pop.AppendMenu(MF_STRING, i + 1, s);
			HBITMAP hbm = CreateBitmapFromImageList(wnd.m_hImage, pci->GetImageIndex());
			if (hbm) {
				SetBitmapToMenuItem(pop, i + 1, hbm);
				arrVistaBitmaps.Add(hbm);
			}

			arrItems.Add(pci);
			i++;
			index++;
		}
	}

	// コピーヒストリをリストアップ
	// ここはオーナー描画メニューではない
	CSimpleArray<CString> arrCopyTo;
	int indexCopyToFirst = -1;
	if (bAddCopyHistory) {
		// まずセパレータ
		pop.AppendMenu(MF_SEPARATOR, (UINT_PTR)0, (LPCTSTR)NULL);

		CMenu sub;
		sub.CreatePopupMenu();
		int cnt = RegLoadHistory(arrCopyTo, IDS_SEC_HISTORY, IDS_KEY_COPYTARGET, MAXACCESS);
		if (cnt > 0) {
			indexCopyToFirst = i;
			cnt = min(cnt, 10);				// アクセスキー 0 ～ 9 の10個

			for (int n = 0; n < cnt; n++) {
				LPCTSTR pszPath = arrCopyTo[n].GetString();
				str.Format(_T("&%C %s"), n + '0', pszPath);
				sub.AppendMenu(MF_STRING, i + 1, str);
				CIDListPtr idl(pszPath);
				int iImage = idl_GetSysIconIndex(idl, PathIsUNC(pszPath));		// UNCなら簡易取得のアイコン
				HBITMAP hbm = CreateBitmapFromImageList(wnd.m_hImage, iImage);
				if (hbm) {
					SetBitmapToMenuItem(sub, i + 1, hbm);
					arrVistaBitmaps.Add(hbm);
				}
				i++;
			}
		}
		else {
			str.LoadString(IDS_UNDEFINED);
			sub.AppendMenu(MF_STRING | MF_GRAYED, (UINT_PTR)0, str);
		}
		str.LoadString(IDS_RECENT_COPYHISTORY);
		pop.AppendMenu(MF_POPUP, sub.Detach(), str);
	}

	int id = wnd.DoTrack(hOwner, pop, pt);
	LPITEMIDLIST pidlTarget = NULL;
	if (id > 0) {
		--id;
		if (indexCopyToFirst >= 0 && id >= indexCopyToFirst)
			pidlTarget = ILCreateFromPath(arrCopyTo[id - indexCopyToFirst]);
		else
			pidlTarget = ILClone(arrItems[id]->m_idl);
	}

	num = arrVistaBitmaps.GetSize();
	for (i = 0; i < num; i++) {
		HBITMAP hbm = arrVistaBitmaps[i];
		if (hbm)
			::DeleteObject(hbm);
	}
	arrVistaBitmaps.RemoveAll();
	arrItems.DeleteAll();

	// shift or ctrlキー併用
	if (!wnd.m_bShift && !wnd.m_bCtrl) {
		// マウスなどで選択した場合は情報取得ができていないので、ここで取得
		wnd.m_bShift = (GetKeyState(VK_SHIFT) < 0);
		wnd.m_bCtrl = (GetKeyState(VK_CONTROL) < 0);
	}

	if      (wnd.m_bCtrl)  openMode = FavoOpenByNewTab;			// タブで開く
	else if (wnd.m_bShift) openMode = FavoOpenByNewWindow;		// 新しいウィンドウで開く
	else                   openMode = FavoOpenDefault;
	return pidlTarget;
}
