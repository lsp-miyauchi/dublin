// syncdata.cpp : implementation file
//

#include "stdafx.h"
#include <stdio.h>
#include "fopstd.h"
#include "syncdata.h"
#include "mapping_file.h"

BOOL SyncFolder(CSyncRelay* psr, CSyncData& sd);

static const TCHAR sszEnvDir[] = _T("%syncbasedir%");

///////////////////////////////////////////////////////////////////////
// フォルダ同期定義ファイルのベースフォルダ

void GetSyncBaseFolder(LPTSTR pszPath, int cchMax)
{
	if (GetRegText(_MKIR(IDS_SEC_FOLDER), _MKIR(IDS_KEY_SYNCBASE), pszPath, cchMax) < 1)
		fopGetSyncDataFolder(pszPath);
}

void GetSyncLogFolder(LPTSTR pszPath, int cchMax)
{
	if (GetRegText(_MKIR(IDS_SEC_SYNCREPORT), _MKIR(IDS_KEY_LOGFOLDER), pszPath, cchMax) < 1) {
		GetSyncBaseFolder(pszPath, cchMax);
		PathAppend(pszPath, _T("LogFiles"));
	}
	if (!_pathisdirectory(pszPath))
		SHCreateDirectory(NULL, pszPath);
}

BOOL SetSyncBaseFolder(LPCTSTR pszPath)
{
	if (!pszPath || *pszPath == 0) return FALSE;
	if (!_pathisdirectory(pszPath) && !fopMakeTree(pszPath)) return FALSE;

	SetRegText(_MKIR(IDS_SEC_FOLDER), _MKIR(IDS_KEY_SYNCBASE), pszPath);
	return TRUE;
}

extern "C"
int WINAPI fopExpandSyncPath(LPTSTR psz, int cchMax)
{
	if (!psz || *psz == 0 || cchMax < 1) return 0;

	int len = lstrlen(sszEnvDir);
	if (!StrCmpN(psz, sszEnvDir, len)) {
		LPTSTR p = psz + len;

		TCHAR buf[MAX_PATH * 2];
		GetSyncBaseFolder(buf, ARRAYSIZE(buf));
		PathAppend(buf, p);
		lstrcpyn(psz, buf, cchMax);
	}
	return lstrlen(psz);
}

extern "C"
int WINAPI fopSubstSyncPath(LPTSTR psz, int cchMax)
{
	if (!psz || *psz == 0 || cchMax < 1) return 0;

	TCHAR path[MAX_PATH * 2];
	GetSyncBaseFolder(path, ARRAYSIZE(path));
	int len = lstrlen(path);

	if (!StrCmpNI(psz, path, len)) {
		CString str = sszEnvDir;
		str += psz + len;
		lstrcpyn(psz, str, cchMax);
	}
	return lstrlen(psz);
}


///////////////////////////////////////////////////////////////////////
// CSyncDataクラス
//

BOOL CSyncData::ExecConfirm()
{
	CString mes;
	if (m_nDirection == DIRSYNC_SYNC)
		mes.Format(IDS_SYNC_EQUALIZE, m_strSrcFolder, m_strDstFolder);
	else {
		int id = m_bMirror ? IDS_SYNC_MIRROR : IDS_SYNC_BACKUP;
		if (m_nDirection == DIRSYNC_BACKUP)
			mes.Format(id, m_strSrcFolder, m_strDstFolder);
		else
			mes.Format(id, m_strDstFolder, m_strSrcFolder);
	}

	CSimpleArray<CString> array;
	SeparateString(mes, _T('\n'), array);
	int res = 0;
	if (TaskDialog(
			::GetActiveWindow(), RESINST,
			MAKEINTRESOURCE(IDS_TITLE_FOLDERSYNC),
			array[0].GetString(), array[1].GetString(),
			TDCBF_OK_BUTTON | TDCBF_CANCEL_BUTTON,
			/*TD_INFORMATION_ICON*/MAKEINTRESOURCE(IDI_DSYNC), &res) != S_OK || res != IDOK)
		return FALSE;
	return TRUE;
}


/////////////////////////////////////////////////////////////////////
// CScriptSync クラス


BOOL CScriptSync::LoadAnsi(LPCTSTR pszScript)
{
	// ANSIで読み込み
	CStringA str(pszScript);
	FILE* fp;
	if (!fopen_s(&fp, str, "rt")) {
		char buf[MAX_PATH * 2];
		while (fgets(buf, ARRAYSIZE(buf), fp)) {
			str = buf;
			str.TrimRight('\n');

			CStringW wtr(str);
			if (_pathfileexists(wtr))
				AddFile(wtr);
		}
		fclose(fp);
		m_strScript = pszScript;
		return TRUE;
	}
	return FALSE;
}

