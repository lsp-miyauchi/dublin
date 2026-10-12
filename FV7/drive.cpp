#include "stdafx.h"
#include "fv7.h"

#ifndef __DRIVE_H__
	#include "drive.h"
#endif

#ifdef AFX_CORE3_SEG
#pragma code_seg(AFX_CORE3_SEG)
#endif

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
//@ #define new DEBUG_NEW
#endif

/////////////////////////////////////////////////////////////////////////////
//

#define TIMERID_DBLCLK		8
#define COMMANDID_BASE		IDM_DRIVEBAR_IDSTART

const int cxyBtnMargin = 7;

/////////////////////////////////////////////////////////////////////////////
//

class CSimplePidlArray : public CSimpleArray<LPITEMIDLIST>
{
public:
	void Sort(IShellFolder* psf)
	{
		int cnt = GetSize();
		LPITEMIDLIST* dataptr = GetData();
		for (int i = cnt - 1; i > 0; i--) {
			int pos = 0;
			for (int j = 1; j <= i; j++) {
				if ((short)HRESULT_CODE(psf->CompareIDs(0, *(dataptr + pos), *(dataptr + j))) < 0)
					pos = j;
			}
			if (i != pos) {
				LPITEMIDLIST pidl = *(dataptr + i);
				*(dataptr + i) = *(dataptr + pos);
				*(dataptr + pos) = pidl;
			}
		}
	}
};

/////////////////////////////////////////////////////////////////////////////
// CDriveBar creation etc

#define DRIVEBAR_HORZ_STYLE		(ATL_SIMPLE_TOOLBAR_PANE_STYLE | TBSTYLE_LIST)
#define DRIVEBAR_VERT_STYLE		(ATL_SIMPLE_TOOLBAR_PANE_STYLE | TBSTYLE_LIST | TBSTYLE_WRAPABLE | TBSTYLE_TRANSPARENT | CCS_LEFT)

HWND CDriveBar::CreateBar(HWND hParent, UINT idCtrl, bool bVertical)
{
	m_bVertical = bVertical;
	DWORD style = m_bVertical ? DRIVEBAR_VERT_STYLE : DRIVEBAR_HORZ_STYLE;
	return Create(hParent, 0, NULL, style, 0, idCtrl, 0);
}

void CDriveBar::SetVertical(bool bVertical)
{
	if (IsWindow() && (bVertical != m_bVertical)) {
		DWORD style = (DWORD)::GetWindowLong(m_hWnd, GWL_STYLE);
		if (bVertical)
			style |= TBSTYLE_WRAPABLE | CCS_LEFT;
		else
			style &= ~(TBSTYLE_WRAPABLE | CCS_LEFT);
		::SetWindowLong(m_hWnd, GWL_STYLE, style);
		m_bVertical = bVertical;
	}
}

void CDriveBar::WepAllItems()
{
	int cnt = m_array.GetSize();
	for (int i = 0; i < cnt; i++) {
		CDrivePidl* pdp = m_array[i];
		if (pdp) delete pdp;
	}
	m_array.RemoveAll();
}

void CDriveBar::OnRButtonDown(UINT flags, CPoint point)
{
	SetDropDownProcessing(true);

	int index = HitTest(&point);
	CDrivePidl* pdp = GetItemData(index);
	if (pdp) {
		CRect rc;
		GetItemRect(index, &rc);

		TBBUTTON tbb;
		UINT id = (GetButton(index, &tbb) ? tbb.idCommand : 0);
		PressButton(id, TRUE);

		CPoint pt(rc.left, rc.bottom - 1);
		ClientToScreen(&pt);
		int cmdBack = TrackContextMenuByFile(GetParent(), pdp->m_strPathName, &pt);

		PressButton(id, FALSE);

		if (cmdBack > 0) {
			switch (cmdBack) {
			case IDM_TAB_NEW:			SendNotify(id, DBN_NEWTAB); break;
			case IDM_DIR_ADDFAVORITE:	SendNotify(id, DBN_ADDFAVORITE); break;
			default: break;
			}
		}
	}

	SetDropDownProcessing(false);
}

