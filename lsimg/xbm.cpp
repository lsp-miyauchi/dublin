#define STRICT
#include "stdafx.h"
#include "shlwapi.h"
#include "lsimg.h"
#include "common.h"

#define MaxTextExtent  		1664

static int XBMInteger(FILE* fp, short int *hex_digits)
{
	int c, flag, value;
	value = 0;
	flag = 0;
	for ( ; ; )
	{
		c = fgetc(fp);
		if (c == EOF)
		{
			value = (-1);
			break;
		}
		c &= 0xff;
		if (isxdigit(c))
		{
			value = (value << 4) + hex_digits[c];
			flag++;
			continue;
		}
		if ((hex_digits[c]) < 0 && flag)
			break;
	}
	return value;
}

extern "C"
HDIB WINAPI imgXbmLoad(LPCTSTR pszFile)
{
	char buffer[MaxTextExtent], name[MaxTextExtent];
	register int x, y;
	register RGBA* q;
	register unsigned char *p;
	register long packets;
	short int hex_digits[256];
	unsigned char bit, *data;
	unsigned int byte, bytes_per_line, padding, value, version;
	unsigned short index;

	FILE* fp = _tfopen(pszFile, _T("rt"));
	if (!fp)
		return NULL;

	// Read X bitmap header.
	int columns = 0, rows = 0;
	while (fgets(buffer, (int)ARRAYSIZE(buffer), fp)) {
		if (sscanf_s(buffer, "#define %s %u", name, (int)ARRAYSIZE(name), &columns) == 2)
			if (lstrlenA(name) >= 6 && !lstrcmpiA(name + lstrlenA(name) - 6, "_width"))
				break;
	}

	while (fgets(buffer, (int)ARRAYSIZE(buffer), fp)) {
		if (sscanf_s(buffer,"#define %s %u", name, (int)ARRAYSIZE(name), &rows) == 2)
			if (lstrlenA(name) >= 7 && !lstrcmpiA(name + lstrlenA(name) - 7, "_height"))
				break;
	}

	// Scan until hex digits.
	version = 11;
	while (fgets(buffer, (int)ARRAYSIZE(buffer), fp)) {
		if (sscanf_s(buffer, "static short %s = {", name, (int)ARRAYSIZE(name)) == 1)
			version = 10;
		else if (sscanf_s(buffer, "static unsigned char %s = {", name, (int)ARRAYSIZE(name)) == 1)
			version = 11;
		else if (sscanf_s(buffer, "static char %s = {", name, (int)ARRAYSIZE(name)) == 1)
			version = 11;
		else
			continue;

		p = (unsigned char*)StrRChrA(name, NULL, '_');
		if (!p)
			p = (unsigned char*)name;
		else
			p++;
		if (!lstrcmpiA("bits[]", (char*)p))
			break;
	}

	if (columns == 0 || rows == 0 || feof(fp)) {
		fclose(fp);
		return NULL;
	}

	// Initialize image structure.
	mif image;
	image.create(columns, rows, 1);
	image.m_map[0].red = 0;
	image.m_map[0].green = 0;
	image.m_map[0].blue = 0;
	image.m_map[1].red = 255;
	image.m_map[1].green = 255;
	image.m_map[1].blue = 255;

	packets = 0;
	padding = 0;
	if ((columns % 16) && ((columns % 16) < 9) && (version == 10))
		padding = 1;
	bytes_per_line = (columns + 7) / 8 + padding;
	data = (unsigned char*)malloc(bytes_per_line * rows * sizeof(unsigned char*));
	if (!data) {
		fclose(fp);
		return NULL;
	}

	// Initialize hex values.
	hex_digits['0']=0;
	hex_digits['1']=1;
	hex_digits['2']=2;
	hex_digits['3']=3;
	hex_digits['4']=4;
	hex_digits['5']=5;
	hex_digits['6']=6;
	hex_digits['7']=7;
	hex_digits['8']=8;
	hex_digits['9']=9;
	hex_digits['A']=10;
	hex_digits['B']=11;
	hex_digits['C']=12;
	hex_digits['D']=13;
	hex_digits['E']=14;
	hex_digits['F']=15;
	hex_digits['a']=10;
	hex_digits['b']=11;
	hex_digits['c']=12;
	hex_digits['d']=13;
	hex_digits['e']=14;
	hex_digits['f']=15;
	hex_digits['x']=0;
	hex_digits[' ']=(-1);
	hex_digits[',']=(-1);
	hex_digits['}']=(-1);
	hex_digits['\n']=(-1);
	hex_digits['\t']=(-1);

	// Read hex image data.
	p = data;
	if (version == 10) {
		for (x=0; x < (int)(bytes_per_line * rows); (x += 2)) {
			value = XBMInteger(fp, hex_digits);
			*p++ = value;
			if (!padding || ((x+2) % bytes_per_line))
				*p++ = value >> 8;
		}
	}
	else {
		for (x=0; x < (int) (bytes_per_line * rows); x++) {
			value = XBMInteger(fp, hex_digits);
			*p++=value;
		}
	}
	fclose(fp);

	// Convert X bitmap image to runlength-encoded packets.
	byte = 0;
	p = data;
	q = image.m_pixels;

	for (y = 0; y < rows; y++) {
		bit = 0;
		for (x = 0; x < columns; x++) {
			if (bit == 0)
				byte = *p++;
			index = byte & 0x01 ? 0 : 1;
			if (packets != 0)
				q++;
			packets++;
			q->alpha = (BYTE)index;
			bit++;
			byte >>= 1;
			if (bit == 8)
				bit = 0;
		}
	}
	free(data);

	return image.createdib();
}
