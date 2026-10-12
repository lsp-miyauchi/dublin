////////////////////////////////////////////////////////////////////////
// exif_ui.h

#pragma once
#ifndef __EXIF_UI_H__
#define __EXIF_UI_H__

const TCHAR emFlashMode[] =			_T("フラッシュモード");
const TCHAR emWhiteBalance[] =		_T("ホワイトバランス");
const TCHAR emWhiteBalanceComp[] =	_T("ホワイトバランス補正");
const TCHAR emMacroMode[] =			_T("マクロモード");
const TCHAR emSharpness[] =			_T("シャープネス");
const TCHAR emContrast[] =			_T("コントラスト");
const TCHAR emSaido[] =				_T("彩度");
const TCHAR emSaturation[] =		_T("色の濃さ");
const TCHAR emColorTone[] =			_T("色あい");
const TCHAR emAdjustSharpness[] =	_T("シャープネス調整");
const TCHAR emAdjustContrast[] =	_T("コントラスト調整");
const TCHAR emAdjustSaturation[] =	_T("色の濃さ調整");
const TCHAR emAdjustColorTone[] =	_T("色あい調整");
const TCHAR emDigitalZoom[] =		_T("デジタルズーム");
const TCHAR emFlashComp[] =			_T("フラッシュ光量補正");
const TCHAR emFlashSetting[] =		_T("フラッシュ設定");
const TCHAR emShootMode[] =			_T("撮影モード");
const TCHAR emQualityMode[] =		_T("画質モード");
const TCHAR emFocusMode[] =			_T("フォーカスモード");
const TCHAR emFocusArea[] =			_T("フォーカスエリア");
const TCHAR emExposureProgram[] =	_T("露出プログラム");
const TCHAR emExposureMode[] =		_T("露出モード");
const TCHAR emISO[] =				_T("ISO感度");
const TCHAR emISOSetting[] =		_T("ISOセッティング");
const TCHAR emColorMode[] =			_T("カラーモード");
const TCHAR emMeteringMode[] = 		_T("測光モード");
const TCHAR emDriveMode[] =			_T("ドライブモード");
const TCHAR emShutterSpeed[] =		_T("シャッター速度");
const TCHAR emAperture[] =			_T("絞り値");
const TCHAR emExpComp[] =			_T("露出補正");
const TCHAR emFocalLength[] =		_T("焦点距離");
const TCHAR emDateTime[] =			_T("撮影日時");
const TCHAR emMaxAperture[] =		_T("開放F値");
const TCHAR emColorFilter[] =		_T("カラーフィルタ");
const TCHAR emBrightness[] =		_T("明るさ");
const TCHAR emVersion[] =			_T("バージョン");
const TCHAR emColorSpace[] =		_T("カラースペース");
const TCHAR emExposureTime[] =		_T("露出時間");
const TCHAR emImageSize[] =			_T("イメージサイズ");
const TCHAR emTargetDistance[] =	_T("被写体距離");
const TCHAR emLensInfo[] =			_T("レンズ情報");
const TCHAR emFocusPoint[] =		_T("フォーカスポイント");
const TCHAR emFirmware[] =			_T("ファームウェア");
const TCHAR emSoftware[] =			_T("ソフトウェア");
const TCHAR emColorTemp[] =			_T("色温度");
const TCHAR emSerialNumber[] =		_T("シリアル番号");
const TCHAR emSceneMode[] =			_T("シーンモード");
const TCHAR emLightSource[] =		_T("光源");
const TCHAR emFocusDistance[] =		_T("フォーカス距離");
const TCHAR emAntiShake[] =			_T("手ブレ補正");
const TCHAR emLensName[] =			_T("使用レンズ名");
const TCHAR emBracketStep[] =		_T("ブラケットステップ");
const TCHAR emHardwareType[] =		_T("ハードウェアタイプ");
const TCHAR emPhotoFinish[] =		_T("画像仕上げ");
const TCHAR emStrobeComp[] =		_T("ストロボ調光補正");
const TCHAR emResultCondition[] =	_T("仕上がり設定");
const TCHAR emTotalTakeCount[] =	_T("全撮影回数");
const TCHAR emShutterCount[] =		_T("シャッター回数");
const TCHAR emAFMode[] =			_T("AFモード");
const TCHAR emAFArea[] =			_T("AFエリア");
const TCHAR emNoiseReduction[] =	_T("ノイズリダクション");
const TCHAR emLongExposureNR[] =	_T("長秒時ノイズリダクション");
const TCHAR emFilterEffect[] =		_T("フィルター効果");
const TCHAR emFirmwareVersion[] =	_T("ファームウェアバージョン");


///////////////////////////////////////////////////////

const TCHAR euNA[] =				_T("n/a");
const TCHAR euHyojun[] =			_T("標準");
const TCHAR euStandard[] =			_T("スタンダード");
const TCHAR euNormal[] =			_T("ノーマル");
const TCHAR euUndefine[] =			_T("未定義");
const TCHAR euUnknown[] =			_T("不明");
const TCHAR euNever[] =				_T("しない");
const TCHAR euNoFunction[] =		_T("機能なし");
const TCHAR euNone[] =				_T("なし");
const TCHAR euExist[] =				_T("あり");
const TCHAR euDisable[] =			_T("禁止");
const TCHAR euOther[] =				_T("その他");
const TCHAR euFlashExternal[] =		_T("外部ストロボ");
const TCHAR euAuto[] =				_T("オート");
const TCHAR euFullAuto[] =			_T("フルオート");
const TCHAR euProgram[] =			_T("プログラム");
const TCHAR euProgramAuto[] =		_T("プログラムオート");
const TCHAR euManual[] =			_T("マニュアル");
const TCHAR euPortrait[] =			_T("ポートレート");
const TCHAR euLandscape[] =			_T("風景");
const TCHAR euSports[] =			_T("スポーツ");
const TCHAR euOn[] =				_T("オン");
const TCHAR euOff[] =				_T("オフ");
const TCHAR euMMAverage[] = 		_T("平均測光");
const TCHAR euMMCenterWait[] = 		_T("中央重点測光");
const TCHAR euMMSpot[] = 			_T("スポット測光");
const TCHAR euMMMultiSegment[] = 	_T("分割測光");
const TCHAR euSoft[] =				_T("ソフト");
const TCHAR euHard[] = 				_T("ハード");
const TCHAR euHigh[] =				_T("高い");
const TCHAR euLow[] = 				_T("低い");
const TCHAR euStrong[] = 			_T("強い");
const TCHAR euWeak[] = 				_T("弱い");
const TCHAR euSunlight[] =			_T("太陽光");
const TCHAR euDaylight[] =			_T("昼光");
const TCHAR euNight[] =				_T("夜景");
const TCHAR euNightPortrait[] =		_T("夜景ポートレート");
const TCHAR euNightAndPeople[] =	_T("夜景と人物");
const TCHAR euSunset[] =			_T("夕景");
const TCHAR euSunny[] =				_T("晴天");
const TCHAR euCloudy[] =			_T("曇り");
const TCHAR euCloudySky[] =			_T("曇天");
const TCHAR euFlourescent[] =		_T("蛍光灯");
const TCHAR euFlstDaylight[] =		_T("昼光蛍光灯");
const TCHAR euFlstDaywhite[] =		_T("昼白色蛍光灯");
const TCHAR euFlstCoolwhite[] =		_T("冷白色蛍光灯");
const TCHAR euFlstWhite[] =			_T("白色蛍光灯");
const TCHAR euFlash[] =				_T("フラッシュ");
const TCHAR euShade[] =				_T("日陰");
const TCHAR euBulblight[] =			_T("電球");
const TCHAR euWhiteBulblight[] =	_T("白熱灯");
const TCHAR euEconomy[] =			_T("エコノミー");
const TCHAR euMacro[] =				_T("マクロ");
const TCHAR euSuperMacro[] =		_T("スーパーマクロ");
const TCHAR euAutoFocus[] =			_T("オートフォーカス");
const TCHAR euManualFocus[] =		_T("マニュアルフォーカス");
const TCHAR euPanFocus[] =			_T("パンフォーカス");
const TCHAR euMono[] =				_T("モノクロ");
const TCHAR euMonochrome[] =		_T("モノクローム");
const TCHAR euBW[] =				_T("白黒");
const TCHAR euSepia[] =				_T("セピア");
const TCHAR euClear[] =				_T("クリア");
const TCHAR euDeep[] =				_T("ディープ");
const TCHAR euLight[] =				_T("ライト");
const TCHAR euProgramAE[] =			_T("プログラムAE");
const TCHAR euShutter[] =			_T("シャッター速度優先");
const TCHAR euShutterAE[] =			_T("シャッター速度優先AE");
const TCHAR euAperture[] =			_T("絞り優先");
const TCHAR euApertureAE[] =		_T("絞り優先AE");
const TCHAR euRedEye[] =			_T("赤目緩和");
const TCHAR euCustom[] =			_T("カスタム");
const TCHAR euCustom1[] =			_T("カスタム1");
const TCHAR euCustom2[] =			_T("カスタム2");
const TCHAR euCustom3[] =			_T("カスタム3");
const TCHAR euCustom4[] =			_T("カスタム4");
const TCHAR euCustom5[] =			_T("カスタム5");
const TCHAR euRAW[] =				_T("RAW");
const TCHAR euSuperFine[] =			_T("スーパーファイン");
const TCHAR euFine[] =				_T("ファイン");
const TCHAR euExtraFine[] =			_T("エクストラファイン");
const TCHAR euSingleShot[] =		_T("1コマ撮影");
const TCHAR euContinuosShot[] =		_T("連続撮影");
const TCHAR euSelfTimer[] = 		_T("セルフタイマー");
const TCHAR euSelfTimer2Sec[] = 	_T("2秒セルフタイマー");
const TCHAR euSelfTimer10Sec[] = 	_T("10秒セルフタイマー");
const TCHAR euNearScene[] =			_T("近景");
const TCHAR euFarScene[] =			_T("遠景");
const TCHAR euHispeedShoot[] =		_T("高速連写");
const TCHAR euAutoBracket[] =		_T("オートブラケット");
const TCHAR euBulb[] =				_T("バルブ");
const TCHAR euSRGB[] =				_T("sRGB");
const TCHAR euAdobeRGB[] =			_T("Adobe RGB");
const TCHAR euNatural[] =			_T("ナチュラル");
const TCHAR euNaturalPlus[] =		_T("ナチュラル+");
const TCHAR euColorTempDirect[] =	_T("色温度直接指定");
const TCHAR euAFCont[] =			_T("AF-C");
const TCHAR euAFSingle[] =			_T("AF-S");
const TCHAR euAFAuto[] =			_T("AF-A");
const TCHAR euDMF[] =				_T("DMF");
const TCHAR euUnsupp[] =			_T("***");
const TCHAR euForce[] =				_T("強制");
const TCHAR euLowKey[] =			_T("ローキー");
const TCHAR euHighKey[] =			_T("ハイキー");
const TCHAR euPosLeft[] =			_T("左");
const TCHAR euPosMidLeft[] =		_T("やや左");
const TCHAR euPosCenter[] =			_T("中央");
const TCHAR euPosRight[] =			_T("右");
const TCHAR euPosMidRight[] =		_T("やや右");
const TCHAR euPosTop[] =			_T("上");
const TCHAR euPosBottom[] =			_T("下");
const TCHAR euPosTopLeft[] =		_T("左上");
const TCHAR euPosBottomLeft[] =		_T("左下");
const TCHAR euPosTopRight[] =		_T("右上");
const TCHAR euPosBottomRight[] =	_T("右下");
const TCHAR euPosTopCenter[] =		_T("中央上");
const TCHAR euPosBottomCenter[] =	_T("中央下");
const TCHAR euPosFarLeft[] =		_T("最左");
const TCHAR euPosFarRight[] =		_T("最右");
const TCHAR euClrRed[] =			_T("赤");
const TCHAR euClrGreen[] =			_T("緑");
const TCHAR euClrBlue[] =			_T("青");
const TCHAR euClrYellow[] =			_T("黄");
const TCHAR euClrMagenta[] =		_T("紫");
const TCHAR euVivid[] =				_T("ビビッド");
const TCHAR euFireworks[] =			_T("花火");
const TCHAR euCandle[] =			_T("キャンドル");
const TCHAR euCandleLight[] =		_T("キャンドルライト");
const TCHAR euNeutral[] =			_T("ニュートラル");
const TCHAR euUnderwater[] =		_T("水中");
const TCHAR euSlowShutter[] =		_T("スローシャッター");
const TCHAR euFastShutter[] =		_T("高速シャッター");
const TCHAR euSnow[] =				_T("スノー");
const TCHAR euBeach[] =				_T("ビーチ");
const TCHAR euFoods[] =				_T("料理");
const TCHAR euMuseum[] =			_T("美術館");
const TCHAR euContTakeNumber[] =	_T("連続撮影時の連番");
const TCHAR euPeople[] =			_T("人物");
const TCHAR euParty[] =				_T("パーティー");
const TCHAR euWide[] =				_T("ワイド");
const TCHAR euSpot[] =				_T("スポット");
const TCHAR euFilmSimuration[] =	_T("フィルムシミュレーション");
const TCHAR euStrobe[] =			_T("ストロボ");
const TCHAR euAutumn[] =			_T("紅葉");
const TCHAR euSurfAndSnow[] =		_T("サーフ＆スノー");
const TCHAR euFlower[] =			_T("花");
const TCHAR euKids[] =				_T("キッズ");
const TCHAR euPet[] =				_T("ペット");


