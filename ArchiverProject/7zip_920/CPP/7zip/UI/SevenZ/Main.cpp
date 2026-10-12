// Main.cpp

#include "StdAfx.h"

#if defined( _7ZIP_LARGE_PAGES)
#include "../../../../C/Alloc.h"
#endif

#include "Common/MyInitGuid.h"

#include "Common/CommandLineParser.h"
#include "Common/IntToString.h"
#include "Common/MyException.h"
#include "Common/StdOutStream.h"
#include "Common/StringConvert.h"
#include "Common/StringToInt.h"

#include "Windows/Error.h"
#include "Windows/MemoryLock.h"

#include "../Common/ArchiveCommandLine.h"
#include "../Common/ExitCode.h"
#include "../Common/Extract.h"
#include "../Common/LoadCodecs.h"

#include "BenchCon.h"
#include "ExtractCallback7z.h"
#include "List.h"
#include "OpenCallback7z.h"
#include "UpdateCallback7z.h"

#include "../../MyVersion.h"

using namespace NWindows;
using namespace NFile;
using namespace NCommandLineParser;

HINSTANCE g_hInstance = 0;
extern CStdOutStream *g_StdStream;

static const char *kHelpString =
    "\nUsage: 7z"
    " <command> [<switches>...] <archive_name> [<file_names>...]\n"
    "       [<@listfiles...>]\n"
    "\n"
    "<Commands>\n"
    "  a: Add files to archive\n"
    "  b: Benchmark\n"
    "  d: Delete files from archive\n"
    "  e: Extract files from archive (without using directory names)\n"
    "  l: List contents of archive\n"
//    "  l[a|t][f]: List contents of archive\n"
//    "    a - with Additional fields\n"
//    "    t - with all fields\n"
//    "    f - with Full pathnames\n"
    "  t: Test integrity of archive\n"
    "  u: Update files to archive\n"
    "  x: eXtract files with full paths\n"
    "<Switches>\n"
    "  -ai[r[-|0]]{@listfile|!wildcard}: Include archives\n"
    "  -ax[r[-|0]]{@listfile|!wildcard}: eXclude archives\n"
    "  -bd: Disable percentage indicator\n"
    "  -i[r[-|0]]{@listfile|!wildcard}: Include filenames\n"
    "  -m{Parameters}: set compression Method\n"
    "  -o{Directory}: set Output directory\n"
    "  -p{Password}: set Password\n"
    "  -r[-|0]: Recurse subdirectories\n"
    "  -scs{UTF-8 | WIN | DOS}: set charset for list files\n"
    "  -sfx[{name}]: Create SFX archive\n"
    "  -si[{name}]: read data from stdin\n"
    "  -slt: show technical information for l (List) command\n"
    "  -so: write data to stdout\n"
    "  -ssc[-]: set sensitive case mode\n"
    "  -ssw: compress shared files\n"
    "  -t{Type}: Set type of archive\n"
    "  -u[-][p#][q#][r#][x#][y#][z#][!newArchiveName]: Update options\n"
    "  -v{Size}[b|k|m|g]: Create volumes\n"
    "  -w[{path}]: assign Work directory. Empty path means a temporary directory\n"
    "  -x[r[-|0]]]{@listfile|!wildcard}: eXclude filenames\n"
    "  -y: assume Yes on all queries\n";

// ---------------------------
// exception messages

static const char *kEverythingIsOk = "Everything is Ok";
static const char *kUserErrorMessage = "Incorrect command line";
static const char *kNoFormats = "7-Zip cannot find the code that works with archives.";
static const char *kUnsupportedArcTypeMessage = "Unsupported archive type";
static const wchar_t *kDefaultSfxModule = L"7zCon.sfx";

static void ShowMessageAndThrowException(CStdOutStream &s, LPCSTR message, NExitCode::EEnum code)
{
	s << message << endl;
	throw code;
}

static void PrintHelpAndExit(CStdOutStream &s)
{
	s << kHelpString;
	ShowMessageAndThrowException(s, kUserErrorMessage, NExitCode::kUserError);
}

static void ShowCopyrightAndHelp(CStdOutStream &s, bool needHelp)
{
	if (needHelp)
		s << kHelpString;
}

static void PrintString(CStdOutStream &stdStream, const AString &s, int size)
{
	int len = s.Length();
	stdStream << s;
	for (int i = len; i < size; i++)
		stdStream << ' ';
}

