///////////////////////////////////////////////////////////////////////
// makernote_minolta.h minolta情報

#pragma once
#ifndef __MAKERNOTE_MINOLTA_H__
#define __MAKERNOTE_MINOLTA_H__

///////////////////////////////////////////////////////////////////////
// MINOLTAモデル

enum {
	MINOLTA_UNKNOWN = 0,
	MINOLTA_DIMAGE = 0x100,			// DiMAGE Series
	MINOLTA_DA1,					// "DiMAGE A1"
	MINOLTA_DA2,					// "DiMAGE A2"
	MINOLTA_DA200,					// "DiMAGE A2"
	MINOLTA_DZ1,					// "DiMAGE Z1"
	MINOLTA_DZ2,					// "DiMAGE Z2"
	MINOLTA_DZ3,					// "DiMAGE Z3"
	MINOLTA_ALPHA = 0x1000,			// ALPHA Series
	MINOLTA_ALPHA_UNKNOWN,
	MINOLTA_ALPHA_7D,				// "ALPHA-7 DIGITAL" or "DYNAX..." or "MAXXUM..."
	MINOLTA_ALPHA_SWEET,			// "ALPHA SWEET DIGITAL"
};

#define IS_MINOLTA_ALPHA(n)			((n)>MINOLTA_ALPHA)

///////////////////////////////////////////////////////////////////////
//

