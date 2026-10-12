///////////////////////////////////////////////////////////////////
// CIdlBar Class
//

#pragma once
#ifndef __IDLBAR_H__
#define __IDLBAR_H__

#include "resource.h"
#include "dropper.h"
#include "..\lscore\tbb_customdraw.h"


///////////////////////////////////////////////////////////////////
// PANEINFO

#define IDL_HIWORD				0xFFFF0000
#define IDL_LOWORD				0x0000FFFF
#define IDL_OPEN				0x00000000
#define IDL_OPENBYFV			0x00000001
#define IDL_OPENTAB				0x00000002
#define IDL_OPENNEWFRAME		0x00000003
#define IDL_OPENMASK			0x0000000F
#define IDL_FOLDERMENU			0x00000010
#define IDL_FOLDERRETRIEVE		0x00000020
#define IDL_FOLDERMASK			0x000000F0
#define IDL_SHOWLABEL			0x00001000

class CIdlPane
{
public:
	CIDListPtr m_idl;
	SFGAOF m_attr;
	int m_iIcon;
	CString m_strDisplayName;
	CString m_strIconLocation;
	CString m_strInfoTip;
	DWORD m_opmode;
	CBitmap m_bmIcon;

public:
	CIdlPane();
	CIdlPane(LPCITEMIDLIST pidl, DWORD opmode = IDL_OPEN);
	CIdlPane(LPCTSTR path, DWORD opmode = IDL_OPEN);

	void ParseIcon();
	LPCTSTR GetDisplayName();

	inline bool isFolder() { return ((m_attr & SFGAO_FOLDER) != 0); }
	inline bool isNull() { return m_idl.isNull(); }
	inline bool isDropTarget() { return ((m_attr & SFGAO_DROPTARGET) != 0); }
	inline bool HasPropSheet() { return ((m_attr & SFGAO_HASPROPSHEET) != 0); }
	inline bool isFolderMenu() { return ((m_opmode & IDL_FOLDERMENU) != 0); }
	inline bool isFolderRetrieve() { return ((m_opmode & IDL_FOLDERRETRIEVE) != 0); }
	inline bool isTreatMenu() { return ((m_opmode & IDL_FOLDERMASK) != 0); }
	inline bool isShowLabel() { return ((m_opmode & IDL_SHOWLABEL) != 0); }
	inline BOOL OpenMode() { return (m_opmode & IDL_OPENMASK); }
	inline bool isOpenByFV() { return (OpenMode() == IDL_OPENBYFV); }
	inline void* GetUIObjectOf(HWND hWnd, REFIID riid) { return m_idl.GetUIObjectOf(hWnd, riid); }
	inline IDataObject* GetDataObject(HWND hWnd) { return (IDataObject*)GetUIObjectOf(hWnd, IID_IDataObject); }
	inline IContextMenu* GetContextMenuObject(HWND hWnd) { return (IContextMenu*)GetUIObjectOf(hWnd, IID_IContextMenu); }
	inline IDropTarget* GetDropTarget(HWND hWnd) { return (IDropTarget*)GetUIObjectOf(hWnd, IID_IDropTarget); }
};



///////////////////////////////////////////////////////////////////
// CIdlBar control

#define IDLN_CHANGED		0x0001


class CIdlBar :	public CWindowImpl<CIdlBar, CToolBarCtrl>,
				public TDropperImpl<CIdlBar>,
				public CTBarCustomDrawImpl<CIdlBar>
{
private:
	CAtlArray<CIdlPane*> m_array;
	HIMAGELIST m_himSystem;
	CSize m_sizeIcon;

public:
	DECLARE_WND_SUPERCLASS(_T("SmartBar"), GetWndClassName())

	CIdlBar() : m_himSystem(NULL) { }
	~CIdlBar() { WepAllItems(); }

BEGIN_MSG_MAP(CIdlBar)
	CHAIN_MSG_MAP(TDropperImpl<CIdlBar>)
	CHAIN_MSG_MAP(CTBarCustomDrawImpl<CIdlBar>)
	ON_WM_CREATE()
	ON_WM_RBUTTONDOWN()
	ON_WM_COMMAND()
	ON_NOTIFY_CODE(TTN_GETDISPINFO, OnToolTipText)
	ON_NOTIFY(TBN_QUERYINSERT, IDW_SMARTBAR, OnNotifyQueryInsert)
	ON_NOTIFY(TBN_QUERYDELETE, IDW_SMARTBAR, OnNotifyQueryDelete)
	ON_NOTIFY(TBN_GETBUTTONINFO, IDW_SMARTBAR, OnNotifyGetButtonInfo)
	ON_NOTIFY(TBN_ENDADJUST, IDW_SMARTBAR, OnNotifyEndAdjust)
	if (uMsg == WM_NOTIFY) {
		bHandled = TRUE;
		lResult = 0;
		return TRUE;
	}
	MESSAGE_HANDLER(TB_GETBUTTONINFO, OnHandlerTBGetButtonInfo)
END_MSG_MAP()

	DWORD OnDragOver(IDataObject* pdo, DWORD keyState, DWORD dropEffect, POINT point);
	DWORD OnDrop(IDataObject* pdo, DWORD keyState, DWORD dropEffect, POINT point);
	void OnDragLeave();

	int OnCreate(LPCREATESTRUCT pcs);
	void OnRButtonDown(UINT flags, CPoint point);
	BOOL OnCommand(UINT id, UINT code, HWND hwnd);
	void OnToolTipText(int id, NMHDR* pNMHDR, LRESULT* pResult);
	void OnNotifyGetButtonInfo(NMHDR* pNmhdr, LRESULT* pResult);
	void OnNotifyQueryInsert(NMHDR* pNmhdr, LRESULT* pResult);
	void OnNotifyQueryDelete(NMHDR* pNmhdr, LRESULT* pResult);
	void OnNotifyEndAdjust(NMHDR* pNmhdr, LRESULT* pResult);
	LRESULT OnHandlerTBGetButtonInfo(UINT msg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);

	int DoToolBarDropDown(TBNOTIFY* ptbn);
	HBITMAP GetMenuImageBitmap(UINT id);

	void Refresh();
	void LoadItems();
	bool SaveItems();
	void ChangeButtonImageSize();

protected:
	void InitButtonImages();
	void DeleteAllButtons();
	CIdlPane* GetItemData(INT_PTR index);
	void WepAllItems();
	void BuildButtons(bool needSerialize = false);
	void OpenItem(LPITEMIDLIST pidl, int openMode);
};


#endif			// __IDLBAR_H__
