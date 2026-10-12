////////////////////////////////////////////////////////////////////////
// foveon.cpp

#include "stdafx.h"
#include "exifparse.h"

////////////////////////////////////////////////////////////////////////
//

#pragma pack(1)

typedef struct _X3FHEADER {
	DWORD filetype_identifer;		// "FOVb"
	DWORD format_version;			// Version of the file. Should be 2.1 or 2.2 for shipments to date.
	BYTE unique_identifer[16];		//
	DWORD mark_bits;				// Can be used to denote that images are marked into one or more subsets.
									// File interface functions allow setting these bits and searching for files based on
									// these bits. This feature will not be usable on writeonce media.
	DWORD image_rows;				// Image rows Width of unrotated image in columns.
									// This is the size output the user expects from this image,
									// not the size of the raw image data. Not necessarily
									// equal to the width of any image entry in the file;
									// this supports images where the raw data has rectangular pixels.
	DWORD image_columns;			// Image columns Height of unrotated image in rows.
									// This is the size output the user expects from this image,
									// not the size of the raw image data. Not necessarily equal
									// to the width of any image entry in the file; this
									// supports images where the raw data has rectangular pixels.
	DWORD rotation;					// Rotation Image rotation in degrees clockwise from normal camera orientation.
									// Valid values are 0, 90, 180, 270.
} X3FHEADER;

typedef struct _X3FEXTHEADER {
	char white_balance[32];			// White balance label string Contains an ASCIIZ string label of the
									// current white balance setting for this image.
	BYTE data_types[32];			// Ext. data types Contains 32 8-bit values indicating the types of the following extended data.
	DWORD extended_data[32];		// Extended data. Contains 32 32-bit values of extended data.
} X3FEXTHEADER;

typedef struct _tagSECp {
	DWORD id;						// "SECp"
	DWORD version;					// Property list format version Should be 2.0 for now
	DWORD entries;					// # of property entries
	DWORD chartype;					// Character format for all entries in this table. 0 = CHAR16 Unicode.
	DWORD reserved;					//
	DWORD length;					// Total length of name/value data in characters.
} _SECp;

typedef struct _tagSECi {
	DWORD id;						// "SECi"
	DWORD version;					//  2.0
	DWORD type;						//  2 = for preview
	DWORD format;					//  3 = uncompressed 24RGB,
									// 11 = Hufman-encodedDPCM RGB,
									// 18 = Jpeg compressed 8/8/8 RGB
	DWORD columns;					// width pixel
	DWORD rows;						// height pixel
	DWORD row_size;					// Will always be a multiple of 4 (32-bit aligned).
									// zero here means Huffman data.
} _SECi;

#pragma pack()

////////////////////////////////////////////////////////////////////////
//


typedef struct decode {
	struct decode* branch[2];
	int leaf;
} decode_t;
decode_t* free_decode;

static void foveon_tree(unsigned huff[1024], unsigned code)
{
	int i, len;
	decode_t* cur = free_decode++;
	if (code) {
		for (i = 0; i < 1024; i++)
			if (huff[i] == code) {
				cur->leaf = i;
				return;
			}
	}
	if ((len = code >> 27) > 26) return;
	code = (len+1) << 27 | (code & 0x3ffffff) << 1;

	cur->branch[0] = free_decode;
	foveon_tree(huff, code);
	cur->branch[1] = free_decode;
	foveon_tree(huff, code + 1);
}

