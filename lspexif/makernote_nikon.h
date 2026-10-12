///////////////////////////////////////////////////////////////////////
// makernote_nikon.h nikon情報

#pragma once
#ifndef __MAKERNOTE_NIKON_H__
#define __MAKERNOTE_NIKON_H__

////////////////////////////////////////////////////////////////////////
// NIKONモデル

enum {
	NIKON_UNKNOWN = 0,
	NIKON_COOLPIX = 0x100,			// Coolpixシリーズ
	NIKON_COOLPIX_S3,				// "COOLPIX S3"
	NIKON_COOLPIX_S5,				// "COOLPIX S5"
	NIKON_COOLPIX_S6,				// "COOLPIX S6"
	NIKON_COOLPIX_L3,				// "COOLPIX L3"
	NIKON_COOLPIX_P3,				// "COOLPIX P3"
	NIKON_COOLPIX_P4,				// "COOLPIX P4"
	NIKON_E_UNKNOWN,
	NIKON_E8800,					// "E8800"
	NIKON_E8700,					// "E8700"
	NIKON_E8400,					// "E8400"
	NIKON_E5700,					// "E5700"
	NIKON_E5400,					// "E5400"
	NIKON_DSLR = 0x1000,			// デジいちシリーズ
	NIKON_D_UNKNOWN,
	NIKON_D1,						// "NIKON D1"
	NIKON_D1X,						// "Mod NIKON D1X"
	NIKON_D1H,						// "NIKON D1H"
	NIKON_D100,						// "NIKON D100"
	NIKON_D70,						// "NIKON D70"
	NIKON_D70S,						// "NIKON D70s"
	NIKON_D50,						// "NIKON D50"
	NIKON_D40,						// "NIKON D40"
	NIKON_D40X,						// "NIKON D40X"
	NIKON_D60,						// "NIKON D60"
		NIKON_AFAREA_11POINTS,
	NIKON_D80,						// "NIKON D80"
	NIKON_D90,						// "NIKON D90"
	NIKON_D2H,						// "NIKON D2H"
	NIKON_D2HS,						// "NIKON D2Hs"
	NIKON_D2X,						// "NIKON D2X"
	NIKON_D200,						// "NIKON D200"
	NIKON_D2XS,						// "NIKON D2Xs"
	NIKON_D3000,					// "NIKON D3000"
	NIKON_D3100,					// "NIKON D3100"
	NIKON_D3200,					// "NIKON D3200"
	NIKON_D3300,					// "NIKON D3300"
		NIKON_AFAREA_15POINTS,
	NIKON_D300,						// "NIKON D300"
	NIKON_D300S,					// "NIKON D300S"
	NIKON_D310,						// "NIKON D310"
	NIKON_D340,						// "NIKON D340"
	NIKON_D350,						// "NIKON D350"
	NIKON_D360,						// "NIKON D360"
	NIKON_D3,						// "NIKON D3"
	NIKON_D3X,						// "NIKON D3X"
	NIKON_D3S,						// "NIKON D3X"
	NIKON_D4,						// "NIKON D4"
	NIKON_D4S,						// "NIKON D4S"
	NIKON_D700,						// "NIKON D700"
	NIKON_D710,						// "NIKON D710"
	NIKON_D740,						// "NIKON D740"
	NIKON_D750,						// "NIKON D750"
	NIKON_D600,						// "NIKON D600"
	NIKON_D610,						// "NIKON D610"
	NIKON_D800,						// "NIKON D800"
	NIKON_D800E,					// "NIKON D800e"
	NIKON_D810,						// "NIKON D810"
	NIKON_D810A,					// "NIKON D810a"
	NIKON_D5000,					// "NIKON D5000"
	NIKON_D5100,					// "NIKON D5100"
	NIKON_D5200,					// "NIKON D5200"
	NIKON_D5300,					// "NIKON D5300"
	NIKON_D5400,					// "NIKON D5400"
	NIKON_D5500,					// "NIKON D5500"
	NIKON_D7000,					// "NIKON D7000"
	NIKON_D7100,					// "NIKON D7100"
	NIKON_D7200,					// "NIKON D7200"
	NIKON_DF,						// "NIKON Df"
	NIKON_1_UNKNOWN = 0x2000,
	NIKON_1_J1,						// "NIKON 1 J1"
	NIKON_1_J2,						// "NIKON 1 J2"
	NIKON_1_J3,						// "NIKON 1 J3"
	NIKON_1_J4,						// "NIKON 1 J4"
	NIKON_1_J5,						// "NIKON 1 J5"
	NIKON_1_S1,						// "NIKON 1 S1"
	NIKON_1_V1,						// "NIKON 1 V1"
	NIKON_1_V2,						// "NIKON 1 V2"
	NIKON_1_V3,						// "NIKON 1 V3"
	NIKON_1_AW1,					// "NIKON 1 AW1"
};

