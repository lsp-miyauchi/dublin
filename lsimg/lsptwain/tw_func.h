/*  
 * TWAIN Plug-in
 * Copyright (C) 1999 Craig Setera
 * Craig Setera <setera@home.com>
 * 03/31/1999
 */

#ifndef _TW_FUNC_H
#define _TW_FUNC_H

/*
 * Pre-image transfer function type
 * Sent to the caller before any of the
 * images are transferred to the application.
 */
typedef void (*TW_PRE_TXFR_CB)(void*);

/*
 * Image transfer begin function type
 *
 * Sent to the caller when an image transfer
 * is about to begin.  The caller may return
 * FALSE if the transfer should not continue.
 * Otherwise, the function should return a
 * TRUE value.
 */
typedef int (*TW_TXFR_BEGIN_CB)(pTW_IMAGEINFO, void*);

/*
 * Image transfer callback function type
 *
 * Expected to return true if the image transfer
 * should continue.  False if the transfer should
 * be cancelled.
 */
typedef int (*TW_TXFR_DATA_CB)(pTW_IMAGEINFO, pTW_IMAGEMEMXFER, void*);

/*
 * Image transfer end function type
 *
 * Sent to the caller when an image transfer
 * is completed.  The caller will be handed
 * the image transfer completion state.  The
 * following values (defined in twain.h) are
 * possible:
 *
 * TWRC_XFERDONE: The transfer completed successfully
 * TWRC_CANCEL  : The transfer was completed by the user
 * TWRC_FAILURE : The transfer failed.
 */
typedef int (*TW_TXFR_END_CB)(int, int, void*);

/*
 * Post-image transfer callback
 *
 * This callback function is called after all
 * of the possible images have been transferred
 * from the datasource.
 */
typedef void (*TW_POST_TXFR_CB)(int, void*);

/*
 * The following structure defines the
 * three callback functions that are called
 * while an image is being transferred.
 * The types of these functions are defined
 * above.  Any function that is NULL will just
 * not be called.
 */
typedef struct _TXFR_CB_FUNCS {
	TW_PRE_TXFR_CB preTxfrCb;			// Pre-transfer function
	TW_TXFR_BEGIN_CB txfrBeginCb;		// Begin function
	TW_TXFR_DATA_CB txfrDataCb;			// Data transfer
	TW_TXFR_END_CB txfrEndCb;			// End function
	TW_POST_TXFR_CB postTxfrCb;			// Post-transfer function
} TXFR_CB_FUNCS, *pTXFR_CB_FUNCS;

class CTwainSession
{
public:
	CTwainSession();
	~CTwainSession();

	TW_UINT16 call(pTW_IDENTITY pDest, TW_UINT32 DG, TW_UINT16 DAT, TW_UINT16 MSG, TW_MEMREF pData);
	char* GetLastTwainError();
	BOOL IsTwainAvailable();
	BOOL getImage();


	BOOL dsmOpen();
	BOOL dsmClose();
	BOOL dsmIsOpen();
	BOOL dsmIsClosed();

	BOOL srcOpen();
	BOOL srcClose();
	BOOL srcSelect();
	BOOL srcSelectDefault();
	BOOL srcDisable();
	BOOL srcIsOpen();
	BOOL srcIsClosed();
	BOOL srcIsEnabled();
	BOOL srcIsDisabled();

	BOOL BeginImageTransfer(pTW_IMAGEINFO info);
	void TransferImage(pTW_IMAGEINFO info);
	void TransferImages();
	void CancelPendingTransfers();
	int EndImageTransfer(int *pendingCount);
	int EndPendingTransfer();

	BOOL setBufferedXfer();
	BOOL RequestImageAcquire(BOOL showUI);

	BOOL ProcessMessage(LPMSG msg);

	DSMENTRYPROC dsmproc;
	HWND hwnd;							// The window handle related to the TWAIN
	HMODULE hdll;						//
	TW_UINT16 code;						// The current TWAIN return code
	TW_IDENTITY idapp;					// The application's TWAIN identity
	TW_IDENTITY idds;					// The datasource's TWAIN identity
	void *userdata;						// user data
	int twainState;						// TWAIN current state
										// The states are:
										//  1) Pre-session: The DSM has not been loaded
										//  2) Source Manager Loaded (not opened)
										//  3) Source Manager Opened
										//  4) Source Open
										//  5) Source Enabled
										//  6) Transfer ready
										//  7) Transferring

	TW_PRE_TXFR_CB   _txfrPre;			// Pre-transfer function
	TW_TXFR_BEGIN_CB _txfrBegin;		// Begin function
	TW_TXFR_DATA_CB  _txfrData;			// Data transfer
	TW_TXFR_END_CB   _txfrEnd;			// End function
	TW_POST_TXFR_CB  _txfrPost;			// Post-transfer function
};

_inline BOOL CTwainSession::IsTwainAvailable()
	{ return (hdll && dsmproc); }
_inline BOOL CTwainSession::dsmIsOpen()
	{ return (twainState >= 3); }
_inline BOOL CTwainSession::dsmIsClosed()
	{ return (twainState < 3); }
_inline BOOL CTwainSession::srcIsOpen()
	{ return (twainState >= 4); }
_inline BOOL CTwainSession::srcIsClosed()
	{ return (twainState < 4); }
_inline BOOL CTwainSession::srcIsEnabled()
	{ return (twainState >= 5); }
_inline BOOL CTwainSession::srcIsDisabled()
	{ return (twainState < 5); }


TW_FIX32 FloatToFix32(float);
float FIX32ToFloat(TW_FIX32);

CTwainSession* CreateTwainSession();

enum {
	TWAIN_INFO = 0,
	TWAIN_ERROR,
};

void LogMessage(int, char *, ...);

#endif /* _TW_FUNC_H */
