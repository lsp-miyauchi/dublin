#include "stdafx.h"
#include "twain.h"
#include "tw_func.h"

/* The DLL to be loaded for TWAIN support */
#define TWAIN_DLL_NAME "TWAIN_32.DLL"

/*
 * Twain error code to string mappings
 */
static int twainErrorCount = 0;
static char *twainErrors[] = {
	"No error",
	"Failure due to unknown causes",
	"Not enough memory to perform operation",
	"No Data Source",
	"DS is connected to max possible apps",
	"DS or DSM reported error, application shouldn't",
	"Unknown capability",
	"Unrecognized MSG DG DAT combination",
	"Data parameter out of range",
	"DG DAT MSG out of expected sequence",
	"Unknown destination App/Src in DSM_Entry",
	"Capability not supported by source",
	"Operation not supported by capability",
	"Capability has dependency on other capability",
	"File System operation is denied (file is protected)",
	"Operation failed because file already exists.",
	"File not found",
	"Operation failed because directory is not empty",
	"The feeder is jammed",
	"The feeder detected multiple pages",
	"Error writing the file (disk full?)",
	"The device went offline prior to or during this operation",
	NULL
};

//////////////////////////////////////////////////////////////////////
// twainError
// Return the TWAIN error message associated with the specified error code.

char* twainError(int errorCode)
{
	/* Check whether we've counted */
	if (twainErrorCount == 0)
		while (twainErrors[twainErrorCount++])
			;

	/* Check out of bounds */
	if (errorCode >= twainErrorCount)
		return "Unknown TWAIN Error Code";
	else
		return twainErrors[errorCode];
}

// #define _DEBUGINFO_TWAIN

void LogMessage(int type, char *format, ...)
{
	char* pOutput = new char[1024];
	va_list argList;
	va_start(argList, format);
	int len = wvsprintf(pOutput, format, argList);
	va_end(argList);
#ifdef _DEBUGINFO_TWAIN
	dbg.puts(pOutput);
#else
	if (type == TWAIN_ERROR) {
		int len = lstrlen(pOutput);
		if (pOutput[len - 1] == '\n') pOutput[len - 1] = 0;
		MessageBox(GetFocus(), pOutput, _T("TWAIN"), MB_OK | MB_ICONSTOP);
	}
#endif


	delete [] pOutput;
}

//////////////////////////////////////////////////////////////////////
// FloatToFix32
// Convert a floating point value into a FIX32.

TW_FIX32 FloatToFIX32(float floater)
{
	TW_FIX32 Fix32_value;
	TW_INT32 value = (TW_INT32) (floater * 65536.0 + 0.5);
	Fix32_value.Whole = (short)(value >> 16);
	Fix32_value.Frac = (short)(value & 0x0000ffffL);
	return (Fix32_value);
}

//////////////////////////////////////////////////////////////////////
// Fix32ToFloat
// Convert a FIX32 value into a floating point value.

float FIX32ToFloat(TW_FIX32 fix32)
{
	float floater;
	floater = (float)fix32.Whole + (float)fix32.Frac / 65536.0F;
	return floater;
}

//////////////////////////////////////////////////////////////////////
//

CTwainSession::CTwainSession()
{
	dsmproc = NULL;
	hwnd = NULL;
	hdll = NULL;
	code = 0;
	memset(&idapp, 0, sizeof(idapp));
	memset(&idds, 0, sizeof(idds));
	userdata = NULL;
	twainState = 0;

	hdll = LoadLibrary(TWAIN_DLL_NAME);
	if (hdll)
		(FARPROC&)dsmproc = GetProcAddress(hdll, "DSM_Entry");
}

CTwainSession::~CTwainSession()
{
	// Explicitly free the SM library
	if (hdll)
		FreeLibrary(hdll);
}

//////////////////////////////////////////////////////////////////////
// callDSM
// Call the specified function on the data source manager.

