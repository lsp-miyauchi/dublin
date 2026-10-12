// fv7.h : main header file for the MT application
//

#pragma once
#ifndef __FILEVISOR7_BASE_HEADER__
#define __FILEVISOR7_BASE_HEADER__

#ifndef VER_PLATFORM_WIN32_WINDOWS
	#define VER_PLATFORM_WIN32_WINDOWS      1
#endif

#include "data.h"
#include "cdll.h"
#include "filtersetting.h"
#include "idlist.h"
#include "folderitem.h"
#include "fileitem.h"
#include "dragdrop.h"
#include "drive.h"
#include "resource.h"       // main symbols
#include "appbar.h"
#include "common.h"
#include "func.h"
#include "name.h"
#include "hotkeymenu.h"
#include "mapicon.h"
#include "quick.h"
#include "shortcut-list.h"
#include "finfogrep.h"
#include "browsefolder.h"

#include "cmwnd.h"

#define USE_HTMLHELP_COOKIE

/////////////////////////////////////////////////////////////////////////////
// HTMLHELP

typedef HWND (WINAPI HTMLHELPPROC)(HWND hwndCaller, LPCTSTR pszFile, UINT uCommand, DWORD_PTR dwData);

class CHtmlHelp
{
private:
	CString m_strHelpPath;
	HMODULE m_hmod;
	HTMLHELPPROC* m_pfnHtmlHelp;
#ifdef USE_HTMLHELP_COOKIE
	DWORD m_dwCookie;
#endif

public:
	CHtmlHelp()
	{
		m_strHelpPath.Empty();
		m_hmod = NULL;
		m_pfnHtmlHelp = NULL;
#ifdef USE_HTMLHELP_COOKIE
		m_dwCookie = 0;
#endif
	}
	~CHtmlHelp()
	{
		if (m_hmod) {
#ifdef USE_HTMLHELP_COOKIE
			if (m_dwCookie) {
				(*m_pfnHtmlHelp)(NULL, NULL, HH_CLOSE_ALL, 0);
				(*m_pfnHtmlHelp)(NULL, NULL, HH_UNINITIALIZE, m_dwCookie);	// cookie を渡す
			}
#endif
			FreeLibrary(m_hmod);
		}
	}
	void SetHelpPath(LPCTSTR pszHelpPath)
	{
		m_strHelpPath = pszHelpPath;
	}
	void ReadyToRun(HWND hwndCaller)
	{
		CWindow wnd = hwndCaller;

		// cancel any tracking modes
		wnd.SendMessage(WM_CANCELMODE);
		wnd.SendMessageToDescendants(WM_CANCELMODE, 0, 0, TRUE);

		// attempt to cancel capture
		wnd = ::GetCapture();
		if (wnd.IsWindow())
			wnd.SendMessage(WM_CANCELMODE, 0, 0);
	}
	bool Initialize()
	{
		ATLASSERT(!m_strHelpPath.IsEmpty());
		if (m_strHelpPath.IsEmpty()) return false;

		if (!m_hmod) {
			m_hmod = LoadLibrary(_T("hhctrl.ocx"));
			if (!m_hmod) return false;

			m_pfnHtmlHelp = (HTMLHELPPROC*)GetProcAddress(m_hmod, "HtmlHelpW");
			if (!m_pfnHtmlHelp) {
				FreeLibrary(m_hmod);
				m_hmod = NULL;
				return false;
			}
#ifdef USE_HTMLHELP_COOKIE
			(*m_pfnHtmlHelp)(NULL, NULL, HH_INITIALIZE, (DWORD_PTR)&m_dwCookie);
#endif
		}
		return true;
	}

	HWND HtmlHelp(HWND hwndCaller, UINT uCommand, DWORD dwData)
	{
		ReadyToRun(hwndCaller);
		if (!Initialize()) return NULL;

		return (*(m_pfnHtmlHelp))(hwndCaller, m_strHelpPath, uCommand, dwData);
	}
	HWND HtmlTopic(HWND hwndCaller, UINT uCommand, DWORD dwData)
	{
		ReadyToRun(hwndCaller);
		if (!Initialize()) return NULL;

		CString str = m_strHelpPath.GetString();
		str += _T(">topic");
		return (*(m_pfnHtmlHelp))(hwndCaller, str, uCommand, dwData);
	}
	LPCTSTR GetHelpPath() const
	{
		ATLASSERT(!m_strHelpPath.IsEmpty());
		return m_strHelpPath;
	}
};


