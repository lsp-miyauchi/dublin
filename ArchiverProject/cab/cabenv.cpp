#include "stdafx.h"
#include "cab.h"


////////////////////////////////////////////////////////////
//

class CPathTreatDlg : public CDialogImpl<CPathTreatDlg>
{
public:
	int m_treat;
	bool m_yesall;

public:
	CPathTreatDlg()
		: m_treat(BRT_QUERY)
		, m_yesall(false)
	{
	}
	~CPathTreatDlg()
	{
	}

	enum { IDD = IDD_PATHTREAT };

BEGIN_MSG_MAP(CPathTreatDlg)
	ON_WM_INITDIALOG()
	ON_BN_CLICKED(IDOK, OnOK)
	ON_BN_CLICKED(IDC_YESTOALL, OnYesToAll)
	ON_BN_CLICKED(IDCANCEL, OnCancel)
END_MSG_MAP()

	BOOL OnInitDialog(LPARAM /*lParam*/)
	{
		CenterWindow();

		UINT id = IDC_TREAT_ERROR;
		switch (m_treat) {
		case BRT_EXTRACT:	id = IDC_TREAT_EXTRACT; break;
		case BRT_IGNORE:	id = IDC_TREAT_IGNORE; break;
		case BRT_STRIPPATH:	id = IDC_TREAT_STRIPPATH; break;
		case BRT_RELATIVE:	id = IDC_TREAT_RELATIVE; break;
		}

		CheckDlgButton(id, TRUE);
		return TRUE;
	}
	void OnOK()
	{
		if      (IsDlgButtonChecked(IDC_TREAT_EXTRACT))		m_treat = BRT_EXTRACT;
		else if (IsDlgButtonChecked(IDC_TREAT_IGNORE))		m_treat = BRT_IGNORE;
		else if (IsDlgButtonChecked(IDC_TREAT_STRIPPATH))	m_treat = BRT_STRIPPATH;
		else if (IsDlgButtonChecked(IDC_TREAT_RELATIVE))	m_treat = BRT_RELATIVE;
		else if (IsDlgButtonChecked(IDC_TREAT_ERROR))		m_treat = BRT_ERROR;
		EndDialog(IDOK);
	}
	void OnCancel()
	{
		EndDialog(IDCANCEL);
	}
	void OnYesToAll()
	{
		m_yesall = true;
		OnOK();
	}
};

////////////////////////////////////////////////////////////
//

CCabEnv::CCabEnv()
{
	WCHAR buf[MAX_PATH * 2];

	m_hwndOwner = NULL;
	m_chCmd = CABCMD_NONE;
	m_cmdOption = 0;
	m_nVolumeSize = CB_MAX_DISK_NEW;
	m_strDefaultPath.Empty();
	m_strCabName.Empty();
	m_strBasePath.Empty();
	m_szTemp[0] = 0;
	m_sSetID = CABSETID;
	m_nCompressionLevel = GetRegInt(NULL, L"LzwCompressLevel", 21);
	m_nMemoryLevel = GetRegInt(NULL, L"QuantumMemoryLevel", 21);
	m_nCompressionMode = GetRegInt(NULL, L"CompressionMode", CAB_COMPRESSION_LZX);
	m_strWorkPath.Empty();
	if (GetRegString(NULL, L"WorkPath", buf, ARRAYSIZE(buf)) > 0)
		m_strWorkPath = buf;
	m_nNumOfFiles = 0;
	m_strStored.Empty();
	m_strExcludes.Empty();
	m_strResult.Empty();
	m_brittle = BRT_QUERY;
}

CCabEnv::~CCabEnv()
{
}

int CCabEnv::BuildTempPathName(LPCWSTR pszFile)
{
	// 戻り値 -1=エラー, 0=無視, 1=正常

	if (!pszFile || *pszFile == 0) return -1;

	if (*pszFile == '\\' ||
		*(pszFile + 1) == ':' ||
		*pszFile == '.' && (*(pszFile + 1) == ':' || (*(pszFile + 1) == '.' && *(pszFile + 2) == '\\'))) {

		if (!(m_brittle & BRT_YESALL)) {
			// 問い合わせる
			CPathTreatDlg dlg;
			dlg.m_treat = m_brittle;
			if (dlg.DoModal(m_hwndOwner) != IDOK)
				return -1;
			m_brittle = dlg.m_treat;
			if (dlg.m_yesall)
				m_brittle |= BRT_YESALL;
		}

		switch ((m_brittle & BRT_TYPEMASK)) {
		case BRT_ERROR:
			return -1;
		case BRT_IGNORE:
			return 0;
		case BRT_EXTRACT:
			if (*(pszFile + 1) == ':')
				wcscpy(m_szTemp, pszFile);
			else {
				WCHAR buf[MAX_PATH * 2];
				PathCombine(buf, m_strBasePath, pszFile);
				PathCanonicalize(m_szTemp, buf);
			}
			break;
		case BRT_STRIPPATH:
			PathCombine(m_szTemp, m_strBasePath, PathFindFileName(pszFile));
			break;
		case BRT_RELATIVE:
			while (*pszFile == '.' || *pszFile == '\\')
				pszFile++;
			if (*(pszFile + 1) == ':')
				pszFile = PathSkipRoot(pszFile);
			PathCombine(m_szTemp, m_strBasePath, pszFile);
			break;
		}
	}
	else
		PathCombine(m_szTemp, m_strBasePath, pszFile);
	return 1;
}

void CCabEnv::msgout(LPCWSTR fmt, ...)
{
	WCHAR* pOutput = new WCHAR[1024];
	va_list ap;
	va_start(ap, fmt);
	wvsprintf(pOutput, fmt, ap);
	va_end (ap);

	m_strResult += pOutput;

	delete [] pOutput;
}

BOOL CCabEnv::GetWorkingPath(LPWSTR pszPath, int cchMax)
{
	BOOL bOK = FALSE;
	if (pszPath && cchMax > 0) {
		if (!m_strWorkPath.IsEmpty() &&
			PathIsDirectory(m_strWorkPath) &&
			m_strWorkPath.GetLength() < cchMax) {
			wcscpy(pszPath, m_strWorkPath);
			bOK = TRUE;
		}
	}
	return bOK;
}


