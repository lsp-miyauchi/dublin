///////////////////////////////////////////////////////////////////////
// makernote_panasonic.h panasonic情報

#pragma once
#ifndef __MAKERNOTE_PANASONIC_H__
#define __MAKERNOTE_PANASONIC_H__

///////////////////////////////////////////////////////////////////////
// PANASONICモデル

enum {
	PANA_UNKNOWN = 0,
	PANA_FX = 0x100,
	PANA_FX1,
	PANA_FX2,
	PANA_FX5,
	PANA_FX7,
	PANA_FX8,
	PANA_FX9,
	PANA_FX01,
	PANA_FX07,
	PANA_FX30,
	PANA_FX33,
	PANA_FX55,
	PANA_FX100,
	PANA_FZ = 0x200,
	PANA_FZ1,
	PANA_FZ2,
	PANA_FZ3,
	PANA_FZ5,
	PANA_FZ7,
	PANA_FZ8,
	PANA_FZ10,
	PANA_FZ20,
	PANA_FZ30,
	PANA_FZ18,
	PANA_FZ50,
	PANA_TZ = 0x300,
	PANA_TZ1,
	PANA_TZ3,
	PANA_LX = 0x400,
	PANA_LX1,
	PANA_LX2,
	PANA_LX3,
	PANA_LC = 0x500,
	PANA_LC1,
	PANA_LC5,
	PANA_LC20,
	PANA_LC33,
	PANA_LC43,
	PANA_LC70,
	PANA_LS = 0x600,
	PANA_LS2,
	PANA_LS75,
	PANA_LZ = 0x700,
	PANA_LZ2,
	PANA_LZ5,
	PANA_LZ7,
	PANA_L = 0x1000,
	PANA_L1,
	PANA_L10,
	PANA_G = 0x1100,
	PANA_G1,
};

///////////////////////////////////////////////////////////////////////
//

typedef struct __panasonic_t {
	int Quality;				// 0x0001 SHORT:画質モード
										// 2 = Fine
										// 6 = SuperFine
		// 0x0002 UNDEFx4:バージョン
	int WhiteBalance;			// 0x0003 USHRT ホワイトバランス
	int FocusMode;				// 0x0007 USHRT フォーカスモード
										// 1 => 'Auto',
										// 2 => 'Manual',
	int AFMode;					// 0x000F BYTEx2 AFモード
										// '01,00' => 'スポット'
										// '10,00' => '1点'
										// '10,10' => '1点高速'
										// '00,10' => '3点高速'
										// '01,01' => '5点'

	int MacroMode;				// 0x001C SHORT:マクロモード
										// 1 = on
										// 2 = off
	int AntiShake;				// 0x001A USHRT:手ブレ補正
										// 2 => 'On, Mode 1',
										// 3 => 'Off',
										// 4 => 'On, Mode 2',
	int ShootingMode;			// 0x001F SHORT:撮影モード
										//  1 = 通常撮影
										//  2 = ポートレート
										//  3 = 風景
										//  4 = 高速連写
										//  5 = プログラムAE
										//  6 = ?
										//  7 = 絞り優先AE
										//  8 = シャッター速度優先AE
										//  9 = マクロモード
										// 10 = ?
										// 11 = マニュアル
		// 0x0020 SHORT count=1
		// 0x0021 UNDEF count=780
		// 0x0022 SHORT count=1
	int WhiteBalanceBias;		// 0x0023 SHORT: ホワイトバランス調整
										// ±10ステップ（オート時以外）
	int FlashBias;				// 0x0024 SHORT: フラッシュ光量補正
	char InternalSerialNumber[24];	// 0x0025 UNDEFx16
		// 0x0026 UNDEF count=4
		// 0x0027 SHORT count=1
	int ColorEffect;			// 0x0028 USHRT
		// 0x0029 LONG  count=1
	int BurstMode;				// 0x002A USHRT: 連射モード
										// 0 => 'なし'
										// 1 => '連射'
										// 2 => 'オートブラケット'
										// 4 => '∞連射'
	int SequenceNumber;			// 0x002B ULONG: 連射シーケンス番号
										// 0 => 'なし'
										// 1 => '1枚目'
										// 2 => '2枚目'
										// 3 => '3枚目'
	int Contrast;				// 0x002C USHRT: コントラスト
										// 0 => 'Standard',
										// 1 => 'Low',
										// 2 => 'High',
										// 0x100 => 'Low',
										// 0x110 => 'Standard',
										// 0x120 => 'High',
	int NoiseReduction;			// 0x002D USHRT: ノイズリダクション
										// 0 => 'Standard',
										// 1 => 'Low',
										// 2 => 'High',
	int SelfTimer;				// 0x002E USHRT: セルフタイマー
										// 1 => 'Off',
										// 2 => '10s',
										// 3 => '2s',
		// 0x002F USHRT count=1
		// 0x0030 USHRT count=1
	int SceneMode;				// 0x0031 USHRT: シーンモード
										// 2 = ポートレート
										// ? = スポーツ
										// ? = 風景
										// 3 = 夜景
										// ? = 夜景ポートレート
										// ? = 流し撮り
										// ? = 花火
										// ? = パーティ
										// ? = 雪
	int ColorMode;				// 0x0032 USHRT: 画質調整
										// 0 = 標準
										// 1 = ナチュラル
										// 2 = ヴィヴィッド

	int exContrast;				// 0x0039
	int MyColor;				// 0x003d USHRT: マイカラー
										// 1  = 標準
										// 7  = ポップ
										// 8  = レトロ
										// 9  = ピュア
										// 10 = シック
										// 12 = モノクローム
										// 13 = ダイナミックアート
										// 14 = シルエット
	int exSaturation;			// 0x0040
	int exSharpness;			// 0x0041
	int FilmMode;				// 0x0042 USHRT: フィルムモード
										//  1 スタンダード
										//  2 ダイナミック
										//  3 ネイチャー
										//  4 スムーズ
										//  5 白黒・スタンダード
										//  6 白黒・ダイナミック
										//  7 白黒・スムーズ
										// 10 バイブラント
										// 11 ノスタルジック
	char LensName[48];			// 0x0051 ASCII 使用レンズ名
	char Accessory[48];			// 0x0053 ASCII アクセサリ

	int LightDetection;			// 0x005d USHRT 暗部補正

		// 0x4449 UNDEF count=512
} panasonic_t;



#endif		// __MAKERNOTE_PANASONIC_H__
