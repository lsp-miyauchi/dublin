//////////////////////////////////////////////////////////////////////
// lspmif.h
//
// 以下の ifdef ブロックは DLL から簡単にエクスポートさせるマクロを作成
// する標準的な方法です。 
// この DLL 内のすべてのファイルはコマンドラインで定義された LSMIF_EXPORTS
// シンボルでコンパイルされます。
// このシンボルはこの DLL が使用するどのプロジェクト上でも未定義でなければ
// なりません。この方法ではソースファイルにこのファイルを含むすべての
// プロジェクトが DLL からインポートされたものとして LSMIF_API 関数を参照し、
// そのためこの DLL はこのマクロで定義されたシンボルをエクスポートされたもの
// として参照します。
//

#pragma once
#ifndef __LSMIF_INCLUDE
#define __LSMIF_INCLUDE

#ifdef LSMIF_EXPORTS
#define LSMIF_API __declspec(dllexport)
#else
#define LSMIF_API __declspec(dllimport)
#endif

typedef BOOL (CALLBACK *PROGRESSCALLBACK)(int, int);

enum {
	PROG_ERROR = -1,
	PROG_INITIALIZE,
	PROG_CONTINUE,
	PROG_DONE,
	PROG_RESET,
};

#define bltRed			0x00000001
#define bltGreen		0x00000002
#define bltBlue			0x00000004
#define bltAlpha		0x00000008

enum {
	planeRed,
	planeGreen,
	planeBlue,
	planeAlpha,
};

enum {
	strokeUpLeft,
	strokeUp,
	strokeUpRight,
	strokeLeft,
	strokeRight,
	strokeDownLeft,
	strokeDown,
	strokeDownRight,
};

enum {
	blendNormal,				// 0 = 通常
	blendDark,					// 1 = 比較（暗）
	blendLite,					// 2 = 比較（明）
	blendHue,					// 3 = 色相
	blendSat,					// 4 = 彩度
	blendColor,					// 5 = カラー
	blendLum,					// 6 = 輝度
	blendMul,					// 7 = 乗算
	blendScreen,				// 8 = スクリーン
	blendDither,				// 9 = ディザ合成
	blendOverlay,				// 10 = オーバーレイ
	blendHardLight,				// 11 = ハードライト
	blendSoftLight,				// 12 = ソフトライト
	blendDiff,					// 13 = 差の絶対値
	blendExclusion,				// 14 = 除外
	blendDodge,					// 15 = 覆い焼きカラー
	blendBurn,					// 16 = 焼き込みカラー
	blendAdd,					// 17 = 加算
	blendDel,					// 18 = 減算
};

enum {
	ditherNo,
	ditherFS,
	ditherNode,
	ditherFSLowBleed,
	ditherFixed
};

enum {
	paletteMake,
	paletteCustom,
	paletteMono
};

enum {
	wdpalWin16,
	wdpalWin256,
	wdpalMac,
	wdpalWeb,
	wdpalSpect,
	wdpalPastel,
	wdpalHot,
	wdpalCold,
	wdpalGold
};

enum {
	zoomBox,
	zoomTriangle,
	zoomHermite,
	zoomHanning,
	zoomHamming,
	zoomBlackman,
	zoomGaussian,
	zoomQuadratic,
	zoomCubic,
	zoomCatrom,
	zoomMitchell,
	zoomLanczos,
	zoomBessel,
	zoomSinc,
	zoomCustom,
	zoomLast
};

enum {
	noiseGaussian,
	noiseMultiplicativeGaussian,
	noiseImpluse,
	noiseLaplacian,
	noisePoisson,
	noiseUniform
};

enum {
	bumpmapLinear,
	bumpmapSpherical,
	bumpmapSinuosidal
};

enum {
	clrspcOHTA,
	clrspcsRGB,
	clrspcXYZ,
	clrspcYCbCr,
	clrspcYCC,
	clrspcYIQ,
	clrspcYPbPr,
	clrspcYUV
};

enum {
	filterMean,
	filterMedian,
	filterAburae,
	filterSharpen,
	filterSoften,
	filterEmboss,
	filterSharpenMore,
	filterDilate,
	filterErode,
	filterEdgeEnhance,
	filterEdgeEnhanceMore,
	filterEdgeLaplacian,
	filterEdgeSobelHorz,
	filterEdgeSobelVert,
	filterSharpenEdge,
	filterSoftenEdge,
};

enum {
	sblurLeft,
	sblurRight,
	sblurTop,
	sblurBottom
};

enum {
	sblurWind,
	sblurBlast
};

enum {
	sblurBoth,
	sblurLeading,
	sblurTrailing
};

enum {
	huesAll,
	huesRed,
	huesYellow,
	huesGreen,
	huesCyan,
	huesBlue,
	huesMagenta,
};


