#include "stdafx.h"
#include <commoncontrols.h>
#include "fv7.h"
#include "mainfrm.h"
#include "shellexec.h"
#include "trackmenuwnd.h"
#include "func.h"

#define SFGAO_FILEFOLDER		(SFGAO_FOLDER|SFGAO_FILESYSTEM)

#define DTEX_SINGLE				(DT_VCENTER | DT_NOPREFIX | DT_END_ELLIPSIS | DT_SINGLELINE)
#define GAPBORDER				6
#define APPBAR_MAGIC			0x20040101
#define TIMER_HOVER_HEADER		600

void __cdecl qsort_ex(void *base, size_t num, size_t width,
    int (__cdecl *comp)(const void*, const void*, const void*), const void* param);

void XmlWriteIDListCore(TiXmlElement* pElement, LPCITEMIDLIST pidl, LPCTSTR pszName);
LPITEMIDLIST XmlLoadIDListCore(TiXmlElement* pElement, CString& str);

static COLORREF clrPanelColors[] = {
/*
	RGB(193, 206, 235),
	RGB(203, 195, 233),
	RGB(215, 223, 205),
	RGB(195, 223, 233),
	RGB(235, 227, 193),
	RGB(233, 230, 236),
	RGB(245, 228, 169),
	RGB(218, 201, 227),
	RGB(250, 218, 181),
	RGB(173, 185, 202),
	RGB(222, 228, 202),
*/
	0xd2cab5,
	0xeaeaea,
	0xc4e1f2,
	0xd9ead5,
	0xdfd7e7,
	0xf2eadd,
	0xf2cdd1,

	0xb9ad8d,
	0xcbcbcb,
	0x89c3e5,
	0xb3d5ab,
	0xbfafcf,
	0xe6d5bc,
	0xe59ca4,

	0x433c29,
	0x656565,
	0x14425d,
	0x3a6331,
	0x48365a,
	0x997339,
	0x771e28,
};

typedef struct tagGROUPLABEL {
	float point;
	LPCWSTR fontface;
} GROUPLABEL;

static GROUPLABEL sGLabel = {
//	9.0F, L"MS Gothic UI"
	8.0F, L"Meiryo UI"
//	8.0F, L"Yu Gothic UI"
};


/////////////////////////////////////////////////////////////////////////////
//
//

#define CY_FRAME				(GetSystemMetrics(SM_CYFRAME))
#define APPITEMMAGIC			0x8000FFFF

static const TCHAR szAppbar[] = _T("Appbar");
static const TCHAR szDummy[] = _T("~x::.exe");

#define SetStyle(a, b) 		(SetWindowLong((a), GWL_STYLE, (b)))
#define GETDROPEFFECT(key)	\
			( (key & MK_CONTROL) ? \
				( (key & MK_SHIFT) ? DROPEFFECT_LINK : DROPEFFECT_COPY ) : \
					( (key & MK_SHIFT) ? DROPEFFECT_MOVE : DROPEFFECT_COPY ) )

/////////////////////////////////////////////////////////////////////////////
// CAppItem

CAppItem::CAppItem()
{
	InitParams();
}

CAppItem::CAppItem(LPCITEMIDLIST pidlFq)
{
	InitParams();
	m_idl.Clone(pidlFq);

	TCHAR name[MAX_PATH * 2];
	if (SHGetPathFromIDList(pidlFq, name)) {
		CFileVersion ver;
		if (ver.Open(name)) {
			if (ver.GetFileDescription(name, ARRAYSIZE(name)))
				m_strDisplayName = name;
			ver.Close();
		}
	}
}

CAppItem::CAppItem(LPCTSTR pszPath)
{
	InitParams();
	m_idl.CreateFromPath(pszPath);
	m_attr = m_idl.GetAttributes(SFGAO_FILEFOLDER | SFGAO_DROPTARGET);

	CFileVersion ver;
	if (ver.Open(pszPath)) {
		CString str;
		if (ver.GetFileDescription(str))
			m_strDisplayName = str;
		ver.Close();
	}
}

void CAppItem::InitParams()
{
	m_strDisplayName.Empty();
	m_iIcon = -1;
	m_idl.Free();
	m_attr = 0;

	m_strParams.Empty();
	m_strIconLocation.Empty();
	m_guid = GUID_NULL;
}

void CAppItem::ParseInfo()
{
	if (m_attr == 0)
		m_attr = m_idl.GetAttributes(SFGAO_FILEFOLDER | SFGAO_DROPTARGET);
	if (m_strDisplayName.IsEmpty()) {
		SHFILEINFO sfi;
		m_idl.GetFileInfo(0, &sfi, sizeof(sfi), SHGFI_DISPLAYNAME);
		m_strDisplayName = sfi.szDisplayName;
	}
}

int CAppItem::GetIconIndex()
{
	if (m_iIcon < 0) {
		if (!m_strIconLocation.IsEmpty()) {
			TCHAR name[MAX_PATH * 2];
			lstrcpy(name, m_strIconLocation);
			int index = PathParseIconLocation(name);
			m_iIcon = Shell_GetCachedImageIndex(name, index, FALSE);
		} else
			m_iIcon = idl_GetSysIconIndex(m_idl);
	}
	return m_iIcon;
}

void CAppItem::SetIconPath(LPCTSTR pszIconPath, int iconIndex)
{
	m_strIconLocation.Format(_T("\"%s\",%d"), pszIconPath, iconIndex);
	m_iIcon = -1;
}

LPCTSTR CAppItem::GetInfoTip()
{
	if (m_strInfoTip.IsEmpty())
		idl_GetInfoTip(NULL, m_idl, m_strInfoTip);
	return (m_strInfoTip.IsEmpty() ? NULL : (LPCTSTR)m_strInfoTip);
}

BOOL CAppItem::Invoke() const
{
	if (InvokeIDList(GetActiveWindow(), m_idl)) {
		SHAddToRecentDocs(SHARD_PIDL, m_idl);
		return TRUE;
	}
	return FALSE;
}

BOOL CAppItem::Open() const
{
	if (isFolder()) {
		if (_ISKEYDOWN(VK_CONTROL))
			fvApp.OpenFolder(m_idl);
		else if (_ISKEYDOWN(VK_SHIFT))
			fvApp.OpenTabFolder(m_idl, TRUE);
		else
			return Invoke();
		return TRUE;
	}
	return Invoke();
}

void CAppItem::ShowProperty(HWND hwnd) const
{
//@	InvokeIDList(hwnd, m_idl, gszVerbProp);
	CComPtr<IDataObject> pdo = GetDataObject(hwnd);
	if (pdo.p != NULL) {
		SHMultiFileProperties(pdo, 0);
	}
}

bool CAppItem::GetGuid(CString& str)
{
	if (m_guid == GUID_NULL)
		CoCreateGuid(&m_guid);
	str.Empty();
	OLECHAR* clsid;
	if (StringFromCLSID(m_guid, &clsid) == S_OK)
		str = clsid;
	return (!str.IsEmpty());
}
bool CAppItem::SetGuid(LPCTSTR pszGuidString)
{
	if (CLSIDFromString((OLECHAR*)pszGuidString, &m_guid) != NOERROR) {
		m_guid = GUID_NULL;
		return false;
	}
	return true;
}

/////////////////////////////////////////////////////////////////////////////
// CAppGroup

CAppGroup::CAppGroup()
	: m_nFocused(0)
	, m_typeIcon(GIT_SMALL)
	, m_clrText(GetSysColor(COLOR_WINDOWTEXT))
	, m_clrBack(CLR_DEFAULT)
{
	m_rc.SetRectEmpty();
}

CAppGroup::CAppGroup(LPCTSTR pszGroupName)
	: m_strGroupName(pszGroupName)
	, m_nFocused(0)
	, m_typeIcon(GIT_SMALL)
	, m_clrText(GetSysColor(COLOR_WINDOWTEXT))
	, m_clrBack(CLR_DEFAULT)
{
	m_rc.SetRectEmpty();
}

CAppGroup::~CAppGroup()
{
	Wep();
}

void CAppGroup::Wep()
{
	int cnt = (int)m_array.GetCount();
	for (int i = 0; i < cnt; i++)
		delete m_array[i];
	m_array.RemoveAll();
}

#define MAGIC_APPGROUP		0xe0e0e0e0

/////////////////////////////////////////////////////////////////////////////
// CPanelPropertyDlg dialog

static GUID guids[] = {
	FOLDERID_Desktop,
	FOLDERID_ComputerFolder,
};

class CPanelPropertyDlg : public CDialogImpl<CPanelPropertyDlg>
{
public:
	COLORREF m_clrText;
	COLORREF m_clrBack;
	CString m_strName;
	CListViewCtrl m_list;

public:
	CPanelPropertyDlg()
		: m_clrText(CLR_DEFAULT)
		, m_clrBack(CLR_DEFAULT)
	{
	}
	~CPanelPropertyDlg()
	{
	}

	enum { IDD = IDD_OBJBAR };

BEGIN_MSG_MAP(CPanelPropertyDlg)
	ON_WM_INITDIALOG()
	ON_BN_CLICKED(IDOK, OnOK)
	ON_BN_CLICKED(IDCANCEL, OnCancel)
	ON_CBN_SELCHANGE(IDC_FGCOLOR, OnColorChanged)
	ON_CBN_SELCHANGE(IDC_BKCOLOR, OnColorChanged)
END_MSG_MAP()

