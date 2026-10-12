// syncdata.h : header file
//

#pragma once
#ifndef __SYNCDATA_H__
#define __SYNCDATA_H__

#include "gdiplus_sup.h"
#include "custom_bkgnd_draw.h"
#include "..\lscore\taskbarlist.h"

////////////////////////////////////////////////////////////////////
//

void GetSyncBaseFolder(LPTSTR pszPath, int cchMax);
void GetSyncLogFolder(LPTSTR pszPath, int cchMax);

////////////////////////////////////////////////////////////////////
//

#define REPTYPE_MKDIR			0x00000001
#define REPTYPE_RMDIR			0x00000002
#define	REPTYPE_OVERWRITE		0x00000003
#define	REPTYPE_NEW				0x00000004
#define REPTYPE_DELETE			0x00000005
#define REPTYPE_RENAME			0x00000006
#define REPTYPE_IGNORE			0x00000007
#define REPTYPE_MESSAGE			0x0000000a
#define REPTYPE_PREXFER			0x0000000b			// 前処理の段階
#define	REPTYPE_SRCPATH			0x10000000
#define	REPTYPE_DSTPATH			0x20000000
#define	REPTYPE_ERROR			0x80000000
#define	REPTYPE_MASK			0xF0000000

////////////////////////////////////////////////////////////////////
// CLASS CSyncData : 同期データクラス
//

#define SYNC_ONLYEXIST			0x00000001			// 相手先に存在するファイルのみを対象
//#define SYNC_VERIFY			0x00000002			// ベリファイする
//#define SYNC_VERIFY_HDD		0x00000004
//#define SYNC_VERIFY_REM		0x00000008
//#define SYNC_VERIFY_NET		0x00000010
#define SYNC_RECYCLE			0x00000020			// 削除ファイルはゴミ箱へ
#define SYNC_COPYSID			0x00000040			// ACLの権限もコピー
#define SYNC_NOCONFIRM			0x00000100			// フォルダ作成時に作成の問い合わせしない
#define SYNC_SILENT				0x00000200			// 進捗非表示で実行
#define SYNC_SAVELOG			0x00000400			// ログを保存する

enum {
	DIRSYNC_BACKUP,
	DIRSYNC_RESTORE,
	DIRSYNC_SYNC,
};

enum ScriptFileType {
	SCRIPT_ANSI,
	SCRIPT_UNICODE,
	SCRIPT_UNKNOWN
};


#define SID_MARKER5				1131
#define SID_MARKER6				1132
#define SID_SUB_MARKER01		2132
#define SID_SUB_MARKER02		2133

#pragma pack(1)
typedef struct tagSYNCEXINFOSTRUCT {
	DWORD ver;
	WORD useRegExp;
	WORD caseSense;
	GUID guid;
	DWORD reserved[2];
} SYNCEXTENDINFOSTRUCT;
#pragma pack()

////////////////////////////////////////////////////////////////////////
//

class CSyncData
{
// データメンバ
public:
	DWORD m_dwMarker;					// マジックナンバー
	CString m_strTitle;					// 同期タイトル
	CString m_strSrcFolder;				// 同期元フォルダ
	CString m_strDstFolder;				// 同期先フォルダ
	CString m_strFilter;				// 拡張子フィルタ
	CString m_strExcludeFolder;			// 対象外フォルダ
	CString m_strPatName;				// 同期パタンファイル名
	GUID m_guid;
	SYSTEMTIME m_st;
	DWORD m_nDirection;					// 動作モード
	DWORD m_dummy1;
	DWORD m_dwFlags;
	int m_iDosCompare;
	bool m_bMirror;						// 存在しないフォルダ・ファイルは削除する（ミラーリング）
	bool m_bSubdir;						// サブフォルダも同期
	bool m_bSaveOld;					// 古いファイルを残す
	bool m_bHasFilter;					// フィルタ制御
	bool m_bReport;						// 結果レポート
	bool m_bExclude;					// 対象外制御
	bool m_bUseRegExp;					// フィルタを正規表現として扱う
	bool m_bCaseSense;					// 大文字小文字の区別

