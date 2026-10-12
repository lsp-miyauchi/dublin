////////////////////////////////////////////////////////////////////////
//

BOOL SetPrivilege(HANDLE hToken, LPCTSTR lpszPrivilege, BOOL bEnablePrivilege)
{
	BOOL bOK = FALSE;
	HANDLE hToken;
	if (OpenProcessToken(GetCurrentProcess(), TOKEN_ADJUST_PRIVILEGES | TOKEN_QUERY, &hToken)) {
		LUID luid;
		if (LookupPrivilegeValue(NULL, lpszPrivilege, &luid)) {
			TOKEN_PRIVILEGES tp;
			tp.PrivilegeCount = 1;
			tp.Privileges[0].Luid = luid;
			if (bEnablePrivilege)
				tp.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;
			else
				tp.Privileges[0].Attributes = 0;

			// Enable the privilege or disable all privileges.
			bOK = AdjustTokenPrivileges(hToken, FALSE, &tp, sizeof(TOKEN_PRIVILEGES), (PTOKEN_PRIVILEGES)NULL, (PDWORD)NULL);
			if (bOK) {
				if (GetLastError() == ERROR_NOT_ALL_ASSIGNED) {
					// The token does not have the specified privilege.
					bOK = FALSE;
				}
			}
		}
		CloseHandle(hToken);
	}
	return bOK;
}

////////////////////////////////////////////////////////////////////////
//

BOOL RunAsAdmin( HWND hWnd, LPTSTR lpFile, LPTSTR lpParameters )
{
    SHELLEXECUTEINFO   sei;
    ZeroMemory ( &sei, sizeof(sei) );

    sei.cbSize          = sizeof(SHELLEXECUTEINFOW);
    sei.hwnd            = hWnd;
    sei.fMask           = SEE_MASK_FLAG_DDEWAIT | SEE_MASK_FLAG_NO_UI;
    sei.lpVerb          = _TEXT("runas");
    sei.lpFile          = lpFile;
    sei.lpParameters    = lpParameters;
    sei.nShow           = SW_SHOWNORMAL;

    if ( ! ShellExecuteEx ( &sei ) )
    {
        printf( "Error: ShellExecuteEx failed 0x%x\n", GetLastError() );
        return FALSE;
    }
    return TRUE;
}

////////////////////////////////////////////////////////////////////////
//

typedef struct tagBIND_OPTS3 : tagBIND_OPTS2 {
	HWND hwnd;
} BIND_OPTS3, * LPBIND_OPTS3;

HRESULT CoCreateInstanceAsAdmin(HWND hwnd, REFCLSID rclsid, REFIID riid, __out void ** ppv)
{
	BIND_OPTS3 bo;
	WCHAR  wszCLSID[50];
	WCHAR  wszMonikerName[300];

	StringFromGUID2(rclsid, wszCLSID, ARRAYSIZE(wszCLSID)); 
	HRESULT hr = StringCchPrintf(wszMonikerName, ARRAYSIZE(wszMonikerName), L"Elevation:Administrator!new:%s", wszCLSID);
	if (FAILED(hr))
		return hr;
	memset(&bo, 0, sizeof(bo));
	bo.cbStruct = sizeof(bo);
	bo.hwnd = hwnd;
	bo.dwClassContext  = CLSCTX_LOCAL_SERVER;
	return CoGetObject(wszMonikerName, &bo, riid, ppv);
}
