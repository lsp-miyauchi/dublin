///////////////////////////////////////////////////////////////////////
// calc_hash.h

#pragma once
#ifndef __CALC_HASH_H__
#define __CALC_HASH_H__

#include <wincrypt.h>


//
// Crypt関連APIを利用してハッシュを計算する
//
//

const DWORD cdwCalcPartSize = 0x20000;

typedef BOOL (WINAPI* PCALCHASH_PROGRESS_CALLBACK)(DWORDLONG dwlProcessed, DWORDLONG dwlTotal, LPARAM userparam);


class CCryptHash
{
private:
	PCALCHASH_PROGRESS_CALLBACK m_callback;
	LPARAM m_param;
public:
	CCryptHash() : m_callback(NULL), m_param(NULL) { }

	// コールバック登録
	void SetProgressCallback(PCALCHASH_PROGRESS_CALLBACK pfn, LPARAM userparam)
	{
		m_callback = pfn;
		m_param = userparam;
	}
	// MD2ハッシュ計算（128ビット）
	bool CalcMD2(const void* pData, DWORDLONG dwlLen, BYTE pcbHash[16])
	{
		return CalcHash(pData,dwlLen,pcbHash,16, CALG_MD2);
	}
	// MD4ハッシュ計算（128ビット）
	bool CalcMD4(const void* pData ,DWORDLONG dwlLen, BYTE pcbHash[16])
	{
		return CalcHash(pData,dwlLen,pcbHash,16, CALG_MD4);
	}
	// MD5ハッシュ計算（128ビット）
	bool CalcMD5(const void* pData, DWORDLONG dwlLen, BYTE pcbHash[16])
	{
		return CalcHash(pData,dwlLen,pcbHash,16, CALG_MD5);
	}
	// SHA-1ハッシュ計算（160ビット）
	bool CalcSHA1(const void* pData, DWORDLONG dwlLen, BYTE pcbHash[20])
	{
		return CalcHash(pData,dwlLen,pcbHash, 20, CALG_SHA1);
	}
	// SHA-256ハッシュ計算（256ビット）
	bool CalcSHA256(const void* pData, DWORDLONG dwlLen, BYTE pcbHash[32])
	{
		return CalcHash(pData,dwlLen,pcbHash, 32, CALG_SHA_256);
	}
	// SHA-384ハッシュ計算（384ビット）
	bool CalcSHA384(const void* pData, DWORDLONG dwlLen, BYTE pcbHash[48])
	{
		return CalcHash(pData,dwlLen,pcbHash, 48, CALG_SHA_384);
	}
	// SHA-512ハッシュ計算（512ビット）
	bool CalcSHA512(const void* pData, DWORDLONG dwlLen, BYTE pcbHash[64])
	{
		return CalcHash(pData,dwlLen,pcbHash, 64, CALG_SHA_512);
	}
private:
	//	各種ハッシュ計算
	bool CalcHash(const void* pData, DWORDLONG dwlLen, BYTE* pcbHash, DWORD dwHashLen, ALG_ID algorithm)
	{
		if (pData == NULL || dwlLen == 0 || dwHashLen == 0 || dwHashLen > 0x7f) {
			ATLASSERT(0);
			return	false;
		}

		memset(pcbHash, 0, dwHashLen);

		bool ret = false;
		HCRYPTPROV hCryptProv;
		if (::CryptAcquireContext(&hCryptProv, NULL, NULL, PROV_RSA_AES, CRYPT_VERIFYCONTEXT | CRYPT_MACHINE_KEYSET)) {
			HCRYPTHASH hHash;
			if (::CryptCreateHash(hCryptProv, algorithm, 0, 0, &hHash)) {
				DWORDLONG dwlLeft = dwlLen;
				BYTE* dataptr = (BYTE*)pData;
				while (dwlLeft) {
					// コールバック
					if (m_callback && !m_callback(dwlLen - dwlLeft, dwlLen, m_param))
						break;
					DWORD len = (dwlLeft >= cdwCalcPartSize) ? cdwCalcPartSize : (DWORD)dwlLeft;
					if (!::CryptHashData(hHash, dataptr, len, 0))
						break;
					dataptr += len;
					dwlLeft -= len;
				}
				if (!dwlLeft) {
					BYTE pbHash[0x7f];
					if (::CryptGetHashParam(hHash, HP_HASHVAL, pbHash, &dwHashLen, 0)) {
						::memcpy_s(pcbHash, dwHashLen, pbHash, dwHashLen);
						ret = true;
					}
				}
				::CryptDestroyHash(hHash);
			}
			::CryptReleaseContext(hCryptProv,0);
		}
		return ret;
	}
};


#endif		// __CALC_HASH_H__
