#include "stdafx.h"
#include "resource.h"
#include "lscore.h"
#include "fnc.h"
#include "common.h"

#include "picker.h"

static const int nptList[18] = {
	90, 95, 100, 105, 110, 120, 140, 160, 180, 200, 220, 240, 260, 280, 360, 480, 640, 720
};

using CHARSETNAME = struct {
	int charset;
	LPCTSTR name;
};

static const CHARSETNAME csnTable[19] = {
	ANSI_CHARSET,				_T("欧文"),
	DEFAULT_CHARSET,		_T("DEFAULT"),
	SYMBOL_CHARSET,			_T("シンボル"),
	SHIFTJIS_CHARSET,		_T("日本語"),
	HANGEUL_CHARSET,		_T("ハングル"),
	GB2312_CHARSET,			_T("簡体字中国語"),
	CHINESEBIG5_CHARSET,_T("繁体字中国語"),
	JOHAB_CHARSET,			_T("Johab"),
	HEBREW_CHARSET,			_T("ヘブライ語"),
	ARABIC_CHARSET,			_T("アラビア語"),
	GREEK_CHARSET,			_T("ギリシャ語"),
	TURKISH_CHARSET,		_T("トルコ語"),
	VIETNAMESE_CHARSET,	_T("ベトナム語"),
	THAI_CHARSET,				_T("タイ語"),
	EASTEUROPE_CHARSET,	_T("中央ヨーロッパ言語"),
	RUSSIAN_CHARSET,		_T("キリル言語"),
	MAC_CHARSET,				_T("Mac"),
	BALTIC_CHARSET,			_T("バルト言語"),
	OEM_CHARSET,				_T("OEM"),
};
static const TCHAR sszOther[] = _T("その他");

