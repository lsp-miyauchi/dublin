/* ---------------------------------------------------------------------
	Raw Photo Parser
	 Copyright 2004 by Dave Coffin, dcoffin a cybercom o net

	This program extracts thumbnail images (preferably JPEGs)
	from any raw digital camera formats that have them, and
	shows table contents.

	http://www.cybercom.net/~dcoffin/dcraw/

	$Revision: 1.12 $
	$Date: 2004/02/20 04:22:08 $

	.CRW		Canon RAW
	.CR2		Canon RAW
	.NEF		Nikon RAW
	.MRW		Minolta RAW
	.PEF		Pentax RAW
	.RAF		Fujifilm RAW
	.X3F		Sigma RAW
	.DNG		Adobe DigitalNegative
	.K25		Kodak DC25

--------------------------------------------------------------------- */

#include "stdafx.h"
#include "exifparse.h"
#include "exif_ui.h"

void equalize(int width, int height, BYTE* rgbptr);

#define SWAP(a,b)		 { a ^= b; a ^= (b ^= a); }

////////////////////////////////////////////////////////////////////////
//

static TCHAR* TagType[] = {
	_T("0:?????"),
	_T("1:BYTE "),
	_T("2:ASCII"),
	_T("3:USHRT"),
	_T("4:ULONG"),
	_T("5:RATNL"),
	_T("6:SBYTE"),
	_T("7:UNDEF"),
	_T("8:SSHRT"),
	_T("9:SLONG"),
	_T("A:SRTNL"),
	_T("B:FLOAT"),
	_T("C:DOUBL"),
};

static int TagTypeSize[] = {
	1,		// unknown
	1,		// BYTE
	1,		// ASCII
	2,		// USHRT
	4,		// ULONG
	8,		// RATNL
	1,		// SBYTE
	1,		// UNDEF
	2,		// SSHRT
	4,		// SLONG
	8,		// SRTNL
	4,		// FLOAT
	8,		// DOUBL
};

inline int CalcTagDataSize(int tag, int count) { return TagTypeSize[tag] * count; }


////////////////////////////////////////////////////////////////////////
//

void rawmap::rawinit()
{
	m_headlen = 0;
	m_head[0] = 0;
	m_diborder = false;
	m_thumb_offset = m_thumb_length = m_thumb_layers = m_tiffbase = 0;
	m_thumb_width = m_thumb_height = m_strip_offset = 0;
	m_thumb_orientation = 0;
	m_largeimage = false;
	m_imagewrite_completed = false;
	m_this_is_jpeg = false;

	// 撮影情報に関する項目
	memset(&m_exif, 0, sizeof(m_exif));
	m_exif.ImageWidth = INT_MAX;
	m_exif.ImageHeight = INT_MAX;
	m_exif.Orientation = INT_MAX;
	m_exif.ISOEquiv = INT_MAX;
	m_exif.ExposureProgram = INT_MAX;
	m_exif.LightSource = INT_MAX;
	m_exif.MeteringMode = INT_MAX;
	m_exif.Flash = INT_MAX;
	m_exif.ColorSpace = INT_MAX;
	m_exif.ExposureMode = INT_MAX;
	m_exif.WhiteBalance = INT_MAX;
	m_exif.SceneCaptureType = INT_MAX;
	m_exif.GainControl = INT_MAX;
	m_exif.Contrast = INT_MAX;
	m_exif.Saturation = INT_MAX;
	m_exif.Sharpness = INT_MAX;
	m_exif.DistanceRange = INT_MAX;

	m_exif.ShutterSpeed = FLT_MAX;
	m_exif.ExposureTime = FLT_MAX;
	m_exif.FNumber = FLT_MAX;
	m_exif.FocalLength = FLT_MAX;
	m_exif.Aperture = FLT_MAX;
	m_exif.MaxAperture = FLT_MAX;
	m_exif.ExposureBias = FLT_MAX;
	m_exif.SubjectDistance = FLT_MAX;
	m_exif.FocalPlaneXResolution = FLT_MAX;
	m_exif.FocalPlaneYResolution = FLT_MAX;
	m_exif.FocalPlaneResolutionUnits = FLT_MAX;
	m_exif.BrightnessValue = FLT_MAX;
	m_exif.FocalLength35 = FLT_MAX;
	m_exif.DigitalZoom = FLT_MAX;
	m_exif.GammaValue = FLT_MAX;

	// gpsに関する項目
	gps_t& gps = m_exif.gps;
	memset(&gps, 0, sizeof(gps));
	gps.exist_gps = false;
	gps.GPSLatitude[0] = FLT_MAX;
	gps.GPSLongitude[0] = FLT_MAX;
	gps.GPSAlititude = FLT_MAX;
	gps.GPSTimeStamp[0] = FLT_MAX;
}

double rawmap::get_tagvalue(int fmt)
{
	double val;
	switch (fmt) {
	case FMT_BYTE:		return (double)(unsigned char)getc();
	case FMT_USHORT:	return (double)fget2();
	case FMT_ULONG:		return (double)(unsigned long)fget4();
	case FMT_SBYTE:		return (double)(signed char)getc();
	case FMT_SSHORT:	return (double)fgetshort();
	case FMT_SLONG:		return (double)(signed long)fget4();
	case FMT_URATIONAL:
	{
		unsigned long num = (unsigned long)fget4();
		unsigned long den = (unsigned long)fget4();
		if (num == 0 || den == 0)
			val = 0;
		else
			val = (double)num / (double)den;
		return val;
	}
	case FMT_SRATIONAL:
	{
		int num = (signed long)fget4();
		int den = (signed long)fget4();
		if (num == 0 || den == 0)
			val = 0;
		else
			val = (double)num / (double)den;
		return val;
	}
	case FMT_FLOAT:
	{
		BYTE cb[4];
		read(cb, 4);
		return (double)(*(float*)cb);
	}
	case FMT_DOUBLE:
	{
		BYTE cb[sizeof(double)];
		read(cb, sizeof(double));
		return *(double*)cb;
	}
	}
	return (double)0;
}

void rawmap::read_ascii(void* buffer, int count, int countMax)
{
	register BYTE* cb = (BYTE*)buffer;
	count = min(count, countMax - 1);
	read(cb, count);
	cb[count] = 0;

	while (*cb && count > 0) {
		if (*cb != 0x20) return;

		cb++;
		count--;
	}
	*((BYTE*)buffer) = 0;
}

/* --------------------------------
static const TCHAR* TagName(int tag)
{
	static TCHAR tagunk[64];
	switch (tag) {
	case TAG_NEW_SUBFILE_TYPE:		return _T("00FE:TAG_NEW_SUBFILE_TYPE");
	case TAG_IMAGE_WIDTH:			return _T("0100:TAG_IMAGE_WIDTH");
	case TAG_IMAGE_LENGTH:			return _T("0101:TAG_IMAGE_LENGTH");
	case TAG_BITS_SAMPLE:			return _T("0102:TAG_BITS_SAMPLE");
	case TAG_COMPRESSION:			return _T("0103:TAG_COMPRESSION");
	case TAG_PHOTOMETRIC_INTERPRET:	return _T("0106:TAG_PHOTOMETRIC_INTERPRET");
	case TAG_FILL_ORDER:			return _T("010A:TAG_FILL_ORDER");
	case TAG_DOCUMENT_NAME:			return _T("010D:TAG_DOCUMENT_NAME");
	case TAG_IMAGE_DESCRIPTION:		return _T("010E:TAG_IMAGE_DESCRIPTION");
	case TAG_MAKE:					return _T("010F:TAG_MAKE");
	case TAG_MODEL:					return _T("0110:TAG_MODEL");
	case TAG_STRIP_OFFSET:			return _T("0111:TAG_STRIP_OFFSET");
	case TAG_ORIENTATION:			return _T("0112:TAG_ORIENTATION");
	case TAG_SAMPLE_PIXEL:			return _T("0115:TAG_SAMPLE_PIXEL");
	case TAG_ROWS_STRIP:			return _T("0116:TAG_ROWS_STRIP");
	case TAG_STRIP_BYTE_COUNT:		return _T("0117:TAG_STRIP_BYTE_COUNT");
	case TAG_XRESOLUTION:			return _T("011A:TAG_XRESOLUTION");
	case TAG_YRESOLUTION:			return _T("011B:TAG_YRESOLUTION");
	case TAG_PLANER_CONFIG:			return _T("011C:TAG_PLANER_CONFIG");
	case TAG_RESOLUTION_UNIT:		return _T("0128:TAG_RESOLUTION_UNIT");
	case TAG_TRANSFER_FUNCTION:		return _T("012D:TAG_TRANSFER_FUNCTION");
	case TAG_SOFTWARE:				return _T("0131:TAG_SOFTWARE");
	case TAG_DATETIME:				return _T("0132:TAG_DATETIME");
	case TAG_ARTIST:				return _T("013B:TAG_ARTIST");
	case TAG_WHITE_POINT:			return _T("013E:TAG_WHITE_POINT");
	case TAG_PRIMARY_CHROMA:		return _T("013F:TAG_PRIMARY_CHROMA");
	case TAG_HALFTONEHINTS:			return _T("0141:TAG_HALFTONEHINTS");
	case TAG_TILE_WIDTH:			return _T("0142:TAG_TILE_WIDTH");
	case TAG_TILE_LENGTH:			return _T("0143:TAG_TILE_LENGTH");
	case TAG_TILE_OFFSET:			return _T("0144:TAG_TILE_OFFSET");
	case TAG_TILE_BYTECOUNTS:		return _T("0145:TAG_TILE_BYTECOUNTS");
	case TAG_SUBIFD:				return _T("014A:TAG_SUBIFD");
	case TAG_INKSET:				return _T("014C:TAG_INKSET");
	case TAG_INKNAMES:				return _T("014D:TAG_INKNAMES");
	case TAG_NUMBEROFINKS:			return _T("014E:TAG_NUMBEROFINKS");
	case TAG_DOTRANGE:				return _T("0150:TAG_DOTRANGE");
	case TAG_TARGETPRINTER:			return _T("0151:TAG_TARGETPRINTER");
	case TAG_EXTRASAMPLES:			return _T("0152:TAG_EXTRASAMPLES");
	case TAG_SAMPLEFORMAT:			return _T("0153:TAG_SAMPLEFORMAT");
	case TAG_SMIN_SAMPLEVALUE:		return _T("0154:TAG_SMIN_SAMPLEVALUE");
	case TAG_SMAX_SAMPLEVALUE:		return _T("0155:TAG_SMAX_SAMPLEVALUE");
	case TAG_TRANSFER_RANGE:		return _T("0156:TAG_TRANSFER_RANGE");
	case TAG_JPEG_PROC:				return _T("0200:TAG_JPEG_PROC");
	case TAG_THUMBNAIL_OFFSET:		return _T("0201:TAG_THUMBNAIL_OFFSET");
	case TAG_THUMBNAIL_LENGTH:		return _T("0202:TAG_THUMBNAIL_LENGTH");
	case TAG_YCBCR_COEFF:			return _T("0211:TAG_YCBCR_COEFF");
	case TAG_YCBCR_SUBSAMPLING:		return _T("0212:TAG_YCBCR_SUBSAMPLING");
	case TAG_YCBCR_POSITIONING:		return _T("0213:TAG_YCBCR_POSITIONING");
	case TAG_REF_BLACKWHITE:		return _T("0214:TAG_REF_BLACKWHITE");
	case TAG_XML_METADATA:			return _T("02BC:TAG_XML_METADATA");
	case TAG_ADOBE_RESOURCE:		return _T("8649:TAG_ADOBE_RESOURCE");
	case TAG_EXIF_OFFSET:			return _T("8769:TAG_EXIF_OFFSET");
	case TAG_INTERCOLOR_PROFILE:	return _T("8773:TAG_INTERCOLOR_PROFILE");
	case TAG_MODEL2:				return _T("827D:TAG_MODEL2");
	case TAG_CFA_REPEATPATTERN_DIM:	return _T("828D:TAG_CFA_REPEATPATTERN_DIM");
	case TAG_CFA_REPEATPATTERN:		return _T("828E:TAG_CFA_REPEATPATTERN");
	case TAG_BATTERY_LEVEL:			return _T("828F:TAG_BATTERY_LEVEL");
	case TAG_COPYRIGHT:				return _T("8298:TAG_COPYRIGHT");
	case TAG_EXPOSURE_TIME:			return _T("829A:TAG_EXPOSURE_TIME");
	case TAG_FNUMBER:				return _T("829D:TAG_FNUMBER");
	case TAG_EXPOSURE_PROGRAM:		return _T("8822:TAG_EXPOSURE_PROGRAM");
	case TAG_SPECTRAL_SENSITIIVITY:	return _T("8824:TAG_SPECTRAL_SENSITIIVITY");
	case TAG_ISO_EQUIVALENT:		return _T("8827:TAG_ISO_EQUIVALENT");
	case TAG_OECF:					return _T("8828:TAG_OECF");
	case TAG_EXIF_VERSION:			return _T("9000:TAG_EXIF_VERSION");
	case TAG_DATETIME_ORIGINAL:		return _T("9003:TAG_DATETIME_ORIGINAL");
	case TAG_DATETIME_DIGITIZED:	return _T("9004:TAG_DATETIME_DIGITIZED");
	case TAG_COMPONENTS_CONFIG:		return _T("9101:TAG_COMPONENTS_CONFIG");
	case TAG_COMPRESSED_BPS:		return _T("9102:TAG_COMPRESSED_BPS");
	case TAG_SHUTTER_SPEED:			return _T("9201:TAG_SHUTTER_SPEED");
	case TAG_APERTURE:				return _T("9202:TAG_APERTURE");
	case TAG_BRIGHTNESS_VALUE:		return _T("9203:TAG_BRIGHTNESS_VALUE");
	case TAG_EXPOSURE_BIAS:			return _T("9204:TAG_EXPOSURE_BIAS");
	case TAG_MAX_APERTURE:			return _T("9205:TAG_MAX_APERTURE");
	case TAG_SUBJECT_DISTANCE:		return _T("9206:TAG_SUBJECT_DISTANCE");
	case TAG_METERING_MODE:			return _T("9207:TAG_METERING_MODE");
	case TAG_LIGHT_SOURCE:			return _T("9208:TAG_LIGHT_SOURCE");
	case TAG_FLASH:					return _T("9209:TAG_FLASH");
	case TAG_FOCAL_LENGTH:			return _T("920A:TAG_FOCAL_LENGTH");
	case 0x920B:					return _T("920B:TAG_FLASH_ENERGY");
	case 0x920C:					return _T("920C:SpatialFrequencyResponse");
	case 0x920E:					return _T("920E:TAG_FOCALPLANE_XRES");
	case 0x920F:					return _T("920F:TAG_FOCALPLANE_YRES");
	case 0x9210:					return _T("9210:TAG_FOCALPLANE_UNITS");
	case TAG_SUBJECT_AREA:			return _T("9214:TAG_SUBJECT_AREA");
	case 0x9215:					return _T("9215:TAG_EXPOSURE_INDEX");
	case 0x9217:					return _T("9217:TAG_SENSING_METHOD");
	case TAG_MAKERNOTE:				return _T("927C:TAG_MAKERNOTE");
	case TAG_USERCOMMENT:			return _T("9286:TAG_USERCOMMENT");
	case TAG_SUBSECTIME:			return _T("9290:TAG_SUBSECTIME");
	case TAG_SUBSECTIME_ORIGINAL:	return _T("9291:TAG_SUBSECTIME_ORIGINAL");
	case TAG_SUBSECTIME_DIGITIZED:	return _T("9292:TAG_SUBSECTIME_DIGITIZED");
	case TAG_FPX_VERSION:			return _T("A000:TAG_FPX_VERSION");
	case TAG_COLOR_SPACE:			return _T("A001:TAG_COLOR_SPACE");
	case TAG_EXIF_IMAGEWIDTH:		return _T("A002:TAG_EXIF_IMAGEWIDTH");
	case TAG_EXIF_IMAGELENGTH:		return _T("A003:TAG_EXIF_IMAGELENGTH");
	case TAG_INTEROP_OFFSET:		return _T("A005:TAG_INTEROP_OFFSET");
	case TAG_FLASH_ENERGY:			return _T("A20B:TAG_FLASH_ENERGY");
	case 0xA20C:					return _T("A20C:SpatialFrequencyResponse");
	case TAG_FOCALPLANE_XRES:		return _T("A20E:TAG_FOCALPLANE_XRES");
	case TAG_FOCALPLANE_YRES:		return _T("A20F:TAG_FOCALPLANE_YRES");
	case TAG_FOCALPLANE_UNITS:		return _T("A210:TAG_FOCALPLANE_UNITS");
	case TAG_SUBJECT_LOCATION:		return _T("A214:TAG_SUBJECT_LOCATION");
	case TAG_EXPOSURE_INDEX:		return _T("A215:TAG_EXPOSURE_INDEX");
	case TAG_SENSING_METHOD:		return _T("A217:TAG_SENSING_METHOD");
	case TAG_FILE_SOURCE:			return _T("A300:TAG_FILE_SOURCE");
	case TAG_SCENE_TYPE:			return _T("A301:TAG_SCENE_TYPE");
	case TAG_CFA_PATTERN:			return _T("A302:TAG_CFA_PATTERN");
	case TAG_EXPOSURE_MODE:			return _T("A402:TAG_EXPOSURE_MODE");
	case TAG_WHITE_BALANCE:			return _T("A403:TAG_WHITE_BALANCE");
	case TAG_DIGITAL_ZOOM:			return _T("A404:TAG_DIGITAL_ZOOM");
	case TAG_FOCALLENGTH_35:		return _T("A405:TAG_FOCALLENGTH_35");
	case TAG_SCENE_CAPTURE_TYPE:	return _T("A406:TAG_SCENE_CAPTURE_TYPE");
	case TAG_GAIN_CONTROL:			return _T("A407:TAG_GAIN_CONTROL");
	case TAG_CONTRAST:				return _T("A408:TAG_CONTRAST");
	case TAG_SATURATION:			return _T("A409:TAG_SATURATION");
	case TAG_SHARPNESS:				return _T("A40A:TAG_SATURATION");
	case TAG_DEVICESETTING:			return _T("A40B:TAG_DEVICESETTING");
	case TAG_DISTANCE_RANGE:		return _T("A40C:TAG_DISTANCE_RANGE");
	case TAG_IMAGE_UNIQUE_ID:		return _T("A420:TAG_IMAGE_UNIQUE_ID");
	case TAG_GAMMA:					return _T("A500:TAG_GAMMA");
	case TAG_DNG_VERSION:			return _T("50706:TAG_DNG_VERSION");
	case TAG_DNG_BACKWORDVERSION:	return _T("50707:TAG_DNG_BACKWORDVERSION");
	case TAG_DNG_CAMERAMODEL_UNIQ:	return _T("50708:TAG_DNG_CAMERAMODEL_UNIQ");
	case TAG_DNG_CAMERAMODEL_LOCAL:	return _T("50709:TAG_DNG_CAMERAMODEL_LOCAL");
	case TAG_DNG_CFA_PLANECOLOR:	return _T("50710:TAG_DNG_CFAPLANECOLOR");
	case TAG_DNG_CAMERA_SERIALNUM:	return _T("50735:TAG_DNG_CAMERA_SERIALNUM");
	case TAG_DNG_LENSINFO:			return _T("50736:TAG_DNG_LENSINFO");
	case TAG_DNG_PRIVATE_DATA:		return _T("50740:TAG_DNG_PRIVATE_DATA");
	case TAG_DNG_MAKERNOTE_SAFTY:	return _T("50741:TAG_DNG_MAKERNOTE_SAFTY");

	}
	if (tag >= 50706)
		wsprintf(tagunk, _T("%04X: DNG tag"), tag);
	else
		wsprintf(tagunk, _T("%04X: ** unknown **"), tag);
	return tagunk;
}
-------------- */

static void tagdump(rawmap* fm, int tag, int type, int count)
{
	dbg.printf(L"TAG=%04X %s cnt=%-5d pos=%p | ", tag, TagType[type], count, fm->tell());
	int i;
	switch (type) {
	case FMT_BYTE:
	case FMT_STRING:
		count = min(count, 32);
		for (i = 0; i < count; i++) { dbg.printf(L"%02X,", fm->getc()); }
		break;
	case FMT_USHORT:
		count = min(count, 10);
		for (i = 0; i < count; i++) { dbg.printf(L"%04X,", (USHORT)fm->fget2()); }
		break;
	case FMT_ULONG:
		count = min(count, 6);
		for (i = 0; i < count; i++) { dbg.printf(L"%08X,", (ULONG)fm->fget4()); }
		break;
	case FMT_URATIONAL:
		count = min(count, 6);
		for (i = 0; i < count; i++) { dbg.printf(L"%d/%d,", (ULONG)fm->fget4(), (ULONG)fm->fget4()); }
		break;
	case FMT_SBYTE:
		count = min(count, 24);
		for (i = 0; i < count; i++) { dbg.printf(L"%d,", (int)fm->getc()); }
		break;
	case FMT_SSHORT:
		count = min(count, 8);
		for (i = 0; i < count; i++) { dbg.printf(L"%d,", (short)fm->fget2()); }
		break;
	case FMT_SLONG:
		count = min(count, 8);
		for (i = 0; i < count; i++) { dbg.printf(L"%d,", (long)fm->fget4()); }
		break;
	case FMT_SRATIONAL:
		count = min(count, 6);
		for (i = 0; i < count; i++) { dbg.printf(L"%d/%d,", (long)fm->fget4(), (long)fm->fget4()); }
		break;
	case FMT_UNDEFINED:
		count = min(count, 24);
		for (i = 0; i < count; i++) { dbg.printf(L"%02X,", fm->getc()); }
		break;
	default:
		dbg.printf(L"------");
	}
	dbg.printf(L"\n");
}

void rawmap::write_bmpfileheader(int width, int height)
{
	if (!m_pstrm) return;

	m_diborder = true;
	int headlen = sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER);
	size_t hsize = headlen + 128;
	BYTE* header = new BYTE[hsize];
	memset(header, 0, hsize);
	BITMAPFILEHEADER* pfh = (BITMAPFILEHEADER*)header;
	BITMAPINFOHEADER* pbh = (BITMAPINFOHEADER*)(header + sizeof(BITMAPFILEHEADER));
	pbh->biSize = sizeof(BITMAPINFOHEADER);
	pbh->biWidth = width;
	pbh->biHeight = height;
	pbh->biBitCount = 24;
	pbh->biPlanes = 1;
	pbh->biCompression = BI_RGB;
	pbh->biSizeImage = WIDTHBYTES(width * 24) * height;
	pfh->bfType = DIB_HEADER_MARKER;
	pfh->bfSize = headlen + pbh->biSizeImage;
	pfh->bfOffBits = headlen;
	m_pstrm->Write(header, headlen, NULL);
	delete [] header;
}

bool rawmap::jpg_getsize(long offset, long* width, long* height)
{
	int save = tell();
	seek(offset + 2, SEEK_SET);

	bool bOK = false;
	BYTE cb[4];
	while (1) {
		// マーカーを取得
		if (eof() || (read(cb, 4) != 4) || (cb[0] != 0xff)) break;

		// ハフマン式コードフレーム？
		if (0xc0 <= cb[1] && cb[1] <= 0xc3) {
			getc();						// 精度のサンプルをスキップ
			*height = getmsbshort();	// イメージ内の行数
			*width = getmsbshort();		// イメージ内の桁数
			bOK = true;
			break;
		}
		seek((cb[2] << 8) + cb[3] - 2, SEEK_CUR);
	}
	seek(save, SEEK_SET);
	return bOK;
}

long rawmap::jpg_datasize()
{
	long start = tell();
	long end = start;
	if ((USHORT)getmsbshort() == 0xffd8) {
		BYTE cb[4];
		while (!eof()) {
			// マーカーを取得
			int n = read(cb, 4);
//@ for (int k=0;k<n;k++) { dbg.printf(L"%02X ",cb[k]); } dbg.printf(L" : pos=%p\n", tell() - n);
			if (n < 2) break;
			if (cb[0] != 0xff) break;

			if (cb[1] == 0xd9) {		// EOI
				end = tell();
				break;
			}
			if (n < 4) break;
			seek((cb[2] << 8) + cb[3] - 2, SEEK_CUR);

			if (cb[1] == 0xda) {
				// 最後のマーカーがSOSであればエントロピー符号化セグメントを読み飛ばす
				while (!eof()) {
					if (getc() == 0xff) {
						if (getc() != 0x00) {
							seek(-2, SEEK_CUR);
							break;
						}
					}
				}
			}
		}
	}
	seek(start, SEEK_SET);
	return (end - start);
}

bool rawmap::data_streamout(long offset, long length, bool rewind)
{
	// ストリームへJPEGデータを書き出す
	bool bOK = false;
	if (m_pstrm) {
		CTypeAlloc<BYTE> data(length);
		long save = tell();
		seek(offset, SEEK_SET);
		long numread = read(data, length);
		if (numread == length) {
			if (rewind) {
				LARGE_INTEGER li;
				li.QuadPart = 0;
				m_pstrm->Seek(li, STREAM_SEEK_SET, NULL);
			}
			// minolta/sonyの場合ffd8がxxd8になっているのでffd8を強制する
			data[0] = 0xff; data[1] = 0xd8;
			m_pstrm->Write(data, numread, NULL);
			bOK = true;
		}
		seek(save, SEEK_SET);
	}
	return bOK;
}

static int sstTagSizeArray[] = { 1,1,1,2,4,8,1,1,2,4,8,4,8 };

static int tagsize(int type, int count)
{
	return (count * sstTagSizeArray[(type < 13) ? type : 0]);
}

bool rawmap::tiff_seek(int type, int count)
{
	if (tagsize(type, count) > 4)
		seek(fget4() + m_tiffbase, SEEK_SET);
	return (eof() ? false : true);
}

void rawmap::makernote_nikon_previewimage()
{
	_imageinfo_t* pimage = create_ifitem();
	int entries = fget2();
	while (entries--) {
		if (eof()) break;

		int save = tell();
		int tag  = fget2();
		int type = fget2();
		int count= fget4();

		if (count < 0 || !tiff_seek(type, count)) {
			// このタグはデータ不全なのでスキップ
			seek(save + 12, SEEK_SET);
			continue;
		}

		switch (tag) {
		case 0x0103:				// Compression
			pimage->compression = (long)get_tagvalue(type);
			break;
//		case 0x011A:				// XResolution
//			break;
//		case 0x011B:				// YResolution
//			break;
//		case 0x0128:				// ResolutionUnit
//										// 1 => 'None'
//										// 2 => 'inches'
//										// 3 => 'cm'
//			break;
		case 0x0201:				// PreviewImageStart
			pimage->thumb_offset = m_tiffbase + (long)get_tagvalue(type);
			if (pimage->compression == 6)
				jpg_getsize(pimage->thumb_offset, &pimage->width, &pimage->height);
			break;
		case 0x0202:				// PreviewImageLength
			pimage->thumb_length = (long)get_tagvalue(type);
			break;
//		case 0x0213:				// YCbCrPositioning
//										// 1 => 'Centered'
//										// 2 => 'Co-sited'
//			break;
		default:
			break;
		}
		seek(save + 12, SEEK_SET);
	}
}

static bool nikon_decrypt(int serial, int count, int pos, int size, BYTE* buf)
{
  static const BYTE xlat[2][256] = {
  { 0xc1,0xbf,0x6d,0x0d,0x59,0xc5,0x13,0x9d,0x83,0x61,0x6b,0x4f,0xc7,0x7f,0x3d,0x3d,
    0x53,0x59,0xe3,0xc7,0xe9,0x2f,0x95,0xa7,0x95,0x1f,0xdf,0x7f,0x2b,0x29,0xc7,0x0d,
    0xdf,0x07,0xef,0x71,0x89,0x3d,0x13,0x3d,0x3b,0x13,0xfb,0x0d,0x89,0xc1,0x65,0x1f,
    0xb3,0x0d,0x6b,0x29,0xe3,0xfb,0xef,0xa3,0x6b,0x47,0x7f,0x95,0x35,0xa7,0x47,0x4f,
    0xc7,0xf1,0x59,0x95,0x35,0x11,0x29,0x61,0xf1,0x3d,0xb3,0x2b,0x0d,0x43,0x89,0xc1,
    0x9d,0x9d,0x89,0x65,0xf1,0xe9,0xdf,0xbf,0x3d,0x7f,0x53,0x97,0xe5,0xe9,0x95,0x17,
    0x1d,0x3d,0x8b,0xfb,0xc7,0xe3,0x67,0xa7,0x07,0xf1,0x71,0xa7,0x53,0xb5,0x29,0x89,
    0xe5,0x2b,0xa7,0x17,0x29,0xe9,0x4f,0xc5,0x65,0x6d,0x6b,0xef,0x0d,0x89,0x49,0x2f,
    0xb3,0x43,0x53,0x65,0x1d,0x49,0xa3,0x13,0x89,0x59,0xef,0x6b,0xef,0x65,0x1d,0x0b,
    0x59,0x13,0xe3,0x4f,0x9d,0xb3,0x29,0x43,0x2b,0x07,0x1d,0x95,0x59,0x59,0x47,0xfb,
    0xe5,0xe9,0x61,0x47,0x2f,0x35,0x7f,0x17,0x7f,0xef,0x7f,0x95,0x95,0x71,0xd3,0xa3,
    0x0b,0x71,0xa3,0xad,0x0b,0x3b,0xb5,0xfb,0xa3,0xbf,0x4f,0x83,0x1d,0xad,0xe9,0x2f,
    0x71,0x65,0xa3,0xe5,0x07,0x35,0x3d,0x0d,0xb5,0xe9,0xe5,0x47,0x3b,0x9d,0xef,0x35,
    0xa3,0xbf,0xb3,0xdf,0x53,0xd3,0x97,0x53,0x49,0x71,0x07,0x35,0x61,0x71,0x2f,0x43,
    0x2f,0x11,0xdf,0x17,0x97,0xfb,0x95,0x3b,0x7f,0x6b,0xd3,0x25,0xbf,0xad,0xc7,0xc5,
    0xc5,0xb5,0x8b,0xef,0x2f,0xd3,0x07,0x6b,0x25,0x49,0x95,0x25,0x49,0x6d,0x71,0xc7 },
  { 0xa7,0xbc,0xc9,0xad,0x91,0xdf,0x85,0xe5,0xd4,0x78,0xd5,0x17,0x46,0x7c,0x29,0x4c,
    0x4d,0x03,0xe9,0x25,0x68,0x11,0x86,0xb3,0xbd,0xf7,0x6f,0x61,0x22,0xa2,0x26,0x34,
    0x2a,0xbe,0x1e,0x46,0x14,0x68,0x9d,0x44,0x18,0xc2,0x40,0xf4,0x7e,0x5f,0x1b,0xad,
    0x0b,0x94,0xb6,0x67,0xb4,0x0b,0xe1,0xea,0x95,0x9c,0x66,0xdc,0xe7,0x5d,0x6c,0x05,
    0xda,0xd5,0xdf,0x7a,0xef,0xf6,0xdb,0x1f,0x82,0x4c,0xc0,0x68,0x47,0xa1,0xbd,0xee,
    0x39,0x50,0x56,0x4a,0xdd,0xdf,0xa5,0xf8,0xc6,0xda,0xca,0x90,0xca,0x01,0x42,0x9d,
    0x8b,0x0c,0x73,0x43,0x75,0x05,0x94,0xde,0x24,0xb3,0x80,0x34,0xe5,0x2c,0xdc,0x9b,
    0x3f,0xca,0x33,0x45,0xd0,0xdb,0x5f,0xf5,0x52,0xc3,0x21,0xda,0xe2,0x22,0x72,0x6b,
    0x3e,0xd0,0x5b,0xa8,0x87,0x8c,0x06,0x5d,0x0f,0xdd,0x09,0x19,0x93,0xd0,0xb9,0xfc,
    0x8b,0x0f,0x84,0x60,0x33,0x1c,0x9b,0x45,0xf1,0xf0,0xa3,0x94,0x3a,0x12,0x77,0x33,
    0x4d,0x44,0x78,0x28,0x3c,0x9e,0xfd,0x65,0x57,0x16,0x94,0x6b,0xfb,0x59,0xd0,0xc8,
    0x22,0x36,0xdb,0xd2,0x63,0x98,0x43,0xa1,0x04,0x87,0x86,0xf7,0xa6,0x26,0xbb,0xd6,
    0x59,0x4d,0xbf,0x6a,0x2e,0xaa,0x2b,0xef,0xe6,0x78,0xb6,0x4e,0xe0,0x2f,0xdc,0x7c,
    0xbe,0x57,0x19,0x32,0x7e,0x2a,0xd0,0xb8,0xba,0x29,0x00,0x3c,0x52,0x7d,0xa8,0x49,
    0x3b,0x2d,0xeb,0x25,0x49,0xfa,0xa3,0xaa,0x39,0xa7,0xc5,0xa7,0x50,0x11,0x36,0xfb,
    0xc6,0x67,0x4a,0xf5,0xa5,0x12,0x65,0x7e,0xb0,0xdf,0xaf,0x4e,0xb3,0x61,0x7f,0x2f } };

/* @@@
	if (memcmp(buf, "02", 2)) return true;
	if (!ci || !cj) return false;

	BYTE ck = 0x60;
	BYTE ci = xlat[0][serial & 0xff];
	BYTE cj = xlat[1][count];
	for (; i < size; i++)
		buf[i] ^= (cj += ci * ck++);
	return true;
@@@ */
	int i, key = 0;
	for (i = 0; i < 4; ++i) {
		key ^= (count >> (i * 8)) & 0xff;
	}
	int ci = xlat[0][serial & 0xff];
	int cj = xlat[1][key];
	int ck = 0x60;
	for (i = pos; i < size; i++) {
		cj = (cj + ci * ck) & 0xff;
		ck = (ck + 1) & 0xff;
		buf[i] ^= cj;
	}
	return true;
}

static float nikon_ApertureConversions(int val)
{
	return powf(2.0F, (float)val / 24.0F);
}

static float nikon_FocalConversions(int val)
{
	return (5.0F * powf(2.0F, (float)val / 24.0F));
}

static float nikon_UnpackC3(int val)
{
	return ((float)(val >> 24) * (float)((val >> 16) & 0xff) / (float)((val >> 8) & 0xff));
}

void rawmap::makernote_nikon_lensdata(BYTE* lensdata, int count)
{
//@ dbg.dump(lensdata, count);
	register nikon_t* nikon = &m_exif.nikon;
	if (!memcmp(lensdata, "0100", 4)) {
		// 0100形式
		nikon->LensID = lensdata[6];
		wsprintfA(nikon->LensData, "%02X%02X%02X%02X%02X%02X%02X%02X", lensdata[6],lensdata[7],lensdata[8],lensdata[9],lensdata[10],lensdata[11],lensdata[12],nikon->LensType);

/* @@
	    0x07 => { #8
	        Name => 'LensFStops',
	        ValueConv => '$val / 12',
	        ValueConvInv => '$val * 12',
	        PrintConv => 'sprintf("%.2f", $val)',
	        PrintConvInv => '$val',
	    },
	    0x08 => { #8/9
	        Name => 'MinFocalLength',
	        %nikonFocalConversions,
	    },
	    0x09 => { #8/9
	        Name => 'MaxFocalLength',
	        %nikonFocalConversions,
	    },
	    0x0a => { #8
	        Name => 'MaxApertureAtMinFocal',
	        %nikonApertureConversions,
	    },
	    0x0b => { #8
	        Name => 'MaxApertureAtMaxFocal',
	        %nikonApertureConversions,
	    },
	    0x0c => 'MCUVersion', #8
@@ */
	}
	else if (!memcmp(lensdata, "0101", 4) ||
			 !memcmp(lensdata, "0201", 4) ||
			 !memcmp(lensdata, "0202", 4) ||
			 !memcmp(lensdata, "0203", 4)) {
		// 0101形式
		// 0201形式
		// 0202形式 --- D40
		// 0203形式 --- D300,D700
		nikon->AFAperture = lensdata[0x05];
		nikon->FocusDistance = lensdata[0x09];
		nikon->FocalLength =  lensdata[0x0A];
		nikon->LensID = lensdata[0x0B];
		wsprintfA(nikon->LensData, "%02X%02X%02X%02X%02X%02X%02X%02X", lensdata[11],lensdata[12],lensdata[13],lensdata[14],lensdata[15],lensdata[16],lensdata[17],nikon->LensType);
/* @@
	    0x08 => 'FocusPosition', #8
	    0x0a => { #8/9
	        Name => 'FocalLength',
	        Priority => 0,
	        %nikonFocalConversions,
	    },
	    0x0c => { #8
	        Name => 'LensFStops',
	        ValueConv => '$val / 12',
	        ValueConvInv => '$val * 12',
	        PrintConv => 'sprintf("%.2f", $val)',
	        PrintConvInv => '$val',
	    },
	    0x0e => { #8/9
	        Name => 'MaxFocalLength',
	        %nikonFocalConversions,
	    },
	    0x0f => { #8
	        Name => 'MaxApertureAtMinFocal',
	        %nikonApertureConversions,
	    },
	    0x10 => { #8
	        Name => 'MaxApertureAtMaxFocal',
	        %nikonApertureConversions,
	    },
	    0x11 => 'MCUVersion', #8
	    0x12 => { #8
	        Name => 'EffectiveMaxAperture',
	        %nikonApertureConversions,
	    },
@@ */
	}
	else if (!memcmp(lensdata, "0204", 4)) {
		// 0204形式 --- D90, D5000, Df
		nikon->AFAperture = lensdata[0x05];
		nikon->FocusDistance = lensdata[0x0a];
		nikon->FocalLength =  lensdata[0x0b];
		nikon->LensID = lensdata[0x0c];
		wsprintfA(nikon->LensData, "%02X%02X%02X%02X%02X%02X%02X%02X", lensdata[12],lensdata[13],lensdata[14],lensdata[15],lensdata[16],lensdata[17],lensdata[18],nikon->LensType);
	}
	else if (!memcmp(lensdata, "04", 2)) {
		if (m_exif.ModelID == NIKON_1_J3) return;	// J3 は正常に取得できない

		int pos = 0;
		if      (!memcmp(lensdata + 2, "00", 2) ||
				 !memcmp(lensdata + 2, "01", 2)) { pos = 0x18a; }		// 0400形式 --- 1_V1, 1_J1, 1_J2
		else if (!memcmp(lensdata + 2, "02", 2)) { pos = 0x18b; }		// 0402形式 --- 1_V2, 1_S1, 1_J3
		else if (!memcmp(lensdata + 2, "03", 2)) { pos = 0x2ac; }		// 0403形式 --- 1_AW1
		if (pos > 0 && count >= (pos + 64)) {
			lstrcpynA(nikon->LensModel, (char*)(lensdata + pos), ARRAYSIZE(nikon->LensModel));
		}
	}
}

void rawmap::makernote_nikon_picture_control(int count)
{
	// Makernote tag 0x23
	if (count < 1) return;

	register nikon_t* nikon = &m_exif.nikon;
	read(nikon->PictureControlInfo, min(count, sizeof(nikon->PictureControlInfo)));

	// offset 00 - 03:  4 bytes PictureControlVersion
	// offset 04 - 23: 20 bytes PictureControlName
	// offset 24 - 43: 20 bytes PictureControlBaseName

	// beginning at byte 44, there is some interesting information.
	// here are the observed bytes for each PictureControlMode:
	//            44 45 46 47 48 49 50 51 52 53 54 55 56 57
	// STANDARD   00 01 00 00 00 80 83 80 80 80 80 ff ff ff
	// NEUTRAL    03 c2 00 00 00 ff 82 80 80 80 80 ff ff ff
	// VIVID      00 c3 00 00 00 80 84 80 80 80 80 ff ff ff
	// MONOCHROME 06 4d 00 01 02 ff 82 80 80 ff ff 80 80 ff
	// (note that up to 9 different custom picture controls can be stored)

	// offset 48:               PictureControlAdjust
									// 0=Default Settings
									// 1=Quick Adjust
									// 2=Full Control
	// offset 49:               PictureControlQuickAdjust -- settings: -2 to +2
									// 0x7e = -2
									// 0x7f = -1
									// 0x80 = Normal
									// 0x81 = +1
									// 0x82 = +2
									// 0xff = n/a
	// offset 50:               Sharpness -- settings: 0 to 9, Auto
									// 0x00 = Auto
									// 0x80 = No Sharpening
									// 0x81 = 1
									// 0x82 = 2
									// 0x83 = 3
									// 0x84 = 4
									// 0x85 = 5
									// 0x86 = 6
									// 0x87 = 7
									// 0x88 = 8
									// 0x89 = 9
	// offset 51:				Contrast -- settings: -3 to +3, Auto
									// 0x00 = Auto
									// 0x7d = -3
									// 0x7e = -2
									// 0x7f = -1
									// 0x80 = Normal
									// 0x81 = +1
									// 0x82 = +2
									// 0x83 = +3
	// offset 52:				Brightness -- settings: -1 to +1
									// 0x7f = -1
									// 0x80 = Normal
									// 0x81 = +1
	// offset 53:               Saturation -- settings: -3 to +3, Auto
									// 0x00 = Auto
									// 0x7d = -3
									// 0x7e = -2
									// 0x7f = -1
									// 0x80 = Normal
									// 0x81 = +1
									// 0x82 = +2
									// 0x83 = +3
									// 0xff = n/a
	// offset 54:               HueAdjustment -- settings: -3 to +3
									// 0x7d = -3
									// 0x7e = -2
									// 0x7f = -1
									// 0x80 = None
									// 0x81 = +1
									// 0x82 = +2
									// 0x83 = +3
									// 0xff = n/a
	// offset 55:               FilterEffect -- settings: Off,Yellow,Orange,Red,Green
									// 0x80 = Off
									// 0x81 = Yellow
									// 0x82 = Orange
									// 0x83 = Red
									// 0x84 = Green
									// 0xff = n/a
	// offset 56:               ToningEffect -- settings: B&W,Sepia,Cyanotype,Red,Yellow,Green,Blue-Green,Blue,Purple-Blue,Red-Purple
									// 0x80 = B&W
									// 0x81 = Sepia
									// 0x82 = Cyanotype
									// 0x83 = Red
									// 0x84 = Yellow
									// 0x85 = Green
									// 0x86 = Blue-green
									// 0x87 = Blue
									// 0x88 = Purple-blue
									// 0x89 = Red-purple
									// 0xff = n/a
	// offset 57:               ToningSaturation -- settings: B&W,Sepia,Cyanotype,Red,Yellow,Green,Blue-Green,Blue,Purple-Blue,Red-Purple
									// 0x81 = 1
									// 0x82 = 2
									// 0x83 = 3
									// 0x84 = 4
									// 0x85 = 5
									// 0x86 = 6
									// 0x87 = 7
									// 0xff = n/a
}

void rawmap::makernote_nikon()
{
	// The MakerNote might have its own TIFF header (possibly with
	// its own byte-order!), or it might just be a table.

	register nikon_t* nikon = &m_exif.nikon;

	int i;
	memset(nikon, 0, sizeof(nikon_t));
	nikon->ISO = INT_MAX;
	nikon->WhiteBalanceFine = INT_MAX;
	nikon->ProgramShift = INT_MAX;
	nikon->ExposureDifference = INT_MAX;
	nikon->FlashComp = INT_MAX;
	nikon->FlashExposureBracket = INT_MAX;
	nikon->ExposureBracket = FLT_MAX;
	nikon->ColorSpace = INT_MAX;
	nikon->LensType = INT_MAX;
	nikon->FlashUsed = INT_MAX;
	nikon->DriveMode = INT_MAX;
	nikon->VibrationReduction = INT_MAX;
	nikon->ImageAuthentication = INT_MAX;
	nikon->ActiveDLighting = INT_MAX;
	nikon->VinetControl = INT_MAX;
	nikon->AutoBracketRelease = INT_MAX;
	nikon->HueAdjustment = INT_MAX;
	nikon->ManualFocusDistance = FLT_MAX;
	nikon->DigitalZoom = FLT_MAX;
	nikon->SaturationComp = INT_MAX;
	nikon->FilterEffect = INT_MAX;
	nikon->TakeCount = INT_MAX;
	nikon->DelTakeCount = INT_MAX;
	nikon->RealTakeCount = INT_MAX;
	nikon->LensID = INT_MAX;
	nikon->AFAperture = INT_MAX;
	nikon->FocalLength = INT_MAX;
	nikon->FocusDistance = INT_MAX;
	nikon->XCCDPixelPitch = FLT_MAX;
	nikon->YCCDPixelPitch = FLT_MAX;
	nikon->HiSensitiveNoiseReduction = INT_MAX;
	for (i = 0; i < 4; i++) {
		nikon->LensInfo[i] = FLT_MAX;
		nikon->AFInfo[i] = BYTE_MAX;
	}

	int val;
	int stiffbase = m_tiffbase;
	short sorder = m_order;					// save order
	unsigned char buf[32];
	int serial = 0;
	int lensdata_offset = 0;
	int lensdata_length = 0;

	read(buf, 10);
	if (!memcmp(buf, "Nikon\0", 6)) {		// starts with "Nikon\0\2\0\0\0" ?
		m_tiffbase = tell();
		m_order = fget2();					// might differ from file-wide byteorder
		val = fget2();						// should be 42 decimal
		int ofs = fget4();
		seek(ofs - 8, SEEK_CUR);
	}
	else {
//		seek(-10, SEEK_CUR);
		m_exif.exist_makenote = false;
		return;
	}

	int entries = fget2();
	while (entries--) {
		if (eof()) break;

		int save = tell();
		int tag  = fget2();
		int type = fget2();
		int count= fget4();

		if (count < 0 || !tiff_seek(type, count)) {
			// このタグはデータ不全なのでスキップ
			seek(save + 12, SEEK_SET);
			continue;
		}

		switch (tag) {
//		case 0x0001:						// UNDEFx4: makernote version
		case 0x0002:						// SHORTx2:ISO
			val = (int)get_tagvalue(type);
			nikon->ISO = (int)get_tagvalue(type);
			if (val) {
				if (val == 1)
					nikon->ISOinc = TRUE;
				else {
					wsprintfA((char*)buf, "%d%d", nikon->ISO, val);
					nikon->ISO = atoi((char*)buf);
				}
			}
			if (!HasInt(m_exif.ISOEquiv)) m_exif.ISOEquiv = nikon->ISO;
			break;
		case 0x0003:						// ASCII:カラーモード
			read_ascii(nikon->ColorMode, count, ARRAYSIZE(nikon->ColorMode));
			break;
		case 0x0004:						// ASCII:画質モード
			read_ascii(nikon->QualityMode, count, ARRAYSIZE(nikon->QualityMode));
			break;
		case 0x0005:						// ASCII:ホワイトバランス
			read_ascii(nikon->WhiteBalance, count, ARRAYSIZE(nikon->WhiteBalance));
			break;
		case 0x0006:						// ASCII:輪郭の強調
			read_ascii(nikon->EdgeMode, count, ARRAYSIZE(nikon->EdgeMode));
			break;
		case 0x0007:						// ASCII:フォーカスモード
			read_ascii(nikon->FocusMode, count, ARRAYSIZE(nikon->FocusMode));
			break;
		case 0x0008:						// ASCII:フラッシュの設定
			read_ascii(nikon->FlashSetting, count, ARRAYSIZE(nikon->FlashSetting));
			break;
		case 0x0009:						// ASCII:フラッシュモード
			read_ascii(nikon->FlashMode, count, ARRAYSIZE(nikon->FlashMode));
			break;
		case 0x000B:						// SSHRT:ホワイトバランス補正
			nikon->WhiteBalanceFine = (int)(short)get_tagvalue(type);
			break;
//		case 0x000C:						// RATNLx4: ColorBalance1

		case 0x000D:						// UNDEFx4: プログラムシフト
			// ValueタイプはUNDEFだが、SLONGで読み込む
			nikon->ProgramShift = (int)get_tagvalue(FMT_SLONG);
			break;
		case 0x000E:						// UNDEFx4: Exposure difference
			// ValueタイプはUNDEFだが、SLONGで読み込む
			nikon->ExposureDifference = (int)get_tagvalue(FMT_SLONG);
			break;
		case 0x000F:						// ASCII:ISOモード
			read_ascii(nikon->ISOMode, count, ARRAYSIZE(nikon->ISOMode));
			break;
		case 0x0011:						// ThumbOffset, Thumbnail IFD offset
			seek(m_tiffbase + (int)get_tagvalue(type), SEEK_SET);
			makernote_nikon_previewimage();
			break;
		case 0x0012:						// UNDEFx4:調光補正
			// ValueタイプはUNDEFだが、SLONGで読み込む?
			nikon->FlashComp = (int)get_tagvalue(FMT_SLONG);
			break;
//		case 0x0013:						// UNDEFx4: ISO(2)
//		case 0x0016:						// USHRTx4: 画像領域
//		case 0x0017:						// UNDEFx4:
		case 0x0018:						// UNDEFx4:フラッシュブラケット
			// ValueタイプはUNDEFだが、SLONGで読み込む?
			nikon->FlashExposureBracket = (int)get_tagvalue(FMT_SLONG);
			break;
		case 0x0019:						// RATIN:ブラケットステップ
			nikon->ExposureBracket = (float)get_tagvalue(type);
			break;
		case 0x001A:						// ASCII:D-ライティング
			read_ascii(nikon->DLighting, count, ARRAYSIZE(nikon->DLighting));
			break;
		case 0x001B:						// USHRTx7: クロップハイスピードモード
			if (!HasInt(m_exif.ImageWidth)) {
				// NIKONのNEFファイルにはイメージサイズ情報がないため
				// このCCDサイズ情報を代用する
				// ただしサイズはCCDサイズであってイメージサイズではないのが(;;)
				get_tagvalue(type);		// skip 1st
				m_exif.ImageWidth = (int)get_tagvalue(type);
				m_exif.ImageHeight = (int)get_tagvalue(type);
			}
			break;
		case 0x001D:						// ASCII:シリアルナンバー
			read_ascii(nikon->SerialNumber, count, ARRAYSIZE(nikon->SerialNumber));

			if (!memcmp(nikon->SerialNumber, "D50", 3)) {		// D50だとシリアルナンバーのところに"D50"が入っているんだよな
				serial = 0x22;			// 0x22,0x36,0x7B,0x8A,0x8C,0xA9,0xCE,0xF3 ... のどれでも
			}
			else {
				switch (m_exif.ModelID) {
				case NIKON_D200:
				case NIKON_D60:
					serial = 0x60;		// 0x07, 0x50, 0x51, 0x60, 0x84, 0x8E, 0x94, 0x9A, 0xBC, 0xCD ... のどれでも
					break;
				default:
					serial = atoi(nikon->SerialNumber);
					if (serial == 0)
						serial = 0x60;
					break;
				}
			}
			break;
		case 0x001E:						// USHRT: 色空間
			nikon->ColorSpace = (int)get_tagvalue(type);
			break;
		case 0x001F:						// UNDEFx8: VRInfo
												// offset 0-3: 4 bytes
												//   VRInfoVersion
												// offset 4: 1 byte VibrationReduction
												//   1 = On, 2 = Off
												// offset 5-7: unknown
			read(buf, min(count, sizeof(buf)));
			nikon->VibrationReduction = buf[4];
			break;
		case 0x0020:						// BYTE: 画像真正性検証  0=Off, 1=On
			nikon->ImageAuthentication = (int)get_tagvalue(type);
			break;
//		case 0x0021:						// Subdir: FaceDetect
		case 0x0022:						// USHRT: アクティブD-ライティング
											// 0xffff = オート → (int)(short)で -1 と判別させる
			nikon->ActiveDLighting = (int)(short)get_tagvalue(type);
			break;
		case 0x0023:						// UNDEFxn: ピクチャーコントロール
			makernote_nikon_picture_control(count);
			break;
//		case 0x0024:						// UNDEFx4:
		case 0x0025:						// UNDEFx14: ISOInfo
			read(nikon->ISOInfo, min(count, sizeof(nikon->ISOInfo)));
			if (m_order != 0x4949) {
				SWAP(nikon->ISOInfo[4], nikon->ISOInfo[5]);
				SWAP(nikon->ISOInfo[10], nikon->ISOInfo[11]);
			}
			break;
		case 0x002A:						// USHORT: ヴィネットコントロール
												// 0 = off
												// 1 = 弱め（30%）
												// 3 = 標準（50%）
												// 5 = 強め（70%）
			nikon->VinetControl = (int)get_tagvalue(type);
			break;
		case 0x0080:						// ASCII:画像モード
			read_ascii(nikon->ImageMode, count, ARRAYSIZE(nikon->ImageMode));
			break;
		case 0x0081:						// ASCII:階調補正
			read_ascii(nikon->ToneAdjust, count, ARRAYSIZE(nikon->ToneAdjust));
			break;
		case 0x0082:						// ASCII:レンズコンバータ
			read_ascii(nikon->LensConverter, count, ARRAYSIZE(nikon->LensConverter));
			break;
		case 0x0083:						// BYTE:レンズタイプ
			nikon->LensType = (int)get_tagvalue(type);
			break;
		case 0x0084:						// RATINx4:レンズ情報
			count = min(4, count);
			for (i = 0; i < count; i++) {
				nikon->LensInfo[i] = (float)get_tagvalue(type);
				if (nikon->LensInfo[i] == 1.0F) {
					switch (i) {
					case 0:
					case 1: nikon->LensInfo[i] = m_exif.FocalLength; break;
					case 2:
					case 3: nikon->LensInfo[i] = m_exif.MaxAperture; break;
					}
				}
			}
			break;
		case 0x0085:						// RATIN:マニュアルフォーカス位置
			nikon->ManualFocusDistance = (float)get_tagvalue(type);
			break;
		case 0x0086:						// RATIN:デジタルズーム
			nikon->DigitalZoom = (float)get_tagvalue(type);
			break;
		case 0x0087:						// BYTE:フラッシュの利用
			nikon->FlashUsed = (int)get_tagvalue(type);
			break;
		case 0x0088:						// BYTEx4:フォーカスエリア
			read(nikon->AFInfo, min(count, sizeof(nikon->AFInfo)));
			break;
		case 0x0089:						// USHORT:ドライブモード
			nikon->DriveMode = (int)get_tagvalue(type);
			break;
		case 0x008A:						// USHRT: レリーズ状態
			nikon->AutoBracketRelease = (int)get_tagvalue(type);
			break;
//@		case 0x008B:						// UNDEFx4: LensFStops?
//@			break;
//@		case 0x008C:						// ContrastCurve
//@			break;
		case 0x008D:						// ASCII: 色空間
			read_ascii(nikon->ColorSpaceName, count, ARRAYSIZE(nikon->ColorSpaceName));
			break;
		case 0x008F:						// ASCII:シーンモード
			read_ascii(nikon->SceneMode, count, ARRAYSIZE(nikon->SceneMode));
			break;
		case 0x0090:						// ASCII:ライトタイプ
			read_ascii(nikon->LightType, count, ARRAYSIZE(nikon->LightType));
			break;
		case 0x0092:						// 色合い調整
			nikon->HueAdjustment = (int)get_tagvalue(type);
			break;
		case 0x0094:						// BYTE:彩度補正 0～255
			nikon->SaturationComp = (int)(char)getc();
			break;
		case 0x0095:						// 長秒時ノイズ除去
			read(nikon->NoiseReduction, count);
			break;
//		case 0x0096:						// NEFCurve2
//		case 0x0097:						// UNDEFx: カラーバランス
												// 先頭4バイト "0100" =  Type1
												// 先頭4バイト "0102" =  Type2 (D2H)
												// 先頭4バイト "0103" =  Type3 (D70)
												// 先頭4バイト "0205" =  Type2+Encrypt(D50)
												// 先頭4バイト "020?" =  Type2+Encrypt(0204=D2X,0206=D2Hs,0207=D200,0209=D700)
												// 先頭4バイト "021?" =  Type2+Encrypt(0211=D5000)
//			break;
		case 0x0098:						// UNDEFx:レンズデータブロック
			lensdata_offset = tell();
			lensdata_length = count;
			break;
//		case 0x0099:						// USHRT: NEFThumbnailSize
		case 0x009A:						// RATINx2: 画素ピッチ（μm）
			nikon->XCCDPixelPitch = (float)get_tagvalue(type);
			nikon->YCCDPixelPitch = (float)get_tagvalue(type);
			break;
		case 0x009C:						// ASCII:アシスト機能付きシーンモード
			read_ascii(nikon->AssSceneMode, count, ARRAYSIZE(nikon->AssSceneMode));
			break;
		case 0x009E:						// USHRTx10
			// オフセット 0 はフィルター効果？
			nikon->FilterEffect = (int)get_tagvalue(type);
			break;
		case 0x00A0:						// ASCII:シリアルナンバー
			read_ascii(nikon->SerialNumber, count, ARRAYSIZE(nikon->SerialNumber));
			break;
//		case 0x00A2:						// ULONG: 画像データサイズ（バイト数）
		case 0x00A5:						// LONG:差引撮影回数
			nikon->TakeCount = (int)get_tagvalue(type);
			break;
		case 0x00A6:						// LONG:削除撮影回数
			nikon->DelTakeCount = (int)get_tagvalue(type);
			break;
		case 0x00A7:						// LONG:撮影回数
			nikon->RealTakeCount = (int)get_tagvalue(type);
			// レンズデータ解析
			if (lensdata_offset && lensdata_length) {
				seek(-4, SEEK_CUR);
				unsigned key = getc() ^ getc() ^ getc() ^ getc();

				// レンズデータをここで読み込み
				BYTE* data = new BYTE[lensdata_length];
				seek(lensdata_offset, SEEK_SET);
				read(data, lensdata_length);

/* ---- 暗号解除チェック用 ------
BYTE* temp = new BYTE[lensdata_length];
for (int s = 1; s < 256; s++) {
	memcpy(temp, data, lensdata_length);
	nikon_decrypt(s, key, 4, lensdata_length, temp);

	for (int j = 0; j < ARRAYSIZE(ekNikonLensID); j++) {
		if (temp[0x0B] == ekNikonLensID[j].key) {
			dbg.printf(L"%s serial=%02X\n", ekNikonLensID[j].text, s);
		}
	}
}
delete [] temp;
 ------------------------------- */

				if (nikon_decrypt(serial, key, 4, lensdata_length, data)) {
					makernote_nikon_lensdata(data, lensdata_length);
				}
				delete [] data;
			}
			break;
//		case 0x00A8:						// UNDEFx20 ?
		case 0x00A9:						// ASCII:仕上がり設定
			read_ascii(nikon->ResultCondition, count, ARRAYSIZE(nikon->ResultCondition));
			break;
		case 0x00AA:						// ASCII:彩度設定
			read_ascii(nikon->ColorEnhance, count, ARRAYSIZE(nikon->ColorEnhance));
			break;
		case 0x00AB:						// ASCII:デジタルイメージプログラム
			read_ascii(nikon->DigiImagePgm, count, ARRAYSIZE(nikon->DigiImagePgm));
			break;
		case 0x00AC:						// ASCII:手ブレ補正
			read_ascii(nikon->VibrationReductionText, count, ARRAYSIZE(nikon->VibrationReductionText));
			break;
		case 0x00B0:						// UNDEFxn: 多重露出設定
			read(nikon->MultiExposureInfo, min(count, sizeof(nikon->MultiExposureInfo)));
			break;
		case 0x00B1:						// USHRT:高感度ノイズ除去
			nikon->HiSensitiveNoiseReduction = (int)get_tagvalue(type);
			break;
		case 0x00B3:						// ASCII:白黒カスタマイズ（フィルター効果）
			read_ascii(nikon->BWFilterEffect, count, ARRAYSIZE(nikon->BWFilterEffect));
			break;
//		case 0x00B7:						// UNDEF x n
//		case 0x00B8:						// UNDEF x n
//		case 0x00B9:						// UNDEF x 4
		case 0x00BD:						// UNDEFxn: ピクチャーコントロール (P6000,P7100,,,)
			makernote_nikon_picture_control(count);
			break;
		case 0x0E09:						// ASCII:ソフトウェア
			read_ascii(nikon->Software, count, ARRAYSIZE(nikon->Software));
			break;
		default:
// tagdump(this, tag, type, count);
			break;
		}
		seek(save + 12, SEEK_SET);
	}

	m_order = sorder;
	m_tiffbase = stiffbase;
}

static void _canon_init(canon_t* canon)
{
	// メーカーノート初期化
	canon->WhiteBalance = INT_MAX;
	canon->MacroMode = INT_MAX;
	canon->Quality = INT_MAX;
	canon->ShootMode = INT_MAX;
	canon->ISO = INT_MAX;
	canon->ExposureProgram = INT_MAX;
	canon->FlashMode = INT_MAX;
	canon->DigitalZoom = INT_MAX;
	canon->SelfTimer = INT_MAX;
	canon->DriveMode = INT_MAX;
	canon->FocusMode = INT_MAX;
	canon->SizeType = INT_MAX;
	canon->AdjustContrast = INT_MAX;
	canon->AdjustSaturation = INT_MAX;
	canon->AdjustSharpness = INT_MAX;
	canon->MeteringMode = INT_MAX;
	canon->FocusRange = INT_MAX;
	canon->FocusContinuous = INT_MAX;
	canon->AESetting = INT_MAX;
	canon->ImageStabilization = INT_MAX;
	canon->FlashCondition = INT_MAX;
	canon->ExposureComp = INT_MAX;
	canon->ContinuousNumber = INT_MAX;
	canon->FlashGuideNumber = INT_MAX;
	canon->AFPoint = INT_MAX;
	canon->AFPointUsed = INT_MAX;
	canon->MaxFocalLength = INT_MAX;
	canon->MinFocalLength = INT_MAX;
	canon->FocalLengthUnit = INT_MAX;
	canon->MaxAperture = FLT_MAX;
	canon->MinAperture = FLT_MAX;
	canon->FocalType = INT_MAX;
	canon->FocalPlaneXSize = INT_MAX;
	canon->FocalPlaneYSize = INT_MAX;
	canon->FocalLength = INT_MAX;
	canon->FlashComp = INT_MAX;
	canon->AEBSeq = INT_MAX;
	canon->AEBComp = INT_MAX;
	canon->ToneCurve = INT_MAX;
	canon->SharpnessPower = INT_MAX;
	canon->SharpnessFrequency = INT_MAX;
	canon->ColorTemp = INT_MAX;
	canon->ColorMatrix = INT_MAX;
	canon->DevParam = INT_MAX;
	canon->ISOapex = INT_MAX;
	canon->Aperture = FLT_MAX;
	canon->ExposureTime = FLT_MAX;
	canon->ColorEffect = INT_MAX;
	canon->AdjustColorTone = INT_MAX;
	canon->ShotNumber = INT_MAX;
	canon->FirmWare[0] = 0;
	canon->ImageType[0] = 0;
	canon->OwnerName[0] = 0;
	canon->SerialNumber = INT_MAX;
	canon->RawWidth = INT_MAX;
	canon->RawHeight = INT_MAX;
	canon->ImageWidth = INT_MAX;
	canon->ImageHeight = INT_MAX;
	canon->TargetShortestDistance = INT_MAX;
	canon->TargetLongestDistance = INT_MAX;
	canon->BulbDuration = INT_MAX;
	canon->CameraType = INT_MAX;
	canon->AutoRotate = INT_MAX;
	canon->NDFilter = INT_MAX;
	canon->SelfTimer2 = INT_MAX;
	canon->DigitalGain = INT_MAX;
	canon->WBBlueAmber = INT_MAX;
	canon->WBMagentaGreen = INT_MAX;
	canon->LensName[0] = 0;
	canon->SlowShutter = INT_MAX;
	canon->MyColor = INT_MAX;
	canon->RedLevel = INT_MAX;
	canon->GreenLevel = INT_MAX;
	canon->BlueLevel = INT_MAX;
	canon->SkinLevel = INT_MAX;
	canon->ColorRange = INT_MAX;

	canon->FiShutterCount = INT_MAX;
	canon->FiNoiseReduction = INT_MAX;
	canon->FiBracketMode = INT_MAX;
	canon->FiBracketValue = INT_MAX;
	canon->FiBracketShotNumber = INT_MAX;
	canon->FiWBBracketMode = INT_MAX;
	canon->FiWBBracketBlueAmber = INT_MAX;
	canon->FiWBBracketMagentaGreen = INT_MAX;
	canon->FiBWFilterEffect = INT_MAX;
	canon->FiBWColorFilter = INT_MAX;

	canon->PaPanoramaFrame = INT_MAX;
	canon->PaPanoramaDirection = INT_MAX;

	canon->CiExposureTime = INT_MAX;
	canon->CiHighlightTonePriority = INT_MAX;
	canon->CiFocalLength = INT_MAX;
	canon->CiLensTypes = INT_MAX;
	canon->CiShortFocal = INT_MAX;
	canon->CiLongFocal = INT_MAX;
	canon->CiFocalType = INT_MAX;
	canon->CiHighISONoiseReduction = INT_MAX;
	canon->CiAutoLightingOptimizer = INT_MAX;

	canon->RetouchMyColor = INT_MAX;
}

static float _canon_ev_conv(int ev)
{
	float f;
	switch (abs(ev)) {
	case  0: return 0.0F;
	case 12: f = 0.33F; break;
	case 16: f = 0.50F; break;
	case 20: f = 0.67F; break;
	case 32: f = 1.00F; break;
	case 44: f = 1.33F; break;
	case 48: f = 1.50F; break;
	case 52: f = 1.67F; break;
	case 64: f = 2.00F; break;
	default: f = abs(ev) / 32.0F; break;
	}

	if (ev < 0) f *= -1;
	return f;
}

void rawmap::makernote_canon_settinginfo(int count)
{
	// Makernote tag 0x01
	if (count < 1) return;

	register canon_t* canon = &m_exif.canon;

	int* val = new int[count];
	for (int i = 0; i < count; i++) {
		short n = (short)fget2();
		val[i] = (n != SHRT_MAX) ? n : INT_MAX;
//@		dbg.printf(L"tag=0x01 shotinfo %02d: %d\n", i, n);
	}

	if (count > 1)	canon->MacroMode			= val[1];				// マクロモード
	if (count > 2)	canon->SelfTimer			= val[2];				// セルフタイマ
	if (count > 3)	canon->Quality				= val[3];				// クォリティ
	if (count > 4)	canon->FlashMode			= val[4];				// フラッシュモード
	if (count > 5)	canon->DriveMode			= val[5];				// ドライブモード
	// val[6]:unknown
	if (count > 7)	canon->FocusMode			= val[7];				// フォーカスモード
	// val[8]:unknown
	// val[9]:unknown
	if (count > 10)	canon->SizeType				= val[10];				// 画像サイズ種別
	if (count > 11)	canon->ShootMode			= val[11];				// イメージセレクトモード
	if (count > 12)	canon->DigitalZoom			= val[12];				// デジタルズーム
	if (count > 13)	canon->AdjustContrast		= val[13];				// コントラスト調整
	if (count > 14)	canon->AdjustSaturation		= val[14];				// 彩度調整
	if (count > 15)	canon->AdjustSharpness		= val[15];				// シャープネス調整
	if (count > 16)	canon->ISO					= val[16];				// ISO感度
	if (!HasInt(m_exif.ISOEquiv)) {
		switch (val[16]) {
		case 14: m_exif.ISOEquiv = 0; break;		// ISO(HI)
		case 15: m_exif.ISOEquiv = 0; break;		// ISO(AUTO)
		case 16: m_exif.ISOEquiv = 50; break;
		case 17: m_exif.ISOEquiv = 100; break;
		case 18: m_exif.ISOEquiv = 200; break;
		case 19: m_exif.ISOEquiv = 400; break;
		default:
			if (val[16] & 0x4000) {
				// ISO値に0x4000を加算した値が入っている
				m_exif.ISOEquiv = val[16] - 0x4000;
			}
			break;
		}
	}
	if (count > 17)	canon->MeteringMode			= val[17];				// 測光方式
	if (count > 18)	canon->FocusRange			= val[18];				// フォーカスレンジ
	if (count > 19)	canon->AFPoint				= val[19];				// AFポイント
	if (count > 20)	canon->ExposureProgram		= val[20];				// 露出制御プログラム
	if (!HasInt(m_exif.ExposureProgram)) {
		switch (val[20]) {
		case 1: m_exif.ExposureProgram = 2; break;
		case 2: m_exif.ExposureProgram = 4; break;
		case 3: m_exif.ExposureProgram = 3; break;
		case 4: m_exif.ExposureProgram = 1; break;
		}
	}
	// val[21]:unknown
	if (count > 22 && val[22] > 0 && IS_CANON_EOS(m_exif.ModelID))
					canon->CiLensTypes			= val[22];				// レンズタイプ
	if (count > 23)	canon->MaxFocalLength		= (unsigned)val[23];	// レンズ最長焦点距離
	if (count > 24)	canon->MinFocalLength		= (unsigned)val[24];	// レンズ最短焦点距離
	if (count > 25)	canon->FocalLengthUnit		= (unsigned)val[25];	// レンズ焦点距離単位(mili-meter)
	if (count > 26 && val[26] >= 0)
					canon->MaxAperture			= APEX2F(val[26]);		// 最大絞り
	if (count > 27 && val[27] >= 0)
					canon->MinAperture			= APEX2F(val[27]);		// 最小絞り
	if (count > 28)	canon->FlashCondition = val[28];					// フラッシュ状態
	// val[29]: FlashBits
		// bit-def
		// 3=On, 4=FP sync enabled, 7=2nd-curtain sync used,
		// 11=FP sync used, 13=Internal flash, 14=External E-TTL,
	// val[30-31]:unknown
	if (count > 32 && val[32] >= 0)
					canon->FocusContinuous		= val[32];				// フォーカス動作
	if (count > 33 && val[33] >= 0)
					canon->AESetting			= val[33];				// AE設定
	if (count > 34 && val[34] >= 0)
					canon->ImageStabilization	= val[34];				// 手ブレ補正
	// val[35]: 表示絞り 'DisplayAperture'
	// val[36]: ズーム解像度（デジタル）
	// val[37]: ズーム解像度（光学）
	// val[38]: unknown
	// val[39]: unknown
	if (count > 40 && val[40] >= 0)
		canon->ColorEffect						= val[40];				// 色効果
	// val[41]: unknown
	if (count > 42)	canon->AdjustColorTone		= val[42];				// 色あい調整
	// val[43]: unknown
	// val[44]: unknown
	// val[45]: unknown
	// val[46]: unknown

	delete [] val;
}

void rawmap::makernote_canon_focalinfo(int count)
{
	// Makernote tag 0x02
	if (count < 1) return;

	register canon_t* canon = &m_exif.canon;

	int* val = new int[count];
	for (int i = 0; i < count; i++) {
		short n = (short)fget2();
		val[i] = (n != SHRT_MAX) ? n : INT_MAX;
//@		dbg.printf(L"tag=0x02 focalinfo %02d: %d\n", i, n);
	}

	if (count > 0)	canon->FocalType			= val[0];				// レンズタイプ
	if (count > 1)	canon->FocalLength			= (unsigned)val[1];		// レンズ焦点距離
	if (count > 2)	canon->FocalPlaneXSize		= val[2];				// 撮像素子の横サイズ
	if (count > 3)	canon->FocalPlaneXSize		= val[3];				// 撮像素子の縦サイズ

	delete [] val;
}

void rawmap::makernote_canon_shotinfo(int count)
{
	// Makernote tag 0x04
	if (count < 1) return;

	register canon_t* canon = &m_exif.canon;

	int* val = new int[count];
	for (int i = 0; i < count; i++) {
		short n = (short)fget2();
		val[i] = (n != SHRT_MAX) ? n : INT_MAX;
//@		dbg.printf(L"tag=0x04 shotinfo %02d: %d\n", i, n);
	}

	//	val[1]:															// ISO Auto感度
																		// ISO Base感度
	if (count > 2)	canon->ISOapex				= (int)(exp(_canon_ev_conv(val[2]) * log(2.0F)) * 100.0F / 32.0F);
	if (count > 4)	canon->Aperture				= APEX2F(val[4]);		// 絞り値
	if (count > 5)	canon->ExposureTime			= 1 / APEX2S(val[5]);	// 露出時間
	if (count > 6)	canon->ExposureComp			= val[6];				// 露出補正
	if (!HasFlt(m_exif.ExposureBias))
		m_exif.ExposureBias = _canon_ev_conv(val[6]);
	if (count > 7)	canon->WhiteBalance			= val[7];				// ホワイトバランス
	if (count > 8)	canon->SlowShutter			= val[8];				// スローシャッター
	if (count > 9)	canon->ContinuousNumber		= val[9];				// 連続撮影時の連番(開始番号 n, ...)
	//	val[10]:unknown
	//	val[11]:unknown
	//	val[12]:unknown
	if (count > 13 && val[13] > 0)
					canon->FlashGuideNumber		= val[13];				// フラッシュガイドNO.
	if (count > 14 && val[14])
					canon->AFPointUsed			= val[14];				// 合焦AFフレーム
	if (count > 15)	canon->FlashComp			= val[15];				// ストロボ調光補正
	if (count > 16)	canon->AEBSeq				= val[16];				// AEBシーケンス
	if (count > 17)	canon->AEBComp				= val[17];				// AEB補正
	//	va[18]:unknown
	if (count > 19)	canon->TargetLongestDistance = val[19];				// 被写体最長距離
	if (count > 20)	canon->TargetShortestDistance = val[20];			// 被写体最短距離
/* --------
	if (count > 21)														// FNumber
	        Description => 'Aperture',
	        Priority => 0,
	        # approximate big translation table by simple calculation - PH
	        ValueConv => '$val ? exp(Image::ExifTool::Canon::CanonEv($val)*log(2)/2) : undef()',
	        ValueConvInv => 'Image::ExifTool::Canon::CanonEvInv(log($val)*2/log(2))',
	        PrintConv => 'sprintf("%.2g",$val)',
	        PrintConvInv => '$val',
	if (count > 22)														// ExposureTime
            Description => 'Shutter Speed',
            # encoding is different for 20D and 350D (darn!)
            Condition => '$self->{CameraModel} =~ /\b(20D|350D|REBEL XT)/',
            Priority => 0,
            # approximate big translation table by simple calculation - PH
            ValueConv => '$val ? exp(-Image::ExifTool::Canon::CanonEv($val)*log(2))*1000/32 : undef()',
            ValueConvInv => 'Image::ExifTool::Canon::CanonEvInv(-log($val*32/1000)/log(2))',
            PrintConv => 'Image::ExifTool::Exif::PrintExposureTime($val)',
            PrintConvInv => 'eval $val',
        },
        {
            Description => 'Shutter Speed',
            Priority => 0,
            # approximate big translation table by simple calculation - PH
            ValueConv => '$val ? exp(-Image::ExifTool::Canon::CanonEv($val)*log(2)) : undef()',
            ValueConvInv => 'Image::ExifTool::Canon::CanonEvInv(-log($val)/log(2))',
            PrintConv => 'Image::ExifTool::Exif::PrintExposureTime($val)',
            PrintConvInv => 'eval $val',
        },
-------- */
	//	val[23]: unknown
	if (count > 24)	canon->BulbDuration				= val[24];			// バルブ撮影時の露出時間(sec x 10)
	//	val[25]: unknown
	if (count > 26)	canon->CameraType				= val[26];			// カメラタイプ
	if (count > 27)	canon->AutoRotate				= val[27];			// 回転
	if (count > 28 && val[28] >= 0)
					canon->NDFilter					= val[28];			// NDフィルター
	if (count > 29)	canon->SelfTimer2				= val[29];			// セルフタイマー2
	//	val[30]:
	//	val[31]:
	//	val[32]:
	//	val[33]:														// FlashOutput

	delete [] val;
}

void rawmap::makernote_canon_panoramainfo(int count)
{
	// MakerNotes tag 0x05
	if (count < 1) return;

	register canon_t* canon = &m_exif.canon;
	for (int i = 0; i < count; i++) {
		unsigned short val = (unsigned short)fget2();

		switch (i) {
		case 2:
			canon->PaPanoramaFrame = val;
			break;
		case 5:
			canon->PaPanoramaDirection = val;
			break;
		}
	}
}

//------------------------------------------------------------------------------
// Convert Canon hex-based EV (modulo 0x20) to real number
// Inputs: 0) value to convert
// ie) 0x00 -> 0
//     0x0c -> 0.33333
//     0x10 -> 0.5
//     0x14 -> 0.66666
//     0x20 -> 1   ...  etc

static float CanonEv(int val)
{
	float sign = (val < 0) ? -1.0F : 1.0F;
	val = abs(val);

	int ifrac = val & 0x1f;
	val -= ifrac;					// remove fraction
	// Convert 1/3 and 2/3 codes
	float frac = (float)ifrac;
	if (ifrac == 0x0c) {
		frac = 0x20 / 3;
	}
	else if (ifrac == 0x14) {
		frac = 0x40 / 3;
	}
	return sign * (val + frac) / 0x20;
}

void rawmap::makernote_canon_camerainfo(int count)
{
	// MakerNotes tag 0x0d
	// Used by the 1D, 1DS, 1DmkII, 1DSmkII and 5D
	if (count < 1) return;
	if (!IS_CANON_EOS(m_exif.ModelID)) return;

	register canon_t* canon = &m_exif.canon;
	BYTE* v = new BYTE[count];
	if (v) {
		read(v, count);
		if      (m_exif.ModelID == CANON_EOS_5DMK2) {
			canon->CiHighlightTonePriority = v[0x07];		// 0x07: HighlightTonePriority
			canon->CiHighISONoiseReduction = v[0xbd];		// 0xbd: HighISONoiseReduction
			canon->CiAutoLightingOptimizer = v[0xbf];		// 0xbf: AutoLightingOptimizer
		}
		else if (m_exif.ModelID == CANON_EOS_40D) {
			canon->CiLensTypes = msbShort(&v[0xd6]);		// 0xd6: レンズタイプ
		}
		else if (m_exif.ModelID == CANON_EOS_50D) {
			canon->CiHighlightTonePriority = v[0x07];		// 0x07: HighlightTonePriority
			canon->CiHighISONoiseReduction = v[0xbd];		// 0xbd: HighISONoiseReduction
			canon->CiAutoLightingOptimizer = v[0xbf];		// 0xbf: AutoLightingOptimizer
			canon->CiLensTypes = msbShort(&v[0xea]);		// 0xea: レンズタイプ
		}
		else if (m_exif.ModelID == CANON_EOS_60D) {
			canon->CiLensTypes = msbShort(&v[0xe8]);		// 0xe8: レンズタイプ
		}
		else if (m_exif.ModelID == CANON_EOS_70D) {
			canon->CiLensTypes = msbShort(&v[0x166]);		// 0x166: レンズタイプ
		}
		else if (m_exif.ModelID == CANON_EOS_KISSX3) {
			canon->CiHighlightTonePriority = v[0x07];		// 0x07: HighlightTonePriority
			canon->CiHighISONoiseReduction = v[0xbb];		// 0xbd: HighISONoiseReduction
			canon->CiAutoLightingOptimizer = v[0xbe];		// 0xbf: AutoLightingOptimizer
		}
		else if (m_exif.ModelID == CANON_EOS_1D) {
			canon->CiLensTypes = msbShort(&v[0x0d]);		// 0x0d: レンズタイプ
		}
		else if (m_exif.ModelID == CANON_EOS_1DMK3 ||
				 m_exif.ModelID == CANON_EOS_5D) {
			canon->CiLensTypes = msbShort(&v[0x0c]);		// 0x0c: レンズタイプ
		}
		else if (m_exif.ModelID == CANON_EOS_7D) {
			canon->CiLensTypes = msbShort(&v[0x112]);		// 0x112: レンズタイプ
		}
/* -----------
		else if (m_exif.ModelID == CANON_EOS_1DS) {
			canon->CiExposureTime = v[0x04];				// 0x04: 露出時間: exp(4 * log(2) * (1 - CanonEv($val - 24)))
			canon->CiFocalLength = *(USHORT*)&v[0x0a];		// 0x0a: 焦点距離
			canon->CiLensTypes = v[0x0d];					// 0x0d: レンズタイプ
			canon->CiShortFocal = *(USHORT*)&v[0x0e];		// 0x0e: Short Focal (mm)
			canon->CiLongFocal = *(USHORT*)&v[0x10];		// 0x10: Long Focal (mm)
			canon->CiSharpnessFrequency = v[0x47];			// 0x41: SharpnessFrequency, 0=n/a, 1=Lowest, 2=Low, 3=Standard, 4=High, 5=Highest
			canon->CiSharpness = v[0x48];					// 0x48: Sharpness
			canon->CiWhiteBalance = v[0x4a];				// 0x4a: WhiteBalance
			canon->CiColorTemperature = *(USHORT*)&v[0x4e];	// 0x4e: ColorTemperature
			canon->CiPictureStyle = v[0x51];				// 0x51: PictureStyle
		}
		else if (m_exif.ModelID == CANON_EOS_1DMK2 ||
				 m_exif.ModelID == CANON_EOS_1DSMK2) {
			canon->CiLensTypes = msbShort(&v[0x0c]);		// 0x0d: レンズタイプ
															// 0x75: ISO, string[5]
		}
		else if (m_exif.ModelID == CANON_EOS_1DMK2N) {
			canon->CiExposureTime = v[0x04];				// 0x04: 露出時間: exp(4 * log(2) * (1 - CanonEv($val - 24)))
			canon->CiFocalLength = *(USHORT*)&v[0x09];		// 0x09: 焦点距離
			canon->CiLensTypes = v[0x0d];					// 0x0d: レンズタイプ
			canon->CiShortFocal = *(USHORT*)&v[0x11];		// 0x11: Short Focal (mm)
			canon->CiLongFocal = *(USHORT*)&v[0x13];		// 0x13: Long Focal (mm)
			canon->CiWhiteBalance = v[0x36];				// 0x36: WhiteBalance
			canon->CiColorTemperature = *(USHORT*)&v[0x37];	// 0x37: ColorTemperature
			canon->CiPictureStyle = v[0x73];				// 0x73: PictureStyle
			canon->CiSharpness = v[0x74];					// 0x74: Sharpness
			canon->CiContrast = v[0x75];					// 0x75: Contrast
			canon->CiSaturation = v[0x76];					// 0x76: Saturation
			canon->CiColorTone = v[0x77];					// 0x77: ColorTone
															// 0x79: ISO, string[5]

		}
		else if (m_exif.ModelID == CANON_EOS_1DMK3 ||
				 m_exif.ModelID == CANON_EOS_1DSMK3) {
			canon->CiFNumber = v[0x03];						// 0x03: F値: exp(($val - 8) / 16 * log(2))
			canon->CiExposureTime = v[0x04];				// 0x04: 露出時間: exp(4 * log(2) * (1 - CanonEv($val - 24)))
			canon->CiISO = v[0x06];							// 0x06: ISO: 100 * exp(($val / 8 - 9) * log(2))
			canon->CiFocalLength = *(USHORT*)&v[0x1d];		// 0x1d: 焦点距離
			canon->CiFocusDistanceUpper = *(USHORT*)&v[0x43];	// 0x43: FocusDistanceUpper(odd-byte big-endian): unpack("n",pack("v",$val)) * 0.01
			canon->CiFocusDistanceLower = *(USHORT*)&v[0x45];	// 0x45: FocusDistanceLower(odd-byte big-endian): unpack("n",pack("v",$val)) * 0.01
			canon->CiWhiteBalance = v[0x5e];				// 0x5e: WhiteBalance
			canon->CiColorTemperature = *(USHORT*)&v[0x62];	// 0x62: ColorTemperature
			canon->CiPictureStyle = v[0x86];				// 0x86: PictureStyle
			canon->CiLensTypes = v[0x112];					// 0x112: レンズタイプ
			canon->CiShortFocal = *(USHORT*)&v[0x113];		// 0x113: Short Focal (mm)
			canon->CiLongFocal = *(USHORT*)&v[0x115];		// 0x115: Long Focal (mm)
															// 0x136: FirmwareVersion, string[6]
		}
		else if (m_exif.ModelID == CANON_EOS_5D) {
			canon->CiFNumber = v[0x03];						// 0x03: F値: exp(($val - 8) / 16 * log(2))
			canon->CiExposureTime = v[0x04];				// 0x04: 露出時間: exp(4 * log(2) * (1 - CanonEv($val - 24)))
			canon->CiISO = v[0x06];							// 0x06: ISO: 100 * exp(($val / 8 - 9) * log(2))
			canon->CiLensTypes = v[0x0d];					// 0x0d: レンズタイプ
			canon->CiFocalLength = *(USHORT*)&v[0x28];		// 0x28: 焦点距離
			canon->CiAFPointsInFocus5D = *(USHORT*)&v[0x38];// 0x38: AFPointsInFocus5D, 
															//			BITMASK
															//            0 => 'Center',
															//            1 => 'Top',
															//            2 => 'Bottom',
															//            3 => 'Upper-left',
															//            4 => 'Upper-right',
															//            5 => 'Lower-left',
															//            6 => 'Lower-right',
															//            7 => 'Left',
															//            8 => 'Right',
															//            9 => 'AI Servo1',
															//           10 => 'AI Servo2',
															//           11 => 'AI Servo3',
															//           12 => 'AI Servo4',
															//           13 => 'AI Servo5',
															//           14 => 'AI Servo6',
			canon->CiWhiteBalance = v[0x54];				// 0x54: WhiteBalance
			canon->CiColorTemperature = *(USHORT*)&v[0x58];	// 0x58: ColorTemperature
			canon->CiPictureStyle = v[0x6c];				// 0x6c: PictureStyle
			canon->CiShortFocal = *(USHORT*)&v[0x93];		// 0x93: Short Focal (mm)
			canon->CiLongFocal = *(USHORT*)&v[0x95];		// 0x95: Long Focal (mm)
			canon->CiLensTypes = v[0x98];					// 0x98: レンズタイプ
															// 0xa4: FirmwareVersion, string[6]
															// 0xac: ShortOwnerName, string[16]
			canon->CiImageNumber = v[0xd0];					// 0xd0: ImageNumber
		}
		else if (m_exif.ModelID == CANON_EOS_40D) {
			canon->CiFocalLength = *(USHORT*)&v[0x1d];		// 0x1d: 焦点距離
			canon->CiFocusDistanceUpper = *(USHORT*)&v[0x43];	// 0x43: FocusDistanceUpper(odd-byte big-endian): unpack("n",pack("v",$val)) * 0.01
			canon->CiFocusDistanceLower = *(USHORT*)&v[0x45];	// 0x45: FocusDistanceLower(odd-byte big-endian): unpack("n",pack("v",$val)) * 0.01
			canon->CiWhiteBalance = v[0x6f];				// 0x6f: WhiteBalance
			canon->CiColorTemperature = *(USHORT*)&v[0x73];	// 0x73: ColorTemperature
			canon->CiLensTypes = v[0xd7];					// 0xd7: レンズタイプ
			canon->CiShortFocal = *(USHORT*)&v[0xd8];		// 0xd8: Short Focal (mm)
			canon->CiLongFocal = *(USHORT*)&v[0xda];		// 0xda: Long Focal (mm)
															// 0xff: FirmwareVersion, string[6]
															// 0x92b: LensModel, string[64]
		}
		else if (m_exif.ModelID == CANON_EOS_50D) {
			canon->CiFocusDistanceUpper = *(USHORT*)&v[0x50];	// 0x50: FocusDistanceUpper(odd-byte big-endian): unpack("n",pack("v",$val)) * 0.01
			canon->CiFocusDistanceLower = *(USHORT*)&v[0x52];	// 0x52: FocusDistanceLower(odd-byte big-endian): unpack("n",pack("v",$val)) * 0.01
			canon->CiColorTemperature = *(USHORT*)&v[0x73];	// 0x73: ColorTemperature
		}
		else if (m_exif.ModelID == CANON_EOS_KISSX2) {
			canon->CiFocalLength = *(USHORT*)&v[0x1d];		// 0x1d: 焦点距離
			canon->CiFocusDistanceUpper = *(USHORT*)&v[0x43];	// 0x43: FocusDistanceUpper(odd-byte big-endian): unpack("n",pack("v",$val)) * 0.01
			canon->CiFocusDistanceLower = *(USHORT*)&v[0x45];	// 0x45: FocusDistanceLower(odd-byte big-endian): unpack("n",pack("v",$val)) * 0.01
			canon->CiWhiteBalance = v[0x6f];				// 0x6f: WhiteBalance
			canon->CiColorTemperature = *(USHORT*)&v[0x73];	// 0x73: ColorTemperature
			canon->CiLensTypes = v[0xdf];					// 0xdf: レンズタイプ
															// 0x107: FirmwareVersion, string[6]
															// 0x933: LensModel, string[64]
		}
		else if (m_exif.ModelID == CANON_EOS_KISSF) {
			canon->CiFocalLength = *(USHORT*)&v[0x1d];		// 0x1d: 焦点距離
			canon->CiFocusDistanceUpper = *(USHORT*)&v[0x43];	// 0x43: FocusDistanceUpper(odd-byte big-endian): unpack("n",pack("v",$val)) * 0.01
			canon->CiFocusDistanceLower = *(USHORT*)&v[0x45];	// 0x45: FocusDistanceLower(odd-byte big-endian): unpack("n",pack("v",$val)) * 0.01
			canon->CiWhiteBalance = v[0x6f];				// 0x6f: WhiteBalance
			canon->CiColorTemperature = *(USHORT*)&v[0x73];	// 0x73: ColorTemperature
			canon->CiLensTypes = v[0xe3];					// 0xe3: レンズタイプ
															// 0x10b: FirmwareVersion, string[6]
															// 0x937: LensModel, string[64]
		}
--------------------------------------- */
		delete [] v;
	}
}

void rawmap::makernote_canon_processinginfo(int count)
{
	// Makenote tag 0xa0
	if (count < 1) return;

	register canon_t* canon = &m_exif.canon;

	int* val = new int[count];
	for (int i = 0; i < count; i++) {
		short n = (short)fget2();
		val[i] = (n != SHRT_MAX) ? n : INT_MAX;
//@		dbg.printf(L"tag=0x04 shotinfo %02d: %d\n", i, n);
	}

	if (count > 1)	canon->ToneCurve			= val[1];					// トーンカーブ
	if (count > 2)	canon->SharpnessPower		= val[2];					// シャープネスの強さ (0 - 5)
	if (count > 3)	canon->SharpnessFrequency	= val[3];					// シャープネスの対象
	// val[4] => 'SensorRedLevel'
	// val[5] => 'SensorBlueLevel'
	// val[6] => 'WhiteBalanceRed'
	// val[7] => 'WhiteBalanceBlue'
	if (count > 8 && val[8] >= 0)
					canon->WhiteBalance			= val[8];					// ホワイトバランス
	if (count > 9)	canon->ColorTemp			= val[9];					// 色温度
	if (count > 10)	canon->ColorMatrix			= val[10];					// カラーマトリクス or ピクチャースタイル
	if (count > 11)	canon->DigitalGain			= val[11];					// DigitalGain
	if (count > 12)	canon->WBBlueAmber			= val[12];					// WB補正 B/A(ブルー/アンバー)
	if (count > 13)	canon->WBMagentaGreen		= val[13];					// WB補正 M/G(マゼンタ/グリーン)

	delete [] val;
}

void rawmap::makernote_canon_pictureinfo(int count)
{
	// Makernote tag 0x12
	if (count < 1) return;

	register canon_t* canon = &m_exif.canon;
	for (int i = 0; i < count; i++) {
		unsigned short val = (unsigned short)fget2();

		switch (i) {
		case  2:								// イメージ幅
			canon->ImageWidth = val;
			break;
		case  3: break;							// イメージ高さ
			canon->ImageHeight = val;
			break;
//@		case  4: break;							// 総イメージ幅
//@		case  5: break;							// 総イメージ高さ
		case 22:								// AFPointsUsed
			val &= 0xff00;
			if (val != 0xff00)
				canon->AFPointUsed = val;
			break;
		case 26:								// AFPointsUsed for EOS20D,30D
			switch (m_exif.ModelID) {
			case CANON_EOS_KISSX:
			case CANON_EOS_KISSX2:
			case CANON_EOS_KISSX3:
			case CANON_EOS_KISSF:
			case CANON_EOS_20D:
			case CANON_EOS_30D:
			case CANON_EOS_40D:
			case CANON_EOS_50D:
				canon->AFPointUsed = val;
				break;
			}
			break;
		default:
//@	dbg.printf(L"tag=0x12 pictureinfo %02d: %d\n", i, val);
			break;
		}
	}
}

void rawmap::makernote_canon_1D_block(int count)
{
	// Makernote tag 0x1D
	if (count < 1) return;

	register canon_t* canon = &m_exif.canon;
	for (int i = 0; i < count; i++) {
		short val = (short)fget2();
		switch (i) {
		case 2:								// マイカラー
			canon->MyColor = val;
			break;
		case 3:								// 赤調整
			canon->RedLevel = val;
			break;
		case 4:								// 緑調整
			canon->GreenLevel = val;
			break;
		case 5:								// 青調整
			canon->BlueLevel = val;
			break;
		case 6:								// 肌色調整
			canon->SkinLevel = val;
			break;
		case 7:								// 色の範囲（ワンポイントカラーまたはスイッチカラー時）
			canon->ColorRange = val;
			break;
		default:
//@	dbg.printf(L"tag=0x1d index=%02d val=%04X\n", i, val);
			break;
		}
	}
}

void rawmap::makernote_canon_1F_block(int count)
{
	// Makernote tag 0x1F
	if (count < 1) return;

	register canon_t* canon = &m_exif.canon;
	unsigned short* v = new unsigned short[count];
	if (v) {
		for (int i = 0; i < count; i++) v[i] = (short)fget2();

		if (count > 13)
			canon->RetouchMyColor = v[13];								// レタッチマイカラー

		delete [] v;
	}
}

void rawmap::makernote_canon_fileinfo(int count)
{
	// Makernote tag 0x93
	if (count < 1) return;

	register canon_t* canon = &m_exif.canon;
	unsigned short* v = new unsigned short[count];

	for (int i = 0; i < count; i++) v[i] = (short)fget2();

	if (count > 2 && IS_CANON_EOS(m_exif.ModelID)) {
		ULONG num = ((m_order == 0x4949) ? ((v[2] << 16) + v[1]) : ((v[1] << 16) + v[2]));
		if (IS_CANON_SHUTTERCOUNT(m_exif.ModelID)) {
			if (m_exif.ModelID == CANON_EOS_30D ||
				m_exif.ModelID == CANON_EOS_40D ||
				m_exif.ModelID == CANON_EOS_KISSX ||
				m_exif.ModelID == CANON_EOS_KISSX2 ||
				m_exif.ModelID == CANON_EOS_KISSF) {
				// The bit pattern for the 30D is: (see 20D notes above for more information)
				//  31....24 23....16 15.....8 7......0
				//  00000000 ffff0000 ddddddFF FFFFFFFF
				// [NOTE: the 4 high order directory bits don't appear in this record, but
				// I have chosen to write them into bits 16-19 since these 4 zero bits look
				// very suspicious, and are a convenient place to store this information - PH]
				ULONG d = (num & 0xffc00) >> 10;
				do {
					d += 0x40;
				} while (d < 100);
				canon->FiShutterCount = d * 10000 + ((num & 0x3ff) << 4) + ((num >> 20) & 0x0f);
			}
			else {
				// シャッター回数
				canon->FiShutterCount = (v[1] << 16) + v[2];
			}
		}
		else if (v[1] && v[2]) {
			// The bit pattern of the 32-bit word is:
			//   31....24 23....16 15.....8 7......0
			//   00000000 ffffffff DDDDDDDD ddFFFFFF
			//     0 = zero bits (not part of the file number?)
			//     f/F = low/high bits of file number
			//     d/D = low/high bits of directory number
			// The directory and file number are then converted into decimal
			// and separated by a '-' to give the file number used in the 20D
			canon->FiShutterCount = ((num & 0xffc0) >> 6) * 10000 + ((num >> 16) & 0xff) + ((num & 0x3f) << 8);
		}
	}
	if (count > 3)	canon->FiBracketMode		= v[3];							// ブラケットモード
	if (count > 4)	canon->FiBracketValue		= v[4];							// ブラケット値
	if (count > 5)	canon->FiBracketShotNumber	= v[5];							// ブラケット撮影番号
	// va[6]: RAW/JPEG 品質
	// va[7]: RAW/JPEG サイズ
	if (count > 8)	canon->FiNoiseReduction		= (int)(short)v[8];				// ノイズリダクション
	if (count > 9)	canon->FiWBBracketMode		= v[9];							// WBブラケティングモード
	// va[10]: unknown
	// va[11]: unknown
	if (count > 12)	canon->FiWBBracketBlueAmber	= (int)(short)v[12];			// WBブラケティング B/A
	if (count > 13)	canon->FiWBBracketMagentaGreen = (int)(short)v[13];			// WBブラケティング M/G
	if (count > 14)	canon->FiBWFilterEffect		= (int)(short)v[14];			// フィルター効果
	if (count > 15)	canon->FiBWColorFilter		= (int)(short)v[15];			// 調色
	// va[16]: unknown
	// va[17]: unknown
	// va[18]: unknown
	// va[19]: unknown
	// va[20]: unknown
	// va[21]: unknown
	// va[22]: unknown
	// va[23]: unknown

	delete [] v;
}

void rawmap::makernote_canon_AA_block(int count)
{
	// Makenote tag 0xAA
	if (count < 1) return;

#if 0
	register canon_t* canon = &m_exif.canon;
	for (int i = 0; i < count; i++) {
		short val = (short)fget2();
		switch (i) {
		default:
//@ dbg.printf(L"%02d: %04X\n", i, val);
			break;
		}
	}
#endif
}

void rawmap::makernote_canon_deviceinfo(int count)
{
	// Makenote tag 0xe0
	if (count < 1) return;

	register canon_t* canon = &m_exif.canon;
	for (int i = 0; i < count; i++) {
		short val = (short)fget2();
		switch (i) {
		case 1: break;						// CMOS横総画素数
		case 2: break;						// CMOS縦総画素数
		case 7: break;						// CMOS横有効画素数
		case 8: break;						// CMOS縦有効画素数
		default:
// dbg.printf(L"%02d: %04X\n", i, val);
			break;
		}
	}
}

void rawmap::makernote_canon_block_dump(int tag, int count)
{
	if (count < 1) return;

#if 0
	register canon_t* canon = &m_exif.canon;
	for (int i = 0; i < count; i++) {
		short val = (short)fget2();
		switch (i) {
		default:
//@	dbg.printf(L"tag=0x%02X index=%02d val=%04X\n", tag, i, val);
			break;
		}
	}
#endif
}

void rawmap::makernote_canonraw_imageinfo(int count)
{
	if (count < 1) return;

	register canon_t* canon = &m_exif.canon;
	for (int i = 0; i < count; i++) {
		long val = (long)fget4();
		switch (i) {
		case 0:								// ImageWidth
			canon->ImageWidth = val;
			break;
		case 1:								// ImageHeight
			canon->ImageHeight = val;
			break;
		case 2:								// PixelAspectRatio,  Format=float
			break;
		case 3:								// Rotation
			break;
		case 4:								// ComponentBitDepth
			break;
		case 5:								// ColorBitDepth
			break;
		case 6:								// ColorBW
			break;
		default:
//@ dbg.printf(L"%02d: %08X\n", i, val);
			break;
		}
	}
}

void rawmap::makernote_canon()
{
	// The MakerNote might have its own TIFF header (possibly with
	// its own byte-order!), or it might just be a table.

	register canon_t* canon = &m_exif.canon;
	_canon_init(canon);

	int base = tell();
	int entries = fget2();

	if (entries >= 0x100)		// FaxViewerなどで書き換えられると IIオーダーがMMオーダーになってデータ位置がずれてしまう
		return;

	while (entries--) {
		if (eof()) break;

		int save = tell();
		int tag  = fget2();
		int type = fget2();
		int count= fget4();

		if (count < 0 || !tiff_seek(type, count)) {
			// このタグはデータ不全なのでスキップ
			seek(save + 12, SEEK_SET);
			continue;
		}

		switch (tag) {
		case 0x0001:
			makernote_canon_settinginfo(count);
			break;
		case 0x0002:							// ciffの0x5029レコードと同一
			makernote_canon_focalinfo(count);
			break;
//@		case 0x0003: break;						// フラッシュ情報ブロック?
		case 0x0004:							// 露出情報
			makernote_canon_shotinfo(count);
			break;
		case 0x0005:							// パノラマ情報ブロック
			makernote_canon_panoramainfo(count);
			break;
		case 0x0006:							// ASCII: イメージの種類
			read_ascii(canon->ImageType, count, ARRAYSIZE(canon->ImageType));
			break;
		case 0x0007:							// ASCII: ファームウェア
			read_ascii(canon->FirmWare, count, ARRAYSIZE(canon->FirmWare));
			break;
		case 0x0008:							// LONG: ファイル番号
			canon->ShotNumber = (int)get_tagvalue(type);
			break;
		case 0x0009:							// ASCII: 所有者名
			read_ascii(canon->OwnerName, count, ARRAYSIZE(canon->OwnerName));
			break;
		case 0x000C:							// LONG: シリアルナンバー
			canon->SerialNumber = (int)get_tagvalue(type);
			break;
		case 0x000D:							// CameraInfoブロック
			makernote_canon_camerainfo(count);
			break;
//@		case 0x000F: break;						// カスタムファンクション
//@		case 0x0010: break;						// ULONG: カメラモデルID
		case 0x0012:							// PictureInfoブロック ??
			makernote_canon_pictureinfo(count);
			break;
//		case 0x0013: break;						// USHRTxN: ??ブロック
//		case 0x0018: break;						// BYTExN: ??
//		case 0x0019: break;						// USHRT:
//		case 0x001C: break;						// USHRT:
		case 0x001D:							// USHRTxN:
			makernote_canon_1D_block(count);
			break;
//		case 0x001E: break;						// ULONG:
		case 0x001F:							// USHRTxN:
			makernote_canon_1F_block(count);
			break;
		case 0x0093:							// フィルタ情報?
			makernote_canon_fileinfo(count);
			break;
//		case 0x0094:							// AFポイント使用(1D)
//			canon->AFPointUsed = (int)get_tagvalue(type);
//			break;
		case 0x0095:							// 使用レンズ
			read_ascii(canon->LensName, count, ARRAYSIZE(canon->LensName));
			break;
		case 0x00A0:							// カラー情報
			makernote_canon_processinginfo(count);
			break;
//@		case 0x00AA:							// ?? パラメタブロック
//@			makernote_canon_AA_block(count);
//@			break;
		case 0x00B4:							// 現像パラメーター
			canon->DevParam = (int)get_tagvalue(type);
			break;
//@		case 0x00E0:							// CCD/CMOS情報ブロック ??
//@			makernote_canon_deviceinfo(count);
//@			break;
//@		case 0x4003: break;						// ??情報ブロック (20Dではすべて0だった)
		default:
//@ tagdump(this, tag, type, count);
			break;
		}
		seek(save + 12, SEEK_SET);
	}
}

void rawmap::makernote_olympus_2010()
{
	// Equipment 情報ブロック
	register olympus_t* olympus = &m_exif.olympus;

	int entries = fget2();
	while (entries--) {
		if (eof()) break;

		int save = tell();
		int tag  = fget2();
		int type = fget2();
		int count= fget4();

		if (count < 0 || !tiff_seek(type, count)) {
			// このタグはデータ不全なのでスキップ
			seek(save + 12, SEEK_SET);
			continue;
		}

		switch (tag) {
		case 0x0000: break;						// UNDEFx4: EquipmentVersion
		case 0x0101:							// ASCII: SerialNumber
			read_ascii(olympus->SerialNumber, count, ARRAYSIZE(olympus->SerialNumber));
			break;
		case 0x0102:
			read_ascii(olympus->IntSerialNumber, count, ARRAYSIZE(olympus->IntSerialNumber));
			break;
//		case 0x0103: break;						// URATION: 撮像面の対角サイズ output to "<value> mm"
		case 0x0104:							// ULONG: BodyFirmwareVersion
			olympus->BodyFirmwareVersion = (int)get_tagvalue(type);
			break;
		case 0x0201:							// BYTEx6: Lens -- 1. Make, 2. Unknown, 3. Model, 4. Release, 5-6. Unknown
			olympus->LensInfo = getmsblong();
			break;
		case 0x0202:							// ASCII: LensSerialNumber
			read_ascii(olympus->LensSerialNumber, count, ARRAYSIZE(olympus->LensSerialNumber));
			break;
		case 0x0204:							// ULONG: LensFirmwareVersion
			olympus->LensFirmwareVersion = (int)get_tagvalue(type);
			break;
		case 0x0205:							// USHORT: 最小焦点距離でのレンズ開放値
			olympus->MaxApertureAtMinFocal = (int)get_tagvalue(type);
			break;
		case 0x0206:							// USHORT: 最大焦点距離でのレンズ開放値
			olympus->MaxApertureAtMaxFocal = (int)get_tagvalue(type);
			break;
		case 0x0207:							// USHORT: 最小焦点距離(mm)
			olympus->MinFocalLength = (int)get_tagvalue(type);
			break;
		case 0x0208:							// USHORT: 最大焦点距離(mm)
			olympus->MaxFocalLength = (int)get_tagvalue(type);
			break;
		case 0x020A:							// USHRT: 実焦点でのレンズ開放F値
			olympus->MaxApertureAtCurrentFocal = (int)get_tagvalue(type);
			break;
//		case 0x020B:							// USHRT: レンズプロパティ
		case 0x0301:							// BYTEx6: エクステンダー識別コード -- 1. Make, 2. Unknown, 3. Model, 4. Release, 5-6. Unknown
			olympus->ExtenderInfo = getmsblong();
			break;
		case 0x0302:							// ASCII: エクステンダーシリアルナンバー
			read_ascii(olympus->ExtenderSerialNumber, count, ARRAYSIZE(olympus->ExtenderSerialNumber));
			break;
		case 0x0303:							// ASCII: エクステンダー名
			read_ascii(olympus->ExtenderModel, count, ARRAYSIZE(olympus->ExtenderModel));
			break;
		case 0x0304:							// ULONG: エクステンダーファームウェアバージョン
			olympus->ExtenderFirmwareVersion = (int)get_tagvalue(type);
			break;
		case 0x1000:							// USHORT: フラッシュシステム
														// 0=None
														// 2=Simple E-System
														// 3=E-System
			olympus->FlashSystem = (int)get_tagvalue(type);
			break;
		case 0x1001:							// USHORT: FlashModel
														// 0=None
														// 1=FL-20
														// 2=FL-50
														// 3=RF-11
														// 4=TF-22
														// 5=FL-36
			olympus->FlashModelInfo = (int)get_tagvalue(type);
			break;

//@		case 0x1002: break;						// ULONG: FlashFirmwareVersion
//@			val = sprintf("%x",$val); val = ~s/(.{3})$/\.$1/;$val
		case 0x1003:							// ASCII: FlashSerialNumber
			read_ascii(olympus->FlashModelSerialNumber, count, ARRAYSIZE(olympus->FlashModelSerialNumber));
			break;
		case 0x1004:							// ULONG: FlashFirmwareVersion
			olympus->FlashModelFirmwareVersion = (int)get_tagvalue(type);
			break;
		default:
//@ tagdump(this, tag, type, count);
			break;
		}
		seek(save + 12, SEEK_SET);
	}
}

void rawmap::makernote_olympus_2020()
{
	// CameraSettings 情報ブロック
	register olympus_t* olympus = &m_exif.olympus;

	_imageinfo_t* pimage = NULL;
	int entries = fget2();
	while (entries--) {
		if (eof()) break;

		int save = tell();
		int tag  = fget2();
		int type = fget2();
		int count= fget4();

		if (count < 0 || !tiff_seek(type, count)) {
			// このタグはデータ不全なのでスキップ
			seek(save + 12, SEEK_SET);
			continue;
		}

		switch (tag) {
		case 0x0000: break;						// UNDEFx4: CameraSettingsVersion
		case 0x0100: break;						// ULONG: PreviewImageValid 0='No', 1='Yes'
		case 0x0101:							// サムネイル画像位置
			if (!pimage) pimage = create_ifitem();
			pimage->thumb_offset = fget4();
			if (olympus->ExifDataType)			// type2（E410など）はベース位置が変わる
				pimage->thumb_offset += m_tiffbase;
			seek(pimage->thumb_offset, SEEK_SET);
			if ((USHORT)getmsbshort() == 0xffd8) {
				pimage->compression = 6;
				jpg_getsize(pimage->thumb_offset, &pimage->width, &pimage->height);
			}
			break;
		case 0x0102:							// サムネイルデータサイズ
			if (!pimage) pimage = create_ifitem();
			pimage->thumb_length = fget4();
			break;
		case 0x0200:							// USHORT: ExposureMode
			olympus->exExposureMode = (int)get_tagvalue(type);
			break;
		case 0x0201:							// USHRT: AELock
														// 0 => 'Off',
														// 1 => 'On',
			olympus->exAELock = (int)get_tagvalue(type);
			break;
		case 0x0202:							// USHORT: MeteringMode
			olympus->exMeteringMode = (int)get_tagvalue(type);
			break;
		case 0x0300:							// USHORT: MacroMode
														// 0=Off
														// 1=On
			olympus->exMacroMode = (int)get_tagvalue(type);
			break;
		case 0x0301:							// USHORT: FocusMode
														// 0=Single AF
														// 1=Sequential shooting AF
														// 2=Continuous AF
														// 3=Multi AF
														// 10=MF
			olympus->exFocusMode = (int)get_tagvalue(type);
			break;
		case 0x0302: break;						// USHORT: FocusProcess
														// 0=AF Not Used
														// 1=AF Used
		case 0x0303:							// USHORT: AFSearch
														// 0=Not Ready
														// 1=Ready
			olympus->exAFSearch = (int)get_tagvalue(type);
			break;
		case 0x0304:							// ULONGx64: AFAreas
//@			olympus->exAFArea = (int)get_tagvalue(type);	// 最初の1要素のみ
			olympus->exAFArea = fget4();					// 最初の1要素のみ
			break;
		case 0x0400: 							// USHORT: FlashMode
														// 0 => 'Off'
														// BITMASK => 0 => 'On',
														//            1 => 'Fill-in',
														//            2 => 'Red-eye',
														//            3 => 'Slow-sync',
														//            4 => 'Forced On',
														//            5 => '2nd Curtain',
			olympus->exFlashMode = (int)get_tagvalue(type);
			break;
		case 0x0401:							// SRATIN: FlashExposureCompensation
			olympus->exFlashExposureComp = (float)get_tagvalue(type);
			break;
		case 0x0403:							// USHRT: フラッシュリモート制御 （E-3以降？）
														// 0=Off
														// Bit4..3 -> Comm. Light Level:
														//            00=Low
														//            01=Middle
														//            10=High
														// Bit2..0 -> Channel (1..4)
			olympus->exFlashRemoteControl = (int)get_tagvalue(type);
			break;
		case 0x0404:							// USHRTx3: フラッシュ制御モード（E-3以降？）
														// 0=Off
														// 3=TTL
														// 4=Auto
														// 5=Manual
			olympus->exFlashControlMode[0] = (int)get_tagvalue(type);
			olympus->exFlashControlMode[1] = (int)get_tagvalue(type);
			olympus->exFlashControlMode[2] = (int)get_tagvalue(type);
			break;
		case 0x0405:							// SRATINx3: TTL
														// -5 ～ +5
			olympus->exFlashIntensityTTL[0] = (float)get_tagvalue(type);
			olympus->exFlashIntensityTTL[1] = (float)get_tagvalue(type);
			olympus->exFlashIntensityTTL[2] = (float)get_tagvalue(type);
			break;
		case 0x0406:							// SRATINx3: Value --- 1/1 ～ 1/128
			olympus->exFlashIntensityValue[0] = (float)get_tagvalue(type);
			olympus->exFlashIntensityValue[1] = (float)get_tagvalue(type);
			olympus->exFlashIntensityValue[2] = (float)get_tagvalue(type);
			break;
		case 0x0500:							// USHRT: ホワイトバランス
			olympus->exWhiteBalance = (int)get_tagvalue(type);
			break;
		case 0x0501:							// USHORT: ホワイトバランス色温度
			olympus->exWBTemperature = (int)get_tagvalue(type);
			break;
		case 0x0502:							// SSHRTx2: ホワイトバランスブラケット
			olympus->exWhiteBalanceBracket[0] = (int)get_tagvalue(type);
			olympus->exWhiteBalanceBracket[1] = (int)get_tagvalue(type);
			break;
		case 0x0503:							// SSHRTx3: SaturationSetting
			olympus->exSaturationSetting[0] = (int)get_tagvalue(type);
			olympus->exSaturationSetting[1] = (int)get_tagvalue(type);
			olympus->exSaturationSetting[2] = (int)get_tagvalue(type);
			break;
		case 0x0504:							// USHORT: ModifiedSaturation
			olympus->exModifiedSaturation = (int)get_tagvalue(type);
			break;
		case 0x0505:							// SHORTx3: ContrastSetting
			olympus->exContrastSetting[0] = (int)get_tagvalue(type);
			olympus->exContrastSetting[1] = (int)get_tagvalue(type);
			olympus->exContrastSetting[2] = (int)get_tagvalue(type);
			break;
		case 0x0506:							// SHORTx3: SharpnessSetting
			olympus->exSharpnessSetting[0] = (int)get_tagvalue(type);
			olympus->exSharpnessSetting[1] = (int)get_tagvalue(type);
			olympus->exSharpnessSetting[2] = (int)get_tagvalue(type);
			break;
		case 0x0507:							// USHORT: ColorSpace
			olympus->exColorSpace = (int)get_tagvalue(type);
			break;
		case 0x0509:							// USHORT: SceneMode
			olympus->exSceneMode = (int)get_tagvalue(type);
			break;
		case 0x050A:							// USHORT: NoiseReduction
			olympus->exNoiseReduction = (int)get_tagvalue(type);
			break;
//		case 0x050B: break;						// USHORT: DistortionCorrection
														// Off or On
		case 0x050C:							// USHORT: ShadingCompensation
														// Off or On
			olympus->exShadingComp = (int)get_tagvalue(type);
			break;
		case 0x050D:							// URATION: CompressionFactor
			olympus->exCompressionFactor = (float)get_tagvalue(type);
			break;
		case 0x050F:							// SHORT: Gradation
														// -1=Low Key
														//  0=Normal
														//  1=High Key
			olympus->exGradation = (int)(short)get_tagvalue(type);
			break;
		case 0x0520:							// USHRT: 仕上がりモード
			olympus->exPictureMode = (int)(short)get_tagvalue(type);
			break;
		// 仕上がりモードが vivid, natural, flat の場合の有効TAG 0x0521, 0x0523, 0x0524
		// 仕上がりモードが mono, sepiaの場合の有効TAG 0x0523, 0x0524, 0x525, 0x526
		case 0x0521:							// SSHRTx3: 仕上がりモード：彩度 (val, min, max)
			olympus->exPicModeSaturation = (int)(short)get_tagvalue(type);
			break;
		case 0x0522: break;						// USHRT: 仕上がりモード：色相
		case 0x0523:							// SSHRTx3: 仕上がりモード：コントラスト (val, min, max)
			olympus->exPicModeContrast = (int)(short)get_tagvalue(type);
			break;
		case 0x0524:							// SSHRTx3: 仕上がりモード：シャープネス (val, min, max)
			olympus->exPicModeSharpness = (int)(short)get_tagvalue(type);
			break;
		case 0x0525:							// SSHRT: 仕上がりモード：フィルター効果
			olympus->exPicModeFilterEffect = (int)(short)get_tagvalue(type);
			break;
		case 0x0526:							// SSHRT: 仕上がりモード：調色
			olympus->exPicModeTone = (int)(short)get_tagvalue(type);
			break;
		case 0x0527:							// ノイズフィルタ
			olympus->exNoiseFilter = (int)get_tagvalue(type);
			break;
		case 0x0600:							// USHORTx2 or 3: Sequence
			olympus->exShotSequence[0] = (int)get_tagvalue(type);
			olympus->exShotSequence[1] = (int)get_tagvalue(type);
			olympus->exShotSequence[2] = (count > 2) ? (int)get_tagvalue(type) : 0;
			break;
		case 0x0601:							// USHRT: パノラマモード
			olympus->exPanoramaMode =  (int)get_tagvalue(type);
			break;
//		case 0x0603: break;						// USHORT: ImageQuality2
		case 0x0900:							// USHRT: 気圧(hPa)
			olympus->ManometerPressure = (int)get_tagvalue(type);
			break;
		case 0x0901:							// SLONGx2: 標高・水深
			olympus->ManometerReading[0] = (int)get_tagvalue(type);			// meter
			olympus->ManometerReading[1] = (int)get_tagvalue(type);			// feet
			break;
		case 0x0902:							// USHRT:
			olympus->ExtendedWBDetect = (int)get_tagvalue(type);
			break;

		default:
//@ tagdump(this, tag, type, count);
			break;
		}
		seek(save + 12, SEEK_SET);
	}
}

void rawmap::makernote_olympus_2030()
{
	// RawDevelopment 情報ブロック
	register olympus_t* olympus = &m_exif.olympus;

	int entries = fget2();
	while (entries--) {
		if (eof()) break;

		int save = tell();
		int tag  = fget2();
		int type = fget2();
		int count= fget4();

		if (count < 0 || !tiff_seek(type, count)) {
			// このタグはデータ不全なのでスキップ
			seek(save + 12, SEEK_SET);
			continue;
		}

		switch (tag) {
/* @@
		case 0x0000: break;						// UNDEFx7: RawDevVersion
		case 0x0100: break;						// SRTNL: RawDevExposureBiasValue
		case 0x0101: break;						// USHRT: RawDevWhiteBalanceValue
		case 0x0102: break;						// SSHRT: RawDevWBFineAdjustment
		case 0x0103: break;						// USHRTx3: RawDevGrayPoint
		case 0x0104: break;						// SSHRTx3: RawDevSaturationEmphasis
		case 0x0105: break;						// USHRT: RawDevMemoryColorEmphasis
		case 0x0106: break;						// SSHRTx3: RawDevContrastValue
		case 0x0107: break;						// SSHRTx3: RawDevSharpnessValue
		case 0x0108: break;						// USHRT: RawDevColorSpace
		case 0x0109: break;						// USHRT: RawDevEngine
		case 0x010A: break;						// USHRT: RawDevNoiseReduction
		case 0x010B: break;						// USHRT: RawDevEditStatus
		case 0x010C: break;						// USHRT: RawDevSettings
@@ */
		default:
//@ tagdump(this, tag, type, count);
			break;
		}
		seek(save + 12, SEEK_SET);
	}
}

void rawmap::makernote_olympus_2031()
{
	// RawDevelopment2 情報ブロック
	register olympus_t* olympus = &m_exif.olympus;

	int entries = fget2();
	while (entries--) {
		if (eof()) break;

		int save = tell();
		int tag  = fget2();
		int type = fget2();
		int count= fget4();

		if (count < 0 || !tiff_seek(type, count)) {
			// このタグはデータ不全なのでスキップ
			seek(save + 12, SEEK_SET);
			continue;
		}

		switch (tag) {
		case 0x0102:							// USHRT: Raw編集 ホワイトバランス値
			olympus->RawWhiteBalance = (int)get_tagvalue(type);
			break;
		case 0x0103:							// SSHRT: Raw編集 ホワイトバランス微調整
			olympus->RawWhiteBalanceAdjust = (int)get_tagvalue(type);
			break;
		case 0x0105:							// SSHRTx3: Raw編集 コントラスト調整(-5～+5)
			olympus->RawContrastAdjust[0] = (int)get_tagvalue(type);
			olympus->RawContrastAdjust[1] = (int)get_tagvalue(type);
			olympus->RawContrastAdjust[2] = (int)get_tagvalue(type);
			break;
		case 0x0106:							// SSHRTx3: Raw編集 シャープネス調整(-5～+5)
			olympus->RawSharpnessAdjust[0] = (int)get_tagvalue(type);
			olympus->RawSharpnessAdjust[1] = (int)get_tagvalue(type);
			olympus->RawSharpnessAdjust[2] = (int)get_tagvalue(type);
			break;
		case 0x0107:							// SSHRTx3: Raw編集 彩度調整(-5～+5)
			olympus->RawSaturationAdjust[0] = (int)get_tagvalue(type);
			olympus->RawSaturationAdjust[1] = (int)get_tagvalue(type);
			olympus->RawSaturationAdjust[2] = (int)get_tagvalue(type);
			break;
		case 0x0109:							// USHRT: Raw編集 カラー設定(sRGB, AdobeRGB, ProPhotoRGB)
			olympus->RawColorSpace = (int)get_tagvalue(type);
			break;
		default:
//@ tagdump(this, tag, type, count);
			break;
		}
		seek(save + 12, SEEK_SET);
	}
}

void rawmap::makernote_olympus_2040()
{
	// ImageProcessing 情報ブロック
	register olympus_t* olympus = &m_exif.olympus;

	int entries = fget2();
	while (entries--) {
		if (eof()) break;

		int save = tell();
		int tag  = fget2();
		int type = fget2();
		int count= fget4();

		if (count < 0 || !tiff_seek(type, count)) {
			// このタグはデータ不全なのでスキップ
			seek(save + 12, SEEK_SET);
			continue;
		}

		switch (tag) {
/*
		case 0x0000: break;						// UNDEFx7: ImageProcessingVersion
		case 0x0100: break;						// USHRTx2: WB_RBLevels
		case 0x0200: break;						// USHRTx9: ColorMatrix
		case 0x0300: break;						// SmoothingParameter1
		case 0x0303: break;						// USHRT:
		case 0x0310: break;						// SmoothingParameter2
		case 0x0313: break;						// USHRT:
		case 0x0400: break;						// USHRTx63:
		case 0x0401: break;						// USHRTx5:
		case 0x0402: break;						// USHRTx5:
		case 0x0403: break;						// USHRTx5:
		case 0x0404: break;						// USHRTx5:
		case 0x0405: break;						// USHRTx5:
		case 0x0406: break;						// USHRTx5:
		case 0x0408: break;						// USHRTx5:
		case 0x0420: break;						// SSHRT:
		case 0x0421: break;						// USHRT:
		case 0x0600: break;						// SmoothingThresholds
		case 0x0610: break;						// SmoothingThreshold2
		case 0x0611: break;						// ValidBits
		case 0x0612: break;						// USHRTx2:
		case 0x0613: break;						// USHRTx2:
		case 0x0614: break;						// ULONG: OlympusImageWidth2
		case 0x0615: break;						// ULONG: OlympusImageHeight2
		case 0x0617: break;						// USHRT:
		case 0x0800: break;						// FLOATx9:
		case 0x0801: break;						// USHRT:
		case 0x0802: break;						// RATNL:
		case 0x1003: break;						// USHRT:
		case 0x1004: break;						// USHRT:
		case 0x1005: break;						// USHRTx2:
		case 0x1006: break;						// USHRTx8:
		case 0x1007: break;						// USHRTx5:
		case 0x1008: break;						// USHRT:
		case 0x1009: break;						// USHRTx3:
		case 0x100A: break;						// USHRTx3:

		// # 0x1010-0x1012 are the processing options used in camera or in
		// # Olympus software, which 0x050a-0x050c are in-camera only (ref 6)

		case 0x1010: break;						// USHRT: NoiseReduction2
														// 0 => 'Off',
														// 1 => 'Noise Filter',
														// 2 => 'Noise Reduction',
														// 3 => 'Noise Reduction + Noise Filter', #6
														// 4 => 'Noise Filter (ISO Boost)', #6
														// 5 => 'Noise Reduction + Noise Filter (ISO Boost)', #6
		case 0x1011: break;						// USHRT: DistortionCorrection2
														// 0 => 'Off',
														// 1 => 'On',
		case 0x1012: break;						// USHRT: ShadingCompensation2
														// offOn
		case 0x1200: break;						// ULONGx2:
		case 0x1201: break;						// SSHRTx80:
		case 0x1300: break;						// SSHRTx4:
		case 0x1301: break;						// USHRTx2:
		case 0x1302: break;						// SSHRTx2:
		case 0x1303: break;						// SSHRTx2:
		case 0x1304: break;						// SSHRTx2:
		case 0x1305: break;						// USHRTx4:
		case 0x1400: break;						// SSHRTx3:
		case 0x1401: break;						// USHRTx2:
		case 0x1402: break;						// SSHRTx2:
		case 0x1403: break;						// USHRTx2:
		case 0x1404: break;						// USHRTx2:
		case 0x1405: break;						// USHRTx2:
		case 0x1406: break;						// USHRTx2:
		case 0x1407: break;						// USHRTx2:
*/
		default:
//@ tagdump(this, tag, type, count);
			break;
		}
		seek(save + 12, SEEK_SET);
	}
}

void rawmap::makernote_olympus_2050()
{
	// FocusInfo 情報ブロック
	register olympus_t* olympus = &m_exif.olympus;

	int entries = fget2();
	while (entries--) {
		if (eof()) break;

		int save = tell();
		int tag  = fget2();
		int type = fget2();
		int count= fget4();

		if (count < 0 || !tiff_seek(type, count)) {
			// このタグはデータ不全なのでスキップ
			seek(save + 12, SEEK_SET);
			continue;
		}

		switch (tag) {
/*
		case 0x0000: break;						// UNDEFx4: FocusInfoVersion
		case 0x0200: break;						// SSHRTx2:
		case 0x0201: break;						// SSHRTx2:
		case 0x0202: break;						// SSHRTx2:
		case 0x0203: break;						// SSHRT:
		case 0x0204: break;						// SSHRTx2:
		case 0x0205: break;						// SSHRT:
		case 0x0206: break;						// SSHRT:
		case 0x0207: break;						// USHRT:
		case 0x0209: break;						// USHRT: AutoFocus offOn
		case 0x020A: break;						// USHRT:
		case 0x020B: break;						// USHRT:
		case 0x020C: break;						// SSHRTx2:
		case 0x020D: break;						// ULONG:
		case 0x020E: break;						// ULONGx5:
		case 0x020F: break;						// ULONGx5:
		case 0x0210: break;						// USHRT:
		case 0x0211: break;						// ULONGx8:
		case 0x0212: break;						// ULONGx720:
		case 0x0213: break;						// USHRT:
		case 0x0214: break;						// USHRT:
		case 0x0215: break;						// USHRT:
		case 0x0219: break;						// SSHRTx4:
		case 0x021A: break;						// SSHRTx4:
		case 0x021B: break;						// SSHRTx3:
		case 0x0300: break;						// USHRT: ZoomStepCount
		case 0x0301: break;						// USHRT:
		case 0x0302: break;						// USHRT:
		case 0x0305: break;						// RATNL: FocusDistance
			// this rational value looks like it is in mm when the denominator is 1 (E-1),
			// and cm when denominator is 10 (E-300), so if we ignore the denominator we are consistently in mm
		case 0x0307: break;						// USHRT:
		case 0x0308: break;						// USHRT:
		case 0x0309: break;						// USHRT:
		case 0x030A: break;						// SSHRTx7:
		case 0x030B: break;						// USHRT:
		case 0x030C: break;						// USHRTx2:
		case 0x030D: break;						// USHRTx22:
		case 0x030E: break;						// USHRT:
		case 0x030F: break;						// SSHRTx4:
		case 0x0318: break;						// USHRT:
		case 0x0319: break;						// USHRT:
		case 0x031A: break;						// USHRTx48:
		case 0x031B: break;						// USHRT:
		case 0x0320: break;						// USHRTx16:
		case 0x0325: break;						// USHRTx8:
		case 0x0326: break;						// USHRT:
		// 0x1200-0x1209 Flash information
		case 0x1200: break;						// USHRT:
		case 0x1201: break;						// USHRTx2: ExternalFlash
														// '0 0' => 'Off',
														// '1 0' => 'On',
		case 0x1202: break;						// USHRT:
		case 0x1203: break;						// SRTNL:
		case 0x1204: break;						// USHRT:
		case 0x1205: break;						// RATNL:
		case 0x1206: break;						// USHRT:
		case 0x1207: break;						// USHRT:
		case 0x1208: break;						// USHRT: InternalFlash
														// '0'   => 'Off',
														// '1'   => 'On',
		case 0x1209: break;						// USHRTx2: ManualFlash
														// '2 numbers: 1. 0=Off, 1=On, 2. Flash strength'
		case 0x120A: break;						// USHRT:
		case 0x1500: break;						// SSHRT: SensorTemperature
		case 0x1600: break;						// UNDEFx53:
*/
		default:
//@ tagdump(this, tag, type, count);
			break;
		}
		seek(save + 12, SEEK_SET);
	}
}

void rawmap::makernote_olympus()
{
	register olympus_t* olympus = &m_exif.olympus;

	memset(olympus, 0, sizeof(olympus_t));

	olympus->ShootingMode = INT_MAX;
	olympus->SequenceNumber = INT_MAX;
	olympus->PanoramaDirection = INT_MAX;
	olympus->Quality = INT_MAX;
	olympus->MacroMode = INT_MAX;
	olympus->DigitalZoom = FLT_MAX;
	olympus->OneTouchWB = INT_MAX;
	olympus->FlashMode = INT_MAX;
	olympus->BracketEVBias = FLT_MAX;
	olympus->FocusMode = INT_MAX;
	olympus->ZoomPosition = INT_MAX;
	olympus->Sharpness = INT_MAX;
	olympus->WhiteBalanceMode[0] = INT_MAX;
	olympus->FlashComp = FLT_MAX;
	olympus->Contrast = INT_MAX;
	olympus->WBCompRed[0] = SHRT_MAX;
	olympus->WBCompBlue[0] = SHRT_MAX;
	olympus->ImageWidth = INT_MAX;
	olympus->ImageHeight = INT_MAX;

	// 2010ブロック
	olympus->MaxApertureAtMinFocal = INT_MAX;
	olympus->MaxApertureAtMaxFocal = INT_MAX;
	olympus->MinFocalLength = INT_MAX;
	olympus->MaxFocalLength = INT_MAX;
	olympus->MaxApertureAtCurrentFocal = INT_MAX;
	olympus->BodyFirmwareVersion = INT_MAX;
	olympus->FlashSystem = INT_MAX;
	olympus->LensInfo = INT_MAX;
	olympus->LensFirmwareVersion = INT_MAX;
	olympus->ExtenderInfo = INT_MAX;
	olympus->ExtenderFirmwareVersion = INT_MAX;
	olympus->FlashModelInfo = INT_MAX;
	olympus->FlashModelFirmwareVersion = INT_MAX;

	// 2020ブロック
	olympus->exExposureMode = INT_MAX;
	olympus->exAELock = INT_MAX;
	olympus->exMeteringMode = INT_MAX;
	olympus->exFocusMode = INT_MAX;
	olympus->exAFSearch = INT_MAX;
	olympus->exAFArea = INT_MAX;
	olympus->exFlashMode = INT_MAX;
	olympus->exFlashExposureComp = FLT_MAX;
	olympus->exFlashRemoteControl = INT_MAX;
	for (int i = 0; i < 3; i++) {
		olympus->exFlashControlMode[i] = INT_MAX;
		olympus->exFlashIntensityTTL[i] = FLT_MAX;
		olympus->exFlashIntensityValue[i] = FLT_MAX;
	}
	olympus->exWhiteBalance = INT_MAX;
	olympus->exWBTemperature = INT_MAX;
	olympus->exWhiteBalanceBracket[0] = INT_MAX;
	olympus->exMacroMode = INT_MAX;
	olympus->exSaturationSetting[0] = INT_MAX;
	olympus->exModifiedSaturation = INT_MAX;
	olympus->exContrastSetting[0] = INT_MAX;
	olympus->exSharpnessSetting[0] = INT_MAX;
	olympus->exColorSpace = INT_MAX;
	olympus->exSceneMode = INT_MAX;
	olympus->exNoiseReduction = INT_MAX;
	olympus->exShadingComp = INT_MAX;
	olympus->exCompressionFactor = FLT_MAX;
	olympus->exGradation = INT_MAX;
	olympus->exPictureMode = INT_MAX;
	olympus->exPicModeSaturation = INT_MAX;
	olympus->exPicModeContrast = INT_MAX;
	olympus->exPicModeSharpness = INT_MAX;
	olympus->exPicModeFilterEffect = INT_MAX;
	olympus->exPicModeTone = INT_MAX;
	olympus->exNoiseFilter = INT_MAX;
	olympus->exShotSequence[0] = INT_MAX;
	olympus->exPanoramaMode = INT_MAX;
	olympus->ManometerPressure = INT_MAX;
	olympus->ManometerReading[0] = INT_MAX;
	olympus->ExtendedWBDetect = INT_MAX;

	// 2030ブロック
	olympus->RawExposureComp = FLT_MAX;
	olympus->RawWhiteBalance = INT_MAX;
	olympus->RawWhiteBalanceAdjust = INT_MAX;
	olympus->RawGrayValue[0] = INT_MAX;
	olympus->RawSaturationAdjust[0] = INT_MAX;
	olympus->RawContrastAdjust[0] = INT_MAX;
	olympus->RawSharpnessAdjust[0] = INT_MAX;
	olympus->RawColorSpace = INT_MAX;
	olympus->RawNoiseCancel = INT_MAX;

	char buf[32];
	read(buf, 8);
	if (memcmp(buf, "OLYMP", 5)) {
		m_exif.exist_makenote = false;
		return;
	}
	int stiffbase = m_tiffbase;
	short sorder = m_order;
	if (!memcmp(buf, "OLYMPUS\0", 8)) {
		// ベース位置は、メーカーノートヘッダ先頭となる
		m_tiffbase = tell() - 8;

		// type2（例えばE-410など）は、tag 0x2010,0x2020,0x2030,0x2040,0x2050 など
		// tagtypeが13(0x0d)という規格に無いタイプ値を持っている。
		// この値の場合、大概countは1で、内容はm_tiffbaseからのオフセット値を指していると
		// 思われる
		olympus->ExifDataType = 1;

		m_order = fget2();
		fget2();
	}

	_imageinfo_t* pimage = NULL;
	int entries = fget2();
	while (entries--) {
		if (eof()) break;

		int save = tell();
		int tag  = fget2();
		int type = fget2();
		int count= fget4();

		if (count < 0 || !tiff_seek(type, count)) {
			// このタグはデータ不全なのでスキップ
			seek(save + 12, SEEK_SET);
			continue;
		}

		switch (tag) {
		case 0x0100:								// サムネイル情報
			if ((USHORT)getmsbshort() == 0xffd8) {
				pimage = create_ifitem();
				pimage->thumb_offset = tell() - 2;
				pimage->thumb_length = count;
				pimage->compression = 6;
				jpg_getsize(pimage->thumb_offset, &pimage->width, &pimage->height);
				pimage = NULL;
			}
			break;
		case 0x0200:								// LONGx3:撮影モード
			olympus->ShootingMode = (int)get_tagvalue(type);
			if (count > 2) olympus->SequenceNumber = (int)get_tagvalue(type);
			if (count > 2) olympus->PanoramaDirection = (int)get_tagvalue(type);
			break;
		case 0x0201:								// SHORT:画質モード
			olympus->Quality = (int)get_tagvalue(type);
			break;
		case 0x0202:								// SHORT:マクロモード
			olympus->MacroMode = (int)get_tagvalue(type);
			break;
		case 0x0204:								// RATIN:デジタルズーム
			olympus->DigitalZoom = (float)get_tagvalue(type);
			break;
		case 0x0207:								// ASCII:ハードウェアタイプ
			read_ascii(olympus->HardwareType, count, ARRAYSIZE(olympus->HardwareType));
			break;
		case 0x0208:								// ASCII:ピクチャ情報
			read_ascii(olympus->PictInfo, count, ARRAYSIZE(olympus->PictInfo));
			break;
		case 0x0209:								// UNDEF:カメラID
			read_ascii(olympus->CameraID, count, ARRAYSIZE(olympus->CameraID));
			break;
		case 0x0302:								// USHRT:ワンタッチWB
			olympus->OneTouchWB = (int)get_tagvalue(type);
			break;
		case 0x0404:								// ASCII:シリアル番号
			// 0x101A も同様
			read_ascii(olympus->SerialNumber, count, ARRAYSIZE(olympus->SerialNumber));
			break;
//		case 0x1004:								// SHORT:フラッシュモード
//			olympus->FlashMode = (int)get_tagvalue(type);
//			break;
		case 0x1006:								// RATIN:ブラケットEV補正値
			olympus->BracketEVBias = (float)get_tagvalue(type);
			break;
		case 0x100B:								// SHORT:フォーカスモード
			olympus->FocusMode = (int)get_tagvalue(type);
			break;
//		case 0x100C:								// FocusDistance
//			break;
		case 0x100D:								// SHORT:ズーム位置
			olympus->ZoomPosition = (int)get_tagvalue(type);
			break;
		case 0x100F:								// SHORT:シャープネス
			olympus->Sharpness = (int)get_tagvalue(type);
			break;
		case 0x101A:								// ASCII:シリアル番号
			read_ascii(olympus->SerialNumber, count, ARRAYSIZE(olympus->SerialNumber));
			break;
		case 0x1015:								// SHORTx2:ホワイトバランス
			olympus->WhiteBalanceMode[0] = (int)get_tagvalue(type);
			olympus->WhiteBalanceMode[1] = (int)get_tagvalue(type);
			break;
		case 0x1017:								// USHORTx2:赤WB補正値
			olympus->WBCompRed[0] = (short)get_tagvalue(type);
			olympus->WBCompRed[1] = (short)get_tagvalue(type);
			break;
		case 0x1018:								// USHORTx2:青WB補正値
			olympus->WBCompBlue[0] = (short)get_tagvalue(type);
			olympus->WBCompBlue[1] = (short)get_tagvalue(type);
			break;
		case 0x1023:								// SRATIN:フラッシュ発光量補正
			olympus->FlashComp = (float)get_tagvalue(type);
			break;
		case 0x1029:								// SHORT:コントラスト
			olympus->Contrast = (int)get_tagvalue(type);
			break;
		case 0x102E:								// ULONG:画像幅
			olympus->ImageWidth = (int)get_tagvalue(type);
			break;
		case 0x102F:								// ULONG:画像高さ
			olympus->ImageHeight = (int)get_tagvalue(type);
			break;
		case 0x1034:								// RATNL:JPEG圧縮率(E-20のみ?)
			olympus->exCompressionFactor = (float)get_tagvalue(type);
			break;

		case 0x1036:								// サムネイル画像位置
			if (!pimage) pimage = create_ifitem();
			pimage->thumb_offset = fget4();
			seek(pimage->thumb_offset, SEEK_SET);
			if ((USHORT)getmsbshort() == 0xffd8) {
				pimage->compression = 6;
				jpg_getsize(pimage->thumb_offset, &pimage->width, &pimage->height);
			}
			break;
		case 0x1037:								// サムネイルデータサイズ
			if (!pimage) pimage = create_ifitem();
			pimage->thumb_length = fget4();
			break;
		case 0x2010:								// Equipment 情報ブロック
			if (olympus->ExifDataType)
				seek(m_tiffbase + fget4(), SEEK_SET);
			makernote_olympus_2010();
			break;
		case 0x2020:								// CameraSettings 情報ブロック
			if (olympus->ExifDataType)
				seek(m_tiffbase + fget4(), SEEK_SET);
			makernote_olympus_2020();
			break;
		case 0x2031:								// ??? 情報ブロック
			makernote_olympus_2031();
			break;
		case 0x2030:								// RawDevelopment 情報ブロック
			if (olympus->ExifDataType)
				seek(m_tiffbase + fget4(), SEEK_SET);
			makernote_olympus_2030();
			break;
		case 0x2040:								// ImageProcessing 情報ブロック
			if (olympus->ExifDataType)
				seek(m_tiffbase + fget4(), SEEK_SET);
			makernote_olympus_2040();
			break;
		case 0x2050:								// FocusInfo 情報ブロック
			if (olympus->ExifDataType)
				seek(m_tiffbase + fget4(), SEEK_SET);
			makernote_olympus_2050();
			break;
		default:
//@tagdump(this, tag, type, count);
			break;
		}
		seek(save + 12, SEEK_SET);
	}

	m_order = sorder;
	m_tiffbase = stiffbase;
}

bool rawmap::makernote_minolta_info(int count)
{
	if (count < 1) return true;

	register minolta_t* minolta = &m_exif.minolta;

	// 読み込み
	long* val = new long[count];
	for (int i = 1; i < count; i++) {
		val[i] = fget4();
// dbg.printf(L"%03d: %08X\n", i, val[i]);
		switch (i) {
		case 1:
			// メーカーノートが無効な場合もある
			if (val[1] == 0x43414341) {
				delete [] val;
				return false;
			}
			break;
		case 2:
			if (val[2] == -1) {
				m_exif.exist_makenote = false;
				delete [] val;
				return false;
			}
			break;
		}
	}

	if (count >   2) minolta->ExpMode			= val[2];	// 露出制御プログラム
	if (count >   3) minolta->FlashMode			= val[3];	// フラッシュモード
	if (count >   4) {										// ホワイトバランス
		if (HIWORD(val[4])) {
					 minolta->WhiteBalance		= HIWORD(val[4]) & 0xff00;
					 minolta->WhiteBalanceTune	= (HIWORD(val[4]) & 0x00ff) - 0x80;
		}
		else
					 minolta->WhiteBalance		= val[4];
	}
	if (count >   5) minolta->ImageSize			= val[5];	// イメージサイズ
	if (count >   6) minolta->ImageQuality		= val[6];	// イメージ品質
	if (count >   7) minolta->DriveMode			= val[7];	// ドライブモード
	if (count >   8) minolta->MeteringMode		= val[8];	// 測光モード
	if (count >   9) minolta->FilmSpeed			= val[9];	// 撮像感度 APEX
	if (count >  10) minolta->ShutterSpeed		= val[10];	// シャッター速度 APEX
	if (count >  11) minolta->Aperture			= val[11];	// 絞り値 APEX
	if (count >  12) minolta->MacroMode			= val[12];	// マクロモード
	if (count >  13) minolta->DigitalZoom		= val[13];	// デジタルズーム
	if (count >  14) minolta->ExpCompensation	= val[14];	// 露出補正
	if (count >  15) minolta->BracketStep		= val[15];	// ブラケットステップ
																// 0 => '1/3 EV',
																// 1 => '2/3 EV',
																// 2 => '1 EV',
		// val[16]
		// val[17] -- 'IntervalLength'
		// val[18] -- 'IntervalNumber'
	if (count >  19) minolta->FocalLength		= val[19];	// 焦点距離
	if (count >  20) minolta->FocusDistance		= val[20];	// フォーカス距離
	if (count >  21) minolta->FlashFired		= val[21];	// フラッシュ発光
	if (count >  22) minolta->Date				= val[22];	// 日付
	if (count >  23) minolta->Time				= val[23];	// 時間
	if (count >  24) minolta->MaxAperture		= val[24];	// 開放F値
		// val[25 - 26];
		// val[27] -- 'FileNumberMemory',  0 => 'Off', 1 => 'On',
		// val[28] -- 'LastFileNumber'
	if (count >  29) minolta->WhiteBalanceRed	= val[29];	// 赤WB
	if (count >  30) minolta->WhiteBalanceGreen	= val[30];	// 緑WB
	if (count >  31) minolta->WhiteBalanceBlue	= val[31];	// 青WB
	if (count >  32) minolta->Saturation		= val[32];	// 彩度
	if (count >  33) minolta->Contrast			= val[33];	// コントラスト
	if (count >  34) minolta->Sharpness			= val[34];	// シャープネス
	if (count >  35) minolta->SubjectProgram	= val[35];	// 被写体プログラム
	if (count >  36) minolta->FlashComp			= val[36];	// フラッシュ補正
	if (count >  37) minolta->ISOSetting		= val[37];	// ISOセッティング
	if (count >  38) minolta->CameraModel		= val[38];	// カメラモデル
																// 0 => 'DiMAGE 7',
																// 1 => 'DiMAGE 5',
																// 2 => 'DiMAGE S304',
																// 3 => 'DiMAGE S404',
																// 4 => 'DiMAGE 7i',
																// 5 => 'DiMAGE 7Hi',
																// 6 => 'DiMAGE A1',
																// 7 => 'DiMAGE A2', # also 'DiMAGE S414'?
	if (count >  39) minolta->IntervalMode		= val[39];	// インターバルモード
		// val[40] -- 'FolderName', 0 => 'Standard Form', 1 => 'Data Form',
	if (count >  41) minolta->ColorMode			= val[41];	// カラーモード
	if (count >  42) minolta->ColorFilter		= val[42];	// カラーフィルター
	if (count >  43) minolta->BWFilter			= val[43];	// モノクロフィルター
	if (count >  44) minolta->InternalFlash		= val[44];	// 内蔵フラッシュ
	if (count >  45) minolta->BrightnessValue	= val[45];	// 輝度 APEX
	if (count >  46) minolta->SpotFocusPointX	= val[46];	// スポットフォーカスX位置
	if (count >  47) minolta->SpotFocusPointY	= val[47];	// スポットフォーカスY位置
	if (count >  48) minolta->WideFocusZone		= val[48];	// ワイドフォーカスゾーン
	if (count >  49) minolta->FocusMode			= val[49];	// フォーカスモード
	if (count >  50) minolta->FocusFrame		= val[50];	// フォーカスエリア
	if (count >  51) minolta->DECPosition		= val[51];	// DECポジション
	if (count >  53) minolta->AntiShake			= val[53];	// AntiShake

	delete [] val;
	return true;
}

void rawmap::makernote_minolta_7d(int count)
{
	// ALPHA-7DIGITAL
	if (count < 1) return;

	register minolta_t* minolta = &m_exif.minolta;

	// 読み込み
	long* val = new long[count];
	for (int i = 0; i < count; i++) {
		val[i] = (int)(short)fget2();
// dbg.printf(L"%03d: %04X\n", i, (unsigned short)val[i]);
	}

	if (count >   0) minolta->ExpMode			= val[0];	// 露出制御プログラム
	if (count >   1) minolta->DriveMode			= val[1];	// ドライブモード
		//  2- 3
	if (count >   4) minolta->WhiteBalance		= val[4];	// ホワイトバランス
		//  5- 7 WB Custom1 value
		//  8-10 WB Custom2 value
		// 11-13 WB Custom3 value
	if (count >  14) minolta->FocusMode			= val[14];	// フォーカスモード
	if (count >  15) minolta->DMF				= val[15];	// AF-A時のDMF
	if (count >  16) minolta->LocalFocusFrame	= val[16];	// ローカルフォーカスフレーム
	if (count >  17) minolta->FocusFrame		= val[17];	// フォーカスフレーム
		// 18-20
	if (count >  21) minolta->FlashFired		= val[21];	// フラッシュ発光
	if (count >  22) minolta->FlashMode			= val[22];	// フラッシュモード
	if (count >  23) minolta->FlashControl		= val[23];	// フラッシュ調光モード
		// 24
	if (count >  25) minolta->FlashComp			= val[25];	// フラッシュ補正
		// 26
	if (count >  27) minolta->MeteringMode		= val[27];	// 測光モード
	if (count >  28) minolta->ISOSetting		= val[28];	// ISOセッティング
		// 29
	if (count >  30) minolta->ExpCompensation	= val[30];	// 露出補正
		// 31-36
	if (count >  37) minolta->ColorSpace		= val[37];	// カラースペース
	if (count >  38) minolta->Sharpness			= val[38];	// シャープネス
	if (count >  39) minolta->Contrast			= val[39];	// コントラスト補正
	if (count >  40) minolta->Saturation		= val[40];	// 彩度
		// 41-42
	if (count >  43) minolta->ShutterSpeed		= val[43];	// シャッター速度？
	if (count >  44) minolta->Aperture			= val[44];	// 露出
		// 45-47
	if (count >  48) minolta->FocusRelease		= val[48];	// フォーカス／レリーズ優先
		// 49-62
	if (count >  63) minolta->WhiteBalanceTune	= val[63];	// ホワイトバランス補正
	if (count >  64) minolta->Hue				= val[64];	// 色合い
		// 64-71
//	if (count >  72) minolta->ShutterSpeed		= val[43];	// シャッター速度？
	if (count >  73) minolta->MeteredManual		= val[73];	// メータードマニュアル

	// アンチシェイク情報はドライババージョンによって位置が異なる？
	// v1.00 では offset109 v1.10 では offset113 のようだ
	int ofs = (count < 144) ? 109 : 113;
	if (count > ofs) minolta->AntiShake			= val[ofs]; // アンチシェイク

	delete [] val;
}

void rawmap::makernote_minolta_sweet(int count)
{
	// ALPHA-SWEET DIGITAL
	if (count < 1) return;
// count = min(count, 256);

	register minolta_t* minolta = &m_exif.minolta;

	// 読み込み
	long* val = new long[count];
	for (int i = 0; i < count; i++) {
		val[i] = (int)(short)fget2();
// dbg.printf(L"%03d: %04X\n", i, (unsigned short)val[i]);
	}

	// ALPHA SWEET DIGITAL
	if (count >  10) minolta->ExpMode			= val[ 10];	// 露出制御プログラム
	if (count >  11) minolta->DriveMode			= val[ 11];	// ドライブモード
	if (count >  14) minolta->WhiteBalance		= val[ 14];	// ホワイトバランス
	if (count >  26) minolta->LocalFocusFrame	= val[ 26];	// ローカルフォーカスフレーム
	if (count >  27) minolta->FocusFrame		= val[ 27];	// フォーカスフレーム
	if (count >  31) minolta->FlashFired		= val[ 31];	// フラッシュ発光
	if (count >  32) minolta->FlashMode			= val[ 32];	// フラッシュモード
	if (count >  33) minolta->FlashControl		= val[ 33];	// フラッシュ調光モード
	if (count >  35) minolta->FlashComp			= val[ 35];	// フラッシュ調光補正
	if (count >  37) minolta->MeteringMode		= val[ 37];	// 測光モード
	if (count >  38) minolta->ISOSetting		= val[ 38];	// ISOセッティング
	if (count >  40) minolta->ExpCompensation	= val[ 40];	// 露出補正
	if (count >  48) minolta->Sharpness			= val[ 48];	// シャープネス
	if (count >  49) minolta->Contrast			= val[ 49];	// コントラスト補正
	if (count >  50) minolta->Saturation		= val[ 50];	// 彩度
	if (count >  53) minolta->ShutterSpeed		= val[ 53];	// シャッター速度？
	if (count >  54) minolta->Aperture			= val[ 54];	// 露出
	if (count >  73) minolta->WhiteBalanceTune	= val[ 73];	// ホワイトバランス補正
	if (count >  74) minolta->Hue				= val[ 74];	// 色合い
	if (count > 111) minolta->ColorCompFilter	= val[111];	// CCフィルター
	if (count > 145) minolta->MeteredManual		= val[145];	// メータードマニュアル
	if (count > 158) {										// フォーカスモード
		if (val[ 72] == 0)
			minolta->FocusMode					= val[158];
		else
			minolta->FocusMode					= 100;
	}
	if (count > 189) minolta->AntiShake			= val[189]; // アンチシェイク

	delete [] val;
}

#define MINOLTA_MLY0			(('M'<<24)|('L'<<16)|('Y'<<8)|'0')

void rawmap::makernote_minolta(int base)
{
	if (getmsblong() == MINOLTA_MLY0) {
		m_exif.exist_makenote = false;
		return;
	}
	seek(-4, SEEK_CUR);

	register minolta_t* minolta = &m_exif.minolta;

	minolta->ExpMode = INT_MAX;
	minolta->FlashMode = INT_MAX;
	minolta->FlashControl = INT_MAX;
	minolta->WhiteBalance = INT_MAX;
	minolta->WhiteBalanceTune = INT_MAX;
	minolta->ImageSize = INT_MAX;
	minolta->ImageQuality = INT_MAX;
	minolta->DriveMode = INT_MAX;
	minolta->MeteringMode = INT_MAX;
	minolta->FilmSpeed = INT_MAX;
	minolta->ShutterSpeed = INT_MAX;
	minolta->Aperture = INT_MAX;
	minolta->MacroMode = INT_MAX;
	minolta->DigitalZoom = INT_MAX;
	minolta->ExpCompensation = INT_MAX;
	minolta->BracketStep = INT_MAX;
	minolta->FocalLength = INT_MAX;
	minolta->FocusDistance = INT_MAX;
	minolta->FlashFired = INT_MAX;
	minolta->Date = INT_MAX;
	minolta->Time = INT_MAX;
	minolta->MaxAperture = INT_MAX;
	minolta->WhiteBalanceRed = INT_MAX;
	minolta->WhiteBalanceGreen = INT_MAX;
	minolta->WhiteBalanceBlue = INT_MAX;
	minolta->Saturation = INT_MAX;
	minolta->Contrast = INT_MAX;
	minolta->Sharpness = INT_MAX;
	minolta->Hue = INT_MAX;
	minolta->SubjectProgram = INT_MAX;
	minolta->FlashComp = INT_MAX;
	minolta->ISOSetting = INT_MAX;
	minolta->CameraModel = INT_MAX;
	minolta->IntervalMode = INT_MAX;
	minolta->ColorMode = INT_MAX;
	minolta->ColorFilter = INT_MAX;
	minolta->BWFilter = INT_MAX;
	minolta->InternalFlash = INT_MAX;
	minolta->BrightnessValue = INT_MAX;
	minolta->SpotFocusPointX = INT_MAX;
	minolta->SpotFocusPointY = INT_MAX;
	minolta->WideFocusZone = INT_MAX;
	minolta->FocusMode = INT_MAX;
	minolta->FocusFrame = INT_MAX;
	minolta->DMF = INT_MAX;
	minolta->LocalFocusFrame = INT_MAX;
	minolta->DECPosition = INT_MAX;
	minolta->AntiShake = INT_MAX;
	minolta->ZoneMatching = INT_MAX;
	minolta->ColorTemp = INT_MAX;
	minolta->LensInfo = INT_MAX;
	minolta->MeteredManual = INT_MAX;
	minolta->FocusRelease = INT_MAX;
	minolta->ColorCompFilter = INT_MAX;
	minolta->ColorSpace = INT_MAX;

	_imageinfo_t* pimage = NULL;
	bool mly0 = false;
	int entries = fget2();
	while (entries--) {
		if (eof()) break;

		int save = tell();
		int tag  = fget2();
		int type = fget2();
		int count= fget4();

		if (count < 0 || !tiff_seek(type, count)) {
			// このタグはデータ不全なのでスキップ
			seek(save + 12, SEEK_SET);
			continue;
		}

		switch (tag) {
		case 0x0000:							// メーカーノートタイプ
			mly0 = (getmsblong() == MINOLTA_MLY0);
			break;
		case 0x0001:							// カメラセッティング(old)
		case 0x0003:							// カメラセッティング(new)
			if (!mly0)
				makernote_minolta_info(count);
			break;
		case 0x0004:							// カメラセッティング
			makernote_minolta_7d(count);
			break;
		case 0x0018:
			// 'ImageStabilization'
			break;
//		case 0x0040:							// RAWイメージデータサイズ？
//			break;
		case 0x0081:							// サムネイル
		case 0x0088:
			// MinoltaのJPEGサムネイルの先頭SOI=ffd8がxxd8になっているので注意
			if (!pimage) pimage = create_ifitem();
			pimage->thumb_offset = base + (long)get_tagvalue(type);
			pimage->compression = 6;
			jpg_getsize(pimage->thumb_offset, &pimage->width, &pimage->height);
			break;
		case 0x0089:							// サムネイルサイズ
			if (!pimage) pimage = create_ifitem();
			pimage->thumb_length = (long)get_tagvalue(type);
			break;
		case 0x0101:							// カラーモード or 画像仕上げ
			minolta->ColorMode = fget4();
			break;
		case 0x0102:							// イメージ品質
			minolta->ImageQuality = fget4();
			break;
		case 0x0103:							// 
			break;
		case 0x0107:							// アンチシェイク + その他のフラグ？
			if (!HasInt(minolta->AntiShake)) {
				long val = fget4();
				if (m_exif.ModelID != MINOLTA_ALPHA_7D)
					minolta->AntiShake = val;
//@				else
//@					minolta->AntiShake = (val & 0x4) ? 1 : 0;
			}
			break;
		case 0x010A:							// ゾーン切替
			minolta->ZoneMatching = fget4();
			break;
		case 0x010B:							// 色温度
			minolta->ColorTemp = fget4();
			break;
		case 0x010C:							// レンズ情報
			minolta->LensInfo = fget4();
			break;
		case 0x0114:							// カメラセッティング(α-SWEET DIGITAL)
			makernote_minolta_sweet(count);
			break;
		default:
// tagdump(this, tag, type, count);
			break;
		}
		seek(save + 12, SEEK_SET);
	}
}

void rawmap::makernote_casio()
{
	BYTE cb[256];
	read(cb, 6);
	if (memcmp(cb, "QVC\0", 4)) {
		m_exif.exist_makenote = false;
		return;
	}

	register casio_t* casio = &m_exif.casio;

	casio->WhiteBalanceBias[0] = INT_MAX;
	casio->WhiteBalance = INT_MAX;
	casio->SubjectDistance = INT_MAX;
	casio->FlashDistance = INT_MAX;
	casio->ShootMode = INT_MAX;
	casio->SelfTimer = INT_MAX;
	casio->ShutterMode = INT_MAX;
	casio->Quality = INT_MAX;
	casio->FocusMode = INT_MAX;
	casio->TimeZone[0] = 0;
	casio->BestShotNumber = INT_MAX;
	casio->ISO = INT_MAX;
	casio->ColorMode = INT_MAX;
	casio->ColorEnhance = INT_MAX;
	casio->ColorFilter = INT_MAX;
	casio->Sharpness = INT_MAX;
	casio->Contrast = INT_MAX;
	casio->Saturation = INT_MAX;

	int entries = fget2();

/* @@
if (entries > 0x100) {
	// とりあえずを256以上(HIBYTE!=0)のパラメタエントリであればエラーとする
	m_exif.exist_makenote = false;
	return;
}
@@ */
	// CASIOのメーカーノートって常にMotorolla形式？
	short sorder = m_order;
	m_order = 0x4d4d;

	_imageinfo_t* pimage = NULL;
	while (entries--) {
		if (eof()) break;

		int val;
		int save = tell();
		int tag  = fget2();
		int type = fget2();
		int count= fget4();

		if (tag == 0 || type < FMT_BYTE || type > FMT_DOUBLE || count == 0) break;

		if (count < 0 || !tiff_seek(type, count)) {
			// このタグはデータ不全なのでスキップ
			seek(save + 12, SEEK_SET);
			continue;
		}

		switch (tag) {
		case 0x0002:										// SHORTx2:サムネイルの大きさ
			if (!pimage) pimage = create_ifitem();
			pimage->width = (int)get_tagvalue(type);
			pimage->height = (int)get_tagvalue(type);
			break;
		case 0x0003:										// LONG:サムネイルデータ長
			if (!pimage) pimage = create_ifitem();
			pimage->thumb_length = (int)get_tagvalue(type);
			break;
		case 0x0004:										// LONG:サムネイルデータ位置
			if (!pimage) pimage = create_ifitem();
			pimage->thumb_offset = (int)get_tagvalue(type);
			if (jpg_getsize(pimage->thumb_offset, &pimage->width, &pimage->height))
				pimage->compression = 6;
			break;
		case 0x2011:										// SHORTx2:ホワイトバランス補正値
			casio->WhiteBalanceBias[0] = (int)get_tagvalue(type);
			casio->WhiteBalanceBias[1] = (int)get_tagvalue(type);
			break;
		case 0x2012:										// SHORT:ホワイトバランス
			casio->WhiteBalance = (int)get_tagvalue(type);
			break;
		case 0x2022:										// LONG:被写体までの距離
			casio->SubjectDistance = (int)get_tagvalue(type);
			break;
		case 0x2034:										// SHORT:フラッシュ距離
			val = (int)get_tagvalue(type);
			if (val) casio->FlashDistance = val;
			break;
		case 0x3000:										// SHORT:撮影モード
			casio->ShootMode = (int)get_tagvalue(type);
			break;
//		case 0x3001:										// SHORT:セルフタイマー
//			casio->SelfTimer = (int)get_tagvalue(type);
//			break;
		case 0x3001:										// SHORT:シャッターモード
			casio->ShutterMode = (int)get_tagvalue(type);
			break;
		case 0x3002:										// SHORT:画質モード
			casio->Quality = (int)get_tagvalue(type);
			break;
		case 0x3003:										// SHORT:フォーカスモード
			casio->FocusMode = (int)get_tagvalue(type);
			break;
		case 0x3006:										// ASCII:タイムゾーン
			read(casio->TimeZone, count);
			break;
		case 0x3007:										// SHORT:ベストショット番号
			casio->BestShotNumber = (int)get_tagvalue(type);
			break;
		case 0x3011:										// UNDEFx2: シャープネス
			casio->Sharpness = fgetshort();		// SHORTで読む
			break;
		case 0x3012:										// UNDEFx2: 彩度
			casio->Contrast = fgetshort();		// SHORTで読む
			break;
		case 0x3013:										// UNDEFx2: コントラスト
			casio->Saturation = fgetshort();	// SHORTで読む
			break;
		case 0x3014:										// SHORT:ISO感度
			casio->ISO = (int)get_tagvalue(type);
			if (!HasInt(m_exif.ISOEquiv)) m_exif.ISOEquiv = casio->ISO;
			break;
		case 0x3015:										// SHORT:カラーモード
			casio->ColorMode = (int)get_tagvalue(type);
			break;
		case 0x3016:										// SHORT:色強調
			casio->ColorEnhance = (int)get_tagvalue(type);
			break;
		case 0x3017:										// SHORT:カラーフィルター
			casio->ColorFilter = (int)get_tagvalue(type);
			break;
/* @@@ ------------------------------------
	EXILIM ZOOM EX-Z1000
		case 0x3025:										// USHRT:
			break;
		case 0x3026:										// USHRT:
			break;
		case 0x3027:										// USHRT:
			break;
		case 0x3028:										// USHRT:
			break;
		case 0x3029:										// USHRT:
			break;
------------------------------------- @@@ */
		default:
//@			tagdump(this, tag, type, count);
			break;
		}
		seek(save + 12, SEEK_SET);
	}
	m_order = sorder;
}

void rawmap::makernote_fuji()
{
	BYTE cb[256];
	read(cb, 12);
	if (memcmp(cb, "FUJIFILM", 8)) {
		m_exif.exist_makenote = false;
		return;
	}

	// メーカーノート内では、tiffbase はメーカーノートタグの先頭
	int stiffbase = m_tiffbase;
	m_tiffbase = tell() - 12;

	// "FUJIFILM" の直後の2バイト（4バイト??）でメーカーノートヘッダの長さを
	// 示しているようだ
	int n = (int)*((short*)(cb + 8));
	if (n != 12) seek(n - 12, SEEK_CUR);

	// FujifilmのデジカメのExif形式はMotorolla形式のデーター並びを使っているが、
	// MakerNoteだけはそれを無視してIntel並びになっている

	short sorder = m_order;
	m_order = 0x4949;

	register fuji_t* fuji = &m_exif.fuji;

	fuji->Version[0] = 0;
	fuji->Quality[0] = 0;
	fuji->Sharpness = INT_MAX;
	fuji->WhiteBalance = INT_MAX;
	fuji->Color = INT_MAX;
	fuji->ColorTemperature = INT_MAX;
	fuji->Tone = INT_MAX;
	fuji->Contrast = INT_MAX;
	fuji->WBFineTune[0] = INT_MAX;
	fuji->NoiseReduction = INT_MAX;
	fuji->FlashMode = INT_MAX;
	fuji->FlashStrength = FLT_MAX;
	fuji->MacroMode = INT_MAX;
	fuji->FocusMode = INT_MAX;
	fuji->SlowSync = INT_MAX;
	fuji->PictureMode = INT_MAX;
	fuji->ContTake = INT_MAX;
	fuji->ContTakeStart = INT_MAX;
	fuji->FinePixColor = INT_MAX;
	fuji->BlurWarning = INT_MAX;
	fuji->FocusWarning = INT_MAX;
	fuji->AEWarning = INT_MAX;
	fuji->CenterFixedAF = INT_MAX;
	fuji->FocusPointX = INT_MAX;
	fuji->FocusPointY = INT_MAX;
	fuji->DynamicRange = INT_MAX;
	fuji->DynamicRangeSetting = INT_MAX;
	fuji->FilmSimulationMode = INT_MAX;
	fuji->DynamicRangeVal = INT_MAX;
	fuji->MinFocalLength = INT_MAX;
	fuji->MaxFocalLength = INT_MAX;
	fuji->MaxApertureAtMinFocal = FLT_MAX;
	fuji->MaxApertureAtMaxFocal = FLT_MAX;
	fuji->FDPoint[0] = INT_MAX;
	fuji->FaceDetection = INT_MAX;

	int entries = fget2();
	while (entries--) {
		if (eof()) break;

		int save = tell();
		int tag  = fget2();
		int type = fget2();
		int count= fget4();

		if (count < 0 || !tiff_seek(type, count)) {
			// このタグはデータ不全なのでスキップ
			seek(save + 12, SEEK_SET);
			continue;
		}

		switch (tag) {
		case 0x0000:										// UNDEF:バージョン
			read_ascii(fuji->Version, count, ARRAYSIZE(fuji->Version));
			break;
		case 0x1000:										// ASCII:クォリティ
			read_ascii(fuji->Quality, count, ARRAYSIZE(fuji->Quality));
			break;
		case 0x1001:										// USHRT:シャープネス
			fuji->Sharpness = (int)get_tagvalue(type);
			break;
		case 0x1002:										// SHORT:ホワイトバランス
			fuji->WhiteBalance = (int)get_tagvalue(type);
			break;
		case 0x1003:										// SHORT:カラー（彩度）
			fuji->Color = (int)get_tagvalue(type);
			break;
		case 0x1004:										// SHORT:トーン（コントラスト）
			fuji->Tone = (int)get_tagvalue(type);
			break;
		case 0x1005:										// 色温度
			fuji->ColorTemperature = (int)get_tagvalue(type);
			break;
		case 0x1006:										// コントラスト
			fuji->Contrast =  (int)get_tagvalue(type);
			break;
		case 0x100A:										// SLONGx2: ホワイトバランス調整
			fuji->WBFineTune[0] = (int)get_tagvalue(type);
			fuji->WBFineTune[1] = (int)get_tagvalue(type);
			break;
		case 0x100B:										// USHRT: ノイズリダクション
			fuji->NoiseReduction = (int)get_tagvalue(type);
			break;
		case 0x1010:										// SHORT:フラッシュ発光モード
			fuji->FlashMode = (int)get_tagvalue(type);
			break;
		case 0x1011:										// SRATIN:フラッシュの発光強度補正値
			fuji->FlashStrength = (float)get_tagvalue(type);
			break;
		case 0x1020:										// SHORT:マクロモード
			fuji->MacroMode = (int)get_tagvalue(type);
			break;
		case 0x1021:										// SHORT:フォーカスモード
			fuji->FocusMode = (int)get_tagvalue(type);
			break;
		case 0x1022:										// SHORT:センター固定AF
			fuji->CenterFixedAF = (int)get_tagvalue(type);
			break;
		case 0x1023:										// SHORTx2:フォーカスポイント
			fuji->FocusPointX = (int)get_tagvalue(type);
			fuji->FocusPointY = (int)get_tagvalue(type);
			break;
		case 0x1030:										// SHORT:スローシンクロ
			fuji->SlowSync = (int)get_tagvalue(type);
			break;
		case 0x1031:										// SHORT:撮影モード
			fuji->PictureMode = (int)get_tagvalue(type);
			break;
		case 0x1100:										// SHORT:連写・自動ブラケットモード
			fuji->ContTake = (int)get_tagvalue(type);
			break;
		case 0x1101:										// SHORT:連写連番開始番号
			fuji->ContTakeStart = (int)get_tagvalue(type);
			break;
		case 0x1210:										// SHORT:FinePixカラー
			fuji->FinePixColor = (int)get_tagvalue(type);
			break;
		case 0x1300:										// SHORT:手振れ警告
			fuji->BlurWarning = (int)get_tagvalue(type);
			break;
		case 0x1301:										// SHORT:フォーカス警告
			fuji->FocusWarning = (int)get_tagvalue(type);
			break;
		case 0x1302:										// SHORT:AE警告
			fuji->AEWarning = (int)get_tagvalue(type);
			break;
		case 0x1400:										// SHORT:ダイナミックレンジ?
			fuji->DynamicRange = (int)get_tagvalue(type);
			break;
		case 0x1401:										// SHORT:フィルムシミュレーションモード?
			fuji->FilmSimulationMode = (int)get_tagvalue(type);
			break;
		case 0x1402:										// SHORT:ダイナミックレンジモード
			fuji->DynamicRangeSetting = (int)get_tagvalue(type);
			break;
		case 0x1403:										// SHORT:ダイナミックレンジ量
			fuji->DynamicRangeVal = (int)get_tagvalue(type);
			break;
		case 0x1404:										// RATINL:最小焦点距離
			fuji->MinFocalLength = (int)get_tagvalue(type);
			break;
		case 0x1405:										// RATINL:最大焦点距離
			fuji->MaxFocalLength = (int)get_tagvalue(type);
			break;
		case 0x1406:										// RATINL:最小焦点距離での開放F値
			fuji->MaxApertureAtMinFocal = (float)get_tagvalue(type);
			break;
		case 0x1407:										// RATINL:最大焦点距離での開放F値
			fuji->MaxApertureAtMaxFocal = (float)get_tagvalue(type);
			break;
		case 0x4005:										// USHRTx4: 検出顔位置
			fuji->FDPoint[0] = (int)get_tagvalue(type);
			fuji->FDPoint[1] = (int)get_tagvalue(type);
			fuji->FDPoint[2] = (int)get_tagvalue(type);
			fuji->FDPoint[3] = (int)get_tagvalue(type);
			break;
//		case 0x4100:										// USHRT:顔キレイナビ??
//			fuji->FaceDetection = (int)get_tagvalue(type);
//			break;
		}
		seek(save + 12, SEEK_SET);
	}

	m_tiffbase = stiffbase;
	m_order = sorder;
}

void rawmap::makernote_panasonic()
{
	BYTE cb[256];
	if (m_exif.MakerID == MAKER_PANASONIC) {
		read(cb, 12);
		if (memcmp(cb, "Panasonic", 9)) {
			m_exif.exist_makenote = false;
			return;
		}
	}
	else if (m_exif.MakerID == MAKER_LEICA) {
		read(cb, 8);
		if (memcmp(cb, "LEICA\0", 6)) {
			m_exif.exist_makenote = false;
			return;
		}
	}

	register panasonic_t* pana = &m_exif.panasonic;
	memset(pana, 0, sizeof(panasonic_t));

	pana->Quality = INT_MAX;
	pana->WhiteBalance = INT_MAX;
	pana->FocusMode = INT_MAX;
	pana->AFMode = INT_MAX;
	pana->AntiShake = INT_MAX;
	pana->MacroMode = INT_MAX;
	pana->ShootingMode = INT_MAX;
	pana->WhiteBalanceBias = INT_MAX;
	pana->FlashBias = INT_MAX;
	pana->ColorEffect = INT_MAX;
	pana->Contrast = INT_MAX;
	pana->BurstMode = INT_MAX;
	pana->SequenceNumber = INT_MAX;
	pana->NoiseReduction = INT_MAX;
	pana->SceneMode = INT_MAX;
	pana->ColorMode = INT_MAX;
	pana->SelfTimer = INT_MAX;
	pana->FilmMode = INT_MAX;
	pana->MyColor = INT_MAX;
	pana->exContrast = INT_MAX;
	pana->exSaturation = INT_MAX;
	pana->exSharpness = INT_MAX;
	pana->LightDetection = INT_MAX;

	int entries = fget2();
	while (entries--) {
		if (eof()) break;

		int save = tell();
		int tag  = fget2();
		int type = fget2();
		int count= fget4();

		if (count < 0 || !tiff_seek(type, count)) {
			// このタグはデータ不全なのでスキップ
			seek(save + 12, SEEK_SET);
			continue;
		}

		switch (tag) {
		case 0x0001:						// SHORT:画質モード
			pana->Quality = (int)get_tagvalue(type);
			break;
		case 0x0003:						// USHRT:ホワイトバランス
			pana->WhiteBalance = (int)get_tagvalue(type);
			break;
		case 0x0007:						// USHRT:フォーカスモード
			pana->FocusMode = (int)get_tagvalue(type);
			break;
		case 0x000F:						// BYTEx2:AFモード
			pana->AFMode = (int)(USHORT)getmsbshort();
			break;
		case 0x001A:						// USHRT:手ブレ補正
			pana->AntiShake = (int)get_tagvalue(type);
			break;
		case 0x001C:						// SHORT:マクロモード
			pana->MacroMode = (int)get_tagvalue(type);
			break;
		case 0x001F:						// SHORT:撮影モード
			pana->ShootingMode = (int)get_tagvalue(type);
			break;
//		case 0x0020:						// USHRT: Audio (1=Yes, 2=No)
//			break;
		case 0x0023:						// USHRT:ホワイトバランス補正
			pana->WhiteBalanceBias = (int)(short)get_tagvalue(type);
			break;
		case 0x0024:						// SHORT:フラッシュ光量補正
			pana->FlashBias = (int)get_tagvalue(type);
			break;
		case 0x0025:						// UNDEFx16: 内部シリアル番号
			read_ascii(pana->InternalSerialNumber, count, ARRAYSIZE(pana->InternalSerialNumber));
			break;
//		case 0x0026:						// UNDEFx4: Panasonic EXIF version --> ex '0210'
//			break;
		case 0x0028:						// USHRT:カラーエフェクト
			pana->ColorEffect = (int)get_tagvalue(type);
			break;
//		case 0x0029:						// ULONG: TimeSincePowerOn
//												// PowerOnからこの写真イメージをメモリカードに記録するまでの経過時間（1/100秒単位）
//			break;
		case 0x002A:						// USHRT: 連射モード
			pana->BurstMode = (int)get_tagvalue(type);
			break;
		case 0x002B:						// ULONG: オートブラケット
			pana->SequenceNumber = (int)get_tagvalue(type);
			break;
		case 0x002C:						// USHRT: コントラスト
			pana->Contrast = (int)get_tagvalue(type);
			break;
		case 0x002D:						// USHRT: ノイズリダクション
			pana->NoiseReduction = (int)get_tagvalue(type);
			break;
		case 0x002E:						// USHRT: セルフタイマー
			pana->SelfTimer = (int)get_tagvalue(type);
			break;
//		case 0x002F:						// USHRT: 
		case 0x0030:						// USHRT: 縦位置検出機能
												// 1 通常
												// 6 左９０°回転
												// 8 右９０°回転 
			break;
		case 0x0031:						// USHRT: シーンモード?
			pana->SceneMode = (int)get_tagvalue(type);
			break;
		case 0x0032:						// USHRT: カラーモード?
			pana->ColorMode = (int)get_tagvalue(type);
			break;
//		case 0x0033:						// ASCII: BabyAge 赤ちゃん誕生日
//		case 0x0034:						// USHRT: 光学ズーム （1=標準ズーム, 2=EX光学ズーム）
//		case 0x0035:						// USHRT: 
//		case 0x0036:						// USHRT: 
//		case 0x0037:						// USHRT: 
//		case 0x0038:						// USHRT: 
		case 0x0039:						// USHRT: コントラスト（符号付2byte整数として読取るDMC-L1のみ）
			pana->exContrast = (int)fgetshort();
			break;
//		case 0x003C:						// USHRT: ISO
		case 0x003D:						// USHRT: マイカラー
			pana->MyColor = (int)fgetshort();
			break;
		case 0x0040:						// USHRT: 彩度（符号付2byte整数として読取るDMC-L1のみ）
			pana->exSaturation = (int)fgetshort();
			break;
		case 0x0041:						// USHRT: シャープネス（符号付2byte整数として読取るDMC-L1のみ）
			pana->exSharpness = (int)fgetshort();
			break;
		case 0x0042:						// USHRT: フィルムモード
			pana->FilmMode = (int)get_tagvalue(type);
			break;
//		case 0x0043:						// USHRT: 
//		case 0x0044:						// USHRT: 
//		case 0x0045:						// USHRT: 
//		case 0x0046:						// USHRT: WBAdjustAB ... positive is a shift toward blue
//		case 0x0047:						// USHRT: WBAdjustGM ... positive is a shift toward green
//		case 0x0048:						// USHRT: 
//		case 0x0049:						// USHRT: 
//		case 0x0050:						// USHRT: 
		case 0x0051:						// ASCII: 使用レンズ
			read_ascii(pana->LensName, count, ARRAYSIZE(pana->LensName));
			break;
//		case 0x0052:						// ASCII: レンズ製造番号
//			break;
		case 0x0053:						// ASCII: アクセサリ
			read_ascii(pana->Accessory, count, ARRAYSIZE(pana->Accessory));
			break;
		case 0x005D:						// USHRT: 暗部補正
			pana->LightDetection = (int)get_tagvalue(type);
			break;
//		case 0x8000:						// UNDEFx4:
//		case 0x8001:						// USHRT:
//		case 0x8002:						// USHRT:
//		case 0x8003:						// USHRT:
//		case 0x8004:						// USHRT: WBRedLevel
//		case 0x8005:						// USHRT: WBGreenLevel
//		case 0x8006:						// USHRT: WBBlueLevel
//		case 0x8007:						// USHRT: WBBlueLevel
//		case 0x8010:						// ASCII: Baby or Pets' Age
		default:
// tagdump(this, tag, type, count);
			break;
		}
		seek(save + 12, SEEK_SET);
	}
}

void rawmap::makernote_sanyo()
{
	register sanyo_t* sanyo = &m_exif.sanyo;

	sanyo->Quality = INT_MAX;
	sanyo->ShootMode = INT_MAX;
	sanyo->DigitalZoom = FLT_MAX;
	sanyo->ContShoot = INT_MAX;
	sanyo->WideRangeShoot = INT_MAX;
	sanyo->RealColorEqualizer = INT_MAX;
	sanyo->QuickShot = INT_MAX;
	sanyo->SelfTimer = INT_MAX;
	sanyo->VoiceMemo = INT_MAX;
	sanyo->FlickerReduce = INT_MAX;
	sanyo->FocusMode = INT_MAX;
	sanyo->OpticalZoomUsing = INT_MAX;
	sanyo->DigitalZoomUsing = INT_MAX;
	sanyo->FlashMode = INT_MAX;

	BYTE cb[256];
	read(cb, 8);
	if (memcmp(cb, "SANYO", 5)) {
		m_exif.exist_makenote = false;
		return;
	}

	int entries = fget2();
	while (entries--) {
		if (eof()) break;

		int save = tell();
		int tag  = fget2();
		int type = fget2();
		int count= fget4();

		if (count < 0 || !tiff_seek(type, count)) {
			// このタグはデータ不全なのでスキップ
			seek(save + 12, SEEK_SET);
			continue;
		}

		switch (tag) {
		case 0x0201:						// USHRT:画質モード
			sanyo->Quality = (int)get_tagvalue(type);
			break;
		case 0x0202:						// SHORT:撮影モード
			sanyo->ShootMode = (int)get_tagvalue(type);
			break;
		case 0x0204:						// RATIN:デジタルズーム
			sanyo->DigitalZoom = (float)get_tagvalue(type);
			break;
		case 0x020E:						// SHORT:連写モード
			sanyo->ContShoot = (int)get_tagvalue(type);
			break;
		case 0x020F:						// SHORT:ワイドレンジショット
			sanyo->WideRangeShoot = (int)get_tagvalue(type);
			break;
		case 0x0210:						// SHORT:リアルカラーイコライザ
			sanyo->RealColorEqualizer = (int)get_tagvalue(type);
			break;
		case 0x0213:						// SHORT:クイックショット
			sanyo->QuickShot = (int)get_tagvalue(type);
			break;
		case 0x0214:						// SHORT:セルフタイマ
			sanyo->SelfTimer = (int)get_tagvalue(type);
			break;
		case 0x0215:						// SHORT:ボイスメモ
			sanyo->VoiceMemo = (int)get_tagvalue(type);
			break;
		case 0x0218:						// SHORT:フリッカー軽減
			sanyo->FlickerReduce = (int)get_tagvalue(type);
			break;
		case 0x0219:						// SHORT:光学ズームの利用
			sanyo->OpticalZoomUsing = (int)get_tagvalue(type);
			break;
		case 0x021A:						// SHORT:フォーカスモード
			sanyo->FocusMode = (int)get_tagvalue(type);
			break;
		case 0x021B:						// SHORT:デジタルズームの利用
			sanyo->DigitalZoomUsing = (int)get_tagvalue(type);
			break;
		case 0x0225:						// SHORT:フラッシュモード
			sanyo->FlashMode = (int)get_tagvalue(type);
			break;
		}
		seek(save + 12, SEEK_SET);
	}
}

static float _pentax_ev_conv(int ev)
{
	float val = (float)ev;
	if (ev & 0x01) {
		int sign = ev < 0 ? -1 : 1;
		int frac = (ev * sign) & 0x07;
		if (frac == 0x03)
			val += sign * ( 8.0F / 3.0F - frac);
		else if (frac == 0x05)
			val += sign * (16.0F / 3.0F - frac);
    }
	return val / 8.0F;
}

void rawmap::makernote_pentax()
{
	BYTE cb[256];
	read(cb, 4);
	if (memcmp(cb, "AOC\0", 4)) {
		read(cb + 4, 4);
		if (memcmp(cb, "PENTAX", 6)) {
			m_exif.exist_makenote = false;
			return;
		}
	}

	register pentax_t* pentax = &m_exif.pentax;

	pentax->Quality = INT_MAX;
	pentax->ImageSize = INT_MAX;
	pentax->PictureMode = INT_MAX;
	pentax->FlashMode = INT_MAX;
	pentax->ShootingDate = INT_MAX;
	pentax->ShootingTime = INT_MAX;
	pentax->FocusMode = INT_MAX;
	pentax->AFPointSelected = INT_MAX;
	pentax->AFPointsInFocus = INT_MAX;
	pentax->ShutterSpeed = INT_MAX;
	pentax->Aperture = INT_MAX;
	pentax->ISO = INT_MAX;
	pentax->WhiteBalance = INT_MAX;
	pentax->WhiteBalanceMode = INT_MAX;
	pentax->BlueBalance = INT_MAX;
	pentax->RedBalance = INT_MAX;
	pentax->ExposureComp = INT_MAX;
	pentax->MeteringMode = INT_MAX;
	pentax->FocalLength = INT_MAX;
	pentax->DigiatlZoom = INT_MAX;
	pentax->TakeCount = INT_MAX;
	pentax->EffectiveLightValue = INT_MAX;
	pentax->ImageProcessing = INT_MAX;
	pentax->ShootMode[0] = INT_MAX;
	pentax->DriveMode[0] = INT_MAX;
	pentax->SensitivityAdjust = INT_MAX;
	pentax->DigitalFilterCount = INT_MAX;
	pentax->CameraTemperature = INT_MAX;
	pentax->AELock = INT_MAX;
	pentax->Saturation = INT_MAX;
	pentax->Contrast = INT_MAX;
	pentax->Sharpness = INT_MAX;
	pentax->HometownCity = INT_MAX;
	pentax->ColorSpace = INT_MAX;
	pentax->LensInfo = INT_MAX;
	pentax->NoiseReduction = INT_MAX;
	pentax->FlashExposureComp = INT_MAX;
	pentax->PhotoFinish = INT_MAX;
	pentax->ColorTemperature = INT_MAX;
	*((int*)pentax->SRInfo) = INT_MAX;
	pentax->ShutterCount = INT_MAX;
	*((int*)pentax->ShotInfo) = INT_MAX;
	*((int*)pentax->AEInfo) = INT_MAX;
	pentax->AutoBracket[0] = INT_MAX;
	pentax->CameraInfo[0] = INT_MAX;

	short sorder = m_order;
	m_order = fget2();

	_imageinfo_t* pimage = NULL;
	int entries = fget2();
	while (entries--) {
		if (eof()) break;

		int save = tell();
		int tag  = fget2();
		int type = fget2();
		int count= fget4();
		int i, val;

		if (count < 0 || !tiff_seek(type, count)) {
			// このタグはデータ不全なのでスキップ
			seek(save + 12, SEEK_SET);
			continue;
		}

		switch (tag) {
		case 0x0001:						// PentaxMode
													// 0 => 'Auto'
													// 1 => 'Night-scene'
													// 2 => 'Manual'
			break;
		case 0x0002:						// PreviewImageSize
			if (!pimage) pimage = create_ifitem();
			pimage->width = (int)get_tagvalue(type);
			pimage->height = (int)get_tagvalue(type);
			break;
		case 0x0003:						// PreviewImageLength
			if (!pimage) pimage = create_ifitem();
			pimage->thumb_length = (int)get_tagvalue(type);
			break;
		case 0x0004:						// PreviewImageStart
			if (!pimage) pimage = create_ifitem();
			pimage->thumb_offset = m_tiffbase + (int)get_tagvalue(type);
			pimage->compression = 6;
			break;
//		case 0x0005:						// ULONG: PentaxモデルNo.
		case 0x0006:						// 日付(UNDEFx4) 0xAABBCCDD AABB=year CC=mon DD=day
			pentax->ShootingDate = getmsblong();
			break;
		case 0x0007:						// Time(UNDEFx3) 0xAABBCC AA=hour BB=min CC=sec
			pentax->ShootingTime = (getmsbshort() << 8) | getc();
			break;
		case 0x0008:						// SHORT 画質モード
			pentax->Quality = (int)get_tagvalue(type);
			break;
		case 0x0009:						// USHRTx2: イメージサイズ（最初のUSHRTのみ取得）
			pentax->ImageSize = (int)get_tagvalue(type);
													// 0 => '640x480'
													// 1 => 'Full', #PH - this can mean 2048x1536 or 2240x1680 or ... ?
													// 2 => '1024x768'
													// 4 => '1600x1200',
													// 5 => '2048x1536',
													// 21 => '2592x1944',
													// 22 => '2304x1728', #2
													// 24 => '3008x2008'
													// 25 => '3008x2000'
													// 36 0' => '3008x2008',  #PH
			break;
		case 0x000B:						// PictureMode
			pentax->PictureMode = (int)get_tagvalue(type);
			break;
		case 0x000C:						// フラッシュモード
			pentax->FlashMode = (int)get_tagvalue(type);
			break;
		case 0x000D:						// SHORT フォーカスモード
			pentax->FocusMode = (int)get_tagvalue(type);
			break;
		case 0x000E:						// USHRT: 測距点選択
			pentax->AFPointSelected = (int)get_tagvalue(type);
			break;
		case 0x000F:						// USHRT: AFエリア
			pentax->AFPointsInFocus = (int)get_tagvalue(type);
			break;
//@		case 0x0010:						// フォーカスポジション
//@			break;
		case 0x0012:						// ULONG シャッター速度
			pentax->ShutterSpeed = (int)get_tagvalue(type);
			break;
		case 0x0013:						// USHRT 絞り
			pentax->Aperture = (int)get_tagvalue(type);
			break;
		case 0x0014:						// SHORT ISO感度
			pentax->ISO = (int)(short)get_tagvalue(type);
			break;
//		case 0x0015:						// LightReading
		case 0x0016:						// USHRT 露出補正
			pentax->ExposureComp = (int)get_tagvalue(type);
			break;
		case 0x0017:						// AE MeteringMode
													// 0 => 'Multi Segment',
													// 1 => 'Center Weighted',
													// 2 => 'Spot',
			pentax->MeteringMode = (int)get_tagvalue(type);
			break;
		case 0x0018:						// USHRT: オートブラケット
			pentax->AutoBracket[0] = (int)get_tagvalue(type);
			pentax->AutoBracket[1] = (int)get_tagvalue(type);
			break;
		case 0x0019:						// SHORT ホワイトバランス
			pentax->WhiteBalance = (int)(short)get_tagvalue(type);
			break;
		case 0x001A:						// WhiteBalanceMode
													// 1 => 'Auto (Daylight)',
													// 2 => 'Auto (Shade)',
													// 3 => 'Auto (Flash)',
													// 4 => 'Auto (Tungsten)',
													// 0xffff => 'User-Selected',
													// 0xfffe => 'Preset (Fireworks?)'
			pentax->WhiteBalanceMode = (int)get_tagvalue(type);
			break;
		case 0x001B:						// USHRT ブルーバランス
			pentax->BlueBalance = (int)get_tagvalue(type);
			break;
		case 0x001C:						// USHRT レッドバランス
			pentax->RedBalance = (int)get_tagvalue(type);
			break;
		case 0x001D:						// LONG レンズの焦点距離
			pentax->FocalLength = (int)get_tagvalue(type);
			break;
		case 0x001E:						// USHRT デジタルズーム
			pentax->DigiatlZoom = (int)get_tagvalue(type);
			break;
		case 0x001F:						// SHORTx2 彩度
			pentax->Saturation = (int)(short)get_tagvalue(type);
			break;
		case 0x0020:						// SHORTx2 コントラスト
			pentax->Contrast = (int)(short)get_tagvalue(type);
			break;
		case 0x0021:						// SHORTx2 シャープネス
			pentax->Sharpness = (int)(short)get_tagvalue(type);
			break;
		case 0x0022:						// USHRT WorldTimeLocation
												// 0 => 'Hometown',
												// 1 => 'Destination',
			break;
		case 0x0023:						// USHRT: 地域
			pentax->HometownCity = (int)get_tagvalue(type);
			break;
		case 0x0024:						// USHRT DestinationCity
			break;
		case 0x0025:						// USHRT HometownDST
			break;
		case 0x0026:						// USHRT DestinationDST
			break;
//		case 0x0027:						// UNDEFx4: DSPFirmwareVersion
//		case 0x0028:						// UNDEFx4: CPUFirmwareVersion
		case 0x0029:						// 総撮影枚数
			pentax->TakeCount = (int)get_tagvalue(type);
			break;
		case 0x002D:						// USHRT: EffectiveLV
			pentax->EffectiveLightValue = (int)get_tagvalue(type);
			break;
		case 0x0032:						// UNDEFx4 編集モード
			if (count == 4) {
				val = 0;
				while (count-- > 0)
					val = (val << 8) | getc();
				pentax->ImageProcessing = val;
			}
			break;
		case 0x0033:						// BYTEx3 撮影モード
			pentax->ShootMode[0] = getc(); 
			pentax->ShootMode[1] = getc();
			pentax->ShootMode[2] = getc();
			break;
		case 0x0034:						// BYTEx4: ドライブモード
			pentax->DriveMode[0] = getc();
			pentax->DriveMode[1] = getc();
			pentax->DriveMode[2] = getc();
			pentax->DriveMode[3] = getc();
			break;
		case 0x0037:						// SHORT カラースペース
			pentax->ColorSpace = (int)get_tagvalue(type);
			break;
//		case 0x0039:						// USHRTx2 RAWイメージサイズ
//		case 0x003C:						// ULONG: AFPointsInFocus (istD only)
//		case 0x003D:						// USHRT:
//		case 0x003E:						// BYTEx4: PreviewImageBorders
		case 0x003F:						// BYTEx2 レンズ情報
			val = 0;
			count = min(2, count);
			while (count-- > 0) val = (val << 8) | getc();
			pentax->LensInfo = val;
			break;
		case 0x0040:						// USHRT: 感度調整
			pentax->SensitivityAdjust = (int)get_tagvalue(type);
			break;
		case 0x0041:						// USHRT デジタルフィルタ適用回数
			pentax->DigitalFilterCount = (int)get_tagvalue(type);
			break;
		case 0x0047:						// SBYTE: カメラ温度
			pentax->CameraTemperature = (int)(char)getc();
			break;
		case 0x0048:						// USHRT: AEロック
			pentax->AELock = (int)get_tagvalue(type);
			break;
		case 0x0049:						// USHRT ノイズリダクション
			pentax->NoiseReduction = (int)get_tagvalue(type);
			break;
		case 0x004D:						// SLONG: フラッシュ露出調整
			pentax->FlashExposureComp = (int)(long)get_tagvalue(type);
			break;
		case 0x004F:						// USHRT 画像仕上げ？
			pentax->PhotoFinish = (int)get_tagvalue(type);
			break;
		case 0x0050:						// USHRT: 色温度
			pentax->ColorTemperature = (int)get_tagvalue(type);
			break;
		case 0x005C:						// BYTEx4:
			read(pentax->SRInfo, min(count, sizeof(pentax->SRInfo)));
			break;
		case 0x005D:						// UNDEFx4: シャッター回数
			pentax->ShutterCount = getmsblong();
			break;
//		case 0x0062:						// USHRT:
//		case 0x0067:						// USHRT:
//		case 0x0068:						// BYTE:
//		case 0x0069:						// UNDEFx4:
//		case 0x0070:						// BYTE:
//		case 0x0071:						// BYTE:
//		case 0x0072:						// SSHRT:
//		case 0x0073:						// USHRT:
//		case 0x0074:						// USHRT:
//		case 0x0200:						// USHRTx4 BlackPoint
//		case 0x0201:						// USHRTx4 WhitePoint
		case 0x0205:						// UNDEFxn: ショット情報
			read(pentax->ShotInfo, min(count, sizeof(pentax->ShotInfo)));
			break;
		case 0x0206:						// UNDEFxn: AE情報
			read(pentax->AEInfo, min(count, sizeof(pentax->AEInfo)));
			break;
//		case 0x0207:						// UNDEFxn: レンズ情報
//		case 0x0208:						// UNDEFxn: ストロボ情報
//		case 0x0209:						// UNDEFxn: AEMeteringSegments
//		case 0x020A:						// UNDEFxn: FlashADump
//		case 0x020B:						// UNDEFxn: FlashBDump
//		case 0x020D:						// USHRTx4: WB_RGGBLevelsDaylight
//		case 0x020E:						// USHRTx4: WB_RGGBLevelsShade
//		case 0x020F:						// USHRTx4: WB_RGGBLevelsCloudy
//		case 0x0210:						// USHRTx4: WB_RGGBLevelsTungsten
//		case 0x0211:						// USHRTx4: WB_RGGBLevelsFluorescentD
//		case 0x0212:						// USHRTx4: WB_RGGBLevelsFluorescentN
//		case 0x0213:						// USHRTx4: WB_RGGBLevelsFluorescentW
//		case 0x0214:						// USHRTx4: WB_RGGBLevelsFlash
		case 0x0215:						// ULONGx5: カメラ情報
			count = min(count, ARRAYSIZE(pentax->CameraInfo));
			for (i = 0; i < count; i++)
				pentax->CameraInfo[i] = (int)get_tagvalue(type);
			break;
//		case 0x0216:						// UNDEF: バッテリー情報
//		case 0x021F:						// UNDEF: AF情報
//		case 0x0222:						// UNDEF: カラー情報
//		case 0x0226:						// UNDEF
//		case 0x03FF:						// UNDEF
		default:
// tagdump(this, tag, type, count);
			break;
		}
		seek(save + 12, SEEK_SET);
	}

	m_order = sorder;
}

void rawmap::makernote_sigma()
{
	BYTE cb[256];
	read(cb, 10);
	if (memcmp(cb, "SIGMA\0", 6)) {
		m_exif.exist_makenote = false;
		return;
	}

	// 初期値設定
	register sigma_t* sigma = &m_exif.sigma;
	memset(sigma, 0, sizeof(sigma_t));
	sigma->FNumber = FLT_MAX;
	sigma->ExposureTime = FLT_MAX;
	sigma->ExposureCompensation = FLT_MAX;
	sigma->FlashExposureComp = FLT_MAX;

	_imageinfo_t* pimage = NULL;

	int lastpos = 0;
	int entries = fget2();
	while (entries--) {
		if (eof()) break;

		int save = tell();
		int tag  = fget2();
		int type = fget2();
		int count= fget4();

		if (count < 0 || !tiff_seek(type, count)) {
			// このタグはデータ不全なのでスキップ
			seek(save + 12, SEEK_SET);
			continue;
		}

		switch (tag) {
		case 0x0002:						// ASCII シリアル番号
			read_ascii(sigma->Serial, count, ARRAYSIZE(sigma->Serial));
			break;
		case 0x0003:						// ASCII ドライブモード
			read_ascii(sigma->DriveMode, count, ARRAYSIZE(sigma->DriveMode));
			break;
		case 0x0004:						// ASCII 解像度
			read_ascii(sigma->Resolution, count, ARRAYSIZE(sigma->Resolution));
			break;
		case 0x0005:						// ASCII AFモード
			read_ascii(sigma->AFMode, count, ARRAYSIZE(sigma->AFMode));
			break;
		case 0x0006:						// ASCII フォーカスモード
			read_ascii(sigma->FocusMode, count, ARRAYSIZE(sigma->FocusMode));
			break;
		case 0x0007:						// ASCII ホワイトバランスモード
			read_ascii(sigma->WhiteBalanceMode, count, ARRAYSIZE(sigma->WhiteBalanceMode));
			break;
		case 0x0008:						// ASCII 撮影モード
			read_ascii(sigma->ShootMode, count, ARRAYSIZE(sigma->ShootMode));
			break;
		case 0x0009:						// ASCII 測光モード
			read_ascii(sigma->MeteringMode, count, ARRAYSIZE(sigma->MeteringMode));
			break;
		case 0x000A:						// ASCII レンズ焦点範囲
			read_ascii(sigma->FocalLengthRange, count, ARRAYSIZE(sigma->FocalLengthRange));
			break;
		case 0x000B:						// ASCII カラースペース
			read_ascii(sigma->ColorSpace, count, ARRAYSIZE(sigma->ColorSpace));
			break;
		case 0x000C:						// ASCII 露出補正
			read_ascii(sigma->ExpCompensation, count, ARRAYSIZE(sigma->ExpCompensation));
			break;
		case 0x000D:						// ASCII コントラスト
			read_ascii(sigma->Contrast, count, ARRAYSIZE(sigma->Contrast));
			break;
		case 0x000E:						// ASCII シャドウ
			read_ascii(sigma->Shadow, count, ARRAYSIZE(sigma->Shadow));
			break;
		case 0x000F:						// ASCII ハイライト
			read_ascii(sigma->Hilight, count, ARRAYSIZE(sigma->Hilight));
			break;
		case 0x0010:						// ASCII 彩度
			read_ascii(sigma->Saturation, count, ARRAYSIZE(sigma->Saturation));
			break;
		case 0x0011:						// ASCII シャープネス
			read_ascii(sigma->Sharpness, count, ARRAYSIZE(sigma->Sharpness));
			break;
		case 0x0012:						// ASCII X3 Fill Light
			read_ascii(sigma->X3FillLight, count, ARRAYSIZE(sigma->X3FillLight));
			break;
		case 0x0014:						// ASCII カラー調整
			read_ascii(sigma->ColorAdjust, count, ARRAYSIZE(sigma->ColorAdjust));
			break;
		case 0x0015:						// ASCII 調整モード
			read_ascii(sigma->AdjustMode, count, ARRAYSIZE(sigma->AdjustMode));
			break;
		case 0x0016:						// ASCII 画質
			read_ascii(sigma->Quality, count, ARRAYSIZE(sigma->Quality));
			break;
		case 0x0017:						// ASCII ファームウェア
			read_ascii(sigma->Firmware, count, ARRAYSIZE(sigma->Firmware));
			break;
		case 0x0018:						// ASCII ソフトウェア
			read_ascii(sigma->Software, count, ARRAYSIZE(sigma->Software));
			break;
		case 0x0019:						// ASCII オートブラケット
			read_ascii(sigma->AutoBracket, count, ARRAYSIZE(sigma->AutoBracket));
			break;
		case 0x001A:						// ULONG: PreviewImageStart
			if (!pimage) pimage = create_ifitem();
			pimage->thumb_offset = m_tiffbase + (int)get_tagvalue(type);
			pimage->compression = 6;
			break;
		case 0x001B:						// ULONG: PreviewImageLength
			if (!pimage) pimage = create_ifitem();
			pimage->thumb_length = (int)get_tagvalue(type);
			break;
		case 0x001C:						// USHRTx2: PreviewImageSize
			if (!pimage) pimage = create_ifitem();
			pimage->width = (int)get_tagvalue(type);
			pimage->height = (int)get_tagvalue(type);
			break;
		case 0x001D:						// ASCII メーカーノートバージョン
			read_ascii(sigma->MakerNoteVersion, count, ARRAYSIZE(sigma->MakerNoteVersion));
			break;
		case 0x0022:						// AdjustmentMode
			read_ascii(sigma->AdjustmentMode, count, ARRAYSIZE(sigma->AdjustmentMode));
			break;
		case 0x0030:						// ASCII LensApertureRange
			read_ascii(sigma->LensApertureRange, count, ARRAYSIZE(sigma->LensApertureRange));
			break;
		case 0x0031:						// RATNL F値
			sigma->FNumber = (float)get_tagvalue(type);
			break;
		case 0x0032:						// RATNL 露出時間
			sigma->ExposureTime = (float)get_tagvalue(type);
			break;
		case 0x0033:						// ASCII: ExposureTime2  1/100 → "10000", 1/200 → "5000", 1/400 → "2500"
			break;
		case 0x0035:						// SRTNL ExposureCompensation
			sigma->ExposureCompensation = (float)get_tagvalue(type);
			break;
		case 0x003A:						// RATNL FlashExposureComp
			sigma->FlashExposureComp = (float)get_tagvalue(type);
			break;
		case 0x003B:						// ASCII ファームウェア
			read_ascii(sigma->Firmware2, count, ARRAYSIZE(sigma->Firmware2));
			break;
		case 0x003C:						// ASCII ホワイトバランス
			read_ascii(sigma->WhiteBalance, count, ARRAYSIZE(sigma->WhiteBalance));
			break;
		default:
//@ tagdump(this, tag, type, count);
			break;
		}
		lastpos = tell();
		seek(save + 12, SEEK_SET);
	}

	if (lastpos) {
		// SD14 では最後のタグ情報の直後にJPEGデータが付属している
		seek(lastpos, SEEK_SET);
		if ((USHORT)getmsbshort() == 0xffd8) {
			seek(-2, SEEK_CUR);
			_imageinfo_t* pimage = create_ifitem();
			pimage->thumb_offset = lastpos;
			pimage->compression = 6;
			pimage->thumb_length = jpg_datasize();
			jpg_getsize(pimage->thumb_offset, &pimage->width, &pimage->height);
		}
	}
}

void rawmap::makernote_epson()
{
	BYTE cb[32];
	read(cb, 8);
	if (memcmp(cb, "EPSON", 5)) {
		m_exif.exist_makenote = false;
		return;
	}

	register epson_t* epson = &m_exif.epson;
	memset(epson, 0, sizeof(epson_t));
	epson->Quality = INT_MAX;
	epson->ImageWidth = INT_MAX;
	epson->ImageHeight = INT_MAX;

	_imageinfo_t* pimage = NULL;
	int entries = fget2();
	while (entries--) {
		if (eof()) break;

		int save = tell();
		int tag  = fget2();
		int type = fget2();
		int count= fget4();

		if (count < 0 || !tiff_seek(type, count)) {
			// このタグはデータ不全なのでスキップ
			seek(save + 12, SEEK_SET);
			continue;
		}

		switch (tag) {
		case 0x0201:					// USHRT 画質モード
			epson->Quality = (int)get_tagvalue(type);
			break;
		case 0x0209:					// UNDEFx32 カメラID
			read_ascii(epson->CameraID, count, ARRAYSIZE(epson->CameraID));
			break;
		case 0x020B:					// ULONG イメージ幅
			epson->ImageWidth = (int)get_tagvalue(type);
			break;
		case 0x020C:					// ULONG イメージ高さ
			epson->ImageHeight = (int)get_tagvalue(type);
			break;
		case 0x020D:					// ASCIIx6 ハードウェアタイプ
			read_ascii(epson->HardwareType, count, ARRAYSIZE(epson->HardwareType));
			break;
		case 0x0280:					// サムネイルデータ（JPEG形式でFFD8がEED8と記録されている）
			// EpsonのJPEGサムネイルの先頭SOI=ffd8がeed8になっているので注意
			if (!pimage) pimage = create_ifitem();
			pimage->thumb_offset = tell();
			pimage->compression = 6;
			pimage->thumb_length = count;
			jpg_getsize(pimage->thumb_offset, &pimage->width, &pimage->height);
			break;
		default:
//@			tagdump(this, tag, type, count);
			break;
		}
		seek(save + 12, SEEK_SET);
	}
}

void rawmap::makernote_sony_A100(int count)
{
	// α100
	if (count < 1) return;

	register sony_t* sony = &m_exif.sony;

	// MMオーダー?で読み込み
	short sorder = m_order;
	m_order = 0x4d4d;

	long* val = new long[count];
	for (int i = 0; i < count; i++) {
		val[i] = (int)(short)fget2();
//@ dbg.printf(L"%03d: %04X\n", i, (unsigned short)val[i]);
	}

	m_order = sorder;


	// α100
	if (count >   0) sony->mltExposureMode		= val[  0];			// 露出制御プログラム
	if (count >   1) sony->mltExpCompensation	= val[  1];			// 露出補正
	if (count >   8) sony->mltExposureTime		= val[  8];			// シャッター速度
	if (count >   9) sony->mltAperture			= val[  9];			// 絞り
	if (count >  11) sony->mltWhiteBalance		= val[ 11];			// ホワイトバランス
	if (count >  12) sony->mltFocusMode			= val[ 12];			// フォーカスモード

	if (count >  18) sony->mltMeteringMode		= val[ 18];			// 測光方式
	if (count >  19) sony->mltFilmSpeed			= val[ 19];			// 撮像感度
	if (count >  20) sony->mltZoneMatching		= val[ 20];			// ゾーン切り替え
	if (count >  21) sony->DRangeOptmizer		= val[ 21];			// Dレンジオプティマイザ
	if (count >  22) sony->mltPhotoFinish		= val[ 22];			// 画像仕上げ
	if (count >  23) sony->mltColorSpace		= val[ 23];			// 色空間
	if (count >  24) sony->mltSharpness			= val[ 24] - 10;	// シャープネス
	if (count >  25) sony->mltContrast			= val[ 25];			// コントラスト補正
	if (count >  26) sony->mltSaturation		= val[ 26];			// 彩度
	if (count >  27) sony->mltHue				= val[ 27];			// 色合い？
	if (count >  56) sony->mltWhiteBalanceTune	= val[ 56];			// ホワイトバランス補正
	if (count >  57) sony->mltColorTemp			= val[ 57] * 100;	// 色温度
	if (count >  63) sony->mltNoiseReduction	= val[ 63];			// ノイズリダクション？
	if (count >  80) sony->mltMeteredManual		= val[ 80];			// メータードマニュアル
	if (count >  87) sony->ImageStabilization	= val[ 87];			// 手ぶれ補正
//	if (count >  90) sony->Orientation			= val[ 90];			// 回転位置
//	if (count >  94) sony->mltColorTemp			= val[ 94];			// 色温度
//	if (count >  97) sony->LeftTakeCount		= val[ 97];			// 撮影可能枚数？

/* @@
	if (count >  11) sony->DriveMode			= val[ 11];	// ドライブモード
	if (count >  26) sony->LocalFocusFrame		= val[ 26];	// ローカルフォーカスフレーム
	if (count >  27) sony->FocusFrame			= val[ 27];	// フォーカスフレーム
	if (count >  31) sony->FlashFired			= val[ 31];	// フラッシュ発光
	if (count >  32) sony->FlashMode			= val[ 32];	// フラッシュモード
	if (count >  33) sony->FlashControl			= val[ 33];	// フラッシュ調光モード
	if (count >  35) sony->FlashComp			= val[ 35];	// フラッシュ調光補正
	if (count >  38) sony->ISOSetting			= val[ 38];	// ISOセッティング
	if (count >  54) sony->Aperture				= val[ 54];	// 露出
	if (count >  73) sony->WhiteBalanceTune		= val[ 73];	// ホワイトバランス補正
	if (count >  74) sony->Hue					= val[ 74];	// 色合い
	if (count > 111) sony->ColorCompFilter		= val[111];	// CCフィルター
	if (count > 158) {										// フォーカスモード
		if (val[ 72] == 0)
			sony->FocusMode					= val[158];
		else
			sony->FocusMode					= 100;
	}
	if (count > 189) sony->AntiShake		= val[189]; // アンチシェイク
@@ */
	delete [] val;
}

void rawmap::makernote_sony_minolta()
{
	register sony_t* sony = &m_exif.sony;

	_imageinfo_t* pimage = NULL;
	int save, tag, type, count;
	int entries = fget2();
	while (entries--) {
		if (eof()) break;

		save = tell();
		tag  = fget2();
		type = fget2();
		count= fget4();

		if (count < 0 || !tiff_seek(type, count)) {
			// このタグはデータ不全なのでスキップ
			seek(save + 12, SEEK_SET);
			continue;
		}

		switch (tag) {
//		case 0x0000:				// UNDEFx4 "MLT0"
//		case 0x0010:				// UNDEFx256
//		case 0x0018:				// UNDEFx4096
//		case 0x0020:				// UNDEFx18928
//		case 0x0040:				// ULONG: イメージデータサイズ?
		case 0x0088:
			// JPEGサムネイルの先頭SOI=ffd8がxxd8になっているので注意
			if (!pimage) pimage = create_ifitem();
			pimage->thumb_offset = 0x0c + (long)get_tagvalue(type);
			pimage->compression = 6;
			jpg_getsize(pimage->thumb_offset, &pimage->width, &pimage->height);
			break;
		case 0x0089:				// ULONG: サムネイルサイズ
			if (!pimage) pimage = create_ifitem();
			pimage->thumb_length = (long)get_tagvalue(type);
			break;
//		case 0x0100:				// ULONG
		case 0x0101:				// ULONG: 画像仕上げ
			sony->mltPhotoFinish = (int)get_tagvalue(type);
			break;
		case 0x0102:				// ULONG: 画質モード
			sony->mltQualityMode = (int)get_tagvalue(type);
			break;
//		case 0x0104:				// SRTNL
//		case 0x0105:				// ULONG
//		case 0x0106:				// ULONG
//		case 0x0109:				// ULONG
		case 0x010A:				// ULONG: ゾーン切り替え
			sony->mltZoneMatching = (int)get_tagvalue(type);
			break;
		case 0x010B:				// ULONG: 色温度
			sony->mltColorTemp = (int)get_tagvalue(type);
			break;
//		case 0x010C:				// ULONG: レンズID
//		case 0x010D:				// ULONG
//		case 0x0111:				// SLONG
//		case 0x0112:				// ULONG
//		case 0x0113:				// ULONG 手ぶれ補正
		case 0x0114:				// UNDEFx260: カメラセッティング情報
			makernote_sony_A100(count / sizeof(short));
			break;
//		case 0x0115:				// ULONG: ホワイトバランス
//										// ---- A100レコードのオフセット11とは値が異なる
//										// 0x0000 オート
//										// 0x0001 色温度指定
//										// 0x0010 太陽光
//										// 0x0020 雲天
//										// 0x0030 日陰
//										// 0x0040 白熱灯
//										// 0x0050 フラッシュ光
//										// 0x0060 蛍光灯
//										// 0x0070 マニュアル
		default:
//@ tagdump(this, tag, type, count);
			break;
		}
		seek(save + 12, SEEK_SET);
	}
}


static BYTE cbSonyDecipherArray[] = {
	0x00,0x01,0x32,0xb1,0x0a,0x0e,0x87,0x28,0x02,0xcc,0xca,0xad,0x1b,0xdc,0x08,0xed,
	0x64,0x86,0xf0,0x4f,0x8c,0x6c,0xb8,0xcb,0x69,0xc4,0x2c,0x03,0x97,0xb6,0x93,0x7c,
	0x14,0xf3,0xe2,0x3e,0x30,0x8e,0xd7,0x60,0x1c,0xa1,0xab,0x37,0xec,0x75,0xbe,0x23,
	0x15,0x6a,0x59,0x3f,0xd0,0xb9,0x96,0xb5,0x50,0x27,0x88,0xe3,0x81,0x94,0xe0,0xc0,
	0x04,0x5c,0xc6,0xe8,0x5f,0x4b,0x70,0x38,0x9f,0x82,0x80,0x51,0x2b,0xc5,0x45,0x49,
	0x9b,0x21,0x52,0x53,0x54,0x85,0x0b,0x5d,0x61,0xda,0x7b,0x55,0x26,0x24,0x07,0x6e,
	0x36,0x5b,0x47,0xb7,0xd9,0x4a,0xa2,0xdf,0xbf,0x12,0x25,0xbc,0x1e,0x7f,0x56,0xea,
	0x10,0xe6,0xcf,0x67,0x4d,0x3c,0x91,0x83,0xe1,0x31,0xb3,0x6f,0xf4,0x05,0x8a,0x46,
	0xc8,0x18,0x76,0x68,0xbd,0xac,0x92,0x2a,0x13,0xe9,0x0f,0xa3,0x7a,0xdb,0x3d,0xd4,
	0xe7,0x3a,0x1a,0x57,0xaf,0x20,0x42,0xb2,0x9e,0xc3,0x8b,0xf2,0xd5,0xd3,0xa4,0x7e,
	0x1f,0x98,0x9c,0xee,0x74,0xa5,0xa6,0xa7,0xd8,0x5e,0xb0,0xb4,0x34,0xce,0xa8,0x79,
	0x77,0x5a,0xc1,0x89,0xae,0x9a,0x11,0x33,0x9d,0xf5,0x39,0x19,0x65,0x78,0x16,0x71,
	0xd2,0xa9,0x44,0x63,0x40,0x29,0xba,0xa0,0x8f,0xe4,0xd6,0x3b,0x84,0x0d,0xc2,0x4e,
	0x58,0xdd,0x99,0x22,0x6b,0xc9,0xbb,0x17,0x06,0xe5,0x7d,0x66,0x43,0x62,0xf6,0xcd,
	0x35,0x90,0x2e,0x41,0x8d,0x6d,0xaa,0x09,0x73,0x95,0x0c,0xf1,0x1d,0xde,0x4c,0x2f,
	0x2d,0xf7,0xd1,0x72,0xeb,0xef,0x48,0xc7,0xf8,0xf9,0xfa,0xfb,0xfc,0xfd,0xfe,0xff
};

static void sony_decipher(BYTE* data, int len, bool bDoubleDecipher)
{
	for (int i = 0; i < len; i++) {
		BYTE cb = cbSonyDecipherArray[data[i]];
		if (bDoubleDecipher) {
			cb = cbSonyDecipherArray[cb];
		}
		data[i] = cb;
	}
}

void rawmap::makernote_sony_camerasettings(int count)
{
	// BigEndian(MM)
	short sorder = m_order;
	m_order = 0x4d4d;
	long* val = new long[count];
	for (int i = 0; i < count; i++) {
		val[i] = (int)(short)fget2();
//@ dbg.printf(L"%03d: %04X\n", i, (unsigned short)val[i]);
	}
	m_order = sorder;

	register sony_t* sony = &m_exif.sony;
	sony->mltExposureTime		= val[  0];			// 露出時間
	sony->mltAperture			= val[  1];			// 絞り
	sony->mltExpCompensation	= val[  3];			// 露出補正 { (val - 0x80) / 24.0F }
	sony->FocusMode				= val[0x10];		// フォーカスモード
	sony->mltFilmSpeed			= val[0x16];		// 撮像感度
	sony->mltSharpness			= val[0x1c] - 10;	// シャープネス
	sony->mltContrast			= val[0x1d] - 10;	// コントラスト補正
	sony->mltSaturation			= val[0x1e] - 10;	// 彩度

	delete [] val;
}

void rawmap::makernote_sony_camerasettings2(int count)
{
	// BigEndian(MM)
	short sorder = m_order;
	m_order = 0x4d4d;
	long* val = new long[count];
	for (int i = 0; i < count; i++) {
		val[i] = (int)(short)fget2();
//@ dbg.printf(L"%03d: %04X\n", i, (unsigned short)val[i]);
	}
	m_order = sorder;

	register sony_t* sony = &m_exif.sony;
	sony->mltExposureTime		= val[  0];			// 露出時間
	sony->mltAperture			= val[  1];			// 絞り

	sony->FocusMode				= val[0x0f];		// フォーカスモード
	sony->mltSharpness			= val[0x19] - 10;	// シャープネス
	sony->mltContrast			= val[0x1a] - 10;	// コントラスト補正
	sony->mltSaturation			= val[0x1b] - 10;	// 彩度

	delete [] val;
}

void rawmap::makernote_sony_camerasettings3(int count)
{
	// LittleEndian(II)
	short sorder = m_order;
	m_order = 0x4949;
	long* val = new long[count];
	for (int i = 0; i < count; i++) {
		val[i] = (int)(short)fget2();
//@ dbg.printf(L"%03d: %04X\n", i, (unsigned short)val[i]);
	}
	m_order = sorder;

	register sony_t* sony = &m_exif.sony;
	sony->mltExposureTime		= val[  0];			// 露出時間
	sony->mltAperture			= val[  1];			// 絞り

	delete [] val;
}

void rawmap::makernote_sony_camerasettings_unknown(int count)
{
	// nothing to do
}

void rawmap::makernote_sony_tag0114(int count)
{
	if (count == 280 || count == 364) {
		// A200/A300/A350/A700=280, A850/A900=364
		makernote_sony_camerasettings(count);
	} else if (count == 332) {
		// A230/A290/A330/A380/A390=332
		makernote_sony_camerasettings2(count);
	} else if (count == 1536 || count == 2048) {
		// A560/A580/A33/A35/A55/NEX3/5/5C/C3/VG10E=1536, A450/A500/A550=2048
		makernote_sony_camerasettings3(count);
	} else {
		makernote_sony_camerasettings_unknown(count);
	}
}

void rawmap::makernote_sony_tag9050(int count)
{
//@dbg.printf(L"tag9050: pos=%p\n", tell());
	BYTE* data = new BYTE[count];
	long n = read(data, count);
	sony_decipher(data, count, false);
//@dbg.dump(data, count);

	register sony_t* sony = &m_exif.sony;
	int lensMount = data[0x105];
	if (lensMount == 2) {
		sony->LensType = 1;
		sony->LensID = *((USHORT*)(&data[0x107]));
	} else if (lensMount == 1) {
		sony->LensType = 0;
		sony->LensID = *((USHORT*)(&data[0x109]));
	}
	delete [] data;
}

void rawmap::makernote_sony_tag9405(int count)
{
//@dbg.printf(L"tag9405: pos=%p\n", tell());
	BYTE* data = new BYTE[count];
	long n = read(data, count);
	BYTE dataType = data[0];
	sony_decipher(data, count, false);
//@dbg.dump(data, count);

	register sony_t* sony = &m_exif.sony;
	if (dataType == 0x1b) {
		int lensMount = data[0x604];
		if (lensMount == 2) {
			sony->LensType = 1;
			sony->LensID = *((USHORT*)(&data[0x605]));
		} else if (lensMount == 1) {
			sony->LensType = 0;
			sony->LensID = *((USHORT*)(&data[0x608]));
		}
	} else if (dataType == 0x3a) {
		int lensMount = data[0x5e];
		if (lensMount == 2) {
			sony->LensType = 1;
			sony->LensID = *((USHORT*)(&data[0x60]));
		} else if (lensMount == 1) {
			sony->LensType = 0;
			sony->LensID = *((USHORT*)(&data[0x62]));
		}
	}
	delete [] data;
}

void rawmap::makernote_sony_tag940A(int count)
{
//@dbg.printf(L"tag940A: pos=%p\n", tell());
	BYTE* data = new BYTE[count];
	long n = read(data, count);
	sony_decipher(data, count, false);
//@dbg.dump(data, count);

	delete [] data;
}

void rawmap::makernote_sony_tag940C(int count)
{
//@dbg.printf(L"tag940C: pos=%p\n", tell());
	BYTE* data = new BYTE[count];
	long n = read(data, count);
	sony_decipher(data, count, false);
//@dbg.dump(data, count);

	register sony_t* sony = &m_exif.sony;
	int lensMount = data[8];
	if (lensMount != 0) {
		sony->LensType = 1;
		sony->LensID = *((USHORT*)(&data[9]));
	}
	delete [] data;
}

void rawmap::makernote_sony_tag9412(int count)
{
//@dbg.printf(L"tag9412: pos=%p\n", tell());
	BYTE* data = new BYTE[count];
	long n = read(data, count);
	sony_decipher(data, count, false);
//@dbg.dump(data, count);

	delete [] data;
}

void rawmap::makernote_sony(int base)
{
	BYTE cb[32];
	read(cb, 12);
	if (memcmp(cb, "SONY DSC", 8)) {
		if (!IS_SONY_ALPHA(m_exif.ModelID)) {
			m_exif.exist_makenote = false;
			return;
		}
		// A700のRAWファイルにはマーカーが無い?
		seek(-12, SEEK_CUR);
	}

	register sony_t* sony = &m_exif.sony;
	memset(sony, 0, sizeof(sony_t));
	sony->FlashExposureCompensation = FLT_MAX;
	sony->TeleConverter = INT_MAX;
	sony->ExposureMode = INT_MAX;
	sony->FocusMode = INT_MAX;
	sony->AutoFocusArea = INT_MAX;
	sony->WhiteBalanceFineTune = INT_MAX;
	sony->WhiteBalance = INT_MAX;
	sony->ColorReproductionMode = INT_MAX;
	sony->Contrast = INT_MAX;
	sony->Saturation = INT_MAX;
	sony->Sharpness = INT_MAX;
	sony->Brightness = INT_MAX;
	sony->LongExposureNR = INT_MAX;
	sony->HighISONR = INT_MAX;
	sony->HDR = INT_MAX;
	sony->MultiShotNR = INT_MAX;
	sony->ColorTemp = INT_MAX;
	sony->SceneMode = INT_MAX;
	sony->ZoneMatching = INT_MAX;
	sony->CreativeStyle = INT_MAX;
	sony->FlashComp = INT_MAX;
	sony->ReleaseMode = INT_MAX;
	sony->DRangeOptmizer = INT_MAX;
	sony->ImageStabilization = INT_MAX;
	sony->LensID = INT_MAX;
	sony->QualityMode = INT_MAX;
	sony->MacroMode = INT_MAX;

	sony->mltPhotoFinish = INT_MAX;
	sony->mltQualityMode = INT_MAX;
	sony->mltZoneMatching = INT_MAX;
	sony->mltColorTemp = INT_MAX;

	sony->mltExpCompensation = INT_MAX;
	sony->mltExposureMode = INT_MAX;
	sony->mltExposureTime = INT_MAX;
	sony->mltAperture = INT_MAX;
	sony->mltWhiteBalance = INT_MAX;
	sony->mltFocusMode = INT_MAX;
	sony->mltMeteringMode = INT_MAX;
	sony->mltFilmSpeed = INT_MAX;
	sony->mltColorSpace = INT_MAX;
	sony->mltSharpness = INT_MAX;
	sony->mltContrast = INT_MAX;
	sony->mltSaturation = INT_MAX;
	sony->mltHue = INT_MAX;
	sony->mltWhiteBalanceTune = INT_MAX;
	sony->mltNoiseReduction = INT_MAX;
	sony->mltMeteredManual = INT_MAX;

	_imageinfo_t* pimage = NULL;
	int val, save, tag, type, count;
	int entries = fget2();
	while (entries--) {
		if (eof()) break;

		save = tell();
		tag  = fget2();
		type = fget2();
		count= fget4();

		if (count < 0 || !tiff_seek(type, count)) {
			// このタグはデータ不全なのでスキップ
			seek(save + 12, SEEK_SET);
			continue;
		}

		switch (tag) {
/* --- A200/350 エントリ ------------------------------------------
		case 0x0010:					// UNDEF x n
		case 0x0018:					// UNDEF x n
		case 0x0020:					// UNDEF x n
		case 0x2002:					// ULONG
		case 0x2003:					// ASCII x n
		case 0xB022:					// ULONG
		case 0xB02A:					// BYTE x 8
----------------------------------------------------------------- */
		case 0x0102:					// ULONG 
			sony->QualityMode = (int)get_tagvalue(type);
			break;
		case 0x0104:					// SRTNL Flash Exposure Compensation
			sony->FlashExposureCompensation = (float)get_tagvalue(type);
			break;
		case 0x0105:					// ULONG テレコン
			sony->TeleConverter = (int)get_tagvalue(type);
			break;
		case 0x0112:					// LONG WhiteBalanceFineTune
			sony->WhiteBalanceFineTune = (int)get_tagvalue(type);
			break;
		case 0x0114:					// UNDEF x n
			makernote_sony_tag0114(count);
			break;
		case 0x0115:					// ULONG ホワイトバランス
			sony->WhiteBalance = (int)get_tagvalue(type);
			break;
		case 0x2000:					// 色再現モード
			sony->ColorReproductionMode = (int)get_tagvalue(type);
			break;
		case 0x2001:					// BYTE x n サムネイル（大）
			// α700
			// JPEGサムネイルの先頭SOI=ffd8が00d8になっているので注意
			if (!pimage) pimage = create_ifitem();
			pimage->thumb_length = fget4();
			pimage->width = fget4();
			pimage->height = fget4();
			pimage->compression = 6;
			pimage->thumb_offset = 20 + tell();
			break;
		case 0x2004:					// LONG: コントラスト
			sony->Contrast = (int)get_tagvalue(type);
			break;
		case 0x2005:					// LONG: 彩度
			sony->Saturation = (int)get_tagvalue(type);
			break;
		case 0x2006:					// LONG: シャープネス
			sony->Sharpness = (int)get_tagvalue(type);
			break;
		case 0x2007:					// LONG: 明るさ
			sony->Brightness = (int)get_tagvalue(type);
			break;
		case 0x2008:					// ULONG: LongExposureNoiseReduction
			val = (int)((DWORD)get_tagvalue(type) & 0xffff);
			if (val != USHRT_MAX)
				sony->LongExposureNR = val;
			break;
		case 0x2009:					// USHRT: HighISONoiseReduction
			val = (int)get_tagvalue(type);
			if (val != USHRT_MAX)
				sony->HighISONR = val;
			break;
		case 0x200a:					// ULONG HDR
			sony->HDR = (int)get_tagvalue(type);
			break;
		case 0x200b:					// ULONG マルチショットNoiseReduction
			val = (int)get_tagvalue(type);
			if (val != UCHAR_MAX)
				sony->MultiShotNR = val;
			break;
		case 0x9050:					// UNDEF x n
			makernote_sony_tag9050(count);
			break;
		case 0x9405:					// UNDEF x n
			makernote_sony_tag9405(count);
			break;
		case 0x940A:					// UNDEF x n
			makernote_sony_tag940A(count);
			break;
		case 0x940C:					// UNDEF x n
			makernote_sony_tag940C(count);
			break;
		case 0x9412:					// UNDEF x n
			makernote_sony_tag9412(count);
			break;
		case 0xB020:					// ASCIIx9 ColorReproduction
			read_ascii(sony->ColorReproduction, count, ARRAYSIZE(sony->ColorReproduction));
			break;
		case 0xB021:					// ULONG: 色温度
			val  = (int)get_tagvalue(type);
			if (val != INT_MIN)
				sony->ColorTemp = val;
			break;
		case 0xB023:					// ULONG: シーンモード
			val = (int)get_tagvalue(type);
			if (val != USHRT_MAX)
				sony->SceneMode = val;
			break;
		case 0xB024:					// ULONG: ゾーン切り替え
			sony->ZoneMatching = (int)get_tagvalue(type);
			break;
		case 0xB025:					// ULONG: Dレンジオプティマイザ
			sony->DRangeOptmizer = (int)get_tagvalue(type);
			break;
		case 0xB026:					// ULONG: 手ブレ補正
			sony->ImageStabilization = (int)get_tagvalue(type);
			break;
		case 0xB027:					// ULONG: レンズID
			sony->LensID = (int)get_tagvalue(type);
			break;
		case 0xB028:					// ULONG: マーカー？
			val = (int)get_tagvalue(type);
			seek(val + base, SEEK_SET);
			makernote_sony_minolta();
			break;
		case 0xB029:					// ULONG: カラーモード
			val  = (int)get_tagvalue(type);
			if (val != INT_MIN)
				sony->CreativeStyle = val;
			break;
		case 0xB040:					// USHRT: Macro 0=off, 1=on
			val = (int)get_tagvalue(type);
			if (val != USHRT_MAX)
				sony->MacroMode = val;
			break;
		case 0xB041:					// USHRT: ExposureMode
			val = (int)get_tagvalue(type);
			if (val != USHRT_MAX)
				sony->ExposureMode = val;
			break;
		case 0xB042:					// USHRT: FocusMode
			val = (int)get_tagvalue(type);
			if (val != USHRT_MAX)
				sony->FocusMode = val;
			break;
		case 0xB043:					// USHRT: FocusMode
			val = (int)get_tagvalue(type);
			if (val != USHRT_MAX)
				sony->AutoFocusArea = val;
			break;
		case 0xB047:					// USHRT: Quality
			if (!HasInt(sony->QualityMode)) {
				val = (int)get_tagvalue(type);
				if (val != USHRT_MAX)
					sony->QualityMode = val;
			}
			break;
		case 0xB048:					// SSHRT: 調光補正
			val = (int)(short)get_tagvalue(type);
			if (val != SHRT_MIN)
				sony->FlashComp = val;
			break;
		case 0xB049:					// USHRT: リリースモード
			val = (int)(short)get_tagvalue(type);
			if (val != USHRT_MAX)
				sony->ReleaseMode = val;
			break;
		case 0xB04E:					// USHRT: 長秒時ノイズリダクション
			val = (int)get_tagvalue(type);
			if (val != USHRT_MAX)
				sony->LongExposureNR = val;
			break;
/* @@@
		case 0xB042:					// USHRT:
		case 0xB043:					// USHRT:
		case 0xB044:					// USHRT:
		case 0xB045:					// USHRT:
		case 0xB046:					// USHRT:
		case 0xB049:					// USHRT:
		case 0xB04A:					// USHRT:
		case 0xB04B:					// USHRT:
		case 0xB04D:					// USHRT:
@@@ */
		default:
//@tagdump(this, tag, type, count);
			break;
		}
		seek(save + 12, SEEK_SET);
	}
}

void rawmap::makernote_ricoh_imageinfo(int count)
{
	if (count < 41) return;

	BYTE cb[32];

	// Ricoh Image Info IFD
	register ricoh_t* ricoh = &m_exif.ricoh;

	ricoh->ImageWidth = fget2();					// offset 0 ImageWidth size 2
	ricoh->ImageHeight = fget2();					// offset 2 ImageHeight size 2
		fget2();									// skip
	read(cb, 7);									// offset 6 DateTime size 7
		getc();										// offset 13 size 1
		fget2();									// offset 14 size 2
		fget2();									// offset 16 size 2
		fget2();									// offset 18 size 2
		fget2();									// offset 20 size 2
		fget2();									// offset 22 size 2
		fget2();									// offset 24 size 2
		fget2();									// offset 26 size 2

	int thumb_offset = fget2();						// offset 28 size 2
	int thumb_length = fget2();						// offset 30 size 2
	if (thumb_offset && thumb_length) {
		_imageinfo_t* pimage = create_ifitem();
		pimage->thumb_offset = thumb_offset;
		pimage->thumb_length = thumb_length;
	}
	ricoh->FlashMode = getc();						// offset 32 size 1
	ricoh->MacroMode = getc();						// offset 33 size 1
	ricoh->Sharpness = getc();						// offset 34 size 1
		getc();										// offset 35 size 1
		getc();										// offset 36 size 1
		getc();										// offset 37 size 1
	ricoh->WhiteBalance = getc();					// offset 38 size 1
	ricoh->ISOSetting = getc();						// offset 39 size 1
	ricoh->Saturation = getc();						// offset 40 size 1

	if (count < 49) return;
		getc();										// offset 41 size 1
		getc();										// offset 42 size 1
		getc();										// offset 43 size 1
		getc();										// offset 44 size 1
		getc();										// offset 45 size 1
		getc();										// offset 46 size 1
		getc();										// offset 47 size 1
	ricoh->ToneEffect = getc();						// offset 48 size 1, 0=normal, 1=red, 2=green, 3=blue, 4=purple, 6=sepia
}

void rawmap::makernote_ricoh_camerainfo()
{
	// Ricoh Camera Info IFD
	BYTE cb[20];
	read(cb, 20);
	if (memcmp(cb, "[Ricoh Camera Info]", 19)) {
		return;
	}

	register ricoh_t* ricoh = &m_exif.ricoh;

	int entries = fget2();
	while (entries--) {
		if (eof()) break;

		int save = tell();
		int tag  = fget2();
		int type = fget2();
		int count= fget4();

		if (count < 0 || !tiff_seek(type, count)) {
			// このタグはデータ不全なのでスキップ
			seek(save + 12, SEEK_SET);
			continue;
		}

		switch (tag) {
		default:
//			tagdump(this, tag, type, count);
			break;
		}
		seek(save + 12, SEEK_SET);
	}
}

void rawmap::makernote_ricoh()
{
	BYTE cb[32];
	read(cb, 8);
	if (memcmp(cb, "Ricoh", 5)) {
		m_exif.exist_makenote = false;
		return;
	}

	register ricoh_t* ricoh = &m_exif.ricoh;
	memset(ricoh, 0, sizeof(ricoh_t));
	ricoh->ImageWidth = INT_MAX;
	ricoh->ImageHeight = INT_MAX;
	ricoh->FlashMode = INT_MAX;
	ricoh->MacroMode = INT_MAX;
	ricoh->Sharpness = INT_MAX;
	ricoh->WhiteBalance = INT_MAX;
	ricoh->ISOSetting = INT_MAX;
	ricoh->Saturation = INT_MAX;
	ricoh->ToneEffect = INT_MAX;

	short sorder = m_order;					// save order
	m_order = 0x4d4d;						// 'MM' フォーマット ?

	int entries = fget2();
	while (entries--) {
		if (eof()) break;

		int save = tell();
		int tag  = fget2();
		int type = fget2();
		int count= fget4();

		if (count < 0 || !tiff_seek(type, count)) {
			// このタグはデータ不全なのでスキップ
			seek(save + 12, SEEK_SET);
			continue;
		}

		switch (tag) {
		case 0x0001:						// ASCII: データ形式 "Rdc"
			read_ascii(ricoh->DataFormat, count, ARRAYSIZE(ricoh->DataFormat));
			break;
		case 0x0002:						// ASCII: ファームウェア
			read_ascii(ricoh->Firmware, count, ARRAYSIZE(ricoh->Firmware));
			break;
/* @@
		case 0x0003:						// USHRTx4:
			break;
		case 0x0004:						// ULONG
			break;
		case 0x0005:						// UNDEFx16
			break;
		case 0x0006:						// UNDEFx16
			break;
		case 0x1002:						// ULONG
			break;
		case 0x1003:						// ULONG
			break;
@@ */
		case 0x1001:						// UNDEFxN
			makernote_ricoh_imageinfo(count);
			break;
		case 0x2001:						// UNDEFx490 "[Ricoh Camera Info]"
			makernote_ricoh_camerainfo();
			break;
		default:
//@ tagdump(this, tag, type, count);
			break;
		}

		seek(save + 12, SEEK_SET);
	}

	m_order = sorder;						// リストア
}

void rawmap::makernote_mamiya()
{
	register mamiya_t* mamiya = &m_exif.mamiya;
	memset(mamiya, 0, sizeof(mamiya_t));

	mamiya->ExposureMode = INT_MAX;
	mamiya->ISO = INT_MAX;
	mamiya->ColorMode = INT_MAX;
	mamiya->WhiteBalance = INT_MAX;
	mamiya->Tone = INT_MAX;

	int entries = fget2();
	while (entries--) {
		if (eof()) break;

		int save = tell();
		int tag  = fget2();
		int type = fget2();
		int count= fget4();

		if (count < 0 || !tiff_seek(type, count)) {
			// このタグはデータ不全なのでスキップ
			seek(save + 12, SEEK_SET);
			continue;
		}

		switch (tag) {
		case 0x0090:												// USHRT: 露出モード
			mamiya->ExposureMode = (int)get_tagvalue(type);
			break;
//		case 0x0091:			// USHRT
//		case 0x0092:			// USHRT
//		case 0x0093:			// USHRT
//		case 0x0094:			// USHRT
		case 0x0095:												// USHRT: ISO
			mamiya->ISO = (int)get_tagvalue(type);
			break;
		case 0x0098:												// USHRT: カラーモード
			mamiya->ColorMode = (int)get_tagvalue(type);	
			break;
//		case 0x0099:			// UNDEFx288
//		case 0x009D:			// USHRT
//		case 0x009E:			// USHRT
//		case 0x009F:			// USHRT
//		case 0x00A0:			// USHRT
//		case 0x00A1:			// USHRT
		case 0x00A2:												// USHRT: ホワイトバランス
			mamiya->WhiteBalance = (int)get_tagvalue(type);
			break;
//		case 0x00A3:			// USHRT
//		case 0x00A4:			// USHRT
		case 0x00A5:												// USHRT: トーン
			mamiya->Tone = (int)get_tagvalue(type);
			break;
//		case 0x00A7:			// USHRT
//		case 0x00A8:			// USHRT
//		case 0x00A9:			// ASCII
//		case 0x00AA:			// ASCII
//		case 0x00AB:			// ASCII
//		case 0x00AC:			// UNDEFx4
//		case 0x00AD:			// UNDEFx4
//		case 0x00AE:			// UNDEF
		default:
//@			tagdump(this, tag, type, count);
			break;
		}

		seek(save + 12, SEEK_SET);
	}
}

void rawmap::makernote_leica()
{
	if (IS_LIECA_PANASONIC_COMPATIBLE(m_exif.ModelID)) {
		// panasonic 互換
		makernote_panasonic();
		return;
	}
	BYTE cb[32];
	read(cb, 8);
	if (memcmp(cb, "LEICA\0", 6)) {
		m_exif.exist_makenote = false;
		return;
	}

	register leica_t* leica = &m_exif.leica;
	memset(leica, 0, sizeof(leica_t));

	leica->LensType = INT_MAX;
	leica->WBColorTemp = INT_MAX;;
	leica->ImageID = INT_MAX;;

	int entries = fget2();
	while (entries--) {
		if (eof()) break;

		int save = tell();
		int tag  = fget2();
		int type = fget2();
		int count= fget4();

		if (count < 0 || !tiff_seek(type, count)) {
			// このタグはデータ不全なのでスキップ
			seek(save + 12, SEEK_SET);
			continue;
		}

		switch (tag) {
//		case 0x0302:			// ULONG
//		case 0x0303:			// ULONG
		case 0x0304:			// USHORT: ホワイトバランス
			leica->WhiteBalance = (int)get_tagvalue(type);
			break;
		case 0x0310:			// ULONG: レンズ検出
			leica->LensType = (int)get_tagvalue(type);
			break;
//		case 0x0311:			// SRATIN
//		case 0x0312:			// SRATIN
//		case 0x0313:			// SRATIN
		case 0x0321:			// ULONG: ホワイトバランス色温度
			leica->WBColorTemp = (int)get_tagvalue(type);
			break;
//		case 0x0325:			// ULONG
//		case 0x0330:			// ULONG
//		case 0x0331:			// ULONG
//		case 0x0332:			// ULONG
//		case 0x0333:			// ULONG
		case 0x0340:			// ULONG: イメージ固有ID
			leica->ImageID = (int)get_tagvalue(type);
			break;
		default:
//			tagdump(this, tag, type, count);
			break;
		}

		seek(save + 12, SEEK_SET);
	}
}

void rawmap::makernote_kddi_ca()
{
	BYTE cb[32];
	read(cb, 8);
	if (memcmp(cb, "KDDI", 4)) {
		m_exif.exist_makenote = false;
		return;
	}

	int entries = fget2();
	while (entries--) {
		if (eof()) break;

		int save = tell();
		int tag  = fget2();
		int type = fget2();
		int count= fget4();

		if (count < 0 || !tiff_seek(type, count)) {
			// このタグはデータ不全なのでスキップ
			seek(save + 12, SEEK_SET);
			continue;
		}

		switch (tag) {
		default:
//@			tagdump(this, tag, type, count);
			break;
		}

		seek(save + 12, SEEK_SET);
	}
}

void rawmap::makernote_hassel()
{
	register hassel_t* hassel = &m_exif.hassel;
	memset(hassel, 0, sizeof(hassel_t));

	int entries = fget2();
	while (entries--) {
		if (eof()) break;

		int save = tell();
		int tag  = fget2();
		int type = fget2();
		int count= fget4();

		if (count < 0 || !tiff_seek(type, count)) {
			// このタグはデータ不全なのでスキップ
			seek(save + 12, SEEK_SET);
			continue;
		}

		switch (tag) {
//		case 0x0005:					// ULONG: 
//		case 0x0007:					// SLONG:
//		case 0x0008:					// SLONG:
//		case 0x000F:					// ULONG x 7:
//		case 0x0011:					// ULONG:
//		case 0x0012:					// ULONG:
//		case 0x0013:					// ULONG:
//		case 0x0014:					// ULONG:
//		case 0x0015:					// ASCII: ex. "Hasselblad CFV"
//		case 0x0016:					// ULONG:
//		case 0x0017:					// UNDEF x n:
//		case 0x0018:					// UNDEF x n:
//		case 0x0019:					// UNDEF x n:
//		case 0x001A:					// UNDEF x n:
//		case 0x001B:					// USHRT x 2:
		default:
//			tagdump(this, tag, type, count);
			break;
		}
		seek(save + 12, SEEK_SET);
	}
}

void rawmap::makernote_default(int base)
{
	// The MakerNote might have its own TIFF header (possibly with
	// its own byte-order!), or it might just be a table.

	int entries = fget2();
	while (entries--) {
		if (eof()) break;

		int save = tell();
		int tag  = fget2();
		int type = fget2();
		int count= fget4();

		if (count < 0 || !tiff_seek(type, count)) {
			// このタグはデータ不全なのでスキップ
			seek(save + 12, SEEK_SET);
			continue;
		}

		seek(save + 12, SEEK_SET);
	}
}

void rawmap::parse_adobe(int base)
{
	short sorder = m_order;
	m_order = 0x4d4d;
	while (1) {
		if (eof()) break;

		DWORD marker = fget4();
		if (marker != 0x3842494d) break;			// "8BIM" ?

		int tag = fget2();							// タグ
		fget2();									// 読み飛ばし
		int count = fget4();						// サイズ
		int save = tell();

		if (tag == 0x040c) {						// サムネイル
			int type = fget4();
			int width = fget4();
			int height = fget4();
			fget4();
			fget4();
			int size = fget4();
			fget4();
			if (type == 1) {
				_imageinfo_t* pimage = create_ifitem();
				pimage->width = width;
				pimage->height = height;
				pimage->thumb_offset = tell();
				pimage->thumb_length = size;
				pimage->compression = 6;
			}
			break;
		}
		if (count & 1) count++;
		seek(save + count, SEEK_SET);
	}
	m_order = sorder;
}

void rawmap::parse_exif(int base)
{
	int entries = fget2();
	while (entries--) {
		if (eof()) break;

		int save = tell();
		int tag  = fget2();
		int type = fget2();
		int count= fget4();

		if (count < 0 || !tiff_seek(type, count)) {
			// このタグはデータ不全なのでスキップ
			seek(save + 12, SEEK_SET);
			continue;
		}

		switch (tag) {
		case TAG_EXIF_VERSION:				// UNDEFx4: EXIFバージョン
			break;
		case TAG_DATETIME_ORIGINAL:			// ASCII:撮影日時
			read_ascii(m_exif.Date, count, ARRAYSIZE(m_exif.Date));
			break;
		case TAG_DATETIME_DIGITIZED:		// ASCII:デジタイズ日時
			read_ascii(m_exif.DateDigitized, count, ARRAYSIZE(m_exif.DateDigitized));
			break;
		case TAG_EXPOSURE_TIME:				// 0x829A RATIONAL: 露出時間
			m_exif.ExposureTime = (float)get_tagvalue(type);
			break;
		case TAG_FNUMBER:					// 0x829D RATIONAL: Fナンバー
			m_exif.FNumber = (float)get_tagvalue(type);
			break;
		case TAG_EXPOSURE_PROGRAM:			// 0x8822 USHRT: 露出プログラム
			m_exif.ExposureProgram = (int)get_tagvalue(type);
			break;
		case TAG_ISO_EQUIVALENT:			// 0x8827 USHRT: ISO感度
			m_exif.ISOEquiv = (int)get_tagvalue(type);
			if (m_exif.ISOEquiv < 50) m_exif.ISOEquiv *= 200;
			break;
		case TAG_SHUTTER_SPEED:				// 0x9201 SRATIONAL: シャッター速度 (APEX)
			m_exif.ShutterSpeed = (float)(1 / exp(get_tagvalue(type) * log(2.0)));
			break;
		case TAG_APERTURE:					// 0x9202 RATIONAL: 絞り値 (APEX)
			m_exif.Aperture = (float)exp(get_tagvalue(type) * log(2.0) * 0.5);
			break;
		case TAG_BRIGHTNESS_VALUE:			// 0x9203 SRATIN: 対象物の明るさ
			m_exif.BrightnessValue = (float)get_tagvalue(type);
			break;
		case TAG_EXPOSURE_BIAS:				// 0x9204 SRATIONAL: 露光補正値
			m_exif.ExposureBias = (float)get_tagvalue(type);
			break;
		case TAG_MAX_APERTURE:				// 0x9205 RATIONAL: レンズ最小F値 (APEX)
			m_exif.MaxAperture = (float)exp(get_tagvalue(type) * log(2.0) * 0.5);
			break;
		case TAG_SUBJECT_DISTANCE:			// 0x9206 RTINL: 被写体距離
			m_exif.SubjectDistance = (float)get_tagvalue(type);
			break;
		case TAG_METERING_MODE:				// 0x9207 USHRT: 測光方式
			m_exif.MeteringMode = (int)get_tagvalue(type);
			break;
		case TAG_LIGHT_SOURCE:				// 0x9208 SHORT: 光源
			m_exif.LightSource = (int)get_tagvalue(type);
			break;
		case TAG_FLASH:						// 0x9209 SHORT: フラッシュ
			m_exif.Flash = (int)get_tagvalue(type);
			break;
		case TAG_FOCAL_LENGTH:				// 0x920A RATIONAL: レンズ焦点距離
			m_exif.FocalLength = (float)get_tagvalue(type);
			break;
//		case TAG_SUBJECT_AREA:				// 0x9214 SHORT x n: 被写体エリア
													// SHORT x 2 : x,yによる点状の被写体位置
													// SHORT x 3 : x,y,rによる円状の被写体位置
													// SHORT x 4 : x,y,width,heightによる長方形状の被写体位置。ただしx,yは中心点
		case TAG_USERCOMMENT:				// UNDEF:ユーザコメント
			break;
		case TAG_COLOR_SPACE:				// SHORT:カラースペース
			m_exif.ColorSpace = (int)get_tagvalue(type);
			break;
		case TAG_EXIF_IMAGEWIDTH:			// SHORT:EXIFイメージの高さ
			m_exif.ImageWidth = (int)get_tagvalue(type);
			break;
		case TAG_EXIF_IMAGELENGTH:			// SHORT:EXIFイメージの幅
			m_exif.ImageHeight = (int)get_tagvalue(type);
			break;
		case TAG_SENSING_METHOD:			// SHORT:SensingMethod
			break;
		case TAG_EXPOSURE_INDEX:			// ISO感度
			if (!HasInt(m_exif.ISOEquiv)) {
				m_exif.ISOEquiv = (int)get_tagvalue(type);
				if (m_exif.ISOEquiv < 50) m_exif.ISOEquiv *= 200;
			}
			break;
		case TAG_SUBJECT_LOCATION:			// 被写体位置
			break;
		case TAG_FOCALPLANE_XRES:			// 焦点面の幅の解像度
			m_exif.FocalPlaneXResolution = (float)get_tagvalue(type);
			break;
		case TAG_FOCALPLANE_YRES:			// 焦点面の高さの解像度
			m_exif.FocalPlaneYResolution = (float)get_tagvalue(type);
			break;
		case TAG_FOCALPLANE_UNITS:			// 画像の幅と高さの解像度の単位
			switch ((int)get_tagvalue(type)) {
			case 1:
			case 2: m_exif.FocalPlaneResolutionUnits = 25.4F;  break;		// inch
			case 3: m_exif.FocalPlaneResolutionUnits = 10.0F;  break;		// centimeter
			case 4: m_exif.FocalPlaneResolutionUnits = 1.0F;   break;		// milimeter
			case 5: m_exif.FocalPlaneResolutionUnits = 0.001F; break;		// micrometer
			}
			break;
		case TAG_EXPOSURE_MODE:				// 0xA402 SHORT: 露光モード
			m_exif.ExposureMode = (int)get_tagvalue(type);
			break;
		case TAG_WHITE_BALANCE:				// 0xA403 SHORT:ホワイトバランスモード
			m_exif.WhiteBalance = (int)get_tagvalue(type);
			break;
		case TAG_DIGITAL_ZOOM:				// RATIN:デジタルズーム RATIONAL
			m_exif.DigitalZoom = (float)get_tagvalue(type);
			break;
		case TAG_FOCALLENGTH_35:			// SHORT:35mm換算の焦点距離
			m_exif.FocalLength35 = (float)get_tagvalue(type);
			if (m_exif.FocalLength35 == 0.0F) m_exif.FocalLength35 = FLT_MAX;
			break;
		case TAG_SCENE_CAPTURE_TYPE:		// SHORT:撮影シーンタイプ
			m_exif.SceneCaptureType = (int)get_tagvalue(type);
			break;
		case TAG_GAIN_CONTROL:				// SHORT:ゲインコントロール
			m_exif.GainControl = (int)get_tagvalue(type);
			break;
		case TAG_CONTRAST:					// SHORT:コントラスト
			m_exif.Contrast = (int)get_tagvalue(type);
			break;
		case TAG_SATURATION:				// SHORT:彩度
			m_exif.Saturation = (int)get_tagvalue(type);
			break;
		case TAG_SHARPNESS:					// SHORT:シャープネス
			m_exif.Sharpness = (int)get_tagvalue(type);
			break;
		case TAG_DISTANCE_RANGE:			// SHORT:被写体の距離範囲
			m_exif.DistanceRange = (int)get_tagvalue(type);
			break;
		case TAG_GAMMA:						// RATIONAL:ガンマ値
			m_exif.GammaValue = (float)get_tagvalue(type);
			break;
		case TAG_MAKERNOTE:					// メーカーノート
			m_exif.exist_makenote = true;
			switch (m_exif.MakerID) {
			case MAKER_CANON:		makernote_canon(); break;
			case MAKER_NIKON:		makernote_nikon(); break;
			case MAKER_MINOLTA:		makernote_minolta(base); break;
			case MAKER_OLYMPUS:		makernote_olympus(); break;
			case MAKER_CASIO:		makernote_casio(); break;
			case MAKER_FUJI:		makernote_fuji(); break;
			case MAKER_PANASONIC:	makernote_panasonic(); break;
			case MAKER_SANYO:		makernote_sanyo(); break;
			case MAKER_PENTAX:		makernote_pentax(); break;
			case MAKER_SIGMA:		makernote_sigma(); break;
			case MAKER_EPSON:		makernote_epson(); break;
			case MAKER_SONY:		makernote_sony(base); break;
			case MAKER_RICOH:		makernote_ricoh(); break;
			case MAKER_MAMIYA:		makernote_mamiya(); break;
			case MAKER_LEICA:		makernote_leica(); break;
			case MAKER_KDDI_CA:		makernote_kddi_ca(); break;
			case MAKER_HASSEL:		makernote_hassel(); break;
			case MAKER_KODAK:
			default: break;
			}
		case TAG_FILE_SOURCE:				// UNDEX: ファイルソース
			break;
		default:
//@	tagdump(this, tag, type, count);
			break;
		}
		seek(save + 12, SEEK_SET);
	}

}

void rawmap::parse_gps(int base)
{
	gps_t& gps = m_exif.gps;
	gps.exist_gps = true;

	int entries = fget2();
	while (entries--) {
		if (eof()) break;

		int save = tell();
		int tag  = fget2();
		int type = fget2();
		int count= fget4();

		if (count < 0 || !tiff_seek(type, count)) {
			// このタグはデータ不全なのでスキップ
			seek(save + 12, SEEK_SET);
			continue;
		}

		int i;
		switch (tag) {
		case GPSVersionID:							// 0x00, BYTEx4: GPS タグのバージョン
			count = min(ARRAYSIZE(gps.GPSVersionID), count);
			read(gps.GPSVersionID, count);
			break;
		case GPSLatitudeRef:						// 0x01, ASCIIx2: 北緯(N)or 南緯(S)。ASCII 値で'N'は北緯、'S'は南緯
			read_ascii(gps.GPSLatitudeRef, count, ARRAYSIZE(gps.GPSLatitudeRef));
			break;
		case GPSLatitude:							// 0x02, RATIONALx3: 緯度（数値）
			count = min(ARRAYSIZE(gps.GPSLatitude), count);
			for (i = 0; i < count; i++)
				gps.GPSLatitude[i] = (float)get_tagvalue(type);
			if ((int)gps.GPSLatitude[0] == 0)
				gps.GPSLatitude[0] = FLT_MAX;
			break;
		case GPSLongitudeRef:						// 0x03, ASCIIx2: 東経(E)or 西経(W)。ASCII 値で'E'は東経、'W'は西経
			read_ascii(gps.GPSLongitudeRef, count, ARRAYSIZE(gps.GPSLongitudeRef));
			break;
		case GPSLongitude:							// 0x04, RATIONALx3: 経度（数値）
			count = min(ARRAYSIZE(gps.GPSLongitude), count);
			for (i = 0; i < count; i++)
				gps.GPSLongitude[i] = (float)get_tagvalue(type);
			if ((int)gps.GPSLongitude[0] == 0)
				gps.GPSLongitude[0] = FLT_MAX;
			break;
		case GPSAltitudeRef:						// 0x05, BYTEx1: 高度の単位
			gps.GPSAlititudeRef = (BYTE)get_tagvalue(type);
			break;
		case GPSAltitude:							// 0x06, RATIONALx1: 高度（数値）m
			gps.GPSAlititude = (float)get_tagvalue(type);
			break;
		case GPSTimeStamp:							// 0x07, RATIONALx3: GPS 時間（原子時計の時間）UTC
			count = min(ARRAYSIZE(gps.GPSTimeStamp), count);
			for (i = 0; i < count; i++)
				gps.GPSTimeStamp[i] = (float)get_tagvalue(type);
			break;
		case GPSSatellites:							// 0x08, ASCIIxN: 測位に使った衛星信号
			read_ascii(gps.GPSSatellites, count, ARRAYSIZE(gps.GPSSatellites));
			break;
//		case GPSStatus:								// 0x09, ASCIIx2: GPS 受信機の状態
//		case GPSMeasureMode:						// 0x0A, ASCIIx2: GPS の測位方法
//		case GPSDOP:								// 0x0B, RATIONALx1: 測位の信頼性
//		case GPSSpeedRef:							// 0x0C, ASCIIx2: 速度の単位
//		case GPSSpeed:								// 0x0D, RATIONALx1: 速度（数値）
//		case GPSTrackRef:							// 0x0E, ASCIIx2: 進行方向の単位
//		case GPSTrack:								// 0x0F, RATIONALx1: 進行方向（数値）
//		case GPSImgDirectionRef:					// 0x10, ASCIIx2: 撮影した画像の方向の単位
//		case GPSImgDirection:						// 0x11, RATIONALx1: 撮影した画像の方向（数値）
		case GPSMapDatum:							// 0x12, ASCIIxN: 測位に用いた地図データ
			read_ascii(gps.GPSMapDatum, count, ARRAYSIZE(gps.GPSMapDatum));
			break;
//		case GPSDestLatitudeRef:					// 0x13, ASCIIx2: 目的地の北緯(N)or 南緯(S)
//		case GPSDestLatitude:						// 0x14, RATIONALx3: 目的地の緯度（数値）
//		case GPSDestLongitudeRef:					// 0x15, ASCIIx2: 目的地の東経(E)or 西経(W)
//		case GPSDestLongitude:						// 0x16, RATIONALx3: 目的地の経度（数値）
//		case GPSDestBearingRef:						// 0x17, ASCIIx2: 目的地の方角の単位
//		case GPSDestBearing:						// 0x18, RATIONALx1: 目的の方角（数値）
//		case GPSDestDistanceRef:					// 0x19, ASCIIx2: 目的地までの距離の単位
//		case GPSDestDistance:						// 0x1A, RATIONALx1: 目的地までの距離（数値）
//		case GPSProcessingMethod:					// 0x1B, UNDEFxN: Name of GPS processing method
//		case GPSAreaInformation:					// 0x1C, UNDEFxN: Name of GPS area
		case GPSDateStamp:							// 0x1D, ASCIIx11: GPS date
			read_ascii(gps.GPSDateStamp, count, ARRAYSIZE(gps.GPSDateStamp));
			break;
//		case GPSDefferential:						// 0x1E, SHORTx1: GPS differential correction
		default:
//@	tagdump(this, tag, type, count);
			break;
		}
		seek(save + 12, SEEK_SET);
	}
}

void rawmap::specify_cameramodel()
{
	LPCSTR model = m_exif.model;

	//----------------------------------
	// キヤノン
	//----------------------------------
	if (m_exif.MakerID == MAKER_CANON) {
		m_exif.ModelID = CANON_UNKNOWN;

		if (!StrCmpNA(model, "Canon", 5))
			model += 6;

		if (*model == 'E') {		// "EOS"
			model += 4;
			m_exif.ModelID = CANON_EOS_UNKNOWN;
			if		(!StrCmpA(model, "1D"))					m_exif.ModelID = CANON_EOS_1D;
			else if (!StrCmpA(model, "1DS"))				m_exif.ModelID = CANON_EOS_1DS;
			else if (!StrCmpA(model, "1D Mark II"))			m_exif.ModelID = CANON_EOS_1DMK2;
			else if (!StrCmpA(model, "1Ds Mark II"))		m_exif.ModelID = CANON_EOS_1DSMK2;
			else if (!StrCmpA(model, "1D Mark II N"))		m_exif.ModelID = CANON_EOS_1DMK2N;
			else if (!StrCmpA(model, "1D Mark III"))		m_exif.ModelID = CANON_EOS_1DMK3;
			else if (!StrCmpA(model, "1D Mark IV"))			m_exif.ModelID = CANON_EOS_1DMK4;
			else if (!StrCmpA(model, "1Ds Mark III"))		m_exif.ModelID = CANON_EOS_1DSMK3;
			else if (!StrCmpA(model, "1D X"))				m_exif.ModelID = CANON_EOS_1DX;
			else if (!StrCmpA(model, "7D"))					m_exif.ModelID = CANON_EOS_7D;
			else if (!StrCmpA(model, "7D Mark II"))			m_exif.ModelID = CANON_EOS_7DMK2;
			else if (!StrCmpA(model, "6D"))					m_exif.ModelID = CANON_EOS_6D;
			else if (!StrCmpA(model, "5D"))					m_exif.ModelID = CANON_EOS_5D;
			else if (!StrCmpA(model, "5D Mark III"))		m_exif.ModelID = CANON_EOS_5DMK3;
			else if (!StrCmpA(model, "5D Mark II"))			m_exif.ModelID = CANON_EOS_5DMK2;
//@			else if (!StrCmpA(model, "3D"))					m_exif.ModelID = CANON_EOS_3D;
			else if (!StrCmpA(model, "20D"))				m_exif.ModelID = CANON_EOS_20D;
			else if (!StrCmpA(model, "30D"))				m_exif.ModelID = CANON_EOS_30D;
			else if (!StrCmpA(model, "40D"))				m_exif.ModelID = CANON_EOS_40D;
			else if (!StrCmpA(model, "50D"))				m_exif.ModelID = CANON_EOS_50D;
			else if (!StrCmpA(model, "60D"))				m_exif.ModelID = CANON_EOS_60D;
			else if (!StrCmpA(model, "70D"))				m_exif.ModelID = CANON_EOS_70D;
			else if (!StrCmpA(model, "80D"))				m_exif.ModelID = CANON_EOS_80D;
			else if (!StrCmpA(model, "400D"))				m_exif.ModelID = CANON_EOS_KISSX;
			else if (!StrCmpA(model, "450D"))				m_exif.ModelID = CANON_EOS_KISSX2;
			else if (!StrCmpA(model, "500D"))				m_exif.ModelID = CANON_EOS_KISSX3;
			else if (!StrCmpA(model, "550D"))				m_exif.ModelID = CANON_EOS_KISSX4;
			else if (!StrCmpA(model, "600D"))				m_exif.ModelID = CANON_EOS_KISSX5;
			else if (!StrCmpA(model, "650D"))				m_exif.ModelID = CANON_EOS_KISSX6I;
			else if (!StrCmpA(model, "1000D"))				m_exif.ModelID = CANON_EOS_KISSF;
			else if (!StrCmpA(model, "1100D"))				m_exif.ModelID = CANON_EOS_KISSX50;
			else if (!StrCmpA(model, "M"))					m_exif.ModelID = CANON_EOS_M;
			else if (!StrCmpA(model, "M2"))					m_exif.ModelID = CANON_EOS_M2;
			else if (!StrCmpA(model, "M3"))					m_exif.ModelID = CANON_EOS_M3;
			else if (!StrCmpNA(model, "Kiss", 4)) {
				model += 5;
				if (!StrCmpNA(model, "Digital ", 8))
					model += 8;
				// モデル名が "Caonon EOS Kiss Digital X2" または "Caonon EOS Kiss X2" の2種類あるゾ
				if (*model == 'X') {
					model++;
					m_exif.ModelID = CANON_EOS_KISSX;
					if      (!StrCmpA(model, "3"))			m_exif.ModelID = CANON_EOS_KISSX3;
					else if (!StrCmpA(model, "2"))			m_exif.ModelID = CANON_EOS_KISSX2;
					else if (!StrCmpA(model, "4"))			m_exif.ModelID = CANON_EOS_KISSX4;
					else if (!StrCmpA(model, "50"))			m_exif.ModelID = CANON_EOS_KISSX50;
					else if (!StrCmpA(model, "5"))			m_exif.ModelID = CANON_EOS_KISSX5;
					else if (!StrCmpA(model, "6i"))			m_exif.ModelID = CANON_EOS_KISSX6I;
					else if (!StrCmpA(model, "6"))			m_exif.ModelID = CANON_EOS_KISSX6;
					else if (!StrCmpA(model, "7i"))			m_exif.ModelID = CANON_EOS_KISSX7I;
					else if (!StrCmpA(model, "7"))			m_exif.ModelID = CANON_EOS_KISSX7;
				}
				else if (!StrCmpA(model, "F"))				m_exif.ModelID = CANON_EOS_KISSF;
				else if (!StrCmpA(model, "N"))				m_exif.ModelID = CANON_EOS_KISSN;
				else										m_exif.ModelID = CANON_EOS_KISS;
			}
			else if (!StrCmpA(model, "10D"))				m_exif.ModelID = CANON_EOS_10D;
		}
		else if (*model == 'P') {	// "PowerShot"
			model += 10;
			m_exif.ModelID = CANON_POWERSHOT_UNKNOWN;
			if (*model == 'S') {
				model++;
				if (*model == 'D') {
					// PowerShot SDxxx
					model++;
					if		(!StrCmpA(model, "30"))			m_exif.ModelID = CANON_IXY_L3;
					else if	(!StrCmpA(model, "400"))		m_exif.ModelID = CANON_IXY_55;
					else if	(!StrCmpA(model, "450"))		m_exif.ModelID = CANON_IXY_60;
					else if	(!StrCmpA(model, "500"))		m_exif.ModelID = CANON_IXY_600;
					else if	(!StrCmpA(model, "550"))		m_exif.ModelID = CANON_IXY_700;
					else if	(!StrCmpA(model, "600"))		m_exif.ModelID = CANON_IXY_70;
					else if	(!StrCmpA(model, "630"))		m_exif.ModelID = CANON_IXY_80;
					else if	(!StrCmpA(model, "750"))		m_exif.ModelID = CANON_IXY_90;
					else if	(!StrCmpA(model, "1000"))		m_exif.ModelID = CANON_IXY_10;
				}
				else if (*model == 'X') {
					// PowerShot SXxxx
					model++;
					if		(!StrCmpA(model, "100 IS"))		m_exif.ModelID = CANON_POWERSHOT_SX100IS;
				}
				else {
					// PowerShot Sxxx
					if		(!StrCmpA(model, "80"))			m_exif.ModelID = CANON_POWERSHOT_S80;
					else if	(!StrCmpA(model, "70"))			m_exif.ModelID = CANON_POWERSHOT_S70;
					else if	(!StrCmpA(model, "60"))			m_exif.ModelID = CANON_POWERSHOT_S60;
					else if	(!StrCmpA(model, "50"))			m_exif.ModelID = CANON_POWERSHOT_S50;
					else if	(!StrCmpA(model, "40"))			m_exif.ModelID = CANON_POWERSHOT_S40;
					else if	(!StrCmpA(model, "30"))			m_exif.ModelID = CANON_POWERSHOT_S30;
					else if	(!StrCmpA(model, "20"))			m_exif.ModelID = CANON_POWERSHOT_S20;
					else if	(!StrCmpA(model, "3 IS"))		m_exif.ModelID = CANON_POWERSHOT_S3IS;
					else if	(!StrCmpA(model, "2 IS"))		m_exif.ModelID = CANON_POWERSHOT_S2IS;
					else if	(!StrCmpA(model, "1 IS"))		m_exif.ModelID = CANON_POWERSHOT_S1IS;
				}
			}
			else if (*model == 'A') {
				// PowerShot Axxx
				model++;
				if		(!StrCmpA(model, "430"))			m_exif.ModelID = CANON_POWERSHOT_A430;
				else if	(!StrCmpA(model, "530"))			m_exif.ModelID = CANON_POWERSHOT_A530;
				else if	(!StrCmpA(model, "540"))			m_exif.ModelID = CANON_POWERSHOT_A540;
				else if	(!StrCmpA(model, "560"))			m_exif.ModelID = CANON_POWERSHOT_A560;
				else if	(!StrCmpA(model, "610"))			m_exif.ModelID = CANON_POWERSHOT_A610;
				else if	(!StrCmpA(model, "620"))			m_exif.ModelID = CANON_POWERSHOT_A620;
				else if	(!StrCmpA(model, "630"))			m_exif.ModelID = CANON_POWERSHOT_A630;
				else if	(!StrCmpA(model, "640"))			m_exif.ModelID = CANON_POWERSHOT_A640;
				else if	(!StrCmpA(model, "640 IS"))			m_exif.ModelID = CANON_POWERSHOT_A650IS;
				else if	(!StrCmpA(model, "700"))			m_exif.ModelID = CANON_POWERSHOT_A700;
				else if	(!StrCmpA(model, "710 IS"))			m_exif.ModelID = CANON_POWERSHOT_A710IS;
				else if	(!StrCmpA(model, "720 IS"))			m_exif.ModelID = CANON_POWERSHOT_A720IS;
			}
			else if (*model == 'G') {
				// PowerShot Gxxx
				model++;
				if		(!StrCmpA(model, "9"))				m_exif.ModelID = CANON_POWERSHOT_G9;
				else if	(!StrCmpA(model, "7"))				m_exif.ModelID = CANON_POWERSHOT_G7;
				else if	(!StrCmpA(model, "6"))				m_exif.ModelID = CANON_POWERSHOT_G6;
				else if	(!StrCmpA(model, "5"))				m_exif.ModelID = CANON_POWERSHOT_G5;
				else if	(!StrCmpA(model, "3"))				m_exif.ModelID = CANON_POWERSHOT_G3;
				else if	(!StrCmpA(model, "2"))				m_exif.ModelID = CANON_POWERSHOT_G2;
			}
			else if (*model == 'T') {
				// PowerShot Txxx
				model++;
				if		(!StrCmpA(model, "X1"))				m_exif.ModelID = CANON_POWERSHOT_TX1;
			}
			else if	(!StrCmpA(model, "Pro1"))				m_exif.ModelID = CANON_POWERSHOT_PRO1;
		}
		else if (*model == 'I') {	// "IXY DIGITAL"
			model += 12;
			m_exif.ModelID = CANON_IXY_UNKNOWN;
			if		(!StrCmpA(model, "450"))				m_exif.ModelID = CANON_IXY_450;
			else if	(!StrCmpA(model, "500"))				m_exif.ModelID = CANON_IXY_500;
			else if	(!StrCmpA(model, "55"))					m_exif.ModelID = CANON_IXY_55;
			else if	(!StrCmpA(model, "600"))				m_exif.ModelID = CANON_IXY_600;
			else if	(!StrCmpA(model, "60"))					m_exif.ModelID = CANON_IXY_60;
			else if	(!StrCmpA(model, "700"))				m_exif.ModelID = CANON_IXY_700;
			else if	(!StrCmpA(model, "1000"))				m_exif.ModelID = CANON_IXY_1000;
			else if	(!StrCmpA(model, "2000 IS"))			m_exif.ModelID = CANON_IXY_2000IS;
			else if	(!StrCmpA(model, "70"))					m_exif.ModelID = CANON_IXY_70;
			else if	(!StrCmpA(model, "800 IS"))				m_exif.ModelID = CANON_IXY_800IS;
			else if	(!StrCmpA(model, "900 IS"))				m_exif.ModelID = CANON_IXY_900IS;
			else if	(!StrCmpA(model, "910 IS"))				m_exif.ModelID = CANON_IXY_910IS;
			else if	(!StrCmpA(model, "80"))					m_exif.ModelID = CANON_IXY_80;
			else if	(!StrCmpA(model, "L2"))					m_exif.ModelID = CANON_IXY_L2;
			else if	(!StrCmpA(model, "L3"))					m_exif.ModelID = CANON_IXY_L3;
			else if	(!StrCmpA(model, "L4"))					m_exif.ModelID = CANON_IXY_L4;
			else if	(!StrCmpA(model, "10"))					m_exif.ModelID = CANON_IXY_10;
			else if	(!StrCmpA(model, "90"))					m_exif.ModelID = CANON_IXY_90;
		}
		else if (*model == 'D') {	// "DIGITAL IXUS"
			model += 13;
			if		(!StrCmpA(model, "IIs"))				m_exif.ModelID = CANON_IXY_30a;
			else if	(!StrCmpA(model, "850 IS"))				m_exif.ModelID = CANON_IXY_900IS;
			else if	(!StrCmpA(model, "900 Ti"))				m_exif.ModelID = CANON_IXY_1000;
			else if	(!StrCmpA(model, "i7 zoom"))			m_exif.ModelID = CANON_IXY_L4;
		}
	}
	//----------------------------------
	// ニコン
	//----------------------------------
	else if (m_exif.MakerID == MAKER_NIKON) {
		m_exif.ModelID = NIKON_UNKNOWN;
		if (*model == 'N') {
			if (*(model + 6) == 'D') {
				model += 7;
				m_exif.ModelID = NIKON_D_UNKNOWN;
				if		(!StrCmpA(model, "f"))				m_exif.ModelID = NIKON_DF;
				else if (!StrCmpA(model, "1"))				m_exif.ModelID = NIKON_D1;
				else if (!StrCmpA(model, "1X"))				m_exif.ModelID = NIKON_D1X;
				else if (!StrCmpA(model, "1H"))				m_exif.ModelID = NIKON_D1H;
				else if (!StrCmpA(model, "2X"))				m_exif.ModelID = NIKON_D2X;
				else if (!StrCmpA(model, "2Xs"))			m_exif.ModelID = NIKON_D2XS;
				else if (!StrCmpA(model, "2Hs"))			m_exif.ModelID = NIKON_D2HS;
				else if (!StrCmpA(model, "2H"))				m_exif.ModelID = NIKON_D2H;
				else if	(!StrCmpA(model, "3"))				m_exif.ModelID = NIKON_D3;
				else if	(!StrCmpA(model, "3X"))				m_exif.ModelID = NIKON_D3X;
				else if	(!StrCmpA(model, "3S"))				m_exif.ModelID = NIKON_D3S;
				else if	(!StrCmpA(model, "4"))				m_exif.ModelID = NIKON_D4;
				else if	(!StrCmpA(model, "4S"))				m_exif.ModelID = NIKON_D4S;
				else if (!StrCmpA(model, "40"))				m_exif.ModelID = NIKON_D40;
				else if (!StrCmpA(model, "40X"))			m_exif.ModelID = NIKON_D40X;
				else if (!StrCmpA(model, "50"))				m_exif.ModelID = NIKON_D50;
				else if (!StrCmpA(model, "60"))				m_exif.ModelID = NIKON_D60;
				else if (!StrCmpA(model, "70"))				m_exif.ModelID = NIKON_D70;
				else if (!StrCmpA(model, "70s"))			m_exif.ModelID = NIKON_D70S;
				else if (!StrCmpA(model, "90"))				m_exif.ModelID = NIKON_D90;
				else if (!StrCmpA(model, "80"))				m_exif.ModelID = NIKON_D80;
				else if (!StrCmpA(model, "100"))			m_exif.ModelID = NIKON_D100;
				else if	(!StrCmpA(model, "200"))			m_exif.ModelID = NIKON_D200;
				else if (!StrCmpA(model, "300"))			m_exif.ModelID = NIKON_D300;
				else if (!StrCmpA(model, "300S"))			m_exif.ModelID = NIKON_D300S;
				else if (!StrCmpA(model, "310"))			m_exif.ModelID = NIKON_D310;
				else if (!StrCmpA(model, "340"))			m_exif.ModelID = NIKON_D340;
				else if (!StrCmpA(model, "350"))			m_exif.ModelID = NIKON_D350;
				else if (!StrCmpA(model, "360"))			m_exif.ModelID = NIKON_D360;
				else if (!StrCmpA(model, "600"))			m_exif.ModelID = NIKON_D600;
				else if (!StrCmpA(model, "610"))			m_exif.ModelID = NIKON_D610;
				else if	(!StrCmpA(model, "700"))			m_exif.ModelID = NIKON_D700;
				else if	(!StrCmpA(model, "710"))			m_exif.ModelID = NIKON_D710;
				else if	(!StrCmpA(model, "740"))			m_exif.ModelID = NIKON_D740;
				else if	(!StrCmpA(model, "750"))			m_exif.ModelID = NIKON_D750;
				else if	(!StrCmpA(model, "800"))			m_exif.ModelID = NIKON_D800;
				else if	(!StrCmpA(model, "800e"))			m_exif.ModelID = NIKON_D800E;
				else if	(!StrCmpA(model, "810"))			m_exif.ModelID = NIKON_D810;
				else if	(!StrCmpA(model, "810a"))			m_exif.ModelID = NIKON_D810A;
				else if (!StrCmpA(model, "3000"))			m_exif.ModelID = NIKON_D3000;
				else if (!StrCmpA(model, "3100"))			m_exif.ModelID = NIKON_D3100;
				else if (!StrCmpA(model, "3200"))			m_exif.ModelID = NIKON_D3200;
				else if (!StrCmpA(model, "3300"))			m_exif.ModelID = NIKON_D3300;
				else if (!StrCmpA(model, "5000"))			m_exif.ModelID = NIKON_D5000;
				else if (!StrCmpA(model, "5100"))			m_exif.ModelID = NIKON_D5100;
				else if (!StrCmpA(model, "5200"))			m_exif.ModelID = NIKON_D5200;
				else if (!StrCmpA(model, "5300"))			m_exif.ModelID = NIKON_D5300;
				else if (!StrCmpA(model, "5400"))			m_exif.ModelID = NIKON_D5400;
				else if (!StrCmpA(model, "5500"))			m_exif.ModelID = NIKON_D5500;
				else if	(!StrCmpA(model, "7000"))			m_exif.ModelID = NIKON_D7000;
				else if	(!StrCmpA(model, "7100"))			m_exif.ModelID = NIKON_D7100;
				else if	(!StrCmpA(model, "7200"))			m_exif.ModelID = NIKON_D7200;
			}
			else if (*(model + 6) == '1') {
				model += 8;
				m_exif.ModelID = NIKON_1_UNKNOWN;
				if		(!StrCmpA(model, "J1"))				m_exif.ModelID = NIKON_1_J1;
				else if	(!StrCmpA(model, "J2"))				m_exif.ModelID = NIKON_1_J2;
				else if	(!StrCmpA(model, "J3"))				m_exif.ModelID = NIKON_1_J3;
				else if	(!StrCmpA(model, "J4"))				m_exif.ModelID = NIKON_1_J4;
				else if	(!StrCmpA(model, "J5"))				m_exif.ModelID = NIKON_1_J5;
				else if	(!StrCmpA(model, "S1"))				m_exif.ModelID = NIKON_1_S1;
				else if	(!StrCmpA(model, "V1"))				m_exif.ModelID = NIKON_1_V1;
				else if	(!StrCmpA(model, "V2"))				m_exif.ModelID = NIKON_1_V2;
				else if	(!StrCmpA(model, "V3"))				m_exif.ModelID = NIKON_1_V3;
				else if	(!StrCmpA(model, "AW1"))			m_exif.ModelID = NIKON_1_AW1;
			}
		}
		else if (*model == 'C') {
			model += 8;
			m_exif.ModelID = NIKON_COOLPIX;
			if		(!StrCmpA(model, "S3"))					m_exif.ModelID = NIKON_COOLPIX_S3;
			else if (!StrCmpA(model, "S5"))					m_exif.ModelID = NIKON_COOLPIX_S5;
			else if (!StrCmpA(model, "S6"))					m_exif.ModelID = NIKON_COOLPIX_S6;
			else if (!StrCmpA(model, "L3"))					m_exif.ModelID = NIKON_COOLPIX_L3;
			else if (!StrCmpA(model, "P3"))					m_exif.ModelID = NIKON_COOLPIX_P3;
			else if (!StrCmpA(model, "P4"))					m_exif.ModelID = NIKON_COOLPIX_P4;
		}
		else if (*model == 'E') {
			model += 1;
			m_exif.ModelID = NIKON_E_UNKNOWN;
			if		(!StrCmpA(model, "8800"))				m_exif.ModelID = NIKON_E8800;
			else if (!StrCmpA(model, "8700"))				m_exif.ModelID = NIKON_E8700;
			else if (!StrCmpA(model, "8400"))				m_exif.ModelID = NIKON_E8400;
			else if (!StrCmpA(model, "5700"))				m_exif.ModelID = NIKON_E5700;
			else if (!StrCmpA(model, "5400"))				m_exif.ModelID = NIKON_E5400;
		}


		else if (StrStrA(model, "NIKON D1X"))				m_exif.ModelID = NIKON_D1X;
	}
	//----------------------------------
	// フジフィルム
	//----------------------------------
	else if (m_exif.MakerID == MAKER_FUJI) {
		m_exif.ModelID = FUJIFILM_UNKNOWN;
		if (!StrCmpNIA(model, "FinePix", 7)) {
			model += 8;
			if (*model == 'S') {
				m_exif.ModelID = FUJIFILM_S;
				model++;
				if      (!memcmp(model, "2Pro", 4))			m_exif.ModelID = FUJIFILM_S2PRO;
				else if (!memcmp(model, "3Pro", 4))			m_exif.ModelID = FUJIFILM_S3PRO;
				else if (!memcmp(model, "5Pro", 4))			m_exif.ModelID = FUJIFILM_S5PRO;
				else if (!memcmp(model, "9100", 4))			m_exif.ModelID = FUJIFILM_S9100;
				else if (!memcmp(model, "100FS", 5))		m_exif.ModelID = FUJIFILM_S100FS;
			}
			else if (*model == 'F') {
				m_exif.ModelID = FUJIFILM_F;
				model++;
				if      (!memcmp(model, "200EXR", 6))		m_exif.ModelID = FUJIFILM_F200EXR;
			}
			else if (*model == 'A') {
				m_exif.ModelID = FUJIFILM_A;
				model++;
			}
			else if (*model == 'E') {
				m_exif.ModelID = FUJIFILM_E;
				model++;
			}
			else if (*model == 'Z') {
				m_exif.ModelID = FUJIFILM_Z;
				model++;
			}
		}
	}
	//----------------------------------
	// ミノルタ
	//----------------------------------
	else if (m_exif.MakerID == MAKER_MINOLTA) {
		m_exif.ModelID = MINOLTA_UNKNOWN;
		if (*model == 'D' && *(model + 1) == 'i') {
			// DiMAGE
			model += 7;
			if		(!memcmp(model, "A1",   2))				m_exif.ModelID = MINOLTA_DA1;
			else if (!memcmp(model, "A200", 4))				m_exif.ModelID = MINOLTA_DA200;
			else if (!memcmp(model, "A2",   2))				m_exif.ModelID = MINOLTA_DA2;
			else if (!memcmp(model, "Z1",   2))				m_exif.ModelID = MINOLTA_DZ1;
			else if (!memcmp(model, "Z2",   2))				m_exif.ModelID = MINOLTA_DZ2;
			else if (!memcmp(model, "Z3",   2))				m_exif.ModelID = MINOLTA_DZ3;
		}
		else {
			m_exif.ModelID = MINOLTA_ALPHA_UNKNOWN;
			if		(!memcmp(model, "ALPHA-7", 7) ||
					 !memcmp(model, "DYNAX 7", 7) ||
					 !memcmp(model, "MAXXUM 7D", 8))		m_exif.ModelID = MINOLTA_ALPHA_7D;
			else if (!memcmp(model, "ALPHA SW", 8) ||
					 !memcmp(model, "DYNAX 5", 7) ||
					 !memcmp(model, "MAXXUM 5D", 8))		m_exif.ModelID = MINOLTA_ALPHA_SWEET;
		}
	}
	//----------------------------------
	// オリンパス
	//----------------------------------
	else if (m_exif.MakerID == MAKER_OLYMPUS) {
		m_exif.ModelID = OLYMPUS_UNKNOWN;
		if (*model == 'C') {
			m_exif.ModelID = OLYMPUS_C;
		}
		else if (*model == 'u') {
			m_exif.ModelID = OLYMPUS_MYU;
		}
		else if (*model == 'E' && *(model + 1) == '-') {
			m_exif.ModelID = OLYMPUS_E;
			model += 2;
			if		(!memcmp(model, "M5 ", 3))				m_exif.ModelID = OLYMPUS_EM5;
			else if	(!memcmp(model, "M1 ", 3))				m_exif.ModelID = OLYMPUS_EM1;
			else if	(!memcmp(model, "P5 ", 3))				m_exif.ModelID = OLYMPUS_EP5;
			else if	(!memcmp(model, "P3 ", 3))				m_exif.ModelID = OLYMPUS_EP3;
			else if	(!memcmp(model, "P1 ", 3))				m_exif.ModelID = OLYMPUS_EP1;
			else if	(!memcmp(model, "PL6 ", 4))				m_exif.ModelID = OLYMPUS_EPL6;
			else if	(!memcmp(model, "PL5 ", 4))				m_exif.ModelID = OLYMPUS_EPL5;
			else if	(!memcmp(model, "PM2 ", 4))				m_exif.ModelID = OLYMPUS_EPM2;
			else if	(!memcmp(model, "300", 3))				m_exif.ModelID = OLYMPUS_E300;
			else if	(!memcmp(model, "330", 3))				m_exif.ModelID = OLYMPUS_E330;
			else if	(!memcmp(model, "400", 3))				m_exif.ModelID = OLYMPUS_E400;
			else if	(!memcmp(model, "410", 3))				m_exif.ModelID = OLYMPUS_E410;
			else if	(!memcmp(model, "420", 3))				m_exif.ModelID = OLYMPUS_E420;
			else if	(!memcmp(model, "450", 3))				m_exif.ModelID = OLYMPUS_E450;
			else if	(!memcmp(model, "500", 3))				m_exif.ModelID = OLYMPUS_E500;
			else if	(!memcmp(model, "510", 3))				m_exif.ModelID = OLYMPUS_E510;
			else if	(!memcmp(model, "620", 3))				m_exif.ModelID = OLYMPUS_E620;
			else if	(!memcmp(model, "20",  2))				m_exif.ModelID = OLYMPUS_E20;
			else if	(!memcmp(model, "1 ",  2))				m_exif.ModelID = OLYMPUS_E1;
			else if	(!memcmp(model, "3 ",  2))				m_exif.ModelID = OLYMPUS_E3;
		}
	}
	//----------------------------------
	// ペンタックス
	//----------------------------------
	else if (m_exif.MakerID == MAKER_PENTAX) {
		model += 7;
		m_exif.ModelID = PENTAX_UNKNOWN;
		if (*model == 'O')
			m_exif.ModelID = PENTAX_OPTIO;
		else if (*model == '*') {
			model += 5;
			m_exif.ModelID = PENTAX_IST;
			if		(!memcmp(model, "DL2", 3))				m_exif.ModelID = PENTAX_ISTDL2;
			else if (!memcmp(model, "DL",  2))				m_exif.ModelID = PENTAX_ISTDL;
			else if (!memcmp(model, "DS2", 3))				m_exif.ModelID = PENTAX_ISTDS2;
			else if (!memcmp(model, "DS",  2))				m_exif.ModelID = PENTAX_ISTDS;
			else if (!memcmp(model, "D",   1))				m_exif.ModelID = PENTAX_ISTD;
		}
		else if (*model == 'K') {
			model += 1;
			m_exif.ModelID = PENTAX_K;
			if		(!memcmp(model, "100D", 4))				m_exif.ModelID = PENTAX_K100D;
			else if	(!memcmp(model, "110D", 4))				m_exif.ModelID = PENTAX_K110D;
			else if	(!memcmp(model, "200D", 4))				m_exif.ModelID = PENTAX_K200D;
			else if	(!memcmp(model, "10D", 3))				m_exif.ModelID = PENTAX_K10D;
			else if	(!memcmp(model, "20D", 3))				m_exif.ModelID = PENTAX_K20D;
			else if	(!memcmp(model, "-m", 2))				m_exif.ModelID = PENTAX_KM;
		}
	}
	//----------------------------------
	// ソニー
	//----------------------------------
	else if (m_exif.MakerID == MAKER_SONY) {
		m_exif.ModelID = SONY_UNKNOWN;
		if (*model == 'C') {
			m_exif.ModelID = SONY_CYBERSHOT;
		}
		else if (!memcmp(model, "DSC", 3)) {
			model += 4;
			m_exif.ModelID = SONY_DSC;
			if		(*model == 'H')							m_exif.ModelID = SONY_DSC_H;
			else if	(*model == 'L')							m_exif.ModelID = SONY_DSC_L;
			else if	(*model == 'M')							m_exif.ModelID = SONY_DSC_M;
			else if	(*model == 'N') {
				model += 1;
				m_exif.ModelID = SONY_DSC_N;
				if (*model == '1')							m_exif.ModelID = SONY_DSC_N1;
			}
			else if	(*model == 'P')							m_exif.ModelID = SONY_DSC_P;
			else if	(*model == 'S')							m_exif.ModelID = SONY_DSC_S;
			else if	(*model == 'T') {
				model += 1;
				m_exif.ModelID = SONY_DSC_T;
				if (*model == '9')							m_exif.ModelID = SONY_DSC_T9;
			}
			else if	(*model == 'T')							m_exif.ModelID = SONY_DSC_T;
			else if	(*model == 'U')							m_exif.ModelID = SONY_DSC_U;
			else if	(*model == 'V')							m_exif.ModelID = SONY_DSC_V;
			else if	(*model == 'W')							m_exif.ModelID = SONY_DSC_W;
			else if	(*model == 'R') {
				model += 1;
				m_exif.ModelID = SONY_DSC_R;
				if (*model == '1')							m_exif.ModelID = SONY_DSC_R1;
			}
			else if	(!memcmp(model, "KAIKAK", 6))			m_exif.ModelID = SONY_DSC_R1;
		}
		else if (!memcmp(model, "DSLR", 4)) {
			model += 5;
			if (*model == 'A') {
				model++;
				m_exif.ModelID = SONY_DSLR_ALPHA;
				if		(!StrCmpA(model, "100"))			m_exif.ModelID = SONY_DSLR_A100;
				else if	(!StrCmpA(model, "200"))			m_exif.ModelID = SONY_DSLR_A200;
				else if	(!StrCmpA(model, "300"))			m_exif.ModelID = SONY_DSLR_A300;
				else if	(!StrCmpA(model, "350"))			m_exif.ModelID = SONY_DSLR_A350;
				else if	(!StrCmpA(model, "700"))			m_exif.ModelID = SONY_DSLR_A700;
				else if	(!StrCmpA(model, "900"))			m_exif.ModelID = SONY_DSLR_A900;
			}
		}
		else if (!memcmp(model, "SLT", 3)) {
			model += 4;
			if (*model == 'A') {
				model++;
				m_exif.ModelID = SONY_SLT_UNKNOWN;
				if		(!StrCmpA(model, "A55V"))			m_exif.ModelID = SONY_SLT_A55;
				else if	(!StrCmpA(model, "A57"))			m_exif.ModelID = SONY_SLT_A57;
				else if	(!StrCmpA(model, "A58"))			m_exif.ModelID = SONY_SLT_A58;
				else if	(!StrCmpA(model, "A65V"))			m_exif.ModelID = SONY_SLT_A65;
				else if	(!StrCmpA(model, "A77V"))			m_exif.ModelID = SONY_SLT_A77;
			}
		}
		else if (!memcmp(model, "NEX", 3)) {
			model += 4;
			m_exif.ModelID = SONY_DSC_NEX;
			if		(!StrCmpA(model, "3"))					m_exif.ModelID = SONY_DSC_NEX3;
			else if	(!StrCmpA(model, "C3"))					m_exif.ModelID = SONY_DSC_NEXC3;
			else if	(!StrCmpA(model, "5"))					m_exif.ModelID = SONY_DSC_NEX5;
			else if	(!StrCmpA(model, "5N"))					m_exif.ModelID = SONY_DSC_NEX5N;
			else if	(!StrCmpA(model, "7"))					m_exif.ModelID = SONY_DSC_NEX7;
		}
		else if (!memcmp(model, "ILCA", 4)) {
			model += 5;
			m_exif.ModelID = SONY_ILCA_UNKNOWN;
			if		(!StrCmpA(model, "77M2"))				m_exif.ModelID = SONY_ILCA_77M2;
		}
		else if (!memcmp(model, "ILCE", 4)) {
			model += 5;
			m_exif.ModelID = SONY_ILCE_UNKNOWN;
			if		(!StrCmpA(model, "7R"))					m_exif.ModelID = SONY_ILCE_7R;
			else if	(!StrCmpA(model, "7S"))					m_exif.ModelID = SONY_ILCE_7S;
			else if	(!StrCmpA(model, "7"))					m_exif.ModelID = SONY_ILCE_7;
			else if	(!StrCmpA(model, "3000"))				m_exif.ModelID = SONY_ILCE_3000;
			else if	(!StrCmpA(model, "5000"))				m_exif.ModelID = SONY_ILCE_5000;
			else if	(!StrCmpA(model, "6000"))				m_exif.ModelID = SONY_ILCE_6000;
		}
	}
	//----------------------------------
	// カシオ
	//----------------------------------
	else if (m_exif.MakerID == MAKER_CASIO) {
		m_exif.ModelID = CASIO_UNKNOWN;
		if (*model == 'E') {
			model += 3;
			if (*model == 'Z') {
				model += 1;
				m_exif.ModelID = CASIO_EXZOOM;
				if		(!memcmp(model, "1200", 4))			m_exif.ModelID = CASIO_EXZ1200;
				else if	(!memcmp(model, "1080", 4))			m_exif.ModelID = CASIO_EXZ1080;
				else if	(!memcmp(model, "1050", 4))			m_exif.ModelID = CASIO_EXZ1050;
				else if	(!memcmp(model, "1000", 4))			m_exif.ModelID = CASIO_EXZ1000;
				else if	(!memcmp(model, "850", 3))			m_exif.ModelID = CASIO_EXZ850;
				else if	(!memcmp(model, "750", 3))			m_exif.ModelID = CASIO_EXZ750;
				else if	(!memcmp(model, "700", 3))			m_exif.ModelID = CASIO_EXZ700;
				else if	(!memcmp(model, "600", 3))			m_exif.ModelID = CASIO_EXZ600;
				else if	(!memcmp(model, "500", 3))			m_exif.ModelID = CASIO_EXZ500;
				else if	(!memcmp(model, "110", 3))			m_exif.ModelID = CASIO_EXZ110;
				else if	(!memcmp(model, "77 ", 3))			m_exif.ModelID = CASIO_EXZ77;
				else if	(!memcmp(model, "60 ", 3))			m_exif.ModelID = CASIO_EXZ60;
				else if	(!memcmp(model, "57 ", 3))			m_exif.ModelID = CASIO_EXZ57;
				else if	(!memcmp(model, "55 ", 3))			m_exif.ModelID = CASIO_EXZ55;
				else if	(!memcmp(model, "50 ", 3))			m_exif.ModelID = CASIO_EXZ50;
				else if	(!memcmp(model, "40 ", 3))			m_exif.ModelID = CASIO_EXZ50;
				else if	(!memcmp(model, "30 ", 3))			m_exif.ModelID = CASIO_EXZ30;
				else if	(!memcmp(model, "4  ", 3))			m_exif.ModelID = CASIO_EXZ4;
				else if	(!memcmp(model, "3  ", 3))			m_exif.ModelID = CASIO_EXZ3;
			}
			else if (*model == 'P') {
				model += 1;
				m_exif.ModelID = CASIO_EXPRO;
				if		(!memcmp(model, "700", 3))			m_exif.ModelID = CASIO_EXP700;
				else if	(!memcmp(model, "600", 3))			m_exif.ModelID = CASIO_EXP600;
				else if	(!memcmp(model, "505", 3))			m_exif.ModelID = CASIO_EXP505;
			}
			else if (*model == 'F') {
				model += 1;
				m_exif.ModelID = CASIO_EXPRO;	// FxxシリーズもPro
				if		(!memcmp(model, "1", 1))			m_exif.ModelID = CASIO_EXF1;
			}
			else if (*model == 'S') {
				model += 1;
				m_exif.ModelID = CASIO_EXCARD;
				if		(!memcmp(model, "880", 3))			m_exif.ModelID = CASIO_EXS880;
				else if	(!memcmp(model, "770", 3))			m_exif.ModelID = CASIO_EXS770;
				else if	(!memcmp(model, "600", 3))			m_exif.ModelID = CASIO_EXS600;
				else if	(!memcmp(model, "500", 3))			m_exif.ModelID = CASIO_EXS500;
				else if	(!memcmp(model, "100", 3))			m_exif.ModelID = CASIO_EXS100;
				else if	(!memcmp(model, "20 ", 3))			m_exif.ModelID = CASIO_EXS20;
				else if	(!memcmp(model, "3  ", 3))			m_exif.ModelID = CASIO_EXS3;
				else if	(!memcmp(model, "2  ", 3))			m_exif.ModelID = CASIO_EXS2;
			}
			else if (*model == 'M') {
				model += 1;
				m_exif.ModelID = CASIO_EXCARD;
				if		(!memcmp(model, "2  ", 3))			m_exif.ModelID = CASIO_EXM2;
			}
			else if (*model == 'V') {
				model += 1;
				m_exif.ModelID = CASIO_EXHiZOOM;
				if		(!memcmp(model, "8  ", 3))			m_exif.ModelID = CASIO_EXV8;
				else if	(!memcmp(model, "7  ", 3))			m_exif.ModelID = CASIO_EXV7;
			}
		}
	}
	//----------------------------------
	// パナソニック
	//----------------------------------
	else if (m_exif.MakerID == MAKER_PANASONIC) {
		m_exif.ModelID = PANA_UNKNOWN;
		if (!memcmp(model, "DMC-", 4)) {
			model += 4;
			if (*model == 'L') {
				model++;
				if (*model == 'C') {
					// LC シリーズ
					m_exif.ModelID = PANA_LC;
					model++;
					if      (!StrCmpA(model, "5"))				m_exif.ModelID = PANA_LC5;
					else if (!StrCmpA(model, "20"))				m_exif.ModelID = PANA_LC20;
					else if (!StrCmpA(model, "33"))				m_exif.ModelID = PANA_LC33;
					else if (!StrCmpA(model, "43"))				m_exif.ModelID = PANA_LC43;
					else if (!StrCmpA(model, "70"))				m_exif.ModelID = PANA_LC70;
				}
				else if (*model == 'S') {
					// LS シリーズ
					m_exif.ModelID = PANA_LS;
					model++;
					if      (!StrCmpA(model, "2"))				m_exif.ModelID = PANA_LS2;
					else if (!StrCmpA(model, "75"))				m_exif.ModelID = PANA_LS75;
				}
				else if (*model == 'Z') {
					// LZ シリーズ
					m_exif.ModelID = PANA_LZ;
					model++;
					if      (!StrCmpA(model, "2"))				m_exif.ModelID = PANA_LZ2;
					else if (!StrCmpA(model, "5"))				m_exif.ModelID = PANA_LZ5;
					else if (!StrCmpA(model, "7"))				m_exif.ModelID = PANA_LZ7;
				}
				else if (*model == 'X') {
					// LX シリーズ
					m_exif.ModelID = PANA_LX;
					model++;
					if      (!StrCmpA(model, "1"))				m_exif.ModelID = PANA_LX1;
					else if (!StrCmpA(model, "2"))				m_exif.ModelID = PANA_LX2;
					else if (!StrCmpA(model, "3"))				m_exif.ModelID = PANA_LX3;
				}
				else if (ISDIGIT(*model)) {
					// L シリーズ（デジタル一眼）
					m_exif.ModelID = PANA_L;
					if      (!StrCmpA(model, "10"))				m_exif.ModelID = PANA_L10;
					else if (!StrCmpA(model, "1"))				m_exif.ModelID = PANA_L1;
				}
			}
			else if (*model == 'F') {
				model++;
				if (*model == 'X') {
					// FX シリーズ
					m_exif.ModelID = PANA_FX;
					model++;
					if      (!StrCmpA(model, "1"))				m_exif.ModelID = PANA_FX1;
					else if (!StrCmpA(model, "2"))				m_exif.ModelID = PANA_FX2;
					else if (!StrCmpA(model, "5"))				m_exif.ModelID = PANA_FX5;
					else if (!StrCmpA(model, "7"))				m_exif.ModelID = PANA_FX7;
					else if (!StrCmpA(model, "8"))				m_exif.ModelID = PANA_FX8;
					else if (!StrCmpA(model, "9"))				m_exif.ModelID = PANA_FX9;
					else if (!StrCmpA(model, "01"))				m_exif.ModelID = PANA_FX01;
					else if (!StrCmpA(model, "07"))				m_exif.ModelID = PANA_FX07;
					else if (!StrCmpA(model, "30"))				m_exif.ModelID = PANA_FX30;
					else if (!StrCmpA(model, "33"))				m_exif.ModelID = PANA_FX33;
					else if (!StrCmpA(model, "55"))				m_exif.ModelID = PANA_FX55;
					else if (!StrCmpA(model, "100"))			m_exif.ModelID = PANA_FX100;
				}
				else if (*model == 'Z') {
					// FZ シリーズ
					m_exif.ModelID = PANA_FZ;
					model++;
					if      (!StrCmpA(model, "1"))				m_exif.ModelID = PANA_FZ1;
					else if (!StrCmpA(model, "2"))				m_exif.ModelID = PANA_FZ2;
					else if (!StrCmpA(model, "3"))				m_exif.ModelID = PANA_FZ3;
					else if (!StrCmpA(model, "5"))				m_exif.ModelID = PANA_FZ5;
					else if (!StrCmpA(model, "7"))				m_exif.ModelID = PANA_FZ7;
					else if (!StrCmpA(model, "8"))				m_exif.ModelID = PANA_FZ8;
					else if (!StrCmpA(model, "10"))				m_exif.ModelID = PANA_FZ10;
					else if (!StrCmpA(model, "20"))				m_exif.ModelID = PANA_FZ20;
					else if (!StrCmpA(model, "30"))				m_exif.ModelID = PANA_FZ30;
					else if (!StrCmpA(model, "18"))				m_exif.ModelID = PANA_FZ18;
					else if (!StrCmpA(model, "50"))				m_exif.ModelID = PANA_FZ50;
				}
			}
			else if (*model == 'T') {
				model++;
				if (*model == 'Z') {
					// TZ シリーズ
					m_exif.ModelID = PANA_TZ;
					model++;
					if      (!StrCmpA(model, "1"))				m_exif.ModelID = PANA_TZ1;
					else if (!StrCmpA(model, "3"))				m_exif.ModelID = PANA_TZ3;
				}
			}
		}
	}
	//----------------------------------
	// マミヤ
	//----------------------------------
	else if (m_exif.MakerID == MAKER_MAMIYA) {
		m_exif.ModelID = MAMIYA_UNKNOWN;
		if	(!memcmp(m_exif.model, "MAMIYA ZD", 9))				m_exif.ModelID = MAMIYA_ZD;
	}
	//----------------------------------
	// ライカ
	//----------------------------------
	else if (m_exif.MakerID == MAKER_LEICA) {
		if		(!memcmp(m_exif.model, "C-LUX 1", 7))			m_exif.ModelID = LEICA_CLUX1;
		else if	(!memcmp(m_exif.model, "D-LUX 2", 7) ||
				 !memcmp(m_exif.model, "DIGILUX 2", 9))			m_exif.ModelID = LEICA_DLUX2;
		else if	(!memcmp(m_exif.model, "M8 ", 3))				m_exif.ModelID = LEICA_M8;
	}
	//----------------------------------
	// サムスン
	//----------------------------------
	else if (m_exif.MakerID == MAKER_SAMSUNG) {
		model += 8;
		m_exif.ModelID = SAMSUNG_UNKNOWN;
		if (!memcmp(model, "GX", 2)) {
			model += 2;
			m_exif.MakerID = MAKER_PENTAX;
			if (!memcmp(model, "10 ", 3)) {
				// GX-10 は Pentax K10D のOEM
				m_exif.ModelID = PENTAX_K10D;
			}
			else if (!memcmp(model, "-1S", 3)) {
				// GX-1S は Pentax iST-DS2 のOEM
				m_exif.ModelID = PENTAX_ISTDS2;
			}
			else if (!memcmp(model, "-1L", 3)) {
				// GX-1L は Pentax iST-DL2 のOEM
				m_exif.ModelID = PENTAX_ISTDL2;
			}
		}
	}
}

////////////////////////////////////////////////////////////////////////
//

typedef struct _IFDTAGINFO {
	int tag;
	int type;
	int count;
	int offset;
} IFDTAGINFO;

bool rawmap::parse_tiff_ifd(int base)
{
	int tag, type, count, save, val, i;

	// ifd毎にサムネイルデータは1個だけ（無い場合もある）あるのでいったん作成しておく
	_imageinfo_t* pimage = create_ifitem();

	int entries = fget2();
	if (entries > 512) return false;

	CSimpleMap<int, IFDTAGINFO> tagmap;
	IFDTAGINFO ti;

	while (entries--) {
		if (eof()) break;

		save = tell();
		tag  = fget2();
		type = fget2();
		count= fget4();

		if (count < 0 || !tiff_seek(type, count)) {
			// このタグはデータ不全なのでスキップ
			seek(save + 12, SEEK_SET);
			continue;
		}

		// TAGを保存
		ti.tag = tag;
		ti.type = type;
		ti.count = count;
		ti.offset = tell();
		tagmap.Add(tag, ti);

		switch (tag) {
		case 0x17:
			if (type == FMT_USHORT)
				m_exif.ISOEquiv = (int)get_tagvalue(type);		// panasonic/leica raw specific ?
			break;
		case TAG_IMAGE_WIDTH:			// 0x100: 画像の幅
			val = (int)get_tagvalue(type);
			pimage->width = val;
			break;
		case TAG_IMAGE_LENGTH:			// 0x101: 画像の高さ
			val = (int)get_tagvalue(type);
			pimage->height = val;
			break;
		case TAG_BITS_SAMPLE:			// 0x102: ビットの深さ(BitsPerSample)
			val = (int)get_tagvalue(type);
			pimage->bps = val;
			break;
		case TAG_COMPRESSION:			// 0x103: 圧縮種別 (1=非圧縮, 6=JPEG, 7=DNG)
			pimage->compression = (long)get_tagvalue(type);
			break;
		case TAG_PHOTOMETRIC_INTERPRET:	// 0x106: 画素構成 (2=RGB, 6=YCbCr)
			pimage->photometric = (long)get_tagvalue(type);
			break;
		case TAG_STRIP_OFFSET:			// 0x111: 画像データのロケーション(StripOffsets)
			// イメージデータが複数のストリップチャンクに分かれている場合には、count数が2以上になる
			// ストリップデータ毎にオフセット位置が記録されているが、実際は連続したデータ（がほとんど）なので
			// 先頭のストリップチャンクのオフセット値のみを取得
			val = (int)get_tagvalue(type);
			pimage->offset = val + base;
			break;
		case TAG_SAMPLE_PIXEL:			// 0x115: Samples Per Pixel
			val = (int)get_tagvalue(type);
			pimage->spp = val;
			break;
		case TAG_STRIP_BYTE_COUNT:		// 0x117: ストリップのデータ量(StripByteCounts)
			// イメージデータが複数のストリップチャンクに分かれている場合には、count数が2以上になる
			// ストリップ毎のバイト数を加算してトータルのバイト数を計算する
			val = 0;
			for (i = 0; i < count; i++)
				val += (int)get_tagvalue(type);
			pimage->bytecount = val;
			break;
		case TAG_ORIENTATION:			// 画像方向(Oriantation)
			// sub-ifd でサムネイルのorientationが余分に指定されているときがある（Alpha-Sweetなど）
			val = (int)get_tagvalue(type);
			if (!HasInt(m_exif.Orientation))
				m_exif.Orientation = val;
			pimage->orientation = val;
			break;
		case TAG_ROWS_STRIP:
			pimage->rowsstrip = (long)get_tagvalue(type);
			break;
		case TAG_YCBCR_COEFF:			// YCbCr Coefficients
			pimage->ycbcr_coeff[0] = (float)get_tagvalue(type);
			pimage->ycbcr_coeff[1] = (float)get_tagvalue(type);
			pimage->ycbcr_coeff[2] = (float)get_tagvalue(type);
			break;
		case TAG_PLANER_CONFIG:			// 0x11C: Planar Configuration
			pimage->planer_config = (long)get_tagvalue(type);
			break;
		case TAG_YCBCR_SUBSAMPLING:		// YCbCr SubSampling
			pimage->ycbcr_subsampling[0] = (long)get_tagvalue(type);
			pimage->ycbcr_subsampling[1] = (long)get_tagvalue(type);
			break;
		case TAG_SOFTWARE:				// ソフトウェア
			read(m_exif.Software, min(count, sizeof(m_exif.Software)));
			break;
		case TAG_DATETIME:				// 編集日時
			read(m_exif.DateEdit, min(count, sizeof(m_exif.DateEdit)));
			break;
		case TAG_MAKE:					// 入力機器のメーカー名
			read_ascii(m_exif.maker, count, ARRAYSIZE(m_exif.maker));
			if      (!StrCmpNA(m_exif.maker,   "Canon", 5))			m_exif.MakerID = MAKER_CANON;
			else if (!StrCmpNA(m_exif.maker,   "NIKON", 5))			m_exif.MakerID = MAKER_NIKON;
			else if (!StrCmpNA(m_exif.maker,   "Minolta", 7) ||
					 !StrCmpNIA(m_exif.maker+7,"MINOLTA", 7))		m_exif.MakerID = MAKER_MINOLTA;
			else if (!StrCmpNA(m_exif.maker,   "OLYMPUS", 7))		m_exif.MakerID = MAKER_OLYMPUS;
			else if (!StrCmpNA(m_exif.maker,   "FUJIFILM", 8))		m_exif.MakerID = MAKER_FUJI;
			else if (!StrCmpNA(m_exif.maker,   "SONY", 4))			m_exif.MakerID = MAKER_SONY;
			else if (!StrCmpNA(m_exif.maker,   "Estman", 6))		m_exif.MakerID = MAKER_KODAK;
			else if (!StrCmpNA(m_exif.maker,   "CASIO", 5))			m_exif.MakerID = MAKER_CASIO;
			else if (!StrCmpNA(m_exif.maker,   "Panaso", 6))		m_exif.MakerID = MAKER_PANASONIC;
			else if (!StrCmpNA(m_exif.maker,   "SANYO", 5))			m_exif.MakerID = MAKER_SANYO;
			else if (!StrCmpNIA(m_exif.maker,  "PENTAX", 6))		m_exif.MakerID = MAKER_PENTAX;
			else if (!StrCmpNA(m_exif.maker,   "SIGMA", 5))			m_exif.MakerID = MAKER_SIGMA;
			else if (!StrCmpNA(m_exif.maker,   "SEIKO", 5))			m_exif.MakerID = MAKER_EPSON;
			else if (!StrCmpNIA(m_exif.maker,  "RICOH", 5))			m_exif.MakerID = MAKER_RICOH;
			else if (!StrCmpNIA(m_exif.maker,  "Mamiya", 6))		m_exif.MakerID = MAKER_MAMIYA;
			else if (!StrCmpNIA(m_exif.maker,  "LEICA", 5))			m_exif.MakerID = MAKER_LEICA;
			else if (!StrCmpNIA(m_exif.maker,  "SAMSUNG", 7))		m_exif.MakerID = MAKER_SAMSUNG;
			else if (!StrCmpNIA(m_exif.maker,  "KDDI-CA", 7))		m_exif.MakerID = MAKER_KDDI_CA;
			else if (!StrCmpNIA(m_exif.maker,  "Hassel", 6))		m_exif.MakerID = MAKER_HASSEL;
			break;
		case TAG_MODEL:					// 入力機器モデル名
			read_ascii(m_exif.model, count, ARRAYSIZE(m_exif.model));
			specify_cameramodel();
			break;
		case TAG_THUMBNAIL_OFFSET:		// サムネイル画像位置
			val = (int)get_tagvalue(type);
			pimage->thumb_offset = val + base;
			// サイズ取得
			if (jpg_getsize(pimage->thumb_offset, &pimage->width, &pimage->height))
				pimage->compression = 6;
			break;
		case TAG_THUMBNAIL_LENGTH:		// サムネイル画像データ長
			val = (int)get_tagvalue(type);
			pimage->thumb_length = val;
			break;
		case TAG_EXIF_OFFSET:			// EXIF IFD へのオフセット
			seek(fget4() + base, SEEK_SET);
			parse_exif(base);
			break;
		case TAG_GPS_OFFSET:			// GPS IFD へのオフセット
			seek(fget4() + base, SEEK_SET);
			parse_gps(base);
			break;
		case TAG_ADOBE_RESOURCE:		// Adobeリソース
			parse_adobe(base);
			break;
		case TAG_SUBIFD:				// SubIFD tag
			while (count--) {
				int save2 = tell();
				seek(fget4() + base, SEEK_SET);
				if (!parse_tiff_ifd(base))
					break;
				seek(save2 + 4, SEEK_SET);
			}
			break;
		case TAG_DNG_PRIVATE_DATA:		// DNG private data
			if (m_exif.MakerID == MAKER_PENTAX) {
				m_exif.exist_makenote = true;
				makernote_pentax();
			}
			break;
/* @@
		case TAG_TILE_WIDTH:
		case TAG_TILE_LENGTH:
		case TAG_TILE_OFFSET:
		case TAG_TILE_BYTECOUNTS:
			val = get_tagvalue(type);
			break;
@@ */
		}
		seek(save + 12, SEEK_SET);
	}

	// -----------------------------------
	// 特殊！ Panasonic RW2 ファイル
	// -----------------------------------
	if (!m_this_is_jpeg && m_exif.MakerID == MAKER_PANASONIC) {
		int index = tagmap.FindKey(0x2e);
		if (index >= 0) {
			ti = tagmap.GetValueAt(index);
/* ----
			seek(ti.offset, SEEK_SET);
			BYTE hdr[8];
			if (read(hdr, 2) == 2 && (hdr[0] == M_LEAD && hdr[1] == M_SOI)) {
				// jpeg file
				bool jpeg_has_exif = false;
				while (read(hdr, 4) == 4) {
					if (hdr[0] != M_LEAD || hdr[1] < 0xD0 || hdr[1] == M_EOI)
						break;
					if (hdr[1] == M_EXIF) {
						read(hdr, 6);
						jpeg_has_exif = true;
						break;
					}
					seek((hdr[2] << 8) + hdr[3] - 2, SEEK_CUR);
				}

				if (jpeg_has_exif && !memcmp(hdr, "Exif\0\0", 6))
					parse_tiff(tell());
			}
---- */
			if (m_largeimage) {
				// rw2のプレビュー用の埋め込みjpeg画像はexif情報を持っている
				// lsimgのローダーでjpegデータを読む際に、exifオリエンテーション情報を
				// 参照してローテートが行われるので、ここはトリッキーに「回転なし」をセット
				pimage->orientation = m_exif.Orientation = ORIENTATION_NONE;
			}
			pimage->thumb_offset = ti.offset;
			pimage->thumb_length = ti.count;
			// サイズ取得
			if (jpg_getsize(pimage->thumb_offset, &pimage->width, &pimage->height)) {
				pimage->compression = 6;
			}
		}
	}

	// 不要なイメージデータを取り除く
	if (pimage) {
/* @@
dbg.msgboxf(NULL,
	L"size=(%d,%d)\ncompress=%d\noffset=%08X\nbytecount=%d\n"
	L"bps=%d\nspp=%d\nthumb_offset=%08X\nthumb_length=%d\norientation=%d",
	pimage->width, pimage->height, pimage->compression,
	pimage->offset, pimage->bytecount, pimage->bps, pimage->spp, pimage->thumb_offset,
	pimage->thumb_length, pimage->orientation);
@@ */
		bool bRemove = true;
		if (pimage->thumb_offset && pimage->thumb_length)
			bRemove = false;
		else if (pimage->width && pimage->height && pimage->offset && pimage->bytecount) {
			switch (pimage->compression) {
			case 1:	// 非圧縮
				if (pimage->photometric == 2) {
					// RGB形式は8bitと16bitを許可
#ifdef SUPPORT_RGB16BIT_DECODE
					if (pimage->bps == 8 || pimage->bps == 16) {
#else
					if (pimage->bps == 8) {
#endif
						bRemove = false;
					}
				}
				else if (pimage->photometric == 6) {
					// YCbCr形式は8bitを許可
					if (pimage->bps == 8) {
						bRemove = false;
					}
				}
				break;
			case 6:	// JPEG
				if (pimage->bps == 8) {
					bRemove = false;
				}
				break;
			case 7:	// DNG
				if (pimage->photometric == 1 || pimage->photometric == 6) {
					if (pimage->bps == 8) {
						bRemove = false;
					}
				}
				break;
			}
		}

		if (bRemove) {
			m_ifarray.Remove(pimage);
			delete pimage;
		}
	}
	return true;
}

///////////////////////////////////////////////////////////////////////
// Parse a TIFF file looking for camera model and decompress offsets.

void rawmap::parse_tiff(int base)
{
	m_tiffbase = base;

	seek(base, SEEK_SET);
	m_order = fget2();
	fget2();			/* Should be 42 for standard TIFF */

	int oldpos = 0;
	while (1) {
		int doff = fget4();
		if (!doff || doff == EOF_ULONG) break;

		seek(doff + base, SEEK_SET);
		int pos = tell();
		if (eof() || pos <= oldpos) break;

		oldpos = pos;
		parse_tiff_ifd(base);
	}

	int i, cnt = (int)m_ifarray.GetSize();
	_imageinfo_t* pt;
	if (m_pstrm && m_exif.model[0]) {
		// イメージを書き出す
		_imageinfo_t* pmin = NULL;
		_imageinfo_t* pmax = NULL;

		for (i = 0; i < cnt; i++) {
			pt = m_ifarray[i];
			if (pt) {
				// サムネイルの面積で比較
				long area = pt->width * pt->height;
				if (area) {
					if (!pmin || (pmin->width * pmin->height > area))
						pmin = pt;
					if (!pmax || (pmax->width * pmax->height < area))
						pmax = pt;
				}
			}
		}

		pt = m_largeimage ? pmax : pmin;
		if (pt) {
			long offset, length;
			if (pt->compression == 6 || pt->compression == 7) {
				// JPEG
				if (pt->thumb_offset && pt->thumb_length) {
					offset = pt->thumb_offset;
					length = pt->thumb_length;
				}
				else {
					offset = pt->offset;
					length = pt->bytecount;
				}

				if (data_streamout(offset, length)) {
					m_imagewrite_completed = true;
				}
			}
			else if (pt->compression == 1) {
				// 非圧縮
				int widthbytes = WIDTHBYTES(pt->width * 24);
				int colbytes = pt->width * pt->spp;

				CTypeAlloc<BYTE> src(pt->bytecount + widthbytes * pt->height);
				BYTE* dst = src + pt->bytecount;

				seek(pt->offset, SEEK_SET);
				if (read(src, pt->bytecount) == pt->bytecount) {
					write_bmpfileheader(pt->width, pt->height);
					if (pt->photometric == 2) {
#ifdef SUPPORT_RGB16BIT_DECODE
						// bpsが16の場合は8bitにスケールダウン
						if (pt->bps == 16) {
							WORD* ps = (WORD*)(BYTE*)src;
							BYTE* pd = src;
							length = pt->width * pt->height * pt->spp;
							while (length-- > 0) {
								WORD w = *ps++;
								*pd++ = (BYTE)((w >> 5) & 0xff);
							}
						}
#endif
						// RGB
						if (m_exif.MakerID == MAKER_MAMIYA && !m_this_is_jpeg) {
							// MAMIYAのRAWの場合、画像にイコライズかけないと暗すぎる
							equalize(pt->width, pt->height, src);
						}

						for (int y = 0; y < pt->height; y++) {
							BYTE* p = src + colbytes * (pt->height - y - 1);
							BYTE* q = dst + widthbytes * y;
							for (int x = 0; x < pt->width; x++) {
								*q++ = p[2];
								*q++ = p[1];
								*q++ = p[0];
								p += 3;
							}
						}
						m_pstrm->Write(dst, widthbytes * pt->height, NULL);
						m_imagewrite_completed = true;
					}
					else if (pt->photometric == 6 && pt->planer_config == 1) {
						// YCbCr 本当は ycbcr_subsampling も考慮しなければいけないが…
						float y1, y2, cb, cr, r, g, b;
						BYTE* p = src;
						for (int y = 0; y < pt->height; y++) {
							BYTE* q = dst + widthbytes * (pt->height - y - 1);
							for (int x = 0; x < pt->width;) {
								y1 = (float)*p++;
								y2 = (float)*p++;
								cb = (float)*p++;
								cr = (float)*p++;

								b = (cb - 128) * (2 - pt->ycbcr_coeff[2] * 2) + y1;
								r = (cr - 128) * (2 - pt->ycbcr_coeff[0] * 2) + y1;
								g = (y1 - pt->ycbcr_coeff[2] * b - pt->ycbcr_coeff[0] * r) / pt->ycbcr_coeff[1];
								*q++ = min(255, max((int)b, 0));
								*q++ = min(255, max((int)g, 0));
								*q++ = min(255, max((int)r, 0));

								b = (cb - 128) * (2 - pt->ycbcr_coeff[2] * 2) + y2;
								r = (cr - 128) * (2 - pt->ycbcr_coeff[0] * 2) + y2;
								g = (y2 - pt->ycbcr_coeff[2] * b - pt->ycbcr_coeff[0] * r) / pt->ycbcr_coeff[1];
								*q++ = min(255, max((int)b, 0));
								*q++ = min(255, max((int)g, 0));
								*q++ = min(255, max((int)r, 0));
								x += 2;
							}
						}
						m_pstrm->Write(dst, widthbytes * pt->height, NULL);
						m_imagewrite_completed = true;
					}
				}
			}

			if (m_imagewrite_completed) {
				m_thumb_orientation = pt->orientation;
			}
		}
	}

	// clean-up
	for (i = 0; i < cnt; i++) {
		pt = m_ifarray[i];
		if (pt) delete pt;
	}
	m_ifarray.RemoveAll();
}

///////////////////////////////////////////////////////////////////////
// Parse the CIFF structure.

void rawmap::parse_ciff(int offset, int length)
{
	register canon_t* canon = &m_exif.canon;

	int len, roff, aoff=0;

	seek(offset + length - 4, SEEK_SET);
	int tboff = fget4() + offset;
	seek(tboff, SEEK_SET);
	int entries = fget2();
	while (entries--) {
		if (eof()) break;

		int save = tell();
		int tag = fget2();

		int tagID = tag & 0x3fff;			// get tag ID
		int tagType;
		switch ((tag >> 8) & 0x38) {
		case 0x00: tagType = FMT_BYTE; break;
		case 0x08: tagType = FMT_STRING; break;
		case 0x10: tagType = FMT_USHORT; break;
		case 0x18: tagType = FMT_ULONG; break;
		default:   tagType = FMT_BYTE; break;
		}

		if (tag & 0x4000)
			len = 8;
		else {
			len  = fget4();
			roff = fget4();
			aoff = offset + roff;
			seek(aoff, SEEK_SET);
		}

		switch (tagID) {
		case 0x0805:									// ASCII: ImageDescription
			break;
		case 0x080A:									// メーカー名・機種名
		{
			char* asc = new char[len + 2];
			read(asc, len);
			lstrcpyA(m_exif.maker, asc);
			lstrcpyA(m_exif.model, asc + lstrlenA(m_exif.maker) + 1);
			delete [] asc;
			specify_cameramodel();
			break;
		}
		case 0x080B:									// ASCII: ファームウェア
			read_ascii(canon->FirmWare, len, ARRAYSIZE(canon->FirmWare));
			break;
		case 0x080C:									// ComponentVersion
			break;
		case 0x080D:									// ROMOperationMode
			break;
		case 0x0810:									// 所有者名
			read_ascii(canon->OwnerName, len, ARRAYSIZE(canon->OwnerName));
			break;
		case 0x0815:									// CanonImageType
			read_ascii(canon->ImageType, len, ARRAYSIZE(canon->ImageType));
			break;
		case 0x0816:									// OriginalFileName
			break;
		case 0x0817:									// ThumbnailFileName
			break;
		case 0x100A:									// TargetImageType
			break;
		case 0x1010:									// ShutterReleaseMethod
															// 0 => 'Single Shot'
															// 2 => 'Continuous Shooting'
			break;
		case 0x1011:									// ShutterReleaseTiming
															// 0 => 'Priority on shutter'
															// 1 => 'Priority on focus'
			break;
		case 0x1016:									// ReleaseSetting
			break;
		case 0x101C:									// BaseISO
			break;
		case 0x1029:									// 焦点距離
			makernote_canon_focalinfo(len / sizeof(short));
			break;
		case 0x102A:									// 露出情報
			makernote_canon_shotinfo(len / sizeof(short));
			break;
		case 0x102D:									// 撮影情報
			makernote_canon_settinginfo(len / sizeof(short));
			break;
		case 0x1031:									// CCD/CMOSセンサー情報
			makernote_canon_deviceinfo(len / sizeof(short));
			break;
		case 0x1038:
			makernote_canon_pictureinfo(len / sizeof(short));
			break;
		case 0x1093:
			makernote_canon_fileinfo(len / sizeof(short));
			break;
		case 0x10AE:									// 色温度
			canon->ColorTemp = (int)get_tagvalue(tagType);
			break;
//@		case 0x10B4:									// カラースペース
//@															// 1 => 'sRGB',
//@															// 2 => 'Adobe RGB',
//@															// 0xffff => 'Uncalibrated',
//@			break;
		case 0x180B:									// シリアル番号
			canon->SerialNumber = (int)get_tagvalue(tagType);
			break;
		case 0x180E:									// 撮影日時
		{
			time_t t = (time_t)get_tagvalue(tagType);
			struct tm* lt = gmtime(&t);
			if (lt) {
				wsprintfA(m_exif.Date, "%04d/%02d/%02d %02d:%02d:%02d",
					lt->tm_year + 1900, lt->tm_mon + 1, lt->tm_mday, lt->tm_hour, lt->tm_min, lt->tm_sec);
			}
			break;
		}
		case 0x1810:									// イメージサイズ
			makernote_canonraw_imageinfo(len / sizeof(long));
			break;
		case 0x1817:									// ファイル番号
			canon->FiShutterCount = (int)get_tagvalue(tagType);
			break;
		case 0x2007:									// JPEG画像（同時記録JPEG）
			if (data_streamout(aoff, len))
				m_imagewrite_completed = true;
			break;
//		case 0x2008:									// JPEG画像（サムネイル）
//			m_thumb_offset = aoff;
//			m_thumb_length = len;
//			break;
		}

		// Subdir parsing
		switch (tagID >> 8) {
		case 0x28:
		case 0x30:
			parse_ciff(aoff, len);
			break;
		}
		seek(save + 10, SEEK_SET);
	}
}

void rawmap::parse_minolta()
{
	m_exif.MakerID = MAKER_MINOLTA;
	seek(4, SEEK_SET);
	int tlen = fget4();							// イメージデータ位置

	int data_offset = tlen + 8;
	while (tell() < data_offset) {
		int save = tell();
		int tag = fget4();
		int len = fget4();
		switch (tag) {
		case 0x505244:				// PRD(Picture Raw Dimensions)
			seek(12, SEEK_CUR);
			m_exif.ImageHeight  = fget2();
			m_exif.ImageWidth = fget2();
			break;
//		case 0x574247:				// WBG(White Balance Gains)
//			fget4();
//			camera_red  = fget2();
//			camera_red /= fget2();
//			camera_blue = fget2();
//			camera_blue = fget2() / camera_blue;
//			break;
/* @@
		case 0x524946:				// RIF(Requested Image Format)
{
	char* val = new char[len];
	read(val, len);
	dbg.printf(L"Saturation=%d\n", (int)val[1]);
	dbg.printf(L"Contrast=%d\n", (int)val[2]);
	dbg.printf(L"Sharpness=%d\n", (int)val[3]);
	dbg.printf(L"WhiteBalance=%d\n", (int)val[4]);
	dbg.printf(L"SubjectProgram=%d\n", (int)val[5]);
	dbg.printf(L"FilmSpeed(ISO)=%d\n", (int)val[6]);
	dbg.printf(L"ColorMode=%d\n", (int)val[7]);
	dbg.printf(L"ColorFilter=%d\n", (int)val[56]);
	dbg.printf(L"B&WFilter=%d\n", (int)val[57]);
	delete [] val;
	break;
}
@@ */
		case 0x545457:				// TTW
			parse_tiff(tell());
			break;
		}
		seek(save + len + 8, SEEK_SET);
	}
}

///////////////////////////////////////////////////////////////////////
// Identify which camera created this file, and set global variables
// accordingly.	 Return nonzero if the file cannot be decoded.

bool rawmap::identify(bool bLargeImage)
{
	BYTE hdr[32];
	unsigned toff, tlen, lsize;

	rawinit();
	m_largeimage = bLargeImage;

	int base = tell();				// 先頭位置を記憶
	m_order = fget2();				// オーダー II or MM
	int hlen = fget4();				// ヘッダ長
	seek(base, SEEK_SET);			// 先頭へ戻る
	read(hdr, 32);

	if (m_order == 0x4949 || m_order == 0x4d4d) {
		if (!memcmp(hdr + 6, "HEAPCCDR", 8)) {
			m_exif.MakerID = MAKER_CANON;

			// メーカーノート初期化
			_canon_init(&m_exif.canon);
			m_exif.exist_makenote = true;
			parse_ciff(hlen, filesize() - hlen);

			// EXIF情報として不足しているものを補完
			canon_t* canon = &m_exif.canon;
			if (HasInt(canon->FocalLength)) {
				if (IS_CANON_EOS(m_exif.ModelID))
					m_exif.FocalLength = (float)canon->FocalLength;
				else if ((m_exif.ModelID == CANON_POWERSHOT_S3IS) ||
						 (m_exif.ModelID == CANON_POWERSHOT_S2IS))
					m_exif.FocalLength = (float)canon->FocalLength / 100.0F;
				else if ((canon->FocalLength < 1000) || (m_exif.ModelID == CANON_POWERSHOT_PRO1))
					m_exif.FocalLength = (float)canon->FocalLength * 3.125F / 100.0F;
				else
					m_exif.FocalLength = (float)canon->FocalLength / 1000.0F;
			}
			if (!HasInt(m_exif.ISOEquiv)) {
				if (HasInt(canon->ISOapex))					// ISO
					m_exif.ISOEquiv = canon->ISOapex;
			}
			if (HasFlt(canon->Aperture))					// 絞り値
				m_exif.Aperture = canon->Aperture;
			if (HasFlt(canon->ExposureTime))				// 露出時間
				m_exif.ExposureTime = canon->ExposureTime;
			if (HasInt(canon->ExposureComp))
				m_exif.ExposureBias = _canon_ev_conv(canon->ExposureComp);
			if (HasInt(canon->FlashCondition))
				m_exif.Flash = (canon->FlashCondition == 0) ? 0 : 1;
			if (HasInt(canon->ImageWidth))
				m_exif.ImageWidth = canon->ImageWidth;
			if (HasInt(canon->ImageHeight))
				m_exif.ImageHeight = canon->ImageHeight;

			seek(base + hlen, SEEK_SET);
		}
		else {
			// NIKON, SONY-α, etc…
			parse_tiff(base);
		}
	}
	else if (!memcmp(hdr, "\0MRM", 4)) {
		// コニカミノルタ
		parse_minolta();
	}
	else if (!memcmp(hdr, "FUJIFILM", 8)) {
		// FUJIFILM
		m_exif.MakerID = MAKER_FUJI;
		seek(base + 84, SEEK_SET);
		toff = fget4();
		tlen = fget4();
		parse_tiff(toff + 12);

		if (m_largeimage) {
			data_streamout(toff, tlen);
			m_imagewrite_completed = true;
		}
	}
	else if (!memcmp(hdr, "FOVb", 4)) {
		// SIGMA
		return foveon_decode();
	}
	else if (hdr[0] == M_LEAD && hdr[1] == M_SOI) {
		// jpeg file
		m_this_is_jpeg = true;
		seek(base + 2, SEEK_SET);
		while (read(hdr, 4) == 4) {
			if (hdr[0] != M_LEAD || hdr[1] < 0xD0 || hdr[1] == M_EOI)
				return false;

			if (hdr[1] == M_EXIF) {
				read(hdr, 6);
				break;
			}

			seek((hdr[2] << 8) + hdr[3] - 2, SEEK_CUR);
		}

		if (memcmp(hdr, "Exif\0\0", 6))
			return false;
		parse_tiff(tell());
	}
#ifdef SUPPORT_ROLLEI
	else if (!memcmp(hdr, "DSC-Image", 9)) {
		parse_rollei();
	}
#endif	// SUPPORT_ROLLEI

	if (m_exif.maker[0] == 0) return false;			// サポートしていないファイル形式
	if (!m_pstrm) return true;						// サムネイル画像必要なし
	if (m_imagewrite_completed) return true;		// サムネイル取得済み

#ifdef SUPPORT_KODAK
	if (!lstrcmpiA(m_exif.model, "DCS Pro 14N"))
		return kodak_yuv_decode();
#endif	// SUPPORT_KODAK

#ifdef SUPPORT_ROLLEI
	if (!memcmp(m_exif.maker, "Rollei", 6)) return rollei_decode();
#endif	// SUPPORT_ROLLEI

	if (!m_thumb_length)
		return false;					// サムネイルイメージが見つからない

	CTypeAlloc<BYTE> thumb(m_thumb_length);
	seek(base + m_thumb_offset, SEEK_SET);
	read(thumb, m_thumb_length);
	if (m_thumb_layers) {
		CTypeAlloc<BYTE> rgb(m_thumb_length);
		lsize = m_thumb_length / 3;
		for (int n = 0; n < m_thumb_length; n++)
			rgb[(n % lsize) * 3 + n / lsize] = thumb[n];
		memcpy(thumb, rgb, m_thumb_length);
	}
	m_pstrm->Write(m_head, m_headlen, NULL);
	if (m_diborder) {
		int widthbytes = WIDTHBYTES(m_thumb_width * 24);
		int linebytes = m_thumb_width * 3;
		CTypeAlloc<BYTE> bgr(widthbytes);
		for (int y = 0; y < m_thumb_height; y++) {
			BYTE* p = thumb + linebytes * (m_thumb_height - y - 1);
			BYTE* q = bgr;
			for (int x = 0; x < m_thumb_width; x++) {
				*q++ = p[2];
				*q++ = p[1];
				*q++ = p[0];
				p += 3;
			}
			m_pstrm->Write(bgr, widthbytes, NULL);
		}
	}
	else
		m_pstrm->Write(thumb, m_thumb_length, NULL);

	return true;
}

///////////////////////////////////////////////////////////////////////
// CheckJpegExif

bool rawmap::isexif()
{
	bool bExif = false;
	int save = tell();
	seek(0, SEEK_SET);
	BYTE hdr[32];
	read(hdr, 32);

	LPTSTR ext = PathFindExtension(m_fname);
	m_order = *(WORD*)hdr;
	if (hdr[0] == M_LEAD && hdr[1] == M_SOI) {
		// jpeg file
		seek(2, SEEK_SET);
		while (read(hdr, 4) == 4) {
			if (hdr[0] != M_LEAD || hdr[1] < 0xD0 || hdr[1] == M_EOI)
				break;

			if (hdr[1] != M_EXIF) {
				seek((hdr[2] << 8) + hdr[3] - 2, SEEK_CUR);
				continue;
			}

			read(hdr, 6);
			break;
		}

		if (!memcmp(hdr, "Exif\0\0", 6)) bExif = true;
	}
	else if (m_order == 0x4949 || m_order == 0x4d4d) {
		if (!StrCmpI(ext, _T(".crw")) && !memcmp(hdr + 6, "HEAPCCDR", 8)) {
			bExif = true;
		}
		else {
			// tiff
			seek(4, SEEK_SET);

			// 最初のIFDのみチェック
			int doff = fget4();
			if (doff) {
				seek(doff, SEEK_SET);
				int entries = fget2();
				while (entries--) {
					int tag = fget2();
					if (tag == TAG_EXIF_OFFSET) {			// EXIFオフセット
						bExif = true;
						break;
					}
					seek(10, SEEK_CUR);
				}
			}
		}
	}
	else if (!StrCmpI(ext, _T(".mrw")) && !memcmp(hdr, "\0MRM", 4))
		bExif = true;
	else if (!StrCmpI(ext, _T(".raf")) && !memcmp(hdr, "FUJIFILM", 8))
		bExif = true;
	else if (!StrCmpI(ext, _T(".x3f")) && !memcmp(hdr, "FOVb", 4))
		bExif = true;
	seek(save, SEEK_SET);
	return bExif;
}

static BOOL exif_parse(LPCTSTR fname, IStream* pstrm, exif_t* info, bool bLargeImage)
{
	BOOL bOK = FALSE;
	rawmap* fm = new rawmap();
	fm->m_pstrm = pstrm;
	if (fm->open(fname)) {
		lstrcpy(fm->m_fname, fname);
		bOK = fm->identify(bLargeImage);
		if (bOK) {
/* --- 無理に計算しない
			// 35mm換算の焦点距離
			if (!HasFlt(fm->m_exif.exFocalLength35) &&
				HasFlt(fm->m_exif.FocalPlaneXResolution) &&
				HasFlt(fm->m_exif.FocalPlaneResolutionUnits) &&
				HasInt(fm->m_exif.ImageWidth)) {
				float ccd = (float)(fm->m_exif.ImageWidth * fm->m_exif.FocalPlaneResolutionUnits / fm->m_exif.FocalPlaneXResolution);
				fm->m_exif.exFocalLength35 = fm->m_exif.FocalLength * 36 / ccd;
			}
--- */
			if (pstrm) {
				LARGE_INTEGER li;
				li.QuadPart = 0;
				pstrm->Seek(li, STREAM_SEEK_SET, NULL);
			}
			if (info)
				memcpy(info, &fm->m_exif, sizeof(exif_t));
		}
	}
	delete fm;
	return bOK;
}

////////////////////////////////////////////////////////////////////////
//

static const TCHAR sszFmtLensInfo[] = _T("レンズ: %s\n");
static const TCHAR sszFmtSize[] = _T("大きさ: %d x %d\n");

static LPTSTR build_nikon_lensinfo(LPTSTR psz, int lenstype, float* lensinfo)
{
	*psz = 0;

	// レンズタイプ
	if (lenstype == 1)
		lstrcpy(psz, _T("MF "));
	else {
		lstrcpy(psz, _T("AF "));
		if (lenstype & 0x08)
			lstrcat(psz, _T("VR "));
	}

	LPTSTR pt = &psz[lstrlen(psz)];

	// レンズ情報
	TCHAR buf[128];
	LPTSTR p0 = buf, p2 = &buf[40];
	_format_int(p0, (int)lensinfo[0]);
	_format_float(p2, lensinfo[2]);
	if (lensinfo[0] == lensinfo[1])
		wsprintf(pt, _T("%smm F%s"), p0, p2);
	else {
		LPTSTR p1 = &buf[20];
		_format_int(p1, (int)lensinfo[1]);
		if (lensinfo[2] == lensinfo[3])
			wsprintf(pt, _T("%s-%smm F%s"), p0, p1, p2);
		else {
			LPTSTR p3 = &buf[60];
			_format_float(p3, lensinfo[3]);
			wsprintf(pt, _T("%s-%smm F%s-%s"), p0, p1, p2, p3);
		}
	}

	if (lenstype & 0x04)
		lstrcat(pt, _T("G"));
	else if (lenstype & 0x02)
		lstrcat(pt, _T("D"));
	return psz;
}

static LPTSTR build_generic_lensinfo(LPTSTR psz, int minfl, int maxfl, float minap, float maxap)
{
	LPTSTR p;
	*psz = 0;
	_format_int(psz, minfl);
	if (minfl != maxfl) {
		p = psz + lstrlen(psz);
		*p++ = '-';
		_format_int(p, maxfl);
	}
	_append_mm(psz);

	if (minap != FLT_MAX && maxap != FLT_MAX) {
		p = psz + lstrlen(psz);
		*p++ = ' ';
		*p++ = 'F';
		_format_float(p, minap);
		if (minap != maxap) {
			p = psz + lstrlen(psz);
			*p++ = '-';
			_format_float(p, maxap);
		}
	}
	return psz;
}

static void build_exif_infotip(exif_t* exif, EXIFINFOSTRUCT* info)
{
	if (!info || !info->pszTipText || info->cchTipText < 1) return;

	*info->pszTipText = 0;

	TCHAR wsz[256];
	TCHAR* tmp = new TCHAR[1024];

	int len = 0;
	if (*info->ModelName)
		len += wsprintf(&tmp[len], _T("モデル名: %s\n"), info->ModelName);
	if (HasInt(info->ImageWidth) && HasInt(info->ImageHeight))
		len += wsprintf(&tmp[len], sszFmtSize, info->ImageWidth, info->ImageHeight);

	if (info->DateTime.wYear) {
		TCHAR* psz = wsz;
		GetDateFormat(LOCALE_USER_DEFAULT, DATE_LONGDATE, &info->DateTime, NULL, psz, ARRAYSIZE(wsz));
		psz += lstrlen(wsz);
		*psz++ = _T(' ');
		GetTimeFormat(LOCALE_USER_DEFAULT, 0, &info->DateTime, NULL, psz, ARRAYSIZE(wsz) - lstrlen(wsz));
		len += wsprintf(&tmp[len], _T("撮影日時: %s\n"), wsz);
	}

	// format = "撮影情報: 1/250 F4.6 ±0EV ISO200"
	TCHAR* p = wsz;
	*p = 0;

	if (HasInt(info->ExposureProgram)) {
		switch (info->ExposureProgram) {
		case 1: *p++ = ' '; *p++ = 'M'; break;
		case 2: *p++ = ' '; *p++ = 'P'; break;
		case 3: *p++ = ' '; *p++ = 'A'; break;
		case 4: *p++ = ' '; *p++ = 'S'; break;
		case 5: *p++ = ' '; *p++ = 'C'; *p++ = 'r'; break;
		case 6: *p++ = ' '; *p++ = 'A'; *p++ = 'c'; break;
		case 7: *p++ = ' '; *p++ = 'P'; *p++ = 'o'; break;
		case 8: *p++ = ' '; *p++ = 'L'; *p++ = 'a'; break;
		}
	}
	if (HasFlt(info->ExposureTime)) {
		*p++ = ' ';
		_format_shutter(p, info->ExposureTime);
		p += lstrlen(p);
	}
	if (HasFlt(info->Aperture)) {
		*p++ = ' ';
		_format_Fnumber(p, info->Aperture);
		p += lstrlen(p);
	}
	if (HasFlt(info->ExposureBias) && info->ExposureBias != 0.0F) {
		*p++ = ' ';
		_format_ev_comp(p, info->ExposureBias);
		p += lstrlen(p);
	}
	if (HasInt(info->ISOEquiv)) {
		*p++ = _T(' ');
		*p++ = _T('I');
		*p++ = _T('S');
		*p++ = _T('O');
		if (info->ISOEquiv != 0)
			_format_int(p, info->ISOEquiv);
		else
			lstrcpy(p, _T("オート"));
		p += lstrlen(p);
	}
	if (*wsz)
		len += wsprintf(&tmp[len], _T("撮影情報:%s\n"), wsz);

	// レンズ情報
	if (info->Lens[0])
		len += wsprintf(&tmp[len], sszFmtLensInfo, info->Lens);

	// メーカー独自情報など
	if (exif->exist_makenote) {
		switch (exif->MakerID) {
		case MAKER_CANON:
			// ピクチャースタイル
			if (HasInt(exif->canon.ColorMatrix) && IS_CANON_PICTURESTYLE(exif->ModelID)) {
				_format_lookup(wsz, exif->canon.ColorMatrix, ekCanonColorMatrix, -1);
				len += wsprintf(&tmp[len], _T("ピクチャースタイル: %s\n"), wsz);
			}
			break;
		case MAKER_NIKON:
			break;
		case MAKER_FUJI:
			break;
		case MAKER_PENTAX:
			break;
		case MAKER_MINOLTA:
			break;
		case MAKER_OLYMPUS:
			break;
		case MAKER_LEICA:
		case MAKER_PANASONIC:
			if (info->Lens[0] == 0 && HasInt(exif->panasonic.ShootingMode)) {
				// 撮影モード
				_format_lookup(wsz, exif->panasonic.ShootingMode, ekPanaShootingMode, -1);
				len += wsprintf(&tmp[len], _T("シーンモード: %s\n"), wsz);
			}
			break;
		case MAKER_SONY:
			break;
		}
	}


	if (len > 0) {
		if (tmp[len - 1] == '\n') tmp[--len] = 0;
		lstrcpyn(info->pszTipText, tmp, info->cchTipText);
	}
	delete [] tmp;
}

static void build_exif_label(EXIFINFOSTRUCT* info)
{
	if (!info) { return; }
	if (!info->pszLabelText) { return; }
	if (info->cchLabelText < 1) { return; }


	*info->pszLabelText = 0;
	TCHAR wsz[128];
	TCHAR* p = wsz;
	*p = 0;
	if (HasFlt(info->ExposureTime)) {
		if (info->ExposureTime < 0.3F)
			p += wsprintf(p, _T("%3d"), (int)(0.5F + 1.0F / info->ExposureTime));
		else {
			float f = info->ExposureTime + 0.05F;
			p += wsprintf(p, _T("%d\"%d"), (int)f, (int)floorf(f * 10) % 10);
		}
		*p++ = ' ';
	}

	if (HasFlt(info->Aperture)) {
		_format_Fnumber(p, info->Aperture);
		p += lstrlen(p);
		*p++ = ' ';
	}
	if (HasFlt(info->ExposureBias) && info->ExposureBias != 0.0F) {
		_format_float_comp(p, info->ExposureBias);
		if (*p == 0x00b1) *p = 0x3b;
		p += lstrlen(p);
		*p++ = ' ';
	}
	if (HasInt(info->ISOEquiv)) {
		if (info->ISOEquiv != 0)
			p += wsprintf(p, _T("%d"), info->ISOEquiv);
		else
			p += wsprintf(p, _T("Auto"));
		*p++ = ' ';
	}

	if (info->FlashFired) {
		if (*(p - 1) == ' ') *p-- = 0;
		*p++ = '*';
		*p++ = ' ';
	}
	*p = 0;

	if (p > wsz) {
		LPTSTR label = info->pszLabelText;
		if (wsz[0] != ' ') *label++ = ' ';
		lstrcpyn(label, wsz, info->cchLabelText);
	}
}

static void get_exif_info(exif_t* exif, EXIFINFOSTRUCT* info)
{
	if (!info) return;

	// メーカー名
	info->ModelName[0] = 0;
	if (HasStr(exif->maker))
		_format_ascii(info->MakerName, exif->maker, ARRAYSIZE(info->MakerName));
	// モデル名
	info->ModelName[0] = 0;
	if (HasStr(exif->model))
		_format_ascii(info->ModelName, exif->model, ARRAYSIZE(info->ModelName));
	// イメージサイズ
	info->ImageWidth = INT_MAX;
	info->ImageHeight = INT_MAX;
	if (HasInt(exif->ImageWidth) && HasInt(exif->ImageHeight)) {
		info->ImageWidth = exif->ImageWidth;
		info->ImageHeight = exif->ImageHeight;
	}
	else {
		switch (exif->MakerID) {
		case MAKER_OLYMPUS:
			if (HasInt(exif->olympus.ImageWidth) && HasInt(exif->olympus.ImageHeight)) {
				info->ImageWidth = exif->olympus.ImageWidth;
				info->ImageHeight = exif->olympus.ImageHeight;
			}
			break;
		}
	}
	// 画像の向き
	if (HasInt(exif->Orientation))
		info->Orientation = exif->Orientation;
	else
		info->Orientation = 1;
	// 露出制御プログラム
	info->ExposureProgram = HasInt(exif->ExposureProgram) ? exif->ExposureProgram : INT_MAX;
	// 撮影日時
	if (HasStr(exif->Date))
		_str2datetime(exif->Date, &info->DateTime);
	// ISO感度
	info->ISOEquiv = HasInt(exif->ISOEquiv) ? exif->ISOEquiv : INT_MAX;
	// 露出時間
	info->ExposureTime = HasFlt(exif->ExposureTime) ? exif->ExposureTime : (HasFlt(exif->ShutterSpeed) ? exif->ShutterSpeed : FLT_MAX);
	// 絞り
	info->Aperture = HasFlt(exif->FNumber) ? exif->FNumber : (HasFlt(exif->Aperture) ? exif->Aperture : FLT_MAX);
	// 露出補正
	info->ExposureBias = exif->ExposureBias;
	// 焦点距離
	info->FocalLength = exif->FocalLength;
	// 焦点距離(35mm換算)
	info->FocalLength35 = exif->FocalLength35;
	// フラッシュ
	info->FlashFired = (HasInt(exif->Flash) && (exif->Flash & 1));
	// 測光モード
	info->MeteringMode = HasInt(exif->MeteringMode) ? exif->MeteringMode : INT_MAX;
	// 開放F値
	info->MaxAperture = HasFlt(exif->MaxAperture) ? exif->MaxAperture : FLT_MAX;
	// ホワイトバランス
	info->WhiteBalance = HasInt(exif->WhiteBalance) ? exif->WhiteBalance : INT_MAX;
	// ホワイトバランス文字列
	TCHAR wsz[256];
	info->WhiteBalanceText[0] = 0;
	if (HasInt(exif->WhiteBalance)) {
		_format_enumtext(wsz, exif->WhiteBalance, euWhiteBalanceMode, 2);
		lstrcpyn(info->WhiteBalanceText, wsz, ARRAYSIZE(info->WhiteBalanceText));
	}

	// メーカー独自情報など
	info->Lens[0] = 0;
	LPTSTR p;
	if (exif->exist_makenote) {
		switch (exif->MakerID) {
		case MAKER_CANON:
			if (HasStr(exif->canon.LensName))
				_format_ascii(wsz, exif->canon.LensName, ARRAYSIZE(wsz));
			else if (HasInt(exif->canon.CiLensTypes)) {
				_format_lookup(wsz, exif->canon.CiLensTypes, ekCanonLensTypes, -1);
			}
			else if (HasInt(exif->canon.MaxFocalLength) &&
					 HasInt(exif->canon.MinFocalLength) &&
					 HasInt(exif->canon.FocalLengthUnit)) {
				_format_float_int(wsz, (float)exif->canon.MinFocalLength / (float)exif->canon.FocalLengthUnit);
				if (exif->canon.MinFocalLength != exif->canon.MaxFocalLength) {
					lstrcat(wsz, _T("-"));
					p = wsz + lstrlen(wsz);
					_format_float_int(p, (float)exif->canon.MaxFocalLength / (float)exif->canon.FocalLengthUnit);
				}
				_append_mm(wsz);
			}
			lstrcpyn(info->Lens, wsz, ARRAYSIZE(info->Lens));

			// ホワイトバランス
			if (HasInt(exif->canon.WhiteBalance)) {
				_format_lookup(wsz, exif->canon.WhiteBalance, ekCanonWhiteBalance, -1);
				lstrcpyn(info->WhiteBalanceText, wsz, ARRAYSIZE(info->WhiteBalanceText));
			}
			break;
		case MAKER_NIKON:
			wsz[0] = 0;
			if (HasStr(exif->nikon.LensData)) {
				_format_string_lookup(wsz, exif->nikon.LensData, ekNikonLensData);
			}
			else if (HasInt(exif->nikon.LensID) && 
				(exif->nikon.LensID != 0) &&		// !Tokina
				(exif->nikon.LensID != 3) &&		// !Soligor
				(exif->nikon.LensID != 38)) {		// !Sigma
				_format_lookup(wsz, exif->nikon.LensID, ekNikonLensID, -1);
			}
			else if (HasStr(exif->nikon.LensModel)) {
				_format_ascii(wsz, exif->nikon.LensModel, ARRAYSIZE(wsz));
			}
			else if (HasInt(exif->nikon.LensType) && HasFlt(exif->nikon.LensInfo[0])) {
				build_nikon_lensinfo(wsz, exif->nikon.LensType, exif->nikon.LensInfo);
			}
			if (wsz[0])
				lstrcpyn(info->Lens, wsz, ARRAYSIZE(info->Lens));

			// ホワイトバランス
			if (HasStr(exif->nikon.WhiteBalance)) {
				_format_string_lookup(wsz, exif->nikon.WhiteBalance, etNikonWhiteBalance);
				lstrcpyn(info->WhiteBalanceText, wsz, ARRAYSIZE(info->WhiteBalanceText));
			}
			break;
		case MAKER_FUJI:
			if (HasInt(exif->fuji.MaxFocalLength) &&
				HasInt(exif->fuji.MinFocalLength)) {
				build_generic_lensinfo(wsz, exif->fuji.MinFocalLength, exif->fuji.MaxFocalLength, exif->fuji.MaxApertureAtMinFocal, exif->fuji.MaxApertureAtMaxFocal);
				lstrcpyn(info->Lens, wsz, ARRAYSIZE(info->Lens));
			}

			// ホワイトバランス
			if (HasInt(exif->fuji.WhiteBalance)) {
				_format_lookup(wsz, exif->fuji.WhiteBalance, ekFujiWhiteBalance, -1);
				lstrcpyn(info->WhiteBalanceText, wsz, ARRAYSIZE(info->WhiteBalanceText));
			}
			break;
		case MAKER_PENTAX:
			if (HasInt(exif->pentax.LensInfo)) {
				_format_lookup(wsz, exif->pentax.LensInfo, ekPentaxLensInfo, -1);
				lstrcpyn(info->Lens, wsz, ARRAYSIZE(info->Lens));
			}

			// ホワイトバランス
			if (HasInt(exif->pentax.WhiteBalance)) {
				_format_lookup(wsz, exif->pentax.WhiteBalance, ekPentaxWhiteBalanceMode, -1);
				lstrcpyn(info->WhiteBalanceText, wsz, ARRAYSIZE(info->WhiteBalanceText));
			}
			break;
		case MAKER_MINOLTA:
			if (HasInt(exif->minolta.LensInfo)) {
////////////////////////////////////////
// use sony alpha-lens-id list
//				_format_lookup(wsz, exif->minolta.LensInfo, ekMinoltaLensInfo, -1);
				_format_lookup(wsz, exif->minolta.LensInfo, ekSonyLensID, -1);
				lstrcpyn(info->Lens, wsz, ARRAYSIZE(info->Lens));
			}
			// ホワイトバランス
			if (HasInt(exif->minolta.WhiteBalance)) {
				if (exif->ModelID == MINOLTA_ALPHA_7D)
					_format_lookup(wsz, exif->minolta.WhiteBalance, ekMinoltaWhiteBalance7D, -1);
				else
					_format_lookup(wsz, exif->minolta.WhiteBalance, ekMinoltaWhiteBalance, -1);
				lstrcpyn(info->WhiteBalanceText, wsz, ARRAYSIZE(info->WhiteBalanceText));
			}
			break;
		case MAKER_OLYMPUS:
			if (HasInt(exif->olympus.LensInfo) && exif->olympus.LensInfo) {
				if (!_format_lookup(wsz, exif->olympus.LensInfo, ekOlymLensInfo, -1)) {
					if (HasInt(exif->olympus.MaxApertureAtMinFocal) && HasInt(exif->olympus.MaxApertureAtMaxFocal) &&
						HasInt(exif->olympus.MinFocalLength) && HasInt(exif->olympus.MaxFocalLength)) {
						float amin = powf(sqrtf(2.0F), (exif->olympus.MaxApertureAtMinFocal / 256.0F));
						float amax = powf(sqrtf(2.0F), (exif->olympus.MaxApertureAtMaxFocal / 256.0F));
						build_generic_lensinfo(wsz, exif->olympus.MinFocalLength, exif->olympus.MaxFocalLength, amin, amax);
					}
				}
				lstrcpyn(info->Lens, wsz, ARRAYSIZE(info->Lens));
			}
			// ホワイトバランス
			wsz[0] = 0;
			if (HasInt(exif->olympus.exWhiteBalance))
				_format_lookup(wsz, exif->olympus.exWhiteBalance, ekOlymWhiteBalance, -1);
			else if (HasInt(exif->olympus.WhiteBalanceMode[0])) {
				int val = ((exif->olympus.WhiteBalanceMode[0] & 0xff) << 8) | (exif->olympus.WhiteBalanceMode[1] & 0xff);
				_format_lookup(wsz, val, ekOlymWhiteBalanceMode, -1);
			}
			if (wsz[0])
				lstrcpyn(info->WhiteBalanceText, wsz, ARRAYSIZE(info->WhiteBalanceText));
			break;
		case MAKER_LEICA:
		case MAKER_PANASONIC:
			if (HasStr(exif->panasonic.LensName)) {
				_format_ascii(wsz, exif->panasonic.LensName, ARRAYSIZE(wsz));
				lstrcpyn(info->Lens, wsz, ARRAYSIZE(info->Lens));
			}
			// ホワイトバランス
			if (HasInt(exif->panasonic.WhiteBalance)) {
				_format_lookup(wsz, exif->panasonic.WhiteBalance, ekPanaWhiteBalance, -1);
				lstrcpyn(info->WhiteBalanceText, wsz, ARRAYSIZE(info->WhiteBalanceText));
			}
			break;
		case MAKER_SONY:
			if (HasInt(exif->sony.LensID)) {
				_format_lookup(wsz, exif->sony.LensID, (exif->sony.LensType == 0) ? ekSonyLensID : ekSonyLensID2, -1);
				lstrcpyn(info->Lens, wsz, ARRAYSIZE(info->Lens));
			}
			// ホワイトバランス
			if (HasInt(exif->sony.WhiteBalance)) {
				_format_lookup(wsz, exif->sony.WhiteBalance, ekSonyWhiteBalance, -1);
				lstrcpyn(info->WhiteBalanceText, wsz, ARRAYSIZE(info->WhiteBalanceText));
			}
			else if (HasInt(exif->sony.mltWhiteBalance)) {
				_format_lookup(wsz, exif->sony.mltWhiteBalance, ekSonyWhiteBalance, -1);
				lstrcpyn(info->WhiteBalanceText, wsz, ARRAYSIZE(info->WhiteBalanceText));
			}
			break;
		case MAKER_SIGMA:
			// ホワイトバランス
			if (HasStr(exif->sigma.WhiteBalanceMode)) {
				_format_ascii(wsz, exif->sigma.WhiteBalanceMode, ARRAYSIZE(wsz));
				lstrcpyn(info->WhiteBalanceText, wsz, ARRAYSIZE(info->WhiteBalanceText));
			}
			break;
		}
	}

	build_exif_infotip(exif, info);
	build_exif_label(info);
}

BOOL exfIsExifFile(LPCTSTR fname)
{
	BOOL bExif = FALSE;
	rawmap* fm = new rawmap();
	fm->m_pstrm = NULL;
	if (fm->open(fname)) {
		lstrcpy(fm->m_fname, fname);
		bExif = fm->isexif();
	}
	delete fm;
	return bExif;
}

//BOOL LoadPanasonicRaw(LPCTSTR fname, IStream* pstrm, BOOL bLargeImage, EXIFINFOSTRUCT* ei);
BOOL DCRaw(LPCTSTR fname, IStream* pstrm, BOOL bLargeImage, EXIFINFOSTRUCT* ei);

BOOL exfGetExifInfo(LPCTSTR fname, IStream* pstrm, int* pnOrientation, BOOL bLargeImage, EXIFINFOSTRUCT* info)
{
	BOOL bOK = FALSE;
	LPCTSTR ext = PathFindExtension(fname);
	if (pstrm) {
		if (!StrCmpI(ext, _T(".raw"))) {
			// 拡張子が ".raw" でイメージを取得したい（pstrm != NULL）場合
			// ".rw2" はファイル内にjpegイメージを持つので、ここでは何もしない。
			bOK = DCRaw(fname, pstrm, bLargeImage, info);
			if (bOK && pnOrientation && info)
				*pnOrientation = info->Orientation;
			return bOK;
		}

		if (bLargeImage) {
			if (!StrCmpI(ext, _T(".erf")) ||
				!StrCmpI(ext, _T(".3fr"))) {
				// 拡張子が ".erf" または ".3fr" で最大のイメージを取得したい（pstrm != NULL && bLargeImae）場合
				bOK = DCRaw(fname, pstrm, bLargeImage, info);
				if (bOK && pnOrientation && info)
					*pnOrientation = info->Orientation;
				return bOK;
			}
		}
	}

	exif_t* exif = new exif_t;
	bOK = exif_parse(fname, pstrm, exif, (bLargeImage != FALSE));
	if (bOK) {
		if (pnOrientation)
			*pnOrientation = exif->Orientation;
		if (info)
			get_exif_info(exif, info);
	}

	delete exif;
	return bOK;
}

BOOL exfGetThumbsnail(LPCTSTR fname, IStream* pstrm, int* pnOrientation, BOOL bLargeImage, LPTSTR tip, int cchMax)
{
	EXIFINFOSTRUCT* infoptr = NULL;
	if (tip && cchMax > 0) {
		long size = sizeof(EXIFINFOSTRUCT) + (1024 + 256) * sizeof(TCHAR);
		infoptr = (EXIFINFOSTRUCT*)malloc(size);
		if (infoptr) {
			memset(infoptr, 0, size);
			infoptr->pszTipText = (TCHAR*)((BYTE*)infoptr + sizeof(EXIFINFOSTRUCT));
			infoptr->cchTipText = 1024;
			infoptr->pszLabelText = infoptr->pszTipText + 1024;
			infoptr->cchLabelText = 256;
		}
	}

	BOOL bOK = exfGetExifInfo(fname, pstrm, pnOrientation, bLargeImage, infoptr);
	if (infoptr) {
		if (bOK) {
			lstrcpy(tip, infoptr->pszLabelText);
			int len = lstrlen(infoptr->pszLabelText) + 1;
			lstrcpy(tip + len, infoptr->pszTipText);
		}
		free(infoptr);
	}
	return bOK;
}

BOOL exfGetDateTimeString(LPCTSTR fname, LPTSTR date, int cchMax)
{
	if (!date || cchMax < 1) return FALSE;

	*date = 0;
	exif_t* exif = new exif_t;
	BOOL bOK = exif_parse(fname, NULL, exif, false);
	if (bOK && HasStr(exif->Date))
		_format_ascii(date, exif->Date, cchMax);
	delete exif;
	return bOK;
}

static int enuminfo_canon(canon_t* canon, int model_id, CExifCallback* pec)
{
	TCHAR buf[256];
	LPTSTR p;

	// 撮影モード
	if (HasInt(canon->ExposureProgram)) {
		_format_lookup(buf, canon->ExposureProgram, ekCanonExpProgram, -1);
		pec->AddItem(emShootMode, buf);
	}

	// 測光モード
	if (HasInt(canon->MeteringMode)) {
		_format_lookup(buf, canon->MeteringMode, ekCanonMeteringMode, -1);
		pec->AddItem(emMeteringMode, buf);
	}

	// AE設定
	if (HasInt(canon->AESetting)) {
		_format_lookup(buf, canon->AESetting, ekCanonAESetting, -1);
		pec->AddItem(_T("AE設定"), buf);
	}

	// イメージセレクト
	if (HasInt(canon->ShootMode)) {
		_format_lookup(buf, canon->ShootMode, ekCanonShootMode, -1);
		pec->AddItem(_T("イメージセレクト"), buf);
	}

	// ドライブモード
	if (HasInt(canon->DriveMode)) {
		_format_lookup(buf, canon->DriveMode, ekCanonDriveMode, -1);
		pec->AddItem(emDriveMode, buf);
	}

	// 連続撮影時の連番
	if (HasInt(canon->ContinuousNumber) && canon->ContinuousNumber > 0) {
		_format_int(buf, canon->ContinuousNumber);
		pec->AddItem(euContTakeNumber, buf);
	}

	// フォーカスモード
	if (HasInt(canon->FocusMode)) {
		_format_lookup(buf, canon->FocusMode, ekCanonFocusMode, -1);
		pec->AddItem(emFocusMode, buf);
	}

	// フォーカスレンジ
	if (HasInt(canon->FocusRange) && !IS_CANON_EOS(model_id)) {
		_format_lookup(buf, canon->FocusRange, ekCanonFocusRange, -1);
		pec->AddItem(_T("フォーカスレンジ"), buf);
	}

/* @@@ 表示は不要かな
	// フォーカス動作
	if (HasInt(canon->FocusContinuous)) {
		_format_lookup(buf, canon->FocusContinuous, ekCanonFocusContinuous, -1);
		pec->AddItem(_T("フォーカス動作"), buf);
	}
@@@ */

	// 手ブレ補正
	if (HasInt(canon->ImageStabilization)) {
		_format_lookup(buf, canon->ImageStabilization, ekCanonIS, -1);
		pec->AddItem(emAntiShake, buf);
	}

	// 記録画質
	if (HasInt(canon->Quality) && HasInt(canon->SizeType)) {
		if (canon->SizeType >= 0) {
			_format_lookup(buf, canon->SizeType, ekCanonImageSize, -1);
			p = buf + lstrlen(buf);
			*p++ = _T('/');
		}
		else
			p = buf;
		_format_lookup(p, canon->Quality, ekCanonQuality, -1);
		pec->AddItem(_T("記録画質"), buf);
	}

	// イメージサイズ
	if (HasInt(canon->ImageWidth) && HasInt(canon->ImageHeight)) {
		_format_xy(buf, canon->ImageWidth, canon->ImageHeight);
		pec->AddItem(emImageSize, buf);
	}

	// RAWサイズ
	if (HasInt(canon->RawWidth) && HasInt(canon->RawHeight)) {
		_format_xy(buf, canon->RawWidth, canon->RawHeight);
		pec->AddItem(_T("RAWイメージサイズ"), buf);
	}

	// 使用レンズ
	if (HasStr(canon->LensName)) {
		_format_ascii(buf, canon->LensName, ARRAYSIZE(buf));
		pec->AddItem(emLensName, buf);
	}
	else if (HasInt(canon->CiLensTypes)) {
		_format_lookup(buf, canon->CiLensTypes, ekCanonLensTypes, -1);
		pec->AddItem(emLensName, buf);
	}

	// レンズ情報
	if (HasInt(canon->MaxFocalLength) &&
		HasInt(canon->MinFocalLength) &&
		HasInt(canon->FocalLengthUnit)) {
		_format_float_int(buf, (float)canon->MinFocalLength / (float)canon->FocalLengthUnit);
		if (canon->MinFocalLength != canon->MaxFocalLength) {
			p = buf + lstrlen(buf);
			*p++ = '-';
			_format_float_int(p, (float)canon->MaxFocalLength / (float)canon->FocalLengthUnit);
		}
		_append_mm(buf);
		pec->AddItem(emLensInfo, buf);
	}

	// 最大絞り
	if (HasFlt(canon->MaxAperture)) {
		_format_Fnumber(buf, canon->MaxAperture);
		pec->AddItem(_T("最大絞り"), buf);
	}

	// 最小絞り
	if (HasFlt(canon->MinAperture)) {
		_format_Fnumber(buf, canon->MinAperture);
		pec->AddItem(_T("最小絞り"), buf);
	}

	// 焦点距離
	if (HasInt(canon->FocalLength)) {
		float f;
		if (IS_CANON_EOS(model_id))
			f = (float)canon->FocalLength;
		else if ((model_id == CANON_POWERSHOT_S3IS) ||
				 (model_id == CANON_POWERSHOT_S2IS))
			f = (float)canon->FocalLength / 100.0F;
		else if ((canon->FocalLength < 1000) || (model_id == CANON_POWERSHOT_PRO1))
			f = (float)canon->FocalLength * 3.125F / 100.0F;
		else
			f = (float)canon->FocalLength / 1000.0F;

		_format_float_int(buf, f);
		_append_mm(buf);
		pec->AddItem(emFocalLength, buf);
	}

	// マクロモード(EOSデジいち以外)
	if (HasInt(canon->MacroMode) && !IS_CANON_EOS(model_id)) {
		_format_lookup(buf, canon->MacroMode, ekCanonOnOff, -1);
		pec->AddItem(emMacroMode, buf);
	}

	// ホワイトバランス
	if (HasInt(canon->WhiteBalance)) {
		_format_lookup(buf, canon->WhiteBalance, ekCanonWhiteBalance, -1);
		pec->AddItem(emWhiteBalance, buf);
	}

	// ホワイトバランス補正
	if (HasInt(canon->WBBlueAmber) || HasInt(canon->WBMagentaGreen)) {
		p = buf;
		if (HasInt(canon->WBBlueAmber) && (canon->WBBlueAmber != 0))
			p += wsprintf(p, _T("%c%d"), (canon->WBBlueAmber > 0) ? _T('A') : _T('B'), abs(canon->WBBlueAmber));
		if (HasInt(canon->WBMagentaGreen) && (canon->WBMagentaGreen != 0)) {
			if (p > buf) *p++ = _T('/');
			p += wsprintf(p, _T("%c%d"), (canon->WBMagentaGreen > 0) ? _T('G') : _T('M'), abs(canon->WBMagentaGreen));
		}

		if (p == buf)
			lstrcpy(buf, euNone);
		pec->AddItem(emWhiteBalanceComp, buf);
	}

	// ホワイトバランスブラケティング
	if (HasInt(canon->FiWBBracketMode)) {
		_format_lookup(buf, canon->FiWBBracketMode, ekCanonWBBracketMode, -1);
		p = buf + lstrlen(buf);
		// ABシフト
		if (HasInt(canon->FiWBBracketBlueAmber) && (canon->FiWBBracketBlueAmber != 0))
			p += wsprintf(p, _T("%c%d"), (canon->FiWBBracketBlueAmber > 0) ? _T('A') : _T('B'), abs(canon->FiWBBracketBlueAmber));
		if (HasInt(canon->FiWBBracketMagentaGreen) && (canon->FiWBBracketMagentaGreen != 0)) {
			if (p > buf) *p++ = _T('/');
			p += wsprintf(p, _T("%c%d"), (canon->FiWBBracketMagentaGreen > 0) ? _T('G') : _T('M'), abs(canon->FiWBBracketMagentaGreen));
		}
		pec->AddItem(_T("WBブラケティング"), buf);
	}

	// 色温度
	if (HasInt(canon->ColorTemp)) {
		_format_int(buf, canon->ColorTemp, _T("K"));
		pec->AddItem(emColorTemp, buf);
	}

	// ISO
	if (HasInt(canon->ISOapex)) {
		_format_int(buf, canon->ISOapex);
		if (HasInt(canon->ISO)) {
			switch (canon->ISO) {
			case 14: lstrcat(buf, _T("(HI)")); break;		// ISO(HI)
			case 15: lstrcat(buf, _T("(AUTO)")); break;		// ISO(AUTO)
			}
		}
		pec->AddItem(emISO, buf);
	}

	// 絞り値
	if (HasFlt(canon->Aperture)) {
		_format_Fnumber(buf, canon->Aperture);
		pec->AddItem(emAperture, buf);
	}

	// 露出時間
	if (HasFlt(canon->ExposureTime)) {
		if (ISFLOAT_ZERO(canon->ExposureTime)) {
			lstrcpy(buf, euBulb);
			if (HasInt(canon->BulbDuration) && canon->BulbDuration != 0) {
				p = buf + lstrlen(buf);
				*p++ = _T(' ');
				_format_float(p, (float)canon->ExposureTime / 10.0F, _T("秒"));
			}
		}
		else
			_format_shutter(buf, canon->ExposureTime);
		pec->AddItem(emExposureTime, buf);
	}

	// 露出補正
	if (HasInt(canon->ExposureComp)) {
		_format_ev_comp(buf, _canon_ev_conv(canon->ExposureComp));
		pec->AddItem(emExpComp, buf);
	}

	// ブラケットモード
	if (HasInt(canon->FiBracketMode)) {
		_format_lookup(buf, canon->FiBracketMode, ekCanonBracketMode, -1);
		pec->AddItem(_T("ブラケットモード"), buf);
	}

	// AEB
	if (HasInt(canon->AEBSeq)) {
		_format_lookup(buf, canon->AEBSeq, ekCanonAEBSeq, -1);
		pec->AddItem(_T("AEB"), buf);
	}

	// AEB補正
	if (HasInt(canon->AEBComp)) {
		_format_ev_comp(buf, _canon_ev_conv(canon->AEBComp));
		pec->AddItem(_T("AEB補正"), buf);
	}

/* @@@ -- 非表示でかまわない
	// フラッシュ状態
	if (HasInt(canon->FlashCondition)) {
		_format_lookup(buf, canon->FlashCondition, ekCanonFlashCondition, -1);
		pec->AddItem(_T("フラッシュ状態"), buf);
	}
@@@ */

	// フラッシュモード
	if (HasInt(canon->FlashMode)) {
		_format_lookup(buf, canon->FlashMode, ekCanonFlashMode, -1);
		pec->AddItem(emFlashMode, buf);
	}

	// フラッシュガイドナンバー
	if (HasInt(canon->FlashGuideNumber)) {
		_format_int(buf, canon->FlashGuideNumber / 32);
		pec->AddItem(_T("ガイドナンバー"), buf);
	}

	// ストロボ調光補正
	if (HasInt(canon->FlashComp)) {
		_format_ev_comp(buf, _canon_ev_conv(canon->FlashComp));
		pec->AddItem(emStrobeComp, buf);
	}

	// 被写体距離
	if (HasInt(canon->TargetLongestDistance) && canon->TargetLongestDistance > 0) {
		if (HasInt(canon->TargetShortestDistance) && canon->TargetShortestDistance != 0) {
			if (canon->TargetLongestDistance == USHRT_MAX) {
				_format_distance(buf, canon->TargetShortestDistance * 10);
				lstrcat(buf, _T(" - ∞"));
			}
			else {
				_format_distance(buf, canon->TargetShortestDistance * 10);
				lstrcat(buf, _T(" - "));
				p = buf + lstrlen(buf);
				_format_distance(p, canon->TargetLongestDistance * 10);
			}
			pec->AddItem(_T("被写体距離範囲"), buf);
		}
		else {
			_format_distance(buf, canon->TargetLongestDistance * 10);
			pec->AddItem(emTargetDistance, buf);
		}
	}

	// マイカラー
	if (HasInt(canon->MyColor)) {
		_format_lookup(buf, canon->MyColor, ekCanonMyColor, -1);

		if (canon->MyColor == 7 || canon->MyColor == 8) {
			// ワンポイントカラー or スイッチカラー
			if (canon->ColorRange) {
				p = buf + lstrlen(buf);
				*p++ = _T('（');
				_format_int_comp(p, canon->ColorRange);
				p += lstrlen(p);
				*p++ = _T('）');
				*p = 0;
			}
		}

		pec->AddItem(_T("マイカラー"), buf);

		if (canon->MyColor != 0) {
			if (HasInt(canon->RedLevel) &&
				HasInt(canon->GreenLevel) &&
				HasInt(canon->BlueLevel) &&
				HasInt(canon->SkinLevel)) {

				TCHAR tmp[128];
				// 赤レベル調整
				_format_int_comp(tmp,      canon->RedLevel - 2);
				// 緑レベル調整
				_format_int_comp(tmp + 32, canon->GreenLevel - 2);
				// 青レベル調整
				_format_int_comp(tmp + 64, canon->BlueLevel - 2);
				// 肌色レベル調整
				_format_int_comp(tmp + 96, canon->SkinLevel - 2);
				wsprintf(buf, _T("赤%s/緑%s/青%s/肌色%s"), tmp, tmp + 32, tmp + 64, tmp + 96);
				pec->AddItem(_T("マイカラー色調整"), buf);
			}
		}
	}

	// レタッチマイカラー
	if (HasInt(canon->RetouchMyColor) && (canon->RetouchMyColor > 0)) {
		_format_lookup(buf, canon->RetouchMyColor, ekCanonMyColor, -1);
		pec->AddItem(_T("レタッチマイカラー"), buf);
	}

	// カラーマトリクス or ピクチャースタイル
	if (HasInt(canon->ColorMatrix)) {
		_format_lookup(buf, canon->ColorMatrix, ekCanonColorMatrix, -1);
		if (IS_CANON_PICTURESTYLE(model_id))
			p = _T("ピクチャースタイル");
		else
			p = _T("カラーマトリックス");
		pec->AddItem(p, buf);
	}

	// オートライティングオプティマイザ
	if (HasInt(canon->CiAutoLightingOptimizer)) {
		const lkup ekCanonAutoLightingOptimizer[] = {
			0,		euHyojun,
			1,		euWeak,
			2,		euStrong,
			3,		euOff,
			INT_MAX,NULL
		};
		_format_lookup(buf, canon->CiAutoLightingOptimizer, ekCanonAutoLightingOptimizer, -1);
		pec->AddItem(_T("オートライティングオプティマイザ"), buf);
	}

	// 高輝度側・階調優先
	if (HasInt(canon->CiHighISONoiseReduction)) {
		const lkup ekCanonHighlightTonePriority[] = {
			0,		euOff,
			1,		euOn,
			INT_MAX,NULL
		};
		_format_lookup(buf, canon->CiHighlightTonePriority, ekCanonHighlightTonePriority, -1);
		pec->AddItem(_T("高輝度側・階調優先"), buf);
	}

	// 高感度時ノイズ低減
	if (HasInt(canon->CiHighISONoiseReduction)) {
		const lkup ekCanonHighISONoiseReduction[] = {
			0,		euHyojun,
			1,		euWeak,
			2,		euStrong,
			3,		euOff,
			INT_MAX,NULL
		};
		_format_lookup(buf, canon->CiHighISONoiseReduction, ekCanonHighISONoiseReduction, -1);
		pec->AddItem(_T("高感度時ノイズ低減"), buf);
	}

	// 現像パラメーター(ピクチャースタイルをサポートしていない機種のみ)
	if (HasInt(canon->DevParam) && !IS_CANON_PICTURESTYLE(model_id)) {
		_format_lookup(buf, canon->DevParam, ekCanonDevParam, -1);
		pec->AddItem(_T("現像パラメーター"), buf);
	}

	// トーンカーブ
	if (HasInt(canon->ToneCurve)) {
		_format_lookup(buf, canon->ToneCurve, ekCanonToneCurve, -1);
		pec->AddItem(_T("トーンカーブ"), buf);
	}

	// シャープネス調整
	if (HasInt(canon->AdjustSharpness)) {
		_format_int_comp(buf, canon->AdjustSharpness);
		pec->AddItem(emSharpness, buf);
	}
	else if (HasInt(canon->SharpnessPower)) {
		// シャープネスの強さ
		_format_int_comp(buf, canon->SharpnessPower);
		pec->AddItem(emSharpness, buf);
	}

	// コントラスト調整
	if (HasInt(canon->AdjustContrast)) {
		_format_int_comp(buf, canon->AdjustContrast);
		pec->AddItem(emContrast, buf);
	}

	// 彩度調整
	if (HasInt(canon->AdjustSaturation)) {
		_format_int_comp(buf, canon->AdjustSaturation);
		pec->AddItem(emSaturation, buf);
	}

	// 色効果
	if (HasInt(canon->ColorEffect)) {
		_format_lookup(buf, canon->ColorEffect, ekCanonColorEffect, -1);
		pec->AddItem(_T("色効果"), buf);
	}

	// 色あい調整
	if (HasInt(canon->AdjustColorTone)) {
		_format_int_comp(buf, canon->AdjustColorTone);
		pec->AddItem(emColorTone, buf);
	}

	// NDフィルター
	if (HasInt(canon->NDFilter)) {
		_format_lookup(buf, canon->NDFilter, ekOffOn, -1);
		pec->AddItem(_T("NDフィルター"), buf);
	}

	// BWフィルター効果
	if (HasInt(canon->FiBWFilterEffect) && canon->FiBWFilterEffect > 0) {
		_format_lookup(buf, canon->FiBWFilterEffect, ekCanonBWFilterEffect, -1);
		pec->AddItem(emFilterEffect, buf);
	}

	// BW調色
	if (HasInt(canon->FiBWColorFilter) && canon->FiBWColorFilter > 0) {
		_format_lookup(buf, canon->FiBWColorFilter, ekCanonBWColorFilter, -1);
		pec->AddItem(_T("調色"), buf);
	}

	// デジタルズーム
	if (HasInt(canon->DigitalZoom) && !IS_CANON_EOS(model_id)) {
		_format_lookup(buf, canon->DigitalZoom, ekCanonDigitalZoom, 5);
		pec->AddItem(emDigitalZoom, buf);
	}

	// AFポイント
	if (HasInt(canon->AFPoint) && canon->AFPoint != 0) {
		_format_lookup(buf, canon->AFPoint, ekCanonAFPoint, -1);
		pec->AddItem(_T("AFポイント"), buf);
	}

	// 合焦AFフレーム
	if (HasInt(canon->AFPointUsed) &&
		(canon->AFPointUsed != 0) &&
		(canon->FocusMode != 3) &&
		(canon->FocusMode != 6)) {
		switch (model_id) {
		case CANON_EOS_5D:
			p = buf;
			if (canon->AFPointUsed & 0x7e00) {
				_format_lookup(buf, canon->AFPointUsed & 0x7e00, ekCanonAIServo5D, -1);
				p = buf + lstrlen(buf);
				*p++ = _T(',');
			}
			_format_bitmask_lookup(p, canon->AFPointUsed & 0x1ff, ekCanonAFPointUsed5D, -1);
			break;
		case CANON_EOS_20D:
		case CANON_EOS_30D:
		case CANON_EOS_40D:
		case CANON_EOS_KISSX:
		case CANON_EOS_KISSX2:
		case CANON_EOS_KISSF:
			_format_bitmask_lookup(buf, canon->AFPointUsed, ekCanonAFPointUsed20D, -1);
			break;
		default:
			if (IS_CANON_EOS(model_id))
				_format_bitmask_lookup(buf, canon->AFPointUsed, ekCanonAFPointUsed, -1);
			else
				_format_lookup(buf, canon->AFPointUsed, ekCanonAFPointUsedPS, -1);
			break;
		}
		pec->AddItem(_T("合焦AFフレーム"), buf);
	}

	// セルフタイマ
	if (HasInt(canon->SelfTimer) && canon->SelfTimer > 0) {
		_format_float_int(buf, (float)canon->SelfTimer / 10.0F, _T("秒"));
		pec->AddItem(euSelfTimer, buf);
	}

	// パノラマ連結方向
	if (HasInt(canon->PaPanoramaDirection)) {
		_format_lookup(buf, canon->PaPanoramaDirection, ekCanonPanoramaDirection, -1);
		pec->AddItem(_T("パノラマ連結"), buf);
	}

	// 回転
	if (HasInt(canon->AutoRotate)) {
		_format_lookup(buf, canon->AutoRotate, ekCanonAutoRotate, -1);
		pec->AddItem(_T("自動回転"), buf);
	}

	// イメージの種類
	if (HasStr(canon->ImageType)) {
		_format_ascii(buf, canon->ImageType, ARRAYSIZE(buf));
		pec->AddItem(_T("イメージの種類"), buf);
	}

	// ファイル番号
	if (HasInt(canon->ShotNumber) && canon->ShotNumber > 10000) {
		wsprintf(buf, _T("%d-%04d"), canon->ShotNumber / 10000, canon->ShotNumber % 10000);
		pec->AddItem(_T("ファイル番号"), buf);
	}

	// シャッター回数
	if (HasInt(canon->FiShutterCount)) {
		if (IS_CANON_SHUTTERCOUNT(model_id) &&
			((model_id != CANON_EOS_30D) &&
			 (model_id != CANON_EOS_40D) &&
			 (model_id != CANON_EOS_KISSX) &&
			 (model_id != CANON_EOS_KISSX2) &&
			 (model_id != CANON_EOS_KISSF))) {
			_format_int(buf, canon->FiShutterCount);
			pec->AddItem(emShutterCount, buf);
		}
		else {
			wsprintf(buf, _T("%d-%d"), canon->FiShutterCount / 10000, canon->FiShutterCount % 10000);
			pec->AddItem(_T("ファイル番号"), buf);
		}
	}

	// カメラタイプ
	if (HasInt(canon->CameraType)) {
		_format_lookup(buf, canon->CameraType, ekCanonCameraType, -1);
		pec->AddItem(_T("カメラタイプ"), buf);
	}

	// ファームウェア
	if (HasStr(canon->FirmWare)) {
		_format_ascii(buf, canon->FirmWare, ARRAYSIZE(buf));
		pec->AddItem(emFirmware, buf);
	}

	// 所有者名
	if (HasStr(canon->OwnerName)) {
		_format_ascii(buf, canon->OwnerName, ARRAYSIZE(buf));
		pec->AddItem(_T("所有者名"), buf);
	}

	// シリアル番号
	if (HasInt(canon->SerialNumber)) {
		ULONG sn = (ULONG)canon->SerialNumber;
		if ((sn >> 16) != 0x7fff) {
			if (IS_CANON_EOS(model_id))
				wsprintf(buf, _T("%010u"), sn);
			else
				wsprintf(buf, _T("%x-%.5d"), sn >> 16, sn & 0xffff);
			pec->AddItem(emSerialNumber, buf);
		}
	}

	return 1;
}

static int enuminfo_nikon(nikon_t* nikon, int model_id, CExifCallback* pec)
{
	TCHAR buf[256];

	// ISO
	if (HasInt(nikon->ISO)) {
		if (nikon->ISO)
			_format_int(buf, nikon->ISO);
		else
			lstrcpy(buf, euUnsupp);
		if (nikon->ISOinc) lstrcat(buf, _T("(+)"));
		pec->AddItem(emISO, buf);
	}

	// ISOモード
	if (HasStr(nikon->ISOMode)) {
		_format_ascii(buf, nikon->ISOMode, ARRAYSIZE(buf));
		pec->AddItem(_T("ISOモード"), buf);
	}

	// カラーモード
	if (HasStr(nikon->ColorMode)) {
		_format_ascii(buf, nikon->ColorMode, ARRAYSIZE(buf));
		pec->AddItem(emColorMode, buf);
	}

	// クォリティ
	if (HasStr(nikon->QualityMode)) {
		_format_ascii(buf, nikon->QualityMode, ARRAYSIZE(buf));
		pec->AddItem(emQualityMode, buf);
	}

	// ホワイトバランス
	if (HasStr(nikon->WhiteBalance)) {
		_format_string_lookup(buf, nikon->WhiteBalance, etNikonWhiteBalance);
		pec->AddItem(emWhiteBalance, buf);
	}

	// ホワイトバランスの微調整
	if (HasInt(nikon->WhiteBalanceFine)) {
		_format_int_comp(buf, nikon->WhiteBalanceFine);
		pec->AddItem(_T("ホワイトバランスの微調整"), buf);
	}

	// 仕上がり設定
	if (HasStr(nikon->ResultCondition)) {
		_format_string_lookup(buf, nikon->ResultCondition, etNikonResultCondition);
		// 白黒カスタマイズ（フィルター効果）
		if (HasStr(nikon->BWFilterEffect) && nikon->BWFilterEffect[0] != ' ') {
			TCHAR tmp[128];
			_format_ascii(tmp, nikon->BWFilterEffect, ARRAYSIZE(tmp));
			wsprintf(&buf[lstrlen(buf)], _T(" - %s"), tmp);
		}
		pec->AddItem(emResultCondition, buf);
	}

	// フィルター効果
	if (HasInt(nikon->FilterEffect)) {
		_format_lookup(buf, nikon->FilterEffect, ekNikonFilterEffect, -1);
		pec->AddItem(emFilterEffect, buf);
	}

	// D-ライティング
	if (HasStr(nikon->DLighting)) {
		_format_ascii(buf, nikon->DLighting, ARRAYSIZE(buf));
		 pec->AddItem(_T("D-ライティング"), buf);
	}

	// アクティブD-ライティング
	if (HasInt(nikon->ActiveDLighting)) {
		_format_lookup(buf, nikon->ActiveDLighting, ekNikonActiveDLighting, -1);
		 pec->AddItem(_T("アクティブD-ライティング"), buf);
	}

	// 輪郭の強調
	if (HasStr(nikon->EdgeMode)) {
		_format_string_lookup(buf, nikon->EdgeMode, etNikonEdgeMode);
		pec->AddItem(_T("輪郭の強調"), buf);
	}

	// 階調補正
	if (HasStr(nikon->ToneAdjust)) {
		_format_string_lookup(buf, nikon->ToneAdjust, etNikonToneAdjust);
		pec->AddItem(_T("階調補正"), buf);
	}

	// 彩度設定
	if (HasStr(nikon->ColorEnhance)) {
		_format_string_lookup(buf, nikon->ColorEnhance, etNikonColorEnhance);
		pec->AddItem(_T("彩度設定"), buf);
	}

	// 色合い調整
	if (HasInt(nikon->HueAdjustment)) {
		_format_int_comp(buf, nikon->HueAdjustment, _T("°"));
		pec->AddItem(_T("色合い調整"), buf);
	}

	// フォーカスモード
	if (HasStr(nikon->FocusMode)) {
		_format_ascii(buf, nikon->FocusMode, ARRAYSIZE(buf));
		pec->AddItem(emFocusMode, buf);
	}

	// 撮影動作モード
	if (HasInt(nikon->DriveMode)) {
//@		_format_lookup(buf, nikon->DriveMode, ekNikonDriveMode, -1);
		pec->AddItem(_T("撮影動作モード"), (nikon->DriveMode & 0x01) ? euContinuosShot : euSingleShot);	// bit 0
		if (nikon->DriveMode & 0x02) pec->AddSubItem(NULL, euSelfTimer);								// bit 1
		if (nikon->DriveMode & 0x04) pec->AddSubItem(NULL, _T("PCコントロール"));						// bit 2
		if (nikon->DriveMode & 0x08) pec->AddSubItem(NULL, _T("Flashブラケティング"));					// bit 3
		if (nikon->DriveMode & 0x10) pec->AddSubItem(NULL, _T("AEブラケティング"));						// bit 4
		if (nikon->DriveMode & 0x20) pec->AddSubItem(NULL, _T("感度オートコントロール"));				// bit 5
		if (nikon->DriveMode & 0x40) pec->AddSubItem(NULL, _T("WBブラケティング"));						// bit 6
		if (nikon->DriveMode & 0x80) pec->AddSubItem(NULL, _T("IRコントロール"));						// bit 7
	}

	// ピクチャーコントロール
	if (HasStr(nikon->PictureControlInfo)) {
		// offset 04 - 23: PictureControlName
		const lkup_text etNikonPictureControlName[] = {
			"STANDARD",		euStandard,
			"NEUTRAL",		euNeutral,
			"VIVID",		euVivid,
			"MONOCHROME",	euMonochrome,
			"PORTRAIT",		euPortrait,
			"LANDSCAPE",	euLandscape,
			NULL,			NULL,
		};
		_format_string_lookup(buf, (char*)&nikon->PictureControlInfo[4], etNikonPictureControlName);
		pec->AddItem(_T("ピクチャーコントロール"), buf);

		// offset 24 - 43: PictureControlBase
		_format_string_lookup(buf, (char*)&nikon->PictureControlInfo[24], etNikonPictureControlName);
		pec->AddSubItem(_T("ベース名"), buf);

		// offset 48: PictureControlAdjust
		const lkup ekNikonPictureControlAdjust[] = {
			0,		_T("標準設定"),
			1,		_T("クイック調整"),
			2,		_T("手動調整"),
			INT_MAX,NULL
		};
		_format_lookup(buf, nikon->PictureControlInfo[48], ekNikonPictureControlAdjust, -1);
		pec->AddSubItem(_T("ピクチャーコントロール調整"), buf);

		// offset 49: PictureControlQuickAdjust -- settings: -2 to +2 (when PictureControlAdjust == QuickAdjust)
		if (nikon->PictureControlInfo[48] == 1) {
			// クイック調整
			if (nikon->PictureControlInfo[49] == 0xff)
				lstrcpy(buf, euUndefine);
			else
				_format_int_comp(buf, (int)nikon->PictureControlInfo[49] - 0x80);
			pec->AddSubItem(_T("クイック調整"), buf);
		}

		if (nikon->PictureControlInfo[48] == 2) {
			// 手動調整
			// offset 50: Sharpness -- settings: 0 to 9, Auto
			if (nikon->PictureControlInfo[50] == 0x00)
				lstrcpy(buf, euAuto);
			else
				_format_int_comp(buf, (int)nikon->PictureControlInfo[50] - 0x80);
			pec->AddSubItem(_T("輪郭強調"), buf);

			// offset 51: Contrast -- settings: -3 to +3, Auto
			if (nikon->PictureControlInfo[51] == 0x00)
				lstrcpy(buf, euAuto);
			else
				_format_int_comp(buf, (int)nikon->PictureControlInfo[51] - 0x80);
			pec->AddSubItem(emContrast, buf);

			// offset 52: Brightness -- settings: -1 to +1
			_format_int_comp(buf, (int)nikon->PictureControlInfo[52] - 0x80);
			pec->AddSubItem(_T("明るさ"), buf);

			// offset 53: Saturation -- settings: -3 to +3, Auto
			if (nikon->PictureControlInfo[53] == 0x00)
				lstrcpy(buf, euAuto);
			else if (nikon->PictureControlInfo[53] == 0xff)
				lstrcpy(buf, euUndefine);
			else
				_format_int_comp(buf, (int)nikon->PictureControlInfo[53] - 0x80);
			pec->AddSubItem(_T("色の濃さ（彩度）"), buf);

			// offset 54: HueAdjustment -- settings: -3 to +3
			if (nikon->PictureControlInfo[54] == 0xff)
				lstrcpy(buf, euUndefine);
			else
				_format_int_comp(buf, (int)nikon->PictureControlInfo[54] - 0x80);
			pec->AddSubItem(_T("色合い（色相）"), buf);

			// offset 55: FilterEffect -- settings: Off,Yellow,Orange,Red,Green
			const lkup ekNikonPictureControlFilterEffect[] = {
				0x80,	euOff,
				0x81,	_T("黄"),
				0x82,	_T("オレンジ"),
				0x83,	_T("赤"),
				0x84,	_T("緑"),
				0xff,	_T("N/A"),
				INT_MAX,NULL
			};
			_format_lookup(buf, nikon->PictureControlInfo[55], ekNikonPictureControlFilterEffect, -1);
			pec->AddSubItem(emFilterEffect, buf);

			// offset 56: ToningEffect
			const lkup ekNikonPictureControlToningEffect[] = {
				0x80,	_T("B&W"),
				0x81,	_T("Sepia"),
				0x82,	_T("Cyanotype"),
				0x83,	_T("Red"),
				0x84,	_T("Yellow"),
				0x85,	_T("Green"),
				0x86,	_T("Blue Green"),
				0x87,	_T("Blue"),
				0x88,	_T("Purple Blue"),
				0x89,	_T("Red Purple"),
				0xff,	_T("N/A"),
				INT_MAX,NULL
			};
			_format_lookup(buf, nikon->PictureControlInfo[56], ekNikonPictureControlToningEffect, -1);
			pec->AddSubItem(_T("調色"), buf);

			// offset 57: ToningSaturation
			const lkup ekNikonPictureControlToningSaturation[] = {
				0x81,	_T("1"),
				0x82,	_T("2"),
				0x83,	_T("3"),
				0x84,	_T("4"),
				0x85,	_T("5"),
				0x86,	_T("6"),
				0x87,	_T("7"),
				0xff,	_T("N/A"),
				INT_MAX,NULL
			};
			_format_lookup(buf, nikon->PictureControlInfo[57], ekNikonPictureControlToningSaturation, -1);
			pec->AddSubItem(_T("色の濃淡"), buf);
		}
	}

	// ISO情報
	if (HasStr(nikon->ISOInfo)) {
		int val = (int)floorf(100.0F * expf(((float)(int)nikon->ISOInfo[0] / 12.0F - 5.0F) * logf(2.0)));
		_format_int(buf, val);
		pec->AddItem(emISO, buf);

		// offset 04 - 05: ISOExpansion
		const lkup ekNikonISOExpansion[] = {
			0x000,	euNone,
			0x101,	_T("Hi 0.3"),
			0x102,	_T("Hi 0.5"),
			0x103,	_T("Hi 0.7"),
			0x104,	_T("Hi 1.0"),
			0x105,	_T("Hi 1.3"),		// (Hi 1.3-1.7 may be possible with future models
			0x106,	_T("Hi 1.5"),
			0x107,	_T("Hi 1.7"),
			0x108,	_T("Hi 2.0"),			// (NC) - D3 should have this mode
			0x201,	_T("Lo 0.3"),
			0x202,	_T("Lo 0.5"),
			0x203,	_T("Lo 0.7"),
			0x204,	_T("Lo 1.0"),
			INT_MAX,NULL
		};

		_format_lookup(buf, *(WORD*)&nikon->ISOInfo[4], ekNikonISOExpansion, -1);
		pec->AddSubItem(_T("ISO感度増減"), buf);
	}

	// ヴィネットコントロール
	if (HasInt(nikon->VinetControl)) {
		const lkup ekNikonVinetControl[] = {
			0,		euNone,
			1,		_T("弱め（30%）"),
			3,		_T("標準（30%）"),
			5,		_T("強め（70%）"),
			INT_MAX,NULL
		};
		_format_lookup(buf, nikon->VinetControl, ekNikonVinetControl, -1);
		pec->AddItem(_T("ヴィネットコントロール"), buf);
	}

	// オートブラケットレリーズ
	if (HasInt(nikon->AutoBracketRelease)) {
		_format_lookup(buf, nikon->AutoBracketRelease, ekNikonAutoBracketRelease, -1);
		pec->AddItem(_T("オートブラケットレリーズ"), buf);
	}

	// プログラムシフト
	if (HasInt(nikon->ProgramShift)) {
		_format_ev_comp(buf, nikon_UnpackC3(nikon->ProgramShift));
		 pec->AddItem(_T("プログラムシフト補正"), buf);
	}

	// 露出差分
	if (HasInt(nikon->ExposureDifference)) {
		_format_ev_comp(buf, nikon_UnpackC3(nikon->ExposureDifference));
		 pec->AddItem(_T("露出差分"), buf);
	}

	// ブラケットステップ
	if (HasFlt(nikon->ExposureBracket)) {
		_format_ev_comp(buf, nikon->ExposureBracket);
		pec->AddItem(emBracketStep, buf);
	}

	// フラッシュの利用
	if (HasInt(nikon->FlashUsed)) {
		_format_lookup(buf, nikon->FlashUsed, ekNikonFlashUsed, -1);
		pec->AddItem(_T("フラッシュの利用"), buf);

		if (nikon->FlashUsed != 0) {
			// フラッシュシンクロモード
			if (HasStr(nikon->FlashSetting)) {
				_format_string_lookup(buf, nikon->FlashSetting, etNikonFlashSetting);
				pec->AddItem(_T("フラッシュシンクロモード"), buf);
			}

/* @@@
			// フラッシュ設定2
			if (HasStr(nikon->FlashSetting2)) {
				_format_ascii(buf, nikon->FlashSetting2, ARRAYSIZE(buf));
				pec->AddItem(emFlashSetting, buf);
			}
@@@ */

			// フラッシュモード
			if (HasStr(nikon->FlashMode)) {
				_format_ascii(buf, nikon->FlashMode, ARRAYSIZE(buf));
				pec->AddItem(emFlashMode, buf);
			}

			// 調光補正
			if (HasInt(nikon->FlashComp)) {
				_format_ev_comp(buf, nikon_UnpackC3(nikon->FlashComp));
				pec->AddItem(emStrobeComp, buf);
			}

			// フラッシュブラケット
			if (HasInt(nikon->FlashExposureBracket)) {
				_format_float_comp(buf, nikon_UnpackC3(nikon->FlashExposureBracket));
				pec->AddItem(_T("フラッシュブラケットステップ"), buf);
			}
		}
	}

	// 画像モード
	if (HasStr(nikon->ImageMode)) {
		_format_ascii(buf, nikon->ImageMode, ARRAYSIZE(buf));
		pec->AddItem(_T("画像モード"), buf);
	}

/* ------ 不確定 ---------
	// AFAperture
	if (HasInt(nikon->AFAperture)) {
		float f = nikon_ApertureConversions(nikon->AFAperture);
		_format_float(buf, f);
		pec->AddItem(_T("AFAperture"), buf);
	}

	// FocalLength
	if (HasInt(nikon->FocalLength)) {
		float f = nikon_FocalConversions(nikon->FocalLength);
		_format_float(buf, f);
		pec->AddItem(_T("FocalLength"), buf);
	}
-------------------------- */

	// レンズコンバータ
	if (HasStr(nikon->LensConverter)) {
		_format_ascii(buf, nikon->LensConverter, ARRAYSIZE(buf));
		pec->AddItem(_T("レンズコンバータ"), buf);
	}

	// レンズID
	if (HasStr(nikon->LensData)) {
		_format_string_lookup(buf, nikon->LensData, ekNikonLensData);
		pec->AddItem(emLensName, buf);
	}
	else if (HasInt(nikon->LensID)) {
		_format_lookup(buf, nikon->LensID, ekNikonLensID, -1);
		pec->AddItem(emLensName, buf);
	}
	else if (HasStr(nikon->LensModel)) {
		_format_ascii(buf, nikon->LensModel, ARRAYSIZE(buf));
		pec->AddItem(emLensName, buf);
	}

	// レンズタイプ
	if (HasInt(nikon->LensType) && HasFlt(nikon->LensInfo[0])) {
		build_nikon_lensinfo(buf, nikon->LensType, nikon->LensInfo);
		pec->AddItem(emLensInfo, buf);
	}

/* ---- データが正常でない -------
	// フォーカス距離
	if (HasInt(nikon->FocusDistance)) {
		float f = 0.01F * powf(10.0F, (float)nikon->FocusDistance / 40.0F);	// m
		_format_float2(buf, f, _T("m"));
		pec->AddItem(emFocusDistance, buf);
	}
--------------------------------- */

	// 色空間
	buf[0] = 0;
	if (HasStr(nikon->ColorSpaceName))
		_format_ascii(buf, nikon->ColorSpaceName, ARRAYSIZE(buf));
	else if (HasInt(nikon->ColorSpace))
		_format_lookup(buf, nikon->ColorSpace, ekNikonColorSpace, -1);
	if (buf[0])
		pec->AddItem(_T("色空間"), buf);

	// マニュアルフォーカス距離
	if (HasFlt(nikon->ManualFocusDistance) && nikon->ManualFocusDistance > 0.0F) {
		_format_float2(buf, nikon->ManualFocusDistance, _T("m"));
		pec->AddItem(_T("マニュアルフォーカス距離"), buf);
	}

	// デジタルズーム
	if (HasFlt(nikon->DigitalZoom)) {
		_format_scale(buf, nikon->DigitalZoom);
		pec->AddItem(emDigitalZoom, buf);
	}

	// フォーカスエリア
	if (HasByte(nikon->AFInfo[0])) {
		// AFエリアモード
		_format_lookup(buf, nikon->AFInfo[0], ekNikonAFAreaMode, -1);
		pec->AddItem(_T("AFエリアモード"), buf);

		// フォーカスエリア
		if (IS_NIKON_HAS_AFAREA_11POINTS(model_id))
			_format_lookup(buf, nikon->AFInfo[1], ekNikonFocusArea11P, -1);
		else
			_format_lookup(buf, nikon->AFInfo[1], ekNikonFocusArea5P, -1);
		pec->AddSubItem(_T("フォーカスエリア"), buf);

		// 合焦AFポイント
		int bit = (nikon->AFInfo[2] << 8) + nikon->AFInfo[3];
		if (bit) {
			_format_bitmask_lookup(buf, bit, ekNikonAFPointUsed, -1);
			pec->AddSubItem(_T("合焦AFポイント"), buf);
		}
	}

	// シーンモード
	if (HasStr(nikon->SceneMode)) {
//@		_format_ascii(buf, nikon->SceneMode, ARRAYSIZE(buf));
		_format_string_lookup(buf, nikon->SceneMode, etNikonSceneMode);
		pec->AddItem(emSceneMode, buf);
	}

	// アシスト機能付きシーンモード
	if (HasStr(nikon->AssSceneMode)) {
//@		_format_ascii(buf, nikon->AssSceneMode, ARRAYSIZE(buf));
		_format_string_lookup(buf, nikon->AssSceneMode, etNikonSpecialEffects);
		pec->AddItem(_T("アシスト機能付きシーンモード"), buf);
	}

	// ライトタイプ
	if (HasStr(nikon->LightType)) {
		_format_ascii(buf, nikon->LightType, ARRAYSIZE(buf));
		pec->AddItem(emLightSource, buf);
	}

	// 彩度補正
	if (HasInt(nikon->SaturationComp)) {
		_format_int_comp(buf, nikon->SaturationComp);
		pec->AddItem(emSaturation, buf);
	}

	// ノイズ除去
	if (HasStr(nikon->NoiseReduction)) {
		_format_ascii(buf, nikon->NoiseReduction, ARRAYSIZE(buf));
		pec->AddItem(emLongExposureNR, buf);
	}

	// 多重露出情報
	if (HasStr(nikon->MultiExposureInfo)) {
		const lkup ekNikonMultiExposureMode[] = {
			0,		euOff,
			1,		_T("多重露出"),
			2,		_T("イメージオーバーレイ"),
			INT_MAX,NULL,
		};
		_format_lookup(buf, nikon->MultiExposureInfo[4], ekNikonMultiExposureMode, -1);
		pec->AddItem(_T("多重露出モード"), buf);

		if (nikon->MultiExposureInfo[4]) {
			_format_int(buf, nikon->MultiExposureInfo[5]);
			pec->AddSubItem(_T("多重露出ショット数"), buf);

			_format_lookup(buf, nikon->MultiExposureInfo[6], ekOffOn, -1);
			pec->AddSubItem(_T("自動ゲイン補正"), buf);
		}
	}

	// 高感度ノイズ除去
	if (HasInt(nikon->HiSensitiveNoiseReduction)) {
		_format_lookup(buf, nikon->HiSensitiveNoiseReduction, ekNikonHiSensitiveNoiseReduction, -1);
		pec->AddItem(_T("高感度ノイズ除去"), buf);
	}

	// 撮影回数
	if (HasInt(nikon->RealTakeCount)) {
		_format_int(buf, nikon->RealTakeCount);
		pec->AddItem(emTotalTakeCount, buf);
	}

	// 削除回数
	if (HasInt(nikon->DelTakeCount)) {
		_format_int(buf, nikon->DelTakeCount);
		pec->AddItem(_T("削除回数"), buf);
	}

	// 差引撮影回数
	if (HasInt(nikon->TakeCount)) {
		_format_int(buf, nikon->TakeCount);
		pec->AddItem(_T("差引撮影回数"), buf);
	}

	// デジタルイメージプログラム
	if (HasStr(nikon->DigiImagePgm)) {
		_format_ascii(buf, nikon->DigiImagePgm, ARRAYSIZE(buf));
		pec->AddItem(_T("デジタルイメージプログラム"), buf);
	}

	// テブレ補正
	if (HasStr(nikon->VibrationReductionText)) {
		_format_ascii(buf, nikon->VibrationReductionText, ARRAYSIZE(buf));
		pec->AddItem(emAntiShake, buf);
	}
	else if (HasInt(nikon->VibrationReduction)) {
		_format_lookup(buf, nikon->VibrationReduction, ekNikonVibrationReduction, -1);
		pec->AddItem(emAntiShake, buf);
	}

	// 画像真正性検証
	if (HasInt(nikon->ImageAuthentication)) {
		_format_lookup(buf, nikon->ImageAuthentication, ekOffOn, -1);
		pec->AddItem(_T("画像真正性検証"), buf);
	}

	// シリアルナンバー
	if (HasStr(nikon->SerialNumber)) {
		if (!memcmp(nikon->SerialNumber, "NO= ", 4)) {
			int n;
			char tmp[32];
			tmp[0] = '0'; tmp[1] = 'x';
			int pos = 4;
			if (nikon->SerialNumber[4] == '-') pos++;
			lstrcpyA(&tmp[2], nikon->SerialNumber + pos);
			StrToIntExA(tmp, STIF_SUPPORT_HEX, &n);
			_format_int(buf, n);
		}
		else
			_format_ascii(buf, nikon->SerialNumber, ARRAYSIZE(buf));
		pec->AddItem(emSerialNumber, buf);
	}

	// ソフトウェア
	if (HasStr(nikon->Software)) {
		_format_ascii(buf, nikon->Software, ARRAYSIZE(buf));
		pec->AddItem(emSoftware, buf);
	}

	return 1;
}

static int enuminfo_olympus(olympus_t* olympus, int model_id, CExifCallback* pec)
{
	TCHAR buf[256];
	LPTSTR p;

	// 画質モード
	if (HasInt(olympus->Quality)) {
		if (IS_OLYMPUS_E(model_id) || olympus->HardwareType[0] == 'D')	// E-XXX またはハードウェアタイプが "D????" の場合
			_format_lookup(buf, olympus->Quality, ekOlymQualityType2, -1);
		else
			_format_lookup(buf, olympus->Quality, ekOlymQualityType1, -1);
		pec->AddItem(emQualityMode, buf);
	}

	// 画像幅と高さ
	if (HasInt(olympus->ImageWidth) && HasInt(olympus->ImageHeight) &&
		olympus->ImageWidth > 0 && olympus->ImageHeight > 0) {
		_format_xy(buf, olympus->ImageWidth, olympus->ImageHeight);
		pec->AddItem(emImageSize, buf);
	}

	// 撮影モード
	if (HasInt(olympus->exExposureMode)) {
		_format_lookup(buf, olympus->exExposureMode, ekOlymExposureMode, -1);
		pec->AddItem(emShootMode, buf);
	}

	// 測光モード
	if (HasInt(olympus->exMeteringMode)) {
		_format_lookup(buf, olympus->exMeteringMode, ekOlymMeteringMode, -1);
		pec->AddItem(_T("測光方式"), buf);
	}

	// フォーカスモード
	if (HasInt(olympus->exFocusMode)) {
		_format_lookup(buf, olympus->exFocusMode, ekOlymFocusMode, -1);
		pec->AddItem(emFocusMode, buf);
	}

	if (!IS_OLYMPUS_E(model_id)) {
		// マクロモード
		if (HasInt(olympus->MacroMode)) {
			_format_lookup(buf, olympus->MacroMode, ekOlymMacroMode, -1);
			pec->AddItem(emMacroMode, buf);
		}

		// デジタルズーム
		if (HasFlt(olympus->DigitalZoom)) {
			_format_scale(buf, olympus->DigitalZoom);
			pec->AddItem(emDigitalZoom, buf);
		}
	}

	// ズーム位置
	if (HasInt(olympus->ZoomPosition)) {
		_format_int(buf, olympus->ZoomPosition);
		pec->AddItem(_T("ズーム位置"), buf);
	}

	// シャープネス
	if (HasInt(olympus->Sharpness)) {
		_format_lookup(buf, olympus->Sharpness, ekOlymSharpness, -1);
		pec->AddItem(emSharpness, buf);
	}

	// コントラスト
	if (HasInt(olympus->Contrast)) {
		_format_lookup(buf, olympus->Contrast, ekOlymContrast, -1);
		pec->AddItem(emContrast, buf);
	}

	// ホワイトバランス
	if (HasInt(olympus->exWhiteBalance)) {
		_format_lookup(buf, olympus->exWhiteBalance, ekOlymWhiteBalance, -1);
		pec->AddItem(emWhiteBalance, buf);
	}
	else if (HasInt(olympus->WhiteBalanceMode[0])) {
		int val = ((olympus->WhiteBalanceMode[0] & 0xff) << 8) | (olympus->WhiteBalanceMode[1] & 0xff);
		_format_lookup(buf, val, ekOlymWhiteBalanceMode, -1);
		pec->AddItem(emWhiteBalance, buf);
	}

	if (HasInt(olympus->exWhiteBalanceBracket[0])) {
		_format_int_comp(buf, olympus->exWhiteBalanceBracket[0], _T(" "));
		_format_int_comp(&buf[lstrlen(buf)], olympus->exWhiteBalanceBracket[1]);
		pec->AddItem(_T("WBブラケット"), buf);
	}

	// 色温度
	if (HasInt(olympus->exWBTemperature) && olympus->exWBTemperature) {
		_format_int(buf, olympus->exWBTemperature, _T("K"));
		// 微調整値を考慮
		if (HasInt(olympus->RawWhiteBalanceAdjust) && olympus->RawWhiteBalanceAdjust) {
			p = buf + lstrlen(buf);
			*p++ = _T(' ');
			_format_int_comp(p, olympus->RawWhiteBalanceAdjust, _T("STEP"));
		}
		pec->AddItem(emColorTemp, buf);
	}

	// ワンタッチWB
	if (HasInt(olympus->OneTouchWB)) {
		_format_lookup(buf, olympus->OneTouchWB, ekOlymOneTouchWB, -1);
		pec->AddItem(_T("ワンタッチWB"), buf);
	}

/* ----- 表示不要 ? ----------
	// ピクチャ情報
	if (HasStr(olympus->PictInfo)) {
		_format_ascii(buf, (char*)olympus->PictInfo, ARRAYSIZE(buf));
		pec->AddItem(_T("ピクチャ情報"), buf);
	}
---------------------------- */

/* ----- 表示不要 ? ----------
	// 赤WB補正
	if (HasShrt(olympus->WBCompRed[0])) {
		_format_xy(buf, olympus->WBCompRed[0], olympus->WBCompRed[1]);
		pec->AddItem(_T("赤WB補正"), buf);
	}
	// 青WB補正
	if (HasShrt(olympus->WBCompBlue[0])) {
		_format_xy(buf, olympus->WBCompBlue[0], olympus->WBCompBlue[1]);
		pec->AddItem(_T("青WB補正"), buf);
	}
---------------------------- */

	// フラッシュモード
	if (HasInt(olympus->FlashMode)) {
		_format_lookup(buf, olympus->FlashMode, ekOlymFlashMode, -1);
		pec->AddItem(emFlashMode, buf);
	}
	else if (HasInt(olympus->exFlashMode)) {
		_format_bitmask_lookup(buf, olympus->exFlashMode, ekOlymFlashModeEx, -1);
		pec->AddItem(emFlashMode, buf);
	}

	// AEロック
	if (HasInt(olympus->exAELock)) {
		_format_lookup(buf, olympus->exAELock, ekOffOn, -1);
		pec->AddItem(_T("AEロック"), buf);
	}

	// AFエリア
	if (HasInt(olympus->exAFArea)) {
		_format_lookup(buf, olympus->exAFArea, ekOlymAFArea, -1);
		pec->AddItem(emAFArea, buf);
	}

	// フラッシュシステム
	if (HasInt(olympus->FlashSystem)) {
		_format_lookup(buf, olympus->FlashSystem, ekOlymFlashSystem, -1);
		pec->AddItem(_T("フラッシュシステム"), buf);
	}

	// フラッシュモデル
	if (HasInt(olympus->FlashModelInfo) && olympus->FlashModelInfo) {
		_format_lookup(buf, olympus->FlashModelInfo, ekOlymFlashModelInfo, -1);
		pec->AddItem(_T("フラッシュモデル"), buf);
	}

	// フラッシュ発光量補正
	if (HasFlt(olympus->FlashComp)) {
		_format_ev_comp(buf, olympus->FlashComp);
		pec->AddItem(emFlashComp, buf);
	}
	else if (HasFlt(olympus->exFlashExposureComp)) {
		_format_ev_comp(buf, olympus->exFlashExposureComp);
		pec->AddItem(emFlashComp, buf);
	}

	// 彩度調整
	if (HasInt(olympus->exSaturationSetting[0])) {
		_format_int_comp(buf, olympus->exSaturationSetting[0]);
		wsprintf(&buf[lstrlen(buf)], _T(" (%d ... +%d)"), olympus->exSaturationSetting[1], olympus->exSaturationSetting[2]);
		pec->AddItem(emSaido, buf);
	}

	// 彩度強調
	if (HasInt(olympus->exModifiedSaturation)) {
		_format_lookup(buf, olympus->exModifiedSaturation, ekOlymModifiedSaturation, -1);
		pec->AddItem(_T("彩度強調"), buf);
	}

	// コントラスト調整
	if (HasInt(olympus->exContrastSetting[0])) {
		_format_int_comp(buf, olympus->exContrastSetting[0]);
		wsprintf(&buf[lstrlen(buf)], _T(" (%d ... +%d)"), olympus->exContrastSetting[1], olympus->exContrastSetting[2]);
		pec->AddItem(emContrast, buf);
	}

	// シャープネス調整
	if (HasInt(olympus->exSharpnessSetting[0])) {
		_format_int_comp(buf, olympus->exSharpnessSetting[0]);
		wsprintf(&buf[lstrlen(buf)], _T(" (%d ... +%d)"), olympus->exSharpnessSetting[1], olympus->exSharpnessSetting[2]);
		pec->AddItem(emSharpness, buf);
	}

	// シーンモード
	if (HasInt(olympus->exSceneMode)) {
		_format_lookup(buf, olympus->exSceneMode, ekOlymSceneMode, -1);
		pec->AddItem(emSceneMode, buf);
	}

	// カラースペース
	if (HasInt(olympus->exColorSpace)) {
		_format_lookup(buf, olympus->exColorSpace, ekOlymColorSpace, -1);
		pec->AddItem(emColorSpace, buf);
	}

	// ノイズリダクション
	if (HasInt(olympus->exNoiseReduction)) {
		_format_lookup(buf, olympus->exNoiseReduction, ekOlymNoiseReduction, -1);
		pec->AddItem(emNoiseReduction, buf);
	}

	// ノイズフィルタ
	if (HasInt(olympus->exNoiseFilter)) {
		_format_lookup(buf, olympus->exNoiseFilter, ekOlymNoiseFilter, -1);
		pec->AddItem(_T("ノイズフィルタ"), buf);
	}

	// シェーディング補正
	if (HasInt(olympus->exShadingComp)) {
		_format_lookup(buf, olympus->exShadingComp, ekOffOn, -1);
		pec->AddItem(_T("シェーディング補正"), buf);
	}

	// JPEG圧縮率
	if (HasFlt(olympus->exCompressionFactor)) {
		buf[0] = _T('1');
		buf[1] = _T('/');
		_format_float(&buf[2], olympus->exCompressionFactor);
		pec->AddItem(_T("JPEG圧縮率"), buf);
	}

	// 階調
	if (HasInt(olympus->exGradation)) {
		_format_lookup(buf, olympus->exGradation, ekOlymGradation, -1);
		pec->AddItem(_T("階調"), buf);
	}

	// 仕上がりモード
	if (HasInt(olympus->exPictureMode)) {
		_format_lookup(buf, olympus->exPictureMode, ekOlymPictureMode, -1);
		pec->AddItem(_T("仕上がりモード"), buf);

		// コントラスト
		_format_int_comp(buf, olympus->exPicModeContrast);
		pec->AddSubItem(emContrast, buf);

		// シャープネス
		_format_int_comp(buf, olympus->exPicModeSharpness);
		pec->AddSubItem(emSharpness, buf);

		if (olympus->exPictureMode & 0xf00) {
			// モノトーン、セピアの場合
			if (olympus->exPicModeFilterEffect) {
				_format_lookup(buf, olympus->exPicModeFilterEffect, ekOlymPicModeFilterEffect, -1);
				pec->AddSubItem(emFilterEffect, buf);
			}
			if (olympus->exPicModeTone) {
				_format_lookup(buf, olympus->exPicModeTone, ekOlymPicModeTone, -1);
				pec->AddSubItem(_T("調色"), buf);
			}
		}
		else {
			// 彩度
			_format_int_comp(buf, olympus->exPicModeSaturation);
			pec->AddSubItem(emSaido, buf);
		}
	}

	// 撮影シーケンス
	if (HasInt(olympus->exShotSequence[0]) && olympus->exShotSequence[0]) {
		if (olympus->exShotSequence[0] != 5)
			_format_lookup(buf, olympus->exShotSequence[0], ekOlymShotSequence, -1);
		else {
			buf[0] = 0;
			if (olympus->exShotSequence[2] & 1) lstrcat(buf, _T("AE+"));
			if (olympus->exShotSequence[2] & 2) lstrcat(buf, _T("WB+"));
			if (olympus->exShotSequence[2] & 4) lstrcat(buf, _T("FL+"));
			if (olympus->exShotSequence[2] & 8) lstrcat(buf, _T("MF+"));
			buf[lstrlen(buf) - 1] = 0;
			lstrcat(buf, _T("ブラケット"));
		}
		wsprintf(&buf[lstrlen(buf)], _T("(%d)"), olympus->exShotSequence[1]);
		pec->AddItem(_T("撮影シーケンス"), buf);
	}

	// パノラマモード
	if (HasInt(olympus->exPanoramaMode)) {
		_format_lookup(buf, olympus->exPanoramaMode, ekOlymPanoramaMode, -1);
		pec->AddItem(_T("パノラマモード"), buf);
	}

	// 気圧
	if (HasInt(olympus->ManometerPressure) && olympus->ManometerPressure) {
		_format_int(buf, olympus->ManometerPressure, _T("hPa"));
		pec->AddItem(_T("気圧"), buf);

		// 標高・水深
		if (HasInt(olympus->ManometerReading[0])) {
			_format_float(buf, olympus->ManometerReading[0] / 10.0F, _T("m ("));
			_format_float(&buf[lstrlen(buf)], olympus->ManometerReading[1] / 10.0F, _T("ft)"));
			pec->AddItem(_T("標高・水深"), buf);
		}
	}

	// ハードウェアタイプ
	if (HasStr(olympus->HardwareType)) {
		_format_ascii(buf, (char*)olympus->HardwareType, ARRAYSIZE(buf));
		pec->AddItem(emHardwareType, buf);
	}

	// レンズ情報
	if (HasInt(olympus->MaxApertureAtMinFocal) && HasInt(olympus->MaxApertureAtMaxFocal) &&
		HasInt(olympus->MinFocalLength) && HasInt(olympus->MaxFocalLength)) {
		float amin = powf(sqrtf(2.0F), (olympus->MaxApertureAtMinFocal / 256.0F));
		float amax = powf(sqrtf(2.0F), (olympus->MaxApertureAtMaxFocal / 256.0F));
		build_generic_lensinfo(buf, olympus->MinFocalLength, olympus->MaxFocalLength, amin, amax);
		pec->AddItem(emLensInfo, buf);

		// レンズ名称
		if (HasInt(olympus->LensInfo) && olympus->LensInfo) {
			_format_lookup(buf, olympus->LensInfo, ekOlymLensInfo, -1);
			pec->AddSubItem(emLensName, buf);
		}
		// シリアル番号
		if (HasStr(olympus->LensSerialNumber)) {
			_format_ascii(buf, olympus->LensSerialNumber, ARRAYSIZE(buf));
			pec->AddSubItem(emSerialNumber, buf);
		}
		// ファームウェアバージョン
		if (HasInt(olympus->LensFirmwareVersion)) {
			_format_hex(buf, olympus->LensFirmwareVersion);
			pec->AddSubItem(emFirmwareVersion, buf);
		}
	}

	// レンズエクステンダー情報
	if (HasInt(olympus->ExtenderInfo) && olympus->ExtenderInfo) {
		_format_lookup(buf, olympus->ExtenderInfo, ekOlymExtenderInfo, -1);
		pec->AddItem(_T("エクステンダー情報"), buf);

		// モデル名
		if (HasStr(olympus->ExtenderModel)) {
			_format_ascii(buf, olympus->ExtenderModel, ARRAYSIZE(buf));
			pec->AddSubItem(_T("モデル名"), buf);
		}
		// シリアル番号
		if (HasStr(olympus->ExtenderSerialNumber)) {
			_format_ascii(buf, olympus->ExtenderSerialNumber, ARRAYSIZE(buf));
			pec->AddSubItem(emSerialNumber, buf);
		}
		// ファームウェアバージョン
		if (HasInt(olympus->ExtenderFirmwareVersion)) {
			_format_hex(buf, olympus->ExtenderFirmwareVersion);
			pec->AddSubItem(emFirmwareVersion, buf);
		}
	}


	// カメラID
	if (HasStr(olympus->CameraID)) {
		_format_ascii(buf, (char*)olympus->CameraID, ARRAYSIZE(buf));
		pec->AddItem(_T("カメラ情報(ID)"), buf);
		// シリアル番号
		if (HasStr(olympus->SerialNumber)) {
			_format_ascii(buf, (char*)olympus->SerialNumber, ARRAYSIZE(buf));
			pec->AddSubItem(emSerialNumber, buf);
		}
		// ボディファームウェア
		if (HasInt(olympus->BodyFirmwareVersion)) {
			_format_hex(buf, olympus->BodyFirmwareVersion);
			pec->AddSubItem(emFirmwareVersion, buf);
		}
	}
	return 1;
}

static int enuminfo_casio(casio_t* casio, int model_id, CExifCallback* pec)
{
	TCHAR buf[256];

	// ホワイトバランス
	if (HasInt(casio->WhiteBalance)) {
		_format_lookup(buf, casio->WhiteBalance, ekCasioWhiteBalance, -1);
		pec->AddItem(emWhiteBalance, buf);
	}

	// ホワイトバランス補正
	if (HasInt(casio->WhiteBalanceBias[0])) {
		wsprintf(buf, _T("%d,%d"), casio->WhiteBalanceBias[0], casio->WhiteBalanceBias[1]);
		pec->AddItem(emWhiteBalanceComp, buf);
	}

	// 被写体までの距離
	if (HasInt(casio->SubjectDistance)) {
		if (casio->SubjectDistance < 0x20000000) {
//@			_format_int(buf, casio->SubjectDistance);
//@			_append_mm(buf);
			_format_distance(buf, casio->SubjectDistance);
		}
		else {
			buf[0] = _T('∞');
			buf[1] = 0;
		}
		pec->AddItem(_T("被写体までの距離"), buf);
	}

	// フラッシュ距離
	if (HasInt(casio->FlashDistance)) {
		_format_int(buf, casio->FlashDistance);
		pec->AddItem(_T("フラッシュ距離"), buf);
	}

	// 撮影モード
	if (HasInt(casio->ShootMode)) {
		_format_lookup(buf, casio->ShootMode, ekCasioShootMode, -1);
		if (casio->ShootMode == CASIO_SHOOTMODE_BESTSHOT && HasInt(casio->BestShotNumber)) {
			const lkup* table = NULL;
			switch (model_id) {
			case CASIO_EXZ3:
			case CASIO_EXZ4:
			case CASIO_EXZ30:
			case CASIO_EXZ40:
			case CASIO_EXZ50:
			case CASIO_EXZ55:
			case CASIO_EXZ57:
			case CASIO_EXS100:
				table = ekCasioBestShot_Z3_4_30_40_50_55_57_S100;
				break;
			case CASIO_EXZ60:
				table = ekCasioBestShot_Z60;
				break;
			case CASIO_EXZ110:
				table = ekCasioBestShot_Z110;
				break;
			case CASIO_EXZ500:
			case CASIO_EXZ600:
				table = ekCasioBestShot_Z500_600;
				break;
			case CASIO_EXZ700:
				table = ekCasioBestShot_Z700;
				break;
			case CASIO_EXZ750:
				table = ekCasioBestShot_Z750;
				break;
			case CASIO_EXZ850:
				table = ekCasioBestShot_Z850;
				break;
			case CASIO_EXZ1000:
				table = ekCasioBestShot_Z1000;
				break;
			case CASIO_EXZ1050:
				table = ekCasioBestShot_Z1050;
				break;
			case CASIO_EXZ1200:
				table = ekCasioBestShot_Z1200;
				break;
			case CASIO_EXV7:
				table = ekCasioBestShot_V7;
				break;
			case CASIO_EXV8:
				table = ekCasioBestShot_V8;
				break;
			case CASIO_EXP505:
				table = ekCasioBestShot_P505;
				break;
			case CASIO_EXP600:
			case CASIO_EXP700:
				table = ekCasioBestShot_P600_700;
				break;
			case CASIO_EXF1:
				table = ekCasioBestShot_EXF1;
				break;
			case CASIO_EXS2:
			case CASIO_EXM2:
				table = ekCasioBestShot_S2_M2;
				break;
			case CASIO_EXS3:
			case CASIO_EXS20:
				table = ekCasioBestShot_S3_20;
				break;
			case CASIO_EXS500:
				table = ekCasioBestShot_S500;
				break;
			case CASIO_EXS600:
				table = ekCasioBestShot_S600;
				break;
			case CASIO_EXS770:
				table = ekCasioBestShot_S770;
				break;
			}

			LPTSTR p = &buf[lstrlen(buf)];
			*p++ = _T('-');
			if (table)
				_format_lookup(p, casio->BestShotNumber, table, -1);
			else
				wsprintf(p, _T("#%d"), casio->BestShotNumber);
		}
		pec->AddItem(emShootMode, buf);
	}

	// シャッターモード
	if (HasInt(casio->ShutterMode)) {
		_format_lookup(buf, casio->ShutterMode, ekCasioShutterMode, -1);
		pec->AddItem(_T("シャッターモード"), buf);
	}

	// セルフタイマー
	if (HasInt(casio->SelfTimer)) {
		_format_lookup(buf, casio->SelfTimer, ekCasioSelfTimer, -1);
		pec->AddItem(euSelfTimer, buf);
	}

	// 画質モード
	if (HasInt(casio->Quality)) {
		_format_lookup(buf, casio->Quality, ekCasioQuality, -1);
		pec->AddItem(emQualityMode, buf);
	}

	// フォーカスモード
	if (HasInt(casio->FocusMode)) {
		_format_lookup(buf, casio->FocusMode, ekCasioFocusMode, -1);
		pec->AddItem(emFocusMode, buf);
	}

	// タイムゾーン
	if (HasStr(casio->TimeZone)) {
		_format_ascii(buf, casio->TimeZone, ARRAYSIZE(buf));
		pec->AddItem(_T("タイムゾーン"), buf);
	}

	// ISO感度
	if (HasInt(casio->ISO)) {
		_format_int(buf, casio->ISO);
		pec->AddItem(emISO, buf);
	}

	// カラーモード
	if (HasInt(casio->ColorMode)) {
		_format_lookup(buf, casio->ColorMode, ekCasioColorMode, -1);
		pec->AddItem(emColorMode, buf);
	}

	// 色強調
	if (HasInt(casio->ColorEnhance)) {
		_format_lookup(buf, casio->ColorEnhance, ekCasioColorType, -1);
		pec->AddItem(_T("色強調"), buf);
	}

	// カラーフィルタ
	if (HasInt(casio->ColorFilter)) {
		_format_lookup(buf, casio->ColorFilter, ekCasioColorType, -1);
		pec->AddItem(emColorFilter, buf);
	}

	// シャープネス
	if (HasInt(casio->Sharpness)) {
		_format_int_comp(buf, casio->Sharpness);
		pec->AddItem(emSharpness, buf);
	}

	// 彩度
	if (HasInt(casio->Saturation)) {
		_format_int_comp(buf, casio->Saturation / 2);
		pec->AddItem(emSaido, buf);
	}

	// コントラスト
	if (HasInt(casio->Contrast)) {
		_format_int_comp(buf, casio->Contrast);
		pec->AddItem(emContrast, buf);
	}

	return 1;
}

static int enuminfo_minolta(minolta_t* minolta, int model_id, CExifCallback* pec)
{
	TCHAR buf[256], tmp[64];
	float f;
	int val;

	// 露出モード
	if (HasInt(minolta->ExpMode)) {
		_format_lookup(buf, minolta->ExpMode, ekMinoltaExpMode, -1);
		pec->AddItem(emExposureMode, buf);
	}

	// ドライブモード
	if (HasInt(minolta->DriveMode)) {
		if (IS_MINOLTA_ALPHA(model_id))
			_format_lookup(buf, minolta->DriveMode, ekMinoltaDriveMode7D, -1);
		else
			_format_lookup(buf, minolta->DriveMode, ekMinoltaDriveMode, -1);
		pec->AddItem(emDriveMode, buf);
	}

	// 測光モード
	if (HasInt(minolta->MeteringMode)) {
		_format_lookup(buf, minolta->MeteringMode, ekMinoltaMeteringMode, -1);
		pec->AddItem(emMeteringMode, buf);
	}

	// 画質モード
	if (HasInt(minolta->ImageQuality)) {
		_format_lookup(buf, minolta->ImageQuality, ekMinoltaImageQuality, -1);
		pec->AddItem(emQualityMode, buf);
	}

	// フラッシュ発光
	if (HasInt(minolta->FlashFired)) {
		_format_lookup(buf, minolta->FlashFired, ekFlashFired, 2);
		pec->AddItem(euFlash, buf);

		if (minolta->FlashFired != 0) {
			// フラッシュモード
			if (HasInt(minolta->FlashMode)) {
				_format_lookup(buf, minolta->FlashMode, ekMinoltaFlashMode, -1);
				pec->AddItem(emFlashMode, buf);
			}

			// フラッシュ調光モード
			if (HasInt(minolta->FlashControl)) {
				_format_lookup(buf, minolta->FlashControl, ekMinoltaFlashControl, -1);
				pec->AddItem(_T("調光モード"), buf);
			}
		}
	}

	// フラッシュ補正
	if (HasInt(minolta->FlashComp)) {
		if (IS_MINOLTA_ALPHA(model_id))
			_format_ev_comp(buf, (float)minolta->FlashComp / 24.0F);
		else
			_format_ev_comp(buf, (minolta->FlashComp - 6) / 3.0F);
		pec->AddItem(emFlashComp, buf);
	}

	// ホワイトバランス
	if (HasInt(minolta->WhiteBalance)) {
		if (model_id == MINOLTA_ALPHA_7D) {
			_format_lookup(buf, minolta->WhiteBalance, ekMinoltaWhiteBalance7D, -1);
		}
		else {
			_format_lookup(buf, minolta->WhiteBalance, ekMinoltaWhiteBalance, -1);
		}

		if (minolta->WhiteBalance != 0 &&				// WBオートではなく
			HasInt(minolta->WhiteBalanceTune) &&		// チューン値が存在し
			minolta->WhiteBalanceTune != 0) {			// チューン値が0ではない場合
			if (IS_MINOLTA_ALPHA(model_id) && minolta->WhiteBalance < 0x100) {
				_format_int_comp(tmp, minolta->WhiteBalanceTune);
				wsprintf(&buf[lstrlen(buf)], _T(" (%s)"), tmp);
			}
		}
		pec->AddItem(emWhiteBalance, buf);
	}

	// 色温度
	if (HasInt(minolta->ColorTemp) && minolta->ColorTemp != 0) {
		_format_int(buf, minolta->ColorTemp, _T("K"));
		pec->AddItem(emColorTemp, buf);
	}

	// CCフィルター
	if (HasInt(minolta->ColorCompFilter)) {
		if (minolta->ColorCompFilter < 9)			// Green
			wsprintf(buf, _T("G%d"), (9 - minolta->ColorCompFilter));
		else if (minolta->ColorCompFilter > 9)		// Magenta
			wsprintf(buf, _T("M%d"), (minolta->ColorCompFilter - 9));
		else
			lstrcpy(buf, euNone);
		pec->AddItem(_T("CC(色補正)フィルター"), buf);
	}

	// カラーモード or 画像仕上げ
	if (HasInt(minolta->ColorMode)) {
		if (model_id == MINOLTA_ALPHA_SWEET) {
			_format_lookup(buf, minolta->ColorMode, ekMinoltaPhotoFinish, -1);
			pec->AddItem(emPhotoFinish, buf);
		}
		else {
			// 下位4ビットがカラーモード、上位4bitがICCプロファイル付きを示す
			_format_lookup(buf, (minolta->ColorMode & 0x0f), ekMinoltaColorMode, -1);
			if (minolta->ColorMode & 0x80) lstrcat(buf, _T(" + ICC"));
			pec->AddItem(emColorMode, buf);
		}
	}

	// コントラスト調整
	if (HasInt(minolta->Contrast)) {
		if (IS_MINOLTA_ALPHA(model_id))
			val = minolta->Contrast - 10;
		else
			val = minolta->Contrast - 5;
		_format_int_comp(buf, val);
		pec->AddItem(emContrast, buf);
	}

	// 彩度調整
	if (HasInt(minolta->Saturation)) {
		if (IS_MINOLTA_ALPHA(model_id))
			val = minolta->Saturation - 10;
		else
			val = minolta->Saturation - 5;
		_format_int_comp(buf, val);
		pec->AddItem(emSaturation, buf);
	}

	// シャープネス
	if (HasInt(minolta->Sharpness)) {
		if (IS_MINOLTA_ALPHA(model_id))
			_format_int_comp(buf, minolta->Sharpness - 10);
		else
			_format_lookup(buf, minolta->Sharpness, ekMinoltaSharpness, -1);
		pec->AddItem(emSharpness, buf);
	}

	// 色合い調整
	if (HasInt(minolta->Hue)) {
		_format_int_comp(buf, minolta->Hue - 10);
		pec->AddItem(emColorTone, buf);
	}

	// シーンセレクタ
	if (HasInt(minolta->SubjectProgram)) {
		_format_lookup(buf, minolta->SubjectProgram, ekMinoltaSubjectProgram, -1);
		pec->AddItem(_T("シーンセレクタ"), buf);
	}

/* --- いらない?
	// フォーカス／レリーズ優先
	if (HasInt(minolta->FocusRelease)) {
		_format_lookup(buf, minolta->FocusRelease, ekMinoltaFocusRelease, 2);
		pec->AddItem(_T("フォーカス/レリーズ"), buf);
	}
--- */

	// 撮像感度
	if (HasInt(minolta->FilmSpeed)) {
		f = powf(2.0F, ((float)minolta->FilmSpeed / 8.0F - 1.0F)) * 3.125F;
		_format_int(buf, (int)floorf(f + 0.5F));
		pec->AddItem(_T("撮像感度"), buf);
	}

	// シャッター速度
	if (HasInt(minolta->ShutterSpeed)) {
		f = powf(2.0f, ((48.0F - minolta->ShutterSpeed) / 8.0F));
		_format_shutter(buf, f);
		pec->AddItem(emShutterSpeed, buf);
	}

	// 絞り値
	if (HasInt(minolta->Aperture)) {
		f = powf(2.0F, (float)minolta->Aperture / 16.0F - 0.5F);
		_format_Fnumber(buf, f);
		pec->AddItem(emAperture, buf);
	}

	// マクロモード
	if (HasInt(minolta->MacroMode)) {
		_format_lookup(buf, minolta->MacroMode, ekOffOn, -1);
		pec->AddItem(emMacroMode, buf);
	}

	// デジタルズーム
	if (HasInt(minolta->DigitalZoom)) {
		_format_int(buf, minolta->DigitalZoom);
		pec->AddItem(emDigitalZoom, buf);
	}

	// 露出補正
	if (HasInt(minolta->ExpCompensation)) {
		if (IS_MINOLTA_ALPHA(model_id))
			_format_ev_comp(buf, (float)minolta->ExpCompensation / 24.0F);
		else
			_format_ev_comp(buf, (float)minolta->ExpCompensation / 3.0F - 2.0F);
		pec->AddItem(emExpComp, buf);
	}

	// メータードマニュアル
	if (HasInt(minolta->MeteredManual)) {
		f = (float)minolta->MeteredManual;
		if (model_id == MINOLTA_ALPHA_SWEET)
			f = (float)(minolta->MeteredManual - 0x80);
		_format_ev_comp(buf, f / 24.0F);
		pec->AddItem(_T("メータードマニュアル"), buf);
	}

	// ブラケットステップ
	if (HasInt(minolta->BracketStep)) {
		_format_lookup(buf, minolta->BracketStep, ekMinoltaBracketStep, -1);
		pec->AddItem(emBracketStep, buf);
	}

	// 焦点距離
	if (HasInt(minolta->FocalLength)) {
		_format_float(buf, (float)minolta->FocalLength / 256.0F, _T("mm"));
		pec->AddItem(emFocalLength, buf);
	}

	// フォーカス距離
	if (HasInt(minolta->FocusDistance)) {

		if (minolta->FocusDistance != 0)
			_format_distance(buf, minolta->FocusDistance);
		else {
			buf[0] = _T('∞');
			buf[1] = 0;
		}
		pec->AddItem(emFocusDistance, buf);
	}

	// 撮影日時
	if (HasInt(minolta->Date) && HasInt(minolta->Time)) {
		wsprintf(buf, _T("%04d/%02d/%02d %02d:%02d:%02d"),
				minolta->Date >> 16, (minolta->Date >> 8) & 0xff, minolta->Date & 0xff,
				minolta->Time >> 16, (minolta->Time >> 8) & 0xff, minolta->Time & 0xff);
		pec->AddItem(emDateTime, buf);
	}

	// 開放F値
	if (HasInt(minolta->MaxAperture)) {
		f = powf(2.0F, ((float)minolta->MaxAperture / 16.0F - 0.5F));
		_format_Fnumber(buf, f);
		pec->AddItem(emMaxAperture, buf);
	}

	// ISOセッティング
	if (HasInt(minolta->ISOSetting)) {
		if (IS_MINOLTA_ALPHA(model_id))
			_format_lookup(buf, minolta->ISOSetting, ekMinoltaISOSetting7D, -1);
		else
			_format_lookup(buf, minolta->ISOSetting, ekMinoltaISOSetting, -1);
		pec->AddItem(emISOSetting, buf);
	}

	// カラーフィルタ
	if (HasInt(minolta->ColorFilter)) {
		_format_int_comp(buf, minolta->ColorFilter - 3);
		pec->AddItem(emColorFilter, buf);
	}

	// 輝度
	if (HasInt(minolta->BrightnessValue)) {
		_format_float(buf, minolta->BrightnessValue / 8.0F - 6.0F);
		pec->AddItem(_T("輝度"), buf);
	}

	// ワイドフォーカスゾーン
	if (HasInt(minolta->WideFocusZone)) {
		_format_lookup(buf, minolta->WideFocusZone, ekMinoltaWideFocusZone, -1);
		pec->AddItem(_T("ワイドフォーカスゾーン"), buf);
	}

	// フォーカスモード
	if (HasInt(minolta->FocusMode)) {
		if (model_id == MINOLTA_ALPHA_7D) {
			_format_lookup(buf, minolta->FocusMode, ekMinoltaFocusMode7D, -1);
			if (minolta->FocusMode == 4 && minolta->DMF != 0)		// AF-A && DMF
				lstrcpy(buf, euDMF);
		}
		else if (model_id == MINOLTA_ALPHA_SWEET) {
			_format_lookup(buf, minolta->FocusMode, ekMinoltaFocusModeSW, -1);
		}
		else
			_format_lookup(buf, minolta->FocusMode, ekMinoltaFocusMode, -1);
		pec->AddItem(emFocusMode, buf);
	}

	// フォーカスフレーム
	if (HasInt(minolta->FocusFrame)) {
		_format_lookup(buf, minolta->FocusFrame, ekMinoltaFocusFrame, -1);
		if (HasInt(minolta->LocalFocusFrame) && minolta->LocalFocusFrame != 0) {
			_format_lookup(tmp, minolta->LocalFocusFrame, ekMinoltaLocalFocusFrame, -1);
			wsprintf(&buf[lstrlen(buf)], _T(" (%s)"), tmp);
		}
		pec->AddItem(_T("フォーカスフレーム"), buf);
	}

	// スポットフォーカスポイント
	if (HasInt(minolta->SpotFocusPointX) && HasInt(minolta->SpotFocusPointY)) {
		_format_xy(buf, minolta->SpotFocusPointX, minolta->SpotFocusPointY);
		pec->AddItem(emFocusPoint, buf);
	}

	// DECポジション
	if (HasInt(minolta->DECPosition)) {
		_format_lookup(buf, minolta->DECPosition, ekMinoltaDECPosition, -1);
		pec->AddItem(_T("DECポジション"), buf);
	}

	// アンチシェイク
	if (HasInt(minolta->AntiShake)) {
		_format_lookup(buf, minolta->AntiShake, ekOffOn, -1);
		pec->AddItem(_T("アンチシェイク"), buf);
	}

	// ゾーン切替
	if (HasInt(minolta->ZoneMatching)) {
		_format_lookup(buf, minolta->ZoneMatching, ekMinoltaZoneMatching, -1);
		pec->AddItem(_T("ゾーン切り替え"), buf);
	}

	// レンズ情報
	if (HasInt(minolta->LensInfo)) {
////////////////////////////////////////
// use sony alpha-lens-id list
//		_format_lookup(buf, minolta->LensInfo, ekMinoltaLensInfo, -1);
		_format_lookup(buf, minolta->LensInfo, ekSonyLensID, -1);
		pec->AddItem(emLensInfo, buf);
	}

	return 1;
}

static int enuminfo_fuji(fuji_t* fuji, int model_id, CExifCallback* pec)
{
	TCHAR buf[256];

	// バージョン
	if (HasStr(fuji->Version)) {
		_format_ascii(buf, fuji->Version, ARRAYSIZE(buf));
		pec->AddItem(emVersion, buf);
	}

	// 画像クォリティ
	if (HasStr(fuji->Quality)) {
		_format_ascii(buf, fuji->Quality, ARRAYSIZE(buf));
		pec->AddItem(emQualityMode, buf);
	}

	// 撮影モード
	if (HasInt(fuji->PictureMode)) {
		_format_lookup(buf, fuji->PictureMode, ekFujiPictureMode, -1);
		pec->AddItem(emShootMode, buf);
	}

	// ホワイトバランス
	if (HasInt(fuji->WhiteBalance)) {
		_format_lookup(buf, fuji->WhiteBalance, ekFujiWhiteBalance, -1);
		if (HasInt(fuji->WBFineTune[0]) && (fuji->WBFineTune[0] != 0 || fuji->WBFineTune[1] != 0))
			wsprintf(&buf[lstrlen(buf)], _T(" ( 赤%c%d, 青%c%d )"),
					fuji->WBFineTune[0] < 0 ? _T('-') : _T('+'), abs(fuji->WBFineTune[0]),
					fuji->WBFineTune[1] < 0 ? _T('-') : _T('+'), abs(fuji->WBFineTune[1]));
		pec->AddItem(emWhiteBalance, buf);
	}

	// 色の濃さ
	if (HasInt(fuji->Color)) {
		if (IS_FUJIFILM_PRO(model_id))
			_format_lookup(buf, fuji->Color, ekFujiColor, -1);
		else
			_format_lookup(buf, fuji->Color, ekFujiHighLow, -1);
		pec->AddItem(_T("色の濃さ"), buf);
	}

	// 色温度
	if (HasInt(fuji->ColorTemperature)) {
		_format_int(buf, fuji->ColorTemperature, _T("K"));
		pec->AddItem(emColorTemp, buf);
	}

	// シャープネス
	if (HasInt(fuji->Sharpness)) {
		_format_lookup(buf, fuji->Sharpness, ekFujiSharpness, -1);
		pec->AddItem(emSharpness, buf);
	}

	// トーン
	if (HasInt(fuji->Tone)) {
		if (IS_FUJIFILM_PRO(model_id))
			_format_lookup(buf, fuji->Tone, ekFujiTone, -1);
		else
			_format_lookup(buf, fuji->Tone, ekFujiHighLow, -1);
		pec->AddItem(_T("トーン"), buf);
	}

	// コントラスト
	if (HasInt(fuji->Contrast)) {
		_format_lookup(buf, fuji->Contrast, ekFujiHighLow, -1);
		pec->AddItem(emContrast, buf);
	}

	// ノイズリダクション
	if (HasInt(fuji->NoiseReduction)) {
		_format_lookup(buf, fuji->NoiseReduction, ekFujiNoiseReduction, -1);
		pec->AddItem(emNoiseReduction, buf);
	}

	// フラッシュモード
	if (HasInt(fuji->FlashMode)) {
		_format_lookup(buf, fuji->FlashMode, ekFujiFlashMode, -1);
		pec->AddItem(emFlashMode, buf);
	}

	// フラッシュの発光強度補正値
	if (HasFlt(fuji->FlashStrength)) {
		_format_float_comp(buf, fuji->FlashStrength);
		pec->AddItem(emFlashComp, buf);
	}

	// マクロモード
	if (HasInt(fuji->MacroMode)) {
		_format_lookup(buf, fuji->MacroMode, ekOffOn, -1);
		pec->AddItem(emMacroMode, buf);
	}

	// フォーカスモード
	if (HasInt(fuji->FocusMode)) {
		_format_lookup(buf, fuji->FocusMode, ekFujiFocusMode, -1);
		pec->AddItem(emFocusMode, buf);
	}

	// センター固定AF
	if (HasInt(fuji->CenterFixedAF)) {
		_format_lookup(buf, fuji->CenterFixedAF, ekOffOn, -1);
		pec->AddItem(_T("センター固定AF"), buf);
	}

	// フォーカスポイント
	if (HasInt(fuji->FocusPointX) &&
		HasInt(fuji->FocusPointY)) {
		_format_xy(buf, fuji->FocusPointX, fuji->FocusPointY);
		pec->AddItem(emFocusPoint, buf);
	}

	// スローシンクロ
	if (HasInt(fuji->SlowSync)) {
		_format_lookup(buf, fuji->SlowSync, ekOffOn, -1);
		pec->AddItem(_T("スローシンクロ"), buf);
	}

	// 連写・自動ブラケットモード
	if (HasInt(fuji->ContTake)) {
		_format_lookup(buf, fuji->ContTake, ekFujiContTake, -1);
		pec->AddItem(_T("連写・自動ブラケットモード"), buf);

		if (fuji->ContTake > 0 && HasInt(fuji->ContTakeStart)) {
			_format_int(buf, fuji->ContTakeStart);
			pec->AddItem(euContTakeNumber, buf);
		}
	}

	// FinePixカラー
	if (HasInt(fuji->FinePixColor)) {
		_format_lookup(buf, fuji->FinePixColor, ekFujiFinePixColor, -1);
		pec->AddItem(_T("FinePixカラー"), buf);
	}

	// 手振れ警告
	if (HasInt(fuji->BlurWarning)) {
		_format_enumtext(buf, fuji->BlurWarning, euAriNashi, 2);
		pec->AddItem(_T("手振れ警告"), buf);
	}

	// フォーカス警告
	if (HasInt(fuji->FocusWarning)) {
		_format_enumtext(buf, fuji->FocusWarning, euAriNashi, 2);
		pec->AddItem(_T("フォーカス警告"), buf);
	}

	// AE警告
	if (HasInt(fuji->AEWarning)) {
		_format_enumtext(buf, fuji->AEWarning, euAriNashi, 2);
		pec->AddItem(_T("AE警告"), buf);
	}

	// ダイナミックレンジ
	if (HasInt(fuji->DynamicRange)) {
		_format_lookup(buf, fuji->DynamicRange, ekFujiDynamicRange, -1);
		if (HasInt(fuji->DynamicRangeVal))
			wsprintf(&buf[lstrlen(buf)], _T(" (%d%%)"), fuji->DynamicRangeVal);
		pec->AddItem(_T("D-レンジ"), buf);
	}

	// ダイナミックレンジ2
	if (HasInt(fuji->DynamicRangeSetting)) {
		_format_lookup(buf, fuji->DynamicRangeSetting, ekFujiDynamicRangeSetting, -1);
		pec->AddItem(_T("D-レンジ モード"), buf);
	}

	// フィルムシミュレーション
	if (HasInt(fuji->FilmSimulationMode)) {
		const lkup* table;
		switch (model_id) {
		case FUJIFILM_S5PRO:	table = ekFujiFilmSimulationModeS5Pro; break;
		case FUJIFILM_S100FS:
		case FUJIFILM_F200EXR:	table = ekFujiFilmSimulationModeS100FS; break;
		default:				table = ekFujiFilmSimulationMode; break;
		}
		_format_lookup(buf, fuji->FilmSimulationMode, table, -1);
		pec->AddItem(_T("フィルムシミュレーション"), buf);
	}

	// レンズおよび絞り値情報
	if (HasInt(fuji->MaxFocalLength) &&
		HasInt(fuji->MinFocalLength)) {
		build_generic_lensinfo(buf, fuji->MinFocalLength, fuji->MaxFocalLength, fuji->MaxApertureAtMinFocal, fuji->MaxApertureAtMinFocal);
		pec->AddItem(emLensInfo, buf);
	}

	// 顔検出位置
	if (HasInt(fuji->FDPoint[0])) {
		wsprintf(buf, _T("(%d,%d)-(%d,%d)"),
				fuji->FDPoint[0], fuji->FDPoint[1],
				fuji->FDPoint[2], fuji->FDPoint[3]);
		pec->AddItem(_T("顔検出位置"), buf);
	}

	return 1;
}

static int enuminfo_panasonic(panasonic_t* pana, CExifCallback* pec)
{
	TCHAR buf[256];
	LPTSTR p;

	// 画質モード
	if (HasInt(pana->Quality)) {
		_format_lookup(buf, pana->Quality, ekPanaQuality, -1);
		pec->AddItem(emQualityMode, buf);
	}

	// ホワイトバランス
	if (HasInt(pana->WhiteBalance)) {
		_format_lookup(buf, pana->WhiteBalance, ekPanaWhiteBalance, -1);
		pec->AddItem(emWhiteBalance, buf);
	}

	// ホワイトバランス微調整
	if (HasInt(pana->WhiteBalanceBias)) {
		_format_int_comp(buf, pana->WhiteBalanceBias);
		if (pana->WhiteBalanceBias != 0) {
			p = &buf[96];
			_format_int_comp(p, 150 * pana->WhiteBalanceBias);
			TCHAR tmp[128];
			wsprintf(tmp, _T(" (%sK)"), p);
			lstrcat(buf, tmp);
		}
		pec->AddItem(_T("WB微調整"), buf);
	}

	// フォーカスモード
	if (HasInt(pana->FocusMode)) {
		_format_lookup(buf, pana->FocusMode, ekPanaFocusMode, -1);
		pec->AddItem(emFocusMode, buf);
	}

	// AFモード
	if (HasInt(pana->AFMode)) {
		_format_lookup(buf, pana->AFMode, ekPanaAFMode, -1);
		pec->AddItem(emAFMode, buf);
	}

	// 手ブレ補正
	if (HasInt(pana->AntiShake)) {
		_format_lookup(buf, pana->AntiShake, ekPanaAntiShake, -1);
		pec->AddItem(emAntiShake, buf);
	}

	// マクロモード
	if (HasInt(pana->MacroMode)) {
		_format_lookup(buf, pana->MacroMode, ekPanaMacroMode, -1);
		pec->AddItem(emMacroMode, buf);
	}

	// フィルムモード
	if (HasInt(pana->FilmMode)) {
		_format_lookup(buf, pana->FilmMode, ekPanaFilmMode, -1);
		pec->AddItem(_T("フィルムモード"), buf);
	}

	// シーン（撮影）モード
	if (HasInt(pana->ShootingMode)) {
		_format_lookup(buf, pana->ShootingMode, ekPanaShootingMode, -1);
		pec->AddItem(emSceneMode, buf);
	}

	// マイカラー
	if (HasInt(pana->MyColor)) {
		_format_lookup(buf, pana->MyColor, ekPanaMyColor, -1);
		pec->AddItem(_T("マイカラー"), buf);
	}

	// シリアル番号
	if (HasStr(pana->InternalSerialNumber)) {
		_format_ascii(buf, pana->InternalSerialNumber, ARRAYSIZE(buf));
		pec->AddItem(emSerialNumber, buf);
	}

	// 連射
	if (HasInt(pana->BurstMode)) {
		if (pana->BurstMode > 0) {
			_format_lookup(buf, pana->BurstMode, ekPanaBurstMode, -1);
			if (pana->SequenceNumber > 0) {
				wsprintf(&buf[lstrlen(buf)], _T(" (%d枚目)"), pana->SequenceNumber);
			}
		}
		else
			lstrcpy(buf, euNone);
		pec->AddItem(euContinuosShot, buf);
	}

	// コントラスト
	if (HasInt(pana->exContrast)) {
		_format_int_comp(buf, pana->exContrast);
		pec->AddItem(emContrast, buf);
	}
	else {
		if (HasInt(pana->Contrast)) {
			_format_lookup(buf, pana->Contrast, ekPanaContrast, -1);
			pec->AddItem(emContrast, buf);
		}
	}

	// シャープネス
	if (HasInt(pana->exSharpness)) {
		_format_int_comp(buf, pana->exSharpness);
		pec->AddItem(emSharpness, buf);
	}

	// 再度
	if (HasInt(pana->exSaturation)) {
		_format_int_comp(buf, pana->exSaturation);
		pec->AddItem(emSaturation, buf);
	}

	// カラーエフェクト
	if (HasInt(pana->ColorEffect)) {
		_format_lookup(buf, pana->ColorEffect, ekPanaColorEffect, -1);
		pec->AddItem(_T("カラ－エフェクト"), buf);
	}

/* @@@
	// シーンモード
	if (HasInt(pana->SceneMode)) {
		_format_lookup(buf, pana->SceneMode, ekPanaSceneMode, -1);
		pec->AddItem(emSceneMode, buf);
	}
@@@ */

	// 画質調整
	if (HasInt(pana->ColorMode)) {
		_format_lookup(buf, pana->ColorMode, ekPanaColorMode, -1);
		pec->AddItem(_T("画質調整"), buf);
	}

	// 暗部補正
	if (HasInt(pana->LightDetection)) {
		const lkup ekPanaLightDetection[] = {
			0,		euOff,
			1,		_T("弱"),
			2,		_T("中"),
			3,		_T("強"),
			INT_MAX,NULL,
		};
		_format_lookup(buf, pana->LightDetection, ekPanaLightDetection, -1);
		pec->AddItem(_T("暗部補正"), buf);
	}

	// セルフタイマー
	if (HasInt(pana->SelfTimer)) {
		_format_lookup(buf, pana->SelfTimer, ekPanaSelfTimer, -1);
		pec->AddItem(euSelfTimer, buf);
	}

	// 使用レンズ
	if (HasStr(pana->LensName)) {
		_format_ascii(buf, pana->LensName, ARRAYSIZE(buf));
		pec->AddItem(emLensName, buf);
	}

	// アクセサリ
	if (HasStr(pana->Accessory)) {
		_format_ascii(buf, pana->Accessory, ARRAYSIZE(buf));
		pec->AddItem(_T("アクセサリ"), buf);
	}

	return 1;
}

static int enuminfo_sanyo(sanyo_t* sanyo, CExifCallback* pec)
{
	TCHAR buf[256];

	// 画質モード
	if (HasInt(sanyo->Quality)) {
		_format_lookup(buf, (sanyo->Quality >> 8), ekSanyoQuality, -1);
		wsprintf(&buf[lstrlen(buf)], _T("(%d)"), (sanyo->Quality & 0xff));
		pec->AddItem(emQualityMode, buf);
	}

	// 撮影モード
	if (HasInt(sanyo->ShootMode)) {
		_format_lookup(buf, sanyo->ShootMode, ekSanyoShootMode, -1);
		pec->AddItem(emShootMode, buf);
	}

	// デジタルズーム
	if (HasFlt(sanyo->DigitalZoom)) {
		_format_scale(buf, sanyo->DigitalZoom);
		pec->AddItem(emDigitalZoom, buf);
	}

	// 連射モード
	if (HasInt(sanyo->ShootMode)) {
		_format_lookup(buf, sanyo->ContShoot, ekSanyoContShoot, -1);
		pec->AddItem(_T("連写モード"), buf);
	}

	// ワイドレンジショット
	if (HasInt(sanyo->WideRangeShoot)) {
		_format_lookup(buf, sanyo->WideRangeShoot, ekOffOn, -1);
		pec->AddItem(_T("ワイドレンジショット"), buf);
	}

	// リアルカラーイコライザ
	if (HasInt(sanyo->WideRangeShoot)) {
		_format_lookup(buf, sanyo->RealColorEqualizer, ekOffOn, -1);
		pec->AddItem(_T("リアルカラーイコライザ"), buf);
	}

	// クイックショット
	if (HasInt(sanyo->QuickShot)) {
		_format_lookup(buf, sanyo->QuickShot, ekOffOn, -1);
		pec->AddItem(_T("クイックショット"), buf);
	}

	// セルフタイマー
	if (HasInt(sanyo->SelfTimer)) {
		_format_lookup(buf, sanyo->SelfTimer, ekOffOn, -1);
		pec->AddItem(euSelfTimer, buf);
	}

	// ボイスメモ
	if (HasInt(sanyo->VoiceMemo)) {
		_format_lookup(buf, sanyo->VoiceMemo, ekOffOn, -1);
		pec->AddItem(_T("ボイスメモ"), buf);
	}

	// フリッカー軽減
	if (HasInt(sanyo->FlickerReduce)) {
		_format_lookup(buf, sanyo->FlickerReduce, ekOffOn, -1);
		pec->AddItem(_T("フリッカー軽減"), buf);
	}

	// フォーカスモード
	if (HasInt(sanyo->FocusMode)) {
		_format_lookup(buf, sanyo->FocusMode, ekSanyoFocusMode, -1);
		pec->AddItem(emFocusMode, buf);
	}

	// フラッシュモード
	if (HasInt(sanyo->FlashMode)) {
		_format_lookup(buf, sanyo->FlashMode, ekSanyoFlashMode, -1);
		pec->AddItem(emFlashMode, buf);
	}

	return 1;
}

static int enuminfo_pentax(pentax_t* pentax, int model_id, CExifCallback* pec)
{
	TCHAR buf[256];

	// 撮影日時
	if (HasInt(pentax->ShootingDate) && HasInt(pentax->ShootingTime)) {
		wsprintf(buf, _T("%04d/%02d/%02d %02d:%02d:%02d"),
				(pentax->ShootingDate >> 16) & 0xffff,
				(pentax->ShootingDate >>  8) & 0xff,
				(pentax->ShootingDate      ) & 0xff,
				(pentax->ShootingTime >> 16) & 0xff,
				(pentax->ShootingTime >>  8) & 0xff,
				(pentax->ShootingTime      ) & 0xff);
		pec->AddItem(emDateTime, buf);
	}

	// 画質モード
	if (HasInt(pentax->Quality)) {
		_format_lookup(buf, pentax->Quality, ekPentaxQualityMode, -1);
		pec->AddItem(emQualityMode, buf);
	}

	// イメージサイズ
	if (HasInt(pentax->ImageSize)) {
		_format_lookup(buf, pentax->ImageSize & 0xff, ekPentaxImageSize, -1);
		pec->AddItem(emImageSize, buf);
	}

	// シャッター速度
	if (HasInt(pentax->ShutterSpeed)) {
		_format_shutter(buf, (float)pentax->ShutterSpeed / 100000.0F);
		pec->AddItem(emShutterSpeed, buf);
	}

	// 絞り
	if (HasInt(pentax->Aperture)) {
		_format_Fnumber(buf, (float)pentax->Aperture / 10.0F);
		pec->AddItem(emAperture, buf);
	}

	// 露出補正
	if (HasInt(pentax->ExposureComp)) {
		_format_ev_comp(buf, (float)(pentax->ExposureComp - 50) / 10.0F);
		pec->AddItem(emExpComp, buf);
	}

	// 焦点距離
	if (HasInt(pentax->FocalLength)) {
		_format_float_int(buf, (float)pentax->FocalLength / 100.0F, _T("mm"));
		pec->AddItem(emFocalLength, buf);
	}

	// ISO
	if (HasInt(pentax->ISO)) {
		int val = _lookup_val(pentax->ISO, evPentaxISO);
		if (val != INT_MAX) _format_int(buf, val);
		else lstrcpy(buf, euUnknown);
		pec->AddItem(emISO, buf);
	}

	// 測光モード
	if (HasInt(pentax->MeteringMode)) {
		_format_lookup(buf, pentax->MeteringMode, ekPentaxMeteringMode, -1);
		pec->AddItem(emMeteringMode, buf);
	}

	// オートブラケット
	if (HasInt(pentax->AutoBracket[0])) {
		float f = (pentax->AutoBracket[0] < 10) ? (float)pentax->AutoBracket[0] / 3.0F : (float)pentax->AutoBracket[0] - 9.5F;
		_format_ev_comp(buf, f);
		pec->AddItem(emBracketStep, buf);
		if (pentax->AutoBracket[0] != 0) {
			const lkup ekPentaxExtendedBracket[] = {
				1,		_T("ホワイトバランス(WB-BA)"),
				2,		_T("ホワイトバランス(WB-GM)"),
				3,		emSaido,
				4,		emSharpness,
				5,		emContrast,
				INT_MAX,NULL,
			};
			_format_lookup(buf, pentax->AutoBracket[1] >> 8, ekPentaxExtendedBracket, -1);
			_format_int_comp(&buf[lstrlen(buf)], (int)(char)(pentax->AutoBracket[1] & 0xff));
			pec->AddSubItem(_T("拡張ブラケット"), buf);
		}
	}

	// 撮影モード
	if (HasInt(pentax->ShootMode[0])) {
		_format_lookup(buf, (pentax->ShootMode[0] << 8) | pentax->ShootMode[1], ekPentaxShootMode, -1);
		pec->AddItem(emShootMode, buf);
	}

	// ドライブモード
	if (HasInt(pentax->DriveMode[0])) {
		const lkup ekPentaxDriveMode[] = {
			0,		euSingleShot,
			1,		euContinuosShot,
			2,		_T("連続撮影 (HI)"),
			INT_MAX,NULL,
		};
		_format_lookup(buf, pentax->DriveMode[0], ekPentaxDriveMode, -1);
		pec->AddItem(emDriveMode, buf);

		const lkup ekPentaxSelfTimer[] = {
			0,		euNone,
			1,		_T("12秒"),
			2,		_T("2秒"),
			INT_MAX,NULL,
		};
		_format_lookup(buf, pentax->DriveMode[1], ekPentaxSelfTimer, -1);
		pec->AddSubItem(euSelfTimer, buf);

		const lkup ekPentaxRelease[] = {
			0,		_T("シャッターボタン"),
			1,		_T("リモートコントロール（3秒ディレイ）"),
			2,		_T("リモートコントロール"),
			INT_MAX,NULL,
		};
		_format_lookup(buf, pentax->DriveMode[2], ekPentaxRelease, -1);
		pec->AddSubItem(_T("レリーズ"), buf);

		const lkup ekPentaxExposure[] = {
			0,		_T("シングル露出"),
			1,		_T("多重露出"),
			INT_MAX,NULL,
		};
		_format_lookup(buf, pentax->DriveMode[3], ekPentaxExposure, -1);
		pec->AddSubItem(_T("露出"), buf);
	}

	// ピクチャーモード
	if (HasInt(pentax->PictureMode)) {
		_format_lookup(buf, pentax->PictureMode, ekPentaxPictureMode, -1);
		pec->AddItem(_T("ピクチャーモード"), buf);
	}

	// フォーカスモード
	if (HasInt(pentax->FocusMode)) {
		_format_lookup(buf, pentax->FocusMode, ekPentaxFocusMode, -1);
		pec->AddItem(emFocusMode, buf);
	}

	// ホワイトバランス
	if (HasInt(pentax->WhiteBalance)) {
		_format_lookup(buf, pentax->WhiteBalance, ekPentaxWhiteBalanceMode, -1);
		pec->AddItem(emWhiteBalance, buf);
	}

	// デジタルズーム
	if (HasInt(pentax->DigiatlZoom) && pentax->DigiatlZoom > 100) {
		_format_scale(buf, (float)pentax->DigiatlZoom / 100.0F);
		pec->AddItem(emDigitalZoom, buf);
	}

	// フラッシュモード
	if (HasInt(pentax->FlashMode)) {
		_format_lookup(buf, pentax->FlashMode, ekPentaxFlashMode, -1);
		pec->AddItem(emFlashSetting, buf);
	}

	// 測距点選択
	if (HasInt(pentax->AFPointSelected)) {
		_format_lookup(buf, pentax->AFPointSelected, ekPentaxAFPointSelected, -1);
		pec->AddItem(_T("測距点選択"), buf);
	}

	// AFエリア
	if (HasInt(pentax->AFPointsInFocus)) {
		_format_lookup(buf, pentax->AFPointsInFocus, ekPentaxAFPointsInFocus, -1);
		pec->AddItem(emAFArea, buf);
	}

	// 彩度
	if (HasInt(pentax->Saturation)) {
		_format_lookup(buf, pentax->Saturation, ekPentaxSCH, -1);
		pec->AddItem(emSaturation, buf);
	}

	// コントラスト
	if (HasInt(pentax->Contrast)) {
		_format_lookup(buf, pentax->Contrast, ekPentaxSCH, -1);
		pec->AddItem(emContrast, buf);
	}

	// シャープネス
	if (HasInt(pentax->Sharpness)) {
		_format_lookup(buf, pentax->Sharpness, ekPentaxSCH, -1);
		pec->AddItem(emSharpness, buf);
	}

	// カラースペース
	if (HasInt(pentax->ColorSpace)) {
		_format_lookup(buf, pentax->ColorSpace, ekPentaxColorSpace, -1);
		pec->AddItem(emColorSpace, buf);
	}

	// レンズ情報
	if (HasInt(pentax->LensInfo)) {
		_format_lookup(buf, pentax->LensInfo, ekPentaxLensInfo, -1);
		pec->AddItem(emLensInfo, buf);
	}

	// 感度調整
	if (HasInt(pentax->SensitivityAdjust)) {
		_format_float_comp(buf, (float)(pentax->SensitivityAdjust - 50) / 10.0F);
		pec->AddItem(_T("感度調整"), buf);
	}

	// デジタルフィルタ適用回数
	if (HasInt(pentax->DigitalFilterCount)) {
		_format_int(buf, pentax->DigitalFilterCount);
		pec->AddItem(_T("デジタルフィルタ適用回数"), buf);
	}

	// カメラ温度
	if (HasInt(pentax->CameraTemperature)) {
		_format_int(buf, pentax->CameraTemperature, _T("℃"));
		pec->AddItem(_T("カメラ温度"), buf);
	}

	// AEロック
	if (HasInt(pentax->AELock)) {
		_format_lookup(buf, pentax->AELock, ekOffOn, -1);
		pec->AddItem(_T("AEロック"), buf);
	}

	// ノイズリダクション
	if (HasInt(pentax->NoiseReduction)) {
		_format_lookup(buf, pentax->NoiseReduction, ekOffOn, -1);
		pec->AddItem(emNoiseReduction, buf);
	}

	// フラッシュ露出調整
	if (HasInt(pentax->FlashExposureComp)) {
		_format_ev_comp(buf, (float)pentax->FlashExposureComp / 256.0F);
		pec->AddItem(_T("フラッシュ露出調整"), buf);
	}

	// 画像仕上げ
	if (HasInt(pentax->PhotoFinish)) {
		_format_lookup(buf, pentax->PhotoFinish, ekPentaxPhotoFinish, -1);
		LPCTSTR p = (IS_PENTAX_CUSTOMIMAGE(model_id)) ? _T("カスタムイメージ") : emPhotoFinish;
		pec->AddItem(p, buf);
	}

	// 色温度
	if (HasInt(pentax->ColorTemperature) && pentax->ColorTemperature != 0) {
		_format_int(buf, 53190 - pentax->ColorTemperature);
		pec->AddItem(emColorTemp, buf);
	}

	// シェイクリダクション情報
	if (HasInt(*(int*)pentax->SRInfo)) {
		// offset 1: ShakeReduction
		const lkup ekPentaxShakeReduction[] = {
			0,		euOff,
			1,		euOn,
			4,		euOff,
			7,		euOn,
			INT_MAX,NULL,
		};
		_format_lookup(buf, pentax->SRInfo[1], ekPentaxShakeReduction, -1);
		pec->AddItem(_T("シェイクリダクション"), buf);

		// offset 0: SRResult
		const lkup ekPentaxSRResult[] = {
			0x00,	_T("未補正"),
			0x01,	_T("完了"),
			0x40,	_T("無効"),
			0x41,	_T("補正中"),
			INT_MAX,NULL,
		};
		_format_lookup(buf, pentax->SRInfo[0], ekPentaxSRResult, -1);
		pec->AddSubItem(_T("SR状態"), buf);

		// offset 2: SRHalfPressTime
		float f = (float)pentax->SRInfo[2] / 60.0F;
		_format_float(buf, f, _T("秒"));
		pec->AddSubItem(_T("SRシャッター半押し時間"), buf);

		// offset 3: SRFocalLength
		int val = (pentax->SRInfo[3] & 1) ? (pentax->SRInfo[3] << 2) : (pentax->SRInfo[3] >> 1);
		_format_int(buf, val, _T("mm"));
		pec->AddSubItem(_T("SR焦点距離"), buf);
	}

	// 撮影枚数
	if (HasInt(pentax->TakeCount) && pentax->TakeCount > 0) {
		_format_int(buf, pentax->TakeCount);
		pec->AddItem(emTotalTakeCount, buf);
	}

	// シャッターカウント
	if (HasInt(pentax->ShutterCount) && HasInt(pentax->ShootingDate) && HasInt(pentax->ShootingTime)) {
		DWORD n = (DWORD)pentax->ShutterCount ^ (DWORD)pentax->ShootingDate ^ (0xffffffff - ((DWORD)pentax->ShootingTime << 8));
		_format_int(buf, n);
		pec->AddItem(emShutterCount, buf);
	}

	// ショット情報
	if (HasInt(*(int*)pentax->ShotInfo)) {
		// offset 00: PictureMode2
		const lkup ekPentaxPictureMode2[] = {
			0,		_T("シーンモード"),
			1,		_T("オートピクチャー"),
			2,		euProgramAE,
			3,		_T("グリーンモード"),
			4,		_T("シャッター優先オート"),
			5,		_T("絞り優先オート"),
			6,		_T("プログラムシフト(Tv)"),
			7,		_T("プログラムシフト(Av)"),
			8,		euManual,
			9,		euBulb,
			10,		_T("絞り優先（Aポジション以外）"),
			11,		_T("マニュアル（Aポジション以外）"),
			12,		_T("バルブ（Aポジション以外）"),
			13,		_T("シャッター＆絞り優先"),
			15,		_T("感度優先"),
			16,		_T("ストロボ同調速度"),
			INT_MAX,NULL,
		};
		_format_lookup(buf, pentax->ShotInfo[0], ekPentaxPictureMode2, -1);
		pec->AddItem(_T("ショット情報"), buf);

		// offset 06: ISOFloor (manual ISO or minimum ISO in Auto ISO mode)
		const lkup_val evPentaxISOFloor[] = {
			32,		100,
			35,		125,
			36,		140,
			37,		160,
			40,		200,
			43,		250,
			44,		280,
			45,		320,
			48,		400,
			51,		500,
			52,		560,
			53,		640,
			56,		800,
			59,		1000,
			60,		1100,
			61,		1250,
			64,		1600,
			67,		2000,
			68,		2200,
			69,		2500,
			72,		3200,
			75,		4000,
			76,		4400,
			77,		5000,
			80,		6400,
//			83,		8000,
//			84,		8800,
//			85,		10000,
//			88,		12800,
			INT_MAX,INT_MAX,
		};
		int val = _lookup_val(pentax->ShotInfo[6], evPentaxISOFloor);
		if (val != INT_MAX)
			_format_int(buf, val);
		else
			lstrcpy(buf, euUndefine);
		pec->AddSubItem(emISO, buf);

		// offset 07: DriveMode2

/*
            0 => 'Single-frame',
            BITMASK => {
                0 => 'Continuous',
                3 => 'Self-timer (2 sec)', #PH
                4 => 'Remote Control (3 sec delay)',
                5 => 'Remote Control',
                # have seen bit 6 set when DriveMode was
                # "Continuous" or "Single-frame" - PH
                # I got bit 6 set when "Single-frame", Exposure Bracketing
                # was On and Flash was On (haven't tried with Flash Off) - DaveN
                7 => 'Multiple Exposure',
*/
		// offset 08: AEBStepSize
		_format_ev_comp(buf, _pentax_ev_conv(pentax->ShotInfo[8]));
		pec->AddSubItem(_T("AEBステップサイズ"), buf);

		// offset 09: BracketShotNumber
		if (pentax->ShotInfo[9])
			wsprintf(buf, _T("%d枚中の%d枚目"), (pentax->ShotInfo[9] & 0xf0) >> 4, pentax->ShotInfo[9] & 0x0f);
		else
			lstrcpy(buf, euUndefine);
		pec->AddSubItem(_T("ブラケットショット番号"), buf);

		// offset 13: RawAndJpgRecording
		buf[0] = 0;
		if (pentax->ShotInfo[13] & 0x04) lstrcat(buf, _T("PEF+"));
		if (pentax->ShotInfo[13] & 0x02) lstrcat(buf, _T("DNG+"));
		if (pentax->ShotInfo[13] & 0x08) lstrcat(buf, _T("DNG+"));		// ?
		if (pentax->ShotInfo[13] & 0x01) lstrcat(buf, _T("JPEG+"));
		buf[lstrlen(buf) - 1] = 0;

		switch (pentax->ShotInfo[13] & 0xf0) {
		case 0x00: lstrcat(buf, _T(" ★★★")); break;
		case 0x10: lstrcat(buf, _T(" ★★")); break;
		case 0x20: lstrcat(buf, _T(" ★")); break;
		}
		pec->AddSubItem(_T("画像の保存形式"), buf);

		// offset 20: SvISOSetting - ISO setting for sensitivity-priority mode
		val = _lookup_val(pentax->ShotInfo[20], evPentaxISOFloor);
		if (val != INT_MAX)
			_format_int(buf, val);
		else
			lstrcpy(buf, euUndefine);
		pec->AddSubItem(_T("感度優先モード時のISO"), buf);

		// offset 21: BaseExposureCompensation
		if (pentax->ShotInfo[21])
			_format_ev_comp(buf, _pentax_ev_conv(64 - (int)(char)pentax->ShotInfo[21]));
		else
			lstrcpy(buf, euUndefine);
		pec->AddSubItem(_T("基準露出補正値"), buf);
	}

/* --------------------
	// AE情報
	if (HasInt(*(int*)pentax->AEInfo)) {
		// offset 00: AEExposureTime
        Notes => 'val = 24 * 2**((32-raw)/8)',
        ValueConv => '24*exp(-($val-32)*log(2)/8)',

		// offset 01: AEAperture
        Notes => 'val = (2**((raw-4)/16)) / 16',
        # (same as val=2**((raw-68)/16), but 68 isn't a nice power of 2)
        ValueConv => 'exp(($val-4)*log(2)/16)/16',
        PrintConv => 'sprintf("%.1f",$val)',

		// offset 02: AE_ISO
        Notes => 'val = 100 * 2**((raw-32)/8)',
        ValueConv => '100*exp(($val-32)*log(2)/8)',
        PrintConv => 'int($val + 0.5)',

		// offset 03: AEXv
        Notes => 'val = (raw-64)/8',
        ValueConv => '($val-64)/8',

		// offset 04: AEBXv
        Format => 'int8s',
        Notes => 'val = raw / 8',
        ValueConv => '$val/8',

		// offset 05: AEFlashTv
        Notes => 'val = 24 * 2**((32-raw)/8)',
        ValueConv => '24*exp(-($val-32)*log(2)/8)', #19
        PrintConv => 'Image::ExifTool::Exif::PrintExposureTime($val)',

		// offset 06: AEProgramMode
            0 => 'M, P or TAv',
            1 => 'Av, B or X',
            2 => 'Tv',
            3 => 'Sv or Green Mode',
            8 => 'Hi-speed Program',
            16 => 'DOF Program', #20
            24 => 'MTF Program', #20
            35 => 'Standard',
            43 => 'Portrait',
            51 => 'Landscape',
            59 => 'Macro',
            67 => 'Sport',
            75 => 'Night Scene Portrait',
            83 => 'No Flash',
            91 => 'Night Scene',
            99 => 'Surf & Snow',
            107 => 'Text',
            115 => 'Sunset',
            123 => 'Kids',
            131 => 'Pet',
            139 => 'Candlelight',
            147 => 'Museum',
	}
-------------- */

	// カメラが計算した光量
	if (HasInt(pentax->EffectiveLightValue)) {
		_format_float(buf, (float)pentax->EffectiveLightValue / 1024.0F);
		pec->AddItem(_T("計測した光量値"), buf);
	}

	// イメージ加工
	if (HasInt(pentax->ImageProcessing)) {
		if (HIWORD(pentax->ImageProcessing) == 0)
			pentax->ImageProcessing = 0;
		const lkup ekPentaxImageProcessing[] = {
			0x00000000,	_T("未加工"),
			0x00000004,	_T("デジタルフィルタ"),
			0x04000000,	_T("カラーフィルタ"),
			0x10000000,	_T("フレーム合成"),
			0x12000000,	_T("フレーム合成"),
			0x14000000,	_T("フレーム合成"),
			INT_MAX,NULL,
		};
		_format_lookup(buf, pentax->ImageProcessing, ekPentaxImageProcessing, -1);
		pec->AddItem(_T("イメージ加工"), buf);
	}

	// 地域
	if (HasInt(pentax->HometownCity)) {
		_format_lookup(buf, pentax->HometownCity, ekPentaxHometownCity, -1);
		pec->AddItem(_T("地域の設定"), buf);
	}

	// カメラ情報
	if (HasInt(pentax->CameraInfo[0])) {
		// offset 0: PentaxModelID
		_format_int(buf, pentax->CameraInfo[0]);
		pec->AddItem(_T("カメラ情報(ID)"), buf);

		// offset 1: ManufactureDate
		int val = pentax->CameraInfo[1];
		if (pentax->CameraInfo[0] == 0x12b6a || pentax->CameraInfo[0] == 0x12c46) {
			// Optio A10 and A20 leave "200" off the year
			val -= 200;
		}
		_format_int(buf, val);
		pec->AddSubItem(_T("カメラ製造年月日"), buf);

		// offset 2 - 3: ModelRevision ULONG x 2
		// offset 4: InternalSerialNumber
		_format_int(buf, pentax->CameraInfo[4]);
		pec->AddSubItem(_T("製造番号"), buf);
	}

	return 1;
}

static int enuminfo_sigma(sigma_t* sigma, CExifCallback* pec)
{
	TCHAR buf[256];

	// 露出時間
	if (HasFlt(sigma->ExposureTime)) {
		_format_shutter(buf, sigma->ExposureTime);
		pec->AddItem(emExposureTime, buf);
	}

	// 絞り値
	if (HasFlt(sigma->FNumber)) {
		_format_Fnumber(buf, sigma->FNumber);
		pec->AddItem(emAperture, buf);
	}

	// 露出補正
	if (HasFlt(sigma->ExposureCompensation)) {
		_format_ev_comp(buf, sigma->ExposureCompensation);
		pec->AddItem(emExpComp, buf);
	}

	// シリアル番号
	if (HasStr(sigma->Serial)) {
		_format_ascii(buf, sigma->Serial, ARRAYSIZE(buf));
		pec->AddItem(emSerialNumber, buf);
	}

	// ドライブモード
	if (HasStr(sigma->DriveMode)) {
		_format_string_lookup(buf, sigma->DriveMode, etSigmaDriveMode);
		pec->AddItem(emDriveMode, buf);
	}

	// 解像度
	if (HasStr(sigma->Resolution)) {
		_format_ascii(buf, sigma->Resolution, ARRAYSIZE(buf));
		pec->AddItem(_T("解像度"), buf);
	}

	// AFモード
	if (HasStr(sigma->AFMode)) {
		_format_ascii(buf, sigma->AFMode, ARRAYSIZE(buf));
		pec->AddItem(emAFMode, buf);
	}

	// フォーカスモード
	if (HasStr(sigma->FocusMode)) {
		_format_ascii(buf, sigma->FocusMode, ARRAYSIZE(buf));
		pec->AddItem(emFocusMode, buf);
	}

	// AFエリア
	if (HasStr(sigma->AFArea)) {
		_format_ascii(buf, sigma->AFArea, ARRAYSIZE(buf));
		pec->AddItem(emAFArea, buf);
	}

	// AFINFOCUS
	if (HasStr(sigma->AFInFocus)) {
		_format_ascii(buf, sigma->AFInFocus, ARRAYSIZE(buf));
		pec->AddItem(_T("AF-In-Focus"), buf);
	}

	// ホワイトバランスモード
	if (HasStr(sigma->WhiteBalanceMode)) {
		_format_ascii(buf, sigma->WhiteBalanceMode, ARRAYSIZE(buf));
		pec->AddItem(emWhiteBalance, buf);
	}

	// 撮影モード
	if (HasStr(sigma->ShootMode)) {
		_format_string_lookup(buf, sigma->ShootMode, etSigmaShootMode);
		pec->AddItem(emShootMode, buf);
	}

	// 測光モード
	if (HasStr(sigma->MeteringMode)) {
		_format_string_lookup(buf, sigma->MeteringMode, etSigmaMeteringMode);
		pec->AddItem(emMeteringMode, buf);
	}

	// レンズ焦点範囲
	if (HasStr(sigma->FocalLengthRange)) {
		_format_ascii(buf, sigma->FocalLengthRange, ARRAYSIZE(buf));
		pec->AddItem(_T("レンズ焦点範囲"), buf);
	}

	// カラースペース
	if (HasStr(sigma->ColorSpace)) {
		_format_ascii(buf, sigma->ColorSpace, ARRAYSIZE(buf));
		pec->AddItem(emColorSpace, buf);
	}

	// 階調
	if (1) {
		pec->AddItem(_T("階調"), NULL);

		// 露出補正
		if (HasStr(sigma->ExpCompensation)) {
			_format_ascii(buf, sigma->ExpCompensation, ARRAYSIZE(buf));
			pec->AddSubItem(_T("露出"), buf);
		}
		// コントラスト
		if (HasStr(sigma->Contrast)) {
			_format_ascii(buf, sigma->Contrast, ARRAYSIZE(buf));
			pec->AddSubItem(emContrast, buf);
		}
		// シャドウ
		if (HasStr(sigma->Shadow)) {
			_format_ascii(buf, sigma->Shadow, ARRAYSIZE(buf));
			pec->AddSubItem(_T("シャドウ"), buf);
		}
		// ハイライト
		if (HasStr(sigma->Hilight)) {
			_format_ascii(buf, sigma->Hilight, ARRAYSIZE(buf));
			pec->AddSubItem(_T("ハイライト"), buf);
		}
		// 彩度
		if (HasStr(sigma->Saturation)) {
			_format_ascii(buf, sigma->Saturation, ARRAYSIZE(buf));
			pec->AddSubItem(emSaido, buf);
		}
		// シャープネス
		if (HasStr(sigma->Sharpness)) {
			_format_ascii(buf, sigma->Sharpness, ARRAYSIZE(buf));
			pec->AddSubItem(emSharpness, buf);
		}
		// X3 Fill Light
		if (HasStr(sigma->X3FillLight)) {
			_format_ascii(buf, sigma->X3FillLight, ARRAYSIZE(buf));
			pec->AddSubItem(_T("X3 Fill Light"), buf);
		}
	}

	// カラー調整
	if (HasStr(sigma->ColorAdjust)) {
		_format_ascii(buf, sigma->ColorAdjust, ARRAYSIZE(buf));
		pec->AddItem(_T("カラー調整"), buf);
	}

	// 調整モード
	if (HasStr(sigma->AdjustMode)) {
		_format_ascii(buf, sigma->AdjustMode, ARRAYSIZE(buf));
		pec->AddItem(_T("調整モード"), buf);
	}

	// 画質
	if (HasStr(sigma->Quality)) {
		_format_ascii(buf, sigma->Quality, ARRAYSIZE(buf));
		pec->AddItem(emQualityMode, buf);
	}

	// ファームウェア
	if (HasStr(sigma->Firmware)) {
		_format_ascii(buf, sigma->Firmware, ARRAYSIZE(buf));
		pec->AddItem(emFirmware, buf);
	}

	// ソフトウェア
	if (HasStr(sigma->Software)) {
		_format_ascii(buf, sigma->Software, ARRAYSIZE(buf));
		pec->AddItem(emSoftware, buf);
	}

	// オートブラケット
	if (HasStr(sigma->AutoBracket)) {
		_format_ascii(buf, sigma->AutoBracket, ARRAYSIZE(buf));
		pec->AddItem(_T("オートブラケット"), buf);
	}
	return 1;
}

static int enuminfo_epson(epson_t* epson, CExifCallback* pec)
{
	TCHAR buf[256];

	// 画質モード
	if (HasInt(epson->Quality)) {
		_format_lookup(buf, epson->Quality, ekEpsonQuality, -1);
		pec->AddItem(emQualityMode, buf);
	}

	// カメラID
	if (HasStr(epson->CameraID)) {
		_format_ascii(buf, epson->CameraID, ARRAYSIZE(buf));
		pec->AddItem(_T("カメラID"), buf);
	}

	// 画像幅と高さ
	if (HasInt(epson->ImageWidth) && HasInt(epson->ImageHeight)) {
		_format_xy(buf, epson->ImageWidth, epson->ImageHeight);
		pec->AddItem(emImageSize, buf);
	}

	// ハードウェアタイプ
	if (HasStr(epson->HardwareType)) {
		_format_ascii(buf, epson->HardwareType, ARRAYSIZE(buf));
		pec->AddItem(emHardwareType, buf);
	}

	return 1;
}

static int enuminfo_sony(sony_t* sony, int model_id, CExifCallback* pec)
{
	TCHAR buf[256];
	float f;

	if (HasInt(sony->QualityMode)) {
		_format_lookup(buf, sony->QualityMode, ekSonyQualityMode, -1);
		pec->AddItem(emQualityMode, buf);
	}

	if (model_id < SONY_DSLR_ALPHA) {
		// 色再現モード
		if (HasStr(sony->ColorReproduction)) {
			_format_ascii(buf, sony->ColorReproduction, ARRAYSIZE(buf));
			pec->AddItem(_T("色再現モード"), buf);
		}

		// 調光補正
		if (HasInt(sony->FlashComp) && sony->FlashComp != 0) {
			_format_int_comp(buf, sony->FlashComp);
			pec->AddItem(emFlashComp, buf);
		}
	}
	else {
		// 色再現モード
		if (HasInt(sony->ColorReproductionMode)) {
			_format_lookup(buf, sony->ColorReproductionMode, ekSonyColorReproductionMode, -1);
			pec->AddItem(_T("色再現モード"), buf);
		}
	}

	// 露出モード
	if (HasInt(sony->ExposureMode)) {
		_format_lookup(buf, sony->ExposureMode, ekSonyCyberExpMode, -1);
		pec->AddItem(_T("シーンセレクション"), buf);
	}
	else if (HasInt(sony->mltExposureMode)) {
		_format_lookup(buf, sony->mltExposureMode, ekSonyExposureMode, -1);
		pec->AddItem(emExposureMode, buf);
	}

	// シーンモード
	if (HasInt(sony->SceneMode) && sony->SceneMode > 0) {
		_format_lookup(buf, sony->SceneMode, ekSonySceneMode, -1);
		pec->AddItem(emSceneMode, buf);
	}

	// クリエイティブスタイル
	if (HasInt(sony->CreativeStyle)) {
		// MINOLTAのものと同一
		_format_lookup(buf, sony->CreativeStyle, ekSonyCreativeStyle, -1);
		pec->AddItem(_T("クリエイティブスタイル"), buf);
	}

	// 露出時間
	if (HasInt(sony->mltExposureTime)) {
		f = powf(2.0f, ((48.0F - sony->mltExposureTime) / 8.0F));
		_format_shutter(buf, f);
		pec->AddItem(emExposureTime, buf);
	}

	// 絞り
	if (HasInt(sony->mltAperture)) {
		f = powf(2.0f, (float)(sony->mltAperture - 8) / 16.0F);
		_format_Fnumber(buf, f);
		pec->AddItem(emAperture, buf);
	}

	// リリースモード
	if (HasInt(sony->ReleaseMode)) {
		_format_lookup(buf, sony->ReleaseMode, ekSonyReleaseMode, -1);
		pec->AddItem(_T("リリースモード"), buf);
	}

	// 露出補正
	if (HasInt(sony->mltExpCompensation)) {
		if (model_id >= SONY_DSLR_A200)		// A200, A700
			_format_ev_comp(buf, (float)(sony->mltExpCompensation - 0x80) / 24.0F);
		else
			_format_ev_comp(buf, (float)sony->mltExpCompensation / 100.0F - 3.0F);
		pec->AddItem(emExpComp, buf);
	}

	// ホワイトバランス
	if (HasInt(sony->WhiteBalance)) {
		_format_lookup(buf, sony->WhiteBalance, ekSonyWhiteBalance, -1);
		// ホワイトバランス補正
		if ((sony->WhiteBalance != 0) &&
			HasInt(sony->WhiteBalanceFineTune) &&
			sony->WhiteBalanceFineTune != 0) {
			TCHAR tmp[64];
			_format_int_comp(tmp, sony->WhiteBalanceFineTune);
			wsprintf(&buf[lstrlen(buf)], _T(" (%s)"), tmp);
		}
		pec->AddItem(emWhiteBalance, buf);
	}
	else if (HasInt(sony->mltWhiteBalance)) {
		_format_lookup(buf, sony->mltWhiteBalance, ekSonyWhiteBalance, -1);
		// ホワイトバランス補正
		if ((sony->mltWhiteBalance != 0) &&
			HasInt(sony->mltWhiteBalanceTune) &&
			sony->mltWhiteBalanceTune != 0) {
			TCHAR tmp[64];
			_format_int_comp(tmp, sony->mltWhiteBalanceTune);
			wsprintf(&buf[lstrlen(buf)], _T(" (%s)"), tmp);
		}
		pec->AddItem(emWhiteBalance, buf);
	}

	// フォーカスモード
	if (HasInt(sony->FocusMode)) {
		_format_lookup(buf, sony->FocusMode, ekSonyFocusMode, -1);
		pec->AddItem(emFocusMode, buf);
	}
	else if (HasInt(sony->mltFocusMode)) {
		_format_lookup(buf, sony->mltFocusMode, ekSonyFocusModeA100, -1);
		pec->AddItem(emFocusMode, buf);
	}

	// オートフォーカスエリア
	if (HasInt(sony->AutoFocusArea)) {
		_format_lookup(buf, sony->AutoFocusArea, ekSonyAutoFocusArea, -1);
		pec->AddItem(_T("オートフォーカスエリア"), buf);
	}

	// 測光方式
	if (HasInt(sony->mltMeteringMode)) {
		_format_lookup(buf, sony->mltMeteringMode, ekSonyMeteringMode, -1);
		pec->AddItem(emMeteringMode, buf);
	}

	// 撮像感度
	if (HasInt(sony->mltFilmSpeed)) {
		if (!_format_lookup(buf, sony->mltFilmSpeed, ekSonyISOSetting, -1)) {
			f = powf(2.0F, ((float)sony->mltFilmSpeed / 8.0F - 1.0F)) * 3.125F;
			_format_int(buf, (int)floorf(f + 0.5F));
		}
		pec->AddItem(_T("撮像感度"), buf);
	}

	// コントラスト調整
	if (HasInt(sony->Contrast)) {
		_format_int_comp(buf, sony->Contrast);
		pec->AddItem(emContrast, buf);
	}
	else if (HasInt(sony->mltContrast)) {
		_format_int_comp(buf, sony->mltContrast - 10);
		pec->AddItem(emContrast, buf);
	}

	// 彩度調整
	if (HasInt(sony->Saturation)) {
		_format_int_comp(buf, sony->Saturation);
		pec->AddItem(emSaido, buf);
	}
	else if (HasInt(sony->mltSaturation)) {
		_format_int_comp(buf, sony->mltSaturation - 10);
		pec->AddItem(emSaido, buf);
	}

	// 色空間
	if (HasInt(sony->mltColorSpace)) {
		_format_lookup(buf, sony->mltColorSpace, ekSonyColorSpace, -1);
		pec->AddItem(emColorSpace, buf);
	}

	// シャープネス
	if (HasInt(sony->Sharpness)) {
		_format_int_comp(buf, sony->Sharpness);
		pec->AddItem(emSharpness, buf);
	}
	else if (HasInt(sony->mltSharpness)) {
		_format_int_comp(buf, sony->mltSharpness);
		pec->AddItem(emSharpness, buf);
	}

	// 明るさ
	if (HasInt(sony->Brightness)) {
		_format_int_comp(buf, sony->Brightness);
		pec->AddItem(emBrightness, buf);
	}

	// 色合い調整
/* ---------
	if (HasInt(sony->mltHue)) {
		_format_int_comp(buf, sony->mltHue - 10);
		pec->AddItem(emColorTone, buf);
	}
---------- */

	// メータードマニュアル
/* ---------
	if (HasInt(sony->MeteredManual)) {
		f = (float)(sony->MeteredManual - 0x80);
		_format_ev_comp(buf, f / 24.0F);
		pec->AddItem(_T("メータードマニュアル"), buf);
	}
---------- */

	// 画質仕上げ
	if (HasInt(sony->mltPhotoFinish)) {
		// Alphaシリーズは別のエントリで持っているので、それが無かったらこれを使う
		if (!HasInt(sony->CreativeStyle)) {
			_format_lookup(buf, sony->mltPhotoFinish, ekSonyCreativeStyle, -1);
			pec->AddItem(emPhotoFinish, buf);
		}
	}

	// 画質モード
	if (HasInt(sony->mltQualityMode)) {
		_format_lookup(buf, sony->mltQualityMode, ekSonyQualityMode, -1);
		pec->AddItem(emQualityMode, buf);
	}

	// ゾーン切替
	if (HasInt(sony->mltZoneMatching)) {
		_format_lookup(buf, sony->mltZoneMatching, ekSonyZoneMatching, -1);
		pec->AddItem(_T("ゾーン切り替え"), buf);
	}

	// 色温度
	int val = INT_MAX;
	if (HasInt(sony->mltColorTemp) && sony->mltColorTemp)
		val = sony->mltColorTemp;
	if (HasInt(sony->ColorTemp) && sony->ColorTemp)
		val = sony->ColorTemp;
	if (HasInt(val)) {
		_format_int(buf, val, _T("K"));
		pec->AddItem(emColorTemp, buf);
	}

	// Dレンジオプティマイザ
	if (HasInt(sony->DRangeOptmizer)) {
		_format_lookup(buf, sony->DRangeOptmizer, ekSonyDRangeOptmizer, -1);
		pec->AddItem(_T("Dレンジオプティマイザ"), buf);
	}

	// オートHDR
	if (HasInt(sony->HDR)) {
		_format_lookup(buf, sony->HDR, ekSonyHDR, -1);
		pec->AddItem(_T("オートHDR"), buf);
	}

	// レンズID
	if (HasInt(sony->LensID)) {
		_format_lookup(buf, sony->LensID, (sony->LensType == 0) ? ekSonyLensID : ekSonyLensID2, -1);
		pec->AddItem(emLensName, buf);
	}

	// テレコン
	if (HasInt(sony->TeleConverter) && sony->TeleConverter) {
		_format_lookup(buf, sony->TeleConverter, ekSonyTeleConverter, -1);
		pec->AddItem(_T("テレコン"), buf);
	}

	// マクロ
	if (HasInt(sony->MacroMode)) {
		_format_lookup(buf, sony->MacroMode, ekOffOn, -1);
		pec->AddItem(emMacroMode, buf);
	}

	// 手ブレ補正
	if (HasInt(sony->ImageStabilization)) {
		_format_lookup(buf, sony->ImageStabilization, ekOffOn, -1);
		pec->AddItem(emAntiShake, buf);
	}

	// 長秒時ノイズリダクション
	if (HasInt(sony->LongExposureNR)) {
		_format_lookup(buf, sony->LongExposureNR, ekOffOnAuto, -1);
		pec->AddItem(emLongExposureNR, buf);
	}

	// ノイズリダクション
	if (HasInt(sony->mltNoiseReduction)) {
		_format_lookup(buf, sony->mltNoiseReduction, ekOffOn, -1);
		pec->AddItem(emNoiseReduction, buf);
	}

	// 高感度ノイズリダクション
	if (HasInt(sony->HighISONR)) {
		_format_lookup(buf, sony->HighISONR, ekSonyHighISONR, -1);
		pec->AddItem(_T("高感度ノイズリダクション"), buf);
	}

	// マルチショットノイズリダクション
	if (HasInt(sony->MultiShotNR)) {
		_format_lookup(buf, sony->MultiShotNR, ekOffOn, -1);
		pec->AddItem(_T("マルチショットノイズリダクション"), buf);
	}

	return 1;
}

static int enuminfo_ricoh(ricoh_t* ricoh, CExifCallback* pec)
{
	TCHAR buf[256];

	// イメージサイズ
	if (HasInt(ricoh->ImageWidth) && HasInt(ricoh->ImageHeight)) {
		_format_xy(buf, ricoh->ImageWidth, ricoh->ImageHeight);
		pec->AddItem(emImageSize, buf);
	}

	// ホワイトバランス
	if (HasInt(ricoh->ISOSetting)) {
		_format_lookup(buf, ricoh->ISOSetting, ekRicohISOSetting, -1);
		pec->AddItem(emISOSetting, buf);
	}

	// ホワイトバランス
	if (HasInt(ricoh->WhiteBalance)) {
		_format_lookup(buf, ricoh->WhiteBalance, ekRicohWhiteBalance, -1);
		pec->AddItem(emWhiteBalance, buf);
	}

	// フラッシュモード
	if (HasInt(ricoh->FlashMode)) {
		_format_lookup(buf, ricoh->FlashMode, ekRicohFlashMode, -1);
		pec->AddItem(emFlashMode, buf);
	}

	// マクロモード
	if (HasInt(ricoh->MacroMode)) {
		_format_lookup(buf, ricoh->MacroMode, ekOffOn, -1);
		pec->AddItem(emMacroMode, buf);
	}

	// シャープネス
	if (HasInt(ricoh->Sharpness)) {
		_format_lookup(buf, ricoh->Sharpness, ekRicohSharpness, -1);
		pec->AddItem(emSharpness, buf);
	}

	// 色の濃さ
	if (HasInt(ricoh->Saturation)) {
		_format_lookup(buf, ricoh->Saturation, ekRicohSaturation, -1);
		if (HasInt(ricoh->ToneEffect) && ricoh->ToneEffect > 0) {
			TCHAR tmp[256];
			_format_lookup(tmp, ricoh->ToneEffect, ekRicohToneEffect, -1);
			wsprintf(buf + lstrlen(buf), _T("（調色：%s）"), tmp);
		}
		pec->AddItem(_T("画像設定"), buf);
	}

	return 1;
}

static int enuminfo_mamiya(mamiya_t* mamiya, CExifCallback* pec)
{
	TCHAR buf[256];

	// 露出モード
	if (HasInt(mamiya->ExposureMode)) {
		_format_lookup(buf, mamiya->ExposureMode, ekMamiyamExposureMode, -1);
		pec->AddItem(emExposureMode, buf);
	}

	// ISO
	if (HasInt(mamiya->ISO)) {
		if (mamiya->ISO)
			_format_int(buf, mamiya->ISO);
		else
			lstrcpy(buf, euAuto);
		pec->AddItem(emISOSetting, buf);
	}

	// ホワイトバランス
	if (HasInt(mamiya->WhiteBalance)) {
		_format_lookup(buf, mamiya->WhiteBalance, ekMamiyamWhiteBalance, -1);
		pec->AddItem(emWhiteBalance, buf);
	}

	// カラーモード
	if (HasInt(mamiya->ColorMode)) {
		_format_lookup(buf, mamiya->ColorMode, ekMamiyamColorMode, -1);
		pec->AddItem(emColorMode, buf);
	}

	// トーン
	if (HasInt(mamiya->Tone)) {
		_format_lookup(buf, mamiya->Tone, ekMamiyamTone, -1);
		pec->AddItem(_T("トーン"), buf);
	}

	return 1;
}

static int enuminfo_leica(leica_t* leica, int model_id, CExifCallback* pec)
{
	if (IS_LIECA_PANASONIC_COMPATIBLE(model_id)) {
		// panasonic 互換
		return enuminfo_panasonic((panasonic_t*)leica, pec);
	}

	TCHAR buf[256];

	// ホワイトバランス
	if (HasInt(leica->WhiteBalance)) {
		_format_lookup(buf, leica->WhiteBalance, ekLeicaWhiteBalance, -1);
		pec->AddItem(emWhiteBalance, buf);
	}

	// ホワイトバランス色温度
	if (HasInt(leica->WBColorTemp)) {
		_format_int(buf, leica->WBColorTemp, _T("K"));
		pec->AddItem(emColorTemp, buf);
	}

	// レンズ検出
	if (HasInt(leica->LensType)) {
		_format_lookup(buf, leica->LensType, ekLeicaLensType, -1);
		pec->AddItem(emLensInfo, buf);
	}

	// イメージ固有ID
	if (HasInt(leica->ImageID)) {
		_format_int(buf, leica->ImageID);
		pec->AddItem(_T("イメージID"), buf);
	}

	return 1;
}

int exfEnumInfo(LPCTSTR fname, CExifCallback* pec)
{
	if (!pec) return -1;

	TCHAR buf[MAX_PATH];
	LPTSTR p;
	exif_t* exif = new exif_t;
	if (!exif_parse(fname, NULL, exif, false)) {
		delete exif;
		return -1;
	}

	// Exif情報
	pec->AddGroup(_T("Exif情報"));

	// メーカー名
	if (HasStr(exif->maker)) {
		_format_ascii(buf, exif->maker, ARRAYSIZE(buf));
		pec->AddItem(_T("メーカー名"), buf);
	}

	// モデル名
	if (HasStr(exif->model)) {
		_format_ascii(buf, exif->model, ARRAYSIZE(buf));
		pec->AddItem(_T("モデル名"), buf);
	}

	// 撮影日時
	if (HasStr(exif->Date)) {
		_format_ascii(buf, exif->Date, ARRAYSIZE(buf));
		pec->AddItem(emDateTime, buf);
	}

	// デジタイス日時
/* --- 非表示（不要？）
	if (HasStr(exif->DateDigitized)) {
		_format_ascii(buf, exif->DateDigitized, ARRAYSIZE(buf));
		pec->AddItem(_T("デジタイズ日時"), buf);
	}
--- */

	// イメージサイズ
	if (HasInt(exif->ImageWidth) && HasInt(exif->ImageHeight)) {
		_format_xy(buf, exif->ImageWidth, exif->ImageHeight);
		pec->AddItem(emImageSize, buf);
	}

	// 露出時間
	if (HasFlt(exif->ExposureTime)) {
		_format_shutter(buf, exif->ExposureTime);
		pec->AddItem(emExposureTime, buf);
	}

	// F値
	if (HasFlt(exif->FNumber)) {
		_format_Fnumber(buf, exif->FNumber);
		pec->AddItem(_T("F値"), buf);
	}

	// シャッター速度
	if (HasFlt(exif->ShutterSpeed)) {
		if (ISFLOAT_ZERO(exif->ShutterSpeed) && exif->MakerID == MAKER_CANON)
			lstrcpy(buf, euBulb);
		else {
			_format_shutter(buf, exif->ShutterSpeed);
		}
		pec->AddItem(emShutterSpeed, buf);
	}

	// 絞り値
	if (HasFlt(exif->Aperture)) {
		_format_Fnumber(buf, exif->Aperture);
		pec->AddItem(emAperture, buf);
	}

	// 対象物の明るさ
	if (HasFlt(exif->BrightnessValue)) {
		_format_ev_comp(buf, exif->BrightnessValue);
		pec->AddItem(emBrightness, buf);
	}

	// 露出補正
	if (HasFlt(exif->ExposureBias)) {
		_format_ev_comp(buf, exif->ExposureBias);
		pec->AddItem(emExpComp, buf);
	}

	// 開放F値
/* --- 非表示（不要？）
	if (HasFlt(exif->MaxAperture)) {
		_format_Fnumber(buf, exif->MaxAperture);
		pec->AddItem(emMaxAperture, buf);
	}
--- */

	// ISO感度
	if (HasInt(exif->ISOEquiv)) {
		if (exif->ISOEquiv != 0)
			_format_int(buf, exif->ISOEquiv);
		else
			lstrcpy(buf, euAuto);
		pec->AddItem(emISO, buf);
	}

	// 光源
	if (HasInt(exif->LightSource)) {
		_format_lookup(buf, exif->LightSource, ekLightSource, -1);
		pec->AddItem(emLightSource, buf);
	}

	// 焦点距離
	if (HasFlt(exif->FocalLength)) {
		_format_float_int(buf, exif->FocalLength, _T("mm"));
		pec->AddItem(emFocalLength, buf);
	}

	// 焦点距離(35mm換算)
	if (HasFlt(exif->FocalLength35)) {
		_format_float_int(buf, exif->FocalLength35, _T("mm"));
		pec->AddItem(_T("焦点距離(35mm)"), buf);
	}

	// 被写体距離
	if (HasFlt(exif->SubjectDistance)) {
		_format_float2(buf, exif->SubjectDistance, _T("m"));
		pec->AddItem(emTargetDistance, buf);
	}

	// フラッシュ
	if (HasInt(exif->Flash)) {
		_format_lookup(buf, exif->Flash, ekFlashFired, -1, _T("予約"));
		pec->AddItem(euFlash, buf);
	}

	// 露出制御プログラム
	if (HasInt(exif->ExposureProgram)) {
		_format_lookup(buf, exif->ExposureProgram, ekExposureProgram, -1);
		pec->AddItem(emExposureProgram, buf);
	}

	// 測光方式
	if (HasInt(exif->MeteringMode)) {
		_format_lookup(buf, exif->MeteringMode, ekMeteringMode, -1);
		pec->AddItem(emMeteringMode, buf);
	}

	// 露光モード
	if (HasInt(exif->ExposureMode)) {
		_format_enumtext(buf, exif->ExposureMode, euExposureMode, 3);
		pec->AddItem(emShootMode, buf);
	}

	// ホワイトバランスモード
	if (HasInt(exif->WhiteBalance)) {
		_format_enumtext(buf, exif->WhiteBalance, euWhiteBalanceMode, 2);
		pec->AddItem(emWhiteBalance, buf);
	}

	// デジタルズーム
	if (HasFlt(exif->DigitalZoom) && exif->DigitalZoom > 1.0F) {
		_format_scale(buf, exif->DigitalZoom);
		pec->AddItem(emDigitalZoom, buf);
	}

	// 撮影シーンタイプ
	if (HasInt(exif->SceneCaptureType)) {
		_format_enumtext(buf, exif->SceneCaptureType, euSceneCaptureType, 4);
		pec->AddItem(_T("撮影シーンタイプ"), buf);
	}

	// ゲインコントロール
	if (HasInt(exif->GainControl)) {
		_format_enumtext(buf, exif->GainControl, euGainControl, 5);
		pec->AddItem(_T("ゲインコントロール"), buf);
	}

	// コントラスト
	if (HasInt(exif->Contrast)) {
		_format_enumtext(buf, exif->Contrast, euHardSoft, 3);
		pec->AddItem(emContrast, buf);
	}

	// 彩度
	if (HasInt(exif->Saturation)) {
		_format_enumtext(buf, exif->Saturation, euHighLow, 3);
		pec->AddItem(emSaturation, buf);
	}

	// シャープネス
	if (HasInt(exif->Sharpness)) {
		_format_enumtext(buf, exif->Sharpness, euHardSoft, 3);
		pec->AddItem(emSharpness, buf);
	}

	// 被写体の距離範囲
	if (HasInt(exif->DistanceRange)) {
		_format_enumtext(buf, exif->DistanceRange, euDistanceRange, 4);
		pec->AddItem(_T("被写体の距離範囲"), buf);
	}

	// カラースペース
	if (HasInt(exif->ColorSpace)) {
		if (exif->ColorSpace == 1)
			p = _T("sRGB");
		else {
			wsprintf(buf, _T("sRGB以外(%d)"), exif->ColorSpace);
			p = buf;
		}
		pec->AddItem(_T("色の表現"), p);
	}

	// ガンマ値
	if (HasFlt(exif->GammaValue)) {
		_format_float2(buf, exif->GammaValue);
		pec->AddItem(_T("ガンマ値"), buf);
	}

	// ソフトウェア
	if (HasStr(exif->Software)) {
		_format_ascii(buf, exif->Software, ARRAYSIZE(buf));
		pec->AddItem(emSoftware, buf);
	}

	// 編集日時
/* --- 非表示（不要？）
	if (HasStr(exif->DateEdit)) {
		_format_ascii(buf, exif->DateEdit, ARRAYSIZE(buf));
		pec->AddItem(_T("編集日時"), buf);
	}
--- */

	// メーカーノート
	if (exif->exist_makenote) {
		pec->AddGroup(_T("メーカーノート"));
		switch (exif->MakerID) {
		case MAKER_CANON:		enuminfo_canon(&exif->canon, exif->ModelID, pec); break;
		case MAKER_NIKON:		enuminfo_nikon(&exif->nikon, exif->ModelID, pec); break;
		case MAKER_OLYMPUS:		enuminfo_olympus(&exif->olympus, exif->ModelID, pec); break;
		case MAKER_CASIO:		enuminfo_casio(&exif->casio, exif->ModelID, pec); break;
		case MAKER_MINOLTA:		enuminfo_minolta(&exif->minolta, exif->ModelID, pec); break;
		case MAKER_FUJI:		enuminfo_fuji(&exif->fuji, exif->ModelID, pec); break;
		case MAKER_PANASONIC:	enuminfo_panasonic(&exif->panasonic, pec); break;
		case MAKER_LEICA:		enuminfo_leica(&exif->leica, exif->ModelID, pec); break;
		case MAKER_SANYO:		enuminfo_sanyo(&exif->sanyo, pec); break;
		case MAKER_PENTAX:		enuminfo_pentax(&exif->pentax, exif->ModelID, pec); break;
		case MAKER_SIGMA:		enuminfo_sigma(&exif->sigma, pec); break;
		case MAKER_EPSON:		enuminfo_epson(&exif->epson, pec); break;
		case MAKER_SONY:		enuminfo_sony(&exif->sony, exif->ModelID, pec); break;
		case MAKER_RICOH:		enuminfo_ricoh(&exif->ricoh, pec); break;
		case MAKER_MAMIYA:		enuminfo_mamiya(&exif->mamiya, pec); break;
		}
	}

	// gps
	if (exif->gps.exist_gps) {
		gps_t& gps = exif->gps;
		pec->AddGroup(_T("GPS位置"));
		// GPS緯度
		if (HasFlt(gps.GPSLatitude[0])) {
			_format_latitude(buf, gps.GPSLatitude[0], gps.GPSLatitude[1], gps.GPSLatitude[2], gps.GPSLatitudeRef);
			pec->AddItem(_T("緯度"), buf);
		}
		if (HasFlt(gps.GPSLongitude[0])) {
			// GPS経度
			_format_latitude(buf, gps.GPSLongitude[0], gps.GPSLongitude[1], gps.GPSLongitude[2], gps.GPSLongitudeRef);
			pec->AddItem(_T("経度"), buf);
		}
		if (HasStr(gps.GPSSatellites)) {
			// 測位に使った衛星信号
			_format_ascii(buf, gps.GPSSatellites, ARRAYSIZE(buf));
			pec->AddItem(_T("人工衛星"), buf);
		}
		if (HasStr(gps.GPSMapDatum)) {
			// 測位に用いた地図データ
			_format_ascii(buf, gps.GPSMapDatum, ARRAYSIZE(buf));
			pec->AddItem(_T("地図データ"), buf);
		}
		if (HasStr(gps.GPSDateStamp)) {
			// GPS日時
			_format_ascii(buf, gps.GPSDateStamp, ARRAYSIZE(buf));
			pec->AddItem(_T("GPS日時"), buf);
		}
		if (HasFlt(gps.GPSTimeStamp[0])) {
			// RATIONALx3: GPS 時間（原子時計の時間）UTC
			wsprintf(buf, _T("%02u:%02u:%02u.%02u UTC"),
						(int)gps.GPSTimeStamp[0],
						(int)gps.GPSTimeStamp[1],
						(int)gps.GPSTimeStamp[2],
						(int)floorf(gps.GPSTimeStamp[2] * 100) % 100);
			pec->AddItem(_T("GPS時刻"), buf);
		}
	}
	delete exif;
	return 1;
}

#if 0

//@ if (HasInt(exif->ShootMode))
//@ if (HasInt(exif->MacroMode))
//@ if (HasInt(exif->Quality))
//@ if (HasInt(exif->FlashMode))
//@ if (HasInt(exif->DigitalZoom))

//@ if (HasFlt(exif->ShutterSpeed))

#endif

