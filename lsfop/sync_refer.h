/////////////////////////////////////////////////////////////////////////
// sync_refer

class CSyncRefer
{
protected:
  CStringA m_strDBFile;

public:
  CSyncRefer() { }
  ~CSyncRefer() { }

  BOOL GetDBFileName(CString& str) { return GetAppDataFolder(str, _T("fsync.sqlite")); }

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
            "CREATE TABLE IF NOT EXISTS fsync_refer("
            "name TEXT PRIMARY KEY, refer INTEGER, lasttime INTEGER);",
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
      ok = (sqlite3_exec(db, "DROP TABLE fsync_refer;", NULL, 0, 0) == SQLITE_OK);
      sqlite3_close(db);
    }
    return ok;
  }

public:
  // --------------------------------------
  // データベースを開いてデータを書き込む
  // --------------------------------------
  bool UpdateItem(LPCWSTR wname, int refer, __time32_t ct)
  {
    sqlite3* db = OpenDB();
    if (db) {
      // UNICODE → UTF8
      char name[MAX_PATH * 2];
      WideCharToMultiByte(CP_UTF8, 0, wname, -1, name, ARRAYSIZE(name), NULL, NULL);
      // コマンドビルド
      char* cmd = sqlite3_mprintf(
            "INSERT OR REPLACE INTO fsync_refer(name,refer,lasttime) "
            "VALUES('%q','%u','%u');",
            name, refer, ct);
      int res = sqlite3_exec(db, cmd, NULL, NULL, NULL);
      sqlite3_free(cmd);
      sqlite3_close(db);
    }
    return (db != NULL);
  }

  // --------------------------------------
  // データベースからデータを読み込む
  // --------------------------------------
  bool FetchItem(LPCWSTR wname, int& refer, __time32_t& ct)
  {
    bool ok = false;
    sqlite3* db = OpenDB();
    if (db) {
      // 最終時刻で降順(DESC)に読み出し
      int row = 0, col = 0;
      char** argv;
      char sname[MAX_PATH * 2];
      WideCharToMultiByte(CP_UTF8, 0, wname, -1, sname, sizeof(sname), NULL, NULL);
      CStringA cmd; cmd.Format("SELECT name, refer, lasttime FROM fsync_refer WHERE name='%s'", sname);
      int res = sqlite3_get_table(db, cmd.GetString(), &argv, &row, &col, NULL);
      if (res == SQLITE_OK) {
        char** pp = argv + col;
        ++pp;	// skip name
        refer = atoi(*pp++);
        ct = (time_t)atoi(*pp++);
        ok = true;
        sqlite3_free_table(argv);
      }
      // データベースを閉じる
      sqlite3_close(db);
    }
    return ok;
  }
};

