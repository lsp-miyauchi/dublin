///////////////////////////////////////////////////////////////////////
// makernote_canon.h canon情報

#pragma once
#ifndef __MAKERNOTE_CANON_H__
#define __MAKERNOTE_CANON_H__

///////////////////////////////////////////////////////////////////////
// CANONモデル

enum {
	CANON_UNKNOWN = 0,
	CANON_IXY = 0x100,				// IXY
	CANON_IXY_UNKNOWN,
	CANON_IXY_30,						//
	CANON_IXY_30a,
	CANON_IXY_40,
	CANON_IXY_50,
	CANON_IXY_55,
	CANON_IXY_60,
	CANON_IXY_70,
	CANON_IXY_80,
	CANON_IXY_200,
	CANON_IXY_200a,
	CANON_IXY_300,
	CANON_IXY_300a,
	CANON_IXY_320,
	CANON_IXY_400,
	CANON_IXY_450,
	CANON_IXY_500,
	CANON_IXY_600,
	CANON_IXY_700,
	CANON_IXY_800IS,
	CANON_IXY_900IS,						// DIGIC III "Canon DIGITAL IXUS 850 IS"
	CANON_IXY_910IS,						// DIGIC III
	CANON_IXY_1000,							// DIGIC III
	CANON_IXY_2000IS,						// DIGIC III
	CANON_IXY_10,							// DIGIC III "Canon PowerShot SD1000"
	CANON_IXY_90,							// DIGIC III "Canon PowerShot SD750"
	CANON_IXY_L,
	CANON_IXY_L2,
	CANON_IXY_L3,						// "Canon PowerShot SD30"
	CANON_IXY_L4,							// DIGIC III
	CANON_POWERSHOT = 0x200,			// PowerShot Series
	CANON_POWERSHOT_UNKNOWN,
	CANON_POWERSHOT_A430,
	CANON_POWERSHOT_A530,
	CANON_POWERSHOT_A540,
	CANON_POWERSHOT_A560,
	CANON_POWERSHOT_A610,
	CANON_POWERSHOT_A620,
	CANON_POWERSHOT_A630,
	CANON_POWERSHOT_A640,
	CANON_POWERSHOT_A650IS,
	CANON_POWERSHOT_A700,
	CANON_POWERSHOT_A710IS,
	CANON_POWERSHOT_A720IS,
	CANON_POWERSHOT_PRO1,				// "Canon PowerShot Pro1"
	CANON_POWERSHOT_S80,				// "Canon PowerShot S80"
	CANON_POWERSHOT_S70,				// "Canon PowerShot S70"
	CANON_POWERSHOT_S60,				// "Canon PowerShot S60"
	CANON_POWERSHOT_S50,				// "Canon PowerShot S50"
	CANON_POWERSHOT_S40,				// "Canon PowerShot S40"
	CANON_POWERSHOT_S30,				// "Canon PowerShot S30"
	CANON_POWERSHOT_S20,				// "Canon PowerShot S20"
	CANON_POWERSHOT_S1IS,
	CANON_POWERSHOT_S2IS,
	CANON_POWERSHOT_S3IS,
	CANON_POWERSHOT_SX100IS,			// "Canon PowerShot SX100 IS" DIGIC III
	CANON_POWERSHOT_G7,					// "Canon PowerShot G7" DIGIC III
	CANON_POWERSHOT_G6,					// "Canon PowerShot G6"
	CANON_POWERSHOT_G5,					// "Canon PowerShot G5"
	CANON_POWERSHOT_G3,
	CANON_POWERSHOT_G2,
	CANON_POWERSHOT_G9,					// "Canon PowerShot G9" DIGIC III RAW Support
	CANON_POWERSHOT_TX1,				// "Canon PowerShot TX1" DIGIC III
	CANON_EOS = 0x1000,				// EOS Series
	CANON_EOS_UNKNOWN,
	CANON_EOS_10D,						// "Canon EOS 10D"
	CANON_EOS_KISS,						// "Canon EOS Kiss Digital"
	CANON_EOS_KISSN,					// "Canon EOS Kiss Digital N"
	CANON_EOS_1D,						// "Canon EOS-1D"
	CANON_EOS_20D,						// "Canon EOS 20D"
		CANON_HAS_SHUTTERCOUNT,
	CANON_EOS_1DS,						// "Canon EOS-1DS"
	CANON_EOS_1DMK2,					// "Canon EOS-1D Mark II"
	CANON_EOS_1DSMK2,					// "Canon EOS-1Ds Mark II"
		CANON_HAS_PICTURE_STYLE,
	CANON_EOS_KISSX,					// "Canon EOS Kiss Digital X / 400D"
	CANON_EOS_KISSX2,					// "Canon EOS Kiss Digital X2"
	CANON_EOS_KISSX3,					// "Canon EOS Kiss Digital X3/REBEL T1i/500D"
	CANON_EOS_KISSF,					// "Canon EOS Kiss Digital F / 1000D"
	CANON_EOS_KISSX4,					// "Canon EOS Kiss X4"
	CANON_EOS_KISSX50,					// "Canon EOS Kiss X50"
	CANON_EOS_KISSX5,					// "Canon EOS Kiss X5"
	CANON_EOS_KISSX6I,					// "Canon EOS Kiss X6i"
	CANON_EOS_KISSX6,					// "Canon EOS Kiss X6"
	CANON_EOS_KISSX7I,					// "Canon EOS Kiss X7i"
	CANON_EOS_KISSX7,					// "Canon EOS Kiss X7"
	CANON_EOS_8000D,					// "Canon EOS 8000D"
	CANON_EOS_M,						// "Canon EOS M"
	CANON_EOS_M2,						// "Canon EOS M2"
	CANON_EOS_M3,						// "Canon EOS M3"
	CANON_EOS_30D,						// "Canon EOS 30D"
	CANON_EOS_40D,						// "Canon EOS 40D"
	CANON_EOS_50D,						// "Canon EOS 50D"
	CANON_EOS_60D,						// "Canon EOS 60D"
	CANON_EOS_70D,						// "Canon EOS 70D"
	CANON_EOS_80D,						// "Canon EOS 80D"
	CANON_EOS_5D,						// "Canon EOS 5D"
	CANON_EOS_5DMK2,					// "Canon EOS 5D Mark II"
	CANON_EOS_5DMK3,					// "Canon EOS 5D Mark III"
	CANON_EOS_1DMK2N,					// "Canon EOS-1D Mark II N"
	CANON_EOS_1DMK3,					// "Canon EOS-1D Mark III"
	CANON_EOS_1DMK4,					// "Canon EOS-1D Mark IV"
	CANON_EOS_1DSMK3,					// "Canon EOS-1Ds Mark III"
	CANON_EOS_1DX,						// "Canon EOS-1D X"
	CANON_EOS_3D,						// "Canon EOS 3D"
	CANON_EOS_7D,						// "Canon EOS 7D"
	CANON_EOS_7DMK2,					// "Canon EOS 7D Mark II"
	CANON_EOS_6D,						// "Canon EOS 6D"
};

