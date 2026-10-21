////////////////////////////////////////////////////////////////////////
// ip_view.h

#pragma once
#ifndef _IP_VIEW_H_
#define _IP_VIEW_H_

#include "imeman.h"
#include "ip_doc.h"
#include "regexp.h"

class CCanvas;
class CPadView;
class CPadCursor;
class CCaret;

//=========================================================================
//	イベントハンドラインターフェイス
//
//	カーソルから発生するイベント色々を
//=========================================================================

#define CEM_INSERTMODE				1
#define CEM_READONLYMODE			2

class CCursorEventHandler
{
	friend class CPadCursor;
	virtual void OnCursorEventMove( const DPos& c, const DPos& s ) {}
	virtual void OnCursorEventChar( CPadCursor* cur, WCHAR wch );
	virtual void OnCursorEventKey( CPadCursor* cur, int vk, bool sft, bool ctl );
	virtual void OnCursorEventIME( CPadCursor* cur, const WCHAR* str, ULONG len );
	virtual void OnCursorEventMode( int modeType, bool mode) { }
};

//=========================================================================
//	カーソル
//=========================================================================

class CPadCursor
{
public:

	// 初期化とか
	CPadCursor(HWND wnd, CPadView* vw, CPadDocument* doc);
	~CPadCursor();
	void AddHandler( CCursorEventHandler* ev );
	void RemoveHandler( CCursorEventHandler* ev );

	// カーソル移動
	void MoveCur(int ad, int line, bool select);
	void MoveCur( const DPos& dp, bool select );
	void LastEditPos();

	// キーによるカーソル移動
	void Left( bool wide, bool select );
	void Right( bool wide, bool select );
	void Up( bool wide, bool select );
	void Down( bool wide, bool select );
	void Home( bool wide, bool select );
	void End( bool wide, bool select );
	void PageUp( bool select );
	void PageDown( bool select );
	void PageHalfUp( bool select );
	void PageHalfDown( bool select );

	// テキスト書き換え
	void Input( const WCHAR* str, ULONG len );
	void Input( const char* str, ULONG len );
	void InputChar( WCHAR ch );
	void DupLine();
	void Del();
	void DelBack();
	void DelLine();
	void DelLeft();
	void DelRight();

	// クリップボード
	void Cut(bool bAppend = false);
	void Copy(bool bFollow = false, bool bAppend = false);
	void Paste();
	void ReplaceSel(LPCTSTR pszReplace);
	bool CanCopy() { return (m_cur != m_sel); }
	bool CanCut() { return (!m_bRO && CanCopy()); }
	bool CanPaste();

	// 選択テキスト取得
	WCHAR GetCurrentChar();
	const WCHAR* GetCurrentText();
	int GetSelectedStr(CString& str);
	int GetCurrentWord(CString& str, bool bFullWord = false);

	// モード切替
	void SetInsMode( bool bIns );
	void SetROMode( bool bRO );

public:
	inline bool isInsMode() const { return m_bIns; }
	inline bool isROMode() const { return m_bRO; }
	inline bool isSelected() const { return (m_cur != m_sel); }

	const VPos& GetCurPos();
	bool GetCurSelRangePos( const VPos** start, const VPos** end ) const;
	bool isInSelRange( const VPos& s ) const;
	void ResetPos();
	void ResetSelection();
	void ScrollBegin();
	void ScrollEnd();
	void TextUpdate( const DPos& s, const DPos& e, const DPos& e2, bool mCur );
	void SetFocus();
	void KillFocus();
	void KeyDown(UINT nChar, UINT nRepCnt, UINT nFlags);
	void Char(UINT nCHar, UINT nRepCnt, UINT nFlags);
	void ImeComposition( LPARAM lp );
	void LButtonDown(long x, long y, bool shift );
	void MouseMove(long x, long y);
	void LButtonUp(long x, long y);
	void LButtonDblClk(long x, long y);
	void Timer();
	int  ImeReconvertString( RECONVERTSTRING* rs );
	bool ImeConfirmReconvertString( RECONVERTSTRING* rs );

private:
	CPadDocument* m_doc;
	CPadView* m_view;
	CCursorEventHandler* m_pEventHandler;
	CCaret* m_caret;

