//////////////////////////////////////////////////////////////////////
// mainmodule.h

#pragma once
#ifndef __MAINMODULE_H__
#define __MAINMODULE_H__

#include "lsshell_i.h"
#include "..\lscore\osplatform.h"

#include "common.h"
#include "regist.h"

//////////////////////////////////////////////////////////////////////
//

class ClsshellModule : public CAtlDllModuleT< ClsshellModule >
{
public:
	DECLARE_LIBID(LIBID_lsshellLib)
	DECLARE_REGISTRY_APPID_RESOURCEID(IDR_LSSHELL, "{1DD53B06-FE14-4B2B-A9B3-D99F45D11007}")

	HFONT m_font;
	COSPlatform m_osp;
	CSimpleMap<UINT, HBITMAP> m_arrButtons;
	HIMAGELIST m_himShellMenu;
	bool m_bAdmin;

	void Construct();
	void InitInstance();
	void ExitInstance();
	HFONT GetMenuFont();
	void InitializeImageList();

	inline HBITMAP GetCommandBitmap(UINT idCommand) { return m_arrButtons.Lookup(idCommand); }
	inline int GetCommandImageIndex(UINT idCommand) { return m_arrButtons.FindKey(idCommand); }
	inline HIMAGELIST GetCommandImageList() { return m_himShellMenu; }
	inline bool IsComm6() { return m_osp.IsComm6(); }
	inline bool IsAdmin() { return m_bAdmin; }
};

extern ClsshellModule _AtlModule;

#endif				// __MAINMODULE_H__
