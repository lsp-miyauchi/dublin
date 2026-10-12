#if !defined(AFX_SAVE_H__BF9CCDAA_E975_11D1_891F_00C04FD8DD3F__INCLUDED_)
#define AFX_SAVE_H__BF9CCDAA_E975_11D1_891F_00C04FD8DD3F__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000
// Save.h : ヘッダー ファイル
//

BOOL SaveFileBrowse(HWND hWnd, LPTSTR pszName, int* ptype);
BOOL SaveGraphic(HDIB hdib, LPTSTR pszName, int nType);


#endif // !defined(AFX_SAVE_H__BF9CCDAA_E975_11D1_891F_00C04FD8DD3F__INCLUDED_)
