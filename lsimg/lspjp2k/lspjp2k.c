/******************************************************************************\
* Includes.
\******************************************************************************/

#include <windows.h>
#include <windowsx.h>
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

#include <jasper/jasper.h>

/******************************************************************************\
*
\******************************************************************************/

#define OPTSMAX				4096
#define WIDTHBYTES(bits)    (((bits) + 31) / 32 * 4)

/******************************************************************************\
* Types.
\******************************************************************************/

/******************************************************************************\
* Local prototypes.
\******************************************************************************/

static HANDLE _readfileall(LPCSTR pFile, LPDWORD pdwRead);
static HANDLE _readfileallW(LPCWSTR pFile, LPDWORD pdwRead);
static BOOL _writefileall(LPCSTR pFile, HANDLE hmem, DWORD dwWrite);
static BOOL _writefileallW(LPCWSTR pFile, HANDLE hmem, DWORD dwWrite);

/******************************************************************************\
* Code.
\******************************************************************************/

void jp2_init()
{
	jas_init();
}


HANDLE WINAPI jp2_load_mem(HANDLE hsrc)
{
	jas_image_t *image;
	jas_stream_t *in;
	jas_stream_t *out;
	int fmtid;
	long rawsize;
	HANDLE hdst;
	void* pv;

	if (!hsrc)
		return NULL;

	pv = GlobalLock(hsrc);
	if (!(in = jas_stream_memopen(pv, GlobalSize(hsrc)))) {
		GlobalUnlock(hsrc);
		fprintf(stderr, "error: cannot open input memory stream\n");
		return NULL;
	}

	fmtid = jas_image_strtofmt("jp2");
	image = jas_image_decode(in, fmtid, 0);
	jas_stream_close(in);
	GlobalUnlock(hsrc);

	if (!image) {
		fprintf(stderr, "cannot load image\n");
		return NULL;
	}

	if (!(out = jas_stream_memopen(NULL, 0))) {
		jas_image_destroy(image);
		fprintf(stderr, "error: cannot open output memory stream\n");
		return NULL;
	}

	fmtid = jas_image_strtofmt("lsp");
	if (jas_image_encode(image, out, fmtid, NULL)) {
		jas_image_destroy(image);
		jas_stream_close(out);
		fprintf(stderr, "error: cannot encode image\n");
	}
	jas_stream_flush(out);
	jas_image_destroy(image);

	rawsize = jas_stream_length(out);
	hdst = GlobalAlloc(GMEM_MOVEABLE, (DWORD)rawsize);
	if (hdst)
	{
		void* pv = GlobalLock(hdst);
		jas_stream_rewind(out);
		jas_stream_read(out, pv, (int)rawsize);
		GlobalUnlock(hdst);
	}
	jas_stream_close(out);

	return hdst;
}

HANDLE WINAPI jp2_load(LPCSTR pName)
{
	DWORD dw;
	HANDLE hmem, hdib;

	hdib = NULL;
	hmem = _readfileall(pName, &dw);
	if (hmem)
	{
		hdib = jp2_load_mem(hmem);
		GlobalFree(hmem);
	}
	return hdib;
}

HANDLE WINAPI jp2_loadW(LPCWSTR pName)
{
	DWORD dw;
	HANDLE hmem, hdib;

	hdib = NULL;
	hmem = _readfileallW(pName, &dw);
	if (hmem)
	{
		hdib = jp2_load_mem(hmem);
		GlobalFree(hmem);
	}
	return hdib;
}

HANDLE WINAPI jp2_save_mem_ex(HANDLE hsrc, DWORD* psize, int rate, int xcb, int ycb, BOOL bNoMct, int reslevel)
{
	// rate は 1 ～ 100 までの整数値 -- 数値が大きいほど圧縮率が高い

	jas_image_t *image;
	jas_stream_t *in;
	jas_stream_t *out;
	int fmtid;
	BOOL bOK;
	char opt[256], buf[128];
	char* popt;
	void* pv;
	HANDLE hdst;

	if (!hsrc)
		return FALSE;

	pv = GlobalLock(hsrc);
	if (!(in = jas_stream_memopen(pv, GlobalSize(hsrc)))) {
		GlobalUnlock(hsrc);
		fprintf(stderr, "error: cannot open memory stream\n");
		return NULL;
	}

	fmtid = jas_image_strtofmt("lsp");
	image = jas_image_decode(in, fmtid, 0);
	GlobalUnlock(hsrc);
	jas_stream_close(in);

	if (!image) {
		fprintf(stderr, "cannot load image\n");
		return NULL;
	}

	if (!(out = jas_stream_memopen(NULL, 0))) {
		jas_image_destroy(image);
		fprintf(stderr, "error: cannot open output image\n");
		return NULL;
	}

	fmtid = jas_image_strtofmt("jp2");

	opt[0] = '\0';
	if (rate > 0 && rate <= 100)
	{
		sprintf(buf, "rate=%f ", (1.0 / (double)rate));
		strcat(opt, buf);
	}
	if (xcb > 0)
	{
		sprintf(buf, "cblkwidth=%d ", xcb);
		strcat(opt, buf);
	}
	if (ycb > 0)
	{
		sprintf(buf, "cblkheight=%d ", ycb);
		strcat(opt, buf);
	}
	if (bNoMct > 0)
		strcat(opt, "nomct ");
	if (reslevel > 0)
	{
		sprintf(buf, "numrlvls=%d ", reslevel);
		strcat(opt, buf);
	}

	popt = opt[0] ? opt : NULL;
	bOK = (jas_image_encode(image, out, fmtid, popt) != 0) ? FALSE : TRUE;
	jas_image_destroy(image);

	hdst = NULL;
	if (bOK)
	{
		jas_stream_flush(out);

		*psize = (DWORD)jas_stream_length(out);
		hdst = GlobalAlloc(GMEM_MOVEABLE, *psize);
		if (hdst)
		{
			pv = GlobalLock(hdst);
			jas_stream_rewind(out);
			jas_stream_read(out, pv, (int)*psize);
			GlobalUnlock(hdst);
		}
		else
			fprintf(stderr, "error: cannot assign memory\n");
	}
	else
		fprintf(stderr, "error: cannot encode image\n");
	jas_stream_close(out);

	return hdst;
}

