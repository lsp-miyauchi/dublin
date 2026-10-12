///////////////////////////////////////////////////////////////////////
// makernote_mamiya.h mamiya情報

#pragma once
#ifndef __MAKERNOTE_MAMIYA_H__
#define __MAKERNOTE_MAMIYA_H__

///////////////////////////////////////////////////////////////////////
// MAMIYAモデル

enum {
	MAMIYA_UNKNOWN = 0,
	MAMIYA_ZD = 0x100,				// "MAMIYA ZD"
};

///////////////////////////////////////////////////////////////////////
//

typedef struct __mamiya_t {
	int ExposureMode;						// 0x0090 USHRT: 露出モード
													// 0x02=マニュアル, 0x08=絞り優先AE, 0x10=オート
	int ISO;								// 0x0095 USHRT: ISO値
													// 0=オート, 50～400がそのままISO値
	int ColorMode;							// 0x0098 USHRT: カラーモード
													// 0=スタンダード, 1=ポートレイト, 2=風景, 3=商品
	int WhiteBalance;						// 0x00A2 USHRT: ホワイトバランス
													// 0=オート, 2=曇り, 3=太陽光, 4=ストロボ, 6=電球, 52=?, 55=?
	int Tone;								// 0x00A5 USHRT: トーン
													// 1=ノーマル, 2=LOW
} mamiya_t;


#endif		// __MAKERNOTE_MAMIYA_H__
