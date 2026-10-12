//{{NO_DEPENDENCIES}}
//
// FUNCTION PROTOTYPE DEFINITION
//

#pragma once

#include "..\fv7\oeminc.h"

////////////////////////////////////////////////////////////////////////
// ADDTINAL DRIVETYPE
//

#ifndef	DRIVE_UNKNOWN
	#define DRIVE_UNKNOWN		0
#endif
#ifndef	DRIVE_CDROM
	#define DRIVE_CDROM			5
#endif
#ifndef	DRIVE_RAMDISK
	#define DRIVE_RAMDISK		6
#endif
#define DRIVE_NETCLOSED			12

#define DRIVE_FDD525			13
#define DRIVE_MOETC				14
#define DRIVE_SECRET            99

extern TCHAR gszAst[];
extern TCHAR gszFSD[];
extern TCHAR gszFSS[];
extern TCHAR gszCrLf[];
extern TCHAR gszApp[];
extern TCHAR gszLNK[];
extern TCHAR gszICO[];
extern TCHAR gszInvalidToken[];
extern TCHAR gszSafetyToken[];
extern TCHAR gszShell32[];
extern TCHAR gszComCtl32[];
extern TCHAR gszRegRoot[];

////////////////////////////////////////////////////////////////////////
//

#define FVN_DEVICECHANGE		0x0001
#define FVN_MAKEDIR				0x0003

////////////////////////////////////////////////////////////////////////
//
