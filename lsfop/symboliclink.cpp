//////////////////////////////////////////////////////////////////////
// symboliclink.cpp

#include "stdafx.h"
#include "fopstd.h"
#include "reparse.h"

////////////////////////////////////////////////////////////////////////
//

class CMakeSymLink : public CDialogImpl<CMakeSymLink>
{
public:
  CMakeSymLink() { m_src.Empty(); m_base.Empty(); m_path.Empty(); }
  ~CMakeSymLink() { }

  enum { IDD = IDD_MAKESYMLINK };

BEGIN_MSG_MAP(CMakeSymLink)
  ON_WM_INITDIALOG()
  ON_WM_HELPINFO()
  ON_BN_CLICKED(IDOK, OnOK)
  ON_BN_CLICKED(IDCANCEL, OnCancel)
  ON_BN_CLICKED(IDC_HELPCALL, OnHelpCall)
  ON_EN_CHANGE(IDC_EDIT, OnEditChange)
END_MSG_MAP()

  BOOL OnInitDialog(LPARAM lParam);
  BOOL OnHelpInfo(HELPINFO* pInfo);
  void OnEditChange();
  void OnOK();
  void OnCancel() { EndDialog(IDCANCEL); }
  void OnHelpCall() { ShowHtmlHelp(m_hWnd, IDD); }

  void BuildPath();

  CString m_src;
  CString m_base;
  CString m_path;
  CString m_found;
};

BOOL CMakeSymLink::OnInitDialog(LPARAM lParam)
{
  CenterWindow();

  if (m_base.IsEmpty() || m_src.IsEmpty())
    EndDialog(IDCANCEL);
  else {
    PathSetDlgItemPath(m_hWnd, IDC_TITLE, m_src);
    // 入力規則
    idl_LimitInputEdit(GetDlgItem(IDC_EDIT), m_src);

//		TCHAR name[MAX_PATH * 2];
//		_pathaddsuffix(PathFindFileName(m_src), _T("(symlink)"), name, ARRAYSIZE(name));
//		SetDlgItemText(IDC_EDIT, name);
    SetDlgItemText(IDC_EDIT, PathFindFileName(m_src));
  }
  return TRUE;
}

void CMakeSymLink::BuildPath()
{
  TCHAR* p = m_path.GetBuffer(MAX_PATH * 2);
  lstrcpy(p, m_base);
  CString str;
  if (GetDlgItemText(IDC_EDIT, str) > 0) {
    PathAppend(p, str);
  }
  m_path.ReleaseBuffer();
  SetDlgItemText(IDC_PATH, m_path);
}

void CMakeSymLink::OnEditChange()
{
  BuildPath();
}

BOOL CMakeSymLink::OnHelpInfo(HELPINFO* pInfo)
{
  if (pInfo->iContextType == HELPINFO_WINDOW) {
    ShowHtmlHelp(m_hWnd, IDD);
    return TRUE;
  }
  return FALSE;
}

void CMakeSymLink::OnOK()
{
  if (m_path.IsEmpty() || !_pathcompare(m_base, m_path)) {
    MessageBeep(0);
    GetDlgItem(IDC_EDIT).SetFocus();
    return;
  }

  // ジャンクションが参照元の子孫ならば中止
  if (!_pathisparent(m_src, m_path)) {
    AppMessage(IDS_TITLE_SYMLINK, IDS_PATH_INCLUDE);
    GetDlgItem(IDC_EDIT).SetFocus();
    return;
  }

  // フォルダ名不適格文字のチェック
  LPTSTR p = PathFindFileName(m_path);

  // 文字列終端が ' ' または '.' で終わるフォルダ名の修正
  if (!validate_makefile(p)) {
    AppMessage(IDS_TITLE_SYMLINK, IDS_INVALIDFILENAME, p);
    return;
  }

  if (_pathfileexists(m_path)) {
     AppMessage(IDS_TITLE_SYMLINK, IDS_SAMENAME, m_path, MB_STOP);
    return;
  }

  EndDialog(IDOK);
}

////////////////////////////////////////////////////////////////////////
//

extern "C"
BOOL WINAPI fopCreateSymbolicLinkUI(HWND hwnd, LPCTSTR pszLinkBase, LPCTSTR pszSrc)
{
  if (!pszLinkBase || *pszLinkBase == 0 || pszLinkBase[1] != _T(':') || !pszSrc || *pszSrc == 0)
    return FALSE;

  // 元ファイルが既にReparsePoint？
  WIN32_FILE_ATTRIBUTE_DATA ad;
  if (!GetFileAttributesEx(pszSrc, GetFileExInfoStandard, &ad) ||
    (ad.dwFileAttributes & FILE_ATTRIBUTE_REPARSE_POINT))
    return FALSE;

  BOOL ok = FALSE;
  CMakeSymLink dlg;
  dlg.m_base = pszLinkBase;
  dlg.m_src = pszSrc;
  if (dlg.DoModal(hwnd) == IDOK) {
    // Win7 x64では権限が無くて作成できなくてもエラーにならない！
    CreateSymbolicLink(dlg.m_path.GetString(), pszSrc, _pathisdirectory(pszSrc) ? SYMBOLIC_LINK_FLAG_DIRECTORY : 0);
    ok = (GetLastError() == NOERROR);
    if (!ok)
      LastError(IDS_TITLE_SYMLINK, dlg.m_path.GetString());
  }
  return ok;
}