void CDriveBar::OnMButtonDblClk(UINT flags, CPoint point)
{
	if (gnDrvStat & DRS_MBTN_MASK) {
		SetDropDownProcessing(true);

		int index = HitTest(&point);
		CDrivePidl* pdp = GetItemData(index);
		if (pdp) {
			CRect rc;
			GetItemRect(index, &rc);

			TBBUTTON tbb;
			UINT id = (GetButton(index, &tbb) ? tbb.idCommand : 0);
			switch (gnDrvStat & DRS_MBTN_MASK) {
			case DRS_MBTN_NEWTAB: SendNotify(id, DBN_NEWTAB); break;
			case DRS_MBTN_NEWWINDOW: SendNotify(id, DBN_NEWWINDOW); break;
			}
			PressButton(id, FALSE);
		}

		SetDropDownProcessing(false);
	}
}

void CDriveBar::SendNotify(UINT id, UINT code)
{
	int index = id - COMMANDID_BASE;
	CDrivePidl* pdp = GetItemData(index);
	if (pdp) {
		if (pdp->isNull()) {
			if (!CheckServerUI(GetTopLevelParent(), pdp->m_strPathName))
				return;
			pdp->CreateFromPath(pdp->m_strPathName);
			if (pdp->isNull())
				return;
		}

		DRIVENOTIFY dn;
		memset(&dn, 0, sizeof(dn));
		dn.hdr.hwndFrom = m_hWnd;
		dn.hdr.idFrom = GetDlgCtrlID();
		dn.hdr.code = code;
		dn.index = index;
		dn.flags  = _ISKEYDOWN(VK_SHIFT) ? MK_SHIFT : 0;
		dn.flags |= _ISKEYDOWN(VK_CONTROL) ? MK_CONTROL : 0;
		lstrcpy(dn.path, pdp->m_strPathName);
		GetTopLevelParent().SendMessage(WM_NOTIFY, dn.hdr.idFrom, (LPARAM)&dn);
	}
}

int CDriveBar::OnCreate(LPCREATESTRUCT pcs)
{
	SetNotifyWnd(m_hWnd);
	InitButtonImages();
	return 0;
}

BOOL CDriveBar::OnCommand(UINT id, UINT code, HWND hwnd)
{
	// ドライブバーダブルクリックが無効か、他のウィンドウから通知された場合
	if (!(gnDrvStat & DRS_LBTN_MASK) || (hwnd != m_hWnd)) {
		// すぐにドライブ変更
		SendNotify(id, DBN_LBUTTONDOWN);
	} else {
		// 自分自身から通知された --- ボタンクリックによる
		// タイマ起動で、ダブルクリックを監視しよう
		if (m_timerid) {
			KillTimer(m_timerid);
			m_timerid = 0;

			UINT cd;
			if (id == m_lastclick_id)
				cd = ((gnDrvStat & DRS_LBTN_MASK) == DRS_LBTN_NEWTAB) ? DBN_NEWTAB : DBN_NEWWINDOW;
			else
				cd = DBN_LBUTTONDOWN;
			m_lastclick_id = 0;
			SendNotify(id, cd);
		} else {
			// タイマを起動
			m_timerid = SetTimer(TIMERID_DBLCLK, GetDoubleClickTime(), NULL);
			m_lastclick_id = id;
		}
	}
	return TRUE;
}

LRESULT CDriveBar::OnTimer(UINT /*uMsg*/, WPARAM wParam, LPARAM /*lParam*/, BOOL& bHandled)
{
	if ((UINT_PTR)wParam == m_timerid) {
		KillTimer(m_timerid);
		m_timerid = 0;
		if (m_lastclick_id) {
			SendNotify(m_lastclick_id, DBN_LBUTTONDOWN);
			m_lastclick_id = 0;
		}
	}
	else
		bHandled = FALSE;
	return 0;
}

