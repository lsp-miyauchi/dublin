//{{NO_DEPENDENCIES}}
//
// FUNCTION PROTOTYPE DEFINITION
//

///////////////////////////////////////////////////////////////////////
// prptotype function written by C
//

class CFileInfo;
class CFileArray;

///////////////////////////////////////////////////////////////////////
// common.cpp
//

int AppMessage(UINT nTitleID, UINT nTextID, LPCTSTR pszInsert = NULL, UINT nType = MB_STOP);
int LastError(UINT nTitleID, LPCTSTR pszTitle, UINT nType = MB_STOP);
BOOL GetFileData(LPCTSTR pszName, WIN32_FIND_DATA* pwfd);
BOOL BrowseForFolder(HWND hwnd, int nFolder, LPSTR path);
void ZapMessage(HWND hwnd);


///////////////////////////////////////////////////////////////////////
// devio.cpp
//

BOOL FormatCylinder(HANDLE hVolume, PDISK_GEOMETRY pgm, int nCylinder);
