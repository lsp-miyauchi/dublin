///////////////////////////////////////////////////////////////////////
// makernote_sony.h sony情報

#pragma once
#ifndef __MAKERNOTE_SONY_H__
#define __MAKERNOTE_SONY_H__

///////////////////////////////////////////////////////////////////////
// SONYモデル

enum {
	SONY_UNKNOWN = 0,
	SONY_CYBERSHOT = 0x100,
	SONY_DSC = 0x200,
	SONY_DSC_P = 0x210,
	SONY_DSC_T = 0x220,
	SONY_DSC_T9,
	SONY_DSC_U = 0x230,
	SONY_DSC_V = 0x240,
	SONY_DSC_M = 0x250,
	SONY_DSC_L = 0x260,
	SONY_DSC_W = 0x270,
	SONY_DSC_S = 0x280,
	SONY_DSC_H = 0x290,
	SONY_DSC_N = 0x2A0,
	SONY_DSC_N1,
	SONY_DSC_R = 0x1000,
	SONY_DSC_R1,					// "DSC-KAIKAK" ? "DSC-R1"
	SONY_DSC_NEX = 0x1800,
	SONY_DSC_NEX3,
	SONY_DSC_NEXC3,
	SONY_DSC_NEX5,
	SONY_DSC_NEX5N,
	SONY_DSC_NEX7,
	SONY_DSLR_ALPHA = 0x2000,
	SONY_DSLR_A100,
	SONY_DSLR_A200,
	SONY_DSLR_A300,
	SONY_DSLR_A350,
	SONY_DSLR_A700,
	SONY_DSLR_A900,
	SONY_SLT_UNKNOWN = 0x2100,
	SONY_SLT_A55,					// Alpha55
	SONY_SLT_A57,					// Alpha57
	SONY_SLT_A58,					// Alpha58
	SONY_SLT_A65,					// Alpha65
	SONY_SLT_A77,					// Alpha77
	SONY_ILCA_UNKNOWN = 0x2500,
	SONY_ILCA_77M2,					// Alpha77II
	SONY_ILCE_UNKNOWN = 0x3000,
	SONY_ILCE_7R,					// Alpha7R
	SONY_ILCE_7S,					// Alpha7S
	SONY_ILCE_7,					// Alpha7
	SONY_ILCE_3000,					// Alpha3000
	SONY_ILCE_5000,					// Alpha3000
	SONY_ILCE_6000,					// Alpha3000
};

#define IS_SONY_ALPHA(n)			((n)>=SONY_DSLR_ALPHA)

///////////////////////////////////////////////////////////////////////
//