typedef struct __minolta_t {
								// メーカーノート内のTAGは 0x0001(old) または 0x0003(new)
								// 各項目はshort値配列
	int ExpMode;				//  2:露出制御モード
										// 0 - P mode
										// 1 - A mode
										// 2 - S mode
										// 3 - M mode
	int FlashMode;				//  3:フラッシュモード
										// 0 - fill-flash
										// 1 - red-eye reduction
										// 2 - rear flash sync
										// 3 - wireless
	int WhiteBalance;			//  4:ホワイトバランス
										// 0 - auto
										// 1 - daylight
										// 2 - cloudy
										// 3 - tungsten
										// 5 - custom
										// 7 - fluorescent
										// 8 - fluorescent 2
										// 11 - custom 2
										// 12 - custom 3
	int WhiteBalanceTune;		//  4: LOBYTE(HIWORD(WhiteBalance)) ホワイトバランスの微調整
	int ImageSize;				//  5:イメージサイズ
										// 0 - 2560 x 1920 (2048x1536)
										// 1 - 1600 x 1200
										// 2 - 1280 x 960
										// 3 - 640 x 480
										// # For D5 the value 0 means 2048x1536.
										// # RAW picture could have here 640x480 but
										// # it has no effect, it is still 2560x1920.
	int ImageQuality;			//  6:イメージ品質
										// 0 - raw
										// 1 - superfine
										// 2 - fine
										// 3 - standard
										// 4 - economy
										// 5 - extra fine
	int DriveMode;				//  7:ドライブモード
										// 0 - single
										// 1 - continuous
										// 2 - self-timer
										// 4 - bracketing
										// 5 - interval
										// 6 - UHS continuous
										// 7 - HS continuous
	int MeteringMode;			//  8:測光モード
										// 0 - multi-segment
										// 1 - center weighted
										// 2 - spot
										// # I have seen values 0x0100 and 0x0102
										// # and I do not know the meaning of them
										// # (all were taken with flash).
	int FilmSpeed;				//  9:フィルムスピード APEX
										// Speed value = x/8-1
										// ISO = (2^(x/8-1))*3.125
	int ShutterSpeed;			// 10:シャッター速度 APEX
										// Time value = x/8-6
										// ShutterSpeed = 2^( (48-x)/8 )
										// # ! Due to rounding error x=8 should be
										// # displayed as 30 sec.
	int Aperture;				// 11:絞り値 APEX
										// ApertureValue = x/8-1
										// Aperture = 2^( x/16-0.5 )
	int MacroMode;				// 12:マクロモード
										// 0 - off
										// 1 - on
	int DigitalZoom;			// 13:デジタルズーム
										// 0 - off
										// 1 - electronic magnification was used
										// 2 - digital zoom 2x
										// 3 - Unknown meaning
										// # I have found some pictures with
										// # this tag equal to three.
										// # I do not know what it means.
	int ExpCompensation;		// 14:露出補正 EV
										// EV = x/3 -2
										// Exposure compensation in EV
	int BracketStep;			// 15:ブラケットステップ
										// 0 - 1/3 EV
										// 1 - 2/3 EV
										// 2 - 1 EV
	int FocalLength;			// 19:焦点距離
										// x / 256 is real focal length in mm
										// x / 256 * 3.9333 is 35-mm equivalent
										// # This value is not equal to FocalLength
										// # in public EXIF as the public one is
										// # tweaked to get 7x zoom to comply with
										// # marketing specification.
										// # The real zoom is 6.51x.
	int FocusDistance;			// 20:フォーカス距離
										// x / 1000 is focus distance in meters
										// 0 - focused at infinity
	int FlashFired;				// 21:フラッシュ発光
										// 0 - no
										// 1 - yes
	int Date;					// 22:日付
										// yyyymmdd
										// year = x/65536
										// month = x/256-x/65536*256
										// day = x%256
	int Time;					// 23:時間
										// hhhhmmss
										// hour = x/65536
										// minute = x/256-x/65536*256
										// second = x%256
	int MaxAperture;			// 24:開放F値
										// Max Aperture available with this focal length
										// Fno = 2^(x/16-0.5)
										// Break point is focal length (45 mm or 11.44 mm)
	int WhiteBalanceRed;		// 29:赤WB
										// x/256 - red white balance coefficient used for this picture
										// Sometimes could be zero even if there is some red in the picture. Strange...
										// Range (/256): 0 and 0.77-2.31 (in my test data)
	int WhiteBalanceGreen;		// 30:緑WB
										// x/256 - green white balance coefficient used for this picture
										// Range (/256): always 1
	int WhiteBalanceBlue;		// 31:青WB
										// x/256 - blue white balance coefficient used for this picture
										// Range (/256): 1.08-3.96 (in my test data)
	int Saturation;				// 32:彩度
										// x-3 - saturation
										// Range: 0-6
	int Contrast;				// 33:コントラスト
										// x-3 - contrast
										// Range: 0-6
										// # I have seen the value 0x000c here also.
										// # What could it mean?
	int Sharpness;				// 34:シャープネス
										// 0 - hard
										// 1 - normal
										// 2 - soft
	int SubjectProgram;			// 35:シーンセレクタ
										// 0 - none
										// 1 - portrait
										// 2 - text
										// 3 - night portrait
										// 4 - sunset
										// 5 - sports action
	int FlashComp;				// 36:フラッシュ補正
										// (x-6)/3 - flash compensation in EV
										// Range: 0-12
	int ISOSetting;				// 37:ISOセッティング
										// 0 - 100
										// 1 - 200
										// 2 - 400
										// 3 - 800
										// 4 - auto
										// 5 - 64
	int CameraModel;			// 38:カメラモデル
										// 0 - DiMAGE 7
										// 1 - DiMAGE 5
										// 2 - DiMAGE S304
										// 3 - DiMAGE S404
										// 4 - DiMAGE 7i
										// 5 - DiMAGE 7Hi
	int IntervalMode;			// 39:インターバルモード
										// 0 - still image
										// 1 - time-lapse movie
										// # I am not so sure now here.
	int ColorMode;				// 41:カラーモード
										// 0 - natural color
										// 1 - black&white
										// 2 - vivid color
										// 3 - solarization
										// 4 - AdobeRGB
										// 9 - Natural sRGB
	int ColorFilter;			// 42:カラーフィルター
										// x-3 - color filter
										// Range: 0-6
	int BWFilter;				// 43:モノクロフィルター
										// Range: 0-10
	int InternalFlash;			// 44:内蔵フラッシュ
										// 0 - no
										// 1 - fired
	int BrightnessValue;		// 45:輝度 APEX
										// Brightness Value = x/8-6
										// Range: from -20.25 to 13.5
	int SpotFocusPointX;		// 46:スポットフォーカスX位置
										// in range 0-2560
	int SpotFocusPointY;		// 47:スポットフォーカスY位置
										// in range 0-1920 
	int WideFocusZone;			// 48:ワイドフォーカスゾーン
										// 0 - no zone
										// 1 - center zone (horizontal orientation)
										// 2 - center zone (vertical orientation)
										// 3 - left zone
										// 4 - right zone
										// # 0 with AF means that AF failed to lock and
										// # the picture is probably out-of-focus.
	int FocusMode;				// 49:フォーカスモード
										// 0 - AF
										// 1 - MF
	int FocusFrame;				// 50:フォーカスフレーム
										// 0 - wide focus (normal way)
										// 1 - spot focus
	int DECPosition;			// 51:DECポジション
										// 0 - exposure
										// 1 - contrast
										// 2 - saturation
										// 3 - filter
										// # It stores the position of DEC switch.
										// # This value very useful when DriveMode is
										// # set to Bracketing to determine what the user was bracketing.
	int AntiShake;				// 53: 手ぶれ補正
										// 0 - オフ
										// 1 - オン

	int ZoneMatching;			// 0x010A ULONG:ゾーン切替
										// 0 - オフ
										// 1 - HIGH
										// 2 - LOW
	int ColorTemp;				// 0x010B ULONG:色温度
	int LensInfo;				// 0x010C ULONG:レンズ情報
										// 7 - 100-300mmF4.5-5.6(D)
										// 14 - 100-400mmF4.5-6.7
										// 16 - 17-35mmF3.5G
										// 19 - 35mmF1.4G
										// 20 - STF135mmF2.8
										// 24 - 24-105mmF3.5-4.5(D)
										// 27 - 85mmF1.4G(D)
										// 31 - 50mmF2.8(D) MACRO
										// 33 - 70-200mmF2.8G(D) SSM
										// 38 - 17-35mmF2.8-4(D)
										// 39 - 28-75mmF2.8(D)
										// 128 - TAMRON28-200mmF3.8-5.6
										// 128 - TAMRON28-300mmF3.5-6.3
										// 25501 - 50mmF1.7
										// 25521 - ?? 35-105mm
										// 25571 - 28mmF2.8
										// 25621 - 50mmF1.4(D)
										// 25631 - SIGMA50-500mmF4-6.3EX RF
										// 25641 - Sigma50mmF2.8EX DG MACRO
										// 25721 - 500mmF8 RF
										// 25791 - 20mmF2.8
										// 25811 - 100mmF2.8(D) MACRO
										// 25858 - TAMRON24-135mmF3.5-5.6
										// 25921 - 85mmF1.4G(D)
										// 26121 - 200mmF4G
										// 26161 - 35-200mmF4.5-5.6Xi

// 以下ALPHA-7D
	int DMF;					// AF-A時のDMF
	int LocalFocusFrame;		// ローカルフォーカスフレーム
	int FlashControl;			// 調光モード
	int Hue;					// 色合い
	int MeteredManual;			// メータードマニュアル
	int FocusRelease;			// フォーカス／レリーズ優先
	int ColorSpace;				// カラースペース

// 以下ALPHA-SWEET-D
	int ColorCompFilter;		// CC(色補正)フィルター M9(0x12) - STD(0x09) - G9(0x00)
} minolta_t;



#endif		// __MAKERNOTE_MINOLTA_H__
