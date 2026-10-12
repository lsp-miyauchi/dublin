////////////////////////////////////////////////////////////////////////
// imeman.cpp

#include "stdafx.h"
#include "imeman.h"


//=========================================================================
// IMEに関するあれこれ
//=========================================================================

IMEManager* gpIme = NULL;



void InitializeImeManager()
{
	gpIme = new IMEManager();
}

void TerminateImeManager()
{
	if (gpIme) {
		delete gpIme;
		gpIme = NULL;
	}
}

