////////////////////////////////////////////////////////////////////////
// exifdata.h

#pragma once
#ifndef __EXIFDATA_H__
#define __EXIFDATA_H__

#include <float.h>
#include <limits.h>

///////////////////////////////////////////////////////////////////////
//

#ifndef BYTE_MAX
	#define BYTE_MAX				255
#endif
#define HasFlt(a)					((a) != FLT_MAX)
#define HasInt(a)					((a) != INT_MAX)
#define HasStr(a)					((a)[0] != '\0')
#define HasByte(a)					((a) != BYTE_MAX)
#define HasShrt(a)					((a) != SHRT_MAX)

// メーカー
enum {
	MAKER_UNKNOWN = 0,
	MAKER_CANON,
	MAKER_NIKON,
	MAKER_MINOLTA,
	MAKER_KODAK,
	MAKER_OLYMPUS,
	MAKER_SONY,
	MAKER_FUJI,
	MAKER_CASIO,
	MAKER_RICOH,
	MAKER_PANASONIC,
	MAKER_ROLLEI,
	MAKER_SANYO,
	MAKER_PENTAX,
	MAKER_SIGMA,
	MAKER_EPSON,
	MAKER_MAMIYA,
	MAKER_LEICA,
	MAKER_SAMSUNG,
	MAKER_KDDI_CA,
	MAKER_HASSEL,
};


///////////////////////////////////////////////////////////////////////
//

#include "exiftag.h"
#include "makernote_canon.h"
#include "makernote_nikon.h"
#include "makernote_olympus.h"
#include "makernote_casio.h"
#include "makernote_panasonic.h"
#include "makernote_fujifilm.h"
#include "makernote_minolta.h"
#include "makernote_sony.h"
#include "makernote_sanyo.h"
#include "makernote_pentax.h"
#include "makernote_sigma.h"
#include "makernote_epson.h"
#include "makernote_ricoh.h"
#include "makernote_mamiya.h"
#include "makernote_leica.h"
#include "makernote_hassel.h"
#include "makernote_samsung.h"

///////////////////////////////////////////////////////////////////////
// GPS情報

typedef struct __gps_t {
	bool exist_gps;						// GPS IFD の有無
	BYTE GPSVersionID[4];				// 0  GPSタグバージョン
	char GPSLatitudeRef[4];				// 1  緯度の北緯もしくは南緯。ASCII 値で'N'は北緯、'S'は南緯
	float GPSLatitude[3];				// 2  緯度（数値）度/分/秒
	char GPSLongitudeRef[4];			// 3  経度の東経もしくは西経。ASCII 値で'E'は東経、'W'は西経
	float GPSLongitude[3];				// 4  経度（数値）度/分/秒
	BYTE GPSAlititudeRef;				// 5  高度の単位、海抜0m = 0
	float GPSAlititude;					// 6  高度（m）
	float GPSTimeStamp[3];				// 7  hour/min/sec by UTC time
	char GPSSatellites[8];				// 8  測位に使った衛星信号
	char GPSMapDatum[32];				// 0x12 測位に用いた地図データ
	char GPSDateStamp[16];				// 0x1D GPS date
} gps_t;

// 度分秒から10進経緯度への変換式 = 度 + ((秒 / 60) + 分) / 60


///////////////////////////////////////////////////////////////////////
// exif情報

