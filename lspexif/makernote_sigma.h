///////////////////////////////////////////////////////////////////////
// makernote_sigma.h sigma情報

#pragma once
#ifndef __MAKERNOTE_SIGMA_H__
#define __MAKERNOTE_SIGMA_H__

///////////////////////////////////////////////////////////////////////
// SIGMAモデル

enum {
	SIGMA_UNKNOWN = 0,
	SIGMA_DP = 0x100,
	SIGMA_DP1,
	SIGMA_SD = 0x200,
	SIGMA_SD9,
	SIGMA_SD10,
	SIGMA_SD14,
};

///////////////////////////////////////////////////////////////////////
//

typedef struct __sigma_t {
	int X3FVersion;				// Header offset:4 DWORD X3Fファイルフォーマットバージョン
	char Serial[16];			// 0x0002 ASCII シリアル番号
	char DriveMode[16];			// 0x0003 ASCII ドライブモード
	char Resolution[16];		// 0x0004 ASCII 解像度
	char AFMode[16];			// 0x0005 ASCII AFモード
	char FocusMode[16];			// 0x0006 ASCII フォーカスモード
	char WhiteBalanceMode[16];	// 0x0007 ASCII ホワイトバランスモード
	char ShootMode[8];			// 0x0008 ASCII 撮影モード
	char MeteringMode[8];		// 0x0009 ASCII 測光モード
	char FocalLengthRange[16];	// 0x000A ASCII レンズ焦点範囲
	char ColorSpace[16];		// 0x000B ASCII カラースペース
	char ExpCompensation[16];	// 0x000C ASCII 露出補正
	char Contrast[16];			// 0x000D ASCII コントラスト
	char Shadow[16];			// 0x000E ASCII シャドウ
	char Hilight[16];			// 0x000F ASCII ハイライト
	char Saturation[16];		// 0x0010 ASCII 彩度
	char Sharpness[16];			// 0x0011 ASCII シャープネス
	char X3FillLight[16];		// 0x0012 ASCII X3 Fill Light
	char ColorAdjust[16];		// 0x0014 ASCII カラー調整
	char AdjustMode[24];		// 0x0015 ASCII 調整モード
	char Quality[16];			// 0x0016 ASCII 画質
	char Firmware[32];			// 0x0017 ASCII ファームウェア
	char Software[32];			// 0x0018 ASCII ソフトウェア
	char AutoBracket[16];		// 0x0019 ASCII オートブラケット
	char MakerNoteVersion[8];	// 0x001D ASCII メーカーノートバージョン
	char AdjustmentMode[16];	// 0x0022 ASCII AdjustmentMode
	char LensApertureRange[16];	// 0x0030 ASCII LensApertureRange
	float FNumber;				// 0x0031 RATNL F値
	float ExposureTime;			// 0x0032 RATNL 露出時間
	float ExposureCompensation;	// 0x0035 SRTNL ExposureCompensation
	float FlashExposureComp;	// 0x003A SRTNL FlashExposureComp
	char Firmware2[16];			// 0x003B ASCII ファームウェアその２
	char WhiteBalance[16];		// 0x003C ASCII ホワイトバランス

	// x3f (dp1)
	char Flash[16];				// "FLASH"
	char FocalLength[8];		// "FLENGTH"
	char FocalLength35[8];		// "FLEQ35MM"
	char Burst[8];				// "BURST" ... when drivemode is multishot mode, then this value is present.
								// Indicates the shot number within the burst; the first shot is "1".
	char Rotation[8];			// "ROTATION" Image rotation in degrees clockwise
	char LensModel[8];			// "LENSMODEL" Lens identifier byte or "NONE" if no lens present.
	char AFArea[16];			// "AFAREA" AFエリア
	char AFInFocus[16];			// "AFINFOCUS" AFinFOCUS
} sigma_t;


#endif		// __MAKERNOTE_SIGMA_H__
