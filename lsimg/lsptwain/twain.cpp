//////////////////////////////////////////////////////////////////////
// twain.cpp

#include "stdafx.h"
#include "twain.h"
#include "tw_func.h"

extern HINSTANCE ghInst;

typedef struct {
	mif* image;
	pTW_PALETTE8 paletteData;
	BOOL xfer_done;
} UserData, *pUserData;


#define APP_NAME "TWAIN"
#define SHOW_WINDOW 0
#define WM_TRANSFER_IMAGE (WM_USER + 100)

void TcbPre(void *);
int  TcbBegin(pTW_IMAGEINFO, void *);
int  TcbData(pTW_IMAGEINFO, pTW_IMAGEMEMXFER, void *);
int  TcbEnd(int, int, void *);
void TcbPost(int, void *);


//////////////////////////////////////////////////////////////////////
// WndProc

static LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	CTwainSession* twain;
	switch (message)
	{
		case WM_CREATE:
		{
			// Register our window handle with the TWAIN support.
			twain = (CTwainSession*)((CREATESTRUCT*)lParam)->lpCreateParams;
			twain->hwnd = hWnd;
			SetWindowLongPtr(hWnd, GWLP_USERDATA, (LONG_PTR)twain);

			PostMessage(hWnd, WM_TRANSFER_IMAGE, 0, 0);
			return 0;
		}

		case WM_TRANSFER_IMAGE:
		{
			twain = (CTwainSession*)GetWindowLongPtr(hWnd, GWLP_USERDATA);
			// Get an image
			if (!twain->getImage())
				DestroyWindow(hWnd);
			return 0;
		}

		default:
			break;
	}
	return DefWindowProc(hWnd, message, wParam, lParam);
}

//////////////////////////////////////////////////////////////////////
// InitApplication
// Initialize window data and register the window class

static BOOL InitApplication(HINSTANCE hInstance)
{
	WNDCLASS wc;
	wc.style = CS_HREDRAW | CS_VREDRAW;
	wc.lpfnWndProc = (WNDPROC) WndProc;
	wc.cbClsExtra = 0;
	wc.cbWndExtra = 0;
	wc.hInstance = hInstance;
	wc.hIcon = LoadIcon(NULL, IDI_APPLICATION);
	wc.hCursor = LoadCursor(NULL, IDC_ARROW);
	wc.hbrBackground = (HBRUSH) (COLOR_WINDOW + 1);
	wc.lpszClassName = APP_NAME;
	wc.lpszMenuName = NULL;
	return RegisterClass(&wc);
}

//////////////////////////////////////////////////////////////////////
// twainWinMain

extern "C"
HANDLE WINAPI TwainGetImage()
{
	if (!InitApplication(ghInst))
		return NULL;

	// Initialize the twain information
	CTwainSession* twain = CreateTwainSession();
	if (!twain)
		return NULL;

	if (!twain->IsTwainAvailable())
	{
		delete twain;
		return NULL;
	}

	twain->_txfrPre   =	TcbPre;
	twain->_txfrBegin = TcbBegin;
	twain->_txfrData  = TcbData;
	twain->_txfrEnd   = TcbEnd;
	twain->_txfrPost  = TcbPost;

	pUserData theData = (pUserData)malloc(sizeof(UserData));
	memset(theData, 0, sizeof(UserData));
	twain->userdata = theData;

	HWND hwnd = CreateWindow(APP_NAME, APP_NAME, WS_OVERLAPPEDWINDOW,
							CW_USEDEFAULT, 0, CW_USEDEFAULT, 0,
							NULL, NULL, ghInst, (void*)twain);
	if (!hwnd)
	{
		free(theData);
		return FALSE;
	}

	ShowWindow(hwnd, SHOW_WINDOW);
	UpdateWindow(hwnd);

	MSG msg;
	while (IsWindow(hwnd) && GetMessage(&msg, NULL, 0, 0))
	{
		if (twain->srcIsClosed() || !twain->ProcessMessage(&msg))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
	}

	delete twain;

	HANDLE hdib = (theData->xfer_done && theData->image) ? theData->image->createdib() : NULL;
	if (theData->image)
		delete theData->image;
	free(theData);
	return hdib;
}

//////////////////////////////////////////////////////////////////////
// TcbPre
// This callback function is called before any images are transferred.  Set up the one time only stuff.

void TcbPre(void *param)
{
}

//////////////////////////////////////////////////////////////////////
// TcbBegin
// The following function is called at the beginning of each image transfer.

