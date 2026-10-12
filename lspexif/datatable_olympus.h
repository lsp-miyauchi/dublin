////////////////////////////////////////////////////////////////////////
// OLYNPUS

#ifndef __DATATABLE_OLYMPUS__
#define __DATATABLE_OLYMPUS__

// 画質モード
const lkup ekOlymQualityType1[] = {
	0,		_T("SQ"),
	1,		_T("HQ"),
	2,		_T("SHQ"),
	6,		euRAW,
	INT_MAX,NULL,
};

const lkup ekOlymQualityType2[] = {
	1,		_T("SQ"),
	2,		_T("HQ"),
	3,		_T("SHQ"),
	4,		euRAW,
	5,		_T("SQ1"),
	6,		_T("SQ2"),
	INT_MAX,NULL,
};

// マクロモード
const lkup ekOlymMacroMode[] = {
	0,		euOff,
	1,		euOn,
	2,		euSuperMacro,
	INT_MAX,NULL,
};

// フラッシュモード
const lkup ekOlymFlashMode[] = {
	1,		euAuto,
	2,		euRedEye,
	3,		euForce,
	4,		euOff,
	INT_MAX,NULL,
};

// フラッシュモード２（bitmask定義）
const lkup ekOlymFlashModeEx[] = {
	0x0001,		_T("フィルイン"),
	0x0002,		_T("赤目軽減"),
	0x0004,		_T("スローシンクロ"),
	0x0008,		_T("強制発光"),
	0x0010,		_T("後幕"),
	INT_MAX,	NULL,
};

// シャープネス
const lkup ekOlymSharpness[] = {
	0,		euHyojun,
	1,		euHard,
	2,		euSoft,
	INT_MAX,NULL,
};

// ホワイトバランスモード
const lkup ekOlymWhiteBalanceMode[] = {
	0x0100,	euAuto,
	0x0102,	euAuto,
	0x0104,	euAuto,
	0x0202,	_T("3000K"),
	0x0203,	_T("3700K"),
	0x0204,	_T("4000K"),
	0x0205,	_T("4500K"),
	0x0206,	_T("5500K"),
	0x0207,	_T("6500K"),
	0x0208,	_T("7500K"),
	0x0300,	_T("ワンタッチ"),
	INT_MAX,NULL,
};

// ホワイトバランス
const lkup ekOlymWhiteBalance[] = {
	0,		euAuto,
	16,		_T("7500K（晴天の日陰）"),
	17,		_T("6000K（曇天）"),
	18,		_T("5300K（晴天）"),
	20,		_T("3000K（電球）"),
	21,		_T("3600K（タングステン光）"),
	33,		_T("6600K（昼光色蛍光灯）"),
	34,		_T("4500K（昼白色蛍光灯）"),
	35,		_T("4000K（白色蛍光灯）"),
	48,		_T("3600K（タングステン光）"),
//	??,		_T("5500K（フラッシュ撮影）"),
	256,	euCustom1,
	257,	euCustom2,
	258,	euCustom3,
	259,	euCustom4,
	512,	_T("カスタム 5400K"),
	513,	_T("カスタム 2900K"),
	514,	_T("カスタム 8000K"),
	INT_MAX,NULL,
};

const lkup ekOlymOneTouchWB[] = {
	0,		euOff,
	1,		euOn,
	2,		_T("オン（プリセット）"),
	INT_MAX,NULL,
};

// コントラスト
const lkup ekOlymContrast[] = {
	0,		euHigh,
	1,		euHyojun,
	2,		euLow,
	INT_MAX,NULL,
};

// フラッシュシステム
const lkup ekOlymFlashSystem[] = {
	0,			euNone,
	2,			_T("シンプルE-System"),
	3,			_T("E-System"),
	INT_MAX,	NULL,
};

// レンズエクステンダー情報
const lkup ekOlymExtenderInfo[] = {
	0x00000400,	_T("ZD EC-14 1.4x Teleconverter"),
	0x00000800,	_T("EX-25 ExtensionTube"),
	INT_MAX,	NULL,
};

// フラッシュモデル情報
const lkup ekOlymFlashModelInfo[] = {
	0,			euNone,
	1,			_T("FL-20"),
	2,			_T("FL-50"),
	3,			_T("RF-11"),
	4,			_T("TF-22"),
	5,			_T("FL-36"),
	INT_MAX,	NULL,
};

// カラースペース情報
const lkup ekOlymColorSpace[] = {
	0,			euSRGB,
	1,			euAdobeRGB,
	2,			_T("ProPhoto RGB"),
	INT_MAX,	NULL,
};

