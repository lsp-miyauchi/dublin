#pragma once
//////////////////////////////////////////////////////////////////////
// CIshPage

#define MAX_ISH_MODE	4
static CMDNAME cnIshMode[MAX_ISH_MODE] = {
	_T("Shift JIS"),				_T("/ss "),
	_T("JIS8"),						_T("/s8 "),
	_T("JIS7"),						_T("/s7 "),
	_T("Shift JIS(カナ未使用)"),	_T("/sn ")
};

#define MAX_ISH_OS		6
static CMDNAME cnIshOS[] = {
	_T("MS-DOS"),					_T("/m "),
	_T("UNIX"),						_T("/u "),
	_T("CP/M"),						_T("/c "),
	_T("OS-9"),						_T("/9 "),
	_T("その他のOS"),				_T("/? "),
	_T("全てのOS"),					_T("/* ")
};


class CIshPage : public CDialogImpl<CIshPage>, public CArchiverPageImpl<CIshPage>
{
public:
	enum { IDD = IDD_PACK_ISH };

	BEGIN_MSG_MAP(CIshPage)
		CHAIN_MSG_MAP(CArchiverPageImpl<CIshPage>)
	END_MSG_MAP()

	BOOL OnInitDialog()
	{
		CComboBox combo = GetDlgItem(IDC_ISH_MODE);
		int i;
		for (i = 0; i < MAX_ISH_MODE; i++)
			combo.AddString(cnIshMode[i].pszName);
		combo.SetCurSel(0);

		combo = GetDlgItem(IDC_ISH_OS);
		for (i = 0; i < MAX_ISH_OS; i++)
			combo.AddString(cnIshOS[i].pszName);
		combo.SetCurSel(0);
		return FALSE;
	}

	BOOL OnOK()
	{
		CString cmd;
		TCHAR szName[MAX_PATH * 2];
		int cnt = DragQueryFile(m_pd->m_hdrop, 0xFFFFFFFF, NULL, 0);
		for (int i = 0; i < cnt; i++) {
			DragQueryFile(m_pd->m_hdrop, i, szName, ARRAYSIZE(szName));
			if (PathFileExists(szName) && !PathIsDirectory(szName)) {
				if (StrChr(szName, ' '))
					GetShortPathName(szName, szName, ARRAYSIZE(szName));
				cmd += szName;
				cmd += _T(" ");
			}
		}

		CComboBox combo = GetDlgItem(IDC_ISH_MODE);
		int iSel = combo.GetCurSel();
		if (iSel == CB_ERR)
			iSel = 0;
		cmd += cnIshMode[iSel].pszCmd;

		combo = GetDlgItem(IDC_ISH_OS);
		iSel = combo.GetCurSel();
		if (iSel == CB_ERR)
			iSel = 0;
		cmd += cnIshOS[iSel].pszCmd;
		cmd += _T("/f=");

		TCHAR pack[MAX_PATH * 2];
		lstrcpy(pack, m_pd->m_arcname);
		if (StrChr(pack, ' '))
			GetShortPathName(pack, pack, ARRAYSIZE(pack));
		cmd += pack;

		m_pd->m_cmd = cmd;
		return TRUE;
	}
};

