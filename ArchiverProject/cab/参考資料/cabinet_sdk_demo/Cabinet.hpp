///////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Author: Luuk Weltevreden
//
// Date: 23-07-2004
//
// Filename: Cabinet.hpp
//
// Classes:
// - CCabinetT<T>
// - CCabinetExT<T>
// - CResourceCabinetT<T>
// - CCabinet
// - CResourceCabinet
//
// Purpose: These templates provide means to extract files from cabinet files.
//

#pragma once

#include "FDI.h"
#include <io.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <errno.h>
#include <hash_set>

template <class T>
class CCabinetT
{
public:
	///////////////////////////////////////////////////////////////////////////////////////////////////////////
	// The constructor makes sure the member variables are intitialized.
	//
	CCabinetT() : m_hFDIContext(NULL)
		{ m_erf.fError = FALSE;}

	///////////////////////////////////////////////////////////////////////////////////////////////////////////
	// The destructor makes sure the contexts are destroyed.
	//
	~CCabinetT()
		{ DestroyFDIContext(); }

	///////////////////////////////////////////////////////////////////////////////////////////////////////////
	// This function aborts the currently active operation.
	//
	void AbortOperation()
		{ m_bAbort = true; }

	///////////////////////////////////////////////////////////////////////////////////////////////////////////
	// This function extracts one or more files from the specified cabinet.
	//
	// Parameters:
	// - LPCSTR lpszCabinet
	//     Full path to the cabinet file.
	// - LPCSTR lpszTarget
	//     Target directory.
	// - void * pParam
	//     User defined value which will be passed to the callback function.
	//
	// Returns:
	//   true if the operation is succesful or false otherwise.
	//
	bool Copy(LPSTR lpszCabinet, LPCSTR lpszTarget, void * pParam = NULL)
	{
		char szName[MAX_PATH];
		char szPath[MAX_PATH];
		char szDir[MAX_PATH] = "";

		if (!m_hFDIContext)
			return false;

		// Verify the paths
		if (!lpszCabinet || (strlen(lpszCabinet) >= MAX_PATH))
			return false;

		if (!lpszTarget)
			lpszTarget = szDir;
		else
		{
			size_t size = strlen(lpszTarget);
			if (lpszTarget[size] != '\\' && (size + 1 < MAX_PATH))
			{
				strcpy(szDir, lpszTarget);
				strcat(szDir, "\\");
			}
			else if (size < MAX_PATH)
				strcpy(szDir, lpszTarget);
			else
				return false;
		}

		// Split the path to a filename and path
		char * p = strrchr(lpszCabinet, '\\');
		if (p == NULL)
		{
			strcpy(szName, lpszCabinet);
			strcpy(szPath, "");
		}
		else
		{
			strcpy(szName, p+1);
			strncpy(szPath, lpszCabinet, (int)(p-lpszCabinet)+1);
			szPath[(int)(p-lpszCabinet)+1] = 0;
		}

		// Initialize the class
		m_pParam = pParam;
		m_bAbort = false;
		m_szTargetDir = szDir;

		// Execute the operation
		BOOL bRet = FDICopy(m_hFDIContext, szName, szPath, 0, (PFNFDINOTIFY)(FDICallback), NULL, this);
		
		// Return success value
		return (bRet == TRUE);
	}

	///////////////////////////////////////////////////////////////////////////////////////////////////////////
	// Creates a new FDI context and returns true if succesful or false otherwise.
	//
	bool CreateFDIContext()
	{
		if (m_hFDIContext)
			return false;

		m_hFDIContext = FDICreate(
			(PFNALLOC)(_Alloc),
			(PFNFREE)(_Free),
			(PFNOPEN)(FDIOpen),
			(PFNREAD)(FDIRead),
			(PFNWRITE)(FDIWrite),
			(PFNCLOSE)(FDIClose),
			(PFNSEEK)(FDISeek),
			cpu80386,
			&m_erf
			);

		return (m_hFDIContext != NULL);
	}