static void PrintString(CStdOutStream &stdStream, const UString &s, int size)
{
	int len = s.Length();
	stdStream << s;
	for (int i = len; i < size; i++)
		stdStream << ' ';
}

static inline char GetHex(Byte value)
{
	return (char)((value < 10) ? ('0' + value) : ('A' + (value - 10)));
}

int Main2()
{
#if !defined(UNDER_CE)
	SetFileApisToOEM();
#endif

	UStringVector commandStrings;
	NCommandLineParser::SplitCommandLine(GetCommandLineW(), commandStrings);

	if (commandStrings.Size() == 1) {
		ShowCopyrightAndHelp(g_StdOut, true);
		return 0;
	}
	commandStrings.Delete(0);

	CArchiveCommandLineOptions options;
	CArchiveCommandLineParser parser;
	parser.Parse1(commandStrings, options);
	if (options.HelpMode) {
		ShowCopyrightAndHelp(g_StdOut, true);
		return 0;
	}

#if defined(_7ZIP_LARGE_PAGES)
	if (options.LargePages) {
		SetLargePageSize();
		NSecurity::EnableLockMemoryPrivilege();
	}
#endif

	CStdOutStream& stdStream = g_StdOut;
	g_StdStream = &stdStream;

	if (options.EnableHeaders)
		ShowCopyrightAndHelp(stdStream, false);

	parser.Parse2(options);

	CCodecs* codecs = new CCodecs;
	CMyComPtr<ICompressCodecsInfo> compressCodecsInfo = codecs;
	HRESULT result = codecs->Load();
	if (result != S_OK)
		throw CSystemException(result);

	bool isExtractGroupCommand = options.Command.IsFromExtractGroup();

	if (codecs->Formats.Size() == 0 &&
		(isExtractGroupCommand ||
		options.Command.CommandType == NCommandType::kList ||
		options.Command.IsFromUpdateGroup()))
		throw kNoFormats;

	CIntVector formatIndices;
	if (!codecs->FindFormatForArchiveType(options.ArcType, formatIndices))
		throw kUnsupportedArcTypeMessage;

	if (options.Command.CommandType == NCommandType::kInfo) {
		stdStream << endl << "Formats:" << endl;
		int i;
		for (i = 0; i < codecs->Formats.Size(); i++) {
			const CArcInfoEx &arc = codecs->Formats[i];
			if (arc.LibIndex >= 0) {
				char s[16];
				ConvertUInt32ToString(arc.LibIndex, s);
				PrintString(stdStream, s, 2);
			}
			else
				stdStream << "  ";
			stdStream << ' ';
			stdStream << (char)(arc.UpdateEnabled ? 'C' : ' ');
			stdStream << (char)(arc.KeepName ? 'K' : ' ');
			stdStream << "  ";
			PrintString(stdStream, arc.Name, 6);
			stdStream << "  ";
			UString s;
			for (int t = 0; t < arc.Exts.Size(); t++) {
				const CArcExtInfo &ext = arc.Exts[t];
				s += ext.Ext;
				if (!ext.AddExt.IsEmpty()) {
					s += L" (";
					s += ext.AddExt;
					s += L')';
				}
				s += L' ';
			}
			PrintString(stdStream, s, 14);
			stdStream << "  ";
			const CByteBuffer &sig = arc.StartSignature;
			for (size_t j = 0; j < sig.GetCapacity(); j++) {
				Byte b = sig[j];
				if (b > 0x20 && b < 0x80) {
					stdStream << (char)b;
				}
				else {
					stdStream << GetHex((Byte)((b >> 4) & 0xF));
					stdStream << GetHex((Byte)(b & 0xF));
				}
				stdStream << ' ';
			}
			stdStream << endl;
		}
		stdStream << endl << "Codecs:" << endl;

		UInt32 numMethods;
		if (codecs->GetNumberOfMethods(&numMethods) == S_OK) {
			for (UInt32 j = 0; j < numMethods; j++) {
				int libIndex = codecs->GetCodecLibIndex(j);
				if (libIndex >= 0) {
					char s[16];
					ConvertUInt32ToString(libIndex, s);
					PrintString(stdStream, s, 2);
				}
				else
					stdStream << "  ";
				stdStream << ' ';
				stdStream << (char)(codecs->GetCodecEncoderIsAssigned(j) ? 'C' : ' ');
				UInt64 id;
				stdStream << "  ";
				HRESULT res = codecs->GetCodecId(j, id);
				if (res != S_OK)
					id = (UInt64)(Int64)-1;
				char s[32];
				ConvertUInt64ToString(id, s, 16);
				PrintString(stdStream, s, 8);
				stdStream << "  ";
				PrintString(stdStream, codecs->GetCodecName(j), 11);
				stdStream << endl;
/*
				if (res != S_OK)
					throw "incorrect Codec ID";
*/
			}
			return S_OK;
		}
	}
	else if (options.Command.CommandType == NCommandType::kBenchmark) {
		if (options.Method.CompareNoCase(L"CRC") == 0) {
			HRESULT res = CrcBenchCon((FILE *)stdStream, options.NumIterations, options.NumThreads, options.DictionarySize);
			if (res != S_OK) {
				if (res == S_FALSE) {
					stdStream << "\nCRC Error\n";
					return NExitCode::kFatalError;
				}
				throw CSystemException(res);
			}
		}
		else {
			HRESULT res;
			CObjectVector<CCodecInfoEx> externalCodecs;
			res = LoadExternalCodecs(compressCodecsInfo, externalCodecs);
			if (res != S_OK)
				throw CSystemException(res);
			res = LzmaBenchCon(
			compressCodecsInfo, &externalCodecs,
			(FILE *)stdStream, options.NumIterations, options.NumThreads, options.DictionarySize);
			if (res != S_OK) {
				if (res == S_FALSE) {
					stdStream << "\nDecoding Error\n";
					return NExitCode::kFatalError;
				}
				throw CSystemException(res);
			}
		}
	}
	else if (isExtractGroupCommand || options.Command.CommandType == NCommandType::kList) {
		if (isExtractGroupCommand) {
			CExtractCallback7z *ecs = new CExtractCallback7z;
			CMyComPtr<IFolderArchiveExtractCallback> extractCallback = ecs;

			ecs->OutStream = &stdStream;
			ecs->PasswordIsDefined = options.PasswordEnabled;
			ecs->Password = options.Password;
			ecs->Init();

			COpenCallback7z openCallback;
			openCallback.OutStream = &stdStream;
			openCallback.PasswordIsDefined = options.PasswordEnabled;
			openCallback.Password = options.Password;

			CExtractOptions eo;
			eo.StdInMode = options.StdInMode;
			eo.StdOutMode = options.StdOutMode;
			eo.PathMode = options.Command.GetPathMode();
			eo.TestMode = options.Command.IsTestMode();
			eo.OverwriteMode = options.OverwriteMode;
			eo.OutputDir = options.OutputDir;
			eo.YesToAll = options.YesToAll;
			eo.CalcCrc = options.CalcCrc;
#if !defined(_7ZIP_ST) && !defined(_SFX)
			eo.Properties = options.ExtractProperties;
#endif
			UString errorMessage;
			CDecompressStat stat;
			HRESULT result = DecompressArchives(
									codecs,
									formatIndices,
									options.ArchivePathsSorted,
									options.ArchivePathsFullSorted,
									options.WildcardCensor.Pairs.Front().Head,
									eo, &openCallback, ecs, errorMessage, stat);
			if (!errorMessage.IsEmpty()) {
				stdStream << endl << "Error: " << errorMessage;
				if (result == S_OK)
					result = E_FAIL;
			}

			stdStream << endl;
			if (ecs->NumArchives > 1)
				stdStream << "Archives: " << ecs->NumArchives << endl;
			if (ecs->NumArchiveErrors != 0 || ecs->NumFileErrors != 0) {
				if (ecs->NumArchives > 1) {
					stdStream << endl;
					if (ecs->NumArchiveErrors != 0)
						stdStream << "Archive Errors: " << ecs->NumArchiveErrors << endl;
					if (ecs->NumFileErrors != 0)
						stdStream << "Sub items Errors: " << ecs->NumFileErrors << endl;
				}
				if (result != S_OK)
					throw CSystemException(result);
				return NExitCode::kFatalError;
			}
			if (result != S_OK)
				throw CSystemException(result);
			if (stat.NumFolders != 0)
				stdStream << "Folders: " << stat.NumFolders << endl;
			if (stat.NumFiles != 1 || stat.NumFolders != 0)
				stdStream << "Files: " << stat.NumFiles << endl;
			stdStream
				<< "Size:       " << stat.UnpackSize << endl
				<< "Compressed: " << stat.PackSize << endl;
			if (options.CalcCrc) {
				char s[16];
				ConvertUInt32ToHexWithZeros(stat.CrcSum, s);
				stdStream << "CRC: " << s << endl;
			}
		}
		else {
			UInt64 numErrors = 0;
/*
			HRESULT result = ListArchives(
									codecs,
									formatIndices,
									options.StdInMode,
									options.ArchivePathsSorted,
									options.ArchivePathsFullSorted,
									options.WildcardCensor.Pairs.Front().Head,
									options.EnableHeaders,
									options.TechMode,
									options.PasswordEnabled,
									options.Password,
									numErrors);
*/
			HRESULT result = xxListArchives(
									codecs,
									formatIndices,
									options.ArchivePathsSorted,
									options.ArchivePathsFullSorted,
									options.WildcardCensor.Pairs.Front().Head,
									options.PasswordEnabled,
									options.Password,
									numErrors);
			if (numErrors > 0) {
				g_StdOut << endl << "Errors: " << numErrors;
				return NExitCode::kFatalError;
			}
			if (result != S_OK)
				throw CSystemException(result);
		}
	}
	else if (options.Command.IsFromUpdateGroup()) {
		CUpdateOptions &uo = options.UpdateOptions;
		if (uo.SfxMode && uo.SfxModule.IsEmpty())
			uo.SfxModule = kDefaultSfxModule;

		COpenCallback7z openCallback;
		openCallback.OutStream = &stdStream;

		bool passwordIsDefined = options.PasswordEnabled && !options.Password.IsEmpty();
		openCallback.PasswordIsDefined = passwordIsDefined;
		openCallback.Password = options.Password;

		CUpdateCallback7z callback;
		callback.EnablePercents = options.EnablePercents;
		callback.PasswordIsDefined = passwordIsDefined;
		callback.AskPassword = options.PasswordEnabled && options.Password.IsEmpty();
		callback.Password = options.Password;
		callback.StdOutMode = uo.StdOutMode;
		callback.Init(&stdStream);

		CUpdateErrorInfo errorInfo;

		if (!uo.Init(codecs, formatIndices, options.ArchiveName))
			throw kUnsupportedArcTypeMessage;
		HRESULT result = UpdateArchive(codecs, options.WildcardCensor, uo, errorInfo, &openCallback, &callback);

		int exitCode = NExitCode::kSuccess;
		if (callback.CantFindFiles.Size() > 0) {
			stdStream << endl;
			stdStream << "WARNINGS for files:" << endl << endl;
			int numErrors = callback.CantFindFiles.Size();
			for (int i = 0; i < numErrors; i++) {
				stdStream << callback.CantFindFiles[i] << " : ";
				stdStream << NError::MyFormatMessageW(callback.CantFindCodes[i]) << endl;
			}
			stdStream << "----------------" << endl;
			stdStream << "WARNING: Cannot find " << numErrors << " file";
			if (numErrors > 1)
				stdStream << "s";
			stdStream << endl;
			exitCode = NExitCode::kWarning;
		}

		if (result != S_OK) {
			UString message;
			if (!errorInfo.Message.IsEmpty()) {
				message += errorInfo.Message;
				message += L"\n";
			}
			if (!errorInfo.FileName.IsEmpty()) {
				message += errorInfo.FileName;
				message += L"\n";
			}
			if (!errorInfo.FileName2.IsEmpty()) {
				message += errorInfo.FileName2;
				message += L"\n";
			}
			if (errorInfo.SystemError != 0) {
				message += NError::MyFormatMessageW(errorInfo.SystemError);
				message += L"\n";
			}
			if (!message.IsEmpty())
				stdStream << L"\nError:\n" << message;
			throw CSystemException(result);
		}
		int numErrors = callback.FailedFiles.Size();
		if (numErrors == 0) {
			if (callback.CantFindFiles.Size() == 0)
				stdStream << kEverythingIsOk << endl;
		}
		else {
			stdStream << endl;
			stdStream << "WARNINGS for files:" << endl << endl;
			for (int i = 0; i < numErrors; i++) {
				stdStream << callback.FailedFiles[i] << " : ";
				stdStream << NError::MyFormatMessageW(callback.FailedCodes[i]) << endl;
			}
			stdStream << "----------------" << endl;
			stdStream << "WARNING: Cannot open " << numErrors << " file";
			if (numErrors > 1)
				stdStream << "s";
			stdStream << endl;
			exitCode = NExitCode::kWarning;
		}
		return exitCode;
	}
	else
		PrintHelpAndExit(stdStream);
	return 0;
}
