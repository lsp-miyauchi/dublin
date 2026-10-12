// gradlist.cpp : インプリメンテーション ファイル
//

#include "stdafx.h"
#include "gradlist.h"
#include "common.h"

#ifdef _DEBUG
//#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#define GRADICON_WIDTH		80
#define GRADICON_HEIGHT		16

static void FillShadeRect(HDC hdc, CRect& rc, COLORREF crFrom, COLORREF crTo)
{
	TRIVERTEX vert[2];
	GRADIENT_RECT grc;

	vert[0].x     = rc.left;
	vert[0].y     = rc.top;
	vert[0].Red   = GetRValue(crFrom) << 8;
	vert[0].Green = GetGValue(crFrom) << 8;
	vert[0].Blue  = GetBValue(crFrom) << 8;
	vert[0].Alpha = 0x0000;

	vert[1].x     = rc.right;
	vert[1].y     = rc.bottom;
	vert[1].Red   = GetRValue(crTo) << 8;
	vert[1].Green = GetGValue(crTo) << 8;
	vert[1].Blue  = GetBValue(crTo) << 8;
	vert[1].Alpha = 0x0000;

	grc.UpperLeft  = 0;
	grc.LowerRight = 1;
	GradientFill(hdc, vert, 2, &grc, 1, GRADIENT_FILL_RECT_H);
}

static HBITMAP make_gradbitmap(int cx, int cy, CGradItem* pgi)
{
	CClientDC dc(NULL);
	HBITMAP hbm = CreateCompatibleBitmap(dc, cx, cy);
	if (!hbm)
		return NULL;

	CDC dcw;
	dcw.CreateCompatibleDC(dc);
	hbm = dcw.SelectBitmap(hbm);

	CRect rc(0, 0, cx, cy);
	int posFrom = 0;
	COLORREF crFrom = pgi->m_grad[0].cr;
	double scale = (double)cx / 1000.0;

	CRect rci = rc;
	for (int i = 0; i < pgi->m_colors; i++) {
		rci.left = rc.left + (int)(scale * posFrom);
		rci.right = rc.left + (int)(scale * pgi->m_grad[i].pos);
		if (rci.Width() > 0) {
			if (crFrom != pgi->m_grad[i].cr)
				FillShadeRect(dcw, rci, crFrom, pgi->m_grad[i].cr);
			else
				dcw.FillSolidRect(&rci, crFrom);
		}
		posFrom = pgi->m_grad[i].pos;
		crFrom = pgi->m_grad[i].cr;
	}

	if (posFrom < 1000) {
		rci.left = rc.left + (int)(scale * posFrom);
		rci.right = rc.right;
		dcw.FillSolidRect(&rci, crFrom);
	}

	return dcw.SelectBitmap(hbm);
}

static BOOL make_gradlist_file(LPCTSTR pszName)
{
	HINSTANCE hInst = RESINST;
	HRSRC hrsrc = FindResource(hInst, _MKIR(IDT_GRADLIST), RT_RCDATA);
	if (hrsrc) {
		DWORD sizeRes = SizeofResource(hInst, hrsrc);
		if (sizeRes > 0) {
			HGLOBAL hglb = LoadResource(hInst, hrsrc);
			if (hglb) {
				void* pr = LockResource(hglb);
				if (pr) {
					HANDLE hf = CreateFile(pszName, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_ARCHIVE, NULL);
					if (hf != INVALID_HANDLE_VALUE) {
						DWORD tmp;
						WriteFile(hf, pr, sizeRes, &tmp, NULL);
						CloseHandle(hf);
						return TRUE;
					}
				}
			}
		}
	}
	return FALSE;
}

/////////////////////////////////////////////////////////////////////////////
// CGradListDlg ダイアログ

