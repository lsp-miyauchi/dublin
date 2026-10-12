///////////////////////////////////////////////////////////////////
// CDriveBar Class
//

#pragma once
#ifndef __DRIVE_H__
#define __DRIVE_H__


#include "resource.h"
#include "dropper.h"
#include "..\lscore\tbb_customdraw.h"

HBITMAP CreateBitmapFromImageList(HIMAGELIST him, int iImage);

/////////////////////////////////////////////////////////////////////////////
// CDrivePidl

class CDrivePidl : public CIDListPtr
{
public:
	SFGAOF m_attr;
	int m_iImage;
	int m_iOverlayImage;
	CString m_strPathName;
	CString m_strDisplayName;
	TCHAR m_prefix;
	CBitmap m_bmIcon;
	bool m_bCustomIcon;

public:
	CDrivePidl()
		: m_attr(0)
		, m_iImage(-1)
		, m_iOverlayImage(-1)
		, m_bCustomIcon(false)
	{
	}

	CDrivePidl(IShellFolder* psf, LPITEMIDLIST pidl, SFGAOF attr, LPCTSTR pszName) : m_bCustomIcon(false)
	{
		m_pidl = idl_GetFullyQualPidl(psf, pidl);
		m_attr = attr;
		m_iImage = SHMapPIDLToSystemImageListIndex(psf, pidl, NULL);
		m_iOverlayImage = 0;

		CComQIPtr<IShellIconOverlay> psio = psf;
		if (psio.p)
			psio->GetOverlayIndex(pidl, &m_iOverlayImage);

		if (!pszName) {
			if (!GetPath(m_strPathName))
				idl_GetName(psf, pidl, SHGDN_FORPARSING, m_strPathName);
		}
		else
			m_strPathName = pszName;

		int num = PathGetDriveNumber(m_strPathName);
		if (num >= 0)
			m_prefix = m_strPathName[0];
	}

	CDrivePidl(LPCTSTR pszPath, LPCTSTR pszIconFile, int index) : m_bCustomIcon(true)
	{
		m_strPathName = pszPath;
		m_attr = 0;
		if (pszIconFile && *pszIconFile)
			m_iImage = Shell_GetCachedImageIndex(pszIconFile, index, FALSE);
		else {
			// 正確なアイコンではなくて、標準のアイコンを取得
			SHFILEINFO sfi;
			SHGetFileInfo(pszPath, FILE_ATTRIBUTE_DIRECTORY, &sfi, sizeof(sfi), SHGFI_USEFILEATTRIBUTES | SHGFI_SYSICONINDEX);
			m_iImage = sfi.iIcon;
		}
		m_iOverlayImage = 0;
	}

	~CDrivePidl()
	{
	}

	const CDrivePidl& copyfrom(const CDrivePidl& src)
	{
		if (&src != this) {
			Clone(src);
			m_attr = src.m_attr;
			m_iImage = src.m_iImage;
			m_iOverlayImage = src.m_iOverlayImage;
			m_strPathName = src.m_strPathName;
			m_strDisplayName = src.m_strDisplayName;
			m_prefix = src.m_prefix;
		}
		return *this;
	}

	inline const CDrivePidl& operator=(const CDrivePidl& src) { return copyfrom(src); }

	LPCTSTR GetDisplayName()
	{
		if (m_strDisplayName.IsEmpty()) {
			SHFILEINFO sfi;
			if (!isEmpty()) {
				GetFileInfo(0, &sfi, sizeof(sfi), SHGFI_DISPLAYNAME);
				m_strDisplayName = sfi.szDisplayName;
			}
			else if (!m_strPathName.IsEmpty()) {
				SHGetFileInfo(m_strPathName, 0, &sfi, sizeof(sfi), SHGFI_USEFILEATTRIBUTES | SHGFI_DISPLAYNAME);
				m_strDisplayName = sfi.szDisplayName;
			}
		}
		return (m_strDisplayName.IsEmpty() ? NULL : (LPCTSTR)m_strDisplayName);
	}

	void ResetImageIndex()
	{
		if (!m_bCustomIcon) {
			m_iImage = -1;
			m_iOverlayImage = 0;
		}
	}

	int GetImageIndex()
	{
		if (!m_bCustomIcon && m_iImage < 0) {
			CComPtr<IShellFolder> psf;
			LPCITEMIDLIST pidl;
			if (SHBindToParent(m_pidl, IID_PPV_ARGS(&psf), &pidl) == S_OK) {
				m_iImage = SHMapPIDLToSystemImageListIndex(psf, pidl, NULL);
				m_iOverlayImage = 0;
				CComQIPtr<IShellIconOverlay> psio = psf;
				if (psio.p)
					psio->GetOverlayIndex(pidl, &m_iOverlayImage);
			}
		}
		return m_iImage;
	}
};


///////////////////////////////////////////////////////////////////
// LOCAL MESSAGES
//

#define DBN_FIRST               (100U)
#define DBN_LAST                (199U)

typedef struct tagDRIVENOTIFY {
	NMHDR hdr;
	int index;
	UINT flags;
	TCHAR path[MAX_PATH * 2];
} DRIVENOTIFY;
typedef DRIVENOTIFY FAR *LPDRIVENOTIFY;

#define DBN_LBUTTONDOWN			(DBN_FIRST + 0U)
#define DBN_DROPFILES			(DBN_FIRST + 1U)
#define DBN_NEWTAB				(DBN_FIRST + 2U)
#define DBN_NEWWINDOW			(DBN_FIRST + 3U)
#define DBN_ADDFAVORITE			(DBN_FIRST + 4U)

#define MAX_DRIVE_ITEM			30

///////////////////////////////////////////////////////////////////
// CDriveBar control

