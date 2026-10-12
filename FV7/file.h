// file.h : ヘッダー ファイル
//

#pragma once
#include "dropper.h"

#include "view_wrapper.h"
#include "shellnotify.h"
#include "watchfolder.h"
#include "dropshadow.h"
#include "findbar.h"


enum {
	FILESEL_REV,		// 選択反転
	FILESEL_ALL,		// 全選択
	FILESEL_NON			// 全選択解除
};

/////////////////////////////////////////////////////////////////////////////
//
//

class CMWDrawListCtrl :	public CWindowImpl<CMWDrawListCtrl, CDrawListCtrl>,
						public CMouseWheelerImpl<CMWDrawListCtrl>
{
public:
BEGIN_MSG_MAP(CMWDrawListCtrl)
	CHAIN_MSG_MAP(CMouseWheelerImpl<CMWDrawListCtrl>)
END_MSG_MAP()
};

/////////////////////////////////////////////////////////////////////////////
//
//

class CDocument;

class CFileBrowseView
	: public CWindowImpl<CFileBrowseView>
	, public CTBarCustomDrawImpl<CFileBrowseView>
	, public TDropperImpl<CFileBrowseView>
	, public CShellChangeNotifyImpl<CFileBrowseView>
	, public CIdleHandler
{
public:
	DECLARE_WND_CLASS_EX(WNDCLASS_FILEBROWSER, 0, COLOR_WINDOW)

	CFileBrowseView();
	~CFileBrowseView();

	// 重要: このブラウザビューは new 演算子でヒープより作成されなければならぬ
	// そうでない場合には OnFinalMessage をオーバーライドして、delete this を止めること
	virtual void OnFinalMessage(HWND /*hWnd*/)
	{
		delete this;
	}

	virtual BOOL OnIdle();
	virtual void OnShellChangeNotify(long lEvent, PIDLIST_ABSOLUTE* pidls);

	BOOL IsMyMessage(const MSG* pMsg) const { return (pMsg->hwnd == m_list); }
	BOOL ProcessMessage(const MSG* pMsg);

BEGIN_MSG_MAP(CFileBrowseView)
	CHAIN_MSG_MAP(TDropperImpl<CFileBrowseView>)
	CHAIN_MSG_MAP(CTBarCustomDrawImpl<CFileBrowseView>)
	CHAIN_MSG_MAP(CShellChangeNotifyImpl<CFileBrowseView>)
	ON_WM_CREATE()
	ON_WM_SIZE()
	ON_WM_SETFOCUS()
	ON_WM_ERASEBKGND()
	ON_WM_MOUSEACTIVATE()
	ON_WM_DRAWITEM()
	ON_WM_CLOSE()
	ON_WM_DESTROY()
	ON_WM_TIMER()
	ON_NOTIFY(NM_DBLCLK, IDW_FILE, OnNotifyDblClk)
	ON_NOTIFY(NM_SETFOCUS, IDW_FILE, OnNotifySetFocus)
	ON_NOTIFY(NM_KILLFOCUS, IDW_FILE, OnNotifyKillFocus)
	ON_NOTIFY(NM_KEYDOWN, IDW_FILE, OnNotifyKeyDown)
	ON_NOTIFY(DLN_CLICK, IDW_FILE, OnNotifyClick)
	ON_NOTIFY(DLN_GETINFOTIP, IDW_FILE, OnNotifyGetInfoTip)
	ON_NOTIFY(DLN_BEGINDRAG, IDW_FILE, OnNotifyBeginDrag)
	ON_NOTIFY(DLN_BEGINRDRAG, IDW_FILE, OnNotifyBeginDrag)
	ON_COMMAND(IDM_FILE_SENDTO, OnSendTo)
	ON_COMMAND(IDM_TOOL_EDITOR, OnExecEditor)
	ON_COMMAND(IDM_TOOL_VIEWER, OnExecViewer)
	ON_COMMAND(IDM_TOOL_BDUMP, OnExecBinDump)
	ON_COMMAND(IDM_FILE_COPY, OnFileCopy)
	ON_COMMAND(IDM_FILE_DELETE, OnFileDelete)
	ON_COMMAND(IDM_FILE_SHRED, OnFileShred)
	ON_COMMAND(IDM_FILE_MOVE, OnFileMove)
	ON_COMMAND(IDM_FILE_RENAME, OnFileRename)
	ON_COMMAND(IDM_FILE_ATTRIB, OnFileAttrib)
	ON_COMMAND(IDM_FILE_PROPERTY, OnFileProperty)
	ON_COMMAND(IDM_VIEW_FILESIZE, OnViewFileSize)
	ON_COMMAND(IDM_VIEW_FILEINFO, OnViewFileInfo)
	ON_COMMAND(IDM_FILE_MERGE, OnFileMerge)
	ON_COMMAND(IDM_FILE_EXECUTE, OnFileExecute)
	ON_COMMAND(IDM_FILE_DIVIDE, OnFileDivide)
	ON_COMMAND(IDM_FILE_PACK, OnFilePack)
	ON_COMMAND(IDM_FILE_UNPACK, OnFileUnpack)
	ON_COMMAND(IDM_FILE_CHECKARC, OnFileCheckArchive)
	ON_COMMAND(IDM_FILE_SHORTCUT, OnFileShortcut)
	ON_COMMAND(IDM_FILE_FILTERDEL, OnFileFilterDelete)
	ON_COMMAND(IDM_FILE_OPEN, OnFileOpen)
	ON_COMMAND(IDM_FILE_CONTEXTMENU, OnFileContextMenu)
	ON_COMMAND(IDM_FILE_SERIESNAME, OnFileSeriesName)
	ON_COMMAND(IDM_FILE_REPNAMES, OnFileRepNames)
//@	ON_COMMAND(IDM_FILE_ENCRYPT, OnFileEncrypt)
//@	ON_COMMAND(IDM_FILE_DECRYPT, OnFileDecrypt)
	ON_COMMAND(IDM_FILE_CHGCAPS, OnFileChangeCaps)
	ON_COMMAND(IDM_FILE_ASSOCIATE, OnFileAssociate)
	ON_COMMAND(IDM_FILE_RENEXT, OnFileRenameExtension)
	ON_COMMAND(IDM_EDIT_COPY, OnEditCopy)
	ON_COMMAND(IDM_EDIT_CUT, OnEditCut)
	ON_COMMAND(IDM_EDIT_LISTOUTPUT, OnEditListOutput)
	ON_COMMAND(IDM_TOOL_CALCHASH, OnToolCalcHash)
	ON_COMMAND_RANGE(IDM_COPY_NAMEONLY, IDM_COPY_UNCNAME, OnCopyNameRange)
	ON_COMMAND_RANGE(IDM_HOTKEY_FIRST, IDM_HOTKEY_LAST, OnExecHotKeyRange)
	ON_COMMAND_RANGE(IDM_FINDER_FIRST, IDM_FINDER_LAST, OnFinderRange)
	ON_NOTIFY(HDN_BEGINTRACK, IDW_HEADER, OnHeaderNotifyBeginTrack)
	ON_NOTIFY(HDN_TRACK, IDW_HEADER, OnHeaderNotifyTracking)
	ON_NOTIFY(HDN_ITEMCHANGING, IDW_HEADER, OnHeaderNotifyItemChanging)
	ON_NOTIFY(HDN_ENDTRACK, IDW_HEADER, OnHeaderNotifyEndTrack)
	ON_NOTIFY(HDN_ITEMCLICK, IDW_HEADER, OnHeaderNotifyItemClick)
	ON_NOTIFY(HDN_DIVIDERDBLCLICK, IDW_HEADER, OnHeaderAutoAdjust)
	ON_MESSAGE(WM_CONTEXTMENU, OnContextMenu)
	ON_MESSAGE(WM_CTLCOLORLISTBOX, OnCtlColorListBox)
	ON_MESSAGE(WM_FILENOTIFY, OnFileNotify)
	ON_MESSAGE(WM_IDLISTNOTIFY, OnIDListNotify)
	ON_REGISTERD_MESSAGE(nRM_GREP_FILE, OnFileGrepDone)
	ON_REGISTERD_MESSAGE(nRM_GREP_DIR, OnDirGrepDone)
	ON_REGISTERD_MESSAGE(nRM_GREP_COLUMN, OnColumnGrepDone)
	ON_REGISTERD_MESSAGE(nRM_GREP_IMAGE, OnImageGrepDone)
	ON_REGISTERD_MESSAGE(nRM_GREP_ICON, OnIconGrepDone)
	ON_MESSAGE_VOID(FVM_UPDATEVIEW, OnUpdateView)
	ON_MESSAGE_VOID(FVM_REFRESHFOLDER, OnRefreshFolder)
	ON_MESSAGE_VOID(FVM_STOPDIRECTORYPARSING, OnStopDirectoryParsing)
	ON_MESSAGE_VOID(FVM_SELECTITEM, OnSelectItem)
	ON_MESSAGE_VOID(FVM_GETTOTALINFORMATION, OnGetTotalInformation)
	ON_MESSAGE_VOID(FVM_GETSELECTINFORMATION, OnGetSelectInformation)
	ON_MESSAGE_VOID(FVM_SORT, OnSort)
	ON_MESSAGE_VOID(FVM_SELECTALL, OnSelectAll)
	ON_MESSAGE_VOID(FVM_GETSELECTIONFILTER, OnGetSelectionFilter)
	ON_MESSAGE_VOID(FVM_GETSELECTIONCENTERPOINT, OnGetSelectionCenterPoint)
	ON_MESSAGE_VOID(FVM_SELECTFILES, OnSelectFiles)
	ON_MESSAGE_VOID(FVM_MARKFILES, OnMarkFiles)
	ON_MESSAGE_VOID(FVM_SAVESETTINGS, OnSaveSettings)
	ON_MESSAGE_VOID(FVM_BUILDSORTMENU, OnBuildSortMenu)
	ON_MESSAGE_VOID(FVM_QUERYSORT, OnQuerySort)
	ON_MESSAGE_VOID(FVM_SAVESELECTION, OnSaveSelection)
	ON_MESSAGE_VOID(FVM_RESTORESELECTION, OnRestoreSelection)
	ON_MESSAGE(FVM_OPENFOLDER, OnOpenFolder)
	ON_MESSAGE(FVM_GETLISTSTYLE, OnHandlerGetListStyle)
	ON_MESSAGE(FVM_SETLISTSTYLE, OnHandlerSetListStyle)
	ON_MESSAGE(FVM_GETCURRENTFOLDER, OnHandlerGetCurrentFolder)
	ON_MESSAGE(FVM_GETCLASSBODY, OnHandlerGetClassBody)
END_MSG_MAP()

protected:
	int OnCreate(LPCREATESTRUCT pcs);
	void OnSize(UINT nType, int cx, int cy);
	void OnSetFocus(HWND hOldWnd) { m_list.SetFocus(); }
	int OnMouseActivate(HWND hDesktopWnd, UINT nHitTest, UINT message);
	BOOL OnEraseBkgnd(HDC hdc);
	void OnDrawItem(int nCtlID, LPDRAWITEMSTRUCT lpdi);
	void OnClose(BOOL& bHandled);
	void OnDestroy();
	inline void OnFileOpen() { DoFileOpen(); }
	void OnTimer(UINT_PTR nIDEvent);
	inline void OnNotifyDblClk(NMHDR* pNMHDR, LRESULT* pResult) { DoFileOpen(true); }
	void OnNotifySetFocus(NMHDR* pNMHDR, LRESULT* pResult);
	void OnNotifyKillFocus(NMHDR* pNMHDR, LRESULT* pResult);
	void OnNotifyKeyDown(NMHDR* pNMHDR, LRESULT* pResult);
	void OnNotifyClick(NMHDR* pNMHDR, LRESULT* pResult);
	void OnNotifyGetInfoTip(NMHDR* pNMHDR, LRESULT* pResult);
	void OnNotifyBeginDrag(NMHDR* pNMHDR, LRESULT* pResult);
	void OnExecEditor();
	void OnExecViewer();
	void OnFileCopy();
	void OnFileDelete();
	void OnFileShred();
	void OnFileMove();
	void OnFileRename();
	void OnFileAttrib();
	void OnFileFilterDelete();
	void OnViewFileSize();
	void OnViewFileInfo();
	void OnExecBinDump();
	void OnFileMerge();
	void OnFileExecute();
	void OnFileDivide();
	void OnFilePack();
	void OnFileUnpack();
	void OnFileCheckArchive();
	void OnEditListOutput();
	void OnToolCalcHash();
	void OnFileShortcut();
	void OnFileProperty();
	void OnFileContextMenu();
	void OnFileSeriesName();
	void OnFileRepNames();
	void OnFileChangeCaps();
//@	void OnFileEncrypt();
//@	void OnFileDecrypt();
	void OnFileAssociate();
	void OnFileRenameExtension();
	void OnEditCopy();
	void OnEditCut();
	void OnSendTo();
	void OnHeaderAutoAdjust();
	void OnCopyNameRange(UINT idCmd, UINT code, HWND hwnd);
	void OnExecHotKeyRange(UINT idCmd, UINT code, HWND hwnd);
	void OnFinderRange(UINT idCmd, UINT code, HWND hwnd);
	void OnHeaderNotifyBeginTrack(NMHDR* pNMHDR, LRESULT* pResult);
	void OnHeaderNotifyItemChanging(NMHDR* pNMHDR, LRESULT* pResult);
	void OnHeaderNotifyTracking(NMHDR* pNMHDR, LRESULT* pResult);
	void OnHeaderNotifyEndTrack(NMHDR* pNMHDR, LRESULT* pResult);
	void OnHeaderNotifyItemClick(NMHDR* pNMHDR, LRESULT* pResult);
	void OnHeaderAutoAdjust(NMHDR* pNMHDR, LRESULT* pResult);
	inline LRESULT OnCtlColorListBox(WPARAM /*wParam*/, LPARAM /*lParam*/) { return (LRESULT)fvApp.GetBkgndBrush(); }
	LRESULT OnContextMenu(WPARAM wParam, LPARAM lParam);
	LRESULT OnFileNotify(WPARAM wParam, LPARAM lParam);
	LRESULT OnIDListNotify(WPARAM wParam, LPARAM lParam);
	LRESULT OnFileGrepDone(WPARAM wParam, LPARAM lParam);
	LRESULT OnDirGrepDone(WPARAM wParam, LPARAM lParam);
	LRESULT OnColumnGrepDone(WPARAM wParam, LPARAM lParam);
	LRESULT OnImageGrepDone(WPARAM wParam, LPARAM lParam);
	LRESULT OnIconGrepDone(WPARAM wParam, LPARAM lParam);
	inline void OnUpdateView(WPARAM, LPARAM lParam) { UpdateView(lParam); }
	inline void OnRefreshFolder(WPARAM, LPARAM) { ShellBrowse(NULL); }
	inline void OnStopDirectoryParsing(WPARAM, LPARAM) { StopDirectoryParsing(); }
	inline void OnSelectItem(WPARAM, LPARAM lParam) { SelectItem((LPITEMIDLIST)lParam); }
	inline void OnGetTotalInformation(WPARAM wParam, LPARAM lParam) { GetTotalInformation((LPTSTR)lParam, (int)wParam); }
	inline void OnGetSelectInformation(WPARAM wParam, LPARAM lParam) { GetSelectInformation((LPTSTR)lParam, (int)wParam); }
	inline void OnGetSelectionCenterPoint(WPARAM, LPARAM lParam) { GetSelectionCenterPoint((LPPOINT)lParam); }
	inline void OnSort(WPARAM wParam, LPARAM lParam);
	inline void OnSelectAll(WPARAM wParam, LPARAM) { SelectAll((int)wParam); }
	inline void OnGetSelectionFilter(WPARAM wParam, LPARAM lParam) { GetSelectionFilter((LPTSTR)lParam, (bool)(wParam != 0)); }
	inline void OnSelectFiles(WPARAM wParam, LPARAM lParam) { SelectFiles((LPCTSTR)lParam, (LOBYTE(LOWORD(wParam)) != 0), (HIBYTE(LOWORD(wParam)) != 0), (LOBYTE(HIWORD(wParam)) != 0), false); }
	inline void OnMarkFiles(WPARAM wParam, LPARAM lParam) { SelectFiles((LPCTSTR)lParam, (LOBYTE(LOWORD(wParam)) != 0), (HIBYTE(LOWORD(wParam)) != 0), (LOBYTE(HIWORD(wParam)) != 0), true); }
	inline void OnSaveSettings(WPARAM wParam, LPARAM lParam) { SaveSettings(); }
	inline void OnSaveSelection(WPARAM, LPARAM) { SaveSelection(); }
	inline void OnRestoreSelection(WPARAM, LPARAM) { RestoreSelection(); }
	void OnBuildSortMenu(WPARAM wParam, LPARAM lParam);
	void OnQuerySort(WPARAM wParam, LPARAM lParam);
	inline LRESULT OnOpenFolder(WPARAM, LPARAM lParam) { return (LRESULT)ShellBrowse((LPCITEMIDLIST)lParam); }
	LRESULT OnHandlerGetListStyle(WPARAM wParam, LPARAM lParam);
	LRESULT OnHandlerSetListStyle(WPARAM wParam, LPARAM lParam);
	inline LRESULT OnHandlerGetCurrentFolder(WPARAM wParam, LPARAM lParam) { return (LRESULT)m_idlFolder.GetData(); }
	inline LRESULT OnHandlerGetClassBody(WPARAM wParam, LPARAM lParam) { return (LRESULT)this; }

	BOOL ProcessKeyDown(UINT nVKey);
	void ProcessIDListNotify(UINT action, LPCITEMIDLIST pidlTarget);

public:
	BOOL OnGetDragImage(SHDRAGIMAGE* psdi);
	DWORD OnDragOver(IDataObject* pdo, DWORD keyState, DWORD dropEffect, POINT point);
	DWORD OnDrop(IDataObject* pdo, DWORD keyState, DWORD dropEffect, POINT point);
	void OnDragLeave();

	DWORDLONG CalcSelectedFileSize();
	DWORDLONG CalcMarkedFileSize();
	int FindNextMatchSpec(int iStartBefore, LPCTSTR pszFilter, bool bRegExp);
	int FindPrevMatchSpec(int iStartBefore, LPCTSTR pszFilter, bool bRegExp);
	CFileItem* FindItem(LPCITEMIDLIST pidl, bool bCompareFullPidl = false);
	int FindItemIndex(LPCITEMIDLIST pidl, bool bCompareFullPidl = false);
	void ModifyItem(CFileItem* pf);
	void GetPopupMenuPoint(UINT nIDCommand, LPPOINT lppt);

	void SaveSelection();
	void RestoreSelection();

// オペレーション
protected:
	int MapSortTypeToIndex(int nsType);
	int MapSortIndexToType(int index);
	void SortItemArray(bool bUseShellCompare, int column, int nsSeq);
	BOOL CreateListWindow();
	void SetHeaderBoundary(int index);
	int ShellBrowse(LPCITEMIDLIST pidlFolder);
	void RebuildHeaderColumns();
	void InvokeContextMenuCommand(LPCTSTR pszVerb);
	void ResetFont();
	void ResetTimer(DWORD dwAdd);

	void SortItems(int column, int seq);
	void UpdateSort(int column);

	void RedrawSelItems();
	int BrowseFolder(BOOL bRoot);
	void RecalcColumn();
	void RecalcColumnList(HDC hdc);
	void RecalcColumnOutline(HDC hdc);
	void RecalcColumnIcon(HDC hdc);
	void RecalcColumnReport(HDC hdc);
	void RecalcColumnImage(HDC hdc);
	HBITMAP GetBarMeterBitmap();
	void DrawDriveUsage(HDC hdc, LPRECT prc, DWORDLONG sfree, DWORDLONG stotal, bool bShading);
	void DrawItemStyleList(CDCHandle& dc, LPDRAWITEMSTRUCT lpdi);
	void DrawItemStyleOutline(CDCHandle& dc, LPDRAWITEMSTRUCT lpdi);
	void DrawItemStyleIcon(CDCHandle& dc, LPDRAWITEMSTRUCT lpdi);
	void DrawItemStyleReport(CDCHandle& dc, LPDRAWITEMSTRUCT lpdi);
	void DrawItemStyleImage(CDCHandle& dc, LPDRAWITEMSTRUCT lpdi);
	void GetItemIconImage(CFileItem* pf);
	CFileItem* GetCurrentSingleFileItem();
	int PackFileItemArray(CSimpleArray<CFileItem*>& array, BOOL bListAll);
	int PackIDListArray(CSimpleArray<LPCITEMIDLIST>& array, BOOL bListAll = FALSE, BOOL bFullyQual = FALSE);
	int PackShellItemArray(BOOL bListAll, IShellItemArray** ppsia);
	int GetUIObjectOf(REFIID riid, void** ppUnk, BOOL bListAll = FALSE);
	SFGAOF GetAttributesOf(SFGAOF attr, BOOL bListAll = FALSE);
	HDROP MakeDropFiles(BOOL bListAll = FALSE);
	int MakeDropFiles(TDropFiles& df, BOOL bListAll = FALSE);
	BOOL StartDragDrop();
	void DoHeaderTracking(HD_NOTIFY* pHDN);

public:
	// tabからも直接呼び出される
	DWORD LocalDropFile(DWORD dropFormats, IDataObject* pdo, DWORD keyState, DWORD dropEffect, POINT point);

protected:
	void SetExtendedListStyle();
	void UpdateView(LPARAM lHint);
	void GetSelectionCenterPoint(LPPOINT ppt);
	void StopDirectoryParsing();
	void SelectItem(LPITEMIDLIST pidlSingle);
	void SelectAll(int nSelMode);
	void GetSelectionFilter(LPTSTR pszFilter, bool useRegExp = false);
	void SelectFiles(LPCTSTR pszFilter, bool bUnmatch = false, bool bRegExp = false, bool bCaseSense = false, bool bMark = false);
	inline void UnselectFiles() { SelectFiles(NULL, false, false, false, false); }
	inline void MarkFiles(LPCTSTR pszFilter, bool bUnmatch = false, bool bRegExp = false, bool bCaseSense = false) { SelectFiles(pszFilter, bUnmatch, bRegExp, bCaseSense, true); }
	inline void UnmarkFiles() { SelectFiles(NULL, false, false, false, true); }
	void SaveSettings();
	void GetTotalInformation(LPTSTR pszText, int cchMax);
	void GetSelectInformation(LPTSTR pszText, int cchMax);
	inline void GetThumbnailSize(SIZE* psize) { if (psize) m_ListStyle.GetThumbSize(psize); }
	void SetThumbnailSize(SIZE* psize);
	void DoFileOpen(bool bByMouseAction = false);
	void DoObjectMoveCopy(bool bCopy = true);

	void LoadCustomIconMap();
	void ExecMacro(const CHotKeyItem& item);
	void ToggleFindBar();

	void OutputHtml(LPCTSTR pszName, BOOL bListAll, BOOL bLaunchBrowser);
	void OutputXml(LPCTSTR pszName, BOOL bListAll, BOOL bLaunchBrowser);
	void OutputCsv(LPCTSTR pszName, BOOL bListAll, BOOL bLaunchBrowser);

// 属性
	inline bool isFileSystemFolder() { return ((m_curattr & (SFGAO_FILESYSANCESTOR | SFGAO_FILESYSTEM | SFGAO_STREAM)) == (SFGAO_FILESYSANCESTOR | SFGAO_FILESYSTEM)); }
	inline bool isDropTargetFolder() { return ((m_curattr & SFGAO_DROPTARGET) != 0); }
	inline bool hasFolderAttributes(SFGAOF attr) { return ((m_curattr & attr) != 0); }

// データメンバ
public:
	CFileStack m_stack;
	CMWDrawListCtrl m_list;
	DWORDLONG m_dlTotal;
	DWORDLONG m_dlSelTotal;
	CFindBar m_finder;
	int m_numMarks;
	DWORDLONG m_dlMarkTotal;

protected:
	CDocument* m_pDocument;

	int m_nMaxName;
	int m_nMaxExt;

	CComPtr<IImageList> m_pilSmall;
	CComPtr<IImageList> m_pilLarge;
	CComPtr<IImageList> m_pilExtra;

	CBitmap m_bmBarMeter;
	CSize m_sizeSmall;
	CSize m_sizeLarge;
	CSize m_sizeExtra;
	ULONGLONG m_magicNotify;
	ULONGLONG m_magicGrep;
	int m_nNotifyResetCounter;
	UINT_PTR m_timerid;
	DWORD m_timerflag;
	CApplyDropShadow m_dropshadow;

	CString m_strCurPath;

	IShellFolder* m_psf;
	CIDListPtr m_idlFolder;
	SFGAOF m_curattr;
	int m_numColumns;					// カラム数
	int m_sortColumn;					// ソート対象になっているカラム
	int m_rawfiles;
	SHELLCOLUMNINFO m_sdt[MAXCOLUMNINFO];
	int m_colWidthMax[MAXCOLUMNINFO];
	CSize m_sizeChar;
	CFont m_font;

	CFGrepThread m_FileGrep;
//@	CColGrepThread m_ColGrep;
	CColumnGrep m_ColGrep;
	CImageGrepThread m_ImageGrep;
	CDirGrepThread m_DirGrep;
	CWatchFolder m_WatchFolder;
	CIconGrepThread m_IconGrep;
	CIconListStyle m_ListStyle;

	CSimpleArray<int> m_arrSaveSelection;

	bool m_bRetrieving:1;
	bool m_bHeaderTracking:1;
};

