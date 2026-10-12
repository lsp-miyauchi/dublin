////////////////////////////////////////////////////////////////////////
// drivemedia.h

#pragma once
#ifndef __DRIVEMEDIA_H__
#define __DRIVEMEDIA_H__

#include "winioctl.h"

class CDriveMediaInfo
{
public:
	CString m_strSerialNo;
	CString m_strProductID;
	CString m_strVendorID;
	CString m_strRevision;
	bool m_bRemovableMedia;
	STORAGE_BUS_TYPE m_enumBusType;

	CDriveMediaInfo()
		: m_bRemovableMedia(false)
		, m_enumBusType(BusTypeUnknown)
	{
	}

	//
	//	ドライブのプロパティ情報を取得する
	//
	bool GetStorageProperty(int nDriveNo)
	{
		bool ok = false;
		CString str;
		str.Format(_T("\\\\.\\%c:"), nDriveNo + 'A');
		HANDLE hDrive = CreateFile(str, GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, 0, NULL);
		if (hDrive != INVALID_HANDLE_VALUE) {
			DWORD maxlen = 8192;		// 十分なバッファー領域を確保
			BYTE* pcbData = new BYTE[maxlen];
			if (pcbData) {
				memset(pcbData, 0, maxlen);
				STORAGE_PROPERTY_QUERY sQuery;
				sQuery.PropertyId = StorageDeviceProperty;
				sQuery.QueryType = PropertyStandardQuery;
				sQuery.AdditionalParameters[0] = NULL;
				DWORD res;
				if (::DeviceIoControl(hDrive, IOCTL_STORAGE_QUERY_PROPERTY, &sQuery,
										sizeof(STORAGE_PROPERTY_QUERY), pcbData, maxlen, &res, NULL)) {
					STORAGE_DEVICE_DESCRIPTOR* desc = (STORAGE_DEVICE_DESCRIPTOR*)pcbData;
					if (desc->SerialNumberOffset && maxlen > desc->SerialNumberOffset)
						m_strSerialNo	= (char*)desc + desc->SerialNumberOffset;
					if (desc->ProductIdOffset && maxlen > desc->ProductIdOffset)
						m_strProductID	= (char*)desc + desc->ProductIdOffset;
					if (desc->ProductRevisionOffset && maxlen > desc->ProductRevisionOffset)
						m_strRevision	= (char*)desc + desc->ProductRevisionOffset;
					if (desc->VendorIdOffset && maxlen > desc->VendorIdOffset)
						m_strVendorID	= (char*)desc + desc->VendorIdOffset;
					m_enumBusType		= desc->BusType;
					m_bRemovableMedia	= desc->RemovableMedia ? true : false;
					ok = true;
				}
				delete pcbData;
			}
			::CloseHandle(hDrive);
		}
		return ok;
	}
};


#endif			// __DRIVEMEDIA_H__
