////////////////////////////////////////////////////////////////////////
// CANON

#ifndef __DATATABLE_CANON__
#define __DATATABLE_CANON__

// クォリティ
const lkup ekCanonQuality[] = {
	0,		euUnknown,
	1,		euEconomy,
	2,		euHyojun,
	3,		euFine,
	4,		euRAW,
	5,		euSuperFine,
	INT_MAX,NULL,
};

// フラッシュ状態
const lkup ekCanonFlashCondition[] = {
	-1,		euDisable,
	0,		euOff,
	1,		_T("発光"),
	INT_MAX,NULL,
};

// フラッシュモード
const lkup ekCanonFlashMode[] = {
	0,		euOff,
	1,		euAuto,
	2,		euOn,
	3,		euRedEye,
	4,		_T("スローシンクロ"),
	5,		_T("自動（赤目緩和）"),
	6,		_T("オン（赤目緩和）"),
	16,		euFlashExternal,
	65535, 	euDisable,
	INT_MAX,NULL,
};

// OnOff
const lkup ekCanonOnOff[] = {
	0,		euNoFunction,
	1,		euOn,
	2,		euOff,
	INT_MAX,NULL,
};

// 手ブレ補正
const lkup ekCanonIS[] = {
	0,		euOff,
	1,		_T("常時オン"),
	2,		_T("撮影時オン"),
	3,		_T("流し撮りモード"),
	INT_MAX,NULL,
};


// ドライブモード
const lkup ekCanonDriveMode[] = {
	0,		euSingleShot,
	1,		euContinuosShot,
	2,		euSelfTimer,
	4,		_T("低速連続撮影"),
	5,		_T("高速連続撮影"),
	INT_MAX,NULL,
};

// フォーカスレンジ
const lkup ekCanonFocusRange[] = {
	0,		euManual,
	1,		euAuto,
	2,		euUnknown,
	3,		euMacro,
	4,		_T("Very Close"),
	5,		_T("Close"),
	6,		_T("Middle Range"),
	7,		_T("遠景"),
	8,		euPanFocus,
	9,		euSuperMacro,
	10,		_T("∞"),
	11,		euSuperMacro,
	INT_MAX,NULL,
};

// フォーカス動作
const lkup ekCanonFocusContinuous[] = {
	0,		_T("シングル"),
	1,		_T("連続"),
	INT_MAX,NULL,
};

// AE設定
const lkup ekCanonAESetting[] = {
	0,		_T("ノーマルAE"),
	1,		_T("露出補正"),
	2,		_T("AEロック"),
	3,		_T("AEロック+露出補正"),
	4,		_T("AEなし"),
	INT_MAX,NULL,
};

// フォーカスモード
const lkup ekCanonFocusMode[] = {
	0,		_T("ワンショット"),
	1,		_T("AIサーボ"),
	2,		_T("AIフォーカス"),
	3,		euManualFocus,
	4,		_T("シングルフォーカス"),
	5,		_T("連続フォーカス"),
	6,		euManualFocus,
	16,		euPanFocus,
	INT_MAX,NULL,
};

// イメージサイズ
const lkup ekCanonImageSize[] = {
	-1,		euUnknown,
	0,		_T("ラージ"),
	1,		_T("ミドル"),
	2,		_T("スモール"),
	5,		_T("ミドル1"),
	6,		_T("ミドル2"),
	8,		_T("L判プリント"),
	9,		euWide,
	INT_MAX,NULL,
};

// イメージセレクトモード
const lkup ekCanonShootMode[] = {
	0,		euFullAuto,
	1,		euManual,
	2,		euLandscape,
	3,		euFastShutter,
	4,		euSlowShutter,
	5,		euNight,
	6,		euMono,
	7,		euSepia,
	8,		euPortrait,
	9,		euSports,
	10,		_T("マクロ・クローズアップ"),
	11,		euMono,
	12,		euPanFocus,
	13,		euVivid,
	14,		euNeutral,
	15,		_T("フラッシュオフ"),
	16,		_T("ロングシャッター"),
	17,		euSuperMacro,
	18,		_T("新緑/紅葉"),
	19,		_T("パーティ/室内"),
	20,		euFireworks,
	21,		euBeach,
	22,		euUnderwater,
	23,		euSnow,
	24,		_T("キッズ＆ペット"),
	25,		_T("ナイトスナップ"),
	26,		_T("デジタルマクロ"),
	27,		_T("マイカラー"),
	29,		euSuperMacro,
	30,		_T("ワンポイントカラー"),
	31,		_T("スイッチカラー"),
	INT_MAX,NULL,
};