TW_UINT16 CTwainSession::call(pTW_IDENTITY pDest, TW_UINT32 DG, TW_UINT16 DAT, TW_UINT16 MSG, TW_MEMREF pData)
{
	return dsmproc(&idapp, pDest, DG, DAT, MSG, pData);
}

//////////////////////////////////////////////////////////////////////
// currentTwainError
// Return the current TWAIN error message.

char* CTwainSession::GetLastTwainError()
{
	// Get the current status code from the DSM
	TW_STATUS stat;
	code = call(&idds, DG_CONTROL, DAT_STATUS, MSG_GET, (TW_MEMREF)&stat);

	// Return the mapped error code
	return twainError(stat.ConditionCode);
}

//////////////////////////////////////////////////////////////////////
// getImage
// This is a "high-level" function that can be called in order
// to take all of the steps necessary to kick off an image-transfer
// from a user-specified TWAIN datasource.  The data will be passed
// back to the callback function specified in the session structure.

BOOL CTwainSession::getImage()
{
	// Do some sanity checking first and bail  if necessary.
	if (!IsTwainAvailable())
	{
		LogMessage(TWAIN_ERROR, "TWAIN is not available for image capture\n");
		return FALSE;
	}

	// One step at a time
	if (!dsmOpen())
	{
		LogMessage(TWAIN_ERROR, "Unable to open data source manager\n");
		return FALSE;
	}

	if (!srcSelect())
	{
		LogMessage(TWAIN_ERROR, "Data source not selected\n");
		return FALSE;
	}

	if (!srcOpen())
	{
		LogMessage(TWAIN_ERROR, "Unable to open datasource\n");
		return FALSE;
	}

	RequestImageAcquire(TRUE);
	return TRUE;
}

//////////////////////////////////////////////////////////////////////
// openDSM
// Open the data source manager

BOOL CTwainSession::dsmOpen()
{
	// Make sure that we aren't already open
	if (dsmIsOpen())
		return TRUE;

	// Open the data source manager
	code = call(NULL, DG_CONTROL, DAT_PARENT, MSG_OPENDSM, (TW_MEMREF)&hwnd);

	// Check the return code
	switch (code)
	{
		case TWRC_SUCCESS:
			twainState = 3;			// We are now at state 3
			return TRUE;

		case TWRC_FAILURE:
		default:
			LogMessage(TWAIN_ERROR, "OpenDSM failure\n");
			break;
	}
	return FALSE;
}

//////////////////////////////////////////////////////////////////////
// closeDSM
// Close the data source manager

BOOL CTwainSession::dsmClose()
{
	if (dsmIsClosed())
	{
		LogMessage(TWAIN_ERROR, "closeDSM: Source Manager not open\n");
		return FALSE;
	}

	if (srcIsOpen())
	{
		LogMessage(TWAIN_ERROR, "closeDSM: Can't close source manager with open source\n");
		return FALSE;
	}

	code = call(NULL, DG_CONTROL, DAT_PARENT, MSG_CLOSEDSM, &hwnd);

	if (code != TWRC_SUCCESS)
		LogMessage(TWAIN_ERROR, "CloseDSM failure -- %s\n", GetLastTwainError());
	else
		twainState = 2;			// We are now in state 2

	// Let the caller know what happened
	return (code==TWRC_SUCCESS);
}

//////////////////////////////////////////////////////////////////////
// selectDS
// Select a datasource using the TWAIN user interface.

BOOL CTwainSession::srcSelect()
{
	// The datasource manager must be open
	if (dsmIsClosed())
	{
		LogMessage(TWAIN_ERROR, "Can't select data source with closed source manager\n");
		return FALSE;
	}

	// Ask TWAIN to present the source select dialog
	code = call(NULL, DG_CONTROL, DAT_IDENTITY, MSG_USERSELECT, (TW_MEMREF)&idds);

	// Check the return to determine what the user decided to do.
	switch (code)
	{
		case TWRC_SUCCESS:
			LogMessage(TWAIN_INFO, "Data source %s selected\n", idds.ProductName);
			return TRUE;

		case TWRC_CANCEL:
			LogMessage(TWAIN_INFO, "User cancelled TWAIN source selection\n");
			break;

		case TWRC_FAILURE:
		default:
			LogMessage(TWAIN_ERROR, "Error \"%s\" during TWAIN source selection\n", GetLastTwainError());
			break;
	}
	return FALSE;
}

