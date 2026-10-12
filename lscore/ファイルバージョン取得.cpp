ファイルのバージョン情報を取得する
← 戻る → 

--------------------------------------------------------------------------------

　ファイルのバージョン情報（会社名, ファイルバージョン, 著作権, ... ）を取得するサンプル。

//*********************************************************
// 説明:
//   この関数は、バージョンリソースのコピーへのポインタ返す。
//   バージョンリソースのコピーは、使用後に解放しなくてはならない。
//
// 戻り値:
//     void * …… 関数が成功すると バージョンリソースのコピーへのポインタ を
//            　　 関数が失敗すると NULL を返す。
//
// 引数:
//     char   *FileName // ファイル名
//     DWORD **dwTrans  // バージョンリソースの言語情報
//*********************************************************
void *GetVersionInfo( char *FileName, DWORD **dwTrans )
{
	void  *pVersionInfo; // バージョンリソースを指すポインタ
	UINT   uDumy;        //
	DWORD  dwDumy;       //
	DWORD  dwSize;       // バージョンリソースのサイズ

	// バージョンリソースのサイズを取得
	dwSize = GetFileVersionInfoSize( FileName, &dwDumy );
	if ( dwSize <= 0 ) return NULL; // 失敗

	pVersionInfo = malloc( dwSize );
	if ( !pVersionInfo ) return NULL; // 失敗

	// バージョンリソースを取得
	if ( GetFileVersionInfo( FileName, 0, dwSize, pVersionInfo ) )
	{
		// バージョンリソースの言語情報を取得
		if ( VerQueryValue( pVersionInfo, "\\VarFileInfo\\Translation", (void **)dwTrans, &uDumy ) )
		{
			return pVersionInfo;
		}
	}
	free( pVersionInfo );
	return NULL; // 失敗
}//GetVersionInfo

//*********************************************************
// 説明:
//   バージョンリソースから、バージョン情報のコピーを取得する。
//
// 戻り値:
//     BOOL …… 関数が成功すると TRUE  を、
//          　　 関数が失敗すると FALSE を返す。
//
// 引数:
//           char  *Buffer       …… 取得するバージョン情報を受け取るバッファ
//           void  *pVersionInfo …… 関数GetVersionInfo() の戻り値
//           DWORD  dwTrans      …… バージョンリソースの言語情報
//     const char  *KeyWord      …… 取得したいバージョン情報の名前
//       "Comments"         // コメント
//       "CompanyName"      // 会社名
//       "FileDescription"  // 説明
//       "FileVersion"      // ファイルバージョン
//       "InternalName"     // 内部名
//       "LegalCopyright"   // 著作権
//       "LegalTrademarks"  // 商標
//       "OriginalFilename" // 正式ファイル名
//       "PrivateBuild"     // プライベートビルド情報
//       "ProductName"      // 製品名
//       "ProductVersion"   // 製品バージョン
//       "SpecialBuild"     // スペシャルビルド情報
//*********************************************************
BOOL GetVersionValue( char *Buffer, void *pVersionInfo, DWORD dwTrans, const char *KeyWord )
{
	char *pValue; // バージョンリソース中のバージョン情報を指すポインタ
	char  Path[MAX_PATH+1];
	UINT  uDumy;
	WORD  wCodePageID[] = { 0, 932, 949, 950, 1200, 1250, 1251, 1252, 1253, 1254, 1255, 1256 };
	WORD  wLanguageID[] = { 0x0401, 0x0402, 0x0403, 0x0404, 0x0405, 0x0406, 0x0407, 0x0408, 0x0409, 0x040A, 0x040B, 0x040C, 0x040D, 0x040E, 0x040F, 0x0410, 0x0411, 0x0412, 0x0413, 0x0414, 0x0415, 0x0416, 0x0417, 0x0418, 0x0419, 0x041A, 0x041B, 0x041C, 0x041D, 0x041E, 0x041F, 0x0420, 0x0421, 0x0804, 0x0807, 0x0809, 0x080A, 0x080C, 0x0810, 0x0813, 0x0814, 0x0816, 0x081A, 0x0C0C, 0x100C };

	sprintf( Path, "\\StringFileInfo\\%04x%04x\\%s", LOWORD(dwTrans), HIWORD(dwTrans), KeyWord );
	if ( VerQueryValue( pVersionInfo, Path, (void **)&pValue, &uDumy ) )
	{
		strcpy( Buffer, pValue );
		return TRUE; // 成功
	}

	// バージョンリソースに言語情報が記録されていない場合
	for( int i = 0; i < ( numof(wCodePageID) ); i++ )
	{
		for( int j = 0; j < numof(wLanguageID); j++ )
		{
			sprintf( Path, "\\StringFileInfo\\%04x%04x\\%s", wLanguageID[j], wCodePageID[i], KeyWord );
			if ( VerQueryValue( pVersionInfo, Path, (void **)&pValue, &uDumy ) )
			{
				strcpy( Buffer, pValue );
				return TRUE; // 成功
			}
		}
	}

	Buffer[0] = '\0';
	return FALSE; // 失敗
}//GetVersionValue


--------------------------------------------------------------------------------

使用例
//*********************************************************
//  ファイルからバージョン情報を取得するサンプル
//*********************************************************
int main( int argc, char **argv )
{
	char   FileDescription[100];
	char   ProductVersion[100];
	void  *pVersionInfo;
	DWORD *dwTrans;

	// argv[0]（自分自身）のバージョン情報
	pVersionInfo = GetVersionInfo( argv[0], &dwTrans );
	if ( pVersionInfo )
	{
		GetVersionValue( FileDescription, pVersionInfo, *dwTrans, "FileDescription" ); // 説明を取得
		GetVersionValue( ProductVersion,  pVersionInfo, *dwTrans, "ProductVersion"  ); // 製品バージョンを取得

		printf( "FileDescription = %s\n", FileDescription ); // 説明を表示
		printf( "ProductVersion  = %s\n", ProductVersion  ); // 製品バージョンを表示

		free( pVersionInfo );
	}

	return 0;
}//main


--------------------------------------------------------------------------------

水無瀬 優sinzan@mail.hidecnet.ne.jp 
