// List.cpp

#include "StdAfx.h"

#include "Common/IntToString.h"
#include "Common/MyCom.h"
#include "Common/StdOutStream.h"
#include "Common/StringConvert.h"

#include "Windows/Error.h"
#include "Windows/FileDir.h"
#include "Windows/PropVariant.h"
#include "Windows/PropVariantConversions.h"

#include "../../Archive/IArchive.h"

#include "../Common/OpenArchive.h"
#include "../Common/PropIDUtils.h"

#include "SevenZClose.h"
#include "List.h"
#include "OpenCallback7z.h"
#include "arcitem.h"

using namespace NWindows;

struct CPropIdToName {
	PROPID PropID;
	const wchar_t *Name;
};

static const CPropIdToName kPropIdToName[] = {
	{ kpidPath, L"Path" },
	{ kpidName, L"Name" },
	{ kpidIsDir, L"Folder" },
	{ kpidSize, L"Size" },
	{ kpidPackSize, L"Packed Size" },
	{ kpidAttrib, L"Attributes" },
	{ kpidCTime, L"Created" },
	{ kpidATime, L"Accessed" },
	{ kpidMTime, L"Modified" },
	{ kpidSolid, L"Solid" },
	{ kpidCommented, L"Commented" },
	{ kpidEncrypted, L"Encrypted" },
	{ kpidSplitBefore, L"Split Before" },
	{ kpidSplitAfter, L"Split After" },
	{ kpidDictionarySize, L"Dictionary Size" },
	{ kpidCRC, L"CRC" },
	{ kpidType, L"Type" },
	{ kpidIsAnti, L"Anti" },
	{ kpidMethod, L"Method" },
	{ kpidHostOS, L"Host OS" },
	{ kpidFileSystem, L"File System" },
	{ kpidUser, L"User" },
	{ kpidGroup, L"Group" },
	{ kpidBlock, L"Block" },
	{ kpidComment, L"Comment" },
	{ kpidPosition, L"Position" },
	{ kpidPrefix, L"Prefix" },
	{ kpidNumSubDirs, L"Folders" },
	{ kpidNumSubFiles, L"Files" },
	{ kpidUnpackVer, L"Version" },
	{ kpidVolume, L"Volume" },
	{ kpidIsVolume, L"Multivolume" },
	{ kpidOffset, L"Offset" },
	{ kpidLinks, L"Links" },
	{ kpidNumBlocks, L"Blocks" },
	{ kpidNumVolumes, L"Volumes" },

	{ kpidBit64, L"64-bit" },
	{ kpidBigEndian, L"Big-endian" },
	{ kpidCpu, L"CPU" },
	{ kpidPhySize, L"Physical Size" },
	{ kpidHeadersSize, L"Headers Size" },
	{ kpidChecksum, L"Checksum" },
	{ kpidCharacts, L"Characteristics" },
	{ kpidVa, L"Virtual Address" },
	{ kpidId, L"ID" },
	{ kpidShortName, L"Short Name" },
	{ kpidCreatorApp, L"Creator Application"},
	{ kpidSectorSize, L"Sector Size" },
	{ kpidPosixAttrib, L"Mode" },
	{ kpidLink, L"Link" },
	{ kpidError, L"Error" },

	{ kpidTotalSize, L"Total Size" },
	{ kpidFreeSpace, L"Free Space" },
	{ kpidClusterSize, L"Cluster Size" },
	{ kpidVolumeName, L"Label" }
};

static const char kEmptyAttribChar = '.';

static const char *kListing = "Listing archive: ";
static const wchar_t *kFilesMessage = L"files";
static const wchar_t *kDirsMessage = L"folders";

static void GetAttribString(DWORD wa, bool isDir, char *s)
{
	s[0] = ((wa & FILE_ATTRIBUTE_DIRECTORY) != 0 || isDir) ? 'D' : kEmptyAttribChar;
	s[1] = ((wa & FILE_ATTRIBUTE_READONLY) != 0) ? 'R': kEmptyAttribChar;
	s[2] = ((wa & FILE_ATTRIBUTE_HIDDEN) != 0) ? 'H': kEmptyAttribChar;
	s[3] = ((wa & FILE_ATTRIBUTE_SYSTEM) != 0) ? 'S': kEmptyAttribChar;
	s[4] = ((wa & FILE_ATTRIBUTE_ARCHIVE) != 0) ? 'A': kEmptyAttribChar;
	s[5] = '\0';
}

