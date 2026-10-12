///////////////////////////////////////////////////////////////////////
//

#include "stdafx.h"
#include "fv7.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
//@ #define new DEBUG_NEW
#endif



static const TCHAR szClipHeader[] =
_T("  date       time     dos name        size      attr   filename\x0d\x0a")
_T("---------- -------- ------------ ------------- ------ ----------------------\x0d\x0a");

//////////////////////////////////////////////////////////////////////
// TDropFiles


BOOL TDropFiles::DragInitHeader(POINT pt, BOOL fInNonClientArea)
{
	DragFinish();
	m_hDrop = fopDragInitBlock(pt, fInNonClientArea);
	return (m_hDrop != NULL);
}

BOOL TDropFiles::DragAppendFile(LPCTSTR pszPath)
{
	if (!m_hDrop)
		DragInitHeader(CPoint(0, 0), FALSE);
	HDROP hdw = fopDragAppendFile(m_hDrop, pszPath);
	if (hdw) {
		m_hDrop = hdw;
		m_nEntry++;
		return TRUE;
	}
	return FALSE;
}

BOOL TDropFiles::IsFolderInclude()
{
	if (m_hDrop) {
		TCHAR buf[MAX_PATH * 2];
		for (int i = 0; i < m_nEntry; i++) {
			::DragQueryFile(m_hDrop, i, buf, ARRAYSIZE(buf));
			if (_pathisdirectory(buf))
				return TRUE;
		}
	}
	return FALSE;
}

static int get_clipboard_text(HWND hwnd, CString& str)
{
	int len = 0;
	::OpenClipboard(hwnd);
	HGLOBAL hmem = ::GetClipboardData(CF_UNICODETEXT);
	if (hmem) {
		LPCWSTR pw = (LPCWSTR)::GlobalLock(hmem);
		if (pw) {
			len = lstrlen(pw);
			str = pw;
			::GlobalUnlock(hmem);
		}
	}
	::CloseClipboard();
	return len;
}

void TDropFiles::CopyPathToClipboard(HWND hwnd, UINT copymode, bool bAppend)
{
	if (m_hDrop) {
		CString strText;
		if (bAppend && get_clipboard_text(hwnd, strText) > 0)
			strText += gszCrLf;

		TCHAR* path = new TCHAR[MAX_URL_LENGTH];
		for (int i = 0; i < m_nEntry; i++) {
			::DragQueryFile(m_hDrop, i, path, MAX_URL_LENGTH);

			LPTSTR p = NULL;
			switch (copymode) {
			case DFCP_NAMEONLY:
				p = PathFindFileName(path);
				break;
			case DFCP_UNCPATH:
			{
				int dno = PathGetDriveNumber(path);
				TCHAR root[64];
				if (dno >= 0) {
					TCHAR* unc = new TCHAR[MAX_URL_LENGTH];

					PathBuildRoot(root, dno);
					DWORD size;
					if (GetDriveType(root) == DRIVE_REMOTE) {
						root[2] = 0;
						size = MAX_URL_LENGTH;
						if (WNetGetConnection(root, unc, &size) == NO_ERROR) {
							PathAppend(unc, PathSkipRoot(path));
							p = lstrcpy(path, unc);
						}
					} else {
						size = ARRAYSIZE(root);
						if (GetComputerName(root, &size)) {
							path[1] = _T('$');
							wsprintf(unc, _T("\\\\%s\\%s"), root, path);
							p = lstrcpy(path, unc);
						}
					}

					delete [] unc;
				}
				else if (PathIsUNC(path)) {
					p = path;
				}
				break;
			}
			case DFCP_SHORTPATH:
				GetShortPathName(path, path, MAX_URL_LENGTH);
				// fall down to
			case DFCP_FULLPATH:
				p = path;
				break;
			default:
				p = NULL;
				break;
			}

			if (p) {
				if (i > 0) strText += gszCrLf;
				strText += p;
			}
		}
		delete [] path;

		lccSetStringToClipboard(hwnd, strText);
	}
}

void TDropFiles::CopyToClipboard(HWND hwnd, bool bAppend)
{
	if (!m_hDrop) return;

	CString str;
	if (bAppend && get_clipboard_text(hwnd, str) > 0)
		str += gszCrLf;

	str += szClipHeader;
	BOOL b2col = ((gnListStat & LTS_FMTDATE) != 0);

	CString dtm, fsiz, attr, path;
	CWin32FindData fd;
	for (int i = 0; i < m_nEntry; i++) {
		DragQueryFile(i, path);
		if (!GetFileInfo(path, fd)) continue;

		// タイムスタンプ
		FormatFileTime(&fd.ftLastWriteTime, dtm, b2col);
		// ファイルサイズ
		if (fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
			fsiz = _T("<dir>");
		else {
			DWORDLONG dlSize;
			SETDWORDLONG(dlSize, fd.nFileSizeLow, fd.nFileSizeHigh);
			num_form(dlSize, fsiz);
		}
		// 属性
		fd.FormatFileAttributes(attr);

		// DOS名（SJis基準で長さを整える）
		CStringA af(fd.cAlternateFileName);
		path = fd.cAlternateFileName;
		for (int n = af.GetLength(); n < 12; n++)
			path += _T(' ');

		str.AppendFormat(_T("%19s %s %13s %6s "), dtm.GetString(), path.GetString(), fsiz.GetString(), attr.GetString());
		str += fd.cFileName;
		str += gszCrLf;
	}
	lccSetStringToClipboard(hwnd, str);
}
