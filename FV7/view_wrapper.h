////////////////////////////////////////////////////////////////////////
// view_wrapper.h

#pragma once
#ifndef __VIEW_WRAPPER_H__
#define __VIEW_WRAPPER_H__

////////////////////////////////////////////////////////////////////////
// CFolderTreeWindow

#define WNDCLASS_FOLDERTREE			_T("FolderTree")

#define TFM_UPDATEVIEW				(WM_USER + 400)
#define TFM_SETOWNER				(WM_USER + 401)
#define TFM_SELECTPARENT			(WM_USER + 402)
#define TFM_SELECTCHILD				(WM_USER + 403)
#define TFM_SELECTFOLDER			(WM_USER + 404)
#define TFM_HASPARENT				(WM_USER + 405)
#define TFM_ADDFOLDER				(WM_USER + 406)
#define TFM_DELETEFOLDER			(WM_USER + 407)
#define TFM_UPDATEFOLDER			(WM_USER + 408)
#define TFM_UPDATESYSIMAGE			(WM_USER + 409)
#define TFM_GETSELECTIONCENTERPOINT	(WM_USER + 410)
#define TFM_EXPANDCURRENTFOLDER		(WM_USER + 411)
#define TFM_REFRESHTREE				(WM_USER + 412)
#define TFM_GETCURRENTIDLIST		(WM_USER + 413)
#define TFM_GETCURRENTPATH			(WM_USER + 414)
#define TFM_ENUMFOLDER				(WM_USER + 415)
#define TFM_GETCLASSBODY			(WM_USER + 416)
#define TFM_SAVESETTINGS			(WM_USER + 417)
#define TFM_SETROOT					(WM_USER + 418)
#define TFM_GETROOT					(WM_USER + 419)

#define TFN_SELCHANGED				1
#define TFN_ENTERFILEOPERATION		2


template <class TBase>
class CFolderTreeWindowT : public TBase
{
public:
// Constructors
	CFolderTreeWindowT(HWND hWnd = NULL) : TBase(hWnd)
	{
	}
	CFolderTreeWindowT< TBase >& operator=(HWND hWnd)
	{
		m_hWnd = hWnd;
		return *this;
	}
	HWND SetOwner(HWND hwndOwner)
	{
		ATLASSERT(::IsWindow(m_hWnd));
		return (HWND)::SendMessage(m_hWnd, TFM_SETOWNER, 0, (LPARAM)hwndOwner);
	}
	PCIDLIST_ABSOLUTE GetRoot()
	{
		ATLASSERT(::IsWindow(m_hWnd));
		return (PCIDLIST_ABSOLUTE)::SendMessage(m_hWnd, TFM_GETROOT, 0, 0);
	}
	void SetRoot(PCIDLIST_ABSOLUTE pidlRoot)
	{
		ATLASSERT(::IsWindow(m_hWnd));
		::SendMessage(m_hWnd, TFM_SETROOT, 0, (LPARAM)pidlRoot);
	}
	void UpdateView(WPARAM wHint = 0, LPARAM lHint = 0)
	{
		ATLASSERT(::IsWindow(m_hWnd));
		::PostMessage(m_hWnd, TFM_UPDATEVIEW, wHint, lHint);
	}
	BOOL SelectParent()
	{
		ATLASSERT(::IsWindow(m_hWnd));
		return (BOOL)::SendMessage(m_hWnd, TFM_SELECTPARENT, 0, 0);
	}
	BOOL SelectChild(LPCITEMIDLIST pidlSingle)
	{
		ATLASSERT(::IsWindow(m_hWnd));
		return (BOOL)::SendMessage(m_hWnd, TFM_SELECTCHILD, 0, (LPARAM)pidlSingle);
	}
	BOOL SelectFolder(LPCITEMIDLIST pidlFullyQual)
	{
		ATLASSERT(::IsWindow(m_hWnd));
		return (BOOL)::SendMessage(m_hWnd, TFM_SELECTFOLDER, 0, (LPARAM)pidlFullyQual);
	}
	BOOL HasParent(HTREEITEM hItem = NULL)
	{
		ATLASSERT(::IsWindow(m_hWnd));
		return (BOOL)::SendMessage(m_hWnd, TFM_HASPARENT, 0, (LPARAM)hItem);
	}
	BOOL AddFolder(LPCITEMIDLIST pidl)
	{
		ATLASSERT(::IsWindow(m_hWnd));
		return (BOOL)::SendMessage(m_hWnd, TFM_ADDFOLDER, 0, (LPARAM)pidl);
	}
	BOOL DeleteFolder(LPCITEMIDLIST pidl)
	{
		ATLASSERT(::IsWindow(m_hWnd));
		return (BOOL)::SendMessage(m_hWnd, TFM_DELETEFOLDER, 0, (LPARAM)pidl);
	}
	BOOL UpdateFolder(LPCITEMIDLIST pidl, BOOL bSweepSubFolder = FALSE)
	{
		ATLASSERT(::IsWindow(m_hWnd));
		return (BOOL)::SendMessage(m_hWnd, TFM_UPDATEFOLDER, (WPARAM)bSweepSubFolder, (LPARAM)pidl);
	}
	void GetSelectionCenterPoint(POINT* ppt)
	{
		ATLASSERT(::IsWindow(m_hWnd));
		::SendMessage(m_hWnd, TFM_GETSELECTIONCENTERPOINT, 0, (LPARAM)ppt);
	}
	void RefreshTree(PCIDLIST_ABSOLUTE pidlRoot = NULL)
	{
		ATLASSERT(::IsWindow(m_hWnd));
		::SendMessage(m_hWnd, TFM_REFRESHTREE, 0, (LPARAM)pidlRoot);
	}
	PCIDLIST_ABSOLUTE GetCurrentIDList()
	{
		// 返されたpidlはfreeする必要有り
		ATLASSERT(::IsWindow(m_hWnd));
		return (PCIDLIST_ABSOLUTE)::SendMessage(m_hWnd, TFM_GETCURRENTIDLIST, 0, 0);
	}
	int GetCurrentPath(LPTSTR pszPath, int cchMax)
	{
		ATLASSERT(::IsWindow(m_hWnd));
		return (int)::SendMessage(m_hWnd, TFM_GETCURRENTPATH, (WPARAM)cchMax, (LPARAM)pszPath);
	}
	BOOL EnumFolder(LPCITEMIDLIST pidl, BOOL bNeedUI)
	{
		ATLASSERT(::IsWindow(m_hWnd));
		return (BOOL)::SendMessage(m_hWnd, TFM_ENUMFOLDER, (WPARAM)bNeedUI, (LPARAM)pidl);
	}
	void* GetClassBody()
	{
		ATLASSERT(::IsWindow(m_hWnd));
		return (void*)::SendMessage(m_hWnd, TFM_GETCLASSBODY, 0, 0);
	}
	void SaveSettings()
	{
		ATLASSERT(::IsWindow(m_hWnd));
		::SendMessage(m_hWnd, TFM_SAVESETTINGS, 0, 0);
	}
};