	BOOL OnInitDialog(LPARAM lParam)
	{
		CenterWindow();
		SetDlgItemText(IDC_TEXT, m_strName);

		m_list = GetDlgItem(IDC_LIST);
		HIMAGELIST himL, himS;
		Shell_GetImageLists(&himL, &himS);
		m_list.SetImageList(himL, LVSIL_NORMAL);

		SHFILEINFO sfi;
		LVITEM lvi;
		lvi.mask = LVIF_IMAGE | LVIF_TEXT;
		lvi.iSubItem = 0;
		for (int i = 0; i < ARRAYSIZE(guids); i++) {
			CIDListPtr idl(guids[i]);
			if (!idl.isNull()) {
				idl.GetFileInfo(0, &sfi, sizeof(sfi), SHGFI_SYSICONINDEX | SHGFI_DISPLAYNAME);
				lvi.iItem = m_list.GetItemCount();
				lvi.pszText = sfi.szDisplayName;
				lvi.iImage = sfi.iIcon;
				m_list.InsertItem(&lvi);
				idl.Free();
			}
		}

		ColorPicker_SetBkColor(GetDlgItem(IDC_FGCOLOR), m_clrText);
		ColorPicker_SetBkColor(GetDlgItem(IDC_BKCOLOR), m_clrBack);
		SetListColors();
		return TRUE;
	}

	void OnOK()
	{
		CString str;
		if (GetDlgItemText(IDC_TEXT, str) < 1) {
			MessageBeep(0);
			GetDlgItem(IDC_TEXT).SetFocus();
			return;
		}
		m_strName = str;
		EndDialog(IDOK);
	}

	inline void OnCancel() { EndDialog(IDCANCEL); }

	void OnColorChanged()
	{
		m_clrText = ColorPicker_GetBkColor(GetDlgItem(IDC_FGCOLOR));
		m_clrBack = ColorPicker_GetBkColor(GetDlgItem(IDC_BKCOLOR));
		SetListColors();
	}

	void SetListColors()
	{
		m_list.SetTextColor(m_clrText);
		m_list.SetTextBkColor(CLR_NONE);
		m_list.SetBkColor(m_clrBack);
		m_list.Invalidate();
	}
};



/////////////////////////////////////////////////////////////////////////////
// CRapperWnd

CAppBar::CAppBar()
	: m_nCurPage(0)
	, m_cySpacer(6)
	, m_nDragItem(-1)
	, m_nHilitedPage(-1)
	, m_nHotPage(-1)
	, m_himLarge(NULL)
	, m_himSmall(NULL)
	, m_himExtra(NULL)
	, m_pLabelFont(NULL)
	, m_bCapture(false)
	, m_bItemLabelEditing(false)
{
	// ヘッダは8pointフォント
	CClientDC dc(NULL);
	m_cyChar = sGLabel.point * dc.GetDeviceCaps(LOGPIXELSY) / 72.0F;
	m_cyHeader = (int)ceil(m_cyChar) + m_cySpacer * 2 + 3;

	m_strAppGroupFile.Empty();
	m_cxMaxTextTab = _GCDATA.m_IconMetrics.iHorzSpacing + GetSystemMetrics(SM_CXVSCROLL);
}

int CAppBar::GetPageIndexFromPoint(CPoint pt)
{
	CAppGroup* group;
	int cnt = (int)m_page.GetCount();
	for (int i = 0; i < cnt; i++) {
		group = m_page[i];
		if (group->m_rc.PtInRect(pt))
			return i;
	}
	return -1;
}

BOOL CAppBar::GetPageRect(int index, CRect& rcPage)
{
	int cnt = (int)m_page.GetCount();
	if (cnt < 1 || index >= cnt)
		return FALSE;

	CRect rcClient, rc;
	GetClientRect(&rcClient);

	rc = rcClient;
	rc.bottom = rc.top + m_cyHeader;
	int i;
	for (i = 0; i <= m_nCurPage; i++) {
		if (i == index) {
			rcPage = rc;
			return TRUE;
		}
		rc.OffsetRect(0, m_cyHeader);
	}

	rc = rcClient;
	rc.top = rc.bottom - m_cyHeader;
	for (i = cnt - 1; i > m_nCurPage; i--) {
		if (i == index) {
			rcPage = rc;
			return TRUE;
		}
		rc.OffsetRect(0, -m_cyHeader);
	}
	return FALSE;
}

/////////////////////////////////////////////////////////////////////////////
// message handlers

int CAppBar::OnCreate(LPCREATESTRUCT lpCreateStruct) 
{
	GetAppDataFolder(m_strAppGroupFile, gszAppbarData);

	// リストの作成
	CRect rc(0, 0, 0, 0);
	DWORD style = WS_CHILD | WS_VISIBLE |
					LVS_NOCOLUMNHEADER | LVS_SINGLESEL | LVS_EDITLABELS |
					LVS_AUTOARRANGE | LVS_ALIGNTOP | LVS_ICON |
					LVS_SHAREIMAGELISTS;
	if (!m_list.Create(m_hWnd, rc, NULL, style, 0, IDW_APPBAR_PANE)) {
		return -1;
	}

	m_list.InsertColumn(0, _T(""), LVCFMT_LEFT, 0, 0);
	m_list.SetExtendedListViewStyle(LVS_EX_INFOTIP | LVS_EX_DOUBLEBUFFER);
	UpdateProperty();

	// アプリケーショングループの読み込み
	LoadState();
	SelectPage(m_nCurPage);

	// IMEを無効に
	ImmAssociateContext(m_hWnd, NULL);
	ImmAssociateContext(m_list, NULL);
	return 0;
}

void CAppBar::UpdateProperty()
{
	LOGFONT lf;
	if (!LoadFontInfo(&lf, IDS_SEC_SETTINGS, IDS_KEY_PANELFONT))
		_GCDATA.GetIconTitleLogFont(&lf);

	if (!m_font.IsNull()) m_font.DeleteObject();
	m_font.CreateFontIndirect(&lf);

	m_list.SetFont(m_font);
	m_list.Invalidate(TRUE);
}

void CAppBar::OnSize(UINT nType, int cx, int cy) 
{
	ResizeList();
}

void CAppBar::ResizeList()
{
	CRect rcc;
	GetClientRect(&rcc);
	CRect rc = rcc;
	int cnt = (int)m_page.GetCount();
	if (cnt > 0) {
		rc.bottom -= cnt * m_cyHeader;
		rc.OffsetRect(0, (m_nCurPage + 1) * m_cyHeader);
	}
	rc.left += 2;
	m_list.SetWindowPos(NULL, rc.left, rc.top, rc.Width(), rc.Height(), SWP_NOZORDER | SWP_NOACTIVATE);
	m_list.SetColumnWidth(0, rc.Width());
	Invalidate(FALSE);
}

void CAppBar::OnSetFocus(HWND /*hOldWnd*/) 
{
	m_list.SetFocus();
}

int CAppBar::OnMouseActivate(HWND hDesktopWnd, UINT nHitTest, UINT message)
{
	LRESULT lr = DefWindowProc();
	if (lr != MA_NOACTIVATE && lr != MA_NOACTIVATEANDEAT) {		// frame does not want to activate
		if (nHitTest == HTCLIENT)
			fvApp.SetActiveView(m_hWnd);
	}
	return (int)lr;
}

void CAppBar::OnPaint()
{
	CPaintDC dc(m_hWnd);

	int cnt = (int)m_page.GetCount();
	for (int i = 0; i < cnt; i++)
		DrawHeader(dc, i, false);

	CRect rc;
	m_list.GetWindowRect(&rc);
	ScreenToClient(&rc);

	CAppGroup* group = GetCurrentPage();
	COLORREF clrFace = group->m_clrBack;
	COLORREF clrLite = mix_color(clrFace, RGB(255, 255, 255), 80);
	COLORREF clrShadow = mix_color(clrFace, RGB(0, 0, 0), 80);
	COLORREF clrHilite = mix_color(clrLite, RGB(255, 255, 255), 50);

	CPen pen;
	pen.CreatePen(PS_SOLID, 0, clrShadow);
	HPEN hpen = dc.SelectPen(pen);
	dc.MoveTo(rc.left - 2, rc.top);
	dc.LineTo(rc.left - 2, rc.bottom);
	dc.SelectPen(hpen);
	pen.DeleteObject();

	pen.CreatePen(PS_SOLID, 0, clrHilite);
	hpen = dc.SelectPen(pen);
	dc.MoveTo(rc.left - 1, rc.top);
	dc.LineTo(rc.left - 1, rc.bottom);
	dc.SelectPen(hpen);
	pen.DeleteObject();
}

void CAppBar::_drawbutton(HDC hdc, int index, CRect& rc, BOOL bPush)
{
	CAppGroup* group = GetPage(index);
	if (!group)
		return;

	CDCHandle dc = hdc;
	COLORREF clrFace = group->m_clrBack;
	COLORREF clrLite = mix_color(clrFace, RGB(255, 255, 255), 80);
	COLORREF clrShadow = mix_color(clrFace, RGB(0, 0, 0), 80);
	COLORREF clrHilite = mix_color(clrLite, RGB(255, 255, 255), 50);

	ShadeRect(dc, rc, clrLite, clrFace);

	CPen pen[2];
	if (bPush) {
		pen[0].CreatePen(PS_SOLID, 0, clrShadow);
		pen[1].CreatePen(PS_SOLID, 0, clrFace);
	} else {
		pen[0].CreatePen(PS_SOLID, 0, clrShadow);
		pen[1].CreatePen(PS_SOLID, 0, clrHilite);
	}

	CRect rcw = rc;
	int delta = m_cyHeader - m_cySpacer * 2;
	int cxSpacing = 0;
	int baseSpace = delta + m_cySpacer + m_cxMaxTextTab;
	if (rcw.left + baseSpace < rcw.right) {
		cxSpacing = MulDiv(rcw.Width() - baseSpace, baseSpace, rcw.Width());
	}
	HPEN hpen;
	for (int i = 0; i < 2; i++) {
		hpen = dc.SelectPen(pen[i]);
		dc.MoveTo(rcw.left, rcw.bottom);
		dc.LineTo(rcw.left, rcw.top);
		dc.LineTo(rcw.right - cxSpacing - m_cySpacer - delta, rcw.top);
		dc.LineTo(rcw.right - cxSpacing - m_cySpacer, rcw.top + delta);
		dc.LineTo(rcw.right, rcw.top + delta);
		dc.SelectPen(hpen);
		rcw.left++;
		rcw.top++;
	}

	CAppGroup* groupPrev = GetPage(index - 1);
	clrFace = groupPrev ? groupPrev->m_clrBack : GetSysColor(COLOR_3DFACE);
	HBRUSH hbr = dc.SelectBrush(CreateSolidBrush(clrFace));
	dc.BeginPath();
	dc.MoveTo(rc.right - cxSpacing - m_cySpacer - delta + 1, rc.top);
	dc.LineTo(rc.right - cxSpacing - m_cySpacer + 1, rc.top + delta);
	dc.LineTo(rc.right, rc.top + delta);
	dc.LineTo(rc.right, rc.top);
	dc.LineTo(rc.right - cxSpacing - m_cySpacer - delta + 1, rc.top);
	dc.EndPath();
	dc.FillPath();

	DeleteObject(dc.SelectBrush(hbr));

	// テキスト描画領域をセット
	rc.right -= (cxSpacing + m_cySpacer + delta);
}