void CDriveBar::OnToolTipText(int id, NMHDR* pNMHDR, LRESULT* pResult)
{
	*pResult = 0;
	if (GetCapture()) return;

	CDrivePidl* pdp = (CDrivePidl*)GetItemData(pNMHDR->idFrom - COMMANDID_BASE);
	if (pdp) {
		NMTTDISPINFOW* ptdi = (NMTTDISPINFO*)pNMHDR;
		CString tip = pdp->GetDisplayName();
		if (!(pdp->m_attr & SFGAO_REMOVABLE)) {
			CString str;
			if (idl_GetInfoTip(m_hWnd, pdp->m_pidl, str) > 0)
				tip.AppendFormat(_T("\n%s"), (LPCTSTR)str);
		}

		if (tip.GetLength() < ARRAYSIZE(ptdi->szText))
			lstrcpy(ptdi->szText, tip);
		else {
			TCHAR text[MAX_PATH * 2];
			lstrcpyn(text, tip, ARRAYSIZE(text));
			ptdi->lpszText = text;
		}
		// bring the tooltip window above other popup windows
		CToolTipCtrl tooltip = pNMHDR->hwndFrom;
		tooltip.SetMaxTipWidth(MY_INFOTIP_LEN);
	}
}

void CDriveBar::ChangeButtonImageSize()
{
	InitButtonImages();
	BuildButtons();
}

void CDriveBar::InitButtonImages()
{
	RemoveCustomDrawBar(m_hWnd);

	// ボタンサイズなど
	SetButtonStructSize(sizeof(TBBUTTON));
	// 全ボタン削除
	DeleteAllButtons();

	HIMAGELIST himl, hims;
	Shell_GetImageLists(&himl, &hims);
	m_himSystem = (gnBarStat & BTS_LARGEICON) ? himl : hims;
	int ix, iy;
	if (!ImageList_GetIconSize(m_himSystem, &ix, &iy)) {
		ix = iy = (gnBarStat & BTS_LARGEICON) ? 32 : 16;
	}
	m_sizeIcon = CSize(ix, iy);
	SetBitmapSize(ix, iy);
	SetButtonSize(ix + cxyBtnMargin, iy + cxyBtnMargin);
	SetImageList(m_himSystem);

	AddCustomDrawBar(m_hWnd);
}

void CDriveBar::DeleteAllButtons()
{
	// 全ボタン削除
	int cnt = GetButtonCount();
	while (cnt-- > 0)
		DeleteButton(0);
}

void CDriveBar::BuildButtons()
{
	// 全ボタン削除
	DeleteAllButtons();
	// ツールバーへ追加する
	int cnt = m_array.GetSize();
	TBBUTTON* tbb = new TBBUTTON[cnt];
	memset(tbb, 0, sizeof(TBBUTTON) * cnt);

	TCHAR prefix[256];
	LPTSTR p = prefix;
	for (int i = 0; i < cnt; i++) {
		tbb[i].fsState = TBSTATE_ENABLED;
		tbb[i].iString = 0;
		CDrivePidl* pdp = m_array[i];
		if (pdp) {
			tbb[i].idCommand = i + COMMANDID_BASE;
			tbb[i].iBitmap = I_IMAGECALLBACK;
			tbb[i].fsStyle = BTNS_BUTTON | BTNS_AUTOSIZE;
			tbb[i].iString = (INT_PTR)p;
			*p++ = pdp->m_prefix;
			*p++ = 0;
		} else {
			tbb[i].idCommand = ID_SEPARATOR;
			tbb[i].iBitmap = 6;
			tbb[i].fsStyle = BTNS_SEP;
		}
	}
	AddButtons(cnt, tbb);
	delete [] tbb;
}

void CDriveBar::UpdateItemImage(LPCITEMIDLIST pidl)
{
	if (pidl) {
		CDrivePidl* pdp;
		int cnt = m_array.GetSize();
		for (int i = 0; i < cnt; i++) {
			pdp = m_array[i];
			if (pdp && !pdp->isNull() && ::ILIsEqual(pdp->m_pidl, pidl)) {
				pdp->ResetImageIndex();
				TBBUTTONINFO tbbi;
				tbbi.cbSize = sizeof(tbbi);
				tbbi.dwMask = TBIF_IMAGE;
				tbbi.iImage =  I_IMAGECALLBACK;
				SetButtonInfo(i + COMMANDID_BASE, &tbbi);
				break;
			}
		}
	}
}