	TiXmlElement* m_pXmlLog;
	CDiskSpace m_diskTarget;

public:
	CSyncData()
	{
		Init();
	}
	CSyncData(const CSyncData& a)
	{
		Init();
		CopyFrom(a);
	}
	~CSyncData()
	{
	}

// オペレーション
public:
	void Init()
	{
		m_dwMarker = SID_MARKER6;
		m_strTitle.Empty();
		m_strSrcFolder.Empty();
		m_strDstFolder.Empty();
		m_strFilter.Empty();
		m_strExcludeFolder.Empty();
		m_guid = GUID_NULL;
		m_nDirection = 0;
		m_bMirror = false;
		m_bSubdir = true;
		m_bSaveOld = false;
		m_bHasFilter = false;
		m_bReport = true;
		m_bExclude = false;
		m_dummy1 = 0;
		m_dwFlags = 0;
		memset(&m_st, 0, sizeof(m_st));
		m_bUseRegExp = false;
		m_bCaseSense = false;

		m_iDosCompare = 0;
		m_pXmlLog = NULL;
	}

	void CopyFrom(const CSyncData& a)
	{
		if (&a != this) {
			m_strTitle = a.m_strTitle.GetString();
			m_strSrcFolder = a.m_strSrcFolder.GetString();
			m_strDstFolder = a.m_strDstFolder.GetString();
			m_strFilter = a.m_strFilter.GetString();
			m_strExcludeFolder = a.m_strExcludeFolder.GetString();
			m_strPatName = a.m_strPatName.GetString();
			m_guid = a.m_guid;
			m_nDirection = a.m_nDirection;
			m_dummy1 = a.m_dummy1;
			m_dwFlags = a.m_dwFlags;
			m_st = a.m_st;
			m_bMirror = a.m_bMirror;
			m_bSubdir = a.m_bSubdir;
			m_bSaveOld = a.m_bSaveOld;
			m_bHasFilter = a.m_bHasFilter;
			m_bExclude = a.m_bExclude;
			m_bReport = a.m_bReport;
			m_bUseRegExp = a.m_bUseRegExp;
			m_bCaseSense = a.m_bCaseSense;
			m_iDosCompare = 0;
			m_pXmlLog = NULL;
		}
	}

	CSyncData& operator=(const CSyncData& a)
	{
		CopyFrom(a);
		return *this;
	}

	bool operator==(const CSyncData& a) const
	{
		return (!m_strTitle.Compare(a.m_strTitle) &&
				!m_strSrcFolder.Compare(a.m_strSrcFolder) &&
				!m_strDstFolder.Compare(a.m_strDstFolder) &&
				!m_strFilter.Compare(a.m_strFilter) &&
				!m_strExcludeFolder.Compare(a.m_strExcludeFolder) &&
				!m_strPatName.CompareNoCase(a.m_strPatName) &&
				(m_guid == a.m_guid) &&
				(m_dwFlags == a.m_dwFlags) &&
				(m_bMirror == a.m_bMirror) &&
				(m_bSubdir == a.m_bSubdir) &&
				(m_bSaveOld == a.m_bSaveOld) &&
				(m_bHasFilter == a.m_bHasFilter) &&
				(m_bExclude == a.m_bExclude) &&
				(m_bReport == a.m_bReport) &&
				(m_bUseRegExp == a.m_bUseRegExp) &&
				(m_bCaseSense == a.m_bCaseSense));
	}
	void CreateReport();
	void OpenReport(TiXmlDocument* pXmlDoc);
	void CloseReport();

	void Report(UINT nReportType, LPCTSTR pszText, DWORD nErrorCode = NOERROR)
	{
		if (m_bReport && m_pXmlLog) {
			// 同期の向きによりパスを省略する長さが変わる
			int lenSrc = (m_nDirection != DIRSYNC_BACKUP) ? m_strDstFolder.GetLength() : m_strSrcFolder.GetLength();
			int lenDst = (m_nDirection != DIRSYNC_BACKUP) ? m_strSrcFolder.GetLength() : m_strDstFolder.GetLength();
			CString str = pszText;
			if (nReportType & REPTYPE_SRCPATH) {
				str.Delete(0, lenSrc);
				if (str[0] == '\\') str.Delete(0, 1);
			}
			else if (nReportType & REPTYPE_DSTPATH) {
				str.Delete(0, lenDst);
				if (str[0] == '\\') str.Delete(0, 1);
			}

			TiXmlNode* pNode = m_pXmlLog->InsertEndChild( TiXmlElement( "item" ) );
			if (pNode) {
				TiXmlElement* pElement = pNode->ToElement();
				if (pElement) {
					char utf[MAX_PATH * 2];
					wsz2utf(str, utf, ARRAYSIZE(utf));
					pElement->SetAttribute( "name", utf );
					pElement->SetAttribute( "type", (int)nReportType );
					pElement->SetAttribute( "code", (int)nErrorCode );
				}
			}
		}
	}

