////////////////////////////////////////////////////////////////////////
// gdip_func.h

#pragma once
#ifndef __GDIP_FUNC_H__
#define __GDIP_FUNC_H__

inline void GdipRoundRect(Graphics& g, Pen* pen, Brush* brush, RectF& r, CSize size)
{
	// Create a GraphicsPath, and add an ellipse.
	REAL left = r.X;
	REAL right = r.GetRight();
	REAL top = r.Y;
	REAL bottom = r.GetBottom();
	SizeF sf((Gdiplus::REAL)size.cx, (Gdiplus::REAL)size.cy);

	GraphicsPath p;
	p.AddLine(left + sf.Width / 2, top, right - sf.Width / 2, top);
	p.AddArc(right - sf.Width, top, sf.Width, sf.Height, 270.0, 90.0);
	p.AddLine(right, top + sf.Height / 2, right, bottom - sf.Height / 2);
	p.AddArc(right - sf.Width, bottom - sf.Height, sf.Width, sf.Height, 0, 90);
	p.AddLine(right - sf.Width / 2, bottom, left + sf.Width / 2, bottom);
	p.AddArc(left, bottom - sf.Height, sf.Width, sf.Height, 90, 90);
	p.AddLine(left, bottom - sf.Height / 2, left, top + sf.Height / 2);
	p.AddArc(left, top, sf.Width, sf.Height, 180, 90);

	if (brush) g.FillPath(brush, &p);			// Fill Path.
	if (pen) g.DrawPath(pen, &p);				// Draw Path.
}


#endif		// __GDIP_FUNC_H__