typedef CFolderTreeWindowT<CWindow>	CFolderTreeWindow;


////////////////////////////////////////////////////////////////////////
// CFileBrowseWindow

#define WNDCLASS_FILEBROWSER		_T("FileBrowser")

#define FVM_UPDATEVIEW				(WM_USER + 450)
#define FVM_OPENFOLDER				(WM_USER + 451)
#define FVM_REFRESHFOLDER			(WM_USER + 452)
#define FVM_STOPDIRECTORYPARSING	(WM_USER + 453)
#define FVM_SELECTITEM				(WM_USER + 454)
#define FVM_GETTOTALINFORMATION		(WM_USER + 457)
#define FVM_GETSELECTINFORMATION	(WM_USER + 458)
#define FVM_GETSELECTIONCENTERPOINT	(WM_USER + 459)
#define FVM_SORT					(WM_USER + 460)
#define FVM_SELECTALL				(WM_USER + 461)
#define FVM_GETSELECTIONFILTER		(WM_USER + 462)
#define FVM_SELECTFILES				(WM_USER + 463)
#define FVM_GETLISTSTYLE			(WM_USER + 464)
#define FVM_SETLISTSTYLE			(WM_USER + 465)
#define FVM_MARKFILES				(WM_USER + 466)
#define FVM_GETCURRENTFOLDER		(WM_USER + 467)
#define FVM_GETCLASSBODY			(WM_USER + 468)
#define FVM_SAVESETTINGS			(WM_USER + 469)
#define FVM_BUILDSORTMENU			(WM_USER + 470)
#define FVM_QUERYSORT				(WM_USER + 471)
#define FVM_SAVESELECTION			(WM_USER + 472)
#define FVM_RESTORESELECTION		(WM_USER + 473)