enum EAdjustment {
	kLeft,
	kCenter,
	kRight
};

struct CFieldInfo {
	PROPID PropID;
	UString Name;
	EAdjustment TitleAdjustment;
	EAdjustment TextAdjustment;
	int PrefixSpacesWidth;
	int Width;
};

struct CFieldInfoInit {
	PROPID PropID;
	const wchar_t* Name;
	EAdjustment TitleAdjustment;
	EAdjustment TextAdjustment;
	int PrefixSpacesWidth;
	int Width;
};

static CFieldInfoInit kStandardFieldTable[] = {
	{ kpidMTime, L"   Date      Time", kLeft, kLeft, 0, 19 },
	{ kpidAttrib, L"Attr", kRight, kCenter, 1, 5 },
	{ kpidSize, L"Size", kRight, kRight, 1, 12 },
	{ kpidPackSize, L"Compressed", kRight, kRight, 1, 12 },
	{ kpidPath, L"Name", kLeft, kLeft, 2, 24 }
};

static void PrintSpaces(int numSpaces)
{
	for (int i = 0; i < numSpaces; i++)
		g_StdOut << ' ';
}

static void PrintString(EAdjustment adjustment, int width, const UString &textString)
{
	const int numSpaces = width - textString.Length();
	int numLeftSpaces = 0;
	switch (adjustment) {
		case kLeft:		numLeftSpaces = 0; break;
		case kCenter:	numLeftSpaces = numSpaces / 2; break;
		case kRight:	numLeftSpaces = numSpaces; break;
	}
	PrintSpaces(numLeftSpaces);
	g_StdOut << textString;
	PrintSpaces(numSpaces - numLeftSpaces);
}

class CFieldPrinter
{
	CObjectVector<CFieldInfo> _fields;
public:
	void Clear() { _fields.Clear(); }
	void Init(const CFieldInfoInit *standardFieldTable, int numItems);
	HRESULT Init(IInArchive *archive);
	void PrintTitle();
	void PrintTitleLines();
	HRESULT PrintItemInfo(const CArc &arc, UInt32 index, bool techMode);
	HRESULT PrintSummaryInfo(UInt64 numFiles, UInt64 numDirs, const UInt64 *size, const UInt64 *compressedSize);
};

void CFieldPrinter::Init(const CFieldInfoInit* standardFieldTable, int numItems)
{
	Clear();
	for (int i = 0; i < numItems; i++) {
		CFieldInfo fi;
		const CFieldInfoInit& fii = standardFieldTable[i];
		fi.PropID				= fii.PropID;
		fi.Name					= fii.Name;
		fi.TitleAdjustment		= fii.TitleAdjustment;
		fi.TextAdjustment		= fii.TextAdjustment;
		fi.PrefixSpacesWidth	= fii.PrefixSpacesWidth;
		fi.Width				= fii.Width;
		_fields.Add(fi);
	}
}

static UString GetPropName(PROPID propID, BSTR name)
{
	for (int i = 0; i < ARRAYSIZE(kPropIdToName); i++) {
		const CPropIdToName &propIdToName = kPropIdToName[i];
		if (propIdToName.PropID == propID)
			return propIdToName.Name;
	}
	if (name)
		return name;
	wchar_t s[16];
	ConvertUInt32ToString(propID, s);
	return s;
}

HRESULT CFieldPrinter::Init(IInArchive *archive)
{
	Clear();
	UInt32 numProps;
	RINOK(archive->GetNumberOfProperties(&numProps));
	for (UInt32 i = 0; i < numProps; i++) {
		CMyComBSTR name;
		PROPID propID;
		VARTYPE vt;
		RINOK(archive->GetPropertyInfo(i, &name, &propID, &vt));
		CFieldInfo fi;
		fi.PropID = propID;
		fi.Name = GetPropName(propID, name);
		_fields.Add(fi);
	}
	return S_OK;
}

void CFieldPrinter::PrintTitle()
{
	for (int i = 0; i < _fields.Size(); i++) {
		const CFieldInfo& fi = _fields[i];
		PrintSpaces(fi.PrefixSpacesWidth);
		PrintString(fi.TitleAdjustment,
		((fi.PropID == kpidPath) ? 0: fi.Width), fi.Name);
	}
}

