#pragma once
//////////////////////////////////////////////////////////////////////
// CBgaPage

class CBgaPage : public CDialogImpl<CBgaPage>, public CArchiverPageImpl<CBgaPage>
{
public:
	enum { IDD = IDD_PACK_BGA };
	BEGIN_MSG_MAP(CBgaPage)
		CHAIN_MSG_MAP(CArchiverPageImpl<CBgaPage>)
	END_MSG_MAP()

	BOOL OnInitDialog()
	{
		InitDlgCtrl(GetDlgItem(IDC_BGA_PATHTREAT), IDS_PATHTREAT);
		CComboBox combo = GetDlgItem(IDC_BGA_PATHTREAT);
		combo.SetCurSel(m_pd->m_incdir ? 1 : 0);

		CLccSliderCtrl slider = GetDlgItem(IDC_BGA_SLIDE);
		slider.SetRange(0, 9);
		int val = GetRegValue(IDS_SEC_ARCHIVES, IDS_KEY_BGALEVEL, 9);
		slider.SetPos(val);

		int n = GetRegValue(IDS_SEC_ARCHIVES, IDS_KEY_BGAPACKMODE, FALSE);
		CheckRadioButton(IDC_BGA_BZA, IDC_BGA_GZA, n + IDC_BGA_BZA);
		return FALSE;
	}

	BOOL OnOK()
	{
		m_pd->m_sfx = IsDlgButtonChecked(IDC_BGA_MAKESFX);
		BOOL bMoveFile = IsDlgButtonChecked(IDC_BGA_MOVEFILE);
		BOOL bNewOnly = IsDlgButtonChecked(IDC_BGA_NEWONLY);

		CComboBox combo = GetDlgItem(IDC_BGA_PATHTREAT);
		switch (combo.GetCurSel()) {
		case 0:  m_pd->m_pathmode = BASEDIR_NOPATH; break;
		case 2:  m_pd->m_pathmode = BASEDIR_ABSOLUTE; break;
		default: m_pd->m_pathmode = BASEDIR_RELATIVE; break;
		}

		CString cmd;
		cmd = bMoveFile ? _T("m ") : _T("a ");					// 移動または更新
		if (bNewOnly)											// 新ファイルのみ
			cmd += _T("-n ");
		cmd += _T("-a ");										// 属性保存
		if (m_pd->m_pathmode != BASEDIR_NOPATH) {				// パス
			if (m_pd->m_incdir)
				cmd += _T("-r ");
		}

		// 圧縮レベル
		CString str;
		CLccSliderCtrl slider = GetDlgItem(IDC_BGA_SLIDE);
		int n = slider.GetPos();
		SetRegValue(IDS_SEC_ARCHIVES, IDS_KEY_BGALEVEL, n);
		str.Format(_T("-l%d "), n);
		cmd += str;

		// 圧縮方式 (BGA or GZA)
		n = GetCheckedRadioButton(m_hWnd, IDC_BGA_BZA, IDC_BGA_GZA);
		cmd += (n == IDC_BGA_BZA) ? _T("-m2 ") : _T("-m1 ");
		SetRegValue(IDS_SEC_ARCHIVES, IDS_KEY_BGAPACKMODE, n - IDC_BGA_BZA);

		LPTSTR p = PathFindExtension(m_pd->m_arcname);
		if (*p == '.') {
			if (n == IDC_BGA_BZA && StrCmpI(p, EXTENSION_BZA))
				m_pd->m_arcname += EXTENSION_BZA;
			else if (n != IDC_BGA_BZA && StrCmpI(p, EXTENSION_GZA))
				m_pd->m_arcname += EXTENSION_GZA;
		}

		TCHAR buf[MAX_PATH * 2];
		lstrcpy(buf, m_pd->m_arcname);
		PathQuoteSpaces(buf);
		cmd += buf;

		if (!m_pd->m_base.IsEmpty() && m_pd->m_pathmode != BASEDIR_ABSOLUTE) {
			lstrcpy(buf, m_pd->m_base);
			PathAddBackslash(buf);
			PathQuoteSpaces(buf);
			cmd += _T(" ");
			cmd += buf;
		}

		MakeTempFile(m_pd->m_hdrop, buf, m_pd->m_pathmode, false, ARC_BGA, (m_pd->m_unicode != false));
		m_pd->m_temp = buf;
		cmd += _T(" @");
		cmd += buf;

		m_pd->m_cmd = cmd;
		return TRUE;
	}
};

