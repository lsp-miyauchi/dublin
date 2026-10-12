////////////////////////////////////////////////////////////////////////
// rollei.cpp

#include "stdafx.h"
#include "exifparse.h"

#ifdef SUPPORT_ROLLEI

void rawmap::parse_rollei()
{
	char line[128], *val;

	m_exif.MakerID = MAKER_ROLLEI;
	seek(0, SEEK_SET);
	do {
		gets(line, 128);
		if ((val = strchr(line, '=')))
			*val++ = 0;
		else
			val = line + lstrlenA(line);
		if (!memcmp(line, "HDR", 3))	m_thumb_offset = atoi(val);
		if (!memcmp(line, "TX ", 3))	m_thumb_width = atoi(val);
		if (!memcmp(line, "TY ", 3))	m_thumb_height = atoi(val);
	} while (memcmp(line, "EOHD",4));
	lstrcpyA(m_exif.maker, "Rollei");
	lstrcpyA(m_exif.model, "d530flex");
	m_thumb_length = m_thumb_width * m_thumb_height * 2;
}

bool rawmap::rollei_decode()
{
	if (!m_pstrm) return true;		// サムネイル画像必要なし

	seek(m_thumb_offset, SEEK_SET);
	int widthbytes = WIDTHBYTES(m_thumb_width * 24);

	CTypeAlloc<BYTE> data(widthbytes);
	write_bmpfileheader(m_thumb_width, m_thumb_height);
	for (int row = 0; row < m_thumb_height; row++) {
		BYTE* p = data;
		for (int col = 0; col < m_thumb_width; col++) {
			USHORT n;
			read(&n, 2);
			n = WORDFLIP(n);
			*p++ = (BYTE)(n << 3);
			*p++ = (BYTE)(n >> 5  << 2);
			*p++ = (BYTE)(n >> 11 << 3);
		}
		m_pstrm->Write(data, widthbytes, NULL);
	}
	m_imagewrite_completed = true;
	return true;
}

#endif	// SUPPORT_ROLLEI
