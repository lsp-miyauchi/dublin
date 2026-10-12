// fv.cpp : Defines the class behaviors for the application.
//

#include "stdafx.h"
#include "fv7.h"

#include "mainfrm.h"
#include "trayfrm.h"
#include "startup.h"

void CheckAndConvertReg();
void ResetApplicationEnvironment();

////////////////////////////////////////////////////////////////////////////
//

CAppModule _Module;
CFvApp fvApp;


////////////////////////////////////////////////////////////////////////////
// アクセラレータキーデータ

#include "keydata.h"

/////////////////////////////////////////////////////////////////////////////
// CFvApp construction

CFvApp::CFvApp()
	: m_nProcessCounter(0)
	, m_hLocalAccel(NULL)
	, m_bTsr(false)
	, m_bPrimaryMonitor(true)
	, m_hStartMonitor(NULL)
{
	nCmdShow = SW_SHOWNORMAL;

	gnListStat = LTS_AUTOADJUST | LTS_ALIGNEXT | LTS_INFOTIP | LTS_UPDIR |
				 LTS_REPCHECKER | LTS_SELTREELINE | LTS_BOLDNAME | LTS_LOGICALCMP | LTS_RELAX;
	gnFileStat = FTS_EXIF | FTS_THUMBCACHE_UPDATE;
	gnSysStat = STS_LASTDIR;
	gnTsrStat = 0;
	gnFopStat = FOS_SHIFTSEARCH;
	gnMosStat = MOS_HOTTRACK;
	gnBarStat = BTS_STATUS | BTS_DRIVE | BTS_APPBAR | BTS_APPRIGHT | BTS_FAVO;
	if (GetSystemMetrics(SM_CXSCREEN) >= 1600) {
		gnBarStat |= BTS_LARGEICON;
	}
	gnTabStat = TAB_BARSHOWANY | TAB_SELECT_LASTACTIVE;
	gnDrvStat = 0;
	gnConfStat = COS_UPDIR_CONFIRM;

	gnElmaxNameLength = 32;
	gnElmaxExtLength = 8;

	// デフォルトの表示スタイル
	CIconListStyle ls;
	if (GetRegBinary(IDS_SEC_OPERATIONS, IDS_KEY_DEFAULTICONLISTSTYLE, &ls, sizeof(ls)) == sizeof(ls))
		gDefaultIconListStyle = ls;
}

CFvApp::~CFvApp()
{
	// カスタムアイコンマップ
	ClearIconMap();
}

/////////////////////////////////////////////////////////////////////////////
// CFvApp initialization

int CFvApp::Run(LPTSTR lpstrCmdLine, int nShow)
{
	if (lpstrCmdLine && *lpstrCmdLine)
		strCmdLine = lpstrCmdLine;
	nCmdShow = nShow;

	int nRet = 0;
	if (Initialize()) {
		CMessageLoop theLoop;
		_Module.AddMessageLoop(&theLoop);

		if (m_bTsr) {
			// 常駐プロセスとして動作
			CTrayFrame tray;
			if (tray.CreateEx()) {
				tray.ShowWindow(SW_HIDE);
				nRet = theLoop.Run();
			}
		} else {
			InitInstance();

			CMainFrame frame;
			gpMainFrame = &frame;
			if (frame.CreateEx()) {
				frame.InitialUpdate();
				frame.SetInitialFolder(strCmdLine);
				frame.ShowWindow(nCmdShow);
				frame.UpdateWindow();
				frame.SetInitCompleted();
				nRet = theLoop.Run();
			} else {
				CannotCreateWindow(NULL, _T("Main Window"));
			}
			ExitInstance();
		}
		_Module.RemoveMessageLoop();
	}
	return nRet;
}

LPCTSTR CFvApp::GetMyModuleName()
{
	if (m_strMyModuleName.IsEmpty()) {
		LPTSTR p = m_strMyModuleName.GetBuffer(MAX_PATH * 2);
		GetModuleFileName(NULL, p, MAX_PATH * 2);
		m_strMyModuleName.ReleaseBuffer();
	}
	return m_strMyModuleName;
}


