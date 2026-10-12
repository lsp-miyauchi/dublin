#pragma once

#define SBPIF_LEFTALIGN		0x00000000
#define SBPIF_RIGHTALIGN	0x00000001
#define SBPIF_CENTERALIGN	0x00000002
#define SBPIF_ALIGNMASK		0x0000000F
#define SBPIF_AUTOWIDTH		0x00000010
#define SBPIF_CHECK			0x00000020
#define SBPIF_BUTTON		0x00001000
#define SBPIF_PULLUP		0x00002000
#define SBPIF_HIDDEN		0x00080000

///////////////////////////////////////////////////////////
// CStatusBarPaneItem

struct _StatusBarPaneItem {
	int paneId;
	UINT flag;
	int iImage;
	TCHAR label[160];
};

class CStatusBarPaneItem : public _StatusBarPaneItem
{
public:
	CStatusBarPaneItem()
	{
		flag = 0;
		paneId = -1;
		iImage = -1;
		label[0] = 0;
	}
	CStatusBarPaneItem(int itemId, UINT itemFlag = 0, int imageIndex = -1, LPCTSTR str = NULL)
	{
		paneId = itemId;
		flag = itemFlag;
		iImage = imageIndex;
		if (str) {
			StrCpyN(label, str, ARRAYSIZE(label));
		}
		else {
			label[0] = 0;
		}
	}
	CStatusBarPaneItem(const _StatusBarPaneItem& src)
	{
		CopyFrom(src);
	}
	~CStatusBarPaneItem()
	{}

	CStatusBarPaneItem& CopyFrom(const _StatusBarPaneItem& src)
	{
		paneId = src.paneId;
		flag = src.flag;
		iImage = src.iImage;
		StrCpy(label, src.label);
		return *this;
	}

	_inline BOOL isNull() const { return paneId < 0; }
	_inline BOOL hasLabel() const { return lstrlen(label) > 0; }
	_inline BOOL isChecked() const { return (flag & SBPIF_CHECK) != 0; }
	_inline BOOL isButton() const { return (flag & SBPIF_BUTTON) != 0; }
	_inline BOOL isPullup() const { return (flag & SBPIF_PULLUP) != 0; }
	_inline BOOL isHidden() const { return (flag & SBPIF_HIDDEN) != 0; }
	_inline BOOL isAutoWidth() const { return (flag & SBPIF_AUTOWIDTH) != 0; }
	_inline BOOL isSameLabel(LPCTSTR str) const { return StrCmp(label, str) == 0; }

	BOOL operator==(const _StatusBarPaneItem& src) const {
		return paneId == src.paneId &&
			!StrCmp(label, src.label) &&
			flag == src.flag &&
			iImage == src.iImage;
	}
	BOOL operator!=(const _StatusBarPaneItem& src) const {
		return paneId != src.paneId ||
			StrCmp(label, src.label) ||
			flag != src.flag ||
			iImage != src.iImage;
	}
	CStatusBarPaneItem& operator=(const _StatusBarPaneItem& src)
	{
		return CopyFrom(src);
	}
	operator _StatusBarPaneItem*()
	{
		return this;
	}
	operator const _StatusBarPaneItem*() const
	{
		return this;
	}
	void SetLabel(LPCTSTR str)
	{
		if (str) {
			StrCpyN(label, str, ARRAYSIZE(label));
		}
		else {
			label[0] = 0;
		}
	}
};

///////////////////////////////////////////////////////////
// CLspStatusBarCtrl

// --- pane inner margin structure ----------
// m_cxPaneMargin|m_cxIconPadding|<icon=m_sizeIcon.cx>|m_cxIconPadding|<text>|m_cxPullupSpace|m_cxPaneMargin
//
class CLspStatusBarCtrl : public CMultiPaneStatusBarCtrlImpl<CLspStatusBarCtrl>
{
public:
	struct _ColorPair {
		COLORREF text;
		COLORREF bkgnd;
	};

protected:
	_ColorPair m_clr;
	_ColorPair m_clrChecked;

	CSimpleMap<int, CStatusBarPaneItem*> m_mapStockItem;
	CFont m_fontStatus;
	CFont m_fontSymbol;
	int m_nHoverPane;
	bool m_bTrackMouseEvent;
	CImageList m_image;
	CSize m_sizeIcon;
	int m_cxPullupSpace;

	enum {
		m_cxIconPadding = 2,
	};


public:
	DECLARE_WND_SUPERCLASS(_T("LSP_StatusBar"), GetWndClassName())

