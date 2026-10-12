////////////////////////////////////////////////////////////////
// gmil.dll api header

#ifdef __cplusplus
extern "C" {
#endif  /* __cplusplus */

HWND WINAPI gmOpen(HWND hwndParent, LPCTSTR pszName);
HWND WINAPI gmOpenFolder(HWND hwndParent, LPCTSTR pszPath);
HWND WINAPI thumbFolder(HWND hwndParent, LPCTSTR pszPath);

#ifdef __cplusplus
}
#endif
