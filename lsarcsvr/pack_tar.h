#pragma once
//////////////////////////////////////////////////////////////////////
// CTarPage

class CTarPage : public CDialogImpl<CTarPage>, public CArchiverPageImpl<CTarPage>
{
private:
	CComboBox m_cbPath;
	CComboBox m_cbMethod;
	CTrackBarCtrl m_slider;
	LPCTSTR m_methodNames = _T("Zstandard|Gzip|Bzip2|Lzma|xz|Store");

public:
	enum { IDD = IDD_PACK_TAR };

	BEGIN_MSG_MAP(CTarPage)
		CHAIN_MSG_MAP(CArchiverPageImpl<CTarPage>)
		ON_CBN_SELCHANGE(IDC_TAR_METHOD, OnSelChangeMethod)
	END_MSG_MAP()

	BOOL OnInitDialog()
	{
		InitDlgCtrl(GetDlgItem(IDC_TAR_PATHTREAT), IDS_PATHTREAT);
		m_cbPath = GetDlgItem(IDC_TAR_PATHTREAT);
		m_cbPath.SetCurSel(m_pd->m_incdir ? 1 : 0);

		// 圧縮方式
    // bzip2(1-9), gzip(1-9), lzma(0-9), xz(0-9), zstd(1-19 20-22 は --zstd-ultraオプション)
		m_cbMethod = GetDlgItem(IDC_TAR_METHOD);
		CSimpleArray<CString> result = Split(m_methodNames, _T("|"));
		for (int i = 0; i < result.GetSize(); i++) {
			m_cbMethod.AddString(result[i]);
		}
		int val = GetRegValue(IDS_SEC_ARCHIVES, IDS_KEY_TARGZIP, 1);
		m_cbMethod.SetCurSel(val);

		// 圧縮レベル
		m_slider = GetDlgItem(IDC_TAR_SLIDE);
		m_slider.SetRange(1, 9);
		val = GetRegValue(IDS_SEC_ARCHIVES, IDS_KEY_TARLEVEL, 9);
		m_slider.SetPos(val);

		OnSelChangeMethod();
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

		int sel = m_cbMethod.GetCurSel();
		SetRegValue(IDS_SEC_ARCHIVES, IDS_KEY_TARGZIP, sel);
		sel += IDC_TAR_METHOD_ZSTD;

		CString cmd = _T("-c ");
		if (m_pd->m_pathmode == BASEDIR_NOPATH)		// サブフォルダ検索なし
			cmd += _T("--use-directory=0 ");

		// 圧縮レベル
		int n = m_slider.GetPos();
		switch(sel) {
			case IDC_TAR_METHOD_ZSTD:		// Zstandard
				cmd.AppendFormat(_T("--zstd=%d' "), n * 2);
				break;
			case IDC_TAR_METHOD_GZIP:		// gzip
				cmd.AppendFormat(_T("-z%d "), n);
				break;
			case IDC_TAR_METHOD_BZIP2:	// bzip2
				cmd.AppendFormat(_T("-B%d "), n);
				break;
			case IDC_TAR_METHOD_LZMA:		// lzma
				cmd.AppendFormat(_T("--lzma=%d "), n);
				break;
			case IDC_TAR_METHOD_XZ:		// xz
				cmd.AppendFormat(_T("-J%d "), n);
				break;
		}
		SetRegValue(IDS_SEC_ARCHIVES, IDS_KEY_TARLEVEL, n);

		TCHAR buf[MAX_PATH * 2];
		lstrcpy(buf, m_pd->m_arcname);
		LPTSTR p = PathFindExtension(buf);
		if (*p != '.') {
			lstrcat(buf, _T(".tar"));
			switch (sel) {
				case IDC_TAR_METHOD_ZSTD:
					lstrcat(buf, _T(".zst"));
					break;
				case IDC_TAR_METHOD_GZIP:
					lstrcat(buf, _T(".gz"));
					break;
				case IDC_TAR_METHOD_BZIP2:
					lstrcat(buf, _T(".bz2"));
					break;
				case IDC_TAR_METHOD_LZMA:
					lstrcat(buf, _T(".lzma"));
					break;
				case IDC_TAR_METHOD_XZ:
					lstrcat(buf, _T(".xz"));
					break;
			}
		}
		PathQuoteSpaces(buf);
		cmd.AppendFormat(_T("-cf %s "), buf);

		// 作業ファイル
		MakeTempFile(m_pd->m_hdrop, &buf[1], m_pd->m_pathmode, false, ARC_TAR, (m_pd->m_unicode != false));
		m_pd->m_temp = &buf[1];
		buf[0] = '@';
		PathQuoteSpaces(buf);
		cmd += buf;

		m_pd->m_cmd = cmd;
		return TRUE;
	}

	void OnSelChangeMethod()
	{
		int sel = m_cbMethod.GetCurSel();
		BOOL enable = (sel + IDC_TAR_METHOD_ZSTD) != IDC_TAR_METHOD_COPY;
		m_slider.EnableWindow(enable);
	}
};
