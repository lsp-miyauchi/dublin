// CDlgTabクラス
//

#pragma once
#ifndef __DLGTAB_H__
#define __DLGTAB_H__

#define WM_ON_INIT				(WM_USER + 20)
#define WM_ON_OK				(WM_USER + 21)

//////////////////////////////////////////////////////////////////////
// CDlgTab


class CDlgTab
{
private:
	HWND m_hWnd;
	DWORD m_dwStatus;
	UINT m_idCommand;

public:
	CDlgTab()
	{
		m_hWnd = NULL;
		m_dwStatus = 0;
		m_idCommand = 0;
	}

// Operations
	BOOL Init(int idDialog, HWND hwnd)
	{
		m_idCommand = idDialog;
		m_hWnd = hwnd;
		return ::IsWindow(m_hWnd);
	}

	void Show(int nCmdShow)
	{
		if (::IsWindow(m_hWnd)) ::ShowWindow(m_hWnd, nCmdShow);
	}

	DWORD GetStatus()
	{
		return m_dwStatus;
	}

	void SetStatus(DWORD dwStatus)
	{
		m_dwStatus = dwStatus;
	}

	UINT GetCommandID()
	{
		return m_idCommand;
	}

	HWND GetHwnd()
	{
		return m_hWnd;
	}
};


//////////////////////////////////////////////////////////////////////
// CPageBar

typedef struct tagPAGEBARITEM {
	UINT idCommand;
	DLGPROC pfnDlg;
} PAGEBARITEM;


class CPageBar
{
private:
	CSimpleArray<CDlgTab*> m_array;
	CWindow m_parent;
	CRect m_rcArea;
	UINT m_idCurrent;
	bool m_bVerticalBar:1;
	CToolBarCtrl m_bar;
	CImageList m_img;

public:
	CPageBar()
		: m_idCurrent(0)
		, m_bVerticalBar(false)
	{
	}

	~CPageBar()
	{
		int cnt = m_array.GetSize();
		for (int i = 0; i < cnt; i++)
			delete m_array[i];
		m_array.RemoveAll();
		if (!m_img.IsNull()) {
			if (m_bar.IsWindow())
				m_bar.SetImageList(NULL);
			m_img.Destroy();
		}
	}

	void Initialize(HWND hParent, HWND hBar, RECT* prc)
	{
		m_parent = hParent;
		m_bar = hBar;

		// ボタン幅設定
		CRect rc;
		m_bar.GetClientRect(&rc);
		m_bVerticalBar = (rc.Width() < rc.Height());
		if (m_bVerticalBar)
			m_bar.SetButtonWidth(rc.Width(), rc.Width());
		else
			m_bar.SetButtonWidth(rc.Height(), rc.Height());

		// 展開先領域
		m_rcArea = *prc;
	}

	void AddPage(UINT idCommand, UINT idIcon, HWND hPage)
	{
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		HINSTANCE hInst = RESINST;
		HICON hic = (HICON)LoadImage(hInst, MAKEINTRESOURCE(idIcon), IMAGE_ICON, cxIcon, cyIcon, LR_DEFAULTCOLOR);
		if (m_img.IsNull()) {
			// イメージリスト設定
			m_img.Create(cxIcon, cyIcon, ILC_COLOR32 | ILC_MASK, 10, 1);
			m_bar.SetImageList(m_img);
		}
		int index = m_img.AddIcon(hic);
		::DeleteObject(hic);

		// ボタンテキストの設定
		TCHAR buf[MAX_PATH];
		LoadString(hInst, idCommand, buf, ARRAYSIZE(buf));
		buf[lstrlen(buf) + 1] = '\0';
		m_bar.AddStrings(buf);

		// ボタンの登録
		int cnt = m_array.GetSize();
		TBBUTTON tbb;
		memset(&tbb, 0, sizeof(tbb));
		tbb.iBitmap = index;
		tbb.idCommand = idCommand;
		tbb.iString = cnt;
		tbb.fsState = TBSTATE_ENABLED | TBSTATE_WRAP;
		tbb.fsStyle = TBSTYLE_CHECKGROUP;
		m_bar.AddButtons(1, &tbb);

		// 展開先領域
		HWND hwndAfter;
		if (cnt > 0)
			hwndAfter = m_array[cnt - 1]->GetHwnd();
		else
			hwndAfter = m_bar;

		// タブの追加
		CDlgTab* pTab = new CDlgTab();
		pTab->Init(idCommand, hPage);
		pTab->Show(SW_HIDE);
		SetClassLong(pTab->GetHwnd(), GCLP_HBRBACKGROUND, NULL);
		SetWindowPos(pTab->GetHwnd(), hwndAfter, m_rcArea.left, m_rcArea.top, 0, 0, SWP_NOSIZE | SWP_NOZORDER);
		m_array.Add(pTab);
	}