// 露出制御モード
const lkup ekExposureProgram[] = {
	0,		euUndefine,
	1,		euManual,
	2,		_T("ノーマルプログラム"),
	3,		euApertureAE,
	4,		euShutterAE,
	5,		_T("Creativeプログラム"),
	6,		_T("Actionプログラム"),
	7,		euPortrait,
	8,		_T("ランドスケープ"),
	INT_MAX,NULL,
};

// 測光方式
const lkup ekMeteringMode[] = {
	1,		euMMAverage,
	2,		euMMCenterWait,
	3,		euMMSpot,
	4,		_T("マルチスポット測光"),
	5,		euMMMultiSegment,
	6,		_T("部分測光"),
	INT_MAX,NULL,
};

// 光源
const lkup ekLightSource[] = {
	0x00,	euUnknown,
	0x01,	euDaylight,
	0x02,	euFlourescent,
	0x03,	euWhiteBulblight,
	0x04,	euFlash,
	0x09,	euSunny,
	0x0A,	euCloudy,
	0x0B,	euShade,
	0x0C,	euFlstDaylight,
	0x0D,	euFlstDaywhite,
	0x0E,	euFlstCoolwhite,
	0x0E,	euFlstWhite,
	0x11,	_T("標準ライトA"),
	0x12,	_T("標準ライトB"),
	0x13,	_T("標準ライトC"),
	0x14,	_T("D55"),
	0x15,	_T("D65"),
	0x16,	_T("D75"),
	0x17,	_T("D50"),
	0x18,	_T("ISO Studio tungsten"),
	0xff,	euOther,
	INT_MAX,NULL,
};

// 露光モード
const TCHAR* euExposureMode[] = {
	euAuto,								// 0
	euManual,							// 1
	euAutoBracket,						// 2
};

// ホワイトバランスモード
const TCHAR* euWhiteBalanceMode[] = {
	euAuto,								// 0
	euManual,							// 1
};

// 撮影シーンタイプ
const TCHAR* euSceneCaptureType[] = {
	euHyojun,							// 0
	euLandscape,						// 1
	euPortrait,							// 2
	euNight,							// 3
};

// HighLow（汎用）
const TCHAR* euHighLow[] = {
	euHyojun,							// 0
	euLow,								// 1
	euHigh,								// 2
};

// HardSoft（汎用）
const TCHAR* euHardSoft[] = {
	euHyojun,							// 0
	euSoft,								// 1
	euHard,								// 2
};

// 被写体の距離範囲
const TCHAR* euDistanceRange[] = {
	euUnknown,							// 0
	euMacro,							// 1
	euNearScene,						// 2
	euFarScene,							// 3
};

// OffOn（汎用）
const lkup ekOffOn[] = {
	0,		euOff,
	1,		euOn,
	INT_MAX,NULL,
};

// OffOn or Auto（汎用）
const lkup ekOffOnAuto[] = {
	0,		euOff,
	1,		euOn,
	2,		euAuto,
	INT_MAX,NULL,
};

// フラッシュ
const lkup ekFlashFired[] = {
	0x00,	euNone,
	0x01,	_T("オン"),
	0x05,	_T("オン,発光未検出"),
	0x07,	_T("オン,発光検出"),
	0x09,	_T("オン（強制）"),
	0x0d,	_T("オン（強制）,発光未検出"),
	0x0f,	_T("オン（強制）,発光検出"),
	0x10,	_T("オフ（強制）"),
	0x18,	_T("オフ（自動）"),
	0x19,	_T("オン（自動）"),
	0x1d,	_T("オン（自動）,発光未検出"),
	0x1f,	_T("オン（自動）,発光検出"),
	0x20,	euNoFunction,
	0x41,	_T("オン（赤目緩和）"),
	0x45,	_T("オン（赤目緩和）,発光未検出"),
	0x47,	_T("オン（赤目緩和）,発光検出"),
	0x49,	_T("オン（強制+赤目緩和）"),
	0x4d,	_T("オン（強制+赤目緩和）,発光未検出"),
	0x4f,	_T("オン（強制+赤目緩和）,発光検出"),
	0x58,	_T("オフ（自動+赤目緩和）"),
	0x59,	_T("オン（自動+赤目緩和）"),
	0x5d,	_T("オン（自動+赤目緩和）,発光未検出"),
	0x5f,	_T("オン（自動+赤目緩和）,発光検出"),
	INT_MAX,NULL,
};

const TCHAR* euGainControl[] = {
	euNone,								// 0 = なし
	_T("LowGainUp"),					// 1 = Low gain up
	_T("HighGainUp"),					// 2 = High gain up
	_T("LowGainDown"),					// 3 = Low gain down
	_T("HighGainDown"),					// 4 = High gain down
};

// ありなし
const TCHAR* euAriNashi[] = {
	euNone,								// 0
	euExist,							// 1
};


////////////////////////////////////////////////////////////////////////
// CASIO

// ホワイトバランス
const lkup ekCasioWhiteBalance[] = {
	0x00,	euSunny,
	0x01,	euAuto,
	0x03,	euShade,
	0x05,	euFlourescent,
	0x06,	euAuto,
	0x09,	euWhiteBulblight,
	0x0A,	_T("（オート）"),		// ???
	0x0C,	euFlash,
	0x0D,	euManual,
	0x0F,	euAuto,
	INT_MAX,NULL,
};

#define CASIO_SHOOTMODE_BESTSHOT		0x06

// 撮影モード
const lkup ekCasioShootMode[] = {
	0x02,						euHyojun,
	0x03,						euShutter,
	0x04,						euAperture,
	0x05,						euManual,
	CASIO_SHOOTMODE_BESTSHOT,	_T("ベストショット"),
	INT_MAX,					NULL,
};

// セルフタイマー
const lkup ekCasioSelfTimer[] = {
	1,		euOff,
	7,		euOn,
	18,		_T("トリプルセルフタイマー"),
	INT_MAX,NULL,
};

// シャッターモード
const lkup ekCasioShutterMode[] = {
	1,		euHyojun,
	3,		_T("ブラケット露出"),
	7,		_T("ズーム連射"),
	18,		_T("パスト連射"),
//	xx,		_T("スローライブ撮影"),
	INT_MAX,NULL,
};


// 画質モード
const lkup ekCasioQuality[] = {
	1,		euEconomy,
	2,		euHyojun,
	3,		_T("高精細"),
	4,		_T("超高精細"),
	INT_MAX,NULL,
};

// フォーカスモード
const lkup ekCasioFocusMode[] = {
	1,		_T("固定"),
	2,		euMacro,
	3,		euAutoFocus,
	4,		euManualFocus,
	5,		euPanFocus,
	6,		_T("マルチエリアAF"),
	7,		_T("スポットAF"),
	INT_MAX,NULL,
};

// カラーモード
const lkup ekCasioColorMode[] = {
	0,		_T("カラー"),
	2,		euMono,
	3,		euSepia,
	INT_MAX,NULL,
};

// 色強調
const lkup ekCasioColorType[] = {
	0,		euNone,
	1,		euClrBlue,
	3,		euClrGreen,
	4,		euClrYellow,
	5,		euClrRed,
	6,		euClrMagenta,
	7,		_T("ピンク"),
	9,		_T("肌色"),
	INT_MAX,NULL,
};

// ベストショット
const TCHAR bsCasioAuto[]					= _T("オート");
const TCHAR bsCasioMovie[]					= _T("ムービー");
const TCHAR bsCasioPeople[]					= _T("人物");
const TCHAR bsCasioLandscape[]				= _T("風景");
const TCHAR bsCasioLandscapeAndPeople[]		= _T("風景と人物");
const TCHAR bsCasioSports[]					= _T("スポーツ");
const TCHAR bsCasioSoftFocus[]				= _T("ソフトフォーカス");
const TCHAR bsCasioCuplingShot[]			= _T("カップリングショット");
const TCHAR bsCasioPreShot[]				= _T("プリショット");
const TCHAR bsCasioChildlen[]				= _T("子供");
const TCHAR bsCasioCandleLight[]			= _T("キャンドルライト");
const TCHAR bsCasioParty[]					= _T("パーティー");
const TCHAR bsCasioPet[]					= _T("ペット");
const TCHAR bsCasioFlower[]					= _T("花");
const TCHAR bsCasioGreen[]					= _T("緑を鮮やかに");
const TCHAR bsCasioSunset[]					= _T("夕日");
const TCHAR bsCasioNight[]					= _T("夜景");
const TCHAR bsCasioNightAndPeople[]			= _T("夜景と人物");
const TCHAR bsCasioFireworks[]				= _T("花火");
const TCHAR bsCasioFoods[]					= _T("食べ物");
const TCHAR bsCasioCharactor[]				= _T("文字");
const TCHAR bsCasioCollection[]				= _T("コレクション");
const TCHAR bsCasioBlackAndWhite[]			= _T("白黒");
const TCHAR bsCasioRetro[]					= _T("レトロ");
const TCHAR bsCasioTwilight[]				= _T("トワイライト");
const TCHAR bsCasioDocuments[]				= _T("名刺や書類");
const TCHAR bsCasioWhiteBoard[]				= _T("ホワイトボード");
const TCHAR bsCasioPastel[]					= _T("パステル");
const TCHAR bsCasioIllust[]					= _T("イラスト");
const TCHAR bsCasioCross[]					= _T("クロス");
const TCHAR bsCasioAutumnRed[]				= _T("紅葉");
const TCHAR bsCasioSlowShutter[]			= _T("水の流れを滑らかに");
const TCHAR bsCasioFastShutter[]			= _T("水しぶきを止めて");
const TCHAR bsCasioAuction[]				= _T("オークション");
const TCHAR bsCasioBacklight[]				= _T("逆光");
const TCHAR bsCasioAntiShake[]				= _T("ブレ軽減");
const TCHAR bsCasioHiSensitibity[]			= _T("高感度");
const TCHAR bsCasioOldPhoto[]				= _T("古い写真");
const TCHAR bsCasioVoiceRecord[]			= _T("ボイスレコード");
const TCHAR bsCasioProofPhoto[]				= _T("証明写真");
const TCHAR bsCasioSilent[]					= _T("サイレント");
const TCHAR bsCasioShortMovie[]				= _T("ショートムービー");
const TCHAR bsCasioPastMovie[]				= _T("パストムービー");
const TCHAR bsCasioUnderWater[]				= _T("水中");
const TCHAR bsCasioLayoutShot2[]			= _T("レイアウトショット（2）");
const TCHAR bsCasioLayoutShot3[]			= _T("レイアウトショット（3）");
const TCHAR bsCasioAutoFraming[]			= _T("オートフレーミング");

