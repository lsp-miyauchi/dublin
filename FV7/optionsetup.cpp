// optionsetup.cpp : implementation file
//

#include "stdafx.h"
#include "fv7.h"
#include "mainfrm.h"
#include "optionsetup.h"
#include "addin.h"
#include "mapicon.h"

#ifndef SSF_SHOWSUPERHIDDEN
#define SSF_SHOWSUPERHIDDEN         0x00040000
#endif

#define INVALID_COLOR		((COLORREF)-1)

static const TCHAR sszSampleServer[] = _T("\\\\Server\\Share");


/////////////////////////////////////////////////////////////////////////////
// CStartPageDlg メッセージ ハンドラ

BOOL CStartPageDlg::OnInitDialog(LPARAM lParam)
{
  // ホーム フォルダ
  CIDListPtr idl = GetRegPidl(IDS_SEC_FOLDER, IDS_KEY_HOME);
  if (!idl.isNull())
    IDListPicker_SetIDList(GetDlgItem(IDC_BROWSE), (LPARAM)idl.GetData());

  // 起動するとき
  m_startmode = GetDlgItem(IDC_OPSTARTMODE);
  InitDlgCtrl(m_startmode, IDS_OPSTARTMODE);

  // 起動場所
  int sel = 0;
  switch (gnSysStat & STS_STARTDIR_MASK) {
  case STS_LASTDIR:			sel = 1; break;		// 前回終了時のフォルダ
  case STS_LASTTABGROUP:		sel = 2; break;		// 前回終了時のタブ グループ
  case STS_SPECIFICTABGROUP:	sel = 3; break;		// 指定のタブ グループ
  default:					sel = 0; break;		// ホーム フォルダ
  }
  m_startmode.SetCurSel(sel);

  // タブグループファイルの指定
  CString str;
  if (GetRegText(IDS_SEC_SETTINGS, IDS_KEY_STARTUP_TABGROUP, str) > 0)
    SetDlgItemText(IDC_OPTABGROUP, str);
  OnSelChangeStartMode();

  // 複数ウィンドウ
  if (gnSysStat & STS_SWITCHEXEC) CheckDlgButton(IDC_OPSWITCH, TRUE);
  // 最大化して表示
  if (gnSysStat & STS_MAXIMIZE) CheckDlgButton(IDC_OPMAX, TRUE);
  // 最初はアイコンで
  if (gnSysStat & STS_MIN1ST) CheckDlgButton(IDC_OPMIN, TRUE);
  // スプラッシュロゴ
  if (gnSysStat & STS_NOSPLASH) CheckDlgButton(IDC_OPNOSPLASH, TRUE);

  return TRUE;
}

BOOL CStartPageDlg::OnApply()
{
  if (!::IsWindow(m_hWnd)) return TRUE;

  // ここでの設定項目以外をマスクして保持
  DWORD nStat = gnSysStat & ~(STS_STARTDIR_MASK | STS_SWITCHEXEC | STS_MAXIMIZE | STS_MIN1ST | STS_NOSPLASH);

  // 起動場所
  switch (m_startmode.GetCurSel()) {
  case 1:  nStat |= STS_LASTDIR; break;			// 前回終了時のフォルダ
  case 2:  nStat |= STS_LASTTABGROUP; break;		// 前回終了時のタブ グループ
  case 3:  nStat |= STS_SPECIFICTABGROUP; break;	// 指定のタブ グループ
  default: nStat |= STS_HOMEDIR; break;			// ホーム フォルダ
  }

  // スタートアップ タブ グループ
  if ((nStat & STS_STARTDIR_MASK) == STS_SPECIFICTABGROUP) {
    CString str;
    if (GetDlgItemText(IDC_OPTABGROUP, str) > 0 && _pathfileexists(str))
      SetRegText(IDS_SEC_SETTINGS, IDS_KEY_STARTUP_TABGROUP, str);
  }

  // 複数ウィンドウ
  if (IsDlgButtonChecked(IDC_OPSWITCH))	nStat |= STS_SWITCHEXEC;
  // 最大化
  if (IsDlgButtonChecked(IDC_OPMAX))		nStat |= STS_MAXIMIZE;
  // 最初はアイコン
  if (IsDlgButtonChecked(IDC_OPMIN))		nStat |= STS_MIN1ST;
  // スプラッシュロゴ
  if (IsDlgButtonChecked(IDC_OPNOSPLASH))	nStat |= STS_NOSPLASH;

  fvApp.SetSysStat(nStat);

  // ホーム保存
  LPITEMIDLIST pidl = IDListPicker_GetIDList(GetDlgItem(IDC_BROWSE));
  SetRegBinary(IDS_SEC_FOLDER, IDS_KEY_HOME, pidl, ILGetSize(pidl));

  return TRUE;
}

void CStartPageDlg::OnSetCurrentFolder()
{
  // ホーム フォルダに現在のフォルダをセット
  CIDListPtrHandle idl = gpMainFrame->GetCurrentTabFolder();
  if (!idl.isNull())
    IDListPicker_SetIDList(GetDlgItem(IDC_BROWSE), (LPARAM)idl.GetData());
}

void CStartPageDlg::OnSelChangeStartMode()
{
  int sel = m_startmode.GetCurSel();
  GetDlgItem(IDC_OPTABGROUP).EnableWindow(sel == 3);
}

void CStartPageDlg::OnNotifyBrowseTabGroup(NMHDR* phdr, LRESULT* pResult)
{
  NMSEGETTEXT* psgt = (NMSEGETTEXT*)phdr;

  *pResult = FALSE;
  CString dir;
  GetAppDataFolder(dir);
  CString str;
  if (FileBrowse(TRUE, m_hWnd, IDS_FILTER_TABGROUP, str, dir)) {
    lstrcpyn(psgt->pszText, str, psgt->cchTextMax);
    *pResult = TRUE;
  }
}

/////////////////////////////////////////////////////////////////////////////
// CResidentPageDlg メッセージ ハンドラ

static CString GetKeyName(UINT vk, BOOL fExtended)
{
  LONG lScan = MapVirtualKey(vk, 0) << 16;
  // if it's an extended key, add the extended flag
  if (fExtended)
    lScan |= 0x01000000L;

  CString str;
  LPTSTR p = str.GetBuffer(MAX_PATH);
  ::GetKeyNameText(lScan, p, MAX_PATH);
  str.ReleaseBuffer();
  return str;
}

static void GetHotKeyName(DWORD hotkey, CString& str)
{
  str.Empty();
  WORD wCode = LOBYTE(LOWORD(hotkey));
  WORD wModifiers = HIBYTE(LOWORD(hotkey));
  if (wCode != 0 || wModifiers != 0) {
    if (wModifiers & HOTKEYF_CONTROL) {
      str += GetKeyName(VK_CONTROL, FALSE);
      str += _T('+');
    }
    if (wModifiers & HOTKEYF_SHIFT) {
      str += GetKeyName(VK_SHIFT, FALSE);
      str += _T('+');
    }
    if (wModifiers & HOTKEYF_ALT) {
      str += GetKeyName(VK_MENU, FALSE);
      str += _T('+');
    }
    str += GetKeyName(wCode, wModifiers & HOTKEYF_EXT);
  }
}

BOOL CResidentPageDlg::OnInitDialog(LPARAM lParam)
{
  // 常駐
  m_active = GetDlgItem(IDC_ACTIVEKEY);
  m_popup = GetDlgItem(IDC_POPUPKEY);
  // shift+alt,  shift+ctrl+alt のみ受け付け
  m_active.SetRules(HKCOMB_NONE | HKCOMB_A | HKCOMB_C | HKCOMB_S | HKCOMB_SC | HKCOMB_SA, HOTKEYF_CONTROL | HOTKEYF_ALT);
  m_popup.SetRules(HKCOMB_NONE | HKCOMB_A | HKCOMB_C | HKCOMB_S | HKCOMB_SC | HKCOMB_SA, HOTKEYF_CONTROL | HOTKEYF_ALT);

  if (gnTsrStat & TSS_TSR) {
    CheckDlgButton(IDC_OPTSR, TRUE);
    if (gnTsrStat & TSS_MINHIDE)
      CheckDlgButton(IDC_OPTSRMINHIDE, TRUE);

    WORD wVirtualKeyCode, wModifiers;

    // 常駐ホットキー
    m_tsractivekey = GetRegValue(IDS_SEC_SETTINGS, IDS_KEY_TSRACTIVEKEY, 0);
    if (m_tsractivekey) {
      wVirtualKeyCode = LOBYTE(LOWORD(m_tsractivekey));
      wModifiers = HIBYTE(LOWORD(m_tsractivekey));
      m_active.SetHotKey(wVirtualKeyCode, wModifiers);
    }

    // ポップアップホットキー
    m_tsrpopupkey = GetRegValue(IDS_SEC_SETTINGS, IDS_KEY_TSRPOPUPKEY, 0);
    if (m_tsrpopupkey) {
      wVirtualKeyCode = LOBYTE(LOWORD(m_tsrpopupkey));
      wModifiers = HIBYTE(LOWORD(m_tsrpopupkey));
      m_popup.SetHotKey(wVirtualKeyCode, wModifiers);
    }
  }

  // メニューで選択したフォルダの取り扱い
  CComboBox combo = GetDlgItem(IDC_COMBO);
  InitDlgCtrl(combo, IDS_TRAY_MENUSELECT);
  int sel = (gnTsrStat & TSS_NEWWINDOW) ? 1 : 0;
  combo.SetCurSel(sel);

  OnTsr();
  return TRUE;
}

BOOL CResidentPageDlg::OnApply()
{
  if (!::IsWindow(m_hWnd)) return TRUE;

  // 常駐
  DWORD nStat = 0;
  if (IsDlgButtonChecked(IDC_OPTSR)) {
    nStat |= TSS_TSR;
    if (IsDlgButtonChecked(IDC_OPTSRMINHIDE)) nStat |= TSS_MINHIDE;

    // 常駐ホットキー
    DWORD hotkey = m_active.GetHotKey();
    if (m_tsractivekey != hotkey) {
      if (hotkey)
        SetRegValue(IDS_SEC_SETTINGS, IDS_KEY_TSRACTIVEKEY, hotkey);
      else
        DelRegValue(IDS_SEC_SETTINGS, IDS_KEY_TSRACTIVEKEY);
    }

    // ポップアップホットキー
    hotkey = m_popup.GetHotKey();
    if (m_tsrpopupkey != hotkey) {
      if (hotkey)
        SetRegValue(IDS_SEC_SETTINGS, IDS_KEY_TSRPOPUPKEY, hotkey);
      else
        DelRegValue(IDS_SEC_SETTINGS, IDS_KEY_TSRPOPUPKEY);
    }
  }
  else {
    DelRegValue(IDS_SEC_SETTINGS, IDS_KEY_TSRACTIVEKEY);
    DelRegValue(IDS_SEC_SETTINGS, IDS_KEY_TSRPOPUPKEY);
  }

  // メニューで選択したフォルダの取り扱い
  CComboBox combo = GetDlgItem(IDC_COMBO);
  if (combo.GetCurSel() > 0)
    nStat |= TSS_NEWWINDOW;

  fvApp.SetTsrStat(nStat);

  return TRUE;
}

void CResidentPageDlg::OnTsr()
{
  BOOL bCheck = (IsDlgButtonChecked(IDC_OPTSR) != FALSE);
  GetDlgItem(IDC_OPTSRMINHIDE).EnableWindow(bCheck);
  m_active.EnableWindow(bCheck);
  m_popup.EnableWindow(bCheck);
  GetDlgItem(IDC_COMBO).EnableWindow(bCheck);
}

BOOL CResidentPageDlg::CheckRegisterKey(DWORD key)
{
  if (!key) return FALSE;

  WORD wModifiers = 0;
  if (HIBYTE(LOWORD(key)) & HOTKEYF_ALT)		wModifiers |= MOD_ALT;
  if (HIBYTE(LOWORD(key)) & HOTKEYF_SHIFT)	wModifiers |= MOD_SHIFT;
  if (HIBYTE(LOWORD(key)) & HOTKEYF_CONTROL)	wModifiers |= MOD_CONTROL;
  WORD wVirtualKeyCode = LOBYTE(LOWORD(key));
  BOOL bOK = ::RegisterHotKey(m_hWnd, (int)key, wModifiers, wVirtualKeyCode);
  if (bOK)
    ::UnregisterHotKey(m_hWnd, (int)key);
  else {
    CString str;
    GetHotKeyName(key, str);
    AppMessage(IDS_HOTKEY_UNAVAILABLE, str, MB_STOP);
  }
  return bOK;
}

void CResidentPageDlg::OnChangeActiveHotKey()
{
  if (!CheckRegisterKey(m_active.GetHotKey()))
    m_active.SetHotKey(0, 0);
}

void CResidentPageDlg::OnChangePopupHotKey()
{
  if (!CheckRegisterKey(m_popup.GetHotKey()))
    m_popup.SetHotKey(0, 0);
}


/////////////////////////////////////////////////////////////////////////////
// CDispPageDlg プロパティ ページ

