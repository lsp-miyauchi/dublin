//////////////////////////////////////////////////////////////////////
// cabsfx

#pragma once
#ifndef __CABSFX_H__
#define __CABSFX_H__


#define MakePtr(cast, base, ofs)	((cast)((BYTE*)(base) + (ofs)))

#define SFXMAGIC			0x4D4B				// "KM"

#define SFX_TITLE			0x0001
#define SFX_MESSAGE			0x0002
#define SFX_PATH			0x0004
#define SFX_COMMAND			0x0008
#define SFX_NOQUERYPATH		0x0010
#define SFX_AUTODELETE		0x0020
#define SFX_RESOURCE		0x8000

#define ID_SFX_INFO			256
#define ID_SFX_CABDATA		257

#pragma pack(1)

typedef struct tagSFX {
	WORD wMagic;
	WORD wSize;
	WORD wStat;
	DWORD lTotalUncompressedSize;
	DWORD lCabDataSize;
	DWORD dummy;
	WCHAR wszString[1];
} SFX;
typedef SFX* LPSFX;

typedef struct tagSFXFOOT {
	WORD wMagic;
	DWORD lCabDataOffset;
	DWORD lCabDataSize;
	WORD dummy[3];
} SFXFOOT;

#pragma pack()


#endif		// __CABSFX_H__
