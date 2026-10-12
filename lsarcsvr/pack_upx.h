#pragma once
//////////////////////////////////////////////////////////////////////
// CUpxPage

#ifdef LSARC_UPX_SUPPORT
typedef struct tagUPXINFO {
	int compression_level;
	int compress_icon;
	bool keep_backup : 1;
	bool force_compression : 1;
	bool strip_extradata : 1;
	bool compress_resources : 1;
	bool compress_exports : 1;
	bool strip_relocations : 1;
} UPXINFO;

static void get_upxdefault(UPXINFO* info)
{
	info->compression_level = 9;
	info->compress_icon = 2;
	info->keep_backup = false;
	info->strip_extradata = false;
	info->compress_resources = true;
	info->compress_exports = true;
	info->strip_relocations = true;
	info->force_compression = false;
}

class CUpxPage : public CDialogImpl<CUpxPage>, public CArchiverPageImpl<CUpxPage>
{
public:
	enum { IDD = IDD_PACK_UPX };
	TCHAR m_target[MAX_PATH * 2];

	BEGIN_MSG_MAP(CUpxPage)
		CHAIN_MSG_MAP(CArchiverPageImpl<CUpxPage>)
		ON_BN_CLICKED(IDC_UPX_BROWSE, OnBrowse)
		ON_BN_CLICKED(IDC_UPX_RESET, OnReset)
	END_MSG_MAP()

	void OnBrowse()
	{
		TCHAR buf[MAX_PATH * 2];
		GetDlgItemText(IDC_UPX_PATH, buf, ARRAYSIZE(buf));
		if (FileBrowse(TRUE, m_hWnd, IDS_CAPTION_EXEMODULE, IDS_FILTER_EXEMODULE, buf)) {
			SetDlgItemText(IDC_UPX_PATH, buf);
			SetRegText(IDS_SEC_ARCHIVES, IDS_KEY_UPXPATH, buf);
		}
	}

	void OnReset()
	{
		UPXINFO info;
		get_upxdefault(&info);
		SetParams(info);
	}

	BOOL OnInitDialog()
	{
		InitDlgCtrl(GetDlgItem(IDC_UPX_ICON), IDS_PACK_UPX_ICON);
		lccDisableCharInput(GetDlgItem(IDC_UPX_TARGET));
		DragQueryFile(m_pd->hDrop, 0, m_target, ARRAYSIZE(m_target));
		HMODULE hmod = LoadLibraryEx(m_target, NULL, LOAD_LIBRARY_AS_DATAFILE);
		SetDlgItemText(IDC_UPX_TARGET, m_target);

		UPXINFO info;
		if (GetRegBinary(IDS_SEC_ARCHIVES, IDS_KEY_UPXINFO, &info, sizeof(info)) != sizeof(info))
			get_upxdefault(&info);

		CLccSliderCtrl slider = GetDlgItem(IDC_UPX_SLIDE);
		slider.SetRange(1, 10);
		SetParams(info);

		TCHAR buf[MAX_PATH * 2];
		buf[0] = '\0';
		if (GetRegText(IDS_SEC_ARCHIVES, IDS_KEY_UPXPATH, buf, ARRAYSIZE(buf)) < 1) {
			LPTSTR p;
			if (SearchPath(NULL, _T("upx.exe"), NULL, ARRAYSIZE(buf), buf, &p) < 1 || !PathFileExists(buf))
				buf[0] = '\0';
		}
		SetDlgItemText(IDC_UPX_PATH, buf);
		return FALSE;
	}

	BOOL OnOK()
	{
		// ターゲットファイルの確認
		HMODULE hmod = LoadLibraryEx(m_target, NULL, LOAD_LIBRARY_AS_DATAFILE);
		if (!hmod) {
			AppMessage(IDS_UPX_NOTEXECUTABLE, m_target);
			return FALSE;
		}
		else
			FreeLibrary(hmod);

		// upx.exe の確認
		TCHAR buf[MAX_PATH * 2];
		if (GetDlgItemText(IDC_UPX_PATH, buf, ARRAYSIZE(buf)) < 1) {
			GetDlgItem(IDC_UPX_PATH).SetFocus();
			return FALSE;
		}
		GetShortPathName(buf, buf, ARRAYSIZE(buf));

		CString cmd = buf;

		UPXINFO info;
		CLccSliderCtrl slider = GetDlgItem(IDC_UPX_SLIDE);
		info.compression_level = slider.GetPos();

		CComboBox combo = GetDlgItem(IDC_UPX_ICON);
		info.compress_icon = combo.GetCurSel();

		info.keep_backup = (IsDlgButtonChecked(IDC_UPX_KEEPBACKUP) != FALSE);
		info.strip_extradata = (IsDlgButtonChecked(IDC_UPX_EXTRADATA) != FALSE);
		info.strip_relocations = (IsDlgButtonChecked(IDC_UPX_RELOCATION) != FALSE);
		info.compress_resources = (IsDlgButtonChecked(IDC_UPX_RESOURCE) != FALSE);
		info.compress_exports = (IsDlgButtonChecked(IDC_UPX_EXPORT) != FALSE);
		info.force_compression = (IsDlgButtonChecked(IDC_UPX_FORCE) != FALSE);

		CString str;
		// 圧縮レベル
		if (info.compression_level < 10) {
			str.Format(_T(" -%d"), info.compression_level);
			cmd += str;
		}
		else
			cmd += _T(" --best");
		// バックアップ作成
		if (info.keep_backup)
			cmd += _T(" -k");
		// 強制圧縮
		if (info.force_compression)
			cmd += _T(" -f");
		// アイコン圧縮
		str.Format(_T(" --compress-icon=%d"), info.compress_icon);
		cmd += str;
		// リソース圧縮
		if (!info.compress_resources)
			cmd += _T(" --compress-resources=0");
		// EXPORTS圧縮
		if (!info.compress_exports)
			cmd += _T(" --compress-exports=0");
		// リロケーションレコード除去
		if (!info.strip_relocations)
			cmd += _T(" --strip-relocs=0");
		// EXTRA_DATA除去
		if (!info.strip_extradata)
			cmd += _T(" --overlay=strip");

		// 出力先
		lstrcpy(buf, m_target);
		PathRemoveFileSpec(buf);
		if (ComparePath(buf, m_pd->arcname)) {
			PathCombine(buf, m_pd->arcname, PathFindFileName(m_target));
			str.Format(_T(" -o \"%s\""), buf);
			cmd += str;
		}

		// ターゲットファイル
		str.Format(_T(" \"%s\""), m_target);
		cmd += str;

		SetRegBinary(IDS_SEC_ARCHIVES, IDS_KEY_UPXINFO, &info, sizeof(info));

		lstrcpy(m_pd->cmd, cmd);
		return TRUE;
	}


	void SetParams(UPXINFO& info)
	{
		CLccSliderCtrl slider = GetDlgItem(IDC_UPX_SLIDE);
		slider.SetPos(info.compression_level);

		CheckDlgButton(IDC_UPX_KEEPBACKUP, info.keep_backup == true);
		CheckDlgButton(IDC_UPX_EXTRADATA, info.strip_extradata == true);
		CheckDlgButton(IDC_UPX_RESOURCE, info.compress_resources == true);
		CheckDlgButton(IDC_UPX_RELOCATION, info.strip_relocations == true);
		CheckDlgButton(IDC_UPX_EXPORT, info.compress_exports == true);
		CheckDlgButton(IDC_UPX_FORCE, info.force_compression == true);

		CComboBox combo = GetDlgItem(IDC_UPX_ICON);
		combo.SetCurSel(info.compress_icon);
	}
};
#endif