	inline BOOL isGuidNull() { return (m_guid == GUID_NULL); }

	void AssignGuid(bool bForceAssign = false)
	{
		if (m_guid == GUID_NULL || bForceAssign) {
			if (CoCreateGuid(&m_guid) == S_OK)
				m_guid = GUID_NULL;
		}
	}

	BOOL Serialize(CFile& file, BOOL bRead)
	{
		SYNCEXTENDINFOSTRUCT exinfo;
		memset(&exinfo, 0, sizeof(exinfo));
		if (bRead) {
			if (file.GetReminder() > 0) {			// 新規パタン以外はサイズ０
				if (!file.Read_DWORD(m_dwMarker) || (m_dwMarker != SID_MARKER5 && m_dwMarker != SID_MARKER6))
					return FALSE;

				StringSerialize(m_strTitle, file, TRUE);
				StringSerialize(m_strSrcFolder, file, TRUE);
				StringSerialize(m_strDstFolder, file, TRUE);
				StringSerialize(m_strFilter, file, TRUE);
				StringSerialize(m_strExcludeFolder, file, TRUE);

				if (!file.Read_DWORD(m_nDirection)) return FALSE;
				if (!file.Read_bool(m_bMirror)) return FALSE;
				if (!file.Read_bool(m_bSubdir)) return FALSE;
				if (!file.Read_bool(m_bSaveOld)) return FALSE;
				if (!file.Read_bool(m_bHasFilter)) return FALSE;
				if (!file.Read_bool(m_bReport)) return FALSE;
				if (!file.Read_bool(m_bExclude)) return FALSE;
				if (!m_bHasFilter) m_bExclude = false;
				if (!file.Read_DWORD(m_dummy1)) return FALSE;
				if (!file.Read_DWORD(m_dwFlags)) return FALSE;
				if (!file.Read(&m_st, sizeof(m_st))) return FALSE;
				if (!file.Read(&exinfo, sizeof(exinfo))) return FALSE;
				if (exinfo.ver == SID_SUB_MARKER01 || exinfo.ver == SID_SUB_MARKER02) {
					m_bUseRegExp = (exinfo.useRegExp != 0);
					m_bCaseSense = (exinfo.caseSense != 0);
					m_guid = exinfo.guid;
				}
				else {
					m_bUseRegExp = false;
					m_bCaseSense = false;
				}
			}
		}
		else {
			if (isGuidNull()) AssignGuid();

			m_dwMarker = SID_MARKER6;
			if (!file.Write_DWORD(m_dwMarker)) return FALSE;

			StringSerialize(m_strTitle, file, FALSE);
			StringSerialize(m_strSrcFolder, file, FALSE);
			StringSerialize(m_strDstFolder, file, FALSE);
			StringSerialize(m_strFilter, file, FALSE);
			StringSerialize(m_strExcludeFolder, file, FALSE);
			if (!file.Write_DWORD(m_nDirection)) return FALSE;
			if (!file.Write_bool(m_bMirror)) return FALSE;
			if (!file.Write_bool(m_bSubdir)) return FALSE;
			if (!file.Write_bool(m_bSaveOld)) return FALSE;
			if (!file.Write_bool(m_bHasFilter)) return FALSE;
			if (!file.Write_bool(m_bReport)) return FALSE;
			if (!m_bHasFilter) m_bExclude = false;
			if (!file.Write_bool(m_bExclude)) return FALSE;
			if (!file.Write_DWORD(m_dummy1)) return FALSE;
			if (!file.Write_DWORD(m_dwFlags)) return FALSE;
			if (!file.Write(&m_st, sizeof(m_st))) return FALSE;
			exinfo.ver = SID_SUB_MARKER02;
			exinfo.useRegExp = m_bUseRegExp ? 1 : 0;
			exinfo.caseSense = m_bCaseSense ? 1 : 0;
			exinfo.guid = m_guid;
			if (!file.Write(&exinfo, sizeof(exinfo))) return FALSE;
		}
		return TRUE;
	}

