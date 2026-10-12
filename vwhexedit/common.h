//////////////////////////////////////////////////////////////////////
// common.h

enum {
	UVM_INITILIZE,
	UVM_NEWFILE,
	UVM_RESETPARAMS,
	UVM_LOADFILE,
	UVM_SAVEFILE,
};

enum {
	e_BINARY,
	e_DEC,
	e_HEX,
	e_WORD,
	e_DWORD
};

extern COSPlatform gosp;
__inline bool OsVISTA() { return gosp.OsVISTA(); }

#define MB_ATTENTION	(MB_OK | MB_ICONEXCLAMATION)
#define MB_QUESTION		(MB_YESNOCANCEL | MB_ICONQUESTION)
#define MB_QYESNO		(MB_YESNO | MB_ICONQUESTION)
#define MB_QOKCANCEL	(MB_OKCANCEL | MB_ICONEXCLAMATION)
#define MB_QABORT		(MB_ICONEXCLAMATION | MB_ABORTRETRYIGNORE)
#define MB_HAND			(MB_OKCANCEL | MB_ICONHAND)
#define MB_STOP			(MB_OK | MB_ICONSTOP)
#define MB_QAST			(MB_OKCANCEL | MB_ICONASTERISK)
#define MB_QEXCM		(MB_OKCANCEL | MB_ICONEXCLAMATION)
#define MB_QRETRY		(MB_RETRYCANCEL | MB_ICONEXCLAMATION)

////////////////////////////////////////////////////////////////////////
//

int AppMessage(UINT nID, LPCTSTR pszInsert = NULL, UINT nType = MB_STOP);
int LastError(LPCTSTR, UINT);
void PathDecorate(LPTSTR psz);
bool ShowOpenSaveFileDialog(HWND hwnd, bool bOpen, LPCTSTR pszTitle, LPCTSTR pszInitialDir, LPTSTR pszFile, int cchMax);
void BootNewProcess(HWND hwnd, LPCTSTR cmd);
