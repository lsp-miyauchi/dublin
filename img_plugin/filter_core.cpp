//////////////////////////////////////////////////////////////////
// filter_core.cpp: 各フィルタライブラリで取り込まれる
//

PROGRESSCALLBACK pfnProgress = NULL;

//////////////////////////////////////////////////////////////////
//

static const int numFilter = ARRAYSIZE(fen);

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

	const FILTER_ENTRY_EX* pfe = &fen[iNumber];
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