BOOL CFvApp::Initialize()
{
#ifndef _DEBUG
#ifndef _WIN64
//	// NeoLiteが解除されていたら終了しましょう。
//	WIN32_FILE_ATTRIBUTE_DATA wfad;
//	if (GetFileAttributesEx(GetMyModuleName(), GetFileExInfoStandard, &wfad) &&
//		wfad.nFileSizeLow > 2000 * 1024) {
//		return FALSE;
//	}
#endif	// !_WIN64
#endif	// !_DEBUG

	// admin
	m_bAdmin = IsUserAnAdmin();
	// HtmlHelp
	CString str;
	LPTSTR p = str.GetBuffer(MAX_PATH * 2);
	lstrcpy(p, fvApp.GetMyModuleName());
	PathRenameExtension(p, _T(".chm"));
	str.ReleaseBuffer();
	m_Help.SetHelpPath(str);

	// 常駐プロセスの存在
	bool otherTsr = (GetWindowNumCount(APP_TRAYCLASS, NULL, FALSE) > 0);
	bool otherApp = (GetWindowNumCount(APP_FRAMECLASS, NULL, FALSE) > 0);

	// リカバリ処理
	if (!StrCmpNI(strCmdLine, _T("/recover"), 7)) {
		if (otherTsr || otherApp) {
			AppMessage(IDS_RECOVERY_START, NULL, MB_STOP);
			return FALSE;
		}
		switch (AppMessage(IDS_RECOVERY_START, NULL, MB_QUESTION)) {
		case IDYES:
			ResetApplicationEnvironment();
			return FALSE;
		case IDCANCEL:
			return FALSE;
		default:
			break;
		}
	}

	// ユーザー登録抹消処理
	if (!StrCmpNI(strCmdLine, _T("/unreg"), 5)) {
		if (otherTsr || otherApp) {
			AppMessage(IDS_UNREGIST_START, NULL, MB_STOP);
		} else if (AppMessage(IDS_RECOVERY_START, NULL, MB_QYESNO) == IDYES) {
			SetRegText(IDS_SEC_USER, IDS_KEY_REGIST, NULL);
		}
		return FALSE;
	}

	// システムステータス取得
	gnSysStat = GetRegValue(IDS_SEC_OPERATIONS, IDS_KEY_SYSSTATUS, gnSysStat);
	// 常駐ステータス取得
	gnTsrStat = GetRegValue(IDS_SEC_OPERATIONS, IDS_KEY_TSRSTATUS, gnTsrStat);

	// 常駐プロセスとして起動？
	if (!StrCmpNI(strCmdLine, _T("/tsr"), 4)) {
		// 常駐プロセスが存在するかどうか
		if (!otherTsr) {
			m_bTsr = true;
			return TRUE;
		}
		// 何もせずプロセス終了
		return FALSE;
	}

	// 起動モニターチェック - 開くウィンドウ位置のモニターを確認
	WINDOWPLACEMENT wpm;
	if (GetRegBinarySeq(IDS_SEC_SETTINGS, IDS_KEY_FRAMEPOS, GetWindowNumCount(APP_FRAMECLASS, NULL, FALSE), (BYTE*)&wpm, sizeof(wpm)) == sizeof(wpm)) {
		// 終了時位置が記録済みなら、その位置情報からモニターを検出
		m_hStartMonitor = MonitorFromRect(&wpm.rcNormalPosition, MONITOR_DEFAULTTONEAREST);
	}
	STARTUPINFO sui = { 0 };
	sui.cb = sizeof(sui);
	GetStartupInfo(&sui);
	if (sui.hStdOutput) {
		MONITORINFO mi = {0};
		mi.cbSize = sizeof(mi);
		if (GetMonitorInfo((HMONITOR)sui.hStdOutput, &mi)) {
			if (!(mi.dwFlags & MONITORINFOF_PRIMARY)) {
				// プライマリモニター以外のモニターのtaskbarあるいはjumplistから起動された場合は
				// そのモニター内で起動する
				m_hStartMonitor = (HMONITOR)sui.hStdOutput;
				m_bPrimaryMonitor = false;
			}
		}
	}

	// 強制的に起動
	if (!StrCmpNI(strCmdLine, _T("/force"), 6)) {
		strCmdLine.Delete(0, 6);
		strCmdLine.Trim();
	} else if (otherApp) {
		if (otherTsr || (gnSysStat & STS_SWITCHEXEC)) {
			// 常駐しているか、スイッチバックが有効の場合
			CWindow wnd = lccFindWindowByClassName(APP_FRAMECLASS);
			if (wnd.IsWindow()) {
				int nShow = 0;
				if (wnd.IsIconic())
					nShow = SW_RESTORE;
				else if (gnSysStat & STS_MAXIMIZE)
					nShow = SW_MAXIMIZE;
				if (nShow)
					wnd.GetLastActivePopup().ShowWindow(nShow);
				::SetForegroundWindow(wnd);

				if (!strCmdLine.IsEmpty()) {
					COPYDATASTRUCT cds;
					cds.dwData = CPD_UNKNOWN;
					if (strCmdLine[0] == _T('/'))
						cds.dwData = CPD_MACROW;
					else {
						DWORD dw = GetFileAttributes(strCmdLine);
						if (dw != 0xFFFFFFFF)
							cds.dwData = (dw & FILE_ATTRIBUTE_DIRECTORY) ? CPD_JUMP_PATHW : CPD_JUMP_FILEW;
					}

					if (cds.dwData != CPD_UNKNOWN) {
						cds.cbData = (strCmdLine.GetLength() + 1) * sizeof(TCHAR);
						cds.lpData = (void*)strCmdLine.GetString();
						wnd.SendMessage(WM_COPYDATA, 0, (LPARAM)&cds);
					}
				}
				return FALSE;
			}
		}
	} else {
		// スプラッシュロゴ（常駐時は表示しない）
		if (!::FindWindow(APP_TRAYCLASS, NULL) && !(gnSysStat & STS_NOSPLASH)) {
			CSplashWindow* pSplash = new CSplashWindow(IDR_SPLASH_IMAGE);
			pSplash->Create(GetDesktopWindow());
		}
	}

	// カスタムアイコン初期化
	LoadIconMap();

	// チップ情報を無視するファイル拡張子初期化
	InitOmitInfoTips();

	// システムイメージを確実のものにする
	SHFILEINFO sfi;
	SHGetFileInfo(_T("fv::.com"), 0, &sfi, sizeof(sfi), SHGFI_SYSICONINDEX | SHGFI_USEFILEATTRIBUTES | SHGFI_LARGEICON);

	if (gnSysStat & STS_MIN1ST && !otherApp && !otherTsr)
		nCmdShow = SW_SHOWMINIMIZED;
	else if (gnSysStat & STS_MAXIMIZE)
		nCmdShow = SW_SHOWMAXIMIZED;

	return TRUE;
}

