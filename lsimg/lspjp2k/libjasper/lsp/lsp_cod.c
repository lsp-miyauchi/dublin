/******************************************************************************\
* Includes.
\******************************************************************************/

#include <assert.h>

#include "jasper/jas_types.h"
#include "jasper/jas_image.h"
#include "jasper/jas_malloc.h"

#include "lsp_cod.h"

/******************************************************************************\
* Constructors and destructors.
\******************************************************************************/

lsp_info_t *lsp_info_create()
{
	lsp_info_t *info;
	if (!(info = (lsp_info_t*)jas_malloc(sizeof(lsp_info_t)))) {
		return 0;
	}
	info->palents = 0;
	return info;
}

void lsp_info_destroy(lsp_info_t *info)
{
	if (info->palents) {
		jas_free(info->palents);
	}
	jas_free(info);
}

/******************************************************************************\
* Miscellaneous functions.
\******************************************************************************/

int lsp_isgrayscalepal(lsp_palent_t *palents, int numpalents)
{
	lsp_palent_t *palent;
	int i;

	for (i = numpalents, palent = palents; i > 0; --i, ++palent) {
		if (palent->red != palent->grn || palent->red != palent->blu) {
			return 0;
		}
	}
	return 1;
}

int lsp_numcmpts(lsp_info_t *info)
{
	int numcmpts;

	if (info->depth == 24) {
		numcmpts = 3;
	} else if (info->depth == 8) {
		numcmpts = lsp_isgrayscalepal(info->palents, info->numcolors) ?
		  1 : 3;
	} else {
		numcmpts = 0;
		abort();
	}
	return numcmpts;
}
