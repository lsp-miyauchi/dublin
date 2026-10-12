#define _CRT_SECURE_NO_DEPRECATE
#define _CRT_NON_CONFORMING_SWPRINTFS

// #define _ATL_CSTRING_EXPLICIT_CONSTRUCTORS
#define _WTL_NO_CSTRING

#include <atlbase.h>
#include <atlstr.h>
#include <atlcoll.h>
#include <stdio.h>
#include <shlwapi.h>

#pragma comment(lib, "shlwapi.lib")


int main(int argc, char** argv)
{
	if (argc < 2) return -1;

	PathUnquoteSpaces(argv[1]);
	CString bakFile = argv[1];
	bakFile += ".bak";

	if (!CopyFile(argv[1], bakFile, FALSE)) {
		fprintf(stderr, "cannot backup %s to %s.", PathFindFileName(argv[1]), PathFindFileName(bakFile));
		return -1;
	}
	CString verOld, verNew;
	int major, minor, build, revision;
	bool inVersionStruct = false;
	FILE* fpr = fopen(bakFile, "rt");
	if (fpr == NULL) {
		fprintf(stderr, "cannot open %s for read.", PathFindFileName(argv[1]));
		return -1;
	}
	FILE* fpw = fopen(argv[1], "wt");
	if (fpw == NULL) {
		fclose(fpr);
		fprintf(stderr, "cannot open %s for write.", PathFindFileName(bakFile));
		return-1;
	}

	char line[256];
	while (fgets(line, ARRAYSIZE(line), fpr) != NULL) {
		CString str = line;
		if (!inVersionStruct) {
			if (str.Find("VS_VERSION_INFO VERSIONINFO") >= 0)
				inVersionStruct = true;
		}
		else if (str.Find("BLOCK \"VarFileInfo\"") >= 0)
			inVersionStruct = false;
		else {
			bool quote = false;
			int pos;
			verOld.Empty();
			// FILEVERSION
			pos = str.Find("FILEVERSION");
			if (pos >= 0)
				verOld = str.Mid(pos + 11);
			// PRODUCTVERSION
			pos = str.Find("PRODUCTVERSION");
			if (pos >= 0)
				verOld = str.Mid(pos + 14);
			// "FileVersion"
			pos = str.Find("VALUE \"FileVersion\",");
			if (pos >= 0) {
				verOld = str.Mid(pos + 20);
				quote = true;
			}
			// "ProductVersion"
			pos = str.Find("VALUE \"ProductVersion\",");
			if (pos >= 0) {
				verOld = str.Mid(pos + 23);
				quote = true;
			}

			if (!verOld.IsEmpty()) {
				verOld.Trim(" \"");
				if (quote) {
					int cnv = sscanf(verOld.GetString(), "%d.%d.%d.%d", &major, &minor, &build, &revision);
					if (cnv < 4) {
						sscanf(verOld.GetString(), "%d,%d,%d,%d", &major, &minor, &build, &revision);
					}
					verOld.Format("%d.%d.%d.%d", major, minor, build, revision);
					verNew.Format("%d.%d.%d.%d", major, minor, build, revision + 1);
				}
				else {
					sscanf(verOld.GetString(), "%d,%d,%d,%d", &major, &minor, &build, &revision);
					verOld.Format("%d,%d,%d,%d", major, minor, build, revision);
					verNew.Format("%d,%d,%d,%d", major, minor, build, revision + 1);
				}
				str.Replace(verOld, verNew);
			}
		}
		fputs(str.GetString(), fpw);
	}
	fclose(fpr);
	fclose(fpw);
	return 0;
}