/////////////////////////////////////////////////////////////////////////////
// CFvApp:
//

#define HB_DLG			HID_BASE_RESOURCE
#define HB_CMD			HID_BASE_COMMAND
#define HB_CTL			HID_BASE_CONTROL

class CFvApp
{
private:
	CString m_strMyModuleName;
	CSimpleMap<CString, CIconMap*> m_iconmap;
	CSimpleMap<CString, int> m_omittipsmap;
	bool m_bTsr:1;
	bool m_bAdmin:1;
	bool m_bPrimaryMonitor:1;
	HMONITOR m_hStartMonitor;

public:
	CString strCmdLine;
	int nCmdShow;
	CHotKeyMenu menuHotKey;
	UINT keymapAbc[78];
	UINT keymapNum[20];
	UINT keymapFnc[48];
	HACCEL m_hLocalAccel;
	CWindow wndMain;
	CWindow wndTray;
	CHtmlHelp m_Help;
	CBrush m_brBkgnd;
	CFont m_fontGui;
	long m_nProcessCounter;
	gdiplus_using gu;

	CSimpleArray<HANDLE> m_arrThreadHandles;

public:
	CFvApp();
	~CFvApp();

	BOOL Initialize();
	BOOL InitInstance();
	void ExitInstance();

	void LoadStates();
	void SaveStates();

	int Run(LPTSTR lpstrCmdLine = NULL, int nCmdShow = SW_SHOWDEFAULT);

// Attributes
public:
	BOOL ValidateExplorerHiddenSetting();
	void InitColor();
	void SaveColor();
	void LoadKeyMap();
	void SaveKeyMap();
	void MakeLocalAccelTable();

	void SetSysStat(DWORD nStat);
	void SetTsrStat(DWORD nStat);
	void SetFopStat(DWORD dwStat);
	void SetMosStat(DWORD dwStat);
	void SetConfStat(DWORD dwStat);

	inline void WinHelpFinder() { m_Help.HtmlHelp(wndMain, HH_HELP_FINDER, 0); }
	inline void Help(UINT idType, UINT idHelp) { m_Help.HtmlHelp(wndMain, HH_HELP_CONTEXT, idHelp + idType); }
	inline void HelpTopic(HWND hwnd, UINT idType, UINT idHelp) { m_Help.HtmlTopic(::IsWindow(hwnd) ? hwnd : wndMain, HH_HELP_CONTEXT, idHelp + idType); }

	// フレームウィンドウへの通知
	inline void ActivateNextView(BOOL bNextView)
	{
		ATLASSERT(::IsWindow(wndMain));
		::SendMessage(wndMain, WM_ACTIVATENEXTVIEW, (WPARAM)bNextView, 0);
	}
	inline BOOL OpenFolder(LPCITEMIDLIST pidl)
	{
		ATLASSERT(::IsWindow(wndMain));
		return (BOOL)::SendMessage(wndMain, WM_OPENFOLDER, 0, (LPARAM)pidl);
	}
	BOOL OpenFolder(LPCTSTR path)
	{
		CIDListPtr idlFq(path);
		if (!idlFq.isNull())
			return OpenFolder(idlFq);
		return FALSE;
	}
	BOOL OpenFolder(IShellFolder* psf, LPITEMIDLIST pidl)
	{
		if (psf) {
			CIDListPtr idlFq(psf, pidl);
			if (!idlFq.isNull())
				return OpenFolder(idlFq);
		}
		return FALSE;
	}
	inline void OpenTabFolder(IShellItem* psi, BOOL bActivate)
	{
		CIDListPtr idlFq(psi);
		if (!idlFq.isNull())
			OpenTabFolder(idlFq, bActivate);
	}
	inline void OpenTabFolder(LPCITEMIDLIST pidl, BOOL bActivate)
	{
		ATLASSERT(::IsWindow(wndMain));
		::SendMessage(wndMain, WM_OPENTABFOLDER, bActivate, (LPARAM)pidl);
	}
	void OpenTabFolder(LPCTSTR path, BOOL bActivate)
	{
		CIDListPtr idlFq;
		if (idlFq.CreateFromPath(path))
			OpenTabFolder(idlFq, bActivate);
	}
	void OpenTabFolder(IShellFolder* psf, LPCITEMIDLIST pidl, BOOL bActivate)
	{
		if (psf) {
			CIDListPtr idlFq(psf, pidl);
			if (!idlFq.isNull())
				OpenTabFolder(idlFq, bActivate);
		}
	}
	LPITEMIDLIST GetTabsFolderIDList(int index)
	{
		return (LPITEMIDLIST)::SendMessage(wndMain, WM_GETTABSFOLDERIDLIST, (WPARAM)index, 0);
	}
	inline BOOL OpenTabGroup(LPCTSTR pszTabGroup)
	{
		ATLASSERT(::IsWindow(wndMain));
		return (BOOL)::SendMessage(wndMain, WM_OPENTABGROUP, 0, (LPARAM)pszTabGroup);
	}
	inline void AddFavorite(LPCITEMIDLIST pidl)
	{
		ATLASSERT(::IsWindow(wndMain));
		::SendMessage(wndMain, WM_ADDFAVORITE, 0, (LPARAM)pidl);
	}
	inline void SetActiveView(HWND hwndView, bool bSetFocus = true)
	{
		ATLASSERT(::IsWindow(wndMain));
		::SendMessage(wndMain, WM_SETACTIVEVIEW, (WPARAM)bSetFocus, (LPARAM)hwndView);
	}
	inline void EnableAccel(bool bEnable)
	{
		ATLASSERT(::IsWindow(wndMain));
		::SendMessage(wndMain, WM_ENABLEACCEL, (WPARAM)bEnable, 0);
	}
	inline void ClearDocumentHistoryAll()
	{
		ATLASSERT(::IsWindow(wndMain));
		::SendMessage(wndMain, WM_CLEARFOLDERHISTORY, 0, 0);
	}

