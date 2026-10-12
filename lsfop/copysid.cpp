#include "stdafx.h"
#include "fopstd.h"

#define myheapalloc(x) (HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, x))
#define myheapfree(x)  (HeapFree(GetProcessHeap(), 0, x))

static BOOL GetUserAccountName(LPTSTR name)
{
	BOOL ok = FALSE;
	HANDLE hProcess = GetCurrentProcess();
	HANDLE hAccessToken;
	if (OpenProcessToken(hProcess, TOKEN_READ, &hAccessToken)) {
		UCHAR InfoBuffer[1000];
		DWORD dwInfoBufferSize;
		if (GetTokenInformation(hAccessToken, TokenUser, InfoBuffer, sizeof(InfoBuffer), &dwInfoBufferSize)) {
			TCHAR szAccountName[200], szDomainName[200];
			DWORD dwAccountSize = ARRAYSIZE(szAccountName), dwDomainSize = ARRAYSIZE(szDomainName);
			SID_NAME_USE snu;
			PTOKEN_USER pTokenUser = (PTOKEN_USER)InfoBuffer;
			if (LookupAccountSid(NULL, pTokenUser->User.Sid, szAccountName, &dwAccountSize, szDomainName, &dwDomainSize, &snu)) {
				lstrcpy(name, szAccountName);
				ok = TRUE;
			}
		}
	}
	return ok;
}

