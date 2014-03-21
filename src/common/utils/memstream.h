#ifndef _MEMSTREAM_MEMSTREAM_H
#define _MEMSTREAM_MEMSTREAM_H

/**
 * @brief   Compatibility memstream methods for MacOS X
 * @file    memstream.h
 */

#ifdef __APPLE__

#include <stdio.h>
#include "common/utils/compiler_defs.h"

BEGIN_DECLS

FILE * open_memstream(char **cp, size_t *lenp);
FILE * fmemopen(void *buf, size_t size, const char *mode);

END_DECLS

#endif // __APPLE__

#endif // _MEMSTREAM_MEMSTREAM_H
