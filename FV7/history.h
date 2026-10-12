//////////////////////////////////////////////////////////////////////
// CHistory

#pragma once
#ifndef __FV_HISTORY_H__
#define __FV_HISTORY_H__

#include <time.h>

//////////////////////////////////////////////////////////////////////
//

class CAccessHistoryArray : CSimpleArray<LPCITEMIDLIST>
{
public:
	int m_maxItems;

	CAccessHistoryArray() : m_maxItems(64)
	{
	}
	~CAccessHistoryArray()
	{
		DeleteAll();
	}
	void DeleteAll()
	{
		int cnt = GetSize();
		for (int i = 0; i < cnt; i++)
			RemoveItem(0);
	}
	void RemoveItem(int pos)
	{
		if (0 <= pos && pos < GetSize()) {
			LPCITEMIDLIST pidl = (*this)[pos];
			if (pidl) {
				(*this)[pos] = NULL;
				CoTaskMemFree((void*)pidl);
			}
			RemoveAt(pos);
		}
	}
	int FindItem(LPCITEMIDLIST pidlSrc)
	{
		if (pidlSrc) {
			int cnt = GetSize();
			for (int i = 0; i < cnt; i++) {
				LPCITEMIDLIST pidlDst = (*this)[i];
				if (pidlDst && ILIsEqual(pidlSrc, pidlDst))
					return i;
			}
		}
		return -1;
	}
	BOOL AddItem(LPCITEMIDLIST pidl)
	{
		ATLASSERT(pidl != NULL);
		if (!pidl) return FALSE;

		// 記録済みのアイテムが見つかれば、いったん削除して先頭に追加する
		// こうすることでアクセスされないデータはケツに追いやられる
		int pos = FindItem(pidl);
		if (pos >= 0)
			RemoveItem(pos);

		LPCITEMIDLIST pidlCopy = ILClone(pidl);
		if (!pidlCopy) return FALSE;

		// まず追加する
		Add(pidlCopy);

		int cnt = GetSize();
		if (cnt > 1) {
			// 登録データが複数ある場合には、追加データは先頭へ
			BYTE* ptr = (BYTE*)GetData();
			memmove(ptr + sizeof(LPCITEMIDLIST), ptr, (cnt - 1) * sizeof(LPCITEMIDLIST));
			*((LPCITEMIDLIST*)ptr) = pidlCopy;		// 高速化
		}

		if (cnt >= m_maxItems) {
			// 最大記録数を超えたら、ケツをカット
			RemoveItem(cnt - 1);
		}

		return TRUE;
	}

	void Load(int maxEntries)
	{
		DeleteAll();

		CRegKey sec;
		if (!GetRegSection(sec, _T("SmartFolder\\Favorite"))) return;

		CString str;
		for (int i = 0; i < maxEntries; i++) {
			str.Format(_T("%d"), i);

 			ULONG len = 0;
			if (sec.QueryBinaryValue(str, NULL, &len) == ERROR_SUCCESS) {
				void* pv = malloc(len);
				if (pv) {
					if (sec.QueryBinaryValue(str, pv, &len) == ERROR_SUCCESS)
						AddItem((LPCITEMIDLIST)pv);
					free(pv);
				}
			}
		}
	}

	void Save(int maxEntries)
	{
		CRegKey sec;
		if (!GetRegSection(sec, _T("SmartFolder\\Favorite"))) return;

		CSimpleArray<CString> array;
		DWORD index = 0;
		do {
			TCHAR buf[MAX_PATH];
			DWORD len = ARRAYSIZE(buf);
			if (RegEnumValue(sec, index, buf, &len, NULL, NULL, NULL, NULL) != ERROR_SUCCESS)
				break;
			array.Add(buf);
			index++;
		} while(1);

		int i, cnt = array.GetSize();
		for (i = 0; i < cnt; i++)
			sec.DeleteValue(array[i]);

		CString str;
		cnt = min(maxEntries, GetSize());
		for (i = 0; i < cnt; i++) {
			str.Format(_T("%d"), i);
			LPCITEMIDLIST pidl = (*this)[i];
			sec.SetBinaryValue(str, pidl, ILGetSize(pidl));
		}
	}
};


