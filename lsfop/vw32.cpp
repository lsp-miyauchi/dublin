#include "stdafx.h"
#include "fopstd.h"

#ifndef VWIN32_DIOC_DOS_IOCTL
	#define VWIN32_DIOC_DOS_IOCTL	1
#endif

#define CARRY_FLAG		0x0001

#pragma pack(1)

typedef struct _DIOCREGISTERS
{
	DWORD reg_EBX;
	DWORD reg_EDX;
	DWORD reg_ECX;
	DWORD reg_EAX;
	DWORD reg_EDI;
	DWORD reg_ESI;
	DWORD reg_Flags;
} DIOC_REGISTERS, *LPDIOC_REGISTERS;

#pragma pack()


HANDLE OpenVWin32()
{
	return CreateFile(_T("\\\\.\\vwin32"), 0, 0, NULL, 0, FILE_FLAG_DELETE_ON_CLOSE, NULL);
}

BOOL CloseVWin32(HANDLE hVWin32)
{
	return CloseHandle(hVWin32);
}

BOOL LockVolume(HANDLE hVWin32, int nDrive, int level, int permission)
{
	DIOC_REGISTERS dr = {0};
	dr.reg_EAX = 0x440d;
	dr.reg_EBX = MAKEWORD(nDrive, level & 3);
	dr.reg_ECX = MAKEWORD(0x4a, 0x48);
	dr.reg_EDX = permission & 3;

	DWORD dw;
	BOOL bOK = DeviceIoControl(hVWin32, VWIN32_DIOC_DOS_IOCTL, &dr, sizeof(dr), &dr, sizeof(dr), &dw, 0);
	if (!bOK || dr.reg_Flags & CARRY_FLAG) {
		dr.reg_EAX = 0x440d;
		dr.reg_EBX = MAKEWORD(nDrive, level & 3);
		dr.reg_ECX = MAKEWORD(0x4a, 0x08);
		dr.reg_EDX = permission & 3;
		bOK = DeviceIoControl(hVWin32, VWIN32_DIOC_DOS_IOCTL, &dr, sizeof(dr), &dr, sizeof(dr), &dw, 0);
	}
	return (bOK && !(dr.reg_Flags & CARRY_FLAG)) ? TRUE : FALSE;
}

BOOL UnlockVolume(HANDLE hVWin32, int nDrive)
{
	DIOC_REGISTERS dr = {0};
	dr.reg_EAX = 0x440d;
	dr.reg_EBX = nDrive;
	dr.reg_ECX = MAKEWORD(0x6a, 0x48);
	DWORD dw;
	BOOL bOK = DeviceIoControl(hVWin32, VWIN32_DIOC_DOS_IOCTL, &dr, sizeof(dr), &dr, sizeof(dr), &dw, 0);
	if (!bOK || dr.reg_Flags & CARRY_FLAG) {
		dr.reg_EAX = 0x440d;
		dr.reg_EBX = nDrive;
		dr.reg_ECX = MAKEWORD(0x6a, 0x08);
		bOK = DeviceIoControl(hVWin32, VWIN32_DIOC_DOS_IOCTL, &dr, sizeof(dr), &dr, sizeof(dr), &dw, 0);
	}
	return (bOK && !(dr.reg_Flags & CARRY_FLAG)) ? TRUE : FALSE;
}