	///////////////////////////////////////////////////////////////////////////////////////////////////////////
	// Destroys the FDI context used by this instance and returns true if succesful or false otherwise.
	//
	bool DestroyFDIContext()
	{
		if (!m_hFDIContext)
			return true;

		BOOL bRet = FDIDestroy(m_hFDIContext);
		if (bRet)
			m_hFDIContext = NULL;

		return (bRet == TRUE);
	}

	///////////////////////////////////////////////////////////////////////////////////////////////////////////
	// Determines whether the cabinet with the specified handle is a valid cabinet. If it is, the structure
	// which is pointed to by pfdici will be filled with information about the cabinet file. This parameter
	// can be NULL in which case no information about the cabinet file will be returned.
	//
	bool IsCabinet(int handle, PFDICABINETINFO pfdici = NULL)
	{
		if (!m_hFDIContext)
			return false;

		if (!pfdici)
		{
			FDICABINETINFO fdici;
			return (FDIIsCabinet(m_hFDIContext, handle, &fdici) == TRUE);
		}

		return (FDIIsCabinet(m_hFDIContext, handle, pfdici) == TRUE);
	}

	bool IsCabinet(LPSTR szFileName, PFDICABINETINFO pfdici = NULL)
	{
		int handle = FDIOpen(szFileName, _O_BINARY | _O_RDONLY | _O_SEQUENTIAL, 0);
		if (!handle)
			return false;

		BOOL bRet = IsCabinet(handle, pfdici);

		FDIClose(handle);

		return (bRet == TRUE);
	}

	///////////////////////////////////////////////////////////////////////////////////////////////////////////
	// Returns the last error that has occured. Some errors have optional error information. This can be
	// retrieved by specifying a pointer to the integer that is to hold the optional error information.
	//
	int GetLastError(int * pnExtra = NULL)
	{
		if (!m_erf.fError)
			return 0;

		if (pnExtra)
			*pnExtra = m_erf.erfType;

		return m_erf.erfOper;
	}

	///////////////////////////////////////////////////////////////////////////////////////////////////////////
	// Structures used as parameters for the notification functions.
	//
	typedef struct CABINETINFO
	{
		LPSTR szNextCabinet;  // Name of the next cabinet excluding path info
		LPSTR szNextDisk;     // Name of the next disk
		LPSTR szPath;         // Path to the cabinet
		USHORT uSetID;        // The set ID of this cabinet
		USHORT uCabinet;      // The cabinet number within the cabinet set (starting at 0)
	};

	typedef struct CABINETFILEINFO
	{
		LPSTR szFile;    // The name of the file
		LONG lSize;      // Uncompressed file size
		FILETIME ftFile; // Time and date of the file
		USHORT uAttribs; // Attributes of the file
	};

protected:
	///////////////////////////////////////////////////////////////////////////////////////////////////////////
	// Allocates a new block of memory and returns the pointer to the newly allocated memory. This function
	// can be overridden.
	//
	static void * Alloc(size_t size)
		{ return operator new(size); }

	///////////////////////////////////////////////////////////////////////////////////////////////////////////
	// Frees a block of memory allocated by Alloc. This function can be overridden.
	//
	static void Free(void * memblock)
		{ operator delete(memblock); }

	///////////////////////////////////////////////////////////////////////////////////////////////////////////
	// Opens a Cabinet. This function has to have identical parameters and return values as '_open'. See the
	// documentation for '_open' function for more information on the parameters and return values. This
	// function can be overridden.
	//
	static int Open(const char * filename, int oflag, int pmode)
		{ return _open(filename, oflag, pmode); }

	///////////////////////////////////////////////////////////////////////////////////////////////////////////
	// Reads data from a cabinet opened by Open. This function has to have identical parameters and return
	// values as '_read'. See the documentation for '_read' function for more information on the parameters
	// and return values. This function can be overridden.
	//
	static int Read(int fp, void * buffer, unsigned int count)
		{ return _read(fp, buffer, count); }

	///////////////////////////////////////////////////////////////////////////////////////////////////////////
	// Writes data to a cabinet opened by Open. This function has to have identical parameters and return values
	// as '_write'. See the documentation for '_write' function for more information on the parameters and
	// return values. This function can be overridden.
	//
	static int Write(int fp, const void * buffer, unsigned int count)
		{ return _write(fp, buffer, count); }