//////////////////////////////////////////////////////////////////////
// selectDefaultDS
// Select the default datasource.

BOOL CTwainSession::srcSelectDefault()
{
	// The datasource manager must be open
	if (dsmIsClosed())
	{
		LogMessage(TWAIN_ERROR, "Can't select data source with closed source manager\n");
		return FALSE;
	}

	// Ask TWAIN to present the source select dialog
	code = call(NULL, DG_CONTROL, DAT_IDENTITY, MSG_GETDEFAULT, (TW_MEMREF)&idds);

	// Check the return code
	return (code == TWRC_SUCCESS);
}

//////////////////////////////////////////////////////////////////////
// openDS
// Open a data source using the TWAIN user interface.

BOOL CTwainSession::srcOpen()
{
	// The datasource manager must be open
	if (dsmIsClosed())
	{
		LogMessage(TWAIN_ERROR, "openDS: Cannot open data source... manager closed\n");
		return FALSE;
	}

	// Is the data source already open?
	if (srcIsOpen())
		return TRUE;

	// Open the TWAIN datasource
	code = call(NULL, DG_CONTROL, DAT_IDENTITY, MSG_OPENDS, (TW_MEMREF)&idds);

	// Check the return to determine what the user decided to do.
	switch (code)
	{
		case TWRC_SUCCESS:
			twainState = 4;			// We are now in TWAIN state 4
			LogMessage(TWAIN_INFO, "Data source %s opened\n", idds.ProductName);
			LogMessage(TWAIN_INFO, "\tVersion.MajorNum = %d\n", idds.Version.MajorNum);
			LogMessage(TWAIN_INFO, "\tVersion.MinorNum = %d\n", idds.Version.MinorNum);
			LogMessage(TWAIN_INFO, "\tVersion.Info = %s\n", idds.Version.Info);
			LogMessage(TWAIN_INFO, "\tProtocolMajor = %d\n", idds.ProtocolMajor);
			LogMessage(TWAIN_INFO, "\tProtocolMinor = %d\n", idds.ProtocolMinor);
			LogMessage(TWAIN_INFO, "\tManufacturer = %s\n", idds.Manufacturer);
			LogMessage(TWAIN_INFO, "\tProductFamily = %s\n", idds.ProductFamily);
			return TRUE;

		default:
			LogMessage(TWAIN_ERROR, "Error \"%s\" opening data source\n", GetLastTwainError());
			break;
	}
	return FALSE;
}

//////////////////////////////////////////////////////////////////////
// disableDS
// Disable the datasource associated with twSession.

BOOL CTwainSession::srcDisable()
{
	// Verify the datasource is enabled
	if (srcIsDisabled())
	{
		LogMessage(TWAIN_ERROR, "disableDS: Data source not enabled\n");
		return TRUE;
	}

	// Set the UI information
    TW_USERINTERFACE ui;
	ui.ShowUI = TRUE;
    ui.ModalUI = TRUE;
    ui.hParent = hwnd;

	// Make the call to the source manager
	code = call(&idds, DG_CONTROL, DAT_USERINTERFACE, MSG_DISABLEDS, (TW_MEMREF)&ui);
	if (code == TWRC_SUCCESS)
	{
		twainState = 4;
		return TRUE;
	}
	LogMessage(TWAIN_ERROR, "Error during data source disable\n");
	return FALSE;
}

//////////////////////////////////////////////////////////////////////
// closeDS
// Close the datasource associated with the specified session.

