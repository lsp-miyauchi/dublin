//////////////////////////////////////////////////////////////////////
// fdiutl.c

#include "stdafx.h"
#include "cabstub.h"
#include <fdi.h>


//////////////////////////////////////////////////////////////////////
//

class CExtractInfo
{
public:
	INT_PTR m_hf;
	CSfxInfo* m_psi;
	int m_action;
	bool m_yesall;
	CProgressDlg m_progress;

public:
	CExtractInfo()
		: m_hf(0)
		, m_psi(NULL)
		, m_action(ACTION_IGNORE)
		, m_yesall(false)
	{
	}
};

CExtractInfo* __EI = NULL;


//////////////////////////////////////////////////////////////////////
//

static LPBYTE spbCabData = NULL;
static DWORD sdwLen = 0;


//////////////////////////////////////////////////////////////////////
//
//

static void* DIAMONDAPI cab_alloc(ULONG cb)
{
	return malloc(cb);
}

static void DIAMONDAPI cab_free(void* pv)
{
	if (pv)
		free(pv);
}

static INT_PTR DIAMONDAPI cab_open(char* /*pszFile*/, int /*oflag*/, int /*pmode*/)
{
	filemap* pfm = new filemap;
	if (!pfm->open_mem(spbCabData, sdwLen)) {
		delete pfm;
		pfm = NULL;
	}
	return (pfm != NULL) ? (INT_PTR)pfm : (-1);
}

static UINT DIAMONDAPI cab_read(INT_PTR hf, void* pv, UINT cb)
{
	filemap* pfm = (filemap*)hf;
	if (pfm->read(pv, cb) != cb)
		cb = (UINT)-1;
	return cb;
}

static UINT FAR DIAMONDAPI cab_write(INT_PTR hf, void* pv, UINT cb)
{
	DWORD dw;
	if (!WriteFile((HANDLE)hf, pv, cb, &dw, NULL) || dw != cb)
		cb = (UINT)-1;

	if (cb > 0 && __EI->m_hf == hf)
		__EI->m_progress.SetComplete(cb);
	return cb;
}

static int FAR DIAMONDAPI cab_close(INT_PTR hf)
{
	return _lclose((int)hf);
}

static long FAR DIAMONDAPI cab_seek(INT_PTR hf, long dist, int seektype)
{
	filemap* pfm = (filemap*)hf;
	return pfm->seek(dist, seektype);
}

INT_PTR FAR DIAMONDAPI CabExtractNotify(FDINOTIFICATIONTYPE fdint, PFDINOTIFICATION pfdin)
{
	CExtractInfo* pei = (CExtractInfo*)pfdin->pv;
	char path[MAX_PATH * 2];
	WCHAR wname[MAX_PATH * 2], wpath[MAX_PATH * 2];

	switch (fdint) {
		case fdintCABINET_INFO: // general information about the cabinet
			break;

		case fdintNEXT_CABINET:	// file continued to next cabinet
		case fdintPARTIAL_FILE: // first file in cabinet is continuation
			return -1;

		case fdintCOPY_FILE:	// file to be copied
			if (pfdin->attribs & _A_NAME_IS_UTF)
				DecodeUTFToWStr((BYTE*)pfdin->psz1, -1, wpath, ARRAYSIZE(wpath));
			else
				MultiByteToWideChar(CP_ACP, 0, pfdin->psz1, -1, wpath, ARRAYSIZE(wpath));

			xPathCombineW(wname, pei->m_psi->m_strPath, wpath);
			xPathGetParentW(wname, wpath);

			WideCharToMultiByte(CP_ACP, 0, wpath, -1, path, ARRAYSIZE(path), NULL, NULL);
			__EI->m_progress.SetTitle(wpath);

			if (!xPathIsDirectory(wpath)) {
				if (!xPathMakeTree(wpath))
					return -1;
				pei->m_psi->m_filearray.Add(wpath);
			}

			if (xPathFileExists(wname)) {
				if (!pei->m_yesall) {
					int res = GetCopyAction(wname, pei->m_action, pei->m_yesall);
					if (res == ACTION_CANCEL)
						return -1;
					pei->m_action = res;
				}
				if (pei->m_action == ACTION_IGNORE)
					return 0;
			}
			pei->m_hf = (INT_PTR)CreateFileW(wname, GENERIC_READ | GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
			return (pei->m_hf != (INT_PTR)INVALID_HANDLE_VALUE) ? pei->m_hf : -1;

		case fdintCLOSE_FILE_INFO:	// close the file, set relevant info
		{
			CloseHandle((HANDLE)pfdin->hf);

			if (pfdin->attribs & _A_NAME_IS_UTF)
				DecodeUTFToWStr((BYTE*)pfdin->psz1, -1, wpath, ARRAYSIZE(wpath));
			else
				MultiByteToWideChar(CP_ACP, 0, pfdin->psz1, -1, wpath, ARRAYSIZE(wpath));
			xPathCombineW(wname, pei->m_psi->m_strPath, wpath);

			HANDLE hf = CreateFileW(wname, GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
			if (hf != INVALID_HANDLE_VALUE) {
				FILETIME ft;
				if (DosDateTimeToFileTime(pfdin->date, pfdin->time, &ft)) {
					FILETIME lft;
					if (LocalFileTimeToFileTime(&ft, &lft))
						SetFileTime(hf, &lft, NULL, &lft);
				}
				CloseHandle(hf);
			}


            DWORD attr = pfdin->attribs;
            attr &= (_A_RDONLY | _A_HIDDEN | _A_SYSTEM | _A_ARCH);
			SetFileAttributesW(wname, attr);
			pei->m_psi->m_filearray.Add(wname);
			return TRUE;
		}
	}
	return 0;
}

BOOL fdiExtract(HWND hwnd, BYTE* pbc, CSfxInfo* psi)
{
	// キャビネットファイルか否かの検査
	if (!pbc || memcmp(pbc, "MSCF", 4) || !psi)
		return FALSE;

	ERF erf;
	HFDI hfdi = FDICreate(
					cab_alloc,
					cab_free,
					cab_open,
					cab_read,
					cab_write,
					cab_close,
					cab_seek,
					cpu80386,
					&erf);
	if (hfdi == NULL)
		return FALSE;

	spbCabData = pbc;
	sdwLen = psi->m_lCabDataSize;

	CExtractInfo ei;
	ei.m_progress.Start(hwnd, psi->m_lTotalUncompressedSize);
	ei.m_psi = psi;
	__EI = &ei;

	char path[MAX_PATH];
	GetTempPathA(ARRAYSIZE(path), path);
	xPathAddBackslashA(path);
	BOOL bOK = FDICopy(hfdi, "dm:", path, 0, CabExtractNotify, NULL, &ei);
	FDIDestroy(hfdi);

	ei.m_progress.Stop();
	__EI = NULL;

	return bOK;
}