void CAppBar::DrawHeader(HDC hdc, int index, bool bPush, bool bSelect)
{
	CDCHandle dc = hdc;

	CAppGroup* group = GetPage(index);
	if (!group) return;

	CRect rc;
	GetPageRect(index, rc);
	group->m_rc = rc;

	CDC dcw;
	dcw.CreateCompatibleDC(dc);
	CBitmap bmp;
	bmp.CreateCompatibleBitmap(dc, rc.Width(), rc.Height());
	HBITMAP hbmOld = dcw.SelectBitmap(bmp);

	CRect rcw(0, 0, rc.Width(), rc.Height());
	_drawbutton(dcw, index, rcw, bPush);			// rcwの内容は変更されて、テキスト描画可能矩形を指している

	// フォント情報
	rcw.left += 4;
	rcw.InflateRect(-1, 0);

	// using gdi+
	Graphics g(dcw);

	// アンダーライン？
	COLORREF clr;
	if (index == m_nHotPage) {
		clr = mix_color(group->m_clrText, group->m_clrBack, 40);
		Rect r(rcw.left, (int)((rcw.top + rcw.bottom + m_cyChar) / 2), rcw.Width(), 2);
		Color cF(255, GetRValue(clr), GetGValue(clr), GetBValue(clr));
		Color cT(0, GetRValue(clr), GetGValue(clr), GetBValue(clr));
		LinearGradientBrush lbr(r, cF, cT, LinearGradientModeHorizontal);
		g.FillRectangle(&lbr, r);
	}

	if (bSelect) {
		clr = GetSysColor(COLOR_HIGHLIGHTTEXT);
		dcw.SetBkColor(GetSysColor(COLOR_HIGHLIGHT));
	} else {
		clr = group->m_clrText;
		dcw.SetBkMode(TRANSPARENT);
	}
	dcw.SetTextColor(clr);

	// ヘッダテキスト描画
	if (bPush) rcw.OffsetRect(1, 1);

	RectF r((REAL)rcw.left, (REAL)rcw.top, (REAL)rcw.Width(), (REAL)rcw.Height());
	StringFormat fmt;
	fmt.SetFormatFlags(StringFormatFlagsNoWrap);				// DT_SINGLELINE
	fmt.SetAlignment(StringAlignmentNear);						// DT_LEFT
	fmt.SetTrimming(StringTrimmingEllipsisCharacter);			// followed by "..."
	fmt.SetLineAlignment(StringAlignmentCenter);				// DT_VCENTER
	SolidBrush sbr(Color(255, GetRValue(clr), GetGValue(clr), GetBValue(clr)));
	g.SetTextRenderingHint(TextRenderingHintAntiAlias);

	// ラベルフォント
	if (!m_pLabelFont) {
//		 m_pLabelFont = new Font(L"MS UI Gothic", m_cyChar, FontStyleRegular, UnitPixel);
		 m_pLabelFont = new Font(sGLabel.fontface, m_cyChar, FontStyleRegular, UnitPixel);
	}
	g.DrawString(group->m_strGroupName, group->m_strGroupName.GetLength(), m_pLabelFont, r, &fmt, &sbr);

	if (bPush) rcw.OffsetRect(-1, -1);

	dc.BitBlt(rc.left, rc.top, rc.Width(), rc.Height(), dcw, 0, 0, SRCCOPY);
	dcw.SelectBitmap(hbmOld);
}

void CAppBar::MouseMove(POINT pt)
{
	int hot = GetPageIndexFromPoint(pt);
	if (hot != m_nHotPage) {
		CClientDC dc(m_hWnd);
		int temp = m_nHotPage;
		m_nHotPage = hot;
		if (temp >= 0)
			DrawHeader(dc, temp, false, false);
		if (m_nHotPage >= 0)
			DrawHeader(dc, m_nHotPage, false, false);
	}
}

void CAppBar::MouseLeave()
{
	if (m_nHotPage >= 0) {
		CClientDC dc(m_hWnd);
		int temp = m_nHotPage;
		m_nHotPage = -1;
		DrawHeader(dc, temp, false, false);
	}
}

void CAppBar::OnLButtonDown(UINT /*nFlags*/, CPoint pt)
{
	if (!m_bCapture) {
		int index = GetPageIndexFromPoint(pt);
		if (index != -1) {
			if (OnTrackMouse(index)) {
				SelectPage(index);
			}
		}
	}
}

void CAppBar::OnRButtonDown(UINT /*nFlags*/, CPoint pt) 
{
	if (!m_bCapture) {
		int index = GetPageIndexFromPoint(pt);
		if (index != -1) {
			if (OnTrackMouse(index)) {
				if (m_nCurPage != index) { SelectPage(index); }
				m_list.SetFocus();
				ClientToScreen(&pt);
				OnPageMenu(pt);
			}
		}
	}
}

BOOL CAppBar::OnTrackMouse(int index)
{
	SetCapture();
	m_bCapture = true;

	CClientDC dc(m_hWnd);

	CRect rc;
	GetPageRect(index, rc);
	DrawHeader(dc, index, true);
	MSG msg;
	while (1) {
		if (PeekMessage(&msg, NULL, NULL, NULL, PM_REMOVE)) {
			if (msg.message == WM_LBUTTONUP || msg.message == WM_RBUTTONUP)
				break;
			else {
				TranslateMessage(&msg);
				DispatchMessage(&msg);
			}
		}
	}
	DrawHeader(dc, index, false);

	CPoint pt((DWORD)msg.lParam);
	ReleaseCapture();
	m_bCapture = false;
	return rc.PtInRect(pt);
}

void CAppBar::OnCancelMode() 
{
	if (m_bCapture) {
		ReleaseCapture();
		m_bCapture = false;
	}
}

void CAppBar::SelectPage(int nPage)
{
	int i;
	CAppItem* pai;

	m_list.DeleteAllItems();

	// システムイメージリスト取得
	Shell_GetImageLists(&m_himLarge, &m_himSmall);
	CComPtr<IImageList> pii;
	if (SHGetImageList(GIT_EXTRALARGE, IID_PPV_ARGS(&pii)) == S_OK) {
		m_himExtra = IImageListToHIMAGELIST(pii);
	} else {
		m_himExtra = m_himLarge;
	}
	// 小さいアイコンイメージだけをここでは設定
	m_list.SetImageList(m_himSmall, LVSIL_SMALL);

	CAppGroup* group = GetCurrentPage();
	if (group)
		group->m_nFocused = m_list.GetNextItem(-1, LVNI_SELECTED);

	group = GetPage(nPage);
	if (group) {
		m_nCurPage = nPage;
		m_list.SetRedraw(FALSE);

		switch (group->m_typeIcon) {
		case GIT_LARGE:			OnPageLargeIcon(); break;
		case GIT_EXTRALARGE:	OnPageExtraIcon(); break;
		case GIT_SMALL:			OnPageReportSmallIcon(); break;
		case GIT_LIST:			OnPageReportLargeIcon(); break;
		}

		int cnt = (int)group->m_array.GetCount();
		for (i = 0; i < cnt; i++) {
			pai = group->m_array[i];
			pai->ParseInfo();
			m_list.InsertItem(LVIF_TEXT | LVIF_IMAGE | LVIF_PARAM,
							m_list.GetItemCount(), pai->m_strDisplayName,
							0, 0, I_IMAGECALLBACK, (LPARAM)pai);
		}

		i = (group->m_nFocused >= 0) ? group->m_nFocused : 0;
		m_list.SelectItem(i);
		m_list.SetRedraw(TRUE);
		m_list.SetTextColor(group->m_clrText);
		m_list.SetTextBkColor(CLR_NONE);
		m_list.SetBkColor(group->m_clrBack);
	}
	else
		m_nCurPage = 0;

	ResizeList();
}

CAppGroup* CAppBar::GetPage(int index)
{
	int cnt = (int)m_page.GetCount();
	return (cnt > 0 && index >= 0 && index < cnt) ? m_page[index] : NULL;
}

void CAppBar::OnDestroy() 
{
	SaveState();

	int cnt = (int)m_page.GetCount();
	for (int i = 0; i < cnt; i++)
		delete m_page[i];
	m_page.RemoveAll();

	if (m_pLabelFont) {
		delete m_pLabelFont;
		m_pLabelFont = NULL;
	}
}

void CAppBar::OnContextMenu(HWND /*hwnd*/, CPoint point)
{
	CRect rc;
	GetWindowRect(&rc);
	if (!rc.PtInRect(point)) {
		GetPageRect(m_nCurPage, rc);
		point = rc.CenterPoint();
		ClientToScreen(&point);
	}
	OnPageMenu(point);
}

void CAppBar::OnParentNotify(UINT msg, LPARAM lParam)
{
	DefWindowProc();
	if (m_list.IsWindow()) {
		CPoint pt((long)(short)LOWORD(lParam), (long)(short)HIWORD(lParam));
		UINT nFlags;
		m_list.HitTest(pt, &nFlags);
	}
}