void CFieldPrinter::PrintTitleLines()
{
	for (int i = 0; i < _fields.Size(); i++) {
		const CFieldInfo &fi = _fields[i];
		PrintSpaces(fi.PrefixSpacesWidth);
		for (int i = 0; i < fi.Width; i++)
			g_StdOut << '-';
	}
}

static BOOL IsFileTimeZero(CONST FILETIME* pft)
{
	return (pft->dwLowDateTime == 0) && (pft->dwHighDateTime == 0);
}

static const char *kEmptyTimeString = "                   ";
static void PrintTime(const NCOM::CPropVariant &prop)
{
	if (prop.vt != VT_FILETIME)
		throw "incorrect item";
	if (IsFileTimeZero(&prop.filetime))
		g_StdOut << kEmptyTimeString;
	else {
		FILETIME ft;
		if (!FileTimeToLocalFileTime(&prop.filetime, &ft))
			throw "FileTimeToLocalFileTime error";
		char s[32];
		if (ConvertFileTimeToString(ft, s, true, true))
			g_StdOut << s;
		else
			g_StdOut << kEmptyTimeString;
	}
}

HRESULT CFieldPrinter::PrintItemInfo(const CArc& arc, UInt32 index, bool techMode)
{
/*
	if (techMode) {
		g_StdOut << "Index = ";
		g_StdOut << (UInt64)index;
		g_StdOut << endl;
	}
*/
	for (int i = 0; i < _fields.Size(); i++) {
		const CFieldInfo& fi = _fields[i];
		if (!techMode)
			PrintSpaces(fi.PrefixSpacesWidth);

		NCOM::CPropVariant prop;
		if (fi.PropID == kpidPath) {
			UString s;
			RINOK(arc.GetItemPath(index, s));
			prop = s;
		}
		else {
			RINOK(arc.Archive->GetProperty(index, fi.PropID, &prop));
		}
		if (techMode) {
			g_StdOut << fi.Name << " = ";
		}
		int width = (fi.PropID == kpidPath) ? 0: fi.Width;
		if (fi.PropID == kpidAttrib && (prop.vt == VT_EMPTY || prop.vt == VT_UI4)) {
			UInt32 attrib = (prop.vt == VT_EMPTY) ? 0 : prop.ulVal;
			bool isFolder;
			RINOK(IsArchiveItemFolder(arc.Archive, index, isFolder));
			char s[8];
			GetAttribString(attrib, isFolder, s);
			g_StdOut << s;
		}
		else if (prop.vt == VT_EMPTY) {
			if (!techMode)
				PrintSpaces(width);
		}
		else if (fi.PropID == kpidMTime) {
			PrintTime(prop);
		}
		else if (prop.vt == VT_BSTR) {
			if (techMode)
				g_StdOut << prop.bstrVal;
			else
				PrintString(fi.TextAdjustment, width, prop.bstrVal);
		}
		else {
			UString s = ConvertPropertyToString(prop, fi.PropID);
			s.Replace(wchar_t(0xA), L' ');
			s.Replace(wchar_t(0xD), L' ');

			if (techMode)
				g_StdOut << s;
			else
				PrintString(fi.TextAdjustment, width, s);
		}
		if (techMode)
			g_StdOut << endl;
	}
	return S_OK;
}

static void PrintNumberString(EAdjustment adjustment, int width, const UInt64 *value)
{
	wchar_t textString[32] = { 0 };
	if (value != NULL)
		ConvertUInt64ToString(*value, textString);
	PrintString(adjustment, width, textString);
}

HRESULT CFieldPrinter::PrintSummaryInfo(UInt64 numFiles, UInt64 numDirs, const UInt64 *size, const UInt64 *compressedSize)
{
	for (int i = 0; i < _fields.Size(); i++) {
		const CFieldInfo &fi = _fields[i];
		PrintSpaces(fi.PrefixSpacesWidth);
		NCOM::CPropVariant prop;
		if (fi.PropID == kpidSize)
			PrintNumberString(fi.TextAdjustment, fi.Width, size);
		else if (fi.PropID == kpidPackSize)
			PrintNumberString(fi.TextAdjustment, fi.Width, compressedSize);
		else if (fi.PropID == kpidPath) {
			wchar_t textString[32];
			ConvertUInt64ToString(numFiles, textString);
			UString temp = textString;
			temp += L" ";
			temp += kFilesMessage;
			temp += L", ";
			ConvertUInt64ToString(numDirs, textString);
			temp += textString;
			temp += L" ";
			temp += kDirsMessage;
			PrintString(fi.TextAdjustment, 0, temp);
		}
		else
			PrintString(fi.TextAdjustment, fi.Width, L"");
	}
	return S_OK;
}

