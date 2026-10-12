////////////////////////////////////////////////////////////////////////
// SONY

#ifndef __DATATABLE_SONY__
#define __DATATABLE_SONY__

// 露出制御モード
const lkup ekSonyCyberExpMode[] = {
	0,			euAuto,
	1,			euProgramAuto,
	5,			euLandscape,
	6,			euMacro,
	7,			euAperture,
	8,			euShutter,
/* -- 以降シーンセレクション ? -- */
	9,			_T("夜景・夕景"),
	10,			_T("ハイスピードシャッター"),
	11,			euNight,
	12,			euNightAndPeople,
	13,			euLandscape,
	14,			euBeach,
	15,			euSnow,
	16,			euFireworks,
	17,			euFastShutter,
	18,			_T("ソフトスナップ"),
	19,			_T("拡大鏡"),
	20,			euCandle,
	29,			_T("水中"),
	33,			_T("料理"),
	34,			_T("パノラマ"),
	35,			_T("手持ち夜景"),
	36,			_T("人物ブレ軽減"),
	37,			_T("ペット"),
	38,			_T("逆光補正HDR"),
	39,			_T("美肌"),
	40,			_T("背景ぼかし"),
	INT_MAX,	NULL,
};

// シーンモード
const lkup ekSonySceneMode[] = {
	0,			_T("Manual (P,A,S or M)"),
	1,			euPortrait,
	4,			euSunset,
	5,			euSports,
	6,			euLandscape,
	8,			euMacro,
	16,			euAuto,
	17,			euNightPortrait,
	INT_MAX,	NULL,
};

// リリースモード
const lkup ekSonyReleaseMode[] = {
	0,			euNormal,
	2,			_T("バースト"),
	5,			_T("露出ブラケット"),
	6,			_T("WBブラケット"),
	INT_MAX,	NULL,
};

const lkup ekSonyColorReproductionMode[] = {
	0,			_T("スタンダード or オフ"),
	1,			_T("リアル"),
	2,			euVivid,
	INT_MAX,	NULL,
};

// Dレンジオプティマイザ
const lkup ekSonyDRangeOptmizer[] = {
	0,			euOff,
	1,			_T("スタンダード"),
	2,			_T("アドバンス オート"),
	3,			_T("オート"),
	8,			_T("アドバンス Lv1"),
	9,			_T("アドバンス Lv2"),
	10,			_T("アドバンス Lv3"),
	11,			_T("アドバンス Lv4"),
	12,			_T("アドバンス Lv5"),
	16,			_T("Lv1"),
	17,			_T("Lv2"),
	18,			_T("Lv3"),
	19,			_T("Lv4"),
	20,			_T("Lv5"),
	INT_MAX,	NULL,
};

// HDR
const lkup ekSonyHDR[] = {
	0,			euOff,
	0x10001,	euAuto,
	0x10010,	_T("1.0 Ev"),
	0x10011,	_T("1.5 Ev"),
	0x10012,	_T("2.0 Ev"),
	0x10013,	_T("2.5 Ev"),
	0x10014,	_T("3.0 Ev"),
	0x10015,	_T("3.5 Ev"),
	0x10016,	_T("4.0 Ev"),
	0x10017,	_T("4.5 Ev"),
	0x10018,	_T("5.0 Ev"),
	0x10019,	_T("5.5 Ev"),
	0x1001a,	_T("6.0 Ev"),
	INT_MAX,	NULL,
};

// 露出制御プログラム
const lkup ekSonyExposureMode[] = {
	0,		euProgramAE,
	1,		euApertureAE,
	2,		euShutterAE,
	3,		euManual,
	4,		euFullAuto,
	5,		_T("プログラムAE(PAシフト)"),
	6,		_T("プログラムAE(PSシフト)"),
	0x1013,	euPortrait,
	0x1023,	euSports,
	0x1053,	euLandscape,
	0x1033,	euSunset,
	0x1043,	euNight,
	INT_MAX,NULL,
};

// ホワイトバランス
const lkup ekSonyWhiteBalance[] = {
	0,		euAuto,
	1,		euSunlight,
	2,		euCloudySky,
	3,		euShade,					//
	4,		euWhiteBulblight,				//
	5,		euFlourescent,				//
	6,		euFlash,					//
	0x10,	euDaylight,
	0x20,	euCloudySky,
	0x30,	euShade,
	0x40,	euWhiteBulblight,
	0x50,	euFlash,
	0x60,	euFlourescent,
	0x70,	euCustom,
	0x100,	euColorTempDirect,			//
	0x200,	euCustom1,
	0x300,	euCustom2,
	0x400,	euCustom3,
	INT_MAX,NULL,
};

