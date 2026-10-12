////////////////////////////////////////////////////////////////////////
// texture.h

#define TEXTURE_MAGIC		0x21727874
#define TEXTURE_MAGIC_RGB	0x22727874
#define TEXTURE_NAME		80

#pragma pack(1)

typedef struct tagTEXTURE {
	DWORD magic;
	int width;
	int height;
	char comment[TEXTURE_NAME];
	BYTE map[1];
} TEXTURE;

#pragma pack()

TEXTURE* txr_create(int width, int height, int bpp);
void txr_destroy(TEXTURE* t);
TEXTURE* txr_load(LPCSTR pszName);
TEXTURE* txr_loadfromresource(HINSTANCE hInst, UINT typeResource, UINT idResource);
BOOL txr_save(TEXTURE* t, LPCSTR pszName);