	CLspStatusBarCtrl() : m_nHoverPane(-1), m_bTrackMouseEvent(false)
	{
		NONCLIENTMETRICSW ncm = { sizeof(ncm) };
		SystemParametersInfo(SPI_GETNONCLIENTMETRICS, sizeof(ncm), (void*)&ncm, 0);
		m_fontStatus.CreateFontIndirect(&ncm.lfStatusFont);
		// シンボルフォント
		LOGFONT lf;
		memcpy(&lf, &ncm.lfStatusFont, sizeof(lf));
		StrCpy(lf.lfFaceName, _T("Webdings"));
		m_fontSymbol.CreateFontIndirect(&lf);
		CClientDC dc(NULL);
		CFontHandle font = dc.SelectFont(m_fontSymbol);
		CSize s;
		dc.GetTextExtent(_T("5"), -1, &s);
		dc.SelectFont(font);
		m_cxPullupSpace = s.cx + 14;

		ApplyColor(GetSysColor(COLOR_MENUTEXT), GetSysColor(COLOR_MENUBAR));
	}
	~CLspStatusBarCtrl()
	{
		m_image.Detach();
		SweepAll();
	}

	/*
	 * インジケータの全データを消去
	 */
	void SweepAll() {
		for (int i = 0; i < m_mapStockItem.GetSize(); i++) {
			CStatusBarPaneItem* item = m_mapStockItem.GetValueAt(i);
			delete item;
		}
		m_mapStockItem.RemoveAll();
	}

protected:
	/*
	 * 位置インデクスからインジケータidを取得
	 */
	_inline int _posIndex2paneId(int pos)
	{
		int itemId = (pos < 0 || pos >= m_nPanes) ? -1 : m_pPane[pos];
		return itemId < 0 ? itemId : m_mapStockItem.FindKey(itemId);
	}
	/*
	 * インジケータidから位置インデクスを取得
	 */
	_inline int _paneId2posIndex(int itemId)
	{
		for (int i = 0; i < m_nPanes; i++) {
			if (m_pPane[i] == itemId)
				return i;
		}
		return -1;
	}

	CStatusBarPaneItem* GetStockItem(int itemId) const
	{
		int index = m_mapStockItem.FindKey(itemId);
		if (index < 0) {
			return NULL;
		}
		return m_mapStockItem.GetValueAt(index);
	}

	void ApplyColor(COLORREF clr, COLORREF bkclr)
	{
		m_clr.text = clr;
		m_clr.bkgnd = bkclr;
		m_clrChecked.bkgnd = ColorAdjustLuma(m_clr.bkgnd, isColorDarken(m_clr.bkgnd) ? 200 : -200, TRUE);
		m_clrChecked.text = isColorDarken(m_clrChecked.bkgnd) ? RGB(255, 255, 255) : RGB(0, 0, 0);
	}

public:
	/*
	 * イメージリストのセット
	 */
	HIMAGELIST SetImageList(HIMAGELIST him)
	{
		HIMAGELIST himT = m_image.Detach();
		m_image.Attach(him);
		if (m_image.IsNull()) {
			m_sizeIcon = CSize(0, 0);
		}
		else {
			m_image.GetIconSize(m_sizeIcon);
		}
		for (int i = 0; i < m_mapStockItem.GetSize(); i++) {
			CStatusBarPaneItem* item = m_mapStockItem.GetValueAt(i);
			if (item->iImage >= 0) {
				RedrawPaneItem(item->paneId);
			}
		}
		return himT;
	}

	/*
	 * インジケータを一括セット 
	 */
	BOOL SetIndicators(_StatusBarPaneItem* pPanes, int nPanes)
	{
		SweepAll();
		int* pia = new int[nPanes];
		int i;
		for (i = 0; i < nPanes; i++) {
			m_mapStockItem.Add(pPanes[i].paneId, new CStatusBarPaneItem(pPanes[i]));
			pia[i] = pPanes[i].paneId;
		}
		BOOL ok = SetPanes(pia, nPanes);
	
		GetParts(nPanes, pia);
		int delta = 0;
		for (i = 0; i < nPanes; i++) {
			CStatusBarPaneItem* item = m_mapStockItem.GetValueAt(i);
			if (item->iImage >= 0) {
				delta += m_sizeIcon.cx + m_cxIconPadding * 2;
			}
			if (item->isPullup()) {
				delta += m_cxPullupSpace;
			}
			pia[i] += delta;
		}
		SetParts(nPanes, pia);
		
		delete[] pia;
		return ok;
	}

