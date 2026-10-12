// List.h

#ifndef __LIST_H
#define __LIST_H

#include "Common/Wildcard.h"
#include "../Common/LoadCodecs.h"

HRESULT ListArchives(CCodecs *codecs, const CIntVector &formatIndices,
    bool stdInMode,
    UStringVector &archivePaths, UStringVector &archivePathsFull,
    const NWildcard::CCensorNode &wildcardCensor,
    bool enableHeaders, bool techMode,
    bool &passwordEnabled, UString &password,
    UInt64 &errors);

#endif