static json treeData = {
  {{"type", "group"}, {"id", IDC_OPDISP_VIEW}, {"label", u8"ファイル ビューに関する設定"},
    {"node", {
      {{"type", "check"}, {"id", IDC_OPWRAPCURSOR},	{"label", u8"リストの先頭と末尾でカーソルをラップ アラウンドする"}},
      {{"type", "check"}, {"id", IDC_OPLOWERCASE},	{"label", u8"ファイル名をすべて小文字で表示する"}},
      {{"type", "check"}, {"id", IDC_OPALIGNEXT},		{"label", u8"一覧・詳細表示では、拡張子を揃える"}},
      {{"type", "check"}, {"id", IDC_OPAUTOADJUST},	{"label", u8"詳細表示では各カラム幅を自動で調整する"}},
      {{"type", "check"}, {"id", IDC_OPREPCHECKER},	{"label", u8"詳細表示では1行おきに背景色を変える"}},
      {{"type", "check"}, {"id", IDC_OPNODOSNAME},	{"label", u8"詳細表示ではDOS名を省略する"}},
      {{"type", "check"}, {"id", IDC_OPBOLDNAME},		{"label", u8"縮小・概要表示ではファイル名を強調表示する"}},
      {{"type", "check"}, {"id", IDC_OPFMTSIZE},		{"label", u8"ファイル サイズをKBなどの単位表示する"}},
      {{"type", "check"}, {"id", IDC_OPFMTDATE},		{"label", u8"ファイルの日時は西暦を下2桁で表示する"}},
      {{"type", "check"}, {"id", IDC_OPRELATIVEDATE},	{"label", u8"ファイルの日時を相対表示する（昨日・今日）"}},
      {{"type", "check"}, {"id", IDC_OPINFOTIP},		{"label", u8"マウス ポイントでファイル情報をチップ表示する"}},
      {{"type", "check"}, {"id", IDC_OPSYSDB},		{"label", u8"「Desktop.ini」や「Thumbs.db」は表示しない"}},
      {{"type", "check"}, {"id", IDC_OPEACHICONSTYLE},{"label", u8"フォルダごと個別に表示スタイルを記憶する"}},
      {{"type", "group"},	{"label", u8"ファイルのソート"},
        {"node", {
          {{"type", "check"}, {"id", IDC_OPCMPLOGICAL},	{"label", u8"ファイル名中の数字は数値の大小で比較する"}},
          {{"type", "check"}, {"id", IDC_OPCMPUNDERSCORE},{"label", u8"ハイフンやアンダースコアを同一視する"}}
        }}
      },
      {{"type", "group"}, {"label", u8"ひとつ上へ"},
        {"node", {
          {{"type", "check"}, {"id", IDC_OPUPDIR},	{"label", u8"「ひとつ上へ」をエミュレーションする"}},
          {{"type", "check"}, {"id", IDC_OPUPDNOROOT},{"label", u8"ドライブのルートフォルダでは表示しない"}}
        }}
      }
    }}
  },
  {{"type", "group"}, {"label", u8"フォルダ サイズの自動計算"},
    {"node", {
      {{"type", "check"}, {"id", IDC_OPDSZ_FIXED},	{"label", u8"ハード ディスク上のフォルダ"}},
      {{"type", "check"}, {"id", IDC_OPDSZ_CDROM},	{"label", u8"CD-ROM上のフォルダ"}},
      {{"type", "check"}, {"id", IDC_OPDSZ_REMOTE},	{"label", u8"ネットワーク上のフォルダ"}},
      {{"type", "check"}, {"id", IDC_OPDSZ_REMOVABLE},{"label", u8"取り外し可能メディア上のフォルダ"}}
    }}
  },
  {{"type", "group"}, {"label", u8"サムネイル表示"},
    {"node", {
      {{"type", "check"}, {"id", IDC_OPEXIF},		{"label", u8"サムネイル表示モード時にExif情報を表示する"}},
      {{"type", "check"}, {"id", IDC_OPNOTHUMBFLDR},	{"label", u8"フォルダ アイコンはサムネイル表示しない"}},
      {{"type", "group"}, {"label", u8"サムネイル キャッシュ"},
        {"node", {
          {{"type", "radio"}, {"id", IDC_OPCACHE_NOUSE},	{"label", u8"システム キャッシュを参照しない"}},
          {{"type", "radio"}, {"id", IDC_OPCACHE_READONLY}, {"label", u8"システム キャッシュを参照するが更新はしない"}},
          {{"type", "radio"}, {"id", IDC_OPCACHE_UPDATE},	{"label", u8"システム キャッシュを参照し更新する"}}
        }}
      }
    }}
  },
  {{"type", "group"}, {"label", u8"フォルダ ツリーに関する設定"},
    {"node", {
      {{"type", "check"}, {"id", IDC_OPNOARCTREE}, {"label", u8"書庫ファイルをフォルダ ツリーに表示しない"}},
      {{"type", "check"}, {"id", IDC_OPSELTREELINE},      {"label", u8"現在のフォルダを1行全選択する"}},
      {{"type", "check"}, {"id", IDC_OPTREENONELINE},     {"label", u8"ツリー項目の階層を示す補助線を表示しない"}},
      {{"type", "check"}, {"id", IDC_OPBOLDSELTREE},      {"label", u8"選択中の項目を強調して表示する"}},
      {{"type", "group"}, {"label", u8"ドライブ ルートごとツリー表示のときの「ひとつ上へ」の動作"},
        {"node", {
          {{"type","radio"}, {"id", IDC_OPUPDIR_DISABLE}, {"label", u8"ルートより上には上がらない"}},
          {{"type","radio"}, {"id", IDC_OPUPDIR_ENABLE},	{"label", u8"ルートより上に上がる"}},
          {{"type","radio"}, {"id", IDC_OPUPDIR_CONFIRM}, {"label", u8"ルートより上に上がるときには確認する"}}
        }}
      }
    }}
  },
  {{"type", "group"}, {"label", u8"メイン ウィンドウに関する設定"},
    {"node", {
      {{"type", "check"}, {"id", IDC_OPFULLPATH}, {"label", u8"タイトル バーに現在のフォルダをフルパスで表示する"}},
      {{"type", "check"}, {"id", IDC_OPPATH1ST},	{"label", u8"タイトル バーにフォルダ名を先に表示する"}},
      {{"type", "check"}, {"id", IDC_OPMENUACCEL},{"label", u8"メニューにショートカット キーのプレフィクスを表示する"}}
    }}
  }
};

static CTITEM ctiData[] = {
  { IDC_OPDISP_VIEW,		CTGROUP_ROOT,	CTIS_GROUP,	0,	_T("ファイル ビューに関する設定") },
  { IDC_OPWRAPCURSOR,		CTGROUP_CURRENT,0,			0,	_T("リストの先頭と末尾でカーソルをラップ アラウンドする") },
  { IDC_OPLOWERCASE,		CTGROUP_CURRENT,0,			0,	_T("ファイル名をすべて小文字で表示する") },
  { IDC_OPALIGNEXT,		CTGROUP_CURRENT,0,			0,	_T("一覧・詳細表示では、拡張子を揃える") },
  { IDC_OPAUTOADJUST,		CTGROUP_CURRENT,0,			0,	_T("詳細表示では各カラム幅を自動で調整する") },
  { IDC_OPREPCHECKER,		CTGROUP_CURRENT,0,			0,	_T("詳細表示では1行おきに背景色を変える") },
  { IDC_OPNODOSNAME,		CTGROUP_CURRENT,0,			0,	_T("詳細表示ではDOS名を省略する") },
  { IDC_OPBOLDNAME,		CTGROUP_CURRENT,0,			0,	_T("縮小・概要表示ではファイル名を強調表示する") },
  { IDC_OPFMTSIZE,		CTGROUP_CURRENT,0,			0,	_T("ファイル サイズをKBなどの単位表示する") },
  { IDC_OPFMTDATE,		CTGROUP_CURRENT,0,			0,	_T("ファイルの日時は西暦を下2桁で表示する") },
  { IDC_OPRELATIVEDATE,	CTGROUP_CURRENT,0,			0,	_T("ファイルの日時を相対表示する（昨日・今日）") },
  { IDC_OPINFOTIP,		CTGROUP_CURRENT,0,			0,	_T("マウス ポイントでファイル情報をチップ表示する") },
  { IDC_OPSYSDB,			CTGROUP_CURRENT,0,			0,	_T("「Desktop.ini」や「Thumbs.db」は表示しない") },
  { IDC_OPEACHICONSTYLE,	CTGROUP_CURRENT,0,			0,	_T("フォルダごと個別に表示スタイルを記憶する") },
  { 0,					CTGROUP_CURRENT,CTIS_GROUP,	0,	_T("ファイルのソート") },
  { IDC_OPCMPLOGICAL,		CTGROUP_CURRENT,0,			0,	_T("ファイル名中の数字は数値の大小で比較する") },
  { IDC_OPCMPUNDERSCORE,	CTGROUP_CURRENT,0,			0,	_T("ハイフンやアンダースコアを同一視する") },
  { 0,					CTGROUP_PARENT,CTIS_GROUP,	0,	_T("ひとつ上へ") },
  { IDC_OPUPDIR,			CTGROUP_CURRENT,CTIS_GROUP,	0,	_T("「ひとつ上へ」をエミュレーションする") },
  { IDC_OPUPDNOROOT,		CTGROUP_CURRENT,0,			0,	_T("ドライブのルートフォルダでは表示しない") },
  { 0,					CTGROUP_ROOT,	CTIS_GROUP,	0,	_T("フォルダ サイズの自動計算") },
  { IDC_OPDSZ_FIXED,		CTGROUP_CURRENT,0,			0,	_T("ハード ディスク上のフォルダ") },
  { IDC_OPDSZ_CDROM,		CTGROUP_CURRENT,0,			0,	_T("CD-ROM上のフォルダ") },
  { IDC_OPDSZ_REMOTE,		CTGROUP_CURRENT,0,			0,	_T("ネットワーク上のフォルダ") },
  { IDC_OPDSZ_REMOVABLE,	CTGROUP_CURRENT,0,			0,	_T("取り外し可能メディア上のフォルダ") },
  { 0,					CTGROUP_PARENT,	CTIS_GROUP,	0,	_T("サムネイル表示") },
  { IDC_OPEXIF,			CTGROUP_CURRENT,0,			0,	_T("サムネイル表示モード時にExif情報を表示する") },
  { IDC_OPNOTHUMBFLDR,	CTGROUP_CURRENT,0,			0,	_T("フォルダ アイコンはサムネイル表示しない") },
  { 0,					CTGROUP_CURRENT,CTIS_GROUP,	0,	_T("サムネイル キャッシュ") },
  { IDC_OPCACHE_NOUSE,	CTGROUP_CURRENT,CTIS_RADIO,	0,	_T("システム キャッシュを参照しない") },
  { IDC_OPCACHE_READONLY,	CTGROUP_CURRENT,CTIS_RADIO,	0,	_T("システム キャッシュを参照するが更新はしない") },
  { IDC_OPCACHE_UPDATE,	CTGROUP_CURRENT,CTIS_RADIO,	0,	_T("システム キャッシュを参照し更新する") },
  { 0,					CTGROUP_ROOT,	CTIS_GROUP,	0,	_T("フォルダ ツリーに関する設定") },
  { IDC_OPNOARCTREE,CTGROUP_CURRENT,0,			0,	_T("書庫ファイルをフォルダ ツリーに表示しない") },
  { IDC_OPSELTREELINE,  CTGROUP_CURRENT,0,			0,	_T("現在のフォルダを1行全選択する") },
  { IDC_OPTREENONELINE,	CTGROUP_CURRENT,0,			0,	_T("ツリー項目の階層を示す補助線を表示しない") },
  { IDC_OPBOLDSELTREE,	CTGROUP_CURRENT,0,			0,	_T("選択中の項目を強調して表示する") },
  { 0,					CTGROUP_CURRENT,CTIS_GROUP,	0,	_T("ドライブ ルートごとツリー表示のときの「ひとつ上へ」の動作") },
  { IDC_OPUPDIR_DISABLE,	CTGROUP_CURRENT,CTIS_RADIO,	0,	_T("ルートより上には上がらない") },
  { IDC_OPUPDIR_ENABLE,	CTGROUP_CURRENT,CTIS_RADIO,	0,	_T("ルートより上に上がる") },
  { IDC_OPUPDIR_CONFIRM,	CTGROUP_CURRENT,CTIS_RADIO,	0,	_T("ルートより上に上がるときには確認する") },
  { 0,					CTGROUP_ROOT,	CTIS_GROUP,	0,	_T("メイン ウィンドウに関する設定") },
  { IDC_OPFULLPATH,		CTGROUP_CURRENT,0,			0,	_T("タイトル バーに現在のフォルダをフルパスで表示する") },
  { IDC_OPMENUACCEL,		CTGROUP_CURRENT,0,			0,	_T("メニューにショートカット キーのプレフィクスを表示する") },
/*
  { 0,					CTGROUP_ROOT,	CTIS_GROUP,	0,	_T("Windowsシェルに関する設定(※システム設定が変更されます)") },
  { OSF_SHOWALLOBJECTS,	CTGROUP_CURRENT,0,			0,	_T("すべてのファイルとフォルダを表示する") },
  { OSF_SHOWEXTENSIONS,	CTGROUP_CURRENT,0,			0,	_T("ファイルの拡張子を表示する") },
  { OSF_SHOWSUPERHIDDEN,	CTGROUP_CURRENT,0,			0,	_T("保護されたオペレーティング システム ファイルを表示する") },
*/
};

/////////////////////////////////////////////////////////////////////////////
// CDispPageDlg メッセージ ハンドラ


BOOL CDispPageDlg::OnInitDialog(LPARAM lParam) 
{

  m_tree = GetDlgItem(IDC_OPCHECKTREE);
  m_tree.SetSpacing(4);
  for (int i = 0; i < ARRAYSIZE(ctiData); i++) {
    m_tree.AddItem(&ctiData[i]);
  }
  m_tree.SetItemState(IDC_OPUPDIR, 0, CTIS_GROUP);

  if (gnListStat & LTS_WRAPCURSOR)		m_tree.CheckItem(IDC_OPWRAPCURSOR, TRUE);
  if (gnListStat & LTS_LOWERCASE)			m_tree.CheckItem(IDC_OPLOWERCASE, TRUE);
  if (gnListStat & LTS_ALIGNEXT)			m_tree.CheckItem(IDC_OPALIGNEXT, TRUE);
  if (gnListStat & LTS_AUTOADJUST)		m_tree.CheckItem(IDC_OPAUTOADJUST, TRUE);
  if (gnListStat & LTS_REPCHECKER)		m_tree.CheckItem(IDC_OPREPCHECKER, TRUE);
  if (gnListStat & LTS_NODOSNAME)			m_tree.CheckItem(IDC_OPNODOSNAME, TRUE);
  if (gnListStat & LTS_BOLDNAME)			m_tree.CheckItem(IDC_OPBOLDNAME, TRUE);
  if (gnListStat & LTS_NOARCTREE)	m_tree.CheckItem(IDC_OPNOARCTREE, TRUE);
  if (gnListStat & LTS_SELTREELINE)		m_tree.CheckItem(IDC_OPSELTREELINE, TRUE);
  if (gnListStat & LTS_TREENONELINE)		m_tree.CheckItem(IDC_OPTREENONELINE, TRUE);
  if (gnListStat & LTS_BOLDSELTREE)		m_tree.CheckItem(IDC_OPBOLDSELTREE, TRUE);
  if (gnListStat & LTS_FMTSIZE)			m_tree.CheckItem(IDC_OPFMTSIZE, TRUE);
  if (gnListStat & LTS_FMTDATE)			m_tree.CheckItem(IDC_OPFMTDATE, TRUE);
  if (gnListStat & LTS_RELATIVEDATE)		m_tree.CheckItem(IDC_OPRELATIVEDATE, TRUE);
  if (gnListStat & LTS_INFOTIP)			m_tree.CheckItem(IDC_OPINFOTIP, TRUE);
  if (gnListStat & LTS_FULLPATH)			m_tree.CheckItem(IDC_OPFULLPATH, TRUE);
  if (gnListStat & LTS_MENUACCEL)			m_tree.CheckItem(IDC_OPMENUACCEL, TRUE);
  if (gnListStat & LTS_UPDIR)				m_tree.CheckItem(IDC_OPUPDIR, TRUE);
  if (gnListStat & LTS_UPDIR_NOROOT)		m_tree.CheckItem(IDC_OPUPDNOROOT, TRUE);
  if (gnListStat & LTS_EACH_ICONSTYLE)	m_tree.CheckItem(IDC_OPEACHICONSTYLE, TRUE);
  if (gnListStat & LTS_DSZ_FIXED)			m_tree.CheckItem(IDC_OPDSZ_FIXED, TRUE);
  if (gnListStat & LTS_DSZ_CDROM)			m_tree.CheckItem(IDC_OPDSZ_CDROM, TRUE);
  if (gnListStat & LTS_DSZ_REMOTE)		m_tree.CheckItem(IDC_OPDSZ_REMOTE, TRUE);
  if (gnListStat & LTS_DSZ_REMOVABLE)		m_tree.CheckItem(IDC_OPDSZ_REMOVABLE, TRUE);
  if (gnFileStat & FTS_SYSDB)				m_tree.CheckItem(IDC_OPSYSDB, TRUE);
  if (gnFileStat & FTS_EXIF)				m_tree.CheckItem(IDC_OPEXIF, TRUE);
  if (gnFileStat & FTS_NOTHUMBFOLDER)		m_tree.CheckItem(IDC_OPNOTHUMBFLDR, TRUE);

  switch (gnFileStat & FTS_THUMBCACHE_MASK) {
  case FTS_THUMBCACHE_READONLY:			m_tree.CheckItem(IDC_OPCACHE_READONLY, TRUE); break;
  case FTS_THUMBCACHE_UPDATE:				m_tree.CheckItem(IDC_OPCACHE_UPDATE, TRUE); break;
  case FTS_THUMBCACHE_NOUSE:
  default:								m_tree.CheckItem(IDC_OPCACHE_NOUSE, TRUE); break;
  }
  switch (gnConfStat & COS_UPDIR_MASK) {
  case COS_UPDIR_ENABLE:					m_tree.CheckItem(IDC_OPUPDIR_ENABLE, TRUE); break;
  case COS_UPDIR_CONFIRM:					m_tree.CheckItem(IDC_OPUPDIR_CONFIRM, TRUE); break;
  case COS_UPDIR_DISABLE:
  default:								m_tree.CheckItem(IDC_OPUPDIR_DISABLE, TRUE); break;
  }
/*
  m_dwShellFlags = 0;
  SHELLSTATE ss;
  memset(&ss, 0, sizeof(ss));
  SHGetSetSettings(&ss, SSF_SHOWALLOBJECTS | SSF_SHOWEXTENSIONS | SSF_SHOWSUPERHIDDEN, FALSE);
  if (ss.fShowAllObjects) {
                    m_tree.CheckItem(OSF_SHOWALLOBJECTS, TRUE);
                    m_dwShellFlags |= SSF_SHOWALLOBJECTS;
  }
  if (ss.fShowExtensions) {
                    m_tree.CheckItem(OSF_SHOWEXTENSIONS, TRUE);
                    m_dwShellFlags |= SSF_SHOWEXTENSIONS;
  }
  if (ss.fShowSuperHidden) {
                    m_tree.CheckItem(OSF_SHOWSUPERHIDDEN, TRUE);
                    m_dwShellFlags |= SSF_SHOWSUPERHIDDEN;
  }
*/
  if (gnListStat & LTS_LOGICALCMP)
    m_tree.CheckItem(IDC_OPCMPLOGICAL, TRUE);
  if (gnListStat & LTS_CMPUNDERSCORE)
    m_tree.CheckItem(IDC_OPCMPUNDERSCORE, TRUE);

  m_tree.EnableItem(IDC_OPUPDNOROOT, (gnListStat & LTS_UPDIR) != 0);
  m_tree.SelectItem(IDC_OPDISP_VIEW);

  SetDlgItemInt(IDC_OPELLP_NAME, gnElmaxNameLength);
  SetDlgItemInt(IDC_OPELLP_EXT, gnElmaxExtLength);
  return TRUE;
}