#pragma pack(1)

typedef struct tagRGBA {
	BYTE red;
	BYTE green;
	BYTE blue;
	BYTE alpha;
} RGBA;

typedef struct tagSYSCOL {
	BYTE red;
	BYTE green;
	BYTE blue;
} SYSCOL;

typedef struct tagMIF_COLORBALANCE {
	int cyan_red[3];
	int magenta_green[3];
	int yellow_blue[3];
	BOOL preserve_lum;
} MIF_COLORBALANCE;

typedef struct tagMIF_EMBOSS {
	double azimuth;				// 0.0 - 360.0  Azimuth
	double elevation;			// 0.5 - 90.0   Elevation
	int depth;					// 1 - 65       Depth
	BOOL embossp;				// TRUE = emboss, FALSE = bumpmap
} MIF_EMBOSS;

typedef struct tagMIF_UNSHARPMASK{
	double radius;				// 半径 1.0 - 25.0    刻み 0.1   初期値 5.0
	double amount;				// 適用度 0.00 - 5.00 刻み 0.01  初期値 0.5
	int threshold;				// しきい値 0 - 255   刻み 1     初期値 0
} MIF_UNSHARPMASK;

typedef struct tagMIF_VIDEO {
	int pattern_number;			// RGBパターン (0 - 5)
	BOOL additive;				// オリジナルイメージの上乗せ
	BOOL rotated;				// RGBパターンの90度回転
} MIF_VIDEO;

typedef struct tagMIF_BUMPMAP {
	double azimuth;				// 0.0 - 360.0  Azimuth
	double elevation;			// 0.5 - 90.0   Elevation
	int depth;					// 1 - 65       Depth
	int xofs;					// -1000 - 1001 X offset
	int yofs;					// -1000 - 1001 Y offset
	int waterlevel;				// 0 - 255      Xparency
	int ambient;				// 0 - 255      Ambient lighting factor
	BOOL compensate;			// Compensate for darkening
	BOOL invert;				// Invert bumpmap
	int type;					// Type of map (LINEAR 0, SPHERICAL 1, SINUOSIDAL 2)
	BOOL tiled;
} MIF_BUMPMAP;

typedef struct tagMIF_COLORCUT {
	int color_space;
	int xstep;
	int ystep;
	int zstep;
} MIF_COLORCUT;

typedef struct tagGRADCOLORS {	// for mif::fillshaderect
	COLORREF cr;
	int pos;
} GRADCOLORS;

#pragma pack()

class mapfile
{
public:
	mapfile();
	~mapfile();

	void* create(DWORD size);
	void close();

protected:
	void* m_base;
	HANDLE m_hfile;
	HANDLE m_hmap;
	char m_tmpfile[MAX_PATH * 2];
};

class LSMIF_API mif
{
public:
	mif();
	mif(int width, int height, int bits);
	mif(mif& src);
	mif(HANDLE hdib);
	mif(HBITMAP hbm, HPALETTE hpal);
	~mif();

protected:
	void clearparam();
	BOOL _attachdib(HANDLE hdib, BOOL ondisk);

public:
	void destroy();
	BOOL create(int width, int height, int bits);
	BOOL create_width_mapfile(int width, int height, int bits);
	BOOL attachdib(HANDLE hdib);
	BOOL attachdib_width_mapfile(HANDLE hdib);
	BOOL attachdib32(int width, int height, void* pixels);
	BOOL attachbitmap(HBITMAP hbm, HPALETTE hpal);
	int width();
	int height();
	int bits();
	int colors();
	int packets();
	RGBA* map();
	RGBA* pixels();
	BOOL copyto(mif* copy);
	BOOL copyfrom(mif* copy);
	BOOL movefrom(mif* move);
	void alloccmap(int colors);
	void blt(int dx, int dy, int width, int height, mif* src, int sx, int sy);
	void bltex(int dx, int dy, int width, int height, mif* src, int sx, int sy, int mode);
	void blt(int dx, int dy, int width, int height, HANDLE hdib, int sx, int sy);
	void increasecolor(int bits);
	void applyalpha(COLORREF bkcolor);
	HANDLE createdib();
	HBITMAP createbitmap(HPALETTE* phpal);
	LOGPALETTE* createlogpalette();
	HPALETTE createpalette();
	BOOL replace_palette(HPALETTE hpal);
	BOOL addlayer(mif* lay, int x, int y, int blendtype, int opaque);
	RGBA* pos(int x, int y);
	BOOL desaturate(LPRECT prc);
	void grayscale();
	void validate_rgb();
	BOOL validaterect(LPRECT prcOrg, LPRECT prcVld);