	VPos m_cur;					// カーソル位置
	VPos m_sel;					// 選択時の軸足位置
	bool m_bIns;				// 挿入モード？
	bool m_bRO;					// 読取専用？

	UINT_PTR m_idTimer;			// マウスドラッグ制御用の
	int  m_keyRepTime;			// タイマー関係
	VPos m_dragS;				// ドラッグ開始時の選択範囲（開始位置）
	VPos m_dragE;				// ドラッグ開始時の選択範囲（終了位置）
	bool m_bLineSelectMode;		// 行選択モード？
	bool m_bDragging;

	CCursorEventHandler m_defaultHandler;

private:
	void MoveByMouse(const CPoint& pt);
	void MoveTo(const VPos& vp, bool sel, bool bKeepVertOffset = false);
	VPos CalcUpDown(int dy) const;
	void UpdateCaretPos();
	void Redraw( const VPos& s, const VPos& e );
	void RedrawLine( const VPos& s );
};


//=========================================================================
//	描画基本ルーチン
//
//	利用するには、CCanvasオブジェクトから GetPainter して使います。
//	画面用デバイスコンテキストのレイヤです。きちんと書いておけば印刷機能を
//	追加するときに楽なのかもしれませんが、そんなことを考える計画性が
//	あるはずもなく極めて適当に…。
//=========================================================================

class CPainter
{
public:

	~CPainter();

	// ペンを選択
	inline HPEN SelectPen(HPEN hpen) { return m_dc.SelectPen(hpen); }

	// ブラシを選択
	inline HBRUSH SelectBrush(HBRUSH hbr) { return m_dc.SelectBrush(hbr); }

	// 指定位置に一文字出力
	inline BOOL CharOut(TCHAR ch, int x, int y) { return m_dc.TextOut(x, y, &ch, 1); }

	// 指定位置に文字列を出力
	inline BOOL StringOut(const TCHAR* str, int len, int x, int y) { return m_dc.TextOut(x, y, str, len); }

	// 指定位置に文字列を出力
	inline int DrawString(const TCHAR* str, int len, LPRECT prc, UINT wFormat) { return m_dc.DrawText(str, len, prc, wFormat); }

	// 文字色切り替え
	inline COLORREF SetTextColor(int index) { return m_dc.SetTextColor(m_colorTable[index]); }

	// 背景色切り替え
	inline COLORREF SetBkColor(int index) { return m_dc.SetBkColor(m_colorTable[index]); }

	// 背景モード切り替え
	inline int SetBkMode(int mode) { return m_dc.SetBkMode(mode); }

	// 指定色で塗りつぶし
	inline void Fill(const RECT& rc, int index) { m_dc.FillSolidRect(&rc, m_colorTable[index]); }

	// 背景色で塗りつぶし
	inline void Fill(const RECT& rc) { Fill(rc, CLR_BG); }

	// 反転
	inline void Invert(const CRect& rc) { m_dc.InvertRect(&rc); }

	// 現在のペンで直線を引く
	inline void DrawLine(int sx, int sy, int ex, int ey) { m_dc.MoveTo(sx, sy); m_dc.LineTo(ex, ey);  }

	// 選択範囲
	void Highlight(const CRect& rc);

	// クリップ領域設定
	inline void SetClip(const RECT& rc) { m_dc.IntersectClipRect(&rc); }

	// クリップ領域解除
	inline void ClearClip() { m_dc.SelectClipRgn(NULL); }

	// 記号描画本体
	void DrawSymbolBitmap( HBITMAP hbm, int x, int y, int times, int indexFore, int indexBack );