void CDispPageDlg::OnNotifyCheckChanged(NMHDR* phdr, LRESULT* pResult)
{
  NMCTITEM* pc = (NMCTITEM*)phdr;
  if (pc->item.id == IDC_OPUPDIR)
    m_tree.EnableItem(IDC_OPUPDNOROOT, (pc->item.state & CTIS_CHECKED) != 0);
}

void CDispPageDlg::OnResetFolder()
{
  gpMainFrame->ResetFolderSettings();
}

BOOL CDispPageDlg::OnApply()
{
  if (!::IsWindow(m_hWnd)) return TRUE;

  int lenName = (int)GetDlgItemInt(IDC_OPELLP_NAME);
  if (lenName < 16 || lenName > 80) {
    MessageBeep(0); GetDlgItem(IDC_OPELLP_NAME).SetFocus(); return FALSE;
  }
  int lenExt  = (int)GetDlgItemInt(IDC_OPELLP_EXT);
  if (lenExt < 6 || lenExt > 18) {
    MessageBeep(0); GetDlgItem(IDC_OPELLP_EXT).SetFocus(); return FALSE;
  }
  if (lenName != gnElmaxNameLength) { gnElmaxNameLength = lenName; m_bUpdate = TRUE; }
  if (lenExt  != gnElmaxExtLength)  { gnElmaxExtLength  = lenExt;  m_bUpdate = TRUE; }

  DWORD nStat = 0;
  if (m_tree.IsItemChecked(IDC_OPWRAPCURSOR))   nStat |= LTS_WRAPCURSOR;
  if (m_tree.IsItemChecked(IDC_OPLOWERCASE))    nStat |= LTS_LOWERCASE;
  if (m_tree.IsItemChecked(IDC_OPALIGNEXT))     nStat |= LTS_ALIGNEXT;
  if (m_tree.IsItemChecked(IDC_OPAUTOADJUST))   nStat |= LTS_AUTOADJUST;
  if (m_tree.IsItemChecked(IDC_OPREPCHECKER))   nStat |= LTS_REPCHECKER;
  if (m_tree.IsItemChecked(IDC_OPNODOSNAME))    nStat |= LTS_NODOSNAME;
  if (m_tree.IsItemChecked(IDC_OPBOLDNAME))     nStat |= LTS_BOLDNAME;
  if (m_tree.IsItemChecked(IDC_OPNOARCTREE))    nStat |= LTS_NOARCTREE;
  if (m_tree.IsItemChecked(IDC_OPSELTREELINE))  nStat |= LTS_SELTREELINE;
  if (m_tree.IsItemChecked(IDC_OPTREENONELINE)) nStat |= LTS_TREENONELINE;
  if (m_tree.IsItemChecked(IDC_OPBOLDSELTREE))  nStat |= LTS_BOLDSELTREE;
  if (m_tree.IsItemChecked(IDC_OPFMTSIZE))      nStat |= LTS_FMTSIZE;
  if (m_tree.IsItemChecked(IDC_OPFMTDATE))      nStat |= LTS_FMTDATE;
  if (m_tree.IsItemChecked(IDC_OPRELATIVEDATE)) nStat |= LTS_RELATIVEDATE;
  if (m_tree.IsItemChecked(IDC_OPINFOTIP))      nStat |= LTS_INFOTIP;
  if (m_tree.IsItemChecked(IDC_OPFULLPATH))     nStat |= LTS_FULLPATH;
  if (m_tree.IsItemChecked(IDC_OPMENUACCEL))    nStat |= LTS_MENUACCEL;
  if (m_tree.IsItemChecked(IDC_OPUPDIR))        nStat |= LTS_UPDIR;
  if (m_tree.IsItemChecked(IDC_OPUPDNOROOT))    nStat |= LTS_UPDIR_NOROOT;
  if (m_tree.IsItemChecked(IDC_OPEACHICONSTYLE)) nStat |= LTS_EACH_ICONSTYLE;
  if (m_tree.IsItemChecked(IDC_OPCMPLOGICAL))   nStat |= LTS_LOGICALCMP;
  if (m_tree.IsItemChecked(IDC_OPCMPUNDERSCORE)) nStat |= LTS_CMPUNDERSCORE;
  if (m_tree.IsItemChecked(IDC_OPDSZ_FIXED))    nStat |= LTS_DSZ_FIXED;
  if (m_tree.IsItemChecked(IDC_OPDSZ_CDROM))    nStat |= LTS_DSZ_CDROM;
  if (m_tree.IsItemChecked(IDC_OPDSZ_REMOTE))   nStat |= LTS_DSZ_REMOTE;
  if (m_tree.IsItemChecked(IDC_OPDSZ_REMOVABLE)) nStat |= LTS_DSZ_REMOVABLE;
  if (gnListStat != nStat) {
    gnListStat = nStat;

    // ファイル名比較ルーチン
    _fCompareFileName = (gnListStat & LTS_LOGICALCMP) ? StrCmpLogicalW : StrCmpI;
    m_bUpdate = TRUE;
  }

  nStat = gnFileStat & ~(FTS_THUMBNAILMASK | FTS_SYSDB | FTS_EXIF | FTS_NOTHUMBFOLDER);
  if      (m_tree.IsItemChecked(IDC_OPSYSDB))           nStat |= FTS_SYSDB;
  if      (m_tree.IsItemChecked(IDC_OPEXIF))            nStat |= FTS_EXIF;
  if      (m_tree.IsItemChecked(IDC_OPNOTHUMBFLDR))     nStat |= FTS_NOTHUMBFOLDER;
  if      (m_tree.IsItemChecked(IDC_OPCACHE_READONLY))  nStat |= FTS_THUMBCACHE_READONLY;
  else if (m_tree.IsItemChecked(IDC_OPCACHE_UPDATE))    nStat |= FTS_THUMBCACHE_UPDATE;
  else                                                  nStat |= FTS_THUMBCACHE_NOUSE;
  if (gnFileStat != nStat) {
    gnFileStat = nStat;
    m_bUpdate = TRUE;
  }

  nStat = gnConfStat & ~COS_UPDIR_MASK;
  if      (m_tree.IsItemChecked(IDC_OPUPDIR_ENABLE))		nStat |= COS_UPDIR_ENABLE;
  else if (m_tree.IsItemChecked(IDC_OPUPDIR_CONFIRM))		nStat |= COS_UPDIR_CONFIRM;
  else													nStat |= COS_UPDIR_DISABLE;
  fvApp.SetConfStat(nStat);

  return TRUE;
}

/////////////////////////////////////////////////////////////////////////////
// CFopPageDlg メッセージ ハンドラ

BOOL CFopPageDlg::OnInitDialog(LPARAM lParam) 
{
  m_senter = GetDlgItem(IDC_OPSENTER);
  m_matchtype = GetDlgItem(IDC_OPMATCHTYPE);

  InitDlgCtrl(m_senter, IDS_OPSENTER);
  InitDlgCtrl(m_matchtype, IDS_OPMATCHTYPES);

  // フォルダ４初期設定
  CheckDlgButton(IDC_OPRECYCLE, gnFopStat & FOS_RECYCLE);
  CheckDlgButton(IDC_OPNOWATCHRECYCLEFOLDER, gnFopStat & FOS_NOWATCHRECYCLEFOLDER);
  CheckDlgButton(IDC_OPDDNOCONFIRM, gnFopStat & FOS_DDNOCONFIRM);
  CheckDlgButton(IDC_OPDDALTOVERWRITE, gnFopStat & FOS_DDALTOVERWRITE);
  CheckDlgButton(IDC_OPRUNCOPY, gnFopStat & FOS_RUNCOPY);
  CheckDlgButton(IDC_OPREXFINDER, gnFopStat & FOS_REXFINDER);
  CheckDlgButton(IDC_OPRESETFILTER, gnFopStat & FOS_RESETFILTER);
  CheckDlgButton(IDC_OPSHIFTSEARCH, gnFopStat & FOS_SHIFTSEARCH);
  // shift+enterキー
  int sel;
  switch (gnFopStat & FOS_EXEC_MASK) {
    case FOS_EXEC_EDITOR:	sel = 1;	break;
    case FOS_EXEC_OPEN:		sel = 2;	break;
    case FOS_EXEC_VIEWER:
    default:				sel = 0;	break;
  }
  m_senter.SetCurSel(sel);
  // 一致手法
  m_matchtype.SetCurSel((gnFopStat & FOS_PREFIX_MATCH) ? 1 : 0);

  return TRUE;
}

BOOL CFopPageDlg::OnApply()
{
  if (!::IsWindow(m_hWnd)) return TRUE;

  DWORD nStat = 0;
  if (IsDlgButtonChecked(IDC_OPRECYCLE))				nStat |= FOS_RECYCLE;
  if (IsDlgButtonChecked(IDC_OPNOWATCHRECYCLEFOLDER))	nStat |= FOS_NOWATCHRECYCLEFOLDER;
  if (IsDlgButtonChecked(IDC_OPDDNOCONFIRM))			nStat |= FOS_DDNOCONFIRM;
  if (IsDlgButtonChecked(IDC_OPDDALTOVERWRITE))		nStat |= FOS_DDALTOVERWRITE;
  if (IsDlgButtonChecked(IDC_OPRUNCOPY))				nStat |= FOS_RUNCOPY;
  if (IsDlgButtonChecked(IDC_OPREXFINDER))			nStat |= FOS_REXFINDER;
  if (IsDlgButtonChecked(IDC_OPRESETFILTER))			nStat |= FOS_RESETFILTER;
  if (IsDlgButtonChecked(IDC_OPSHIFTSEARCH))			nStat |= FOS_SHIFTSEARCH;
  // shift+enter
  switch (m_senter.GetCurSel()) {
  case 1:		nStat |= FOS_EXEC_EDITOR;	break;
  case 2:		nStat |= FOS_EXEC_OPEN;	break;
  default:	nStat |= FOS_EXEC_VIEWER; break;
  }
  // 一致手法
  if (m_matchtype.GetCurSel() != 0)
    nStat |= FOS_PREFIX_MATCH;
  if (gnFopStat != nStat) {
    gnFopStat = nStat;
    m_bUpdate = TRUE;
  }

  return TRUE;
}

/////////////////////////////////////////////////////////////////////////////
// CMousePageDlg メッセージ ハンドラ

BOOL CMousePageDlg::OnInitDialog(LPARAM lParam) 
{
  m_lbtn = GetDlgItem(IDC_OPCLICK);
  m_rbtn = GetDlgItem(IDC_OPRCLICK);
  m_cbtn = GetDlgItem(IDC_OPCCLICK);
  m_fbtn = GetDlgItem(IDC_OPFAVOCLICK);

  InitDlgCtrl(m_lbtn, IDS_OPCLICK);
  InitDlgCtrl(m_rbtn, IDS_OPRCLICK);
  InitDlgCtrl(m_cbtn, IDS_OPCCLICK);
  InitDlgCtrl(m_fbtn, IDS_OPFAVOCLICK);

  // マウスボタン操作
  CheckDlgButton(IDC_OPHOTTRACK, gnMosStat & MOS_HOTTRACK);

  // 左ボタンの取り扱い
  int sel = 0;
  switch ((gnMosStat & MOS_LBTN_MASK)) {
  case MOS_LBTN_ONECLICK: sel = 1; break;
  }
  m_lbtn.SetCurSel(sel);

  CheckDlgButton(IDC_OPDBLCLKUPDIR, gnMosStat & MOS_SPC_DBLCLK_UPDIR);

  // 右ボタンの取り扱い
  sel = 0;
  switch (gnMosStat & MOS_RBTN_MASK) {
  case MOS_RBTN_SHORTCUTMENU: sel = 1; break;
  case MOS_RBTN_SINGLE_SEL: sel = 2; break;
  }
  m_rbtn.SetCurSel(sel);

  CheckDlgButton(IDC_OPSPACECTX, gnMosStat & MOS_SPC_FOLDER_CTXMENU);

  // 中央ボタン
  sel = 0;
  switch (gnMosStat & MOS_CBTN_MASK) {
  case MOS_CBTN_SINGLE_SEL: sel = 1; break;
  case MOS_CBTN_SHORTCUTMENU: sel = 2; break;
  }
  m_cbtn.SetCurSel(sel);

  // しおりでの取り扱い
  sel = (gnMosStat & MOS_FAVO_ONECLICK) ? 1 : 0;
  m_fbtn.SetCurSel(sel);

  return TRUE;
}

BOOL CMousePageDlg::OnApply()
{
  if (!::IsWindow(m_hWnd)) return TRUE;

  DWORD nStat = 0;
  if (IsDlgButtonChecked(IDC_OPHOTTRACK))		nStat |= MOS_HOTTRACK;

  if (m_lbtn.GetCurSel() == 1)
    nStat |= MOS_LBTN_ONECLICK;

  if (IsDlgButtonChecked(IDC_OPDBLCLKUPDIR))	nStat |= MOS_SPC_DBLCLK_UPDIR;

  switch (m_rbtn.GetCurSel()) {
  case 1:  nStat |= MOS_RBTN_SHORTCUTMENU; break;
  case 2:  nStat |= MOS_RBTN_SINGLE_SEL; break;
  case 0:
  default: nStat |= MOS_RBTN_CONTEXTMENU; break;
  }

  if (IsDlgButtonChecked(IDC_OPSPACECTX))	nStat |= MOS_SPC_FOLDER_CTXMENU;

  switch (m_cbtn.GetCurSel()) {
  case 1:  nStat |= MOS_CBTN_SINGLE_SEL; break;
  case 2:  nStat |= MOS_CBTN_SHORTCUTMENU; break;
  case 0:
  default: nStat |= MOS_CBTN_DEFAULT; break;
  }

  // しおりでの取り扱い
  if (m_fbtn.GetCurSel() == 1)
    nStat |= MOS_FAVO_ONECLICK;

  if (gnMosStat != nStat) {
    gnMosStat = nStat;
    m_bUpdate = TRUE;
  }
  return TRUE;
}

