////////////////////////////////////////////////////////////////////////
// texture.h

#pragma once
#ifndef __TEXTURE_H__
#define __TEXTURE_H__

#define TEXTURE_MAGIC		0x21727874
#define TEXTURE_MAGIC_RGB	0x22727874
#define TEXTURE_NAME		80

#pragma pack(1)

typedef struct tagTEXTURE {
	DWORD magic;
	int width;
	int height;
	char comment[TEXTURE_NAME];
	BYTE map[1];
} TEXTURE;

#pragma pack()

TEXTURE* txr_create(int width, int height, int bpp);
TEXTURE* txr_create_from_dib(HDIB hdib, int bpp);
void txr_destroy(TEXTURE* t);
void txr_blend(RGBPixel* image, int width, int height, TEXTURE* t, int blendtype, int opaque);
TEXTURE* txr_load(LPCSTR pszName);
BOOL txr_save(TEXTURE* t, LPCSTR pszName);



#endif		// __TEXTURE_H__