	void RecalcLayout()
	{
		int cnt = m_bar.GetButtonCount();
		CRect rcw;
		m_bar.GetItemRect(0, &rcw);
		CRect rc = rcw;
		m_bar.GetItemRect(cnt - 1, &rcw);
		if (m_bVerticalBar)
			rc.bottom = rcw.bottom;
		else
			rc.right = rcw.right;

		m_bar.GetWindowRect(&rcw);
		m_parent.ScreenToClient(&rcw);

		m_bar.SetWindowPos(
					NULL,
					rcw.left + (rcw.Width() - rc.Width()) / 2,
					m_bVerticalBar ? rcw.top : rcw.top + (rcw.Height() - rc.Height()) / 2,
					rc.Width(),
					rc.Height(),
					SWP_NOACTIVATE | SWP_NOZORDER | SWP_NOCOPYBITS);
	}

	BOOL SetActivePage(UINT idCommand)
	{
		CDlgTab* pTab;
		if (m_idCurrent) {
			pTab = GetPage(m_idCurrent);
			if (pTab)
				pTab->Show(SW_HIDE);
		}
		if (m_bar.IsWindow() && m_bar.CheckButton(idCommand, TRUE)) {
			GetPage(idCommand)->Show(SW_SHOW);
			m_idCurrent = idCommand;
			return TRUE;
		}
		return FALSE;
	}

	UINT GetActivePage()
	{
		for (int i = 0; i < GetPageCount(); i++) {
			UINT id = m_array[i]->GetCommandID();
			if (m_bar.IsWindow() && m_bar.IsButtonChecked(id))
				return id;
		}
		return 0;
	}

	CDlgTab* GetPage(UINT idCommand)
	{
		int index = CommandToIndex(idCommand);
		if (index >= 0 && index < m_array.GetSize())
			return m_array[index];
		return NULL;
	}

	DWORD GetPageStatus(UINT idCommand)
	{
		CDlgTab* pTab = GetPage(idCommand);
		if (pTab)
			return pTab->GetStatus();
		return 0;
	}

	void SetPageStatus(UINT idCommand, DWORD dwStatus)
	{
		CDlgTab* pTab = GetPage(idCommand);
		if (pTab)
			pTab->SetStatus(dwStatus);
	}

	inline void OnInit(UINT idCommand, LPARAM lData) { SendMessage(GetPage(idCommand)->GetHwnd(), WM_ON_INIT, 0, lData); }

	inline BOOL OnOK(UINT idCommand) { return (BOOL)SendMessage(GetPage(idCommand)->GetHwnd(), WM_ON_OK, 0, 0); }

	inline int GetPageCount() { return m_bar.GetButtonCount(); }

	inline int CommandToIndex(UINT idCommand) { return m_bar.CommandToIndex(idCommand); }

	UINT IndexToCommand(int index)
	{
		TBBUTTON tbb;
		if (m_bar.IsWindow() && m_bar.GetButton(index, &tbb))
			return tbb.idCommand;
		return 0;
	}
};



#endif			// __DLGTAB_H__