	// 半角スペース用記号描画
	inline void DrawHSP( int x, int y, int times, int indexFore, int indexBack ) { DrawSymbolBitmap( m_hbmHS, x, y, times, indexFore, indexBack ); }

	// 全角スペース用記号描画
	inline void DrawZSP( int x, int y, int times, int indexFore, int indexBack ) { DrawSymbolBitmap( m_hbmZS, x, y, times, indexFore, indexBack ); }

	// 改行記号
	inline void DrawEOL(int x, int y, int indexFore, int indexBack)  { DrawSymbolBitmap( m_hbmCR, x, y, 1, indexFore, indexBack ); }

	// TAB用記号描画
	inline void DrawTAB( int x, int y, int indexFore, int indexBack ) { DrawSymbolBitmap( m_hbmTB, x, y, 1, indexFore, indexBack ); }

	// カラーインデクスからカラー値を取得
	inline COLORREF GetColor(int index) { return (0 <= index && index <= ARRAYSIZE(m_colorTable)) ? m_colorTable[index] : (COLORREF)-1; }

public:
	// 高さ(pixel)
	int H() const { return m_height; }

	// 行間マージン(pixel)
	int LM() const { return m_margin_v; }

	// 数字幅(pixel)
	int F() const { return m_figWidth; }

	// 文字幅(pixel)
	int W(TCHAR ch) const
	{
		if (m_widthTable[ch] == -1) {
/*
			ABC abc;
			m_dc.GetCharABCWidths(ch, ch, &abc);
			m_widthTable[ch] = abc.abcA + abc.abcB + abc.abcC;
*/
      SIZE sz;
      m_dc.GetTextExtent(&ch, 1, &sz);
      m_widthTable[ch] = sz.cx;
		}
		return m_widthTable[ch];
	}

	// 標準文字幅(pixel)
	int W() const { return m_widthTable[ L'x' ]; }

	// 次のタブ揃え位置を計算
	//int nextTab(int x) const { int t=T(); return (x/t+1)*t; }
	int nextTab(int x) const
	{
		int t = T();
		return ((x + 4) / t + 1) * t;
	}

private:
	int T() const { return m_widthTable[ L'\t' ]; }

public:
	// 現在のフォント情報
	const LOGFONT& LogFont() const { return m_logfont; }

	// 特別文字を描画するか否か
	bool sc( int i ) const { return m_scDraw[i]; }

private:
	CDCHandle	m_dc;
	Graphics	m_gr;

	const HFONT	m_hfont;
	int			m_height;
	int			m_margin_v;
	int*		m_widthTable;
	int			m_figWidth;
	LOGFONT		m_logfont;
	COLORREF	m_colorTable[15];
	bool		m_scDraw[6];
	HBITMAP		m_hbmCR;
	HBITMAP		m_hbmTB;
	HBITMAP		m_hbmZS;
	HBITMAP		m_hbmHS;

private:

	CPainter( HDC hdc, const VConfig& vc );
	HDC GetDC() { return m_dc; }
	void CreateSymbolBitmaps();
	friend class CCanvas;
};



//=========================================================================
//	描画可能領域
//
//	ウインドウサイズの変更や折り返しの有無やフォントの設定などに
//	対応して、描画領域のサイズを適当に管理します。やることは
//	とりあえずそれだけ。
//=========================================================================

class CCanvas
{
public:

	CCanvas( const CPadView* vw );
	~CCanvas();

	// Viewの大きさ変更イベント処理
	//	 @return 折り返し幅が変わったらtrue
	bool ResizeView( int cx, int cy );

	// 行数変更イベント処理
	//	 @return テキスト領域の幅が変わったらtrue
	bool TotalLineNumberChange( ULONG tln );

	// フォント変更イベント処理
	void SetConfig( const VConfig& vc );

public:

	// [行番号を表示するか否か]
	bool showLN() const { return m_bShowLineNumber; }

