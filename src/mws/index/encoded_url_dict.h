/*

Copyright (C) 2010-2013 KWARC Group <kwarc.info>

This file is part of MathWebSearch.

MathWebSearch is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

MathWebSearch is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with MathWebSearch.  If not, see <http://www.gnu.org/licenses/>.

*/
/**
 * @brief   Encoded URL dictionary
 * @file    encoded_url_dict.h
 * @date    03 Feb 2013
 *
 * License: GPLv3
 */

#ifndef __MWS_INDEX_ENCODED_URL_DICT_H
#define __MWS_INDEX_ENCODED_URL_DICT_H

// System includes

#include <stdint.h>

// Local includes

#include "common/utils/compiler_defs.h"
#include "mws/index/memsector_allocator.h"

/*--------------------------------------------------------------------------*/
/* Type declarations                                                        */
/*--------------------------------------------------------------------------*/

/**
 * @brief encoded_token -> offset_ptr pair
 */
struct encoded_url_dict_entry_s {
    uint32_t        encoded_url;
    memsector_off_t off;
} PACKED;
typedef struct encoded_url_dict_entry_s encoded_url_dict_entry_t;

/**
 * @brief Encoded Token Dictionary in-memory header
 */
struct encoded_url_dict_header_s {
    uint32_t size;
    encoded_url_dict_entry_t data[];
    /* followed by bulk of null-terminated URLs */
} PACKED;
typedef struct encoded_url_dict_header_s encoded_url_dict_header_t;

/**
 * @brief Encoded Token Dictionary handle
 */
typedef struct encoded_url_dict_handle_s {
    char*    data;
    uint32_t size;
} encoded_url_dict_handle_t;

/*--------------------------------------------------------------------------*/
/* Methods                                                                  */
/*--------------------------------------------------------------------------*/

BEGIN_DECLS


END_DECLS

#endif // __MWS_INDEX_ENCODED_TOKEN_DICT_H