	BOOL Load(LPCTSTR pszPatSrc = NULL, LPCTSTR pszPatDst = NULL)
	{
		if (!pszPatSrc || *pszPatSrc == 0) {
			Init();
			m_strTitle = _T("(新規)の同期");
			m_strPatName.Format(_T("%s.fsd"), m_strTitle.GetString());
			return TRUE;
		}

		if (_pathisdirectory(pszPatSrc)) {
			Init();

			SHFILEINFO sfi;
			SHGetFileInfo(pszPatSrc, 0, &sfi, sizeof(sfi), SHGFI_DISPLAYNAME);
			m_strTitle.Format(_T("%sの同期"), sfi.szDisplayName);
			m_strPatName.Format(_T("%s.fsd"), m_strTitle.GetString());
			m_strSrcFolder = pszPatSrc;
			m_strDstFolder = pszPatDst;
			return TRUE;
		}

		BOOL bOK = FALSE;
		CFile file;
		if (file.Open(pszPatSrc)) {
			bOK = Serialize(file, TRUE);
			if (bOK)
				m_strPatName = pszPatSrc;
			file.Close();
		}
		return bOK;
	}

	BOOL Save(LPCTSTR pszPatName)
	{
		CFile file;

		if (file.Create(pszPatName)) {
			Serialize(file, FALSE);
			file.Close();
			m_strPatName = pszPatName;
			return TRUE;
		}
		return FALSE;
	}

	BOOL isFolderExclude(LPCTSTR pszFolder)
	{
		// 同期対象外かどうかをチェック
		if (!pszFolder || *pszFolder == '\0') return TRUE;
		if (m_strExcludeFolder.IsEmpty()) return FALSE;

		// rootをスキップして、パスリストを作成
		// 例）c:\windows\system\command なら
		// [ "windows\system\command", "system\command", "command" ] の配列を作成
		LPTSTR tp = PathSkipRoot(pszFolder);
		CSimpleArray<CString> sarr;
		do {
			sarr.Add(tp);
			tp = StrChr(tp, _T('\\'));
			if (tp) {
				tp++;
			}
		} while(tp);
		int sn = sarr.GetSize();

		// パスが対象外かチェック
		// \\付き複合パスもチェック
		CSimpleArray<CString> darr;
		int dn = SeparateString(m_strExcludeFolder, _T(';'), darr);
		if (dn > 0) {
			int i, j;
			for (i = 0; i < dn; i++) {
				for (j = sn - 1; j >= 0; j--) {
					if (!StrCmpI(sarr[j], darr[i])) {
						return TRUE;
					}
				}
			}
			return FALSE;
		}
		return TRUE;

/*
		LPTSTR src = PathFindFileName(pszFolder);
		if (src) {
			BOOL bOK = FALSE;
			LPTSTR flt = StrDup(m_strExcludeFolder);
			if (flt) {
				LPTSTR dst = flt;
				while (dst && *dst) {
					LPTSTR p = StrChr(dst, _T(';'));
					if (p)
						*p++ = 0;
					if (!StrCmpI(src, dst)) {
						bOK = TRUE;
						break;
					}
					dst = p;
				}
				LocalFree(flt);
			}
			return bOK;
		}
		return TRUE;
*/
	}