void CFvApp::LoadStates()
{
	// ステータス取得
	gnBarStat = GetRegValue(IDS_SEC_OPERATIONS, IDS_KEY_BARSTATUS, gnBarStat);
	gnFopStat = GetRegValue(IDS_SEC_OPERATIONS, IDS_KEY_FOPSTATUS, gnFopStat);
	gnMosStat = GetRegValue(IDS_SEC_OPERATIONS, IDS_KEY_MOSSTATUS, gnMosStat);
	gnListStat = GetRegValue(IDS_SEC_OPERATIONS, IDS_KEY_LISTSTATUS, gnListStat);
	gnFileStat = GetRegValue(IDS_SEC_OPERATIONS, IDS_KEY_FILESTATUS, gnFileStat);
	gnTabStat = GetRegValue(IDS_SEC_OPERATIONS, IDS_KEY_TABSTATUS, gnTabStat);
	gnDrvStat = GetRegValue(IDS_SEC_OPERATIONS, IDS_KEY_DRVSTATUS, gnDrvStat);
	gnConfStat = GetRegValue(IDS_SEC_OPERATIONS, IDS_KEY_CONFSTATUS, gnConfStat);

	// 名前の省略値
	long val = GetRegValue(IDS_SEC_OPERATIONS, IDS_KEY_NAMEELLPSIS, MAKELONG(gnElmaxNameLength, gnElmaxExtLength));
	gnElmaxNameLength = (int)LOWORD(val);
	gnElmaxExtLength = (int)HIWORD(val);
}