int TcbBegin(pTW_IMAGEINFO info, void *param)
{
	CTwainSession* twain = (CTwainSession*)param;
	pUserData theData = (pUserData)twain->userdata;

	// Decide on the image type
	int i;
	RGBA* map;
	switch (info->PixelType)
	{
		case TWPT_BW:
			// Set up the image and layer types
			theData->image = new mif(info->ImageWidth, info->ImageLength, 1);
			map = theData->image->m_map;
			map->red = map->green = map->blue = 0;
			map++;
			map->red = map->green = map->blue = 255;
			break;

		case TWPT_GRAY:
			// Set up the image and layer types
			theData->image = new mif(info->ImageWidth, info->ImageLength, 8);
			map = theData->image->m_map;
			for (i = 0; i < 256; i++)
			{
				map->red = map->green = map->blue = i;
				map++;
			}
			break;

		case TWPT_RGB:
			// Set up the image and layer types
			theData->image = new mif(info->ImageWidth, info->ImageLength, 24);
			break;

		case TWPT_PALETTE:
			// Get the palette data
			theData->paletteData = (TW_PALETTE8*)malloc(sizeof(TW_PALETTE8));
			twain->code = twain->call(&twain->idds, DG_IMAGE, DAT_PALETTE8, MSG_GET, (TW_MEMREF)theData->paletteData);
			if (twain->code != TWRC_SUCCESS)
			{
				LogMessage(TWAIN_INFO, "TcbBegin: cannot get palette data\n");
				return FALSE;
			}

			switch (theData->paletteData->PaletteType)
			{
				case TWPA_RGB:
				case TWPA_GRAY:
					// Set up the image and layer types
					theData->image = new mif(info->ImageWidth, info->ImageLength, 8);
					break;

				default:
					LogMessage(TWAIN_INFO, "TcbBegin: unsupported palette type\n");
					return FALSE;
			}

			map = theData->image->m_map;
			for (i = 0; i < 256; i++)
			{
				switch (theData->paletteData->PaletteType)
				{
					case TWPA_GRAY:
						map->red =
						map->green =
						map->blue = theData->paletteData->Colors[i].Channel1;
						break;

					case TWPA_RGB:
						map->red   = theData->paletteData->Colors[i].Channel1;
						map->green = theData->paletteData->Colors[i].Channel2;
						map->blue  = theData->paletteData->Colors[i].Channel3;
						break;
				}
				map++;
			}
			break;

		default:
			// We don't know how to deal with anything other than
			// the types listed above.  Bail for any other image type.
			LogMessage(TWAIN_INFO, "TcbBegin: unsupported pixel type = %d\n", info->PixelType);
			return FALSE;
	}
	return TRUE;
}

//////////////////////////////////////////////////////////////////////
// TcbBit
// The following function is called for each memory block that is transferred from
// the data source if the image type is Black/White.
//
// Black and white data is unpacked from bit data into byte data and written into
// a gray scale GIMP image.

static BYTE bitMasks[] = { 128, 64, 32, 16, 8, 4, 2, 1 };

static int TcbBit(pTW_IMAGEINFO info, pTW_IMAGEMEMXFER memxfer, void *param)
{
	CTwainSession* twain = (CTwainSession*)param;
	pUserData theData = (pUserData)twain->userdata;

	int rows = memxfer->Rows;
	int cols = memxfer->Columns;

	// Unpack the image data from bits into bytes
	BYTE* src = (BYTE*)memxfer->Memory.TheMem;
	for (int row = 0; row < rows; row++)
	{
		RGBA* ps = theData->image->pos(memxfer->XOffset, memxfer->YOffset + row);
		for (int col = 0; col < cols; col++)
		{
			BYTE cb = src[(row * memxfer->BytesPerRow) + (col / 8)];
			cb = ((cb & bitMasks[col % 8]) != 0) ? 255 : 0;
			ps->red = ps->green = ps->blue = cb;
			ps->alpha = (cb == 255) ? 1 : 0;
		}
	}

	return TRUE;
}

//////////////////////////////////////////////////////////////////////
// Tcb1BPS (1byte per sample)
// The following function is called for each memory block that is transferred
// from the data source if the image type is Grayscale or RGB.
// This transfer mode is quicker than the modes that require translation
// from a greater number of bits per sample down to the 8 bits per sample understood by The GIMP.

static int Tcb1BPS(pTW_IMAGEINFO info, pTW_IMAGEMEMXFER memxfer, void *param)
{
	CTwainSession* twain = (CTwainSession*)param;
	pUserData theData = (pUserData)twain->userdata;

	int rows = memxfer->Rows;
	int cols = memxfer->Columns;

	BYTE* src = (BYTE*)memxfer->Memory.TheMem;
	for (int row = 0; row < rows; row++)
	{
		RGBA* ps = theData->image->pos(memxfer->XOffset, memxfer->YOffset + row);
		BYTE* src = (BYTE*)memxfer->Memory.TheMem + (row * memxfer->BytesPerRow);
		for (int col = 0; col < cols; col++)
		{
			if (info->PixelType == TWPT_GRAY)
				ps->red = ps->green = ps->blue = ps->alpha = *src++;
			else
			{
				ps->red = *src++;
				ps->green = *src++;
				ps->blue = *src++;
			}
			ps++;
		}
	}
	return TRUE;
}

//////////////////////////////////////////////////////////////////////
// Tcb2BPS (2byte per sample)
// The following function is called for each memory block that is transferred
// from the data source if the image type is Grayscale or RGB.

