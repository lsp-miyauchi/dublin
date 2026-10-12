//////////////////////////////////////////////////////////////////////
// txrcode.cpp 独自テクスチャマップファイルの読み込み

#include "stdafx.h"
#include "lsimg.h"
#include "common.h"
#include "filemap.h"
#include "texture.h"

static void* load_resource_data(HINSTANCE hInst, UINT typeResource, UINT idResource, DWORD& size)
{
	HRSRC hrsrc = FindResource(hInst, MAKEINTRESOURCE(idResource), MAKEINTRESOURCE(typeResource));
	if (hrsrc) {
		size = SizeofResource(hInst, hrsrc);
		if (size) {
			HGLOBAL hglb = LoadResource(hInst, hrsrc);
			if (hglb)
				return LockResource(hglb);
		}
	}
	return NULL;
}

static BOOL txr_get_info(filemap& fm, LPSTR comment, int* width, int* height, int* bpp)
{
	TEXTURE* t = (TEXTURE*)fm.m_base;
	if (t->magic != TEXTURE_MAGIC && t->magic != TEXTURE_MAGIC_RGB) return FALSE;

	if (bpp)		*bpp = (t->magic == TEXTURE_MAGIC_RGB) ? 3 : 1;
	if (width)		*width = t->width;
	if (height)		*height = t->height;
	if (comment)	lstrcpynA(comment, t->comment, TEXTURE_NAME);
	return TRUE;
}

extern "C"
BOOL WINAPI imgGetTxrInfo(LPCTSTR pszName, LPSTR comment, int* width, int* height, int* bpp)
{
	BOOL bOK = FALSE;
	filemap fm;
	if (fm.open(pszName))
		bOK = txr_get_info(fm, comment, width, height, bpp);
	return bOK;
}

extern "C"
BOOL WINAPI imgGetResourceTxrInfo(HINSTANCE hInst, UINT typeResource, UINT idResource, LPSTR comment, int* width, int* height, int* bpp)
{
	BOOL bOK = FALSE;
	DWORD size;
	void* pr = load_resource_data(hInst, typeResource, idResource, size);
	if (pr) {
		filemap fm;
		if (fm.open_mem(pr, size))
			bOK = txr_get_info(fm, comment, width, height, bpp);
	}
	return bOK;
}

static HDIB txr_load_core(filemap& fm)
{
	TEXTURE* t = (TEXTURE*)fm.m_base;
	if (t->magic != TEXTURE_MAGIC && t->magic != TEXTURE_MAGIC_RGB) return NULL;

	int bpp = (t->magic != TEXTURE_MAGIC) ? 3 : 1;
	int size = (t->width * t->height) * bpp + sizeof(TEXTURE) - sizeof(BYTE);
	if (fm.filesize() < size)
		return NULL;

	mif image(t->width, t->height, 8 * bpp);
	RGBA* ps;
	int i;
	if (bpp == 1) {
		ps = image.m_map;
		for (i = 0; i < 256; i++) {
			ps->red = ps->green = ps->blue = i;
			ps++;
		}
	}

	BYTE* pd = t->map;
	ps = image.m_pixels;
	for (i = 0; i < image.m_packets; i++) {
		if (bpp == 1)
			ps->red = ps->green = ps->blue = ps->alpha = *pd++;
		else {
			ps->red   = *pd++;
			ps->green = *pd++;
			ps->blue  = *pd++;
		}
		ps++;
	}

	return image.createdib();
}

extern "C"
HDIB WINAPI imgTxrLoad(LPCTSTR pszName)
{
	HDIB hdib = NULL;
	filemap fm;
	if (fm.open(pszName))
		hdib = txr_load_core(fm);
	return hdib;
}

extern "C"
HDIB WINAPI imgTxrLoadFromResource(HINSTANCE hInst, UINT typeResource, UINT idResource)
{
	DWORD size;
	void* pr = load_resource_data(hInst, typeResource, idResource, size);
	if (pr) {
		filemap fm;
		fm.open_mem(pr, size);
		return txr_load_core(fm);
	}
	return NULL;
}
