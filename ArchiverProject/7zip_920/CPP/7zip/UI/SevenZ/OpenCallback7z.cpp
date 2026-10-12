// OpenCallback7z.cpp

#include "StdAfx.h"

#include "OpenCallback7z.h"

#include "SevenZClose.h"
#include "UserInputUtils.h"

HRESULT COpenCallback7z::Open_CheckBreak()
{
	if (N7zClose::TestBreakSignal())
		return E_ABORT;
	return S_OK;
}

HRESULT COpenCallback7z::Open_SetTotal(const UInt64 *, const UInt64 *)
{
	return Open_CheckBreak();
}

HRESULT COpenCallback7z::Open_SetCompleted(const UInt64 *, const UInt64 *)
{
	return Open_CheckBreak();
}
 
HRESULT COpenCallback7z::Open_CryptoGetTextPassword(BSTR *password)
{
	PasswordWasAsked = true;
	RINOK(Open_CheckBreak());
	if (!PasswordIsDefined) {
		Password = GetPassword(OutStream);
		PasswordIsDefined = true;
	}
	return StringToBstr(Password, password);
}

HRESULT COpenCallback7z::Open_GetPasswordIfAny(UString &password)
{
	if (PasswordIsDefined)
		password = Password;
	return S_OK;
}

bool COpenCallback7z::Open_WasPasswordAsked()
{
	return PasswordWasAsked;
}

void COpenCallback7z::Open_ClearPasswordWasAskedFlag()
{
	PasswordWasAsked = false;
}