	void MakeHelpUrl(LPCTSTR pszTitle, LPCTSTR pszTopic, LPTSTR pszUrl, int cchMax);
	void MakeHelpLink(LPCTSTR pszTitle, LPCTSTR pszTopic, LPTSTR pszLink, int cchMax);
	LPCTSTR GetMyModuleName();

	// カスタムアイコン
	void ClearIconMap();
	void LoadIconMap();
	int LookupIconMap(LPCTSTR pszName);

	// チップ情報無効
	void InitOmitInfoTips();
	bool IsFileOmitInfoTips(LPCTSTR pszName);

	inline HBRUSH GetBkgndBrush() { return (HBRUSH)m_brBkgnd; }
	inline HACCEL GetLocalAccelerator() { return m_hLocalAccel; }
	inline bool isAdmin() { return m_bAdmin; }
	inline bool isStartPrimaryMonitor() { return m_bPrimaryMonitor; }
	inline HMONITOR StartMonitorHandle() { return m_hStartMonitor; }

	// 常駐
	void ResetTSR();

	// スレッド管理
#ifdef __USE_CREATETHREAD__
	DWORD AddOperationThread(LPTHREAD_START_ROUTINE pfnThread, void* param);
#else
	unsigned AddOperationThread(LPBEGINTHREADEX_START_ROUTINE pfnThread, void* param);
#endif
	void RemoveOperationThread(int index);
	void OnIdleOperationThreadManager();
	int GetOperationThreadCount();
};

extern CFvApp fvApp;

//////////////////////////////////////////////////////////////////////
//

class CEnterProcessSection
{
private:
	long* m_ptrCounter;
public:
	CEnterProcessSection(long* pcounter) : m_ptrCounter(pcounter) { Enter(); }
	~CEnterProcessSection() { Leave(); }
	void Enter() { *m_ptrCounter = *m_ptrCounter + 1; }
	void Leave() { *m_ptrCounter = *m_ptrCounter - 1; }
};

//////////////////////////////////////////////////////////////////////
//


// 表示色
COLORREF GetAttribColor(DWORD attr, bool bSelected, bool bMarked = false);

// アーカイバ処理
int ArcIsSFX(HWND hWnd, LPCTSTR pszFile);
BOOL ArcOpen(HWND hWnd, LPCTSTR pszName);
BOOL ArcCompress(HWND hWnd, LPCTSTR lpPath, HDROP hdrop);
BOOL ArcPackAdd(HWND hWnd, LPCTSTR pArc, HDROP hDrop);
BOOL ArcExtract(HWND hWnd, LPCTSTR lpPath, HDROP hdrop);
BOOL ArcCheck(HWND hWnd, HDROP hdrop);

class CEzmem
{
private:
	void* m_pdata;
	size_t m_size;
public:

