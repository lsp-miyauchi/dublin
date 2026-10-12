// Ctx.cpp : CCtx の実装

#include "stdafx.h"
#include "mainmodule.h"
#include "Ctx.h"
#include "common.h"
#include "setup.h"

////////////////////////////////////////////////////////////////////////
//

// vwfnd/lsshellでも同じものを使う
enum {
	CPD_JUMP_PATHA = 1,
	CPD_JUMP_PATHW,
	CPD_JUMP_IDLIST,
	CPD_JUMP_FILEA,
	CPD_JUMP_FILEW,
	CPD_EXEC_IDLIST = 11,
	CPD_EXEC_FILEA,
	CPD_EXEC_FILEW,
	CPD_VIEW_IDLIST = 21,
	CPD_VIEW_FILEA,
	CPD_VIEW_FILEW,
};

#define CPD_COMMAND_MASK		0x0000ffff
#define CPD_NEW_TAB				0x80000000

////////////////////////////////////////////////////////////////////////
// CCtx

HRESULT CCtx::FinalConstruct()
{
	m_flags = 0;
	return S_OK;
}

void CCtx::FinalRelease()
{
	m_arrCommand.RemoveAll();
	m_arrAccessKey.RemoveAll();
}

void CCtx::InsertCommandMenu(HMENU hmenu, UINT iInsertAfter, UINT idCommand, UINT idResource, UINT flags, UINT idImage)
{
	CString str; str.LoadString(idResource);

	// アクセスキーとコマンドIDをマップ配列に追加する
	TCHAR key = 0;
	TCHAR* p = StrChr(str, _T('&'));
	if (p) {
		++p;
		m_arrAccessKey.Add((TCHAR)CharLower((LPTSTR)*p), idCommand);
	}

	InsertMenu(hmenu, iInsertAfter, flags, idCommand, str);
	m_arrCommand.Add(idCommand, idResource);

	if (idImage == 0)
		idImage = idResource;
	SetBitmapToMenuItem(hmenu, idCommand, idImage);
}