#define DBM_RESETDRIVES			(WM_USER + 440)
#define DBM_SELECTDRIVE			(WM_USER + 441)
#define DBM_GETCURRENTDRIVE		(WM_USER + 442)
#define DBM_UPDATEITEM			(WM_USER + 443)


class CDriveBar :	public CWindowImpl<CDriveBar, CToolBarCtrl>,
					public TDropperImpl<CDriveBar>,
					public CTBarCustomDrawImpl<CDriveBar>
{
private:
	CSimpleArray<CDrivePidl*> m_array;
	CSize m_sizeIcon;
	int m_nCurDrive;
	UINT_PTR m_timerid;
	UINT m_lastclick_id;
	bool m_bVertical;
	HIMAGELIST m_himSystem;

public:
	DECLARE_WND_SUPERCLASS(_T("DriveBar"), GetWndClassName())

	CDriveBar()
		: m_nCurDrive(-1)
		, m_timerid(0)
		, m_lastclick_id(0)
		, m_bVertical(false)
		, m_himSystem(NULL)
	{ }
	~CDriveBar()
	{
		WepAllItems();
	}

public:
	HWND CreateBar(HWND hParent, UINT idCtrl, bool bVertical);

	LPCITEMIDLIST GetCurrentDrive()
	{
		ATLASSERT(::IsWindow(m_hWnd));
		return (LPCITEMIDLIST)::SendMessage(m_hWnd, DBM_GETCURRENTDRIVE, 0, 0);
	}
	void SelectDrive(LPCITEMIDLIST pidlFullyQual)
	{
		ATLASSERT(::IsWindow(m_hWnd));
		::SendMessage(m_hWnd, DBM_SELECTDRIVE, 0, (LPARAM)pidlFullyQual);
	}
	void ResetDrives()
	{
		ATLASSERT(::IsWindow(m_hWnd));
		::SendMessage(m_hWnd, DBM_RESETDRIVES, 0, 0);
	}
	void UpdateItem(LPCITEMIDLIST pidlFullyQual)
	{
		ATLASSERT(::IsWindow(m_hWnd));
		::SendMessage(m_hWnd, DBM_UPDATEITEM, 0, (LPARAM)pidlFullyQual);
	}
	__inline bool isVertical() const { return m_bVertical; }
	void SetVertical(bool bVertical);


BEGIN_MSG_MAP(CDriveBar)
	CHAIN_MSG_MAP(TDropperImpl<CDriveBar>)
	CHAIN_MSG_MAP(CTBarCustomDrawImpl<CDriveBar>)
	MESSAGE_HANDLER(TB_GETBUTTONINFO, OnHandlerToolbarGetButtonInfo)
	ON_WM_CREATE()
	ON_WM_RBUTTONDOWN()
	ON_WM_MBUTTONDBLCLK()
	ON_WM_COMMAND()
	MESSAGE_HANDLER(WM_QUERYIMAGELIST, OnHandlerQueryImageList)
	MESSAGE_HANDLER(WM_TIMER, OnTimer)
	ON_NOTIFY_CODE(TTN_GETDISPINFO, OnToolTipText)
	ON_MESSAGE_VOID(DBM_RESETDRIVES, OnResetDrives)
	ON_MESSAGE_VOID(DBM_SELECTDRIVE, OnSelectDrive)
	ON_MESSAGE_VOID(DBM_UPDATEITEM, OnUpdateItem)
	ON_MESSAGE(DBM_GETCURRENTDRIVE, OnGetCurDrive)
	if (uMsg == WM_NOTIFY) {
		bHandled = TRUE;
		lResult = 0;
		return TRUE;
	}

END_MSG_MAP()

	DWORD OnDragOver(IDataObject* pdo, DWORD keyState, DWORD dropEffect, POINT point);
	DWORD OnDrop(IDataObject* pdo, DWORD keyState, DWORD dropEffect, POINT point);
	void OnDragLeave();
	int QeuryOverlayImageIndex(UINT id);

	int GetDriveItemCount() { return (int)m_array.GetSize(); }
	HBITMAP GetMenuImageBitmap(UINT id);
	void ChangeButtonImageSize();

protected:
	int OnCreate(LPCREATESTRUCT pcs);
	void OnRButtonDown(UINT flags, CPoint point);
	void OnMButtonDblClk(UINT flags, CPoint point);
	void OnToolTipText(int id, NMHDR* pNMHDR, LRESULT* pResult);
	LRESULT OnTimer(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT OnHandlerQueryImageList(UINT msg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	BOOL OnCommand(UINT id, UINT code, HWND hwnd);
	LRESULT OnHandlerToolbarGetButtonInfo(UINT msg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	__inline void OnResetDrives(WPARAM, LPARAM lParam) { DriveReset(); }
	__inline void OnSelectDrive(WPARAM, LPARAM lParam) { DriveSelect((LPCITEMIDLIST)lParam); }
	__inline void OnUpdateItem(WPARAM, LPARAM lParam) { UpdateItemImage((LPCITEMIDLIST)lParam); }
	__inline LRESULT OnGetCurDrive(WPARAM, LPARAM) { return (LRESULT)GetCurDrive(); }

	LPCITEMIDLIST GetCurDrive();
	void DriveSelect(LPCITEMIDLIST pidlFq);
	void DriveReset();

	void SendNotify(UINT id, UINT code);
	void InitButtonImages();
	void BuildButtons();
	void UpdateItemImage(LPCITEMIDLIST pidl);
	void DeleteAllButtons();

	CDrivePidl* GetItemData(UINT_PTR idx) const
	{
		UINT_PTR cnt = m_array.GetSize();
		return (cnt > 0 && idx >= 0 && idx < cnt) ? m_array[(int)idx] : NULL;
	}

	void WepAllItems();
};

#endif			// __DRIVE_H__
