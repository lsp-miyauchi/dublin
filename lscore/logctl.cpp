// logctl.cpp : インプリメンテーション ファイル
//

#include "stdafx.h"
#include "resource.h"
#include "lscore.h"
#include "fnc.h"
#include "common.h"

#define MAX_MSG			512

//////////////////////////////////////////////////////////////////////
//

static int __fastcall spr_str(LPCTSTR pText, TCHAR dlm, LPTSTR* argv, int argc)
{
	int i = 0;
	LPTSTR pDup = StrDup(pText);
	LPTSTR p = pDup;
	LPTSTR q;
	while (p && *p && i < argc) {
		q = StrChr(p, dlm);
		if (q)
			*q++ = '\0';

		argv[i] = (LPTSTR)malloc((lstrlen(p) + 2) * sizeof(TCHAR));
		lstrcpy(argv[i], p);
		if (q)
			lccAddChar(argv[i], '\n');
		i++;
		p = q;
	}
	LocalFree(pDup);
	return i;
}


//////////////////////////////////////////////////////////////////////
//

class CLogCtrl : public CWindowImpl<CLogCtrl>, public CThemeImpl<CLogCtrl>
{
private:
	UINT m_nPos;
	UINT m_nMaxPos;
	CAtlArray<LPTSTR> m_array;

	COLORREF m_rgbFore;
	COLORREF m_rgbBack;
	HFONT m_hFont;
	UINT m_nLineHeight;
	UINT m_nRow;
	bool m_bContinued;

public:
	CLogCtrl();
	~CLogCtrl();

	virtual void OnFinalMessage(HWND /*hWnd*/)
	{
		delete this;
	}

BEGIN_MSG_MAP(CLogCtrl)
	CHAIN_MSG_MAP(CThemeImpl<CLogCtrl>)
	ON_WM_GETDLGCODE()
	ON_WM_PAINT()
	ON_WM_SIZE()
	ON_WM_ERASEBKGND()
	ON_WM_KEYDOWN()
	ON_WM_VSCROLL()
	ON_MESSAGE_VOID(LGM_ADDLOG, OnHandlerAdd)
	ON_MESSAGE_VOID(LGM_CLEARLOG, OnHandlerClear)
END_MSG_MAP()

	void OnPaint();
	void OnSize(UINT nType, int cx, int cy);
	LRESULT OnGetDlgCode();
	BOOL OnEraseBkgnd(HDC hdc);
	void OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags);
	void OnVScroll(UINT nSBCode, UINT nPos, HWND hScrollBar);
	void OnHandlerAdd(WPARAM wParam, LPARAM lParam) { Add((LPCTSTR)lParam); }
	void OnHandlerClear(WPARAM wParam, LPARAM lParam) { Clear(); }

	void ClearLog();
	void Add(LPCTSTR pText);
	void Clear();
};


CLogCtrl::CLogCtrl()
{
	m_nPos = 0;
	m_nMaxPos = 0;
	m_rgbFore = GetSysColor(COLOR_WINDOWTEXT);
	m_rgbBack = GetSysColor(COLOR_WINDOW);
	m_bContinued = false;

	LOGFONT lf;
	SystemParametersInfo(SPI_GETICONTITLELOGFONT, sizeof(LOGFONT), &lf, 0);
	lstrcpy(lf.lfFaceName, _T("ＭＳ ゴシック"));
	lf.lfPitchAndFamily = FIXED_PITCH | FF_DONTCARE;
	m_hFont = CreateFontIndirect(&lf);

	CClientDC dc(NULL);
	HFONT hfOld = dc.SelectFont(m_hFont);
	TEXTMETRIC tm;
	dc.GetTextMetrics(&tm);
	dc.SelectFont(hfOld);

	m_nLineHeight = tm.tmHeight + tm.tmExternalLeading;

	// Theme
	SetThemeClassList(L"Edit");
	SetThemeExtendedStyle(THEME_EX_THEMECLIENTEDGE, 0);
}

CLogCtrl::~CLogCtrl()
{
	ClearLog();
	DeleteObject(m_hFont);
}

void CLogCtrl::ClearLog()
{
	LPTSTR p;
	int cnt = (int)m_array.GetCount();
	for (int i = 0; i < cnt; i++) {
		p = m_array[i];
		if (p) free(p);
	}
	m_array.RemoveAll();
	m_nPos = 0;
	m_nMaxPos = 0;
}

LRESULT CLogCtrl::OnGetDlgCode()
{
	return DLGC_WANTCHARS | DLGC_WANTARROWS;
}

void CLogCtrl::OnPaint()
{
	CPaintDC dc(m_hWnd);

	HFONT hfont = dc.SelectFont(m_hFont);
	dc.SetBkMode(TRANSPARENT);
	COLORREF rgbFore = dc.SetTextColor(m_rgbFore);
	COLORREF rgbBack = dc.SetBkColor(m_rgbBack);

	LPTSTR p;
	for (int i = 0; i + m_nPos < m_nMaxPos; i++) {
		p = m_array[i + m_nPos];
		if (p)
			dc.TextOut(2, i * m_nLineHeight, p, lstrlen(p));
	}

	dc.SetTextColor(rgbFore);
	dc.SetBkColor(rgbBack);
	dc.SelectFont(hfont);
}

void CLogCtrl::OnSize(UINT nType, int cx, int cy) 
{
	if (nType != SIZE_MINIMIZED) {
		m_nRow = cy / m_nLineHeight;
		ShowScrollBar(SB_VERT, (m_nRow < m_nMaxPos));
	}
}