	CEzmem() { m_pdata = NULL; m_size = 0; }
	~CEzmem() { Free(); }
	void Free()
	{
		if (m_pdata) { free(m_pdata); m_pdata = NULL; }
		m_size = 0;
	}
	void* Alloc(size_t size)
	{
		Free();
		m_pdata = malloc(size);
		if (m_pdata) { m_size = size; }
		return m_pdata;
	}
	__inline bool IsNull() const { return (m_pdata == NULL); }
	__inline void* GetData() { return m_pdata; }
	__inline size_t GetSize() { return m_size; }
};

// レジストリ
void GetAppRootKeyName(CString& str);
bool GetRegApp(CRegKey& app, LPCTSTR pszApp = NULL);
bool GetRegSection(CRegKey& sec, LPCTSTR pszSection);
BOOL RegSectionExists(_U_STRINGorID sec);
BOOL CreateRegSection(_U_STRINGorID sec);
void DeleteRegSection(_U_STRINGorID sec, _U_STRINGorID key);
int GetRegText(_U_STRINGorID sec, _U_STRINGorID key, CString& str);
BOOL SetRegText(_U_STRINGorID sec, _U_STRINGorID key, LPCTSTR pszText);
int GetRegTextSeq(_U_STRINGorID sec, _U_STRINGorID key, CString& str, int nEntry);
BOOL SetRegTextSeq(_U_STRINGorID sec, _U_STRINGorID key, LPCTSTR pszText, int nEntry);
int GetRegValue(_U_STRINGorID sec, _U_STRINGorID key, int nDefValue);
BOOL SetRegValue(_U_STRINGorID sec, _U_STRINGorID key, int nValue);
BOOL DelRegValue(_U_STRINGorID sec, _U_STRINGorID key);
int GetRegBinary(_U_STRINGorID sec, _U_STRINGorID key, LPVOID lpData, int nLen);
LPITEMIDLIST GetRegPidl(_U_STRINGorID sec, _U_STRINGorID key);
int GetRegBinarySeq(_U_STRINGorID sec, _U_STRINGorID key, int nEntry, LPBYTE pd, int nLen);
LPBYTE GetRegBinaryEx(_U_STRINGorID sec, _U_STRINGorID key, LPINT lpnLen);
LPBYTE GetRegBinarySeqEx(_U_STRINGorID sec, _U_STRINGorID key, int nEntry, LPINT pnLen);
BOOL SetRegBinary(_U_STRINGorID sec, _U_STRINGorID key, LPVOID lpData, int nLen);
BOOL SetRegBinarySeq(_U_STRINGorID sec, _U_STRINGorID key, int nEntry, void* pd, int nLen);
IStream* RegOpenStream(_U_STRINGorID sec, _U_STRINGorID key, DWORD grfMode);
//@ void RegLoadComboHistory(HWND hCombo, _U_STRINGorID sec, _U_STRINGorID key, int nMaxEntry);
//@ BOOL RegSaveComboHistory(HWND hCombo, _U_STRINGorID sec, _U_STRINGorID key, int nMaxEntry, LPCTSTR pszString);
//@ void RegClearComboHistory(HWND hCombo, _U_STRINGorID sec, _U_STRINGorID key);
int RegLoadHistory(CSimpleArray<CString>& rArray, _U_STRINGorID sec, _U_STRINGorID key, int maxEntry);
void RegLoadHistoryEx(HWND hCombo, _U_STRINGorID sec, _U_STRINGorID key, int nMaxEntry);
BOOL RegSaveHistory(_U_STRINGorID sec, _U_STRINGorID key, int nMaxEntry, LPCTSTR pszString);
void RegClearHistoryEx(HWND hCombo, _U_STRINGorID sec, _U_STRINGorID key);

void RegLoadWindowPlacement(HWND hwnd, UINT id);
void RegSaveWindowPlacement(HWND hwnd, UINT id);
int RegKeyCombine(_U_STRINGorID sec, _U_STRINGorID key, CString& strKey);

int EnumRegValue(HKEY hkey, CSimpleArray<CString>& arr);
int GetRegEnumValue(_U_STRINGorID sec, CSimpleArray<CString>& arr);

BOOL RegSaveGuidOperation(const void* dataptr, int len, LPTSTR guidstr);
void* RegLoadGuidOperation(LPCTSTR guidstr, int* lenptr);

BOOL GetAppDataFolder(CString& strFolder, LPCTSTR pszAppend = NULL);
BOOL GetLocalAppDataFolder(CString& strFolder, LPCTSTR pszAppend = NULL);


#endif		// __FILEVISOR7_BASE_HEADER__
