/////////////////////////////////////////////////////////////////////
// reg.h

#pragma once
#include "..\lscore\app_registery.h"

class CAppReg : public CAppRegisteryImpl<CAppReg>
{
public:
	DECLARE_APP_REGISTERY_INFO(_T("LightShip"), _T("Applets\\vwhexedit"))
};

