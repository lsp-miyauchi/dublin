// trayfrm.h : interface of the CTrayFrame class
//
/////////////////////////////////////////////////////////////////////////////

#pragma once
#ifndef __TRAYFRM_H__
#define __TRAYFRM_H__

#define TIMERID_TRAY							10
#define ID_TRAY									100

int LoadTabReopenArray(CTabReopenArray& array, int maxEntry);
int EnumTabGroup(CCmItemArray& array);


///////////////////////////////////////////////////////////////////////
//

class CNotifyIconData : public NOTIFYICONDATA
{
public:
	CNotifyIconData()
	{
		memset(this, 0, sizeof(NOTIFYICONDATA));
		cbSize = sizeof(NOTIFYICONDATA);
	}
	~CNotifyIconData()
	{
		if (hIcon)
			DestroyIcon(hIcon);
	}
	void Initialize(HWND hwnd, UINT id)
	{
		hWnd = hwnd;
		uID = id;
	}
};

////////////////////////////////////////////////////////////////////////
//

template <class T>
class CTrayIconImpl
{
private:
	CNotifyIconData m_nid;
	UINT m_nRM_TRAYMESSAGE;
	BOOL m_bTrayActive;

public:
	CTrayIconImpl()
		: m_bTrayActive(false)
	{
	}

	BOOL StartTray(
				UINT uID,
				HICON hIcon = NULL,
				LPCTSTR pszTooltipText = NULL,
				LPCTSTR pszBalloonText = NULL,
				LPCTSTR pszBalloonCaption = NULL,
				UINT nTimeout = 10,
				DWORD dwBalloonStyle = NIIF_INFO,
				HICON hBalloonIcon = NULL)
	{
		T* pT = static_cast<T*>(this);

		// 受け取るcallback_messageの登録
		CString str;
		str.Format(_T("wm_ti%d"), GetTickCount());
		m_nRM_TRAYMESSAGE = RegisterWindowMessage(str);

		m_nid.hWnd = pT->operator HWND();
		m_nid.uID = uID;
		m_nid.uFlags = NIF_MESSAGE;
		m_nid.uCallbackMessage = m_nRM_TRAYMESSAGE;
		if (hIcon) {
			m_nid.uFlags |= NIF_ICON;
			m_nid.hIcon = hIcon;
		}
		if (pszTooltipText) {
			m_nid.uFlags |= NIF_TIP;
			lstrcpyn(m_nid.szTip, pszTooltipText, ARRAYSIZE(m_nid.szTip));
		}
		if (pszBalloonText && pszBalloonCaption) {
			m_nid.uFlags |= NIF_INFO | NIF_SHOWTIP;
			lstrcpyn(m_nid.szInfo, pszBalloonText, ARRAYSIZE(m_nid.szInfo));
			lstrcpyn(m_nid.szInfoTitle, pszBalloonCaption, ARRAYSIZE(m_nid.szInfoTitle));
			m_nid.dwInfoFlags = dwBalloonStyle;
			if (dwBalloonStyle == NIIF_USER)
				m_nid.hBalloonIcon = hBalloonIcon;
			m_nid.uTimeout = nTimeout;
		}
		m_bTrayActive = Shell_NotifyIcon(NIM_ADD, &m_nid);
		if (m_bTrayActive) {
			m_nid.uVersion = NOTIFYICON_VERSION_4;
			Shell_NotifyIcon(NIM_SETVERSION, &m_nid);
		}
		return m_bTrayActive;
	}

	void EndTray()
	{
		if (m_bTrayActive) {
			Shell_NotifyIcon(NIM_DELETE, &m_nid);
			m_bTrayActive = FALSE;
		}
	}

	BOOL SetTrayBalloon(
				LPCTSTR pszBalloonText,
				LPCTSTR pszBalloonCaption,
				DWORD dwBalloonStyle = NIIF_INFO,
				UINT nTimeout = 10,
				HICON hUserIcon = NULL,
				HICON hBalloonIcon = NULL)
	{
		if (!m_bTrayActive) return FALSE;

		m_nid.uFlags = NIF_INFO | NIF_SHOWTIP;
		lstrcpyn(m_nid.szInfo, pszBalloonText, ARRAYSIZE(m_nid.szInfo));
		lstrcpyn(m_nid.szInfoTitle, pszBalloonCaption, ARRAYSIZE(m_nid.szInfoTitle));
		m_nid.uTimeout = nTimeout;
		m_nid.dwInfoFlags = dwBalloonStyle;
		if (dwBalloonStyle == NIIF_USER) {
			if (hBalloonIcon)
				m_nid.hBalloonIcon = hBalloonIcon;
			else
				m_nid.hIcon = hUserIcon;
		}
		return Shell_NotifyIcon(NIM_MODIFY, &m_nid);
	}


BEGIN_MSG_MAP(CTrayIconImpl)
	MESSAGE_HANDLER(m_nRM_TRAYMESSAGE, OnHandlerTITrayMessage)
END_MSG_MAP()