const lkup ekCasioBestShot_Z3_4_30_40_50_55_57_S100[] = {
	1,		bsCasioPeople,
	2,		bsCasioLandscape,
	3,		bsCasioLandscapeAndPeople,
	4,		bsCasioCuplingShot,
	5,		bsCasioPreShot,
	6,		bsCasioChildlen,
	7,		bsCasioCandleLight,
	8,		bsCasioParty,
	9,		bsCasioPet,
	10,		bsCasioFlower,
	11,		bsCasioGreen,
	12,		bsCasioSunset,
	13,		bsCasioNight,
	14,		bsCasioNightAndPeople,
	15,		bsCasioFireworks,
	16,		bsCasioFoods,
	17,		bsCasioCharactor,
	18,		bsCasioCollection,
	19,		bsCasioBlackAndWhite,
	20,		bsCasioRetro,
	21,		bsCasioTwilight,
	22,		bsCasioDocuments,
	23,		bsCasioWhiteBoard,
	INT_MAX,NULL,
};

const lkup ekCasioBestShot_Z60[] = {
	1,		bsCasioAuto,
	2,		bsCasioMovie,
	3,		bsCasioPeople,
	4,		bsCasioLandscape,
	5,		bsCasioLandscapeAndPeople,
	6,		bsCasioChildlen,
	7,		bsCasioSports,
	8,		bsCasioCandleLight,
	9,		bsCasioParty,
	10,		bsCasioPet,
	11,		bsCasioFlower,
	12,		bsCasioGreen,
	13,		bsCasioAutumnRed,
	14,		bsCasioSlowShutter,
	15,		bsCasioFastShutter,
	16,		bsCasioSunset,
	17,		bsCasioNight,
	18,		bsCasioNightAndPeople,
	19,		bsCasioFireworks,
	20,		bsCasioFoods,
	21,		bsCasioCharactor,
	22,		bsCasioCollection,
	23,		bsCasioAuction,
	24,		bsCasioBacklight,
	25,		bsCasioAntiShake,
	26,		bsCasioHiSensitibity,
	27,		bsCasioBlackAndWhite,
	28,		bsCasioRetro,
	29,		bsCasioTwilight,
	30,		bsCasioOldPhoto,
	31,		bsCasioDocuments,
	32,		bsCasioWhiteBoard,
	33,		bsCasioVoiceRecord,
	INT_MAX,NULL,
};


const lkup ekCasioBestShot_Z110[] = {
	1,		bsCasioPeople,
	2,		bsCasioLandscape,
	3,		bsCasioLandscapeAndPeople,
	4,		bsCasioChildlen,
	5,		bsCasioSports,
	6,		bsCasioCandleLight,
	7,		bsCasioParty,
	8,		bsCasioPet,
	9,		bsCasioFlower,
	10,		bsCasioGreen,
	11,		bsCasioAutumnRed,
	12,		bsCasioSlowShutter,
	13,		bsCasioFastShutter,
	14,		bsCasioSunset,
	15,		bsCasioNight,
	16,		bsCasioNightAndPeople,
	17,		bsCasioFireworks,
	18,		bsCasioFoods,
	19,		bsCasioCharactor,
	20,		bsCasioCollection,
	21,		bsCasioBacklight,
	22,		bsCasioAntiShake,
	23,		bsCasioHiSensitibity,
	24,		bsCasioBlackAndWhite,
	25,		bsCasioRetro,
	26,		bsCasioTwilight,
	27,		bsCasioDocuments,
	28,		bsCasioWhiteBoard,
	INT_MAX,NULL,
};

const lkup ekCasioBestShot_Z500_600[] = {
	1,		bsCasioAuto,
	2,		bsCasioMovie,
	3,		bsCasioPeople,
	4,		bsCasioLandscape,
	5,		bsCasioLandscapeAndPeople,
	6,		bsCasioChildlen,
	7,		bsCasioSports,
	8,		bsCasioCandleLight,
	9,		bsCasioParty,
	10,		bsCasioPet,
	11,		bsCasioFlower,
	12,		bsCasioGreen,
	13,		bsCasioAutumnRed,
	14,		bsCasioSlowShutter,
	15,		bsCasioFastShutter,
	16,		bsCasioSunset,
	17,		bsCasioNight,
	18,		bsCasioNightAndPeople,
	19,		bsCasioFireworks,
	20,		bsCasioFoods,
	21,		bsCasioCharactor,
	22,		bsCasioCollection,
	23,		bsCasioBacklight,
	24,		bsCasioAntiShake,
	25,		bsCasioHiSensitibity,
	26,		bsCasioBlackAndWhite,
	27,		bsCasioRetro,
	28,		bsCasioTwilight,
	29,		bsCasioOldPhoto,
	30,		bsCasioDocuments,
	31,		bsCasioWhiteBoard,
	32,		bsCasioVoiceRecord,
	INT_MAX,NULL,
};

const lkup ekCasioBestShot_Z700[] = {
	1,		bsCasioAuto,
	2,		bsCasioMovie,
	3,		bsCasioPeople,
	4,		bsCasioLandscape,
	5,		bsCasioLandscapeAndPeople,
	6,		bsCasioChildlen,
	7,		bsCasioSports,
	8,		bsCasioCandleLight,
	9,		bsCasioParty,
	10,		bsCasioPet,
	11,		bsCasioFlower,
	12,		bsCasioGreen,
	13,		bsCasioAutumnRed,
	14,		bsCasioSlowShutter,
	15,		bsCasioFastShutter,
	16,		bsCasioSunset,
	17,		bsCasioNight,
	18,		bsCasioNightAndPeople,
	19,		bsCasioFireworks,
	20,		bsCasioFoods,
	21,		bsCasioCharactor,
	22,		bsCasioCollection,
	23,		bsCasioAuction,
	24,		bsCasioBacklight,
	25,		bsCasioAntiShake,
	26,		bsCasioHiSensitibity,
	27,		bsCasioUnderWater,
	28,		bsCasioBlackAndWhite,
	29,		bsCasioRetro,
	30,		bsCasioTwilight,
	31,		bsCasioLayoutShot2,
	32,		bsCasioLayoutShot3,
	33,		bsCasioAutoFraming,
	34,		bsCasioOldPhoto,
	35,		bsCasioDocuments,
	36,		bsCasioWhiteBoard,
	37,		bsCasioVoiceRecord,
	INT_MAX,NULL,
};

const lkup ekCasioBestShot_Z750[] = {
	1,		bsCasioPeople,
	2,		bsCasioLandscape,
	3,		bsCasioLandscapeAndPeople,
	4,		bsCasioChildlen,
	5,		bsCasioSports,
	6,		bsCasioCandleLight,
	7,		bsCasioParty,
	8,		bsCasioPet,
	9,		bsCasioFlower,
	10,		bsCasioGreen,
	11,		bsCasioSlowShutter,
	12,		bsCasioFastShutter,
	13,		bsCasioSunset,
	14,		bsCasioNight,
	15,		bsCasioNightAndPeople,
	16,		bsCasioFireworks,
	17,		bsCasioFoods,
	18,		bsCasioCharactor,
	19,		bsCasioCollection,
	20,		bsCasioBacklight,
	21,		bsCasioAntiShake,
	22,		bsCasioPastel,
	23,		bsCasioIllust,
	24,		bsCasioCross,
	25,		bsCasioBlackAndWhite,
	26,		bsCasioRetro,
	27,		bsCasioTwilight,
	28,		bsCasioProofPhoto,
	29,		bsCasioDocuments,
	30,		bsCasioWhiteBoard,
	INT_MAX,NULL,
};

const lkup ekCasioBestShot_Z850[] = {
	1,		bsCasioPeople,
	2,		bsCasioLandscape,
	3,		bsCasioLandscapeAndPeople,
	4,		bsCasioChildlen,
	5,		bsCasioSports,
	6,		bsCasioCandleLight,
	7,		bsCasioParty,
	8,		bsCasioPet,
	9,		bsCasioFlower,
	10,		bsCasioGreen,
	11,		bsCasioAutumnRed,
	12,		bsCasioSlowShutter,
	13,		bsCasioFastShutter,
	14,		bsCasioSunset,
	15,		bsCasioNight,
	16,		bsCasioNightAndPeople,
	17,		bsCasioFireworks,
	18,		bsCasioFoods,
	19,		bsCasioCharactor,
	20,		bsCasioCollection,
	21,		bsCasioAuction,
	22,		bsCasioBacklight,
	23,		bsCasioAntiShake,
	24,		bsCasioHiSensitibity,
	25,		bsCasioPastel,
	26,		bsCasioIllust,
	27,		bsCasioCross,
	28,		bsCasioBlackAndWhite,
	29,		bsCasioRetro,
	30,		bsCasioTwilight,
	31,		bsCasioProofPhoto,
	32,		bsCasioOldPhoto,
	33,		bsCasioDocuments,
	34,		bsCasioWhiteBoard,
	INT_MAX,NULL,
};

const lkup ekCasioBestShot_Z1000[] = {
	1,		bsCasioAuto,
	2,		bsCasioMovie,
	3,		bsCasioPeople,
	4,		bsCasioLandscape,
	5,		bsCasioLandscapeAndPeople,
	6,		bsCasioChildlen,
	7,		bsCasioSports,
	8,		bsCasioCandleLight,
	9,		bsCasioParty,
	10,		bsCasioPet,
	11,		bsCasioFlower,
	12,		bsCasioGreen,
	13,		bsCasioAutumnRed,
	14,		bsCasioSlowShutter,
	15,		bsCasioFastShutter,
	16,		bsCasioSunset,
	17,		bsCasioNight,
	18,		bsCasioNightAndPeople,
	19,		bsCasioFireworks,
	20,		bsCasioFoods,
	21,		bsCasioCharactor,
	22,		bsCasioCollection,
	23,		bsCasioAuction,
	24,		bsCasioBacklight,
	25,		bsCasioAntiShake,
	26,		bsCasioHiSensitibity,
	27,		bsCasioPastel,
	28,		bsCasioIllust,
	29,		bsCasioCross,
	30,		bsCasioBlackAndWhite,
	31,		bsCasioRetro,
	32,		bsCasioTwilight,
	33,		bsCasioProofPhoto,
	34,		bsCasioOldPhoto,
	35,		bsCasioDocuments,
	36,		bsCasioWhiteBoard,
	37,		bsCasioVoiceRecord,
	INT_MAX,NULL,
};

const lkup ekCasioBestShot_Z1050[] = {
	1,		bsCasioAuto,
	2,		bsCasioMovie,
	3,		bsCasioPeople,
	4,		bsCasioLandscape,
	5,		bsCasioLandscapeAndPeople,
	6,		bsCasioChildlen,
	7,		bsCasioSports,
	8,		bsCasioCandleLight,
	9,		bsCasioParty,
	10,		bsCasioPet,
	11,		bsCasioFlower,
	12,		bsCasioGreen,
	13,		bsCasioAutumnRed,
	14,		bsCasioSlowShutter,
	15,		bsCasioFastShutter,
	16,		bsCasioSunset,
	17,		bsCasioNight,
	18,		bsCasioNightAndPeople,
	19,		bsCasioFireworks,
	20,		bsCasioFoods,
	21,		bsCasioCharactor,
	22,		bsCasioCollection,
	23,		bsCasioAuction,
	24,		bsCasioBacklight,
	25,		bsCasioAntiShake,
	26,		bsCasioHiSensitibity,
	27,		bsCasioUnderWater,
	28,		bsCasioBlackAndWhite,
	29,		bsCasioRetro,
	30,		bsCasioTwilight,
	31,		bsCasioLayoutShot2,
	32,		bsCasioLayoutShot3,
	33,		bsCasioAutoFraming,
	34,		bsCasioProofPhoto,
	35,		bsCasioOldPhoto,
	36,		bsCasioDocuments,
	37,		bsCasioWhiteBoard,
	38,		bsCasioVoiceRecord,
	INT_MAX,NULL,
};

