////////////////////////////////////////////////////////////////
// shellexec.cpp

#include "stdafx.h"
#include "resource.h"
#include "data.h"
#include "common.h"
#include "func.h"
#include "idlist.h"
#include "fileitem.h"
#include "dragdrop.h"
#include "cmwnd.h"
#include "shellexec.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
//@ #define new DEBUG_NEW
#endif


BOOL __fastcall InvokeFile(HWND hParent, LPCTSTR pszFile, LPCTSTR pszParam, LPCTSTR pszDir, LPCTSTR lpVerb)
{
	TCHAR path[MAX_PATH * 2];
	if (!pszDir || *pszDir == '\0') {
		lstrcpy(path, pszFile);
		PathRemoveFileSpec(path);
	} else {
		lstrcpy(path, pszDir);
	}
	SHELLEXECUTEINFO sei = {0};
	sei.cbSize = sizeof(SHELLEXECUTEINFO);
	sei.fMask = SEE_MASK_UNICODE;
	sei.hwnd = (hParent && IsWindow(hParent)) ? hParent : NULL;
	sei.nShow = SW_SHOWDEFAULT;
	sei.lpVerb = lpVerb;
	sei.lpFile = pszFile;
	sei.lpParameters = pszParam;
	sei.lpDirectory = path;
	if (sei.lpVerb == NULL && sei.lpParameters == NULL) {
		// コンテキストメニュー内のデフォルトverbを実行
		sei.fMask |= SEE_MASK_INVOKEIDLIST;
	}
	return ShellExecuteEx(&sei);
}

BOOL __fastcall InvokeIDList(HWND hParent, LPCITEMIDLIST pidl, LPCTSTR lpVerb)
{
	SHELLEXECUTEINFO sei;
	memset(&sei, 0, sizeof(SHELLEXECUTEINFO));
	sei.cbSize = sizeof(SHELLEXECUTEINFO);
	sei.fMask = SEE_MASK_IDLIST | SEE_MASK_INVOKEIDLIST | SEE_MASK_UNICODE;
	sei.hwnd = (hParent && IsWindow(hParent)) ? hParent : NULL;
	sei.nShow = SW_SHOWDEFAULT;
	sei.lpIDList = (void*)pidl;
	sei.lpVerb = lpVerb;

	CString str;
	LPTSTR p = str.GetBuffer(MAX_PATH * 2);
	if (SHGetPathFromIDList(pidl, p) && !_pathisdirectory(p)) {
		PathRemoveFileSpec(p);
		str.ReleaseBuffer();
		sei.lpDirectory = str;
	}
	BOOL bOK = ShellExecuteEx(&sei);
	if (bOK)
		bOK = (sei.hInstApp > (HINSTANCE)32);
	return bOK;
}

BOOL __fastcall ctx_Execute(HWND hwnd, IContextMenu* pcm, LPCTSTR pVerb)
{
	// Execute the command that was selected.
	char verb[128];
	WideCharToMultiByte(CP_ACP, 0, pVerb, -1, verb, ARRAYSIZE(verb), NULL, NULL);

	CMINVOKECOMMANDINFOEX cmi;
	memset(&cmi, 0, sizeof(cmi));
	cmi.cbSize  = sizeof(cmi);
	cmi.hwnd    = hwnd;
	cmi.nShow   = SW_SHOWNORMAL;
	cmi.fMask   = CMIC_MASK_UNICODE;
	cmi.lpVerbW = pVerb;
	cmi.lpVerb  = verb;
	return SUCCEEDED(pcm->InvokeCommand((CMINVOKECOMMANDINFO*)&cmi)) ? TRUE : FALSE;
}


BOOL __fastcall SendTo(HWND hParent, IDataObject* pdt, CPoint pt)
{
	BOOL bOK = FALSE;
	CMenu pop; pop.CreatePopupMenu();
	CCmWnd wnd(pop, CSIDL_SENDTO, TRUE);

	// ハードディスク以外を追加
	CIDListPtr idlDrives(NULL, CSIDL_DRIVES);
	CComPtr<IShellFolder> psf = idlDrives.GetShellFolder();
	if (psf.p != NULL) {
		CComPtr<IEnumIDList> pe;
		if (psf->EnumObjects(NULL, SHCONTF_FOLDERS | SHCONTF_FASTITEMS, &pe) == S_OK) {
			LPITEMIDLIST pidl;
			while (pe->Next(1, &pidl, NULL) == NOERROR) {
				int descid = idl_GetDescriptionId(psf, pidl);
				if ((SHDID_COMPUTER_DRIVE35 <= descid) &&
					(descid <= SHDID_COMPUTER_RAMDISK) &&
					descid != SHDID_COMPUTER_FIXED) {
					CIDListPtr idl;
					idl.Combine(idlDrives, pidl);
					CCmMenuItem* pci = new CCmMenuItem(idl);
					wnd.AddMenuItem(pop, pci);
				}
				CoTaskMemFree(pidl);
			}
		}
	}

	// ポップメニュー実行
	int idCmd = wnd.DoTrack(hParent, pop, pt);
	if (idCmd > 0) {
		CCmItem* pci = wnd.FindItem(idCmd);
		if (pci && !pci->m_idl.isNull())
			bOK = idl_SendTo(hParent, pci->m_idl, pdt);
	}
	return bOK;
}

BOOL __fastcall ShellNewMenu(HWND hwnd, LPITEMIDLIST pidlFolder, LPPOINT ppt)
{
	// シェルの新規作成メニューを表示する
	BOOL bOK = FALSE;
	CComPtr<IShellExtInit> psei;
	if (psei.CoCreateInstance(CLSID_NewMenu, NULL, CLSCTX_INPROC) == S_OK) {
		if (psei->Initialize(pidlFolder, NULL, NULL) == S_OK) {
			CComQIPtr<IContextMenu> pcm = psei;
			if (pcm.p != NULL) {
				TrackContextMenu(hwnd, pcm, ppt);
				bOK = TRUE;
			}
		}
	}
	return bOK;
}
