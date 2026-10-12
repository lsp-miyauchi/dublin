//////////////////////////////////////////////////////////////////////
// exiftag.h
// Describes tag values

#pragma once
#ifndef __EXIFTAG_H__
#define __EXIFTAG_H__


#define TAG_NEW_SUBFILE_TYPE		0xfe			// NewSubFileType
#define TAG_IMAGE_WIDTH				0x100			// イメージ幅
#define TAG_IMAGE_LENGTH			0x101			// イメージ高さ
#define TAG_BITS_SAMPLE				0x102			// ビットの深さ（Bits Per Sample）例 8,8,8
#define TAG_COMPRESSION				0x103			// 圧縮の種類
													// 1=非圧縮
													// 2=RLE
													// 5=LZW
													// 6=JPEG（サムネイルのみ）
													// 7=JPEG/lossless JPEG（DNGのみ）
													// 32773=PackBits
#define TAG_PHOTOMETRIC_INTERPRET	0x106			// SHORT:画素構成
													// 2=RGB
													// 6=YCbCr
#define TAG_FILL_ORDER				0x10A			// Fill Order
#define TAG_DOCUMENT_NAME			0x10D			// Document Name
#define TAG_IMAGE_DESCRIPTION		0x10E			// Image Description
#define TAG_MAKE					0x10F			// メーカー名
#define TAG_MODEL					0x110			// モデル名
#define TAG_STRIP_OFFSET			0x111			// Strip Offsets
#define TAG_ORIENTATION				0x112			// オリエンテーション
													// 1: そのまま
													// 2: 左右反転
													// 3: 180°回転
													// 4: 上下反転
													// 6: 左へ90°回転する
													// 8: 右へ90°回転する
#define TAG_SAMPLE_PIXEL			0x115			// Samples Per Pixel
#define TAG_ROWS_STRIP				0x116			// Rows Per Strip
#define TAG_STRIP_BYTE_COUNT		0x117			// StripByteCounts
#define TAG_XRESOLUTION				0x11A			// XResolution
#define TAG_YRESOLUTION				0x11B			// YResolution
#define TAG_PLANER_CONFIG			0x11C			// Planar Configuration
													// 非圧縮イメージがYCbCr形式で格納されている場合に、
													// Y/Cb/Crがどのような並びで格納されているかを示す。
													// '1'の場合(chunky format)、サブサンプリングの点集合ごとにY/Cb/Crを並べて格納
													// '2'の場合(planar format)、全てのYデータ、全てのCrデータ、全てのCbデータの順にそれぞれ
													// 分けて格納。
#define TAG_RESOLUTION_UNIT			0x128			// ResolutionUnit
#define TAG_TRANSFER_FUNCTION		0x12D			// TransferFunction
#define TAG_SOFTWARE				0x131			// Software
#define TAG_DATETIME				0x132			// 日時
#define TAG_ARTIST					0x13B			// Artist
#define TAG_WHITE_POINT				0x13E			// WhitePoint
#define TAG_PRIMARY_CHROMA			0x13F			// Primary Chromaticities
#define TAG_HALFTONEHINTS			0x141			// HalfToneHints
#define TAG_TILE_WIDTH				0x142			// タイル幅
#define TAG_TILE_LENGTH				0x143			// タイル高さ
#define TAG_TILE_OFFSET				0x144			// タイルオフセット x タイル個数分続く
#define TAG_TILE_BYTECOUNTS			0x145			// タイルバイトサイズ x タイル個数分続く
									// TilesAcross = (ImageWidth + TileWidth - 1) / TileWidth
									// TilesDown = (ImageLength + TileLength - 1) / TileLength
									// イメージあたりのタイル数 = TilesAcross * TilesDown