const lkup ekCasioBestShot_Z1200[] = {
	1,		bsCasioPeople,
	2,		bsCasioLandscape,
	3,		bsCasioLandscapeAndPeople,
	4,		bsCasioChildlen,
	5,		bsCasioSports,
	6,		bsCasioCandleLight,
	7,		bsCasioParty,
	8,		bsCasioPet,
	9,		bsCasioFlower,
	10,		bsCasioGreen,
	11,		bsCasioAutumnRed,
	12,		bsCasioSlowShutter,
	13,		bsCasioFastShutter,
	14,		bsCasioSunset,
	15,		bsCasioNight,
	16,		bsCasioNightAndPeople,
	17,		bsCasioFireworks,
	18,		bsCasioFoods,
	19,		bsCasioCharactor,
	20,		bsCasioCollection,
	21,		bsCasioAuction,
	22,		bsCasioBacklight,
	23,		bsCasioHiSensitibity,
	24,		bsCasioUnderWater,
	25,		bsCasioBlackAndWhite,
	26,		bsCasioRetro,
	27,		bsCasioTwilight,
	28,		bsCasioLayoutShot2,
	29,		bsCasioLayoutShot3,
	30,		bsCasioAutoFraming,
	31,		bsCasioProofPhoto,
	32,		bsCasioOldPhoto,
	33,		bsCasioDocuments,
	34,		bsCasioWhiteBoard,
	INT_MAX,NULL,
};

const lkup ekCasioBestShot_V7[] = {
	1,		bsCasioPeople,
	2,		bsCasioLandscape,
	3,		bsCasioLandscapeAndPeople,
	4,		bsCasioChildlen,
	5,		bsCasioSports,
	6,		bsCasioCandleLight,
	7,		bsCasioParty,
	8,		bsCasioPet,
	9,		bsCasioFlower,
	10,		bsCasioGreen,
	11,		bsCasioAutumnRed,
	12,		bsCasioSlowShutter,
	13,		bsCasioFastShutter,
	14,		bsCasioSunset,
	15,		bsCasioNight,
	16,		bsCasioNightAndPeople,
	17,		bsCasioFireworks,
	18,		bsCasioFoods,
	19,		bsCasioCharactor,
	20,		bsCasioCollection,
	21,		bsCasioAuction,
	22,		bsCasioBacklight,
	23,		bsCasioHiSensitibity,
	24,		bsCasioBlackAndWhite,
	25,		bsCasioRetro,
	26,		bsCasioTwilight,
	27,		bsCasioLayoutShot2,
	28,		bsCasioLayoutShot3,
	29,		bsCasioAutoFraming,
	30,		bsCasioProofPhoto,
	31,		bsCasioOldPhoto,
	32,		bsCasioDocuments,
	33,		bsCasioWhiteBoard,
	INT_MAX,NULL,
};

const lkup ekCasioBestShot_V8[] = {
	1,		bsCasioPeople,
	2,		bsCasioLandscape,
	3,		bsCasioLandscapeAndPeople,
	4,		bsCasioChildlen,
	5,		bsCasioSports,
	6,		bsCasioCandleLight,
	7,		bsCasioParty,
	8,		bsCasioPet,
	9,		bsCasioFlower,
	10,		bsCasioGreen,
	11,		bsCasioAutumnRed,
	12,		bsCasioSlowShutter,
	13,		bsCasioFastShutter,
	14,		bsCasioSunset,
	15,		bsCasioNight,
	16,		bsCasioNightAndPeople,
	17,		bsCasioFireworks,
	18,		bsCasioFoods,
	19,		bsCasioCharactor,
	20,		bsCasioCollection,
	21,		bsCasioAuction,
	22,		bsCasioBacklight,
	23,		bsCasioHiSensitibity,
	24,		bsCasioUnderWater,
	25,		bsCasioBlackAndWhite,
	26,		bsCasioRetro,
	27,		bsCasioTwilight,
	28,		bsCasioLayoutShot2,
	29,		bsCasioLayoutShot3,
	30,		bsCasioAutoFraming,
	31,		bsCasioProofPhoto,
	32,		bsCasioOldPhoto,
	33,		bsCasioDocuments,
	34,		bsCasioWhiteBoard,
	INT_MAX,NULL,
};

const lkup ekCasioBestShot_P505[] = {
	1,		bsCasioPeople,
	2,		bsCasioLandscape,
	3,		bsCasioLandscapeAndPeople,
	4,		bsCasioChildlen,
	5,		bsCasioCandleLight,
	6,		bsCasioParty,
	7,		bsCasioPet,
	9,		bsCasioFlower,
	10,		bsCasioGreen,
	11,		bsCasioSlowShutter,
	12,		bsCasioFastShutter,
	13,		bsCasioSunset,
	14,		bsCasioNight,
	15,		bsCasioNightAndPeople,
	16,		bsCasioFireworks,
	17,		bsCasioFoods,
	18,		bsCasioCharactor,
	19,		bsCasioCollection,
	20,		bsCasioBlackAndWhite,
	21,		bsCasioRetro,
	22,		bsCasioTwilight,
	INT_MAX,NULL,
};

const lkup ekCasioBestShot_P600_700[] = {
	1,		bsCasioPeople,
	2,		bsCasioLandscape,
	3,		bsCasioLandscapeAndPeople,
	4,		bsCasioCuplingShot,
	5,		bsCasioPreShot,
	6,		bsCasioSoftFocus,
	7,		bsCasioChildlen,
	8,		bsCasioSports,
	9,		bsCasioCandleLight,
	10,		bsCasioParty,
	11,		bsCasioPet,
	12,		bsCasioFlower,
	13,		bsCasioGreen,
	14,		bsCasioSlowShutter,
	15,		bsCasioFastShutter,
	16,		bsCasioSunset,
	17,		bsCasioNight,
	18,		bsCasioNightAndPeople,
	19,		bsCasioFireworks,
	20,		bsCasioFoods,
	21,		bsCasioCharactor,
	22,		bsCasioCollection,
	23,		bsCasioBlackAndWhite,
	24,		bsCasioRetro,
	25,		bsCasioTwilight,
	26,		bsCasioDocuments,
	27,		bsCasioWhiteBoard,
	INT_MAX,NULL,
};

const lkup ekCasioBestShot_S2_M2[] = {
	1,		bsCasioPeople,
	2,		bsCasioLandscape,
	3,		bsCasioNight,
	4,		bsCasioRetro,
	5,		bsCasioTwilight,
	6,		bsCasioBlackAndWhite,
};

const lkup ekCasioBestShot_S3_20[] = {
	1,		bsCasioPeople,
	2,		bsCasioLandscape,
	3,		bsCasioLandscapeAndPeople,
	4,		bsCasioCuplingShot,
	5,		bsCasioPreShot,
	6,		bsCasioPet,
	7,		bsCasioFlower,
	8,		bsCasioGreen,
	9,		bsCasioSunset,
	10,		bsCasioNight,
	11,		bsCasioNightAndPeople,
	12,		bsCasioFireworks,
	13,		bsCasioBlackAndWhite,
	14,		bsCasioRetro,
	15,		bsCasioTwilight,
	INT_MAX,NULL,
};

const lkup ekCasioBestShot_S500[] = {
	1,		bsCasioAuto,
	2,		bsCasioPeople,
	3,		bsCasioLandscape,
	4,		bsCasioLandscapeAndPeople,
	5,		bsCasioChildlen,
	6,		bsCasioSports,
	7,		bsCasioCandleLight,
	8,		bsCasioParty,
	9,		bsCasioPet,
	10,		bsCasioFlower,
	11,		bsCasioGreen,
	12,		bsCasioAutumnRed,
	13,		bsCasioSlowShutter,
	14,		bsCasioFastShutter,
	15,		bsCasioSunset,
	16,		bsCasioNight,
	17,		bsCasioNightAndPeople,
	18,		bsCasioFireworks,
	19,		bsCasioFoods,
	20,		bsCasioCharactor,
	21,		bsCasioCollection,
	22,		bsCasioBacklight,
	23,		bsCasioAntiShake,
	24,		bsCasioHiSensitibity,
	25,		bsCasioBlackAndWhite,
	26,		bsCasioRetro,
	27,		bsCasioTwilight,
	28,		bsCasioDocuments,
	29,		bsCasioWhiteBoard,
	30,		bsCasioSilent,
	31,		bsCasioShortMovie,
	32,		bsCasioPastMovie,
	33,		bsCasioVoiceRecord,
	INT_MAX,NULL,
};

const lkup ekCasioBestShot_S600[] = {
	1,		bsCasioAuto,
	2,		bsCasioPeople,
	3,		bsCasioLandscape,
	4,		bsCasioLandscapeAndPeople,
	5,		bsCasioChildlen,
	6,		bsCasioSports,
	7,		bsCasioCandleLight,
	8,		bsCasioParty,
	9,		bsCasioPet,
	10,		bsCasioFlower,
	11,		bsCasioGreen,
	12,		bsCasioAutumnRed,
	13,		bsCasioSlowShutter,
	14,		bsCasioFastShutter,
	15,		bsCasioSunset,
	16,		bsCasioNight,
	17,		bsCasioNightAndPeople,
	18,		bsCasioFireworks,
	19,		bsCasioFoods,
	20,		bsCasioCharactor,
	21,		bsCasioCollection,
	22,		bsCasioBacklight,
	23,		bsCasioAntiShake,
	24,		bsCasioHiSensitibity,
	25,		bsCasioBlackAndWhite,
	26,		bsCasioRetro,
	27,		bsCasioTwilight,
	28,		bsCasioOldPhoto,
	29,		bsCasioDocuments,
	30,		bsCasioWhiteBoard,
	31,		bsCasioSilent,
	32,		bsCasioShortMovie,
	33,		bsCasioPastMovie,
	34,		bsCasioVoiceRecord,
	INT_MAX,NULL,
};

const lkup ekCasioBestShot_S770[] = {
	1,		bsCasioAuto,
	2,		bsCasioPeople,
	3,		bsCasioLandscape,
	4,		bsCasioLandscapeAndPeople,
	5,		bsCasioChildlen,
	6,		bsCasioSports,
	7,		bsCasioCandleLight,
	8,		bsCasioParty,
	9,		bsCasioPet,
	10,		bsCasioFlower,
	11,		bsCasioGreen,
	12,		bsCasioAutumnRed,
	13,		bsCasioSlowShutter,
	14,		bsCasioFastShutter,
	15,		bsCasioSunset,
	16,		bsCasioNight,
	17,		bsCasioNightAndPeople,
	18,		bsCasioFireworks,
	19,		bsCasioFoods,
	20,		bsCasioCharactor,
	21,		bsCasioCollection,
	22,		bsCasioAuction,
	23,		bsCasioBacklight,
	24,		bsCasioAntiShake,
	25,		bsCasioHiSensitibity,
	26,		bsCasioBlackAndWhite,
	27,		bsCasioRetro,
	28,		bsCasioTwilight,
	29,		bsCasioOldPhoto,
	30,		bsCasioDocuments,
	31,		bsCasioWhiteBoard,
	32,		bsCasioSilent,
	33,		bsCasioShortMovie,
	34,		bsCasioPastMovie,
	35,		bsCasioVoiceRecord,
	INT_MAX,NULL,
};