HRESULT CCtx::QueryContextMenu(HMENU hmenu, UINT index, UINT idCmdFirst, UINT idCmdLast, UINT uFlags)
{
	if (!IsValidExecuteModule()) return MAKE_HRESULT(SEVERITY_SUCCESS, FACILITY_NULL, 0);

	// イメージリスト
	_AtlModule.InitializeImageList();

	// スタートメニューのプログラムメニュー内のフォルダメニューで
	// コンテキストメニューの動作が変だ
	if ((uFlags & ~CMF_RESERVED) == 0) return MAKE_HRESULT(SEVERITY_SUCCESS, FACILITY_NULL, 0);
	// CMF_OPTIMIZEFORINVOKE が立っているときにメニューを追加すると、「コンピュータの管理」が起動できない
	if (uFlags & (CMF_DEFAULTONLY | CMF_OPTIMIZEFORINVOKE)) return MAKE_HRESULT(SEVERITY_SUCCESS, FACILITY_NULL, 0);

	UINT idCmd = idCmdFirst;
	m_idCmdFirst = idCmdFirst;

	if (uFlags & CMF_VERBSONLY) {
		// CMF_EXTENDEDVERBS, CMF_ASYNCVERBSTATE が立っているときにメニューを追加すると
		//「管理者として実行」が実行できない
		if (!(uFlags & (CMF_EXTENDEDVERBS | CMF_ASYNCVERBSTATE))) {
			// リンク先を開く
			InsertCommandMenu(hmenu, index, idCmd++, IDM_CTX_OPENLINKTARGET, MF_STRING | MF_BYPOSITION);
		}
		return MAKE_SCODE(SEVERITY_SUCCESS, FACILITY_NULL, ((idCmd == idCmdFirst) ? 0 : (idCmd - idCmdFirst) + 1));
	}

	if (!m_flags) return MAKE_HRESULT(SEVERITY_SUCCESS, FACILITY_NULL, 0);

	UINT mflg = MF_STRING | MF_BYPOSITION;
	if (m_userlevel == USRL_EXPIRE) mflg |= MF_GRAYED;

	CMenuItemInfo mi;
	mi.fMask = MIIM_SUBMENU | MIIM_TYPE | MIIM_ID;
	mi.fType = MFT_STRING;

	CString str;
	if (m_flags & STS_SUBMENU) {
		HMENU hSub = CreatePopupMenu();

		// Add it to the context menu 
		// don't use InsertMenu:  See MSDN:
		// PRB: Duplicate Menu Items In the File Menu For a Shell Extension
		// ID: Q214477

		str.LoadString(IDS_GROUP_SHELL);
		mi.dwTypeData = (LPTSTR)(LPCTSTR)str;
		mi.wID = idCmd++;
		mi.hSubMenu = hSub;
		InsertMenuItem(hmenu, index++, TRUE, &mi);
		hmenu = hSub;
	}
	else
		InsertMenu(hmenu, index++, MF_SEPARATOR | MF_BYPOSITION, 0, NULL);

	HMENU hPop;
	UINT ipop;
	if (m_flags & STS_SYNC)			InsertCommandMenu(hmenu, index++, idCmd++, IDM_CTX_FOLDERSYNC, mflg);
	if (m_flags & STS_DFLUSH)		InsertCommandMenu(hmenu, index++, idCmd++, IDM_CTX_DISKFLUSH, mflg);
	if (m_flags & STS_DRVINFO)		InsertCommandMenu(hmenu, index++, idCmd++, IDM_CTX_DRIVEINFO, mflg);
	if (m_flags & STS_FILTERDEL)	InsertCommandMenu(hmenu, index++, idCmd++, IDM_CTX_FILTERDEL, mflg);
	if (m_flags & STS_SHREDDER)		InsertCommandMenu(hmenu, index++, idCmd++, IDM_CTX_SHREDDER, mflg);
	if (m_flags & STS_FSIZE)		InsertCommandMenu(hmenu, index++, idCmd++, IDM_CTX_FILESIZEINFO, mflg);
	if (m_flags & STS_ARCPACK)		InsertCommandMenu(hmenu, index++, idCmd++, IDM_CTX_PACK, mflg);
	if (m_flags & STS_ARCUNPACK)	InsertCommandMenu(hmenu, index++, idCmd++, IDM_CTX_UNPACK, mflg);
	if (m_flags & STS_ARCCHECK)		InsertCommandMenu(hmenu, index++, idCmd++, IDM_CTX_CHECKARC, mflg);
	if (m_flags & STS_BDUMP)		InsertCommandMenu(hmenu, index++, idCmd++, IDM_CTX_BINDUMP, mflg);
	if (m_flags & STS_MERGE)		InsertCommandMenu(hmenu, index++, idCmd++, IDM_CTX_MERGEFILE, mflg);
	if (m_flags & STS_DIV)			InsertCommandMenu(hmenu, index++, idCmd++, IDM_CTX_DIVIDEFILE, mflg);
	if (m_flags & STS_ATTRIB)		InsertCommandMenu(hmenu, index++, idCmd++, IDM_CTX_FILEATTRIB, mflg);
	if (m_flags & STS_ICONINFO)		InsertCommandMenu(hmenu, index++, idCmd++, IDM_CTX_ICONINFO, mflg);
	if (m_flags & STS_ICONPICKER)	InsertCommandMenu(hmenu, index++, idCmd++, IDM_CTX_ICONPICKER, mflg & ~MF_OWNERDRAW);
	if (m_flags & STS_FILEINFO)		InsertCommandMenu(hmenu, index++, idCmd++, IDM_CTX_FILEINFO, mflg);
	if (m_flags & STS_CRYPT)		InsertCommandMenu(hmenu, index++, idCmd++, IDM_CTX_ENCRYPT, mflg);
	if (m_flags & STS_CALCHASH)		InsertCommandMenu(hmenu, index++, idCmd++, IDM_CTX_CALCHASH, mflg);
	if (m_flags & (STS_SERIESNM | STS_REPNAMES | STS_CHGCAPS)) {
		hPop = CreatePopupMenu();
		if (hPop) {
			ipop = 0;
			if (m_flags & STS_RENAME)	InsertCommandMenu(hPop, ipop++, idCmd++, IDM_CTX_RENAME, mflg);
			if (m_flags & STS_SERIESNM)	InsertCommandMenu(hPop, ipop++, idCmd++, IDM_CTX_SERIESNAME, mflg);
			if (m_flags & STS_REPNAMES)	InsertCommandMenu(hPop, ipop++, idCmd++, IDM_CTX_REPLACENAME, mflg);
			if (m_flags & STS_CHGCAPS)	InsertCommandMenu(hPop, ipop++, idCmd++, IDM_CTX_NAMECAP, mflg);

			str.LoadString(IDS_GROUP_CHNAME);
			mi.dwTypeData = (LPTSTR)(LPCTSTR)str;
			mi.wID = idCmd++;
			mi.hSubMenu = hPop;
			InsertMenuItem(hmenu, index++, TRUE, &mi);
		}
	}
	if (m_flags & (STS_PATHCLIP)) {
		hPop = CreatePopupMenu();
		if (hPop) {
			ipop = 0;
			InsertCommandMenu(hPop, ipop++, idCmd++, IDM_CTX_CFNAMEONLY, mflg & ~MF_OWNERDRAW);
			InsertCommandMenu(hPop, ipop++, idCmd++, IDM_CTX_CFFULLPATH, mflg & ~MF_OWNERDRAW);
			InsertCommandMenu(hPop, ipop++, idCmd++, IDM_CTX_CFDOSNAME, mflg & ~MF_OWNERDRAW);

			str.LoadString(IDS_GROUP_NAMECLIP);
			mi.dwTypeData = (LPTSTR)(LPCTSTR)str;
			mi.wID = idCmd++;
			mi.hSubMenu = hPop;
			InsertMenuItem(hmenu, index++, TRUE, &mi);
		}
	}
	if (m_flags & STS_REPARSE) {
		InsertCommandMenu(hmenu, index++, idCmd++, IDM_CTX_OPENJUNCTION, mflg);
		InsertCommandMenu(hmenu, index++, idCmd++, IDM_CTX_REMOVEJUNCTION, mflg & ~MF_OWNERDRAW);
	}
	if (!(m_flags & STS_SUBMENU))
		InsertMenu(hmenu, index++, MF_SEPARATOR | MF_BYPOSITION, 0, NULL);

	return MAKE_SCODE(SEVERITY_SUCCESS, FACILITY_NULL, ((idCmd == idCmdFirst) ? 0 : (idCmd - idCmdFirst) + 1));
}

