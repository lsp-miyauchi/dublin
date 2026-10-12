#include "stdafx.h"
#include "resource.h"
#include "lscore.h"
#include "fnc.h"
#include "common.h"

#include "picker.h"

static int nptList[] = {
	8, 9, 10, 11, 12, 14, 16, 18, 20, 22, 24, 26, 28, 36, 48, 72
};

typedef struct tagCHARSETNAME {
	int charset;
	LPCTSTR name;
} CHARSETNAME;

CHARSETNAME csnTable[] = {
	ANSI_CHARSET,			_T("欧文"),
	DEFAULT_CHARSET,		_T("DEFAULT"),
	SYMBOL_CHARSET,			_T("シンボル"),
	SHIFTJIS_CHARSET,		_T("日本語"),
	HANGEUL_CHARSET,		_T("ハングル"),
	GB2312_CHARSET,			_T("簡体字中国語"),
	CHINESEBIG5_CHARSET,	_T("繁体字中国語"),
	JOHAB_CHARSET,					_T("Johab"),
	HEBREW_CHARSET,			_T("ヘブライ語"),
	ARABIC_CHARSET,			_T("アラビア語"),
	GREEK_CHARSET,			_T("ギリシャ語"),
	TURKISH_CHARSET,		_T("トルコ語"),
	VIETNAMESE_CHARSET,		_T("ベトナム語"),
	THAI_CHARSET,			_T("タイ語"),
	EASTEUROPE_CHARSET,		_T("中央ヨーロッパ言語"),
	RUSSIAN_CHARSET,		_T("キリル言語"),
	MAC_CHARSET,			_T("Mac"),
	BALTIC_CHARSET,			_T("バルト言語"),
	OEM_CHARSET,			_T("OEM"),
	-1,						NULL,
};
static const TCHAR sszOther[] = _T("その他");

LPCTSTR __lookup_charset(int charset)
{
	int i = 0;
	while (csnTable[i].charset >= 0) {
		if (csnTable[i].charset == charset)
			return csnTable[i].name;
		i++;
	}
	return sszOther;
}


////////////////////////////////////////////////////////////////////////
// CFontPickerCtrl

class CFontPickerCtrl : public CPickerCtrlImpl<CFontPickerCtrl>
{
private:
	LOGFONT m_lf;
	int m_pointsize;		// x 10倍

public:
	CFontPickerCtrl();

BEGIN_MSG_MAP(CFontPickerCtrl)
	CHAIN_MSG_MAP(CPickerCtrlImpl<CFontPickerCtrl>)
	ON_WM_RBUTTONUP()
	ON_MESSAGE(FPM_GETLOGFONT, OnGetLogFont)
	ON_MESSAGE(FPM_SETLOGFONT, OnSetLogFont)
END_MSG_MAP()

	void OnRButtonUp(UINT nFlags, CPoint point);
	LRESULT OnGetLogFont(WPARAM wParam, LPARAM lParam);
	LRESULT OnSetLogFont(WPARAM wParam, LPARAM lParam);
	void Browse();
};

///////////////////////////////////////////////////////
//

static int __fastcall get_pointsize(HDC hdc, LOGFONT* plf)
{
	int j, point = 0, height;
	if (plf->lfHeight < 0)
		height = plf->lfHeight * -1;
	else
		height = plf->lfHeight;
	switch (GetMapMode(hdc)) {
		case MM_TWIPS:
			point = (height + 10) / 2;
			break;

		case MM_TEXT:
			j = GetDeviceCaps(hdc, LOGPIXELSY);
			point = ((height * 72 + j / 2) * 10) / j;
			break;

		case MM_LOENGLISH:
			point = (height * 72 + 50) / 10;
			break;

		case MM_HIENGLISH:
			point = (height * 72 + 500) / 100;
			break;

		default:
			point = 0;
			break;
	}
	return point;
}

static int __fastcall get_fontheight(HDC hdc, int point)
{
	int j, height;
	switch (GetMapMode(hdc)) {
		case MM_TWIPS:
			height = point * 2;
			break;

		case MM_TEXT:
			j = GetDeviceCaps(hdc, LOGPIXELSY);
			height = MulDiv(point, j, 720);
			break;

		case MM_LOENGLISH:
			height = MulDiv(point, 10, 72);
			break;

		case MM_HIENGLISH:
			height = MulDiv(point, 100, 72);
			break;

		default:
			height = 0;
			break;
	}
	return height;
}

static int __fastcall FormatFontInfo(HDC hdc, LOGFONT* plf, CString& str)
{
	int point = get_pointsize(hdc, plf);
	CString s;
	if (plf->lfWeight > FW_NORMAL)	s += _T(" 太字");
	if (plf->lfItalic)				s += _T(" 斜体");
	if (s.GetLength() < 1)			s = _T(" 標準");

	LPCTSTR cname = __lookup_charset(plf->lfCharSet);
	str.Format(_T("%s %dpt%s %s"), plf->lfFaceName, point / 10, (LPCTSTR)s, cname);
	return point;
}