	/*
	 * インジケータを個別に描画
	 */
	void DrawItem(DRAWITEMSTRUCT* pdi)
	{
		CStatusBarPaneItem* item = (CStatusBarPaneItem*)pdi->itemData;
		if (!item || item->isHidden())
			return;

		CRect rc = pdi->rcItem;
		CDCHandle dc = pdi->hDC;
		int nSave = dc.SaveDC();
		if (m_nHoverPane == item->paneId) {
			dc.FillSolidRect(&rc, ColorAdjustLuma(m_clr.bkgnd, isColorDarken(m_clr.bkgnd) ? 100 : -100, TRUE));
		}
		else if (item->isChecked()) {
			dc.FillSolidRect(&rc, m_clrChecked.bkgnd);
		}
		rc.DeflateRect(m_cxPaneMargin, 0);

		dc.SetBkMode(TRANSPARENT);
		if (m_clr.text != CLR_DEFAULT) {
			dc.SetTextColor(m_clr.text);
		}

		if (item->iImage >= 0 && !m_image.IsNull()) {
			rc.left += m_cxIconPadding;
			m_image.Draw(dc, item->iImage, rc.left, (rc.top + rc.bottom - m_sizeIcon.cy) / 2, ILD_TRANSPARENT);
			rc.left += m_sizeIcon.cx + m_cxIconPadding;
		}

		UINT flag = DT_SINGLELINE | DT_END_ELLIPSIS | DT_VCENTER;
		switch (item->flag & SBPIF_ALIGNMASK) {
			case SBPIF_RIGHTALIGN: flag |= DT_RIGHT; break;
			case SBPIF_CENTERALIGN: flag |= DT_CENTER; break;
		}
		if (lstrlen(item->label) > 0) {
			dc.DrawText(item->label, -1, &rc, flag);
		}
		if (item->isPullup()) {
			rc.left = rc.right - m_cxPullupSpace + 6;
			CFontHandle font = dc.SelectFont(m_fontSymbol);
			dc.DrawTextEx(_T("5"), -1, &rc, DT_SINGLELINE | DT_VCENTER);
			dc.SelectFont(font);
		}
		dc.RestoreDC(nSave);
	}

	/*
	 * インジケータのテキストを取得
	 */
	bool GetStockText(int itemId, CString& str) const
	{
		CStatusBarPaneItem* item = GetStockItem(itemId);
		if (item == NULL) {
			return false;
		}
		str = item->label;
		return true;
	}

	/*
	 * インジケータのテキストをセット
	 */
	void SetStockText(int itemId, LPCTSTR txt)
	{
		CStatusBarPaneItem* item = GetStockItem(itemId);
		if (!item) {
			item = new CStatusBarPaneItem(itemId, 0, -1, txt);
			m_mapStockItem.Add(itemId, item);
		}
		else if (!item->isSameLabel(txt)) {
			item->SetLabel(txt);
		}
		else
			return;

		SetPaneText(itemId, (LPCTSTR)item, SBT_OWNERDRAW | SBT_NOBORDERS);
		if (item->isAutoWidth()) {
			int cxWidth = m_cxPaneMargin * 2;		// 左右の基本マージン
			if (item->iImage >= 0) {
				// アイコンあり
				cxWidth += m_sizeIcon.cx + 4;
			}
			// 文字列幅
			CClientDC dc(NULL);
			CFontHandle font = dc.SelectFont(m_fontStatus);
			CSize s;
			dc.GetTextExtent(txt, -1, &s);
			cxWidth += s.cx;
			if (item->isPullup()) {
				cxWidth += m_cxPullupSpace;
			}
			SetPaneWidth(itemId, cxWidth);
			dc.SelectFont(font);
		}
	}

	/*
	 * インジケータのフラグをセット
	 */
	void SetPaneItemFlag(int itemId, UINT itemFlag)
	{
		CStatusBarPaneItem* item = GetStockItem(itemId);
		if (item) {
			item->flag = itemFlag;
			RedrawPaneItem(itemId);
		}
	}

	/*
	 * インジケータのフラグを取得
	 */
	UINT GetPaneItemFlag(int itemId)
	{
		CStatusBarPaneItem* item = GetStockItem(itemId);
		return item ? item->flag : 0;
	}

	/*
	 * インジケータのチェック状態をセット
	 */
	void CheckPaneItem(int itemId, BOOL bCheck = TRUE)
	{
		CStatusBarPaneItem* item = GetStockItem(itemId);
		if (item) {
			UINT flagT = item->flag;
			if (bCheck) {
				item->flag |= SBPIF_CHECK;
			} 
			else {
				item->flag &= ~SBPIF_CHECK;
			}
			if (item->flag != flagT) {
				RedrawPaneItem(itemId);
			}
		}
	}

	/*
	 * インジケータのチェック状態を取得
	 */
	BOOL isPaneItemChecked(int itemId)
	{
		CStatusBarPaneItem* item = GetStockItem(itemId);
		return item ? item->isChecked() : FALSE;
	}