#define TAG_SUBIFD					0x14A			// Sub IFD
#define TAG_INKSET					0x14C			// InkSet
#define TAG_INKNAMES				0x14D			// InkNames
#define TAG_NUMBEROFINKS			0x14E			// NumberOfInks
#define TAG_DOTRANGE				0x150			// DotRange
#define TAG_TARGETPRINTER			0x151			// TargetPrinter
#define TAG_EXTRASAMPLES			0x152			// ExtraSamples
#define TAG_SAMPLEFORMAT			0x153			// SampleFormat
#define TAG_SMIN_SAMPLEVALUE		0x154			// SMinSampleValue
#define TAG_SMAX_SAMPLEVALUE		0x155			// SMaxSampleValue
#define TAG_TRANSFER_RANGE			0x156			// Transfer Range
#define TAG_JPEG_PROC				0x200			// JPEG Proc
#define TAG_THUMBNAIL_OFFSET		0x201			// サムネイルデータオフセット
#define TAG_THUMBNAIL_LENGTH		0x202			// サムネイルデータサイズ
#define TAG_YCBCR_COEFF				0x211			// YCbCr Coefficients
#define TAG_YCBCR_SUBSAMPLING		0x212			// YCbCr SubSampling
#define TAG_YCBCR_POSITIONING		0x213			// YCbCr Positioning
#define TAG_REF_BLACKWHITE			0x214			// ReferenceBlackWhite
#define TAG_XML_METADATA			0x2BC			// XML メタデータ
#define TAG_ADOBE_RESOURCE			0x8649			// Adobeリソース
#define TAG_EXIF_OFFSET				0x8769			// EXIF IFD オフセット
#define TAG_INTERCOLOR_PROFILE		0x8773
#define TAG_MODEL2					0x827D			// モデル2
#define TAG_CFA_REPEATPATTERN_DIM	0x828D			// CFA Repeat Pataeern Dim
#define TAG_CFA_REPEATPATTERN		0x828E			// CFA Pattern
#define TAG_BATTERY_LEVEL			0x828F			// バッテリーレベル
#define TAG_COPYRIGHT				0x8298			// Copyright
#define TAG_EXPOSURE_TIME			0x829A			// 露出時間
#define TAG_FNUMBER					0x829D			// F値
#define TAG_EXPOSURE_PROGRAM		0x8822			// 露出制御プログラム
													// 0 = 未定義
													// 1 = マニュアル
													// 2 = ノーマルプログラム
													// 3 = 露出優先
													// 4 = シャッター優先
													// 5 = creative プログラム（被写界深度方向にバイアス）
													// 6 = action プログラム（シャッタースピード高速側にバイアス）
													// 7 = ポートレイトモード（クローズアップ撮影、背景はフォーカス外す）
													// 8 = ランズケープモード（landscape 撮影、背景はフォーカス合う）
#define TAG_SPECTRAL_SENSITIIVITY	0x8824			// スペクトル感度 Spectral Sensitivity
#define TAG_GPS_OFFSET				0x8825			// GPS IFD オフセット
#define TAG_ISO_EQUIVALENT			0x8827			// ISO感度
#define TAG_OECF					0x8828			// 光電変換関数 OECF
#define TAG_EXIF_VERSION			0x9000			// EXIFバージョン
#define TAG_DATETIME_ORIGINAL		0x9003			// 撮影日時
#define TAG_DATETIME_DIGITIZED		0x9004			// デジタイズ日時
#define TAG_COMPONENTS_CONFIG		0x9101			// ComponentsConfiguration
#define TAG_COMPRESSED_BPS			0x9102			// CompressedBitsPerPixel
#define TAG_SHUTTER_SPEED			0x9201			// シャッター速度
#define TAG_APERTURE				0x9202			// 絞り値
#define TAG_BRIGHTNESS_VALUE		0x9203			// 対象物の明るさ
#define TAG_EXPOSURE_BIAS			0x9204			// 露出補正値
#define TAG_MAX_APERTURE			0x9205			// レンズ開放(最小)F値
#define TAG_SUBJECT_DISTANCE		0x9206			// 被写体距離
#define TAG_METERING_MODE			0x9207			// 測光方式
													// 0:不明
													// 1:平均測光
													// 2:中央重点測光
													// 3:スポット測光
													// 4:マルチスポット測光
													// 5:分割測光
													// 6:部分測光
