//////////////////////////////////////////////////////////////////////
// monthcal.h

class CMonthCalCtrl : public CWnd
{
	DECLARE_DYNAMIC(CMonthCalCtrl)

// Constructors
public:
	CMonthCalCtrl();
	BOOL Create(DWORD dwStyle, const RECT& rect, CWnd* pParentWnd, UINT nID);

// Attributes
	BOOL GetCurSel(LPSYSTEMTIME pst);
	BOOL SetCurSel(LPSYSTEMTIME pst);
	DWORD GetMaxSelCount();
	BOOL SetMaxSelCount(UINT n);
	BOOL GetSelRange(LPSYSTEMTIME rgst);
	BOOL SetSelRange(LPSYSTEMTIME rgst);
	DWORD GetMonthRange(DWORD gmr, LPSYSTEMTIME rgst);
	BOOL SetDayState(int cbds, LPMONTHDAYSTATE rgds);
	BOOL GetMinReqRect(LPRECT prc);
	BOOL SetToday(LPSYSTEMTIME pst);
	void HitTest(PMCHITTESTINFO pinfo);
	void SetColor(int iColor, COLORREF clr);
	COLORREF GetColor(int iColor);
	void SetFirstDayOfWeek(int iDay);
	DWORD GetFirstDayOfWeek();
	BOOL SetRange(DWORD gdtr, LPSYSTEMTIME rgst);
	int GetMonthDelta();
	int SetMonthDelta(int n);

// Implementation
public:
	virtual ~CMonthCalCtrl();
};

_inline CMonthCalCtrl::CMonthCalCtrl()
	{ }
_inline BOOL CMonthCalCtrl::GetCurSel(LPSYSTEMTIME pst)
	{ return (BOOL)SendMessage(MCM_GETCURSEL, 0, (LPARAM)(pst)); }
_inline BOOL CMonthCalCtrl::SetCurSel(LPSYSTEMTIME pst)
	{ return (BOOL)SendMessage(MCM_SETCURSEL, 0, (LPARAM)(pst)); }
_inline DWORD CMonthCalCtrl::GetMaxSelCount()
	{ return (DWORD)SendMessage(MCM_GETMAXSELCOUNT, 0, 0L); }
_inline BOOL CMonthCalCtrl::SetMaxSelCount(UINT n)
	{ return (BOOL)SendMessage(MCM_SETMAXSELCOUNT, (WPARAM)(n), 0L); }
_inline BOOL CMonthCalCtrl::GetSelRange(LPSYSTEMTIME rgst)
	{ return (BOOL)SendMessage(MCM_GETSELRANGE, 0, (LPARAM)(rgst)); }
_inline BOOL CMonthCalCtrl::SetSelRange(LPSYSTEMTIME rgst)
	{ return (BOOL)SendMessage(MCM_SETSELRANGE, 0, (LPARAM)(rgst)); }
_inline DWORD CMonthCalCtrl::GetMonthRange(DWORD gmr, LPSYSTEMTIME rgst)
	{ return (DWORD)SendMessage(MCM_GETMONTHRANGE, (WPARAM)(gmr), (LPARAM)(rgst)); }
_inline BOOL CMonthCalCtrl::SetDayState(int cbds, LPMONTHDAYSTATE rgds)
	{ return (BOOL)SendMessage(MCM_SETDAYSTATE, (WPARAM)(cbds), (LPARAM)(rgds)); }
_inline BOOL CMonthCalCtrl::GetMinReqRect(LPRECT prc)
	{ return (BOOL)SendMessage(MCM_GETMINREQRECT, 0, (LPARAM)(prc)); }
_inline BOOL CMonthCalCtrl::SetToday(LPSYSTEMTIME pst)
	{ return (BOOL)SendMessage(MCM_SETTODAY, 0, (LPARAM)pst); }
_inline void CMonthCalCtrl::HitTest(PMCHITTESTINFO pinfo)
	{ SendMessage(MCM_HITTEST, 0, (LPARAM)(PMCHITTESTINFO)pinfo); }
_inline void CMonthCalCtrl::SetColor(int iColor, COLORREF clr)
	{ SendMessage(MCM_SETCOLOR, iColor, clr); }
_inline COLORREF CMonthCalCtrl::GetColor(int iColor)
	{ return SendMessage(MCM_SETCOLOR, iColor, 0); }
