///////////////////////////////////////////////////////////////////////
// makernote_olympus.h olympus情報

#pragma once
#ifndef __MAKERNOTE_OLYMPUS_H__
#define __MAKERNOTE_OLYMPUS_H__

///////////////////////////////////////////////////////////////////////
// OLYMPUSモデル

enum {
	OLYMPUS_UNKNOWN = 0,
	OLYMPUS_C = 0x100,
	OLYMPUS_MYU = 0x200,
	OLYMPUS_E = 0x1000,				// Eシリーズ
	OLYMPUS_E1,						// "E-1"
	OLYMPUS_E20,					// "E-20,E-20N,E20P"
	OLYMPUS_E300,					// "E-300"
	OLYMPUS_E500,					// "E-500"
	OLYMPUS_E330,					// "E-330"
	OLYMPUS_E400,					// "E-400"
	OLYMPUS_E410,					// "E-410"
	OLYMPUS_E420,					// "E-420"
	OLYMPUS_E450,					// "E-450"
	OLYMPUS_E510,					// "E-510"
	OLYMPUS_E620,					// "E-620"
	OLYMPUS_E3,						// "E-3"
	OLYMPUS_EP,						// "E-PXX"
	OLYMPUS_EP1,					// "E-P1"
	OLYMPUS_EP3,					// "E-P3"
	OLYMPUS_EP5,					// "E-P5"
	OLYMPUS_EPM2,					// "E-PM2"
	OLYMPUS_EPL5,					// "E-PL5"
	OLYMPUS_EPL6,					// "E-PL6"
	OLYMPUS_EM,						// "E-MXX"
	OLYMPUS_EM1,					// "E-M1"
	OLYMPUS_EM5,					// "E-M5"
};

#define IS_OLYMPUS_E(n)				((n)>=OLYMPUS_E)

///////////////////////////////////////////////////////////////////////
//

