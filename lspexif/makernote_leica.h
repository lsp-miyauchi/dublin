///////////////////////////////////////////////////////////////////////
// makernote_leica.h leica情報

#pragma once
#ifndef __MAKERNOTE_LEICA_H__
#define __MAKERNOTE_LEICA_H__

///////////////////////////////////////////////////////////////////////
// ライカモデル

enum {
	LEICA_UNKNOWN = 0,
	LEICA_DLUX2 = 0x100,					// "D-LUX 2"
	LEICA_CLUX1 = 0x200,					// "C-LUX 1"
	LEICA_M8 = 0x300,						// "M8 Digital Camera"
};

#define IS_LIECA_PANASONIC_COMPATIBLE(n)	((n)<LEICA_M8)

///////////////////////////////////////////////////////////////////////
//

typedef struct __leica_t {
	int WhiteBalance;			// 0x0304 USHRT: ホワイトバランス
									// 0=オート
									// 2=蛍光灯
									// 3=白熱灯
	int LensType;				// 0x0310 ULONG: レンズ検出
									// 0x00 OFF
									// 0x71 28mm F2.8
									// 0x83 40mm
									// 0x87 50mm F2.0
	int WBColorTemp;			// 0x0321 ULONG: ホワイトバランス色温度
	int ImageID;				// 0x0340 ULONG: イメージ固有ID
} leica_t;


#endif		// __MAKERNOTE_LEICA_H__