//////////////////////////////////////////////////////////////////////
//

#define INTERVAL_FIREUPDATE		900				// 15min


class CListStyleMap
{
protected:
	int m_maxItems;
	__time32_t m_timeFire;
	bool m_bUpdateInteractive;
	CStringA m_strDBFile;
	CSimpleArray<CString> m_arrName;				// ItemIDList
	CSimpleArray<CIconListStyle> m_arrStyle;		// スタイル
	CSimpleArray<__time32_t> m_arrTime;				// 更新日時
	CSimpleArray<BOOL> m_arrUpdate;					// 更新フラグ

public:
	CListStyleMap() : m_maxItems(128), m_bUpdateInteractive(false)
	{
		CString str;
		if (GetDBFileName(str)) {
			LPSTR p = m_strDBFile.GetBuffer(MAX_PATH * 2);
			WideCharToMultiByte(CP_UTF8, 0, str, -1, p, MAX_PATH * 2, NULL, NULL);
			m_strDBFile.ReleaseBuffer();
			if (!InitDB())
				m_strDBFile.Empty();
		}

		// 次回更新時刻をセット
		_time32(&m_timeFire);
		m_timeFire += INTERVAL_FIREUPDATE;			// 次回更新時刻
	}

	~CListStyleMap()
	{
		DeleteAll();
	}

	BOOL GetDBFileName(CString& str)
	{
		return GetAppDataFolder(str, _T("places.sqlite"));
	}

	void DiscardAllSettings()
	{
		DropDB();
		DeleteAll();
		InitDB();
	}

private:
	// --------------------------------------
	// データベースを開く
	// --------------------------------------
	sqlite3* OpenDB()
	{
		sqlite3* db;
		if (!m_strDBFile.IsEmpty() && (sqlite3_open(m_strDBFile, &db) == SQLITE_OK))
			return db;
		return NULL;
	}

	// --------------------------------------
	// データベースにテーブルを作成（必要ならば）
	// --------------------------------------
	BOOL InitDB()
	{
		BOOL ok = FALSE;
		sqlite3* db = OpenDB();
		if (db) {
			// 存在しなければテーブル作成
			ok = (sqlite3_exec(db,
						"CREATE TABLE IF NOT EXISTS liststyles("
						"name TEXT PRIMARY KEY, status INTEGER, style INTEGER, enlarge INTEGER, sort INTEGER, thumb INTEGER, lasttime INTEGER);",
						NULL, 0, 0) == SQLITE_OK);
			sqlite3_close(db);
		}
		return ok;
	}

	BOOL DropDB()
	{
		BOOL ok = FALSE;
		sqlite3* db = OpenDB();
		if (db) {
			// テーブル削除
			ok = (sqlite3_exec(db, "DROP TABLE liststyles;", NULL, 0, 0) == SQLITE_OK);
			sqlite3_close(db);
		}
		return ok;
	}

public:
	// --------------------------------------
	// 配列データをすべて削除
	// --------------------------------------
	void DeleteAll()
	{
		m_arrName.RemoveAll();
		m_arrStyle.RemoveAll();
		m_arrTime.RemoveAll();
		m_arrUpdate.RemoveAll();
	}

	// --------------------------------------
	// 配列データの指定位置を削除
	// --------------------------------------
	void RemoveItem(int pos)
	{
		if (0 <= pos && pos < m_arrName.GetSize()) {
			m_arrName.RemoveAt(pos);
			m_arrStyle.RemoveAt(pos);
			m_arrTime.RemoveAt(pos);
			m_arrUpdate.RemoveAt(pos);
		}
	}

	// --------------------------------------
	// 配列からデータを見つける
	// --------------------------------------
	int FindItem(LPCITEMIDLIST pidlSrc)
	{
		int index = -1;
		CString name;
		if (idl_GetName(pidlSrc, SHGDN_FORPARSING, name))
			index = m_arrName.Find(name);
		return index;
	}