void CFvApp::SaveStates()
{
	// ステータス保存
	SetRegValue(IDS_SEC_OPERATIONS, IDS_KEY_BARSTATUS, gnBarStat);
	SetRegValue(IDS_SEC_OPERATIONS, IDS_KEY_FOPSTATUS, gnFopStat);
	SetRegValue(IDS_SEC_OPERATIONS, IDS_KEY_MOSSTATUS, gnMosStat);
	SetRegValue(IDS_SEC_OPERATIONS, IDS_KEY_LISTSTATUS, gnListStat);
	SetRegValue(IDS_SEC_OPERATIONS, IDS_KEY_FILESTATUS, gnFileStat);
	SetRegValue(IDS_SEC_OPERATIONS, IDS_KEY_TABSTATUS, gnTabStat);
	SetRegValue(IDS_SEC_OPERATIONS, IDS_KEY_DRVSTATUS, gnDrvStat);
	SetRegValue(IDS_SEC_OPERATIONS, IDS_KEY_CONFSTATUS, gnConfStat);
	// 名前の省略値
	SetRegValue(IDS_SEC_OPERATIONS, IDS_KEY_NAMEELLPSIS, MAKELONG(gnElmaxNameLength, gnElmaxExtLength));
	// gnSysStat と gnTsrStat は保存しない
}

BOOL CFvApp::InitInstance()
{
	InitShell();
	LoadStates();

	// ファイル名比較ルーチン
	_fCompareFileName = (gnListStat & LTS_LOGICALCMP) ? StrCmpLogicalW : StrCmpI;

	// エクスプローラのフォルダオプション設定
//@ --- 環境設定の表示タブで設定
//@	ValidateExplorerHiddenSetting();

	// キーマップの初期化
	LoadKeyMap();

	// 配色の初期化
	InitColor();

	// ホットキーメニューの初期化
	menuHotKey.Restore();

	return TRUE;
}

void CFvApp::ExitInstance()
{
	// ステータス保存
	SaveStates();

	if (m_hLocalAccel) {
		DestroyAcceleratorTable(m_hLocalAccel);
		m_hLocalAccel = NULL;
	}
}

static const TCHAR exKey[] = _T("Software\\Microsoft\\Windows\\CurrentVersion\\Explorer\\Advanced");
static const TCHAR exHidden[] = _T("Hidden");
static const TCHAR exSupHidden[] = _T("ShowSuperHidden");

BOOL CFvApp::ValidateExplorerHiddenSetting()
{
#if 0		// 環境設定の表示タブで設定
	if (!(gnFileStat & FTS_HIDDEN)) {
		CRegKey reg;
		if (reg.Open(HKEY_CURRENT_USER, exKey, (KEY_READ | KEY_WRITE) & ~KEY_NOTIFY) == ERROR_SUCCESS) {
			BOOL bUpdate = FALSE;
			DWORD val, type, len;

			// 隠しファイルの表示可否
			if (reg.QueryDWORDValue(exHidden, val) == ERROR_SUCCESS) {
				if (val != 1) {
					val = 1;
					reg.SetDWORDValue(exHidden, val);
					bUpdate = TRUE;
				}
			}

/* @@@@ やっぱりまずいっすかねぇ
			// 保護されたオペレーティングシステムファイルの表示可否
			if (reg.QueryDWORDValue(exSupHidden, val) == ERROR_SUCCESS) {
				if (!val) {
					val = 1;
					reg.SetDWORDValue(exSupHidden, val);
					bUpdate = TRUE;
				}
			}
@@@@ */
			reg.Close();

			if (bUpdate)
				::SendMessage(HWND_BROADCAST, WM_SETTINGCHANGE, 0, 0);
			return bUpdate;
		}
	}
#endif		// 環境設定の表示タブで設定
	return FALSE;
}

