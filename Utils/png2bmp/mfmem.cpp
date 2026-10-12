#define     STRICT      // enable strict type checking

#include "stdafx.h"
#include "mfmem.h"

#define GLOWSIZE				0x10000

static HGLOBAL __fastcall readfileall_core(HANDLE hf, LPDWORD pdwRead)
{
	*pdwRead = 0;
	DWORD dwSize = GetFileSize(hf, NULL);
	HGLOBAL hmem = GlobalAlloc(GMEM_MOVEABLE, dwSize + 16);
	if (!hmem)
	{
		*pdwRead = GetLastError();
		return NULL;
	}

	LPVOID pv = GlobalLock(hmem);
	BOOL bOK = ReadFile(hf, pv, dwSize, pdwRead, NULL);
	if (!bOK)
		*pdwRead = GetLastError();

	GlobalUnlock(hmem);

	if (!bOK)
	{
		GlobalFree(hmem);
		return NULL;
	}
	return hmem;
}

HGLOBAL readfileall(LPCTSTR lpFile, LPDWORD pdwRead)
{
	if (!lpFile || !(*lpFile) || !pdwRead)
		return NULL;

	HANDLE hf = CreateFile(lpFile,
							GENERIC_READ,
							FILE_SHARE_READ | FILE_SHARE_WRITE,
							NULL,
							OPEN_EXISTING,
							0,
							NULL);
	if (hf == INVALID_HANDLE_VALUE)
	{
		*pdwRead = GetLastError();
		return NULL;
	}

	HGLOBAL hmem = readfileall_core(hf, pdwRead);
	CloseHandle(hf);
	return hmem;
}

_inline DWORD _mfspacesize(LPMF pmf)
	{ return (pmf->dwSize - pmf->dwOffset); }

static BOOL _mfglow(LPMF pmf, DWORD glow)
{
	GlobalLock(pmf->hmem);

	DWORD dwNew = pmf->dwSize + glow;
	HGLOBAL hNew = GlobalReAlloc(pmf->hmem, dwNew, GMEM_MOVEABLE);
	if (hNew)
	{
		pmf->dwSize = dwNew;
		pmf->hmem = hNew;
		pmf->pTop = (LPBYTE)GlobalLock(hNew);
		pmf->pRead = pmf->pTop + pmf->dwOffset;
		return TRUE;
	}

	pmf->pTop = (LPBYTE)GlobalLock(pmf->hmem);
	pmf->pRead = pmf->pTop + pmf->dwOffset;
	return FALSE;
}

LPMF _mfopen(LPCTSTR pszName)
{
	LPMF pmf = (LPMF)malloc(sizeof(MF));
	if (!pmf)
		return NULL;

	memset(pmf, 0, sizeof(MF));
	pmf->hmem = readfileall(pszName, &pmf->dwSize);
	if (!pmf->hmem)
	{
		free(pmf);
		return NULL;
	}

	pmf->pTop = pmf->pRead = (LPBYTE)GlobalLock(pmf->hmem);
	if (!pmf->pRead)
	{
		GlobalFree(pmf->hmem);
		free(pmf);
		return NULL;
	}
	return pmf;
}

LPMF _mfnew(DWORD dwSize)
{
	LPMF pmf = (LPMF)malloc(sizeof(MF));
	if (!pmf)
		return NULL;

	memset(pmf, 0, sizeof(MF));
	pmf->hmem = GlobalAlloc(GMEM_MOVEABLE, dwSize);
	if (!pmf->hmem)
	{
		free(pmf);
		return NULL;
	}

	pmf->dwSize = dwSize;
	pmf->pTop = pmf->pRead = (LPBYTE)GlobalLock(pmf->hmem);
	if (!pmf->pRead)
	{
		GlobalFree(pmf->hmem);
		free(pmf);
		return NULL;
	}
	return pmf;
}

LPMF _mfmemopen(HGLOBAL hmem, DWORD dwSize)
{
	if (!hmem)
		return NULL;

	LPMF pmf = (LPMF)malloc(sizeof(MF));
	if (!pmf)
		return NULL;

	memset(pmf, 0, sizeof(MF));

	if (!dwSize)
		dwSize = GlobalSize(hmem);
	pmf->hmem = GlobalAlloc(GMEM_MOVEABLE, dwSize);
	if (!pmf->hmem)
	{
		free(pmf);
		return NULL;
	}

	pmf->dwSize = dwSize;
	pmf->pTop = pmf->pRead = (LPBYTE)GlobalLock(pmf->hmem);
	if (!pmf->pRead)
	{
		GlobalFree(pmf->hmem);
		free(pmf);
		return NULL;
	}
	LPVOID lpw = GlobalLock(hmem);
	memcpy(pmf->pTop, lpw, dwSize);
	GlobalUnlock(hmem);

	return pmf;
}