void CLogCtrl::OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags)
{
	UINT code;
	switch (nChar) {
		case VK_UP:
			code = SB_LINEUP;
			break;

		case VK_DOWN:
			code = SB_LINEDOWN;
			break;

		case VK_PRIOR:
			code = SB_PAGEUP;
			break;

		case VK_NEXT:
			code = SB_PAGEDOWN;
			break;

		default:
			DefWindowProc();
			return;
	}
	OnVScroll(code, 0, m_hWnd);
}

void CLogCtrl::OnVScroll(UINT nSBCode, UINT nPos, HWND hScrollBar)
{
	CRect rc;
	GetClientRect(&rc);

	switch (nSBCode) {
		case SB_LINEDOWN:
			if (m_nPos >= m_nMaxPos - 1)
				return;

			ScrollWindow(0, -(int)m_nLineHeight, &rc, &rc);
			rc.top = rc.bottom - m_nLineHeight;
			m_nPos++;
			break;

		case SB_LINEUP:
			if (m_nPos < 1)
				return;
			ScrollWindow(0, m_nLineHeight, &rc, &rc);
			rc.bottom = rc.top + m_nLineHeight;
			m_nPos--;
			break;

		case SB_THUMBTRACK:
		case SB_THUMBPOSITION:
			m_nPos = min(m_nMaxPos - 1, nPos);
			break;

		case SB_PAGEUP:
			if (m_nPos > m_nRow)
				m_nPos -= m_nRow;
			else
				m_nPos = 0;
			break;

		case SB_PAGEDOWN:
			if (m_nPos < m_nMaxPos - m_nRow)
				m_nPos += m_nRow;
			else
				m_nPos = m_nMaxPos - 1;
			break;

		default:
			return;
	}
	SetScrollPos(SB_VERT, m_nPos, TRUE);
	RedrawWindow(&rc, NULL, RDW_INVALIDATE | RDW_UPDATENOW | RDW_ERASE);
}

BOOL CLogCtrl::OnEraseBkgnd(HDC hdc) 
{
	CDCHandle dc = hdc;
	CRect rc;
	GetClientRect(&rc);
	dc.FillSolidRect(&rc, m_rgbBack);
	return TRUE;
}

#define TABSIZE		4

static LPTSTR _format_line(LPTSTR pszSrc)
{
	int numtabs = 0;
	LPTSTR p = pszSrc;
	while (p && *p) {
		if (*p == '\t')
			numtabs++;
		p = CharNext(p);
	}
	if (!numtabs)
		return NULL;

	int len = lstrlen(pszSrc);
	DWORD size = len * sizeof(TCHAR) + numtabs * TABSIZE;
	LPTSTR pszDst = (LPTSTR)malloc(size);
	if (!pszDst)
		return NULL;

	register int i, j;
	for (i = j = 0; i < len; i++) {
		if (pszSrc[i] == '\t') {
			for (int k = j % TABSIZE; k < TABSIZE; k++)
				pszDst[j++] = ' ';
		}
		else
			pszDst[j++] = pszSrc[i];
	}
	pszDst[j] = '\0';
	return pszDst;
}

void CLogCtrl::Add(LPCTSTR pText)
{
	if (!pText || *pText == '\0') return;

	LPTSTR argv[256];
	LPTSTR pNew = StrDup(pText);
	rpl_str(pNew, _T("\x0d\x0a"), _T("\n"));
	lccReplaceChar(pNew, '\x0d', '\n');
	lccReplaceChar(pNew, '\x0a', '\n');
	int cnt = spr_str(pNew, '\n', argv, 256);
	LocalFree(pNew);

	bool bCont;
	LPTSTR p;
	for (int i = 0; i < cnt; i++) {
		bCont = (StrChr(argv[i], '\n') == NULL);
		lccReplaceChar(argv[i], '\n', '\0');

		p = _format_line(argv[i]);
		if (p) {
			free(argv[i]);
			argv[i] = p;
		}

		// 最後の行が継続状態ならば
		int size = (int)m_array.GetCount();
		if (size > 0 && m_bContinued) {
			p = m_array[size - 1];
			LPTSTR tmp = (LPTSTR)realloc(p, (lstrlen(p) + lstrlen(argv[i]) + 1) * sizeof(TCHAR));
			if (tmp) {
				lstrcat(tmp, argv[i]);
				m_array[size - 1] = tmp;
			}
		}
		else {
			if (m_nMaxPos < MAX_MSG - 1) {
				m_nMaxPos++;
				SetScrollRange(SB_VERT, 0, m_nMaxPos, TRUE);
			}
			else {
				p = m_array[0];
				m_array.RemoveAt(0);
				free(p);
			}
			m_array.Add(argv[i]);
		}
		m_bContinued = bCont;
	}

	ShowScrollBar(SB_VERT, (m_nRow < m_nMaxPos));
	if (m_nPos + m_nRow < m_nMaxPos) {
		m_nPos = m_nMaxPos - m_nRow;
		SetScrollPos(SB_VERT, m_nPos, TRUE);
	}
	RedrawWindow(NULL, NULL, RDW_INVALIDATE | RDW_UPDATENOW | RDW_ERASE);
}

void CLogCtrl::Clear()
{
	ClearLog();
	SetScrollPos(SB_VERT, 0, FALSE);
	SetScrollRange(SB_VERT, 0, 0, FALSE);
	RedrawWindow(NULL, NULL, RDW_INVALIDATE | RDW_UPDATENOW | RDW_ERASE);
}

/////////////////////////////////////////////////////////////////////////////
// logctl


extern "C"
LRESULT CALLBACK LogWndFn(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp)
{
	LRESULT lr = DefWindowProc(hwnd, msg, wp, lp);
	if (msg == WM_NCCREATE) {
		CLogCtrl* pWnd = new CLogCtrl();
		pWnd->SubclassWindow(hwnd);
	}
	return lr;
}