_inline void CMonthCalCtrl::SetFirstDayOfWeek(int iDay)
	{ SendMessage(MCM_SETFIRSTDAYOFWEEK, 0, iDay); }
_inline DWORD CMonthCalCtrl::GetFirstDayOfWeek()
	{ return (DWORD)SendMessage(MCM_GETFIRSTDAYOFWEEK, 0, 0); }
_inline BOOL CMonthCalCtrl::SetRange(DWORD gdtr, LPSYSTEMTIME rgst)
	{ return (BOOL)SendMessage(MCM_SETRANGE, (WPARAM)(gdtr), (LPARAM)(rgst)); }
_inline int CMonthCalCtrl::GetMonthDelta()
	{ return (int)SendMessage(MCM_GETMONTHDELTA, 0, 0); }
_inline int CMonthCalCtrl::SetMonthDelta(int n)
	{ return (int)SendMessage(MCM_SETMONTHDELTA, n, 0); }


//////////////////////////////////////////////////////////////////////
//

class CDatePickCtrl : public CWnd
{
	DECLARE_DYNAMIC(CDatePickCtrl)

// Constructors
public:
	CDatePickCtrl();
	BOOL Create(DWORD dwStyle, const RECT& rect, CWnd* pParentWnd, UINT nID);

// Attributes
	DWORD GetSystemTime(LPSYSTEMTIME pst);
	BOOL SetSystemTime(DWORD gd, LPSYSTEMTIME pst);
	DWORD GetRange(LPSYSTEMTIME rgst);
	BOOL SetRange(DWORD gdtr, LPSYSTEMTIME rgst);
	BOOL SetFormat(LPCTSTR sz);
	void SetMonthCalColor(int iColor, COLORREF clr);
	COLORREF GetMonthCalColor(int iColor);
	HWND GetMonthCal();
	void SetMonthCalFont(HFONT hfont, BOOL fRedraw);
	HFONT GetMonthCalFont();

// Implementation
public:
	virtual ~CDatePickCtrl();
};

_inline CDatePickCtrl::CDatePickCtrl()
	{ }
_inline DWORD CDatePickCtrl::GetSystemTime(LPSYSTEMTIME pst)
	{ return (DWORD)SendMessage(DTM_GETSYSTEMTIME, 0, (LPARAM)(pst)); }
_inline BOOL CDatePickCtrl::SetSystemTime(DWORD gd, LPSYSTEMTIME pst)
	{ return (BOOL)SendMessage(DTM_SETSYSTEMTIME, (LPARAM)(gd), (LPARAM)(pst)); }
_inline DWORD CDatePickCtrl::GetRange(LPSYSTEMTIME rgst)
	{ return (DWORD)SendMessage(DTM_GETRANGE, 0, (LPARAM)(rgst)); }
_inline BOOL CDatePickCtrl::SetRange(DWORD gdtr, LPSYSTEMTIME rgst)
	{ return (BOOL)SendMessage(DTM_SETRANGE, (WPARAM)(gdtr), (LPARAM)(rgst)); }
_inline BOOL CDatePickCtrl::SetFormat(LPCTSTR sz)
	{ return (BOOL)SendMessage(DTM_SETFORMAT, 0, (LPARAM)(sz)); }
_inline void CDatePickCtrl::SetMonthCalColor(int iColor, COLORREF clr)
	{ SendMessage(DTM_SETMCCOLOR, (WPARAM)iColor, (LPARAM)clr); }
_inline COLORREF CDatePickCtrl::GetMonthCalColor(int iColor)
	{ return (COLORREF)SendMessage(DTM_GETMCCOLOR, (WPARAM)iColor, 0); }
_inline HWND CDatePickCtrl::GetMonthCal()
	{ return (HWND)SendMessage(DTM_GETMONTHCAL, 0, 0); }
_inline void CDatePickCtrl::SetMonthCalFont(HFONT hfont, BOOL fRedraw)
	{ SendMessage(DTM_SETMCFONT, (WPARAM)hfont, (LPARAM)fRedraw); }
_inline HFONT CDatePickCtrl::GetMonthCalFont()
	{ return (HFONT)SendMessage(DTM_GETMCFONT, 0, 0); }