	// --------------------------------------
	// 配列のデータを更新または追加
	// --------------------------------------
	BOOL SetItemStyle(LPCITEMIDLIST pidl, CIconListStyle style)
	{
		ATLASSERT(pidl != NULL);
		CString name;
		if (!idl_GetName(pidl, SHGDN_FORPARSING, name)) return FALSE;

		// 記録済みのアイテムが見つかれば、いったん削除して先頭に追加する
		// こうすることでアクセスされないデータはケツに追いやられる
		int pos = m_arrName.Find(name);
		if (pos >= 0)
			RemoveItem(pos);

		// 現在時刻（経過時間）を取得
		__time32_t curtime;
		_time32(&curtime);

		// まず追加する
		m_arrName.Add(name);
		m_arrStyle.Add(style);
		m_arrTime.Add(curtime);
		m_arrUpdate.Add(pos >= 0);		// すでに存在していた場合はTRUE

		int cnt = m_arrName.GetSize();
		if (cnt > 1) {
			// 登録データが複数ある場合には、追加データは先頭へ
			memmove((void*)(m_arrName.GetData() + 1), (void*)m_arrName.GetData(), (cnt - 1) * sizeof(CString));
			m_arrName.SetAtIndex(0, name);

			memmove((void*)(m_arrStyle.GetData() + 1), (void*)m_arrStyle.GetData(), (cnt - 1) * sizeof(CIconListStyle));
			m_arrStyle.SetAtIndex(0, style);

			memmove((void*)(m_arrTime.GetData() + 1), (void*)m_arrTime.GetData(), (cnt - 1) * sizeof(__time32_t));
			m_arrTime.SetAtIndex(0, curtime);

			memmove((void*)(m_arrUpdate.GetData() + 1), (void*)m_arrUpdate.GetData(), (cnt - 1) * sizeof(BOOL));
			m_arrUpdate.SetAtIndex(0, pos >= 0);
		}

		if (cnt >= m_maxItems) {
			// 最大記録数を超えたら、ケツをカット
			RemoveItem(cnt - 1);
		}

		// 同時更新の場合
		if (m_bUpdateInteractive)
			UpdateDBItem(name, style, curtime);
		else {
			if (curtime >= m_timeFire) {
				Save(true);
				m_timeFire = curtime + INTERVAL_FIREUPDATE;			// 次回更新時刻
			}
		}

		return TRUE;
	}

	// --------------------------------------
	// 配列から一致するデータのリストスタイル情報を取得
	// --------------------------------------
	BOOL GetItemStyle(LPCITEMIDLIST pidl, CIconListStyle& rstyle)
	{
		int pos = FindItem(pidl);
		if (pos < 0) return FALSE;

		rstyle = m_arrStyle[pos];
		return TRUE;
	}

	// --------------------------------------
	// データベースにデータを書き込む
	// --------------------------------------
	int dbSetItem(sqlite3* db, LPCWSTR wname, const CIconListStyle& ils, __time32_t ct)
	{
		// UNICODE → UTF8
		char name[MAX_PATH * 2];
		WideCharToMultiByte(CP_UTF8, 0, wname, -1, name, ARRAYSIZE(name), NULL, NULL);
		// コマンドビルド
		char* cmd = sqlite3_mprintf(
						"INSERT OR REPLACE INTO liststyles(name,status,style,enlarge,sort,thumb,lasttime) "
						"VALUES('%q','%u','%u','%u','%u','%u','%u');",
						name, ils.status, ils.style, ils.enlarge,
						MAKELONG(ils.sort_type, ils.sort_seq),
						MAKELONG(ils.size_thumb.cx, ils.size_thumb.cy), ct);
		int res = sqlite3_exec(db, cmd, NULL, NULL, NULL);
		sqlite3_free(cmd);
#ifdef DEBUG
		if (res != SQLITE_OK) dbg.printfA("sqlite3_exec(INSERT): %s\n", sqlite3_errmsg(db));
#endif
		return res;
	}

	// --------------------------------------
	// データベースを開いてデータを書き込む
	// --------------------------------------
	BOOL UpdateDBItem(LPCWSTR wname, const CIconListStyle& ils, __time32_t ct)
	{
		sqlite3* db = OpenDB();
		if (db) {
			dbSetItem(db, wname, ils, ct);
			sqlite3_close(db);
		}
		return TRUE;
	}