#define IS_CANON_EOS(n)				((n)>CANON_EOS)
#define IS_CANON_SHUTTERCOUNT(n)	((n)>CANON_HAS_SHUTTERCOUNT)
#define IS_CANON_PICTURESTYLE(n)	((n)>CANON_HAS_PICTURE_STYLE||(n)==CANON_EOS_KISSX)

////////////////////////////////////////////////////////////////////////
//

typedef struct __canon_t {
									// TAG    ##
	int MacroMode;					// 0x0001- 1:マクロモード
										// 0 = 機能なし
										// 1 = on
										// 2 = off
	int SelfTimer;					// 0x0001- 2:セルフタイマ
										// 0  = off
										// 20 = 2秒
										// 100 = 10秒
	int Quality;					// 0x0001- 3:クォリティ
										// Unknown
										// 1 = エコノミー
										// 2 = ノーマル
										// 3 = ファイン
										// 4 = RAW
										// 5 = スーパーファイン
	int FlashMode;					// 0x0001- 4:フラッシュモード
										//  0 = 禁止
										//  1 = 自動
										//  2 = 常時
										//  3 = 赤目緩和
										//  4 = スローシンクロ
										//  5 = 赤目緩和自動
										//  6 = 赤目緩和常時
										// 16 = 外部ストロボ
	int DriveMode;					// 0x0001- 5:ドライブモード
										// 0 = 1コマ撮影
										// 1 = 連続撮影
										// 2 = セルフタイマー撮影
	int FocusMode;					// 0x0001- 7:フォーカスモード
										// 0 = ワンショット
										// 1 = AIサーボ
										// 2 = AIフォーカス
										// 3 = マニュアルフォーカス
										// 4 = シングルフォーカス --> 0x0000-32参照
										// 5 = 連続フォーカス
										// 6 = マニュアルフォーカス
	int SizeType;					// 0x0001-10:イメージサイズ
										// 0 = Large
										// 1 = Medium
										// 2 = Small
										// 5 = Medium1
										// 6 = Medium2
	int ShootMode;					// 0x0001-11:イメージセレクトモード
										// 	 0 = フルオート
										//	 1 = マニュアル
										//	 2 = 風景
										//	 3 = 高速シャッター
										//	 4 = スローシャッター
										//	 5 = 夜景
										//	 6 = モノクロ
										//	 7 = セピア
										//	 8 = ポートレート
										//	 9 = スポーツ
										//	10 = マクロ・クローズアップ
										//	11 = パンフォーカス
	int DigitalZoom;				// 0x0001-12:デジタルズーム
										//	 0 = オフ
										//	 1 = 2倍
										//	 2 = 4倍
										//	 3 = オン
	int AdjustContrast;				// 0x0001-13:コントラスト調整
										//	 0 = 標準
										//	 ±1 = ±1
										//	 ±n = ±n
	int AdjustSaturation;			// 0x0001-14:彩度調整
										//	 0 = 標準
										//	 ±1 = ±1
										//	 ±n = ±n
	int AdjustSharpness;			// 0x0001-15:シャープネス調整
										//	 0 = 標準
										//	 ±1 = ±1
										//	 ±n = ±n
	int ISO;						// 0x0001-16:ISO感度
										//	  0 = 機能なし
										//	 15 = オート
										//	 16 = ISO50
										//	 17 = ISO100
										//	 18 = ISO200
										//	 19 = ISO400
	int MeteringMode;				// 0x0001-17:測光方式
										//	 0 = 中央重点測光
										//	 1 = スポット測光
										//	 2 = 平均測光
										//	 3 = 評価測光
										//	 4 = 中央部部分測光
										//	 5 = 中央部重点平均測光
	int FocusRange;					// 0x0001-18:フォーカスレンジ
										//	 0 => 'Manual',
										//	 1 => 'Auto',
										//	 2 => 'Not Known',
										//	 3 => 'Macro',
										//	 4 => 'Very Close', #PH
										//	 5 => 'Close', #PH
										//	 6 => 'Middle Range', #PH
										//	 7 => 'Far Range',
										//	 8 => 'Pan Focus',
										//	 9 => 'Super Macro', #PH
										//	10 => 'Infinity', #PH
	int AFPoint;					// 0x0001-19:AFポイント
										// 0x2005: 'Manual AF point selection'
										// 0x3000: 無し(MF)
										// 0x3001: 自動選択
										// 0x3002: 右
										// 0x3003: 中央
										// 0x3004: 左
										// 0x4001: 'Auto AF point selection'
	int ExposureProgram;			// 0x0001-20:露出制御プログラム
										// 0 = イメージセレクト
										// 1 = プログラムAE
										// 2 = シャッター速度優先AE
										// 3 = 絞り優先AE
										// 4 = マニュアル設定
										// 5 = 自動深度優先(A-DEP)
										// 6 = 深度優先(DEP)
	int MaxFocalLength;				// 0x0001-23:レンズ最長焦点距離
										// レンズ最長焦点距離(mm) = MaxFocalLength * FocalLengthUnit
	int MinFocalLength;				// 0x0001-24:レンズ最短焦点距離
										// レンズ最長焦点距離(mm) = MinFocalLength * FocalLengthUnit
	int FocalLengthUnit;			// 0x0001-25:レンズ焦点距離単位(mili-meter)
	float MaxAperture;				// 0x0001-26:最小(開放)F値(APEX?)
	float MinAperture;				// 0x0001-27:最大F値(APEX?)
	int FlashCondition;				// 0x0001-28:フラッシュ状態
										// 0 = オフ
										// 1 = 発光
	int FocusContinuous;			// 0x0001-32:フォーカス動作
										// 注：G1の場合はこちらがオフセット7の値より優先されるようだ
										// 0 = シングルAF
										// 1 = 連続AF
	int AESetting;					// 0x0001-33:AE設定
										// 0 => 'Normal AE',
										// 1 => 'Exposure Compensation',
										// 2 => 'AE Lock',
										// 3 => 'AE Lock + Exposure Comp.',
										// 4 => 'No AE',
	int ImageStabilization;			// 0x0001-34:テブレ補正
										// 0 = off
										// 1 = on
	int ColorEffect;				// 0x0001-40:色効果
										// 0=効果なし
										// 1=くっきりカラー
										// 2=すっきりカラー
										// 3=ソフト
										// 4=セピア
										// 5=白黒
										// 6=カスタム
	int AdjustColorTone;			// 0x0001-42:色あい調整

	// TAG 0x0002
	int FocalType;					// 0x0002- 0:焦点タイプ
										// 0: unknown
										// 1: Fixed
										// 2: Zoom
	int FocalLength;				// 0x0002- 2:焦点距離
	int FocalPlaneXSize;			// 0x0002- 3:
	int FocalPlaneYSize;			// 0x0002- 4:

	// TAG 0x0004
	int ISOapex;					// 0x0004- 2: ISO(APEX?)
	float Aperture;					// 0x0004- 4: 露出時間
	float ExposureTime;				// 0x0004- 5: シャッター速度(APEX?)
	int ExposureComp;				// 0x0004- 6:露出補正
										//	 ±12 = ±0.33EV
										//	 ±16 = ±0.50EV
										//	 ±20 = ±0.67EV
										//	 ±32 = ±1.00EV
										//	 ±44 = ±1.33EV
										//	 ±48 = ±1.50EV
										//	 ±52 = ±1.67EV
										//	 ±64 = ±2.00EV
	int WhiteBalance;				// 0x0004- 7:ホワイトバランス
										//	  0 = オート
										//	  1 = 太陽光
										//	  2 = 曇り
										//	  3 = 白熱灯
										//	  4 = 蛍光灯
										//	  5 = フラッシュ
										//	  6 = カスタム
										//	  7 = モノクロ
										//	  8 = 日陰
										//	  9 = 色温度直接設定
										//	 10 = PC Set1
										//	 11 = PC Set2
										//	 12 = PC Set3
										//	 14 = 蛍光灯 H
										//	 15 = カスタム1
										//	 16 = カスタム2
										//	 17 = 水中
	int SlowShutter;				// 0x0004- 8:スローシャッター
										// 0 => 'Off',
										// 1 => 'Night Scene',
										// 2 => 'On',
										// 3 => 'None',
	int ContinuousNumber;			// 0x0004- 9:連続撮影時の連番(開始番号 n, ...)
	int FlashGuideNumber;			// 0x0004-13:フラッシュガイドナンバー
	int AiAF;						// 0x0004-14:合焦AFフレーム(AiAF)
										// Bits 15..0:
										// 15-12: 使用可能な測距点数
										// 2: 左
										// 1: 中央
										// 0: 右
										// ワンショットAF時しか使われていない？
										// 使われていない場合は、AF合焦失敗またはMF
	int FlashComp;					// 0x0004-15:フラッシュ発光強度補正
										//	 ±12 = ±0.33EV
										//	 ±16 = ±0.50EV
										//	 ±20 = ±0.67EV
										//	 ±32 = ±1.00EV
										//	 ±44 = ±1.33EV
										//	 ±48 = ±1.50EV
										//	 ±52 = ±1.67EV
										//	 ±64 = ±2.00EV
	int AEBSeq;						// 0x0004-16:AEBシーケンス
										// -1 => 'On',
										// 0 => 'Off',
										// 1 => 'On (shot 1)',
										// 2 => 'On (shot 2)',
										// 3 => 'On (shot 3)',
	int AEBComp;					// 0x0004-17:AEB補正
										//	 ±12 = ±0.33EV
										//	 ±16 = ±0.50EV
										//	 ±20 = ±0.67EV
										//	 ±32 = ±1.00EV
										//	 ±44 = ±1.33EV
										//	 ±48 = ±1.50EV
										//	 ±52 = ±1.67EV
										//	 ±64 = ±2.00EV
	int TargetLongestDistance;		// 0x0004-19:被写体最長距離 (==65535 then ∞)
	int TargetShortestDistance;		// 0x0004-20:被写体最短距離 (==0 then no data)
	int BulbDuration;				// 0x0004-24:バルブ撮影時の露出時間(sec x 10)
	int CameraType;					// 0x0004-26:カメラタイプ
										// 248 => 'EOS High-end',
										// 250 => 'Compact',
										// 252 => 'EOS Mid-range',
										// 255 => 'DV Camera', #PH
	int AutoRotate;					// 0x0004-27 自動回転
										// -1: ソフトウェア
										//  0: なし
										//  1: 左90°
										//  2: 左180°
										//  3: 左270°
	int NDFilter;					// 0x0004-28 NDフィルター
										// 0: オフ
										// 1: オン
	int SelfTimer2;					// 0x0004-29:セルフタイマ2

	// TAG 0x0005 パノラマ情報
	int PaPanoramaFrame;			// 0x0005 - 2 パノラマフレーム
	int PaPanoramaDirection;		// 0x0005 - 5 パノラマ方向

	char ImageType[40];				// 0x0006 ASCII イメージの種類
	char FirmWare[40];				// 0x0007 ASCII ファームウェア
	int ShotNumber;					// 0x0008 LONG 連続撮影時の連番
	char OwnerName[40];				// 0x0009 ASCII 所有者名
	int SerialNumber;				// 0x000C LONG シリアル番号

	// TAG 0x000D undef構造体
	int CiExposureTime;				// 0x000D - 0x04
	int CiHighlightTonePriority;	// 0x000D - 0x07
	int CiFocalLength;				// 0x000D - 0x0A
	int CiLensTypes;				// 0x000D - 0x0D
	int CiShortFocal;				// 0x000D - 0x12
	int CiLongFocal;				// 0x000D - 0x14
	int CiFocalType;				// 0x000D - 0x2D
	int CiHighISONoiseReduction;	// 0x000D - 
										// 0 => 'Standard',
										// 1 => 'Low',
										// 2 => 'Strong',
										// 3 => 'Off',
	int CiAutoLightingOptimizer;	// 0x000D - 
										// 0 => 'Standard',
										// 1 => 'Low',
										// 2 => 'Strong',
										//3 => 'Off',

	// TAG 0x0012
	int ImageWidth;					// 0x0012 - 2 or CIF 0x1810 LONGx2 イメージサイズ
	int ImageHeight;				// 0x0012 - 3 or CIF 0x1810 LONGx2 イメージサイズ

	// TAG 0x001D USHORT構造体
	int MyColor;					// 0x001D - 2: マイカラー
	int RedLevel;					// 0x001D - 3: 赤レベル
	int GreenLevel;					// 0x001D - 4: 緑レベル
	int BlueLevel;					// 0x001D - 5: 青レベル
	int SkinLevel;					// 0x001D - 6: 肌色レベル
	int ColorRange;					// 0x001D - 7: 色の範囲（-5～+5）ワンポイントカラー または スイッチカラー時

	// TAG 0x001F USHORT構造体
	int RetouchMyColor;				// 0x001F - 13: レタッチマイカラー

	// TAG 0x0093 ファイル情報 SHORT構造体
	int FiShutterCount;				// 0x0093 -  1 & 2 シャッター回数
	int FiBracketMode;				// 0x0093 -  3 ブラケットモード
										// 0 => 'Off',
										// 1 => 'AEB',
										// 2 => 'FEB',
										// 3 => 'ISO',
										// 4 => 'WB',
	int FiBracketValue;				// 0x0093 -  4 ブラケット値
	int FiBracketShotNumber;		// 0x0093 -  5 ブラケット撮影番号
	int FiNoiseReduction;			// 0x0093 -  8 ノイズリダクション
	int FiWBBracketMode;			// 0x0093 -  9 WBブラケティングモード
	int FiWBBracketBlueAmber;		// 0x0093 - 12 WBブラケティング ブルー/アンバー マイナス値でBlue
	int FiWBBracketMagentaGreen;	// 0x0093 - 13 WBブラケティング マゼンタ/グリーン マイナス値でMagenta
	int FiBWFilterEffect;			// 0x0093 - 14 フィルター効果
										// 0: なし
										// 1: 黄
										// 2: オレンジ
										// 3: 赤
										// 4: 緑
	int FiBWColorFilter;			// 0x0093 - 15 調色
										// 0: なし
										// 1: セピア
										// 2: 青
										// 3: 紫
										// 4: 緑

	char LensName[64];				// 0x0095 ASCII 使用レンズ名

	// TAG 0x00A0 SHORT構造体
	int ToneCurve;					// 0x00A0 -  1
										// 0 => 'Standard',
										// 1 => 'Manual',
										// 2 => 'Custom',
	int SharpnessPower;				// 0x00A0 -  2 シャープネスの強さ (0 - 5)
	int SharpnessFrequency;			// 0x00A0 -  3 シャープネスの対象
										// 0: なし
										// 1: 粗い
										// 2: やや粗い
										// 3: 標準
										// 4: やや細かい
										// 5: 細かい
	int ColorTemp;					// 0x00A0 -  9 指定色温度(数値 + K)
	int ColorMatrix;				// 0x00A0 - 10 カラーマトリックス
										// 1: 標準, マトリックス1(sRGB)
										// 2: ポートレート, マトリックス2(sRGB)
										// 3: 高彩度, マトリックス3(sRGB)
										// 4: AdobeRGB, マトリックス4(AdobeRGB)
										// 5: 低彩度, マトリックス5(sRGB)
										// 6: CMセット1
										// 7: CMセット2
	int DigitalGain;				// 0x00A0 - 11 DigitalGain --- val / 10.0
	int WBBlueAmber;				// 0x00A0 - 12 WB補正 ブルー/アンバー マイナス値でBlue
	int WBMagentaGreen;				// 0x00A0 - 13 WB補正 マゼンタ/グリーン マイナス値でMagenta
	int DevParam;					// 0x00B4 SHORT 現像パラメタ
										// 1: 標準
										// 2: AdobeRGB
										// 3: セピア
	int RawWidth;					// 0x1031 SHORTx2 イメージサイズ
	int RawHeight;
	int AFPointUsed;
} canon_t;


#endif		// __MAKERNOTE_CANON_H__
