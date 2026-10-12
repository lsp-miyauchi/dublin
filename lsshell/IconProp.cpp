// IconProp.cpp : CIconProp の実装

#include "stdafx.h"
#include "IconProp.h"


////////////////////////////////////////////////////////////////////////////////
// CIconPropPage Overrides

BOOL CIconPropPage::OnApply(void)
{
  return TRUE;
}

void CIconPropPage::EnableButtons(BOOL enable)
{
  GetDlgItem(IDC_ICON_COPY).EnableWindow(enable);
  GetDlgItem(IDC_ICON_WRITE).EnableWindow(enable);
  GetDlgItem(IDC_ICON_DETAIL).EnableWindow(enable);
}

BOOL CIconPropPage::OnInitDialog(LPARAM lParam)
{
  m_slider = GetDlgItem(IDC_ICON_SIZE);
  m_slider.SetRange(0, 3);
  m_slider.SetPos(2);
  EnableButtons(FALSE);

  m_list = GetDlgItem(IDC_ICON_LIST);
  m_list.SetViewType(LVS_ICON);
  m_list.SetExtendedListViewStyle(m_list.GetExtendedListViewStyle() | LVS_EX_DOUBLEBUFFER);

  CWaitCursor wc;
  m_hIcm = m_fop.IcmOpen(m_name.GetString());
  int numIcons = m_fop.IcmGetIconCount(m_hIcm);
  if (numIcons > 0) {
    EnableButtons(TRUE);
    for (int i = 0; i < numIcons; i++) {
      m_list.InsertItem(LVIF_IMAGE, i, NULL, 0, 0, i, 0);
    }
    m_list.SetItemState(0, LVIS_SELECTED | LVIS_FOCUSED, LVIS_SELECTED | LVIS_FOCUSED);
    ChangeIconView(32);
  }
  return TRUE;
}

void CIconPropPage::OnHScroll(UINT nSBCode, UINT nPos, HWND hctl)
{
  if (hctl != m_slider) {
    DefWindowProc();
    return;
  }

  int val;
  switch (nSBCode) {
    case TB_THUMBPOSITION: val = (int)nPos; break;
    case TB_TOP:
    case TB_BOTTOM:
    case TB_LINEUP:
    case TB_LINEDOWN:
    case TB_PAGEUP:
    case TB_PAGEDOWN: val = m_slider.GetPos(); break;
    default: return;
  }
  int dx;
  switch (val) {
    case 0:  dx = 16; break;
    case 1:  dx = 24; break;
    case 2:  dx = 32; break;
    default: dx = 48; break;
  }

  ChangeIconView(dx);
}

void CIconPropPage::ChangeIconView(int dx)
{
  HIMAGELIST him = m_fop.IcmGetImageList(m_hIcm, dx);
  m_list.SetIconSpacing(dx + 8, dx + 8);
  m_list.SetImageList(him, LVSIL_NORMAL);

  int index = m_list.GetNextItem(-1, LVNI_SELECTED);
  if (index >= 0) {
    m_list.EnsureVisible(index, TRUE);
  }
}

void CIconPropPage::OnNotifyBeginDrag(NMHDR* /*pNmhdr*/, LRESULT* /*pResult*/)
{
  LPCTSTR pszIconPath = m_fop.IcmGetIconPath(m_hIcm);
  if (pszIconPath == NULL)
    return;

  CPoint pt;
  GetCursorPos(&pt);

  // ドラッグ＆ドロップ処理
  int cnt = m_list.GetSelectedCount();
  if (cnt < 1) return;

  CSimpleArray<int> arr;
  int index = -1;
  while (1) {
    index = m_list.GetNextItem(index, LVNI_ALL | LVNI_SELECTED);
    if (index < 0)
      break;
    arr.Add(index);
  }

  if (lccBeginTrack(m_hWnd, pt)) {
    IDataObject* pdo;
    if (m_fop.IcmCreateDataObject(m_hIcm, arr.GetData(), arr.GetSize(), &pdo) == S_OK) {
      DWORD eff = DROPEFFECT_COPY;
      SHDoDragDrop(m_hWnd, pdo, NULL, eff, &eff);
      pdo->Release();
    }
  }
}