template <class TBase>
class CFileBrowseWindowT : public TBase
{
public:
// Constructors
	CFileBrowseWindowT(HWND hWnd = NULL) : TBase(hWnd)
	{
	}
	CFileBrowseWindowT< TBase >& operator=(HWND hWnd)
	{
		m_hWnd = hWnd;
		return *this;
	}
	void UpdateView(LPARAM lHint)
	{
		ATLASSERT(::IsWindow(m_hWnd));
		::SendMessage(m_hWnd, FVM_UPDATEVIEW, 0, lHint);
	}
	void Browse(LPCITEMIDLIST pidlFolder)
	{
		ATLASSERT(::IsWindow(m_hWnd));
		::SendMessage(m_hWnd, FVM_OPENFOLDER, 0, (LPARAM)pidlFolder);
	}
	void RefreshFolder()
	{
		ATLASSERT(::IsWindow(m_hWnd));
		::SendMessage(m_hWnd, FVM_REFRESHFOLDER, 0, 0);
//@		::PostMessage(m_hWnd, FVM_REFRESHFOLDER, 0, 0);
	}
	void StopDirectoryParsing()
	{
		ATLASSERT(::IsWindow(m_hWnd));
		::SendMessage(m_hWnd, FVM_STOPDIRECTORYPARSING, 0, 0);
	}
	void SelectItem(LPCITEMIDLIST pidlSingle)
	{
		ATLASSERT(::IsWindow(m_hWnd));
		::SendMessage(m_hWnd, FVM_SELECTITEM, 0, (LPARAM)pidlSingle);
	}
	void GetTotalInformation(LPTSTR pszText, int cchMax)
	{
		ATLASSERT(::IsWindow(m_hWnd));
		::SendMessage(m_hWnd, FVM_GETTOTALINFORMATION, (WPARAM)cchMax, (LPARAM)pszText);
	}
	void GetSelectInformation(LPTSTR pszText, int cchMax)
	{
		ATLASSERT(::IsWindow(m_hWnd));
		::SendMessage(m_hWnd, FVM_GETSELECTINFORMATION, (WPARAM)cchMax, (LPARAM)pszText);
	}
	void GetSelectionCenterPoint(LPPOINT ppt)
	{
		ATLASSERT(::IsWindow(m_hWnd));
		::SendMessage(m_hWnd, FVM_GETSELECTIONCENTERPOINT, 0, (LPARAM)ppt);
	}
	void Sort(int type, int seq)
	{
		ATLASSERT(::IsWindow(m_hWnd));
		::SendMessage(m_hWnd, FVM_SORT, type, seq);
	}
	void SelectAll(int nSelMode)
	{
		ATLASSERT(::IsWindow(m_hWnd));
		::SendMessage(m_hWnd, FVM_SELECTALL, (WPARAM)nSelMode, 0);
	}
	void GetSelectionFilter(LPCTSTR pszFilter, bool useRegExp = false)
	{
		ATLASSERT(::IsWindow(m_hWnd));
		::SendMessage(m_hWnd, FVM_GETSELECTIONFILTER, (WPARAM)useRegExp, (LPARAM)pszFilter);
	}
	void FilterSelect(LPCTSTR pszPattern, BOOL bUnmatchFilter = FALSE, BOOL bUseRegExp = FALSE, BOOL bCaseSense = FALSE)
	{
		ATLASSERT(::IsWindow(m_hWnd));
		WPARAM wp = MAKEWPARAM(MAKEWORD(bUnmatchFilter, bUseRegExp), MAKEWORD(bCaseSense, 0));
		::SendMessage(m_hWnd, FVM_SELECTFILES, wp, (LPARAM)pszPattern);
	}
	void FilterMark(LPCTSTR pszPattern, BOOL bUnmatchFilter = FALSE, BOOL bUseRegExp = FALSE, BOOL bCaseSense = FALSE)
	{
		ATLASSERT(::IsWindow(m_hWnd));
		WPARAM wp = MAKEWPARAM(MAKEWORD(bUnmatchFilter, bUseRegExp), MAKEWORD(bCaseSense, 0));
		::SendMessage(m_hWnd, FVM_MARKFILES, wp, (LPARAM)pszPattern);
	}
	int GetListStyle(ICONLISTSTYLE* ps)
	{
		ATLASSERT(::IsWindow(m_hWnd));
		return (int)::SendMessage(m_hWnd, FVM_GETLISTSTYLE, 0, (LPARAM)ps);
	}
	BOOL SetListStyle(const ICONLISTSTYLE* ps)
	{
		ATLASSERT(::IsWindow(m_hWnd));
		return (BOOL)::SendMessage(m_hWnd, FVM_SETLISTSTYLE, 0, (LPARAM)ps);
	}
	LPCITEMIDLIST GetCurrentFolder()
	{
		ATLASSERT(::IsWindow(m_hWnd));
		return (LPCITEMIDLIST)::SendMessage(m_hWnd, FVM_GETCURRENTFOLDER, 0, 0);
	}
	void* GetClassBody()
	{
		ATLASSERT(::IsWindow(m_hWnd));
		return (void*)::SendMessage(m_hWnd, FVM_GETCLASSBODY, 0, 0);
	}
	void SaveSettings()
	{
		ATLASSERT(::IsWindow(m_hWnd));
		::SendMessage(m_hWnd, FVM_SAVESETTINGS, 0, 0);
	}
	void BuildSortMenu(HMENU hmenu)
	{
		ATLASSERT(::IsWindow(m_hWnd));
		::SendMessage(m_hWnd, FVM_BUILDSORTMENU, 0, (LPARAM)hmenu);
	}
	void QuerySort()
	{
		ATLASSERT(::IsWindow(m_hWnd));
		::SendMessage(m_hWnd, FVM_QUERYSORT, 0, 0);
	}
	void SaveSelection()
	{
		ATLASSERT(::IsWindow(m_hWnd));
		::SendMessage(m_hWnd, FVM_SAVESELECTION, 0, 0);
	}
	void RestoreSelection()
	{
		ATLASSERT(::IsWindow(m_hWnd));
		::SendMessage(m_hWnd, FVM_RESTORESELECTION, 0, 0);
	}
};

typedef CFileBrowseWindowT<CWindow>	CFileBrowseWindow;


#endif		// __VIEW_WRAPPER_H__