void CFvApp::InitColor()
{
	COLORREF colors[colEndValue];
	int size = GetRegBinary(IDS_SEC_SETTINGS, IDS_KEY_COLORMAP, (BYTE*)colors, sizeof(colors));
	if (size == sizeof(colors))
		gColors.Set(colors, colEndValue);
	else
		gColors.Reset();
	m_brBkgnd.CreateSolidBrush(gColors[colBack]);
}

void CFvApp::SaveColor()
{
	if (!m_brBkgnd.IsNull()) m_brBkgnd.DeleteObject();

	m_brBkgnd.CreateSolidBrush(gColors[colBack]);
	SetRegBinary(IDS_SEC_SETTINGS, IDS_KEY_COLORMAP, (BYTE*)(COLORREF*)gColors, gColors.GetSize() * sizeof(COLORREF));
	// アプリ共通設定として記録
	lccRegSetBinary(_T("CommonSetting"), _T("Colors"), _T("ColorMap"), (BYTE*)(COLORREF*)gColors, gColors.GetSize() * sizeof(COLORREF));
}

void CFvApp::LoadKeyMap()
{
	if (GetRegBinary(IDS_SEC_SETTINGS, IDS_KEY_KEYMAP, (LPBYTE)keymapAbc, sizeof(keymapAbc)) < sizeof(keymapAbc))
		memcpy(keymapAbc, keymapDefAbc, sizeof(keymapAbc));
	if (GetRegBinary(IDS_SEC_SETTINGS, IDS_KEY_NUMKEYMAP, (LPBYTE)keymapNum, sizeof(keymapNum)) < sizeof(keymapNum))
		memcpy(keymapNum, keymapDefNum, sizeof(keymapNum));
	if (GetRegBinary(IDS_SEC_SETTINGS, IDS_KEY_FNKEYMAP, (LPBYTE)keymapFnc, sizeof(keymapFnc)) < sizeof(keymapFnc))
		memcpy(keymapFnc, keymapDefFnc, sizeof(keymapFnc));
	MakeLocalAccelTable();
}

void CFvApp::SaveKeyMap()
{
	SetRegBinary(IDS_SEC_SETTINGS, IDS_KEY_KEYMAP, (LPBYTE)keymapAbc, sizeof(keymapAbc));
	SetRegBinary(IDS_SEC_SETTINGS, IDS_KEY_NUMKEYMAP, (LPBYTE)keymapNum, sizeof(keymapNum));
	SetRegBinary(IDS_SEC_SETTINGS, IDS_KEY_FNKEYMAP, (LPBYTE)keymapFnc, sizeof(keymapFnc));
}

