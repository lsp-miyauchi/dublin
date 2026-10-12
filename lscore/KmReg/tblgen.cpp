#include <windows.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <limits.h>
#include <time.h>

static char gszOriginal[] = "ABCDEFGHJKLMNPQRSTVWXYZ123456789";


// 利用方法：
//       tblgen <output_dataname>
//
//       <output_dataname>.bin と <output_dataname>.h が出力される

void main(int argc, char** argv)
{
	char buf[128];
	FILE* fpt = NULL;
	FILE* fpb = NULL;
	if (argc > 1) {
		sprintf(buf, "%s.h", argv[1]);
		fpt = fopen(buf, "wt");
		sprintf(buf, "%s.bin", argv[1]);
		fpb = fopen(buf, "wb");
	}

	if (fpt == NULL) fpt = stdout;

	SYSTEMTIME st;
	GetLocalTime(&st);
	fprintf(fpt, "///////////////////////////////////////////////////////\n");
	fprintf(fpt, "// MAPTABLE: %s  (%d/%02d/%02d)\n\n#pragma once\n\n", argv[1], st.wYear, st.wMonth, st.wDay);

	fprintf(fpt, "BYTE mapbyte_%s[] = {\n", argv[1]);

	int len = strlen(gszOriginal);
	strcpy(buf, gszOriginal);
	char ch;
	srand( (unsigned)time( NULL ) );
	for (int i = 0; i < 6; i++) {
		int num = 0;
		while (num < 999) {
			int s = rand() % len;
			int t = rand() % len;
			if (s != t) {
				ch = buf[s];
				buf[s] = buf[t];
				buf[t] = ch;
				num++;
			}
		}

		fprintf(fpt, "\t");
		for (int j = 0; j < len; j++) {
			ch = buf[j] - 0x20;
			fprintf(fpt, "0x%02X,", ch);
			if (fpb) fwrite(&ch, 1, sizeof(char), fpb);
		}
		fprintf(fpt, "\t// %d\n", i);
	}

	fprintf(fpt, "};\n");

	if (fpt != stdout) fclose(fpt);
	if (fpb) fclose(fpb);
}
