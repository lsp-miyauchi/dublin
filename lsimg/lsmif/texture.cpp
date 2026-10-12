////////////////////////////////////////////////////////////////////////
// texture.cpp

#include "stdafx.h"
#include "lsmif.h"
#include "common.h"
#include "texture.h"
#include "filemap.h"


TEXTURE* txr_create(int width, int height, int bpp)
{
	int size = width * height;
	if (bpp == 3)
		size *= 3;

	TEXTURE* t = (TEXTURE*)GlobalAllocPtr(GMEM_MOVEABLE, size + sizeof(TEXTURE));
	memset(t, 0, sizeof(TEXTURE));
	t->magic = (bpp > 1) ? TEXTURE_MAGIC_RGB : TEXTURE_MAGIC;
	t->width = width;
	t->height = height;
	memset(t->map, 0, size);
	return t;
}

void txr_destroy(TEXTURE* t)
{
	if (t) GlobalFreePtr(t);
}

TEXTURE* txr_load(LPCSTR pszName)
{
	filemap fm;
	if (!fm.open(pszName))
		return NULL;

	TEXTURE* t = (TEXTURE*)fm.m_base;
	if (t->magic != TEXTURE_MAGIC && t->magic != TEXTURE_MAGIC_RGB)
		return NULL;

	int size = t->width * t->height;
	if (t->magic != TEXTURE_MAGIC)
		size *= 3;
	size += (sizeof(TEXTURE) - sizeof(BYTE));
	if (fm.m_size < size)
		return NULL;

	t = (TEXTURE*)GlobalAllocPtr(GMEM_MOVEABLE, size);
	fm.read(t, size);
	return t;
}

TEXTURE* txr_loadfromresource(HINSTANCE hInst, UINT typeResource, UINT idResource)
{
	DWORD sizeResource;
	void* pr = load_resource_data(hInst, typeResource, idResource, sizeResource);
	if (pr && sizeResource > 0) {
		TEXTURE* t = (TEXTURE*)pr;
		if (t->magic == TEXTURE_MAGIC || t->magic == TEXTURE_MAGIC_RGB) {
			DWORD size = t->width * t->height;
			if (t->magic != TEXTURE_MAGIC)
				size *= 3;
			size += (sizeof(TEXTURE) - sizeof(BYTE));
			if (sizeResource >= size) {
				t = (TEXTURE*)GlobalAllocPtr(GMEM_MOVEABLE, size);
				if (t) {
					memcpy(t, pr, size);
					return t;
				}
			}
		}
	}
	return NULL;
}


BOOL txr_save(TEXTURE* t, LPCSTR pszName)
{
	HANDLE hf = CreateFile(pszName, GENERIC_WRITE, 0, NULL,
							CREATE_ALWAYS, FILE_ATTRIBUTE_ARCHIVE, NULL);
	if (hf == INVALID_HANDLE_VALUE)
		return FALSE;

	DWORD dw;
	DWORD size = t->width * t->height;
	if (t->magic != TEXTURE_MAGIC)
		size *= 3;
	size += (sizeof(TEXTURE) - sizeof(BYTE));
	BOOL bOK = (WriteFile(hf, (void*)t, size, &dw, NULL) && dw == size);
	CloseHandle(hf);

	if (!bOK)
		DeleteFile(pszName);
	return bOK;
}