void CFvApp::MakeLocalAccelTable()
{
	// 機能キーのアクセラレータテーブルを作成
	if (m_hLocalAccel) {
		DestroyAcceleratorTable(m_hLocalAccel);
		m_hLocalAccel = NULL;
	}

	CSimpleArray<ACCEL> array;
	ACCEL acc;

	// Ctrl + A,B,C...
	int i;
	for (i = 0; i < 26; i++) {
		if (keymapAbc[i]) {
			acc.fVirt = FVIRTKEY;
			acc.key = (WORD)('A' + i);
			acc.cmd = (WORD)keymapAbc[i];
			array.Add(acc);
		}
		if (keymapAbc[i + 26]) {			// control
			acc.fVirt = FVIRTKEY | FCONTROL;
			acc.key = (WORD)('A' + i);
			acc.cmd = (WORD)keymapAbc[i + 26];
			array.Add(acc);
		}
		if (keymapAbc[i + 52]) {			// shift + control
			acc.fVirt = FVIRTKEY | FSHIFT | FCONTROL;
			acc.key = (WORD)('A' + i);
			acc.cmd = (WORD)keymapAbc[i + 52];
			array.Add(acc);
		}
	}

	// 1, 2, ... NUMPAD と 通常の数字キーの双方を有効にする
	for (i = 0; i < 10; i++) {
		if (keymapNum[i]) {
			acc.fVirt = FVIRTKEY;
			acc.key = (WORD)(VK_NUMPAD0 + i);
			acc.cmd = (WORD)keymapNum[i];
			array.Add(acc);

			acc.fVirt = FVIRTKEY;
			acc.key = (WORD)('0' + i);
			acc.cmd = (WORD)keymapNum[i];
			array.Add(acc);
		}
		if (keymapNum[i + 10]) {
			acc.fVirt = FVIRTKEY | FCONTROL;
			acc.key = (WORD)(VK_NUMPAD0 + i);
			acc.cmd = (WORD)keymapNum[i + 10];
			array.Add(acc);

			acc.fVirt = FVIRTKEY | FCONTROL;
			acc.key = (WORD)('0' + i);
			acc.cmd = (WORD)keymapNum[i + 10];
			array.Add(acc);
		}
	}

	// Fxx ...
	for (i = 0; i < 12; i++) {
		if (keymapFnc[i]) {
			acc.fVirt = FVIRTKEY;
			acc.key = (WORD)(VK_F1 + i);
			acc.cmd = (WORD)keymapFnc[i];
			array.Add(acc);
		}
		if (keymapFnc[i + 12]) {			// shift
			acc.fVirt = FVIRTKEY | FSHIFT;
			acc.key = (WORD)(VK_F1 + i);
			acc.cmd = (WORD)keymapFnc[i + 12];
			array.Add(acc);
		}
		if (keymapFnc[i + 24]) {			// control
			acc.fVirt = FVIRTKEY | FCONTROL;
			acc.key = (WORD)(VK_F1 + i);
			acc.cmd = (WORD)keymapFnc[i + 24];
			array.Add(acc);
		}
		if (keymapFnc[i + 36]) {			// shift + control
			acc.fVirt = FVIRTKEY | FSHIFT | FCONTROL;
			acc.key = (WORD)(VK_F1 + i);
			acc.cmd = (WORD)keymapFnc[i + 36];
			array.Add(acc);
		}
	}

	// ホットキー Ctrl+Alt+アルファベット
	TCHAR key[2];
	CHotKeyItem* pKeys[26];
	memset(pKeys, 0, sizeof(pKeys));
	if (RestoreHotKey(pKeys) > 0) {
		for (i = 0; i < 26; i++) {
			CHotKeyItem* pck = pKeys[i];
			if (pck) {
				key[0] = (TCHAR)('A' + pck->m_key);
				key[1] = 0;
				acc.fVirt = FVIRTKEY | FCONTROL | FALT;
				acc.key = (WORD)key[0];
				acc.cmd = (WORD)(IDM_HOTKEY_FIRST + i);
				array.Add(acc);
				delete pck;
			}
		}
	}
	int cnt = array.GetSize();
	if (cnt > 0)
		m_hLocalAccel = CreateAcceleratorTable(array.GetData(), cnt);
}
/////////////////////////////////////////////////////////////////////////////
// CFvApp commands


void CFvApp::ResetTSR()
{
	CWindow tray = ::FindWindow(APP_TRAYCLASS, NULL);
	if (tray.IsWindow())
		tray.SendMessage(nRM_TRAYSERVICE, 999, 0);

	if (gnTsrStat & TSS_TSR)
		ExecNewFrame((LPITEMIDLIST)NULL, _T("/tsr"));
}

void CFvApp::SetTsrStat(DWORD newStat)
{
	if (gnTsrStat != newStat) {
		SetRegValue(IDS_SEC_OPERATIONS, IDS_KEY_TSRSTATUS, newStat);
		gnTsrStat = newStat;
		ResetTSR();
	}
}

void CFvApp::SetSysStat(DWORD newStat)
{
	if (gnSysStat != newStat) {
		SetRegValue(IDS_SEC_OPERATIONS, IDS_KEY_SYSSTATUS, newStat);
		gnSysStat = newStat;
	}
}

void CFvApp::SetFopStat(DWORD dwStat)
{
	if (gnFopStat != dwStat) {
		SetRegValue(IDS_SEC_OPERATIONS, IDS_KEY_FOPSTATUS, dwStat);
		gnFopStat = dwStat;
	}
}

void CFvApp::SetMosStat(DWORD dwStat)
{
	if (gnMosStat != dwStat) {
		SetRegValue(IDS_SEC_OPERATIONS, IDS_KEY_MOSSTATUS, dwStat);
		gnMosStat = dwStat;
	}
}

