//////////////////////////////////////////////////////////////////////
// font_util.cpp

#include "stdafx.h"
#include "math.h"
#include "font_util.h"

#define WIDTHBYTES(bits)    (((bits) + 31) / 32 * 4)

HFONT create_scalefont(LOGFONT* plf, int char_scale)
{
	if (!plf || char_scale < 1) return NULL;
	LOGFONT lf = *plf;
	lf.lfWidth = MulDiv(abs(lf.lfHeight) >> 1, char_scale, 100);
	return CreateFontIndirect(&lf);
}

static FIXED __ffd(double d)
{
	long l = (long)(d * 65536L);
	return *(FIXED*)&l;
}

static void __calcmat(MAT2& mat, double shear)
{
	mat.eM11 = __ffd(1);
	mat.eM12 = __ffd(0);
	mat.eM21 = __ffd(shear);
	mat.eM22 = __ffd(1);
}

static void calc_string_rect(HDC hdc, LPCTSTR pText, MAT2& mat, CRect& rc)
{
	TEXTMETRIC tm;
	GetTextMetrics(hdc, &tm);

	GLYPHMETRICS gm;
	int xEnd = 0, xStart = 0;
	int yEnd = tm.tmHeight;

	int len = lstrlen(pText);
	for (int i = 0; i < len; i++) {
		// 文字情報取得
		GetGlyphOutline(hdc, *pText, GGO_METRICS, &gm, 0, NULL, &mat);
		if (i == 0)
			xEnd = xStart = gm.gmptGlyphOrigin.x;
		if (i < len - 1)
			xEnd += gm.gmCellIncX;
		else
			xEnd += gm.gmBlackBoxX + abs(gm.gmptGlyphOrigin.x);

		yEnd = max(yEnd, (int)(gm.gmptGlyphOrigin.y + gm.gmBlackBoxY));
		pText++;
	}

	rc.SetRectEmpty();
	rc.right = xEnd - xStart;
	rc.bottom = yEnd;
}

static BOOL calc_minimum_string_rect(mif* image, CRect& rc)
{
	if (!image) return FALSE;

	register RGBA* p = image->m_pixels;
	rc.SetRect(INT_MAX, INT_MAX, INT_MIN, INT_MIN);
	for (int y = 0; y < image->m_height; y++) {
		for (int x = 0; x < image->m_width; x++) {
			if (p->red || p->green || p->blue) {
				rc.left = min(rc.left, x);
				rc.right = max(rc.right, x);
				rc.top = min(rc.top, y);
				rc.bottom = max(rc.bottom, y);
			}
			p++;
		}
	}

	if (rc.left == INT_MAX || rc.right == INT_MIN || rc.top == INT_MAX || rc.bottom == INT_MIN)
		return FALSE;
	return TRUE;
}

static mif* __fastcall create_single_string(HDC hdc, LPCTSTR pText, MAT2& mat, UINT state)
{
	TEXTMETRIC tm;
	GetTextMetrics(hdc, &tm);

	CRect rc;
	calc_string_rect(hdc, pText, mat, rc);

	// 文字列イメージの作成
	mif* m = new mif;
	m->create(rc.Width(), rc.Height(), 24);

	// アンチエリアスのかけ具合
	DWORD fmt;
	DWORD depth;
	switch (state & DSS_AA_MASK) {
		case DSS_AA_STRONG:	fmt = GGO_GRAY2_BITMAP; depth = 128; break;
		case DSS_AA_CLEAR:	fmt = GGO_GRAY4_BITMAP; depth = 16; break;
		case DSS_AA_SMOOTH:
		default:			fmt = GGO_GRAY8_BITMAP; depth = 4; break;
	}

	GLYPHMETRICS gm;
	int sx = 0, xOffset = 0;
	BOOL b1st = TRUE;
	while (pText && *pText && *pText != '\x0d') {
		UINT letter = *pText++;

		// 文字データ格納メモリの割り当て
		DWORD dw = GetGlyphOutline(hdc, letter, fmt, &gm, 0, NULL, &mat);
		void* pBits = calloc(dw, 1);

		// 文字グリフイメージデータの取得
		GetGlyphOutline(hdc, letter, fmt, &gm, dw, pBits, &mat);
		if (b1st) {
			sx -= gm.gmptGlyphOrigin.x;
			b1st = FALSE;
		}

		BYTE* pb = (BYTE*)pBits;
		DWORD widthbytes = WIDTHBYTES(gm.gmBlackBoxX * 8);
		for (int y = 0; y < (int)gm.gmBlackBoxY; y++) {
			if ((tm.tmAscent - gm.gmptGlyphOrigin.y) + y < 0)
				continue;

			RGBA* pixels = m->pos(sx + gm.gmptGlyphOrigin.x, (tm.tmAscent - gm.gmptGlyphOrigin.y) + y);
			for (int x = 0; x < (int)gm.gmBlackBoxX; x++) {
				// 文字同士の重なりを考慮
				BYTE cb = (BYTE)min(255, (DWORD)pb[x] * depth + (DWORD)pixels->red);
				pixels->red = pixels->green = pixels->blue = cb;
				pixels++;
			}
			pb += widthbytes;
		}
		free(pBits);
		sx += gm.gmCellIncX;
	}
	return m;
}