	// --------------------------------------
	// 配列データをデータベースに書き出す
	// --------------------------------------
	BOOL Save(bool bDeleteOldLog = false)
	{
		// データベースを開く
		sqlite3* db = OpenDB();
		if (db) {
			// トランザクション開始
			sqlite3_exec(db, "BEGIN;", NULL, NULL, NULL);

			char* cmd;
			int res;
			// 同時更新ONの場合は何もしない
			if (!m_bUpdateInteractive) {
				// レコード挿入
				int cnt = m_arrName.GetSize();
				for (int i = 0; i < cnt; i++) {
					// 更新のあったものだけDBに反映する
					if (m_arrUpdate[i]) {
						dbSetItem(db, m_arrName[i], m_arrStyle[i], m_arrTime[i]);
						m_arrUpdate.SetAtIndex(i, FALSE);
					}
				}
			}

			if (bDeleteOldLog) {
				// 不要な古いデータを削除（今現在より新しいものもデータ異常なので削除）
				// 2592000 = 過去30日以上前 = 60sec * 60min * 24hour * 30day
				__time32_t t;
				_time32(&t);
				cmd = sqlite3_mprintf("DELETE FROM liststyles WHERE lasttime NOT BETWEEN %u AND %u", t - 2592000, t);
				res = sqlite3_exec(db, cmd, NULL, NULL, NULL);
#ifdef DEBUG
				if (res != SQLITE_OK) dbg.printfA("sqlite3_exec(DELETE): %s\n", sqlite3_errmsg(db));
#endif
				sqlite3_free(cmd);
				// トランザクションをコミット
			}
			sqlite3_exec(db, "COMMIT;", NULL, NULL, NULL);
			// データベースを閉じる
			sqlite3_close(db);
		}
		return TRUE;
	}

	// --------------------------------------
	// データベースから配列にデータを読み込む
	// --------------------------------------
	BOOL Load()
	{
		sqlite3* db = OpenDB();
		if (db) {
			// 現在の情報を破棄
			DeleteAll();
			// 最終時刻で降順(DESC)に読み出し
			int row = 0, col = 0;
			char** argv;
			int res = sqlite3_get_table(db, "SELECT name,status,style,enlarge,sort,thumb,lasttime FROM liststyles ORDER BY lasttime DESC", &argv, &row, &col, NULL);
			if (res == SQLITE_OK) {
				CIconListStyle ils;
				TCHAR name[MAX_PATH * 2];
				int val;
				for (int i = 1; i <= row; i++) {
					char** pp = argv + col * i;
					MultiByteToWideChar(CP_UTF8, 0, *pp++, -1, name, ARRAYSIZE(name));
					val = atoi(*pp++); ils.status = val;
					val = atoi(*pp++); ils.style = val;
					val = atoi(*pp++); ils.enlarge = val;
					val = atoi(*pp++); ils.sort_type = LOWORD(val); ils.sort_seq = HIWORD(val);
					val = atoi(*pp++); ils.size_thumb.cx = LOWORD(val); ils.size_thumb.cy = HIWORD(val);
					val = atoi(*pp++);
					m_arrName.Add(name);
					m_arrStyle.Add(ils);
					m_arrTime.Add((time_t)val);
					m_arrUpdate.Add(FALSE);
				}
				sqlite3_free_table(argv);
			}
#ifdef DEBUG
			else dbg.printfA("sqlite3_get_table(SELECT): %s\n", sqlite3_errmsg(db));
#endif

			// データベースを閉じる
			sqlite3_close(db);
		}
		return TRUE;
	}
};

//////////////////////////////////////////////////////////////////////
//

enum FBType {
	FB_ID = 1,
	FB_INDEX,
	FB_NAME,
	FB_IDLIST
};

class CHistory : public CAtlArray<CCmItem*>
{
public:
	int m_posCur;
	int m_nLimit;
	bool m_bDisableUpdate;