void CFvApp::SetConfStat(DWORD dwStat)
{
	if (gnConfStat != dwStat) {
		SetRegValue(IDS_SEC_OPERATIONS, IDS_KEY_CONFSTATUS, dwStat);
		gnConfStat = dwStat;
	}
}

void CFvApp::ClearIconMap()
{
	CString key;
	CIconMap* pim;
	int cnt = (int)m_iconmap.GetSize();
	for (int i = 0; i < cnt; i++) {
		pim = m_iconmap.GetValueAt(i);
		if (pim) delete pim;
	}
	m_iconmap.RemoveAll();
}

void CFvApp::LoadIconMap()
{
	ClearIconMap();

	CSimpleArray<CIconMap*> arr;
	int cnt = RestoreMapIcon(arr);
	for (int i = 0; i < cnt; i++) {
		m_iconmap.Add(arr[i]->m_ext, arr[i]);
	}
}

int CFvApp::LookupIconMap(LPCTSTR pszName)
{
	// アイコンマップの処理
	int index = -1;
	if (pszName && *pszName) {
		LPTSTR p = PathFindExtension(pszName);
		if (*p == '.') {
			CString ext = ++p;
			ext.MakeUpper();
			CIconMap* pim = m_iconmap.Lookup(ext);
			if (pim) {
				index = Shell_GetCachedImageIndex(pim->m_file, pim->m_index, FALSE);
			}
		}
	}
	return index;
}

void CFvApp::InitOmitInfoTips()
{
	// ファイルオブジェクトのチップ情報を省略する拡張子の設定
	// （DivX等はチップ情報を取得するのに時間がかかりオーバーヘッドが大きい）
	m_omittipsmap.RemoveAll();

	CString str;
	if (GetRegText(IDS_SEC_SETTINGS, IDS_KEY_OMITTIPS, str) > 0) {
		str.MakeUpper();
		CSimpleArray<CString> ext;
		SeparateString(str, _T(';'), ext);
		int index = 1;
		for (int i = 0; i < ext.GetSize(); i++) {
			LPCTSTR p = ext[i];
			if (*p == _T('.')) p++;
			if (*p && !m_omittipsmap.Lookup(p))
				m_omittipsmap.Add(p, index++);
		}
	}
}

bool CFvApp::IsFileOmitInfoTips(LPCTSTR pszName)
{
	if (pszName && *pszName) {
		LPTSTR p = PathFindExtension(pszName);
		if (*p == '.') {
			CString ext = ++p;
			ext.MakeUpper();
			return (m_omittipsmap.Lookup(ext) != 0);
		}
	}
	return false;
}

void CFvApp::MakeHelpUrl(LPCTSTR pszTitle, LPCTSTR pszTopic, LPTSTR pszUrl, int cchMax)
{
	// VISTAではアプリケーション本体のフォルダには書き込みできないので、ユーザのデータフォルダに作成する
	CString path;
	GetAppDataFolder(path, pszTitle);
	path += _T(".url");
	if (!PathFileExists(path)) {
		// ファイルが存在しない場合のみ作成
		CString str;
		str.Format(_T("mk:@MSITStore:%s::%s"), m_Help.GetHelpPath(), pszTopic);
		::WritePrivateProfileString(_T("InternetShortcut"), _T("URL"), str, path);
	}
	StrCpyN(pszUrl, path, cchMax);
}

void CFvApp::MakeHelpLink(LPCTSTR pszTitle, LPCTSTR pszTopic, LPTSTR pszLink, int cchMax)
{
	CString path;
	GetAppDataFolder(path, pszTitle);
	path += _T(".lnk");
	if (!PathFileExists(path)) {
		CComPtr<IShellLink> psl;
		HRESULT hr = CoCreateInstance(CLSID_ShellLink, NULL, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&psl));
		if (SUCCEEDED(hr)) {
			psl->SetPath(_T("hh.exe"));
			psl->SetDescription(pszTitle);

			TCHAR buf[MAX_PATH * 2];
			PathGetParent(m_Help.GetHelpPath(), buf);
			psl->SetWorkingDirectory(buf);

			GetShortPathName(m_Help.GetHelpPath(), buf, ARRAYSIZE(buf));
			CString str;
			str.Format(_T("mk:@MSITStore:%s::%s"), buf, pszTopic);
			psl->SetArguments(str.GetString());

			CComQIPtr<IPersistFile> ppf = psl;
			if (ppf.p != NULL) {
				ppf->Save(path, TRUE);
			}
		}
	}
	StrCpyN(pszLink, path, cchMax);
}