// シーンモード
const lkup ekOlymSceneMode[] = {
	0,			euHyojun,
	6,			euAuto,
	7,			euSports,
	8,			euPortrait,
	9,			_T("記念撮影"),
	10,			euLandscape,
	11,			euNight,
	12,			_T("自分撮り"),
	13,			_T("パノラマ"),
	15,			_T("ムービー"),
	16,			_T("風景＋ポートレート"),
	17,			euNightAndPeople,
	18,			_T("屋内撮影"),
	19,			euFireworks,
	20,			_T("夕日"),
	22,			euMacro,
	23,			_T("スーパーマクロ"),
	24,			euFoods,
	25,			_T("文書"),
	26,			_T("マナーショット"),
	27,			_T("ショット＆セレクト"),
	28,			_T("ビーチ＆スノー"),
	29,			_T("自分撮り＋セルフタイマー"),
	30,			euCandle,
	31,		_T("Available light"),
	32,			_T("ガラス越し"),
	33,		_T("My Mode"),
	34,			euPet,
	35,			_T("水中ワイド1"),
	36,			_T("水中マクロ"),
	37,			_T("ショット&セレクト1"),
	38,			_T("ショット&セレクト2"),
	39,			euHighKey,
	40,			_T("ぶれ軽減"),
	41,			_T("オークション"),
	42,			_T("ビーチ"),
	43,			_T("スノー"),
	44,			_T("水中ワイド2"),
	45,			euLowKey,
	46,		_T("Children"),
	47,		_T("Vivid"),
	48,		_T("Nature Macro"),
	49,			_T("水中スナップ"),
	50,		_T("撮影ガイド"),
//	xx,			_T("寝顔"),
	INT_MAX,	NULL,
};

// 撮影モード
const lkup ekOlymExposureMode[] = {
	1,			euManual,
	2,			euProgramAE,
	3,			euApertureAE,
	4,			euShutterAE,
	5,			_T("プログラムシフト"),
	INT_MAX,	NULL,
};

// 測光方式
const lkup ekOlymMeteringMode[] = {
	0x002,		euMMCenterWait,
	0x003,		euMMSpot,
	0x005,		_T("デジタルESP測光"),
	0x105,		_T("49分割デジタルESP測光"),					// Pattern + AF ?
	0x203,		_T("スポット測光ハイライトコントロール"),
	0x403,		_T("スポット測光シャドウコントロール"),
	INT_MAX,	NULL,
};

// フォーカスモード
const lkup ekOlymFocusMode[] = {
	0,			_T("シングルAF"),
	1,			_T("連射AF"),
	2,			_T("コンティニュアスAF"),
	3,			_T("マルチAF"),
	10,			euManualFocus,
	INT_MAX,	NULL,
};

// AFエリア
const lkup ekOlymAFArea[] = {
	0,			euAuto,
	// E-1
	0x36794285,	euPosLeft,
	0x79798585,	euPosCenter,
	0xBD79C985,	euPosRight,
	// E-3
	0x49376962,	_T("左上"),
	0x6F379062,	_T("中央上"),
	0x9637B662,	_T("右上"),
	0x236A4395,	_T("中央左"),
	0x496A6995,	_T("中央やや左"),
	0x6F6A9095,	_T("中央"),
	0x966AB695,	_T("中央やや右"),
	0xBC6ADC95,	_T("中央右"),
	0x499D69C8,	_T("左下"),
	0x6F9D90C8,	_T("中央下"),
	0x969DB6C8,	_T("右下"),
	INT_MAX,	NULL,
};

// ノイズリダクション
const lkup ekOlymNoiseReduction[] = {
	0,			euOff,
	1,			euOn,
	2,			_T("ノイズフィルタ"),
	3,			_T("ノイズリダクション＋ノイズフィルタ"),
	4,			_T("ノイズフィルタ(ISO Boost)"),
	5,			_T("ノイズリダクション＋ノイズフィルタ(ISO Boost)"),
	INT_MAX,	NULL,
};

// ノイズフィルタ
const lkup ekOlymNoiseFilter[] = {
	-2,			euOff,
	-1,			_T("弱"),
	0,			euHyojun,
	1,			_T("強"),
	INT_MAX,	NULL,
};

// 階調
const lkup ekOlymGradation[] = {
	-1,			euLowKey,
	0,			euHyojun,
	1,			euHighKey,
	INT_MAX,	NULL,
};

// 仕上がりモード
const lkup ekOlymPictureMode[] = {
	0x0001,		_T("VIVID"),
	0x0002,		_T("NATURAL"),
	0x0003,		_T("FLAT"),
	0x0004,		euPortrait,
	0x0100,		_T("モノトーン"),
	0x0200,		euSepia,
	INT_MAX,	NULL,
};