LRESULT CDriveBar::OnHandlerToolbarGetButtonInfo(UINT msg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	// シェブロンメニュー対応：altframe.h内でメニュー構築の際に呼び出される
	TBBUTTONINFO* ptbbi = (TBBUTTONINFO*)lParam;
	if (ptbbi) {
		CDrivePidl* pdp = (CDrivePidl*)GetItemData((int)wParam - COMMANDID_BASE);
		if (pdp) {
			if (ptbbi->dwMask & TBIF_TEXT) {
				lstrcpyn(ptbbi->pszText, pdp->GetDisplayName(), ptbbi->cchText);
				// テキストフラグを落とす
				ptbbi->dwMask &= ~TBIF_TEXT;
			}
			if (ptbbi->dwMask & TBIF_IMAGE) {
				ptbbi->iImage = pdp->GetImageIndex();
				// イメージインデクスフラグを落とす
				ptbbi->dwMask &= ~TBIF_IMAGE;
			}
		}
	}
	// まだ必要なマスクがあれば、元TOOLBARに残りの処理を引き渡す
	bHandled = (ptbbi->dwMask == 0);
	return 0;
}

LRESULT CDriveBar::OnHandlerQueryImageList(UINT msg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	CDrivePidl* pdp = (CDrivePidl*)GetItemData((int)wParam - COMMANDID_BASE);
	int* piImage = (int*)lParam;
	if (piImage)
		*piImage = (pdp != NULL) ? pdp->m_iImage : -1;
	return (LRESULT)m_himSystem;
}

void CDriveBar::DriveReset()
{
	BYTE cb[26];
	if (GetRegBinary(IDS_SEC_DRIVE, IDS_KEY_DRIVEMASK, cb, 26) != 26)
		memset(cb, 0, sizeof(cb));

	CIDListPtr idlDrives(FOLDERID_ComputerFolder);
	CComPtr<IShellFolder> psf = idlDrives.GetShellFolder();
	idl_ValidateCacheFolderInformation(psf);

	CString str;
	CDrivePidl* pdp;
	CSimpleArray<CDrivePidl*> drives;
	CSimpleArray<CDrivePidl*> netdev;
	CComPtr<IEnumIDList> pe;
	TCHAR prefix = _T('0');
	if (psf->EnumObjects(NULL, SHCONTF_FOLDERS | SHCONTF_INCLUDEHIDDEN | SHCONTF_FASTITEMS, &pe) == S_OK) {
		// 列挙
		LPITEMIDLIST pidl;
		CSimplePidlArray pidlDrives;
		while (pe->Next(1, &pidl, NULL) == NOERROR) { pidlDrives.Add(pidl); }

		// ソート
		pidlDrives.Sort(psf);
		// 選別
		int i, cnt = pidlDrives.GetSize();
		for (i = 0; i < cnt; i++) {
			pidl = pidlDrives[i];
			int descid = idl_GetDescriptionId(psf, pidl);
			if (_INRANGE(descid, SHDID_COMPUTER_DRIVE35, SHDID_COMPUTER_RAMDISK) ||
				((descid == SHDID_MOBILE_DEVICE) && (gnDrvStat & DRS_MOBILEDEVICE))) {
				SFGAOF ula = 0x70000000 | SFGAO_REMOVABLE | SFGAO_LINK;
				if (psf->GetAttributesOf(1, (LPCITEMIDLIST*)&pidl, &ula) == S_OK &&
					idl_GetName(psf, pidl, SHGDN_FORPARSING, str)) {
					if (PathIsRoot(str)) {
						int num = PathGetDriveNumber(str);
						if (num < 0 || cb[num] == 0) {
							pdp = new CDrivePidl(psf, pidl, ula, str);
							drives.Add(pdp);
						}
					} else if ((ula & SFGAO_LINK) == 0 && (gnDrvStat & DRS_NETDEVICE)) {
						// Windows8以降ではネットデバイスが通常ドライブより前になるようにソートされてしまうので、まず分けて記録
						pdp = new CDrivePidl(psf, pidl, ula, str);
						pdp->m_prefix = prefix++;
						netdev.Add(pdp);
					}
				}
			}
			CoTaskMemFree(pidl);
		}
	}

	// 追加フォルダ
	int i;
	for (i = netdev.GetSize(); i < 10; i++) {
		if (GetRegTextSeq(IDS_SEC_DRIVE, IDS_KEY_NETCOM, str, i) < 1)
			break;

		int iImage = -1;
		LPTSTR p = StrChr(str, ',');
		if (p) {
			*p++ = 0;
			iImage = PathParseIconLocation(p);
		}
		pdp = new CDrivePidl(str, p, iImage);
		pdp->m_prefix = prefix++;
		netdev.Add(pdp);
	}

	WepAllItems();
	// ドライブを適用
	for (i = 0; i < drives.GetSize(); i++) {
		m_array.Add(drives[i]);
	}
	// ネットフォルダを適用
	for (i = 0; i < netdev.GetSize(); i++) {
		m_array.Add(netdev[i]);
	}

	BuildButtons();
	Invalidate(FALSE);
}

