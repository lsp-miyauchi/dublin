// execdlg.cpp : implementation file
//

#include "stdafx.h"
#include "fv7.h"
#include "execdlg.h"

#define MAXEXECHISTORY		16

static const TCHAR extExec[] = _T(".exe");

BOOL CExecDlg::OnInitDialog(LPARAM lParam)
{
	DlgResize_Init();
	CenterWindow();

	m_list = GetDlgItem(IDC_LIST);
	HIMAGELIST himL, himS;
	Shell_GetImageLists(&himL, &himS);
	m_list.SetImageList(himL, LVSIL_NORMAL);
	m_list.SetImageList(himS, LVSIL_SMALL);

	m_spc = GetDlgItem(IDC_COMBO);
	CheckDlgButton(IDC_NORMAL, TRUE);

	GetDlgItem(IDC_ASADMIN).EnableWindow(TRUE);

	// 実行履歴
	BOOL bReturnCode = TRUE;
	CSimpleArray<CString> arr;
	RegLoadHistory(arr, IDS_SEC_RUNAS, IDS_KEY_HISTORY, MAXEXECHISTORY);

	int i, cnt = arr.GetSize();
	for (i = 0; i < cnt; i++)
		m_spc.AddString(arr[i]);

	int len = lstrlen(m_pei->m_strCommand);
	if (len > 0) {
		m_spc.SetFocus();
		m_spc.SetWindowText(m_pei->m_strCommand);
		long lsel = (m_pei->m_bExecType ? MAKELONG(len, len) : 0L);
		m_spc.SetEditSel(0, lsel);
		bReturnCode = FALSE;
	}

	// 既定のプログラムをリストアップ
	ListupStandardPrograms();

	return bReturnCode;
}