void CAppBar::OnInitMenuPopup(HMENU hMenu, UINT, BOOL bSysMenu) 
{
	if (bSysMenu)
		return;     // don't support system menu

	CAppGroup* group = GetCurrentPage();

	UINT flg;
	flg = (m_page.GetCount() > 0) ? MF_ENABLED : MF_GRAYED;
	EnableMenuItem(hMenu, IDM_APPPAGE_DELGROUP, MF_BYCOMMAND | flg);
	EnableMenuItem(hMenu, IDM_APPPAGE_RENAME, MF_BYCOMMAND | flg);

	UINT idm = IDM_APPPAGE_EXTRAICON;
	switch (group->m_typeIcon) {
		case GIT_EXTRALARGE: idm = IDM_APPPAGE_EXTRAICON; break;
		case GIT_LARGE: idm = IDM_APPPAGE_LARGEICON; break;
		case GIT_SMALL: idm = IDM_APPPAGE_SMALLICON; break;
		case GIT_LIST: idm = IDM_APPPAGE_LARGEICONLIST; break;
	}
	CheckMenuItem(hMenu, idm, MF_BYCOMMAND | MF_CHECKED);

	if (gnBarStat & BTS_APPRIGHT) { CheckMenuItem(hMenu, IDM_APPPAGE_RIGHT, MF_BYCOMMAND | MF_CHECKED); }
	else { CheckMenuItem(hMenu, IDM_APPPAGE_LEFT, MF_BYCOMMAND | MF_CHECKED); }
}

CAppItem* CAppBar::AddItem(LPCTSTR pszFile, int nInsBefore, LPCTSTR pszName, int iSysImageIndex)
{
	CAppItem* pai = NULL;
	CIDListPtr idl;
	if (idl.CreateFromPath(pszFile))
		pai = AddItem(idl, nInsBefore, pszName, iSysImageIndex);
	return pai;
}

CAppItem* CAppBar::AddItem(LPCITEMIDLIST pidlFq, int nInsBefore, LPCTSTR pszName, int iSysImageIndex)
{
	if (m_page.GetCount() < 1) {
		OnPageAdd();
		SelectPage(0);
	}

	if (nInsBefore < 0)
		nInsBefore = m_list.GetItemCount();

	CAppGroup* group = GetCurrentPage();
	if (group) {
		CAppItem* pai = new CAppItem(pidlFq);
		group->m_array.InsertAt(nInsBefore, pai);
		pai->ParseInfo();
		if (pszName && *pszName)
			pai->m_strDisplayName = pszName;
		m_list.InsertItem(LVIF_TEXT | LVIF_IMAGE | LVIF_PARAM,
							nInsBefore, pai->m_strDisplayName,
							0, 0, iSysImageIndex, (LPARAM)pai);
		return pai;
	}
	return NULL;
}

void CAppBar::OnNotifyDblClk(NMHDR* /*pNmhdr*/, LRESULT* pResult)
{
	CPoint pt;
	GetCursorPos(&pt);
	m_list.ScreenToClient(&pt);
	LV_HITTESTINFO lvhti;
	lvhti.pt = pt;
	m_list.HitTest(&lvhti);
	if (lvhti.flags & LVHT_ONITEM) {
		CAppItem* pai = (CAppItem*)m_list.GetItemData(lvhti.iItem); 
		if (pai) {
			if (_ISKEYDOWN(VK_MENU))
				pai->ShowProperty(m_hWnd);
			else
				pai->Open();
		}
	}
	*pResult = TRUE;
}

void CAppBar::TrackItemContextMenu(CAppItem* pai, const POINT& pt)
{
	if (pai) {
		CTrackMenuWnd wnd;
		CMenu menu;
		menu.LoadMenu(ID_MENU_PAGEITEM);
		CMenu pop = menu.GetSubMenu(0);

		if (!pai->isFolder()) {
			// フォルダ項目のコマンドは除外
			pop.DeleteMenu(IDM_PAGEITEM_OPENNEWTAB, MF_BYCOMMAND);
			pop.DeleteMenu(IDM_PAGEITEM_OPENNEWWINDOW, MF_BYCOMMAND);
		}
		int cmd = wnd.DoTrack(m_hWnd, pop, pt);
		if (cmd > 0) PostMessage(WM_COMMAND, cmd);
	}
}

void CAppBar::OnNotifyRClick(NMHDR* /*pNmhdr*/, LRESULT* pResult)
{
	CPoint pt;
	GetCursorPos(&pt);

	CPoint ptClient = pt;
	m_list.ScreenToClient(&ptClient);
	LV_HITTESTINFO lvhti;
	lvhti.pt = ptClient;
	m_list.HitTest(&lvhti);
	if (lvhti.flags & LVHT_ONITEM)
		TrackItemContextMenu((CAppItem*)m_list.GetItemData(lvhti.iItem), pt);
	else
		OnPageMenu(pt);
	*pResult = TRUE;
}

void CAppBar::OnPageMenu(CPoint pt)
{
	CTrackMenuWnd wnd;
	int cmd = wnd.DoTrack(m_hWnd, ID_MENU_APPPAGE, pt);
	if (cmd > 0) PostMessage(WM_COMMAND, cmd);
}

void CAppBar::OnNotifyKeyDown(NMHDR* pNmhdr, LRESULT* pResult)
{
	LV_KEYDOWN* plvkd = (LV_KEYDOWN*)pNmhdr;
	*pResult = TRUE;

	BOOL vkeyControl = _ISKEYDOWN(VK_CONTROL);
	BOOL vkeyMenu = _ISKEYDOWN(VK_MENU);
	switch (plvkd->wVKey) {
	case VK_RETURN:
		if (vkeyMenu) {
			int sel = m_list.GetNextItem(-1, LVNI_SELECTED);
			if (sel >= 0) {
				CRect rc;
				m_list.GetItemRect(sel, &rc, LVIR_SELECTBOUNDS);
				m_list.ClientToScreen(&rc);
				TrackItemContextMenu((CAppItem*)m_list.GetItemData(sel), rc.CenterPoint());
			}
		}
		else
		SendMessage(WM_COMMAND, IDM_PAGEITEM_OPEN);
		return;

	case VK_DELETE:
		OnItemDelete();
		return;

	case VK_UP:
		if (vkeyControl) {
			OnItemMoveUp();
			return;
		}
		break;

	case VK_DOWN:
		if (vkeyControl) {
			OnItemMoveDown();
			return;
		}
		break;

	case VK_PRIOR:
		if (vkeyControl) {
			if (m_page.GetCount() > 0 && m_nCurPage > 0) {
				SelectPage(m_nCurPage - 1);
				return;
			}
		}
		break;

	case VK_NEXT:
		if (vkeyControl) {
			if (GetPage(m_nCurPage + 1)) {
				SelectPage(m_nCurPage + 1);
				return;
			}
		}
		break;

	default:
		break;
	}
	*pResult = FALSE;
}

void CAppBar::OnNotifyBeginLabelEdit(NMHDR* /*pNmhdr*/, LRESULT* pResult)
{
	fvApp.EnableAccel(false);
	m_bItemLabelEditing = true;
	*pResult = FALSE;
}

void CAppBar::OnNotifyEndLabelEdit(NMHDR* pNmhdr, LRESULT* /*pResult*/)
{
	LV_DISPINFO* pdi = (LV_DISPINFO*)pNmhdr;
	if (pdi->item.pszText != NULL && lstrlen(pdi->item.pszText) > 0) {
		CAppItem* pai = (CAppItem*)m_list.GetItemData(pdi->item.iItem);
		pai->m_strDisplayName = pdi->item.pszText;
		m_list.SetItemText(pdi->item.iItem, 0, pai->m_strDisplayName);
	}
	m_bItemLabelEditing = false;
	fvApp.EnableAccel(true);
}

void CAppBar::OnNotifyBeginDrag(NMHDR* pNmhdr, LRESULT* /*pResult*/)
{
	NM_LISTVIEW* pnmlv = (NM_LISTVIEW*)pNmhdr;
	m_nDragItem = pnmlv->iItem;
	CAppItem* pai = (CAppItem*)m_list.GetItemData(pnmlv->iItem);
	if (pai) {
		CComPtr<IDataObject> pdo = pai->GetDataObject(m_hWnd);
		if (pdo.p != NULL) {
			DROPEFFECT eff = DROPEFFECT_COPY | DROPEFFECT_MOVE | DROPEFFECT_LINK;
			DragStart(m_list, pdo, 1, &eff);
		}
	}
	m_nDragItem = -1;
}

void CAppBar::OnNotifySetFocus(NMHDR* /*pNmhdr*/, LRESULT* /*pResult*/)
{
	int index = m_list.GetNextItem(-1, LVNI_SELECTED);
	if (index < 0)
		m_list.SelectItem(0);
}

void CAppBar::OnNotifyItemChanged(NMHDR* pNmhdr, LRESULT* /*pResult*/)
{
	NM_LISTVIEW* pnmlv = (NM_LISTVIEW*)pNmhdr;
	CAppItem* pai = (CAppItem*)pnmlv->lParam;
	if (pai && pai->isFolder() && !pai->isFileSystem()) {
		int index = idl_GetSysIconIndex(pai->m_idl);
		if (pai->GetIconIndex() != index) {
			pai->m_iIcon = index;
			m_list.RedrawItems(pnmlv->iItem, pnmlv->iItem);
			m_list.UpdateWindow();
		}
	}
}

void CAppBar::OnNotifyItemGetInfoTip(NMHDR* pNmhdr, LRESULT* /*pResult*/)
{
	NMLVGETINFOTIP* pgit = (NMLVGETINFOTIP*)pNmhdr;
	CAppItem* pai = (CAppItem*)m_list.GetItemData(pgit->iItem);
	if (pai) {
		LPCTSTR ptips = pai->GetInfoTip();
		if (ptips) {
			if (pgit->dwFlags)
				lstrcpy(pgit->pszText, ptips);
			else {
				lstrcat(pgit->pszText, _T("\n"));
				lstrcat(pgit->pszText, ptips);
			}
		}
	}
}