// InvokeCommand
HRESULT CCtx::InvokeCommand(LPCMINVOKECOMMANDINFO pci)
{
	HRESULT hr = E_INVALIDARG;
	if (HIWORD(pci->lpVerb) == 0) {
		int cnt = m_df.GetCount();
		LPTSTR p;
		UINT idCommand = m_arrCommand.Lookup(LOWORD(pci->lpVerb) + m_idCmdFirst);

		CString name;
		m_df.DragQueryFile(0, name);

		CFopLib fop;
		TCHAR buf[MAX_PATH * 2];
		switch (idCommand) {
		case IDM_CTX_FOLDERSYNC:
			RunDll(pci->hwnd, DLL_FOPSRV, _T("FolderSyncManager"), name);
			break;
		case IDM_CTX_DISKFLUSH:
			fop.FlushDrive(name);
			break;
		case IDM_CTX_DRIVEINFO:
			fop.DriveInfo(pci->hwnd, PathGetDriveNumber(name));
			break;
		case IDM_CTX_FILTERDEL:
			fop.FilterDelete(pci->hwnd, m_df);
			break;
		case IDM_CTX_SHREDDER:
			fop.FileShredder(pci->hwnd, m_df);
			break;
		case IDM_CTX_FILESIZEINFO:
			if (cnt == 1 && PathIsRoot(name))
				fop.EnumPathSize(pci->hwnd, name);
			else
				fop.EnumFileSize(pci->hwnd, m_df);
			break;
		case IDM_CTX_PACK:
			if (cnt > 0) {
				lstrcpy(buf, name);
				PathRemoveFileSpec(buf);
				CompressFiles(pci->hwnd, buf, m_df);
			}
			break;
		case IDM_CTX_UNPACK:
			if (cnt > 0) {
				lstrcpy(buf, name);
				PathRemoveFileSpec(buf);
				ExtractFiles(pci->hwnd, buf, m_df);
			}
			break;
		case IDM_CTX_CHECKARC:
			CheckArchive(pci->hwnd, m_df);
			break;
		case IDM_CTX_BINDUMP:
			PathQuoteSpaces(name.GetBuffer(name.GetLength() + 4));
			name.ReleaseBuffer();
			RunDll(pci->hwnd, DLL_BED, BED_EDIT, name);
			break;
		case IDM_CTX_MERGEFILE:
			fop.MergeFile(pci->hwnd, m_df);
			break;
		case IDM_CTX_DIVIDEFILE:
			DIVIDEDATA dd;
			memset(&dd, 0, sizeof(dd));
			lstrcpy(dd.szTarget, name);
			if (fop.QueryDivide(pci->hwnd, &dd))
				fop.DivideFile(pci->hwnd, &dd);
			break;
		case IDM_CTX_FILEATTRIB:
			fop.FileAttributes(pci->hwnd, m_df);
			break;
		case IDM_CTX_ICONINFO:
			fop.ShowFileInfoEx(pci->hwnd, m_df, FINFO_ICON);
			break;
		case IDM_CTX_ICONPICKER:
			fop.ExportIconFromFile(pci->hwnd, name, NULL);
			break;
		case IDM_CTX_FILEINFO:
			fop.ShowFileInfo(pci->hwnd, m_df);
			break;
		case IDM_CTX_ENCRYPT:
			fop.EncryptFilesTo(pci->hwnd, m_df, NULL);
			break;
		case IDM_CTX_RENAME:
			fop.RenameFiles(pci->hwnd, m_df);
			break;
		case IDM_CTX_SERIESNAME:
			fop.SeriesName(pci->hwnd, m_df);
			break;
		case IDM_CTX_REPLACENAME:
			fop.RepStrNames(pci->hwnd, m_df);
			break;
		case IDM_CTX_NAMECAP:
			fop.ChangeFileCaps(pci->hwnd, m_df);
			break;
		case IDM_CTX_CALCHASH:
			fop.CalcFileHash(pci->hwnd, m_df);
			break;
		case IDM_CTX_CFNAMEONLY:
		case IDM_CTX_CFFULLPATH:
		case IDM_CTX_CFDOSNAME:
			p = (LPTSTR)GlobalAllocPtr(GHND, (MAX_PATH + 3) * cnt);
			*p = 0;
			for (int i = 0; i < cnt; i++) {
				m_df.DragQueryFile(i, name);

				if (i > 0) lstrcat(p, _T("\r\n"));

				switch (idCommand) {
				case IDM_CTX_CFNAMEONLY:
					lstrcat(p, PathFindFileName(name));
					break;
				case IDM_CTX_CFFULLPATH:
					lstrcat(p, name);
					break;
				default:
					GetShortPathName(name, buf, ARRAYSIZE(buf));
					lstrcat(p, buf);
					break;
				}
			}
			CopyTextToClipboard(pci->hwnd, p, _ISKEYDOWN(VK_CONTROL));
			GlobalFreePtr(p);
			break;
		case IDM_CTX_REMOVEJUNCTION:
			m_df.DragQueryFile(0, name);
			fop.RemoveJunction(name);
			break;
		case IDM_CTX_OPENLINKTARGET:
		case IDM_CTX_OPENJUNCTION:
		{
			// ショートカットの場合、DragQueryFileして得られるのは
			// リンクターゲットのファイル名！
			m_df.DragQueryFile(0, name);
			if (idCommand == IDM_CTX_OPENJUNCTION) {
				DWORD type;
				LPTSTR psz;
				if (fop.GetReparsePointInfo(name, &psz, &type) != NOERROR)
					break;
				name = psz;
				CoTaskMemFree(psz);
			}

			LPITEMIDLIST pidl = ILCreateFromPath(name);
			if (pidl) {
				bool bNewTab = true;
				HWND hwnd = ::FindWindow(APP_FRAMECLASS, NULL);
				if (!hwnd) {
					TCHAR exe[MAX_PATH * 2];
					if (!lccRegGetAppPath(APP_EXENAME, exe, ARRAYSIZE(exe)))
						lstrcpy(exe, APP_EXENAME);

					// FileVisorの起動
					TCHAR path[MAX_PATH * 2];
					lstrcpy(path, name);
					PathRemoveFileSpec(path);
					if (ShellExecute(pci->hwnd, NULL, exe, path, path, SW_SHOW) > (HINSTANCE)32) {
						// ウィンドウ待ち
						DWORD timeout = GetTickCount() + 10000;			// 10sec
						while (!hwnd && (timeout > GetTickCount())) {
							hwnd = ::FindWindow(APP_FRAMECLASS, NULL);
							ZapMessage();
						}
						bNewTab = false;
					}
				}

				if (hwnd) {
					if (!::IsWindowVisible(hwnd))
						::ShowWindow(hwnd, SW_SHOWNORMAL);
					if (::IsIconic(hwnd))
						::ShowWindow(hwnd, SW_RESTORE);

					COPYDATASTRUCT cds;
					cds.dwData = CPD_JUMP_IDLIST;
					if (bNewTab)
						cds.dwData |= CPD_NEW_TAB;
					cds.cbData = ILGetSize(pidl);
					cds.lpData = pidl;
					::SendMessage(hwnd, WM_COPYDATA, 0, (LPARAM)&cds);
				}
				CoTaskMemFree(pidl);
			}
			break;
		}
		default:
			break;
		}
	}
	return S_OK;
}

