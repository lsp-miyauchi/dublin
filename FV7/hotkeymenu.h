////////////////////////////////////////////////////////////////
// HOTKEYMENU.H

#pragma once
#ifndef __HOTKEYMENU_H__
#define __HOTKEYMENU_H__

#include "pagedialog.h"

#define CTLS_CHASE		0x0001
#define CTLS_SELCLEAR	0x0002

////////////////////////////////////////////////////////////////
//

enum {
	HOTKEY_UNKNOWN = 0,
	HOTKEY_EXEC,
	HOTKEY_FOLDER,
	HOTKEY_SORT,
	HOTKEY_FILTER,
};

class CHotKeyItem
{
public:
	int m_key;
	int m_type;
	bool m_bEdit;
	bool m_bAsAdmin;
	CString m_name;
	CString m_exec;
	CString m_param;
	CString m_workdir;

	CHotKeyItem() : m_key(-1), m_type(HOTKEY_UNKNOWN), m_bEdit(false), m_bAsAdmin(false)
	{
	}

	CHotKeyItem& CopyFrom(const CHotKeyItem& src)
	{
		if (&src != this) {
			m_key = src.m_key;
			m_type = src.m_type;
			m_bEdit = src.m_bEdit;
			m_bAsAdmin = src.m_bAsAdmin;
			m_name = src.m_name.GetString();
			m_exec = src.m_exec.GetString();
			m_param = src.m_param.GetString();
			m_workdir = src.m_workdir.GetString();
		}
		return *this;
	}

	inline CHotKeyItem& operator=(const CHotKeyItem& src)
	{
		return CopyFrom(src);
	}

	inline bool isNull() { return (m_key < 0); }

	void Clear()
	{
		m_key = -1;
		m_type = HOTKEY_UNKNOWN;
		m_bEdit = false;
		m_bAsAdmin = false;
		m_name.Empty();
		m_exec.Empty();
		m_param.Empty();
		m_workdir.Empty();
	}

	bool Initialize(int id, LPCTSTR pszCommand)
	{
		Clear();
		m_key = id;
		if (!pszCommand || *pszCommand == 0) return false;

		CString line = pszCommand;
		CSimpleArray<CString> arr;
		LPTSTR p = (LPTSTR)line.GetString();
		while (p) {
			LPTSTR q = StrChr(p, _T('^'));
			if (q)
				*q++ = 0;
			arr.Add(p);
			p = q;
		}
		int cnt = arr.GetSize();
		if (cnt < 3) return false;

		// タイプ取得
		m_type = StrToInt(arr[0]);
		// タイトル取得
		m_name = arr[1];
		// プログラム名取得
		p = (LPTSTR)arr[2].GetString();
		while (1) {
			if      (*p == '@') m_bEdit = true;
			else if (*p == '#') m_bAsAdmin = true;
			else break;
			p++;
		}
		m_exec = p;
		// 引数取得
		if (cnt > 3) m_param = arr[3];
		// 実行ディレクトリ取得
		if (cnt > 4) m_workdir = arr[4];
		return true;
	}

	void BuildCommandString(CString& str)
	{
		str.Empty();
		if (m_name.IsEmpty() || m_exec.IsEmpty()) return;

		str.Format(_T("%d^%s^"), m_type, m_name.GetString());
		if (m_bEdit)	str += '@';
		if (m_bAsAdmin)	str += '#';
		if (m_exec[0] == '"' && m_exec.Find(' ') >= 0) {
			m_exec.Trim(' ');
			m_exec.Insert(0, '"');
			m_exec += '"';
		}
		str += m_exec;
		str += '^';
		if (!m_param.IsEmpty()) str += m_param;
		str += '^';
		if (!m_workdir.IsEmpty()) str += m_workdir;
	}
};

////////////////////////////////////////////////////////////////
// class CHotKeyMenu

class CHotKeyMenu : public CMenu
{
public:
	int m_nEntries;
protected:
	CHotKeyItem* m_pKey[26];

public:
	CHotKeyMenu() : m_nEntries(0)
	{
		memset(m_pKey, 0, sizeof(m_pKey));
	}
	virtual ~CHotKeyMenu() { Clear(); }

// Attributes
public:
	BOOL IsMenuEnabled(){ return (m_nEntries > 0); }
	int GetPositionFromChar(UINT nChar);

// Operation
public:
	void Clear()
	{
		for (int i = 0; i < 26; i++) {
			CHotKeyItem* pck = m_pKey[i];
			if (pck)
				delete pck;
		}
		memset(m_pKey, 0, sizeof(m_pKey));
		m_nEntries = 0;
	}
	bool Save(LPCTSTR pszHotKeyFileName);
	bool Load(LPCTSTR pszHotKeyFileName);
	BOOL Restore();
	BOOL GetCommandLine(UINT nMenuID, CHotKeyItem& item);
	int DoTrack(HWND hWnd, CPoint pt);
};

////////////////////////////////////////////////////////////////////////
// CHotKeyTrackWnd

class CHotKeyTrackWnd : public CWindowImpl<CHotKeyTrackWnd>
{
private:
	CHotKeyMenu* m_pm;
public:

	CHotKeyTrackWnd()
	{
		m_pm = NULL;
	}

	~CHotKeyTrackWnd()
	{
	}

BEGIN_MSG_MAP(CHotKeyTrackWnd)
//	ON_WM_INITMENUPOPUP()
	ON_WM_MENUCHAR()
END_MSG_MAP()

