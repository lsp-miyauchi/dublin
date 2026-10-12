////////////////////////////////////////////////////////////////////////
// funckeybar.h

#pragma once
#ifndef __FUNCKEYBAR_H__
#define __FUNCKEYBAR_H__

#include "data.h"

////////////////////////////////////////////////////////////////////////
//

class CCmdFunctionKey
{
public:
	UINT m_id;
	CString m_label;

	CCmdFunctionKey() : m_id(0) { }
	~CCmdFunctionKey() { }
};

////////////////////////////////////////////////////////////////////////
//

class CFuncKeyBar : public CWindowImpl<CFuncKeyBar>
{
private:
	CWindow m_owner;
	UINT_PTR m_timerid;
	CButton m_buttons[12];
	CFont m_font;
	int m_keyState;
	int m_numButtonGroups;
	int m_nTimerCount;
	int m_buttonHeight;
	CSimpleArray<CCmdFunctionKey> m_cmdArray;

public:
	DECLARE_WND_CLASS_EX(_T("FuncKeyBar"), 0, COLOR_3DFACE)

	CFuncKeyBar()
		: m_timerid(0)
		, m_keyState(-1)
		, m_numButtonGroups(4)
		, m_nTimerCount(0)
		, m_buttonHeight(16)
	{
	}
	~CFuncKeyBar() {}

BEGIN_MSG_MAP(CFuncKeyBar)
	ON_WM_CREATE()
	ON_WM_SIZE()
	ON_WM_DESTROY()
	ON_WM_TIMER()
	ON_MESSAGE_VOID(WM_COMMAND, OnCommand)
END_MSG_MAP()

private:
	int OnCreate(LPCREATESTRUCT pcs);
	void OnSize(UINT nType, int cx, int cy);
	void OnDestroy();
	void OnTimer(UINT_PTR nIDEvent);
	void OnCommand(WPARAM wParam, LPARAM lParam);

	void LoadSettings();
	void CreateButtons();
	int CalcButtonSize();
	void ApplyFont();

public:
	HWND CreateBar(HWND hParent, UINT idCtrl);
	void UpdateProperty(WPARAM wHint, LPARAM lHint = NULL);
	void ResetButtons(BOOL needRedraw = TRUE);
	void EnableTimer(bool bEnable);
	int GetBarHeight();
};

#endif	// __FUNCKEYBAR_H__