BOOL CGradListDlg::OnInitDialog(LPARAM lParam)
{
	CenterWindow();
	m_list = GetDlgItem(IDC_LIST);

	GradEdit_SetNumLimit(GetDlgItem(IDC_GRADBAR), MAX_GRADCOLORS);
	GradEdit_SetColors(GetDlgItem(IDC_GRADBAR), m_colors, m_grad);

	m_list.InsertColumn(0, _T(""), LVCFMT_LEFT, 100, 0);
	HIMAGELIST him = ImageList_Create(GRADICON_WIDTH, GRADICON_HEIGHT, ILC_COLORDDB, 20, 4);
	m_list.SetImageList(him, LVSIL_SMALL);

	m_lock = lccLoadImageList(RESINST, RT_RCDATA, MAKEINTRESOURCE(IDB_LOCK), 14, RGB(255, 0, 255));
	m_list.SetImageList(m_lock, LVSIL_STATE);

	// グラデーションリストファイル
	TCHAR buf[MAX_PATH * 2];
	GetDataFolder(buf);
	PathAppend(buf, _T("gradient.dat"));
	m_csPreset = buf;
	if (!PathFileExists(m_csPreset)) {
		if (!make_gradlist_file(m_csPreset)) {
			EndDialog(IDCANCEL);
			return TRUE;
		}
	}

	int cnt = GetPrivateProfileInt(_T("Order"), _T("OrderCount"), 0, m_csPreset);
	if (cnt < 1) {
		EndDialog(IDCANCEL);
		return TRUE;
	}

	m_bInitialize = TRUE;
	for (int i = 0; i < cnt; i++) {
		TCHAR key[MAX_PATH];
		wsprintf(key, _T("%d"), i);
		if (GetPrivateProfileString(_T("Order"), key, _T(""), buf, _COUNTOF(buf), m_csPreset) < 1)
			continue;

		int items = (int)GetPrivateProfileInt(buf, _T("segments"), 0, m_csPreset);
		items = min(MAX_GRADCOLORS, max(0, items));
		if (items < 1)
			continue;

		CGradItem* pgi = new CGradItem();
		pgi->m_name = buf;
		pgi->m_colors = items;
		for (int i = 0; i < items; i++) {
			TCHAR item[MAX_PATH];
			wsprintf(key, _T("item%d"), i);
			GetPrivateProfileString(buf, key, _T(""), item, _COUNTOF(buf), m_csPreset);

			int pos, r, g, b;
			_stscanf_s(item, _T("%d,%d,%d,%d"), &pos, &r, &g, &b);
			pgi->m_grad[i].pos = pos;
			pgi->m_grad[i].cr = RGB(r, g, b);
		}
		pgi->m_fixed = (BOOL)GetPrivateProfileInt(buf, _T("fixed"), 0, m_csPreset);

		HBITMAP hbm = make_gradbitmap(GRADICON_WIDTH, GRADICON_HEIGHT, pgi);
		int index = ImageList_Add(him, hbm, NULL);
		DeleteObject(hbm);

		index = m_list.InsertItem(m_list.GetItemCount(), pgi->m_name, index);
		m_list.SetItemData(index, (DWORD_PTR)pgi);
		if (pgi->m_fixed)
			m_list.SetItemState(index, INDEXTOSTATEIMAGEMASK(2), LVIS_STATEIMAGEMASK);
	}
	m_list.SetColumnWidth(0, LVSCW_AUTOSIZE);
	m_bInitialize = FALSE;
	return TRUE;
}

static TCHAR sszNewName[] = _T("新しいグラデーション");

void CGradListDlg::BuildNewItemName(LPCTSTR pszPrefix, CString& csNewName)
{
	LVFINDINFO lvfi;
	lvfi.flags = LVFI_STRING;
	lvfi.psz = pszPrefix;
	if (m_list.FindItem(&lvfi, -1) < 0)
		csNewName = pszPrefix;
	else {
		TCHAR newname[MAX_PATH];
		lvfi.psz = newname;
		for (int i = 1; i < 999; i++) {
			wsprintf(newname, _T("%s(%d)"), pszPrefix, i);
			if (m_list.FindItem(&lvfi, -1) < 0) {
				csNewName = newname;
				break;
			}
		}
	}
}

