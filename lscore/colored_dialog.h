#pragma once

///////////////////////////////////////////////////////////////////////////////
// CColoerdDialogImpl

template <class T>
class CColoredDialogImpl
{
public:
	COLORREF m_colorBkgnd;
	CBrush m_brBkgnd;

	CColoredDialogImpl() : m_colorBkgnd(GetSysColor(COLOR_BTNHIGHLIGHT))
	{
		m_brBkgnd.CreateSolidBrush(m_colorBkgnd);
	}
	~CColoredDialogImpl()
	{}

	void SetDialogBkgndColor(COLORREF color)
	{
		m_colorBkgnd = color;
		m_brBkgnd.Attach(::CreateSolidBrush(color));
	}

	BEGIN_MSG_MAP(CColoredDialogImpl)
		MESSAGE_HANDLER(WM_CTLCOLORDLG, OnHandlerBkgndCtrlColor)
		MESSAGE_HANDLER(WM_CTLCOLORSTATIC, OnHandlerBkgndCtrlColor)
		MESSAGE_HANDLER(WM_CTLCOLOREDIT, OnHandlerBkgndCtrlColor)
		MESSAGE_HANDLER(WM_CTLCOLORLISTBOX, OnHandlerBkgndCtrlColor)
		MESSAGE_HANDLER(WM_CTLCOLORBTN, OnHandlerBkgndCtrlColor)
	END_MSG_MAP()

	LRESULT OnHandlerBkgndCtrlColor(UINT msg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
	{
		T* pT = static_cast<T*>(this);
		HBRUSH hbr = pT->GetBkgndBrush((HWND)lParam);
		if (hbr == NULL) {
			bHandled = FALSE;
			return 0;
		}
		return (LRESULT)hbr;
	}

	virtual HBRUSH GetBkgndBrush(HWND hwnd = NULL)
	{
		return m_brBkgnd.m_hBrush;
	}
};