bool GetUInt64Value(IInArchive* archive, UInt32 index, PROPID propID, UInt64& value)
{
	NCOM::CPropVariant prop;
	if (archive->GetProperty(index, propID, &prop) != S_OK)
		throw "GetPropertyValue error";
	if (prop.vt == VT_EMPTY)
		return false;
	value = ConvertPropVariantToUInt64(prop);
	return true;
}

bool GetProperty(IInArchive* archive, UInt32 index, PROPID propID, PROPVARIANT* value)
{
	NCOM::CPropVariant prop;
	if (archive->GetProperty(index, propID, &prop) != S_OK)
		throw "GetPropertyValue error";
	if (prop.vt == VT_EMPTY)
		return false;
	prop.Detach(value);
	return true;
}

static void PrintPropPair(const wchar_t *name, const wchar_t *value)
{
	g_StdOut << name << " = " << value << endl;
}

HRESULT ListArchives(
	CCodecs* codecs,
	const CIntVector& formatIndices,
	bool stdInMode,
	UStringVector &arcPaths,
	UStringVector &arcPathsFull,
	const NWildcard::CCensorNode& wildcardCensor,
	bool enableHeaders,
	bool techMode,
	bool& passwordEnabled,
	UString& password,
	UInt64& numErrors)
{
	numErrors = 0;
	CFieldPrinter fp;
	if (!techMode)
		fp.Init(kStandardFieldTable, ARRAYSIZE(kStandardFieldTable));

	UInt64 numFiles2 = 0, numDirs2 = 0, totalPackSize2 = 0, totalUnPackSize2 = 0;
	UInt64 *totalPackSizePointer2 = 0, *totalUnPackSizePointer2 = 0;
	int numArcs = /* stdInMode ? 1 : */ arcPaths.Size();
	for (int i = 0; i < numArcs; i++) {
		const UString &archiveName = arcPaths[i];
		UInt64 arcPackSize = 0;
		if (!stdInMode) {
			NFile::NFind::CFileInfoW fi;
			if (!fi.Find(archiveName) || fi.IsDir()) {
				g_StdOut << endl << "Error: " << archiveName << " is not file" << endl;
				numErrors++;
				continue;
			}
			arcPackSize = fi.Size;
		}

		CArchiveLink archiveLink;
		COpenCallback7z openCallback;
		openCallback.OutStream = &g_StdOut;
		openCallback.PasswordIsDefined = passwordEnabled;
		openCallback.Password = password;
		HRESULT result = archiveLink.Open2(codecs, formatIndices, stdInMode, NULL, archiveName, &openCallback);
		if (result != S_OK) {
			if (result == E_ABORT)
				return result;
			g_StdOut << endl << "Error: " << archiveName << ": ";
			if (result == S_FALSE) {
				if (openCallback.Open_WasPasswordAsked())
					g_StdOut << "Can not open encrypted archive. Wrong password?";
				else
					g_StdOut << "Can not open file as archive";
			}
			else if (result == E_OUTOFMEMORY)
				g_StdOut << "Can't allocate required memory";
			else
				g_StdOut << NError::MyFormatMessage(result);
			g_StdOut << endl;
			numErrors++;
			continue;
		}

		if (!stdInMode)
			for (int v = 0; v < archiveLink.VolumePaths.Size(); v++) {
				int index = arcPathsFull.FindInSorted(archiveLink.VolumePaths[v]);
				if (index >= 0 && index > i) {
					arcPaths.Delete(index);
					arcPathsFull.Delete(index);
					numArcs = arcPaths.Size();
				}
			}

		if (enableHeaders) {
			g_StdOut << endl << kListing << archiveName << endl << endl;

			for (int i = 0; i < archiveLink.Arcs.Size(); i++) {
				const CArc &arc = archiveLink.Arcs[i];

				g_StdOut << "--\n";
				PrintPropPair(L"Path", arc.Path);
				PrintPropPair(L"Type", codecs->Formats[arc.FormatIndex].Name);
				if (!arc.ErrorMessage.IsEmpty())
					PrintPropPair(L"Error", arc.ErrorMessage);
				UInt32 numProps;
				IInArchive *archive = arc.Archive;
				if (archive->GetNumberOfArchiveProperties(&numProps) == S_OK) {
					for (UInt32 j = 0; j < numProps; j++) {
						CMyComBSTR name;
						PROPID propID;
						VARTYPE vt;
						RINOK(archive->GetArchivePropertyInfo(j, &name, &propID, &vt));
						NCOM::CPropVariant prop;
						RINOK(archive->GetArchiveProperty(propID, &prop));
						UString s = ConvertPropertyToString(prop, propID);
						if (!s.IsEmpty())
							PrintPropPair(GetPropName(propID, name), s);
					}
				}
				if (i != archiveLink.Arcs.Size() - 1) {
					UInt32 numProps;
					g_StdOut << "----\n";
					if (archive->GetNumberOfProperties(&numProps) == S_OK) {
						UInt32 mainIndex = archiveLink.Arcs[i + 1].SubfileIndex;
						for (UInt32 j = 0; j < numProps; j++) {
							CMyComBSTR name;
							PROPID propID;
							VARTYPE vt;
							RINOK(archive->GetPropertyInfo(j, &name, &propID, &vt));
							NCOM::CPropVariant prop;
							RINOK(archive->GetProperty(mainIndex, propID, &prop));
							UString s = ConvertPropertyToString(prop, propID);
							if (!s.IsEmpty())
								PrintPropPair(GetPropName(propID, name), s);
						}
					}
				}
			}
			g_StdOut << endl;
			if (techMode)
				g_StdOut << "----------\n";
		}

		if (enableHeaders && !techMode) {
			fp.PrintTitle();
			g_StdOut << endl;
			fp.PrintTitleLines();
			g_StdOut << endl;
		}

		const CArc &arc = archiveLink.Arcs.Back();
		IInArchive *archive = arc.Archive;
		if (techMode) {
			RINOK(fp.Init(archive));
		}
		UInt64 numFiles = 0, numDirs = 0, totalPackSize = 0, totalUnPackSize = 0;
		UInt64 *totalPackSizePointer = 0, *totalUnPackSizePointer = 0;
		UInt32 numItems;
		RINOK(archive->GetNumberOfItems(&numItems));
		for (UInt32 i = 0; i < numItems; i++) {
			if (N7zClose::TestBreakSignal())
				return E_ABORT;

			UString filePath;
			HRESULT res = arc.GetItemPath(i, filePath);
			if (stdInMode && res == E_INVALIDARG)
				break;
			RINOK(res);

			bool isFolder;
			RINOK(IsArchiveItemFolder(archive, i, isFolder));
			if (!wildcardCensor.CheckPath(filePath, !isFolder))
				continue;

			fp.PrintItemInfo(arc, i, techMode);

			UInt64 packSize, unpackSize;
			if (!GetUInt64Value(archive, i, kpidSize, unpackSize))
				unpackSize = 0;
			else
				totalUnPackSizePointer = &totalUnPackSize;
			if (!GetUInt64Value(archive, i, kpidPackSize, packSize))
				packSize = 0;
			else
				totalPackSizePointer = &totalPackSize;

			g_StdOut << endl;

			if (isFolder)
				numDirs++;
			else
				numFiles++;
			totalPackSize += packSize;
			totalUnPackSize += unpackSize;
		}

		if (!stdInMode && totalPackSizePointer == 0) {
			if (archiveLink.VolumePaths.Size() != 0)
				arcPackSize += archiveLink.VolumesSize;
			totalPackSize = (numFiles == 0) ? 0 : arcPackSize;
			totalPackSizePointer = &totalPackSize;
		}
		if (totalUnPackSizePointer == 0 && numFiles == 0) {
			totalUnPackSize = 0;
			totalUnPackSizePointer = &totalUnPackSize;
		}
		if (enableHeaders && !techMode) {
			fp.PrintTitleLines();
			g_StdOut << endl;
			fp.PrintSummaryInfo(numFiles, numDirs, totalUnPackSizePointer, totalPackSizePointer);
			g_StdOut << endl;
		}
		if (totalPackSizePointer != 0) {
			totalPackSizePointer2 = &totalPackSize2;
			totalPackSize2 += totalPackSize;
		}
		if (totalUnPackSizePointer != 0) {
			totalUnPackSizePointer2 = &totalUnPackSize2;
			totalUnPackSize2 += totalUnPackSize;
		}
		numFiles2 += numFiles;
		numDirs2 += numDirs;
	}
	if (enableHeaders && !techMode && numArcs > 1) {
		g_StdOut << endl;
		fp.PrintTitleLines();
		g_StdOut << endl;
		fp.PrintSummaryInfo(numFiles2, numDirs2, totalUnPackSizePointer2, totalPackSizePointer2);
		g_StdOut << endl;
		g_StdOut << "Archives: " << numArcs << endl;
	}
	return S_OK;
}