// スローシャッター
const lkup ekCanonSlowShutter[] = {
	0,		euOff,
	1,		euNight,
	2,		euOn,
	3,		euNone,
	INT_MAX,NULL,
};


// デジタルズーム
const lkup ekCanonDigitalZoom[] = {
	-1,		euNoFunction,
	0,		euOff,
	1,		_T("2倍"),
	2,		_T("4倍"),
	3,		euOn,
};

// 露出制御プログラム
const lkup ekCanonExpProgram[] = {
	0,		_T("イメージセレクト"),
	1,		euProgramAE,
	2,		euShutterAE,
	3,		euApertureAE,
	4,		euManual,
	5,		_T("自動深度優先(A-DEP)"),
	6,		_T("深度優先(DEP)"),
	INT_MAX,NULL,
};

// 測光方式
const lkup ekCanonMeteringMode[] = {
	0,		euMMCenterWait,
	1,		euMMSpot,
	2,		euMMAverage,
	3,		_T("評価測光"),
	4,		_T("中央部部分測光"),
	5,		_T("中央部重点平均測光"),
	INT_MAX,NULL,
};

// AFポイント
const lkup ekCanonAFPoint[] = {
	0x2005, _T("マニュアルAFポイント選択"),
	0x3000,	_T("無し(MF)"),	
	0x3001,	_T("オート選択"),
	0x3002,	euPosRight,
	0x3003,	euPosCenter,
	0x3004,	euPosLeft,
	0x4001,	_T("オートAFポイント選択"),
	0x4006,	_T("オートAF顔優先選択"),
	INT_MAX,NULL,
};

// 合焦AFフレーム(PowerShot/Ixy)
const lkup ekCanonAFPointUsedPS[] = {
	0x3000,	_T("無し(MF)"),
	0x3001,	euPosRight,
	0x3002,	euPosCenter,
	0x3003,	_T("中央+右"),
	0x3004,	euPosLeft,
	0x3005,	_T("左+右"),
	0x3006,	_T("左+中央"),
	0x3007,	_T("全て"),
	INT_MAX,NULL,
};

// 合焦AFフレーム(EOS)
const lkup ekCanonAFPointUsed[] = {
	0x0001,	euPosRight,					// bit 0
	0x0002,	euPosMidRight,				// bit 1
	0x0004,	euPosBottom,				// bit 2
	0x0008,	euPosCenter,				// bit 3
	0x0010,	euPosTop,					// bit 4
	0x0020,	euPosMidLeft,				// bit 5
	0x0040,	euPosLeft,					// bit 6
	INT_MAX,NULL,
};

// 合焦AFフレーム(EOS20D)
const lkup ekCanonAFPointUsed20D[] = {
	0x0001,	euPosTop,
	0x0002,	euPosTopLeft,
	0x0004,	euPosTopRight,
	0x0008,	euPosLeft,
	0x0010,	euPosCenter,
	0x0020,	euPosRight,
	0x0040,	euPosBottomLeft,
	0x0080,	euPosBottomRight,
	0x0100,	euPosBottom,
	INT_MAX,NULL,
};

// 合焦AFフレーム(EOS5D)
const lkup ekCanonAFPointUsed5D[] = {
	0x0001,	euPosCenter,
	0x0002,	euPosTop,
	0x0004,	euPosBottom,
	0x0008,	euPosTopLeft,
	0x0010,	euPosTopRight,
	0x0020,	euPosBottomLeft,
	0x0040,	euPosBottomRight,
	0x0080,	euPosLeft,
	0x0100,	euPosRight,
	INT_MAX,NULL,
};

// AIサーボ(EOS5D)
const lkup ekCanonAIServo5D[] = {
	0x0200,	_T("AIサーボ1"),
	0x0400,	_T("AIサーボ2"),
	0x0800,	_T("AIサーボ3"),
	0x1000,	_T("AIサーボ4"),
	0x2000,	_T("AIサーボ5"),
	0x4000,	_T("AIサーボ6"),
	INT_MAX,NULL,
};

// ホワイトバランス
const lkup ekCanonWhiteBalance[] = {
	0,		euAuto,
	1,		euSunlight,
	2,		euCloudy,
	3,		euWhiteBulblight,
	4,		euFlourescent,
	5,		euFlash,
	6,		euCustom,
	7,		euMono,
	8,		euShade,
	9,		euColorTempDirect,
	10,		_T("PC Set1"),
	11,		_T("PC Set2"),
	12,		_T("PC Set3"),
	14,		euFlstDaylight,
	15,		euCustom1,
	16,		euCustom2,
	17,		euUnderwater,
	INT_MAX,NULL,
};

