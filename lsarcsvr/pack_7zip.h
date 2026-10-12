#pragma once

//////////////////////////////////////////////////////////////////////
// C7zPage

extern CSimpleArray<CString> Split(LPCTSTR, LPCTSTR);

#define STAT_7Z_COMPRESSHEADER		0x00000001
#define STAT_7Z_SOLIDARCHIVE			0x00000002

typedef struct tagPARAM7ZIP {
	int method;
	int level;
	unsigned int status;
} PARAM7ZIP;

class C7zPage : public CDialogImpl<C7zPage>, public CArchiverPageImpl<C7zPage>
{
private:
	bool m_bVolumeUnitIsKB;
	PARAM7ZIP m_param = { 0, 4, STAT_7Z_COMPRESSHEADER };
	CComboBox m_cbPath;
	CComboBox m_cbMethod;
	CTrackBarCtrl m_slider;
	LPCTSTR m_methodNames = _T("LZMA|Deflate|BZip2|TAR|Copy");

public:
	enum { IDD = IDD_PACK_7ZIP };

	BEGIN_MSG_MAP(C7zPage)
		CHAIN_MSG_MAP(CArchiverPageImpl<C7zPage>)
		ON_BN_CLICKED(IDC_7Z_MAKESFX, OnMakeSfx)
		ON_CBN_SELCHANGE(IDC_7Z_METHOD, OnCheckMode)
		// @
		ON_BN_CLICKED(IDC_7Z_VOLUME_UNIT, OnVolumeUnit)
		ON_EN_CHANGE(IDC_7Z_VOLUME_VALUE, OnVolumeChange)
	END_MSG_MAP()

	void OnMakeSfx()
	{
		m_pd->m_sfx = IsDlgButtonChecked(IDC_7Z_MAKESFX);
	}

	BOOL OnInitDialog()
	{
		InitDlgCtrl(GetDlgItem(IDC_7Z_PATHTREAT), IDS_PATHTREAT);
    GetRegBinary(IDS_SEC_ARCHIVES, IDS_KEY_7ZIP, &m_param, sizeof(m_param));

		m_cbPath = GetDlgItem(IDC_7Z_PATHTREAT);
		m_cbPath.DeleteString(0);		// 「パス名無し」のオプションは無い
		m_cbPath.SetCurSel(0);

		// 圧縮レベル
		m_slider = GetDlgItem(IDC_7Z_SLIDE);
		m_slider.SetRange(0, 4);
		m_slider.SetPos(m_param.level);

		// 圧縮方式
		m_cbMethod = GetDlgItem(IDC_7Z_METHOD);
		CSimpleArray<CString> result = Split(m_methodNames, _T("|"));
		for (int i = 0; i < result.GetSize(); i++) {
			m_cbMethod.AddString(result[i]);
		}
		m_cbMethod.SetCurSel(m_param.method);

		//
		if (m_param.status & STAT_7Z_COMPRESSHEADER)
			CheckDlgButton(IDC_7Z_COMPRESSHEADER, TRUE);			// ヘッダ圧縮
		if (m_param.status & STAT_7Z_SOLIDARCHIVE)
			CheckDlgButton(IDC_7Z_SOLIDARCHIVE, TRUE);				// ソリッド書庫

		// 除外条件
		CEdit edit = GetDlgItem(IDC_7Z_EXCLUDE);
		CString str; str.LoadString(IDS_EXCLUDE_FILTER);
		edit.SetCueBannerText(str);

		// マルチボリューム
		CUpDownCtrl spin = GetDlgItem(IDC_7Z_VOLUME_SPIN);
		spin.SetRange32(0, 4096);
		spin.SetPos32(0);

		m_bVolumeUnitIsKB = false;
		return FALSE;
	}