// GetCommandString
HRESULT CCtx::GetCommandString(UINT_PTR idCmd, UINT uFlags, UINT *pwReserved, LPSTR pszText, UINT cchMax)
{
	return S_OK;
}


/////////////////////////////////////////////////////////////////////////////
// Methods of IContextMenu2

// constants used for drawing
const int CXGAP = 2;				// num pixels button and text
const int CXTEXTMARGIN = 2;			// num pixels after hilite to start text
const int CXBUTTONMARGIN = 4;		// num pixels wider button is than bitmap

HRESULT CCtx::HandleMenuMsg(UINT uMsg,WPARAM wParam, LPARAM lParam)
{
	LRESULT lr;
	return HandleMenuMsg2(uMsg, wParam, lParam, &lr);
}

HRESULT CCtx::HandleMenuMsg2(UINT uMsg, WPARAM wParam, LPARAM lParam, LRESULT* plResult)
{
	switch(uMsg) {
		case WM_INITMENUPOPUP:
			break;
		case WM_MEASUREITEM:
			MeasureItem((LPMEASUREITEMSTRUCT) lParam);
			break;
		case WM_DRAWITEM:
			DrawMenuItem((LPDRAWITEMSTRUCT) lParam);
			break;
		case WM_MENUCHAR:
			MenuChar((TCHAR)LOWORD(wParam), (UINT)HIWORD(wParam), (HMENU)lParam, plResult);
			break;
	}
	return S_OK;
}