#define TAG_LIGHT_SOURCE			0x9208			// 光源
													//  1 = 昼光
													//  2 = 蛍光灯
													//  3 = 電球
													//  4 = フラッシュ
													//  9 = 晴天
													// 10 = 曇り
													// 11 = 日陰
													// 12 = 昼光蛍光灯(D 5700 - 7100K)
													// 13 = 昼白色蛍光灯(N 4600 - 5400K)
													// 14 = 冷白色蛍光灯(W 3900 - 4500K)
													// 15 = 白色蛍光灯(WW 3200 - 3700K)
													// 17 = 標準ライトA
													// 18 = 標準ライトB
													// 19 = 標準ライトC
													// 20 = D55
													// 21 = D65
													// 22 = D75
													// 23 = D50
													// 24 = ISOスタジオタングステン
													// 255 = その他
#define TAG_FLASH					0x9209			// フラッシュ
													// 0x00 => 'No Flash',
													// 0x01 => 'Fired',
													// 0x05 => 'Fired, Return not detected',
													// 0x07 => 'Fired, Return detected',
													// 0x09 => 'On',
													// 0x0d => 'On, Return not detected',
													// 0x0f => 'On, Return detected',
													// 0x10 => 'Off',
													// 0x18 => 'Auto, Did not fire',
													// 0x19 => 'Auto, Fired',
													// 0x1d => 'Auto, Fired, Return not detected',
													// 0x1f => 'Auto, Fired, Return detected',
													// 0x20 => 'No flash function',
													// 0x41 => 'Fired, Red-eye reduction',
													// 0x45 => 'Fired, Red-eye reduction, Return not detected',
													// 0x47 => 'Fired, Red-eye reduction, Return detected',
													// 0x49 => 'On, Red-eye reduction',
													// 0x4d => 'On, Red-eye reduction, Return not detected',
													// 0x4f => 'On, Red-eye reduction, Return detected',
													// 0x59 => 'Auto, Fired, Red-eye reduction',
													// 0x5d => 'Auto, Fired, Red-eye reduction, Return not detected',
													// 0x5f => 'Auto, Fired, Red-eye reduction, Return detected',
#define TAG_FOCAL_LENGTH			0x920A			// 焦点距離
#define TAG_SUBJECT_AREA			0x9214			// SubjectArea SHORTx2 or 3 or 4
#define TAG_MAKERNOTE				0x927C			// メーカーノート
#define TAG_USERCOMMENT				0x9286			// ユーザコメント
#define TAG_SUBSECTIME				0x9290			// SubSecTime
#define TAG_SUBSECTIME_ORIGINAL		0x9291			// SubSecTimeOriginal
#define TAG_SUBSECTIME_DIGITIZED	0x9292			// SubSecTimeDigitized
#define TAG_FPX_VERSION				0xA000			// FlashPixバージョン
#define TAG_COLOR_SPACE				0xA001			// カラースペース
#define TAG_EXIF_IMAGEWIDTH			0xA002			// EXIFイメージ幅
#define TAG_EXIF_IMAGELENGTH		0xA003			// EXIFイメージ高さ
#define TAG_INTEROP_OFFSET			0xA005			// InteroperabilityOffset
#define TAG_FLASH_ENERGY			0xA20B			// 0x920B フラッシュ強度 FlashEnergy
//									0xA20C			// 0x920C 空間周波数応答 SpatialFrequencyResponse
#define TAG_FOCALPLANE_XRES			0xA20E			// 0x920E 焦点面の幅の解像度 FocalPlaneXResolution
#define TAG_FOCALPLANE_YRES			0xA20F			// 0x920F 焦点面の高さの解像度 FocalPlaneYResolution
#define TAG_FOCALPLANE_UNITS		0xA210			// 0x9210 焦点面解像度単位 FocalPlaneResolutionUnit
#define TAG_SUBJECT_LOCATION		0xA214			// 0x9214 被写体位置 SubjectLocation
#define TAG_EXPOSURE_INDEX			0xA215			// 0x9215 CCD感度 ExposureIndex
													// データ形式が符号なし分数である事を除き、
													// ISOSpeedRatings(0x8827)と同じ
#define TAG_SENSING_METHOD			0xA217			// 0x9217 センサ方式 SensingMethod
													// 1 = 未定義
													// 2 = 1チップカラーエリアセンサ
													// 3 = 2チップカラーエリアセンサ
													// 4 = 3チップカラーエリアセンサ
													// 5 = カラーシーケンシャルエリアセンサ
													// 7 = トリリニアセンサ
													// 8 = カラーシーケンシャルリニアセンサ