// フォーカスモード
const lkup ekSonyFocusMode[] = {
	0,		euManual,
	1,		euAFSingle,
	2,		euAFCont,
	4,		euDMF,
	INT_MAX,NULL,
};

// フォーカスモード(ALPHA-100)
const lkup ekSonyFocusModeA100[] = {
	0,		euAFSingle,
	1,		euAFCont,
	4,		euAFAuto,
	5,		euManual,
	6,		euDMF,
	INT_MAX,NULL,
};

// オートフォーカスエリア
const lkup ekSonyAutoFocusArea[] = {
	1,		_T("マルチ"),
	2,		_T("中央重点"),
	3,		_T("スポット"),
	4,		_T("フレキシブルスポット"),
	6,		_T("タッチ"),
	14,		euManual,
	15,		_T("顔検出"),
	INT_MAX,NULL,
};

// カラースペース
const lkup ekSonyColorSpace[] = {
	0, 		euSRGB,
	2, 		euBW,
	5,		euAdobeRGB,
	INT_MAX,NULL,
};

// クリエイティブスタイル（画像仕上げ）
const lkup ekSonyCreativeStyle[] = {
	0,		euStandard,
	1,		euVivid,
	2,		euPortrait,
	3,		euLandscape,
	4,		euSunset,
	5,		euNightPortrait,
	6,		euMono,
	7,		euAdobeRGB,
	12,		euNeutral,
	13,		euClear,
	14,		euDeep,
	15,		euLight,
	16,		euAutumn,
	17,		euSepia,
	100,	euNeutral,
	101,	euClear,
	102,	euDeep,
	103,	euLight,
	104,	euNight,
	105,	euAutumn,
	INT_MAX,NULL,
};

// 画質モード
const lkup ekSonyQualityMode[] = {
	0,		euRAW,
	1,		euSuperFine,
	2,		euFine,
	3,		euHyojun,
	4,		euEconomy,
	5,		euExtraFine,
	6,		_T("RAW + JPEG"),
	7,		_T("cRAW"),
	8,		_T("cRAW + JPEG"),
	INT_MAX,NULL,
};

const lkup ekSonyTeleConverter[] = {
	0,		euNone,
	72,		_T("Minolta AF 2x APO (D)"),
	80,		_T("Minolta AF 2x APO II"),
	136,	_T("Minolta AF 1.4x APO (D)"),
	144,	_T("Minolta AF 1.4x APO II"),
	INT_MAX,NULL,
};

// 測光モード
const lkup ekSonyMeteringMode[] = {
	0,		_T("多分割測光"),
	1,		_T("中央重点平均測光"),
	2,		euMMSpot,
	INT_MAX,NULL,
};

// ゾーン切替
const lkup ekSonyZoneMatching[] = {
	0,			euOff,
	1,			_T("Hi200"),
	2,			_T("Lo80"),
	INT_MAX,	NULL,
};

// ISOセッティング
const lkup ekSonyISOSetting[] = {
	0,			euAuto,
	0x30,		_T("100"),
	0x38,		_T("200"),
	0x40,		_T("400"),
	0x48,		_T("800"),
	0x50,		_T("1600"),
	0x58,		_T("3200"),
	0xAE,		_T("Lo80"),
	0xB8,		_T("Hi200"),
	INT_MAX,	NULL,
};

// 高感度ノイズリダクション
const lkup ekSonyHighISONR[] = {
	0,			euOff,
	1,			_T("弱"),
	2,			euNormal,
	3,			_T("強"),
	256,		euAuto,
	65535,		euNA,
	INT_MAX,	NULL,
};

