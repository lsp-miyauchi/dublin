//////////////////////////////////////////////////////////////////////
// lspmedia.h

#ifndef __LSPMEDIA_H__
#define __LSPMEDIA_H__

typedef BOOL (CALLBACK* LLFPROC)(UINT iPart, LPARAM lParam);

#ifdef __cplusplus
extern "C" {                    // Assume C declarations for C++
#endif

HANDLE WINAPI fmOpenVolume(int iDrive, DWORD dwAccessFlags);
BOOL WINAPI fmCloseVolume(HANDLE hVolume);
BOOL WINAPI fmLockVolume(HANDLE hVolume);
BOOL WINAPI fmUnlockVolume(HANDLE hVolume);
BOOL WINAPI fmDismountVolume(HANDLE hVolume);
BOOL WINAPI fmPreventRemovalOfVolume(HANDLE hVolume, BOOL fPreventRemoval);
BOOL WINAPI fmAutoEjectVolume(HANDLE hVolume);
BOOL WINAPI fmEjectVolume(int iDrive);
BOOL WINAPI fmGetDiskGeometry(HANDLE hVolume, PDISK_GEOMETRY lpGeometry);
int WINAPI fmGetMediaTypes(HANDLE hVolume, LPVOID lpOutBuf, DWORD dwSize);
BOOL WINAPI fmLowLevelFormat(HANDLE hVolume, PDISK_GEOMETRY lpgm, LLFPROC pfn, LPARAM lParam);
DWORD WINAPI fmDiskCheck(int nDrive);
void WINAPI fmMediaInfo(HWND hwnd, int nDrive);
BOOL WINAPI fmRestoreImage(HWND hwnd, int nDrive, LPCTSTR lpImage, BOOL bFormat);
BOOL WINAPI fmRestoreImageUI(HWND hwnd);
BOOL WINAPI fmMakeImage(HWND hwnd, int nDrive, LPCTSTR lpImage);
BOOL WINAPI fmMakeImageUI(HWND hwnd);
void WINAPI fmCheckDisk(HWND hwnd);

#ifdef __cplusplus
}
#endif

#endif				// __LSPMEDIA_H__
