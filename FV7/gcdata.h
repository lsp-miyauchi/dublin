////////////////////////////////////////////////////////////////////////
// gcdata.h

#pragma once
#ifndef __GCDATA_H__
#define __GCDATA_H__

extern COSPlatform gosp;

class CWinMetrics
{
public:
	NONCLIENTMETRICS m_NcMetrics;
	ICONMETRICS m_IconMetrics;
	CFont m_FontMenu;
	CFont m_FontCaption;
	CFont m_FontSmCaption;
	CFont m_FontStatus;
	CFont m_FontMessage;
	CFont m_FontIconTitle;
	CFont m_FontRibbon;
	CString m_strWinDir;
	CBrush m_BrushMenu;

	CWinMetrics()
	{
		LPTSTR p = m_strWinDir.GetBuffer(MAX_PATH);
		GetWindowsDirectory(p, MAX_PATH);
		m_strWinDir.ReleaseBuffer();

		UpdateWindowsSettings();
	}

	HFONT GetCaptionFont()
	{
		// キャプションフォント
		if (m_FontCaption.IsNull()) {
			m_FontCaption.CreateFontIndirect(&m_NcMetrics.lfCaptionFont);
		}
		return (HFONT)m_FontCaption;
	}

	inline int GetCaptionLogFont(LOGFONT* plf)
	{
		ATLASSERT(plf != NULL);
		return GetObject(GetCaptionFont(), sizeof(LOGFONT), plf);
	}

	HFONT GetSmallCaptionFont()
	{
		// スモールキャプションフォント
		if (m_FontSmCaption.IsNull()) {
			m_FontSmCaption.CreateFontIndirect(&m_NcMetrics.lfSmCaptionFont);
		}
		return (HFONT)m_FontSmCaption;
	}

	inline int GetSmallCaptionLogFont(LOGFONT* plf)
	{
		ATLASSERT(plf != NULL);
		return GetObject(GetSmallCaptionFont(), sizeof(LOGFONT), plf);
	}

	HFONT GetMenuFont()
	{
		if (m_FontMenu.IsNull()) {
			// メニューフォント
			m_FontMenu.CreateFontIndirect(&m_NcMetrics.lfMenuFont);
		}
		return (HFONT)m_FontMenu;
	}

	inline int GetMenuLogFont(LOGFONT* plf)
	{
		ATLASSERT(plf != NULL);
		return GetObject(GetMenuFont(), sizeof(LOGFONT), plf);
	}

	HFONT GetStatusFont()
	{
		if (m_FontStatus.IsNull()) {
			// ステータスフォント
			m_FontStatus.CreateFontIndirect(&m_NcMetrics.lfStatusFont);
		}
		return (HFONT)m_FontStatus;
	}

	inline int GetStatusLogFont(LOGFONT* plf)
	{
		ATLASSERT(plf != NULL);
		return GetObject(GetStatusFont(), sizeof(LOGFONT), plf);
	}

	HFONT GetMessageFont()
	{
		if (m_FontMessage.IsNull()) {
			// メッセージフォント
			m_FontMessage.CreateFontIndirect(&m_NcMetrics.lfMessageFont);
		}
		return (HFONT)m_FontMessage;
	}

	inline int GetMessageLogFont(LOGFONT* plf)
	{
		ATLASSERT(plf != NULL);
		return GetObject(GetMessageFont(), sizeof(LOGFONT), plf);
	}

	HFONT GetIconTitleFont()
	{
		// アイコンタイトルフォント
		if (m_FontIconTitle.IsNull()) {
			LOGFONT lf;
			SystemParametersInfo(SPI_GETICONTITLELOGFONT, sizeof(LOGFONT), &lf, FALSE);
			m_FontIconTitle.CreateFontIndirect(&lf);
		}
		return (HFONT)m_FontIconTitle;
	}

	inline int GetIconTitleLogFont(LOGFONT* plf)
	{
		ATLASSERT(plf != NULL);
		return GetObject(GetIconTitleFont(), sizeof(LOGFONT), plf);
	}

	HFONT GetRibbonFont()
	{
		// リボンフォント
		if (m_FontRibbon.IsNull()) {
			LOGFONT lf = m_NcMetrics.lfStatusFont;
			lstrcpy(lf.lfFaceName, _T("Meiryo UI"));
			CClientDC dc(NULL);
			lf.lfHeight = -::MulDiv(8, dc.GetDeviceCaps(LOGPIXELSY), 72);
			m_FontRibbon.CreateFontIndirect(&lf);
		}
		return (HFONT)m_FontRibbon;
	}

	inline int GetRibbonLogFont(LOGFONT* plf)
	{
		ATLASSERT(plf != NULL);
		return GetObject(GetRibbonFont(), sizeof(LOGFONT), plf);
	}

	inline HFONT GetGuiFont()
	{
		return (HFONT)::GetStockObject(DEFAULT_GUI_FONT);
	}

	inline int GetGuiLogFont(LOGFONT* plf)
	{
		ATLASSERT(plf != NULL);
		return GetObject(GetGuiFont(), sizeof(LOGFONT), plf);
	}

	HBRUSH GetMenuBackgroundBrush()
	{
		if (m_BrushMenu.IsNull()) {
			// メニュー背景ブラシ
			m_BrushMenu.CreateSolidBrush(COLOR_VISTA_MENUBKGND);
		}
		return (HBRUSH)m_BrushMenu;
	}

	void UpdateWindowsSettings()
	{
		// ノンクライアントメトリクス
		memset(&m_NcMetrics, 0, sizeof(m_NcMetrics));
		m_NcMetrics.cbSize = RunTimeHelper::SizeOf_NONCLIENTMETRICS();
		SystemParametersInfo(SPI_GETNONCLIENTMETRICS, sizeof(m_NcMetrics), &m_NcMetrics, FALSE);

		if (!m_FontMenu.IsNull())		m_FontMenu.DeleteObject();
		if (!m_FontCaption.IsNull())	m_FontCaption.DeleteObject();
		if (!m_FontSmCaption.IsNull())	m_FontSmCaption.DeleteObject();
		if (!m_FontStatus.IsNull())		m_FontStatus.DeleteObject();
		if (!m_FontMessage.IsNull())	m_FontMessage.DeleteObject();
		if (!m_FontIconTitle.IsNull())	m_FontIconTitle.DeleteObject();

		// アイコン関連
		memset(&m_IconMetrics, 0, sizeof(m_IconMetrics));
		m_IconMetrics.cbSize = sizeof(m_IconMetrics);
		SystemParametersInfo(SPI_GETICONMETRICS, 0, &m_IconMetrics, FALSE);
	}

	bool IsAlphaImageSupported()
	{
		CClientDC dc(NULL);
		return (dc.GetDeviceCaps(BITSPIXEL) > 24);
	}
};

extern CWinMetrics _GCDATA;

#endif		// __GCDATA_H__