void CExecDlg::ListupStandardPrograms()
{
	// 作業用マップ配列
	CSimpleMap<CString, CString, CExecNameEqualHelper> map;

	// ----------------------------------
	// レジストリ内の実行プログラム
	// ----------------------------------
	TCHAR buf[MAX_PATH * 2], exe[MAX_PATH * 2], env[MAX_PATH * 2];
	CRegKey reg, sec;

	// Applications エントリ
	DWORD index, size;
	if (reg.Open(HKEY_CLASSES_ROOT, _T("Applications"), KEY_READ) == ERROR_SUCCESS) {
		index = 0;
		while (1) {
			size = ARRAYSIZE(buf);
			if (reg.EnumKey(index, buf, &size) != ERROR_SUCCESS)
				break;

			bool canopen = true;
			lstrcpy(exe, buf);
			PathCombine(buf, exe, _T("shell\\open\\command"));
			if (sec.Open(reg, buf, KEY_READ) != ERROR_SUCCESS) {
				PathCombine(buf, exe, _T("shell\\edit\\command"));
				if (sec.Open(reg, buf, KEY_READ) != ERROR_SUCCESS) {
					PathCombine(buf, exe, _T("shell\\play\\command"));
					if (sec.Open(reg, buf, KEY_READ) != ERROR_SUCCESS) {
						canopen = false;
					}
				}
			}

			if (canopen) {
				ULONG cnt = ARRAYSIZE(buf);
				if (sec.QueryStringValue(NULL, buf, &cnt) == ERROR_SUCCESS && cnt > 1) {
					if (!ExpandEnvironmentStrings(buf, env, ARRAYSIZE(env)))
						lstrcpy(env, buf);

					TCHAR* p;
					if (*env == _T('"')) {
						// "..." 形式の場合
						p = StrChr(env + 1, _T('"'));
						if (p) *(p + 1) = 0;
						PathUnquoteSpaces(env);
					}
					else {
						// コマンド引数を分離
						p = StrChr(env, _T(' '));
						if (p) *p = 0;
					}

					// 拡張子が.exeのものだけ
					if (!StrCmpI(extExec, PathFindExtension(exe)))
						map.Add(exe, env);
				}
				sec.Close();
			}
			index++;
		}
		reg.Close();
	}

	// App Paths エントリ
	if (reg.Open(HKEY_LOCAL_MACHINE, _T("SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\App Paths"), KEY_READ) == ERROR_SUCCESS) {
		index = 0;
		while (1) {
			size = ARRAYSIZE(buf);
			if (reg.EnumKey(index, buf, &size) != ERROR_SUCCESS)
				break;

			if (sec.Open(reg, buf, KEY_READ) == ERROR_SUCCESS) {
				if (map.FindKey(buf) < 0) {
					// 未登録ならば
					ULONG cnt = ARRAYSIZE(exe);
					if (sec.QueryStringValue(NULL, exe, &cnt) == ERROR_SUCCESS && cnt > 1) {
						// パス変数を拡張
						if (!ExpandEnvironmentStrings(exe, env, ARRAYSIZE(env)))
							lstrcpy(env, exe);
						PathUnquoteSpaces(env);

						// 拡張子が.exeのものだけ
						if (!StrCmpI(extExec, PathFindExtension(buf)))
							map.Add(buf, env);
					}
				}
				sec.Close();
			}
			index++;
		}
		reg.Close();
	}
	CFileFinder ff;

#if 0
	// ----------------------------------
	// パス配下のプログラム
	// ----------------------------------
	// System32フォルダ
	GetSystemDirectory(buf, ARRAYSIZE(buf));
	lstrcpy(exe, buf);
	PathAppend(exe, _T("*.exe"));
	if (ff.BeginFind(exe)) {
		do {
			if (map.FindKey(ff.cFileName) < 0) {
				// 未登録ならば
				PathCombine(exe, buf, ff.cFileName);
				map.Add(ff.cFileName, exe);
			}
		} while (ff.NextFind());
		ff.EndFind();
	}

	// Windowsフォルダ
	GetWindowsDirectory(buf, ARRAYSIZE(buf));
	lstrcpy(exe, buf);
	PathAppend(exe, _T("*.exe"));
	if (ff.BeginFind(exe)) {
		do {
			if (map.FindKey(ff.cFileName) < 0) {
				// 未登録ならば
				PathCombine(exe, buf, ff.cFileName);
				map.Add(ff.cFileName, exe);
			}
		} while (ff.NextFind());
		ff.EndFind();
	}
#endif

	// 今までに利用してきた既定のプログラムを読む
	CSimpleArray<CString> arr;
	RegLoadHistory(arr, IDS_SEC_RUNAS, IDS_KEY_FAVORITEAPP, MAXEXECHISTORY);

	// 配列先頭が優先度HIGH
	int cnt = map.GetSize();
	int numapp = arr.GetSize();
	int i;
	for (i = 0; i < numapp; i++) {
		int n = map.FindKey(arr[i]);
		if (n >= 0) {
			// 正規の配列に登録
			m_mapPrograms.Add(map.GetKeyAt(n), map.GetValueAt(n));
			// 作業用マップから削除
			map.RemoveAt(n);
		}
	}

	// 残ったデータを処理
	cnt = map.GetSize();
	for (i = 0; i < cnt; i++)
		m_mapPrograms.Add(map.GetKeyAt(i), map.GetValueAt(i));

	// リストに登録
	cnt = m_mapPrograms.GetSize();
	for (i = 0; i < cnt; i++)
		m_list.InsertItem(i, m_mapPrograms.GetKeyAt(i), I_IMAGECALLBACK);
}


void CExecDlg::OnNotifyListViewGetDispInfo(NMHDR* pNmhdr, LRESULT* pResult)
{
	*pResult = 0;
	LV_DISPINFO* pdi = (LV_DISPINFO*)pNmhdr;
	if (pdi) {
		if (pdi && pdi->item.mask & LVIF_IMAGE) {
			CString exe;
			if (m_list.GetItemText(pdi->item.iItem, 0, exe) > 0) {
				int index = m_mapPrograms.FindKey(exe);
				if (index >= 0) {
					SHFILEINFO sfi;
					SHGetFileInfo(m_mapPrograms.GetValueAt(index), 0, &sfi, sizeof(sfi), SHGFI_SYSICONINDEX);
					pdi->item.iImage = sfi.iIcon;
					pdi->item.mask |= LVIF_DI_SETITEM;
				}
			}
		}
	}
}