void CAppBar::OnNotifyItemGetDispInfo(NMHDR* pNmhdr, LRESULT* pResult)
{
	*pResult = 0;
	LV_DISPINFO* pdi = (LV_DISPINFO*)pNmhdr;
	if (pdi) {
		if (pdi && pdi->item.mask & LVIF_IMAGE) {
			CAppItem* pai = (CAppItem*)pdi->item.lParam;
			if (pai) {
				pdi->item.iImage = pai->GetIconIndex();
				pdi->item.mask |= LVIF_DI_SETITEM;
			}
		}
	}
}

DWORD CAppBar::OnDragEnter(IDataObject* pdo, DWORD keyState, DWORD dropEffect, CPoint point)
{
	SetDragScrollTimer(TIMER_HOVER_HEADER);
	m_nHilitedPage = -1;
	return OnDragOver(pdo, keyState, dropEffect, point);
}

DWORD CAppBar::OnDragOver(IDataObject* pdo, DWORD keyState, DWORD dropEffect, CPoint point)
{
	CPoint pt = point;
	ScreenToClient(&pt);

	CClientDC dc(m_hWnd);
	int pageindex = GetPageIndexFromPoint(pt);

	// VISTAのエアロD&Dテキスト
	CString strTargetDisplayName;

	DROPEFFECT eff;
	int index;
	if (pageindex >= 0) {
		// ページヘッダにカーソルがある場合
		// ページ内のリスト項目でドロップハイライトされているものをクリア
		index = m_list.GetNextItem(-1, LVNI_DROPHILITED);
		if (index >= 0) {
			ShowDropHelper(FALSE);
			m_list.SetItemState(index, 0, LVIS_DROPHILITED);
		}

		if (m_nHilitedPage != pageindex) {
			// ハイライト中のページと現在指しているページが異なる場合
			ShowDropHelper(FALSE);
			// 以前のハイライトを元に戻す
			if (m_nHilitedPage >= 0)
				DrawHeader(dc, m_nHilitedPage, false);

			// ページタイトルのハイライトを切り替え
			m_nHilitedPage = pageindex;
			DrawHeader(dc, pageindex, false, true);
			// タイマ監視セット
			if (m_nHilitedPage != m_nCurPage)
				SetDragScrollTimer(TIMER_HOVER_HEADER);
		}
		else {
			if (!isDragMyself() && (m_nHilitedPage != m_nCurPage)) {
				if (IsTimeToDragScroll()) {
					// ページを切り替え
					ShowDropHelper(FALSE);
					SelectPage(pageindex);
					m_nHilitedPage = -1;
					// タイマ監視停止
					CancelDragScrollTimer();
				}
			}
		}

		if (!isDragMyself()) {
			eff = DROPEFFECT_LINK;
		}
		else {
			// 自分自身のアイテムをD&Dしている場合
			eff = (pageindex != m_nCurPage) ? DROPEFFECT_MOVE : DROPEFFECT_NONE;
		}

		ShowDropHelper(TRUE);

		CAppGroup* page = GetPage(pageindex);
		strTargetDisplayName = _T("'") + page->m_strGroupName + _T("' グループ");
		SetDropTargetDescription(pdo, strTargetDisplayName, eff);
		return eff;
	}

	if (m_nHilitedPage >= 0) {
		// ハイライト表示があればそれをクリア
		ShowDropHelper(FALSE);
		DrawHeader(dc, m_nHilitedPage, false);
		m_nHilitedPage = -1;
	}

	pt = point;
	m_list.ScreenToClient(&pt);

	// 以前のドロップターゲット項目インデクスを取得
	int prevTarget = m_list.GetNextItem(-1, LVNI_DROPHILITED);
	// カーソル位置の項目インデクスを取得
	UINT flags;
	index = m_list.HitTest(pt, &flags);
	if (prevTarget >= 0 && prevTarget != index) {
		// ターゲットが変わったので古いターゲットのフラグはオフ
		ShowDropHelper(FALSE);
		m_list.SetItemState(prevTarget, 0, LVIS_DROPHILITED);
	}

	CAppItem* pai;
	eff = DROPEFFECT_NONE;
	if (!isDragMyself() || index != m_nDragItem) {
		if (index >= 0) {
			pai = (CAppItem*)m_list.GetItemData(index);
			if (pai && pai->isDropTarget()) {
				DROPEFFECT capa = DROPEFFECT_COPY | DROPEFFECT_MOVE | DROPEFFECT_LINK;
				eff = idl_GetDropEffect(m_hWnd, pai->m_idl, pdo, keyState, capa);
			}
			else
				eff = DROPEFFECT_NONE;
		}
		else if (!isDragMyself())
			eff = DROPEFFECT_LINK;

		if (index >= 0 && index != prevTarget) {
			ShowDropHelper(FALSE);
			m_list.SetItemState(index, LVIS_DROPHILITED, LVIS_DROPHILITED);
		}

	}
	ShowDropHelper(TRUE);

	if (index < 0) {
		CAppGroup* page = GetCurrentPage();
		strTargetDisplayName = _T("'") + page->m_strGroupName + _T("' グループ");
	} else {
		pai = (CAppItem*)m_list.GetItemData(index);
		strTargetDisplayName = pai->m_strDisplayName;
	}
	SetDropTargetDescription(pdo, strTargetDisplayName, eff);
	return eff;
}

void CAppBar::OnDragLeave()
{
	if (m_nHilitedPage != -1) {
		CClientDC dc(m_hWnd);
		DrawHeader(dc, m_nHilitedPage, false);
		m_nHilitedPage = -1;
	}

	int nDropHilited = m_list.GetNextItem(-1, LVNI_DROPHILITED);
	if (nDropHilited != -1)
		m_list.SetItemState(nDropHilited, 0, LVIS_DROPHILITED);
}

DWORD CAppBar::OnDrop(IDataObject* pdo, DWORD keyState, DWORD dropEffect, CPoint point)
{
	CAppItem* pai;

	CPoint pt = point;
	ScreenToClient(&pt);

	// ページインデクスを取得
	int pageIndex = GetPageIndexFromPoint(pt);
	if (pageIndex != -1) {
		// 何れかのページヘッダにドロップされた
		if (isDragMyself()) {
			// 自分自身でD&Dした場合、項目のグループ間移動を実行
			if (pageIndex != m_nCurPage) {
				CAppGroup* group = GetPage(m_nCurPage);
				if (group) {
					pai = (CAppItem*)group->m_array[m_nDragItem];
					group->m_array.RemoveAt(m_nDragItem);
					group = GetPage(pageIndex);
					if (group) {
						group->m_array.Add(pai);
						SaveState();
						// 再度読み込み直し
						SelectPage(m_nCurPage);
					}
				}
			}
			return DROPEFFECT_NONE;
		}

		// ページを選択
		if (pageIndex != m_nCurPage)
			SelectPage(pageIndex);

		return DROPEFFECT_NONE;
	}

	// リストへのD&D
	DROPEFFECT eff = DROPEFFECT_NONE;
	ClientToScreen(&pt);
	m_list.ScreenToClient(&pt);

	// リスト内の項目へのD&Dをチェック
	UINT flgs;
	int index = m_list.HitTest(pt, &flgs);
	if (index == -1) {
		// 自分自身のD&Dならこれでおしまい
		if (isDragMyself()) return DROPEFFECT_NONE;

		// グループに項目を追加する
		CSimpleArray<PCIDLIST_ABSOLUTE> array;
		int cnt = GetIDListsFromDataObject(pdo, array);
		if (cnt > 0) {
			// ShellIDList形式のデータ
			for (int i = 0; i < cnt; i++) {
				PCIDLIST_ABSOLUTE pidl = array[i];
				AddItem(pidl, index);
				CoTaskMemFree((void*)pidl);
			}
			SaveState();
		}
	} else {
		if (!isDragMyself() || index != m_nDragItem) {
			// 対象項目へのD&Dを実行
			CString str;
			pai = (CAppItem*)m_list.GetItemData(index);
			if (pai->isFolder() &&
				(IsDropFormatsSupported(SDO_HDROP|SDO_FILENAMEMAPA|SDO_FILENAMEMAPW) == SDO_HDROP) &&
				pai->m_idl.GetPath(str) &&
				(keyState & MK_LBUTTON) != 0)
			{
				TDropFiles df;
				df.Attach((HDROP)GetDataFromDataObject(pdo, CF_HDROP));
				if (df.GetCount() > 0) {
					BOOL bThreaded = FALSE;
					BOOL bAsAdmin = FALSE;
					if (QueryDrop(m_hWnd, pai->m_strDisplayName, dropEffect, bThreaded, bAsAdmin)) {
						eff = dropEffect;
						AcceptFiles(df, eff, str, bThreaded);
					}
					return eff;
				}
			}
			eff = AcceptDataObject(m_hWnd, pai->m_idl, pdo, keyState, dropEffect, point);
		}
	}

	return eff;
}

void CAppBar::OnPageExtraIcon()
{
	m_list.SetRedraw(FALSE);
	DWORD type = m_list.GetViewType();
	if (type != LVS_ICON) {
		m_list.SetViewType(LVS_ICON);
		m_list.Arrange(LVA_SNAPTOGRID);
	}
	CAppGroup* group = GetCurrentPage();
	if (group)
		group->m_typeIcon = GIT_EXTRALARGE;

	m_list.SetImageList(m_himExtra, LVSIL_NORMAL);
	m_list.SendMessage(LVM_SETICONSPACING, 0, (LPARAM)-1);		// x86とx64で有効
	m_list.SetRedraw(TRUE);
	m_list.Invalidate();
}

