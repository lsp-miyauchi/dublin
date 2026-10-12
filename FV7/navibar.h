// navibar.h

#pragma once
#ifndef __NAVIBAR_H__
#define __NAVIBAR_H__

#include "gcdata.h"
#include "navicore.h"

////////////////////////////////////////////////////////////////////////
//

class CNaviBar : public CWindowImpl<CNaviBar, CToolBarCtrl>
{
private:
	CNaviCoreCtrl m_navicore;
	CSimpleMap<CString, int> m_arrFolder;
	UINT m_idCtrl;

public:
	CNaviBar() : m_idCtrl((UINT)-1)
	{
	}

	BEGIN_MSG_MAP(CNaviBar)
		MESSAGE_HANDLER(WM_WINDOWPOSCHANGING, OnHandlerWindowPosChanging)
		MESSAGE_HANDLER(WM_SETFOCUS, OnHandlerSetFocus)
		ON_WM_MOUSEACTIVATE()
		ON_NOTIFY(NVCN_GETDISPINFO, m_idCtrl, OnNotifyGetDispInfo)
		ON_NOTIFY(NVCN_BEGINDRAG, m_idCtrl, OnNotifyDragBegin)
		ON_REGISTERD_MESSAGE(nRM_GETDRAGIMAGE, OnGetDragImage)
		ON_COMMAND(m_idCtrl, OnCommandGoTo)
	END_MSG_MAP()