BOOL CTwainSession::srcClose()
{
	// Can't close a closed data source
	if (srcIsClosed())
	{
		LogMessage(TWAIN_ERROR, "closeDS: Data source already closed\n");
		return TRUE;
	}

	// Open the TWAIN datasource
	code = call(NULL, DG_CONTROL, DAT_IDENTITY, MSG_CLOSEDS, (TW_MEMREF)&idds);

	// Check the return to determine what the user decided to do.
	switch (code)
	{
		case TWRC_SUCCESS:
			twainState = 3;			// We are now in TWAIN state 3
			LogMessage(TWAIN_ERROR, "Data source %s closed\n", idds.ProductName);
			return TRUE;

		default:
			LogMessage(TWAIN_ERROR, "Error \"%s\" closing data source\n", GetLastTwainError());
			break;
	}
	return FALSE;
}

//////////////////////////////////////////////////////////////////////
// setBufferedXfer

BOOL CTwainSession::setBufferedXfer()
{
	TW_CAPABILITY bufXfer;
	pTW_ONEVALUE pvalOneValue;

	// Make sure the data source is open first
	if (srcIsClosed())
		return FALSE;

	// Create the capability information
	bufXfer.Cap = ICAP_XFERMECH;
	bufXfer.ConType = TWON_ONEVALUE;
	bufXfer.hContainer = GlobalAlloc(GHND, sizeof(TW_ONEVALUE));
	pvalOneValue = (pTW_ONEVALUE) GlobalLock(bufXfer.hContainer);
	pvalOneValue->ItemType = TWTY_UINT16;
	pvalOneValue->Item = TWSX_MEMORY;
	GlobalUnlock(bufXfer.hContainer);

	// Make the call to the source manager
	code = call(&idds, DG_CONTROL, DAT_CAPABILITY, MSG_SET, (TW_MEMREF)&bufXfer);

	// Free the container
	GlobalFree(bufXfer.hContainer);

	// Let the caller know what happened
	return (code==TWRC_SUCCESS);
}

//////////////////////////////////////////////////////////////////////
// requestImageAcquire
// Request that the acquire user interface be displayed.
// This may or may not cause an image to actually be transferred.

BOOL CTwainSession::RequestImageAcquire(BOOL showUI)
{
	// Make sure in the correct state
	if (srcIsClosed())
	{
		LogMessage(TWAIN_ERROR, "Can't acquire image with closed datasource\n");
		return FALSE;
	}

	// Set the transfer mode
	if (setBufferedXfer())
	{
		// Set the UI information
		TW_USERINTERFACE ui;
		ui.ShowUI = TRUE;
		ui.ModalUI = TRUE;
		ui.hParent = hwnd;

		// Make the call to the source manager
		code = call(&idds, DG_CONTROL, DAT_USERINTERFACE, MSG_ENABLEDS, (TW_MEMREF)&ui);

		if (code == TWRC_SUCCESS)
		{
			twainState = 5;		// We are now at a new twain state
			return TRUE;
		}
		LogMessage(TWAIN_ERROR, "Error during data source enable\n");
	}
	else
		LogMessage(TWAIN_ERROR, "Unable to set buffered transfer mode: %s\n", GetLastTwainError());
	return FALSE;
}

//////////////////////////////////////////////////////////////////////
// beginImageTransfer
// Begin an image transfer.

BOOL CTwainSession::BeginImageTransfer(pTW_IMAGEINFO info)
{
	// Clear our structures
	memset(info, 0, sizeof(TW_IMAGEINFO));

	// Query the image information
	code = call(&idds, DG_IMAGE, DAT_IMAGEINFO, MSG_GET, (TW_MEMREF)info);

	if (code != TWRC_SUCCESS)
	{
		LogMessage(TWAIN_ERROR, "Get Image Info failure - %s\n", GetLastTwainError());
		return FALSE;
	}

	// Call the begin transfer callback if registered
	if (_txfrBegin)
		if (!_txfrBegin(info, this))
			return FALSE;

	return TRUE;
}

//////////////////////////////////////////////////////////////////////
// transferImage
// The Source indicated it is ready to transfer data.
// It is waiting for the application to inquire about the image details,
// initiate the actual transfer, and, hence, transition the session from
// State 6 to 7.  Return the reason for exiting the transfer.

