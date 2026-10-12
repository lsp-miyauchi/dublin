////////////////////////////////////////////////////////////////////////
// exifparse.h

#pragma once
#ifndef __EXIFPARSE_H__
#define __EXIFPARSE_H__

#include <math.h>
#include <time.h>
#include <limits.h>
#include "typealloc.h"
#include "filemap.h"
#include "lspexifapi.h"
#include "util.h"
#include "exifdata.h"

#ifndef ISDIGIT
#define ISDIGIT(a)		(('0'<=(a))&&((a)<='9'))
#endif

////////////////////////////////////////////////////////////////////////
//

class CExifCallback
{
public:
	EXIFCBPROC m_proc;
	LPARAM m_param;

	CExifCallback(EXIFCBPROC proc = NULL, LPARAM param = NULL) : m_proc(proc), m_param(param) { }
	inline bool IsNull() { return (m_proc == NULL); }
	inline int AddGroup(LPCTSTR pszGroupName) { return (m_proc != NULL) ? m_proc(EXIFCB_GROUP, pszGroupName, NULL, m_param) : -1; }
	inline int AddItem(LPCTSTR pszItemName, LPCTSTR pszItemValue) { return (m_proc != NULL) ? m_proc(EXIFCB_ITEM, pszItemName, pszItemValue, m_param) : -1; }
	int AddSubItem(LPCTSTR pszItemName, LPCTSTR pszItemValue)
	{
		CString str = _T("　+ ");
		if (pszItemName) str += pszItemName;
		return (m_proc != NULL) ? m_proc(EXIFCB_ITEM, str, pszItemValue, m_param) : -1;
	}
};

///////////////////////////////////////////////////////////////////////
// TIFF and CIFF data blocks can be quite large.
// Display only the first DLEN bytes.

#ifndef DLEN
#define DLEN				768
#endif

#define WORDFLIP(a)			(((a)<<8)|((a)>>8))

#define M_LEAD				0xFF            // MARKER LEADER
#define M_SOI				0xD8            // Start Of Image (beginning of datastream)
#define M_EOI				0xD9            // End Of Image (end of datastream)
#define M_JFIF				0xE0            // Jfif marker
#define M_EXIF				0xE1            // Exif marker
#define DIB_HEADER_MARKER   ((WORD) ('M' << 8) | 'B')
#define WIDTHBYTES(bits)    (((bits) + 31) / 32 * 4)

#define FMT_BYTE			1
#define FMT_STRING			2
#define FMT_USHORT			3
#define FMT_ULONG			4
#define FMT_URATIONAL		5
#define FMT_SBYTE			6
#define FMT_UNDEFINED		7
#define FMT_SSHORT			8
#define FMT_SLONG			9
#define FMT_SRATIONAL		10
#define FMT_FLOAT			11
#define FMT_DOUBLE			12

#define LOG2				0.69314718055994529

#define APEX2F(v)			(powf(2.0F, (v)/64.0F))
#define APEX2S(v)			(powf(2.0F, (v)/32.0F))
#define ISFLOAT_ZERO(v)		((int)((v) * 10000) == 0)


///////////////////////////////////////////////////////////////////////
//

#define MAXIFDS				4

typedef struct imageinfo {
	long width;							// tag 0x100
	long height;						// tag 0x101
	long spp;							// tag 0x115 SamplePerPixel
	long bps;							// tag 0x102 BitsPerSample
	long offset;						// tag 0x111
	long orientation;					// tag 0x112 イメージの向き
	long bytecount;						// tag 0x117 StripByteCounts
	long samplepixel;
	long rowsstrip;						// tag 0x116 Rows Per Strip ストリップ中のライン数
	long compression;
	long photometric;
	long thumb_offset;
	long thumb_length;
	long planer_config;
	long ycbcr_subsampling[2];
	float ycbcr_coeff[3];
} _imageinfo_t;

class rawmap : public filemap
{
public:
	TCHAR m_fname[MAX_PATH];
	short m_order;
	int m_headlen;
	BYTE m_head[128];
	bool m_diborder;
	int m_tiffbase;

	CSimpleArray<_imageinfo_t*> m_ifarray;
	_imageinfo_t* create_ifitem()
	{
		_imageinfo_t* pi = new _imageinfo_t;
		memset(pi, 0, sizeof(_imageinfo_t));
		pi->ycbcr_coeff[0] = pi->ycbcr_coeff[1] = pi->ycbcr_coeff[2] = 0.0F;
		pi->orientation = ORIENTATION_NONE;
		m_ifarray.Add(pi);
		return pi;
	}