LPMF _mfattach(HGLOBAL hmem)
{
	if (!hmem)
		return NULL;

	LPMF pmf = (LPMF)malloc(sizeof(MF));
	if (!pmf)
		return NULL;
	memset(pmf, 0, sizeof(MF));

	pmf->hmem = hmem;
	pmf->dwSize = GlobalSize(hmem);
	pmf->pTop = pmf->pRead = (LPBYTE)GlobalLock(hmem);
	return pmf;
}

HGLOBAL _mfdetach(LPMF pmf, DWORD* pdwSize)
{
	if (!pmf)
		return NULL;

	GlobalUnlock(pmf->hmem);
	HGLOBAL hmem = pmf->hmem;
	if (pdwSize)
		*pdwSize = pmf->dwOffset;
	free(pmf);

	return hmem;
}

void _mfclose(LPMF pmf)
{
	if (pmf)
	{
		if (pmf->hmem)
		{
			if (pmf->pTop)
				GlobalUnlock(pmf->hmem);
			GlobalFree(pmf->hmem);
		}
		free(pmf);
	}
}

BYTE _mfgetc(LPMF pmf)
{
	if (pmf && pmf->dwSize > pmf->dwOffset)
	{
		BYTE ch = *pmf->pRead++;
		pmf->dwOffset++;
		return ch;
	}
	return 0;
}

BYTE _mfgetc_ns(LPMF pmf)
{
	if (pmf && pmf->dwSize > pmf->dwOffset)
		return (*pmf->pRead);
	return 0;
}

WORD _mfgetw(LPMF pmf)
{
	if (pmf && pmf->dwSize > pmf->dwOffset + 1)
	{
		WORD w = *((LPWORD)pmf->pRead);
		pmf->pRead += 2;
		pmf->dwOffset += 2;
		return w;
	}
	return 0;
}

WORD _mfgetw_ns(LPMF pmf)
{
	if (pmf && pmf->dwSize > pmf->dwOffset + 1)
		return *((LPWORD)pmf->pRead);
	return 0;
}

int _mfread(LPMF pmf, LPVOID lpv, DWORD size)
{
	if (!pmf)
		return 0;

	DWORD nRead;
	if (pmf->dwSize >= pmf->dwOffset + size)
		nRead = size;
	else
		nRead = pmf->dwSize - pmf->dwOffset;

	if (nRead > 0)
	{
		memcpy(lpv, pmf->pRead, nRead);
		pmf->pRead += nRead;
		pmf->dwOffset += nRead;
	}
	return nRead;
}

long _mfseek(LPMF pmf, long lOffset, int nOrign)
{
	if (!pmf || !pmf->hmem || !pmf->pRead)
		return 0;

	switch (nOrign)
	{
		case FILE_CURRENT:
			lOffset += pmf->dwOffset;
			if (lOffset < 0)
				lOffset = 0;
			else if (lOffset > (long)pmf->dwSize)
				lOffset = (long)pmf->dwSize;
			// fall through

		case FILE_BEGIN:
			pmf->pRead = pmf->pTop;
			pmf->dwOffset = 0;

			if (lOffset <= 0)
				break;

			if (lOffset > (long)pmf->dwSize)
				lOffset = (long)pmf->dwSize;
			pmf->dwOffset = lOffset;
			pmf->pRead += lOffset;
			break;

		case FILE_END:
			pmf->pRead = pmf->pTop;
			pmf->dwOffset = pmf->dwSize;
			pmf->pRead += pmf->dwSize;

			if (lOffset >= 0)
				break;

			lOffset *= (-1);
			if (lOffset > (long)pmf->dwSize)
				lOffset = (long)pmf->dwSize;
			pmf->dwOffset -= lOffset;
			pmf->pRead -= lOffset;
			break;

		default:
			break;
	}
	return pmf->dwOffset;
}

BOOL _mfeof(LPMF pmf)
{
	return (!pmf || pmf->dwSize <= pmf->dwOffset) ? TRUE : FALSE;
}

BOOL _mfputc(LPMF pmf, BYTE ch)
{
	if (!pmf)
		return 0;

	if (_mfspacesize(pmf) < 1)
	{
		if (!_mfglow(pmf, GLOWSIZE))
			return FALSE;
	}

	*pmf->pRead++ = ch;
	pmf->dwOffset++;
	return TRUE;
}

BOOL _mfputw(LPMF pmf, WORD ch)
{
	if (!pmf)
		return 0;

	if (_mfspacesize(pmf) < 2)
	{
		if (!_mfglow(pmf, GLOWSIZE))
			return FALSE;
	}

	*((LPWORD)pmf->pRead) = ch;
	pmf->pRead += 2;
	pmf->dwOffset += 2;
	return TRUE;
}