// ブラケットモード
const lkup ekCanonBracketMode[] = {
	0,		euOff,
	1,		_T("AEB"),
	2,		_T("FEB"),
	3,		_T("ISO"),
	4,		_T("WB"),
	INT_MAX,NULL,
};

const lkup ekCanonWBBracketMode[] = {
	0,		euOff,
	1,		_T("オン（BAシフト）"),
	2,		_T("オン（MGシフト）"),
	INT_MAX,NULL,
};

// AEB
const lkup ekCanonAEBSeq[] = {
	-1,		euOn,
	0,		euOff,
	1,		_T("ショット#1"),
	2,		_T("ショット#2"),
	3,		_T("ショット#3"),
	INT_MAX,NULL,
};

// 現像パラメーター
const lkup ekCanonDevParam[] = {
	1,		euHyojun,
	2,		euAdobeRGB,
	3,		euSepia,
	INT_MAX,NULL,
};

// カラーマトリックス
const lkup ekCanonColorMatrix[] = {
	0,		euNone,
	1,		euHyojun,
	2,		euPortrait,
	3,		_T("高彩度"),
	4,		euAdobeRGB,
	5,		_T("低彩度"),
	6,		_T("CMセット1"),
	7,		_T("CMセット2"),
	// 以下EOS5D/1DMKIINのピクチャースタイル
	33,		euCustom1,
	34,		euCustom2,
	35,		euCustom3,
	65,		_T("ノスタルジア"),
	66,		euClear,
	67,		_T("トワイライト"),
	68,		_T("エメラルド"),
	69,		euAutumn,
	129,	euHyojun,
	130,	euPortrait,
	131,	euLandscape,
	132,	euNeutral,
	133,	_T("忠実設定"),
	134,	euMono,
	135,	euCustom1,				// 自信なし
	136,	euCustom2,				// 自信なし
	137,	euCustom3,				// 自信なし
	INT_MAX,NULL,
};

// マイカラー
const lkup ekCanonMyColor[] = {
	0,		euOff,
	0x01,	_T("ポジフィルムカラー"),
	0x02,	_T("色白肌"),
	0x03,	_T("褐色肌"),
	0x04,	_T("あざやかブルー"),
	0x05,	_T("あざやかグリーン"),
	0x06,	_T("あざやかレッド"),
	0x07,	_T("ワンポイントカラー"),
	0x08,	_T("スイッチカラー"),
	0x09,	_T("カスタムカラー"),
	0x0c,	_T("くっきりカラー"),
	0x0d,	_T("すっきりカラー"),
	0x0e,	euSepia,
	0x0f,	euBW,
	INT_MAX,NULL,
};

// 色効果（0x0001タグのオフセット40）
const lkup ekCanonColorEffect[] = {
	0,		euOff,
	1,		_T("くっきりカラー"),
	2,		_T("すっきりカラー"),
	3,		_T("ソフト"),
	3,		euSepia,
	5,		euBW,
	6,		_T("カスタムカラー"),
	INT_MAX,NULL,
};

const lkup ekCanonAutoRotate[] = {
	-1,		_T("ソフトウェア"),
	0,		euNone,
	1,		_T("左90°"),
	2,		_T("左180°"),
	3,		_T("左270°"),
	INT_MAX,NULL,
};

// BWフィルター効果
const lkup ekCanonBWFilterEffect[] = {
	-1,		euNone,
	0,		euNone,
	1,		euClrYellow,
	2,		_T("オレンジ"),
	3,		euClrRed,
	4,		euClrGreen,
	INT_MAX,NULL,
};

// BW調色
const lkup ekCanonBWColorFilter[] = {
	-1,		euNone,
	0,		euNone,
	1,		euSepia,
	2,		euClrBlue,
	3,		euClrMagenta,
	4,		euClrGreen,
	INT_MAX,NULL,
};

// カメラタイプ
const lkup ekCanonCameraType[] = {
	248,	_T("EOS ハイエンド"),
	250,	_T("コンパクト"),
	252,	_T("EOS ミッドレンジ"),
	255,	_T("DV カメラ"),
	INT_MAX,NULL,
};

