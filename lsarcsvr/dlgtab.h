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
// Data member
protected:
	HWND m_hWnd;
	DWORD m_dwStatus;
	UINT m_idCommand;

// Construction
public:
	CDlgTab()
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
// CPageBar

typedef struct tagPAGEBARITEM {
	UINT idCommand;
	DLGPROC pfnDlg;
} PAGEBARITEM;


class CPageBar
{
private:
	CSimpleArray<CDlgTab*> m_tabs;
	CWindow m_par;
	CToolBarCtrl m_bar;
	CRect m_rcArea;
	UINT m_idCurrent;
	CImageList m_im;
	bool m_bVerticalBar:1;

public:
	CPageBar(HWND hToolBar)
		: m_bar(hToolBar)
		, m_par(::GetParent(hToolBar))
		, m_idCurrent(0)
		, m_bVerticalBar(false)
	{
	}

	~CPageBar()
	{
		int cnt = m_tabs.GetSize();
		for (int i = 0; i < cnt; i++)
			delete m_tabs[i];
		m_im.Destroy();
	}

// Operations
	void Initialize(LPRECT prcArea, HIMAGELIST him = NULL)
	{
		// 展開先領域
		m_rcArea = *prcArea;

		// ボタン幅設定
		CRect rc;
		m_bar.GetClientRect(&rc);
		m_bVerticalBar = (rc.Width() < rc.Height());
		if (m_bVerticalBar)
			m_bar.SetButtonWidth(rc.Width(), rc.Width());
		else
			m_bar.SetButtonWidth(rc.Height(), rc.Height());


		if (him) {
			m_im.Attach(him);
			m_bar.SetImageList(m_im);
		}
	}

	void AddPage(UINT idCommand, HWND hwnd)
	{
		// ボタンテキストの設定
		TCHAR buf[MAX_PATH];
		LoadString(ATL::_AtlBaseModule.GetResourceInstance(), idCommand, buf, ARRAYSIZE(buf));
		buf[lstrlen(buf) + 1] = 0;
		m_bar.AddStrings(buf);

		// ボタンの登録
		int cnt = m_tabs.GetSize();
		int cntImage = m_im.IsNull() ? 100 : m_im.GetImageCount();
		TBBUTTON tbb;
		memset(&tbb, 0, sizeof(tbb));
		tbb.iBitmap = cnt % cntImage;
		tbb.idCommand = idCommand;
		tbb.iString = cnt;
		tbb.fsState = TBSTATE_ENABLED | (m_bVerticalBar ? TBSTATE_WRAP : 0);
		tbb.fsStyle = TBSTYLE_CHECKGROUP;
		m_bar.AddButtons(1, &tbb);

		// 展開先領域
		HWND hwndAfter;
		if (cnt > 0)
			hwndAfter = m_tabs[cnt - 1]->GetHwnd();
		else
			hwndAfter = m_bar.m_hWnd;

		// タブの追加
		CDlgTab* pTab = new CDlgTab();
		pTab->Init(m_par, idCommand, hwnd);
		pTab->Show(SW_HIDE);
		SetClassLongPtr(pTab->GetHwnd(), GCLP_HBRBACKGROUND, NULL);
		SetWindowPos(pTab->GetHwnd(), hwndAfter, m_rcArea.left, m_rcArea.top, 0, 0, SWP_NOSIZE | SWP_NOZORDER);
		m_tabs.Add(pTab);
	}

	void RecalcLayout()
	{
		// バーを再配置
		int cnt = m_bar.GetButtonCount();
		CRect rcw;
		m_bar.GetItemRect(0, &rcw);
		CRect rc = rcw;
		m_bar.GetItemRect(cnt - 1, &rcw);
		rc.right = rcw.right;

		m_bar.GetWindowRect(&rcw);
		m_par.ScreenToClient(&rcw);

		int x, y;
		if (m_bVerticalBar) {
			x = rcw.left + (rcw.Width() - rc.Width()) / 2;
			y = rcw.top + 4;
		}
		else {
			x = rcw.left + 4;;
			y = rcw.top + (rcw.Height() - rc.Height()) / 2;
		}

		m_bar.SetWindowPos(NULL, x, y, rc.Width(), rc.Height(), SWP_NOACTIVATE | SWP_NOZORDER | SWP_NOCOPYBITS);
	}

	UINT IndexToCommand(int index)
	{
		TBBUTTON tbb;
		if (m_bar.IsWindow() && m_bar.GetButton(index, &tbb))
			return tbb.idCommand;
		return 0;
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
		for (int i = 0; i < m_bar.GetButtonCount(); i++) {
			UINT id = m_tabs[i]->GetCommand();
			if (m_bar.IsWindow() && m_bar.IsButtonChecked(id))
				return id;
		}
		return 0;
	}

	CDlgTab* GetPage(UINT idCommand)
	{
		int cnt = m_tabs.GetSize();
		int index = m_bar.CommandToIndex(idCommand);
		if (index >= 0 && index < cnt)
			return m_tabs[index];
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

	inline BOOL OnOK(UINT idCommand) { return (BOOL)SendMessage(GetPage(idCommand)->GetHwnd(), WM_ON_OK, 0, 0); }

	CToolBarCtrl GetToolBarCtrl()
	{
		return m_bar.m_hWnd;
	}
};


#endif			// __DLGTAB_H__