COLORREF GetAttribColor(DWORD attr, bool bSelected, bool bMarked)
{
	COLORREF clr = (bSelected ? gColors[colSelBack] : (bMarked ? gColors[colMarkBack] : gColors[colNorm]));
	if (!bMarked) {
		if      (attr & FAT_SYSTEM)		clr = gColors[colSystem];
		else if (attr & FAT_HIDDEN)		clr = gColors[colHidden];
		else if (attr & FAT_RDONLY)		clr = gColors[colRdonly];
		else if (attr & FAT_CMPRSD)		clr = gColors[colCmprs];
		else if (attr & FAT_ENCRYPT)	clr = gColors[colEncr];
		else if (attr & FAT_REPP)		clr = gColors[colRepp];
	}
	return clr;
}


///////////////////////////////////////////////////////////////////
//

#ifdef __USE_CREATETHREAD__
DWORD CFvApp::AddOperationThread(LPTHREAD_START_ROUTINE pfnThread, void* param)
{
	DWORD nThreadID;
	HANDLE hThread = CreateThread(NULL, 0, pfnThread, param, 0, &nThreadID);
#else
unsigned CFvApp::AddOperationThread(LPBEGINTHREADEX_START_ROUTINE pfnThread, void* param)
{
	unsigned nThreadID;
	HANDLE hThread = (HANDLE)_beginthreadex(NULL, 0, pfnThread, param, 0, &nThreadID);
#endif
	if (hThread != NULL) {
		m_arrThreadHandles.Add(hThread);
		return nThreadID;
	}
	return 0;
}

void CFvApp::RemoveOperationThread(int index)
{
	int cnt = m_arrThreadHandles.GetSize();
	if (cnt > 0 && index >= 0 && index < cnt) {
		::CloseHandle(m_arrThreadHandles[index]);
		m_arrThreadHandles.RemoveAt(index);
	}
}

void CFvApp::OnIdleOperationThreadManager()
{
	// CMainFrame::OnIdle() より呼び出される

	int cnt = m_arrThreadHandles.GetSize();
	while (cnt > 0) {
		DWORD ret = ::WaitForMultipleObjects(cnt, m_arrThreadHandles.GetData(), FALSE, 0);
		if (WAIT_OBJECT_0 <= ret && ret < WAIT_OBJECT_0 + cnt) {
			RemoveOperationThread(ret - WAIT_OBJECT_0);
			cnt = m_arrThreadHandles.GetSize();
		}
		else
			break;
	}
}

int CFvApp::GetOperationThreadCount()
{
	return m_arrThreadHandles.GetSize();
}

///////////////////////////////////////////////////////////////////
//

int WINAPI _tWinMain(HINSTANCE hInstance, HINSTANCE /*hPrevInstance*/, LPTSTR lpstrCmdLine, int nCmdShow)
{
	HRESULT hRes;
	hRes = ::CoInitialize(NULL);
	ATLASSERT(SUCCEEDED(hRes));

	// this resolves ATL window thunking problem when Microsoft Layer for Unicode (MSLU) is used
	::DefWindowProc(NULL, 0, 0, 0L);

	AtlInitCommonControls(ICC_COOL_CLASSES | ICC_BAR_CLASSES);	// add flags to support other controls

	hRes = _Module.Init(NULL, hInstance);
	ATLASSERT(SUCCEEDED(hRes));

	// インストール後の最初の起動のチェックと処理(convet_setting.cpp)
	CheckAndConvertReg();
	int nRet = fvApp.Run(lpstrCmdLine, nCmdShow);

	_Module.Term();
	::CoUninitialize();

#ifdef _DEBUG
	ATLASSERT(__gThreadCount == 0);
#endif
	return nRet;
}