// トーンカーブ
const lkup ekCanonToneCurve[] = {
	0,		euHyojun,
	1,		euManual,
	2,		euCustom,
	INT_MAX,NULL,
};

// レンズID
const lkup ekCanonLensTypes[] = {
     1, _T("EF 50mm f/1.8"),
     2, _T("EF 28mm f/2.8"),
     3, _T("EF 135mm f/2.8 Soft"),
     4, _T("EF 35-105mm f/3.5-4.5"),
     5, _T("EF 35-70mm f/3.5-4.5"),
     6, _T("EF 28-70mm f/3.5-4.5"),
     7, _T("EF 100-300mm f/5.6L"),
     8, _T("EF 100-300mm f/5.6"),
     9, _T("EF 70-210mm f/4"),
    10, _T("EF 50mm f/2.5 Macro"),
    11, _T("EF 35mm f/2"),
    13, _T("EF 15mm f/2.8 Fisheye"),
    14, _T("EF 50-200mm f/3.5-4.5L"),
    15, _T("EF 50-200mm f/3.5-4.5"),
    16, _T("EF 35-135mm f/3.5-4.5"),
    17, _T("EF 35-70mm f/3.5-4.5A"),
    18, _T("EF 28-70mm f/3.5-4.5"),
    20, _T("EF 100-200mm f/4.5A"),
    21, _T("EF 80-200mm f/2.8L"),
    22, _T("EF 20-35mm f/2.8L"),
    23, _T("EF 35-105mm f/3.5-4.5"),
    24, _T("EF 35-80mm f/4-5.6 Power Zoom"),
    25, _T("EF 35-80mm f/4-5.6 Power Zoom"),
    26, _T("EF 100mm f/2.8 Macro"),
    27, _T("EF 35-80mm f/4-5.6"),
    28, _T("EF 80-200mm f/4.5-5.6"),
    29, _T("EF 50mm f/1.8 II"),
    30, _T("EF 35-105mm f/4.5-5.6"),
    31, _T("EF 75-300mm f/4-5.6"),
    32, _T("EF 24mm f/2.8"),
    35, _T("EF 35-80mm f/4-5.6"),
    36, _T("EF 38-76mm f/4.5-5.6"),
    37, _T("EF 35-80mm f/4-5.6"),
    38, _T("EF 80-200mm f/4.5-5.6"),
    39, _T("EF 75-300mm f/4-5.6"),
    40, _T("EF 28-80mm f/3.5-5.6"),
    41, _T("EF 28-90mm f/4-5.6"),
    42, _T("EF 28-200mm f/3.5-5.6"),
    43, _T("EF 28-105mm f/4-5.6"),
    44, _T("EF 90-300mm f/4.5-5.6"),
    45, _T("EF-S 18-55mm f/3.5-5.6 [II]"),
    46, _T("EF 28-90mm f/4-5.6"),
    48, _T("EF-S 18-55mm f/3.5-5.6 IS"),
    49, _T("EF-S 55-250mm f/4-5.6 IS"),
    50, _T("EF-S 18-200mm f/3.5-5.6 IS"),
    51, _T("EF-S 18-135mm f/3.5-5.6 IS"),
    52, _T("EF-S 18-55mm f/3.5-5.6 IS II"),
    53, _T("EF-S 18-55mm f/3.5-5.6 III"),
    54, _T("EF-S 55-250mm f/4-5.6 IS II"),
    94, _T("TS-E 17mm f/4L"),
    95, _T("TS-E 24.0mm f/3.5 L II"),
    124, _T("MP-E 65mm f/2.8 1-5x Macro Photo"),
    125, _T("TS-E 24mm f/3.5L"),
    126, _T("TS-E 45mm f/2.8"),
    127, _T("TS-E 90mm f/2.8"),
    129, _T("EF 300mm f/2.8L"),
    130, _T("EF 50mm f/1.0L"),
    131, _T("EF 28-80mm f/2.8-4L or Sigma Lens"),
    132, _T("EF 1200mm f/5.6L"),
    134, _T("EF 600mm f/4L IS"),
    135, _T("EF 200mm f/1.8L"),
    136, _T("EF 300mm f/2.8L"),
    137, _T("EF 85mm f/1.2L or Sigma or Tamron Lens"),
    138, _T("EF 28-80mm f/2.8-4L"),
    139, _T("EF 400mm f/2.8L"),
    140, _T("EF 500mm f/4.5L"),
    141, _T("EF 500mm f/4.5L"),
    142, _T("EF 300mm f/2.8L IS"),
    143, _T("EF 500mm f/4L IS"),
    144, _T("EF 35-135mm f/4-5.6 USM"),
    145, _T("EF 100-300mm f/4.5-5.6 USM"),
    146, _T("EF 70-210mm f/3.5-4.5 USM"),
    147, _T("EF 35-135mm f/4-5.6 USM"),
    148, _T("EF 28-80mm f/3.5-5.6 USM"),
    149, _T("EF 100mm f/2 USM"),
    150, _T("EF 14mm f/2.8L or Sigma Lens"),
    151, _T("EF 200mm f/2.8L"),
    152, _T("EF 300mm f/4L IS or Sigma Lens"),
    153, _T("EF 35-350mm f/3.5-5.6L or Sigma or Tamron Lens"),
    154, _T("EF 20mm f/2.8 USM"),
    155, _T("EF 85mm f/1.8 USM"),
    156, _T("EF 28-105mm f/3.5-4.5 USM or Tamron Lens"),
    160, _T("EF 20-35mm f/3.5-4.5 USM or Tamron or Tokina Lens"),
    161, _T("EF 28-70mm f/2.8L or Sigma or Tamron Lens"),
    162, _T("EF 200mm f/2.8L"),
    163, _T("EF 300mm f/4L"),
    164, _T("EF 400mm f/5.6L"),
    165, _T("EF 70-200mm f/2.8 L"),
    166, _T("EF 70-200mm f/2.8 L + 1.4x"),
    167, _T("EF 70-200mm f/2.8 L + 2x"),
    168, _T("EF 28mm f/1.8 USM"),
    169, _T("EF 17-35mm f/2.8L or Sigma Lens"),
    170, _T("EF 200mm f/2.8L II"),
    171, _T("EF 300mm f/4L"),
    172, _T("EF 400mm f/5.6L or Sigma Lens"),
    173, _T("EF 180mm Macro f/3.5L or Sigma Lens"),
    174, _T("EF 135mm f/2L or Sigma Lens"),
    175, _T("EF 400mm f/2.8L"),
    176, _T("EF 24-85mm f/3.5-4.5 USM"),
    177, _T("EF 300mm f/4L IS"),
    178, _T("EF 28-135mm f/3.5-5.6 IS"),
    179, _T("EF 24mm f/1.4L"),
    180, _T("EF 35mm f/1.4L or Sigma Lens"),
    181, _T("EF 100-400mm f/4.5-5.6L IS + 1.4x"),
    182, _T("EF 100-400mm f/4.5-5.6L IS + 2x"),
    183, _T("EF 100-400mm f/4.5-5.6L IS"),
    184, _T("EF 400mm f/2.8L + 2x"),
    185, _T("EF 600mm f/4L IS"),
    186, _T("EF 70-200mm f/4L"),
    187, _T("EF 70-200mm f/4L + 1.4x"),
    188, _T("EF 70-200mm f/4L + 2x"),
    189, _T("EF 70-200mm f/4L + 2.8x"),
    190, _T("EF 100mm f/2.8 Macro USM"),
    191, _T("EF 400mm f/4 DO IS"),
    193, _T("EF 35-80mm f/4-5.6 USM"),
    194, _T("EF 80-200mm f/4.5-5.6 USM"),
    195, _T("EF 35-105mm f/4.5-5.6 USM"),
    196, _T("EF 75-300mm f/4-5.6 USM"),
    197, _T("EF 75-300mm f/4-5.6 IS USM"),
    198, _T("EF 50mm f/1.4 USM"),
    199, _T("EF 28-80mm f/3.5-5.6 USM"),
    200, _T("EF 75-300mm f/4-5.6 USM"),
    201, _T("EF 28-80mm f/3.5-5.6 USM"),
    202, _T("EF 28-80mm f/3.5-5.6 USM IV"),
    208, _T("EF 22-55mm f/4-5.6 USM"),
    209, _T("EF 55-200mm f/4.5-5.6"),
    210, _T("EF 28-90mm f/4-5.6 USM"),
    211, _T("EF 28-200mm f/3.5-5.6 USM"),
    212, _T("EF 28-105mm f/4-5.6 USM"),
    213, _T("EF 90-300mm f/4.5-5.6 USM"),
    214, _T("EF-S 18-55mm f/3.5-5.6 USM"),
    215, _T("EF 55-200mm f/4.5-5.6 II USM"),
    224, _T("EF 70-200mm f/2.8L IS"),
    225, _T("EF 70-200mm f/2.8L IS + 1.4x"),
    226, _T("EF 70-200mm f/2.8L IS + 2x"),
    227, _T("EF 70-200mm f/2.8L IS + 2.8x"),
    228, _T("EF 28-105mm f/3.5-4.5 USM"),
    229, _T("EF 16-35mm f/2.8L"),
    230, _T("EF 24-70mm f/2.8L"),
    231, _T("EF 17-40mm f/4L"),
    232, _T("EF 70-300mm f/4.5-5.6 DO IS USM"),
    233, _T("EF 28-300mm f/3.5-5.6L IS"),
    234, _T("EF-S 17-85mm f/4-5.6 IS USM"),
    235, _T("EF-S 10-22mm f/3.5-4.5 USM"),
    236, _T("EF-S 60mm f/2.8 Macro USM"),
    237, _T("EF 24-105mm f/4L IS"),
    238, _T("EF 70-300mm f/4-5.6 IS USM"),
    239, _T("EF 85mm f/1.2L II"),
    240, _T("EF-S 17-55mm f/2.8 IS USM"),
    241, _T("EF 50mm f/1.2L"),
    242, _T("EF 70-200mm f/4L IS"),
    243, _T("EF 70-200mm f/4L IS + 1.4x"),
    244, _T("EF 70-200mm f/4L IS + 2x"),
    245, _T("EF 70-200mm f/4L IS + 2.8x"),
    246, _T("EF 16-35mm f/2.8L II"),
    247, _T("EF 14mm f/2.8L II USM"),
    248, _T("EF 200mm f/2L IS"),
    249, _T("EF 800mm f/5.6L IS"),
    250, _T("EF 24 f/1.4L II"),
    251, _T("EF 70-200mm f/2.8L IS II USM"),
    252, _T("EF 70-200mm f/2.8L IS II USM + 1.4x"),
    253, _T("EF 70-200mm f/2.8L IS II USM + 2x"),
    254, _T("EF 100mm f/2.8L Macro IS USM"),
    488, _T("EF-S 15-85mm f/3.5-5.6 IS USM"),
    489, _T("EF 70-300mm f/4-5.6L IS USM"),
    490, _T("EF 8-15mm f/4L Fisheye USM"),
    491, _T("EF 300mm f/2.8L IS II USM"),
    492, _T("EF 400mm f/2.8L IS II USM"),
    493, _T("EF 500mm f/4L IS II USM or EF 24-105mm f4L IS USM"),
    494, _T("EF 600mm f/4.0L IS II USM"),
    495, _T("EF 24-70mm f/2.8L II USM"),
    496, _T("EF 200-400mm f/4L IS USM"),
    499, _T("EF 200-400mm f/4L IS USM + 1.4x"),
    502, _T("EF 28mm f/2.8 IS USM"),
    503, _T("EF 24mm f/2.8 IS USM"),
    504, _T("EF 24-70mm f/4L IS USM"),
    505, _T("EF 35mm f/2 IS USM"),
    506, _T("EF 400mm f/4 DO IS II USM"),
    507, _T("EF 16-35mm f/4L IS USM"),
    508, _T("EF 11-24mm f/4L USM"),
    4142, _T("EF-S 18-135mm f/3.5-5.6 IS STM"),
    4143, _T("EF-M 18-55mm f/3.5-5.6 IS STM"),
    4144, _T("EF 40mm f/2.8 STM"),
    4145, _T("EF-M 22mm f/2 STM"),
    4146, _T("EF-S 18-55mm f/3.5-5.6 IS STM"),
    4147, _T("EF-M 11-22mm f/4-5.6 IS STM"),
    4148, _T("EF-S 55-250mm f/4-5.6 IS STM"),
    4149, _T("EF-M 55-200mm f/4.5-6.3 IS STM"),
    4150, _T("EF-S 10-18mm f/4.5-5.6 IS STM"),
    4152, _T("EF 24-105mm f/3.5-5.6 IS STM"),
    4154, _T("EF-S 24mm f/2.8 STM"),
    4156, _T("EF 50mm f/1.8 STM"),
	INT_MAX,NULL,
};

// パノラマ方向
const lkup ekCanonPanoramaDirection[] = {
	0,		_T("左→右"),
	1,		_T("右→左"),
	2,		_T("下→上"),
	3,		_T("上→下"),
	4,		_T("2x2マトリクス(時計回り)"),
	INT_MAX,NULL,
};


#endif __DATATABLE_CANON__
