//////////////////////////////////////////////////////////////////////
// syslink.cpp
// シンボルリンク・ジャンクション・ハードリンク・ショートカットを選択してリンク作成

#include "stdafx.h"
#include "fopstd.h"
#include "reparse.h"

////////////////////////////////////////////////////////////////////////
//

static DWORD GetVolumeSysFlag(int driveNumber)
{
  TCHAR buf[MAX_PATH];
  PathBuildRoot(buf, driveNumber);
  DWORD flen, sflag;
  return GetVolumeInformation(buf, NULL, 0, NULL, &flen, &sflag, NULL, 0) ? sflag : 0;
}

static BOOL SetPrivilege(LPCTSTR lpszPrivilege, BOOL bEnablePrivilege)
{
  BOOL bOK = FALSE;
  HANDLE hToken;
  if (OpenProcessToken(GetCurrentProcess(), TOKEN_ADJUST_PRIVILEGES | TOKEN_QUERY, &hToken)) {
    LUID luid;
    if (LookupPrivilegeValue(NULL, lpszPrivilege, &luid)) {
      TOKEN_PRIVILEGES tp;
      tp.PrivilegeCount = 1;
      tp.Privileges[0].Luid = luid;
      if (bEnablePrivilege)
        tp.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;
      else
        tp.Privileges[0].Attributes = 0;

      // Enable the privilege or disable all privileges.
      bOK = AdjustTokenPrivileges(hToken, FALSE, &tp, sizeof(TOKEN_PRIVILEGES), (PTOKEN_PRIVILEGES)NULL, (PDWORD)NULL);
      if (bOK) {
        if (GetLastError() == ERROR_NOT_ALL_ASSIGNED) {
          // The token does not have the specified privilege.
          bOK = FALSE;
        }
      }
    }
    CloseHandle(hToken);
  }
  return bOK;
}

class CMakeSysLink : public CDialogImpl<CMakeSysLink>
{
public:
  CMakeSysLink() {
    m_src.Empty();
    m_base.Empty();
    m_path.Empty();
    m_linkType = _T("symblic");	// "symblic", "junction", "hard", "shortcut"
  }
  ~CMakeSysLink() { }

  enum { IDD = IDD_MAKESYSLINK };

BEGIN_MSG_MAP(CMakeSysLink)
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
  CString m_linkType;
};

BOOL CMakeSysLink::OnInitDialog(LPARAM lParam)
{
  CenterWindow();

  if (m_base.IsEmpty() || m_src.IsEmpty() || !_pathfileexists(m_src) || m_src[1] != _T(':')) {
    EndDialog(IDCANCEL);
  } else {
    DWORD vsflg = GetVolumeSysFlag(PathGetDriveNumber(m_src));
    UINT target;
    if (_pathisdirectory(m_src)) {
      // available: symbolic + junction 
      GetDlgItem(IDC_LINK_HARD).EnableWindow(false);
      target = IDC_LINK_JUNCTION;		// 初期値はjunctionを選択
      // このボリュームがリパースポイントをサポートしているか確認
      if (!(vsflg & FILE_SUPPORTS_REPARSE_POINTS)) {
        GetDlgItem(IDC_LINK_JUNCTION).EnableWindow(false);
        target = IDC_LINK_SYMBOLIC;		// 初期値はjunctionを選択
      }
    } else {
      // available: symbolic + hard
      GetDlgItem(IDC_LINK_JUNCTION).EnableWindow(false);
      target = IDC_LINK_HARD;			// 初期値はhard-linkを選択
      if (!(vsflg & FILE_SUPPORTS_HARD_LINKS)) {
        GetDlgItem(IDC_LINK_HARD).EnableWindow(false);
        target = IDC_LINK_SYMBOLIC;		// 初期値はjunctionを選択
      }
    }
    CheckRadioButton(IDC_LINK_SYMBOLIC, IDC_LINK_SHORTCUT, target);
    PathSetDlgItemPath(m_hWnd, IDC_TITLE, m_src);
    // 入力規則
    idl_LimitInputEdit(GetDlgItem(IDC_EDIT), m_src);

    SetDlgItemText(IDC_EDIT, PathFindFileName(m_src));
  }
  return TRUE;
}

void CMakeSysLink::BuildPath()
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

void CMakeSysLink::OnEditChange()
{
  BuildPath();
}