	///////////////////////////////////////////////////////////////////////////////////////////////////////////
	// Closes a cabinet opened by Open. This function has to have identical parameters and return values as
	// '_close'. See the documentation for '_close' function for more information on the parameters and return
	// values. This function can be overridden.
	//
	static int Close(int fp)
		{ return _close(fp); }

	///////////////////////////////////////////////////////////////////////////////////////////////////////////
	// Moves the cursor in the cabinet file opened by Open. This function has to have identical parameters
	// and return values as '_lseek'. See the documentation for '_lseek' function for more information on the
	// parameters and return values. This function can be overridden.
	//
	static long Seek(int fp, long offset, int origin)
		{ return _lseek(fp, offset, origin); }

	///////////////////////////////////////////////////////////////////////////////////////////////////////////
	// Handle to the FDI context.
	//
	HFDI m_hFDIContext;

	///////////////////////////////////////////////////////////////////////////////////////////////////////////
	// Struct that is used for storing error information.
	//
	ERF m_erf;

	///////////////////////////////////////////////////////////////////////////////////////////////////////////
	// User defined parameter to pass to the callback function.
	//
	void * m_pParam;

	///////////////////////////////////////////////////////////////////////////////////////////////////////////
	// Flag that can be set to abort the current operation.
	//
	bool m_bAbort;

	///////////////////////////////////////////////////////////////////////////////////////////////////////////
	// Holds a pointer to the name of the target directory.
	//
	LPCSTR m_szTargetDir;

private:
	///////////////////////////////////////////////////////////////////////////////////////////////////////////
	// Functions used by the FDI context.
	//
	static int FDIOpen(const char * filename, int oflag, int pmode)
		{ return T::Open(filename, oflag, pmode); }

	static int FDIRead(int fd, void * buffer, unsigned int count)
		{ return T::Read(fd, buffer, count); }

	static int FDIWrite(int fd, const void * buffer, unsigned int count)
		{ return T::Write(fd, buffer, count); }

	static int FDIClose(int fd)
		{ return T::Close(fd); }

	static long FDISeek(int fd, long offset, int origin)
		{ return T::Seek(fd, offset, origin); }

	///////////////////////////////////////////////////////////////////////////////////////////////////////////
	// Memory management.
	//
	static void * _Alloc(size_t size)
		{ return T::Alloc(size); }

	static void _Free(void * memblock)
		{ T::Free(memblock); }