/////////////////////////////////////////////////////////////////////////////
// CColorPageDlg プロパティ ページ

class CUSTOMCOLORPROPERTY {
public:
  int id;
  COLORREF text_clr;
  COLORREF back_clr;
  bool changed:1;

  CUSTOMCOLORPROPERTY() : id(0), text_clr(CLR_NONE), back_clr(CLR_NONE), changed(false){}
  CUSTOMCOLORPROPERTY(int sid, COLORREF sclrt, COLORREF sclrb) : id(sid), text_clr(sclrt), back_clr(sclrb), changed(false){}
  ~CUSTOMCOLORPROPERTY(){}
};

class CUSTOMFONTPROPERTY {
public:
  int id;
  LOGFONT lf;
  bool changed:1;

  CUSTOMFONTPROPERTY() : id(0), changed(false){ memset(&lf, 0, sizeof(lf)); }
  CUSTOMFONTPROPERTY(int sid, LOGFONT* plf) : id(sid), changed(false){ lf = *plf; }
  int CompareLogFont(LOGFONT& rlf) { return memcmp(&lf, &rlf, sizeof(lf)); }
};

/////////////////////////////////////////////////////////////////////////////
// CColorPageDlg メッセージ ハンドラ

BOOL CColorPageDlg::OnInitDialog(LPARAM lParam)
{
  m_colorlist = GetDlgItem(IDC_COLOR_LIST);
  m_fontlist = GetDlgItem(IDC_FONT_LIST);
  m_fontcombo = GetDlgItem(IDC_FONT_NAME);
  m_sizecombo = GetDlgItem(IDC_FONT_SIZE);

  LOGFONT lfDefault;
  _GCDATA.GetIconTitleLogFont(&lfDefault);

  // ------------------------
  // 各フォント
  // ------------------------
  m_fontlist.SetExtendedStyle(TVS_EX_DOUBLEBUFFER, TVS_EX_DOUBLEBUFFER);
  m_fontlist.SetItemHeight(GetSystemMetrics(SM_CYSMICON) + 3);

  CUSTOMFONTPROPERTY *pcfp;
  LOGFONT lf;

  CString str;
  str.LoadString(IDS_OPFONT_FOLDERTREE);
  if (!LoadFontInfo(&lf, IDS_SEC_SETTINGS, IDS_KEY_TREEFONT))
    lf = lfDefault;
  pcfp = new CUSTOMFONTPROPERTY(0, &lf);
  m_fontlist.InsertItem(TVIF_TEXT|TVIF_PARAM, str.GetString(), 0, 0, 0, 0, (LPARAM)pcfp, TVI_ROOT, TVI_LAST);

  str.LoadString(IDS_OPFONT_FILEPANE);
  if (!LoadFontInfo(&lf, IDS_SEC_SETTINGS, IDS_KEY_FILEFONT))
    lf = lfDefault;
  pcfp = new CUSTOMFONTPROPERTY(1, &lf);
  m_fontlist.InsertItem(TVIF_TEXT|TVIF_PARAM, str.GetString(), 0, 0, 0, 0, (LPARAM)pcfp, TVI_ROOT, TVI_LAST);

  str.LoadString(IDS_OPFONT_PANEL);
  if (!LoadFontInfo(&lf, IDS_SEC_SETTINGS, IDS_KEY_PANELFONT))
    lf = lfDefault;
  pcfp = new CUSTOMFONTPROPERTY(2, &lf);
  m_fontlist.InsertItem(TVIF_TEXT|TVIF_PARAM, str.GetString(), 0, 0, 0, 0, (LPARAM)pcfp, TVI_ROOT, TVI_LAST);

  str.LoadString(IDS_OPFONT_FUNCTION);
  if (!LoadFontInfo(&lf, IDS_SEC_SETTINGS, IDS_KEY_FUNCTIONFONT))
    _GCDATA.GetRibbonLogFont(&lf);
  pcfp = new CUSTOMFONTPROPERTY(3, &lf);
  m_fontlist.InsertItem(TVIF_TEXT|TVIF_PARAM, str.GetString(), 0, 0, 0, 0, (LPARAM)pcfp, TVI_ROOT, TVI_LAST);

  // フォント名とサイズの列挙
  m_fontcombo.SetItemHeight(0, GetSystemMetrics(SM_CYSMICON) + 3);
  CClientDC dc(NULL);
  EnumFontFamilies(dc, NULL, (FONTENUMPROC)FontCallback, (LPARAM)this);
  for (int i = 6; i <= 24; i++) {
    str.Format(_T("%d"), i);
    m_sizecombo.AddString(str);
  }

  // 先頭項目を選択
  m_fontlist.SelectItem(m_fontlist.GetRootItem());

  // ------------------------
  // カラー
  // ------------------------
  m_colorlist.SetExtendedStyle(TVS_EX_DOUBLEBUFFER, TVS_EX_DOUBLEBUFFER);
  m_colorlist.SetItemHeight(GetSystemMetrics(SM_CYSMICON) + 3);

  CUSTOMCOLORPROPERTY* pccp;

  str.LoadString(IDS_OPCOLOR_NORMAL);				// 通常のファイル
  pccp = new CUSTOMCOLORPROPERTY(colNorm, gColors[colNorm], gColors[colBack]);
  m_colorlist.InsertItem(TVIF_TEXT|TVIF_PARAM, str.GetString(), 0, 0, 0, 0, (LPARAM)pccp, TVI_ROOT, TVI_LAST);

  str.LoadString(IDS_OPCOLOR_SELECTED);			// 選択中のファイル"
  pccp = new CUSTOMCOLORPROPERTY(colSelect, gColors[colSelect], gColors[colSelBack]);
  m_colorlist.InsertItem(TVIF_TEXT|TVIF_PARAM, str.GetString(), 0, 0, 0, 0, (LPARAM)pccp, TVI_ROOT, TVI_LAST);

  str.LoadString(IDS_OPCOLOR_MARKED);				// マークされたファイル"
  pccp = new CUSTOMCOLORPROPERTY(colMark, gColors[colMark], gColors[colMarkBack]);
  m_colorlist.InsertItem(TVIF_TEXT|TVIF_PARAM, str.GetString(), 0, 0, 0, 0, (LPARAM)pccp, TVI_ROOT, TVI_LAST);

  str.LoadString(IDS_OPCOLOR_SYSTEM);				// システム ファイル
  pccp = new CUSTOMCOLORPROPERTY(colSystem, gColors[colSystem], CLR_NONE);
  m_colorlist.InsertItem(TVIF_TEXT|TVIF_PARAM, str.GetString(), 0, 0, 0, 0, (LPARAM)pccp, TVI_ROOT, TVI_LAST);

  str.LoadString(IDS_OPCOLOR_HIDDEN);				// 隠しファイル
  pccp = new CUSTOMCOLORPROPERTY(colHidden, gColors[colHidden], CLR_NONE);
  m_colorlist.InsertItem(TVIF_TEXT|TVIF_PARAM, str.GetString(), 0, 0, 0, 0, (LPARAM)pccp, TVI_ROOT, TVI_LAST);

  str.LoadString(IDS_OPCOLOR_READONLY);			// 読み取り専用ファイル
  pccp = new CUSTOMCOLORPROPERTY(colRdonly, gColors[colRdonly], CLR_NONE);
  m_colorlist.InsertItem(TVIF_TEXT|TVIF_PARAM, str.GetString(), 0, 0, 0, 0, (LPARAM)pccp, TVI_ROOT, TVI_LAST);

  str.LoadString(IDS_OPCOLOR_COMPRESSED);			// NTFS圧縮ファイル
  pccp = new CUSTOMCOLORPROPERTY(colCmprs, gColors[colCmprs], CLR_NONE);
  m_colorlist.InsertItem(TVIF_TEXT|TVIF_PARAM, str.GetString(), 0, 0, 0, 0, (LPARAM)pccp, TVI_ROOT, TVI_LAST);

  str.LoadString(IDS_OPCOLOR_CRYPTED);			// NTFS暗号化ファイル
  pccp = new CUSTOMCOLORPROPERTY(colEncr, gColors[colEncr], CLR_NONE);
  m_colorlist.InsertItem(TVIF_TEXT|TVIF_PARAM, str.GetString(), 0, 0, 0, 0, (LPARAM)pccp, TVI_ROOT, TVI_LAST);

  str.LoadString(IDS_OPCOLOR_REPARSEPOINT);		// リパース ポイント フォルダ
  pccp = new CUSTOMCOLORPROPERTY(colRepp, gColors[colRepp], CLR_NONE);
  m_colorlist.InsertItem(TVIF_TEXT|TVIF_PARAM, str.GetString(), 0, 0, 0, 0, (LPARAM)pccp, TVI_ROOT, TVI_LAST);

  str.LoadString(IDS_OPCOLOR_TREELINE);			// ツリー線
  pccp = new CUSTOMCOLORPROPERTY(colLine, gColors[colLine], CLR_NONE);
  m_colorlist.InsertItem(TVIF_TEXT|TVIF_PARAM, str.GetString(), 0, 0, 0, 0, (LPARAM)pccp, TVI_ROOT, TVI_LAST);

  // 先頭項目を選択
  m_colorlist.SelectItem(m_colorlist.GetRootItem());

  // ツリー線の色はシステム標準かどうか
  CheckDlgButton(IDC_TREE_LINE, (gColors[colLine] == CLR_DEFAULT));

  RectCtrl_SetEdgeColor(GetDlgItem(IDC_SAMPLE), GetSysColor(COLOR_3DSHADOW));
  return TRUE;
}

BOOL CColorPageDlg::OnApply()
{
  if (!::IsWindow(m_hWnd)) return TRUE;

  HTREEITEM hItem;
  hItem = m_colorlist.GetRootItem();
  while (hItem) {
    CUSTOMCOLORPROPERTY* pccp = (CUSTOMCOLORPROPERTY*)m_colorlist.GetItemData(hItem);
    if (pccp && pccp->changed) {
      m_bUpdate = TRUE;
      switch (pccp->id) {
      case colNorm:		gColors[colNorm] = pccp->text_clr; gColors[colBack] = pccp->back_clr; break;
      case colSelect:		gColors[colSelect] = pccp->text_clr; gColors[colSelBack] = pccp->back_clr; break;
      case colMark:		gColors[colMark] = pccp->text_clr; gColors[colMarkBack] = pccp->back_clr; break;
      case colSystem:		gColors[colSystem] = pccp->text_clr; break;
      case colHidden:		gColors[colHidden] = pccp->text_clr; break;
      case colRdonly:		gColors[colRdonly] = pccp->text_clr; break;
      case colCmprs:		gColors[colCmprs] = pccp->text_clr; break;
      case colRepp:		gColors[colRepp] = pccp->text_clr; break;
      case colLine:		gColors[colLine] = pccp->text_clr; break;
      default: break;
      }
    }
    hItem = m_colorlist.GetNextSiblingItem(hItem);
  }
  // 例外
  if (IsDlgButtonChecked(IDC_TREE_LINE))
    gColors[colLine] = CLR_DEFAULT;
  // 保存
  fvApp.SaveColor();

  // フォント情報
  hItem = m_fontlist.GetRootItem();
  while (hItem) {
    CUSTOMFONTPROPERTY* pcfp = (CUSTOMFONTPROPERTY*)m_fontlist.GetItemData(hItem);
    if (pcfp && pcfp->changed) {
      m_bUpdate = TRUE;
      switch (pcfp->id) {
      case 0: SaveFontInfo(&pcfp->lf, IDS_SEC_SETTINGS, IDS_KEY_TREEFONT); break;
      case 1: SaveFontInfo(&pcfp->lf, IDS_SEC_SETTINGS, IDS_KEY_FILEFONT); break;
      case 2: SaveFontInfo(&pcfp->lf, IDS_SEC_SETTINGS, IDS_KEY_PANELFONT); break;
      case 3: SaveFontInfo(&pcfp->lf, IDS_SEC_SETTINGS, IDS_KEY_FUNCTIONFONT); break;
      default: break;
      }
    }
    hItem = m_fontlist.GetNextSiblingItem(hItem);
  }

  return TRUE;
}

int CColorPageDlg::EnumComboFonts(const ENUMLOGFONT* pelf, const NEWTEXTMETRIC* pntm, DWORD fontType)
{
  // 縦書きフォント、シンボルフォントは除く
  if ((pelf->elfLogFont.lfFaceName[0] != '@') &&
    (pelf->elfLogFont.lfCharSet != SYMBOL_CHARSET)) {
    LOGFONT* plf = new LOGFONT;
    *plf = pelf->elfLogFont;
    int index = m_fontcombo.AddString(pelf->elfLogFont.lfFaceName);
    m_fontcombo.SetItemData(index, (DWORD_PTR)plf);
  }
  return 1;
}

void CColorPageDlg::OnUseTreeLine() 
{
  HTREEITEM hItem = m_colorlist.GetSelectedItem();
  if (hItem) {
    CUSTOMCOLORPROPERTY* pccp = (CUSTOMCOLORPROPERTY*)m_colorlist.GetItemData(hItem);
    if (pccp && pccp->id == colLine)
      GetDlgItem(IDC_COLOR_FORE).EnableWindow(!IsDlgButtonChecked(IDC_TREE_LINE));
  }
}

void CColorPageDlg::OnNotifyFontListSelChanged(NMHDR* pNMHDR, LRESULT* pResult)
{
  *pResult = 0;
  NMTREEVIEW* pnmtv = (NMTREEVIEW*)pNMHDR;

  CUSTOMFONTPROPERTY* pcfp = (CUSTOMFONTPROPERTY*)m_fontlist.GetItemData(pnmtv->itemNew.hItem);
  if (pcfp) {
    // フォント名
    m_fontcombo.SelectString(-1, pcfp->lf.lfFaceName);
    // フォントサイズ
    CClientDC dc(NULL);
    CString str;
    str.Format(_T("%d"), ::MulDiv(abs(pcfp->lf.lfHeight), 72, dc.GetDeviceCaps(LOGPIXELSY)));
    int index = m_sizecombo.FindStringExact(-1, str);
    if (index >= 0)
      m_sizecombo.SetCurSel(index);
    else
      m_sizecombo.SetWindowText(str);
    // 太字
    CheckDlgButton(IDC_FONT_BOLD, (pcfp->lf.lfWeight > FW_NORMAL));
  }
}

void CColorPageDlg::OnNotifyFontListDeleteItem(NMHDR* pNMHDR, LRESULT* pResult)
{
  *pResult = 0;
  NMTREEVIEW* pnmtv = (NMTREEVIEW*)pNMHDR;
  CUSTOMFONTPROPERTY* pcfp = (CUSTOMFONTPROPERTY*)pnmtv->itemOld.lParam;
  if (pcfp)
    delete pcfp;
}

