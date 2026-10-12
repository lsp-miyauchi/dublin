////////////////////////////////////////////////////////////////////////
// xjgcode.cpp

#include "stdafx.h"
#include <shlwapi.h>
#include "lsimg.h"
#include "common.h"

#include "lspcrypt\lspcrypt.h"


extern "C"
BOOL WINAPI imgXjgSave(LPCTSTR pszFile, HDIB hdib, int quality, LPCSTR code)
{
	DWORD dwSize;
	HGLOBAL hmem = imgJpgSaveMem(hdib, quality, JC_ISLOW, 0, &dwSize);
	if (hmem) {
		HANDLE hf = CreateFile(pszFile, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_ARCHIVE, NULL);
		if (hf == INVALID_HANDLE_VALUE)
			return FALSE;

		LPVOID lpv = (LPSTR)GlobalLock(hmem);

		CRYPTINFO cc;
		memset(&cc, 0, sizeof(cc));
		cc.cbSize = sizeof(cc);
		cc.pData = (LPBYTE)lpv;
		cc.pszCode = code;
		cc.dwSize = dwSize;
		cc.crypttype = CRYPT_XOR;
		BOOL bOK = lspCrypt(&cc);

		XJGHEADER xjg;
		memset(&xjg, 0 ,sizeof(xjg));
		xjg.magic = XJGMAGIC;
		xjg.version = XJGVERSION;
		xjg.size = dwSize;
		xjg.gtype = IMG_JPEG;
		if (bOK) {
			xjg.crypttype = CRYPT_XOR;
			xjg.crcCode = cc.crcCode;
			xjg.crcOriginal = cc.crcSrc;
		}
		else
			xjg.crypttype = CRYPT_NONE;

		DWORD dw;
		bOK = FALSE;
		if (WriteFile(hf, &xjg, sizeof(xjg), &dw, NULL) && dw == sizeof(xjg))
			bOK = (WriteFile(hf, lpv, dwSize, &dw, NULL) && dw == dwSize);
		CloseHandle(hf);
		GlobalUnlock(hmem);
		GlobalFree(hmem);

		if (!bOK)
			DeleteFile(pszFile);
		return bOK;
	}
	return FALSE;
}

extern "C"
int WINAPI imgXjgLoadMem(LPCTSTR fname, LPCSTR code, HANDLE* phmem, DWORD* pdwSize)
{
	if (!phmem || !pdwSize)
		return RXJG_ERROR;

	// ファイル全体をメモリへ読み込む
	MemStream* pms = _mfcreate(fname);
	if (!pms)
		return RXJG_ERROR;

	// ヘッダの検査
	XJGHEADER* pxjg = (XJGHEADER*)pms->m_baseptr;
	LPBYTE pSrc = (LPBYTE)pxjg;
	LPBYTE pDst = pSrc + sizeof(XJGHEADER);
	*pdwSize = pxjg->size;

	int result = RXJG_ERROR;
	if (pxjg->magic != XJGMAGIC) {
		result = RXJG_NOTXJG;
		goto exit_loadparams;
	}

	if (pxjg->crypttype != CRYPT_NONE) {
		if (!code || *code == '\0') {
			result = RXJG_PASSREQUIRED;
			goto exit_loadparams;
		}

		if (pxjg->crcCode != lspCodeCRC(code)) {
			result = RXJG_INVALIDPASS;
			goto exit_loadparams;
		}

		CRYPTINFO cc;
		memset(&cc, 0, sizeof(cc));
		cc.cbSize = sizeof(cc);
		cc.pData = pDst;
		cc.pszCode = code;
		cc.dwSize = *pdwSize;
		cc.crypttype = pxjg->crypttype;
		lspCrypt(&cc);
		if (cc.crcDst != pxjg->crcOriginal) {
			result = RXJG_CRCERROR;
			goto exit_loadparams;
		}
	}

	*phmem = GlobalAlloc(GMEM_MOVEABLE, *pdwSize);
	if (*phmem) {
		pSrc = (LPBYTE)GlobalLock(*phmem);
		memmove(pSrc, pDst, *pdwSize);
		GlobalUnlock(*phmem);
		result = pxjg->gtype;
	}
	else
		result = RXJG_ERROR;

exit_loadparams:
	_mfclose(pms);
	return result;
}