	void OnInitMenuPopup(HMENU hPopupMenu, UINT nIndex, BOOL bSysMenu);
	LRESULT OnMenuChar(UINT nChar, UINT nFlags, HMENU hMenu);

	void DoTrack(HWND hwnd, CHotKeyMenu* pm, CPoint pt);
};

/////////////////////////////////////////////////////////////////////////////
// CHotKeyXXXXPage

template <class T>
class CHotKeyPageImpl
{
public:
	CHotKeyItem* m_itemptr;

public:
	CHotKeyPageImpl() : m_itemptr(NULL)
	{
	}

protected:

BEGIN_MSG_MAP(CHotKeyPageImpl)
	ON_WM_INITDIALOG()
	ON_BN_CLICKED(IDOK, OnClickedOK)
END_MSG_MAP()

	BOOL OnInitDialog(LPARAM lParam)
	{
		T* pT = static_cast<T*>(this);
		return pT->OnInit();
	}

	void OnClickedOK()
	{
		T* pT = static_cast<T*>(this);
		pT->OnOK();
	}

public:
	virtual BOOL OnInit()
	{
		return TRUE;
	}

	virtual void OnOK()
	{
	}
};


class CHotKeyExecPage : public CPropertyPageImpl<CHotKeyExecPage>, public CHotKeyPageImpl<CHotKeyExecPage>
{
public:
	enum { IDD = IDD_EDITHOTKEY_EXEC };

protected:
BEGIN_MSG_MAP(CHotKeyExecPage)
	CHAIN_MSG_MAP(CHotKeyPageImpl<CHotKeyExecPage>)
	ON_NOTIFY(SEN_GETTEXT, IDC_PROGRAM, OnNotifyExeBrowse)
	ON_NOTIFY(SEN_GETTEXT, IDC_DEFDIR, OnNotifyWorkingPathBrowse)
END_MSG_MAP()

	void OnNotifyExeBrowse(NMHDR* phdr, LRESULT* pResult);
	void OnNotifyWorkingPathBrowse(NMHDR* phdr, LRESULT* pResult);

public:
	BOOL OnInit();
	void OnOK();
};

class CHotKeyFolderPage : public CPropertyPageImpl<CHotKeyFolderPage>, public CHotKeyPageImpl<CHotKeyFolderPage>
{
public:
	enum { IDD = IDD_EDITHOTKEY_FOLDER };

protected:
BEGIN_MSG_MAP(CHotKeyFolderPage)
	CHAIN_MSG_MAP(CHotKeyPageImpl<CHotKeyFolderPage>)
	ON_NOTIFY(SEN_GETTEXT, IDC_FOLDER, OnNotifyPathBrowse)
END_MSG_MAP()

	void OnNotifyPathBrowse(NMHDR* phdr, LRESULT* pResult);

public:
	BOOL OnInit();
	void OnOK();
};

class CHotKeySortPage : public CPropertyPageImpl<CHotKeySortPage>, public CHotKeyPageImpl<CHotKeySortPage>
{
public:
	enum { IDD = IDD_EDITHOTKEY_SORT };

protected:
BEGIN_MSG_MAP(CHotKeySortPage)
	CHAIN_MSG_MAP(CHotKeyPageImpl<CHotKeySortPage>)
END_MSG_MAP()

public:
	BOOL OnInit();
	void OnOK();
};

class CHotKeyFilterPage : public CPropertyPageImpl<CHotKeyFilterPage>, public CHotKeyPageImpl<CHotKeyFilterPage>
{
public:
	enum { IDD = IDD_EDITHOTKEY_FILTER };

protected:
BEGIN_MSG_MAP(CHotKeyFilterPage)
	CHAIN_MSG_MAP(CHotKeyPageImpl<CHotKeyFilterPage>)
END_MSG_MAP()

public:
	BOOL OnInit();
	void OnOK();
};

/////////////////////////////////////////////////////////////////////////////
// CEditHotKeySheet

class CEditHotKeySheet : public CPropertySheetImpl<CEditHotKeySheet>
{
public:	
	CEditHotKeySheet(_U_STRINGorID title = (LPCTSTR)NULL, UINT uStartPage = 0, HWND hWndParent = NULL)
		: CPropertySheetImpl<CEditHotKeySheet>(title, uStartPage, hWndParent)
	{
	}

	BEGIN_MSG_MAP(CEditHotKeySheet)
		MESSAGE_HANDLER(WM_COMMAND, OnEHKCommand)
		CHAIN_MSG_MAP(CPropertySheetImpl<CEditHotKeySheet>)
	END_MSG_MAP()

	LRESULT OnEHKCommand(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
	{
		if (HIWORD(wParam) == BN_CLICKED && LOWORD(wParam) == IDOK) {
			CWindow child = GetActivePage();
			if (child.IsWindow())
				child.SendMessage(WM_COMMAND, MAKEWPARAM(IDOK, BN_CLICKED), NULL);
		}
		bHandled = FALSE;
		return 1;
	}
};

////////////////////////////////////////////////////////////////////////
//

void BuildHotKeyFileName(CString& str);
bool SaveHotKey(CHotKeyItem** pKeys, LPCTSTR pszHotKeyFileName);
int LoadHotKey(CHotKeyItem** pKeys, LPCTSTR pszHotKeyFileName);
int RestoreHotKey(CHotKeyItem** pKeys);


#endif				// __HOTKEYMENU_H__