void CColorPageDlg::OnNotifyColorListSelChanged(NMHDR* pNMHDR, LRESULT* pResult)
{
  *pResult = 0;
  NMTREEVIEW* pnmtv = (NMTREEVIEW*)pNMHDR;

  CUSTOMCOLORPROPERTY* pccp = (CUSTOMCOLORPROPERTY*)m_colorlist.GetItemData(pnmtv->itemNew.hItem);
  if (pccp) {
    CWindow wnd = GetDlgItem(IDC_COLOR_FORE);
    if (pccp->text_clr != CLR_NONE) {
      wnd.EnableWindow(TRUE);
      ColorPicker_SetBkColor(wnd, pccp->text_clr);
    }
    else {
      wnd.EnableWindow(FALSE);
    }
    if (pccp->id == colLine)
      wnd.EnableWindow(!IsDlgButtonChecked(IDC_TREE_LINE));

    wnd = GetDlgItem(IDC_COLOR_BACK);
    if (pccp->back_clr != CLR_NONE) {
      wnd.EnableWindow(TRUE);
      ColorPicker_SetBkColor(wnd, pccp->back_clr);
    }
    else {
      wnd.EnableWindow(FALSE);
    }
  }
}

void CColorPageDlg::OnNotifyColorListDeleteItem(NMHDR* pNMHDR, LRESULT* pResult)
{
  *pResult = 0;
  NMTREEVIEW* pnmtv = (NMTREEVIEW*)pNMHDR;
  CUSTOMCOLORPROPERTY* pccp = (CUSTOMCOLORPROPERTY*)pnmtv->itemOld.lParam;
  if (pccp)
    delete pccp;
}

void CColorPageDlg::OnDeleteItem(int idCtrl, DELETEITEMSTRUCT* pds)
{
  if (idCtrl == IDC_FONT_NAME) {
    LOGFONT* plf = (LOGFONT*)pds->itemData;
    if (plf)
      delete plf;
  }
}

void CColorPageDlg::OnSelChangeFontName()
{
  HTREEITEM hItem = m_fontlist.GetSelectedItem();
  if (hItem) {
    CUSTOMFONTPROPERTY* pcfp = (CUSTOMFONTPROPERTY*)m_fontlist.GetItemData(hItem);
    LOGFONT* plf = (LOGFONT*)m_fontcombo.GetItemData(m_fontcombo.GetCurSel());
    if (pcfp && plf) {
      // 高さをkeep
      int height = pcfp->lf.lfHeight;
      pcfp->lf = *plf;
      pcfp->lf.lfHeight = height;
      pcfp->changed = true;
    }
  }
}

void CColorPageDlg::OnFontBold()
{
  HTREEITEM hItem = m_fontlist.GetSelectedItem();
  if (hItem) {
    CUSTOMFONTPROPERTY* pcfp = (CUSTOMFONTPROPERTY*)m_fontlist.GetItemData(hItem);
    LOGFONT* plf = (LOGFONT*)m_fontcombo.GetItemData(m_fontcombo.GetCurSel());
    if (pcfp && plf) {
      pcfp->lf.lfWeight = IsDlgButtonChecked(IDC_FONT_BOLD) ? FW_BOLD : FW_DONTCARE;
      pcfp->changed = true;
    }
  }
}

void CColorPageDlg::OnFontSameAll()
{
  HTREEITEM hItem = m_fontlist.GetSelectedItem();
  if (hItem) {
    // 現在選択中の項目のフォント情報
    CUSTOMFONTPROPERTY* pcfpBase = (CUSTOMFONTPROPERTY*)m_fontlist.GetItemData(hItem);
    // フォント情報を同一にする
    hItem = m_fontlist.GetRootItem();
    while (hItem) {
      CUSTOMFONTPROPERTY* pcfp = (CUSTOMFONTPROPERTY*)m_fontlist.GetItemData(hItem);
      if (pcfp && pcfp != pcfpBase && pcfpBase->CompareLogFont(pcfp->lf) != 0) {
        m_bUpdate = TRUE;
        pcfp->lf = pcfpBase->lf;
        pcfp->changed = true;
      }
      hItem = m_fontlist.GetNextSiblingItem(hItem);
    }
  }
}

void CColorPageDlg::OnSelChangeFontSize()
{
  HTREEITEM hItem = m_fontlist.GetSelectedItem();
  if (hItem) {
    CUSTOMFONTPROPERTY* pcfp = (CUSTOMFONTPROPERTY*)m_fontlist.GetItemData(hItem);
    if (pcfp) {
      CClientDC dc(NULL);
      CString str;
      m_sizecombo.GetLBText(m_sizecombo.GetCurSel(), str);
      int pointsize = StrToInt(str);
      if (4 <= pointsize && pointsize <= 32) {
        pcfp->lf.lfHeight = -::MulDiv(pointsize, dc.GetDeviceCaps(LOGPIXELSY), 72);
        pcfp->changed = true;
      }
    }
  }
}

void CColorPageDlg::OnEditChangeFontSize()
{
  HTREEITEM hItem = m_fontlist.GetSelectedItem();
  if (hItem) {
    CUSTOMFONTPROPERTY* pcfp = (CUSTOMFONTPROPERTY*)m_fontlist.GetItemData(hItem);
    if (pcfp) {
      CClientDC dc(NULL);
      CString str;
      m_sizecombo.GetWindowText(str);
      int pointsize = StrToInt(str);
      if (4 <= pointsize && pointsize <= 32) {
        pcfp->lf.lfHeight = -::MulDiv(pointsize, dc.GetDeviceCaps(LOGPIXELSY), 72);
        pcfp->changed = true;
      }
    }
  }
}

void CColorPageDlg::OnSelChangeForeColor()
{
  COLORREF clr = ColorPicker_GetBkColor(GetDlgItem(IDC_COLOR_FORE));
  HTREEITEM hItem = m_colorlist.GetSelectedItem();
  if (hItem) {
    CUSTOMCOLORPROPERTY* pccp = (CUSTOMCOLORPROPERTY*)m_colorlist.GetItemData(hItem);
    if (pccp && pccp->text_clr != clr) {
      pccp->text_clr = clr;
      pccp->changed = true;
    }
  }
}

void CColorPageDlg::OnSelChangeBackColor()
{
  COLORREF clr = ColorPicker_GetBkColor(GetDlgItem(IDC_COLOR_BACK));
  HTREEITEM hItem = m_colorlist.GetSelectedItem();
  if (hItem) {
    CUSTOMCOLORPROPERTY* pccp = (CUSTOMCOLORPROPERTY*)m_colorlist.GetItemData(hItem);
    if (pccp && pccp->back_clr != clr) {
      pccp->back_clr = clr;
      pccp->changed = true;
    }
  }
}

/////////////////////////////////////////////////////////////////////////////
// CToolbarPageDlg ダイアログ


CToolbarPageDlg::CToolbarPageDlg()
{
  m_himChk = lccLoadImageList(RESINST, RT_RCDATA, _MKIR(IDB_CHECK), 20, RGB(255, 0, 255));
  m_himBar = ImageList_Create(16, 16, ILC_COLOR32, 1, 0);
  HICON hIcon = (HICON)LoadImage(RESINST, _MKIR(IDI_CUSTOMBAR), IMAGE_ICON, 16, 16, 0);
  ImageList_AddIcon(m_himBar, hIcon);
  DestroyIcon(hIcon);
}

CToolbarPageDlg::~CToolbarPageDlg()
{
  if (m_himChk) ImageList_Destroy(m_himChk);
  if (m_himBar) ImageList_Destroy(m_himBar);
}

BOOL CToolbarPageDlg::OnInitDialog(LPARAM lParam) 
{
  CenterWindow();
  m_list = GetDlgItem(IDC_LIST);

  // リストコントロールの設定
  m_list.SetImageList(m_himBar, LVSIL_SMALL);
  m_list.SetImageList(m_himChk, LVSIL_STATE);
  LV_COLUMN lvc;
  lvc.mask = LVCF_FMT | LVCF_WIDTH | LVCF_TEXT | LVCF_SUBITEM;
  lvc.iSubItem = 0;
  lvc.cx = 200;
  lvc.fmt = LVCFMT_LEFT;
  CString str;
  str.LoadString(IDS_CUSTBAR_TITLE);
  lvc.pszText = (LPTSTR)(LPCTSTR)str;
  m_list.InsertColumn(0, &lvc);

  CSimpleArray<CToolBarCtrl*>& arrToolBar = gpMainFrame->GetToolBarArray();

  int ovl;
  CToolBarCtrl* pBar;
  for (int i = 0; i < arrToolBar.GetSize(); i++) {
    pBar = arrToolBar[i];
    pBar->GetWindowText(str);
    ovl = pBar->IsWindowVisible() ? 2 : 1;
    m_list.InsertItem(LVIF_TEXT | LVIF_IMAGE | LVIF_STATE | LVIF_PARAM,
              i, str, INDEXTOSTATEIMAGEMASK(ovl),
              LVIS_STATEIMAGEMASK, 0, (LPARAM)pBar);
  }

  if (m_list.GetItemCount() > 0)
    m_list.SelectItem(0);
  else {
    GetDlgItem(IDC_DELETE).EnableWindow(FALSE);
    GetDlgItem(IDC_CUSTOMIZE).EnableWindow(FALSE);
  }

  return TRUE;
}

void CToolbarPageDlg::OnItemChangedList(NMHDR* /*pNMHDR*/, LRESULT* pResult) 
{
  BOOL bOK = (m_list.GetNextItem(-1, LVNI_SELECTED) >= 0);
  GetDlgItem(IDC_DELETE).EnableWindow(bOK);
  GetDlgItem(IDC_CUSTOMIZE).EnableWindow(bOK);
  *pResult = 0;
}

void CToolbarPageDlg::OnNew() 
{
  CString str;
  str.LoadString(IDS_NEWTOOLBAR);
  CToolBarCtrl* pBar = gpMainFrame->CreateEmptyToolBar(str);
  if (pBar) {
    int index = m_list.InsertItem(LVIF_TEXT | LVIF_IMAGE | LVIF_STATE | LVIF_PARAM,
                m_list.GetItemCount(), str, INDEXTOSTATEIMAGEMASK(2),
                LVIS_STATEIMAGEMASK, 0, (LPARAM)pBar);
    m_list.SelectItem(index);
    m_list.SetFocus();
    m_list.EditLabel(index);
  }
}

void CToolbarPageDlg::OnCustomize() 
{
  int index = m_list.GetNextItem(-1, LVNI_SELECTED);
  if (index >= 0) {
    CToolBarCtrl* pBar = (CToolBarCtrl*)m_list.GetItemData(index);
    if (pBar) pBar->Customize();
  }
}

void CToolbarPageDlg::OnRename()
{
  int index = m_list.GetNextItem(-1, LVNI_SELECTED);
  if (index != -1) {
    m_list.SetFocus();
    m_list.EditLabel(index);
  }
}

void CToolbarPageDlg::OnEndLabelEditList(NMHDR* pNMHDR, LRESULT* pResult) 
{
  LV_DISPINFO* pdi = (LV_DISPINFO*)pNMHDR;
  CToolBarCtrl* pBar = (CToolBarCtrl*)m_list.GetItemData(pdi->item.iItem);

  CString str;
  pBar->GetWindowText(str);
  if ((pdi->item.mask & LVIF_TEXT) && pdi->item.pszText && *pdi->item.pszText) {
    pBar->SetWindowText(pdi->item.pszText);
    gpMainFrame->SaveRebarSettings();
    *pResult = TRUE;
  }
  else {
    m_list.SetItemText(pdi->item.iItem, 0, str);
    *pResult = FALSE;
  }
}

void CToolbarPageDlg::OnBeginLabelEditList(NMHDR* /*pNMHDR*/, LRESULT* pResult) 
{
  *pResult = 0;
}

void CToolbarPageDlg::OnClickList(NMHDR* pNmhdr, LRESULT* pResult) 
{
  LV_HITTESTINFO lvht;
  GetCursorPos(&lvht.pt);
  m_list.ScreenToClient(&lvht.pt);
  int nIndex = m_list.HitTest(&lvht);
  if (pNmhdr->code == NM_CLICK && lvht.flags & LVHT_ONITEMSTATEICON) {
    CToolBarCtrl* pBar = (CToolBarCtrl*)m_list.GetItemData(nIndex);
    BOOL bVisible = pBar->IsWindowVisible();
    gpMainFrame->ShowBar(*pBar, !bVisible);
    UINT ovl = bVisible ? 1 : 2;
    m_list.SetItemState(nIndex, INDEXTOSTATEIMAGEMASK(ovl), LVIS_STATEIMAGEMASK);
  }
  *pResult = 0;
}

void CToolbarPageDlg::OnKeydownList(NMHDR* pNMHDR, LRESULT* pResult) 
{
  LV_KEYDOWN* plvk = (LV_KEYDOWN*)pNMHDR;
  if (plvk->wVKey == VK_SPACE) {
    int nIndex = m_list.GetNextItem(-1, LVNI_ALL | LVNI_SELECTED);
    if (nIndex >= 0) {
      CToolBarCtrl* pBar = (CToolBarCtrl*)m_list.GetItemData(nIndex);
      BOOL bVisible = pBar->IsWindowVisible();
      gpMainFrame->ShowBar(*pBar, !bVisible);
      UINT ovl = bVisible ? 1 : 2;
      m_list.SetItemState(nIndex, INDEXTOSTATEIMAGEMASK(ovl), LVIS_STATEIMAGEMASK);
    }
  }
  *pResult = 0;
}

void CToolbarPageDlg::OnDelete()
{
  int index = m_list.GetNextItem(-1, LVNI_ALL | LVNI_SELECTED);
  if (index >= 0 && AppMessage(IDS_QUERY_BARDELETE, NULL, MB_QYESNO) == IDYES) {
    gpMainFrame->DeleteToolBar(index);
    m_list.DeleteItem(index);
  }
}

BOOL CToolbarPageDlg::OnApply()
{
  if (!::IsWindow(m_hWnd)) return TRUE;

  return TRUE;
}


/////////////////////////////////////////////////////////////////////////////
// CDrivePageDlg メッセージ ハンドラ

class CNetComputer
{
public:
  CString m_strPath;
  CString m_strIconPath;
  int m_iImage;
public:
  CNetComputer()
    : m_iImage(-1)
  {
  }
  ~CNetComputer()
  {
  }
};

