#include "stdafx.h"

//////////////////////////////////////////////////////////////////////
//
//	CLSID_DocFileColumnProvider : ドキュメント情報
//	CLSID_FileSysColumnProvider : 所有者情報？
//	CLSID_LinkColumnProvider : 作成者…
//  szCLSID[] = _T("{66742402-F9B9-11D1-A202-0000F81FEDEE}"); : バージョン情報
//		CLSID clsid;
//		CLSIDFromString(sszCLSIDVersionInfo, &clsid);

typedef struct tragFATTRINFO {
	DWORD flag;
	LPCWSTR info;
} FATTRINFO;

static FATTRINFO stfi[] = {
	FILE_ATTRIBUTE_READONLY,				L"r",
	FILE_ATTRIBUTE_HIDDEN,					L"h",
	FILE_ATTRIBUTE_SYSTEM,					L"s",
	FILE_ATTRIBUTE_DIRECTORY,				L"d",
	FILE_ATTRIBUTE_ARCHIVE,					L"a",
	FILE_ATTRIBUTE_TEMPORARY,				L"TEMPORARY",
	FILE_ATTRIBUTE_SPARSE_FILE,				L"SPARSE_FILE",
	FILE_ATTRIBUTE_REPARSE_POINT,			L"REPARSE_POINT",
	FILE_ATTRIBUTE_COMPRESSED,				L"COMPRESSED",
	FILE_ATTRIBUTE_OFFLINE,					L"OFFLINE",
	FILE_ATTRIBUTE_NOT_CONTENT_INDEXED,		L"NOT_CONTENT_INDEXED",
	FILE_ATTRIBUTE_ENCRYPTED,				L"ENCRYPTED",
	FILE_ATTRIBUTE_INTEGRITY_STREAM,		L"INTEGRITY_STREAM",
	FILE_ATTRIBUTE_VIRTUAL,					L"VIRTUAL",
	FILE_ATTRIBUTE_NO_SCRUB_DATA,			L"NO_SCRUB_DATA",
	FILE_ATTRIBUTE_EA,						L"EA",
	FILE_ATTRIBUTE_PINNED,					L"PINNED",
	FILE_ATTRIBUTE_UNPINNED,				L"UNPINNED",
	FILE_ATTRIBUTE_RECALL_ON_OPEN,			L"RECALL_ON_OPEN",
	FILE_ATTRIBUTE_RECALL_ON_DATA_ACCESS,	L"RECALL_ON_DATA_ACCESS",
	FILE_ATTRIBUTE_STRICTLY_SEQUENTIAL,		L"STRICTLY_SEQUENTIAL",
};

LPCWSTR DumpFileAttributes(DWORD flag, CString& str)
{
	str = L"";
	if (flag == 0) {
		str = L"NORMAL";
	}
	else {
		CString s = L"";
		for (int i = 0; i < ARRAYSIZE(stfi); i++) {
			if (stfi[i].flag & flag) {
				if (s.GetLength() > 0) {
					s += L" | ";
				}
				s += stfi[i].info;
			}
		}
		str.Format(L"0x%08X: %s", flag, s.GetString());
	}
	return str.GetString();
}

void ShowFileColumnProvider(REFCLSID rclsid, LPCTSTR pszFile)
{
	DWORD attr = GetFileAttributes(pszFile);
	if (attr == 0xffffffff) return;

	CString s;
	dbg.printf(L"%s: file attributes - %s\n", PathFindFileName(pszFile), DumpFileAttributes(attr, s));

	CString path = pszFile;
	if (attr & FILE_ATTRIBUTE_DIRECTORY)
		PathRemoveFileSpec(path.GetBuffer());

	CComPtr<IColumnProvider> pcp;
	if (CoCreateInstance(rclsid, NULL, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&pcp)) == S_OK) {
		SHCOLUMNINIT sci;
		memset(&sci, 0, sizeof(sci));
		StrCpyN(sci.wszFolder, path, ARRAYSIZE(sci.wszFolder));
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
				StrCpyN(data.wszFile, pszFile, ARRAYSIZE(data.wszFile));

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
	else {
		dbg.puts(L"interface NOT available.");
	}
}