extern "C"
int WINAPI imgXjgLoad(LPCTSTR pszFile, LPCSTR code, HDIB* phdib)
{
	if (!phdib)
		return RXJG_ERROR;
	*phdib = NULL;

	XJGHEADER xjg;
	if (!imgXjgGetImageInfo(pszFile, &xjg))
		return RXJG_ERROR;

	HANDLE hmem;
	DWORD dwSize;
	int result = imgXjgLoadMem(pszFile, code, &hmem, &dwSize);

	if (result < 1)
		return result;

	if (result == IMG_JPEG)
		*phdib = imgJpgLoadMemParams(hmem, 0, 1, JC_ISLOW, JD_NONE, FALSE);
	else if (result == IMG_BMP)
		*phdib = imgDibLoadMem(hmem);
	else if (result == IMG_GIF)
		*phdib = imgGifLoadMem(hmem);
	else if (result == IMG_TGA)
		*phdib = imgTgaLoadMem(hmem);
	else if (result == IMG_PCX)
		*phdib = imgPcxLoadMem(hmem, dwSize);
	else if (result == IMG_RASTER)
		*phdib = imgRasLoadMem(hmem);
	else if (result == IMG_PNG)
		*phdib = imgPngLoadMem(hmem);
//	else if (result == IMG_TIFF)
//		*phdib = imgTifLoadMem(hmem);
//	else if (result == IMG_MAG)
//		*phdib = imgMagLoadMem(hmem);
//	else if (result == IMG_MAKI)
//		*phdib = imgMkiLoadMem(hmem);
//	else if (result == IMG_PSD)
//		*phdib = imgPsdLoadMem(hmem);
	else
	{
		// 一旦ファイルへ書き出す
		TCHAR path[MAX_PATH], name[MAX_PATH];
		GetTempPath(MAX_PATH, path);
		GetTempFileName(path, _T("~xjg"), 0, name);
		if (imgXjgDecryptFile(pszFile, name, code) == RXJG_NOERROR)
			*phdib = imgLoadImage(name);
		DeleteFile(name);
		return (*phdib != NULL) ? RXJG_NOERROR : RXJG_ERROR;
	}
	GlobalFree(hmem);
	return (*phdib != NULL) ? RXJG_NOERROR : RXJG_ERROR;
}

extern "C"
BOOL WINAPI imgXjgEncryptFile(LPCTSTR pszSrc, LPCTSTR pszDst, LPCSTR pszCode)
{
	int gtype = imgCheckImageType(pszSrc);
	if (gtype == IMG_NOTSUPPORTED || gtype == IMG_XJG)
		return FALSE;

	XJGHEADER xjg;
	memset(&xjg, 0, sizeof(xjg));
	xjg.magic = XJGMAGIC;
	xjg.version = XJGVERSION;
	xjg.gtype = (BYTE)gtype;
	xjg.crypttype = (pszCode && *pszCode) ? CRYPT_XOR : CRYPT_NONE;

	MemStream* pms = _mfcreate(pszSrc);
	if (!pms)
		return FALSE;

	DWORD dwSize = (DWORD)pms->m_size;
	xjg.size = dwSize;

	WIN32_FIND_DATA wfd;
	GetFileData(pszSrc, &wfd);

	// 暗号化
	LPBYTE pSrc = pms->m_baseptr;
	if (xjg.crypttype != CRYPT_NONE) {
		CRYPTINFO cc;
		memset(&cc, 0, sizeof(cc));
		cc.cbSize = sizeof(cc);
		cc.pData = pSrc;
		cc.pszCode = pszCode;
		cc.dwSize = dwSize;
		cc.crypttype = xjg.crypttype;
		lspCrypt(&cc);
		xjg.crcCode = cc.crcCode;
		xjg.crcOriginal = cc.crcSrc;
	}

	TCHAR dst[MAX_PATH];
	if (!pszDst || *pszDst == '\0') {
		lstrcpy(dst, pszSrc);
		TCHAR* p = PathFindExtension(dst);
		if (*p == '.')
			*p = 0;
		lstrcat(dst, _T(".xjg"));
	}
	else
		lstrcpy(dst, pszDst);

	BOOL bOK = FALSE;
	HANDLE hf = CreateFile(dst, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_ARCHIVE, NULL);
	if (hf != INVALID_HANDLE_VALUE) {
		DWORD dw, dww = 0;
		bOK = WriteFile(hf, &xjg, sizeof(xjg), &dw, NULL);
		if (bOK)
			dww += dw;

		bOK = WriteFile(hf, pSrc, dwSize, &dw, NULL);
		if (bOK)
			dww += dw;

		if (dww != (sizeof(xjg) + dwSize)) {
			CloseHandle(hf);
			DeleteFile(dst);
			bOK = FALSE;
		}
		else {
			SetFileTime(hf, &wfd.ftCreationTime, NULL, &wfd.ftLastWriteTime);
			CloseHandle(hf);
		}
	}
	_mfclose(pms);

	return bOK;
}