	BOOL filter_3x3(LPRECT prc, UINT nType);
	BOOL filter_custom3x3(LPRECT prc, int* pFilter, int nDivide);
	BOOL filter_sharpen(LPRECT prc, int nValue);
	BOOL filter_soften(LPRECT prc, int nValue);

	void fillsolidrect(int sx, int sy, int width, int height, COLORREF rgb, int opaque);
	void fillsolidrect(LPRECT prc, COLORREF rgb, int opaque);
	void fillshaderect(int sx, int sy, int width, int height, COLORREF* rgbs, int colors, int bounds, int opaque);
	void fillshaderect(LPRECT prc, COLORREF* rgbs, int colors, int bounds, int opaque);
	LOGPALETTE* make_gradtable(GRADCOLORS* pcol, int numcolors, int numtables);
	void free_gradtable(LOGPALETTE* plgp);
	void fillgradientrect(int sx, int sy, int width, int height, GRADCOLORS* pcol, int colors, int bounds, int opaque);
	void fillgradientrect(LPRECT prc, GRADCOLORS* pcol, int colors, int bounds, int opaque);

	void solidfill(int sx, int sy, int width, int height, COLORREF rgb, int opaque, int blendmode = blendNormal);
	void solidfill(LPRECT prc, COLORREF rgb, int opaque, int blendmode = blendNormal);
	void shadefill(int sx, int sy, int width, int height, COLORREF* rgbs, int colors, int bounds, int opaque, int blendmode = blendNormal);
	void shadefill(LPRECT prc, COLORREF* rgbs, int colors, int bounds, int opaque, int blendmode = blendNormal);
	void gradientfill(int sx, int sy, int width, int height, GRADCOLORS* pcol, int colors, int bounds, int opaque, int blendmode = blendNormal);
	void gradientfill(LPRECT prc, GRADCOLORS* pcol, int colors, int bounds, int opaque, int blendmode = blendNormal);

