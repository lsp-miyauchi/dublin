////////////////////////////////////////////////////////////////////////
// imeman.cpp

#include "stdafx.h"
#include "imeman.h"


//=========================================================================
// IMEに関するあれこれ
//=========================================================================

IMEManager* gpIme = nullptr;



void InitializeImeManager()
{
	gpIme = new IMEManager();
}

void TerminateImeManager()
{
	if (gpIme) {
		delete gpIme;
		gpIme = nullptr;
	}
}

