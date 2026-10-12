/////////////////////////////////////////////////////////////////////////////
// funckeybar.cpp

#include "stdafx.h"
#include "fv7.h"
#include "mainfrm.h"
#include "funckeybar.h"


#define TIMERID_CHECKSTATE				1
#define TIMER_TIMEOUT					200
#define TIMER_CHECKFUNCENABLE			300
#define BUTTON_COMMAND_BASE				1000

////////////////////////////////////////////////////////////////////////
//

enum {
	KEY_CONTROL = 1,
	KEY_SHIFT = 2,
	KEY_MENU = 4,
};

static int getCtrlKeyState()
{
	int state = 0;
	if (GetKeyState(VK_CONTROL) < 0)	state |= KEY_CONTROL;
	if (GetKeyState(VK_SHIFT) < 0)		state |= KEY_SHIFT;
	if (GetKeyState(VK_MENU) < 0)		state |= KEY_MENU;
	return state;
}

////////////////////////////////////////////////////////////////////////
//

HWND CFuncKeyBar::CreateBar(HWND hParent, UINT idCtrl)
{
	LoadSettings();

	// ボタン高
	CClientDC dc(NULL);
	HFONT hfont = dc.SelectFont(m_font);
	TEXTMETRIC tm;
	dc.GetTextMetrics(&tm);
	dc.SelectFont(hfont);
	m_buttonHeight = tm.tmHeight + tm.tmExternalLeading + 8;

	return Create(hParent, NULL, NULL, WS_CHILD | WS_VISIBLE | WS_CLIPCHILDREN | WS_CLIPSIBLINGS, 0, idCtrl);
}

void CFuncKeyBar::UpdateProperty(WPARAM wHint, LPARAM lHint)
{
	if (wHint == DCM_CHANGEFORMAT) {
		LoadSettings();
		ApplyFont();
	}
}

void CFuncKeyBar::LoadSettings()
{
	if (!m_font.IsNull())
		m_font.DeleteObject();
	LOGFONT lf;
	if (!LoadFontInfo(&lf, IDS_SEC_SETTINGS, IDS_KEY_FUNCTIONFONT))
		_GCDATA.GetRibbonLogFont(&lf);
	m_font.CreateFontIndirect(&lf);
}

int CFuncKeyBar::OnCreate(CREATESTRUCT* pcs)
{
	CreateButtons();
	return 0;
}

void CFuncKeyBar::OnSize(UINT nType, int cx, int cy)
{
	CRect rc;
	GetWindowRect(&rc);
	int top = (rc.Height() - m_buttonHeight) / 2;
	int width = CalcButtonSize();				// ボタンのサイズを計算

	int numButtons = ARRAYSIZE(m_buttons);
	int delta = 2;
	for (int i = 0; i < numButtons; i++) {
		if (i && !(i % m_numButtonGroups)) {
			delta += 12;
		}
		m_buttons[i].MoveWindow(delta, top, width, m_buttonHeight, TRUE);
		delta += width + 1;
	}
	Invalidate(TRUE);
}

void CFuncKeyBar::OnTimer(UINT_PTR idEvent)
{
	if (GetActiveWindow() != GetTopLevelParent() && m_keyState != -1)
		return;

	int state = getCtrlKeyState();
	int numButtons = ARRAYSIZE(m_buttons);

	// ALT,Shift,Ctrlキーの状態が変化したか
	int i;
	if (state != m_keyState) {
		int baseIndex = 0;
		if (state & KEY_SHIFT)		baseIndex += 12;
		if (state & KEY_CONTROL)	baseIndex += 24;
		m_nTimerCount = TIMER_CHECKFUNCENABLE + 1;
		// ファンクションキーの機能名を取得 
		for (i = 0; i < numButtons; i++) {
			CCmdFunctionKey& key = m_cmdArray[baseIndex + i];
			m_buttons[i].SetWindowText(key.m_label);
			m_buttons[i].EnableWindow(key.m_id != 0);
		}
	}
	m_nTimerCount += TIMER_TIMEOUT;
	if (m_nTimerCount > TIMER_CHECKFUNCENABLE || state != m_keyState) {
		m_nTimerCount = 0;
		// 必要なら機能が利用可能か調べる
//@		for (i = 0; i < numButtons; i++) {
//@			BOOL bEnable = fvApp.wndMain.IsFuncEnable();
//@			m_buttons[i].EnableWindow(bEnable);
//@		}
	}
	m_keyState = state;
}