//////////////////////////////////////////////////////////////

HRESULT xxListArchives(
	CCodecs* codecs,
	const CIntVector& formatIndices,
	UStringVector &arcPaths,
	UStringVector &arcPathsFull,
	const NWildcard::CCensorNode& wildcardCensor,
	bool& passwordEnabled,
	UString& password,
	UInt64& numErrors)
{
	numErrors = 0;
	CFieldPrinter fp;

	int numArcs = arcPaths.Size();
	for (int i = 0; i < numArcs; i++) {
		const UString &archiveName = arcPaths[i];
		UInt64 arcPackSize = 0;
		{
			NFile::NFind::CFileInfoW fi;
			if (!fi.Find(archiveName) || fi.IsDir()) {
				g_StdOut << endl << "Error: " << archiveName << " is not file" << endl;
				numErrors++;
				continue;
			}
			arcPackSize = fi.Size;
		}

		CArchiveLink archiveLink;
		COpenCallback7z openCallback;
		openCallback.OutStream = &g_StdOut;
		openCallback.PasswordIsDefined = passwordEnabled;
		openCallback.Password = password;
		HRESULT result = archiveLink.Open2(codecs, formatIndices, false, NULL, archiveName, &openCallback);
		if (result != S_OK) {
			if (result == E_ABORT)
				return result;
			g_StdOut << endl << "Error: " << archiveName << ": ";
			if (result == S_FALSE) {
				if (openCallback.Open_WasPasswordAsked())
					g_StdOut << "Can not open encrypted archive. Wrong password?";
				else
					g_StdOut << "Can not open file as archive";
			}
			else if (result == E_OUTOFMEMORY)
				g_StdOut << "Can't allocate required memory";
			else
				g_StdOut << NError::MyFormatMessage(result);
			g_StdOut << endl;
			numErrors++;
			continue;
		}

		for (int v = 0; v < archiveLink.VolumePaths.Size(); v++) {
			int index = arcPathsFull.FindInSorted(archiveLink.VolumePaths[v]);
			if (index >= 0 && index > i) {
				arcPaths.Delete(index);
				arcPathsFull.Delete(index);
				numArcs = arcPaths.Size();
			}
		}

		const CArc &arc = archiveLink.Arcs.Back();
		IInArchive *archive = arc.Archive;
		RINOK(fp.Init(archive));

		UInt64 numFiles = 0, numDirs = 0;
		UInt32 numItems;
		RINOK(archive->GetNumberOfItems(&numItems));
		for (UInt32 i = 0; i < numItems; i++) {
			if (N7zClose::TestBreakSignal())
				return E_ABORT;

			UString filePath;
			HRESULT res = arc.GetItemPath(i, filePath);
			RINOK(res);

			bool isFolder;
			RINOK(IsArchiveItemFolder(archive, i, isFolder));
			if (!wildcardCensor.CheckPath(filePath, !isFolder))
				continue;

			NCOM::CPropVariant prop;
			CArcItem ai;
			// 名前
			if (GetProperty(archive, i, kpidPath, &prop))
				ai.m_name = ConvertPropVariantToString(prop);
			else
				ai.m_name = L"";
			// サイズ
			if (!GetUInt64Value(archive, i, kpidSize, ai.m_size))
				ai.m_size = 0;
			// 圧縮サイズ
			if (!GetUInt64Value(archive, i, kpidPackSize, ai.m_packSize))
				ai.m_packSize = 0;
			// 日時
			GetProperty(archive, i, kpidMTime, &prop);
			ai.m_ft = prop.filetime;
			// method
			if (GetProperty(archive, i, kpidMethod, &prop))
				ai.m_method = ConvertPropVariantToString(prop);
			else
				ai.m_method = L"";
			// 暗号化
			if (GetProperty(archive, i, kpidEncrypted, &prop))
				ai.m_encrypted = VARIANT_BOOLToBool(prop.boolVal);
			else
				ai.m_encrypted = FALSE;

			if (isFolder)
				numDirs++;
			else
				numFiles++;
		}
	}
	return S_OK;
}