BOOL CDrivePageDlg::OnInitDialog(LPARAM lParam) 
{
  m_list = GetDlgItem(IDC_LIST);
  m_net = GetDlgItem(IDC_NETCOM);

  HIMAGELIST him, dummy;
  Shell_GetImageLists(&dummy, &him);

  CRect rc;
  m_list.GetClientRect(&rc);
  m_net.InsertColumn(0, NULL, LVCFMT_LEFT, rc.Width() - GetSystemMetrics(SM_CXVSCROLL), 0);
  m_net.SetImageList(him, LVSIL_SMALL);

  m_list.GetClientRect(&rc);
  m_list.InsertColumn(0, NULL, LVCFMT_LEFT, rc.Width() - GetSystemMetrics(SM_CXVSCROLL), 0);
  m_list.SetImageList(him, LVSIL_SMALL);
  him = lccLoadImageList(RESINST, RT_RCDATA, MAKEINTRESOURCE(IDB_CHECK), 16, RGB(255, 0, 255));
  m_list.SetImageList(him, LVSIL_STATE);

  TCHAR buf[MAX_PATH];
  int i;
  for (i = 0; i < 26; i++) {
    PathBuildRoot(buf, i);
    int index = GetDriveType(buf);
    if (index < 2) {
      // ドライブ+？アイコン
      index = Shell_GetCachedImageIndex(fvApp.GetMyModuleName(), -IDI_DRIVEUNKNOWN, FALSE);
    } else {
      SHFILEINFO sfi = {0};
      SHGetFileInfo(buf, 0, &sfi, sizeof(sfi), SHGFI_SYSICONINDEX | SHGFI_SMALLICON | SHGFI_DISPLAYNAME);
      lstrcpy(buf, sfi.szDisplayName);
      index = sfi.iIcon;
    }
    index = m_list.InsertItem(i, buf, index);
    m_list.SetItemData(index, FALSE);
    m_list.SetItemState(index, INDEXTOSTATEIMAGEMASK(2), LVIS_STATEIMAGEMASK);
  }

  BYTE cb[26];
  if (GetRegBinary(IDS_SEC_DRIVE, IDS_KEY_DRIVEMASK, cb, 26) == 26) {
    for (i = 0; i < 26; i++) {
      if (cb[i]) {
        m_list.SetItemData(i, TRUE);
        m_list.SetItemState(i, INDEXTOSTATEIMAGEMASK(1), LVIS_STATEIMAGEMASK);
      }
    }
  }

  // ネットワークコンピュータ
  CString str;
  int index;
  for (i = 0; i < 10; i++) {
    if (GetRegTextSeq(IDS_SEC_DRIVE, IDS_KEY_NETCOM, str, i) < 1)
      break;

    LPTSTR p = StrChr(str, ',');
    if (p)
      *p++ = '\0';

    CNetComputer* pnc = new CNetComputer;
    pnc->m_strPath = str;
    pnc->m_strIconPath = p;
    if (p) {
      index = PathParseIconLocation(p);
      pnc->m_iImage = Shell_GetCachedImageIndex(p, index, FALSE);
    } else {
      CIDListPtr idl;
      if (idl.CreateFromPath(pnc->m_strPath)) {
        pnc->m_iImage = idl_GetSysIconIndex(idl, TRUE);
      } else {
        GetModuleFileName(GetModuleHandle(gszShell32), buf, ARRAYSIZE(buf));
        pnc->m_iImage = Shell_GetCachedImageIndex(buf, 15, FALSE);
      }
    }
    index = m_net.InsertItem(m_net.GetItemCount(), pnc->m_strPath, pnc->m_iImage);
    m_net.SetItemData(index, (DWORD_PTR)pnc);
  }

  // モバイルデバイスの取り扱い
  if (gnDrvStat & DRS_MOBILEDEVICE)	CheckDlgButton(IDC_OPMOBILEDEVICE, TRUE);
  if (gnDrvStat & DRS_NETDEVICE)		CheckDlgButton(IDC_OPNETDEVICE, TRUE);
  // ドライブバーでのLBUTTON ダブル クリック
  int sel;
  CComboBox combo = GetDlgItem(IDC_DRIVE_LBTNDBLCLK);
  InitDlgCtrl(combo, IDS_DRIVE_DBLCLICK);
  switch (gnDrvStat & DRS_LBTN_MASK) {
  case DRS_LBTN_NEWTAB: sel = 1; break;
  case DRS_LBTN_NEWWINDOW: sel = 2; break;
  default: sel = 0; break;
  }
  combo.SetCurSel(sel);
  // ドライブバーでのMBUTTON ダブル クリック
  combo = GetDlgItem(IDC_DRIVE_MBTNDBLCLK);
  InitDlgCtrl(combo, IDS_DRIVE_DBLCLICK);
  switch (gnDrvStat & DRS_MBTN_MASK) {
  case DRS_MBTN_NEWTAB: sel = 1; break;
  case DRS_MBTN_NEWWINDOW: sel = 2; break;
  default: sel = 0; break;
  }
  combo.SetCurSel(sel);

  return TRUE;
}

BOOL CDrivePageDlg::OnApply()
{
  if (!::IsWindow(m_hWnd)) return TRUE;

  if (!m_bUpdate) return TRUE;

  BYTE cb[26];
  int i;
  for (i = 0; i < 26; i++)
    cb[i] = (m_list.GetItemData(i) != 0) ? 1 : 0;
  SetRegBinary(IDS_SEC_DRIVE, IDS_KEY_DRIVEMASK, cb, 26);

  int numCustomDrives = m_net.GetItemCount();
  for (i = 0; i < 10; i++) {
    CString cs;
    LPCTSTR psz = NULL;
    if (i < numCustomDrives) {
      CNetComputer* pnc = (CNetComputer*)m_net.GetItemData(i);
      if (pnc->m_strIconPath.IsEmpty())
        cs = pnc->m_strPath;
      else
        cs.Format(_T("%s,%s"), pnc->m_strPath.GetString(), pnc->m_strIconPath.GetString());
      psz = cs.GetString();
    }
    SetRegTextSeq(IDS_SEC_DRIVE, IDS_KEY_NETCOM, psz, i);
  }

  // モバイルデバイス
  DWORD nStat = 0;
  if (IsDlgButtonChecked(IDC_OPMOBILEDEVICE))	nStat |= DRS_MOBILEDEVICE;
  if (IsDlgButtonChecked(IDC_OPNETDEVICE))	nStat |= DRS_NETDEVICE;
  // ドライブバーでのL/M BUTTON ダブルクリック
  // ここでの設定項目以外をマスクして保持
  CComboBox combo = GetDlgItem(IDC_DRIVE_LBTNDBLCLK);
  switch (combo.GetCurSel()) {
  case 1: nStat |= DRS_LBTN_NEWTAB; break;
  case 2: nStat |= DRS_LBTN_NEWWINDOW; break;
  }
  combo = GetDlgItem(IDC_DRIVE_MBTNDBLCLK);
  switch (combo.GetCurSel()) {
  case 1: nStat |= DRS_MBTN_NEWTAB; break;
  case 2: nStat |= DRS_MBTN_NEWWINDOW; break;
  }

  if (gnDrvStat != nStat) {
    gnDrvStat = nStat;
    m_bUpdate = TRUE;
  }
  return TRUE;
}

void CDrivePageDlg::OnNotifyClick(NMHDR* /*pNmhdr*/, LRESULT* /*pResult*/)
{
  CPoint pt;
  GetCursorPos(&pt);
  m_list.ScreenToClient(&pt);
  UINT nFlags;
  int nIndex = m_list.HitTest(pt, &nFlags);
  if (nIndex != -1 && nFlags & LVHT_ONITEMSTATEICON)
    ToggleMark(nIndex);
}

void CDrivePageDlg::OnNotifyKeyDown(NMHDR* pNmhdr, LRESULT* /*pResult*/)
{
  LV_KEYDOWN* plvk = (LV_KEYDOWN*)pNmhdr;
  if (plvk->wVKey == VK_SPACE) {
    int nIndex = m_list.GetTopIndex();
    nIndex = m_list.GetNextItem(nIndex - 1, LVNI_SELECTED);
    if (nIndex != -1)
      ToggleMark(nIndex);
  }
}

void CDrivePageDlg::ToggleMark(int nIndex)
{
  BOOL bMark = (m_list.GetItemData(nIndex) == FALSE);
  m_list.SetItemData(nIndex, bMark);
  bMark = (bMark == FALSE) ? 2 : 1;
  m_list.SetItemState(nIndex, INDEXTOSTATEIMAGEMASK(bMark), LVIS_STATEIMAGEMASK);
  m_bUpdate = TRUE;
}

void CDrivePageDlg::OnNotifyCustomDriveKeyDown(NMHDR* pNmhdr, LRESULT* pResult)
{
  LV_KEYDOWN* plvkd = (LV_KEYDOWN*)pNmhdr;
  *pResult = TRUE;

  int sel;
  BOOL vkeyControl = _ISKEYDOWN(VK_CONTROL);
  switch (plvkd->wVKey) {
  case VK_UP:
    if (vkeyControl) {
      sel = m_net.GetNextItem(-1, LVNI_SELECTED);
      if (sel > 0)
        ExchangeNetCom(sel - 1, sel);
    }
    break;
  case VK_DOWN:
    if (vkeyControl) {
      sel = m_net.GetNextItem(-1, LVNI_SELECTED);
      if (sel >= 0 && sel < m_net.GetItemCount() - 1)
        ExchangeNetCom(sel, sel + 1);
    }
    break;
  default:
    *pResult = FALSE;
    break;
  }
}

void CDrivePageDlg::OnAdd()
{
  if (m_net.GetItemCount() < 10) {
    CNetComputer* pnc = new CNetComputer;
    pnc->m_strPath = sszSampleServer;

    TCHAR buf[MAX_PATH];
    GetModuleFileName(GetModuleHandle(gszShell32), buf, ARRAYSIZE(buf));
    int index = m_net.InsertItem(m_net.GetItemCount(), pnc->m_strPath, -1);
    m_net.SetItemData(index, (DWORD_PTR)pnc);
    m_net.SelectItem(index);
    m_net.SetFocus();
    m_net.EditLabel(index);
  }
  else
    AppMessage(IDS_ERR_MAXNETCOM);
}

void CDrivePageDlg::OnDelete()
{
  int sel = m_net.GetNextItem(-1, LVNI_SELECTED);
  if (sel >= 0) {
    m_net.DeleteItem(sel);
    m_bUpdate = TRUE;
  }
}

void CDrivePageDlg::OnChangeIcon()
{
  int sel = m_net.GetNextItem(-1, LVNI_SELECTED);
  if (sel >= 0) {
    CNetComputer* pnc = (CNetComputer*)m_net.GetItemData(sel);
    TCHAR buf[MAX_PATH * 2];
    int index;
    if (!pnc->m_strIconPath.IsEmpty()) {
      lstrcpy(buf, pnc->m_strIconPath);
      index = PathParseIconLocation(buf);
    } else {
      LPTSTR pw;
      if (SearchPath(NULL, _T("imageres.dll"), NULL, ARRAYSIZE(buf), buf, &pw) == 0) {
        HMODULE hmod = GetModuleHandle(gszShell32);
        GetModuleFileName(hmod, buf, ARRAYSIZE(buf));
      }
      index = -1;
    }

    if (fopGetIconPath(m_hWnd, buf, &index)) {
      if (buf[0]) {
        pnc->m_strIconPath.Format(_T("%s,%d"), buf, index);
        pnc->m_iImage = Shell_GetCachedImageIndex(buf, index, FALSE);
      } else {
        pnc->m_strIconPath.Empty();
        CIDListPtr idl;
        if (idl.CreateFromPath(pnc->m_strPath)) {
          pnc->m_iImage = idl_GetSysIconIndex(idl);
        } else {
          TCHAR shl[MAX_PATH];
          GetModuleFileName(GetModuleHandle(gszShell32), shl, ARRAYSIZE(shl));
          pnc->m_iImage = Shell_GetCachedImageIndex(shl, 15, FALSE);
        }
      }

      LVITEM lvi;
      memset(&lvi, 0, sizeof(lvi));
      lvi.mask = LVIF_IMAGE;
      lvi.iImage = pnc->m_iImage;
      lvi.iItem = sel;
      m_net.SetItem(&lvi);

      m_bUpdate = TRUE;
    }
  }
}

void CDrivePageDlg::ExchangeNetCom(int item1, int item2)
{
  TCHAR src[MAX_PATH], dst[MAX_PATH];
  LVITEM si, di;

  memset(&si, 0, sizeof(si));
  si.mask = LVIF_IMAGE | LVIF_PARAM | LVIF_TEXT | LVIF_STATE;
  si.stateMask = LVIS_FOCUSED | LVIS_SELECTED;
  si.iItem = item1;
  si.pszText = src;
  si.cchTextMax = sizeof(src);
  m_net.GetItem(&si);

  memset(&di, 0, sizeof(di));
  di.mask = LVIF_IMAGE | LVIF_PARAM | LVIF_TEXT | LVIF_STATE;
  di.stateMask = LVIS_FOCUSED | LVIS_SELECTED;
  di.iItem = item2;
  di.pszText = dst;
  di.cchTextMax = sizeof(dst);
  m_net.GetItem(&di);

  si.iItem = item2;
  m_net.SetItem(&si);
  di.iItem = item1;
  m_net.SetItem(&di);

  m_bUpdate = TRUE;
}

void CDrivePageDlg::OnNotifyCustomDriveDeleteItem(NMHDR* pNmhdr, LRESULT* /*pResult*/)
{
  NMLISTVIEW* plv = (NMLISTVIEW*)pNmhdr;
  CNetComputer* pnc = (CNetComputer*)m_net.GetItemData(plv->iItem);
  if (pnc)
    delete pnc;
}

void CDrivePageDlg::OnBeginLabelEditList(NMHDR* /*pNMHDR*/, LRESULT* pResult) 
{
  *pResult = 0;
}

void CDrivePageDlg::OnEndLabelEditList(NMHDR* pNMHDR, LRESULT* pResult) 
{
  *pResult = FALSE;
  LV_DISPINFO* info_ptr = (LV_DISPINFO*)pNMHDR;
  if (info_ptr->item.pszText == NULL)
    return;

  CString name = info_ptr->item.pszText;
  if (name.IsEmpty()) {
    // エントリを削除
    m_net.DeleteItem(info_ptr->item.iItem);
    return;
  }
  else if (!PathIsUNC(name)) {
    // 元の名前
//		m_net.SetItemText(info_ptr->item.iItem, 0, pnc->m_strPath.GetString());
    AppMessage(IDS_NETNAME_FORMAT);
    return;
  }

  CNetComputer* pnc = (CNetComputer*)m_net.GetItemData(info_ptr->item.iItem);
  if (name.CompareNoCase(pnc->m_strPath)) {
    // 名前に変更があれば
    pnc->m_strPath = name;
    pnc->m_strIconPath.Empty();
    pnc->m_iImage = -1;

    // 正確なアイコンではなくて、標準のアイコンを取得
    SHFILEINFO sfi;
    SHGetFileInfo(pnc->m_strPath, FILE_ATTRIBUTE_DIRECTORY, &sfi, sizeof(sfi), SHGFI_USEFILEATTRIBUTES | SHGFI_SYSICONINDEX);
    pnc->m_iImage = sfi.iIcon;

    LVITEM lvi;
    memset(&lvi, 0, sizeof(lvi));
    lvi.mask = LVIF_IMAGE;
    lvi.iItem = info_ptr->item.iItem;
    lvi.iImage = pnc->m_iImage;
    m_net.SetItem(&lvi);

    m_bUpdate = TRUE;
    *pResult = TRUE;
  }
}

/////////////////////////////////////////////////////////////////////////////
// CProgramPageDlg メッセージ ハンドラ

BOOL CProgramPageDlg::OnInitDialog(LPARAM lParam) 
{
  CString str;
  if (GetAddin(addinMsdos, str))			SetDlgItemText(IDC_MSDOS, str);
  if (GetAddin(addinEditor, str))			SetDlgItemText(IDC_EDITOR, str);
  if (GetAddin(addinViewer, str))			SetDlgItemText(IDC_VIEWER, str);
  if (GetAddin(addinBmpViewer, str))		SetDlgItemText(IDC_PHOTOVIEWER, str);
  if (GetAddin(addinBDump, str))			SetDlgItemText(IDC_HEXEDITOR, str);
  return TRUE;
}

BOOL CProgramPageDlg::OnApply()
{
  if (!::IsWindow(m_hWnd)) return TRUE;

  CString str;
  GetDlgItemText(IDC_MSDOS, str);			SetAddin(addinMsdos, str);
  GetDlgItemText(IDC_EDITOR, str);		SetAddin(addinEditor, str);
  GetDlgItemText(IDC_VIEWER, str);		SetAddin(addinViewer, str);
  GetDlgItemText(IDC_PHOTOVIEWER, str);	SetAddin(addinBmpViewer, str);
  GetDlgItemText(IDC_HEXEDITOR, str);		SetAddin(addinBDump, str);
  return TRUE;
}