// フィルター効果（仕上がりモード：mono/sepia）
const lkup ekOlymPicModeFilterEffect[] = {
	0,			euUnknown,
	1,			euNone,
	2,			euClrYellow,
	3,			_T("オレンジ"),
	4,			euClrRed,
	5,			euClrGreen,
	INT_MAX,	NULL,
};

// 調色（仕上がりモード：mono/sepia）
const lkup ekOlymPicModeTone[] = {
	0,			euUnknown,
	1,			euNone,
	2,			euSepia,
	3,			euClrBlue,
	4,			euClrMagenta,
	5,			euClrGreen,
	INT_MAX,	NULL,
};

// 彩度強調
const lkup ekOlymModifiedSaturation[] = {
	0,		euOff,
	1,		_T("CM1（赤を強調）"),
	2,		_T("CM2（緑を強調）"),
	3,		_T("CM3（青を強調）"),
	4,		_T("CM4（肌色を強調）"),
	INT_MAX,NULL
};

// 撮影シーケンス
const lkup ekOlymShotSequence[] = {
	1,			_T("連写"),
	2,			_T("AEブラケット"),
	3,			_T("WBブラケット"),
	4,			_T("AE+WBブラケット"),
	INT_MAX,	NULL,
};

// パノラマモード
const lkup ekOlymPanoramaMode[] = {
	0,			euOff,
	1,			_T("左から右へ"),
	2,			_T("右から左へ"),
	3,			_T("下から上へ"),
	4,			_T("上から下へ"),
	INT_MAX,	NULL,
};

// レンズ情報
const lkup ekOlymLensInfo[] = {
	272,		_T("M.ZD 14-42mm F3.5-5.6"),
	528,		_T("M.ZD 17mm F2.8"),
	784,		_T("M.ZD ED 14-150mm F4.0-5.6"),
	1040,		_T("M.ZD ED 9-18mm F4.0-5.6"),
	1808,		_T("M.ZD ED 12mm F2.0"),
	2320,		_T("M.ZD 14-42mm F3.5-5.6Ⅱ"),
	4112,		_T("M.ZD ED 12-50mm F3.5-6.3 EZ"),
	4368,		_T("M.ZD 45mm F1.8"),
	4624,		_T("M.ZD ED 60mm F2.8 Macro"),
	4880,		_T("M.ZD 14-42mm F3.5-5.6ⅡR"),
	5136,		_T("M.ZD ED 40-150mm F4.0-5.6 R"),
	5392,		_T("M.ZD 75mm F1.8"),
	5648,		_T("M.ZD 17mm F1.8"),
	6416,		_T("M.ZD ED 12-40mm F2.8 PRO"),
	0x00000100,	_T("ZD ED 50mm F2.0 Macro"),
	0x00000101,	_T("ZD 40-150mm F3.5-4.5"),
	0x00000200,	_T("ZD ED 150mm F2.0"),
	0x00000300,	_T("ZD ED 300mm F2.8"),
	0x00000500,	_T("ZD 14-54mm F2.8-3.5"),
	0x00000501,	_T("ZD ED 90-250mm F2.8"),
	0x00000600,	_T("ZD ED 50-200mm F2.8-3.5"),
	0x00000601,	_T("ZD ED 8mm F3.5 Fisheye"),
	0x00000700,	_T("ZD 11-22mm F2.8-3.5"),
	0x00000701,	_T("ZD ED 18-180mm F3.5-6.3"),
	0x00000801, _T("ZD ED 70-300mm F4-5.6"),
	0x00001500,	_T("ZD ED 7-14mm F4.0"),
	0x00001700,	_T("ZD ED 35-100mm F2.0"),
	0x00001800,	_T("ZD 14-45mm F3.5-5.6"),
	0x00002000,	_T("ZD 35mm F3.5 Macro"),
	0x00002300,	_T("ZD ED 14-42mm F3.5-5.5"),
	0x00002400,	_T("ZD ED 40-150mm F4-5.6"),
	0x00003000,	_T("ZD ED 50-200mm F2.8-3.5 SWD"),
	0x00003100,	_T("ZD ED 12-60mm F2.8-4.0 SWD"),
	0x00003200,	_T("ZD ED 14-35mm F2 SWD"),
	0x00003300,	_T("ZD 25mm F2.8"),
	0x00003400,	_T("ZD ED 9-18mm F4-5.6"),
	0x01000100,	_T("SG 18-50mm F3.5-5.6 DC"),
	0x01000200,	_T("SG 55-200mm F4.0-5.6 DC"),
	0x01000300,	_T("SG 18-125mm F3.5-5.6 DC"),
	0x01000400,	_T("SG 18-125mm F3.5-5.6"),
	INT_MAX,	NULL,
};


#endif __DATATABLE_OLYMPUS__