	/*
	 * インジケータの表示・非表示をセット
	 */
	void HidePaneItem(int itemId, BOOL bHide = TRUE)
	{
		CStatusBarPaneItem* item = GetStockItem(itemId);
		if (item) {
			UINT flagT = item->flag;
			if (bHide) {
				item->flag |= SBPIF_HIDDEN;
			}
			else {
				item->flag &= ~SBPIF_HIDDEN;
			}
			if (item->flag != flagT) {
				RedrawPaneItem(itemId);
			}
		}
	}

	/*
	 * インジケータが非表示かどうかを返す
	 */
	BOOL isPaneItemHidden(int itemId)
	{
		CStatusBarPaneItem* item = GetStockItem(itemId);
		return item ? item->isHidden() : FALSE;
	}

	/*
	 * ステータスバーの背景カラーテーマをセット
	 */
	void SetColorTheme(COLORREF clr, COLORREF bkclr)
	{
		_ColorPair clrT = m_clr;
		ApplyColor(clr, bkclr);
		if (clrT.text != clr || clrT.bkgnd != bkclr) {
			Invalidate(TRUE);
		}
	}

	/*
	 * インジケータを再描画
	 */
	void RedrawPaneItem(int itemId)
	{
		CRect rc;
		if (GetPaneRect(itemId, &rc)) {
			InvalidateRect(&rc, TRUE);
		}
	}

	/*
	 * マウス位置からインジケータのidを取得
	 */
	int GetPaneIDFromPoint(CPoint& pt)
	{
		for (int i = 0; i < m_nPanes; i++) {
			CRect rc;
			if (GetPaneRect(m_pPane[i], &rc) && rc.PtInRect(pt)) {
				return m_pPane[i];
			}
		}
		return -1;
	}

protected:
	BEGIN_MSG_MAP(CLspStatusBarCtrl)
		ON_WM_ERASEBKGND()
		ON_WM_MOUSEMOVE()
		ON_WM_MOUSELEAVE()
		ON_WM_LBUTTONUP()
		CHAIN_MSG_MAP(CMultiPaneStatusBarCtrlImpl<CLspStatusBarCtrl>)
	END_MSG_MAP()

	BOOL OnEraseBkgnd(HDC hdc)
	{
		CRect rc;
		GetClientRect(rc);
		CDCHandle dc = hdc;
		dc.FillSolidRect(&rc, m_clr.bkgnd);
		return TRUE;
	}

	void OnMouseMove(UINT vKey, CPoint pt)
	{
		if (!m_bTrackMouseEvent) {
			// マウストラック
			TRACKMOUSEEVENT te = { sizeof(te) };
			te.dwFlags = TME_LEAVE;
			te.hwndTrack = m_hWnd;
			te.dwHoverTime = HOVER_DEFAULT;
			m_bTrackMouseEvent = _TrackMouseEvent(&te);
		}


		if (m_bTrackMouseEvent) {
			int paneT = m_nHoverPane;
			m_nHoverPane = GetPaneIDFromPoint(pt);
			if (paneT != m_nHoverPane) {
				CStatusBarPaneItem* item = GetStockItem(paneT);
				if (item && item->isButton()) {
					RedrawPaneItem(paneT);
				}
				item = GetStockItem(m_nHoverPane);
				if (item && item->isButton()) {
					RedrawPaneItem(m_nHoverPane);
				}
			}
		}
	}
	void OnMouseLeave()
	{
		if (m_bTrackMouseEvent) {
			int paneT = m_nHoverPane;
			m_nHoverPane = -1;
			CStatusBarPaneItem* item = GetStockItem(paneT);
			if (item && item->isButton()) {
				RedrawPaneItem(paneT);
			}
			m_bTrackMouseEvent = false;
		}
	}

	void OnLButtonUp(UINT vKey, CPoint pt)
	{
		int paneID = GetPaneIDFromPoint(pt);
		CStatusBarPaneItem* item = GetStockItem(paneID);
		if (item && item->isButton() && !item->isHidden()) {
			NMMOUSE nmm;
			nmm.hdr.hwndFrom = m_hWnd;
			nmm.hdr.code = NM_CLICK;
			nmm.hdr.idFrom = GetWindowLong(GWL_ID);
			nmm.dwItemSpec = _paneId2posIndex(paneID);		// index
			nmm.dwItemData = paneID;						// id
			nmm.pt.x = pt.x;
			nmm.pt.y = pt.y;
			nmm.dwHitInfo = vKey;
			GetParent().SendMessage(WM_NOTIFY, (WPARAM)m_hWnd, (LPARAM)&nmm);
		}
	}
};
