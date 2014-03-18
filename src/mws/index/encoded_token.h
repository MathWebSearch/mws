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
 * @brief   Encoded token dictionary
 * @file    encoded_token_dict.h
 * @date    26 Jan 2013
 *
 * License: GPLv3
 */

#ifndef __MWS_INDEX_ENCODED_TOKEN_DICT_H
#define __MWS_INDEX_ENCODED_TOKEN_DICT_H

// System includes

#include <stdint.h>
#include <stdbool.h>

// Local includes

#include "common/utils/compiler_defs.h"
#include "mws/index/memsector_allocator.h"

/*--------------------------------------------------------------------------*/
/* Constants                                                                */
/*--------------------------------------------------------------------------*/

#define HVAR_ID_MIN         1
#define HVAR_ID_MAX         32
#define ANON_HVAR_ID_MIN    33
#define ANON_HVAR_ID_MAX    64
#define QVAR_ID_MIN         65
#define QVAR_ID_MAX         96
#define ANON_QVAR_ID_MIN    97
#define ANON_QVAR_ID_MAX    128
#define VAR_ID_MAX          128
#define CONSTANT_ID_MIN     129

/*--------------------------------------------------------------------------*/
/* Type declarations                                                        */
/*--------------------------------------------------------------------------*/

/**
 * @brief CmmlToken encoding as uint32_t
 */
struct encoded_token_s {
    uint32_t  arity : 8;
    uint32_t  id    : 20;
    uint32_t  sort  : 4;
} PACKED;
typedef struct encoded_token_s encoded_token_t;

/**
 * @brief encoded_token -> offset_ptr pair
 */
struct encoded_token_dict_entry_s {
    encoded_token_t token;
    memsector_off_t off;
} PACKED;
typedef struct encoded_token_dict_entry_s encoded_token_dict_entry_t;

/**
 * @brief Encoded formula
 */
typedef struct encoded_formula_s {
    encoded_token_t* data;
    uint32_t         size;
} encoded_formula_t;

/*--------------------------------------------------------------------------*/
/* Methods                                                                  */
/*--------------------------------------------------------------------------*/

BEGIN_DECLS

static inline
uint32_t encoded_token_get_arity(encoded_token_t tok) {
    return tok.arity;
}

static inline
uint32_t encoded_token_get_sort(encoded_token_t tok) {
    return tok.sort;
}

static inline
uint32_t encoded_token_get_id(encoded_token_t tok) {
    return tok.id;
}

static inline
encoded_token_t encoded_token(uint32_t id, uint32_t arity, uint32_t sort) {
    encoded_token_t tok;

    tok.arity = arity;
    tok.id = id;
    tok.sort = sort;

    return tok;
}

static inline
bool encoded_token_is_var(encoded_token_t token) {
    return (token.id <= VAR_ID_MAX);
}

static inline
bool encoded_token_is_anon_var(encoded_token_t token) {
    return (ANON_HVAR_ID_MIN <= token.id && token.id < ANON_HVAR_ID_MAX) ||
           (ANON_QVAR_ID_MIN <= token.id && token.id < ANON_QVAR_ID_MAX);
}

END_DECLS

#endif // __MWS_INDEX_ENCODED_TOKEN_DICT_H