	///////////////////////////////////////////////////////////////////////////////////////////////////////////
	// Callback function for the FDI context. This will get used when the Copy function is called. This
	// function will call the appropriate handlers which can be overridden to change the default behaviour.
	//
	static int FDICallback(FDINOTIFICATIONTYPE fdint, PFDINOTIFICATION pfdin)
	{
		int nRet = 0; // Allow unsupported notifications to continue

		// Get pointer to the instance
		CCabinetT<T> * pThis = (CCabinetT<T> *)(pfdin->pv);

		// Abort if necessary
		if (pThis->m_bAbort)
			return -1;

		// Relay all currently known notifications
		switch (fdint)
		{
		case fdintCABINET_INFO:
			{
				CABINETINFO ci = {
					pfdin->psz1,
					pfdin->psz2,
					pfdin->psz3,
					pfdin->setID,
					pfdin->iCabinet
				};

				static_cast<T *>(pThis)->OnCabinetInfo(ci, pThis->m_pParam);	
			}
			break;

		case fdintCOPY_FILE:
			{
				CABINETFILEINFO cfi;
				char szPath[MAX_PATH] = "";

				// Copy target directory
				strcpy(szPath, pThis->m_szTargetDir);

				// Split the path to a filename and path
				char * p = strrchr(pfdin->psz1, '\\');
				if (p == NULL)
					cfi.szFile = pfdin->psz1;
				else
				{
					cfi.szFile = p+1;
					size_t size = (int)(p-pfdin->psz1)+strlen(szPath)+1;
					if (size + 1 >= MAX_PATH)
						return 0;
					strncat(szPath, pfdin->psz1, size);
					szPath[size] = 0;
				}

				// Fill rest of the struct
				cfi.lSize = pfdin->cb;
				cfi.uAttribs = pfdin->attribs;
				DosDateTimeToFileTime(pfdin->date, pfdin->time, &cfi.ftFile);

				// Call notification function
				if (!static_cast<T *>(pThis)->OnCopyFile(cfi, szPath, pThis->m_pParam))
					nRet = 0;
				else
				{
					char szFolder[MAX_PATH];

					// Make sure the directory exists
					p = strchr(szPath, '\\');
					while (p != NULL)
					{
						strncpy(szFolder, szPath, (p - szPath));
						szFolder[p - szPath] = 0;
						CreateDirectory(szFolder, NULL);
						p = strchr(p+1, '\\');
					}

					// Create the file
					strncat(szPath, cfi.szFile, MAX_PATH);
					nRet = FDIOpen(szPath, _O_BINARY | _O_CREAT | _O_WRONLY | _O_SEQUENTIAL, _S_IREAD | _S_IWRITE);
				}
			}
			break;

		case fdintCLOSE_FILE_INFO:
			{
				char szPath[MAX_PATH] = {0};

				// Close the file
				FDIClose(pfdin->hf);

				// Set the file attributes
				strcpy(szPath, pThis->m_szTargetDir);
				strncat(szPath, pfdin->psz1, MAX_PATH);
				SetAttributesAndDate(szPath, pfdin->date, pfdin->time, pfdin->attribs);

				// Call notification function
				static_cast<T *>(pThis)->OnCopyFileComplete(pfdin->psz1, pThis->m_pParam);
				nRet = TRUE;
			}
			break;

		case fdintNEXT_CABINET :
			{
				CABINETINFO ci = {
					pfdin->psz1,
					pfdin->psz2,
					pfdin->psz3,
					pfdin->setID,
					pfdin->iCabinet
				};

				static_cast<T *>(pThis)->OnNextCabinet(ci, pfdin->fdie, pThis->m_pParam);
			}
			break;
		}

		return nRet;
	}

	///////////////////////////////////////////////////////////////////////////////////////////////////////////
	// Helper function that sets the date and attributes for the specified file.
	//
	static void SetAttributesAndDate(LPCSTR lpszFileName, USHORT uDate, USHORT uTime, USHORT uAttribs)
	{
		HANDLE hFile = CreateFile(
			lpszFileName,
			GENERIC_READ | GENERIC_WRITE,
			FILE_SHARE_READ,
			NULL,
			OPEN_EXISTING,
			FILE_ATTRIBUTE_NORMAL,
			NULL
			);

		if (hFile != INVALID_HANDLE_VALUE)
		{
			FILETIME ftFile;
			if (DosDateTimeToFileTime(uDate, uTime, &ftFile))
			{
				FILETIME ftLocal;
				if (LocalFileTimeToFileTime(&ftFile, &ftLocal))
					SetFileTime(hFile, &ftLocal, NULL, &ftLocal);
			}

			CloseHandle(hFile);
		}

		SetFileAttributes(lpszFileName, uAttribs & (FILE_ATTRIBUTE_READONLY | FILE_ATTRIBUTE_HIDDEN | FILE_ATTRIBUTE_SYSTEM | FILE_ATTRIBUTE_ARCHIVE));
	}

	///////////////////////////////////////////////////////////////////////////////////////////////////////////
	// This function will be called exactly once for each cabinet opened by Copy, including continuation
	// cabinets opened due to files spanning cabinet boundaries. This function can be overridden.
	//
	void OnCabinetInfo(CABINETINFO&, LPVOID)
		{ }

	///////////////////////////////////////////////////////////////////////////////////////////////////////////
	// This function will be called for each file in a cabinet at the moment it is about to be extracted. This
	// function can be overridden.
	//
	// Return true to extract the file to the path specified by the second parameter or false to skip.
	//
	bool OnCopyFile(CABINETFILEINFO&, LPCSTR, LPVOID)
		{ return true; }

	///////////////////////////////////////////////////////////////////////////////////////////////////////////
	// This function will be called when a file is succesfully extracted.
	//
	void OnCopyFileComplete(LPCSTR, LPVOID)
		{ }