void CAppBar::OnPageLargeIcon()
{
	m_list.SetRedraw(FALSE);
	DWORD type = m_list.GetViewType();
	if (type != LVS_ICON) {
		m_list.SetViewType(LVS_ICON);
		m_list.Arrange(LVA_SNAPTOGRID);
	}
	CAppGroup* group = GetCurrentPage();
	if (group)
		group->m_typeIcon = GIT_LARGE;

	m_list.SetImageList(m_himLarge, LVSIL_NORMAL);
	m_list.SendMessage(LVM_SETICONSPACING, 0, (LPARAM)-1);		// x86とx64で有効
	m_list.SetRedraw(TRUE);
	m_list.Invalidate();
}

void CAppBar::OnPageReportSmallIcon() { OnPageReportIcon(true); }
void CAppBar::OnPageReportLargeIcon() { OnPageReportIcon(false); }

void CAppBar::OnPageReportIcon(bool bSmall)
{
	m_list.SetRedraw(FALSE);
	m_list.SetViewType(LVS_REPORT);
	CRect rc;
	m_list.GetClientRect(&rc);
	m_list.SetColumnWidth(0, rc.Width());
	CAppGroup* group = GetCurrentPage();
	if (group)
		group->m_typeIcon = bSmall ? GIT_SMALL : GIT_LIST;
	m_list.SetImageList(bSmall ? m_himSmall : m_himLarge, LVSIL_SMALL);
	m_list.SetRedraw(TRUE);
	m_list.Invalidate();
}

void CAppBar::OnPageHideBar()
{
	GetTopLevelParent().SendMessage(WM_COMMAND, IDM_BAR_APPBAR, 0);
}

void CAppBar::OnPageRename()
{
	CAppGroup* group = GetCurrentPage();
	if (group)
		RenamePage(group);
}

void CAppBar::OnPageImport()
{
	CString str;
	str.LoadString(IDS_APPBAR_BROWSE);

	TCHAR name[MAX_PATH];
	BROWSEINFO bi;
	memset(&bi, 0, sizeof(bi));
	bi.hwndOwner = m_hWnd;
	bi.pidlRoot = NULL;
	bi.pszDisplayName = name;
	bi.lpszTitle = str.GetString();
	bi.ulFlags = BIF_USENEWUI/*BIF_NEWDIALOGSTYLE*/;

	CIDListPtr idlParent = SHBrowseForFolder(&bi);
	if (!idlParent.isNull()) {
		CComPtr<IShellFolder> psf = idlParent.GetShellFolder();
		if (!psf) {
			SHFILEINFO sfi;
			idlParent.GetFileInfo(0, &sfi, sizeof(sfi), SHGFI_ATTRIBUTES);
			if (sfi.dwAttributes & SFGAO_FILESYSANCESTOR)
				SHGetDesktopFolder(&psf);
		}

		if (psf.p != NULL) {
			CAppGroup* group = new CAppGroup(name);
			group->m_clrBack = clrPanelColors[m_page.GetCount() % ARRAYSIZE(clrPanelColors)];

			int index = (int)m_page.Add(group);
			ResizeList();
			SelectPage(index);

			LPITEMIDLIST pidl;
			CComPtr<IEnumIDList> pe;
			if (psf->EnumObjects(m_hWnd, SHCONTF_NONFOLDERS | SHCONTF_FOLDERS | SHCONTF_FASTITEMS, &pe) == S_OK) {
//@				ULONG celt;
//@				while (pe->Next(1, &pidl, &celt) == NOERROR && celt > 0) {
				while (pe->Next(1, &pidl, NULL) == NOERROR) {
					CIDListPtr idlFq;
					if (idlFq.Combine(idlParent, pidl)) {
						AddItem(idlFq);
						idlFq.Free();
					}
					CoTaskMemFree(pidl);
				}
			}
			SaveState();
		}
	}
}

void CAppBar::OnPageBkgnd()
{
	CAppGroup* group = GetCurrentPage();
	if (group) {
		CPanelPropertyDlg dlg;
		dlg.m_clrText = group->m_clrText;
		dlg.m_clrBack = group->m_clrBack;
		dlg.m_strName = group->m_strGroupName;
		if (dlg.DoModal() == IDOK) {
			group->m_clrText = dlg.m_clrText;
			group->m_clrBack = dlg.m_clrBack;
			group->m_strGroupName = dlg.m_strName;
			SelectPage(m_nCurPage);
		}
	}
}

static int __cdecl __compare(const void* arg1, const void* arg2, const void* /*param*/)
{
	CAppItem* pai1 = *(CAppItem**)arg1;
	CAppItem* pai2 = *(CAppItem**)arg2;
	return pai1->m_strDisplayName.CompareNoCase(pai2->m_strDisplayName);
}

void CAppBar::OnPageSort()
{
	CAppGroup* group = GetCurrentPage();
	if (group) {
		int cnt = (int)group->m_array.GetCount();
		if (cnt > 1) {
			qsort_ex(group->m_array.GetData(), cnt, sizeof(CAppItem*), __compare, NULL);
			SaveState();
			SelectPage(m_nCurPage);
		}
	}
}

void CAppBar::OnPageDelete()
{
	CAppGroup* group = GetCurrentPage();
	if (group && AppMessage(IDS_APPBAR_DELGROUP, group->m_strGroupName, MB_QYESNO) == IDYES) {
		m_page.RemoveAt(m_nCurPage);
		delete group;
		if (m_nCurPage > 0)
			m_nCurPage--;

		if (m_page.GetCount() > 0)
			SelectPage(m_nCurPage);
		else {
			m_list.DeleteAllItems();
			RedrawWindow();
		}
		SaveState();
	}
}

void CAppBar::RenamePage(CAppGroup* group)
{
	if (group) {
		CRect rc = group->m_rc;
		ClientToScreen(&rc);
		TCHAR buf[MAX_PATH * 2];
		lstrcpy(buf, group->m_strGroupName);
		if (lccTipEdit(GetTopLevelParent(), &rc, buf)) {
			group->m_strGroupName = buf;
			SaveState();
			InvalidateRect(&group->m_rc, TRUE);
		}
	}
}

CAppGroup* CAppBar::AddPage(LPCTSTR pszTitle)
{
	CAppGroup* group = new CAppGroup(pszTitle);
	group->m_clrBack = clrPanelColors[m_page.GetCount() % ARRAYSIZE(clrPanelColors)];
	m_page.Add(group);

	ResizeList();
	UpdateWindow();
	return group;
}

void CAppBar::OnPageAdd()
{
	CString cs;
	cs.LoadString(IDS_APPBAR_NEWGROUP);
	CAppGroup* group = AddPage(cs);
	RenamePage(group);
}

CAppItem* CAppBar::GetSelectedItem()
{
	int index = m_list.GetNextItem(-1, LVNI_SELECTED | LVNI_FOCUSED);
	if (index != -1) {
		return (CAppItem*)m_list.GetItemData(index);
	}
	return NULL;
}

void CAppBar::OnItemOpen()
{
	CAppItem* pai = GetSelectedItem();
	if (pai) {
		if (_ISKEYDOWN(VK_MENU))
			pai->ShowProperty(m_hWnd);
		else
			pai->Open();
	}
}

void CAppBar::OnItemDelete()
{
	CAppGroup* group = GetCurrentPage();
	if (group) {
		int index = m_list.GetNextItem(-1, LVNI_SELECTED);
		if (index != -1) {
			CAppItem* pai = (CAppItem*)group->m_array[index];
			if (AppMessage(IDS_QUERY_DELPAGEITEM, pai->m_strDisplayName, MB_QYESNO) == IDYES) {
				group->m_array.RemoveAt(index);
				m_list.DeleteItem(index);
				delete pai;

				index = m_list.GetNextItem(-1, LVNI_FOCUSED);
				if (index != -1)
					m_list.SelectItem(index);
				SaveState();
			}
		}
	}
}

void CAppBar::OnItemRename()
{
	int index = m_list.GetNextItem(-1, LVNI_SELECTED | LVNI_FOCUSED);
	if (index != -1)
		m_list.EditLabel(index);
}

void CAppBar::OnItemIcon()
{
	int index = m_list.GetNextItem(-1, LVNI_SELECTED | LVNI_FOCUSED);
	if (index != -1) {
		CAppItem* pai = (CAppItem*)m_list.GetItemData(index);
		if (pai) {
			TCHAR path[MAX_PATH * 2];
			lstrcpy(path, pai->m_strIconLocation);
			int iIcon = PathParseIconLocation(path);
			if (fopGetIconPath(GetTopLevelParent(), path, &iIcon)) {
				pai->SetIconPath(path, iIcon);
				m_list.SetItem(index, 0, LVIF_IMAGE, NULL, I_IMAGECALLBACK, 0, 0, 0);
				m_list.RedrawItems(index, index);
				SaveState();
			}
		}
	}
}

void CAppBar::OnItemProperty()
{
	CAppItem* pai = GetSelectedItem();
	if (pai) {
		pai->ShowProperty(m_hWnd);
	}
}

void CAppBar::OnItemOpenSrcDir()
{
	CAppItem* pai = GetSelectedItem();
	if (pai) {
		if (pai->isFolder()) {
			// 対象がフォルダなら、フォルダを開く
			fvApp.OpenFolder(pai->m_idl);
		}
		else {
			// 対象がフォルダで無いなら、親フォルダを開き、アイテムを選択する
			GetTopLevelParent().SendMessage(WM_SELECTFOLDERITEM, 0, (LPARAM)pai->m_idl.GetData());
		}
	}
}

void CAppBar::OnGroupMoveUp()
{
	if (m_nCurPage > 0) {
		CAppGroup* group1 = GetPage(m_nCurPage);
		CAppGroup* group2 = GetPage(m_nCurPage - 1);
		if (group1 && group2) {
			m_page[m_nCurPage]     = group2;
			m_page[m_nCurPage - 1] = group1;
			SelectPage(m_nCurPage - 1);
			SaveState();
		}
	}
}