BOOL WINAPI jp2_save_ex(LPSTR pName, HANDLE hsrc, int rate, int xcb, int ycb, BOOL bNoMct, int reslevel)
{
	DWORD size;
	HANDLE hdst;
	BOOL bOK;

	bOK = FALSE;
	hdst = jp2_save_mem_ex(hsrc, &size, rate, xcb, ycb, bNoMct, reslevel);
	if (hdst)
	{
		bOK = _writefileall(pName, hdst, size);
		GlobalFree(hdst);
	}
	return bOK;
}

BOOL WINAPI jp2_save_exW(LPWSTR pName, HANDLE hsrc, int rate, int xcb, int ycb, BOOL bNoMct, int reslevel)
{
	DWORD size;
	HANDLE hdst;
	BOOL bOK;

	bOK = FALSE;
	hdst = jp2_save_mem_ex(hsrc, &size, rate, xcb, ycb, bNoMct, reslevel);
	if (hdst)
	{
		bOK = _writefileallW(pName, hdst, size);
		GlobalFree(hdst);
	}
	return bOK;
}

BOOL WINAPI jp2_save(LPSTR pName, HANDLE hmem)
{
	return jp2_save_ex(pName, hmem, 8, 0, 0, FALSE, 0);
}

BOOL WINAPI jp2_saveW(LPWSTR pName, HANDLE hmem)
{
	return jp2_save_exW(pName, hmem, 8, 0, 0, FALSE, 0);
}

static HANDLE _readfileall_core(HANDLE hf, LPDWORD pdwRead)
{
	HANDLE hmem;
	DWORD dwSize;
	void* pv;
	BOOL bOK;

	*pdwRead = 0;
	dwSize = GetFileSize(hf, NULL);
	hmem = GlobalAlloc(GMEM_MOVEABLE, dwSize + 16);
	if (!hmem)
		return NULL;

	pv = GlobalLock(hmem);
	bOK = ReadFile(hf, pv, dwSize, pdwRead, NULL);
	GlobalUnlock(hmem);
	if (!bOK)
	{
		GlobalFree(hmem);
		return NULL;
	}
	return hmem;
}

static HANDLE _readfileall(LPCSTR pFile, LPDWORD pdwRead)
{
	HANDLE hf, hmem;
	if (!pFile || !(*pFile))
		return NULL;

	*pdwRead = 0;
	hf = CreateFile(pFile, GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE,
					NULL, OPEN_EXISTING, FILE_FLAG_SEQUENTIAL_SCAN, NULL);
	if (hf == INVALID_HANDLE_VALUE)
		return NULL;

	hmem = _readfileall_core(hf, pdwRead);
	CloseHandle(hf);
	return hmem;
}

static HANDLE _readfileallW(LPCWSTR pFile, LPDWORD pdwRead)
{
	HANDLE hf, hmem;
	if (!pFile || !(*pFile))
		return NULL;

	*pdwRead = 0;
	hf = CreateFileW(pFile, GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE,
					NULL, OPEN_EXISTING, FILE_FLAG_SEQUENTIAL_SCAN, NULL);
	if (hf == INVALID_HANDLE_VALUE)
		return NULL;

	hmem = _readfileall_core(hf, pdwRead);
	CloseHandle(hf);
	return hmem;
}

static BOOL _writefileall(LPCSTR pFile, HANDLE hmem, DWORD dwWrite)
{
	HANDLE hf;
	void* pv;
	BOOL bOK;
	DWORD dw;

	if (!pFile || !(*pFile))
		return FALSE;

	hf = CreateFile(pFile, GENERIC_READ | GENERIC_WRITE, 0,
					NULL, CREATE_ALWAYS, FILE_FLAG_SEQUENTIAL_SCAN, NULL);
	if (hf == INVALID_HANDLE_VALUE)
		return FALSE;

	pv = GlobalLock(hmem);
	bOK = WriteFile(hf, pv, dwWrite, &dw, NULL);
	GlobalUnlock(hmem);
	CloseHandle(hf);
	if (!bOK || dwWrite != dw)
		return FALSE;
	return TRUE;
}

static BOOL _writefileallW(LPCWSTR pFile, HANDLE hmem, DWORD dwWrite)
{
	HANDLE hf;
	void* pv;
	BOOL bOK;
	DWORD dw;

	if (!pFile || !(*pFile))
		return FALSE;

	hf = CreateFileW(pFile, GENERIC_READ | GENERIC_WRITE, 0,
					NULL, CREATE_ALWAYS, FILE_FLAG_SEQUENTIAL_SCAN, NULL);
	if (hf == INVALID_HANDLE_VALUE)
		return FALSE;

	pv = GlobalLock(hmem);
	bOK = WriteFile(hf, pv, dwWrite, &dw, NULL);
	GlobalUnlock(hmem);
	CloseHandle(hf);
	if (!bOK || dwWrite != dw)
		return FALSE;
	return TRUE;
}