	void settoalpha();
	void copyplane(int planeFrom, int planeTo);
	void invertrect(int sx, int sy, int width, int height);
	void invertrect(LPRECT prc);
	void texturefill(mif* teture, int sx, int sy, int width, int height, int opaque, int blendmode = blendNormal);
	void texturefill(mif* texture, LPRECT prc, int opaque, int blendmode = blendNormal);
	void getcol(RGBA* data, int sx, int sy, int height);
	void setcol(RGBA* data, int sx, int sy, int height);
	void getrow(RGBA* data, int sx, int sy, int width);
	void setrow(RGBA* data, int sx, int sy, int width);
	void getcol_rgb(BYTE* data, int sx, int sy, int height);
	void setcol_rgb(BYTE* data, int sx, int sy, int height);
	void getrow_rgb(BYTE* data, int sx, int sy, int width);
	void setrow_rgb(BYTE* data, int sx, int sy, int width);
	void getpacket(RGBA* data, int sx, int sy, int dx, int dy);
	void setpacket(RGBA* data, int sx, int sy, int dx, int dy);
	void getplane(BYTE* data, int sx, int sy, int dx, int dy, int plane);
	void setplane(BYTE* data, int sx, int sy, int dx, int dy, int plane);
	void rotate(BOOL rotate_left);
	BOOL mirror(int sx, int sy, int dx, int dy);
	BOOL flip(int sx, int sy, int dx, int dy);
	BOOL rotate180(int sx, int sy, int dx, int dy);
	BOOL crop(RECT* prc);
	void addborder(int left, int right, int top, int bottom, COLORREF rgb);
	void sharpen(RECT* prc);
	BOOL sharpen2(LPRECT prc, int per);
	void negative(LPRECT prc);
	BOOL merge(mif* dst, BOOL horz, int span, COLORREF rgb);
	void quantize(int dither_type, int numcolors, LOGPALETTE* plgp);
	void quantize_wpal(int dither_type, const SYSCOL* sc, int numpal);
	BOOL quantize_wdp(int dither_type, int wdpal);
	BOOL bright_contrast(LPRECT prc, int brightness, int contrast);
	BOOL bumpmap(LPRECT prc, MIF_BUMPMAP* pbp);
	BOOL bumpmap_merge(mif& bmp, POINT pt, MIF_BUMPMAP* pbp);
	BOOL autocontrast(LPRECT prc);
	BOOL autocolorenhance(LPRECT prc);
	BOOL adjust_rgb(LPRECT prc, int red, int green, int blue);
	BOOL addnoise(LPRECT prc, int noise_type);
	BOOL blur(LPRECT prc, int xfactor, int yfactor);
	BOOL blur_simple(LPRECT prc, int radius);
	BOOL blur_gauss_rle(LPRECT prc, double radius);
	BOOL blur_gauss_rle_plane(LPRECT prc, double radius, UINT planes);
	BOOL blur_gauss_iir(LPRECT prc, double horz, double vert);
	BOOL blur_gauss_sel(LPRECT prc, double radius, int maxdelta);
	BOOL colorbalance(LPRECT prc, MIF_COLORBALANCE* pcb);
	BOOL colorify(LPRECT prc, COLORREF cr);
	BOOL colorcut(LPRECT prc, MIF_COLORCUT* pcp);
	DWORD countcolors();
	BOOL edge(LPRECT prc, double amount);
	BOOL emboss(LPRECT prc, MIF_EMBOSS* pep);
	BOOL enhance(LPRECT prc, int threshold);
	BOOL equalize(LPRECT prc);
	BOOL fadeinrect(LPRECT prc, int delta, COLORREF cr);
	BOOL fadepaste(mif& paste, int delta, int xpos, int ypos);
	BOOL gamma(LPRECT prc, double rgb_amt);
	BOOL gamma_rgb(LPRECT prc, double r_amt, double g_amt, double b_amt, double rgb_amt);
	BOOL mblur_linear(LPRECT prc, int length, int angle);
	BOOL mblur_radial(LPRECT prc, int angle);
	BOOL mblur_zoom(LPRECT prc, int length);
	BOOL mblur_round(LPRECT prc, int rot);
	BOOL modulate(LPRECT prc, int per_h, int per_s, int per_l);
	BOOL hue_saturation(LPRECT prc, int hue_range, int per_h, int per_s, int per_l);
	BOOL normalize(LPRECT prc);
	BOOL oilpaint(LPRECT prc, int rad);
	BOOL pixelize(LPRECT prc, int xblock, int yblock, BOOL renga, BOOL cross);
	BOOL posterize(LPRECT prc, int levels);
	BOOL pyramid(LPRECT prc, int block);
	BOOL raise_edge(LPRECT prc, int delta);
	BOOL scroll(int sx, int sy, BOOL wrap);
	BOOL spread(LPRECT prc, int amount);
	BOOL sblur_blast(LPRECT prc, int threshold, int strength, int direction, int edge);
	BOOL sblur_wind(LPRECT prc, int threshold, int strength, int direction, int edge);
	BOOL tile_divide(LPRECT prc, int numdiv);
	BOOL tile(LPRECT prc, int xblock, int yblock);
	BOOL texturize(LPRECT prc, char* texture, int texwidth, int texheight, int direction, int depth);
	BOOL texturize_by_file(LPRECT prc, LPCSTR txrfile, int direction, int depth);
	BOOL texturize_by_resource(LPRECT prc, HINSTANCE hInst, UINT typeResource, UINT idResource, int direction, int depth);
	BOOL texture_save(LPCSTR title, LPCSTR txrfile, BOOL gray);
	BOOL unsharp_mask(LPRECT prc, MIF_UNSHARPMASK* pup);
	BOOL unsharp_mask2(LPRECT prc, double radius, double depth);
	BOOL video(LPRECT prc, MIF_VIDEO* pvp);
	BOOL zoom(int cx, int cy, int zoom_method, BOOL ondisk);
	BOOL zoom_pixel(int cx, int cy);
	BOOL zoom_near(int cx, int cy);
	BOOL zoom_linear(int cx, int cy);
	BOOL zoom_inframe(int cx, int cy);
	void convert_colorspace(int color_space);
	void convert_rgb(int color_space);
	void progress_init(int maxprogress);
	void progress_continue();
	void progress_term();
	void progress_reset();

	int m_width;
	int m_height;
	int m_bits;
	int m_colors;
	int m_packets;
	RGBA* m_map;
	RGBA* m_pixels;

	mapfile* m_mapfile;
	PROGRESSCALLBACK m_progress;
};
_inline mif::~mif()
	{ destroy(); }
_inline RGBA* mif::pos(int x, int y)
	{ return m_pixels + y * m_width + x; }
_inline int mif::width()
	{ return m_width; }
_inline int mif::height()
	{ return m_height; }
_inline int mif::bits()
	{ return m_bits; }
_inline int mif::colors()
	{ return m_colors; }
_inline int mif::packets()
	{ return m_packets; }
_inline RGBA* mif::map()
	{ return m_map; }
_inline RGBA* mif::pixels()
	{ return m_pixels; }


HANDLE LSMIF_API mifQuantize(HANDLE hsrc, int dither_type, int numcolors);
HANDLE LSMIF_API mifQuantizeWcp(HANDLE hsrc, int dither_type, LOGPALETTE* plgp);
HANDLE LSMIF_API mifQuantizeWdp(HANDLE hsrc, int dither_type, int wdpal);
HANDLE LSMIF_API mifCrop(HANDLE hsrc, RECT* prc);


#endif			// __LSMIF_INCLUDE