BOOL CProgramPageDlg::Browse(LPTSTR psz, int cchMax)
{
  TCHAR buf[MAX_PATH * 2];
  SHGetSpecialFolderPath(NULL, buf, CSIDL_PROGRAM_FILES, FALSE);
  CString str;
  if (FileBrowse(TRUE, m_hWnd, IDS_FILTER_EXECUTE, str, buf)) {
    lstrcpyn(buf, str, ARRAYSIZE(buf));
    PathQuoteSpaces(buf);
    lstrcpyn(psz, buf, cchMax);
    return TRUE;
  }
  return FALSE;
}

void CProgramPageDlg::OnNotifyBrowse(int idCtrl, NMHDR* pNmhdr, LRESULT* pResult)
{
  NMSEGETTEXT* psgt = (NMSEGETTEXT*)pNmhdr;
  *pResult = (LRESULT)Browse(psgt->pszText, psgt->cchTextMax);
}

/////////////////////////////////////////////////////////////////////////////
// CAddinPageDlg メッセージ ハンドラ

BOOL CAddinPageDlg::OnInitDialog(LPARAM lParam) 
{
  m_tab = GetDlgItem(IDC_TAB);
  m_list = GetDlgItem(IDC_LIST);

  CString str;
  str.LoadString(IDS_ADDIN_VIEWER);
  m_tab.InsertItem(0, str);
  str.LoadString(IDS_ADDIN_EDITOR);
  m_tab.InsertItem(1, str);

  str.LoadString(IDS_OPVIEWER_EXT);
  m_list.InsertColumn(0, str, LVCFMT_LEFT, 70, 0);
  str.LoadString(IDS_OPVIEWER_TOOLS);
  m_list.InsertColumn(1, str, LVCFMT_LEFT, 300, 0);

  HIMAGELIST himL, himS;
  Shell_GetImageLists(&himL, &himS);
  m_list.SendMessage(LVM_SETIMAGELIST, LVSIL_SMALL, (LPARAM)himS);

  // アドインビュアで定義されている拡張子を取得
  CSimpleArray<CString> arr;
  CAddinTools* pat;
  int i, cnt;
  cnt = GetRegEnumValue(IDS_SEC_ADDIN_VIEWER, arr);
  for (i = 0; i < cnt; i++) {
    if (arr[i][0] == '.') {
      if (GetRegText(IDS_SEC_ADDIN_VIEWER, (LPCTSTR)arr[i], str) > 0) {
        pat = new CAddinTools(arr[i], str);
        pat->m_strExt.MakeUpper();
        m_mapViewer.Add(pat->m_strExt, pat);
      }
    }
  }
  arr.RemoveAll();
  cnt = GetRegEnumValue(IDS_SEC_ADDIN_EDITOR, arr);
  for (i = 0; i < cnt; i++) {
    if (arr[i][0] == '.') {
      if (GetRegText(IDS_SEC_ADDIN_EDITOR, (LPCTSTR)arr[i], str) > 0) {
        pat = new CAddinTools(arr[i], str);
        pat->m_strExt.MakeUpper();
        m_mapEditor.Add(pat->m_strExt, pat);
      }
    }
  }

  SetActivePage(0);
  return TRUE;
}

void CAddinPageDlg::OnDestroy()
{
  CAddinTools* pav;
  int i, cnt;
  cnt = m_mapViewer.GetSize();
  for (i = 0; i < cnt; i++) {
    pav = m_mapViewer.GetValueAt(i);
    if (pav)
      delete pav;
  }
  m_mapViewer.RemoveAll();
  cnt = m_mapEditor.GetSize();
  for (i = 0; i < cnt; i++) {
    pav = m_mapEditor.GetValueAt(i);
    if (pav)
      delete pav;
  }
  m_mapEditor.RemoveAll();
}

void CAddinPageDlg::SetActivePage(int page)
{
  CSimpleMap<CString, CAddinTools*>& map = (page == 0) ? m_mapViewer : m_mapEditor;
  m_list.DeleteAllItems();
  int cnt = map.GetSize();
  for (int i = 0; i < cnt; i++)
    InsertItem(map.GetValueAt(i));
}

void CAddinPageDlg::OnTabChanged(NMHDR* pNmhdr, LRESULT* pResult)
{
  SetActivePage(m_tab.GetCurSel());
}

void CAddinPageDlg::OnGetDispInfo(NMHDR* pNmhdr, LRESULT* pResult)
{
  *pResult = 0;
  LV_DISPINFO* plvdi  = (LV_DISPINFO*)pNmhdr;
  if (plvdi) {
    if (plvdi && plvdi->item.mask & LVIF_IMAGE) {
      plvdi->item.iImage = -1;

      CAddinTools* pat = (CAddinTools*)plvdi->item.lParam;
      if (pat && pat->m_strPath[0] != '#') {
        TCHAR buf[MAX_PATH * 2], ww[MAX_PATH * 2];
        lstrcpy(buf, pat->m_strPath);
        LPTSTR p = NULL;
        if (buf[0] == '"') {
          lstrcpy(ww, &buf[1]);
          p = StrChr(ww, '"');
        }
        else {
          lstrcpy(ww, buf);
          p = StrChr(ww, ' ');
        }

        if (p)
          *p = '\0';
        SHFILEINFO sfi;
        SHGetFileInfo(ww, 0, &sfi, sizeof(sfi), SHGFI_SYSICONINDEX | SHGFI_SMALLICON);
        plvdi->item.iImage = sfi.iIcon;
        plvdi->item.mask |= LVIF_DI_SETITEM;
      }
    }
  }
}

BOOL CAddinPageDlg::OnApply()
{
  if (!::IsWindow(m_hWnd)) return TRUE;

  CRegKey app;
  GetRegApp(app);

  CString str;
  CAddinTools* pat;
  int i, cnt;

  // 既存のViewerエントリを削除
  str.LoadString(IDS_SEC_ADDIN_VIEWER);
  SHDeleteKey(app, str);
  // Viewerエントリを登録
  cnt = m_mapViewer.GetSize();
  for (i = 0; i < cnt; i++) {
    pat = m_mapViewer.GetValueAt(i);
    SetRegText(IDS_SEC_ADDIN_VIEWER, pat->m_strExt.GetString(), pat->m_strPath);
  }

  // 既存のEditorエントリを削除
  str.LoadString(IDS_SEC_ADDIN_EDITOR);
  SHDeleteKey(app, str);
  // Editorエントリを登録
  cnt = m_mapEditor.GetSize();
  for (i = 0; i < cnt; i++) {
    pat = m_mapEditor.GetValueAt(i);
    SetRegText(IDS_SEC_ADDIN_EDITOR, pat->m_strExt.GetString(), pat->m_strPath);
  }

  return TRUE;
}

void CAddinPageDlg::OnAddTools()
{
  CSimpleMap<CString, CAddinTools*>& map = (m_tab.GetCurSel() == 0) ? m_mapViewer : m_mapEditor;

  CAddinToolsDlg dlg;
  dlg.m_bAdd = TRUE;
  if (dlg.DoModal() == IDOK) {
    LV_FINDINFO lvf;
    lvf.flags = LVFI_STRING;
    lvf.psz = dlg.m_at.m_strExt;
    if (m_list.FindItem(&lvf, -1) < 0) {
      CAddinTools* pat = new CAddinTools(dlg.m_at);
      pat->m_strExt.MakeUpper();
      map.Add(pat->m_strExt, pat);
      InsertItem(pat);
    }
    else
      AppMessage(IDS_ADDINTOOLS_EXIST, dlg.m_at.m_strExt, MB_STOP);
  }
}

void CAddinPageDlg::OnDeleteTools()
{
  CSimpleMap<CString, CAddinTools*>& map = (m_tab.GetCurSel() == 0) ? m_mapViewer : m_mapEditor;

  int sel = m_list.GetNextItem(-1, LVNI_SELECTED);
  if (sel < 0) return;

  CAddinTools* pat = (CAddinTools*)m_list.GetItemData(sel);
  int index = map.FindKey(pat->m_strExt);
  if (index >= 0) {
    pat = map.GetValueAt(index);
    if (pat)
      delete pat;
    map.RemoveAt(index);
  }

  m_list.DeleteItem(sel);
  int cnt = m_list.GetItemCount();
  if (cnt > 0) {
    sel = min(sel, cnt - 1);
    m_list.SelectItem(sel);
  }
}

void CAddinPageDlg::OnEditTools()
{
  int sel = m_list.GetNextItem(-1, LVNI_SELECTED);
  if (sel < 0) return;

  CAddinTools* pat = (CAddinTools*)m_list.GetItemData(sel);
  CAddinToolsDlg dlg;
  dlg.m_at = *pat;
  if (dlg.DoModal()) {
    *pat = dlg.m_at;
    m_list.SetItemText(sel, 0, pat->m_strExt);
    m_list.SetItemText(sel, 1, pat->m_strPath);

    LV_ITEM lvi;
    lvi.iItem = sel;
    lvi.iSubItem = 0;
    lvi.mask = LVIF_IMAGE;
    lvi.iImage = I_IMAGECALLBACK;
    m_list.SetItem(&lvi);
  }
}

void CAddinPageDlg::OnDuplicateTools()
{
  int sel = m_list.GetNextItem(-1, LVNI_SELECTED);
  if (sel < 0) return;

  CAddinTools* pat = (CAddinTools*)m_list.GetItemData(sel);
  CAddinTools* patNew = new CAddinTools(_T(""), pat->m_strPath);

  CSimpleMap<CString, CAddinTools*>& map = (m_tab.GetCurSel() == 0) ? m_mapViewer : m_mapEditor;
  map.Add(patNew->m_strExt, patNew);

  int n = InsertItem(patNew);
  m_list.SelectItem(n);
  OnEditTools();
}

int CAddinPageDlg::InsertItem(CAddinTools* pat)
{
  pat->m_strExt.MakeUpper();

  int index = m_list.InsertItem(m_list.GetItemCount(), pat->m_strExt, I_IMAGECALLBACK);
  m_list.SetItemText(index, 1, pat->m_strPath);
  m_list.SetItemData(index, (DWORD_PTR)pat);
  return index;
}

/////////////////////////////////////////////////////////////////////////////
// CHotkeyPageDlg dialog

typedef struct tagHDENTRY {
  UINT id;
  int cx;
  DWORD fmt;
  int image;
} HDENTRY;
static HDENTRY shdHot[] = {
  IDS_CTLMENU_HEADER_TITLE,	140, LVCFMT_LEFT,	-1,			// タイトル
  IDS_CTLMENU_HEADER_OPTION,	30,  LVCFMT_LEFT,	-1,			// 事前編集
  IDS_CTLMENU_HEADER_PROGRAM,	100, LVCFMT_LEFT,   -1,			// プログラム
  IDS_CTLMENU_HEADER_ARGMENT,	90,  LVCFMT_LEFT,	-1,			// 引数
  IDS_CTLMENU_HEADER_WORKDIR,	100, LVCFMT_LEFT,	-1,			// 作業フォルダ
};

BOOL CHotkeyPageDlg::OnInitDialog(LPARAM lParam)
{
  CenterWindow();

  GetDlgItem(IDC_COPY).EnableWindow(FALSE);
  GetDlgItem(IDC_PASTE).EnableWindow(FALSE);

  m_list = GetDlgItem(IDC_LIST);

  // アルファベットキーのイメージリスト
  m_hImage = lccLoadImageList(RESINST, RT_RCDATA, MAKEINTRESOURCE(IDB_KEYBOARD), 20, RGB(255, 0, 255));
  m_list.SetImageList(m_hImage, LVSIL_SMALL);
  DWORD dwStyle = (DWORD)m_list.SendMessage(LVM_GETEXTENDEDLISTVIEWSTYLE);
  dwStyle |= LVS_EX_FULLROWSELECT;
  m_list.SendMessage(LVM_SETEXTENDEDLISTVIEWSTYLE, 0, dwStyle);

  // カラム初期設定
  CString name;
  int i;
  for (i = 0; i < ARRAYSIZE(shdHot); i++) {
    name.Empty();
    name.LoadString(shdHot[i].id);
    m_list.InsertColumn(i, name, shdHot[i].fmt, shdHot[i].cx, i, shdHot[i].image);
  }

  CHotKeyItem* pKey[26];
  memset(pKey, 0, sizeof(pKey));
  RestoreHotKey(pKey);
  for (i = 0; i < 26; i++) {
    CHotKeyItem* pck = pKey[i];
    if (pck) {
      int index = m_list.InsertItem(i, pck->m_name, i + 10);
      m_list.SetItemData(index, (DWORD_PTR)pck);
      SetItemColumn(index, pck);
    } else {
      m_list.InsertItem(i, NULL, i + 10);
    }
  }
  m_list.SelectItem(0);

  return TRUE;
}

BOOL CHotkeyPageDlg::OnApply()
{
  if (!::IsWindow(m_hWnd)) return TRUE;
  if (!m_bUpdate) return TRUE;

  CHotKeyItem* pKey[26];
  memset(pKey, 0, sizeof(pKey));
  for (int i = 0; i < 26; i++) {
    CHotKeyItem* pck = (CHotKeyItem*)m_list.GetItemData(i);
    if (pck) {
      pck->m_key = i;
      pKey[i] = pck;
    }
  }

  CString str;
  BuildHotKeyFileName(str);
  if (SaveHotKey(pKey, str)) {
    fvApp.menuHotKey.Restore();
    fvApp.MakeLocalAccelTable();
  }
  return TRUE;
}

void CHotkeyPageDlg::SetItemColumn(int index, CHotKeyItem* item)
{
  if (index < 0 || index >= m_list.GetItemCount()) return;

  CString str;
  if (item->m_bEdit)		str += _T("事前編集,");
  if (item->m_bAsAdmin)	str += _T("管理者権限");
  if (!str.IsEmpty())
    str.TrimRight(_T(','));
  else
    str.LoadString(IDS_UNDEFINED);
  m_list.SetItemText(index, 1, str.GetString());
  m_list.SetItemText(index, 2, item->m_exec);
  m_list.SetItemText(index, 3, item->m_param);
  m_list.SetItemText(index, 4, item->m_workdir);

  m_list.SetColumnWidth(0, LVSCW_AUTOSIZE);
  m_list.SetColumnWidth(1, LVSCW_AUTOSIZE);
  m_list.SetColumnWidth(2, LVSCW_AUTOSIZE);
}