bool rawmap::foveon_thumbs(_imageinfo_t* pi)
{
	if (!m_pstrm || !pi) return false;

	seek(pi->thumb_offset, SEEK_SET);
	if (pi->compression == 18) {
		// IMAGTAGの直後からJPEGデータが始まる
		// jpegデータをそのままストリームに書き出す
		long end = seek(0, SEEK_END);
		seek(pi->thumb_offset, SEEK_SET);
		if (data_streamout(pi->thumb_offset, end - pi->thumb_offset))
			m_imagewrite_completed = true;
		return m_imagewrite_completed;
	}
	else {
		m_thumb_width = pi->width;
		m_thumb_height = pi->height;

		write_bmpfileheader(m_thumb_width, m_thumb_height);

		// --------------------------------------------------------------
		// ※注意※ ここでは pi->bytecount は 1行あたりのバイト数を指す
		// --------------------------------------------------------------
		long widthbytes = pi->bytecount;

		int x, y;
		BYTE* data;
		if (pi->compression == 3) {
			// uncompressed 24RGB
			data = new BYTE[widthbytes * pi->height];
			read(data, widthbytes * pi->height);
			for (y = 0; y < m_thumb_height; y++) {
				BYTE* p = data + widthbytes * (m_thumb_height - y - 1);
				BYTE* q = p;
				for (x = 0; x < m_thumb_width; x++) {
					BYTE ch = q[2];
					q[2] = q[0];
					q[0] = ch;
					q += 3;
				}
				m_pstrm->Write(p, widthbytes, NULL);
			}
			delete [] data;
		}
		else if (pi->compression == 11) {
			// Hufman-encodedDPCM RGB
			widthbytes = WIDTHBYTES(m_thumb_width * 24);

			unsigned* huff = new unsigned[1024];
			int i;
			for (i = 0; i < 256; i++)
				huff[i] = fget4();

			decode_t* first_decode = new decode_t[640];
			memset(first_decode, 0, sizeof(decode_t) * 640);
			free_decode = first_decode;
			foveon_tree(huff, 0);

			int datasize = widthbytes * m_thumb_height;
			data = new BYTE[datasize];
			memset(data, 0, datasize);

			decode_t* dindex;
			unsigned bitbuf = 0;
			int bit = -1;
			for (y = 0; y < m_thumb_height; y++) {
				short pred[3];
				memset(pred, 0, sizeof pred);
				if (!bit) fget4();

				BYTE* p = data + widthbytes * (m_thumb_height - y - 1);
				bit = 0;
				for (x = 0; x < m_thumb_width; x++) {
					for (int c = 0; c < 3; c++) {
						for (dindex = first_decode; dindex->branch[0]; ) {
							if ((bit = (bit-1) & 31) == 31)
								for (i=0; i < 4; i++)
									bitbuf = (bitbuf << 8) + getc();
							dindex = dindex->branch[bitbuf >> bit & 1];
						}
						pred[c] += dindex->leaf;
					}
					*p++ = (BYTE)pred[2];
					*p++ = (BYTE)pred[1];
					*p++ = (BYTE)pred[0];
				}
			}

			m_pstrm->Write(data, datasize, NULL);
			delete [] data;
			delete [] first_decode;
			delete [] huff;
		}
	}
	return true;
}

/* --- sample ---------
  VERSION_BF: 0.8.7.2904 Release
      AEMODE: 8
      AFMODE: AF-S
      AFAREA: CENTER_V
     AP_DESC: 5.6
    APERTURE: 5.60000
     BRACKET:
       BURST: 0
    CAMMANUF: SIGMA
    CAMMODEL: SIGMA DP1
     CAMNAME:
   CAMSERIAL: 1001735
       DRIVE: SINGLE
     EXPCOMP: +0.00
      EXPNET: +0.00
     EXPTIME: 800
    FIRMVERS: 1.00
       FLASH: OFF
     FLENGTH: 16.6
    FLEQ35MM: 28
       FOCUS: AF
IMAGEBOARDID:
  IMAGERTEMP: 20
         ISO: 100
  LENSARANGE: 45 to 2.8
  LENSFRANGE: 16.6
   LENSMODEL: 10
       PMODE: A
  RESOLUTION: HI
    SENSORID:
     SH_DESC: 1/1250
     SHUTTER: 0.00080
  EVAL_STATE: POST-EXPOSURE
        TIME: 1204618554
     WB_DESC: Auto
----------------- */