void CAppBar::OnGroupMoveDown()
{
	int cnt = (int)m_page.GetCount();
	if (m_nCurPage + 1 < cnt) {
		CAppGroup* group1 = GetPage(m_nCurPage);
		CAppGroup* group2 = GetPage(m_nCurPage + 1);
		if (group1 && group2) {
			m_page[m_nCurPage]     = group2;
			m_page[m_nCurPage + 1] = group1;
			SelectPage(m_nCurPage + 1);
			SaveState();
		}
	}
}

void CAppBar::OnItemMoveUp()
{
	CAppGroup* group = GetCurrentPage();
	int sel = m_list.GetNextItem(-1, LVNI_SELECTED);
	if (group && sel > 0) {
		CAppItem* paiSrc = (CAppItem*)m_list.GetItemData(sel);
		CAppItem* paiDst = (CAppItem*)m_list.GetItemData(sel - 1);
		group->m_array.SetAt(sel, paiDst);
		group->m_array.SetAt(sel - 1, paiSrc);
		m_list.SetItem(sel, 0, LVIF_TEXT | LVIF_IMAGE | LVIF_PARAM,
						paiDst->m_strDisplayName, I_IMAGECALLBACK,
						0, 0,
						(LPARAM)paiDst);
		m_list.SetItem(sel - 1, 0, LVIF_TEXT | LVIF_IMAGE | LVIF_PARAM | LVIF_STATE,
						paiSrc->m_strDisplayName, I_IMAGECALLBACK,
						LVIS_SELECTED | LVIS_FOCUSED, LVIS_SELECTED | LVIS_FOCUSED,
						(LPARAM)paiSrc);
		SaveState();
	}
}

void CAppBar::OnItemMoveDown()
{
	CAppGroup* group = GetCurrentPage();
	int sel = m_list.GetNextItem(-1, LVNI_SELECTED);
	if (group && sel != -1 && sel < m_list.GetItemCount() - 1) {
		CAppItem* paiSrc = (CAppItem*)m_list.GetItemData(sel);
		CAppItem* paiDst = (CAppItem*)m_list.GetItemData(sel + 1);
		group->m_array.SetAt(sel, paiDst);
		group->m_array.SetAt(sel + 1, paiSrc);
		m_list.SetItem(sel, 0, LVIF_TEXT | LVIF_IMAGE | LVIF_PARAM,
						paiDst->m_strDisplayName, I_IMAGECALLBACK,
						0, 0,
						(LPARAM)paiDst);
		m_list.SetItem(sel + 1, 0, LVIF_TEXT | LVIF_IMAGE | LVIF_PARAM | LVIF_STATE,
						paiSrc->m_strDisplayName, I_IMAGECALLBACK,
						LVIS_SELECTED | LVIS_FOCUSED, LVIS_SELECTED | LVIS_FOCUSED,
						(LPARAM)paiSrc);
		SaveState();
	}
}

void CAppBar::OnItemOpenNewTab()
{
	CAppItem* pai = GetSelectedItem();
	if (pai && pai->isFolder()) {
		fvApp.OpenTabFolder(pai->m_idl, FALSE);
	}
}

void CAppBar::OnItemOpenNewWindow()
{
	CAppItem* pai = GetSelectedItem();
	if (pai && pai->isFolder()) {
		ExecNewFrame(pai->m_idl);
	}
}


void CAppBar::OnBarLeft()
{
	if (gnBarStat & BTS_APPRIGHT) GetTopLevelParent().SendMessage(WM_COMMAND, IDM_BAR_APPBAR_SIDECHANGE, 0);
}

void CAppBar::OnBarRight()
{
	if (!(gnBarStat & BTS_APPRIGHT)) GetTopLevelParent().SendMessage(WM_COMMAND, IDM_BAR_APPBAR_SIDECHANGE, 0);
}

static bool GetAppFullPath(LPCTSTR pszExecName, CString& path)
{
	path.Empty();
	TCHAR buf[MAX_PATH * 2];
	// レジストリから検索
	if (lccRegGetAppPath(pszExecName, buf, ARRAYSIZE(buf))) {
		path = buf;
		return true;
	}
	// まずSystem32ディレクトリ
	GetSystemDirectory(buf, ARRAYSIZE(buf));
	PathAppend(buf, pszExecName);
	if (_pathfileexists(buf)) {
		path = buf;
		return true;
	}
	// Windowsディレクトリ
	GetWindowsDirectory(buf, ARRAYSIZE(buf));
	PathAppend(buf, pszExecName);
	if (_pathfileexists(buf)) {
		path = buf;
		return true;
	}
	return false;
}

struct DEFAPPENTRY {
	int type;			// 0=LPCTSTR, 1=CSIDL_xxx, 2=index of FOLDERID_xxx
	LPCTSTR exec;
	LPCTSTR title;
};

static GUID stKnownFolderID[] = {
	FOLDERID_ComputerFolder,			//
	FOLDERID_ControlPanelFolder,
};


static DEFAPPENTRY stSystemApp[] = {
	2, (LPCTSTR)0,				NULL,		// マイコンピュータ
	2, (LPCTSTR)1,				NULL,		// コントロールパネル
	0, _T("powershell.exe"),	NULL,		// PowerShell
	0, _T("mobsync.exe"),		NULL,		// 同期センター
	0, NULL,					NULL
};

static DEFAPPENTRY stSysManApp[] = {
	0, _T("compmgmt.msc"),	(LPCTSTR)IDS_APPBARITEM_COMPMGMT,	// 管理ツール
	0, _T("diskmgmt.msc"),	(LPCTSTR)IDS_APPBARITEM_DISKMGMT,	// ディスクの管理
	0, _T("fsmmgmt.msc"),	(LPCTSTR)IDS_APPBARITEM_FSMMGMT,	// 共有フォルダ
	0, _T("devmgmt.msc"),	(LPCTSTR)IDS_APPBARITEM_DEVMGMT,	// デバイスマネージャ
	0, _T("gpedit.msc"),	(LPCTSTR)IDS_APPBARITEM_GPEDIT,		// ローカルグループポリシー
	0, _T("secpol.msc"),	(LPCTSTR)IDS_APPBARITEM_SECPOL,		// ローカルセキュリティポリシー
	0, _T("mstsc.exe"),		NULL,								// リモートデスクトップ
	0, _T("msconfig.exe"),	NULL,								// システム構成
	0, _T("SystemPropertiesAdvanced.exe"),	NULL,				// システムの詳細設定
	0, _T("msinfo32.exe"),	NULL,								// システム情報
	0, _T("wf.msc"),		(LPCTSTR)IDS_APPBARITEM_WF,			// ファイアウォール
	0, _T("printmanagement.msc"),(LPCTSTR)IDS_APPBARITEM_PRINTMAN,	// 印刷の管理
	0, NULL,					NULL
};

static DEFAPPENTRY stWindowApp[] = {
	0, _T("winmail.exe"),	NULL,								// Windows Mail
	0, _T("wab.exe"),		NULL,								// Windows Address Book
	0, _T("wincal.exe"),	NULL,								// Windows Calendar
	0, _T("wincollab.exe"),	NULL,								// Windows Meeting Space
	0, _T("calc.exe"),		NULL,								// 電卓
	0, _T("stikynot.exe"),	NULL,								// 付箋
	0, _T("snippingtool.exe"),	NULL,							// Snipping tool
	0, _T("wfs.exe"),		NULL,								// Windows FAXとスキャン
	0, _T("wmenc.exe"),		NULL,						
	0, _T("msimn.exe"),		NULL,								// Outlook Express
	0, NULL,				NULL,
};

static DEFAPPENTRY stOfficeApp[] = {
	0, _T("winword.exe"),	NULL,		// Word
	0, _T("excel.exe"),		NULL,		// excel
	0, _T("powerpnt.exe"),	NULL,		// Power Point
	0, _T("outlook.exe"),	NULL,		// Outlook
	0, _T("msaccess.exe"),	NULL,		// Access
	0, _T("mspub.exe"),		NULL,		// Publisher
	0, _T("onenote.exe"),	NULL,		// One Note
//	0, _T("groove.exe"),	NULL,		// OneDrive
//	0, _T("lync.exe"),		NULL,		// Skype for Biz
	0, NULL,				NULL,
};

static DEFAPPENTRY stDesignApp[] = {
	0, _T("wordpad.exe"),	NULL,		// WordPad
	0, _T("windowsphotogallary.exe"),NULL,		// Windows Photo Gallery
	0, _T("wmplayer.exe"),	NULL,
	0, _T("moviemk.exe"),	NULL,		// Windows Movie Maker
	0, _T("dvdmaker.exe"),	NULL,		// Windows DVD Maker
	0, _T("photoshop.exe"),	NULL,
	0, _T("illustrator.exe"),	NULL,
	0, _T("pixeflt.exe"),	NULL,
//	0, _T("pbrush.exe"),	NULL,		// ペイント(mspaint.exe)
	0, NULL,				NULL,
};

typedef struct tagDEFAPPGROUP {
	LPCTSTR title;
	DEFAPPENTRY* table;
} DEFAPPGROUP;

static DEFAPPGROUP stDefAppGroups[] = {
	(LPCTSTR)IDS_APPBARGROUP_SHELL,	stSystemApp,
	(LPCTSTR)IDS_APPBARGROUP_MANAGE,stSysManApp,
	_T("Windows"),					stWindowApp,
	_T("Office"),					stOfficeApp,
	_T("Design"),					stDesignApp,
	NULL,							NULL,
};

