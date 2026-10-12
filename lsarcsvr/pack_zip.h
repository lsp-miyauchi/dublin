#pragma once
//////////////////////////////////////////////////////////////////////
// CZipPage

class CZipPage : public CDialogImpl<CZipPage>, public CArchiverPageImpl<CZipPage>
{
private:
	CTrackBarCtrl m_slider;
	CComboBox m_cbPath;

public:
	enum { IDD = IDD_PACK_ZIP };

	BEGIN_MSG_MAP(CZipPage)
		CHAIN_MSG_MAP(CArchiverPageImpl<CZipPage>)
	END_MSG_MAP()

	BOOL OnInitDialog()
	{
		InitDlgCtrl(GetDlgItem(IDC_ZIP_PATHTREAT), IDS_PATHTREAT);
		m_cbPath = GetDlgItem(IDC_ZIP_PATHTREAT);
		m_cbPath.SetCurSel(m_pd->m_incdir ? 1 : 0);

		m_slider = GetDlgItem(IDC_ZIP_SLIDE);
		m_slider.SetRange(0, 9);
		int val = GetRegValue(IDS_SEC_ARCHIVES, IDS_KEY_ZIPLEVEL, 9);
		m_slider.SetPos(val);

		return FALSE;
	}

	BOOL OnOK()
	{
		BOOL bMoveFile = IsDlgButtonChecked(IDC_ZIP_MOVEFILE);
		BOOL bNewOnly = IsDlgButtonChecked(IDC_ZIP_NEWONLY);

		// システム・隠し属性ファイル
		CString cmd = _T("-S -X ");

		// 移動
		if (bMoveFile)
			cmd += _T("-m ");
		if (bNewOnly)
			cmd += _T("-u ");

		// サブディレクトリ有り
		if (m_pd->m_incdir)
			cmd += _T("-r ");

		// パス名の処理
		switch (m_cbPath.GetCurSel()) {
		case 0:  m_pd->m_pathmode = BASEDIR_NOPATH; cmd += _T("-j "); break;
		case 2:  m_pd->m_pathmode = BASEDIR_ABSOLUTE; break;
		default: m_pd->m_pathmode = BASEDIR_RELATIVE; break;
		}

		// 圧縮レベル
		int n = m_slider.GetPos();
		SetRegValue(IDS_SEC_ARCHIVES, IDS_KEY_ZIPLEVEL, n);
		CString str;
		str.Format(_T("-%d "), n);
		cmd += str;

		// パスワード
		if (IsDlgButtonChecked(IDC_ZIP_PASSWORD))
			cmd += _T("-e ");

		TCHAR buf[MAX_PATH * 2];
		lstrcpy(buf, m_pd->m_arcname);
		PathQuoteSpaces(buf);
		cmd += buf;

		// 作業ファイル
		MakeTempFile(m_pd->m_hdrop, &buf[1], m_pd->m_pathmode, false, ARC_ZIP, (m_pd->m_unicode != false));
		m_pd->m_temp = &buf[1];
		buf[0] = '@';
		PathQuoteSpaces(buf);
		cmd += _T(' ');
		cmd += buf;

		m_pd->m_cmd = cmd;
		return TRUE;
	}

};