	CHistory() :
			m_nLimit(16),
			m_posCur(0),
			m_bDisableUpdate(false)
	{
	}
	inline ~CHistory() { Clear(); }
	void Clear()
	{
		int cnt = (int)GetCount();
		for (int i = 0; i < cnt; i++) {
			CCmItem* pci = (*this)[i];
			if (pci) delete pci;
		}
		RemoveAll();
		m_posCur = 0;
	}
	void Reset()
	{
		// m_posCurの位置のみ残して他を削除
		int cnt = (int)GetCount();
		if (cnt > 0) {
			CCmItem* pci = (*this)[m_posCur];
			RemoveAt(m_posCur);		// 指針位置だけpciを残して項目位置を削除
			Clear();				// 他を全削除
			Add(pci);				// 残しておいた項目を再登録
		}
		m_posCur = 0;
	}
	inline void DisableUpdate(bool bDisableUpdate) { m_bDisableUpdate = bDisableUpdate; }
	inline bool IsUpdateDisabled() { return m_bDisableUpdate; }

	bool RemoveItem(int index)
	{
		if (index < 0 || index >= (int)GetCount()) {
			return false;
		}

		CCmItem* pci = (*this)[m_posCur];
		int idRestore = pci->m_id;
		pci = (*this)[index];
		bool sameId = (pci->m_id == idRestore);
		if (pci) delete pci;
		RemoveAt(index);

		m_posCur = sameId ? 0 : FindIndexBy(FB_ID, (LPARAM)idRestore);
		return true;
	}

	bool RemoveItem(LPCITEMIDLIST pidl)
	{
		return RemoveItem(FindIndexBy(FB_IDLIST, (LPARAM)pidl));
	}

	int RemoveItemAll(LPCITEMIDLIST pidl)
	{
		int cnt = (int)GetCount();
		Clear();
		return cnt;
	}

	void PushItem(LPCITEMIDLIST pidl);

	CCmItem* GetItemAt(int index) const
	{
		if (0 <= index && index < GetCount())
			return (*this)[index];
		return NULL;
	}
	inline CCmItem* GetCurrentItem() const { return GetItemAt(m_posCur); }

	int FindIndexBy(FBType type, LPARAM key) const
	{
		if (type == FB_INDEX) {
			return -1;
		}
		int cnt = (int)GetCount();
		for (int i = 0; i < cnt; i++) {
			CCmItem* pci = (*this)[i];
			if ((type == FB_ID && pci->m_id == (int)key) ||
				(type == FB_NAME && pci->m_name.CompareNoCase((LPCTSTR)key) == 0) ||
				(type == FB_IDLIST && pci->m_idl.isEqual((LPCITEMIDLIST)key))) {
				return (int)i;
			}
		}
		return -1;
	}

	CCmItem* FindBy(FBType type, LPARAM key) const
	{
		if (type == FB_INDEX) {
			return GetItemAt((int)key);
		}
		int index = FindIndexBy(type, key);
		return index >= 0 ? GetItemAt(index) : NULL;
	}

	LPITEMIDLIST HistoryMenu(HWND hParent, CPoint pt);
	LPITEMIDLIST BackwardHistoryMenu(HWND hParent, CPoint pt);
	LPITEMIDLIST ForwardHistoryMenu(HWND hParent, CPoint pt);

	LPITEMIDLIST Back()
	{
		LPITEMIDLIST pidl = NULL;
		int cnt = (int)GetCount();
		if (cnt > 0 && m_posCur < cnt - 1) {
			++m_posCur;
			pidl = (*this)[m_posCur]->m_idl;
		}
		return pidl;
	}

	LPITEMIDLIST Forward()
	{
		LPITEMIDLIST pidl = NULL;
		int cnt = (int)GetCount();
		if (m_posCur > 0 && m_posCur < cnt) {
			--m_posCur;
			pidl = (*this)[m_posCur]->m_idl;
		}
		return pidl;
	}
	__inline bool isEmpty() const { return (GetCount() == 0); }
	__inline bool canBack() const { int cnt = (int)GetCount(); return (cnt > 0 && m_posCur < cnt - 1); }
	__inline bool canForward() const { int cnt = (int)GetCount(); return (cnt > 0 && m_posCur > 0); }

private:
	int TrackMenu(HWND hParent, CSimpleArray<int>& array, CPoint pt, BOOL bAddClear = FALSE, int posCur = -1);
};


#endif		// __FV_HISTORY_H__
