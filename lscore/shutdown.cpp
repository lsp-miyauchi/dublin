///////////////////////////////////////////////////////
// shutdown.cpp

#include "stdafx.h"
#include "resource.h"
#include "lscore.h"
#include "fnc.h"

extern "C"
BOOL WINAPI lccShutdown(UINT nFlags)
{
	BOOL bNeedShutdownName = (nFlags & (EWX_POWEROFF | EWX_REBOOT | EWX_SHUTDOWN));

	HANDLE hToken = NULL;		// handle to process token
	TOKEN_PRIVILEGES tkp;		// ptr. to token structure

	if (bNeedShutdownName)
	{
		// Get the current process token handle
		// so we can get shutdown privilege.

		if (!OpenProcessToken(GetCurrentProcess(),
					TOKEN_ADJUST_PRIVILEGES | TOKEN_QUERY, &hToken))
			return FALSE;

		// Get the LUID for shutdown privilege.
		LookupPrivilegeValue(NULL, SE_SHUTDOWN_NAME, &tkp.Privileges[0].Luid);

		tkp.PrivilegeCount = 1;		// one privilege to set
		tkp.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;

		// Get shutdown privilege for this process.
		AdjustTokenPrivileges(hToken, FALSE, &tkp, 0, (PTOKEN_PRIVILEGES) NULL, 0);

		// Cannot test the return value of AdjustTokenPrivileges.
		if (GetLastError() != ERROR_SUCCESS)
			return FALSE;
	}

	BOOL fResult = ExitWindowsEx(nFlags, 0);

	if (bNeedShutdownName)
	{
		// Disable shutdown privilege.
		tkp.Privileges[0].Attributes = 0;
		AdjustTokenPrivileges(hToken, FALSE, &tkp, 0, (PTOKEN_PRIVILEGES)NULL, 0);
	}
	return fResult;
}
