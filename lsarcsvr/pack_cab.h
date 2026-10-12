#pragma once
//////////////////////////////////////////////////////////////////////
// CCabPage

class CCabPage : public CDialogImpl<CCabPage>, public CArchiverPageImpl<CCabPage>
{
private:
	bool m_bVolumeUnitIsKB;
	CComboBox m_cbPath;
	CComboBox m_cbMethod;
	CTrackBarCtrl m_slider;
	LPCTSTR m_methodNames = _T("MSzip|LZX|Store");

public:
	enum { IDD = IDD_PACK_CAB };

	BEGIN_MSG_MAP(CCabPage)
		CHAIN_MSG_MAP(CArchiverPageImpl<CCabPage>)
		ON_BN_CLICKED(IDC_CAB_VOLUME_UNIT, OnVolumeUnit)
		ON_EN_CHANGE(IDC_CAB_VOLUME_VALUE, OnVolumeChange)
		ON_CBN_SELCHANGE(IDC_CAB_METHOD, OnCheckMethod)
	END_MSG_MAP()

	BOOL OnInitDialog()
	{
		InitDlgCtrl(GetDlgItem(IDC_CAB_PATHTREAT), IDS_PATHTREAT);
		m_cbPath = GetDlgItem(IDC_CAB_PATHTREAT);
		m_cbPath.SetCurSel(m_pd->m_incdir ? 1 : 0);

		// 圧縮方式
		m_cbMethod = GetDlgItem(IDC_CAB_METHOD);
		CSimpleArray<CString> result = Split(m_methodNames, _T("|"));
		for (int i = 0; i < result.GetSize(); i++) {
			m_cbMethod.AddString(result[i]);
		}
		int val = GetRegValue(IDS_SEC_ARCHIVES, IDS_KEY_CABPACKMODE, -1);
		m_cbMethod.SetCurSel(val);

		// 圧縮レベル
		m_slider = GetDlgItem(IDC_CAB_SLIDE);
		m_slider.SetRange(15, 21);
		val = GetRegValue(IDS_SEC_ARCHIVES, IDS_KEY_CABLEVEL, 21);
		m_slider.SetPos(val);

		// 除外条件
		CEdit edit = GetDlgItem(IDC_CAB_EXCLUDE);
		CString str; str.LoadString(IDS_EXCLUDE_FILTER);
		edit.SetCueBannerText(str);

		// マルチボリューム
		CUpDownCtrl spin = GetDlgItem(IDC_CAB_VOLUME_SPIN);
		spin.SetRange32(0, 4096);
		spin.SetPos32(0);
		m_bVolumeUnitIsKB = false;

		OnCheckMethod();
		return FALSE;
	}

	BOOL OnOK()
	{
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
		cmd = _T("a ");
		if (m_pd->m_incdir) {
			// サブフォルダ検索
			cmd += _T("-r ");
			if (m_pd->m_pathmode == BASEDIR_NOPATH)
				cmd += _T("-j ");		// パス名を付加しない
		}

		// 除外するファイルのフィルタ
		CString str;
		if (GetDlgItemText(IDC_CAB_EXCLUDE, str) > 0) {
			cmd.AppendFormat(_T("-ze%s "), str.GetString());
		}

		// マルチボリューム
		int vol = GetDlgItemInt(IDC_CAB_VOLUME_VALUE, NULL, FALSE);
		if (vol > 0) {
			if (!m_bVolumeUnitIsKB)
				vol *= 1024;
			cmd.AppendFormat(_T("-v:%d "), vol);
		}

		// 自己解凍形式書庫 (マルチボリュームでない場合）
#ifdef _CABSFX_SUPPORT
		m_pd->m_sfx = (vol == 0) ? IsDlgButtonChecked(IDC_CAB_MAKESFX) : FALSE;
#endif
		int n = m_cbMethod.GetCurSel() + IDC_CAB_MSZIP;
		switch (n) {
			case IDC_CAB_MSZIP:
				cmd += _T("-mz ");
				break;
			case IDC_CAB_LZX: {
				// 圧縮レベル
				int r = m_slider.GetPos();
				SetRegValue(IDS_SEC_ARCHIVES, IDS_KEY_CABLEVEL, r);
				str.Format(_T("-ml:%d "), r);
				cmd += str;
				break;
			}
			case IDC_CAB_STORE:
			default:
				cmd += _T("-ms ");
				break;
		}
		SetRegValue(IDS_SEC_ARCHIVES, IDS_KEY_CABPACKMODE, n - IDC_CAB_MSZIP);

		TCHAR buf[MAX_PATH * 2];
		lstrcpy(buf, m_pd->m_arcname);
		PathQuoteSpaces(buf);
		cmd += buf;

		// 作業ファイル
		MakeTempFile(m_pd->m_hdrop, &buf[2], m_pd->m_pathmode, false, ARC_CAB, (m_pd->m_unicode != false));
		m_pd->m_temp = &buf[1];
		buf[0] = '@';
		buf[1] = '@';
		PathQuoteSpaces(buf);
		cmd += _T(' ');
		cmd += buf;

		m_pd->m_cmd = cmd;
		return TRUE;
	}

	void OnCheckMethod()
	{
		int sel = m_cbMethod.GetCurSel() + IDC_CAB_MSZIP;
		m_slider.EnableWindow(sel == IDC_CAB_LZX);
	}

	void OnVolumeUnit()
	{
		if (m_bVolumeUnitIsKB) {
			m_bVolumeUnitIsKB = false;
			SetDlgItemText(IDC_CAB_VOLUME_UNIT, _T("MB"));
		}
		else {
			m_bVolumeUnitIsKB = true;
			SetDlgItemText(IDC_CAB_VOLUME_UNIT, _T("KB"));
		}
	}

	void OnVolumeChange()
	{
#ifdef _CABSFX_SUPPORT
		BOOL bEnable = (GetDlgItemInt(IDC_CAB_VOLUME_VALUE, NULL, FALSE) == 0);
		GetDlgItem(IDC_CAB_MAKESFX).EnableWindow(bEnable);
#endif
	}

};

