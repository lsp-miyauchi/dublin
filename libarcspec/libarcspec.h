#pragma once

enum ArcType {
	ARC_ERROR = -1,
	ARC_LZH,
	ARC_ZIP,
	ARC_ARJ,
	ARC_ISH,
	ARC_CAB,
	ARC_TAR,
	ARC_RAR,
	ARC_BGA,
	ARC_YZ1,
	ARC_GCA,
	ARC_7ZIP,
	ARC_ISO,
	ARC_RPM,
	ARC_UPX = 99,
	ARC_ZIPPACK = 100,
};

int __cdecl SpecifyArchiveType(LPCTSTR pszName);
int __cdecl SpecifyArchiveTypeByFileHeader(LPCTSTR pszName);