	void SetLogger(TiXmlElement* pXmlLog)
	{
		m_pXmlLog = pXmlLog;
		if (m_pXmlLog) {
			// タイトル
			char utf[MAX_PATH * 2];
			wsz2utf(m_strTitle, utf, ARRAYSIZE(utf));
			m_pXmlLog->SetAttribute( "title", utf );

			if (m_nDirection == DIRSYNC_BACKUP || m_nDirection == DIRSYNC_SYNC) {
				// Source
				wsz2utf(m_strSrcFolder, utf, ARRAYSIZE(utf));
				m_pXmlLog->SetAttribute( "source", utf );
				// Destination
				wsz2utf(m_strDstFolder, utf, ARRAYSIZE(utf));
				m_pXmlLog->SetAttribute( "destination", utf );
			}
			else {
				// Source
				wsz2utf(m_strDstFolder, utf, ARRAYSIZE(utf));
				m_pXmlLog->SetAttribute( "source", utf );
				// Destination
				wsz2utf(m_strSrcFolder, utf, ARRAYSIZE(utf));
				m_pXmlLog->SetAttribute( "destination", utf );
			}
		}
	}

	void SetLogDescription(LPCTSTR pDesc)
	{
		if (m_pXmlLog) {
			char utf[MAX_PATH * 2];
			wsz2utf(pDesc, utf, ARRAYSIZE(utf));
			m_pXmlLog->SetAttribute( "description", utf );
		}
	}

	inline bool isRecycle() { return ((m_dwFlags & SYNC_RECYCLE) != 0); }
	inline bool isOnlyExist() { return ((m_dwFlags & SYNC_ONLYEXIST) != 0); }
	inline bool isNoConfirm() { return ((m_dwFlags & SYNC_NOCONFIRM) != 0); }
	inline bool isSilent() { return ((m_dwFlags & SYNC_SILENT) != 0); }
	inline bool isLogSave() { return ((m_dwFlags & SYNC_SAVELOG) != 0); }
	inline bool isCopyAccessRights() { return ((m_dwFlags & SYNC_COPYSID) != 0); }
	inline bool isCaseSensitive() { return m_bCaseSense; }
	inline bool hasFilter() { return (m_bHasFilter && !m_strFilter.IsEmpty()); }
	inline bool UnmatchFilter() { return (hasFilter() && m_bExclude); }
	inline bool UseRegExp() { return (hasFilter() && m_bUseRegExp); }

	BOOL ExecConfirm();
};



////////////////////////////////////////////////////////////////////
// 
//

BOOL __fastcall _pathfileexists(LPCTSTR pszName);

class CSyncInfo
{
public:
	CString m_strSyncFile;
	CString m_strTitle;
	CString m_strSrc;
	CString m_strDst;

public:
	CSyncInfo()
	{
		m_strSyncFile.Empty();
		m_strSrc.Empty();
		m_strDst.Empty();
	}
	~CSyncInfo() { }

	BOOL SetInfo(CSyncData* psd)
	{
		if (psd) {
			m_strSyncFile = psd->m_strPatName;
			m_strTitle = psd->m_strTitle;
			m_strSrc = psd->m_strSrcFolder;
			m_strDst = psd->m_strDstFolder;
			return TRUE;
		}
		return FALSE;
	}

	BOOL SetInfo(LPCTSTR pszSyncFile)
	{
		if (_pathfileexists(pszSyncFile)) {
			CSyncData sd;
			if (sd.Load(pszSyncFile)) {
				return SetInfo(&sd);
			}
		}
		return FALSE;
	}

	__inline LPCTSTR FileName() { return m_strSyncFile; }
	__inline LPCTSTR Title() { return m_strTitle; }
	__inline LPCTSTR Source() { return m_strSrc; }
	__inline LPCTSTR Target() { return m_strDst; }
};

////////////////////////////////////////////////////////////////////
// class CScriptSync
//

class CScriptSync : public CSimpleArray<CSyncInfo*>
{
public:
	CString m_strScript;
	CSimpleArray<int> m_execmap;
	BOOL m_bBackup;

public:
	CScriptSync()
	{
		m_strScript.Empty();
	}
	CScriptSync(LPCTSTR pszScript)
	{
		m_strScript.Empty();
		Load(pszScript);
	}

	~CScriptSync()
	{
		DeleteAllItems();
	}

	void DeleteAllItems()
	{
		CSyncInfo* info;
		int cnt = GetSize();
		for (int i = 0; i < cnt; i++) {
			info = (*this)[i];
			if (info)
				delete info;
		}
		RemoveAll();
	}