void CExecDlg::OnNotifyListViewItemChanged(NMHDR* pNmhdr, LRESULT* pResult)
{
	NMLISTVIEW* plv = (NMLISTVIEW*)pNmhdr;
	CString exe;
	if (m_list.GetItemText(plv->iItem, 0, exe) > 0) {
		CString str;
		if (m_spc.GetWindowText(str) < 1)
			m_spc.SetWindowText(exe);
		else {
			LRESULT lr = m_spc.SendMessage(EM_GETSEL, 0, 0);
			int len = (int)LOWORD(lr);
			if (len > 0) {
				CString mid = str.Mid(len);
				mid.TrimLeft();
				str.Format(_T("%s %s"), (LPCTSTR)exe, (LPCTSTR)mid);
			}
			else {
				str.Insert(0, _T(' '));
				str.Insert(0, exe);
			}
			len = exe.GetLength();

			m_bIgnoreUpdateList = true;
			m_spc.SetWindowText(str);
			m_spc.SendMessage(EM_SETSEL, len, len);
		}
	}
}

void CExecDlg::OnNotifyBrowse(NMHDR* pNmhdr, LRESULT* pResult)
{
	NMSEGETTEXT* psgt = (NMSEGETTEXT*)pNmhdr;

	CMemTask path;
	SHGetKnownFolderPath(FOLDERID_ProgramFiles, 0, NULL, (PWSTR*)&path);
	CString str;
	if (FileBrowse(TRUE, m_hWnd, IDS_FILTER_ALLFILES, str, path)) {
		StrCpyN(psgt->pszText, str, psgt->cchTextMax);
		*pResult = TRUE;
	}
}

void CExecDlg::OnNotifyEditChange(NMHDR* pNmhdr, LRESULT* pResult)
{
	if (m_bIgnoreUpdateList) {
		m_bIgnoreUpdateList = false;
		return;
	}

	m_list.SetRedraw(FALSE);
	m_list.DeleteAllItems();

	int cnt = m_mapPrograms.GetSize();
	if (cnt > 0) {
		// 入力された文字列に一致するプレフィクスを持つプログラムをリストアップ
		CString str;
		int i;
		if (m_spc.GetWindowText(str) > 0) {
			LRESULT lr = m_spc.SendMessage(EM_GETSEL, 0, 0);
			int len = (int)LOWORD(lr);
			if (len > 0) {
				CString exe = str.Left(len);
				for (i = 0; i < cnt; i++) {
					str = m_mapPrograms.GetKeyAt(i);
					if (!StrCmpNI(exe, str, len))
						m_list.InsertItem(m_list.GetItemCount(), str, I_IMAGECALLBACK);
				}
			}
		}

		if (m_list.GetItemCount() < 1) {
			// 一致するものがない場合はすべてのエントリを再度リストアップ
			for (i = 0; i < cnt; i++)
				m_list.InsertItem(m_list.GetItemCount(), m_mapPrograms.GetKeyAt(i), I_IMAGECALLBACK);
		}
	}
	m_list.SetRedraw(TRUE);
	m_list.Invalidate();
}

void CExecDlg::OnOK()
{
	CString str;
	if (m_spc.GetWindowText(str) < 1) {
		MessageBeep(0);
		m_spc.SetFocus();
		return;
	}

	// wait動作のチェック
	LPTSTR p = (LPTSTR)(LPCTSTR)str;
	if (*p == '!') {
		m_pei->m_bWait = TRUE;
		p++;
	}
	else
		m_pei->m_bWait = FALSE;

	// 管理者として実行
	m_pei->m_bAsAdmin = (IsDlgButtonChecked(IDC_ASADMIN) != FALSE);

	// マップリストにあるプログラムを選択した場合
	m_pei->m_strCommand = p;
	TCHAR buf[MAX_PATH * 2];
	lstrcpy(buf, p);
	p = StrChr(buf, _T(' '));
	if (p) *p = 0;
	int index = m_mapPrograms.FindKey(buf);
	if (index >= 0) {
		// プログラム名をフルパスに置き換える
		str = m_mapPrograms.GetValueAt(index);
		if (StrChr((LPCTSTR)str, _T(' '))) {
			str.Insert(0, _T('"'));
			str += _T('"');
		}
		m_pei->m_strCommand.Replace(buf, str);

		// 選択された既定のプログラムはリストで保存して、次回のリストでは上位に並ぶようにする
		RegSaveHistory(IDS_SEC_RUNAS, IDS_KEY_FAVORITEAPP, MAXEXECHISTORY, buf);
	}

	// 実行履歴保存
	RegSaveHistory(IDS_SEC_RUNAS, IDS_KEY_HISTORY, MAXEXECHISTORY, m_pei->m_strCommand);

	EndDialog(IDOK);
}