void CFuncKeyBar::OnCommand(WPARAM wParam, LPARAM lParam)
{
	int index = (int)LOWORD(wParam) - BUTTON_COMMAND_BASE;
	if (index >= 0 && index < ARRAYSIZE(m_buttons)) {
		if (m_keyState & KEY_SHIFT)		index += 12;
		if (m_keyState & KEY_CONTROL)	index += 24;
		if (fvApp.keymapFnc[index] != 0)
			fvApp.wndMain.SendMessage(WM_COMMAND, fvApp.keymapFnc[index], (LPARAM)m_hWnd);
	}
	fvApp.wndMain.SetFocus();
}

// WM_DESTROY処理
void CFuncKeyBar::OnDestroy()
{
	EnableTimer(false);
	for (int i = 0; i < ARRAYSIZE(m_buttons); i++) {
		if (m_buttons[i].IsWindow())
			m_buttons[i].DestroyWindow();
	}
}

// ボタンの生成
void CFuncKeyBar::CreateButtons()
{
	// ボタン情報
	ResetButtons();

	CRect rc;
	GetWindowRect(&rc);
	int top = (rc.Height() - m_buttonHeight) / 2;
	for (int i = 0; i < ARRAYSIZE(m_buttons); i++ ) {
		rc.SetRect(0, top, 8, top + m_buttonHeight);
		m_buttons[i].Create(m_hWnd, rc, NULL, WS_VISIBLE | WS_CHILD | BS_CENTER | BS_FLAT, 0, BUTTON_COMMAND_BASE + i);
		m_buttons[i].SetFont(m_font, TRUE);
	}
	m_keyState = -1;
}

void CFuncKeyBar::ApplyFont()
{
	// フォント変更/
	for (int i = 0; i < ARRAYSIZE(m_buttons); i++ ) {
		m_buttons[i].SetFont(m_font, TRUE);
	}
}

// ボタンのサイズを計算
int CFuncKeyBar::CalcButtonSize()
{
	CRect rc;
	GetWindowRect(&rc);
	int numButtons = ARRAYSIZE(m_buttons);
	return ( rc.Width() - numButtons -  ( (numButtons + m_numButtonGroups - 1) / m_numButtonGroups - 1 ) * 12 ) / numButtons;
}

void CFuncKeyBar::ResetButtons(BOOL needRedraw)
{
	m_cmdArray.RemoveAll();
	for (int i = 0; i < 48; i++) {
		CCmdFunctionKey key;
		key.m_id = fvApp.keymapFnc[i];
		if (key.m_id)
			GetCommandLabel(key.m_id, key.m_label);
		m_cmdArray.Add(key);
	}
	m_keyState = -1;
	if (needRedraw)
		Invalidate(TRUE);
}


// タイマーの更新を開始／停止する。 20060126 aroka
// ファンクションキー表示はタイマーにより更新しているが、
// アプリのフォーカスが外れたときに親ウィンドウからON/OFFを
//	呼び出してもらうことにより、余計な負荷を停止したい。
void CFuncKeyBar::EnableTimer(bool bEnable)
{
	if (bEnable) {
		if (!m_timerid)
			m_timerid = SetTimer(TIMERID_CHECKSTATE, TIMER_TIMEOUT);
	}
	else if (m_timerid) {
		KillTimer(m_timerid);
		m_timerid = 0;
		m_keyState = -1;
	}
}

int CFuncKeyBar::GetBarHeight()
{
	return m_buttonHeight + 7;
}