static mif* __fastcall create_string(LPCTSTR pText, LPLOGFONT plf, int char_scale, UINT state, double font_shear)
{
	MAT2 mat;
	__calcmat(mat, font_shear);		// 傾けるときは0以外の-0.9～0.9までの値を指定

	// 矩形サイズを計算
	HDC hdc = GetDC(NULL);
	HFONT hFont = (HFONT)SelectObject(hdc, create_scalefont(plf, char_scale));

	TEXTMETRIC tm;
	GetTextMetrics(hdc, &tm);

	mif* array[256];
	int line = 0;

	LPTSTR pLine = StrDup(pText);
	LPTSTR p = pLine;
	while (p && *p) {
		LPTSTR q = StrChr(p, '\n');
		if (q) *q++ = '\0';

		array[line++] = create_single_string(hdc, p, mat, state);
		p = q;
	}

	// 全ての文字列が収まる矩形を計算
	CRect rc(0, 0, 0, line * tm.tmHeight);
	int i;
	for (i = 0; i < line; i++)
		rc.right = max(rc.right, array[i]->width());

	// 袋文字の処理のためにサイズを拡大
	rc.InflateRect(4, 4);

	// 文字列イメージの作成
	mif* m = new mif;
	m->create(rc.Width(), rc.Height(), 24);

	int sx;
	for (i = 0; i < line; i++) {
		switch (state & DSS_FMT_MASK) {
			case DSS_FMT_RIGHT:		sx = rc.Width() - 4 - array[i]->width(); break;
			case DSS_FMT_CENTER:	sx = (rc.Width() - array[i]->width()) / 2; break;
			case DSS_FMT_LEFT:
			default:				sx = 4; break;
		}
		m->blt(sx, 4 + i * tm.tmHeight, array[i]->width(), array[i]->height(), array[i], 0, 0);
		delete array[i];
	}
	LocalFree(pLine);

	DeleteObject(SelectObject(hdc, hFont));
	ReleaseDC(NULL, hdc);

	// 袋文字
	if (state & DSS_OUTLINE) {
		mif ol;
		if (!(state & DSS_OUTLINE_INNER)) {
			ol.copyfrom(m);
			ol.settoalpha();
			ol.solidfill(NULL, RGB(255, 255, 255), 255);
		}
		m->filter_3x3(NULL, filterDilate);
		m->filter_3x3(NULL, filterDilate);
		if (!(state & DSS_OUTLINE_INNER))
			m->addlayer(&ol, 0, 0, blendDiff, 255);
	}

	if (state & DSS_TATEGAKI)	m->rotate(FALSE);
	if (state & DSS_FLIP_VERT)	m->flip(0, 0, m->m_width, m->m_height);
	if (state & DSS_FLIP_HORZ)	m->mirror(0, 0, m->m_width, m->m_height);
	return m;
}

HDIB create_dragstring(LPCTSTR pszString, LPLOGFONT plf, int char_scale, UINT state, int shear)
{
	HDIB hdib = NULL;
	mif* m = create_string(pszString, plf, char_scale, state, DEG2RAD(shear));
	if (m) {
		m->invertrect(NULL);
		hdib = m->createdib();
		delete m;
	}
	return hdib;
}