	// [-1:折り返し無し  0:窓右端  else:指定文字数]
	int wrapType() const { return m_wrapType; }

	// 折り返し幅(pixel)
	ULONG wrapWidth() const { return m_wrapWidth; }

	// 表示領域の位置(pixel)
	const CRect& zone() const { return m_rcTextZone; }

	// 描画用オブジェクト
	CPainter* GetPainter() const { return m_painter; }

private:
	int  m_wrapType;				// [ -1:折り返し無し  0:窓右端  else:指定文字数 ]
	bool m_bShowLineNumber;			// [ 行番号を表示するか否か ]

	CPainter* m_painter;			// 描画用オブジェクト
	ULONG m_wrapWidth;				// 折り返し幅(pixel)
	CRect m_rcTextZone;				// テキスト表示域の位置(pixel)
	int m_numFigColumns;			// 行番号の桁数

private:

	bool CalcLNAreaWidth();
	void CalcWrapWidth();
};



//=========================================================================
//	行毎の折り返し情報
//=========================================================================

struct WLine : public CStorage<ULONG>
{
	// [0]   : その行の折り返し無しでの横幅を格納（pixel）
	// [1-n] : n行目の終端のindexを格納。
	//
	//   例えば "aaabbbcccc" という論理行を "aaab" "bbbc" "ccc" と折るなら
	//   {48, 4, 8, 11} などという長さ4の配列となる。

	WLine() : CStorage<ULONG>(2) {}
	ULONG& width()      { return (*this)[0]; }
	ULONG width() const { return (*this)[0]; }
	ULONG rln() const   { return size()-1; }
};



//=========================================================================
//@{
//	再描画範囲を指定するためのフラグ
//@}
//=========================================================================

enum ReDrawType
{
	LNAREA, // 行番号ゾーンのみ
	LINE,   // 変更のあった一行のみ
	AFTER,  // 変更のあった行以下全部
	ALL     // 全画面
};



//=========================================================================
//@{
//	描画処理を細かく指定する構造体
//@}
//=========================================================================

struct VDrawInfo {
	const RECT rc;			// 再描画範囲
	int xBase;				// 一番左の文字のx座標
	int xMin;				// テキスト再描画範囲左端
	int xMax;				// テキスト再描画範囲右端
	int yMin;				// テキスト再描画範囲上端
	int yMax;				// テキスト再描画範囲下端
	ULONG tlMin;			// テキスト再描画範囲上端論理行番号
	int xSelBegin;
	int xSelEnd;	// 選択範囲のx座標
	int ySelBegin;
	int ySelEnd;	// 選択範囲のy座標

	explicit VDrawInfo( const RECT& r ) : rc(r) {}
};



//=========================================================================
//	折り返しedテキストの管理・表示等
//
//	CCanvasクラスによって計算された領域サイズを参考に、テキストの
//	折り返し処理を実行する。ここで、スクロール制御、描画処理など
//	主要な処理は全て実行することになる。
//=========================================================================

class CPadView : public CWindowImpl<CPadView>, public CDocumentEventHandler
{
public:
	DECLARE_WND_CLASS_EX(_T("PadEdit"), CS_DBLCLKS | CS_OWNDC, COLOR_WINDOW)

	BOOL PreTranslateMessage(MSG* pMsg);

