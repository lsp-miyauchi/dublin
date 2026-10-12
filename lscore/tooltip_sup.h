////////////////////////////////////////////////////////////////////////
//

#pragma once
#ifndef __TOOLTIP_SUP_H__
#define __TOOLTIP_SUP_H__

#define MY_INFOTIP_LEN			400

template < class T >
class CToolTipUsing
{
public:
	CToolTipCtrl& GetTooltipCtrl(){ return m_tip; }

protected:
	CToolTipCtrl m_tip;
	bool m_bEnableTip;

// Construction
	CToolTipUsing() : m_tip(NULL), m_bEnableTip(false)
	{
	}

	void TTInitialize(DWORD style, UINT flags)
	{
		T* pT= (T*)this;

		HWND hwnd = *pT;
		ATLASSERT(::IsWindow(hwnd));

		// tool tips
		m_tip.Create(hwnd, NULL, NULL, WS_POPUP | style, WS_EX_TOOLWINDOW);
		m_tip.SetMaxTipWidth(MY_INFOTIP_LEN);
		CRect rc(3, 3, 3, 3);
		m_tip.SetMargin(&rc);
		CToolInfo ti(flags, hwnd, PtrToUint(hwnd), NULL, LPSTR_TEXTCALLBACK);
		m_tip.AddTool(&ti);
		m_tip.SetMaxTipWidth(0);
	}

// Message map and handlers
	BEGIN_MSG_MAP(CToolTipDialog)
		MESSAGE_RANGE_HANDLER(WM_MOUSEFIRST, WM_MOUSELAST, OnTTHandleMouseMessages)
		MESSAGE_HANDLER(WM_CREATE, OnTTHandleCreate)
		MESSAGE_HANDLER(WM_DESTROY, OnTTHandleDestroy)
	END_MSG_MAP()

	LRESULT OnTTHandleCreate(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
	{
		TTInitialize(TTS_NOPREFIX, TTF_IDISHWND);
		bHandled = FALSE;
		return 1;
	}

	LRESULT OnTTHandleMouseMessages(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
	{
		if (m_tip.IsWindow()) {
			T* pT= (T*)this;
			DWORD pos = GetMessagePos();
			MSG msg;
			msg.hwnd = *pT;
			msg.message = uMsg;
			msg.wParam = wParam;
			msg.lParam = lParam;
			msg.time = GetMessageTime();
			msg.pt.x = LOWORD(pos);
			msg.pt.y = HIWORD(pos);
			m_tip.RelayEvent(&msg);
		}
		bHandled = FALSE;
		return 1;
	}

	LRESULT OnTTHandleDestroy(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
	{
		if (m_tip.IsWindow()) {
			m_tip.Activate(FALSE);
			m_tip.DestroyWindow();
		}
		bHandled = FALSE;
		return 0;
	}

public:
	void TTEnable(bool bEnable)
	{
		if (bEnable != m_bEnableTip) {
			m_bEnableTip = bEnable;
			if (m_bEnableTip)
				TTActivate(FALSE);
		}
	}

	void TTActivate(BOOL bActivate)
	{
		m_tip.Activate( bActivate );
	}

	bool TTIsEnabled()
	{
		return m_bEnableTip;
	}

	void TTAttach(HWND hwndTips)
	{
		if (IsWindow(hwndTips) && !m_tip.IsWindow())
			m_tip.Attach(hwndTips);
	}

	void TTRelay(UINT uMsg, WPARAM wParam, LPARAM lParam)
	{
		BOOL bHandled;
		OnTTHandleMouseMessages(uMsg, wParam, lParam, bHandled);
	}

};


#endif		// __TOOLTIP_SUP_H__