LPCTSTR __lookup_charset(int charset)
{
  for (auto& csn : csnTable) {
    if (csn.charset == charset)
      return csn.name;
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
	ON_MESSAGE_VOID(FPM_GETLOGFONT, OnGetLogFont)
	ON_MESSAGE(FPM_SETLOGFONT, OnSetLogFont)
END_MSG_MAP()

	void OnRButtonUp(UINT nFlags, CPoint point);
	void OnGetLogFont(WPARAM wParam, LPARAM lParam);
	LRESULT OnSetLogFont(WPARAM wParam, LPARAM lParam);
	void Browse();
};

///////////////////////////////////////////////////////
//

/**
 * @brief フォントサイズを取得する
 * @return フォント（ポイント）サイズ（10倍）
 */
static int __fastcall get_pointsize(HWND hwnd, const LOGFONT& lf)
{
	CClientDC dc(hwnd);
	int point = 0;
	int height = abs(lf.lfHeight);

	switch (dc.GetMapMode()) {
		case MM_TWIPS:
			point = MulDiv(height, 10, 20);
			break;
		case MM_TEXT:
			point = MulDiv(height, 720, dc.GetDeviceCaps(LOGPIXELSY));
			break;
		case MM_LOMETRIC:
			point = MulDiv(height, 720, 254);
			break;
		case MM_HIMETRIC:
			point = MulDiv(height, 720, 2540);
			break;
		case MM_LOENGLISH:
			point = MulDiv(height, 720, 100);
			break;
		case MM_HIENGLISH:
			point = MulDiv(height, 720, 1000);
			break;
		default:
			point = 0;
			break;
	}
	return point;
}

/**
 * @brief フォントの高さを取得する
 * @param hdc デバイスコンテキスト
 * @param point フォントサイズ（10倍）
 * @return フォントの高さ
 */
static int __fastcall get_fontheight(HWND hwnd, int point)
{
	CClientDC dc(hwnd);

	int height;
	switch (dc.GetMapMode()) {
		case MM_TWIPS:
			height = MulDiv(point, 20, 10);
			break;
		case MM_TEXT:
			height = MulDiv(point, dc.GetDeviceCaps(LOGPIXELSY), 720);
			break;
		case MM_LOMETRIC:
			height = MulDiv(point, 254, 720);
			break;
		case MM_HIMETRIC:
			height = MulDiv(point, 2540, 720);
			break;
		case MM_LOENGLISH:
			height = MulDiv(point, 10, 720);
			break;
		case MM_HIENGLISH:
			height = MulDiv(point, 100, 720);
			break;
		default:
			height = 0;
			break;
	}
	return height;
}

static int __fastcall FormatFontInfo(HWND hwnd, const LOGFONT& lf, int pointsize, CString& str)
{
  if (pointsize == 0)
    pointsize = get_pointsize(hwnd, lf);

	CString s;
	if (lf.lfWeight > FW_NORMAL)
		s += _T(" 太字");
	if (lf.lfItalic)
		s += _T(" 斜体");
	if (s.GetLength() < 1)
		s = _T(" 標準");

	LPCTSTR cname = __lookup_charset(lf.lfCharSet);
  CString pts;
  pts.Format((pointsize % 10) == 0 ? _T("%.0f") : _T("%.1f"), (float)pointsize / 10.0f);
	str.Format(_T("%s %spt%s %s"), lf.lfFaceName, pts.GetString(), s.GetString(), cname);
	return pointsize;
}

CFontPickerCtrl::CFontPickerCtrl()
{
	NONCLIENTMETRICS nm;
	nm.cbSize = RunTimeHelper::SizeOf_NONCLIENTMETRICS();
	SystemParametersInfo(SPI_GETNONCLIENTMETRICS, sizeof(nm), &nm, FALSE);
	m_lf = nm.lfMenuFont;
	m_pointsize = FormatFontInfo(nullptr, m_lf, 0, m_strDisplayName);

	HIMAGELIST him = lccLoadImageList(RESINST, RT_RCDATA, MAKEINTRESOURCE(IDB_PICKER_FONT), 16, RGB(255, 0, 255));
	SetButtonImageList(him);

	SetExtendedStyle(PICKER_CENTERTEXT, PICKER_CENTERTEXT);
}

static UINT_PTR CALLBACK fonthook(HWND hdlg, UINT msg, WPARAM /*wParam*/, LPARAM /*lParam*/)
{
	if (msg == WM_INITDIALOG) {
		ShowWindow(GetDlgItem(hdlg, 1091), SW_HIDE);
		ShowWindow(GetDlgItem(hdlg, 1139), SW_HIDE);
	}
	return FALSE;
}

void CFontPickerCtrl::Browse()
{
	CString styleName;
	CHOOSEFONT cf = { 0 };
	LOGFONT lf = { 0 };
	memcpy(&lf, &m_lf, sizeof(lf));

	cf.lStructSize = sizeof(cf);
	cf.hwndOwner = m_hWnd;
	cf.lpszStyle = styleName.GetBufferSetLength(LF_FACESIZE * 2);
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
	if (style & FPS_NOVERTFONTS)
		cf.Flags |= CF_NOVERTFONTS;
	cf.hInstance = RESINST;

	if (ChooseFont(&cf)) {
		if (!lstrlen(lf.lfFaceName) && lstrlen(m_lf.lfFaceName))
			(void)lstrcpyn(lf.lfFaceName, m_lf.lfFaceName, ARRAYSIZE(lf.lfFaceName));

		m_pointsize = cf.iPointSize;
		memcpy(&m_lf, cf.lpLogFont, sizeof(LOGFONT));
		FormatFontInfo(nullptr, m_lf, m_pointsize, m_strDisplayName);
		Invalidate(FALSE);

		GetParent().SendMessage(WM_COMMAND, MAKEWPARAM(GetDlgCtrlID(), CBN_SELCHANGE), (LPARAM)m_hWnd);
	}
}

LRESULT CFontPickerCtrl::OnSetLogFont(WPARAM /*wParam*/, LPARAM lParam)
{
	if (lParam) {
		memcpy(&m_lf, (LOGFONT*)lParam, sizeof(LOGFONT));

		m_pointsize = FormatFontInfo(nullptr, m_lf, 0, m_strDisplayName);
		Invalidate(FALSE);
	}
	return 0;
}

void CFontPickerCtrl::OnGetLogFont(WPARAM /*wParam*/, LPARAM lParam)
{
	if (lParam)
		memcpy((LOGFONT*)lParam, &m_lf, sizeof(LOGFONT));
}

void CFontPickerCtrl::OnRButtonUp(UINT /*nFlags*/, CPoint pt)
{
	if (IsCaptured())
		return;

	if (GetFocus() != m_hWnd)
		SetFocus();

	CMenu pop;
	pop.CreatePopupMenu();
	CString s;
	for (auto& npt : nptList) {
		s.Format((npt % 10) == 0 ? _T("%.0f") : _T("%.1f"), (float)npt / 10.0f);
		UINT flag = MF_STRING;
		if (npt == m_pointsize)
			flag |= MF_CHECKED;
		pop.AppendMenu(flag, npt, s);
	}

	ClientToScreen(&pt);
	int id = pop.TrackPopupMenu(TPM_RETURNCMD, pt.x, pt.y, m_hWnd);
	if (id > 0 && id != m_pointsize) {
		m_pointsize = id;
		m_lf.lfHeight = -get_fontheight(m_hWnd, m_pointsize);
		FormatFontInfo(m_hWnd, m_lf, m_pointsize, m_strDisplayName);
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