void CDriveBar::DriveSelect(LPCITEMIDLIST pidl)
{
	BOOL bFound = FALSE;
	CDrivePidl* pdp;
	int sel = 0;
	if (pidl) {
		CString path;
		if (!idl_GetPathFromIDList(pidl, path))
			path.Empty();

		int cnt = m_array.GetSize();
		for (sel = 0; sel < cnt; sel++) {
			pdp = m_array[sel];
			if (!pdp) continue;

			if (!pdp->isNull()) {
				if (ILIsParent(pdp->m_pidl, pidl, FALSE)) {
					bFound = TRUE;
					break;
				}
			}
			else {
				if (!path.IsEmpty() &&
					!pdp->m_strPathName.IsEmpty() &&
					!StrCmpNI(pdp->m_strPathName, path, pdp->m_strPathName.GetLength())) {
					bFound = TRUE;
					break;
				}
			}
		}
	}

	CRect rc;
	if (m_nCurDrive >= 0 && (!bFound || m_nCurDrive != sel)) {
		pdp = GetItemData(m_nCurDrive);
		if (pdp) {
			CheckButton(m_nCurDrive + COMMANDID_BASE, FALSE);
		}
	}

	if (!bFound)
		m_nCurDrive = -1;
	else {
		CheckButton(sel + COMMANDID_BASE, TRUE);
		m_nCurDrive = sel;
	}
}

LPCITEMIDLIST CDriveBar::GetCurDrive()
{
	CDrivePidl* pdp = GetItemData(m_nCurDrive);
	return ((pdp != NULL) ? pdp->m_pidl : NULL);
}

int CDriveBar::QeuryOverlayImageIndex(UINT id)
{
	CDrivePidl* pdp = GetItemData(id - COMMANDID_BASE);
	return ((pdp != NULL) ? pdp->m_iOverlayImage : 0);
}

DWORD CDriveBar::OnDragOver(IDataObject* pdo, DWORD keyState, DWORD dropEffect, POINT point)
{
	ScreenToClient(&point);

	// ドロップ可能パスを特定する
	int indexHot = GetHotItem();
	int nNewPos = HitTest(&point);

	// ドロップ可能パスに変化がない場合
	DROPEFFECT eff = GetDropEffect(dropEffect, keyState);
	if (nNewPos == indexHot) {
		if (nNewPos < 0)
			eff = DROPEFFECT_NONE;
	}
	else {
		ShowDropHelper(FALSE);
		SetHotItem(nNewPos);
		ShowDropHelper(TRUE);
	}

	// VISTAのエアロD&Dテキスト
	CString strTargetDisplayName;
	if (nNewPos != -1) {
		CDrivePidl* pdp = GetItemData(nNewPos);
		if (pdp)
			strTargetDisplayName = pdp->GetDisplayName();
	}
	SetDropTargetDescription(pdo, strTargetDisplayName, eff);
	return eff;
}

