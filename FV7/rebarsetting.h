////////////////////////////////////////////////////////////////////////
// rebarsetting.h

#pragma once
#ifndef __REBARSETTING_H__
#define __REBARSETTING_H__


#define S_BAR_BANDCOUNT _T("BandCount")

class CRebarSettings
{
public:
	struct BandInfo {
		UINT id;
		UINT cx;
		UINT style;
		HWND hwnd;
	}* m_pBands;

	UINT m_cbBandCount;

	CRebarSettings() : m_pBands(NULL), m_cbBandCount(0)
	{
	}

	CRebarSettings(CReBarCtrl& rebar) : m_pBands(NULL), m_cbBandCount(0)
	{
		GetFrom(rebar);
	}

	~CRebarSettings()
	{
		if (m_pBands) delete[] m_pBands;
	}

	void GetFrom(CReBarCtrl& rebar)
	{
		ATLASSERT(rebar.IsWindow());
		if (m_pBands) delete[] m_pBands;

		m_pBands = NULL;
		m_cbBandCount = rebar.GetBandCount();
		if (m_cbBandCount > 0)
			m_pBands = new BandInfo[m_cbBandCount];

		for (UINT i = 0; i < m_cbBandCount; i++) {
			REBARBANDINFO rbbi = { RunTimeHelper::SizeOf_REBARBANDINFO(), RBBIM_ID | RBBIM_SIZE | RBBIM_STYLE | RBBIM_CHILD };
			rebar.GetBandInfo(i, &rbbi);
			m_pBands[i].id = rbbi.wID;
			m_pBands[i].cx = rbbi.cx;
			m_pBands[i].style = (rbbi.fStyle & (RBBS_BREAK | RBBS_HIDDEN));
			m_pBands[i].hwnd = rbbi.hwndChild;
		}
	}

	int LookupIndex(HWND hwndChild)
	{
		ATLASSERT(m_cbBandCount != 0);
		ATLASSERT(m_pBands != NULL);

		for (UINT i = 0; i < m_cbBandCount; i++) {
			if (m_pBands[i].hwnd == hwndChild)
				return (int)i;
		}
		return -1;
	}

	void ApplyTo(CReBarCtrl& rebar) const
	{
		ATLASSERT(rebar.IsWindow());
		ATLASSERT(rebar.GetBandCount() == m_cbBandCount);

		for (UINT i = 0; i < m_cbBandCount; i++) {
			rebar.MoveBand(rebar.IdToIndex(m_pBands[i].id), i);
			REBARBANDINFO rbbi = { RunTimeHelper::SizeOf_REBARBANDINFO(), RBBIM_ID | RBBIM_SIZE | RBBIM_STYLE };
			rebar.GetBandInfo(i, &rbbi);

			rbbi.cx = m_pBands[i].cx;
			rbbi.fStyle &= ~(RBBS_BREAK | RBBS_HIDDEN);
			rbbi.fStyle |= m_pBands[i].style  & (RBBS_BREAK | RBBS_HIDDEN);
			rebar.SetBandInfo(i, &rbbi);
		}
	}

	bool Load(LPCTSTR szRegKey, LPCTSTR szPrefix)
	{
		if (m_pBands) delete[] m_pBands;

		m_pBands = NULL;
		CString cs = szPrefix;
		cs += S_BAR_BANDCOUNT;
		m_cbBandCount = GetRegValue(szRegKey, (LPCTSTR)cs, 0);
		if (m_cbBandCount < 1)
			return false;

		m_pBands = new BandInfo[m_cbBandCount];
		CString s;
		for (DWORD i = 0; i < m_cbBandCount;i++) {
			s.Format(_T("%s%d"), szPrefix, i);
			if (GetRegBinary(szRegKey, s.GetString(), &m_pBands[i], sizeof(BandInfo)) != sizeof(BandInfo))
				return false;
		}
		return true;
	}

	bool Save(LPCTSTR szRegKey, LPCTSTR szPrefix) const
	{
		CString s = szPrefix;
		s += S_BAR_BANDCOUNT;
		SetRegValue(szRegKey, (LPCTSTR)s, m_cbBandCount);
		DWORD i;
		for (i = 0; i < m_cbBandCount;i++) {
			s.Format(_T("%s%d"), szPrefix, i);
			SetRegBinary(szRegKey, s.GetString(), &m_pBands[i], sizeof(BandInfo));
		}

		while (1) {
			// 余分なエントリを削除
			s.Format(_T("%s%d"), szPrefix, i++);
			if (!DelRegValue(szRegKey, s.GetString()))
				break;
		}
		return true;
	}
};


#endif			// __REBARSETTING_H__