typedef struct __exif_t {
	int MakerID;						// メーカーID
	int ModelID;						// モデルID
	char Software[64];					// ソフトウェア
	char DateEdit[32];					// 編集日時
	char maker[64];						// 0x010F: メーカー名
	char model[64];						// 0x0110: モデル名
	int Orientation;					// 0x0112: SHORT 画像の向き
	int ISOEquiv;						// 0x8827: ISO感度
	int ExposureProgram;				// 0x8822: 露出制御プログラム
	char Date[32];						// 0x9003: 撮影日時
	char DateDigitized[32];				// 0x9004: デジタイズ日時
	int Flash;							// 0x9209: フラッシュ
	float ExposureTime;					// 0x829A: 露出時間 Second
	float FNumber;						// 0x829D: F値
	float ShutterSpeed;					// 0x9201: シャッター速度 APEX
	float Aperture;						// 0x9202: 絞り値
	float BrightnessValue;				// 0x9203: 対象物の明るさ
	float ExposureBias;					// 0x9204: 露光補正値 APEX
	float MaxAperture;					// 0x9205: レンズ開放F値 APEX
	float SubjectDistance;				// 0x9206: 被写体距離 meter
	int MeteringMode;					// 0x9207: 測光方式
	int LightSource;					// 0x9208: 光源
	float FocalLength;					// 0x920A: 焦点距離 mili-meter
	int ColorSpace;						// 0xA001: カラースペース
	int ImageWidth;						// 0xA002: イメージ幅
	int ImageHeight;					// 0xA003: イメージ高さ
	float FocalPlaneXResolution;		// 0xA20E: 焦点面の幅の解像度
	float FocalPlaneYResolution;		// 0xA20F: 焦点面の高さの解像度
	float FocalPlaneResolutionUnits;	// 0xA210: 焦点面解像度単位 mili-meter

	// tag value >= 0xA400
	int ExposureMode;					// 0xA402: 撮影シーンモード
	int WhiteBalance;					// 0xA403: ホワイトバランスモード
	float DigitalZoom;					// 0xA404: デジタルズーム
	float FocalLength35;				// 0xA405: 35mm換算の焦点距離	mili-meter
	int SceneCaptureType;				// 0xA406: 撮影シーンタイプ
	int GainControl;					// 0xA407: ゲインコントロール
	int Contrast;						// 0xA408: コントラスト
	int Saturation;						// 0xA409: 彩度
	int Sharpness;						// 0xA40A: シャープネス
	int DistanceRange;					// 0xA40C: 被写体の距離範囲

	float GammaValue;					// 0xA500: ガンマ値

	// メーカーノート
	bool exist_makenote;				// メーカーノートの有無
	union {
		canon_t canon;
		nikon_t nikon;
		olympus_t olympus;
		fuji_t fuji;
		minolta_t minolta;
		casio_t casio;
		panasonic_t panasonic;
		sanyo_t sanyo;
		pentax_t pentax;
		sigma_t sigma;
		epson_t epson;
		sony_t sony;
		ricoh_t ricoh;
		mamiya_t mamiya;
		leica_t leica;
		hassel_t hassel;
	};

	// GPS
	gps_t gps;

} exif_t;



////////////////////////////////////////////////////////////////////////
//


// 撮影モード
enum {
	SHT_AUTO = 1,					// オート
	SHT_MANUAL,						// マニュアル
	SHT_LANDSCAPE,					// 風景
	SHT_FASTSHUTTER,				// 高速シャッター
	SHT_SLOWSHUTTER,				// スローシャッター
	SHT_NIGHT,						// 夜景
	SHT_BW,							// モノクロ
	SHT_SEPIA,						// セピア
	SHT_PORTRAIT,					// ポートレート
	SHT_SPORTS,						// スポーツ
	SHT_CLOSEUP,					// クローズアップ
	SHT_PANFOCUS,					// パンフォーカス(A-DEP)
};