void CDriveBar::OnDragLeave()
{
	// ドロップ待機状態になっているパスを元に戻す
	int indexHot = GetHotItem();
	if (indexHot >= 0)
		SetHotItem(-1);
}

DWORD CDriveBar::OnDrop(IDataObject* pdo, DWORD keyState, DWORD dropEffect, POINT point)
{
	int indexHot = GetHotItem();

	DROPEFFECT eff = DROPEFFECT_NONE;
	if (dropEffect && indexHot >= 0) {
		TCHAR dir[MAX_PATH * 2];
		CDrivePidl* pdp = GetItemData(indexHot);
		if (!pdp)
			return eff;

		if (_pathispathspec(pdp->m_strPathName)) {
			// ディレクトリパスへのドロップ
			int drive;
			if (pdp->m_strPathName[1] == ':') {
				drive = PathGetDriveNumber(pdp->m_strPathName);
				if (!fopGetDirRemind(drive, dir)) {
					fopSetDirRemind(drive, NULL);
					PathBuildRoot(dir, drive);
				}
			}
			else
				lstrcpy(dir, pdp->m_strPathName);

			BOOL bThreaded = (gnFopStat & FOS_RUNCOPY) ? TRUE : FALSE;
			BOOL bAsAdmin = FALSE;
			if (QueryFolder(NULL, pdp->m_strPathName, dir, ARRAYSIZE(dir), bThreaded, bAsAdmin)) {
				drive = PathGetDriveNumber(dir);
				if (drive >= 0)
					fopSetDirRemind(drive, dir);

				// HDROPとFileNameMapの両方を持っている場合は、ごみ箱からのD&Dの可能性あり。
				// なのでこの場合はCF_HDROPでデータを取得せずにシステムに処理を任せる
				HDROP hdrop = NULL;
				if ((GetSupportedDropFormats() & (SDO_HDROP | SDO_FILENAMEMAPA | SDO_FILENAMEMAPW)) == SDO_HDROP)
					hdrop = (HDROP)GetDataFromDataObject(pdo, CF_HDROP);

				if (!hdrop || (keyState & MK_RBUTTON)) {
					CIDListPtr idl;
					if (idl.CreateFromPath(dir)) {
						eff = AcceptDataObject(m_hWnd, idl, pdo, keyState, dropEffect, point);
					}
				}
				else {
					AcceptFiles(hdrop, dropEffect, dir, bThreaded, bAsAdmin);
					eff = dropEffect;
				}

				if (hdrop)
					DragFinish(hdrop);

				RegSaveHistory(IDS_SEC_HISTORY, IDS_KEY_COPYTARGET, MAXACCESS, dir);
			}
		}
		else {
			// 仮想フォルダへのドロップ
			CString str(MAKEINTRESOURCE(IDS_SELECT_DROPTARGET));
			CFolderDialog browser(m_hWnd, str, BIF_USENEWUI | BIF_NONEWFOLDERBUTTON);
			browser.m_bi.pidlRoot = pdp->m_pidl;
			if (browser.DoModal() == IDOK) {
				idl_SendTo(m_hWnd, browser.GetSelectedItem(), pdo);
			}
		}
	}
	return eff;
}

HBITMAP CDriveBar::GetMenuImageBitmap(UINT id)
{
	CDrivePidl* pdp = GetItemData(id - COMMANDID_BASE);
	if (pdp && pdp->m_iImage >= 0 && pdp->m_bmIcon.IsNull()) {
		HIMAGELIST himL, himS;
		Shell_GetImageLists(&himL, &himS);
		pdp->m_bmIcon.Attach(CreateBitmapFromImageList(himS, pdp->m_iImage));
	}
	return (pdp != NULL) ? pdp->m_bmIcon.m_hBitmap : NULL;
}