	///////////////////////////////////////////////////////////////////////////////////////////////////////////
	// This function will be called when the next cabinet file in the sequence needs to be opened. The path
	// has to be verified and can be changed if necessary. If the cabinet file cannot be opened this function
	// will be called again with the second parameter set to an error that describes the problem.
	//
	void OnNextCabinet(CABINETINFO&, FDIERROR, LPVOID)
		{ }
};

class CCabinet : public CCabinetT<CCabinet> { };

template <class T>
class CCabinetExT : public CCabinetT<T>
{
public:
	///////////////////////////////////////////////////////////////////////////////////////////////////////////
	// Override this function to specify the name of the extension.
	//
	static LPCSTR GetIdentifier()
		{ return _T(""); }

	///////////////////////////////////////////////////////////////////////////////////////////////////////////
	// This function will build a valid name that can be used to pass to the Copy function so it can make use
	// of this extension.
	//
	// Parameters:
	// - LPCTSTR lpszCabinet
	//     The name of the cabinet.
	// - LPTSTR lpszBuffer
	//     The buffer which will hold the full resource name when it's build.
	// - size_t size
	//     Size of the buffer in characters.
	//
	// Returns:
	//   If lpszBuffer is NULL it will return the required size of the buffer (excluding null terminating 0).
	//   If lpszBuffer is a valid pointer it will return TRUE if succesful or FALSE otherwise.
	//
	size_t BuildExName(LPCSTR lpszCabinet, LPSTR lpszBuffer, size_t size)
	{
		// Get the identifier
		LPCSTR szIdentifier = _GetIdentifier();

		// Calculate the required size
		size_t sizeReq = strlen(lpszCabinet) + strlen(szIdentifier);

		// Return the length if lpszBuffer is NULL
		if (!lpszBuffer)
			return sizeReq;

		// Check if the buffer is big enough to hold the data
		if (sizeReq + 1 > size)
			return FALSE;

		// Build the string
		strcpy(lpszBuffer, szIdentifier);
		strcat(lpszBuffer, lpszCabinet);

		// Success
		return TRUE;
	}

protected:
	///////////////////////////////////////////////////////////////////////////////////////////////////////////
	// These function makes it possible to use the cabinet classes on objects other than files. This is done
	// by parsing the filename, checking if it matches the identifier and if so call different callback
	// functions than normal. To use this class override the 'Ex' functions and the 'GetIdentifier' function.
	//
	static int Open(const char * filename, int oflag, int pmode)
	{
		int nRet = -1;

		// Get the identifier
		LPCSTR szIdentifier = _GetIdentifier();
		size_t size = strlen(szIdentifier);

		// Check if the target is a resource
		if (strncmp(filename, szIdentifier, size))
			nRet = CCabinetT<T>::Open(filename, oflag, pmode);
		else
		{
			nRet = T::OpenEx(filename + size, oflag, pmode);
			if (nRet != -1)
				GetHashSet().insert(nRet);
		}

		// Return the handle
		return nRet;
	}

	static int Read(int fp, void * buffer, unsigned int count)
	{
		HANDLES_SET& set = GetHashSet();
		if (set.find(fp) != set.end())
			return T::ReadEx(fp, buffer, count);

		return CCabinetT<T>::Read(fp, buffer, count);
	}

	static int Write(int fp, const void * buffer, unsigned int count)
	{
		HANDLES_SET& set = GetHashSet();
		if (set.find(fp) != set.end())
			return T::WriteEx(fp, buffer, count);

		return CCabinetT<T>::Write(fp, buffer, count);
	}

	static int Close(int fp)
	{
		int nRet;

		HANDLES_SET& set = GetHashSet();
		HANDLES_SET::iterator it = set.find(fp);
		if (it != set.end())
		{
			nRet = T::CloseEx(fp);
			set.erase(it);
		}
		else
			nRet = CCabinetT<T>::Close(fp);

		return nRet;
	}