extern "C"
int WINAPI imgXjgDecryptFile(LPCTSTR pszSrc, LPCTSTR pszDst, LPCSTR pszCode)
{
	MemStream* pms = _mfcreate(pszSrc);
	if (!pms)
		return RXJG_ERROR;

	DWORD dwSize = pms->m_size;
	WIN32_FIND_DATA wfd;
	GetFileData(pszSrc, &wfd);

	LPBYTE pSrc = pms->m_baseptr;
	XJGHEADER* pxjg = (XJGHEADER*)pSrc;
	if (pxjg->magic != XJGMAGIC) {
		_mfclose(pms);
		return RXJG_NOTXJG;
	}

	BOOL bOK;
	dwSize = pxjg->size;
	pSrc += sizeof(XJGHEADER);
	if (pxjg->crypttype != CRYPT_NONE) {
		if (!pszCode || *pszCode == '\0') {
			_mfclose(pms);
			return RXJG_PASSREQUIRED;
		}

		if (pxjg->crcCode != lspCodeCRC(pszCode)) {
			_mfclose(pms);
			return RXJG_INVALIDPASS;
		}

		CRYPTINFO cc;
		memset(&cc, 0, sizeof(cc));
		cc.cbSize = sizeof(cc);
		cc.pData = pSrc;
		cc.pszCode = pszCode;
		cc.dwSize = dwSize;
		cc.crypttype = pxjg->crypttype;
		lspCrypt(&cc);
		if (cc.crcDst != pxjg->crcOriginal) {
			_mfclose(pms);
			return RXJG_CRCERROR;
		}
	}

	TCHAR dst[MAX_PATH];
	if (!pszDst || *pszDst == '\0') {
		lstrcpy(dst, pszSrc);
		TCHAR* p = PathFindExtension(dst);
		if (*p == '.')
			*p = 0;

		if (pxjg->gtype == IMG_BMP)
			lstrcat(dst, _T(".bmp"));
		else if (pxjg->gtype == IMG_JPEG)
			lstrcat(dst, _T(".jpg"));
		else if (pxjg->gtype == IMG_GIF)
			lstrcat(dst, _T(".gif"));
		else if (pxjg->gtype == IMG_TIFF)
			lstrcat(dst, _T(".tif"));
		else if (pxjg->gtype == IMG_TGA)
			lstrcat(dst, _T(".tga"));
		else if (pxjg->gtype == IMG_PCX)
			lstrcat(dst, _T(".pcx"));
		else if (pxjg->gtype == IMG_MAG)
			lstrcat(dst, _T(".mag"));
		else if (pxjg->gtype == IMG_MAKI)
			lstrcat(dst, _T(".mki"));
		else if (pxjg->gtype == IMG_RASTER)
			lstrcat(dst, _T(".ras"));
		else if (pxjg->gtype == IMG_PNG)
			lstrcat(dst, _T(".png"));
		else if (pxjg->gtype == IMG_PSD)
			lstrcat(dst, _T(".psd"));
	}
	else
		lstrcpy(dst, pszDst);

	int result = RXJG_ERROR;
	HANDLE hf = CreateFile(dst, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_ARCHIVE, NULL);
	if (hf != INVALID_HANDLE_VALUE) {
		DWORD dw;
		bOK = WriteFile(hf, pSrc, dwSize, &dw, NULL);
		if (!bOK || dw != dwSize) {
			CloseHandle(hf);
			DeleteFile(dst);
		}
		else {
			SetFileTime(hf, &wfd.ftCreationTime, NULL, &wfd.ftLastWriteTime);
			CloseHandle(hf);
			result = RXJG_NOERROR;
		}
	}
	_mfclose(pms);
	return result;
}

extern "C"
BOOL WINAPI imgXjgGetImageInfo(LPCTSTR pszFile, XJGHEADER* pHeader)
{
	if (!pszFile || *pszFile == '\0' || !pHeader)
		return FALSE;

	HANDLE hf = CreateFile(pszFile, GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE,
						NULL, OPEN_EXISTING, 0, NULL);
	if (hf == INVALID_HANDLE_VALUE)
		return FALSE;

	DWORD dw;
	BOOL bOK = ReadFile(hf, pHeader, sizeof(XJGHEADER), &dw, NULL);
	CloseHandle(hf);

	return (bOK && dw == sizeof(XJGHEADER) && pHeader->magic == XJGMAGIC);
}
