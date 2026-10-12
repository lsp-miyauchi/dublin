//////////////////////////////////////////////////////////////////////
//
//
//	CLSID_DocFileColumnProvider : ドキュメント情報
//	CLSID_FileSysColumnProvider : 所有者情報？
//	CLSID_LinkColumnProvider : 作成者…
//  szCLSID[] = _T("{66742402-F9B9-11D1-A202-0000F81FEDEE}"); : バージョン情報
//		CLSID clsid;
//		CLSIDFromString(sszCLSIDVersionInfo, &clsid);


void ShowFileColumnProvider(REFCLSID rclsid, LPCTSTR pszFile)
{
	DWORD attr = GetFileAttributes(pszFile);
	if (attr == 0xffffffff) return;

	TCHAR path[MAX_PATH * 2];
	lstrcpy(path, pszFile);
	PathRemoveFileSpec(path);

	CComPtr<IColumnProvider> pcp;
	if (CoCreateInstance(rclsid, NULL, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&pcp)) == S_OK) {
		SHCOLUMNINIT sci;
		memset(&sci, 0, sizeof(sci));
		lstrcpy(sci.wszFolder, path);
		if (pcp->Initialize(&sci) == S_OK) {
			SHCOLUMNINFO info;
			for (DWORD index = 0; ; index++) {
				if (pcp->GetColumnInfo(index, &info) != S_OK) break;

				dbg.printf(L"%d:<%s>:", index, info.wszTitle);

				SHCOLUMNDATA data;
				memset(&data, 0, sizeof(data));
				data.dwFlags = SHCDF_UPDATEITEM;
				data.dwFileAttributes = attr;
				data.pwszExt = PathFindExtension(pszFile);
				lstrcpy(data.wszFile, pszFile);

				CComVariant var;
				if (pcp->GetItemData(&info.scid, &data, &var) == S_OK) {
					if (var.vt == VT_DATE) {
						SYSTEMTIME st;
						VariantTimeToSystemTime(var.dblVal, &st);
						SYSTEMTIME lt;
						SystemTimeToTzSpecificLocalTime(NULL, &st, &lt);
						dbg.printf(L"%04d/%02d/%02d %02d:%02d:%02d\n", lt.wYear, lt.wMonth, lt.wDay, lt.wHour, lt.wMinute, lt.wSecond);
					}
					else if (var.vt == VT_BSTR) {
						dbg.printf(L"%s\n", var.bstrVal);
						SysFreeString(var.bstrVal);
					}
					else if (var.vt == VT_I4) {
						dbg.printf(L"%d\n", var.intVal);
					}
					else
						dbg.printf(L"ok:<%d>\n", V_VT(&var));
				}
				else dbg.printf(L"error\n");
			}
		}
	}
}