// レンズ情報
const lkup ekSonyLensID[] = {
	0,			_T("28-85mm F3.5-4.5"),
	1,			_T("80-200mm F2.8G HS-APO"),
	2,			_T("28-70mm F2.8G"),
	3,			_T("28-80mm F4-5.6"),
	5,			_T("35-70mm F3.5-4.5"),
	6,			_T("24-85mm F3.5-4.5 New"),
	7,			_T("100-300mm F4.5-5.6"),
	8,			_T("70-210mm F4.5-5.6"),
	9,			_T("50mm F3.5 Macro"),
	10,			_T("28-105mm F3.5-4.5 New"),
	11,			_T("300mm F4G HS-APO"),
	12,			_T("100mm F2.8 Soft Focus"),
	13,			_T("75-300mm F4.5-5.6"),
	14,			_T("100-400mm F4.5-6.7"),
	15,			_T("400mm F4.5G HS-APO"),
	16,			_T("17-35mm F3.5G"),
	17,			_T("20-35mm F3.5-4.5"),
	18,			_T("28-80mm F3.5-5.6 II"),
	19,			_T("35mm F1.4G"),
	20,			_T("135mm F2.8 STF"),
	22,			_T("35-80mm F4-5.6"),
	23,			_T("200mm F4G APO Macro"),
	24,			_T("24-105mm F3.5-4.5"),
	25,			_T("100-300mm F4.5-5.6(D)"),
	27,			_T("85mm F1.4G"),
	28,			_T("100mm F2.8 Macro"),
	29,			_T("75-300mm F4.5-5.6"),
	30,			_T("28-80mm F3.5-5.6(D)"),		// or Sigma AF 12-24mm F4.5-5.6 EX DG
	31,			_T("50mm F2.8 Macro"),				// or AF 50mm F3.5 Macro',
	32,			_T("300mm F2.8G"),
	33,			_T("70-200mm F2.8G SSM"),
	35,			_T("85mm F1.4 G (D) Limited"),
	36,			_T("28-100mm F3.5-5.6 (D)"),
	38,			_T("17-35mm F2.8-4"),
	39,			_T("28-75mm F2.8"),
	40,			_T("DT18-70mm F3.5-5.6"),
	41,			_T("DT11-18mm F4.5-5.6"),
	42,			_T("DT18-200mm F3.5-F6.3"),
	43,			_T("35mm F1.4G"),
	44,			_T("50mm F1.4"),
	45,			_T("Planar T* 85mm F1.4 ZA"),
	46,			_T("Sonnar T* DT16-80mm F3.5-4.5 ZA"),
	47,			_T("Sonnar T* 135mm F1.8 ZA"),
	48,			_T("Sonnar T* 24-70mm F2.8 ZA SSM"),
	49,			_T("DT55-200mm F4-5.6"),
	50,			_T("DT16-105mm F3.5-5.6"),
	51,			_T("DT16-105mm F3.5-5.6"),
	52,			_T("70-300mm F4.5-5.6 G SSM"),
	53,			_T("70-400mm F4-5.6 G SSM"),
	54,			_T("Sonnar T* 16-35mm F2.8 ZA SSM"),
	55,			_T("DT18-55mm F3.5-5.6 SAM (SAL-1855)"),
	56,			_T("DT55-200mm F4-5.6 SAM"),
	57,			_T("DT50mm F1.8 SAM"),
	58,			_T("DT30mm F2.8 SAM Macro"),
	59,			_T("28-75/2.8 SAM"),
	60,			_T("Distagon T* 24mm F2 SSM"),
	61,			_T("85mm F2.8 SAM (SAL-85F28)"),
	62,			_T("DT35mm F1.8 SAM (SAL-35F18)"),
	63,			_T("DT16-50mm F2.8 SSM"),
	128,		_T("Tamron18-200,28-200,28-300mm F3.5-6.3"),
	129,		_T("Tamron200-400mm F5.6 LD"),
	135,		_T("Vivitar 28-210mm F3.5-5.6"),
	138,		_T("Soligor 19-35mm F3.5-4.5"),
	25501,		_T("50mm F1.7"),
	25511,		_T("35-70mm F4"),					// or Sigma UC AF 28-70mm F3.5-4.5 or Sigma M-AF 70-200mm F2.8 EX Aspherical', #/12/12
	25521,		_T("28-85mm F3.5-4.5 New"),			// or Tokina 19-35mm F3.5-4.5 or Tokina 28-70mm F2.8 AT-X', #3/7
	25531,		_T("28-135mm F4-4.5"),
	25541,		_T("35-105mm F3.5-4.5"),
	25551,		_T("70-210mm F4 Macro"),			// or Sigma 70-210mm F4-5.6 APO or Sigma M-AF 70-200mm F2.8 EX APO', #7/6
	25561,		_T("135mm F2.8"),
	25571,		_T("28mm F2.8"),
	25581,		_T("24-50mm F4"),
	25601,		_T("100-200mm F4.5"),
	25611,		_T("75-300mm F4.5-5.6"),			// or Sigma 70-300mm F4-5.6 or Sigma 300mm F4 APO Macro', #12/3/7
	25621,		_T("50mm F1.4 New"),
	25631,		_T("300mm F2.8G"),
	25641,		_T("50mm F2.8 Macro"),
	25651,		_T("600mm F4"),
	25661,		_T("24mm F2.8"),
	25721,		_T("500mm F8 Reflex"),
	25781,		_T("16mm F2.8 Fisheye"),			// or Sigma 8mm F4 Fisheye or Sigma 14mm F3.5',
	25791,		_T("20mm F2.8"),
	25811,		_T("100mm F2.8 Macro New"),			// or Tamron 90mm F2.8 Macro or Sigma 180mm F5.6 Macro',
	25858,		_T("35-105mm F3.5-4.5 New"),		// or Tamron 24-135mm F3.5-5.6',
	25881,		_T("70-210mm F3.5-4.5"),
	25891,		_T("80-200 F2.8 APO"),				// or Tokina 80-200mm F2.8',
	25911,		_T("35mm F1.4"),
	25921,		_T("85mm F1.4G (D)"),
	25931,		_T("200mm F2.8G APO"),
	25941,		_T("3X-1X F1.7-2.8 Macro"),
	25961,		_T("28mm F2"),
	25971,		_T("35mm F2"),
	25981,		_T("100mm F2"),
	26041,		_T("80-200mm F4.5-5.6"),
	26051,		_T("35-80mm F4-5.6"),
	26061,		_T("100-300mm F4.5-5.6(D)"),
	26071,		_T("35-80mm F4-5.6"),
	26081,		_T("300mm F2.8G"),
	26091,		_T("600mm F4g HS-APO"),
	26121,		_T("200mm F2.8G HS-APO"),
	26131,		_T("50mm F1.7 New"),
	26151,		_T("28-105mm F3.5-4.5 Power Zoom"),
	26161,		_T("35-200mm F4.5-5.6 Power Zoom"),
	26181,		_T("28-80mm F4-5.6 Power Zoom"),
	26191,		_T("80-200mm F4.5-5.6 Power Zoom"),
	26201,		_T("28-70mm F2.8G"),
	26211,		_T("100-300mm F4.5-5.6 Power Zoom"),
	26241,		_T("35-80mm F4-5.6 Power Zoom"),
	26281,		_T("80-200mm F2.8G"),
	26291,		_T("85mm F1.4 New"),
	26311,		_T("100-300mm F4.5-5.6 APO"),
	26321,		_T("24-50mm F4 New"),
	26381,		_T("50mm F2.8 Macro New"),
	26391,		_T("100mm F2.8 Macro"),
	26411,		_T("20mm F2.8 New"),
	26421,		_T("24mm F2.8 New"),
	26441,		_T("100-400mm F4.5-6.7 APO"),
	26621,		_T("50mm F1.4 New"),
	26671,		_T("35mm F2 New"),
	26681,		_T("28mm F2 New"),
	26721,		_T("24-105mm F3.5-4.5(D)"),
	45741,		_T("200mm F2.8G x2"),				// or Tokina 300mm F2.8 x2
	65535,		euUnknown,
	INT_MAX,	NULL,
};