void CAppBar::LoadDefaultState()
{
	// グループを設定
	CString tit, name;
	DEFAPPGROUP* group_ptr = stDefAppGroups;
	while (group_ptr->title != NULL) {
		// グループ作成
		if (IS_INTRESOURCE(group_ptr->title))
			tit.LoadString(LOWORD(group_ptr->title));
		else
			tit = group_ptr->title;
		AddPage(tit);
		SelectPage(++m_nCurPage);

		DEFAPPENTRY* table = group_ptr->table;
		while (table->exec != NULL) {
			// タイトル
			tit.Empty();
			if (table->title) {
				if (IS_INTRESOURCE(table->title))
					tit.LoadString(PtrToInt(table->title));
				else
					tit = table->title;
			}
			// アプリケーション
			LPITEMIDLIST pidl;
			switch (table->type) {
			case 0:
				if (GetAppFullPath(table->exec, name)) {
					AddItem(name, -1, tit);
				}
				break;
			case 1:
				if (SHGetSpecialFolderLocation(NULL, PtrToInt(table->exec), &pidl) == S_OK) {
					AddItem(pidl, -1, tit);
					CoTaskMemFree(pidl);
				}
				break;
			case 2:
				if (SHGetKnownFolderIDList(stKnownFolderID[PtrToInt(table->exec)], 0, NULL, &pidl) == S_OK) {
					AddItem(pidl, -1, tit);
					CoTaskMemFree(pidl);
				}
				break;
			}
			table++;
		}
		group_ptr++;
	}
}

BOOL CAppBar::LoadState()
{
	if (LoadPanelGroups() < 0) {
		// データファイルがなかったので初期設定を読む
		LoadDefaultState();
		SelectPage(0);
	}
	return TRUE;
}

BOOL CAppBar::SaveState()
{
	return SavePanelGroups();
}

static void BuildPanelGroupFileName(CString& str)
{
	GetAppDataFolder(str, _T("filevisor7.panels"));
}

bool CAppBar::SavePanelGroups()
{
	char ssz[MAX_PATH * 2];
	// Save XML
	TiXmlDocument xmlDoc;
	xmlDoc.InsertEndChild( TiXmlDeclaration( "1.0", "UTF-8", "yes" ) );
	// --------------------------
	// セレクション
	// --------------------------
	TiXmlNode* pDoc = xmlDoc.InsertEndChild( TiXmlElement( "selection" ) );
	TiXmlElement* pRoot = pDoc->ToElement();
	pRoot->SetAttribute( "curpage", m_nCurPage );

	int numGroups = (int)m_page.GetCount();
	for (int i = 0; i < numGroups; i++) {
		CAppGroup* group = m_page[i];
		pDoc = xmlDoc.InsertEndChild( TiXmlElement( "group" ) );
		TiXmlElement* pGroup = pDoc->ToElement();
		//--------------------------------
		// グループ情報
		//--------------------------------
		// グループ名
		WideCharToMultiByte(CP_UTF8, 0, group->m_strGroupName, -1, ssz, ARRAYSIZE(ssz), NULL, NULL);
		pGroup->SetAttribute( "name", ssz );
		// 文字色
		pGroup->SetAttribute( "clrText", group->m_clrText );
		// 背景色
		pGroup->SetAttribute( "clrBack", group->m_clrBack );
		// アイコン状態
		pGroup->SetAttribute( "style", group->m_typeIcon );
		// フォーカス
		pGroup->SetAttribute( "focused", group->m_nFocused );
		//--------------------------------
		// エントリー項目
		//--------------------------------
		int numItems = (int)group->m_array.GetCount();
		for (int j = 0; j < numItems; j++) {
			CAppItem* pai = group->m_array[j];
			TiXmlNode* pItem = pGroup->InsertEndChild( TiXmlElement( "item" ) );
			TiXmlElement* pElement = pItem->ToElement();
			XmlWriteIDListCore(pElement, pai->m_idl, pai->m_strDisplayName);
			// guid
			CString str;
			pai->GetGuid(str);
			WideCharToMultiByte(CP_UTF8, 0, str, -1, ssz, ARRAYSIZE(ssz), NULL, NULL);
			pElement->SetAttribute( "guid", ssz );
			// 引数
			if (!pai->m_strParams.IsEmpty()) {
				WideCharToMultiByte(CP_UTF8, 0, pai->m_strParams, -1, ssz, ARRAYSIZE(ssz), NULL, NULL);
				pElement->SetAttribute( "params", ssz );
			}
			// アイコンパス
			if (!pai->m_strIconLocation.IsEmpty()) {
				WideCharToMultiByte(CP_UTF8, 0, pai->m_strIconLocation, -1, ssz, ARRAYSIZE(ssz), NULL, NULL);
				pElement->SetAttribute( "iconPath", ssz );
			}
		}
	}
	// Save XML
	CString str;
	BuildPanelGroupFileName(str);
	WideCharToMultiByte(CP_ACP, 0, str, -1, ssz, ARRAYSIZE(ssz), NULL, NULL);
	// 書き出し可能か？
	if (!CheckFileWritable(ssz)) {
		;	// とりあえずreadonly属性がついていれば解除される
	}
	return xmlDoc.SaveFile(ssz);
}

int CAppBar::LoadPanelGroups()
{
	CString str;
	BuildPanelGroupFileName(str);
	char xml[MAX_PATH * 2];
	WideCharToMultiByte(CP_ACP, 0, str, -1, xml, ARRAYSIZE(xml), NULL, NULL);

	TiXmlDocument xmlDoc;
	if (!xmlDoc.LoadFile( xml )) return -1;

	const char* psz;
	WCHAR wsz[MAX_PATH * 2];
	TiXmlNode* pDoc = xmlDoc.ToDocument();
	if (pDoc) {
		//--------------------------------
		// セレクション
		//--------------------------------
		TiXmlElement* pSelNode = pDoc->FirstChildElement("selection");
		if (pSelNode->QueryIntAttribute("curpage", &m_nCurPage) != TIXML_SUCCESS) m_nCurPage = 0;
		//--------------------------------
		// グループ情報
		//--------------------------------
		TiXmlElement* pGroup = pDoc->FirstChildElement("group");
		while (pGroup) {
			CAppGroup* group = new CAppGroup();
			// グループ名
			wsz[0] = 0;
			psz = pGroup->Attribute("name");
			if (psz) {
				MultiByteToWideChar(CP_UTF8, 0, psz, -1, wsz, ARRAYSIZE(wsz));
				group->m_strGroupName = wsz;
			}
			int val;
			// 文字色
			if (pGroup->QueryIntAttribute("clrText", &val) != TIXML_SUCCESS) val = 0;
			group->m_clrText = (COLORREF)val;
			// 背景色
			if (pGroup->QueryIntAttribute("clrBack", &val) != TIXML_SUCCESS) val = RGB(255,255,255);
			group->m_clrBack = (COLORREF)val;
			if (group->m_clrBack == CLR_DEFAULT)
				group->m_clrBack = clrPanelColors[m_page.GetCount() % ARRAYSIZE(clrPanelColors)];
			// アイコン状態
			if (pGroup->QueryIntAttribute("style", &group->m_typeIcon) != TIXML_SUCCESS) group->m_typeIcon = 0;
			// フォーカス
			if (pGroup->QueryIntAttribute("focused", &group->m_nFocused) != TIXML_SUCCESS) group->m_nFocused = 0;
			m_page.Add(group);
			//--------------------------------
			// アイテム情報
			//--------------------------------
			TiXmlElement* pItem = pGroup->FirstChildElement("item");
			while (pItem) {
				CAppItem* pai = new CAppItem();
				pai->m_idl.Attach( XmlLoadIDListCore(pItem, pai->m_strDisplayName) );
/* --- 以前の構造データを読む --- */
				if (pai->m_idl.isNull()) {
					// 表示名
					psz = pItem->Attribute("name");
					if (psz) {
						MultiByteToWideChar(CP_UTF8, 0, psz, -1, wsz, ARRAYSIZE(wsz));
						pai->m_strDisplayName = wsz;
					}
					// ITEMIDLIST
					TiXmlElement* pIdlist = pItem->FirstChildElement("idlist");
					if (pIdlist) {
						// datasize
						int idlsize; if (pIdlist->QueryIntAttribute("size", &idlsize) != TIXML_SUCCESS) idlsize = 0;
						// crc
						int crc; if (pIdlist->QueryIntAttribute("crc", &crc) != TIXML_SUCCESS) crc = 0;
						// idlist
						int size;
						BYTE* data = b64Decode(pIdlist->Attribute("body"), &size);
						if (data) {
							// サイズとcrcのチェック
							if (size >= idlsize && lccCalcCrc16(data, idlsize) == crc)
								pai->m_idl.Clone((LPCITEMIDLIST)data);
							free(data);
						}
					}
				}
/* --- ここまで ------------------ */
				if (!pai->m_idl.isNull()) {
					// guid
					psz = pItem->Attribute("guid");
					if (psz) {
						MultiByteToWideChar(CP_UTF8, 0, psz, -1, wsz, ARRAYSIZE(wsz));
						pai->SetGuid(wsz);
					}
					// 引数
					psz = pItem->Attribute("params");
					if (psz) {
						MultiByteToWideChar(CP_UTF8, 0, psz, -1, wsz, ARRAYSIZE(wsz));
						pai->m_strParams = wsz;
					}
					// アイコンパス
					psz = pItem->Attribute("iconPath");
					if (psz) {
						MultiByteToWideChar(CP_UTF8, 0, psz, -1, wsz, ARRAYSIZE(wsz));
						pai->m_strIconLocation = wsz;
					}
					group->m_array.Add(pai);
				}
				else
					delete pai;

				pItem = pItem->NextSiblingElement("item");
			}
			pGroup = pGroup->NextSiblingElement("group");
		}
	}
	return (int)m_page.GetCount();
}

void CAppBar::Refresh()
{
	SelectPage(m_nCurPage);
}

LRESULT CAppBar::OnHelpHitTest(WPARAM /*wParam*/, LPARAM /*lParam*/)
{
	return (HID_BASE_CONTROL + GetDlgCtrlID());
}
