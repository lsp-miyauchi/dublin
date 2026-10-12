///////////////////////////////////////////////////////////////////////
// makernote_sanyo.h sanyo情報

#pragma once
#ifndef __MAKERNOTE_SANYO_H__
#define __MAKERNOTE_SANYO_H__


typedef struct __sanyo_t {
								// 0x0100 UNDEF:サムネイル情報
								// 0x0200 LONGx3:不明
		int Quality;			// 0x0201 SHORT:画質モード
										// Hi-Byte 0: normal
										//         1: fine
										//         2: super fine
										// Lo-Byte 0: very low
										//         1: low
										//         2: medium low
										//         3: medium
										//         4: medium high
										//         5: high
										//         6: very high
										//         7: super high
		int ShootMode;			// 0x0202 SHORT:撮影モード
										// 0 = 通常
										// 1 = マクロ
										// 2 = 遠景
										// 3 = マニュアル
								// 0x0203 SHORT:不明
		float DigitalZoom;		// 0x0204 RATIN:デジタルズーム
		int ContShoot;			// 0x020E SHORT:連写モード
										// 0 = オフ
										// 1 = 高速連写
										// 2 = ベストセレクト
										// 3 = オートブラケット
		int WideRangeShoot;		// 0x020F SHORT:ワイドレンジショット
										// 0 = オフ
										// 1 = オン
		int RealColorEqualizer;	// 0x0210 SHORT:リアルカラーイコライザ
										// 0 = オフ
										// 1 = オン
								// 0x0211 SHORT:不明
								// 0x0212 SHORT:不明
		int QuickShot;			// 0x0213 SHORT:クイックショット
										// 0 = オフ
										// 1 = オン
		int SelfTimer;			// 0x0214 SHORT:セルフタイマ
										// 0 = オフ
										// 1 = オン
								// 0x0215 SHORT:不明
		int VoiceMemo;			// 0x0216 SHORT:ボイスメモ
										// 0 = オフ
										// 1 = オン
								// 0x0217 SHORT:不明
		int FlickerReduce;		// 0x0218 SHORT:フリッカー軽減
										// 0 = オフ
										// 1 = オン
		int OpticalZoomUsing;	// 0x0219 SHORT:光学ズームの利用
										// 0 = 禁止
										// 1 = 許可
		int FocusMode;			// 0x021A SHORT:フォーカスモード
										// 0 = ノーマル
										// 1 = 不明
										// 2 = 不明
										// 3 = スーパーマクロ
		int DigitalZoomUsing;	// 0x021B SHORT:デジタルズームの利用
										// 0 = 禁止
										// 1 = 許可
								// 0x021C SHORT:不明
								// 0x021D SHORT:不明
								// 0x021E SHORT:不明
								// 0x021F SHORT:不明
		int FlashMode;			// 0x0225 USHRT:フラッシュモード
										// 0 = オート
										// 1 = 強制
										// 2 = 禁止
										// 3 = 赤目軽減
								// 0x0F00 UNDEF:不明
} sanyo_t;


#endif		// __MAKERNOTE_SANYO_H__