	static long Seek(int fp, long offset, int origin)
	{
		HANDLES_SET& set = GetHashSet();
		if (set.find(fp) != set.end())
			return T::SeekEx(fp, offset, origin);

		return CCabinetT<T>::Seek(fp, offset, origin);
	}

private:
	///////////////////////////////////////////////////////////////////////////////////////////////////////////
	// Overwrite the following functions to implement your extension. The parameters and return values should
	// be identical to the normal functions.
	//
	static int OpenEx(const char * filename, int oflag, int pmode)
		{ return CCabinetT<T>::Open(filename, oflag, pmode); }

	static int ReadEx(int fp, void * buffer, unsigned int count)
		{ return CCabinetT<T>::Read(fp, buffer, count); }

	static int WriteEx(int fp, const void * buffer, unsigned int count)
		{ return CCabinetT<T>::Write(fp, buffer, count); }

	static int CloseEx(int fp)
		{ return CCabinetT<T>::Close(fp); }

	static long SeekEx(int fp, long offset, int origin)
		{ return CCabinetT<T>::Seek(fp, offset, origin); }

	///////////////////////////////////////////////////////////////////////////////////////////////////////////
	// Returns the identifier of this extension.
	//
	static LPCSTR _GetIdentifier()
		{ return T::GetIdentifier(); }

	///////////////////////////////////////////////////////////////////////////////////////////////////////////
	// Typedef used to reduce typing.
	//
	typedef stdext::hash_set<int> HANDLES_SET;

	///////////////////////////////////////////////////////////////////////////////////////////////////////////
	// This set holds all handles used by extension.
	//
	static HANDLES_SET& GetHashSet()
	{
		static HANDLES_SET set;
		return set;
	}

	///////////////////////////////////////////////////////////////////////////////////////////////////////////
	// Declare this class as a friend so it can access the protected members.
	//
	friend class CCabinetT<T>;
};

template <class T>
class CResourceCabinetT : public CCabinetExT<T>
{
public:
	///////////////////////////////////////////////////////////////////////////////////////////////////////////
	// The identifier the class uses to separate file names from resource names.
	//
	static LPCSTR GetIdentifier()
	{ 
		static const char szIdentifier[] = "//Resource\\";
		return szIdentifier;
	}

	///////////////////////////////////////////////////////////////////////////////////////////////////////////
	// This function will build a valid name that can be used to pass to the Copy function so it can make use
	// of this extension. See BuildExName for more details.
	//
	size_t BuildResourceName(LPCSTR lpszResourceName, LPCSTR lpszResourceTypeName, LPSTR lpszBuffer, size_t size)
	{
		// Get the identifier
		LPCSTR szIdentifier = GetIdentifier();

		// Calculate the required size
		size_t sizeReq = strlen(lpszResourceName) + strlen(lpszResourceTypeName) + strlen(szIdentifier) + 1;

		// Return the length if lpszBuffer is NULL
		if (!lpszBuffer)
			return sizeReq;

		// Check if the buffer is big enough to hold the data
		if (sizeReq + 1 > size)
			return FALSE;

		// Build the string
		strcpy(lpszBuffer, szIdentifier);
		strcat(lpszBuffer, lpszResourceTypeName);
		strcat(lpszBuffer, "\\");
		strcat(lpszBuffer, lpszResourceName);

		// Success
		return TRUE;
	}

	///////////////////////////////////////////////////////////////////////////////////////////////////////////
	// Helper functions that will relieve you from having to build the resource string yourself. See Copy
	// from the base class for more details.
	//
	bool CopyFromResource(UINT uResourceID, UINT uResourceTypeID, LPCSTR szTarget, void * pParam = NULL)
		{ return CopyFromResource(MAKEINTRESOURCE(uResourceID), MAKEINTRESOURCE(uResourceTypeID), szTarget, pParam); }

	bool CopyFromResource(UINT uResourceID, LPCSTR lpszResourceTypeName, LPCSTR szTarget, void * pParam = NULL)
		{ return CopyFromResource(MAKEINTRESOURCE(uResourceID), lpszResourceTypeName, szTarget, pParam); }

	bool CopyFromResource(LPCSTR lpszResourceName, UINT uResourceTypeID, LPCSTR szTarget, void * pParam = NULL)
		{ return CopyFromResource(lpszResourceName, MAKEINTRESOURCE(uResourceTypeID), szTarget, pParam); }