BOOL GetDeviceParameters(HANDLE hVWin32, int nDrive, LPFAT_DEVICEPARAMS pdp)
{
	DIOC_REGISTERS dr = {0};
	dr.reg_EAX = 0x440d;
	dr.reg_EBX = nDrive;
	dr.reg_ECX = MAKEWORD(0x60, 0x48);
	dr.reg_EDX = PtrToUlong(pdp);

	DWORD dw;
	BOOL bOK = DeviceIoControl(hVWin32, VWIN32_DIOC_DOS_IOCTL, &dr, sizeof(dr), &dr, sizeof(dr), &dw, 0);
	if (!bOK || dr.reg_Flags & CARRY_FLAG) {
		dr.reg_EAX = 0x440d;
		dr.reg_EBX = nDrive;
		dr.reg_ECX = MAKEWORD(0x60, 0x08);
		dr.reg_EDX = PtrToUlong(pdp);
		bOK = DeviceIoControl(hVWin32, VWIN32_DIOC_DOS_IOCTL, &dr, sizeof(dr), &dr, sizeof(dr), &dw, 0);
	}
	return (bOK && !(dr.reg_Flags & CARRY_FLAG)) ? TRUE : FALSE;
}

/* @@@@
BOOL FlushDrive(HANDLE hVWin32, int nDrive)
{
	DIOC_REGISTERS dr = {0};
	dr.reg_EAX = 0x710d;
	dr.reg_EBX = 0;
	dr.reg_ECX = 0;					// flag 0/1/2
	dr.reg_EDX = nDrive;

	DWORD dw;
	BOOL bOK = DeviceIoControl(hVWin32, VWIN32_DIOC_DOS_IOCTL, &dr, sizeof(dr), &dr, sizeof(dr), &dw, 0);
	return (bOK && !(dr.reg_Flags & CARRY_FLAG)) ? TRUE : FALSE;
}

extern "C"
BOOL WINAPI fopGetDriveSectorInfo(LPCTSTR pszRoot, LPDWORD lpdwBytesPerSector, LPDWORD lpdwSectorsPerCluster)
{
	BOOL bOK = FALSE;
	int nDrive = 0;
	if (pszRoot)
		nDrive = PathGetDriveNumber(pszRoot) + 1;

	HANDLE hVWin32 = OpenVWin32();
	if (hVWin32 != INVALID_HANDLE_VALUE) {
		FAT_DEVICEPARAMS* pdp = new FAT_DEVICEPARAMS;
		memset(pdp, 0, sizeof(FAT_DEVICEPARAMS));
		if (GetDeviceParameters(hVWin32, nDrive, pdp)) {
			*lpdwBytesPerSector = pdp->wBytesPerSector;
			*lpdwSectorsPerCluster = pdp->bSectorsPerCluster;
			bOK = TRUE;
		}
		delete pdp;
		CloseVWin32(hVWin32);
	}
	return bOK;
}

extern "C"
BOOL WINAPI fopGetFatDeviceParams(LPCTSTR pszRoot, LPFAT_DEVICEPARAMS pdp)
{
	// no suppoort for NT
	return FALSE;
}
@@@@ */

static BOOL checkmedia(LPTSTR pszDriveName)
{
	UINT mode = SetErrorMode(SEM_FAILCRITICALERRORS);

	CString str = pszDriveName;
	str += _T('.');

	BOOL bOK = _pathfileexists(str);
	SetErrorMode(mode);
	return bOK;
}

extern "C"
BOOL WINAPI fopFlushDrive(LPCTSTR pszRoot)
{
	TCHAR root[MAX_PATH * 2];
	if (!pszRoot || !ISALPHA(*pszRoot))
		GetCurrentDirectory(ARRAYSIZE(root), root);
	else
		lstrcpy(root, pszRoot);

	root[1] = ':';
	root[2] = '\\';
	root[3] = '\0';
	CharUpper(root);

	switch (GetDriveType(root)) {
	case DRIVE_REMOVABLE:
		if (!checkmedia(root)) return FALSE;
		break;

	case DRIVE_FIXED:
		break;

	default:
		return FALSE;
	}

	BOOL bOK = FALSE;
	TCHAR vol[MAX_PATH * 2];
	wsprintf(vol, _T("\\\\.\\%c:"), root[0]);
	HANDLE hf = CreateFile(vol, GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, 0, 0);
	if (hf != INVALID_HANDLE_VALUE) {
		bOK = FlushFileBuffers(hf);
		CloseHandle(hf);
	}
	return bOK;
}
