#include "stdafx.h"
#include "fopstd.h"

bool validate_dir(HWND hwndOwner, LPCTSTR path, bool bUserConfirm)
{
	// GetLastErrorでエラー原因が特定可能とする
	TCHAR buf[MAX_PATH * 2];
	if (PathGetUNCServerShare(path, buf) && !ValidateFolderConnection(hwndOwner, buf)) {
		SetLastError(ERROR_NETWORK_ACCESS_DENIED);
		return false;
	}

	GetCurrentDirectory(ARRAYSIZE(buf), buf);
retry_dir:
	if (SetCurrentDirectory(path)) {
		SetCurrentDirectory(buf);
		return true;
	}

	DWORD dw = GetLastError();
	if (dw == ERROR_FILE_NOT_FOUND || dw == ERROR_PATH_NOT_FOUND) {
		if (bUserConfirm && AppMessage(IDS_TITLE_MKDIR, IDS_QUERY_MKDIR, path, MB_QYESNO) != IDYES) {
			SetLastError(ERROR_CANCELLED);
			return false;
		}
		if (!fopMakeTree(path))
			return false;
		return true;
	}

	if (bUserConfirm && LastError(IDS_TITLE_FOPSERVER, path, MB_QRETRY) == IDRETRY)
		goto retry_dir;
	return false;
}

extern "C"
BOOL WINAPI fopValidateDir(HWND hwndOwner, LPCTSTR path)
{
	return validate_dir(hwndOwner, path, true);
}