	bool CopyFromResource(LPCSTR lpszResourceName, LPCSTR lpszResourceTypeName, LPCSTR szTarget, void * pParam = NULL)
	{
		LPCSTR pszResName = lpszResourceName;
		LPCSTR pszResType = lpszResourceTypeName;
		char szTemp1[12] = {0};
		char szTemp2[12] = {0};

		// Make string resources if necessary
		if (IS_INTRESOURCE(lpszResourceName))
		{
			itoa((int)(PtrToInt(lpszResourceName)), szTemp1, 10);
			memmove(szTemp1 + 1, szTemp1, strlen(szTemp1));
			szTemp1[0] = '#';
			pszResName = szTemp1;
		}

		if (IS_INTRESOURCE(lpszResourceTypeName))
		{
			itoa((int)(PtrToInt(lpszResourceTypeName)), szTemp2, 10);
			memmove(szTemp2 + 1, szTemp2, strlen(szTemp2));
			szTemp2[0] = '#';
			pszResType = szTemp2;
		}

		// Build the name
		size_t size = BuildResourceName(pszResName, pszResType, NULL, 0) + 1;
		LPSTR szCabinet = new char[size];
		if (!szCabinet)
			return false;

		BuildResourceName(pszResName, pszResType, szCabinet, size);

		// Pass it on to the base class
		bool bRet = Copy(szCabinet, szTarget, pParam);
		
		// Free memory
		delete [] szCabinet;

		return bRet;
	}

	///////////////////////////////////////////////////////////////////////////////////////////////////////////
	// Helper functions that will relieve you from having to build the resource string yourself. See IsCabinet
	// from the base class for more details.
	//
	bool IsResourceCabinet(UINT uResourceID, UINT uResourceType, PFDICABINETINFO pfdici = NULL)
		{ return IsResourceCabinet(MAKEINTRESOURCE(uResourceID), MAKEINTRESOURCE(uResourceTypeID), pfdici); }

	bool IsResourceCabinet(UINT uResourceID, LPCSTR lpszResourceTypeName, PFDICABINETINFO pfdici = NULL)
		{ return IsResourceCabinet(MAKEINTRESOURCE(uResourceID), lpszResourceTypeName, pfdici); }

	bool IsResourceCabinet(LPCSTR lpszResourceName, UINT uResourceTypeID, PFDICABINETINFO pfdici = NULL)
		{ return IsResourceCabinet(lpszResourceName, MAKEINTRESOURCE(uResourceTypeID), pfdici); }

	bool IsResourceCabinet(LPCSTR lpszResourceName, LPCSTR lpszResourceTypeName, PFDICABINETINFO pfdici = NULL)
	{
		LPCSTR pszResName = lpszResourceName;
		LPCSTR pszResType = lpszResourceTypeName;
		char szTemp1[12] = {0};
		char szTemp2[12] = {0};

		// Make string resources if necessary
		if (IS_INTRESOURCE(lpszResourceName))
		{
			itoa((int)(PtrToInt(lpszResourceName)), szTemp1, 10);
			memmove(szTemp1 + 1, szTemp1, strlen(szTemp1));
			szTemp1[0] = '#';
			pszResName = szTemp1;
		}

		if (IS_INTRESOURCE(lpszResourceTypeName))
		{
			itoa((int)(PtrToInt(lpszResourceTypeName)), szTemp2, 10);
			memmove(szTemp2 + 1, szTemp2, strlen(szTemp2));
			szTemp2[0] = '#';
			pszResType = szTemp2;
		}

		// Build the name
		size_t size = BuildResourceName(pszResName, pszResType, NULL, 0) + 1;
		LPSTR szCabinet = new char[size];
		if (!szCabinet)
			return false;

		BuildResourceName(pszResName, pszResType, szCabinet, size);

		// Pass it on to the base class
		bool bRet = IsCabinet(szCabinet, pfdici);

		// Free memory
		delete [] szCabinet;

		return bRet;
	}

private:
	///////////////////////////////////////////////////////////////////////////////////////////////////////////
	// This struct is used to store information about the resource we are examining.
	//
	typedef struct HRESEX
	{
		void * address;
		long ptr;
		size_t size;
	};

