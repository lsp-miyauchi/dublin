////////////////////////////////////////////////////////////////
// pidl.h

typedef BOOL CALLBACK IDLFTPROC(IShellFolder*, LPITEMIDLIST, LPVOID);
typedef IDLFTPROC *LPIDLFTPROC;

#ifdef PIDL_LOCAL_FUNCTIONS
	UINT __fastcall idl_GetSize(LPCITEMIDLIST pidl);
	LPITEMIDLIST __fastcall idl_ConcatPidls(LPCITEMIDLIST pidl1, LPCITEMIDLIST pidl2);
	LPITEMIDLIST __fastcall idl_CopyITEMID(LPITEMIDLIST lpi);
	LPITEMIDLIST __fastcall idl_Copy(LPCITEMIDLIST pidlSource);
	LPCITEMIDLIST __fastcall idl_SkipToLastItem(LPITEMIDLIST pidlFq);
	LPITEMIDLIST __fastcall idl_GetFromPath(LPCTSTR pszFile);
#else
	__inline UINT idl_GetSize(LPCITEMIDLIST pidl) { return ILGetSize(pidl); }
	__inline LPITEMIDLIST idl_ConcatPidls(LPCITEMIDLIST pidl1, LPCITEMIDLIST pidl2) { return ILCombine(pidl1, pidl2); }
	__inline LPITEMIDLIST idl_CopyITEMID(LPITEMIDLIST pidl) { return ILCloneFirst(pidl); }
	__inline LPITEMIDLIST idl_Copy(LPCITEMIDLIST pidl) { return ILClone(pidl); }
	__inline LPCITEMIDLIST idl_SkipToLastItem(LPITEMIDLIST pidl) { return ILFindLastID(pidl); }
	__inline LPITEMIDLIST idl_GetFromPath(LPCTSTR pszFile) { return ILCreateFromPath(pszFile); }
#endif

LPITEMIDLIST __fastcall idl_Next(LPCITEMIDLIST pidl);
LPITEMIDLIST __fastcall idl_Create(UINT cbSize);
BOOL __fastcall idl_GetInfoTip(HWND hwnd, IShellFolder* psf, LPITEMIDLIST pidl, LPTSTR pTip);
BOOL __fastcall idl_GetName(IShellFolder* psf, LPITEMIDLIST pidl, DWORD dwFlags, CString& name);
BOOL __fastcall idl_GetName(IShellFolder* psf, LPITEMIDLIST pidl, DWORD dwFlags, LPTSTR pName, UINT cchName);
BOOL __fastcall idl_GetName(LPITEMIDLIST pidl, DWORD dwFlags, CString& name);
BOOL __fastcall idl_GetName(LPITEMIDLIST pidl, DWORD dwFlags, LPTSTR pName, UINT cchName);
BOOL __fastcall idl_GetIcon(IShellFolder* lpsf, LPITEMIDLIST pidl, HICON* phLarge, HICON* phSmall);
IShellFolder* __fastcall idl_GetFileFolder(LPITEMIDLIST pidl);
LPVOID __fastcall idl_GetUIObjectOf(HWND hwnd, LPITEMIDLIST pidlTarget, REFIID riid);
BOOL __fastcall InvokeIDList(HWND hParent, LPITEMIDLIST pidl, LPCTSTR lpVerb);
BOOL __fastcall ContextVerb(HWND hwnd, LPCTSTR lpName, LPCTSTR lpVerb);
BOOL __fastcall idl_FindTree(IShellFolder* psf, LPIDLFTPROC pfnCallback, LPVOID pv);
HBITMAP __fastcall idl_ExtractImage(LPITEMIDLIST pidl, SIZE* prgSize, DWORD depth);
BOOL __fastcall idl_LimitInputEdit(HWND hEdit, IShellFolder* psf);
BOOL __fastcall idl_LimitInputEdit(HWND hEdit, LPCTSTR pszPath);

__inline void* idl_Alloc(UINT cb) { return ((cb != 0) ? CoTaskMemAlloc(cb) : NULL); }
__inline void idl_Free(void* pv) { if (pv) CoTaskMemFree(pv); }
__inline BOOL idl_isDesktop(LPCITEMIDLIST pidl) { return (pidl != NULL && (::ILGetSize(pidl) == 2)); }