void CTwainSession::TransferImage(pTW_IMAGEINFO info)
{
	TW_SETUPMEMXFER setupMemXfer;
	TW_IMAGEMEMXFER imageMemXfer;

	memset(&setupMemXfer, 0, sizeof(TW_SETUPMEMXFER));
	memset(&imageMemXfer, 0, sizeof(TW_IMAGEMEMXFER));

	// Find out how the source would like to transfer...
	code = call(&idds, DG_CONTROL, DAT_SETUPMEMXFER, MSG_GET, (TW_MEMREF)&setupMemXfer);

	// Allocate the buffer for the transfer
	imageMemXfer.Memory.Flags = TWMF_APPOWNS | TWMF_POINTER;
	imageMemXfer.Memory.Length = setupMemXfer.Preferred;
	imageMemXfer.Memory.TheMem = (TW_MEMREF)malloc(setupMemXfer.Preferred);

	// Get the data
	do {
		// Setup for the memory transfer
		imageMemXfer.Compression = TWON_DONTCARE16;
		imageMemXfer.BytesPerRow = TWON_DONTCARE32;
		imageMemXfer.Columns = TWON_DONTCARE32;
		imageMemXfer.Rows = TWON_DONTCARE32;
		imageMemXfer.XOffset = TWON_DONTCARE32;
		imageMemXfer.YOffset = TWON_DONTCARE32;
		imageMemXfer.BytesWritten = TWON_DONTCARE32;

		// Get the next block of memory
		code = call(&idds, DG_IMAGE, DAT_IMAGEMEMXFER, MSG_GET, (TW_MEMREF)&imageMemXfer);

		if (code == TWRC_SUCCESS || code == TWRC_XFERDONE)
		{
			// Call the callback function
			if (!_txfrData(info, &imageMemXfer, this))
			{
				// Callback function requested to cancel
				code = TWRC_CANCEL;
				break;
			}
		}
	} while (code == TWRC_SUCCESS);

	free(imageMemXfer.Memory.TheMem);
}

//////////////////////////////////////////////////////////////////////
// endPendingTransfer
// Cancel the currently pending transfer.
// Return the count of pending transfers.

int CTwainSession::EndPendingTransfer()
{
	TW_PENDINGXFERS pendingXfers;
	code = call(&idds, DG_CONTROL, DAT_PENDINGXFERS, MSG_ENDXFER, (TW_MEMREF)&pendingXfers);
	if (!pendingXfers.Count)
		twainState = 5;

	return pendingXfers.Count;
}

//////////////////////////////////////////////////////////////////////
// cancelPendingTransfers
// Cancel all pending image transfers.

void CTwainSession::CancelPendingTransfers()
{
	TW_PENDINGXFERS pendingXfers;
	code = call(&idds, DG_CONTROL, DAT_PENDINGXFERS, MSG_RESET, (TW_MEMREF)&pendingXfers);
}

//////////////////////////////////////////////////////////////////////
// endImageTransfer
// Finish transferring an image.  Return the count of pending images.

int CTwainSession::EndImageTransfer(int *pendingCount)
{
	BOOL continueTransfers;
	int exitCode = code;

	// Have now exited the transfer for some reason... Figure out
	// why and what to do about it
	switch (code)
	{
		case TWRC_XFERDONE:
		case TWRC_CANCEL:
			LogMessage(TWAIN_INFO, "Xfer done received\n");
			*pendingCount = EndPendingTransfer();
			break;

		case TWRC_FAILURE:
			LogMessage(TWAIN_ERROR, "Failure received\n");
			*pendingCount = EndPendingTransfer();
			break;
	}

	// Call the end transfer callback
	if (_txfrEnd)
		continueTransfers = _txfrEnd(exitCode, *pendingCount, this);

	return (*pendingCount && continueTransfers);
}

//////////////////////////////////////////////////////////////////////
// transferImages
// Transfer all of the images that are available from the datasource.