void CIconPropPage::OnExportIcons()
{
  LPCTSTR pszIconPath = m_fop.IcmGetIconPath(m_hIcm);
  if (pszIconPath == NULL)
    return;

  int cnt = m_list.GetSelectedCount();
  if (cnt < 1) {
    AppMessage(IDR_ICONPROP, IDS_NOSELICONS, NULL, MB_STOP);
    return;
  }

  CSimpleArray<int> arrSel;
  int n = -1;
  for (int i = 0; i < cnt; i++) {
    arrSel.Add( m_list.GetNextItem(n, LVNI_SELECTED) );
  }
  TCHAR path[MAX_PATH * 2];
  PathGetParent(m_name, path);

  if (cnt == 1) {
    TCHAR name[MAX_PATH * 2];
    lstrcpy(name, m_name);
    wsprintf(&name[lstrlen(name)], _T(".(#%d).ico"), arrSel[0]);
    if (FileBrowse(FALSE, m_hWnd, IDS_TITLE_ICONPATH, IDS_FILTER_ICON, path, name))
      m_fop.ExportSingleIcon(pszIconPath, name, arrSel[0]);
  } else {
    if (m_fop.BrowseForFolder(m_hWnd, path)) {
      m_fop.ExportMultipleIcons(pszIconPath, path, arrSel.GetData(), cnt);
    }
  }
}

void CIconPropPage::OnShowIconDetail()
{
  LPCTSTR pszIconPath = m_fop.IcmGetIconPath(m_hIcm);
  if (pszIconPath == NULL)
    return;

  int sel = m_list.GetNextItem(-1, LVNI_SELECTED);
  if (sel < 0) {
    AppMessage(IDR_ICONPROP, IDS_NOSELICONS, NULL, MB_STOP);
    return;
  }
  m_fop.ShowIconDetail(m_hWnd, pszIconPath, sel);
}


// CIconProp

HRESULT CIconProp::Initialize(LPCITEMIDLIST pidlFolder, LPDATAOBJECT lpdobj, HKEY hKeyProgID)
{
  m_userlevel = _check_user();
  if (m_userlevel == USRL_NOUSER) return E_INVALIDARG;

  DWORD arc, etc, img;
  GetShellSetting(arc, etc, img);
  if (!(etc & SHLE_PROPICON)) return E_INVALIDARG;

  FORMATETC fmt = { CF_HDROP, NULL, DVASPECT_CONTENT, -1, TYMED_HGLOBAL };
  STGMEDIUM stg = { TYMED_HGLOBAL };

  HRESULT hr = lpdobj->GetData(&fmt, &stg);
  if (FAILED(hr)) return E_INVALIDARG;

  m_file = _T("");
  UINT cnt = DragQueryFile((HDROP)stg.hGlobal, 0xFFFFFFFF, NULL, 0);
  if (cnt == 1) {
    TCHAR buf[MAX_PATH * 2];
    DragQueryFile((HDROP)stg.hGlobal, 0, buf, ARRAYSIZE(buf));
    LPCTSTR pe = PathFindExtension(buf);
    if (!StrCmpI(pe, _T(".exe")) || !StrCmpI(pe, _T(".dll")) || !StrCmpI(pe, _T(".cpl"))) {
      m_file = buf;
    }
  }

  GlobalUnlock(stg.hGlobal);
  ReleaseStgMedium(&stg);
  return (m_file.IsEmpty() ? E_INVALIDARG : S_OK);
}

/////////////////////////////////////////////////////////////////////////////
// IShellPropSheetExt

HRESULT CIconProp::AddPages(LPFNADDPROPSHEETPAGE lpfnAddPage, LPARAM lParam)
{
  if (m_file.IsEmpty()) return E_FAIL;

  CIconPropPage* page = new CIconPropPage;
  page->m_name = m_file;

  // Add small icon (assuming resource identifier IDI_ICON)
//	page->m_psp.pszIcon = MAKEINTRESOURCE(IDI_ICON);
//	page->m_psp.dwFlags |= PSP_USEICONID;

  HPROPSHEETPAGE hPage = page->Create();
  if (!lpfnAddPage(hPage, lParam)) delete page;
  return S_OK;
}

