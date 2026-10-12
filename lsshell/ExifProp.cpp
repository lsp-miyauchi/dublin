// ExifProp.cpp : CExifProp の実装

#include "stdafx.h"
#include "mainmodule.h"
#include "ExifProp.h"

////////////////////////////////////////////////////////////////////////////////
//

static COLORREF calc_mixed_color(COLORREF clrA, COLORREF clrB, int perA)
{
	// (9a + 1b)/10
	int r = (perA * GetRValue(clrA) + (100 - perA) * GetRValue(clrB)) / 100;
	int g = (perA * GetGValue(clrA) + (100 - perA) * GetGValue(clrB)) / 100;
	int b = (perA * GetBValue(clrA) + (100 - perA) * GetBValue(clrB)) / 100;
	return RGB(r, g, b);
}


////////////////////////////////////////////////////////////////////////////////
// CExifPropPage Overrides

BOOL CExifPropPage::OnApply(void)
{
	return TRUE;
}

LRESULT CExifPropPage::OnInitDialog(UINT msg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	m_list = GetDlgItem(IDC_LIST);
	if (_AtlModule.IsComm6()) m_list.EnableGroupView(TRUE);

	// カラム設定
	m_list.InsertColumn(0, _T("項目"), LVCFMT_LEFT, 80, 0);
	m_list.InsertColumn(1, _T("内容"), LVCFMT_LEFT, 80, 1);

	// グループ情報初期化
	memset(&m_group, 0, sizeof(m_group));
#ifdef _WIN64
	m_group.cbSize = sizeof(m_group);
#else
	m_group.cbSize = _AtlModule.OsVISTA() ? sizeof(m_group) : LVGROUP_V5_SIZE;
#endif
	m_group.mask = LVGF_GROUPID | LVGF_HEADER | LVGF_STATE | LVGF_ALIGN;
	m_group.stateMask = LVGS_NORMAL;
	m_group.state = LVGS_NORMAL;
	m_group.uAlign = LVGA_HEADER_LEFT | LVGA_FOOTER_LEFT;

	// アイテム情報初期化
	memset(&m_item, 0, sizeof(m_item));
	m_item.mask = LVIF_TEXT | LVIF_INDENT | LVIF_PARAM;
	if (_AtlModule.IsComm6()) {
		m_item.mask |= LVIF_GROUPID;
		m_item.iIndent = 1;
	}

	CComPtr<IExifParser> pep;
	HRESULT hr = pep.CoCreateInstance(__uuidof(ExifParser));
	if (SUCCEEDED(hr)) {
		pep->EnumExifInfo(m_file, (UINT_PTR)ExifProc, (LPARAM)this);
		m_list.SetColumnWidth(0, LVSCW_AUTOSIZE);
		m_list.SetColumnWidth(1, LVSCW_AUTOSIZE);
	}
	return TRUE;
}

int CALLBACK CExifPropPage::ExifProc(int type, LPCTSTR pszTitle, LPCTSTR pszText, LPARAM param)
{
	CExifPropPage* page = (CExifPropPage*)param;

	switch (type) {
	case EXIFCB_GROUP:
		page->AddItemOrGroup(true, pszTitle, NULL);
		break;
	case EXIFCB_ITEM:
		page->AddItemOrGroup(false, pszTitle, pszText);
		break;
	}
	return 1;
}

int CExifPropPage::AddItemOrGroup(bool group, LPCTSTR pszTitle, LPCTSTR pszText)
{
	CString str;
	int index = 0;
	if (group) {
		if (_AtlModule.IsComm6()) {
			m_group.iGroupId++;
#ifdef UNICODE
			m_group.pszHeader = (LPTSTR)pszTitle;
			m_group.cchHeader = lstrlen(m_group.pszHeader);
			m_list.InsertGroup(-1, &m_group);
#else
			WCHAR wsz[MAX_PATH];
			MultiByteToWideChar(CP_ACP, 0, pszTitle, -1, wsz, ARRAYSIZE(wsz));
			m_group.pszHeader = wsz;
			m_group.cchHeader = lstrlenW(wsz);
			m_list.InsertGroup(-1, &m_group);
#endif
			return 0;
		}
		m_item.lParam = 1;
	}
	else {
		if (!_AtlModule.IsComm6()) {
			str = _T("  ");
			str += pszTitle;
			pszTitle = str;
		}
		m_item.lParam = 0;
	}

	m_item.iGroupId = m_group.iGroupId;
	m_item.pszText = (LPTSTR)pszTitle;
	index = m_list.InsertItem(&m_item);
	if (pszText && *pszText) m_list.SetItemText(index, 1, pszText);
	m_item.iItem++;
	return index;
}