void CHotkeyPageDlg::OnConfirm()
{
  int sel = m_list.GetNextItem(-1, LVNI_SELECTED);
  if (sel >= 0) {
    CHotKeyExecPage pageExec;
    CHotKeyFolderPage pageFolder;
    CHotKeySortPage pageSort;
    CHotKeyFilterPage pageFilter;

    CHotKeyItem item;
    CHotKeyItem* pCurItem = (CHotKeyItem*)m_list.GetItemData(sel);
    if (pCurItem)
      item.CopyFrom(*pCurItem);
    else
      item.m_key = sel;
    pageExec.m_itemptr = &item;
    pageFolder.m_itemptr = &item;
    pageSort.m_itemptr = &item;
    pageFilter.m_itemptr = &item;

    // キャプション設定
    CString str;
    str.Format(IDS_CAPTION_EDITHOTKEY, item.m_key + 'A');
    CEditHotKeySheet sheet(str.GetString());
    sheet.AddPage(pageExec);
    sheet.AddPage(pageFolder);
    sheet.AddPage(pageSort);
    sheet.AddPage(pageFilter);

    int pageIndex;
    switch (item.m_type) {
    case HOTKEY_EXEC:		pageIndex = 0; break;
    case HOTKEY_FOLDER:		pageIndex = 1; break;
    case HOTKEY_SORT:		pageIndex = 2; break;
    case HOTKEY_FILTER:		pageIndex = 3; break;
    default:				pageIndex = 0; break;
    }
    sheet.SetActivePage(pageIndex);

    if (sheet.DoModal() == IDOK) {
      if (!pCurItem)
        pCurItem = new CHotKeyItem();
      pCurItem->CopyFrom(item);
      m_list.SetItemData(sel, (DWORD_PTR)pCurItem);
      m_list.SetItemText(sel, 0, pCurItem->m_name);
      SetItemColumn(sel, pCurItem);

      m_bUpdate = TRUE;
    }
  }
}

void CHotkeyPageDlg::OnCopy()
{
  int sel = m_list.GetNextItem(-1, LVNI_SELECTED);
  if (sel >= 0) {
    CHotKeyItem* item = (CHotKeyItem*)m_list.GetItemData(sel);
    if (item) {
      m_clip = *item;
      GetDlgItem(IDC_PASTE).EnableWindow(TRUE);
    }
  }
}

void CHotkeyPageDlg::OnPaste()
{
  if (m_clip.isNull()) {
    MessageBeep(0);
    return;
  }

  int sel = m_list.GetNextItem(-1, LVNI_SELECTED);
  if (sel >= 0) {
    CHotKeyItem* item = (CHotKeyItem*)m_list.GetItemData(sel);
    if (item)
      *item = m_clip;
    else {
      item = new CHotKeyItem();
      *item = m_clip;
      m_list.SetItemData(sel, (DWORD_PTR)item);
    }
    item->m_key = sel;

    m_list.SetItemText(sel, 0, item->m_name);
    SetItemColumn(sel, item);

    m_bUpdate = TRUE;
  }
}

void CHotkeyPageDlg::OnDelete()
{
  int sel = m_list.GetNextItem(-1, LVNI_SELECTED);
  if (sel >= 0) {
    CHotKeyItem* item = (CHotKeyItem*)m_list.GetItemData(sel);
    if (item) {
      delete item;
      m_list.SetItemData(sel, 0);
      m_list.SetItemText(sel, 0, NULL);
      m_list.SetItemText(sel, 1, NULL);
      m_list.SetItemText(sel, 2, NULL);
      m_list.SetItemText(sel, 3, NULL);
      m_list.SetItemText(sel, 4, NULL);
      m_bUpdate = TRUE;
    }
  }
}

void CHotkeyPageDlg::OnNotifyDblclklist(NMHDR* /*pNMHDR*/, LRESULT* pResult) 
{
  OnConfirm();
  *pResult = 0;
}

void CHotkeyPageDlg::OnNotifyCustomDraw(NMHDR *pHdr, LRESULT *pResult)
{
  NMLVCUSTOMDRAW* pcd = (NMLVCUSTOMDRAW*)pHdr;
  *pResult = CDRF_DODEFAULT;
  switch (pcd->nmcd.dwDrawStage) {
  case CDDS_PREPAINT:
    *pResult = CDRF_NOTIFYITEMDRAW;
    break;
  case CDDS_ITEMPREPAINT:
    if (pcd->nmcd.dwItemSpec & 1)
      *pResult = CDRF_DODEFAULT;
    else {
      pcd->clrTextBk = mix_color(GetSysColor(COLOR_WINDOW), GetSysColor(COLOR_HIGHLIGHT), 94);
      *pResult = CDRF_NEWFONT;
    }
    break;
  }
}

void CHotkeyPageDlg::OnNotifyItemChanged(NMHDR *pHdr, LRESULT *pResult)
{
  NMLISTVIEW* plv = (NMLISTVIEW*)pHdr;
  BOOL bHasItem = (m_list.GetItemData(plv->iItem) != NULL);
  GetDlgItem(IDC_COPY).EnableWindow(bHasItem);
  GetDlgItem(IDC_DELETE).EnableWindow(bHasItem);
}

void CHotkeyPageDlg::OnNotifyDeleteItem(NMHDR *pHdr, LRESULT *pResult)
{
  NMLISTVIEW* plv = (NMLISTVIEW*)pHdr;
  CHotKeyItem* item = (CHotKeyItem*)m_list.GetItemData(plv->iItem);
  if (item) delete item;
}


/////////////////////////////////////////////////////////////////////////////
// CMapIconPageDlg message handlers

BOOL CMapIconPageDlg::OnInitDialog(LPARAM lParam)
{
  CenterWindow();
  m_list = GetDlgItem(IDC_LIST);

  // 列の挿入
  LV_COLUMN lvc;
  lvc.mask = LVCF_FMT | LVCF_WIDTH | LVCF_TEXT | LVCF_SUBITEM;

  int i = 0;
  lvc.iSubItem = i;
  lvc.pszText = _T("拡張子");
  lvc.cx = 70;
  lvc.fmt = LVCFMT_LEFT;
  m_list.InsertColumn(i++, &lvc);

  lvc.iSubItem = i;
  lvc.pszText = _T("抽出元");
  lvc.cx = 230;
  lvc.fmt = LVCFMT_LEFT;
  m_list.InsertColumn(i++, &lvc);

  lvc.iSubItem = i;
  lvc.pszText = _T("インデクス");
  lvc.cx = 60;
  lvc.fmt = LVCFMT_RIGHT;
  m_list.InsertColumn(i++, &lvc);

  Listup();
  return TRUE;
}

void CMapIconPageDlg::Listup()
{
  int nCaret = m_list.GetNextItem(-1, LVNI_SELECTED);
  if (nCaret < 0)
    nCaret = 0;

  m_list.SetRedraw(FALSE);
  m_list.DeleteAllItems();

  HIMAGELIST hil, his;
  Shell_GetImageLists(&hil, &his);
  ListView_SetImageList(m_list, his, LVSIL_SMALL);
  ListView_SetImageList(m_list, hil, LVSIL_NORMAL);

  CSimpleArray<CIconMap*> arr;
  int cnt = RestoreMapIcon(arr);

  CString str;
  for (int i = 0; i < cnt; i++) {
    CIconMap* pim = arr[i];
    int index = Shell_GetCachedImageIndex(pim->m_file, pim->m_index, FALSE);
    index = m_list.InsertItem(LVIF_TEXT | LVIF_IMAGE | LVIF_PARAM, i, pim->m_ext, 0, 0, index, (LPARAM)pim);
    m_list.SetItemText(index, 1, pim->m_file);
    str.Format(_T("%d"), pim->m_index);
    m_list.SetItemText(index, 2, str.GetString());
  }

  m_list.SelectItem(nCaret);
  m_list.SetRedraw(TRUE);
  m_list.Invalidate();
}

void CMapIconPageDlg::OnNotifyDeleteItem(NMHDR* pNmhdr, LRESULT* pResult)
{
  *pResult = 0;
  NM_LISTVIEW* pnmlv = (NM_LISTVIEW*)pNmhdr;
  CIconMap* pim = (CIconMap*)m_list.GetItemData(pnmlv->iItem);
  if (pim)
    delete pim;
}

void CMapIconPageDlg::OnAdd()
{
  CSelectIconDlg dlg;
  if (dlg.DoModal() == IDOK) {
    CIconMap* pim = new CIconMap(dlg.m_im);
    LV_ITEM lvi = { 0 };
    lvi.mask = LVIF_TEXT | LVIF_IMAGE;
    lvi.iItem = m_list.GetItemCount();
    lvi.pszText = (LPTSTR)pim->GetExtension();
    lvi.iImage = Shell_GetCachedImageIndex(pim->GetIconPath(), pim->GetIconIndex(), FALSE);
    int index = m_list.InsertItem(&lvi);

    m_list.SetItemText(index, 1, pim->GetIconPath());
    CString str;
    str.Format(_T("%d"), pim->GetIconIndex());
    m_list.SetItemText(index, 2, str);
    m_list.SetItemData(index, (LPARAM)pim);
    m_bUpdate = TRUE;
  }
}

void CMapIconPageDlg::OnEdit()
{
  int index = m_list.GetNextItem(-1, LVNI_SELECTED);
  if (index < 0) {
    MessageBeep(0);
    m_list.SetFocus();
    return;
  }

  CIconMap* pim = (CIconMap*)m_list.GetItemData(index);
  if (pim) {
    CSelectIconDlg dlg;
    dlg.m_im = *pim;
    if (dlg.DoModal() == IDOK) {
      *pim = dlg.m_im;
      m_list.SetItemText(index, 1, pim->GetIconPath());
      CString str;
      str.Format(_T("%d"), pim->GetIconIndex());
      m_list.SetItemText(index, 2, str);
      m_bUpdate = TRUE;
    }
  }
}

void CMapIconPageDlg::OnDelete()
{
  int index = m_list.GetNextItem(-1, LVNI_SELECTED);
  if (index < 0) {
    MessageBeep(0);
    m_list.SetFocus();
    return;
  }

  m_list.DeleteItem(index);
  m_bUpdate = TRUE;
}

BOOL CMapIconPageDlg::OnApply()
{
  if (!::IsWindow(m_hWnd)) return TRUE;

  if (m_bUpdate) {
    CSimpleArray<CIconMap*> arr;
    int cnt = m_list.GetItemCount();
    for (int i = 0; i < cnt; i++) {
      CIconMap* pim = (CIconMap*)m_list.GetItemData(i);
      if (pim)
        arr.Add(pim);
    }

    CString str;
    BuildMapIconFileName(str);
    SaveMapIcon(arr, str);
  }
  return TRUE;
}

/////////////////////////////////////////////////////////////////////////////
// CTabPageDlg メッセージ ハンドラ

BOOL CTabPageDlg::OnInitDialog(LPARAM lParam)
{
  if (gnTabStat & TAB_BARSHOWANY)				CheckDlgButton(IDC_OPTABBARSHOWANY, TRUE);
  if (gnTabStat & TAB_CLOSENOCONFIRM) 		CheckDlgButton(IDC_OPTABCLOSEALL, TRUE);
  if (gnTabStat & TAB_FULLPATH)		 		CheckDlgButton(IDC_OPTABFULLPATH, TRUE);
  if (gnTabStat & TAB_MKDIR)		 			CheckDlgButton(IDC_OPTABMKDIR, TRUE);
  if (gnTabStat & TAB_DRIVEADD)	 			CheckDlgButton(IDC_OPTABDRIVEADD, TRUE);
  if (gnTabStat & TAB_MEDIAINSERTED)			CheckDlgButton(IDC_OPTABMEDIAINSERTED, TRUE);
  if (gnTabStat & TAB_HOVERSELECT)			CheckDlgButton(IDC_OPTABHOVERSELECT, TRUE);
  if (gnTabStat & TAB_DBLCLK_LOCK)			CheckDlgButton(IDC_OPTABDBLCLK, TRUE);
  if (gnTabStat & TAB_SAVEGROUPTOBOOKMARK)	CheckDlgButton(IDC_OPTABGROUPTOBOOKMARK, TRUE);
  if (gnTabStat & TAB_NODUPLICATE)			CheckDlgButton(IDC_OPTABNODUPLICATE, TRUE);
  if (gnTabStat & TAB_SELECT_LASTACTIVE)		CheckDlgButton(IDC_OPTABSELLASTACTIVE, TRUE);
  if (gnTabStat & TAB_DROPOPEN)				CheckDlgButton(IDC_OPTABDROPOPEN, TRUE);

  return TRUE;
}

BOOL CTabPageDlg::OnApply()
{
  if (!::IsWindow(m_hWnd)) return TRUE;

  DWORD nStat = 0;
  if (IsDlgButtonChecked(IDC_OPTABBARSHOWANY))		nStat |= TAB_BARSHOWANY;
  if (IsDlgButtonChecked(IDC_OPTABCLOSEALL))			nStat |= TAB_CLOSENOCONFIRM;
  if (IsDlgButtonChecked(IDC_OPTABFULLPATH))			nStat |= TAB_FULLPATH;
  if (IsDlgButtonChecked(IDC_OPTABMKDIR))				nStat |= TAB_MKDIR;
  if (IsDlgButtonChecked(IDC_OPTABDRIVEADD))			nStat |= TAB_DRIVEADD;
  if (IsDlgButtonChecked(IDC_OPTABMEDIAINSERTED))		nStat |= TAB_MEDIAINSERTED;
  if (IsDlgButtonChecked(IDC_OPTABHOVERSELECT))		nStat |= TAB_HOVERSELECT;
  if (IsDlgButtonChecked(IDC_OPTABDBLCLK))			nStat |= TAB_DBLCLK_LOCK;
  if (IsDlgButtonChecked(IDC_OPTABGROUPTOBOOKMARK))	nStat |= TAB_SAVEGROUPTOBOOKMARK;
  if (IsDlgButtonChecked(IDC_OPTABNODUPLICATE))		nStat |= TAB_NODUPLICATE;
  if (IsDlgButtonChecked(IDC_OPTABSELLASTACTIVE))		nStat |= TAB_SELECT_LASTACTIVE;
  if (IsDlgButtonChecked(IDC_OPTABDROPOPEN))			nStat |= TAB_DROPOPEN;
  if (gnTabStat != nStat) {
    gnTabStat = nStat;
    m_bUpdate = TRUE;
  }

  return TRUE;
}

/////////////////////////////////////////////////////////////////////////////
// CPrivacyPageDlg メッセージ ハンドラ

BOOL CPrivacyPageDlg::OnInitDialog(LPARAM lParam)
{
  return TRUE;
}

BOOL CPrivacyPageDlg::OnApply()
{
  if (!::IsWindow(m_hWnd)) return TRUE;

  return TRUE;
}

void CPrivacyPageDlg::OnClear()
{
  // フォルダ履歴
  if (IsDlgButtonChecked(IDC_HISTORY_FOLDER)) {
    fvApp.ClearDocumentHistoryAll();
  }
  // コピー・移動履歴
  if (IsDlgButtonChecked(IDC_HISTORY_COPY)) {
    RegClearHistoryEx(NULL, IDS_SEC_HISTORY, IDS_KEY_COPYTARGET);
  }
  // ドライブごとの履歴
  if (IsDlgButtonChecked(IDC_HISTORY_DRIVEREMIND)) {
    for (int i = 'A'; i <= 'Z'; i++)
      fopSetDirRemind(i - 'A', NULL);
  }
  // フィルタ設定の履歴
  CString str;
  if (IsDlgButtonChecked(IDC_HISTORY_WILDCARD)) {
    // 保存したリスト
    class CFilterSettingList fs;
    fs.GetDefaultXmlFile(str);
    fopRemoveFileNoConfirm(str);
    // 履歴
    RegClearHistoryEx(NULL, IDS_SEC_HISTORY, IDS_KEY_WILDCARD);
    RegClearHistoryEx(NULL, IDS_SEC_HISTORY, IDS_KEY_FILTERSELECT);
  }
  // 最近使ったファイルの一覧
  if (IsDlgButtonChecked(IDC_HISTORY_RECENTFILE)) {
    fopEmptyRecentFiles();
  }
  // フォルダごとに記録しているアイコンやソート順
  if (IsDlgButtonChecked(IDC_HISTORY_ICONLISTSTYLE)) {
    gpMainFrame->ResetFolderSettings();
  }
}