BOOL CMakeSysLink::OnHelpInfo(HELPINFO* pInfo)
{
  if (pInfo->iContextType == HELPINFO_WINDOW) {
    ShowHtmlHelp(m_hWnd, IDD);
    return TRUE;
  }
  return FALSE;
}

void CMakeSysLink::OnOK()
{
  if (m_path.IsEmpty() || !_pathcompare(m_base, m_path)) {
    MessageBeep(0);
    GetDlgItem(IDC_EDIT).SetFocus();
    return;
  }

  // ジャンクションが参照元の子孫ならば中止
  if (!_pathisparent(m_src, m_path)) {
    AppMessage(IDS_TITLE_SYSLINK, IDS_PATH_INCLUDE);
    GetDlgItem(IDC_EDIT).SetFocus();
    return;
  }

  // フォルダ名不適格文字のチェック
  LPTSTR p = PathFindFileName(m_path);

  // 文字列終端が ' ' または '.' で終わるフォルダ名の修正
  if (!validate_makefile(p)) {
    AppMessage(IDS_TITLE_SYSLINK, IDS_INVALIDFILENAME, p);
    return;
  }

  if (_pathfileexists(m_path)) {
     AppMessage(IDS_TITLE_SYSLINK, IDS_SAMENAME, m_path, MB_STOP);
    return;
  }

  if (IsDlgButtonChecked(IDC_LINK_SYMBOLIC)) {
    m_linkType = _T("symbolic");
  } else if (IsDlgButtonChecked(IDC_LINK_JUNCTION)) {
    m_linkType = _T("junction");
  } else if (IsDlgButtonChecked(IDC_LINK_HARD)) {
    m_linkType = _T("hard");
  } else {
    m_linkType = _T("shortcut");
  }

  EndDialog(IDOK);
}

////////////////////////////////////////////////////////////////////////
//

extern "C"
BOOL WINAPI fopCreateSystemLinkUI(HWND hwnd, LPCTSTR pszLinkBase, LPCTSTR pszSrc)
{
  if (!pszLinkBase || *pszLinkBase == 0 || pszLinkBase[1] != _T(':') || !pszSrc || *pszSrc == 0)
    return FALSE;

  // 元ファイルが既にReparsePoint？
  WIN32_FILE_ATTRIBUTE_DATA ad;
  if (!GetFileAttributesEx(pszSrc, GetFileExInfoStandard, &ad) ||
    (ad.dwFileAttributes & FILE_ATTRIBUTE_REPARSE_POINT))
    return FALSE;

  BOOL ok = FALSE;
  CMakeSysLink dlg;
  dlg.m_base = pszLinkBase;
  dlg.m_src = pszSrc;
  if (dlg.DoModal(hwnd) == IDOK) {
    if (dlg.m_linkType == _T("symbolic")) {
      BOOL bAdmin = IsUserAnAdmin();
      if (bAdmin || SetPrivilege(SE_CREATE_SYMBOLIC_LINK_NAME, TRUE)) {
        // Win7 x64では権限が無くて作成できなくてもエラーにならない！
        ok = CreateSymbolicLink(dlg.m_path.GetString(), pszSrc, _pathisdirectory(pszSrc) ? SYMBOLIC_LINK_FLAG_DIRECTORY : 0);
      } else {
        ILSFileOperation* pfo;
        if (CoCreateInstanceAsAdmin(hwnd, true, __uuidof(LSFileOperation), __uuidof(ILSFileOperation), (void**)&pfo) == S_OK) {
          pfo->MakeSymLink(hwnd, dlg.m_path.GetString(), pszSrc);
          pfo->Release();
        }
      }
    } else if (dlg.m_linkType == _T("junction")) {
      ok = fopCreateJunction(dlg.m_path.GetString(), pszSrc);
    } else if (dlg.m_linkType == _T("hard")) {
      ok = CreateHardLink(dlg.m_path.GetString(), pszSrc, NULL);
    } else {
      HRESULT hr = CreateLinkObject(pszSrc, dlg.m_path.GetString(), NULL, NULL, FALSE);
      ok = SUCCEEDED(hr) ? TRUE : FALSE;
    }

    if (!ok)
      LastError(IDS_TITLE_SYSLINK, dlg.m_path.GetString());
  }
  return ok;
}