void CGradListDlg::OnItemAdd() 
{
	CGradItem* pgi = new CGradItem();
	pgi->m_colors = GradEdit_GetColors(GetDlgItem(IDC_GRADBAR), pgi->m_grad);
	BuildNewItemName(sszNewName, pgi->m_name);

	HIMAGELIST him = m_list.GetImageList(LVSIL_SMALL);
	HBITMAP hbm = make_gradbitmap(GRADICON_WIDTH, GRADICON_HEIGHT, pgi);
	int index = ImageList_Add(him, hbm, NULL);
	DeleteObject(hbm);

	index = m_list.InsertItem(m_list.GetItemCount(), pgi->m_name, index);
	m_list.SetItemData(index, (DWORD_PTR)pgi);

	m_list.SetColumnWidth(0, LVSCW_AUTOSIZE);
	m_list.EnsureVisible(index, FALSE);
	m_list.SetItemState(index, LVIS_SELECTED|LVIS_FOCUSED, LVIS_SELECTED|LVIS_FOCUSED);

	UpdatePreset(NULL, pgi);

	m_list.SetFocus();
	m_list.EditLabel(index);
}

void CGradListDlg::OnItemCopy() 
{
	int index = m_list.GetSelectedIndex();
	if (index >= 0) {
		CGradItem* pgi = (CGradItem*)m_list.GetItemData(index);
		CGradItem* pgiNew = new CGradItem();
		pgiNew->m_colors = pgi->m_colors;
		CopyMemory(pgiNew->m_grad, pgi->m_grad, sizeof(GRADCOLORS) * pgi->m_colors);
		BuildNewItemName(pgi->m_name, pgiNew->m_name);

		LVITEM lvi;
		lvi.mask = LVIF_IMAGE;
		lvi.iItem = index;
		lvi.iSubItem = 0;
		m_list.GetItem(&lvi);

		index = m_list.InsertItem(index + 1, pgiNew->m_name, lvi.iImage);
		m_list.SetItemData(index, (DWORD_PTR)pgiNew);
		m_list.SetColumnWidth(0, LVSCW_AUTOSIZE);
		m_list.EnsureVisible(index, FALSE);
		m_list.SetItemState(index, LVIS_SELECTED|LVIS_FOCUSED, LVIS_SELECTED|LVIS_FOCUSED);

		UpdatePreset(NULL, pgiNew);

		m_list.SetFocus();
		m_list.EditLabel(index);
	}
}

void CGradListDlg::OnItemDelete() 
{
	int index = m_list.GetSelectedIndex();
	if (index >= 0) {
		CGradItem* pgi = (CGradItem*)m_list.GetItemData(index);
		if (pgi->m_fixed) {
			AppMessage(IDR_FRAME, IDS_GRADATION_LOCKED, pgi->m_name);
			return;
		}

		if (AppMessage(IDR_FRAME, IDS_GRADATION_DELETE, pgi->m_name, MB_QYESNO) == IDYES) {
			UpdatePreset(pgi->m_name, NULL);
			m_list.DeleteItem(index);
		}
	}
}

void CGradListDlg::OnItemRename() 
{
	int index = m_list.GetSelectedIndex();
	if (index >= 0) {
		CGradItem* pgi = (CGradItem*)m_list.GetItemData(index);
		if (pgi->m_fixed) {
			AppMessage(IDR_FRAME, IDS_GRADATION_LOCKED, pgi->m_name);
			return;
		}

		m_list.SetFocus();
		m_list.EditLabel(index);
	}
}