	///////////////////////////////////////////////////////////////////////////////////////////////////////////
	// This function opens the specified resource from the current module.
	//
	static int OpenEx(const char * filename, int, int)
	{
		int nRet = -1;

		// Split the filename
		char * p = strrchr(filename, '\\');
		if (p == NULL)
			return -1;

		// Create buffers
		size_t size = strlen(filename);
		LPSTR szResourceName = new char[size + 1];
		if (!szResourceName)
			return -1;

		LPSTR szResourceType = new char[size + 1];
		if (!szResourceType)
		{
			delete [] szResourceName;
			return -1;
		}

		// Copy the resource name and type
		strcpy(szResourceName, p+1);
		strncpy(szResourceType, filename, (int)(p-filename));
		szResourceType[(int)(p-filename)] = 0;

		// Load the resource
		HMODULE hModule = GetModuleHandle(NULL);
		HRSRC hResource = FindResourceA(hModule, szResourceName, szResourceType);
		if (hResource)
		{
			HGLOBAL hGlobal = LoadResource(hModule, hResource);
			if (hGlobal)
			{
				HRESEX * phHandle = new HRESEX;
				if (phHandle)
				{
					phHandle->address = LockResource(hGlobal);
					phHandle->ptr = 0L;
					phHandle->size = (size_t)(SizeofResource(hModule, hResource));

					nRet = PtrToInt(phHandle);
				}
			}
		}

		// Free allocated memory
		delete [] szResourceName;
		delete [] szResourceType;

		return nRet;
	}

	///////////////////////////////////////////////////////////////////////////////////////////////////////////
	// Copies a part of the memory from the resource to the buffer.
	//
	static int ReadEx(int fp, void * buffer, unsigned int count)
	{
		HRESEX * phHandle = (HRESEX *)(IntToPtr(fp));

		// Reached the end of the resource
		if (phHandle->ptr >= (long)(phHandle->size - 1))
			return 0;

		// Do not try to read past the end of the resource
		if (phHandle->size - phHandle->ptr < count)
			count = (unsigned int)(phHandle->size - phHandle->ptr);

		// Copy the memory in the buffer
		memcpy(buffer, (char *)(phHandle->address) + phHandle->ptr, count);

		// Increase the pointer
		phHandle->ptr += count;

		// Return the amount of bytes copied
		return count;
	}

	///////////////////////////////////////////////////////////////////////////////////////////////////////////
	// This class cannot write to the resource. There is no need for that.
	//
	static int WriteEx(int, const void *, unsigned int)
		{ return -1; errno = ENOSPC; }

	///////////////////////////////////////////////////////////////////////////////////////////////////////////
	// Deletes the memory we allocated for this resource.
	//
	static int CloseEx(int fp)
	{
		HRESEX * phHandle = (HRESEX *)(IntToPtr(fp));
		delete phHandle;
		return 0;
	}

	///////////////////////////////////////////////////////////////////////////////////////////////////////////
	// Changes the pointer used for walking the memory of this resource.
	//
	static long SeekEx(int fp, long offset, int origin)
	{
		HRESEX * phHandle = (HRESEX *)(IntToPtr(fp));
	
		// Calculate the new pointer
		long ptr;
		switch (origin)
		{
		case SEEK_SET:
			ptr = offset;
			break;

		case SEEK_CUR:
			ptr = phHandle->ptr + offset;
			break;

		case SEEK_END:
			ptr = (long)(phHandle->size + offset);
			break;
		}

		// Can't pass before the start of the file
		if (ptr < 0)
		{
			errno = EBADF;
			return -1;
		}

		// Store the new pointer
		phHandle->ptr = ptr;
		return ptr;
	}

	///////////////////////////////////////////////////////////////////////////////////////////////////////////
	// Declare this class as a friend so it can access the private members.
	//
	friend class CCabinetExT<T>;
};

class CResourceCabinet : public CResourceCabinetT<CResourceCabinet> { };