// 光源
enum {
	LS_AUTO = 0,					// オート
	LS_SUNLIGHT,					//  1:太陽光
	LS_FLUORESCENT,					//  2:蛍光灯
	LS_TUNGSTEN,					//  3:白熱電球
	LS_FLASH,						//  4:フラッシュ
	// 5 ～ 8 予約
	LS_SUNNY = 9,					//  9:晴天
	LS_CLOUDY,						// 10:曇天
	LS_SHADE,						// 11:日陰
	LS_FLUORESCENT_SUNLIGHT,		// 12:昼光色蛍光灯
	LS_FLUORESCENT_DAYLIGHT,		// 13:昼!白!色蛍光灯
	LS_FLUORESCENT_COOL,			// 14:冷!白!色蛍光灯
	LS_FLUORESCENT_WHITE,			// 15:白色蛍光灯
	// 16 予約
	LS_STDLIGHT_A = 17,				// 17:標準光 A
	LS_STDLIGHT_B,					// 18:標準光 B
	LS_STDLIGHT_C,					// 19:標準光 C
	LS_D55,							// 20:D55
	LS_D65,							// 21:D65
	LS_D75,							// 22:D75
	// 23～254 予約
	LS_OTHER = 255,					// その他
};

// ホワイトバランス（メーカーごと異なる）
enum {		// Canonの場合
	WB_CANON_AUTO = 0,					//  0: オート
	WB_CANON_SUNLIGHT,					//  1: 太陽光
	WB_CANON_CLOUDY,					//  2: くもり
	WB_CANON_TUNGSTEN,					//  3: 白熱電球
	WB_CANON_FLUORESCENT,				//  4: 蛍光灯
	WB_CANON_FLASH,						//  5: ストロボ
	WB_CANON_CUSTOM1,					//  6: カスタム1
	WB_CANON_CUSTOM2,					//  7: カスタム2
	WB_CANON_SHADE,						//  8: 日陰
	WB_CANON_FLUORESCENT_H = 14,		// 14: 蛍光灯H
};

// 測光方式
enum {
	MTM_UNKNOWN = 0,					// 不明
	MTM_AVERAGE,						// 1:平均測光
	MTM_CENTERWAIT,						// 2:中央重点測光
	MTM_SPOT,							// 3:スポット測光
	MTM_MULTISPOT,						// 4:マルチスポット測光
	MTM_PATTERN,						// 5:分割測光
	MTM_PARTIAL,						// 6:部分測光
	// 7～254 = 予約
	MTM_OTHER = 255,					// その他
};

// 露出プログラム
enum {
	EXPP_UNKNOWN = 0,					// 未定義
	EXPP_MANUAL,						//  1:マニュアル
	EXPP_NORMAL,						//  2:プログラムAE
	EXPP_AV,							//  3:露出優先AE
	EXPP_TV,							//  4:シャッター優先AE
	EXPP_CREATIVE,						//  5:Creativeプログラム（被写界深度方向にバイアス）
	EXPP_ACTION,						//  6:Actionプログラム（シャッタースピード高速側にバイアス）
	EXPP_PORTRAIT,						//  7:ポートレイトモード（クローズアップ撮影、背景はフォーカス外す）
	EXPP_LANDSCAPE,						//  8:ランズケープモード（landscape 撮影、背景はフォーカス合う）
	// 9～255
};

// シーン撮影タイプ TAG_SCENE_EXPTYPE
enum {
	SCT_NORMAL = 0,						// 0 = 標準
	SCT_LANDSCAPE,						// 1 = 風景
	SCT_PORTRAIT,						// 2 = ポートレート
	SCT_NIGHT,							// 3 = 夜景
};

// 被写体の距離範囲 TAG_SUBJECT_DISTANCERANGE
enum {
	SDR_UNKNOWN = 0,					// 0 = 不明
	SDR_MACRO,							// 1 = マクロ
	SDR_PORTRAIT,						// 2 = 近景
	SDR_LANDSCAPE,						// 3 = 遠景
};

// 圧縮種別 TAG_COMPRESSION
enum {
	COMPRESSION_NONE = 1,				// 非圧縮
	COMPRESSION_RLE = 2,				// RLE
	COMPRESSION_LZW = 5,				// LZW
	COMPRESSION_JPEG = 6,				// JPEG
	COMPRESSION_LJPEG = 7,				// Lossless JPEG
};





#endif		// __EXIFDATA_H__