typedef struct __olympus_t {
	int ExifDataType;					// Exifデータタイプ情報（解析時のみに利用）
	int ShootingMode;					// 0x0200 LONGx3	(3-0) 撮影モード
	int SequenceNumber;							//			(3-1) シーケンス番号
	int PanoramaDirection;						//			(3-2) パノラマ方向
	int Quality;						// 0x0201 SHORT:画質モード
												// 1 = SQ
												// 2 = HQ
												// 3 = SHQ
	int MacroMode;						// 0x0202 SHORT:マクロモード
												// 0 = OFF
												// 1 = ON
										// 0x0203 USHRT: Black & White Mode
												// Off or On
	float DigitalZoom;					// 0x0204 RATIN:デジタルズーム
										// 0x0205 RATINL: FocalPlaneDiagonal
										// 0x0206 SSHRTx6
	char HardwareType[32];				// 0x0207 ASCII:ハードウェアタイプ
	char PictInfo[80];					// 0x0208 ASCII:ピクチャ情報
	BYTE CameraID[60];					// 0x0209 UNDEF:カメラID
	int OneTouchWB;						// 0x0302 USHRT:ワンタッチWB
	int FlashMode;						// 0x1004 SHORT:フラッシュモード
												// 1 = 自動
												// 2 = 赤目軽減
												// 3 = 強制
												// 4 = オフ
										// 0x1005 FlashDevice
												// 0='None'
												// 1='Internal'
												// 4='External'
												// 5='Internal+External'
	float BracketEVBias;				// 0x1006 RATIN:ブラケットEV補正値
	int FocusMode;						// 0x100B SHORT:フォーカスモード
											// 0 = オートフォーカス
											// 1 = マニュアルフォーカス
										// 0x100C FocusDistance
	int ZoomPosition;					// 0x100D SHORT:ズーム位置
										// 0x100E MacroFocus
	int Sharpness;						// 0x100F SHORT:シャープネス
												// 0 = Normal2
												// 1 = Hard2
												// 2 = Soft2
										// 0x1011 USHRTx9: ColorMatrix
										// 0x1012 USHRTx14: BlackLevel
	int WhiteBalanceMode[2];			// 0x1015 SHORTx2:ホワイトバランス
	short WBCompRed[2];					// 0x1017 USHORTx2 赤WB補正値
	short WBCompBlue[2];				// 0x1018 USHORTx2 青WB補正値
	char SerialNumber[40];				// 0x101A or 0x0404 ASCII シリアルナンバー
	float FlashComp;					// 0x1023 SRATIN:フラッシュ発光量補正
	int Contrast;						// 0x1029 SHORT:コントラスト
												// 0 = High
												// 1 = Normal
												// 2 = Low
										// 0x102A USHRT: SharpnessFactor(2)
										// 0x102B USHRTx6: ColorControl
										// 0x102C USHRTx2: ValidBits
										// 0x102D USHRT: CoringFilter
	int ImageWidth;						// 0x102E ULONG 画像幅
	int ImageHeight;					// 0x102F ULONG 画像幅
										// 0x1034 RATNL JPEG圧縮率 --> exCompressionFactor
										// 0x1035 PreviewImageValid 0 = 'No', 1 = 'Yes'
										// 0x1036 PreviewImageStart
										// 0x1037 PreviewImageLength

	// 0x2010 Equipment 情報ブロック
										// 0x0000 UNDEFx4: EquipmentVersion
										// 0x0100 ASCII: FirmwareVersion2
		// SerialNumber					// 0x0101 ASCII: SerialNumber
	char IntSerialNumber[32];			// 0x0102 ASCII: InternalSerialNumber
												// 16 digits: 0-3=model, 4=year, 5-6=month, 8-12=unit number
										// 0x0103 URTNL: FocalPlaneDiagonal
	int BodyFirmwareVersion;			// 0x0104 ULONG: BodyFirmwareVersion
	int LensInfo;						// 0x0201 BYTEx6: Lens
												// 1. Make, 2. Unknown, 3. Model, 4. Release, 5-6. Unknown
	int LensFirmwareVersion;			// 0x0204 ULONG: LensFirmwareVersion
	char LensSerialNumber[32];			// 0x0202 ASCII: LensSerialNumber
	int MaxApertureAtMinFocal;			// 0x0205 USHRT: レンズ開放F値(W)
	int MaxApertureAtMaxFocal;			// 0x0206 USHRT: レンズ開放F値(W)
	int MinFocalLength;					// 0x0207 USHRT: 最小焦点距離(mm)
	int MaxFocalLength;					// 0x0208 USHRT: 最大焦点距離(mm)
	int MaxApertureAtCurrentFocal;		// 0x020A USHRT: 実焦点でのレンズ開放F値
	int ExtenderInfo;					// 0x0301 BYTEx6: Extender
												// 1. Make, 2. Unknown, 3. Model, 4. Release, 5-6. Unknown
	char ExtenderSerialNumber[32];		// 0x0302 ASCII: ExtenderSerialNumber
	char ExtenderModel[32];				// 0x0303 ASCII: ExtenderModel
	int ExtenderFirmwareVersion;		// 0x0304 ULONG: ExtenderFirmwareVersion
	int FlashSystem;					// 0x1000 USHRT: フラッシュシステム
												// 0=None
												// 2=Simple E-System
												// 3=E-System
	int FlashModelInfo;					// 0x1001 USHRT: FlashModel
												// 0=None
												// 1=FL-20
												// 2=FL-50
												// 3=RF-11
												// 4=TF-22
												// 5=FL-36
	char FlashModelSerialNumber[32];	// 0x1003 ASCII: FlashSerialNumber
	int FlashModelFirmwareVersion;		// 0x1004 ULONG: FlashFirmwareVersion

	// --------------------------------------
	// 0x2020 CameraSettings 情報ブロック
	// --------------------------------------
	int exExposureMode;					// 0x0200 USHORT: ExposureMode
												// 1='Manual'
												// 2='Program AE'
												// 3='Aperture-priority AE'
												// 4='Shutter speed priority AE'
												// 5='Program Shift'
	int exAELock;						// 0x0201 USHRT: AELock
												// 0='off'
												// 1='on'
	int exMeteringMode;					// 0x0202 USHORT: MeteringMode
												// 2 => 'Center Weighted',
												// 3 => 'Spot',
												// 5 => 'ESP',
												// 261 => 'Pattern+AF', #6
												// 515 => 'Spot+Highlight control', #6
												// 1027 => 'Spot+Shadow control', #6
	int exMacroMode;					// 0x0300 USHORT: MacroMode
												// 0=Off
												// 1=On
	int exFocusMode;					// 0x0301 USHORT: FocusMode
												// 0=Single AF
												// 1=Sequential shooting AF
												// 2=Continuous AF
												// 3=Multi AF
												// 10=MF
										// 0x0302 USHORT: FocusProcess
												// 0=AF Not Used
												// 1=AF Used
	int exAFSearch;						// 0x0303 USHORT: AFSearch
												// 0=Not Ready
												// 1=Ready
	int exAFArea;						// 0x0304 ULONGx64: AFAreas
												// 最初の1要素 0x36794285 => 'Left',
												//             0x79798585 => 'Center',
												//             0xBD79C985 => 'Right',
	int exFlashMode;					// 0x0400
												// 0=Off
												// 1=On(Auto-flash)  
												// 3=On(Fill-in Flash)  
												// 5=On(Red-Eye Reduction Flash)  
												// 7=On(Fill-in Flash (Red-Eye Reduction))  
												// 9=On(SLOW (1st Curtain))  
												// 13=On(SLOW (1st Curtain with Red-Eye Reduction))  
												// 17=On  
												// 19=On(Fill-in Flash)  
												// 35=On(Fill-in Flash (2nd Curtain))  
												// 41=On(SLOW (2nd Curtain))  
												// 57=On(SLOW (2nd Curtain))  
	float exFlashExposureComp;			// 0x0401 SRTNL: FlashExposureCompensation
	int exFlashRemoteControl;			// 0x0403 USHRT: フラッシュリモート制御 （E-3以降？）
												// 0=Off
												// Bit4..3 -> Comm. Light Level:
												//            00=Low
												//            01=Middle
												//            10=High
												// Bit2..0 -> Channel (1..4)
	int exFlashControlMode[3];			// 0x0404 USHRTx3: 外部フラッシュリモート制御
												// 0=Off
												// Bit4..3 -> Comm. Light Level:
												//            00=Low
												//            01=Middle
												//            10=High
												// Bit2..0 -> Channel (1..4)
	float exFlashIntensityTTL[3];		// 0x0405 SRATINx3:
	float exFlashIntensityValue[3];		// 0x0406 SRATINx3:
	int exWhiteBalance;					// 0x0500 USHRT: WhiteBalance2
												//   0 => 'Auto',
												//  16 => '7500K (Fine Weather with Shade)',
												//  17 => '6000K (Cloudy)',
												//  18 => '5300K (Fine Weather)',
												//  20 => '3000K (Tungsten light)',
												//  21 => '3600K (Tungsten light-like)',
												//  33 => '6600K (Daylight fluorescent)',
												//  34 => '4500K (Neutral white fluorescent)',
												//  35 => '4000K (Cool white fluorescent)',
												//  48 => '3600K (Tungsten light-like)',
												// 256 => 'Custom WB 1',
												// 257 => 'Custom WB 2',
												// 258 => 'Custom WB 3',
												// 259 => 'Custom WB 4',
												// 512 => 'Custom WB 5400K',
												// 513 => 'Custom WB 2900K',
												// 514 => 'Custom WB 8000K',
	int exWBTemperature;				// 0x0501 USHORT: WhiteBalanceTemperature
	int exWhiteBalanceBracket[2];		// 0x0502 SSHRTx2: ホワイトバランスブラケット
	int exSaturationSetting[3];			// 0x0503 SHORTx3: SaturationSetting
												// 1. CS Value, 2. Min, 3. Max
	int exModifiedSaturation;			// 0x0504 USHORT: ModifiedSaturation
												// 0=Off
												// 1=CM1 (Red Enhance)
												// 2=CM2 (Green Enhance)
												// 3=CM3 (Blue Enhance)
												// 4=CM4 (Skin Tones)
	int exContrastSetting[3];			// 0x0505 SHORTx3: ContrastSetting
												// 1. Contrast, 2. Min, 3. Max
	int exSharpnessSetting[3];			// 0x0506 SHORTx3: SharpnessSetting
												// 1. Sharpness, 2. Min, 3. Max
	int exColorSpace;					// 0x0507 USHORT: ColorSpace
												// 0=sRGB
												// 1=Adobe RGB
												// 2=Pro Photo RGB
	int exSceneMode;					// 0x0509 USHORT: SceneMode
												//  0=Standard
												//  6=Auto
												//  7=Sport
												//  8=Portrait
												//  9=Landscape + Portrait
												// 10=Landscape
												// 11=Night scene
												// 12=Self Portrait
												// 13=Panorama
												// 16=Landscape+Portrait
												// 17=Night + Portrait
												// 19=Fireworks
												// 20=Sunset
												// 22=Macro
												// 25=Documents
												// 26=Museum
												// 28=Beach & Snow
												// 30=Candle
												// 35=Underwater Wide1
												// 36=Underwater Macro
												// 39=High Key
												// 40=Digital Image Stabilization
												// 44=Underwater Wide2
												// 45=Low Key
												// 46=Children
												// 48=Nature Macro
	int exNoiseReduction;				// 0x050A USHORT: NoiseReduction
												// 0=Off
												// 1=Noise Reduction
												// 2=Noise Filter
												// 3=Noise Reduction + Noise Filter
												// 4=Noise Filter (ISO Boost)
												// 5=Noise Reduction + Noise Filter (ISO Boost)
	int exShadingComp;					// 0x050C USHORT: ShadingCompensation
												// Off or On
	float exCompressionFactor;			// 0x050D URATION: CompressionFactor
												// JPEG圧縮率 1/2.7 1/8 1/4 等
	int exGradation;					// 0x050F SHORT: Gradation
												// -1=Low Key
												//  0=Normal
												//  1=High Key
	int exPictureMode;					// 0x0520 USHRT: 仕上がりモード
												// 0x0001=vivid
												// 0x0002=natural
												// 0x0003=flat
												// 0x0100=mono
												// 0x0200=sepia
	int exPicModeSaturation;			// 0x0521 SSHRTx3: 仕上がりモード：彩度 (val, min, max)
	int exPicModeContrast;				// 0x0523 SSHRTx3: 仕上がりモード：コントラスト (val, min, max)
	int exPicModeSharpness;				// 0x0524 SSHRTx3: 仕上がりモード：シャープネス (val, min, max)
	int exPicModeFilterEffect;			// 0x0525 SSHRT: 仕上がりモード：フィルター効果
												// 0 => 'n/a',
												// 1 => 'Neutral',
												// 2 => 'Yellow',
												// 3 => 'Orange',
												// 4 => 'Red',
												// 5 => 'Green',
	int exPicModeTone;					// 0x0526 SSHRT: 仕上がりモード：調色
												// 0 => 'n/a',
												// 1 => 'Neutral',
												// 2 => 'Sepia',
												// 3 => 'Blue',
												// 4 => 'Purple',
												// 5 => 'Green',
	int exNoiseFilter;					// 0x0527 ノイズフィルタ
												// -2=Off
												// -1=Low
												//  0=Standard
												//  1=High
	int exShotSequence[3];				// 0x0600 USHORTx3: Sequence
												// 2 or 3 numbers: 1. Mode, 2. Shot number, 3. Mode bits,
												// 1=Continuous Shooting
												// 2=Exposure Bracketing
												// 3=White Balance Bracketing
	int exPanoramaMode;					// 0x0601 USHRT: パノラマモード
												// 0=Off
												// 1=Left to right
												// 2=Right to left
												// 3=Bottom to top
												// 4=Top to bottom
//	int exImageQuality;					// 0x0603 USHORT: ImageQuality2
												// 0=TIFF
												// 1=SQ
												// 2=HQ
												// 3=SHQ
												// 4=RAW
												// 5=SQ
												// 8=TIFF
												// 36=TIFF
	int ManometerPressure;				// 0x0900 USHRT: 気圧(hPa)
	int ManometerReading[2];			// 0x0901 SLONG: 標高・水深 offset 0=meter, 1=feet
	int ExtendedWBDetect;				// 0x0902 USHRT: OffOn

	// --------------------------------------
	// 0x2030 RawDevelopment 情報ブロック
	// RAW画像をOLYMPUSStudioで編集すると付加される
	// --------------------------------------
										// 0x0000 UNDEFx4: RawDevVersion
	float RawExposureComp;				// 0x0100 SRTNL: Raw編集 露出補正
	int RawWhiteBalance;				// 0x0101 USHRT: Raw編集 ホワイトバランス値
	int RawWhiteBalanceAdjust;			// 0x0102 SSHRT: Raw編集 ホワイトバランス微調整(-10～+10)
	int RawGrayValue[3];				// 0x0103 USHRTx3: Raw編集 グレー値(R,G,B値)
	int RawSaturationAdjust[3];			// 0x0104 SSHRTx3: Raw編集 彩度調整(-5～+5)
										// 0x0105 USHRT: Raw編集 偽色抑制(0～10)
	int RawContrastAdjust[3];			// 0x0106 SSHRTx3: Raw編集 コントラスト調整(-5～+5)
	int RawSharpnessAdjust[3];			// 0x0107 SSHRTx3: Raw編集 シャープネス調整(-5～+5)
	int RawColorSpace;					// 0x0108 USHRT: Raw編集 カラー設定(sRGB, AdobeRGB, ProPhotoRGB)
												// 0 => 'sRGB',
												// 1 => 'Adobe RGB',
												// 2 => 'Pro Photo RGB',
										// 0x0109 USHRT: RawDevEngine
												// 0 => 'High Speed',
												// 1 => 'High Function',
												// 2 => 'Advanced High Speed',
												// 3 => 'Advanced High Function',
	int RawNoiseCancel;					// 0x010A USHRT: Raw編集 ノイズキャンセル(0～10)
												// 0 => 'Noise Reduction',
												// 1 => 'Noise Filter',
												// 2 => 'Noise Filter (ISO Boost)',
										// 0x010B USHRT: RawDevEditStatus
										// 0x010C USHRT: RawDevSettings
												// ※ビットマスク
												// 0 => 'WB Color Temp',
												// 2 => 'WB Gray Point',
												// 3 => 'Saturation',
												// 4 => 'Contrast',
												// 5 => 'Sharpness',
												// 6 => 'Color Space',
												// 7 => 'High Function',
												// 8 => 'Noise Reduction',

	// --------------------------------------
	// 0x2031 RawDevelopment2 情報ブロック
	// RAW画像をOLYMPUSStudioで編集すると付加される
	// --------------------------------------
										// 0x0000 UNDEFx4
										// 0x0100 RATNL: 例 1/0
										// 0x0101 USHRT: 例 0x0001
										// 0x0102 USHRT: Raw編集 ホワイトバランス値 Subdir(0x2030) -> 	0x0501
										// 0x0103 SSHRT: Raw編集 ホワイトバランス微調整(-10～10)
										// 0x0104 USHRTx2: 例 0000,0000
			// exContrastSetting		// 0x0105 SSHRTx3: Raw編集 コントラスト調整(-5～+5)
			// exSharpnessSetting		// 0x0106 SSHRTx3: Raw編集 シャープネス調整(-5～+5)
			// exSaturationSetting		// 0x0107 SSHRTx3: Raw編集 彩度調整(-5～+5)
										// 0x0108 SSHRT:
			// exColorSpace				// 0x0109 USHRT: Raw編集 カラー設定(sRGB, AdobeRGB, ProPhotoRGB)
										// 0x010A USHRT: 例 0002
										// 0x010B USHRT: 例 0000
	// --------------------------------------
	// 0x2040 ImageProcessing 情報ブロック
	// RAW画像をOLYMPUSStudioで編集すると付加される
	// --------------------------------------
										// 0x0000 UNDEF: ImageProcessingVersion
										// 0x0100 USHORTx2: RedBlueBias
										// 0x011F USHORT: ?
										// 0x0200 USHORTx9: ColorMatrix
										// 0x0300 USHORT: SmoothingParameter1
										// 0x0310 USHORT: SmoothingParameter2
										// 0x0331 USHORTx8: ?
										// 0x0600 USHORT: SmoothingThresholds
										// 0x0610 USHORT: SmoothingThreshold2
										// 0x0611 USHORTx2: ValidBits
										// 0x0612 USHORTx2: ValidBits
										// 0x0613 USHORTx2: ValidBits
										// 0x0614 ULONG: OlympusImageWidth2
										// 0x0615 ULONG: OlympusImageHeight2
										// 0x0617 USHORT: ?
										// 0x0800 FLOATx9: ?
										// 0x0801 USHORTx16: ?
										// 0x0802 RATINL: ?
										// 0x1000 USHORT: ?
										// 0x1001 USHORT: ?
										// 0x1002 USHORT: ?
										// 0x1003 USHORT: ?
										// 0x1004 USHORT: ?
										// 0x1010 USHORT: NoiseFilter2
												// 0 => 'Off',
												// 1 => 'Noise Filter',
												// 2 => 'Noise Reduction',
										// 0x1011 USHORT: ?
										// 0x1012 USHORT: ShadingCompensation2
												// Off or On
	// --------------------------------------
	// 0x2050 FocusInfo 情報ブロック
	// RAW画像をOLYMPUSStudioで編集すると付加される
	// --------------------------------------
										// 0x0000 UNDEF FocusInfoVersion
										// 0x0200 SSHRTx2:
										// 0x0201 SSHRTx2:
										// 0x0202 SSHRTx2:
										// 0x0203 SSHRT:
										// 0x0204 SSHRTx2:
										// 0x0205 SSHRT:
										// 0x0206 SSHRT:
										// 0x0207 USHRT:
										// 0x0209 USHORT AutoFocus
												// Off or On
										// 0x020A USHRT:
										// 0x020B USHRT:
										// 0x020C SSHRTx2:
										// 0x020D ULONG:
										// 0x020E ULONGx5:
										// 0x020F ULONGx5:
										// 0x0210 USHRT:
										// 0x0211 ULONGx8:
										// 0x0212 ULONGx720:
										// 0x0213 USHRT:
										// 0x0214 USHRT:
										// 0x0215 USHRT:
										// 0x0300 USHRT: ZoomPosition
										// 0x0301 USHRT: Related to inverse of focus distance
										// 0x0302 USHRT:
										// 0x0305 RATNL: FocusDistance
												// this rational value looks like it is in mm when the denominator is
												// 1 (E-1), and cm when denominator is 10 (E-300), so if we ignore the
												// denominator we are consistently in mm - PH

										// 0x031A Continuous AF parameters?
										// 0x1201 USHORTx2: ExternalFlash
												// '0 0' => 'Off'
												// '1 0' => 'On'
										// 0x1208 USHORT: InternalFlash
												// Off or On
										// 0x102a same as Subdir4(0x2030) - 0x300
} olympus_t;


#endif			// __MAKERNOTE_OLYMPUS_H__