void CCtx::MeasureItem(LPMEASUREITEMSTRUCT pms)
{
	if (pms->CtlType != ODT_MENU)
		return;					// 自分では処理しない

	UINT idCommand = m_arrCommand.Lookup(pms->itemID);
	CString str(MAKEINTRESOURCE(idCommand));

	// テキスト幅の計算:
	CClientDC dc(NULL);
	CRect rc(0, 0, 0, 0);

	HFONT hf = dc.SelectFont(_AtlModule.GetMenuFont());
	dc.DrawText(str, -1, &rc, DT_SINGLELINE | DT_LEFT | DT_VCENTER | DT_CALCRECT);
	dc.SelectFont(hf);

	// 項目の高さ
	pms->itemHeight= max(GetSystemMetrics(SM_CYMENU), max(SIZE_TOOLBUTTON, rc.Height()));

	// width is width of text plus a bunch of stuff
	int cx = rc.Width();				// text width 
	cx += CXGAP;
	cx += CXTEXTMARGIN << 1;			// L/R margin for readability
	cx += SIZE_TOOLBUTTON + (CXBUTTONMARGIN << 1);	// button width (L=button)
	pms->itemWidth = cx;				// done deal
}

void CCtx::DrawMenuItem(LPDRAWITEMSTRUCT pds)
{
	bool bGrayed = ((pds->itemState & (ODS_GRAYED | ODS_DISABLED)) != 0);
	bool bSelect = ((pds->itemState & ODS_SELECTED) != 0);

	UINT idCommand = m_arrCommand.Lookup(pds->itemID);
	CString str(MAKEINTRESOURCE(idCommand));

	CDCHandle dc = pds->hDC;
	CRect rc = pds->rcItem;

	COLORREF clrBkgnd = GetSysColor(bSelect ? COLOR_HIGHLIGHT : COLOR_MENU);
	COLORREF clrText = GetSysColor(bGrayed ? COLOR_GRAYTEXT : (bSelect ? COLOR_HIGHLIGHTTEXT : COLOR_MENUTEXT));
	dc.FillSolidRect(&rc, clrBkgnd);

	rc.DeflateRect(CXTEXTMARGIN, 0);

	COLORREF clr = dc.SetTextColor(clrText);
	int bkMode = dc.SetBkMode(TRANSPARENT);

	HIMAGELIST him = _AtlModule.GetCommandImageList();
	if (him) {
		int iImage = _AtlModule.GetCommandImageIndex(idCommand);
		ImageList_Draw(him, iImage, dc, rc.left + CXBUTTONMARGIN, rc.top + (rc.Height() - SIZE_TOOLBUTTON) / 2, ILD_TRANSPARENT);
	}
	rc.left += SIZE_TOOLBUTTON + (CXBUTTONMARGIN << 1) + CXGAP;
	dc.DrawText(str, -1, &rc, DT_LEFT | DT_SINGLELINE | DT_VCENTER);

	dc.SetBkMode(bkMode);
	dc.SetTextColor(clr);
}

