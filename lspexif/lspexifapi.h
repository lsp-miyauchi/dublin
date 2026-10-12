////////////////////////////////////////////////////////////////////////
// lspexifapi.h

#pragma once
#ifndef __LSEXIFAPI_H__
#define __LSEXIFAPI_H__

#include "lspexif.h"

////////////////////////////////////////////////////////////////////////
//

typedef int (CALLBACK* EXIFCBPROC)(int type, LPCTSTR pszTitle, LPCTSTR pszText, LPARAM param);

enum {
	EXIFCB_GROUP = 1,
	EXIFCB_ITEM,
};

////////////////////////////////////////////////////////////////////////
//
enum {
	ORIENTATION_NONE = 1,
	ORIENTATION_FLIPX = 2,
	ORIENTATION_ROTATE180 = 3,
	ORIENTATION_FLIPY = 4,
	ORIENTATION_ROTATELEFTFLIPY = 5,
	ORIENTATION_ROTATERIGHT = 6,
	ORIENTATION_ROTATERIGHTFLIPY = 7,
	ORIENTATION_ROTATELEFT = 8,
};

#pragma pack(1)

typedef struct tagEXIFINFOSTRUCT_V1 {
	TCHAR ModelName[32];				// モデル名
	SYSTEMTIME DateTime;				// 撮影日時
	int ImageWidth;						// イメージ幅
	int ImageHeight;					// イメージ高さ
	int ExposureProgram;				// 露出制御モード
	float FocalLength;					// 焦点距離
	float FocalLength35;				// 焦点距離(35mm換算)
	float ExposureTime;					// 露出時間
	float ExposureBias;					// 露出補正
	float Aperture;						// 絞り
	int ISOEquiv;						// ISO感度 (0 for Auto)
	BOOL FlashFired;					// フラッシュの使用
	LPTSTR pszTipText;					// チップ情報バッファ
	int cchTipText;						// チップ情報バッファ長
	LPTSTR pszLabelText;				// ラベルテキストバッファ
	int cchLabelText;					// ラベルテキストバッファ長
	int Orientation;					// 画像の向き
	DWORD padding[4];
} EXIFINFOSTRUCT_V1;

typedef struct tagEXIFINFOSTRUCT {
	TCHAR ModelName[32];				// モデル名
	SYSTEMTIME DateTime;				// 撮影日時
	int ImageWidth;						// イメージ幅
	int ImageHeight;					// イメージ高さ
	int ExposureProgram;				// 露出制御モード
	float FocalLength;					// 焦点距離
	float FocalLength35;				// 焦点距離(35mm換算)
	float ExposureTime;					// 露出時間
	float ExposureBias;					// 露出補正
	float Aperture;						// 絞り
	int ISOEquiv;						// ISO感度 (0 for Auto)
	BOOL FlashFired;					// フラッシュの使用
	LPTSTR pszTipText;					// チップ情報バッファ
	int cchTipText;						// チップ情報バッファ長
	LPTSTR pszLabelText;				// ラベルテキストバッファ
	int cchLabelText;					// ラベルテキストバッファ長
	int Orientation;					// 画像の向き
	DWORD padding[4];
	// ---------------------
	// 追加
	// ---------------------
	int MeteringMode;					// 測光モード(EXIF)
	int WhiteBalance;					// ホワイトバランス(EXIF)
	float MaxAperture;					// 最大絞り(EXIF)
	TCHAR MakerName[32];				// メーカー名(EXIF)
	TCHAR Lens[64];						// レンズ名称(MAKER_NOTE)
	TCHAR WhiteBalanceText[20];			// ホワイトバランス文字列(MAKER_NOTE)
} EXIFINFOSTRUCT;

#pragma pack()


////////////////////////////////////////////////////////////////////////
//

#ifdef __cplusplus
extern "C" {                    // Assume C declarations for C++
#endif

//----- ヘッダで公開しない
//@ BOOL WINAPI ExifGetThumbsnail(LPCTSTR fname, IStream* pstrm, LPTSTR tip, int cchMax);
//@ BOOL WINAPI ExifGetPrimaryImage(LPCTSTR fname, IStream* pstrm);
//@ BOOL WINAPI ExifGetPrimaryImageEx(LPCTSTR fname, IStream* pstrm, int* pnOrientation);
//@ BOOL WINAPI ExifGetInfoTip(LPCTSTR fname, LPTSTR tip, int cchMax);
//@ void WINAPI ExifDrawDigiText(HDC hdc, LPCTSTR pszText, int cchMax, RECT* prc, UINT flags);
//@ int WINAPI ExifEnumInfo(LPCTSTR fname, EXIFCBPROC proc, LPARAM param);
//@ BOOL WINAPI ExifIsExifFile(LPCTSTR fname);
//@ BOOL WINAPI ExifGetDateTimeString(LPCTSTR fname, LPTSTR date, int cchMax);
//@ BOOL WINAPI ExifGetExifInfo(LPCTSTR fname, IStream* pstrm, EXIFINFOSTRUCT_V1* info);
//@ BOOL WINAPI ExifGetExifInfoEx(LPCTSTR fname, IStream* pstrm, int* pnOrientation, BOOL bLargeImage, EXIFINFOSTRUCT* info);
//@ BOOL WINAPI ExifGetThumbnailImage(LPCTSTR fname, IStream* pstrm, int* pnOrientation, BOOL bLargeImage, LPTSTR tip, int cchMax);


#ifdef __cplusplus
}
#endif

#endif			// __LSPEXIFAPI_H__