HDIB draw_string(HDIB hdib, DRAWSTRINGINFO* pds)
{
	if (!hdib || !pds) return NULL;

	mif dst;
	if (!dst.attachdib(hdib) || pds->pt.x > dst.m_width || pds->pt.y > dst.m_height) return NULL;

	mif* str = create_string(pds->text, &pds->lf, pds->char_scale, pds->state, DEG2RAD(pds->shear));
	if (!str) return NULL;

	mif* inner = NULL;
	if ((pds->state & DSS_OUTLINE) && (pds->state & DSS_OUTLINE_INNER)) {
		// 袋文字内を塗りつぶす色の指定
		UINT state = pds->state & ~(DSS_OUTLINE | DSS_OUTLINE_INNER);
		inner = create_string(pds->text, &pds->lf, pds->char_scale, state, DEG2RAD(pds->shear));
		if (!inner) {
			delete str;
			return NULL;
		}
	}

	if (dst.m_bits < 24)
		dst.m_bits = 24;

	CRect rcm;
	if (!calc_minimum_string_rect(str, rcm))
		return NULL;

	rcm.right = min(rcm.right + 1, str->m_width);
	rcm.bottom = min(rcm.bottom + 1, str->m_height);
	// ドロップシャドウ
	if (pds->state & DSS_DROPSHADOW) {
		// ぼかしの適用範囲を保証する意味で、直径ピクセル分を確保する
		int delta;
		if (pds->shadow_gauss > 0)
			delta = (int)floor((double)(pds->shadow_gauss + 10) / 5.0);
		else
			delta = 0;

		mif shadow;
		shadow.create(str->m_width + delta * 2, str->m_height + delta * 2, 24);
		shadow.blt(delta, delta, str->m_width, str->m_height, str, 0, 0);
		shadow.copyplane(planeRed, planeAlpha);
		if (pds->shadow_gauss > 0)
			shadow.blur_gauss_rle_plane(NULL, (double)pds->shadow_gauss / 10.0, bltAlpha);

		shadow.solidfill(NULL, pds->shadow_cr, 255);
		dst.addlayer(&shadow, pds->pt.x - delta + pds->shadow_distance,
							  pds->pt.y - delta + pds->shadow_distance,
							  blendNormal,
							  (255 * pds->opaque * pds->shadow_opaque) / 10000);
	}

	// バンプマップ用に複製
	mif* bump = NULL;
	if (pds->state & DSS_EMBOSS_EDGE)
		bump = new mif(*str);

	// マスクを保存
	str->copyplane(planeRed, planeAlpha);
	if (pds->state == DSS_USE_TEXTURE) {
		mif texture;
		if (texture.attachdib(pds->htxr))
			str->texturefill(&texture, NULL, 255);
	}
	else
		str->gradientfill(&rcm, pds->grad, pds->numcolors, pds->direction, 255);
	dst.addlayer(str, pds->pt.x, pds->pt.y, blendNormal, 255 * pds->opaque / 100);
	delete str;

	if (bump) {
		// バンプマップ
		MIF_BUMPMAP bp;
		memset(&bp, 0, sizeof(bp));
		bp.azimuth    = pds->ee_azimuth;
		if (pds->state & DSS_DROPSHADOW && pds->shadow_distance < 0)
			bp.azimuth = 300.0;
		bp.elevation  = pds->ee_elevation;
		bp.depth      = 1;
		bp.compensate = TRUE;
		bp.type       = bumpmapLinear;
		dst.bumpmap_merge(*bump, pds->pt, &bp);
		delete bump;
	}

	if (inner) {
		inner->copyplane(planeRed, planeAlpha);
		inner->solidfill(NULL, pds->outline_cr, 255);
		dst.addlayer(inner, pds->pt.x, pds->pt.y, blendNormal, 255 * pds->opaque / 100);
		delete inner;
	}

	return dst.createdib();
}

HDIB draw_bumpmapstring(HDIB hdib, BUMPMAPSTRINGINFO* pbs)
{
	if (!hdib || !pbs) return NULL;

	mif dst;
	if (!dst.attachdib(hdib) || pbs->pt.x > dst.m_width || pbs->pt.y > dst.m_height)
		return NULL;

	mif* str = create_string(pbs->text, &pbs->lf, pbs->char_scale, pbs->state, DEG2RAD(pbs->shear));
	if (!str)
		return NULL;

	if (dst.m_bits < 24)
		dst.m_bits = 24;

	dst.bumpmap_merge(*str, pbs->pt, &pbs->bp);
	delete str;
	return dst.createdib();
}

