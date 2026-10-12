//////////////////////////////////////////////////////////////////
// filter.cpp

#include "stdafx.h"
#include "resource.h"

PROGRESSCALLBACK pfnProgress = NULL;

HANDLE video_noise_ui(HDIB hdib, LPRECT prc, FILTER_EXEC* pfe);
HANDLE tv_ghost_ui(HDIB hdib, LPRECT prc, FILTER_EXEC* pfe);
HANDLE noise_dusty_ui(HDIB hdib, LPRECT prc, FILTER_EXEC* pfe);
HANDLE noise_mixer_ui(HDIB hdib, LPRECT prc, FILTER_EXEC* pfe);
HANDLE noise_whiten_ui(HDIB hdib, LPRECT prc, FILTER_EXEC* pfe);
HANDLE noise_bandsuppress_ui(HDIB hdib, LPRECT prc, FILTER_EXEC* pfe);
HANDLE noise_blackstain_ui(HDIB hdib, LPRECT prc, FILTER_EXEC* pfe);
HANDLE noise_interlace_ui(HDIB hdib, LPRECT prc, FILTER_EXEC* pfe);
HANDLE noise_grayscale_ui(HDIB hdib, LPRECT prc, FILTER_EXEC* pfe);
HANDLE noise_background_ui(HDIB hdib, LPRECT prc, FILTER_EXEC* pfe);

typedef struct tagFILTER_ENTRY {
	LPCTSTR title;
	LPCTSTR group;
	UINT icon;
	SIZE dim;
	int style;
	HANDLE (*filter)(HDIB, LPRECT, FILTER_EXEC*);
} FILTER_ENTRY;

static const FILTER_ENTRY fen[] = {
	{ _T("ビデオ ノイズ(&V)"),		LPSTR_GID_NOISE, IDB_VIDEONOISE, {20,20}, 1,	video_noise_ui },
	{ _T("TV ゴースト(&T)"),		LPSTR_GID_NOISE, IDB_TVGOAST, {20,20}, 1,	tv_ghost_ui },
	{ _T("埃っぽさ(&D)"),			LPSTR_GID_NOISE,	0, {20,20}, 0, noise_dusty_ui },
	{ _T("帯域制御(&B)"),			LPSTR_GID_NOISE,	0, {20,20}, 0, noise_bandsuppress_ui },
	{ _T("ミキサー(&M)"),			LPSTR_GID_NOISE,	0, {20,20}, 0, noise_mixer_ui },
	{ _T("漂白(&W)"),				LPSTR_GID_NOISE,	0, {20,20}, 0, noise_whiten_ui },
	{ _T("ブラック ステイン(&L)"),	LPSTR_GID_NOISE,	0, {20,20}, 0, noise_blackstain_ui },
	{ _T("インタレース(&I)"),		LPSTR_GID_NOISE,	0, {20,20}, 0, noise_interlace_ui },
	{ _T("グレイスケール(&G)"),		LPSTR_GID_NOISE, IDB_GRAYSCALE,  {20,20}, 1, noise_grayscale_ui },
	{ _T("背景ノイズ(&A)"),			LPSTR_GID_NOISE,	0, {20,20}, 0, noise_background_ui },
};

static const int numFilter = sizeof(fen) / sizeof(FILTER_ENTRY);

//////////////////////////////////////////////////////////////////
//

extern "C"
BOOL WINAPI ipInitialize(PLUGIN_INIT* init)
{
	init->verMajor = 1;
	init->verMinor = 0;
	init->numFilters = numFilter;
	return TRUE;
}

extern "C"
BOOL WINAPI ipFilterInfo(int iNumber, FILTER_INFO* info)
{
	if (iNumber < 0 || iNumber >= numFilter) return FALSE;

	const FILTER_ENTRY* pfe = &fen[iNumber];
	if (FilterInfoVersion(info) > 1) {
		info->flags = PLUG_ALLOW_24BPP;
		lstrcpy(info->strFilterName, pfe->title);
		lstrcat(info->strFilterName, _T("..."));
		info->groupName = pfe->group;
		info->idIcon = pfe->icon;
		info->sizeIcon = pfe->dim;
		info->style = pfe->style;
	}
	else {
		FILTER_INFO_V1* v1 = (FILTER_INFO_V1*)info;
		v1->flags = PLUG_ALLOW_24BPP;
		lstrcpy(v1->strFilterName, pfe->title);
		lstrcat(v1->strFilterName, _T("..."));
		v1->groupName = pfe->group;
	}
	return TRUE;
}

extern "C"
HANDLE WINAPI ipFilterExecute(int iNumber, HANDLE hdib, LPRECT prc, FILTER_EXEC* pfe)
{
	if (!hdib || iNumber < 0 || iNumber >= numFilter) return NULL;

	pfe->param = (void*)fen[iNumber].group;
	return (*fen[iNumber].filter)(hdib, prc, pfe);
}


////////////////////////////////////////////////////////
//

void progress_init(int maxprogress)
{
	if (pfnProgress)
		(*pfnProgress)(PROG_INITIALIZE, maxprogress);
}

void progress_continue()
{
	if (pfnProgress)
		(*pfnProgress)(PROG_CONTINUE, 0);
}

void progress_term()
{
	if (pfnProgress)
		(*pfnProgress)(PROG_DONE, 0);
}