	BOOL OnOK()
	{
		// パス名の付加
		switch (m_cbPath.GetCurSel()) {
			case 1:  m_pd->m_pathmode = BASEDIR_ABSOLUTE; break;
			default: m_pd->m_pathmode = BASEDIR_RELATIVE; break;
		}

		// 書庫を7zタイプで追加
		TCHAR buf[MAX_PATH * 2];
		lstrcpy(buf, m_pd->m_arcname);
		PathQuoteSpaces(buf);

		CString cmd;
		cmd.Format(_T("a -t7z %s "), buf);

		// サブディレクトリ有り
		if (m_pd->m_incdir)
			cmd += _T("-r0 ");

		// 圧縮方式
		m_param.method = max(0, m_cbMethod.GetCurSel());
		int id = m_param.method + IDC_7Z_METHOD_LZMA;
		cmd += _T("-m0=");
		switch (id) {
    case IDC_7Z_METHOD_LZMA:
			cmd += _T("LZMA ");
			break;
		case IDC_7Z_METHOD_DEFLATE:
			cmd += _T("Deflate ");
			break;
		case IDC_7Z_METHOD_BZIP2:
			cmd += _T("BZip2 ");
			break;
		case IDC_7Z_METHOD_PPMD:
			cmd += _T("PPMd ");
			break;
		case IDC_7Z_METHOD_COPY:
    default:
			cmd += _T("Copy ");
			break;
		}

		// 書庫の形式
		m_param.status = 0;
		cmd += _T("-mhc=");
		if (IsDlgButtonChecked(IDC_7Z_COMPRESSHEADER)) {
			cmd += _T("on ");
			m_param.status |= STAT_7Z_COMPRESSHEADER;
		}
		else
			cmd += _T("off ");

		// ソリッド書庫
		cmd += _T("-ms=");
		if (IsDlgButtonChecked(IDC_7Z_SOLIDARCHIVE)) {
			cmd += _T("on ");
			m_param.status |= STAT_7Z_SOLIDARCHIVE;
		}
		else
			cmd += _T("off ");

		// マルチボリューム
		int vol = GetDlgItemInt(IDC_7Z_VOLUME_VALUE, NULL, FALSE);
		if (vol > 0)
			cmd.AppendFormat(_T("-v%d%c "), vol, (m_bVolumeUnitIsKB ? 'k' : 'm'));

		// 除外するファイルのフィルタ
		CString str;
		if (GetDlgItemText(IDC_7Z_EXCLUDE, str) > 0) {
			int pos = 0;
			CString token = str.Tokenize(_T(";"), pos);
			while (token != _T("")) {
				if (token.Find(_T('?')) >= 0 || token.Find(_T('*')) >= 0)
					cmd.AppendFormat(_T("-x!%s "), token.GetString());
				else
					cmd.AppendFormat(_T("-x%s "), token.GetString());
				token = str.Tokenize(_T(";"), pos);
			}
		}

		// 自己解凍形式書庫 (マルチボリュームでなく、LZMA または PPMD 圧縮方式の場合のみ）
		m_pd->m_sfx = IsDlgButtonChecked(IDC_7Z_MAKESFX);
		if (m_pd->m_sfx) {
			if (vol == 0)
				cmd += _T("-sfx ");
			else
				m_pd->m_sfx = FALSE;
		}

		// パスワード
		if (IsDlgButtonChecked(IDC_7Z_PASSWORD))
			cmd += _T("-p ");

		// 圧縮レベル
		m_param.level = m_slider.GetPos();
		wsprintf(buf, _T("-mx%d "), (m_param.level != 0) ? m_param.level * 2 + 1 : 0);
		cmd += buf;

		// 基準ディレクトリ
		if (m_pd->m_pathmode == BASEDIR_ABSOLUTE) {
			if (GetPathCommonPrefix(m_pd->m_hdrop, buf)) {
				PathStripToRoot(buf);
				cmd.AppendFormat(_T(" %s"), buf);
			}
		}

		// 作業ファイル
		MakeTempFile(m_pd->m_hdrop, &buf[1], m_pd->m_pathmode, false, ARC_7ZIP, (m_pd->m_unicode != false), (m_pd->m_utf8 != false));
		m_pd->m_temp = &buf[1];
		buf[0] = '@';
		PathQuoteSpaces(buf);
		cmd.AppendFormat(_T(" -ssw %s"), buf);	// 開いているファイルも格納できる -ssw
		m_pd->m_cmd = cmd;

		SetRegBinary(IDS_SEC_ARCHIVES, IDS_KEY_7ZIP, &m_param, sizeof(m_param));
		return TRUE;
	}

	void OnCheckMode()
	{
		int sel = m_cbMethod.GetCurSel() + IDC_7Z_METHOD_LZMA;
		BOOL bEnable = sel != IDC_7Z_METHOD_COPY;
		GetDlgItem(IDC_7Z_SLIDE).EnableWindow(bEnable);
		GetDlgItem(IDC_7Z_STATIC_COMPRESS).EnableWindow(bEnable);
		GetDlgItem(IDC_7Z_STATIC_LOW).EnableWindow(bEnable);
		GetDlgItem(IDC_7Z_STATIC_HIGH).EnableWindow(bEnable);
		GetDlgItem(IDC_7Z_COMPRESSHEADER).EnableWindow(bEnable);
	}

	void OnVolumeUnit()
	{
		if (m_bVolumeUnitIsKB) {
			m_bVolumeUnitIsKB = false;
			SetDlgItemText(IDC_7Z_VOLUME_UNIT, _T("MB"));
		}
		else {
			m_bVolumeUnitIsKB = true;
			SetDlgItemText(IDC_7Z_VOLUME_UNIT, _T("KB"));
		}
	}

	void OnVolumeChange()
	{
		BOOL bEnable = (GetDlgItemInt(IDC_7Z_VOLUME_VALUE, NULL, FALSE) == 0);
		GetDlgItem(IDC_7Z_MAKESFX).EnableWindow(bEnable);
	}

};