	ScriptFileType CheckFileType(LPCTSTR pszScript)
	{
		ScriptFileType type = SCRIPT_UNKNOWN;
		HANDLE hf = CreateFile(pszScript, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, 0, NULL);
		if (hf != INVALID_HANDLE_VALUE) {
			type = SCRIPT_ANSI;
			BYTE bom[2];
			DWORD dw;
			if (ReadFile(hf, &bom, 2, &dw, NULL) && dw == 2) {
				// UNICODEチェック
				if (bom[0] == 0xff && bom[1] == 0xfe)
					type = SCRIPT_UNICODE;
			}
			CloseHandle(hf);
		}
		return type;
	}

	BOOL LoadAnsi(LPCTSTR pszScript);
	BOOL Load(LPCTSTR pszScript);
	BOOL Save();

	BOOL SaveAs(LPCTSTR pszAsFile)
	{
		m_strScript = pszAsFile;
		return Save();
	}

	void AddFile(LPCTSTR pszSyncFile)
	{
		CSyncInfo* info = new CSyncInfo();
		if (info) {
			if (info->SetInfo(pszSyncFile))
				AddItem(info);
			else
				delete info;
		}
	}

	void AddItem(CSyncInfo* info)
	{
		if (info) {
			Add(info);
			m_execmap.Add(TRUE);
		}
	}

	BOOL Delete(int index)
	{
		int cnt = GetSize();
		if (index >= 0 && index < cnt) {
			CSyncInfo* info = (*this)[index];
			RemoveAt(index);
			m_execmap.RemoveAt(index);
			if (info)
				delete info;
			return TRUE;
		}
		return FALSE;
	}

	BOOL Exchange(int si, int di)
	{
		int cnt = GetSize();
		if (si < 0 || si >= cnt || di < 0 || di >= cnt)
			return FALSE;

		CSyncInfo* src = (*this)[si];
		CSyncInfo* dst = (*this)[di];
		int sm = m_execmap[si];
		int dm = m_execmap[di];

		SetAtIndex(si, dst);
		m_execmap.SetAtIndex(si, dm);
		SetAtIndex(di, src);
		m_execmap.SetAtIndex(di, sm);

		return TRUE;
	}

	BOOL DoSync(HWND hwnd, TiXmlDocument* pXmlDoc);
};


/////////////////////////////////////////////////////////////////////////////
// CSyncRelay dialog


