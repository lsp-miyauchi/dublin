///////////////////////////////////////////////////////////////////////
// makernote_epson.h epson情報

#pragma once
#ifndef __MAKERNOTE_EPSON_H__
#define __MAKERNOTE_EPSON_H__


typedef struct __epson_t {
	int Quality;				// 0x0201 USHRT 画質モード
								// 0x0203 USHRT 白黒モード
								// 0x0206 USHRT
	char CameraID[32];			// 0x0209 UNDEFx32
								// 0x020A UNDEFx64
	int ImageWidth;				// 0x020B ULONG イメージ幅
	int ImageHeight;			// 0x020C ULONG イメージ高さ
	char HardwareType[16];		// 0x020D ASCIIx6 ハードウェアタイプ
								// 0x0280 サムネイルデータ（JPEG形式でFFD8がEED8と記録されている）
								// 0x0480 UNDEFx12
								// 0x0D00 UNDEFx10
								// 0x0E00 PrintIM
								// 0x0E80 UNDEXFx256
} epson_t;


#endif		// __MAKERNOTE_EPSON_H__