void CExifPropPage::OnNotifyCustomDraw(NMHDR* phdr, LRESULT* pResult)
{
	NMLVCUSTOMDRAW* pcd = (NMLVCUSTOMDRAW*)phdr;
	switch (pcd->nmcd.dwDrawStage) {
	case CDDS_PREPAINT:
		*pResult = CDRF_NOTIFYITEMDRAW;
		break;
	case CDDS_ITEMPREPAINT:
		if (!_AtlModule.IsComm6() && pcd->nmcd.lItemlParam) {
			pcd->clrText = GetSysColor(COLOR_HIGHLIGHTTEXT);
			pcd->clrTextBk = GetSysColor(COLOR_HIGHLIGHT);
		}
		else if (pcd->nmcd.dwItemSpec & 1)
			*pResult = CDRF_DODEFAULT;
		else {
			pcd->clrTextBk = calc_mixed_color(GetSysColor(COLOR_WINDOW), GetSysColor(COLOR_HIGHLIGHT), 90);
			*pResult = CDRF_NEWFONT;
		}
		break;
	default:
		*pResult = CDRF_DODEFAULT;
		break;
	}
}

////////////////////////////////////////////////////////////////////////
// CExifProp

////////////////////////////////////////////////////////////////////////
// IShellExtInit

HRESULT CExifProp::Initialize(LPCITEMIDLIST pidlFolder, LPDATAOBJECT lpdobj, HKEY hKeyProgID)
{
	m_userlevel = _check_user();
	if (m_userlevel == USRL_NOUSER) return E_INVALIDARG;

	DWORD arc, etc, img;
	GetShellSetting(arc, etc, img);
	if (!(img & SHLI_PROPEXIF)) return E_INVALIDARG;

	FORMATETC fmt = { CF_HDROP, NULL, DVASPECT_CONTENT, -1, TYMED_HGLOBAL };
	STGMEDIUM stg = { TYMED_HGLOBAL };

	HRESULT hr = lpdobj->GetData(&fmt, &stg);
	if (FAILED(hr)) return E_INVALIDARG;

	m_file = _T("");
	UINT cnt = DragQueryFile((HDROP)stg.hGlobal, 0xFFFFFFFF, NULL, 0);
	if (cnt == 1) {
		TCHAR buf[MAX_PATH * 2];
		DragQueryFile((HDROP)stg.hGlobal, 0, buf, ARRAYSIZE(buf));

		if (imgTypeIsExif(imgSpecifyImageType(buf)))
			m_file = buf;
	}

	GlobalUnlock(stg.hGlobal);
	ReleaseStgMedium(&stg);
	return (m_file.IsEmpty() ? E_INVALIDARG : S_OK);
}

/////////////////////////////////////////////////////////////////////////////
// IShellPropSheetExt

HRESULT CExifProp::AddPages(LPFNADDPROPSHEETPAGE lpfnAddPage, LPARAM lParam)
{
	if (m_file.IsEmpty()) return E_FAIL;

	CExifPropPage* page = new CExifPropPage;
	page->m_file = m_file;

	// Add small icon (assuming resource identifier IDI_ICON)
//	page->m_psp.pszIcon = MAKEINTRESOURCE(IDI_ICON);
//	page->m_psp.dwFlags |= PSP_USEICONID;

	HPROPSHEETPAGE hPage = page->Create();
	if (!lpfnAddPage(hPage, lParam)) delete page;
	return S_OK;
}

