////////////////////////////////////////////////////////////////////////
// kodak.cpp

#include "stdafx.h"
#include "exifparse.h"

#ifdef SUPPORT_KODAK

bool rawmap::kodak_yuv_decode()
{
	BYTE c, blen[384];
	unsigned len, bits=0;
	INT64 bitbuf=0;
	int i, li=0, si, diff, six[6], y[4], cb=0, cr=0, rgb[3];

	seek(m_thumb_offset, SEEK_SET);
	m_thumb_width = (m_thumb_width + 1) & -2;

//
// 未完成 USHORT rgb -> BYTE rgb で保存しなきゃ
//

	CTypeAlloc<USHORT> out(m_thumb_width * 6);
	write_bmpfileheader(m_thumb_width, m_thumb_height);
	for (int row = 0; row < m_thumb_height; row += 2) {
		for (int col = 0; col < m_thumb_width; col += 2) {
			if ((col & 127) == 0) {
				len = (m_thumb_width - col + 1) * 3 & -4;
				if (len > 384) len = 384;
				for (i = 0; i < len; ) {
					c = getc();
					blen[i++] = c & 15;
					blen[i++] = c >> 4;
				}
				li = bitbuf = bits = y[1] = y[3] = cb = cr = 0;
			}
			for (si = 0; si < 6; si++) {
				len = blen[li++];
				if (bits < len) {
					for (i=0; i < 32; i+=8)
						bitbuf += (INT64)getc() << (bits + (i ^ 8));
					bits += 32;
				}
				diff = bitbuf & (0xffff >> (16-len));
				bitbuf >>= len;
				bits -= len;
				if ((diff & (1 << (len-1))) == 0)
					diff -= (1 << len) - 1;
				six[si] = diff;
			}
			y[0] = six[0] + y[1];
			y[1] = six[1] + y[0];
			y[2] = six[2] + y[3];
			y[3] = six[3] + y[2];
			cb  += six[4];
			cr  += six[5];
			for (i = 0; i < 4; i++) {
				USHORT* op = out + ((i >> 1) * m_thumb_width + col + (i & 1)) * 3;
				rgb[0] = y[i] + 1.40200 / 2 * cr;
				rgb[1] = y[i] - 0.34414 / 2 * cb - 0.71414/2 * cr;
				rgb[2] = y[i] + 1.77200 / 2 * cb;
				for (c = 0; c < 3; c++) {
					if (rgb[c] > 0)
						op[c] = WORDFLIP((USHORT)rgb[c]);
				}
			}
		}
		m_pstrm->Write(out, sizeof(*out) * m_thumb_width * 6, NULL);
	}
	return true;
}

#endif	// SUPPORT_KODAK