	LRESULT OnHandlerTITrayMessage(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
	{
		T* pT = static_cast<T*>(this);

		UINT_PTR id;
		CPoint pt;
		UINT_PTR msg;
		// (トレイ作成時に uVersion に NOTIFYICON_VERSION_4 を指定した場合）
		msg = LOWORD(lParam);
		id = HIWORD(lParam);
		pt.x = GET_X_LPARAM(wParam);
		pt.y = GET_Y_LPARAM(wParam);

		if (m_nid.uID == id) {
			switch (msg) {
			case WM_LBUTTONDBLCLK:		pT->OnTrayIconLButtonDoubleClick(pt); break;
			case WM_LBUTTONDOWN:		pT->OnTrayIconLButtonDown(pt); 	break;
			case WM_LBUTTONUP:			pT->OnTrayIconLButtonUp(pt); break;
			case WM_RBUTTONDOWN:		pT->OnTrayIconRButtonDown(pt); 	break;
			case WM_RBUTTONUP:			pT->OnTrayIconRButtonUp(pt); break;
			case NIN_BALLOONUSERCLICK:	pT->OnTrayIconBaloonUserClick(pt); break;
			case NIN_BALLOONSHOW:		pT->OnTrayIconBaloonShow(); break;
			case NIN_BALLOONHIDE:		pT->OnTrayIconBaloonHide(); break;
			case NIN_BALLOONTIMEOUT:	pT->OnTrayIconBaloonTimeout(); break;
			case NIN_POPUPOPEN:			pT->OnTrayIconPopupOpen(); break;
			case NIN_POPUPCLOSE:		pT->OnTrayIconPopupClose(); break;
			default: break;
			}
		}
		return 0;
	}

	// overridable
	virtual void OnTrayIconLButtonDoubleClick(CPoint pt) { }
	virtual void OnTrayIconLButtonDown(CPoint pt) { }
	virtual void OnTrayIconLButtonUp(CPoint pt) { }
	virtual void OnTrayIconRButtonDown(CPoint pt) { }
	virtual void OnTrayIconRButtonUp(CPoint pt){ }
	virtual void OnTrayIconBaloonUserClick(CPoint pt) { }
	virtual void OnTrayIconBaloonShow() { }
	virtual void OnTrayIconBaloonHide() { }
	virtual void OnTrayIconBaloonTimeout() { }
	virtual void OnTrayIconPopupOpen() { }
	virtual void OnTrayIconPopupClose() { }
};


////////////////////////////////////////////////////////////////////////
//

class CTrayFrame
		: public CFrameWindowImpl<CTrayFrame>
		, public CTrayIconImpl<CTrayFrame>
		, public CUpdateUI<CTrayFrame>
		, public CMessageFilter
		, public CIdleHandler
{
private:
	int m_hotactive_id;
	int m_hotpopup_id;
	bool m_fLButtonDoubleClick;
	HIMAGELIST m_hSysImage;

public:
	CTrayFrame() : m_hotactive_id(0), m_hotpopup_id(0), m_fLButtonDoubleClick(false)
	{
	}

	~CTrayFrame()
	{
	}

	DECLARE_FRAME_WND_CLASS(APP_TRAYCLASS, 0)

	virtual BOOL PreTranslateMessage(MSG* pMsg)
	{
		return CFrameWindowImpl<CTrayFrame>::PreTranslateMessage(pMsg);
	}

	virtual BOOL OnIdle()
	{
		return FALSE;
	}

	BEGIN_UPDATE_UI_MAP(CTrayFrame)
	END_UPDATE_UI_MAP()

	BEGIN_MSG_MAP(CTrayFrame)
		CHAIN_MSG_MAP(CTrayIconImpl<CTrayFrame>)
		ON_WM_CREATE()
		ON_MESSAGE(WM_SHOWWINDOW, OnShowWindow)
		ON_WM_COMMAND()
		ON_WM_ENDSESSION()
		ON_WM_DESTROY()
		ON_WM_DRAWITEM()
		ON_WM_MEASUREITEM()
		ON_MESSAGE_VOID(WM_HOTKEY, OnHotKey)
		ON_REGISTERD_MESSAGE(nRM_TRAYSERVICE, OnTrayService)
		CHAIN_MSG_MAP(CUpdateUI<CTrayFrame>)
		CHAIN_MSG_MAP(CFrameWindowImpl<CTrayFrame>)
	END_MSG_MAP()

	int OnCreate(LPCREATESTRUCT pcs)
	{
		fvApp.wndTray = m_hWnd;

		HIMAGELIST himL;
		Shell_GetImageLists(&himL, &m_hSysImage);

		// register object for message filtering and idle updates
		CMessageLoop* pLoop = _Module.GetMessageLoop();
		ATLASSERT(pLoop != NULL);
		pLoop->AddMessageFilter(this);
		pLoop->AddIdleHandler(this);

		CString str; str.LoadString(IDR_MAINFRAME);
		HICON hIcon = (HICON)::LoadImage(RESINST, MAKEINTRESOURCE(IDR_MAINFRAME), IMAGE_ICON, 16, 16, LR_DEFAULTCOLOR);
		StartTray(ID_TRAY, hIcon, str);
		if (hIcon)
			DestroyIcon(hIcon);

		SetHotkey();
		return 0;
	}

	LRESULT OnShowWindow(WPARAM /*wParam*/, LPARAM /*lParam*/)
	{
		return 0;
	}

	void SetHotkey()
	{
		DWORD dw;
		WORD wModifiers, wVirtualKeyCode;
		if (!m_hotactive_id) {
			// アクティブ化ホットキー
			dw = GetRegValue(IDS_SEC_SETTINGS, IDS_KEY_TSRACTIVEKEY, 0);
			if (dw) {
				wModifiers = 0;
				if (HIBYTE(LOWORD(dw)) & HOTKEYF_ALT)		wModifiers |= MOD_ALT;
				if (HIBYTE(LOWORD(dw)) & HOTKEYF_SHIFT)		wModifiers |= MOD_SHIFT;
				if (HIBYTE(LOWORD(dw)) & HOTKEYF_CONTROL)	wModifiers |= MOD_CONTROL;
				wVirtualKeyCode = LOBYTE(LOWORD(dw));

				if (::RegisterHotKey(m_hWnd, (int)dw, wModifiers, wVirtualKeyCode))
					m_hotactive_id = (int)dw;
			}
		}
		if (!m_hotpopup_id) {
			// ポップアップホットキー
			dw = GetRegValue(IDS_SEC_SETTINGS, IDS_KEY_TSRPOPUPKEY, 0);
			if (dw) {
				wModifiers = 0;
				if (HIBYTE(LOWORD(dw)) & HOTKEYF_ALT)		wModifiers |= MOD_ALT;
				if (HIBYTE(LOWORD(dw)) & HOTKEYF_SHIFT)		wModifiers |= MOD_SHIFT;
				if (HIBYTE(LOWORD(dw)) & HOTKEYF_CONTROL)	wModifiers |= MOD_CONTROL;
				wVirtualKeyCode = LOBYTE(LOWORD(dw));

				if (::RegisterHotKey(m_hWnd, (int)dw, wModifiers, wVirtualKeyCode))
					m_hotpopup_id = (int)dw;
			}
		}
	}

	void CancelHotkey()
	{
		// ホットキーが登録されていれば
		if (m_hotactive_id) {
			::UnregisterHotKey(m_hWnd, m_hotactive_id);
			m_hotactive_id = 0;
		}
		if (m_hotpopup_id) {
			::UnregisterHotKey(m_hWnd, m_hotpopup_id);
			m_hotpopup_id = 0;
		}
	}

	void ExitSession()
	{
		CancelHotkey();
		EndTray();
	}

	void OnEndSession(BOOL bEnding)
	{
		ExitSession();
	}

	void OnDestroy()
	{
		ExitSession();
	}

	void OnHotKey(WPARAM wParam, LPARAM)
	{
		if ((int)wParam == m_hotactive_id) {
			OpenFolder();
		}
		else if ((int)wParam == m_hotpopup_id) {
			TrayMenu();
		}
	}

	void OnDrawItem(int /*idCtrl*/, LPDRAWITEMSTRUCT pdi)
	{
		if (pdi->CtlType != ODT_MENU) return;

		if (pdi->itemData) {
			CCmItem* pci = (CCmItem*)pdi->itemData;
			CRect rc = pdi->rcItem;
			CDCHandle dc = pdi->hDC;
			dc.SetBkMode(TRANSPARENT);

			COLORREF clr = ((pdi->itemState & ODS_SELECTED) != 0) ? GetSysColor(COLOR_HIGHLIGHT) : GetSysColor(COLOR_MENU);
			dc.FillSolidRect(&rc, clr);

			int index = pci->GetImageIndex();
			if (index >= 0)
				ImageList_Draw(m_hSysImage, index, dc, 2, (rc.top + rc.bottom - 16) / 2, ILD_TRANSPARENT);

			clr = ((pdi->itemState & ODS_SELECTED) != 0) ? GetSysColor(COLOR_HIGHLIGHTTEXT) : GetSysColor(COLOR_MENUTEXT);
			dc.SetTextColor(clr);
			rc.left += 22;
			rc.right -= 2;
			dc.DrawText(pci->m_name, -1, &rc, DT_LEFT | DT_SINGLELINE | DT_VCENTER | DT_NOPREFIX);
		}
	}

	void OnMeasureItem(int /*idCtrl*/, LPMEASUREITEMSTRUCT pmi)
	{
		if (pmi->CtlType != ODT_MENU) return;

		if (pmi->itemData) {
			pmi->itemWidth = _GCDATA.m_NcMetrics.iMenuWidth;
			pmi->itemHeight = _GCDATA.m_NcMetrics.iMenuHeight;

			CClientDC dc(NULL);
			HFONT hfont = dc.SelectFont(_GCDATA.GetMenuFont());
			TEXTMETRIC tm;
			dc.GetTextMetrics(&tm);

			CCmItem* pci = (CCmItem*)pmi->itemData;
			CSize csz;
			dc.GetTextExtent(pci->m_name, pci->m_name.GetLength(), &csz);

			pmi->itemWidth = csz.cx + 25;
			pmi->itemHeight = max(pmi->itemHeight, MENU_OWNERDRAW_HEIGHT);
			dc.SelectFont(hfont);
		}
	}

	BOOL OnCommand(UINT idCtrl, UINT code, HWND hwnd)
	{
/*
		switch (idCtrl) {
		}
*/
		return TRUE;
	}

	LRESULT OnTrayService(WPARAM wParam, LPARAM lParam)
	{
		switch (wParam) {
		case 100:
			SetHotkey();
			break;
		case 101:
			CancelHotkey();
			break;
		case 999:
			PostMessage(WM_CLOSE, 0, 0);
			break;
		}
		return 0;
	}

	// ------------------------------------------
	// CTrayIconImplからの派生
	// ------------------------------------------

	void OpenFolder(CCmItem* pci = NULL)
	{
		CWindow frame = ::FindWindow(APP_FRAMECLASS, NULL);

		// 他のアプリで更新されている可能性があるので読み直し
		DWORD stat = GetRegValue(IDS_SEC_OPERATIONS, IDS_KEY_TSRSTATUS, 0);
		if (!(stat & TSS_NEWWINDOW) && frame.IsWindow()) {
			if      (!frame.IsWindowVisible())		frame.ShowWindow(SW_SHOWNORMAL);
			else if (frame.IsIconic())				frame.ShowWindow(SW_RESTORE);

			if (::GetForegroundWindow() != frame)	::SetForegroundWindow(frame);

			if (pci) {
				CString str;
				pci->GetParseName(str);
				COPYDATASTRUCT cds;
				if (pci->isFileSystem() && !pci->isFolder()) {
					cds.dwData = CPD_EXEC_FILEW;
					cds.cbData = sizeof(TCHAR) * (str.GetLength() + 1);
					cds.lpData = (void*)str.GetString();
				}
				else {
					cds.dwData = CPD_JUMP_IDLIST;
					cds.dwData |= CPD_NEW_TAB;		// new tab
					cds.cbData = pci->m_idl.GetSize();
					cds.lpData = (void*)pci->m_idl.m_pidl;
				}
				frame.SendMessage(WM_COPYDATA, 0, (LPARAM)&cds);
			}
		}
		else {
			ExecNewFrame((pci != NULL) ? pci->m_idl.m_pidl : NULL);
		}
	}

	void OnTrayIconLButtonDoubleClick(CPoint pt)
	{
		m_fLButtonDoubleClick = true;
		OpenFolder();
	}

	void OnTrayIconLButtonDown(CPoint pt)
	{
		m_fLButtonDoubleClick = false;
	}

	void OnTrayIconLButtonUp(CPoint pt)
	{
		if (m_fLButtonDoubleClick)
			m_fLButtonDoubleClick = false;
		else
			TrayMenu();
	}

	void OnTrayIconRButtonUp(CPoint pt)
	{
		TraySysMenu();
	}

	void OnTrayIconBaloonUserClick(CPoint pt)
	{
	}

	void TrayMenu()
	{
		SetForegroundWindow(m_hWnd);

		CSimpleArray<HBITMAP> arrBitmaps;

		// カーソル位置取得
		CPoint pt;
		GetCursorPos(&pt);

		CCmItem* pci;
		CSimpleMap<UINT, CCmItem*> map;

		CWaitCursor wc;

		CMenu pop;
		pop.CreatePopupMenu();
		int id = IDM_TRAY_START;

		CString str;
		// ----------------------------------
		// ホームフォルダ
		// ----------------------------------
		CIDListPtr idlFq = GetRegPidl(IDS_SEC_FOLDER, IDS_KEY_HOME);
		if (!idlFq.isNull()) {
			pci = new CCmItem(idlFq);
			pci->m_name.LoadString(IDS_HOME_FOLDER);
			pop.AppendMenu(MF_STRING, id, pci->m_name);
			HBITMAP hbm = CreateBitmapFromImageList(m_hSysImage, pci->GetImageIndex());
			if (hbm) {
				SetBitmapToMenuItem(pop, id, hbm);
				arrBitmaps.Add(hbm);
			}
			map.Add(id++, pci);
			idlFq.Free();

			pop.AppendMenu(MF_SEPARATOR);			// セパレータ
		}

		// ----------------------------------
		// ドライブのリストアップ
		// ----------------------------------
		CComPtr<IShellFolder> psf = idl_GetSpecialFolder(CSIDL_DRIVES, NULL);
		if (psf.p != NULL) {
			CComPtr<IEnumIDList> pe;
			if (psf->EnumObjects(m_hWnd, SHCONTF_FOLDERS | SHCONTF_FASTITEMS, &pe) == S_OK) {
				LPITEMIDLIST pidl;
				while (pe->Next(1, &pidl, NULL) == S_OK) {
					idlFq = idl_GetFullyQualPidl(psf, pidl);
					if (!idlFq.isNull()) {
						// ドライブのみ
						CString name;
						if (idlFq.GetPath(name) && PathIsRoot(name)) {
							pci = new CCmItem(idlFq);
							pop.AppendMenu(MF_STRING, id, pci->m_name);
							HBITMAP hbm = CreateBitmapFromImageList(m_hSysImage, pci->GetImageIndex());
							if (hbm) {
								SetBitmapToMenuItem(pop, id, hbm);
								arrBitmaps.Add(hbm);
							}
							map.Add(id++, pci);
						}
						idlFq.Free();
					}
					CoTaskMemFree(pidl);
				}
			}
		}
		pop.AppendMenu(MF_SEPARATOR);			// セパレータ

		// ----------------------------------
		// クイックアクセスのリストアップ
		// ----------------------------------
		CMenu sub;
		CFavoriteItemArray favo;
		favo.Load();
		int i;
		int cnt = favo.GetSize();
		if (cnt > 0) {
			sub.CreatePopupMenu();
			for (i = 0; i < cnt; i++) {
				pci = favo[i];
				sub.AppendMenu(MF_STRING, id, pci->m_name);
				HBITMAP hbm = CreateBitmapFromImageList(m_hSysImage, pci->GetImageIndex());
				if (hbm) {
					SetBitmapToMenuItem(sub, id, hbm);
					arrBitmaps.Add(hbm);
				}
				map.Add(id++, pci);
			}
			favo.RemoveAll();
			str.LoadString(IDS_TRAY_FAVORITE);
			pop.AppendMenu(MF_POPUP, sub.Detach(), str);
		}

		// ----------------------------------
		// 最近閉じたタブ
		// ----------------------------------
		CTabReopenArray array;
		cnt = LoadTabReopenArray(array, 0);
		if (cnt > 0) {
			sub.CreatePopupMenu();
			for (i = 0; i < cnt; i++) {
				pci = new CCmItem(*array.GetItemAt(i));
				sub.AppendMenu(MF_STRING, id, pci->m_name);
				HBITMAP hbm = CreateBitmapFromImageList(m_hSysImage, pci->GetImageIndex());
				if (hbm) {
					SetBitmapToMenuItem(sub, id, hbm);
					arrBitmaps.Add(hbm);
				}
				map.Add(id++, pci);
			}
			str.LoadString(IDS_TABRECENT);
			pop.AppendMenu(MF_POPUP, sub.Detach(), str);
		}

		// ----------------------------------
		// タブ グループ
		// ----------------------------------
		CCmItemArray tabgroup;
		cnt = EnumTabGroup(tabgroup);
		if (cnt > 0) {
			sub.CreatePopupMenu();
			for (i = 0; i < cnt; i++) {
				pci = tabgroup[i];
				sub.AppendMenu(MF_STRING, id, pci->m_name);
				HBITMAP hbm = CreateBitmapFromImageList(m_hSysImage, pci->GetImageIndex());
				if (hbm) {
					SetBitmapToMenuItem(sub, id, hbm);
					arrBitmaps.Add(hbm);
				}
				map.Add(id++, pci);
			}
			tabgroup.RemoveAll();
			str.LoadString(IDS_TABGROUP);
			pop.AppendMenu(MF_POPUP, sub.Detach(), str);
		}

		// ----------------------------------
		// コピーヒストリをリストアップ
		// ----------------------------------
		sub.CreatePopupMenu();
		CSimpleArray<CString> arrCopyTo;
		cnt = RegLoadHistory(arrCopyTo, IDS_SEC_HISTORY, IDS_KEY_COPYTARGET, MAXACCESS);
		if (cnt > 0) {
			cnt = min(cnt, 10);				// 最大10個
			for (int n = 0; n < cnt; n++) {
				if (idlFq.CreateFromPath(arrCopyTo[n])) {
					pci = new CCmItem(idlFq);
					sub.AppendMenu(MF_STRING, id, pci->m_name);
					HBITMAP hbm = CreateBitmapFromImageList(m_hSysImage, pci->GetImageIndex());
					if (hbm) {
						SetBitmapToMenuItem(sub, id, hbm);
						arrBitmaps.Add(hbm);
					}
					map.Add(id++, pci);
					idlFq.Free();
				}
			}
		}
		else {
			str.LoadString(IDS_UNDEFINED);
			sub.AppendMenu(MF_STRING | MF_GRAYED, (UINT_PTR)0, str);
		}
		str.LoadString(IDS_RECENT_COPYHISTORY);
		pop.AppendMenu(MF_POPUP, sub.Detach(), str);

		wc.Restore();

		id = pop.TrackPopupMenu(TPM_CENTERALIGN | TPM_BOTTOMALIGN | TPM_RETURNCMD, pt.x, pt.y, m_hWnd);
		if (id > 0) {
			if (id == IDM_TRAY_EXITTSR) {
				PostMessage(WM_CLOSE, 0, 0);
			}
			else {
				pci = map.Lookup(id);
				if (pci)
					OpenFolder(pci);
			}
		}

		// 配列エントリの削除
		cnt = map.GetSize();
		for (i = 0; i < cnt; i++) {
			pci = map.GetValueAt(i);
			delete pci;
		}
		cnt = arrBitmaps.GetSize();
		for (i = 0; i < cnt; i++)
			::DeleteObject(arrBitmaps[i]);
	}

	void TraySysMenu()
	{
		SetForegroundWindow(m_hWnd);

		// カーソル位置取得
		CPoint pt;
		GetCursorPos(&pt);
		CMenu pop;
		pop.CreatePopupMenu();
		// ----------------------------------
		// 常駐終了メニュー
		// ----------------------------------
		CString str; str.LoadString(IDM_TRAY_EXITTSR);
		pop.AppendMenu(MF_STRING, IDM_TRAY_EXITTSR, str);

		int id = pop.TrackPopupMenu(TPM_CENTERALIGN | TPM_BOTTOMALIGN | TPM_RETURNCMD, pt.x, pt.y, m_hWnd);
		if (id == IDM_TRAY_EXITTSR) {
			CSimpleArray<HWND> array;
			int cnt = GetSiblings(m_hWnd, array);
			for (int i = 0; i < cnt; i++)
				::SendMessage(array[i], WM_CLOSE, 0, 0L);
			PostMessage(WM_CLOSE, 0, 0);
		}
	}
};

#endif		// __TRAYFRM_H__
