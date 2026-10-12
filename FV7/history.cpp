////////////////////////////////////////////////////////////
// class CHistory

#include "stdafx.h"
#include <commoncontrols.h>
#include "fv7.h"
#include "history.h"

void CHistory::PushItem(LPCITEMIDLIST pidl)
{
	if (!pidl) return;

	if (IsUpdateDisabled()) {
		// 更新無効になっていれば、更新を有効にして戻る
		DisableUpdate(false);
		return;
	}
	int cnt = (int)GetCount();
	CCmItem* pci;
	if (cnt > 0) {
		pci = (*this)[m_posCur];
		if (pci->m_idl.isEqual(pidl)) {
			return;
		}
	}

	// 履歴の途中に追加の場合は、先頭から指針位置のデータを削除
	int i;
	if (0 < m_posCur && m_posCur < cnt - 1) {
		for (i = 0; i < m_posCur - 1; i++) {
			pci = (*this)[i];
			delete pci;
		}
		RemoveAt(0, m_posCur);
		m_posCur = 0;
	}

	pci = new CCmItem(pidl, NULL);
	pci->SetPathDisplayName();
	pci->SetTime();							// 現在時刻をセット
	InsertAt(0, pci);

	int numOver = (int)GetCount() - m_nLimit;
	if (numOver > 0) {
		// 保持可能最大数を超えたとき
		for (i = m_nLimit; i < m_nLimit + numOver; i++) {
			pci = (*this)[i];
			if (pci) delete pci;
		}
		RemoveAt(m_nLimit, numOver);
	}
}

int CHistory::TrackMenu(HWND hParent, CSimpleArray<int>& array, CPoint pt, BOOL bAddClear, int posCur)
{
	// ※array配列の中身はここでデリートされるので注意

	int cnt = array.GetSize();
	if (cnt < 1) return NULL;

	CMenu pop;
	pop.CreatePopupMenu();

	HIMAGELIST him;
	Shell_GetImageLists(NULL, &him);

	CWaitCursor wc;
	CCmItem* pci;
	int i;
	for (i = 0; i < cnt; i++) {
		UINT id = i + IDM_HISTORY_START;
		pci = (*this)[array[i]];
		UINT flags = 0;
		if (i == posCur)
			flags |= MF_CHECKED;
		pop.AppendMenu(flags, id, pci->m_name.GetString());

		CMenuItemInfo mii;
		mii.fMask = MIIM_BITMAP;
		if (i != m_posCur) {
			mii.hbmpItem = CreateBitmapFromImageList(him, pci->GetImageIndex());
			pop.SetMenuItemInfo(id, false, &mii);
		}
/* 独自にアイコンをコールバックで描く
		else {
			mii.hbmpItem = HBMMENU_CALLBACK;
			pop.SetMenuItemInfo(id, false, &mii);
		}
*/
	}

/* メニュー項目の折り返し
	if (cnt >= 18) {
		int sep = (cnt >> 1) + (cnt & 1);
		CMenuItemInfo mii;
		mii.fState = MIIM_TYPE;
		GetMenuItemInfo(pop.m_hMenu, sep, TRUE, &mii);
		mii.fType |= MFT_MENUBARBREAK;
		SetMenuItemInfo(pop.m_hMenu, sep, TRUE, &mii);
	}
*/
	CCmItem ciClear((LPCITEMIDLIST)NULL, NULL);		// 履歴クリア用ダミー項目
	if (bAddClear) {
		ciClear.m_name.LoadString(IDS_CLEARHISTORY);
		pop.AppendMenu(MF_SEPARATOR, (UINT_PTR)0, (LPCTSTR)NULL);
		pop.AppendMenu(0, 1, ciClear.m_name.GetString());
	}
	wc.Restore();

	CIDListWnd wnd;
	int sel = wnd.DoTrack(hParent, pop, pt);

	int index = -1;
	if (sel == 1) {
//@		Clear();
		// ナビバーの履歴の削除に対応するため、処理はメインウィンドウでおこなう
		::SendMessage(hParent, WM_COMMAND, IDM_DIR_HISTORYCLEAR, 0);
	} else if (sel > 1) {
		index = array[sel - IDM_HISTORY_START];
		m_posCur = index;
	}

	return index;
}

LPITEMIDLIST CHistory::HistoryMenu(HWND hParent, CPoint pt)
{
	int num = (int)GetCount();
	if (num < 1) return NULL;

	CSimpleArray<int> array;
	for (int i = 0; i < num; i++) {
		array.Add(i);
	}

	LPITEMIDLIST pidl = NULL;
	num = TrackMenu(hParent, array, pt, TRUE, m_posCur);
	if (num >= 0) {
		pidl = (*this)[num]->m_idl;
	}
	return pidl;
}

LPITEMIDLIST CHistory::BackwardHistoryMenu(HWND hParent, CPoint pt)
{
	int num = (int)GetCount();
	if (num < 1 || m_posCur < 1) return NULL;

	CSimpleArray<int> array;
	for (int i = m_posCur; i < num; i++) {
		array.Add(i);
	}

	LPITEMIDLIST pidl = NULL;
	num = TrackMenu(hParent, array, pt);
	if (num >= 0) {
		pidl = (*this)[num]->m_idl;
	}
	return pidl;
}

LPITEMIDLIST CHistory::ForwardHistoryMenu(HWND hParent, CPoint pt)
{
	int num = (int)GetCount();
	if (num < 1 || m_posCur >= num - 1) return NULL;

	CSimpleArray<int> array;
	for (int i = 0; i < m_posCur; i++) {
		array.Add(i);
	}

	LPITEMIDLIST pidl = NULL;
	num = TrackMenu(hParent, array, pt);
	if (num >= 0) {
		pidl = (*this)[num]->m_idl;
	}
	return pidl;
}

