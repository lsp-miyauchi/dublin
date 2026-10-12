#include <tchar.h>
#include <windows.h>
#include <shlwapi.h>
#include <stdio.h>
#include <string.h>

#pragma comment(lib, "shlwapi.lib")
#pragma comment(lib, "user32.lib")

TCHAR root[MAX_PATH];

void utf2sjis(LPSTR pstr)
{
	SIZE_T size = strlen(pstr) + 1;
	WCHAR* wp = new WCHAR[size * 2];
	MultiByteToWideChar(CP_UTF8, 0, pstr, -1, wp, size * 2);
	WideCharToMultiByte(CP_ACP, 0, wp, -1, pstr, size * 2, NULL, NULL);
	delete [] wp;
}

void ParseFile(LPTSTR file)
{
	TCHAR buf[2048];
	TCHAR rel[MAX_PATH];
	PathRelativePathTo(rel, root, FILE_ATTRIBUTE_DIRECTORY, file, 0);

	FILE* fp = fopen(file, _T("rt"));
	if (!fp) return;

	TCHAR* p = rel;
	TCHAR* q;
	while (*p) {
		if (*p == _T('\\'))
			*p = _T('/');
		p = CharNext(p);
	}

	bool utf = false;
	while (fgets(buf, 2048, fp)) {
		p = StrStrI(buf, _T("</head>"));
		if (p) break;

		if (StrStrI(buf, "charset=utf-8")) {
			utf = true;
			continue;
		}

		if (utf) utf2sjis(buf);
		p = StrStrI(buf, _T("<title>"));
		if (p) {
			p += 7;
			q = StrStrI(p, _T("</title>"));
			if (q) *q = 0;

			if (*p) {
				printf("\t<LI> <OBJECT type=\"text/sitemap\">\n");
				printf("\t\t<param name=\"Name\" value=\"%s\">\n", p);
				printf("\t\t<param name=\"Local\" value=\"%s\">\n", rel);
				printf("\t\t</OBJECT>\n");
			}
		}
		p = StrStrI(buf, _T("<meta name=\"keywords\""));
		if (p) {
			p = StrStrI(p, _T("content="));
			if (p) {
				while (*p && *p != _T('"')) p++;
				if (*p) {
					p++;
					q = StrChr(p, _T('"'));
					if (q) *q = 0;

					while (p && *p) {
						q = StrChr(p, _T(';'));
						if (q) *q++ = 0;

						while (*p == _T(' ')) p++;
						if (*p) {
							printf("\t<LI> <OBJECT type=\"text/sitemap\">\n");
							printf("\t\t<param name=\"Name\" value=\"%s\">\n", p);
							printf("\t\t<param name=\"Local\" value=\"%s\">\n", rel);
							printf("\t\t</OBJECT>\n");
						}

						p = q;
					}
				}
			}
		}
	}
	fclose(fp);
}

void ParseDir(LPTSTR path)
{
	WIN32_FIND_DATA wfd;
	TCHAR dir[MAX_PATH];

	PathCombine(dir, path, _T("*.*"));
	HANDLE hFind = FindFirstFile(dir, &wfd);
	if (hFind != INVALID_HANDLE_VALUE) {
		do {
			if (wfd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
				if (wfd.cFileName[0] != _T('.')) {
					PathCombine(dir, path, wfd.cFileName);
					ParseDir(dir);
				}
			}
			else {
				LPTSTR p = StrRChr(wfd.cFileName, NULL, _T('.'));
				if (p && (!StrCmpI(p, _T(".htm")) || !StrCmpI(p, _T(".html")))) {
					PathCombine(dir, path, wfd.cFileName);
					ParseFile(dir);
				}
			}
		} while (FindNextFile(hFind, &wfd));
		FindClose(hFind);
	}
}

void main()
{
	GetCurrentDirectory(MAX_PATH, root);

	printf("<!DOCTYPE HTML PUBLIC \"-//IETF//DTD HTML//EN\">\n");
	printf("<HTML>\n<HEAD>\n");
	printf("<meta name=\"GENERATOR\" content=\"Microsoft&reg; HTML Help Workshop 4.1\">\n");
	printf("<!-- Sitemap 1.0 -->\n");
	printf("</HEAD><BODY>\n");
	printf("<UL>\n");

	ParseDir(root);

	printf("</UL>\n");
	printf("</BODY></HTML>\n");
}
