#define UNZIP_INTERNAL
#include "unzip.h"   /* defines COPYRIGHT_CLEAN by default */

#ifdef WIN32

#include "implode.h"

unsigned int impdcl_read(char* pdata, unsigned int* size, void* pv)
{
	__GPRO = (Uz_Globs*)pv;
	register unsigned int ch;
	unsigned int left = *size, cnt = 0;
	register char* pd = pdata;
	while (left-- > 0)
	{
		ch = readbyte(__G);
		if (ch == EOF)
			break;
		*pd++ = ch;
		cnt++;
	}
	return cnt;
}

void impdcl_write(char* pdata, unsigned int* size, void* pv)
{
	__GPRO = (Uz_Globs*)pv;
	flush(__G__ pdata, *size, 0);
}

void impdcl(__G)
     __GDEF
{
	char* pw = (char*)malloc(EXP_BUFFER_SIZE);
	if (pw)
	{
		unsigned int err = explode(impdcl_read, impdcl_write, pw, __G);
		free(pw);
	}
}

#endif			// WIN32
