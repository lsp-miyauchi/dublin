//////////////////////////////////////////////////////////////////////
// plugin.h

#pragma once
#ifndef __PIXEF_PLUGIN_INCLUDE
#define __PIXEF_PLUGIN_INCLUDE

#define PLUG_ALLOW_1BPP			0x00000001
#define PLUG_ALLOW_4BPP			0x00000002
#define PLUG_ALLOW_8BPP			0x00000004
#define PLUG_ALLOW_24BPP		0x00000008
#define PLUG_ALLOW_RECT			0x00000010

#define PLUGIN_EXTENSION		_T("*.7ip")
#define PLUGIN_SECTION			_T("plugins")				// subdir & registery
#define PLUGIN_FILTER_INIT		"ipInitialize"
#define PLUGIN_FILTER_INFO		"ipFilterInfo"
#define PLUGIN_FILTER_EXEC		"ipFilterExecute"

//////////////////////////////////////////////////
// 進捗コールバック

#ifndef PROGRESSCALLBACK
	typedef BOOL (CALLBACK *PROGRESSCALLBACK)(int, int);
	#define PROG_INITIALIZE		0
	#define PROG_CONTINUE		1
	#define PROG_DONE			2
	#define PROG_RESET			3
	#define PROG_ERROR			-1
#endif

//////////////////////////////////////////////////
// グラデーション選択

#define MAX_GRADCOLORS			16

typedef struct tagGRADEDITINFO {
	int numcolors;						// 
	GRADCOLORS grad[MAX_GRADCOLORS];	// 
} GRADEDITINFO;

typedef BOOL (CALLBACK *GRADEDITPROC)(HWND hwndParent, GRADEDITINFO* pgsi);

//////////////////////////////////////////////////
// ポジショントラッキングコールバック

typedef BOOL (CALLBACK *POSTRACKPROC)(HDIB hdibTrack, LPPOINT ppt, COLORREF clrMask);

//////////////////////////////////////////////////
//　フィルタグループ定義

enum {
	gidArtistic = 1,
	gidColor,
	gidSharpen,
	gidShading,
	gidBlur,
	gidStroke,
	gidMosaic,
	gidTexture,
	gidTrasnform,
	gidExpression,
	gidFrame,
	gidEdgeDetect,
	gidNoise,
	gidPaint,
	gidString,
	gidRotate,
	gidWriteout,
	gidMontage,
	gidGlass,
	gidTone,
	gidOther,
};

#define LPSTR_GID_ARTISTIC		(LPTSTR)gidArtistic
#define LPSTR_GID_SHARPEN		(LPTSTR)gidSharpen
#define LPSTR_GID_COLOR			(LPTSTR)gidColor
#define LPSTR_GID_SHADING		(LPTSTR)gidShading
#define LPSTR_GID_BLUR			(LPTSTR)gidBlur
#define LPSTR_GID_STROKE		(LPTSTR)gidStroke
#define LPSTR_GID_MOSAIC		(LPTSTR)gidMosaic
#define LPSTR_GID_TEXTURE		(LPTSTR)gidTexture
#define LPSTR_GID_TRANSFORM		(LPTSTR)gidTrasnform
#define LPSTR_GID_EXPRESSION	(LPTSTR)gidExpression
#define LPSTR_GID_FRAME			(LPTSTR)gidFrame
#define LPSTR_GID_EDGEDETECT	(LPTSTR)gidEdgeDetect
#define LPSTR_GID_NOISE			(LPTSTR)gidNoise
#define LPSTR_GID_PAINT			(LPTSTR)gidPaint
#define LPSTR_GID_STRING		(LPTSTR)gidString
#define LPSTR_GID_ROTATE		(LPTSTR)gidRotate
#define LPSTR_GID_WRITEOUT		(LPTSTR)gidWriteout				// [ファイル]メニューの[形式を指定して書き出し]
#define LPSTR_GID_MONTAGE		(LPTSTR)gidMontage
#define LPSTR_GID_GLASS			(LPTSTR)gidGlass
#define LPSTR_GID_TONE			(LPTSTR)gidTone					// [イメージ]メニューの[色調の変更]
#define LPSTR_GID_OTHER			(LPTSTR)gidOther

//////////////////////////////////////////////////
// フィルタ初期化

typedef struct tagPLUGIN_INIT {
	TCHAR strCopyright[160];
	unsigned short verMajor;
	unsigned short verMinor;
	int numFilters;
} PLUGIN_INIT;

typedef BOOL (WINAPI *PLUGINITPROC)(PLUGIN_INIT*);

//////////////////////////////////////////////////
// フィルタ情報

typedef struct tagFILTER_INFO_V1 {
	TCHAR strFilterName[80];
	UINT flags;
	LPCTSTR groupName;
	UINT idIcon;				// icon id
} FILTER_INFO_V1;

typedef struct tagFILTER_INFO {
	DWORD magic;
	TCHAR strFilterName[80];
	UINT flags;
	LPCTSTR groupName;
	UINT idIcon;				// image id
	SIZE sizeIcon;				// image dimension
	int style;					// image type 0=icon, 1=png
} FILTER_INFO;

typedef BOOL(WINAPI* FILTERINFOPROC)(int, FILTER_INFO*);

_inline int FilterInfoVersion(const FILTER_INFO* info)
{
	return (info->magic == sizeof(FILTER_INFO)) ? 2 : 1;
}

_inline void ConvertFilterInfoStruct(FILTER_INFO_V1& v1, FILTER_INFO& v2)
{
	v2.magic = sizeof(FILTER_INFO);
	memcpy(&v2.strFilterName[0], &v1, sizeof(v1));
	v2.sizeIcon = { 16, 16 };
	v2.style = 0;
}


//////////////////////////////////////////////////
// フィルタ実行

typedef struct tagFILTER_EXEC {
	PROGRESSCALLBACK progress;
	HKEY hkey;
	HWND hparent;
	void* param;					// プラグイン固有で自由にどうぞ
	LPCTSTR name;					// 画像ファイル名
	int scale;						// 現在のスケール (x1000)
	POSTRACKPROC tracker;			// ポジショントラッカー
	GRADEDITPROC gradeditor;		// グラデーション
	LPCTSTR userdir;				// ユーザデータフォルダ
} FILTER_EXEC;

typedef HANDLE (WINAPI *FILTERPROC)(int, HANDLE, LPRECT, FILTER_EXEC*);

typedef HANDLE (*FILTERCALLBACK)(HDIB, LPRECT, FILTER_EXEC*);

typedef struct tagFILTER_ENTRY_EX {
	LPCTSTR title;
	LPCTSTR group;
	UINT icon;
	SIZE dim;
	int style;
	FILTERCALLBACK filter;
} FILTER_ENTRY_EX;


#endif		// __PIXEF_PLUGIN_INCLUDE
