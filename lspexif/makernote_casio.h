///////////////////////////////////////////////////////////////////////
// makernote_casio.h casio情報

#pragma once
#ifndef __MAKERNOTE_CASIO_H__
#define __MAKERNOTE_CASIO_H__

///////////////////////////////////////////////////////////////////////
// CASIOモデル

enum {
	CASIO_UNKNOWN = 0,
	CASIO_EXCARD = 0x100,
	CASIO_EXS2,
	CASIO_EXM2,
	CASIO_EXS3,
	CASIO_EXS20,
	CASIO_EXS100,
	CASIO_EXS500,
	CASIO_EXS600,
	CASIO_EXS770,
	CASIO_EXS880,					// 2007-09
	CASIO_EXZOOM = 0x200,
	CASIO_EXZ3,
	CASIO_EXZ4,
	CASIO_EXZ30,
	CASIO_EXZ40,
	CASIO_EXZ50,
	CASIO_EXZ55,
	CASIO_EXZ57,
	CASIO_EXZ60,
	CASIO_EXZ77,					// 2007-09
	CASIO_EXZ110,
	CASIO_EXZ500,
	CASIO_EXZ600,
	CASIO_EXZ700,
	CASIO_EXZ750,
	CASIO_EXZ850,
	CASIO_EXZ1000,
	CASIO_EXZ1050,
	CASIO_EXZ1080,					// 2007-10
	CASIO_EXZ1200,
	CASIO_EXPRO = 0x300,
	CASIO_EXP505,
	CASIO_EXP600,
	CASIO_EXP700,
	CASIO_EXF1,
	CASIO_EXHiZOOM = 0x400,
	CASIO_EXV7,
	CASIO_EXV8,						// 2007-09
};

///////////////////////////////////////////////////////////////////////
//

typedef struct __casio_t {
	int WhiteBalanceBias[2];	// 0x2011 SSHRTx2:ホワイトバランス補正
	int WhiteBalance;			// 0x2012 SHORT:ホワイトバランス
										// 0 = 太陽光
										// 1 = オート
										// 3 = 日陰
										// 5 = 蛍光灯
										// 9 = 電球
										// 12 = フラッシュ
										// 13 = マニュアル
										// 15 = オート
	int SubjectDistance;		// 0x2022 LONG:被写体までの距離(mili-meter)
	int FlashDistance;			// 0x2034 SHORT:フラッシュ距離
	int ShootMode;				// 0x3000 SHORT:撮影モード
										// 2 = 通常撮影
										// 6 = ベストショット
	int SelfTimer;				// ??? 0x3001 SHORT:セルフタイマー
	int ShutterMode;			// 0x3001 SHORT:シャッターモード
	int Quality;				// 0x3002 SHORT:画質モード
										// 1 = エコノミー
										// 2 = 標準
										// 3 = 高精細
										// 4 = 超高精細
	int FocusMode;				// 0x3003 SHORT:フォーカスモード
										// 1 = 固定
										// 2 = マクロ
										// 3 = オートフォーカス
										// 4 = マニュアルフォーカス
										// 5 = パンフォーカス（無限遠）
										// 6 = マルチエリアAF
										// 7 = スポットAF
	char TimeZone[32];			// 0x3006 ASCII:タイムゾーン
	int BestShotNumber;			// 0x3007 SHORT:ベストショット番号
	int Sharpness;				// 0x3011 UNDEFx2 シャープネス
	int Saturation;				// 0x3012 UNDEFx2 彩度
	int Contrast;				// 0x3013 UNDEFx2 コントラスト（の倍数）
	int ISO;					// 0x3014 SHORT:ISO感度
	int ColorMode;				// 0x3015 SHORT:カラーモード
										// 0 = カラー
										// 2 = モノクロ
										// 3 = セピア
	int ColorEnhance;			// 0x3016 SHORT:色強調
										// 0 = なし
										// 1 = 青
										// 3 = 緑
										// 4 = 黄色
										// 5 = 赤
										// 6 = 紫
										// 7 = ピンク
										// 9 = 肌色
	int ColorFilter;			// 0x3017 SHORT:カラーフィルター
										// 0 = なし
										// 1 = 青
										// 3 = 緑
										// 4 = 黄色
										// 5 = 赤
										// 6 = 紫
										// 7 = ピンク
										// 9 = 肌色
} casio_t;



#endif		// __MAKERNOTE_CASIO_H__
