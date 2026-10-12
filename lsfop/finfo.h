////////////////////////////////////////////////////////////////////////
// finfo.h

#pragma once
#ifndef __FINFO_H__
#define __FINFO_H__

/////////////////////////////////////////////////////////////////////
//

#ifndef OIC_WARNING
	#define OIC_WARNING				32515
#endif

#ifndef CLR_NONE
	#define CLR_NONE				0xFFFFFFFFL
#endif

#define CLR_ICONBACK				(GetSysColor(COLOR_3DFACE))
#define MAX_NAME_LENGTH				1024


/////////////////////////////////////////////////////////////////////
//

class CInfoListViewCtrl : public CWindowImpl<CInfoListViewCtrl, CListViewCtrl>
{
private:
	int m_iLastGroupId;
	CSimpleArray<bool> m_arrTypes;
	CFont m_FontMessage;

public:
	CInfoListViewCtrl()
		: m_iLastGroupId(-1)
	{
	}

	HFONT GetMessageFont()
	{
		if (m_FontMessage.IsNull()) {
			NONCLIENTMETRICS ncm;
			memset(&ncm, 0, sizeof(ncm));
			ncm.cbSize = RunTimeHelper::SizeOf_NONCLIENTMETRICS();
			SystemParametersInfo(SPI_GETNONCLIENTMETRICS, sizeof(ncm), &ncm, FALSE);
			m_FontMessage.CreateFontIndirect(&ncm.lfMessageFont);
		}
		return m_FontMessage;
	}

BEGIN_MSG_MAP(CInfoListViewCtrl)
	MESSAGE_HANDLER(WM_ERASEBKGND, OnHandlerEraseBkgnd)
	REFLECTED_NOTIFY_CODE_HANDLER(NM_RCLICK, OnNotifyRClick)
	REFLECTED_NOTIFY_CODE_HANDLER(NM_CUSTOMDRAW, OnNotifyCustomDraw)
END_MSG_MAP()

