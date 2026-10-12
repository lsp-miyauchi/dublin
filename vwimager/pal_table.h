////////////////////////////////////////////////////////////////
// pal_table.h

#define DEFAULT_PALETTE_COUNT		8

extern const COLORREF pal_win[];		// Windowsシステムパレット
extern const COLORREF pal_mac[];		// Macシステムパレット
extern const COLORREF pal_spc[];		// スペクトルパレット
extern const COLORREF pal_pastel[];		// パステルパレット
extern const COLORREF pal_kok[];		// 黒体輻射パレット
extern const COLORREF pal_cold[];		// コールドパレット
extern const COLORREF pal_web[];		// Webパレット
extern const COLORREF pal_gold[];		// Goldパレット

HPALETTE make_default_palette(int index);