#define TAG_FILE_SOURCE				0xA300			// ファイルソース FileSource
													// 3 = DSC
#define TAG_SCENE_TYPE				0xA301			// シーンタイプ SceneType
													// 1 = 直接撮影イメージ
#define TAG_CFA_PATTERN				0xA302			// UNDEF:CFAパターン
#define TAG_EXPOSURE_MODE			0xA402			// 露光モード
													// 0 = オート
													// 1 = マニュアル
													// 2 = オートブラケット
#define TAG_WHITE_BALANCE			0xA403			// ホワイトバランスモード
													// 0 = オート
													// 1 = マニュアル
#define TAG_DIGITAL_ZOOM			0xA404			// デジタルズーム RATIONAL
#define TAG_FOCALLENGTH_35			0xA405			// 35mm換算の焦点距離
													// 0 = 不明
#define TAG_SCENE_CAPTURE_TYPE		0xA406			// 撮影シーンタイプ
													// 0 = 標準
													// 1 = 風景
													// 2 = ポートレート
													// 3 = 夜景
#define TAG_GAIN_CONTROL			0xA407			// ゲインコントロール
													// 0 = なし
													// 1 = Low gain up
													// 2 = High gain up
													// 3 = Low gain down
													// 4 = High gain down
#define TAG_CONTRAST				0xA408			// コントラスト
													// 0 = 標準
													// 1 = Soft
													// 2 = Hard
#define TAG_SATURATION				0xA409			// 彩度
													// 0 = 標準
													// 1 = Low
													// 2 = High
#define TAG_SHARPNESS				0xA40A			// シャープネス
													// 0 = 標準
													// 1 = Soft
													// 2 = Hard
#define TAG_DEVICESETTING			0xA40B			// UNDEF:デバイス設置記述
#define TAG_DISTANCE_RANGE			0xA40C			// 被写体の距離範囲
													// 0 = 不明
													// 1 = マクロ
													// 2 = 近景
													// 3 = 遠景
#define TAG_IMAGE_UNIQUE_ID			0xA420			// ASCIIx33:イメージユニークID
#define TAG_GAMMA					0xA500			// RATIONAL ガンマ値

#define TAG_DNG_VERSION				50706			// BYTEx4: DNGバージョン IFD_0
#define TAG_DNG_BACKWORDVERSION		50707			// BYTEx4: DNGバージョン IFD_0
#define TAG_DNG_CAMERAMODEL_UNIQ	50708			// ASCII カメラモデル名
#define TAG_DNG_CAMERAMODEL_LOCAL	50709			// ASCII カメラモデル名(UTF-8)
#define TAG_DNG_COLOR_MATRIX1		50721			// ColorMatrix1
#define TAG_DNG_COLOR_MATRIX2		50722			// ColorMatrix2
#define TAG_DNG_CAMERA_CALIB1		50723			// CameraCalibration1
#define TAG_DNG_CAMERA_CALIB2		50724			// CameraCalibration2
#define TAG_DNG_REDUCT_MATRIX1		50725			// ReductionMatrix1
#define TAG_DNG_REDUCT_MATRIX2		50726			// ReductionMatrix2
#define TAG_DNG_ANALOG_BALANCE		50727			// AnalogBalance
#define TAG_DNG_CAMERA_SERIALNUM	50735			// ASCII カメラのシリアルナンバ
#define TAG_DNG_LENSINFO			50736			// RATIONALx4 レンズ情報
													// index0: Min focal length in mm.
													// index1: Max focal length in mm.
													// index2: Min (max aperture) f-stop at min focal length.
													// index3: Min (max aperture) f-stop at max focal length.
#define TAG_DNG_PRIVATE_DATA		50740			// BYTE
#define TAG_DNG_MAKERNOTE_SAFTY		50741			// SHORT

#define TAG_DNG_CALIB_ILLUMINANT1	50778			// SHORT CalibrationIlluminant1
#define TAG_DNG_CALIB_ILLUMINANT2	50779			// SHORT CalibrationIlluminant2

	// 以下 RawIFDに記録される