const lkup ekSonyLensID2[] = {
	0,			_T("Unknown E-Mount lens or other lens"),
	1,			_T("LA-EA1 Adapter"),
	2,			_T("LA-EA2 Adapter"),
	3,			_T("LA-EA3 Adapter"), // (NC) ILCE-7 image with A-mount lens, but also has 0x940e 2nd byte=2
	6,			_T("LA-EA4 Adapter"), // (NC) ILCE-7R image with A-mount lens and having phase-detect info blocks in 0x940e AFInfo
	44,			_T("Metabones Canon EF Smart Adapter"), // (NC) ILCE-7R with Zeiss Otus 55mm F1.4 ZE via Metabones Smart Adapter
	32784,		_T("E 16mm F2.8"),
	32785,		_T("E 18-55mm F3.5-5.6 OSS"),
	32786,		_T("E 55-210mm F4.5-6.3 OSS"),
	32787,		_T("E 18-200mm F3.5-6.3 OSS"),
	32788,		_T("E 30mm F3.5 Macro"),
	32789,		_T("E 24mm F1.8 ZA"),
	32790,		_T("E 50mm F1.8 OSS"),
	32791,		_T("E 16-70mm F4 ZA OSS"),
	32792,		_T("E 10-18mm F4 OSS"),
	32793,		_T("E PZ 16-50mm F3.5-5.6 OSS"),
	32794,		_T("Sonnar T* FE 35mm F2.8 ZA"),
	32795,		_T("Vario-Tessar T* FE 24-70mm F4 ZA OSS"),
	32797,		_T("E 18-200mm F3.5-6.3 OSS LE"),
	32798,		_T("E 20mm F2.8"),
	32799,		_T("E 35mm F1.8 OSS"),
	32807,		_T("E PZ 18-200mm F3.5-6.3 OSS"),
	32808,		_T("Sonnar T* FE 55mm F1.8 ZA"),
	32813,		_T("FE 28-70mm F3.5-5.6 OSS"),
};

#endif __DATATABLE_SONY__