	BEGIN_MSG_MAP(CPadView)
		ON_WM_CREATE()
		ON_WM_DESTROY()
		MESSAGE_HANDLER(WM_NCPAINT, OnHandlerNcPaint)
		ON_WM_PAINT()
		ON_WM_ERASEBKGND()
		ON_WM_SIZE()
		ON_WM_HSCROLL()
		ON_WM_VSCROLL()
		MESSAGE_HANDLER(WM_MOUSEWHEEL, OnHandlerMouseWheel)
		ON_WM_KILLFOCUS()
		ON_WM_SETFOCUS()
		ON_WM_KEYDOWN()
		ON_WM_CHAR()
		ON_WM_LBUTTONDOWN()
		ON_WM_LBUTTONUP()
		ON_WM_LBUTTONDBLCLK()
		ON_WM_MOUSEMOVE()
		ON_WM_TIMER()
		ON_MESSAGE_VOID(WM_COPY, OnHandlerCopy)
		ON_MESSAGE_VOID(WM_CUT, OnHandlerCut)
		ON_MESSAGE_VOID(WM_PASTE, OnHandlerPaste)
		ON_MESSAGE(WM_UNDO, OnHandlerUndo)
		ON_MESSAGE(EM_UNDO, OnHandlerUndo)
		ON_MESSAGE(EM_CANUNDO, OnHandlerCanUndo)
		ON_MESSAGE(EM_SETREADONLY, OnHandlerSetReadOnly)
		ON_MESSAGE(WM_IME_REQUEST, OnHandlerImeRequest)
		MESSAGE_HANDLER(WM_IME_STARTCOMPOSITION, OnHandlerImeStartComposition)
		MESSAGE_HANDLER(WM_IME_COMPOSITION, OnHandlerImeComposition)
		MESSAGE_HANDLER(WM_SETCURSOR, OnHandlerSetCursor)
	END_MSG_MAP()

private:
	int OnCreate(CREATESTRUCT* pcs);
	void OnDestroy();
	LRESULT OnHandlerNcPaint(UINT msg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	void OnPaint();
	BOOL OnEraseBkgnd(HDC hdc);
	void OnSize(UINT type, int cx, int cy);
	void OnHScroll(UINT nSBCode, UINT nPos, HWND hwndScrollBar);
	void OnVScroll(UINT nSBCode, UINT nPos, HWND hwndScrollBar);
	LRESULT OnHandlerMouseWheel(UINT msg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	void OnSetFocus(HWND hwndOld);
	void OnKillFocus(HWND hwndNew);
	void OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags);
	void OnChar(UINT nChar, UINT nRepCnt, UINT nFlags);
	void OnLButtonDown(UINT nFlags, CPoint point);
	void OnLButtonUp(UINT nFlags, CPoint point);
	void OnLButtonDblClk(UINT nFlags, CPoint point);
	void OnMouseMove(UINT nFlags, CPoint point);
	void OnTimer(UINT_PTR nIDEvent);
	void OnHandlerCopy(WPARAM wParam, LPARAM lParam);
	void OnHandlerCut(WPARAM wParam, LPARAM lParam);
	void OnHandlerPaste(WPARAM wParam, LPARAM lParam);
	LRESULT OnHandlerUndo(WPARAM wParam, LPARAM lParam);
	LRESULT OnHandlerCanUndo(WPARAM wParam, LPARAM lParam);
	LRESULT OnHandlerSetReadOnly(WPARAM wParam, LPARAM lParam);
	LRESULT OnHandlerImeRequest(WPARAM wParam, LPARAM lParam);
	LRESULT OnHandlerImeStartComposition(UINT msg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT OnHandlerImeComposition(UINT msg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT OnHandlerSetCursor(UINT msg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);

	void OnDraw(const PAINTSTRUCT& ps);

	// CDocumentEventHandlerの派生メンバ
	void OnDocEventTextUpdate( const DPos& s, const DPos& e, const DPos& e2, bool bAft, bool mCur );
	void OnDocEventKeywordChanged();

public:
	CPadView();
	~CPadView();

	// 表示色・フォント切替
	inline void SetConfig( const VConfig& vc )
	{
		m_config = vc;
		m_canvas->SetConfig(vc);
		m_cursor->SetFocus();
		CalcEveryLineWidth(); // 行幅再計算

		DoConfigChange();
	}
	void DoResize( bool wrapWidthChanged );
	void DoConfigChange();

public:

	// 全表示行数
	ULONG vln() const { return m_vlNum; }

	// 一行の表示行数
	ULONG rln( ULONG tl ) const { return m_wraplines[tl].rln(); }

	// 折り返し位置
	ULONG rlend( ULONG tl, ULONG rl ) const { return m_wraplines[tl][rl+1]; }

	// 一個でも折り返しが存在するか否か
	bool wrapexists() const { return m_doc->tln() != vln(); }

	// ドキュメント
	CPadDocument* document() { return m_doc; }

	// カーソル
	CPadCursor* cursor() { return m_cursor; }

	// フォント
	const CPainter* painter() const { return m_canvas->GetPainter(); }

	void GetVPos( int x, int y, VPos* vp, bool linemode=false ) const;
	inline void GetOrigin( int* x, int* y ) const
	{
		*x = left() - m_scrHorz.nPos;
		*y = -m_scrVert.nPos * m_canvas->GetPainter()->H();
	}

	void GetLineRect(CRect& rc, ULONG vl);
	void SetCurrentLine(ULONG vl);
	void ConvDPosToVPos( DPos dp, VPos* vp, const VPos* base=NULL ) const;
	void ScrollTo(const VPos& vp, bool bKeepVertOffset = false);
	int  GetLastWidth( ULONG tl ) const;

public:
	const CRect& zone() const { return m_canvas->zone(); }
	int left()  const { return m_canvas->zone().left; }
	int right() const { return m_canvas->zone().right; }
	int bottom()const { return m_canvas->zone().bottom; }
	int lna()   const { return m_canvas->zone().left; }
	int cx()    const { return m_canvas->zone().Width(); }
	int cxAll() const { return m_canvas->zone().right; }
	int cy()    const { return m_canvas->zone().bottom; }

	// 検索関連
	bool FindPrev(CFind* pFinder);
	bool FindNext(CFind* pFinder);
	int FindAll(CSimpleMap<ULONG, CString>& array, CSimpleArray<CString>& finder);

private:
	bool ReSetScrollInfo();
	void ForceScrollTo( ULONG tl );
	void UpdateScrollBar();
	ReDrawType TextUpdate_ScrollBar( const DPos& s, const DPos& e, const DPos& e2 );

	ULONG tl2vl( ULONG tl ) const;
	void GetDrawPosInfo( VDrawInfo& v ) const;
	void InvalidateView( const DPos& dp, bool afterall );
	void ScrollView( int dx, int dy, bool update );
	void UpDown( int dy, bool thumb );

private:
	CPadDocument*    m_doc;
	CCanvas*         m_canvas;
	CPadCursor*      m_cursor;
	CGapBufObj<WLine> m_wraplines;
	ULONG            m_vlNum;
	ULONG            m_textCx;

private:
	void DrawTXT( const VDrawInfo v);
	void Invert( int y, int xb, int xe);

	void CalcEveryLineWidth();
	ULONG CalcLineWidth( const WCHAR* txt, ULONG len ) const;
	void ModifyWrapInfo( const WCHAR* txt, ULONG len, WLine& wl, ULONG stt );
	void ReWrapAll();
	int ReWrapSingle( const DPos& s );
	int InsertMulti( ULONG ti_s, ULONG ti_e );
	int DeleteMulti( ULONG ti_s, ULONG ti_e );
	void UpdateTextCx();
	void ReDraw( ReDrawType r, const DPos* s=NULL );
public:
	ULONG topvl();				// 画面上に表示されている開始行の論理行番号

private:
	VConfig m_config;
	SCROLLINFO m_scrHorz;		// 横スクロール情報（pixel単位）
	SCROLLINFO m_scrVert;		// 縦スクロール情報（行単位）
public:
	ULONG m_top_tl;				// 一番上に表示される論理行のTLine_Index
	ULONG m_top_vrl;			// 一番上に表示される表示行のVRLine_Index
	ULONG m_cur_vl;				// 現在行の論理行番号
};


#endif // _IP_VIEW_H_