void CCtx::MenuChar(TCHAR ch, UINT flags, HMENU hmenu, LRESULT* plResult)
{
	UINT idCommand = m_arrAccessKey.Lookup((TCHAR)CharLower((LPTSTR)ch));
	if (idCommand) {
		int cnt = GetMenuItemCount(hmenu);
		for (int i = 0; i < cnt; i++) {
			if (GetMenuItemID(hmenu, i) == idCommand) {
				*plResult = MAKELONG(i, MNC_EXECUTE);
				break;
			}
		}
	}
}

/////////////////////////////////////////////////////////////////////////////
// Methods of IShellExtInit

// Initialize
HRESULT CCtx::Initialize(LPCITEMIDLIST pidlFolder, IDataObject* pdo, HKEY hProgID)
{
	m_userlevel = _check_user();
	if (m_userlevel == USRL_NOUSER) return E_INVALIDARG;

	DWORD arc, etc, img;
	GetShellSetting(arc, etc, img);

	m_flags = 0;
	if (etc & SHLE_SUBMENU)		m_flags |= STS_SUBMENU;
	if (etc & SHLE_SYNC)		m_flags |= STS_SYNC;
	if (arc & SHLA_PACK)		m_flags |= STS_ARCPACK;
	if (arc & SHLA_CHECK)		m_flags |= STS_ARCCHECK;
	if (arc & SHLA_UNPACK)		m_flags |= STS_ARCUNPACK;
	if (etc & SHLE_BDUMP)		m_flags |= STS_BDUMP;
	if (etc & SHLE_DIV)			m_flags |= (STS_DIV | STS_MERGE);
	if (etc & SHLE_RENAME)		m_flags |= (STS_RENAME | STS_SERIESNM | STS_REPNAMES | STS_CHGCAPS);
	if (etc & SHLE_DFLUSH)		m_flags |= STS_DFLUSH;
	if (etc & SHLE_ATTR)		m_flags |= STS_ATTRIB;
	if (etc & SHLE_FILTERDEL)	m_flags |= STS_FILTERDEL;
	if (etc & SHLE_SHREDDER)	m_flags |= STS_SHREDDER;
	if (etc & SHLE_FSIZE)		m_flags |= STS_FSIZE;
	if (etc & SHLE_DRVINFO)		m_flags |= STS_DRVINFO;
	if (etc & SHLE_ICONDUMP)	m_flags |= (STS_ICONINFO | STS_ICONPICKER);
	if (etc & SHLE_FILEINFO)	m_flags |= STS_FILEINFO;
	if (etc & SHLE_PATHCLIP)	m_flags |= STS_PATHCLIP;
	if (etc & SHLE_CRYPT)		m_flags |= STS_CRYPT;
	if (etc & SHLE_CALCHASH)	m_flags |= STS_CALCHASH;

	if (!m_flags) return E_INVALIDARG;
	// Initialize can be called more than once
	if (pdo == NULL) return E_INVALIDARG;

	// Get the data as CF_HDROP
	STGMEDIUM sm;
	FORMATETC fe = {CF_HDROP, NULL, DVASPECT_CONTENT, -1, TYMED_HGLOBAL};
	if (FAILED(pdo->GetData(&fe, &sm))) return E_INVALIDARG;

	m_df.Attach((HDROP)DuplicateMemoryHandle(sm.hGlobal, GHND | GMEM_SHARE));
	ReleaseStgMedium(&sm);

bool packfile = true;

	TCHAR top[MAX_PATH * 2], buf[MAX_PATH * 2];
	int cnt = m_df.GetCount();
	for (int i = 0; i < cnt; i++) {
		m_df.DragQueryFile(i, buf, ARRAYSIZE(buf));
		if (i == 0) lstrcpy(top, buf);

		if (PathIsRoot(buf) || PathIsDirectory(buf)) {
			m_flags &= ~(STS_ARCUNPACK | STS_ARCCHECK | STS_BDUMP |
						 STS_DIV | STS_MERGE | STS_ICONINFO | STS_ICONPICKER | STS_FILEINFO |
						 STS_SHREDDER | STS_CRYPT | STS_CALCHASH);
			break;
		} else {
			m_flags &= ~(STS_SYNC | STS_FILTERDEL);
		}
		if (packfile) {
			// 拡張子が書庫か .exe 以外であれば、書庫解凍関連のコマンドは非表示
			if (SpecifyArchiveType(buf) == ARC_ERROR && lstrcmpi(PathFindExtension(buf), _T(".exe"))) {
				m_flags &= ~(STS_ARCUNPACK | STS_ARCCHECK);
				packfile = false;
			}
		}
	}

	if (cnt > 1)
		m_flags &= ~(STS_BDUMP | STS_DFLUSH | STS_SYNC | STS_DIV | STS_DRVINFO | STS_ICONINFO | STS_ICONPICKER | STS_FILEINFO);
	if (cnt == 1) {
		if (!PathIsRoot(top)) {
			m_flags &= ~(STS_DFLUSH | STS_DRVINFO);
			DWORD attr = GetFileAttributes(top);
			if (attr != 0xffffffff && attr & FILE_ATTRIBUTE_REPARSE_POINT) {
				m_flags |= STS_REPARSE;
			} else if (!(attr & FILE_ATTRIBUTE_DIRECTORY)) {
				LPCTSTR pext = PathFindExtension(top);
				if (StrCmpI(pext, _T(".exe")) && StrCmpI(pext, _T(".dll")) && StrCmpI(pext, _T(".cpl")) && StrCmpI(pext, _T(".ico"))) {
					m_flags &= ~(STS_ICONINFO | STS_ICONPICKER);
				}
			}
		} else {
			m_flags &= ~(STS_ARCPACK | STS_ARCUNPACK | STS_ARCCHECK |
						 STS_BDUMP | STS_DIV | STS_RENAME | STS_SERIESNM | STS_REPNAMES |
						 STS_CHGCAPS | STS_ATTRIB | STS_SHREDDER | STS_FILEINFO | STS_CALCHASH);
			if (m_flags & STS_DFLUSH) {
				int type = GetDriveType(top);
//@				if ((type != DRIVE_FIXED && type != DRIVE_REMOVABLE) || !checkmedia(top))
				if (type != DRIVE_FIXED && type != DRIVE_REMOVABLE)
					m_flags &= ~STS_DFLUSH;
			}
			if (top[0] == '\\')
				m_flags &= ~STS_DRVINFO;
		}
	}

	// duplicate the object pointer and registry handle
	return NOERROR;
}

STDMETHODIMP CCtx::Setup(HWND hwnd)
{
	DoSetup(hwnd);
	return S_OK;
}
