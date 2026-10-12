#pragma once
//////////////////////////////////////////////////////////////////////
// CLzhPage

static TCHAR dtformat[] = _T("yyyy'/'MM'/'dd");

class CLzhPage : public CDialogImpl<CLzhPage>, public CArchiverPageImpl<CLzhPage>
{
private:
	CComboBox m_cbPath;
	CComboBox m_cbMethod;
	LPCTSTR m_methodNames = _T("-lh0- (ストア)|-lh5- (標準)|-lh6-|-lh7-");

public:
	enum { IDD = IDD_PACK_LZH };

	BEGIN_MSG_MAP(CLzhPage)
		CHAIN_MSG_MAP(CArchiverPageImpl<CLzhPage>)
#ifndef _WIN64
		ON_BN_CLICKED(IDC_LZH_MAKESFX, OnMakeSfx)
		ON_BN_CLICKED(IDC_LZH_DATERANGE, OnDateRange)
#endif
	END_MSG_MAP()

#ifndef _WIN64
	void OnMakeSfx()
	{
		m_pd->m_sfx = IsDlgButtonChecked(IDC_LZH_MAKESFX);
	}

	void OnDateRange()
	{
		BOOL bEnable = IsDlgButtonChecked(IDC_LZH_DATERANGE);
		GetDlgItem(IDC_LZH_DTSTART).EnableWindow(bEnable);
		GetDlgItem(IDC_LZH_DTEND).EnableWindow(bEnable);
	}
#endif

	BOOL OnInitDialog()
	{
		InitDlgCtrl(GetDlgItem(IDC_LZH_PATHTREAT), IDS_PATHTREAT);

#ifdef _WIN64
		GetDlgItem(IDC_LZH_MAKESFX).ShowWindow(SW_HIDE);
		GetDlgItem(IDC_LZH_DATERANGE).ShowWindow(SW_HIDE);
		GetDlgItem(IDC_LZH_DTSTART).ShowWindow(SW_HIDE);
		GetDlgItem(IDC_LZH_DTEND).ShowWindow(SW_HIDE);
		GetDlgItem(IDC_LZH_DATESEPARATOR).ShowWindow(SW_HIDE);
#endif
		m_cbPath = GetDlgItem(IDC_LZH_PATHTREAT);
		m_cbPath.SetCurSel(m_pd->m_incdir ? 1 : 0);

		// 圧縮方式
		m_cbMethod = GetDlgItem(IDC_LZH_METHOD);
		CSimpleArray<CString> result = Split(m_methodNames, _T("|"));
		for (int i = 0; i < result.GetSize(); i++) {
			m_cbMethod.AddString(result[i]);
		}
		int val = GetRegValue(IDS_SEC_ARCHIVES, IDS_KEY_LZHPACKMODE, 1);
		m_cbMethod.SetCurSel(val);

#ifndef _WIN64
		SYSTEMTIME st;
		CDateTimePickerCtrl dtm = GetDlgItem(IDC_LZH_DTSTART);
		dtm.SetFormat(dtformat);
		if (GetRegBinary(IDS_SEC_ARCHIVES, IDS_KEY_LZHDTSTART, &st, sizeof(st)) == sizeof(st))
			dtm.SetSystemTime(GDT_VALID, &st);
		dtm.SetSystemTime(GDT_NONE, NULL);

		dtm = GetDlgItem(IDC_LZH_DTEND);
		dtm.SetFormat(dtformat);
		if (GetRegBinary(IDS_SEC_ARCHIVES, IDS_KEY_LZHDTEND, &st, sizeof(st)) == sizeof(st))
			dtm.SetSystemTime(GDT_VALID, &st);
		dtm.SetSystemTime(GDT_NONE, NULL);

		OnDateRange();
#endif
		return FALSE;
	}

	BOOL OnOK()
	{
		BOOL bMoveFile = IsDlgButtonChecked(IDC_LZH_MOVEFILE);
		BOOL bNewOnly = IsDlgButtonChecked(IDC_LZH_NEWONLY);

		switch (m_cbPath.GetCurSel()) {
			case 0:
				m_pd->m_pathmode = BASEDIR_NOPATH;
				break;
			case 2:
				m_pd->m_pathmode = BASEDIR_ABSOLUTE;
				break;
			default:
				m_pd->m_pathmode = BASEDIR_RELATIVE;
				break;
		}

		CString cmd;
		cmd = bMoveFile ? _T("m ") : _T("u ");					// 移動または更新
		if (!bNewOnly)											// 新ファイルのみ
			cmd += _T("-c ");
		cmd += _T("-a2 -gn2 -jp1 -+ ");							// 属性保存・簡易表示
		if (m_pd->m_pathmode != BASEDIR_NOPATH) {				// パス
			if (m_pd->m_incdir)
				cmd += _T("-r2 -x1 ");
		}

		// 圧縮モード
		int n = m_cbMethod.GetCurSel();
		SetRegValue(IDS_SEC_ARCHIVES, IDS_KEY_LZHPACKMODE, n);

		switch (n + IDC_LZH_LH0) {
			case IDC_LZH_LH6:
				n = 3;
				break;
			case IDC_LZH_LH7:
				n = 4;
				break;
			case IDC_LZH_LH5:
				n = 2;
				break;
			case IDC_LZH_LH0:
				n = 0;
				break;
		}

		CString str;
		str.Format(_T("-jm%d "), n);
		cmd += str;

#ifndef _WIN64
		// 格納日時範囲
		if (IsDlgButtonChecked(IDC_LZH_DATERANGE)) {
			SYSTEMTIME st;
			CDateTimePickerCtrl dtm = GetDlgItem(IDC_LZH_DTSTART);
			if (dtm.GetSystemTime(&st) == GDT_VALID) {
				str.Format(_T("-jol%04u%02u%02u000000 "), st.wYear, st.wMonth, st.wDay);
				cmd += str;
				st.wSecond = 0;
				st.wMilliseconds = 0;
				SetRegBinary(IDS_SEC_ARCHIVES, IDS_KEY_LZHDTSTART, &st, sizeof(st));
			}

			dtm = GetDlgItem(IDC_LZH_DTEND);
			if (dtm.GetSystemTime(&st) == GDT_VALID) {
				str.Format(_T("-jobl%04u%02u%02u235959 "), st.wYear, st.wMonth, st.wDay);
				cmd += str;
				st.wSecond = 59;
				st.wMilliseconds = 0;
				SetRegBinary(IDS_SEC_ARCHIVES, IDS_KEY_LZHDTEND, &st, sizeof(st));
			}
		}
#endif

		// 書庫名
		TCHAR buf[MAX_PATH * 2];
		lstrcpy(buf, m_pd->m_arcname);
		PathQuoteSpaces(buf);
		cmd += buf;

		// 作業ファイル
		MakeTempFile(m_pd->m_hdrop, &buf[1], m_pd->m_pathmode, false, ARC_LZH, (m_pd->m_unicode != false));
		m_pd->m_temp = &buf[1];
		buf[0] = '@';
		PathQuoteSpaces(buf);
		cmd += _T(' ');
		cmd += buf;

		m_pd->m_cmd = cmd;
		return TRUE;
	}

};