bool rawmap::foveon_info(long start, long /*datasize*/)
{
	register sigma_t* sigma = &m_exif.sigma;

	seek(start, SEEK_SET);
	_SECp secp;
	if (read(&secp, sizeof(secp)) != sizeof(secp) || secp.id != 0x70434553)			// "SECp" ?
		return false;

	m_exif.exist_makenote = true;

	int i;
	WORD ch;
	char name[128], value[128], *p;
	long tableptr = tell() + secp.entries * sizeof(DWORD) * 2;
	long entries = secp.entries;
	while (entries-- > 0) {
		if (eof()) return false;

		long offset_name = fget4();				// name offset
		long offset_value = fget4();			// value offset
		long save = tell();						// current position

		// name 取得
		seek(tableptr + offset_name * sizeof(WORD), SEEK_SET);
		i = 0;
		while (!eof() && (ch = fget2()) != 0) name[i++] = (char)(ch & 0xff);
		name[i] = 0;

		// value 取得
		seek(tableptr + offset_value * sizeof(WORD), SEEK_SET);
		i = 0;
		while (!eof() && (ch = fget2()) != 0) value[i++] = (char)(ch & 0xff);
		value[i] = 0;

// dbg.printfA("%s: %s\n", name, value);

		if		(!StrCmpA(name, "CAMMANUF"))		lstrcpynA(m_exif.maker, value, ARRAYSIZE(m_exif.maker));
		else if (!StrCmpA(name, "CAMMODEL"))		lstrcpynA(m_exif.model, value, ARRAYSIZE(m_exif.model));
		else if (!StrCmpA(name, "ISO"))				m_exif.ISOEquiv = atoi(value);
		else if (!StrCmpA(name, "AP_DESC"))			m_exif.FNumber = m_exif.Aperture = (float)atof(value);
		else if (!StrCmpA(name, "SH_DESC")) {
			p = StrChrA(value, '/');
			if (p) sigma->ExposureTime = (float)(1.0 / atof(++p));
			else   sigma->ExposureTime = (float)atof(value);
			m_exif.ExposureTime = sigma->ExposureTime;
		}
		else if (!StrCmpA(name, "EXPCOMP")) {
			sigma->ExposureCompensation = (float)atof(value);
			m_exif.ExposureBias = sigma->ExposureCompensation;
		}
		else if (!StrCmpA(name, "FLENGTH")) {
			lstrcpynA(sigma->FocalLength, value, ARRAYSIZE(sigma->FocalLength));
			m_exif.FocalLength = (float)atof(value);
		}
		else if (!StrCmpA(name, "FLEQ35MM")) {
			lstrcpynA(sigma->FocalLength35, value, ARRAYSIZE(sigma->FocalLength35));
			m_exif.FocalLength35 = (float)atof(value);
		}
		else if (!StrCmpA(name, "FLASH")) {
			lstrcpynA(sigma->Flash, value, ARRAYSIZE(sigma->Flash));
			m_exif.Flash = !memcmp(value, "OFF", 3) ? 0 : 1;
		}
		else if (!StrCmpA(name, "WB_DESC")) {
			lstrcpynA(sigma->WhiteBalance, value, ARRAYSIZE(sigma->WhiteBalance));
			if		(!StrCmpA(value, "Auto"))		m_exif.LightSource = LS_AUTO;
			else if (!StrCmpA(value, "Sunlight"))	m_exif.LightSource = LS_SUNLIGHT;
			else									m_exif.LightSource = LS_OTHER;
		}
		else if (!StrCmpA(name, "AEMODE")) {
			lstrcpynA(sigma->MeteringMode, value, ARRAYSIZE(sigma->MeteringMode));
			switch (*value) {
			case '8': m_exif.MeteringMode = MTM_PATTERN; break;
			case 'C': m_exif.MeteringMode = MTM_CENTERWAIT; break;
			case 'A': m_exif.MeteringMode = MTM_AVERAGE; break;
			default:  m_exif.MeteringMode = MTM_UNKNOWN; break;
			}
		}
		else if (!StrCmpA(name, "DRIVE"))		lstrcpynA(sigma->DriveMode, value, ARRAYSIZE(sigma->DriveMode));
		else if (!StrCmpA(name, "AFMODE"))		lstrcpynA(sigma->AFMode, value, ARRAYSIZE(sigma->AFMode));
		else if (!StrCmpA(name, "PMODE")) {
			lstrcpynA(sigma->ShootMode, value, ARRAYSIZE(sigma->ShootMode));
			switch (*value) {
			case 'P': m_exif.ExposureProgram = EXPP_NORMAL; break;
			case 'A': m_exif.ExposureProgram = EXPP_AV; break;
			case 'S': m_exif.ExposureProgram = EXPP_TV; break;
			case 'M': m_exif.ExposureProgram = EXPP_MANUAL; break;
			default:  m_exif.ExposureProgram = EXPP_UNKNOWN; break;
			}
		}
		else if (!StrCmpA(name, "BRACKET"))		lstrcpynA(sigma->AutoBracket, value, ARRAYSIZE(sigma->AutoBracket));
		else if (!StrCmpA(name, "RESOLUTION"))	lstrcpynA(sigma->Resolution, value, ARRAYSIZE(sigma->Resolution));
		else if (!StrCmpA(name, "TIME")) {
			time_t t = (time_t)atol(value);
			struct tm* lt = gmtime(&t);
			if (lt) {
				wsprintfA(m_exif.Date, "%04d/%02d/%02d %02d:%02d:%02d",
					lt->tm_year + 1900, lt->tm_mon + 1, lt->tm_mday, lt->tm_hour, lt->tm_min, lt->tm_sec);
			}
		}
		else if (!StrCmpA(name, "LENSFRANGE"))	lstrcpynA(sigma->FocalLengthRange, value, ARRAYSIZE(sigma->FocalLengthRange));
		else if (!StrCmpA(name, "LENSARANGE"))	lstrcpynA(sigma->LensApertureRange, value, ARRAYSIZE(sigma->LensApertureRange));
		else if (!StrCmpA(name, "LENSMODEL"))	lstrcpynA(sigma->LensModel, value, ARRAYSIZE(sigma->LensModel));
		else if (!StrCmpA(name, "CAMSERIAL"))	lstrcpynA(sigma->Serial, value, ARRAYSIZE(sigma->Serial));
		else if (!StrCmpA(name, "BURST"))		lstrcpynA(sigma->Burst, value, ARRAYSIZE(sigma->Burst));
		else if (!StrCmpA(name, "ROTATION"))	lstrcpynA(sigma->Rotation, value, ARRAYSIZE(sigma->Rotation));
		else if (!StrCmpA(name, "AFAREA"))		lstrcpynA(sigma->AFArea, value, ARRAYSIZE(sigma->AFArea));
		else if (!StrCmpA(name, "AFINFOCUS"))	lstrcpynA(sigma->AFInFocus, value, ARRAYSIZE(sigma->AFInFocus));
		else if (!StrCmpA(name, "COLORSPACE"))	lstrcpynA(sigma->ColorSpace, value, ARRAYSIZE(sigma->ColorSpace));
		else if (!StrCmpA(name, "FOCUS"))		lstrcpynA(sigma->FocusMode, value, ARRAYSIZE(sigma->FocusMode));

		seek(save, SEEK_SET);
	}


	return true;
}