typedef struct __sony_t {
	BOOL NewType;
	int QualityMode;					// 0x0102 ULONG 画質モード
											// 0 RAW
											// 1 SuperFine
											// 2 Fine
											// 3 Standard
											// 4 Economy
											// 5 ExtraFine
											// 6 RAW + JPEG"
											// 7 cRAW (Compressed)
											// 8 cRAW + JPEG
	float FlashExposureCompensation;	// 0x0104 SRTNL FlashExposureCompensation
	int TeleConverter;					// 0x0105
											// 0 なし
											// 72   Minolta AF 2x APO (D)
											// 80   Minolta AF 2x APO II
											// 136  Minolta AF 1.4x APO (D)
											// 144  Minolta AF 1.4x APO II
	int WhiteBalanceFineTune;			// 0x0112 LONG ホワイトバランス補正
	int WhiteBalance;					// 0x0115 ULONG ホワイトバランス
											// 0x00 => 'Auto'
											// 0x01 => 'Color Temperature/Color Filter'
											// 0x10 => 'Daylight'
											// 0x20 => 'Cloudy'
											// 0x30 => 'Shade'
											// 0x40 => 'Tungsten'
											// 0x50 => 'Flash'
											// 0x60 => 'Fluorescent'
											// 0x70 => 'Custom'
	int ColorReproductionMode;			// 0x2000 UNDEF 色再現モード
											// 0=スタンダード, 1=リアルモード, 2=ビビッドモード
	int Contrast;						// 0x2004 LONG
	int Saturation;						// 0x2005 LONG
	int Sharpness;						// 0x2006 LONG
	int Brightness;						// 0x2007 LONG
	int LongExposureNR;					// 0x2008 ULONG 長秒時ノイズリダクション
										// 0xB04E USHRT 長秒時ノイズリダクション
											// 0 => 'Off'
											// 1 => 'On'
											// 0 => 'Off',
											// 1 => 'On',
											// 65535 => 'n/a',
	int HighISONR;						// 0x2009 USHRT 高感度ノイズリダクション
											// 0 => 'Off',
											// 1 => 'Low',
											// 2 => 'Normal',
											// 3 => 'High',
											// 256 => 'Auto',
											// 65535 => 'n/a',
	int HDR;							// 0x200a ULONG HDR
											// 0x0 => 'Off'
											// 0x10001 => 'Auto'
											// 0x10010 => '1.0 EV', # (NEX_5
											// 0x10011 => '1.5 EV'
											// 0x10012 => '2.0 EV'
											// 0x10013 => '2.5 EV'
											// 0x10014 => '3.0 EV'
											// 0x10015 => '3.5 EV'
											// 0x10016 => '4.0 EV'
											// 0x10017 => '4.5 EV'
											// 0x10018 => '5.0 EV'
											// 0x10019 => '5.5 EV'
											// 0x1001a => '6.0 EV', # (SLT-A55V)
    int MultiShotNR;					// 0x200b ULONG マルチショットノイズリダクション
											// 0 => 'Off'
											// 1 => 'On'
											// 255 => 'n/a'
	char ColorReproduction[16];			// 0xB020 ASCII
	int ColorTemp;						// 0xB021 ULONG 色温度
	int SceneMode;						// 0xB023 ULONG シーンモード
											// 0 => 'Manual (P,A,S or M)',
											// 1 => 'Portrait',
											// 4 => 'Sunset',
											// 5 => 'Sports',
											// 6 => 'Landscape',
											// 8 => 'Macro',
											// 16 => 'Auto',
											// 17 => 'Night Portrait',
	int ZoneMatching;					// 0xB024 ULONG
											// 0 => 'ISO Setting Used',
											// 1 => 'High Key',
											// 2 => 'Low Key',
	int DRangeOptmizer;					// 0xB025 ULONG Dレンジオプティマイザ
											// 0 = off
											// 1 = standard
											// 2 = advance
	int ImageStabilization;				// 0xB026 ULONG 手ブレ補正
	int LensType;						// 0...ekSonyLensID,  1...ekSonyLensID2
	int LensID;							// 0xB027 ULONG レンズID
	int CreativeStyle;					// 0xB029 ULONG クリエイティブスタイル
											// ミノルタのカラーモードの延長
	int MacroMode;						// 0xB040 USHRT マクロ
											// 0 = off
											// 1 = on
	int ExposureMode;					// 0xB041 USHRT
	int FocusMode;						// 0xB042 USHRT フォーカスモード
											// 1 => 'AF-S'
											// 2 => 'AF-C'
	int AutoFocusArea;					// 0xB043 USHRT オートフォーカスエリア
											// 0 => 'Default'
											// 1 => 'マルチ'
											// 2 => '中央重点'
											// 3 => 'スポット',
											// 4 => 'フレキシブル'
											// 6 => 'タッチ'
											// 14 => 'Manual'
											// 15 => '顔検出'
	int FlashComp;						// 0xB048 SSHRT
	int ReleaseMode;					// 0xB049 USHRT リリースモード
											// 0 => 'Normal'
											// 2 => 'Burst'
											// 5 => 'Exposure Bracketing'
											// 6 => 'White Balance Bracketing'
									// Minolta(MLT0)ブロック
	int mltPhotoFinish;					// 0x0101 ULONG 画像仕上げ
	int mltQualityMode;					// 0x0102 ULONG 画質モード
	int mltZoneMatching;				// 0x010A ULONG ゾーン切り替え
	int mltColorTemp;					// 0x010B ULONG 色温度

									// Minolta(SWEET)フォーマット 0x11Cブロック
	int mltExposureMode;					// offs 00 露出制御プログラム
	int mltExpCompensation;					// offs 01 露出補正 値÷100 - 3 (EV)
	int mltExposureTime;					// offs 08 露出時間
	int mltAperture;						// offs 09 絞り
	int mltWhiteBalance;					// offs 11 ホワイトバランス
	int mltFocusMode;						// offs 12 フォーカスモード
													// A100:  0=AF-S, 1=AF-C, 4=AF-A, 5=Manual, 6=DMF
	int mltMeteringMode;					// offs 18 測光方式
	int mltFilmSpeed;						// offs 19 撮像感度
	int mltColorSpace;						// offs 23 色空間
												// 0 sRGB
												// 2 白黒シロクロ
												// 5 AdobeRGB
	int mltSharpness;						// offs 24 シャープネス
	int mltContrast;						// offs 25 コントラスト補正
	int mltSaturation;						// offs 26 彩度
	int mltHue;								// offs 27 色合い？
	int mltWhiteBalanceTune;				// offs 56 ホワイトバランス補正
	int mltNoiseReduction;					// offs 63 ノイズリダクション？
	int mltMeteredManual;					// offs 80 メータードマニュアル？
} sony_t;



#endif		// __MAKERNOTE_SONY_H__
