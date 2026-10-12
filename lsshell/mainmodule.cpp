#include "stdafx.h"
#include "mainmodule.h"
#include "common.h"

ClsshellModule _AtlModule;

////////////////////////////////////////////////////////////////////////
//

typedef struct tagRCTOOLBARDATA {
	WORD wVersion;
	WORD wWidth;
	WORD wHeight;
	WORD wItemCount;
	//WORD aItems[wItemCount]

	WORD* items() { return (WORD*)(this+1); }
} RCTOOLBARDATA;

////////////////////////////////////////////////////////////////////////
//

void ClsshellModule::Construct()
{
	m_font = NULL;
	m_himShellMenu = NULL;
	m_bAdmin = IsUserAnAdmin();
}

void ClsshellModule::InitInstance()
{
	AtlInitCommonControls(	ICC_WIN95_CLASSES |
							ICC_DATE_CLASSES |
							ICC_USEREX_CLASSES |
							ICC_COOL_CLASSES	);
	Construct();
}

void ClsshellModule::ExitInstance()
{
	if (m_font) {
		DeleteObject(m_font);
		m_font = NULL;
	}

	int cnt = m_arrButtons.GetSize();
	for (int i = 0; i < cnt; i++) {
		HBITMAP hbm = m_arrButtons.GetValueAt(i);
		if (hbm)
			::DeleteObject(hbm);
	}
	m_arrButtons.RemoveAll();

	if (m_himShellMenu) {
		ImageList_Destroy(m_himShellMenu);
		m_himShellMenu = NULL;
	}
}

#ifndef _SIZEOF_STRUCT
  #define _SIZEOF_STRUCT(structname, member)	(((int)((BYTE*)(&((structname*)0)->member) - ((BYTE*)((structname*)0)))) + sizeof(((structname*)0)->member))
#endif
#if (WINVER >= 0x0600) && !defined(NONCLIENTMETRICS_V1_SIZE)
  #define NONCLIENTMETRICS_V1_SIZE				_SIZEOF_STRUCT(NONCLIENTMETRICS, lfMessageFont)
#endif // !defined(_WIN32_WCE) && (WINVER >= 0x0600) && !defined(NONCLIENTMETRICS_V1_SIZE)

HFONT ClsshellModule::GetMenuFont()
{
	if (!m_font) {
		NONCLIENTMETRICS info;
		info.cbSize = sizeof(info);
		SystemParametersInfo(SPI_GETNONCLIENTMETRICS, sizeof(info), &info, 0);
		m_font = CreateFontIndirect(&info.lfMenuFont);
	}
	return m_font;
}

void ClsshellModule::InitializeImageList()
{
	if (m_himShellMenu == NULL) {
		m_himShellMenu = CreateBitmapMapArrayFromImageList(IDB_TOOLBAR, m_arrButtons, SIZE_TOOLBUTTON);

		// システムアイコンの追加
		HBITMAP hbm;
		hbm = CreateOemBitmap(-106);
		if (hbm)
			m_arrButtons.Add(PtrToUint(IDI_SHIELD), hbm);
	}
}
