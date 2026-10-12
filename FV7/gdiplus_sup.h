///////////////////////////////////////////////////////////////////////
//

#pragma once
#ifndef __GDIPLUS_SUP_H__
#define __GDIPLUS_SUP_H__

class gdiplus_using
{
public:
	GdiplusStartupInput m_gdiplusStartupInput;
	ULONG_PTR m_gdiplusToken;

	gdiplus_using() { GdiplusStartup(&m_gdiplusToken, &m_gdiplusStartupInput, NULL); }
	~gdiplus_using() { GdiplusShutdown(m_gdiplusToken); }
};


#endif __GDIPLUS_SUP_H__
