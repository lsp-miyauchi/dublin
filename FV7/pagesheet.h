// CPageSheetクラス
//

#pragma once
#ifndef __PAGESHEET_H__
#define __PAGESHEET_H__

#define WM_ON_INIT				(WM_USER + 20)
#define WM_ON_OK				(WM_USER + 21)

//////////////////////////////////////////////////////////////////////
// CPageSheet

class CPageSheet
{
// Data member
protected:
	HWND m_hWnd;
	DWORD m_dwStatus;
	UINT m_idCommand;

// Construction
public:
	CPageSheet()
	{
		m_hWnd = NULL;
		m_dwStatus = 0;
		m_idCommand = 0;
	}

// Operations
	void Init(HWND hParent, int idCommand, HWND hwnd)
	{
		m_hWnd = hwnd;
		if (m_hWnd)
			m_idCommand = idCommand;
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

	UINT GetCommand()
	{
		return m_idCommand;
	}

	HWND GetHwnd()
	{
		return m_hWnd;
	}
};


//////////////////////////////////////////////////////////////////////
// CPageList

typedef struct tagPAGEITEM {
	UINT idCommand;
	DLGPROC pfnDlg;
} PAGEITEM;


class CPageList
{
private:
	CSimpleArray<CPageSheet*> m_tabs;
	CWindow m_parent;
	CTreeViewCtrl m_listtree;
	CRect m_rcArea;
	UINT m_idCurrent;

public:
	CPageList(HWND hTreeCtrl)
		: m_listtree(hTreeCtrl)
		, m_parent(::GetParent(hTreeCtrl))
		, m_idCurrent(0)
	{
	}

	~CPageList()
	{
		int cnt = m_tabs.GetSize();
		for (int i = 0; i < cnt; i++)
			delete m_tabs[i];
	}

// Operations
	void Initialize(RECT* prcArea)
	{
		// 展開先領域
		m_rcArea = *prcArea;
	}

	void AddPage(UINT idCommand, HWND hwnd)
	{
		// テキストの設定
		CString str;
		str.LoadString(idCommand);

		HTREEITEM hItem = m_listtree.InsertItem(str, TVI_ROOT, TVI_LAST);

		// 展開先領域
		HWND hwndAfter;
		if (cnt > 0)
			hwndAfter = m_tabs[cnt - 1]->GetHwnd();
		else
			hwndAfter = m_listtree.m_hWnd;

		// タブの追加
		CPageSheet* pTab = new CPageSheet();
		pTab->Init(m_parent, idCommand, hwnd);
		pTab->Show(SW_HIDE);
		SetClassLongPtr(pTab->GetHwnd(), GCLP_HBRBACKGROUND, NULL);
		SetWindowPos(pTab->GetHwnd(), hwndAfter, m_rcArea.left, m_rcArea.top, 0, 0, SWP_NOSIZE | SWP_NOZORDER);
		m_tabs.Add(pTab);
	}

	UINT IndexToCommand(int index)
	{
		TBBUTTON tbb;
		if (m_listtree.IsWindow() && m_listtree.GetButton(index, &tbb))
			return tbb.idCommand;
		return 0;
	}

	BOOL SetActivePage(UINT idCommand)
	{
		CPageSheet* pTab;
		if (m_idCurrent) {
			pTab = GetPage(m_idCurrent);
			if (pTab)
				pTab->Show(SW_HIDE);
		}
		if (m_listtree.IsWindow() && m_listtree.CheckButton(idCommand, TRUE)) {
			GetPage(idCommand)->Show(SW_SHOW);
			m_idCurrent = idCommand;
			return TRUE;
		}
		return FALSE;
	}

	UINT GetActivePage()
	{
		for (int i = 0; i < m_listtree.GetButtonCount(); i++) {
			UINT id = m_tabs[i]->GetCommand();
			if (m_listtree.IsWindow() && m_listtree.IsButtonChecked(id))
				return id;
		}
		return 0;
	}

	CPageSheet* GetPage(UINT idCommand)
	{
		int cnt = m_tabs.GetSize();
		int index = m_listtree.CommandToIndex(idCommand);
		if (index >= 0 && index < cnt)
			return m_tabs[index];
		return NULL;
	}

	DWORD GetPageStatus(UINT idCommand)
	{
		CPageSheet* pTab = GetPage(idCommand);
		if (pTab)
			return pTab->GetStatus();
		return 0;
	}

	void SetPageStatus(UINT idCommand, DWORD dwStatus)
	{
		CPageSheet* pTab = GetPage(idCommand);
		if (pTab)
			pTab->SetStatus(dwStatus);
	}

	inline BOOL OnOK(UINT idCommand) { return (BOOL)SendMessage(GetPage(idCommand)->GetHwnd(), WM_ON_OK, 0, 0); }

	CTreeViewCtrl GetTreeViewCtrl()
	{
		return m_listtree.m_hWnd;
	}
};


#endif			// __DLGTAB_H__
