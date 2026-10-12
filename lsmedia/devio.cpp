// devio.cpp : implementation file
//

#include "stdafx.h"
#include "lsmedapi.h"

extern "C"
HANDLE WINAPI fmOpenVolume(int iDrive, DWORD dwAccessFlags)
{
	TCHAR root[MAX_PATH];
	wsprintf(root, _T("\\\\.\\%c:"), iDrive + 'A');
	return CreateFile(root, dwAccessFlags,
						FILE_SHARE_READ | FILE_SHARE_WRITE,
						NULL, OPEN_EXISTING, 0, NULL);
}

extern "C"
BOOL WINAPI fmCloseVolume(HANDLE hVolume)
{
	return CloseHandle(hVolume);
}

#define LOCK_TIMEOUT	1000
#define LOCK_RETRY		10

extern "C"
BOOL WINAPI fmLockVolume(HANDLE hVolume)
{
	DWORD dw;
	for (DWORD i = 0; i < LOCK_RETRY; i++) {
		if (DeviceIoControl(hVolume, FSCTL_LOCK_VOLUME, NULL, 0, NULL, 0, &dw, NULL))
			return TRUE;
		Sleep(LOCK_TIMEOUT / LOCK_RETRY);
	}
	return FALSE;
}

extern "C"
BOOL WINAPI fmUnlockVolume(HANDLE hVolume)
{
	DWORD dw;
    return DeviceIoControl(hVolume, FSCTL_UNLOCK_VOLUME, NULL, 0, NULL, 0, &dw, NULL);
}

extern "C"
BOOL WINAPI fmDismountVolume(HANDLE hVolume)
{
	DWORD dw;
	return DeviceIoControl(hVolume, FSCTL_DISMOUNT_VOLUME, NULL, 0, NULL, 0, &dw, NULL);
}

extern "C"
BOOL WINAPI fmPreventRemovalOfVolume(HANDLE hVolume, BOOL fPreventRemoval)
{
	PREVENT_MEDIA_REMOVAL pmr;
	pmr.PreventMediaRemoval = fPreventRemoval;
	DWORD dw;
	return DeviceIoControl(hVolume, IOCTL_STORAGE_MEDIA_REMOVAL, &pmr, sizeof(pmr), NULL, 0, &dw, NULL);
}

extern "C"
BOOL WINAPI fmAutoEjectVolume(HANDLE hVolume)
{
	DWORD dw;
	return DeviceIoControl(hVolume, IOCTL_STORAGE_EJECT_MEDIA, NULL, 0, NULL, 0, &dw, NULL);
}

extern "C"
BOOL WINAPI fmEjectVolume(int iDrive)
{
	HANDLE hVolume = fmOpenVolume(iDrive, 0);
	if (hVolume == INVALID_HANDLE_VALUE)
		return FALSE;

	BOOL fRemoveSafely = FALSE;
	BOOL fAutoEject = FALSE;
	if (fmLockVolume(hVolume) && fmDismountVolume(hVolume)) {
		fRemoveSafely = TRUE;
		if (fmPreventRemovalOfVolume(hVolume, FALSE) && fmAutoEjectVolume(hVolume))
			fAutoEject = TRUE;
	}
	return fmCloseVolume(hVolume);
}

extern "C"
BOOL WINAPI fmGetDiskGeometry(HANDLE hVolume, PDISK_GEOMETRY lpGeometry)
{
	DWORD dw;
	return DeviceIoControl(hVolume, IOCTL_DISK_GET_DRIVE_GEOMETRY, NULL, 0, lpGeometry, sizeof(*lpGeometry), &dw, NULL);
}

extern "C"
int WINAPI fmGetMediaTypes(HANDLE hVolume, LPVOID lpOutBuf, DWORD dwSize)
{
	DWORD dw;
	if (DeviceIoControl(hVolume, IOCTL_DISK_GET_MEDIA_TYPES, NULL, 0, lpOutBuf, dwSize, &dw, NULL) && dw > 0)
		return (int)(dw / sizeof(DISK_GEOMETRY));
	return 0;
}

