#include "stdafx.h"
#include "lscore.h"
#include "common.h"
#include "idlist.h"
#include "fnc.h"
#include "resource.h"

#include "picker.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


class CIDListPickerCtrl : public CPickerCtrlImpl<CIDListPickerCtrl>
{
private:
	LPITEMIDLIST m_pidl;
	CString m_strPath;

public:
	CIDListPickerCtrl();
	~CIDListPickerCtrl();

BEGIN_MSG_MAP(CIDListPickerCtrl)
	CHAIN_MSG_MAP(CPickerCtrlImpl<CIDListPickerCtrl>)
	ON_WM_CREATE()
	ON_MESSAGE(IPM_GETIDLIST, OnGetIDList)
	ON_MESSAGE(IPM_SETIDLIST, OnSetIDList)
END_MSG_MAP()

	int OnCreate(LPCREATESTRUCT pcs);
	inline LRESULT OnGetIDList(WPARAM wParam, LPARAM lParam) { return (LRESULT)m_pidl; }
	inline LRESULT OnSetIDList(WPARAM wParam, LPARAM lParam) { SetIDList((LPITEMIDLIST)lParam); return 0; }

	void SetIDList(LPITEMIDLIST pidl);
	void Browse();
};

CIDListPickerCtrl::CIDListPickerCtrl()
{
	m_pidl = NULL;
	HIMAGELIST him = lccLoadImageList(RESINST, RT_RCDATA, MAKEINTRESOURCE(IDB_PICKER_FOLDER), 14, RGB(255, 0, 255));
	SetButtonImageList(him);
}

CIDListPickerCtrl::~CIDListPickerCtrl()
{
	if (m_pidl) CoTaskMemFree(m_pidl);
}

int CIDListPickerCtrl::OnCreate(LPCREATESTRUCT pcs)
{
	HIMAGELIST himL, himS;
	Shell_GetImageLists(&himL, &himS);
	SetLabelImageList(himS);

	m_strDisplayName.LoadString(IDS_UNDEFINED);
	m_strPath.Empty();
	return 0;
}

#define DONT_DISMISS		1

static int CALLBACK bcproc(HWND hwnd, UINT msg, LPARAM lParam, LPARAM lpData)
{
	switch (msg) {
	case BFFM_INITIALIZED:
		if (lpData)
			SendMessage(hwnd, BFFM_SETSELECTION, FALSE, lpData);
		break;

	case BFFM_SELCHANGED:
		break;

	case BFFM_VALIDATEFAILED:
//		return DONT_DISMISS;
		return 0;

	default:
		break;
	}
	return 0;
}

void CIDListPickerCtrl::Browse()
{
	TCHAR title[MAX_PATH];
	BROWSEINFO bi;
	memset(&bi, 0, sizeof(bi));
	bi.hwndOwner = m_hWnd;
	bi.pidlRoot = NULL;
	bi.pszDisplayName = title;
	bi.lpszTitle = _T("フォルダを選択してください。");
			// BIF_NOTRANSLATETARGETS の指定を含めないとマイドキュメント（仮想）が取得できない
	bi.ulFlags = BIF_NEWDIALOGSTYLE | BIF_NONEWFOLDERBUTTON | BIF_NOTRANSLATETARGETS | BIF_UAHINT | BIF_EDITBOX | BIF_VALIDATE;
	bi.lpfn = bcproc;
	bi.lParam = (LPARAM)m_pidl;

	TCHAR dir[MAX_PATH * 2];
	GetCurrentDirectory(ARRAYSIZE(dir), dir);
	LPITEMIDLIST pidl = SHBrowseForFolder(&bi);
	SetCurrentDirectory(dir);

	if (pidl) {
		SetIDList(pidl);
		CoTaskMemFree(pidl);
	}
}

void CIDListPickerCtrl::SetIDList(LPITEMIDLIST pidl)
{
	if (m_pidl) {
		CoTaskMemFree(m_pidl);
		m_pidl = NULL;
	}
	m_iLabelImage = -1;

	if (pidl) {
		m_pidl = ILClone(pidl);
		SHFILEINFO sfi;
		SHGetFileInfo((LPCTSTR)m_pidl, 0, &sfi, sizeof(sfi),
									SHGFI_PIDL | SHGFI_DISPLAYNAME |
									SHGFI_SYSICONINDEX | SHGFI_SMALLICON);
		m_iLabelImage = sfi.iIcon;

		TCHAR buf[MAX_PATH * 2];
		if (!SHGetPathFromIDList(m_pidl, buf))
			m_strPath.Empty();
		else {
			// ターゲットフォルダがリンク属性を持っていることがあるので
			// 実際のリンク先を取得する
			m_strPath = buf;
			SHFILEINFO sfi;
			sfi.dwAttributes = SFGAO_CAPABILITYMASK | SFGAO_LINK;
			SHGetFileInfo((LPCTSTR)pidl, 0, &sfi, sizeof(sfi), SHGFI_ATTRIBUTES | SHGFI_ATTR_SPECIFIED);
			if (sfi.dwAttributes & SFGAO_LINK) {
				if (parselinkpath(m_strPath, buf, ARRAYSIZE(buf)))
					m_strPath = buf;
				else
					m_strPath.Empty();
			}
		}

		if (GetStyle() & 1 && !m_strPath.IsEmpty())
			m_strDisplayName.Format(_T("%s - %s"), sfi.szDisplayName, (LPCTSTR)m_strPath);
		else
			m_strDisplayName = sfi.szDisplayName;
	}
	else {
		m_strDisplayName.LoadString(IDS_UNDEFINED);
		m_strPath.Empty();
	}

	InvalidateRect(&m_rcText, FALSE);
	UpdateWindow();

	GetParent().SendMessage(WM_COMMAND, MAKEWPARAM(GetDlgCtrlID(), CBN_SELCHANGE), (LPARAM)m_hWnd);
}

////////////////////////////////////////////////////////////////////////
//

extern "C"
LRESULT CALLBACK IDListPickWnd(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	LRESULT lr = DefWindowProc(hwnd, msg, wParam, lParam);
	if (msg == WM_NCCREATE) {
		CIDListPickerCtrl* pWnd = new CIDListPickerCtrl();
		pWnd->SubclassWindow(hwnd);
	}
	return lr;
}