	LRESULT OnHandlerWindowPosChanging(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
	{
		// navicoreコントロールの位置とサイズを制御
		if (!m_navicore.IsWindow()) {
			bHandled = FALSE;
			return 1;
		}

		// ツールバー全体の幅からすべてのボタンの幅合計を引いたスペースを
		// navicoreコントロールで埋める
		int comboIndex = CommandToIndex(m_idCtrl);
		CRect rc;
		CRect rcCombo;
		int button_total_width = 0;
		int button_height = 0;
		int numButtons = GetButtonCount();
		for (int i = 0; i < numButtons; i++) {
			GetItemRect(i, &rc);
			if (i != comboIndex) {
				button_total_width += rc.Width();
				button_height = rc.Height();
			}
			else
				rcCombo = rc;
		}

		WINDOWPOS* pwp = (WINDOWPOS*)lParam;
		int combo_width = max(48, pwp->cx - (button_total_width + 8));
		SetButtonInfo(m_idCtrl, TBIF_SIZE, 0, 0, NULL, 0, (WORD)combo_width, 0, 0);

		LRESULT lRet = ::DefWindowProc(m_hWnd, uMsg, wParam, lParam);

		m_navicore.GetWindowRect(&rc);
		m_navicore.SetWindowPos(NULL, rcCombo.left, rcCombo.top + (rcCombo.Height() - rc.Height()) / 2, combo_width, rc.Height(), SWP_SHOWWINDOW | SWP_NOACTIVATE | SWP_NOOWNERZORDER);

		return lRet;
	}

	LRESULT OnHandlerSetFocus(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
	{
		if (m_navicore.IsWindow()) {
			m_navicore.SetFocus();
		}
		return 0;
	}

	int OnMouseActivate(HWND hDesktopWnd, UINT nHitTest, UINT message)
	{
		LRESULT lr = DefWindowProc();
		if (lr != MA_NOACTIVATE && lr != MA_NOACTIVATEANDEAT) {		// frame does not want to activate
			if (nHitTest == HTCLIENT)
				fvApp.SetActiveView(m_hWnd);
		}
		return (int)lr;
	}

	HWND Initialize(int index, UINT idCtrl)
	{
		if (m_navicore.IsWindow()) return NULL;

		TBBUTTON tbb;
		memset(&tbb, 0, sizeof(tbb));
		tbb.fsStyle = TBSTYLE_BUTTON;
		tbb.idCommand = idCtrl;
		tbb.iBitmap = 9999;
		if (InsertButton(index, &tbb)) {
			TBBUTTONINFO tbbi;
			memset(&tbbi, 0, sizeof(tbbi));
			tbbi.cbSize = sizeof(TBBUTTONINFO);
			tbbi.dwMask = TBIF_SIZE;
			tbbi.cx = 32;
			SetButtonInfo(idCtrl, &tbbi);

			HFONT hf = GetFont();
			CClientDC dc(NULL);
			hf = dc.SelectFont(hf);
			TEXTMETRIC tm;
			dc.GetTextMetrics(&tm);
			dc.SelectFont(hf);
			int bcy = (tm.tmHeight + tm.tmExternalLeading) + 8;
			DWORD s = GetButtonSize();
			bcy = min((int)HIWORD(s), bcy);
			DWORD style = WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS | WS_CLIPCHILDREN;
			CRect rc(0, 0, 32, bcy);
			if (m_navicore.Create(m_hWnd, rc, NULL, style, WS_EX_CLIENTEDGE, idCtrl)) {
				m_idCtrl = idCtrl;
				// イメージリスト設定
				HIMAGELIST himL, himS;
				Shell_GetImageLists(&himL, &himS);
				m_navicore.SetImageList(himS);

				// フォント設定
				m_navicore.SetFont(GetFont(), FALSE);
				return m_navicore.m_hWnd;
			}
		}
		return NULL;
	}

	void ApplyRebarInitialSetting(CReBarCtrl rebar, UINT idBand)
	{
		// ここで親リバーに対する設定をおこなう
		// Prev/Nextツールバーの位置確保など
	}

	inline BOOL IsMyMessage(const MSG* pMsg) const
	{
		CEdit edit = m_navicore.GetEditCtrl();
		return (edit.IsWindow() && pMsg->hwnd == edit);
	}

	void UpdateList(CHistory* pHistory)
	{
		if (!m_navicore.IsWindow()) return;

		m_navicore.SetRedraw(FALSE);
		m_navicore.ListResetContent();

		CCmItem* pciCur = pHistory->GetCurrentItem();
		ATLASSERT(pciCur != NULL);

		// ヒストリから順次ピックアップ
		CSimpleArray<CString> arrExists;
		int sel = 0;
		int i = (int)pHistory->GetCount() - 1;
		int index = 0;
		while (i >= 0) {
			CCmItem* pci = (*pHistory)[i];
			if (pci) {
				// ヒストリのリストアップ（重複しているものは除く）
				if (arrExists.Find(pci->m_name) < 0) {
					if (m_navicore.ListAddItem(pci->m_name, I_IMAGECALLBACK, (LPARAM)pci) >= 0) {
						if (*pci == *pciCur) sel = index;
						arrExists.Add(pci->m_name);
						index++;
					}
				}
			}
			i--;
		}

		m_navicore.SetRedraw(TRUE);
		m_navicore.ListSetCurSel(sel);
	}

	// ----------------------------------------------------------------
	// CSIDL_xxx で登録済みのフォルダを探す
	// [フォルダ]メニュー - [システムフォルダ] 配下にあるフォルダにアクセスできるようにする
	// ----------------------------------------------------------------
	BOOL LookupSpecialFolder(LPCTSTR pszName, CIDListPtr& idlTarget)
	{
		idlTarget.Free();
		CString str;
		if (m_arrFolder.GetSize() < 1) {
			CIDListPtr idl;
			SHFILEINFO sfi;
			for (int i = 0; i <= CSIDL_COMPUTERSNEARME; i++) {
				if (idl.GetFolderLocation(NULL, i) == S_OK) {
					idl.GetFileInfo(0, &sfi, sizeof(sfi), SHGFI_DISPLAYNAME);
					// 全・半角スペースを削除し、インデクス + 1 を登録する。後のLookupで見つけたか否かを判断するため。
					str = sfi.szDisplayName;
					str.MakeUpper();
					while (1) {
						int pos = str.FindOneOf(_T(" 　"));
						if (pos < 0) break;
						str.Delete(pos);
					}
					m_arrFolder.Add(str, i + 1);
					idl.Free();
				}
			}
		}

		str = pszName;
		str.MakeUpper();
		// 全・半角スペースを削除し、一致を検索
		while (1) {
			int pos = str.FindOneOf(_T(" 　"));
			if (pos < 0) break;
			str.Delete(pos);
		}
		int nFolder = m_arrFolder.Lookup(str);
		if (nFolder > 0 && idlTarget.GetFolderLocation(NULL, nFolder - 1) == S_OK)
			return TRUE;
		return FALSE;
	}

	void GotoFolder()
	{
		CString str;
		int len = m_navicore.GetWindowText(str);
		if (len == 0) return;

		if (str[0] == _T('@')) {
			KNOWNFOLDERID id = LookupKnownFolder(str.GetString() + 1);
			LPITEMIDLIST pidl;
			if (id != GUID_NULL && SHGetKnownFolderIDList(id, 0, NULL, &pidl) == S_OK) {
				fvApp.OpenFolder(pidl);
				ILFree(pidl);
				return;
			}
		}
		str.GetBuffer(MAX_PATH * 2);
		PathAnalyze(str.GetBuffer(), str.GetAllocLength());
		int index = m_navicore.ListFindString(-1, str, TRUE, FALSE);
		if (index >= 0) {
			CCmItem* pci = (CCmItem*)m_navicore.GetItemData(index);
			if (pci)
				fvApp.OpenFolder(pci->m_idl);
		} else {
			// '/' を '\' に変換
			if (str.Find(_T('/')) >= 0) {
				str.Replace(_T('/'), _T('\\'));
			}

			// 環境変数パス
			TCHAR buf[MAX_PATH * 2];
			if (ExpandEnvironmentStrings(str, buf, ARRAYSIZE(buf)) > 0) {
				str = buf;
				len = str.GetLength();
			}

			CAtlRegExp<> re;
			CAtlREMatchContext<> mc;
			re.Parse(_T("^[A-Za-z]:?$"));
			if (re.Match(str.GetString(), &mc)) {
				// ドライブのみ指定
				int drive = ToUpper(str[0]) - 'A';
				if (!fopGetDirRemind(drive, buf))
					PathBuildRoot(buf, drive);
				str = buf;
			}

			// 文字列中に ':' や '\' が含まれていなかチェック
			if (str.Find(_T(':')) < 0 && str.Find(_T('\\')) < 0) {
				// special-folder検索
				CIDListPtr idl;
				if (LookupSpecialFolder(str, idl)) {
					fvApp.OpenFolder(idl);
					return;
				}
			}

			if (_pathisdirectoryUI(m_hWnd, str))
				fvApp.OpenFolder(str);
		}
	}

	void OnCommandGoTo()
	{
		GotoFolder();
	}

	BOOL ProcessMessage(const MSG* pMsg)
	{
		switch (pMsg->message) {
		case WM_CHAR:
			switch (pMsg->wParam) {
			case VK_RETURN:
				GotoFolder();
				return TRUE;
			}
			break;
		}
		return FALSE;
	}

	void OnNotifyGetDispInfo(NMHDR* pNmhdr, LRESULT* pResult)
	{
		NMNAVICORE* pdi = (NMNAVICORE*)pNmhdr;
		CCmItem* pci = (CCmItem*)pdi->item.lParam;

		if (pdi->item.mask & NVCIF_IMAGE) {
			pdi->item.iImage = pci->GetImageIndex();
			pdi->item.mask |= NVCIF_DI_SETITEM;
		}
		*pResult = 0;
	}

	void OnNotifyDragBegin(NMHDR* pNmhdr, LRESULT* pResult)
	{
		// アイコンラベル部のD&D開始
		// ドラッグした項目が、キーボードで入力されたテキストではないことを保証する
		CString str;
		int len = m_navicore.GetWindowText(str);
		if (len > 0) {
			int index = m_navicore.ListFindString(-1, str, TRUE, FALSE);
			if (index != CB_ERR) {
				CCmItem* pci = (CCmItem*)m_navicore.GetItemData(index);
				if (pci) {
					CComPtr<IDataObject> pdo = (IDataObject*)pci->GetUIObjectOf(NULL, IID_IDataObject);
					if (pdo.p != NULL) {
						TDropSource* pds = new TDropSource(m_hWnd, pdo, FALSE);
						DROPEFFECT eff = DROPEFFECT_LINK;
						DoDragDrop(pdo, pds, eff, &eff);
						pds->Release();
					}
				}
			}
		}
		*pResult = 0;
	}

	LRESULT OnGetDragImage(WPARAM, LPARAM lParam)
	{
		SHDRAGIMAGE* image = (SHDRAGIMAGE*)lParam;
		int index = m_navicore.ListGetCurSel();
		if (index != CB_ERR) {
			CCmItem* pci = (CCmItem*)m_navicore.GetItemData(index);
			if (pci) {
				memset(image, 0, sizeof(SHDRAGIMAGE));

				CImageList il = m_navicore.GetImageList();
				CSize sizeIcon;
				il.GetIconSize(sizeIcon);

				CClientDC dc(m_hWnd);
				CDC dcw;
				dcw.CreateCompatibleDC(dc);
				HFONT hfont = dcw.SelectFont(m_navicore.GetFont());

				// ドラッグイメージのマウスポインタ位置
				CPoint pt;
				GetCursorPos(&pt);
				m_navicore.ScreenToClient(&pt);

				// 文字列サイズを取得
				CString str;
				m_navicore.GetWindowText(str);

				CRect rc;
				rc.SetRectEmpty();
				dcw.DrawText(str, -1, &rc, DT_CALCRECT | DT_SINGLELINE | DT_NOPREFIX);
				rc.right += 6;
				rc.bottom += 2;

				image->crColorKey = RGB(255, 255, 255);
				image->ptOffset = pt;
				image->sizeDragImage.cx = sizeIcon.cx + rc.Width();
				image->sizeDragImage.cy = max(sizeIcon.cy, rc.Height());

				HBITMAP hbm = CreateCompatibleBitmap(dc, image->sizeDragImage.cx, image->sizeDragImage.cy);
				if (hbm) {
					hbm = dcw.SelectBitmap(hbm);
					dcw.SetBkMode(TRANSPARENT);

					rc.SetRect(0, 0, image->sizeDragImage.cx, image->sizeDragImage.cy);
					dcw.FillSolidRect(&rc, image->crColorKey);
					ImageList_Draw(il, pci->GetImageIndex(), dcw, 0, (rc.Height() - sizeIcon.cy) / 2, ILD_TRANSPARENT);
					rc.left += (sizeIcon.cx + 2);
					dcw.DrawText(str, -1, &rc, DT_SINGLELINE | DT_NOPREFIX | DT_VCENTER | DT_CENTER);

					image->hbmpDragImage = dcw.SelectBitmap(hbm);
				}
				dcw.SelectFont(hfont);
			}
		}
		return (image->hbmpDragImage != NULL);
	}
};


#endif		// __NAVIBAR_H__
