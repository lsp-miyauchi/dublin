///////////////////////////////////////////////////////////////////////
// makernote_pentax.h pentax情報

#pragma once
#ifndef __MAKERNOTE_PENTAX_H__
#define __MAKERNOTE_PENTAX_H__


///////////////////////////////////////////////////////////////////////
// PENTAXモデル

enum {
	PENTAX_UNKNOWN = 0,
	PENTAX_OPTIO = 0x1000,
	PENTAX_IST = 0x2000,
	PENTAX_ISTD,
	PENTAX_ISTDL,
	PENTAX_ISTDL2,
	PENTAX_ISTDS,
	PENTAX_ISTDS2,
	PENTAX_K = 0x3000,
	PENTAX_K100D,
	PENTAX_K110D,
	PENTAX_K10D,
	PENTAX_K20D,
	PENTAX_K200D,
	PENTAX_KM,
};

#define IS_PENTAX_CUSTOMIMAGE(n)	((n)>=PENTAX_K20D)

///////////////////////////////////////////////////////////////////////
//

typedef struct __pentax_t {
								// 0x0001 SHORT
								// 0x0002 SHORTx2 サムネイルの画像サイズ X x Y
								// 0x0003 LONG サムネイルデータサイズ
								// 0x0004 LONG サムネイルデータ位置
								// 0x0005 LONG
	int ShootingDate;			// 0x0006 UNDEFx4 撮影日付 0xAABBCCDD AABB=year CC=mon DD=day
	int ShootingTime;			// 0x0007 UNDEFx3 撮影時刻 0xAABBCC AA=hour BB=min CC=sec
	int Quality;				// 0x0008 SHORT 画質モード
										// 0 = エコノミー
										// 1 = ファイン
										// 2 = S.ファイン
										// 3 = TIFF
										// 4 = CCDRAW
	int ImageSize;				// 0x0009 SHORTx2 RAW画像サイズ？
										// 0 => '640x480',
										// 1 => 'Full', #PH - this can mean 2048x1536 or 2240x1680 or ... ?
										// 2 => '1024x768',
										// 3 => '1280x960', #PH (Optio WP)
										// 4 => '1600x1200',
										// 5 => '2048x1536',
										// 8 => '2560x1920', #PH (Optio WP)
										// 19 => '320x240', #PH (Optio WP)
										// 21 => '2592x1944',
										// 22 => '2304x1728', #2
										// 36 => '3008x2008',  #PH
										// 37 => '3008x2000'
								// 0x000A SHORT
	int PictureMode;			// 0x000B ピクチャーモード
										// 0 => 'Not Selected'
										// 5 => 'Portrait'
										// 6 => 'Landscape'
										// 9 => 'Night Scene'
										// 12 => 'Surf & Snow'
										// 13 => 'Sunset'
										// 14 => 'Autumn'
										// 15 => 'Flower'
										// 17 => 'Fireworks'
										// 18 => 'Text'
	int FlashMode;				// 0x000C USHRT フラッシュモード
										// 0x000,		_T("自動(非発光)"),
										// 0x001,		euOff,
										// 0x003,		_T("自動(非発光)+赤目軽減"),
										// 0x100,		_T("自動(発光)"),
										// 0x102,		euOn,
										// 0x103,		_T("自動(発光)+赤目軽減"),
										// 0x104,		_T("オン+赤目軽減"),
										// 0x108,		_T("オン+ソフト"),
	int FocusMode;				// 0x000D SHORT フォーカスモード
										// 0x0003=MF
										// 0x0010=AF-S
										// 0x0011=AF-C コンティニュアス
	int AFPointSelected;		// 0x000E USHRT: 測距点選択
										// 0xffff => 'Auto',
										// 0xfffe => 'Fixed Center',
										// 1 => 'Upper-left',
										// 2 => 'Top',
										// 3 => 'Upper-right',
										// 4 => 'Left',
										// 5 => 'Mid-left',
										// 6 => 'Center',
										// 7 => 'Mid-right',
										// 8 => 'Right',
										// 9 => 'Lower-left',
										// 10 => 'Bottom',
										// 11 => 'Lower-right',
	int AFPointsInFocus;		// 0x000F USHRT: AFエリア
										// 0xffff => 'None',
										// 1 => 'Top-left',
										// 2 => 'Top-center',
										// 3 => 'Top-right',
										// 4 => 'Left',
										// 5 => 'Center',
										// 6 => 'Right',
										// 7 => 'Bottom-left',
										// 8 => 'Bottom-center',
										// 9 => 'Bottom-right',
	int ShutterSpeed;			// 0x0012 ULONG シャッター速度
										// シャッター速度 x 100,000 の数値 1/100秒ならば 1000が格納されている
	int Aperture;				// 0x0013 USHRT 絞り
										// 数値の10分の1が絞り値。数値が56ならF5.6
	int ISO;					// 0x0014 SHORT ISO感度
										//  3 = 50
										//  4 = 64
										//  5 = 80
										//  6 = 100
										//  8 = 100
										//  9 = 200
										// 12 = 400
										// 18 = 1600
										// 21 = 3200
	int ExposureComp;			// 0x0016 USHORT 露出補正
										// 数値50を補正±0としてその差分の10分の1が補正値となる
	int MeteringMode;			// 0x0017 SHORT 測光モード
	int AutoBracket[2];			// 0x0018 USHRTx2: オートブラケット
										// 1st val: ブラケットステップ 1=.3ev, 2=.7, 3=1.0, ... 10=.5ev, 11=1.5, ...
										//     if (val < 10) ? val / 3.0F : val - 9.5F;
										// 2nd val: 拡張ブラケット（0=なし）
										//     上位バイト: ブラケット種別 1='WB-BA',2='WB-GM',3='Saturation',4='Sharpness',5='Contrast'
										//     下位バイト: ±値
	int WhiteBalance;			// 0x0019 SHORT ホワイトバランス
										//  0 = オート
										//  1 = 昼光
										//  2 = 日陰
										//  4 = 白熱灯
										//  5 = マニュアル
										//  6 = 蛍光灯(D)
										//  7 = 蛍光灯(N)
										//  8 = 蛍光灯(W)
										//  9 = ストロボ
										// 10 = 曇天
	int WhiteBalanceMode;		// 0x001A SHORT ホワイトバランス
										// 1 => 'Auto (Daylight)',
										// 2 => 'Auto (Shade)',
										// 3 => 'Auto (Flash)',
										// 4 => 'Auto (Tungsten)',
										// 0xffff => 'User-Selected',
										// 0xfffe => 'Preset (Fireworks?)'
	int BlueBalance;			// 0x001B USHRT: BlueBalance
										// val / 256
	int RedBalance;				// 0x001C USHRT: RedBalance
										// val / 256
	int FocalLength;			// 0x001D LONG レンズの焦点距離
										// XX(mm) x 100 の数値
	int DigiatlZoom;			// 0x001E USHRT デジタルズーム
	int Saturation;				// 0x001F SHORTx2 彩度
	int Contrast;				// 0x0020 SHORTx2 コントラスト
	int Sharpness;				// 0x0021 SHORTx2 シャープネス
										// 彩度・コントラスト・シャープネスは同一形式
										// 0 = -2
										// 1 = 標準
										// 2 = +2
										// 3 = -1
										// 4 = +1
										// 5 = -3
										// 6 = +3
	int WorldTimeLocation;		// 0x0022 USHRT ワールドタイム
										// 0 => 現在地 'Hometown',
										// 1 => 目的地 'Destination',
	int HometownCity;			// 0x0023 USHRT 地域（現在地）
	int DestinationCity;		// 0x0024 USHRT 地域（目的地）
	int HometownDST;			// 0x0025 USHRT 
										// 0 => 'No', 1 => 'Yes'
	int DestinationDST;			// 0x0026 USHRT 
										// 0 => 'No', 1 => 'Yes'
								// 0x0027 UNDEF
								// 0x0028 UNDEF
	int TakeCount;				// 0x0029 LONG 撮影枚数
								// 0x002B LONG
								// 0x002C LONG
	int EffectiveLightValue;	// 0x002D USHRT: カメラが計算した光量
	int ImageProcessing;		// 0x0032 UNDEFx4: イメージ加工
										// 0x00,00,00,00 未加工
										// 0x00,00,00,04 デジタルフィルタ
										// 0x04,00,00,00 カラーフィルタ
										// 0x10,00,00,00 フレーム合成
										// 0x12,00,00,00 フレーム合成
										// 0x14,00,00,00 フレーム合成
	int ShootMode[3];			// 0x0033 BYTEx3 撮影モード 上位2バイトと最下位バイトで分ける
	int DriveMode[4];			// 0x0034 BYTEx4 ドライブモード
								// 0x0035 SHORTx2
								// 0x0036 SHORT
	int ColorSpace;				// 0x0037 SHORT カラースペース
										// 0 = sRGB
										// 1 = AdobeRGB
								// 0x003A SHORT
								// 0x003C UNDEF
								// 0x003D SHORT
								// 0x003E BYTEx4
	int LensInfo;				// 0x003F BYTEx2 レンズ情報
										// 0x0328 FAマクロ100mmF2.8
										// 0x0319 SIGMA AF55-200mm F4-5.6DC
										// 0x0418 FA77mmF1.8 Limited
										// 0x0426 FA28-105mmFxx-Fxx
										// 0x042F FAJ18-35mmF4-5.6 AL
										// 0x0431 TAMRON SP AF28-75mm F2.8 XR Di
										// 0x04FC DA18-55mmF3.5-5.6 AL
										// 0x04FD DA14mmF2.8ED[IF]
										// 0x04FE DA16-45mmF4ED AL
										// 0x0509 FAマクロ100mmF2.8
										// 0x050B FA★200mmF2.8ED[IF]
										// 0x0604 FA★28-70mmF2.8 AL
	int SensitivityAdjust;		// 0x0040 USHRT: 感度調整
	int DigitalFilterCount;		// 0x0041 
	int CameraTemperature;		// SBYTE: カメラ温度
	int AELock;					// 0x0048 USHRT AEロック
	int NoiseReduction;			// 0x0049 USHRT ノイズリダクション
	int FlashExposureComp;		// 0x004D LONG フラッシュ調整
	int PhotoFinish;			// 0x004F USHRT 画像仕上げ？
										// 0 = ナチュラル
										// 1 = 鮮やか
	int ColorTemperature;		// 0x0050 USHRT: 色温度 (53190 - value)
	BYTE SRInfo[8];				// 0x005C SRInfo シェイクリダクション情報
	int ShutterCount;			// 0x005D UNDEFx4: シャッター回数
										// val ^ ShootingDate ^ (0xffffffff - (ShootingTime<<8));
								// 0x0200 USHRTx4 BlackPoint
								// 0x0201 USHRTx4 WhitePoint
	BYTE ShotInfo[32];			// 0x0205 UNDEFxn: ショット情報
	BYTE AEInfo[32];			// 0x0206 UNDEFxn: AE情報
	int CameraInfo[5];			// 0x0215 ULONGx5: カメラ情報
								// 0x0402 ToneCurve
								// 0x0403 ToneCurves
								// 0x1000 UNDEXx4 HometownCityCode
								// 0x1001 UNDEXx4 DestinationCityCode
} pentax_t;



#endif		// __MAKERNOTE_PENTAX_H__
