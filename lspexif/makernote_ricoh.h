///////////////////////////////////////////////////////////////////////
// makernote_ricoh.h ricoh情報

#pragma once
#ifndef __MAKERNOTE_RICOH_H__
#define __MAKERNOTE_RICOH_H__

typedef struct __ricoh_t {
	char DataFormat[8];					// 0x0001: ASCII データ形式
	char Firmware[16];					// 0x0002: ASCII ファームウェア

										// 0x1001: UNDEF ImageInfo block
	int ImageWidth;							// offset 0 ImageWidth size 2
	int ImageHeight;						// offset 2 ImageHeight size 2
//	BYTE DateTime[16];						// offset 6 DateTime size 7
	int FlashMode;							// offset 32 size 1
	int MacroMode;							// offset 33 size 1
	int Sharpness;							// offset 34 size 1
	int WhiteBalance;						// offset 38 size 1
	int ISOSetting;							// offset 39 size 1
	int Saturation;							// offset 40 size 1
	int ToneEffect;							// offset 48 size 1
} ricoh_t;


#endif		// __MAKERNOTE_RICOH_H__