void CTwainSession::TransferImages()
{
	TW_IMAGEINFO info;
	int pendingCount;

	// Check the image transfer callback function
	// before even attempting to do the transfer
	if (!_txfrData)
	{
		LogMessage(TWAIN_ERROR, "Attempting image transfer without callback function\n");
		return;
	}

	// Inform our application that we are getting ready to transfer images.
	if (_txfrPre)
		_txfrPre(this);

	// Loop through the available images
	do {
		twainState = 6;		// Move to the new state

		// Begin the image transfer
		if (!BeginImageTransfer(&info))
			continue;

		// Call the image transfer function
		TransferImage(&info);
	} while (EndImageTransfer(&pendingCount));

	// Inform our application that we are done transferring images.
	if (_txfrPost)
		_txfrPost(pendingCount, this);
}

//////////////////////////////////////////////////////////////////////
// TwainProcessMessage
// Returns TRUE if the application should process message as usual.
// Returns FALSE if the application should skip processing of this message

BOOL CTwainSession::ProcessMessage(LPMSG msg)
{
	TW_EVENT   twEvent;
	code = TWRC_NOTDSEVENT;

	// Only ask Source Manager to process event if there is a Source connected.
	if (dsmIsOpen() && srcIsOpen())
	{
		// A Source provides a modeless dialog box as its user interface.
		// The following call relays Windows messages down to the Source's
		// UI that were intended for its dialog box.  It also retrieves TWAIN
		// messages sent from the Source to our Application.

		twEvent.pEvent = (TW_MEMREF)msg;
		code = call(&idds, DG_CONTROL, DAT_EVENT, MSG_PROCESSEVENT, (TW_MEMREF)&twEvent);

		// Check the return code
		if (code == TWRC_NOTDSEVENT)
			return FALSE;

		// Process the message as necessary
		switch (twEvent.TWMessage)
		{
			case MSG_XFERREADY:
				LogMessage(TWAIN_INFO, "Source says that data is ready\n");
				TransferImages();
				break;

			case MSG_CLOSEDSREQ:
				// Disable the datasource, Close the Data source
				// and close the data source manager
				LogMessage(TWAIN_INFO, "CloseDSReq\n");
				srcDisable();
				srcClose();
				dsmClose();
				break;

				// No message from the Source to the App break;
				// possible new message
			case MSG_NULL:
			default:
				break;
		}
	}

	// tell the caller what happened
	return (code == TWRC_DSEVENT);
}

/**********************************************************************
 * Session related functions
 **********************************************************************/

//////////////////////////////////////////////////////////////////////
// CreateTwainSession
// Create a new TWAIN session.

CTwainSession* CreateTwainSession()
{
	// Create the structure
	CTwainSession* session = new CTwainSession();


	// Set the structure fields
	session->hwnd = 0;
	session->code = TWRC_SUCCESS;

	session->idapp.Id = 0;
	session->idapp.Version.MajorNum = 0;
	session->idapp.Version.MinorNum = 1;
	session->idapp.Version.Language = TWLG_JAPANESE;
	session->idapp.Version.Country = TWCY_JAPAN;
	lstrcpy(session->idapp.Version.Info, "LSP TWAIN 0.1");
	session->idapp.ProtocolMajor = TWON_PROTOCOLMAJOR;
	session->idapp.ProtocolMinor = TWON_PROTOCOLMINOR;
	session->idapp.SupportedGroups = DG_IMAGE;
	lstrcpy(session->idapp.Manufacturer, "LightShip Software");
	lstrcpy(session->idapp.ProductFamily, "LSP");
	lstrcpy(session->idapp.ProductName, "Twain Loader");

	session->idds.Id = 0;
	session->idds.ProductName[0] = '\0';

	session->_txfrPre = NULL;
	session->_txfrBegin = NULL;
	session->_txfrData = NULL;
	session->_txfrEnd = NULL;
	session->_txfrPost = NULL;

	if (session->IsTwainAvailable())
		session->twainState = 2;
	else
		session->twainState = 0;

	return session;
}