BOOL CScriptSync::Load(LPCTSTR pszScript)
{
	DeleteAllItems();

	ScriptFileType type = CheckFileType(pszScript);
	if (type == SCRIPT_ANSI)
		return LoadAnsi(pszScript);
	else if (type == SCRIPT_UNKNOWN) {
		HANDLE hf = CreateFile(pszScript, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, 0, NULL);
		if (hf != INVALID_HANDLE_VALUE) {
			BYTE bom[2] = { 0xff, 0xfe };
			DWORD dw;
			WriteFile(hf, &bom, 2, &dw, NULL);
			CloseHandle(hf);
			m_strScript = pszScript;
			return TRUE;
		}
		return FALSE;
	}

	mapping_file mf;
	if (mf.open(pszScript)) {
		// UNICODEヘッダをスキップ
		mf.seek(2, FILE_BEGIN);

		CString str;
		while (mf.getstr(str)) {
			str.TrimRight(_T('\n'));
			// 相対パス→絶対パス変換
			int len = str.GetLength() + MAX_PATH;
			fopExpandSyncPath(str.GetBuffer(len), len);
			str.ReleaseBuffer();

			if (_pathfileexists(str))
				AddFile(str);
		}
		mf.close();
		m_strScript = pszScript;
		return TRUE;
	}
	return FALSE;
}

BOOL CScriptSync::Save()
{
	if (m_strScript.IsEmpty())
		return FALSE;

	HANDLE hf = CreateFile(m_strScript, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, 0, NULL);
	if (hf != INVALID_HANDLE_VALUE) {
		// UNICODEヘッダ
		BYTE bom[2] = { 0xff, 0xfe };
		DWORD dw;
		WriteFile(hf, &bom, 2, &dw, NULL);

		// 同期ファイル名
		CSyncInfo* info;
		TCHAR buf[MAX_PATH * 2 + 2];
		int cnt = GetSize();
		for (int i = 0; i < cnt; i++) {
			info = (*this)[i];
			if (info) {
				lstrcpy(buf, info->FileName());
				// 絶対パス→相対パス変換
				fopSubstSyncPath(buf, ARRAYSIZE(buf));

				lstrcat(buf, _T("\n"));
				WriteFile(hf, buf, lstrlen(buf) * sizeof(TCHAR), &dw, NULL);
			}
		}
		CloseHandle(hf);
		return TRUE;
	}

	DWORD code = GetLastError();
	CString str; str.Format(IDS_SSYNC_SAVEERROR, PathFindFileName(m_strScript), code);
	AtlMessageBox(::GetActiveWindow(), (LPCTSTR)str, IDS_CAPTION_SCRIPTSYNC, MB_STOP);
	return FALSE;
}

BOOL CScriptSync::DoSync(HWND hwnd, TiXmlDocument* pXmlDoc)
{
	int nRes = m_bBackup ? IDS_MULTISYNC_BACKUP : IDS_MULTISYNC_RESTORE;
	if (AppMessage(IDS_TITLE_MULTISYNC, nRes, NULL, MB_QOKCANCEL) != IDOK)
		return FALSE;

	CSyncInfo* info;
	CSyncData* psd;
	CSimpleArray<CSyncData*> arr;
	CSyncRelay sr;
	sr.Create(hwnd);
	sr.ShowWindow(SW_SHOWNORMAL);
	int cnt = GetSize();
	int i;
	for (i = 0; i < cnt; i++) {
		if (m_execmap[i]) {
			info = (*this)[i];
			if (info) {
				psd = new CSyncData;
				if (psd->Load(info->FileName())) {
					arr.Add(psd);
					sr.AddPat(psd->m_strTitle);
				}
				else
					delete psd;
			}
		}
	}

	if (arr.GetSize() < 1) {
		AppMessage(IDS_TITLE_MULTISYNC, IDS_MULTISYNC_NOPAT, NULL, MB_STOP);
		return FALSE;
	}

	cnt = 0;
	UINT nStat;
	for (i = 0; i < arr.GetSize(); i++) {
		sr.SetPatState(i, 1);
		nStat = 3;
		psd = arr[i];
		if (fopValidateDir(hwnd, psd->m_strSrcFolder) && fopValidateDir(hwnd, psd->m_strDstFolder)) {
			psd->m_nDirection = (m_bBackup) ? DIRSYNC_BACKUP : DIRSYNC_RESTORE;
			if (pXmlDoc) {
				TiXmlNode* pNode = pXmlDoc->InsertEndChild( TiXmlElement( "log" ) );
				psd->SetLogger(pNode->ToElement());
			}
			else
				psd->SetLogger(NULL);

			if (SyncFolder(&sr, *psd))
				nStat = 2;
			cnt++;
		}
		sr.SetPatState(i, nStat);

		arr[i] = NULL;
		delete psd;
	}
	return (cnt > 0) ? TRUE : FALSE;
}