	LRESULT OnHandlerEraseBkgnd(UINT msg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
	{
		LRESULT lr = 0;
		bHandled = EraseBackground((HDC)wParam, &lr);
		return lr;
	}

	virtual BOOL EraseBackground(HDC hdc, LRESULT* pResult)
	{
		static const TCHAR sszNoInfomation[] = _T("表示する情報はありません。");

		if (GetItemCount() == 0) {
			CRect rc;
			GetClientRect(&rc);
			CDCHandle dc = hdc;
			dc.FillSolidRect(&rc, GetSysColor(COLOR_WINDOW));

			HFONT hfont = dc.SelectFont(GetMessageFont());
			TEXTMETRIC tm;
			dc.GetTextMetrics(&tm);
			rc.top += tm.tmHeight * 2;
			rc.bottom = rc.top + max(16, tm.tmHeight);

			HICON hic = (HICON)LoadImage(GetModuleHandle(_T("user32.dll")), MAKEINTRESOURCE(101), IMAGE_ICON, 16, 16, LR_DEFAULTCOLOR);
			CSize size;
			dc.GetTextExtent(sszNoInfomation, lstrlen(sszNoInfomation), &size);

			rc.left += (rc.Width() - size.cx - 16) / 2;
			dc.DrawIconEx(rc.left, rc.top + (rc.Height() - 16) / 2, hic, 0, 0, 0, NULL, DI_NORMAL);
			rc.left += 18;
			dc.DrawText(sszNoInfomation, -1, &rc, DT_LEFT | DT_VCENTER | DT_SINGLELINE | DT_NOPREFIX);

			DestroyIcon(hic);
			dc.SelectFont(hfont);

			return TRUE;
		}
		return FALSE;
	}

	void Initialize(HWND hwnd, LPCTSTR pszHeaders = NULL)
	{
		SubclassWindow(hwnd);

		EnableGroupView(TRUE);
		SetExtendedListViewStyle(LVS_EX_FULLROWSELECT, LVS_EX_FULLROWSELECT);

		if (pszHeaders) {
			LPCTSTR p = pszHeaders;
			int index = 0;
			while (p && *p) {
				InsertColumn(index, p, LVCFMT_LEFT, 80, index);
				p += lstrlen(p) + 1;
				index++;
			}
		}
		else {
			InsertColumn(0, _T("項目"), LVCFMT_LEFT, 80, 0);
			InsertColumn(1, _T("内容"), LVCFMT_LEFT, 80, 1);
		}
	}

	void ResetItems()
	{
		m_iLastGroupId = -1;
		DeleteAllItems();
	}

	int AddItemOrGroup(bool bGroup, LPCTSTR pszTitle, LPCTSTR pszText, LPARAM param = NULL)
	{
		// pszTitle, pszTextは IDS_xxx のリソース文字列でもOK

		// アイテム情報初期化
		LVITEM item;
		memset(&item, 0, sizeof(item));
		item.lParam = param;
		item.mask = LVIF_TEXT | LVIF_INDENT | LVIF_PARAM | LVIF_STATE | LVIF_GROUPID;
		item.iIndent = 1;

		CString tit(pszTitle);
		CString str;
		int index = 0;
		if (bGroup) {
			LVGROUP group;
			memset(&group, 0, sizeof(group));
#ifdef _WIN64
			group.cbSize = sizeof(group);
#else
			group.cbSize = OsVISTA() ? sizeof(group) : LVGROUP_V5_SIZE;
#endif
			group.mask = LVGF_GROUPID | LVGF_HEADER | LVGF_STATE | LVGF_ALIGN;
			group.stateMask = LVGS_NORMAL | LVGS_COLLAPSIBLE;
			group.state = LVGS_NORMAL | LVGS_COLLAPSIBLE;
			group.uAlign = LVGA_HEADER_LEFT | LVGA_FOOTER_LEFT;

			group.iGroupId = ++m_iLastGroupId;
			group.pszHeader = (LPTSTR)tit.GetString();
			group.cchHeader = lstrlen(group.pszHeader);
			AddGroup(&group);
			return 0;
		}

		item.iGroupId = m_iLastGroupId;
		item.iItem = GetItemCount();
		item.pszText = (LPTSTR)tit.GetString();
		index = InsertItem(&item);

		CString value(pszText);
		if (!value.IsEmpty())
			SetItemText(index, 1, value.GetString());

		// このインデクスがグループかそうでないかを記録
		m_arrTypes.Add(bGroup);
		return index;
	}

	LRESULT OnNotifyCustomDraw(int idCtrl, NMHDR* phdr, BOOL& bHandled)
	{
		NMLVCUSTOMDRAW* pcd = (NMLVCUSTOMDRAW*)phdr;
		LRESULT lr = CDRF_DODEFAULT;
		switch (pcd->nmcd.dwDrawStage) {
		case CDDS_PREPAINT:
			lr = CDRF_NOTIFYITEMDRAW;
			break;
		case CDDS_ITEMPREPAINT:
			if (pcd->nmcd.dwItemSpec & 1)
				lr = CDRF_DODEFAULT;
			else {
				pcd->clrTextBk = calc_mixed_color(GetSysColor(COLOR_WINDOW), GetSysColor(COLOR_HIGHLIGHT), 94);
				lr = CDRF_NEWFONT;
			}
			break;
		default:
			lr = CDRF_DODEFAULT;
			break;
		}
		return lr;
	}

	LRESULT OnNotifyRClick(int idCtrl, NMHDR* phdr, BOOL& bHandled)
	{
		CPoint pt;
		GetCursorPos(&pt);
		ScreenToClient(&pt);
		LV_HITTESTINFO hit;
		hit.pt = pt;
		HitTest(&hit);

		if (hit.flags & LVHT_ONITEM) {
			CMenu pop;
			pop.CreatePopupMenu();
			pop.AppendMenu(MF_STRING, 1, _T("コピー(&C)"));
			ClientToScreen(&pt);
			int cmd = pop.TrackPopupMenu(TPM_LEFTALIGN | TPM_RETURNCMD | TPM_RIGHTBUTTON, pt.x, pt.y, m_hWnd);
			if (cmd == 1) {
				CString str;
				if (GetItemText(hit.iItem, 1, str) > 0)
					lccSetStringToClipboard(m_hWnd, str);
			}
		}
		return 0;
	}

	void AutoSizeColumns()
	{
		int numCol = GetHeader().GetItemCount();
		for (int i = 0; i < numCol; i++)
			SetColumnWidth(i, LVSCW_AUTOSIZE);
	}
};

#endif		// __FINFO_H__