BOOL CopyAccessRights(LPCTSTR account, LPCTSTR src, LPCTSTR dst)
{
	TCHAR accname[200];
	if (account == NULL) {
		if (!GetUserAccountName(accname))
			return FALSE;
	}
	else
		lstrcpy(accname, account);

	// SID variables.
	TCHAR *        szDomain       = NULL;
	DWORD          cbDomain       = 0;
	LPVOID         pUserSID       = NULL;
	DWORD          cbUserSID      = 0;
	// File SD variables.
	PSECURITY_DESCRIPTOR pFileSD  = NULL;
	DWORD          cbFileSD       = 0;
	// New SD variables.
	SECURITY_DESCRIPTOR newSD;
	// ACL variables.
	PACL           pACL           = NULL;
	BOOL           fDaclPresent;
	BOOL           fDaclDefaulted;
	ACL_SIZE_INFORMATION AclInfo;
	// New ACL variables.
	PACL           pNewACL        = NULL;
	DWORD          cbNewACL       = 0;
	// Temporary ACE.
	LPVOID         pTempAce       = NULL;
	UINT           CurrentAceIndex = 0;
	UINT           newAceIndex = 0;
	// Assume function will fail.
	BOOL           fResult        = FALSE;
	BOOL           ok;

	SECURITY_INFORMATION secInfo = DACL_SECURITY_INFORMATION;

	__try {
		// 
		// STEP 1: 指定のユーザー名またはグループ名のSIDを取得
		// 
		SID_NAME_USE snuType;
		ok = LookupAccountName(NULL, accname, pUserSID, &cbUserSID, szDomain, &cbDomain, &snuType);

		// API should have failed with insufficient buffer.
		if (ok)
			__leave;
		else if (GetLastError() != ERROR_INSUFFICIENT_BUFFER) {
			_tprintf(TEXT("LookupAccountName() failed. Error %d\n"), GetLastError());
			__leave;
		}

		pUserSID = myheapalloc(cbUserSID);
		if (!pUserSID) {
			_tprintf(TEXT("HeapAlloc() failed. Error %d\n"), GetLastError());
			__leave;
		}

		szDomain = (TCHAR *) myheapalloc(cbDomain * sizeof(TCHAR));
		if (!szDomain) {
			_tprintf(TEXT("HeapAlloc() failed. Error %d\n"), GetLastError());
			__leave;
		}

		ok = LookupAccountName(NULL, accname, pUserSID, &cbUserSID, szDomain, &cbDomain, &snuType);
		if (!ok) {
			_tprintf(TEXT("LookupAccountName() failed. Error %d\n"), GetLastError());
			__leave;
		}

		// 
		// STEP 2: ファイルのSD(セキュリティ記述子)のコピーを取得
		// 
		ok = GetFileSecurity(src, secInfo, pFileSD, 0, &cbFileSD);

		// API should have failed with insufficient buffer.
		if (ok)
			__leave;
		else if (GetLastError() != ERROR_INSUFFICIENT_BUFFER) {
			_tprintf(TEXT("GetFileSecurity() failed. Error %d\n"), GetLastError());
			__leave;
		}

		pFileSD = myheapalloc(cbFileSD);
		if (!pFileSD) {
			_tprintf(TEXT("HeapAlloc() failed. Error %d\n"), GetLastError());
			__leave;
		}

		ok = GetFileSecurity(src, secInfo, pFileSD, cbFileSD, &cbFileSD);
		if (!ok) {
			_tprintf(TEXT("GetFileSecurity() failed. Error %d\n"), GetLastError());
			__leave;
		}

		// 
		// STEP 3: 新しいSD(セキュリティ記述子)を初期化
		// 
		if (!InitializeSecurityDescriptor(&newSD, SECURITY_DESCRIPTOR_REVISION)) {
			_tprintf(TEXT("InitializeSecurityDescriptor() failed. Error %d\n"), GetLastError());
			__leave;
		}

		// 
		// STEP 4: SD内のDACLへのポインタを取得
		// 
		if (!GetSecurityDescriptorDacl(pFileSD, &fDaclPresent, &pACL, &fDaclDefaulted)) {
			_tprintf(TEXT("GetSecurityDescriptorDacl() failed. Error %d\n"), GetLastError());
			__leave;
		}

		// 
		// STEP 5: ファイルの DACL のサイズ情報をACL_SIZE_INFORMATION構造体として取得
		// この情報は新しい DACL のサイズを計算するときや、ACE をコピーするときに使用する
		// 
		AclInfo.AceCount = 0; // Assume NULL DACL.
		AclInfo.AclBytesFree = 0;
		AclInfo.AclBytesInUse = sizeof(ACL);

		if (pACL == NULL)
			fDaclPresent = FALSE;

		// If not NULL DACL, gather size information from DACL.
		if (fDaclPresent) {
			if (!GetAclInformation(pACL, &AclInfo, sizeof(ACL_SIZE_INFORMATION), AclSizeInformation)) {
				_tprintf(TEXT("GetAclInformation() failed. Error %d\n"), GetLastError());
				__leave;
			}
		}

		// 
		// STEP 6: 新しいDACLに割り当てるバイト数を計算
		// 
		cbNewACL = AclInfo.AclBytesInUse + sizeof(ACCESS_ALLOWED_ACE) + GetLengthSid(pUserSID) - sizeof(DWORD);

		// 
		// STEP 7: 新しい ACL にメモリを割り当て。最終的にはここにファイルの既存のACEとアクセス許可ACEが格納される
		// 
		pNewACL = (PACL) myheapalloc(cbNewACL);
		if (!pNewACL) {
			_tprintf(TEXT("HeapAlloc() failed. Error %d\n"), GetLastError());
			__leave;
		}

		// 
		// STEP 8: ACL 構造体を初期化
		// 
		if (!InitializeAcl(pNewACL, cbNewACL, ACL_REVISION2)) {
			_tprintf(TEXT("InitializeAcl() failed. Error %d\n"), GetLastError());
			__leave;
		}

		// 
		// STEP 9: ファイルの SD 内に DACL があるかどうかを確認
		//
		// DACL がない場合は、新しい DACL にファイルの ACE をコピーするコードの実行を省略
		// DACL がある場合は、古い DACL のすべての ACE を新しい DACL にコピー
		// 古い DACL が既に Windows 2000 の標準順序になっていることを前提としています。
		// 新しい DACL が確実に新しい Windows 2000 の標準順序になるように、
		// まず、ACE の種類に関係なく古い DACL のすべての非継承 ACE を新しい DACL にコピーします。
		// ACL_SIZE_INFORMATION 構造体の AceCount メンバの値は、DACL 内の ACE の数を示します。
		// 各 ACE につき 1 回ずつループします。
		// 
		
		newAceIndex = 0;
		if (fDaclPresent && AclInfo.AceCount) {
			for (CurrentAceIndex = 0; CurrentAceIndex < AclInfo.AceCount; CurrentAceIndex++) {
				// 
				// STEP 10: Get an ACE.
				// 
				if (!GetAce(pACL, CurrentAceIndex, &pTempAce)) {
					_tprintf(TEXT("GetAce() failed. Error %d\n"), GetLastError());
					__leave;
				}

				// 
				// STEP 11:取得した ACE が非継承 ACE であるかどうかを調べる
				// 
				// この 1 つ目の for ループでは、非継承 ACE のみをコピーする
				// 取得した ACE が継承 ACE である場合は、この for ループ内の処理は省略
				// このようにすることで、既存の非継承 ACE をすべてコピーした直後に、
				// 新しいアクセス許可非継承 ACE を正しい場所に追加できる
				// 
				if (((ACCESS_ALLOWED_ACE *)pTempAce)->Header.AceFlags & INHERITED_ACE)
					break;

				// 
				// STEP 12: SID が指定のアカウントと一致する ACE は追加しない
				// これは別のアクセス マスクでアクセス許可 ACE を追加するため
				// 
				if (EqualSid(pUserSID, &(((ACCESS_ALLOWED_ACE *)pTempAce)->SidStart)))
					continue;

				// 
				// STEP 13: ACE を新しい ACL にコピー
				//
				// AddAce関数の dwStartingAceIndex パラメータに MAXDWORD を渡して、DACL の末尾に ACE が追加されるようにする
				// ACE_HEADER の AceSize メンバは ACE のサイズを示す
				// 
				if (!AddAce(pNewACL, ACL_REVISION, MAXDWORD, pTempAce, ((PACE_HEADER) pTempAce)->AceSize)) {
					_tprintf(TEXT("AddAce() failed. Error %d\n"), GetLastError());
					__leave;
				}

				newAceIndex++;
			}
		}
/* ---
		// 
		// STEP 14: ファイルの既存の非継承 ACE をすべて新しい ACL にコピーし終えたら、
		// dwAccessMask の値に基づいてアクセス許可 ACE を追加する
		// ここで追加する新しい ACE は正しい標準順序となり、既存の非継承 ACE すべての直後に追加される
		// 
		if (!AddAccessAllowedAce(pNewACL, ACL_REVISION2, dwAccessMask, pUserSID)) {
			_tprintf(TEXT("AddAccessAllowedAce() failed. Error %d\n"), GetLastError());
			__leave;
		}
--- */
		// 
		// STEP 15: 古いDACLがあったら、古いDACL内のCurrentAceIndex以降の残りの継承 ACE を新しい DACL にコピーする
		// 各継承 ACE につき 1 回ずつループする
		// 
		if (fDaclPresent && AclInfo.AceCount) {
			for (; CurrentAceIndex < AclInfo.AceCount; CurrentAceIndex++) {
				// 
				// STEP 16: ファイルの既存の DACL 内の ACE へのポインタを取得
				// 
				if (!GetAce(pACL, CurrentAceIndex, &pTempAce)) {
					_tprintf(TEXT("GetAce() failed. Error %d\n"), GetLastError());
					__leave;
				}

				// 
				// STEP 17: ACE を新しい ACL にコピーする
				// AddAce 関数の dwStartingAceIndex パラメータに MAXDWORD を渡して、DACL の末尾に ACE が追加されるようにする
				// ACE_HEADER の AceSize メンバは ACE のサイズを示す
				// 
				if (!AddAce(pNewACL, ACL_REVISION, MAXDWORD, pTempAce, ((PACE_HEADER) pTempAce)->AceSize)) {
					_tprintf(TEXT("AddAce() failed. Error %d\n"), GetLastError());
					__leave;
				}
			}
		}

		// 
		// STEP 18: 新しい DACL を新しい SD に設定
		// 
		if (!SetSecurityDescriptorDacl(&newSD, TRUE, pNewACL, FALSE)) {
			_tprintf(TEXT("SetSecurityDescriptorDacl() failed. Error %d\n"), GetLastError());
			__leave;
		}

		// 
		// STEP 19: 古いセキュリティ記述子の DACL 自動継承に関するコントロールフラグを、新しいセキュリティ記述子にコピーする
		// 
		SECURITY_DESCRIPTOR_CONTROL cboi = 0;
		SECURITY_DESCRIPTOR_CONTROL cbts = 0;
		SECURITY_DESCRIPTOR_CONTROL ocb = 0;
		DWORD rev = 0;
		if (!GetSecurityDescriptorControl(pFileSD, &ocb, &rev)) {
			_tprintf(TEXT("GetSecurityDescriptorControl() failed. Error %d\n"), GetLastError());
			__leave;
		}

		if (ocb & SE_DACL_AUTO_INHERITED) {
			cboi = SE_DACL_AUTO_INHERIT_REQ | SE_DACL_AUTO_INHERITED;
			cbts = cboi;
		}
		else if (ocb & SE_DACL_PROTECTED) {
			cboi = SE_DACL_PROTECTED;
			cbts = cboi;
		}
		
		if (cboi) {
			if (!SetSecurityDescriptorControl(&newSD, cboi, cbts)) {
				_tprintf(TEXT("SetSecurityDescriptorControl() failed. Error %d\n"), GetLastError());
				__leave;
			}
		}

		// 
		// STEP 20: 新しい SD をファイルに設定
		// DACL_SECURITY_INFORMATION フラグを指定しているので、新しい SD 内の DACL がファイルの SD に適用される
		// ファイルの DACL のみが設定され、ファイルの SD 内のそれ以外のセキュリティ情報は変わらない
		// 
		if (!SetFileSecurity(dst, secInfo, &newSD)) {
			_tprintf(TEXT("SetFileSecurity() failed. Error %d\n"), GetLastError());
			__leave;
		}

		fResult = TRUE;
	}
	__finally {
		if (pUserSID) myheapfree(pUserSID);
		if (szDomain) myheapfree(szDomain);
		if (pFileSD) myheapfree(pFileSD);
		if (pNewACL) myheapfree(pNewACL);
	}
	return fResult;
}