void CGradListDlg::OnItemUpdate() 
{
	int index = m_list.GetSelectedIndex();
	if (index >= 0) {
		CGradItem* pgi = (CGradItem*)m_list.GetItemData(index);
		if (pgi->m_fixed) {
			AppMessage(IDR_FRAME, IDS_GRADATION_LOCKED, pgi->m_name);
			return;
		}

		pgi->m_colors = GradEdit_GetColors(GetDlgItem(IDC_GRADBAR), pgi->m_grad);

		HIMAGELIST him = m_list.GetImageList(LVSIL_SMALL);
		HBITMAP hbm = make_gradbitmap(GRADICON_WIDTH, GRADICON_HEIGHT, pgi);
		int iImage = ImageList_Add(him, hbm, NULL);
		DeleteObject(hbm);

		LVITEM lvi;
		lvi.mask = LVIF_IMAGE;
		lvi.iItem = index;
		lvi.iImage = iImage;
		lvi.iSubItem = 0;
		m_list.SetItem(&lvi);

		UpdatePreset(NULL, pgi);
	}
}

void CGradListDlg::OnItemLock()
{
	int index = m_list.GetSelectedIndex();
	if (index >= 0) {
		CGradItem* pgi = (CGradItem*)m_list.GetItemData(index);
		pgi->m_fixed = pgi->m_fixed ? FALSE : TRUE;

		m_list.SetItemState(index,
			INDEXTOSTATEIMAGEMASK((pgi->m_fixed ? 2 : 1)), LVIS_STATEIMAGEMASK);

		UpdatePreset(NULL, pgi);
	}
}

void CGradListDlg::OnNotifyClick(NMHDR* pNMHDR, LRESULT* pResult)
{
	CPoint pt;
	GetCursorPos(&pt);
	m_list.ScreenToClient(&pt);
	UINT nFlags;
	int index = m_list.HitTest(pt, &nFlags);
	if (index >= 0 && nFlags & LVHT_ONITEMSTATEICON) {
		CGradItem* pgi = (CGradItem*)m_list.GetItemData(index);
		pgi->m_fixed = pgi->m_fixed ? FALSE : TRUE;
		m_list.SetItemState(index, INDEXTOSTATEIMAGEMASK((pgi->m_fixed ? 2 : 1)), LVIS_STATEIMAGEMASK);

		UpdatePreset(NULL, pgi);
	}
}

void CGradListDlg::OnNotifyDblClick(NMHDR* pNMHDR, LRESULT* pResult)
{
	CPoint pt;
	GetCursorPos(&pt);
	m_list.ScreenToClient(&pt);
	UINT nFlags;
	if (m_list.HitTest(pt, &nFlags) >= 0 && nFlags & (LVHT_ONITEMICON | LVHT_ONITEMLABEL))
		PostMessage(WM_COMMAND, MAKELONG(IDOK, BN_CLICKED), NULL);
}

void CGradListDlg::OnNotifyItemChanged(NMHDR* pNMHDR, LRESULT* pResult) 
{
	NM_LISTVIEW* pnmlv = (NM_LISTVIEW*)pNMHDR;

	CGradItem* pgi = (CGradItem*)pnmlv->lParam;
	UINT chg = pnmlv->uNewState ^ pnmlv->uOldState;
	if (!m_bInitialize && pgi && chg & LVIS_SELECTED) {
		GradEdit_SetColors(GetDlgItem(IDC_GRADBAR), pgi->m_colors, pgi->m_grad);
		m_colors = pgi->m_colors;
		memcpy(m_grad, pgi->m_grad, sizeof(GRADCOLORS) * m_colors);
	}
	*pResult = 0;
}

void CGradListDlg::OnNotifyBeginLabelEdit(NMHDR* pNMHDR, LRESULT* pResult) 
{
	LV_DISPINFO* pdi = (LV_DISPINFO*)pNMHDR;
	CGradItem* pgi = (CGradItem*)m_list.GetItemData(pdi->item.iItem);
	*pResult = pgi->m_fixed;
	if (pgi->m_fixed)
		MessageBeep(0);
}