const lkup ekCasioBestShot_EXF1[] = {
	1,		bsCasioPeople,
	2,		bsCasioLandscape,
	3,		bsCasioLandscapeAndPeople,
	4,		bsCasioChildlen,
	5,		bsCasioSports,
	6,		bsCasioCandleLight,
	7,		bsCasioParty,
	8,		bsCasioPet,
	9,		bsCasioFlower,
	10,		bsCasioGreen,
	11,		bsCasioAutumnRed,
	12,		bsCasioSlowShutter,
	13,		bsCasioFastShutter,
	14,		bsCasioSunset,
	15,		bsCasioNight,
	16,		bsCasioNightAndPeople,
	17,		bsCasioFireworks,
	18,		bsCasioFoods,
	19,		bsCasioCharactor,
	20,		bsCasioCollection,
	21,		bsCasioAuction,
	22,		_T("デジタル手ブレが補正"),
	23,		_T("デジタル流し撮り"),
	24,		_T("ムーブアウト連写"),
	25,		_T("ムーブイン連写"),
	26,		_T("YouTube"),
	INT_MAX,NULL,
};

////////////////////////////////////////////////////////////////////////
// MINOLTA

// 露出制御プログラム
const lkup ekMinoltaExpMode[] = {
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

// フラッシュモード
const lkup ekMinoltaFlashMode[] = {
	0,		_T("通常発光"),
	1,		euRedEye,
	2,		_T("後幕シンクロ"),
	3,		_T("ワイヤレス"),
	INT_MAX,NULL,
};

// 絞り値
const lkup ekMinoltaAperture[] = {
	0x0028,		_T("F4"),
	0x002B,		_T("F4.5"),
	0x0030,		_T("F5.6"),
	0x003D,		_T("F10"),
	0x0048,		_T("F16"),
	0x004B,		_T("F18"),
	INT_MAX,	NULL,
};

// 調光モード
const lkup ekMinoltaFlashControl[] = {
	0,		_T("ADI調光"),
	1,		_T("P-TTL調光"),
	2,		_T("内蔵マニュアル"),
	INT_MAX,NULL,
};

// ホワイトバランス
const lkup ekMinoltaWhiteBalance[] = {
	0,		euAuto,						//
	1,		euDaylight,					//
	2,		euCloudy,					//
	3,		euWhiteBulblight,				//
	5,		euCustom,					//
	7,		euFlourescent,				//
	8,		_T("蛍光灯2"),				//
	11,		euCustom2,					//
	12,		euCustom3,					//
	0x0100,	euDaylight,					// DiMAGE A2 and after ↓
	0x0200,	euCloudy,					//
	0x0300,	euWhiteBulblight,				//
	0x0400,	euFlash,					//
	0x0500,	euFlourescent,				//
	0x0600,	euShade,					//
	0x0700,	euCustom1,					//
	0x0800,	euCustom2,					//
	0x0900,	euCustom3,					//
	INT_MAX,NULL,
};

// ホワイトバランス(ALPHA-7D)
const lkup ekMinoltaWhiteBalance7D[] = {
	0,		euAuto,						//
	1,		euDaylight,					//
	2,		euCloudy,					//
	3,		euShade,					//
	4,		euWhiteBulblight,				//
	5,		euFlourescent,				//
	6,		euFlash,					//
	0x100,	euColorTempDirect,			//
	0x200,	euCustom1,
	0x300,	euCustom2,
	0x400,	euCustom3,
	INT_MAX,NULL,
};

// イメージ品質
const lkup ekMinoltaImageQuality[] = {
	0,		euRAW,
	1,		euSuperFine,
	2,		euFine,
	3,		euHyojun,
	4,		euEconomy,
	5,		euExtraFine,
	6,		euFine,
	7,		euFine,
	INT_MAX,NULL,
};

// ドライブモード
const lkup ekMinoltaDriveMode[] = {
	0,		euSingleShot,					//
	1,		euContinuosShot,				//
	2,		euSelfTimer,					//
	4,		_T("ブラケティング撮影"),		//
	5,		_T("インターバル撮影"),			//
	6,		_T("ウルトラハイスピード連写"),	//
	7,		_T("ハイスピード連写"),			//
	INT_MAX,NULL,
};

// ドライブモード(ALPHA-7D)
const lkup ekMinoltaDriveMode7D[] = {
	5,		euSingleShot,					//
	1,		euContinuosShot,				//
	0,		euSelfTimer10Sec,				// 10秒
	4,		euSelfTimer2Sec,				// 2秒
	2,		_T("1コマブラケット撮影"),		//
	3,		_T("連続ブラケット撮影"),		//
	7,		_T("WBブラケット撮影"),
	INT_MAX,NULL,
};

// 測光モード
const lkup ekMinoltaMeteringMode[] = {
	0,		_T("マルチセグメント"),
	1,		euMMCenterWait,
	2,		euMMSpot,
	INT_MAX,NULL,
};

// フォーカス／レリーズ
const lkup ekMinoltaFocusRelease[] = {
	0,		_T("オートフォーカス優先"),
	1,		_T("レリーズ優先"),
	INT_MAX,NULL,
};


// ブラケットステップ
const lkup ekMinoltaBracketStep[] = {
	0,		_T("1/3EV"),
	1,		_T("2/3EV"),
	2,		_T("1EV"),
	INT_MAX,NULL,
};

// カラーモード
const lkup ekMinoltaColorMode[] = {
	0,		euNatural,					//
	1,		euMono,						//
	2,		euVivid,					//
	3,		_T("ソラリゼーション"),		//
	4,		euAdobeRGB,
	5,		euSepia,
	9,		euNatural,
	12,		euPortrait,
	13, 	euNatural,
	14, 	euNaturalPlus,
	15,		euLandscape,
	16,		euSunset,
	17,		euNight,
	18,		euNightPortrait,
	INT_MAX,NULL,
};

// カラースペース
const lkup ekMinoltaColorSpace[] = {
	0, 		euNatural,
	1, 		euNaturalPlus,
	4,		euAdobeRGB,
	INT_MAX,NULL,
};

// 画像仕上げ (α-SWEET-Digital)
const lkup ekMinoltaPhotoFinish[] = {
	1,		euMono,
	4,		euAdobeRGB,
	12,		euPortrait,
	13, 	euNatural,
	14, 	euNaturalPlus,
	15,		euLandscape,
	16,		euSunset,
	17,		euNight,
	18,		euNightPortrait,
	INT_MAX,NULL,
};


// シャープネス
const lkup ekMinoltaSharpness[] = {
	0,		euHard,
	1,		euHyojun,
	2,		euSoft,
	INT_MAX,NULL,
};

// シーンセレクタ
const lkup ekMinoltaSubjectProgram[] = {
	0,		euNone,
	1,		euPortrait,
	2,		euNight,
	3,		euNightPortrait,
	4,		_T("夕焼け"),
	5,		euSports,
	INT_MAX,NULL,
};

// ISOセッティング
const lkup ekMinoltaISOSetting[] = {
	0,		_T("100"),
	1,		_T("200"),
	2,		_T("400"),
	3,		_T("800"),
	4,		euAuto,
	5,		_T("64"),
	INT_MAX,NULL,
};

// ISOセッティング(ALPHA-7D)
const lkup ekMinoltaISOSetting7D[] = {
	0,		euAuto,						//
	1,		_T("100"),					//
	3,		_T("200"),					//
	4,		_T("400"),					//
	5,		_T("800"),					//
	6,		_T("1600"),					//
	7,		_T("3200"),					//
	8,		_T("Hi200"),
	10,		_T("Lo80"),
	INT_MAX,NULL,
};

// ワイドフォーカスゾーン
const lkup ekMinoltaWideFocusZone[] = {
	0,		euNone,
	1,		euPosTopLeft,
	2,		euPosTopCenter,
	3,		euPosTopRight,
	4,		_T("左端"),
	5,		_T("さらに左"),
	6,		euPosMidLeft,
	7,		euPosCenter,
	8,		euPosMidRight,
	9,		_T("さらに右"),
	10,		_T("右端"),
	11,		euPosBottomCenter,
	INT_MAX,NULL,
};

// フォーカスモード
const lkup ekMinoltaFocusMode[] = {
	0,		euAutoFocus,
	1,		euManualFocus,
	INT_MAX,NULL,
};

// フォーカスモード(ALPHA-7D)
const lkup ekMinoltaFocusMode7D[] = {
	0,		euAFSingle,
	1,		euAFCont,
	3,		euManual,
	4,		euAFAuto,
	INT_MAX,NULL,
};

// フォーカスモード(ALPHA-SWEET-D)
const lkup ekMinoltaFocusModeSW[] = {
	0,		euAFAuto,
	1,		euAFSingle,
	2,		euAFCont,
	3,		euDMF,
	100,	euManual,
	INT_MAX,NULL,
};

// フォーカスフレーム
const lkup ekMinoltaFocusFrame[] = {
	0,		euWide,
	1,		_T("ローカル"),
	2,		euSpot,
	INT_MAX,NULL,
};

// ローカルフォーカスフレーム
const lkup ekMinoltaLocalFocusFrame[] = {
	1,		euPosCenter,
	2,		euPosTop,
	3,		euPosTopRight,
	4,		euPosRight,
	5,		euPosBottomRight,
	6,		euPosBottom,
	7,		euPosBottomLeft,
	8,		euPosLeft,
	9,		euPosTopLeft,
	INT_MAX,NULL,
};

// DECポジション
const lkup ekMinoltaDECPosition[] = {
	0,		_T("露出"),
	1,		_T("コントラスト"),
	2,		emSaido,
	3,		_T("フィルタ"),
	INT_MAX,NULL,
};

// ゾーン切替
const lkup ekMinoltaZoneMatching[] = {
	0,			euOff,
	1,			_T("HIGH"),
	2,			_T("LOW"),
	INT_MAX,	NULL,
};

// レンズ情報
// SONYレンズ情報に代替
//const lkup ekMinoltaLensInfo[] = {
//	INT_MAX,	NULL,
//};

////////////////////////////////////////////////////////////////////////
// FUJIFILM

// シャープネス
const lkup ekFujiSharpness[] = {
	0,			NULL,
	0x01,		euSoft,								//
	0x02,		_T("ソフト2"),						//
	0x03,		euHyojun,							//
	0x04,		euHard,								//
	0x05,		_T("ハード2"),						//
	0x82,		_T("ややソフト"),
	0x84,		_T("ややハード"),
	0x8000,		euFilmSimuration,
	INT_MAX,	NULL,
};

// フォーカスモード
const lkup ekFujiFocusMode[] = {
	0,		euAutoFocus,
	1,		euManualFocus,
	INT_MAX,NULL,
};

// ホワイトバランス
const lkup ekFujiWhiteBalance[] = {
	0x0000,	euAuto,
	0x0100,	euSunny,
	0x0200,	euShade,
	0x0300,	_T("蛍光灯1（昼光色）"),
	0x0301,	_T("蛍光灯2（昼白色）"),
	0x0302,	_T("蛍光灯3（白色）"),
	0x0303,	_T("蛍光灯4（温白色）"),
	0x0304,	_T("蛍光灯5（電球色）"),
	0x0400,	euWhiteBulblight,
	0x0F00,	euCustom,
	0x0F01,	euCustom2,
	0x0F02,	euCustom3,
	0x0F03,	euCustom4,
	0x0F04,	euCustom5,
	0x0FF0,	_T("ケルビン"),
	INT_MAX,NULL,
};

// HighLow
const lkup ekFujiHighLow[] = {
	0x0000,	euHyojun,
	0x0100,	euHigh,
	0x0200,	euLow,			// !
	0x0300,	euLow,			// !
	0x8000,	euFilmSimuration,
	INT_MAX,NULL,
};

// カラー
const lkup ekFujiColor[] = {
	0x0000,	_T("STD"),
	0x0080,	_T("MEDIUM HIGH"),
	0x0100,	_T("HIGH"),
	0x0180,	_T("MEDIUM LOW"),
	0x0200,	_T("LOW"),
	0x0300,	_T("B/W"),
	0x8000,	euFilmSimuration,
	INT_MAX,NULL,
};

// トーン（コントラスト）
const lkup ekFujiTone[] = {
	0x0000,	_T("STD"),
	0x0080,	_T("MEDIUM HIGH"),
	0x0100,	_T("HIGH"),
	0x0180,	_T("MEDIUM LOW"),
	0x0200,	_T("LOW"),
	0x8000,	euFilmSimuration,
	INT_MAX,NULL,
};

// フラッシュ発光モード
const lkup ekFujiFlashMode[] = {
	0,		euAuto,
	1,		_T("強制発光"),
	2,		_T("発光禁止"),
	3,		euRedEye,
	4,		euFlashExternal,
	INT_MAX,NULL,
};

// 撮影モード
const lkup ekFujiPictureMode[] = {
	0x0000,	euAuto,
	0x0001,	euPortrait,
	0x0002,	euLandscape,
	0x0004,	euSports,
	0x0005,	euNight,
	0x0006,	euProgramAuto,
	0x0007,	_T("ナチュラルフォト"),
	0x0008,	_T("ブレ軽減"),
	0x000A,	_T("夕焼け"),
	0x000B,	euMuseum,				// 美術館
	0x000C,	euParty,
	0x000D,	_T("花の接写"),
	0x000E,	_T("文字の撮影"),
	0x000F,	_T("高感度2枚撮り"),
	0x0010,	_T("ビーチ"),
	0x0011,	_T("スノー"),
	0x0012,	_T("花火"),
	0x0013,	euUnderwater,			// 水中
	0x0100,	euApertureAE,
	0x0200,	euShutterAE,
	0x0300,	euManual,
	INT_MAX,NULL,
};

// 連射モード
const lkup ekFujiContTake[] = {
	0,		euOff,
	1,		euOn,
	2,		_T("エンドレス連射"),
	3,		_T("サイクル連射"),
	INT_MAX,NULL,
};

// ノイズリダクション
const lkup ekFujiNoiseReduction[] = {
	0x40,	euLow,
	0x80,	euNormal,
	INT_MAX,NULL,
};

// FinePixカラー
const lkup ekFujiFinePixColor[] = {
	0x00,	_T("F-スタンダード"),
	0x10,	_T("F-クローム"),
	0x30,	_T("F-モノクロ"),
	INT_MAX,NULL,
};

// ダイナミックレンジ
const lkup ekFujiDynamicRange[] = {
	1,		euStandard,
	3,		euWide,
	INT_MAX,NULL,
};

const lkup ekFujiDynamicRangeSetting[] = {
	0,		_T("オート (100-400%)"),
	0x001,	euRAW,
	0x100,	_T("標準 (100%)"),
	0x200,	_T("ワイド1 (230%)"),
	0x201,	_T("ワイド2 (400%)"),
	0x8000,	euFilmSimuration,
	INT_MAX,NULL,
};

// フィルムシミュレーションモード
const lkup ekFujiFilmSimulationMode[] = {
	0,		_T("STD/なし"),
	0x100,	euPortrait,
	0x200,	_T("フジクローム"),
	0x300,	_T("スタジオポートレート"),
	0x400,	_T("ベルビア"),
	INT_MAX,NULL,
};

// フィルムシミュレーションモード(S5Pro)
const lkup ekFujiFilmSimulationModeS5Pro[] = {
	0,		euStandard,
	0x100,	_T("F1"),
	0x110,	_T("F1a"),
	0x120,	_T("F1b"),
	0x130,	_T("F1c"),
	0x200,	_T("F2"),
	0x300,	_T("F3"),
	0x400,	_T("F4"),
	INT_MAX,NULL,
};

// フィルムシミュレーションモード(S100FS/F200EXR)
const lkup ekFujiFilmSimulationModeS100FS[] = {
	0,		_T("PROVIA"),
	0x100,	_T("PORTRAIT"),				// ?
	0x120,	_T("ASTIA"),				// ?
	0x130,	_T("PORTRAIT"),				// ?
	0x200,	_T("Velvia"),
	0x300,	_T("ASTIA"),				// ?
	INT_MAX,NULL,
};



////////////////////////////////////////////////////////////////////////
// PANASONIC

// 画質モード
const lkup ekPanaQuality[] = {
	0x02,		euFine,					// 0
	0x03,		euHyojun,
	0x06,		euSuperFine,			// 1
	0x07,		euRAW,
//@	0x09,		_T("動画"),
	INT_MAX,	NULL,
};

// ホワイトバランス
const lkup ekPanaWhiteBalance[] = {
	1,			euAuto,
	2,			euSunny,
	3,			euCloudy,
	4,			euWhiteBulblight,
	5,			euManual,
	8,			euFlash,
	10,			euBW,
	12,			_T("ホワイトセット1"),
	13,			_T("ホワイトセット2"),
	INT_MAX,	NULL,
};

// フォーカスモード
const lkup ekPanaFocusMode[] = {
	1,			euAuto,
	2,			euManual,
	INT_MAX,	NULL,
};

// AFモード
const lkup ekPanaAFMode[] = {
	0x0100,		euSpot,
	0x1000,		_T("オートフォーカス（1点）"),
	0x1010,		_T("オートフォーカス（1点）高速"),
	0x0010,		_T("オートフォーカス（3点）高速"),
	0x0101,		_T("オートフォーカス（5点）"),
//@	0x0004,		_T("マクロ"),
//@	0x2002,		_T("????"),

	// --------------------------------
	// L1, L10, ...
	// --------------------------------
	0x0001,		euMMSpot,
	0x2000,		_T("インテリジェント評価測光"),
	0x2002,		euMMCenterWait,

	// --------------------------------
	// G1, ...
	// --------------------------------
	0x4000,		_T("オートフォーカス（23点）"),

	INT_MAX,	NULL,
};

// 手ブレ補正
const lkup ekPanaAntiShake[] = {
	2,			_T("オン/モード1"),
	3,			euOff,
	4,			_T("オン/モード2"),
	INT_MAX,	NULL,
};

// マクロモード
const lkup ekPanaMacroMode[] = {
	1,			euOn,
	2,			euOff,
	257,		_T("AFマクロ"),
	INT_MAX,	NULL,
};

// 露出制御モード
const lkup ekPanaShootingMode[] = {
	1,		euHyojun,
	2,		euPortrait,
	3,		euLandscape,
	4,		euSports,
	5,		euNightPortrait,
	6,		euProgramAE,
	7,		euApertureAE,
	8,		euShutterAE,
	9,		euMacro,
	11,		euManual,
	12,		_T("動画"),
	13,		_T("流し撮り"),
	14,		_T("かんたんモード"),
	16,		_T("自分撮り"),
	18,		euFireworks,
	19,		euParty,
	20,		_T("雪"),
	21,		euNight,
	22,		euFoods,
	23,		_T("赤ちゃん"),
	24,		_T("美肌"),
	27,		_T("高感度"),
	29,		euUnderwater,
	31,		_T("空撮"),
//	??,		_T("人物"),
//	??,		_T("キャンドル"),
//	??,		_T("星空"),
//	??,		_T("ビーチ"),
	41,		_T("変身"),
	43,		_T("ピンホール"),
	44,		_T("サンドブラスト"),
	45,		_T("マイカラーモード"),
	INT_MAX,NULL,
};

// コントラスト
const lkup ekPanaContrast[] = {
	0,		euHyojun,
	1,		euLow,
	2,		euHigh,
	3,		_T("とても高い"),
	18,		euHard,
	0x100,	euLow,
	0x110,	euHyojun,
	0x120,	euHigh,
	INT_MAX,NULL,
};


// カラーエフェクト
const lkup ekPanaColorEffect[] = {
	1,		euNone,
	2,		_T("ウォーム"),
	3,		_T("クール"),
	4,		euMono,
	5,		euSepia,
	INT_MAX,NULL,
};

// 画質調整
const lkup ekPanaColorMode[] = {
	0,		euHyojun,
	1,		euNatural,
	2,		euVivid,
	INT_MAX,NULL,
};

// シーンモード
const lkup ekPanaSceneMode[] = {
	2,		euPortrait,
	3,		euNight,
	INT_MAX,NULL,
};

// セルフタイマー
const lkup ekPanaSelfTimer[] = {
	1,		euOff,
	2,		euSelfTimer10Sec,
	3,		euSelfTimer2Sec,
	257,	_T("シャッターリモコン"),
	INT_MAX,NULL,
};

// 連射モード
const lkup ekPanaBurstMode[] = {
	0,		euOff,
	1,		_T("連射"),
	2,		euAutoBracket,
	4,		_T("フリー連射"),
	INT_MAX,NULL,
};

// フィルムモード
const lkup ekPanaFilmMode[] = {
	0,		euNone,
	1,		euStandard,
	2,		_T("ダイナミック"),
	3,		_T("ネイチャー"),
	4,		_T("スムーズ"),
	5,		_T("白黒・スタンダード"),
	6,		_T("白黒・ダイナミック"),
	7,		_T("白黒・スムーズ"),
	10,		_T("ノスタルジック"),
	11,		_T("バイブラント"),
	INT_MAX,NULL,
};

const lkup ekPanaMyColor[] = {
	0,		euNone,
	1,		euStandard,
	7,		_T("ポップ"),
	8,		_T("レトロ"),
	9,		_T("ピュア"),
	10,		_T("シック"),
	12,		_T("モノクローム"),
	13,		_T("ダイナミックアート"),
	14,		_T("シルエット"),
	INT_MAX,NULL,
};


////////////////////////////////////////////////////////////////////////
// SANYO

// 画質モード
const lkup ekSanyoQuality[] = {
	0,		euHyojun,
	1,		euFine,
	2,		euSuperFine,
	INT_MAX,NULL,
};

// 撮影モード
const lkup ekSanyoShootMode[] = {
	0,		euHyojun,
	1,		euMacro,
	2,		euFarScene,
	3,		euManual,
	INT_MAX,NULL,
};

// 連写モード
const lkup ekSanyoContShoot[] = {
	0,		euOff,
	1,		euHispeedShoot,
	2,		_T("ベストセレクト"),
	3,		euAutoBracket,
	INT_MAX,NULL,
};

// フォーカスモード
const lkup ekSanyoFocusMode[] = {
	0x00,	euHyojun,					// 0
	0x03,	euSuperMacro,				// 1
	INT_MAX,NULL,
};

// フラッシュモード
const lkup ekSanyoFlashMode[] = {
	0,		euAuto,
	1,		euForce,
	2,		euDisable,
	3,		euRedEye,
	INT_MAX,NULL,
};

////////////////////////////////////////////////////////////////////////
// PENTAX

// 画質モード
const lkup ekPentaxQualityMode[] = {
	0,		_T("★"),				// エコノミー
	1,		_T("★★"),				// ファイン
	2,		_T("★★★"),			// S.ファイン
	3,		_T("TIFF"),
	4,		euRAW,
	5,		_T("★★★★"),			// プレミアム
	INT_MAX,NULL,
};

// フォーカスモード
const lkup ekPentaxFocusMode[] = {
	0x00,	euNormal,
	0x01,	euMacro,
	0x02,	euPanFocus,
	0x03,	euManual,
	0x04,	euSuperMacro,
	0x10,	euAFSingle,
	0x11,	euAFCont,
	0x12,	euAFAuto,
	INT_MAX,NULL,
};

// ホワイトバランス
const lkup ekPentaxWhiteBalanceMode[] = {
	0,		euAuto,
	1,		euDaylight,
	2,		euShade,
	4,		euWhiteBulblight,
	5,		euManual,
	6,		euFlstDaylight,				// 昼光蛍光灯
	7,		euFlstDaywhite,				// 昼白色蛍光灯
	8,		euFlstWhite,				// 白色蛍光灯
	9,		euStrobe,
	10,		euCloudy,
	16,		_T("グレー点"),
	17,		_T("色温度指定"),
	65535,	_T("ユーザ設定"),
	INT_MAX,NULL,
};

// 測光モード
const lkup ekPentaxMeteringMode[] = {
	0,		euMMMultiSegment,
	1,		euMMCenterWait,
	2,		euMMSpot,
	INT_MAX,NULL,
};

// イメージサイズ
const lkup ekPentaxImageSize[] = {
	0,		_T("フル"),
	1,		_T("フル"),
	2,		_T("1024x768"),
	3,		_T("1280x960"),
	4,		_T("1600x1200"),
	5,		_T("2048x1536"),
	8,		_T("2560x1920"),
	10,		_T("3264x2448"),
	19,		_T("320x240"),
	20,		_T("2288x1712"),
	21,		_T("2592x1944"),
	22,		_T("2304x1728"),
	23,		_T("3056x2296"),
	25,		_T("2816x2112"),
	35,		_T("3056x2296"),
	36,		_T("3008x2008"),
	37,		_T("3008x2000"),
	INT_MAX,NULL,
};

// ISO
const lkup_val evPentaxISO[] = {
	3,		50,
	4,		64,
	5,		80,
	6,		100,
	7,		125,
	8,		160,
	9,		200,
	10,		250,
	11,		320,
	12,		400,
	13,		500,
	14,		640,
	15,		800,
	16,		1000,
	17,		1250,
	18,		1600,
	19,		2000,
	20,		2500,
	21,		3200,
	50,		50,
	100,	100,
	200,	200,
	400,	400,
	800,	800,
	1600,	1600,
	3200,	3200,
	258,	50,
	259,	70,
	260,	100,
	261,	140,
	262,	200,
	263,	280,
	264,	400,
	265,	560,
	266,	800,
	267,	1100,
	268,	1600,
	269,	2200,
	270,	3200,
	INT_MAX,INT_MAX,
};

// 撮影モード
const lkup ekPentaxShootMode[] = {
	// Program dial modes (from K110D)
	0x0000,		euProgramAE,
	0x0004,		euStandard,
	0x0005,		euPortrait,
	0x0006,		euLandscape,
	0x0007,		euMacro,
	0x0008,		_T("動体"),
	0x0009,		euNightAndPeople,
	0x000A,		_T("ストロボオフ"),
	// SCN modes (menu-selected) (from K100D)
	0x000B,		euNight,
	0x000C,		euSurfAndSnow,
	0x000D,		_T("テキスト"),
	0x000E,		euSunset,
	0x000F,		euKids,
	0x0010,		euPet,
	0x0011,		euCandleLight,
	0x0012,		euMuseum,
	0x0013,		euFoods,
	// AUTO PICT modes (auto-selected)
	0x0104,		_T("オートピクチャー（標準）"),
	0x0105,		_T("オートピクチャー（人物）"),
	0x0106,		_T("オートピクチャー（風景）"),
	0x0107,		_T("オートピクチャー（マクロ）"),
	0x0108,		_T("オートピクチャー（動体）"),
	// *istD modes (ref 7)
	0x0200,		euProgramAE,
	0x0201,		_T("高速優先プログラム"),
	0x0202,		_T("深度優先プログラム"),
	0x0203,		_T("MTF優先プログラム"),
	0x0300,		_T("グリーンモード"),
	0x0400,		_T("シャッター優先オート"),
	0x0500,		_T("絞り優先オート"),
	0x0600,		_T("プログラムシフト(Tv)"),
	0x0700,		_T("プログラムシフト(Av)"),
	0x0800,		euManual,
	0x0900,		euBulb,
	0x0A00,		_T("絞り優先（Aポジション以外）"),
	0x0B00,		_T("マニュアル（Aポジション以外）"),
	0x0C00,		_T("バルブ（Aポジション以外）"),
	// extra K10D modes (ref 16)
	0x0D00,		_T("シャッター＆絞り優先"),
	0x0E00,		_T("感度優先"),
	0x0F00,		_T("ストロボ同調速度"),
	INT_MAX,	NULL,
};

// 彩度・コントラスト・シャープネス
const lkup ekPentaxSCH[] = {
	-1,		euUndefine,
	0,		_T("-2"),
	1,		euHyojun,
	2,		_T("+2"),
	3,		_T("-1"),
	4,		_T("+1"),
	5,		_T("-3"),
	6,		_T("+3"),
	INT_MAX,NULL,
};

// カラースペース
const lkup ekPentaxColorSpace[] = {
	0,		euSRGB,
	1,		euAdobeRGB,
	INT_MAX,NULL,
};

// 画像仕上げ
const lkup ekPentaxPhotoFinish[] = {
	0,			euNatural,						//
	1,			_T("鮮やか"),					//
	2,			_T("人物"),
	3,			_T("風景"),
	4,			_T("雅（MIYABI）"),
	5,			_T("モノトーン"),
	INT_MAX,	NULL,
};

// ピクチャーモード
const lkup ekPentaxPictureMode[] = {
	0,			_T("選択なし"),
	2,			euProgramAE,
	3,			euManual,
	5,			euPortrait,
	6,			euLandscape,
	8,			euSports,
	9,			euNight,
	11,			euSoft,
	12,			euSurfAndSnow,
	13,			euSunset,
	14,			euAutumn,
	15,			euFlower,
	17,			euFireworks,
	18,			_T("テキスト"),
	19,			_T("パノラマアシスト"),
	30,			_T("セルフポートレート"),
	31,			_T("イラストレーション"),
	33,			_T("デジタルフィルター"),
	37,			euMuseum,
	38,			euFoods,
	40,			_T("グリーン"),
	49,			_T("ペット/白"),
	50,			_T("ペット/黒"),
	51,			_T("ペット/灰"),
	53,			_T("マーメード"),
	54,			euCandleLight,
	55,			_T("美肌"),					// 'Natural Skin Tone'
	56,			_T("ピクチャーボイス"),
	INT_MAX,	NULL,
};

// フラッシュ
const lkup ekPentaxFlashMode[] = {
	0x000,		_T("オフ（自動）"),
	0x001,		euOff,
	0x003,		_T("オフ（自動+赤目軽減）"),
	0x100,		_T("オン（自動)"),
	0x102,		euOn,
	0x103,		_T("オン（自動+赤目軽減）"),
	0x104,		_T("オン（赤目軽減）"),
	0x108,		_T("オン（ソフト）"),
	INT_MAX,	NULL,
};

// AFポイント選択
const lkup ekPentaxAFPointSelected[] = {
	0xffff,		euAuto,
	0xfffe,		_T("センター固定"),
	1,			euPosTopLeft,
	2,			euPosTopCenter,
	3,			euPosTopRight,
	4,			euPosLeft,
	5,			euPosMidLeft,
	6,			euPosCenter,
	7,			euPosMidRight,
	8,			euPosRight,
	9,			euPosBottomLeft,
	10,			euPosBottomCenter,
	11,			euPosBottomRight,
	INT_MAX,	NULL,
};

// オートAFポイント選択
const lkup ekPentaxAFPointsInFocus[] = {
	0xffff,		euNone,
	0,			_T("センター固定/マルチ"),
	1,			euPosTopLeft,
	2,			euPosTopCenter,
	3,			euPosTopRight,
	4,			euPosLeft,
	5,			euPosCenter,
	6,			euPosRight,
	7,			euPosBottomLeft,
	8,			euPosBottomCenter,
	9,			euPosBottomRight,
	INT_MAX,	NULL,
};

// レンズ情報
const lkup ekPentaxLensInfo[] = {
	0x0100,		_T("K/M Lens"),
	0x0200,		_T("A Lens"),
	0x0300,		_T("SG/"),
	0x0311,		_T("FA 85mm F2.8 SOFT"),
	0x0312,		_T("F 1.7X AF ADAPTER"),
	0x0313,		_T("F 24-50mm F4"),
	0x0314,		_T("F 35-80mm F4-5.6"),
	0x0315,		_T("F 80-200mm F4.7-5.6"),
	0x0316,		_T("FISH-EYE17-28mm F3.5-4.5"),
	0x0317,		_T("F 100-300mm F4.5-5.6"),
	0x0318,		_T("F 35-135mm F3.5-4.5"),
	0x0319,		_T("F 35-105mm F4-5.6"),
	0x031a,		_T("F* 250-600mm F5.6 ED[IF]"),
	0x031b,		_T("F 28-80mm F3.5-4.5"),
	0x031c,		_T("F 35-70mm F3.5-4.5"),
	0x031d,		_T("F 28-80mm F3.5-4.5"),
	0x031e,		_T("F 70-200mm F4-5.6"),
	0x031f,		_T("F 70-210mm F4-5.6"),
	0x0320,		_T("F 50mm F1.4"),
	0x0321,		_T("F 50mm F1.7"),
	0x0322,		_T("F 135mm F2.8 [IF]"),
	0x0323,		_T("F 28mm F2.8"),
	0x0324,		_T("SG/20mm F1.8 EX DG ASP RF"),
	0x0326,		_T("F* 300mm F4.5 ED[IF]"),
	0x0327,		_T("F* 600mm F4 ED[IF]"),
	0x0328,		_T("F Macro 100mm F2.8"),
	0x0329,		_T("F Macro 50mm F2.8"),
	0x032c,		_T("SG/18-50mm F3.5-5.6 DC"),
	0x032e,		_T("SG/APO 70-200mm F2.8 EX"),
	0x0332,		_T("FA 28-70mm F4 AL"),
	0x0333,		_T("SG/28mm F1.8 EX DG ASP Macro"),
	0x0334,		_T("FA 28-200mm F3.8-5.6 AL[IF]"),
	0x0335,		_T("FA 28-80mm F3.5-5.6 AL"),
	0x03fd,		_T("DA 14mm F2.8 ED[IF]"),
	0x03fe,		_T("DA 16-45mm F4 ED AL"),
	0x03ff,		_T("SG/18-50mm F2.8"),
	0x0401,		_T("FA 28mm F2.8 SOFT"),
	0x0402,		_T("FA 80-320mm F4.5-5.6"),
	0x0403,		_T("FA 43mm F1.9 Limited"),
	0x0406,		_T("FA 35-80mm F4-5.6"),
	0x040f,		_T("FA 28-105mm F4-5.6 [IF]"),
	0x0410,		_T("TA/AF 80-210mm F4-5.6(178D)"),
	0x0413,		_T("TA/SPAF 90mm F2.8"),
	0x0414,		_T("FA 28-80mm F3.5-5.6"),
	0x0417,		_T("FA 20-35mm F4 AL"),
	0x0418,		_T("FA 77mm F1.8 Limited"),
	0x041a,		_T("FA Macro 100mm F3.5"),
	0x041c,		_T("FA 35mm F2AL"),
	0x0422,		_T("FA 24-90mm F3.5-4.5 AL[IF]"),
	0x0423,		_T("FA 100-300mm F4.7-5.8"),
	0x0424,		_T("TA/AF 70-300mm F4-5.6 LD Macro"),
	0x0425,		_T("TA/SPAF 24-135mm F3.5-5.6 AD AL(190D)"),
	0x0426,		_T("FA 28-105mm F3.2-4.5 AL[IF]"),
	0x0427,		_T("FA 31mm F1.8AL Limited"),		//
	0x0429,		_T("TA/AF 28-200mm F3.8-5.6 ASP XR[IF] Macro"),
	0x042b,		_T("FA 28-90mm F3.5-5.6"),
	0x042c,		_T("FAJ 75-300mm F4.5-5.8 AL"),
	0x042d,		_T("TA/28-300mm F3.5-6.3 XR"),
	0x042e,		_T("FAJ 28-80mm F3.5-5.6 AL"),
	0x042f,		_T("FAJ 18-35mm F4-5.6 AL"),
	0x0431,		_T("TA/SPAF 28-75mm F2.8 XR Di"),
	0x0433,		_T("DFA Macro 50mm F2.8"),
	0x0434,		_T("DFA Macro 100mm F2.8"),
	1268,		_T("DA 21mm F3.2 AL Limited"),
	1269,		_T("D-XENON 50-200mm"),
	1270,		_T("D-XENON 18-55mm"),
	1271,		_T("DA 10-17mm F3.5-4.5 ED[IF]"),
	1272,		_T("DA 12-24mm F4 ED AL[IF]"),
	1274,		_T("DA 50-200mm F4-5.6 ED"),
	1275,		_T("DA 40mm F2.8 Limited"),		//
	1276,		_T("DA 18-55mm F3.5-5.6 AL"),		//
	1277,		_T("DA 14mm F2.8 ED[IF]"),			//
	1278,		_T("DA 16-45mm F4 ED AL"),			//
	0x0501,		_T("FA* 24mm F2 AL[IF]"),			//
	0x0502,		_T("FA 28mm F2.8 AL"),				//
	0x0503,		_T("FA 50mm F1.7"),				//
	0x0504,		_T("FA 50mm F1.4"),				//
	0x0505,		_T("FA* 600mm F4 ED[IF]"),			//
	0x0506,		_T("FA* 300mm F4.5 ED[IF]"),		//
	0x0507,		_T("FA 135mm F2.8 [IF]"),			//
	0x0508,		_T("FA Macro 50mm F2.8"),			//
	0x0509,		_T("FA Macro 100mm F2.8"),		//
	0x050a,		_T("FA* 85mm F1.4 [IF]"),			//
	0x050b,		_T("FA* 200mm F2.8 ED[IF]"),		//
	0x050c,		_T("FA 28-80mm F3.5-4.7"),		//
	0x050d,		_T("FA 70-200mm F4-5.6"),			//
	0x050e,		_T("FA* 250-600mm F5.6 ED[IF]"),	//
	0x050f,		_T("FA 28-105mm F4-5.6"),			//
	0x0510,		_T("FA 100-300mm F4.5-5.6"),		//
	0x0601,		_T("FA* 85mm F1.4 [IF]"),			//
	0x0602,		_T("FA* 200mm F2.8 ED[IF]"),		//
	0x0603,		_T("FA* 300mm F2.8 ED[IF]"),		//
	0x0604,		_T("FA* 28-70mm F2.8 AL"),			//
	0x0605,		_T("FA* 80-200mm F2.8 ED[IF]"),	//
	0x0606,		_T("FA* 28-70mm F2.8 AL"),			//
	0x0607,		_T("FA* 80-200mm F2.8 ED[IF]"),	//
	0x0608,		_T("FA 28-70mm F4 AL"),			//
	0x0609,		_T("FA 20mm F2.8"),				//
	0x060a,		_T("FA* 400mm F5.6 ED[IF]"),		//
	0x060d,		_T("FA* 400mm F5.6 ED[IF]"),		//
	0x060e,		_T("FA* Macro 200mm F4 ED[IF]"),	//
	1792,		_T("DA 21mm F3.2 AL Limited"),
	1850,		_T("DFA Macro 100mm F2.8 WR"),
	2005,		_T("DA 40mm F2.8 XS"),
	2013,		_T("DA L50-200mm F4-5.6 ED"),
	2014,		_T("DA L18-55mm F3.5-5.6 AL"),
	2016,		_T("DA 15mm F4 ED AL Limited"),
	2021,		_T("DA 18-55mm F3.5-5.6 AL II"),
	2025,		_T("DA 35mm F2.8 Macro Limited"),
	2028,		_T("DA 55-300mm F4-5.8 ED"),
	2035,		_T("DA 70mm F2.4 Limited"),
	2036,		_T("DA 16-45mm F4 ED AL"),
	2274,		_T("DA* 55mm F1.4 SDM"),
	2282,		_T("DA* 300 mm F4 ED[IF] SDM"),
	2283,		_T("DA* 200 mm F2.8 ED[IF] SDM"),
	2289,		_T("DA* 50-135mm F2.8 ED[IF] SDM"),
	2290,		_T("DA* 16-50mm F2.8 ED AL[IF] SDM"),
//smc PENTAX-DA35 mm F2.8 Macro Limited
//smc PENTAX-DA 55-300 mm F4-5.8ED
	INT_MAX,	NULL,
};

// 地域
const lkup ekPentaxHometownCity[] = {
	0,			_T("Pago Pago"),
	1,			_T("Honolulu"),
	2,			_T("Anchorage"),
	3,			_T("Vancouver"),
	4,			_T("San Fransisco"),
	5,			_T("Los Angeles"),
	6,			_T("Calgary"),
	7,			_T("Denver"),
	8,			_T("Mexico City"),
	9,			_T("Chicago"),
	10,			_T("Miami"),
	11,			_T("Toronto"),
	12,			_T("New York"),
	13,			_T("Santiago"),
	14,			_T("Caracus"),
	15,			_T("Halifax"),
	16,			_T("Buenos Aires"),
	17,			_T("Sao Paulo"),
	18,			_T("Rio de Janeiro"),
	19,			_T("Madrid"),
	20,			_T("London"),
	21,			_T("Paris"),
	22,			_T("Milan"),
	23,			_T("Rome"),
	24,			_T("Berlin"),
	25,			_T("Johannesburg"),
	26,			_T("Istanbul"),
	27,			_T("Cairo"),
	28,			_T("Jerusalem"),
	29,			_T("Moscow"),
	30,			_T("Jeddah"),
	31,			_T("Tehran"),
	32,			_T("Dubai"),
	33,			_T("Karachi"),
	34,			_T("Kabul"),
	35,			_T("Male"),
	36,			_T("Delhi"),
	37,			_T("Colombo"),
	38,			_T("Kathmandu"),
	39,			_T("Dacca"),
	40,			_T("Yangon"),
	41,			_T("Bangkok"),
	42,			_T("Kuala Lumpur"),
	43,			_T("Vientiane"),
	44,			_T("Singapore"),
	45,			_T("Phnom Penh"),
	46,			_T("Ho Chi Minh"),
	47,			_T("Jakarta"),
	48,			_T("Hong Kong"),
	49,			_T("Perth"),
	50,			_T("Beijing"),
	51,			_T("Shanghai"),
	52,			_T("Manila"),
	53,			_T("Taipei"),
	54,			_T("Seoul"),
	55,			_T("Adelaide"),
	56,			_T("Tokyo"),
	57,			_T("Guam"),
	58,			_T("Sydney"),
	59,			_T("Noumea"),
	60,			_T("Wellington"),
	61,			_T("Auckland"),
	62,			_T("Lima"),
	63,			_T("Dakar"),
	64,			_T("Algiers"),
	65,			_T("Helsinki"),
	66,			_T("Athens"),
	67,			_T("Nairobi"),
	68,			_T("Amsterdam"),
	69,			_T("Stockholm"),
	INT_MAX,	NULL,
};

////////////////////////////////////////////////////////////////////////
// EPSON

// 画質モード
const lkup ekEpsonQuality[] = {
	1,			euHyojun,
	2,			_T("詳細"),
	3,			_T("高詳細"),
	INT_MAX,	NULL,
};


////////////////////////////////////////////////////////////////////////
// RICOH

// フラッシュモード
const lkup ekRicohFlashMode[] = {
	0,			euOff,
	1,			euAuto,
	2,			euOn,
	INT_MAX,	NULL,
};

// シャープネス
const lkup ekRicohSharpness[] = {
	0,			_T("シャープ"),
	1,			euHyojun,
	2,			euSoft,
	INT_MAX,	NULL,
};

// ホワイトバランス
const lkup ekRicohWhiteBalance[] = {
	0,			euAuto,
	1,			_T("屋外"),
	2,			euCloudySky,
	3,			euWhiteBulblight,
	4,			euFlourescent,
	5,			_T("手動設定"),
	7,			_T("詳細設定"),
	9,			_T("マルチパターンAUTO"),
	INT_MAX,	NULL,
};

// ISOセッティング
const lkup ekRicohISOSetting[] = {
	0,			euAuto,
	1,			_T("64"),
	2,			_T("100"),
	4,			_T("200"),
	6,			_T("400"),
	7,			_T("800"),
	8,			_T("1600"),
	9,			_T("オートHI"),
	10,			_T("3200"),
	INT_MAX,	NULL,
};

// 画像設定
const lkup ekRicohSaturation[] = {
	0,			_T("濃い"),
	1,			_T("普通"),
	2,			_T("薄い"),
	3,			euBW,
	4,			_T("クロスプロセス"),
	6,			euBW,						// ToneEffectを持つ
	9,			euVivid,
	10,			_T("Natural"),
	14,			_T("ブリーチ バイパス"),
	//??,		_T("設定1"),
	//??,		_T("設定2"),
	//??,		_T("ハイコントラスト白黒"),
	//??,		_T("ポジフィルム調"),
	INT_MAX,	NULL,
};

const lkup ekRicohToneEffect[] = {
	0,			euNormal,
	1,			_T("レッド"),
	2,			_T("グリーン"),
	3,			_T("ブルー"),
	4,			_T("パープル"),
	6,			euSepia,
	INT_MAX,	NULL,
};

////////////////////////////////////////////////////////////////////////
// SIGMA

// 測光モード            (ex. DP1→評価測光, 中央部重点平均測光, スポット測光)
const lkup_text etSigmaMeteringMode[] = {
	"8",				_T("8分割式評価測光"),
	"C",				_T("中央部重点平均測光"),
	"A",				_T("中央部分測光"),
	"S",				_T("スポット測光"),			// ?
	NULL,				NULL,
};

// ドライブモード
const lkup_text etSigmaDriveMode[] = {
	"SINGLE",			euSingleShot,
	"MULTI",			euContinuosShot,
	"2S",				euSelfTimer2Sec,
	"10S",				euSelfTimer10Sec,
	"UP",				_T("ミラーアップ"),
	"AB",				euAutoBracket,
	"OFF",				euOff,
	NULL,				NULL,
};

// 撮影モード
const lkup_text etSigmaShootMode[] = {
	"P",				euProgram,
	"A",				euAperture,
	"S",				euShutter,
	"M",				euManual,
	NULL,				NULL,
};


////////////////////////////////////////////////////////////////////////
// MAMIYA

// 露出モード
const lkup ekMamiyamExposureMode[] = {
	0x02,		euManual,
	0x08,		euApertureAE,
	0x10,		euAuto,
	INT_MAX,	NULL,
};

// カラーモード
const lkup ekMamiyamColorMode[] = {
	0,			euStandard,
	1,			euPortrait,
	2,			euLandscape,
	3,			_T("商品"),
	INT_MAX,	NULL,
};

// ホワイトバランス
const lkup ekMamiyamWhiteBalance[] = {
	0,			euAuto,
	2,			euCloudy,
	3,			euSunlight,
	4,			euStrobe,
	6,			euBulblight,
//	52,
//	55,
	INT_MAX,	NULL,
};

// トーン
const lkup ekMamiyamTone[] = {
	1,			euNormal,
	2,			euLow,
	INT_MAX,	NULL,
};

////////////////////////////////////////////////////////////////////////
// LEICA

// ホワイトバランス
const lkup ekLeicaWhiteBalance[] = {
	0,			euAuto,
	2,			euFlourescent,
	3,			euWhiteBulblight,
	INT_MAX,	NULL,
};

// レンズ検出
const lkup ekLeicaLensType[] = {
	0x00,		euOff,
	0x61,		_T("21mm F2.8"),
	0x71,		_T("28mm F2.8"),
	0x83,		_T("40mm"),
	0x87,		_T("50mm F2.0"),
	INT_MAX,	NULL,
};




#include "datatable_nikon.h"
#include "datatable_canon.h"
#include "datatable_sony.h"
#include "datatable_olympus.h"




#endif			// __EXIF_UI_H__
