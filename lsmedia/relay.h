//////////////////////////////////////////////////////////////////////
// relay.h


#ifdef __cplusplus
extern "C" {                    // Assume C declarations for C++
#endif

extern "C" BOOL WINAPI RelayDlgFn(HWND hdlg, UINT msg, WPARAM wParam, LPARAM lParam);

#ifdef __cplusplus
}
#endif

void ry_InitBar(HWND hdlg, long lTotal, UINT nIDAnimate, int cx);
void ry_SetFile(HWND hdlg, LPCTSTR lpFile);
void ry_SetCaption(HWND hdlg, LPCTSTR lpCaption);
void ry_SetComplete(HWND hdlg, long lDone);
HWND ry_Create(HWND hwnd);
void ry_Close(HWND hdlg);
BOOL ry_IsCanceled(HWND hdlg);
