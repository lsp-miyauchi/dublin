// name.h : header file
//


/////////////////////////////////////////////////////////////////////////////
// CQueryMkdirDlg dialog

#define QMD_MAKEDIR			100
#define QMD_RENAME			101


INT_PTR QueryCopy(HWND hWnd, IDataObject* pdo, LPCITEMIDLIST pidlBase, LPTSTR pszTo, BOOL bCopy, BOOL& bTabOpen, BOOL& bThreaded, BOOL& bAsAdmin);
INT_PTR QueryCopy2(HWND hWnd, HDROP hdrop, LPCITEMIDLIST pidlBase, LPTSTR pszTo, BOOL bCopy, BOOL& bTabOpen, BOOL& bThreaded, BOOL& bAsAdmin);
INT_PTR QueryMakeDir(HWND hWnd, LPCTSTR pszName);
BOOL QueryDrop(HWND hwnd, LPCTSTR pszDropTarget, DROPEFFECT eff, BOOL& bThreded, BOOL& bAsAdmin, BOOL bForceOverwrite = FALSE);