extern "C"
int WINAPI fmStgGetMediaTypesEx(HANDLE hVolume, LPVOID lpOutBuf, DWORD dwSize)
{
	DWORD dw;
	return DeviceIoControl(hVolume, IOCTL_STORAGE_GET_MEDIA_TYPES_EX, NULL, 0, lpOutBuf, dwSize, &dw, NULL);
}

BOOL FormatCylinder(HANDLE hVolume, PDISK_GEOMETRY pgm, int nCylinder)
{
	FORMAT_PARAMETERS fpm;
	fpm.MediaType = pgm->MediaType;
	fpm.StartHeadNumber = 0;
	fpm.EndHeadNumber = pgm->TracksPerCylinder - 1;

	PBAD_TRACK_NUMBER pbtn = new BAD_TRACK_NUMBER[pgm->TracksPerCylinder];
	DWORD dwBtn = pgm->TracksPerCylinder * sizeof(BAD_TRACK_NUMBER);
	ZeroMemory(pbtn, dwBtn);

	DWORD dw;
	fpm.StartCylinderNumber = nCylinder;
	fpm.EndCylinderNumber = nCylinder;

	BOOL bOK = DeviceIoControl(hVolume, IOCTL_DISK_FORMAT_TRACKS,
							&fpm, sizeof(fpm), pbtn, dwBtn, &dw, NULL);
	delete [] pbtn;
	return bOK;
}

extern "C"
BOOL WINAPI fmLowLevelFormat(HANDLE hVolume, PDISK_GEOMETRY lpgm, LLFPROC pfn, LPARAM lParam)
{
/* @@@@
	FORMAT_PARAMETERS fpm;
	fpm.MediaType = lpgm->MediaType;
	fpm.StartHeadNumber = 0;
	fpm.EndHeadNumber = lpgm->TracksPerCylinder - 1;

	PBAD_TRACK_NUMBER pbtn = new BAD_TRACK_NUMBER[lpgm->TracksPerCylinder];
	DWORD dwBtn = lpgm->TracksPerCylinder * sizeof(BAD_TRACK_NUMBER);
	ZeroMemory(pbtn, dwBtn);

	DWORD dw;
	BOOL bOK = FALSE;
	for (UINT i = 0; i < lpgm->Cylinders.LowPart; i++)
	{
		fpm.StartCylinderNumber = i;
		fpm.EndCylinderNumber = i;
		bOK = DeviceIoControl(hVolume, IOCTL_DISK_FORMAT_TRACKS,
							&fpm, sizeof(fpm), pbtn, dwBtn, &dw, NULL);
		if (!bOK)
			break;

		if (pfn)
		{
			bOK = (*pfn)(i, lParam);
			if (!bOK)
				break;
		}
	}
	delete [] pbtn;
	return bOK;
@@@@ */

	BOOL bOK = FALSE;
	for (UINT i = 0; i < lpgm->Cylinders.LowPart; i++)
	{
		bOK = FormatCylinder(hVolume, lpgm, i);
		if (!bOK)
			break;
		if (pfn)
		{
			bOK = (*pfn)(i, lParam);
			if (!bOK)
				break;
		}
	}
	return bOK;
}

extern "C"
DWORD WINAPI fmDiskCheck(int nDrive)
{
	DWORD dwError = NOERROR;
	UINT mode = SetErrorMode(SEM_FAILCRITICALERRORS);
	HANDLE hVolume = fmOpenVolume(nDrive, GENERIC_READ);
	if (hVolume != INVALID_HANDLE_VALUE)
	{
		DWORD dw;
		if (!DeviceIoControl(hVolume, IOCTL_DISK_CHECK_VERIFY, 0, 0, 0, 0, &dw, 0))
			dwError = GetLastError();
		fmCloseVolume(hVolume);
	}
	else
		dwError = GetLastError();
	SetErrorMode(mode);

	return dwError;
}