class CSyncRelay
		: public CDialogImpl<CSyncRelay>
		, public CAnimateDialogIcon<CSyncRelay>
		, public CCustomBkgndDrawImpl<CSyncRelay>
		, public CTaskbarListImpl<CSyncRelay>
{
private:
	gdiplus_using m_gu;
	CListViewCtrl m_list;
	CAnimateCtrl m_anim;
	DWORDLONG m_dlTotal;
	DWORDLONG m_dlDone;
	BOOL m_bCanceled;

public:
	enum { IDD = IDD_DOSYNC };

	CSyncRelay()
		: m_dlTotal(100)
		, m_dlDone(0)
		, m_bCanceled(FALSE)
	{
	}

	~CSyncRelay()
	{
		if (IsWindow()) DestroyWindow();
	}

	virtual void TaskbarButtonCreated(ITaskbarList3* pTaskbarList);

BEGIN_MSG_MAP(CSyncRelay)
	CHAIN_MSG_MAP(CCustomBkgndDrawImpl<CSyncRelay>)
	CHAIN_MSG_MAP(CAnimateDialogIcon<CSyncRelay>)
	CHAIN_MSG_MAP(CTaskbarListImpl<CSyncRelay>)
	ON_WM_INITDIALOG()
	ON_WM_DESTROY()
	ON_BN_CLICKED(IDCANCEL, OnCancel)
END_MSG_MAP()

	BOOL OnInitDialog(LPARAM lParam);
	void OnDestroy();

	void OnCancel() { m_bCanceled = TRUE; }

	void InitSearch()
	{
		if (IsWindow()) {
			LedClock_Reset(GetDlgItem(IDC_CLOCK));

			m_dlDone = 0;
			m_dlTotal = 0;

			SendDlgItemMessage(IDC_METER, PBM_SETRANGE, 0, MAKELPARAM(0, 100));
			SendDlgItemMessage(IDC_METER, PBM_SETPOS, 0, 0);

			m_anim.Close();
			m_anim.Open(IDA_SHELL_150);

			TaskbarList_SetProgressState(TBPF_INDETERMINATE);
		}
	}

	void InitSync( DWORDLONG dlTotal)
	{
		if (IsWindow()) {
			m_dlTotal = dlTotal;

			m_anim.Close();
			m_anim.Open(IDA_SHELL_161);

			TaskbarList_SetProgressState(TBPF_NORMAL);
			TaskbarList_SetProgressValue(0, m_dlTotal);
		}
	}

	void SetDirectory(LPCTSTR lpSrc, LPCTSTR lpDst)
	{
		if (IsWindow()) {
			PathSetDlgItemPath(m_hWnd, IDC_SRCNAME, lpSrc);
			PathSetDlgItemPath(m_hWnd, IDC_DSTNAME, lpDst);
		}
	}

	void SetLine(LPCTSTR pszText)
	{
		if (IsWindow()) {
			if (IS_INTRESOURCE(pszText)) {
				CString str; str.LoadString((UINT)(UINT_PTR)pszText);
				SetDlgItemText(IDC_TEXT, str);
			}
			else
				SetDlgItemText(IDC_TEXT, pszText);
			ZapMessage();
		}
	}

	void SetComplete(DWORDLONG dlDone)
	{
		if (IsWindow()) {
			m_dlDone += dlDone;
			int par = (int)((m_dlDone * 100) / m_dlTotal);
			SendDlgItemMessage(IDC_METER, PBM_SETPOS, par, 0);
			TaskbarList_SetProgressValue(m_dlDone, m_dlTotal);
			ZapMessage();
		}
	}

	void ZapMessage()
	{
		MSG	msg;
		while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) {
			if (!IsDialogMessage(&msg)) {
				TranslateMessage(&msg);
				DispatchMessage(&msg);
			}
		}
	}

	void AddPat(LPCTSTR pszText)
	{
		if (IsWindow())
			m_list.InsertItem(m_list.GetItemCount(), pszText, 0);
	}

	void SetPatState(int index, int state)
	{
		if (IsWindow()) {
			m_list.EnsureVisible(index, FALSE);

			LVITEM lvi;
			memset(&lvi, 0, sizeof(lvi));
			lvi.mask = LVIF_IMAGE;
			lvi.iItem = index;
			lvi.iImage = state;
			m_list.SetItem(&lvi);

			LPCTSTR psz;
			switch (state) {
			case 1:  psz = _T("実行中..."); break;
			case 2:  psz = _T("完了"); break;
			case 3:  psz = _T("中止"); break;
			default: psz = NULL; break;
			}
			m_list.SetItemText(index, 1, psz);
		}
	}

	BOOL isCanceled()
	{
		ZapMessage();
		return m_bCanceled;
	}

	void ResetCancel()
	{
		m_bCanceled = FALSE;
	}

};

/////////////////////////////////////////////////////////////////////////////
// CSyncContinue dialog

class CSyncContinue : public CDialogImpl<CSyncContinue>
{
private:
	int m_answer;
public:
	enum { IDD = IDD_SYNCCONTINUE };

	CSyncContinue() : m_answer(0) { }

BEGIN_MSG_MAP(CSyncContinue)
	ON_WM_INITDIALOG()
	ON_BN_CLICKED(IDOK, OnOK)
END_MSG_MAP()

	BOOL OnInitDialog(LPARAM lParam)
	{
		CenterWindow();

		CStatic icon = GetDlgItem(IDC_INFO);
		icon.SetIcon(LoadIcon(NULL, IDI_QUESTION));
		CheckDlgButton(IDC_IGNORE, TRUE);
		return TRUE;
	}
	void OnOK()
	{
		if      (IsDlgButtonChecked(IDC_STOPAFTER)) m_answer = 1;
		else if (IsDlgButtonChecked(IDC_STOP))      m_answer = 2;
		else                                        m_answer = 0;
		EndDialog(IDOK);
	}

	int Result() { return m_answer; }
};


#endif		// __SYNCDATA_H__