bool rawmap::foveon_decode()
{
	//
	// http://www.x3f.info/technotes/FileDocs/X3F_Format.pdf
	//

	// ヘッダを読む
	seek(0, SEEK_SET);
	X3FHEADER xhd;
	if (read(&xhd, sizeof(xhd)) != sizeof(xhd) || xhd.filetype_identifer != 0x62564F46)			// "FOVb" ?
		return false;

	m_exif.MakerID = MAKER_SIGMA;
	m_order = 0x4949;							// Little-endian

	// 初期化
	register sigma_t* sigma = &m_exif.sigma;
	memset(sigma, 0, sizeof(sigma_t));
	sigma->FNumber = FLT_MAX;
	sigma->ExposureTime = FLT_MAX;
	sigma->ExposureCompensation = FLT_MAX;
	sigma->FlashExposureComp = FLT_MAX;
	sigma->X3FVersion = xhd.format_version;

	switch (xhd.rotation) {						// Ratation
	case 90:	m_exif.Orientation = ORIENTATION_ROTATERIGHT; break;
	case 180:	m_exif.Orientation = ORIENTATION_ROTATE180; break;
	case 270:	m_exif.Orientation = ORIENTATION_ROTATELEFT; break;
	default:	m_exif.Orientation = ORIENTATION_NONE; break;
	}
	m_exif.ImageHeight = xhd.image_rows;		// height
	m_exif.ImageWidth = xhd.image_columns;		// width;

	// ver2.1以降はExtendedDataがある、必要なら読む
	int i;
	if (xhd.format_version >= 0x20001) {
		X3FEXTHEADER ehd;
		if (read(&ehd, sizeof(ehd)) != sizeof(ehd))
			return false;

		float f;
		char ssz[16];
		for (i = 0; i < ARRAYSIZE(ehd.data_types); i++) {
			f = ((float*)ehd.extended_data)[i];
			sprintf(ssz, "%s%.1f", (f < 0) ? "-" : "+", fabsf(f));
			switch (ehd.data_types[i]) {
			case 1:							// ExposureAdjust
				lstrcpyA(sigma->ExpCompensation, ssz);
				break;
			case 2:							// Contrast
				lstrcpyA(sigma->Contrast, ssz);
				break;
			case 3:							// Shadow
				lstrcpyA(sigma->Shadow, ssz);
				break;
			case 4:							// Hilight
				lstrcpyA(sigma->Hilight, ssz);
				break;
			case 5:							// Saturation
				lstrcpyA(sigma->Saturation, ssz);
				break;
			case 6:							// Sharpness
				lstrcpyA(sigma->Sharpness, ssz);
				break;
			case 7:							// RedAdjust
			case 8:							// GreenAdjust
			case 9:							// BlueAdjust
				if (sigma->ColorAdjust[0]) lstrcatA(sigma->ColorAdjust, ", ");
				lstrcatA(sigma->ColorAdjust, ssz);
				break;
			case 10:						// X3FillLight
				lstrcpyA(sigma->X3FillLight, ssz);
				break;
			default:
				break;
			}
		}
	}

	// ファイルエンドの4バイトがディレクトリエントリ "SECd" へのオフセットを指しているのでそれを取得
	seek(-4, SEEK_END);
	long secd = fget4();
	seek(secd, SEEK_SET);
	if (fget4() != 0x64434553)
		return false;

	CSimpleArray<_imageinfo_t*> arr;

	_imageinfo_t* pimage;
	fget4();								// skip section version
	long entries = fget4();					// # of directory entries
	while (entries-- > 0) {
		if (eof()) return false;

		long dirstart = fget4();			// directory start offset
		long dirsize = fget4();				// directory data size
		long dirtype = fget4();				// directory data type ("PROP", "CAMF", "IMA2", ... )
		long save = tell();

		switch (dirtype) {
		case 0x504F5250:					// "PROP" PropertyList Section
			if (!foveon_info(dirstart, dirsize))
				return false;
			break;
		case 0x47414d49:					// "IMAG" or
		case 0x32414d49:					// "IMA2"
		{
			// サムネイル不要ならば何もしない
			if (!m_pstrm) break;

			// サムネイルイメージデータを抽出し、配列にしまっておく
			seek(dirstart, SEEK_SET);

			_SECi it;
			read(&it, sizeof(it));
			if (it.id == 0x69434553 &&		// "SECi"
				it.type == 2) {				// for preview

				switch (it.format) {
				case 3:						// uncompressed 24-bit 8/8/8 RGB
				case 11:					// Huffman-encoded DPCM 8/8/8 RGB
				case 18:					// JPEG-compressed 8/8/8 RGB
					pimage = new _imageinfo_t;
					pimage->compression = it.format;
					pimage->width = it.columns;
					pimage->height = it.rows;
					pimage->bytecount = it.row_size;
					pimage->thumb_offset = tell();
					arr.Add(pimage);
					break;
				default:
					break;
				}
			}
			break;
		}
		default:
			break;
		}
		seek(save, SEEK_SET);
	}

	bool done_thumbimage = true;
	int cnt = arr.GetSize();
	if (cnt > 0) {
		// サムネイルイメージを書き出す
		_imageinfo_t* pmin = NULL;
		_imageinfo_t* pmax = NULL;

		for (i = 0; i < cnt; i++) {
			pimage = arr[i];
			if (pimage) {
				// サムネイルの面積で比較
				long area = pimage->width * pimage->height;
				if (area) {
					if (!pmin || (pmin->width * pmin->height > area))
						pmin = pimage;
					if (!pmax || (pmax->width * pmax->height < area))
						pmax = pimage;
				}
			}
		}

		pimage = m_largeimage ? pmax : pmin;
		if (pimage)
			done_thumbimage = foveon_thumbs(pimage);

		for (i = 0; i < cnt; i++) {
			pimage = arr[i];
			if (pimage) delete pimage;
		}
	}
	return done_thumbimage;
}