DWORD _mfwrite(LPMF pmf, LPVOID lpv, DWORD size)
{
	if (!pmf)
		return 0;

	if (_mfspacesize(pmf) < size)
	{
		if (!_mfglow(pmf, max(size, GLOWSIZE)))
			return 0;
	}

	memcpy(pmf->pRead, lpv, size);
	pmf->pRead += size;
	pmf->dwOffset += size;

	return size;
}

BOOL _mfsave(LPMF pmf, LPTSTR pszName)
{
	if (!pmf)
		return FALSE;

	HANDLE hf = CreateFile(pszName, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS,
					FILE_ATTRIBUTE_ARCHIVE, NULL);
	if (hf == INVALID_HANDLE_VALUE)
		return FALSE;

	DWORD dw;
	BOOL bOK = WriteFile(hf, pmf->pTop, pmf->dwOffset, &dw, NULL);
	CloseHandle(hf);
	if (!bOK || dw != pmf->dwOffset)
	{
		bOK = FALSE;
		DeleteFile(pszName);
	}
	return bOK;
}

BOOL _mfsetsize(LPMF pmf, DWORD dwSize)
{
	if (!pmf)
		return FALSE;

	if (dwSize == pmf->dwSize)
		return TRUE;

	if (!pmf->hmem)
	{
		pmf->hmem = GlobalAlloc(GMEM_MOVEABLE, dwSize);
		if (!pmf->hmem)
			return FALSE;

		pmf->dwSize = dwSize;
		pmf->pTop = pmf->pRead = (LPBYTE)GlobalLock(pmf->hmem);
		pmf->dwOffset = 0;
		return TRUE;
	}

	HGLOBAL h = GlobalAlloc(GMEM_MOVEABLE, dwSize);
	if (h)
	{
		LPBYTE pTop = (LPBYTE)GlobalLock(h);
		DWORD offs;
		if (dwSize < pmf->dwSize)
		{
			memcpy(pTop, pmf->pTop, dwSize);
			offs = min(dwSize, max(0, pmf->dwOffset));
		}
		else
		{
			memcpy(pTop, pmf->pTop, pmf->dwSize);
			offs = pmf->dwOffset;
		}
		GlobalUnlock(pmf->hmem);
		GlobalFree(pmf->hmem);

		pmf->hmem = h;
		pmf->dwSize = dwSize;
		pmf->pTop = pTop;
		pmf->dwOffset = offs;
		pmf->pRead = pTop + offs;
		return TRUE;
	}
	return FALSE;
}

HGLOBAL _mfdup(LPMF pmf, DWORD* pdwSize)
{
	if (!pmf)
		return NULL;

	HGLOBAL hmem = GlobalAlloc(GMEM_MOVEABLE, pmf->dwOffset);
	if (hmem)
	{
		LPVOID lpw = GlobalLock(hmem);
		memcpy(lpw, pmf->pTop, pmf->dwOffset);
		GlobalUnlock(hmem);
		if (pdwSize)
			*pdwSize = pmf->dwOffset;
	}
	return hmem;
}

USHORT _mf_msb1streadshort(LPMF pmf)
{
	BYTE buf[2];
	USHORT value;

	if (_mfread(pmf, buf, 2) == 2)
	{
		value = (UINT)(buf[0] << 8);
		value |= (UINT)(buf[1]);
		return value;
	}
	return (USHORT)(~0);
}

ULONG _mf_msb1streadlong(LPMF pmf)
{
	BYTE buf[4];
	ULONG value;

	if (_mfread(pmf, buf, 4) == 4)
	{
		value = (ULONG)buf[0] << 24;
		value |= (ULONG)buf[1] << 16;
		value |= (ULONG)buf[2] << 8;
		value |= (ULONG)buf[3];
		return value;
	}
	return (ULONG)(~0);
}

DWORD _mf_msb1stwritelong(LPMF pmf, ULONG value)
{
	BYTE cb[4];
	cb[0] = (BYTE)((value & 0xff000000) >> 24);
	cb[1] = (BYTE)((value & 0xff0000) >> 16);
	cb[2] = (BYTE)((value & 0xff00) >> 8);
	cb[3] = (BYTE)(value & 0xff);
	return _mfwrite(pmf, cb, 4);
}

DWORD _mf_msb1stwriteshort(LPMF pmf, ULONG value)
{
	BYTE cb[2];
	cb[0] = (BYTE)((value) >> 8);
	cb[1] = (BYTE)(value & 0xff);
	return _mfwrite(pmf, cb, 2);
}