	int m_thumb_width;					// IFD(1)
	int m_thumb_height;					// IFD(1)
	int m_thumb_offset;					// IFD(1)
	int m_thumb_length;					// IFD(1)
	int m_thumb_layers;					// IFD(1)

	int m_strip_offset;
	int m_thumb_orientation;

	IStream* m_pstrm;
	bool m_largeimage;
	bool m_imagewrite_completed;
	bool m_this_is_jpeg;

	exif_t m_exif;
	gps_t m_gps;

	__inline USHORT fget2()
	{
		return ((m_order == 0x4949) ? getshort() : getmsbshort());
	}

	__inline short fgetshort()
	{
		return ((m_order == 0x4949) ? getshort() : getmsbshort());
	}

	__inline int fget4()
	{
		return ((m_order == 0x4949) ? getlong() : getmsblong());
	}

	void rawinit();
	double get_tagvalue(int fmt);
	void read_ascii(void* buffer, int count, int countMax);
	bool jpg_getsize(long offset, long* width, long* height);
	long jpg_datasize();
	bool data_streamout(long offset, long length, bool rewind = true);
	void write_bmpfileheader(int width, int height);
	bool tiff_seek(int type, int count);
	void makernote_nikon();
	void makernote_nikon_previewimage();
	void makernote_nikon_lensdata(BYTE* lensdata, int count);
	void makernote_nikon_picture_control(int count);
	void makernote_nikon_isoinfo(int count);
	void makernote_canon();
	void makernote_canon_settinginfo(int count);
	void makernote_canon_shotinfo(int count);
	void makernote_canon_panoramainfo(int count);
	void makernote_canon_processinginfo(int count);
	void makernote_canon_focalinfo(int count);
	void makernote_canon_fileinfo(int count);
	void makernote_canon_pictureinfo(int count);
	void makernote_canon_camerainfo(int count);
	void makernote_canon_deviceinfo(int count);
	void makernote_canon_AA_block(int count);
	void makernote_canon_1D_block(int count);
	void makernote_canon_1F_block(int count);
	void makernote_canon_block_dump(int tag, int count);
	void makernote_canonraw_imageinfo(int count);
	void makernote_olympus();
	void makernote_olympus_2010();
	void makernote_olympus_2020();
	void makernote_olympus_2030();
	void makernote_olympus_2031();
	void makernote_olympus_2040();
	void makernote_olympus_2050();
	void makernote_minolta(int base);
	bool makernote_minolta_info(int count);
	void makernote_minolta_7d(int count);
	void makernote_minolta_sweet(int count);
	void makernote_casio();
	void makernote_fuji();
	void makernote_panasonic();
	void makernote_sanyo();
	void makernote_pentax();
	void makernote_sigma();
	void makernote_epson();
	void makernote_sony(int base);
	void makernote_sony_minolta();
	void makernote_sony_A100(int count);
	void makernote_sony_tag0114(int count);
	void makernote_sony_tag9050(int count);
	void makernote_sony_tag9405(int count);
	void makernote_sony_tag940A(int count);
	void makernote_sony_tag940C(int count);
	void makernote_sony_tag9412(int count);
	void makernote_sony_camerasettings(int count);
	void makernote_sony_camerasettings2(int count);
	void makernote_sony_camerasettings3(int count);
	void makernote_sony_camerasettings_unknown(int count);
	void makernote_ricoh();
	void makernote_ricoh_imageinfo(int count);
	void makernote_ricoh_camerainfo();
	void makernote_mamiya();
	void makernote_leica();
	void makernote_kddi_ca();
	void makernote_hassel();
	void makernote_default(int base);
	void parse_adobe(int base);
	void specify_cameramodel();
	void parse_exif(int base);
	void parse_gps(int base);
	bool parse_tiff_ifd(int base);
	void parse_tiff(int base);
	void parse_ciff(int offset, int length);
	void parse_minolta();
#ifdef SUPPORT_ROLLEI
	void parse_rollei();
	bool rollei_decode();
#endif	// SUPPORT_ROLLEI
	bool foveon_thumbs(_imageinfo_t* pimage);
	bool foveon_decode();
	bool foveon_info(long start, long datasize);
#ifdef SUPPORT_KODAK
	bool kodak_yuv_decode();
#endif	// SUPPORT_KODAK
	bool identify(bool get_primary_image = false);
	bool isexif();
};



#endif			// __EXIFPARSE_H__