static int Tcb2BPS(pTW_IMAGEINFO info, pTW_IMAGEMEMXFER memxfer, void *param)
{
	CTwainSession* twain = (CTwainSession*)param;
	pUserData theData = (pUserData)twain->userdata;

	int rows = memxfer->Rows;
	int cols = memxfer->Columns;

	// Work through the rows
	for (int row = 0; row < rows; row++)
	{
		TW_UINT16* src = (TW_UINT16*)((BYTE*)memxfer->Memory.TheMem + (row * memxfer->BytesPerRow));
		RGBA* ps = theData->image->pos(memxfer->XOffset, memxfer->YOffset + row);
		for (int col = 0; col < cols; col++)
		{
			switch (info->PixelType)
			{
				case TWPT_GRAY:
					ps->red = ps->green = ps->blue = ps->alpha = (BYTE)(*src++ >> 8);
					break;

				case TWPT_RGB:
					ps->red = (BYTE)(*src++ >> 8);
					ps->green = (BYTE)(*src++ >> 8);
					ps->blue = (BYTE)(*src++ >> 8);
					break;
			}
			ps++;

		}
	}
	return TRUE;
}

//////////////////////////////////////////////////////////////////////
// TcbPaletted
// The following function is called for each memory block that is transferred
// from the data source if the image type is paletted.  This does not create
// an indexed image type in The GIMP because for some reason it does not allow
// creation of a specific palette.  This function will create an RGB or Gray
// image and use the palette to set the details of the pixels.

static int TcbPaletted(pTW_IMAGEINFO info, pTW_IMAGEMEMXFER memxfer, void *param)
{
	CTwainSession* twain = (CTwainSession*)param;
	pUserData theData = (pUserData)twain->userdata;

	int rows = memxfer->Rows;
	int cols = memxfer->Columns;

	for (int row = 0; row < rows; row++)
	{
		BYTE* src = (BYTE*)((BYTE*)memxfer->Memory.TheMem + (row * memxfer->BytesPerRow));
		RGBA* ps = theData->image->pos(memxfer->XOffset, memxfer->YOffset + row);
		for (int col = 0; col < cols; col++)
		{
			// Get the palette index
			ps->alpha = *src++;
			switch (theData->paletteData->PaletteType)
			{
				case TWPA_GRAY:
					ps->red = ps->green = ps->blue = theData->paletteData->Colors[ps->alpha].Channel1;
					break;

				case TWPA_RGB:
					ps->red   = theData->paletteData->Colors[ps->alpha].Channel1;
					ps->green = theData->paletteData->Colors[ps->alpha].Channel2;
					ps->blue  = theData->paletteData->Colors[ps->alpha].Channel3;
			}
			ps++;
		}
	}

	return TRUE;
}

//////////////////////////////////////////////////////////////////////
// TcbData
// The following function is called for each memory block that is transferred from the data source.

int TcbData(pTW_IMAGEINFO info, pTW_IMAGEMEMXFER memxfer, void *param)
{
	// Choose the appropriate transfer handler
	switch (info->PixelType)
	{
		case TWPT_PALETTE:
			return TcbPaletted(info, memxfer, param);

		case TWPT_BW:
			return TcbBit(info, memxfer, param);

		case TWPT_GRAY:
		case TWPT_RGB:
			switch (info->BitsPerPixel / info->SamplesPerPixel)
			{
				case 8:
					return Tcb1BPS(info, memxfer, param);

				case 16:
					return Tcb2BPS(info, memxfer, param);

				default:
					break;
			}
			break;

		default:
			LogMessage(TWAIN_INFO, "TcbData: unsupported pixel type = %d\n", info->PixelType);
			break;
	}
	return FALSE;
}

//////////////////////////////////////////////////////////////////////
// TcbEnd
// The following function is called at the end of the image transfer.
// The caller will be handed the image transfer completion state.
// The following values (defined in twain.h) are possible:
// TWRC_XFERDONE: The transfer completed successfully
// TWRC_CANCEL  : The transfer was completed by the user
// TWRC_FAILURE : The transfer failed.

int TcbEnd(int completionState, int pendingCount, void *param)
{
	CTwainSession* twain = (CTwainSession*)param;
	pUserData theData = (pUserData)twain->userdata;

	theData->xfer_done = (completionState == TWRC_XFERDONE);
	return theData->xfer_done;
}

//////////////////////////////////////////////////////////////////////
// TcbPost
// This callback function will be called after all possible images have been transferred.

void TcbPost(int pendingCount, void *param)
{
	CTwainSession* twain = (CTwainSession*)param;
	pUserData theData = (pUserData)twain->userdata;

	// Shut things down.
	if (pendingCount != 0)
		twain->CancelPendingTransfers();

	// This will close the datasource and datasource manager.
	// Then the message queue will be shut down and the run() procedure will
	// finally be able to finish.

	twain->srcDisable();
	twain->srcClose();
	twain->dsmClose();

	DestroyWindow(twain->hwnd);
}