#define TAG_DNG_CFA_PLANECOLOR		50710			// BYTE Count ColorPlanes
#define TAG_DNG_CFA_LAYOUT			50711			// SHORT CFALayout
													// 1 = Rectangular (or square) layout
													// 2 = Staggered layout A: even columns are offset down by 1/2 row
													// 3 = Staggered layout B: even columns are offset up by 1/2 row
													// 4 = Staggered layout C: even rows are offset right by 1/2 column
													// 5 = Staggered layout D: even rows are offset left by 1/2 column
#define TAG_DNG_LINEARIZATIONTABLE	50712			// SHORTxN LinearizationTable
#define TAG_DNG_BLACKLEVELREPEATDIM	50713			// SHORTx2 BlackLevelRepeatDim
#define TAG_DNG_BLACKLEVEL			50714			// BlackLevel
#define TAG_DNG_BLACKLEVEL_DELTAH	50715			// BlackLevelDeltaH
#define TAG_DNG_BLACKLEVEL_DELTAV	50716			// BlackLevelDeltaV
#define TAG_DNG_WHITELEVEL			50717			// WhiteLevel
#define TAG_DNG_DEFAULT_SCALE		50718			// DefaultScale
#define TAG_DNG_BESTQUALITY_SCALE	50719			// BestQualityScale



////////////////////////////////////////////////////////////////////////
// gpsに関するIFD

enum {
	GPSVersionID 		= 0,						// BYTEx4: GPS タグのバージョン
	GPSLatitudeRef		= 1,						// ASCIIx2: 北緯(N)or 南緯(S) 
	GPSLatitude			= 2,						// RATIONALx3: 緯度（数値）
	GPSLongitudeRef		= 3,						// ASCIIx2: 東経(E)or 西経(W)
	GPSLongitude		= 4,						// RATIONALx3: 経度（数値）
	GPSAltitudeRef		= 5,						// BYTEx1: 高度の単位
	GPSAltitude			= 6,						// RATIONALx1: 高度（数値）
	GPSTimeStamp		= 7,						// RATIONALx3: GPS 時間（原子時計の時間）
	GPSSatellites		= 8,						// ASCIIxN: 測位に使った衛星信号
	GPSStatus			= 9,						// ASCIIx2: GPS 受信機の状態
	GPSMeasureMode		= 10,						// ASCIIx2: GPS の測位方法
	GPSDOP				= 11,						// RATIONALx1: 測位の信頼性
	GPSSpeedRef			= 12,						// ASCIIx2: 速度の単位
	GPSSpeed			= 13,						// RATIONALx1: 速度（数値）
	GPSTrackRef			= 14,						// ASCIIx2: 進行方向の単位
	GPSTrack			= 15,						// RATIONALx1: 進行方向（数値）
	GPSImgDirectionRef	= 16,						// ASCIIx2: 撮影した画像の方向の単位
	GPSImgDirection		= 17,						// RATIONALx1: 撮影した画像の方向（数値）
	GPSMapDatum			= 18,						// ASCIIxN: 測位に用いた地図データ
	GPSDestLatitudeRef	= 19,						// ASCIIx2: 目的地の北緯(N)or 南緯(S)
	GPSDestLatitude		= 20,						// RATIONALx3: 目的地の緯度（数値）
	GPSDestLongitudeRef	= 21,						// ASCIIx2: 目的地の東経(E)or 西経(W)
	GPSDestLongitude	= 22,						// RATIONALx3: 目的地の経度（数値）
	GPSDestBearingRef	= 23,						// ASCIIx2: 目的地の方角の単位
	GPSDestBearing		= 24,						// RATIONALx1: 目的の方角（数値）
	GPSDestDistanceRef	= 25,						// ASCIIx2: 目的地までの距離の単位
	GPSDestDistance		= 26,						// RATIONALx1: 目的地までの距離（数値）
	GPSProcessingMethod = 27,						// UNDEFxN: Name of GPS processing method
	GPSAreaInformation	= 28,						// UNDEFxN: Name of GPS area
	GPSDateStamp		= 29,						// ASCIIx11: GPS date
	GPSDefferential		= 30,						// SHORTx1: GPS differential correction
};

#endif		// __EXIFTAG_H__

