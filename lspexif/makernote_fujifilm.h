///////////////////////////////////////////////////////////////////////
// makernote_fujifilm.h fujifilm情報

#pragma once
#ifndef __MAKERNOTE_FUJIFILM_H__
#define __MAKERNOTE_FUJIFILM_H__

///////////////////////////////////////////////////////////////////////
// FUJIFILMモデル

enum {
	FUJIFILM_UNKNOWN = 0,
	FUJIFILM_F = 0x100,
	FUJIFILM_F200EXR,
	FUJIFILM_A = 0x200,
	FUJIFILM_E = 0x300,
	FUJIFILM_Z = 0x400,
	FUJIFILM_S = 0x1000,
	FUJIFILM_S5000,
	FUJIFILM_S5200,
	FUJIFILM_S7000,
	FUJIFILM_S9000,
	FUJIFILM_S6000FD,
	FUJIFILM_S9100,
	FUJIFILM_S100FS,
	FUJIFILM_PRO = 0x2000,
	FUJIFILM_S2PRO,
	FUJIFILM_S3PRO,
	FUJIFILM_S5PRO,
};

#define IS_FUJIFILM_PRO(n)			((n)>FUJIFILM_PRO)

///////////////////////////////////////////////////////////////////////
//

typedef struct __fuji_t {
	char Version[16];				// 0x0000 UNDEF:バージョン
	char Quality[32];				// 0x1000 ASCII:画像クオィティ
	int Sharpness;					// 0x1001 SHORT:シャープネス
										// 1,2 = Soft
										// 3 = Normal
										// 4,5 = Hard
	int WhiteBalance;				// 0x1002 SHORT:ホワイトバランス
										// 0x0000 = オート
										// 0x0100 = 昼光
										// 0x0200 = 曇り
										// 0x0300 = 昼光色蛍光灯
										// 0x0301 = 昼白色蛍光灯
										// 0x0302 = 白色蛍光灯
										// 0x0400 = 白熱電球
										// 0x0F00 = カスタムホワイトバランス
	int Color;						// 0x1003 SHORT:カラー（彩度）の濃さ
										// 0x0000 = normal(STD)
										// 0x0100 = High
										// 0x0200 = Low(ORG)
	int Tone;						// 0x1004 SHORT:トーン（コントラスト）
										// 0x0000 = normal(STD)
										// 0x0100 = High(HARD)
										// 0x0200 = Low(ORG)
	int ColorTemperature;			// 0x1005 色温度
	int Contrast;					// 0x1006 コントラスト
										// 0x0   => 'Normal',
										// 0x100 => 'High',
										// 0x300 => 'Low',
	int WBFineTune[2];				// 0x100A SLONGx2: ホワイトバランス調整
	int NoiseReduction;				// 0x100B USHRT: ノイズリダクション
										// 0x40 => 'Low'
										// 0x80 => 'Normal'
	int FlashMode;					// 0x1010 SHORT:フラッシュ発光モード
										// 0 = 自動発光
										// 1 = 強制発光
										// 2 = 発光禁止
										// 3 = 赤目防止
	float FlashStrength;			// 0x1011 SRATIN:フラッシュの発光強度補正値
										// -6/10, -3/10, 0/10, 3/10, 6/10 等
	int MacroMode;					// 0x1020 SHORT:マクロモード
										// 0 = Off,  1 = On
	int FocusMode;					// 0x1021 SHORT:フォーカスモード
										// 0 = オートフォーカス  1 = マニュアルフォーカス
	int CenterFixedAF;				// 0x1022 SHORT: センター固定AF
	int FocusPointX;				// 0x1023 SHORTx2:フォーカスポイント
	int FocusPointY;


	int SlowSync;					// 0x1030 SHORT:スローシンクロ
										// 0 = Off,  1 = On
	int PictureMode;				// 0x1031 SHORT:撮影モード
										// 0x0000 = オート
										// 0x0001 = ポートレート
										// 0x0002 = 風景
										// 0x0004 = スポーツ
										// 0x0005 = 夜景
										// 0x0006 = プログラムAE
										// 0x0100 = 絞り優先AE
										// 0x0200 = シャッター速度優先AE
										// 0x0300 = マニュアル
	int ContTake;					// 0x1100 SHORT:連写・自動ブラケットモード
										// 0 = off,  1 = on
	int ContTakeStart;				// 0x1101 SHORT:連写連番開始番号
	int FinePixColor;				// 0x1210 SHORT:FinePixカラー
										// 0x00 = F-スタンダード
										// 0x10 = F-クローム
										// 0x30 = F-モノクロ
	int BlurWarning;				// 0x1300 SHORT:手振れ警告
										// 0 = No blur warning,  1 = Blur warning
	int FocusWarning;				// 0x1301 SHORT:フォーカス警告
										// 0 = Auto Focus good   1 = Out of focus
	int AEWarning;					// 0x1302 SHORT:AE警告
										// 0 = AE good,   1 = Over exposure (>1/1000s, F11)
	int DynamicRange;				// 0x1400 SHORT:ダイナミックレンジ?
	int FilmSimulationMode;			// 0x1401 SHORT:フィルムシミュレーション
	int DynamicRangeSetting;		// 0x1402 SHORT:ダイナミックレンジモード
										// 0x0000 Auto(100～400%)
										// 0x0001 RAW
										// 0x0100 Standard(100%)
										// 0x0200 Wide1(230%)
										// 0x0201 Wide2(400%)
										// 0x8000 Film Simulation Mode
	int DynamicRangeVal;			// 0x1403 SHORT:ダイナミックレンジ量?
	int MinFocalLength;				// 0x1404:レンズ最短焦点距離
	int MaxFocalLength;				// 0x1405:レンズ最長焦点距離
	float MaxApertureAtMinFocal;	// 0x1406:最短焦点距離での開放F値
	float MaxApertureAtMaxFocal;	// 0x1407:最長焦点距離での開放F値
	int FDPoint[4];					// 0x4005: USHRTx4 顔検出位置
	int FaceDetection;				// 0x4100: USHRT 顔キレイナビ?
} fuji_t;


#endif		// __MAKERNOTE_FUJIFILM_H__