void CGradListDlg::OnNotifyEndLabelEdit(NMHDR* pNMHDR, LRESULT* pResult) 
{
	LV_DISPINFO* pdi = (LV_DISPINFO*)pNMHDR;
	*pResult = 0;

	CGradItem* pgi = (CGradItem*)m_list.GetItemData(pdi->item.iItem);
	if (pgi &&
		pdi->item.pszText &&
		*pdi->item.pszText &&
		StrCmp(pdi->item.pszText, pgi->m_name))
	{
		if (pgi->m_fixed) {
			AppMessage(IDR_FRAME, IDS_GRADATION_LOCKED, pgi->m_name);
			return;
		}

		if (StrChr(pdi->item.pszText, '[') || StrChr(pdi->item.pszText, ']')) {
			AppMessage(IDR_FRAME, IDS_GRADATION_INVALIDNAME, pdi->item.pszText, MB_STOP);
			return;
		}

		CString cs = pgi->m_name;
		pgi->m_name = pdi->item.pszText;
		m_list.SetItemText(pdi->item.iItem, 0, pdi->item.pszText);
		UpdatePreset(cs, pgi);
	}
}

void CGradListDlg::OnNotifyDeleteItem(NMHDR* pNMHDR, LRESULT* pResult) 
{
	NM_LISTVIEW* pnmlv = (NM_LISTVIEW*)pNMHDR;
	CGradItem* pgi = (CGradItem*)m_list.GetItemData(pnmlv->iItem);
	if (pgi)
		delete pgi;
	*pResult = 0;
}

void CGradListDlg::UpdatePreset(LPCTSTR pszOldName, CGradItem* pgi)
{
	if (pszOldName)
		WritePrivateProfileStruct(pszOldName, NULL, NULL, 0, m_csPreset);

	int len;
	CAutoVectorPtr<TCHAR> keys;
	keys.Allocate(4096);
	if (pgi) {
		len = 0;
		for (int i = 0; i < pgi->m_colors; i++) {
			len += wsprintf(&keys[len], _T("item%d=%d,%d,%d,%d"),
							i,
							pgi->m_grad[i].pos,
							GetRValue(pgi->m_grad[i].cr),
							GetGValue(pgi->m_grad[i].cr),
							GetBValue(pgi->m_grad[i].cr)) + 1;
		}
		len += wsprintf(&keys[len], _T("segments=%d"), pgi->m_colors) + 1;
		if (pgi->m_fixed)
			len += wsprintf(&keys[len], _T("fixed=%d"), pgi->m_fixed) + 1;
		keys[len] = '\0';
		WritePrivateProfileSection(pgi->m_name, keys, m_csPreset);
	}

	int cnt = m_list.GetItemCount();
	len = wsprintf(keys, _T("OrderCount=%d"), cnt) + 1;
	for (int index = 0; index < cnt; index++) {
		CGradItem* pgi = (CGradItem*)m_list.GetItemData(index);
		len += wsprintf(&keys[len], _T("%d=%s"), index, pgi->m_name) + 1;
	}
	keys[len] = '\0';
	WritePrivateProfileSection(_T("Order"), keys, m_csPreset);
}

void CGradListDlg::OnGradChanged() 
{
	m_colors = GradEdit_GetColors(GetDlgItem(IDC_GRADBAR), m_grad);
}

void CGradListDlg::OnReverseMap() 
{
	GradEdit_ReverseMap(GetDlgItem(IDC_GRADBAR));
}

////////////////////////////////////////////////////////////////////////
//

BOOL GradationEdit(HWND hwnd, GRADEDITINFO* pi)
{
	if (!pi) return FALSE;

	CGradListDlg dlg;
	dlg.m_colors = pi->numcolors;
	memcpy(dlg.m_grad, pi->grad, pi->numcolors * sizeof(GRADCOLORS));
	if (dlg.DoModal(hwnd) == IDOK) {
		pi->numcolors = dlg.m_colors;
		memcpy(pi->grad, dlg.m_grad, sizeof(GRADCOLORS) * pi->numcolors);
		return TRUE;
	}
	return FALSE;
}