CFontPickerCtrl::CFontPickerCtrl()
{
	NONCLIENTMETRICS nm;
	nm.cbSize = RunTimeHelper::SizeOf_NONCLIENTMETRICS();
	SystemParametersInfo(SPI_GETNONCLIENTMETRICS, sizeof(nm), &nm, FALSE);
	m_lf = nm.lfMenuFont;
	CClientDC dc(NULL);
	m_pointsize = FormatFontInfo(dc, &m_lf, m_strDisplayName);

	HIMAGELIST him = lccLoadImageList(RESINST, RT_RCDATA, MAKEINTRESOURCE(IDB_PICKER_FONT), 16, RGB(255, 0, 255));
	SetButtonImageList(him);

	SetExtendedStyle(PICKER_CENTERTEXT, PICKER_CENTERTEXT);
}

static UINT_PTR CALLBACK fonthook(HWND hdlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
	if (msg == WM_INITDIALOG) {
		ShowWindow(GetDlgItem(hdlg, 1091), SW_HIDE);
		ShowWindow(GetDlgItem(hdlg, 1139), SW_HIDE);
	}
	return FALSE;
}

void CFontPickerCtrl::Browse()
{
	TCHAR szStyleName[MAX_PATH];

	CHOOSEFONT cf;
	LOGFONT lf;
	memset(&cf, 0, sizeof(cf));
	memcpy(&lf, &m_lf, sizeof(lf));
	memset(&szStyleName, 0, sizeof(szStyleName));

	cf.lStructSize = sizeof(cf);
	cf.hwndOwner = m_hWnd;
	cf.lpszStyle = szStyleName;
	cf.lpLogFont = &lf;
	cf.Flags = CF_SCREENFONTS | CF_INITTOLOGFONTSTRUCT | CF_EFFECTS | CF_ENABLEHOOK;
	cf.lpfnHook = (LPCFHOOKPROC)fonthook;

	DWORD style = GetStyle();
	if (style & FPS_TTONLY) {
		cf.Flags |= CF_TTONLY | CF_ENABLETEMPLATE;
		cf.lpTemplateName = MAKEINTRESOURCE(IDD_COMM_FONT);
	}
	if (style & FPS_PRINTERFONTS)
		cf.Flags |= CF_PRINTERFONTS;
	if (style & FPS_ANSIONLY)
		cf.Flags |= CF_ANSIONLY;
	if (style & FPS_FIXEDPITCHONLY)
		cf.Flags |= CF_FIXEDPITCHONLY;
	cf.hInstance = RESINST;

	if (ChooseFont(&cf)) {
		if (!lstrlen(lf.lfFaceName) && lstrlen(m_lf.lfFaceName))
			lstrcpyn(lf.lfFaceName, m_lf.lfFaceName, ARRAYSIZE(lf.lfFaceName));
		OnSetLogFont(0, (LPARAM)&lf);
		GetParent().SendMessage(WM_COMMAND, MAKEWPARAM(GetDlgCtrlID(), CBN_SELCHANGE), (LPARAM)m_hWnd);
	}
}

LRESULT CFontPickerCtrl::OnSetLogFont(WPARAM wParam, LPARAM lParam)
{
	if (lParam) {
		memcpy(&m_lf, (LOGFONT*)lParam, sizeof(LOGFONT));

		CClientDC dc(NULL);
		m_pointsize = FormatFontInfo(dc, &m_lf, m_strDisplayName);
		Invalidate(FALSE);
	}
	return 0;
}

LRESULT CFontPickerCtrl::OnGetLogFont(WPARAM wParam, LPARAM lParam)
{
	if (lParam)
		memcpy((LOGFONT*)lParam, &m_lf, sizeof(LOGFONT));
	return 0;
}

void CFontPickerCtrl::OnRButtonUp(UINT nFlags, CPoint pt)
{
	if (IsCaptured()) return;

	if (GetFocus() != m_hWnd) SetFocus();

	CMenu pop;
	pop.CreatePopupMenu();
	TCHAR buf[32];
	for (int i = 0; i < sizeof(nptList) / sizeof(int); i++) {
		wsprintf(buf, _T("%d"), nptList[i]);
		UINT flag = MF_STRING;
		if (nptList[i] == m_pointsize / 10)
			flag |= MF_CHECKED;
		pop.AppendMenu(flag, nptList[i], buf);
	}

	ClientToScreen(&pt);
	int id = pop.TrackPopupMenu(TPM_RETURNCMD, pt.x, pt.y, m_hWnd);
	if (id > 0 && id != m_pointsize / 10) {
		CClientDC dc(m_hWnd);
		m_lf.lfHeight = get_fontheight(dc, id * 10);
		m_pointsize = FormatFontInfo(dc, &m_lf, m_strDisplayName);
		InvalidateRect(&m_rcText);
		GetParent().SendMessage(WM_COMMAND, MAKEWPARAM(GetDlgCtrlID(), CBN_SELCHANGE), (LPARAM)m_hWnd);
	}
}


////////////////////////////////////////////////////////////////////////
//

extern "C"
LRESULT CALLBACK FontPickWndFn(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	LRESULT lr = DefWindowProc(hwnd, msg, wParam, lParam);
	if (msg == WM_NCCREATE) {
		CFontPickerCtrl* pWnd = new CFontPickerCtrl();
		pWnd->SubclassWindow(hwnd);
	}
	return lr;
}
