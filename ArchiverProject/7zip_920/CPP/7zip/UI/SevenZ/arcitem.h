////////////////////////////////////////////////////////////////////////////////
// arcitem.h

#pragma once
#ifndef __ARCITEM_H__
#define __ARCITEM_H__

class CArcItem
{
public:
	CArcItem()
	{
		m_size = 0;
		m_attr = 0;
		memset(&m_ft, 0, sizeof(FILETIME));
	}
	~CArcItem() { }

	UString m_name;
	UInt64 m_size;
	UInt64 m_packSize;
	UInt64 m_attr;
	UInt64 m_crc;
	FILETIME m_ft;
	BOOL m_encrypted;
	UString m_method;
};

#endif	// __ARCITEM_H__