#define IS_NIKON_HAS_AFAREA_11POINTS(n)	(((n)>NIKON_AFAREA_11POINTS)||		\
										 ((n)==NIKON_COOLPIX_P3)||			\
										 ((n)==NIKON_COOLPIX_P4))


////////////////////////////////////////////////////////////////////////
//

typedef struct __nikon_t {
	int ISO;						// 0x0002 SHORTx2:ISO感度
	BOOL ISOinc;						// ISO増感度フラグ
	char ColorMode[32];				// 0x0003 ASCII:カラーモード
	char QualityMode[16];			// 0x0004 ASCII:画質モード
	char WhiteBalance[16];			// 0x0005 ASCII:ホワイトバランス
	char EdgeMode[16];				// 0x0006 ASCII:輪郭の強調
	char FocusMode[16];				// 0x0007 ASCII:フォーカスモード
	char FlashSetting[40];			// 0x0008 ASCII:フラッシュの設定
	char FlashMode[32];				// 0x0009 ASCII:フラッシュモード
	int WhiteBalanceFine;			// 0x000B SSHRT:ホワイトバランス補正
									// 0x000C       カラーバランス1
	int ProgramShift;				// 0x000D UNDEFx4 ProgramShift
	int ExposureDifference;			// 0x000E UNDEFx4 ExposureDifference
	char ISOMode[32];				// 0x000F ASCII:ISOモード
									// 0x0011       NikonPreview -> SubIFD
	int FlashComp;					// 0x0012 UNDEFx4:調光補正
	int FlashExposureBracket;		// 0x0018 UNDEFx4:フラッシュブラケット
	float ExposureBracket;			// 0x0019 RATIN:ブラケットステップ
	char DLighting[40];				// 0x001A ASCII:D-ライティング
	char SerialNumber[32];			// 0x001D or 0x00A0 ASCII:シリアルナンバー
	int ColorSpace;					// 0x001E USHRT 色空間
										// 1 = sRGB
										// 2 = AdobeRGB

	int VibrationReduction;			// 0x001F: UNDEFx8: VRInfo offset 0-3=vesrion, offset 4=VibrationReduction(1=0n,2=Off)
	int ImageAuthentication;		// 0x0020 BYTE: 画像真正性検証  0=Off, 1=On
	int ActiveDLighting;			// 0x0022 USHRT: アクティブD-ライティング
										// 0=Off, 1=Low, 3=Normal, 5=High

	BYTE PictureControlInfo[64];	// 0x0023 UNDEFxn: ピクチャーコントロール情報
	BYTE ISOInfo[16];				// 0x0025 UNDEFxn: ISO情報
										// offset 00:       ISO -- val = 100 * 2**(raw/12-5)
										// offset 04 - 05:  ISOExpansion
															// 0x000 = Off
															// 0x101 = Hi 0.3
															// 0x102 = Hi 0.5
															// 0x103 = Hi 0.7
															// 0x104 = Hi 1.0
															// 0x105 = Hi 1.3, # (Hi 1.3-1.7 may be possible with future models)
															// 0x106 = Hi 1.5
															// 0x107 = Hi 1.7
															// 0x108 = Hi 2.0, #(NC) - D3 should have this mode
															// 0x201 = Lo 0.3
															// 0x202 = Lo 0.5
															// 0x203 = Lo 0.7
															// 0x204 = Lo 1.0
										// offset 06 - 11: same as 0-4 in my samples (why is this duplicated?)
										// offset 12 - 13: 00 00
	int VinetControl;				// 0x002A USORT:ヴィネットコントロール
										// 0 = off
										// 1 = 弱め（30%）
										// 3 = 標準（50%）
										// 5 = 強め（70%）
	char ImageMode[32];				// 0x0080 ASCII:画像モード
	char ToneAdjust[32];			// 0x0081 ASCII:階調補正
	char LensConverter[32];			// 0x0082 ASCII:レンズコンバータ
	int LensType;					// 0x0083      :レンスタイプ
										// 0 = AF non D lens
										// 1 = Manula
										// 2 = AF-D or AF-S lens
										// 6 = AF-D G lens
										// 10 = AF-D VR lens
	float LensInfo[4];				// 0x0084 RATINx4:レンズ情報
										// 最初の２つが焦点距離A～B
										// 後の２つが開放F値C～D
										// (例) 35.0-50.0(mm) F2.8-F5.6
										// ## 値が1.0の場合にはexif_tから焦点距離と開放F値を取得
	float ManualFocusDistance;		// 0x0085 マニュアルフォーカス距離(meter)
	float DigitalZoom;				// 0x0086 デジタルズーム
	int FlashUsed;					// 0x0087 BYTE:フラッシュの利用
										// 0 = flash unit did not fire
										// 4 = flash unit unknown
										// 7 = flash unit is external
										// 9 = flash unit is on camera
	BYTE AFInfo[4];					// 0x0088 AF情報
										// offset 0		0 = シングルエリアAF
										//				1 = ダイナミックAF
										//				2 = 至近優先ダイナミックAF
										//				3 = グループダイナミックAF
										//				4 = シングルエリアAF（ワイドフレーム）
										//				5 = ダイナミックAF（ワイドフレーム）
										// offset 1		0 = 中央
										//				1 = 上
										//				2 = 下
										//				3 = 左
										//				4 = 右
										// offset 2,3	unknown
	int DriveMode;					// 0x0089 USHRT ドライブモード
										// 0 = 1コマ撮影
										// 1 = 連続撮影
										// 2 = セルフタイマー撮影
										// 16 = 2秒リモコン撮影
										// 17 = 瞬時リモコン撮影
	int AutoBracketRelease;			// 0x008A USHRT: オートブラケットレリーズ
										// 0 => 'None',
										// 1 => 'Auto Release',
										// 2 => 'Manual Release',
	char ColorSpaceName[16];		// 0x008D ASCII:色空間
	char SceneMode[32];				// 0x008F ASCII:シーンモード
	char LightType[16];				// 0x0090 ASCII:ライトタイプ
	int HueAdjustment;				// 0x0092 色合い調整
	int SaturationComp;				// 0x0094 彩度補正
										// 0 - 255
	char NoiseReduction[16];		// 0x0095 ノイズ除去
	int LensID;						// 0x0098 UNDEF レンズデータブロック内
	int FocusDistance;				//  〃
	int AFAperture;					//  〃
	int FocalLength;				//  〃
	char LensData[24];				// 0x0098 UNDEF レンズデータブロック内
	char LensModel[64];				//  〃
	float XCCDPixelPitch;			// 0x009A RATINx2: 画素ピッチ(XY) μm
	float YCCDPixelPitch;			//  〃
	char AssSceneMode[32];			// 0x009C ASCII:アシスト機能付きシーンモード
									// 0x009E USHRTx10
	int FilterEffect;					// offset 0		0 = なし
										//				3 = モノトーン（白黒）
										//				4 = モノトーン（セピア）
										//				9 = モノトーン（クール）
										//				10 = スカイライト
										//				11 = ウォームトーン
										//				12 = カラーカスタマイズ
	int TakeCount;					// 0x00A5 LONG: 差引撮影回数 TakeCount = RealTakeCount - DelTakeCount
	int DelTakeCount;				// 0x00A6 LONG: 削除回数
	int RealTakeCount;				// 0x00A7 LONG: 撮影回数
	char ResultCondition[32];		// 0x00A9 ASCII 仕上がり設定
	char ColorEnhance[32];			// 0x00AA ASCII:彩度設定
	char DigiImagePgm[40];			// 0x00AB ASCII デジタルイメージプログラム
	char VibrationReductionText[16];// 0x00AC ASCII 手ブレ補正
	BYTE MultiExposureInfo[16];		// 0x00B0 UNDEF:多重露出情報
										// offset 0 - 3: MultiExposureVersion
										// offset     4: MultiExposureMode, 0=Off, 1=Multiple Exposure, 2=Image Overlay
										// offset     5: MultiExposureShots, # of shots
										// offset     6: MultiExposureAutoGain, 0=Off, 1=On
	int HiSensitiveNoiseReduction;	// 0x00B1 USHRT 高感度ノイズ除去
										// 1 = OFF
										// 2 = LOW
										// 4 = NORMAL
										// 6 = HIGH
	char BWFilterEffect[32];		// 0x00B3 ASCII 白黒カスタマイズ（フィルター効果）
	char Software[40];				// 0x0E09 ASCII ソフトウェア
} nikon_t;


#endif		// __MAKERNOTE_NIKON_H__
