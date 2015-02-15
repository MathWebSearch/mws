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
 * @brief   Query engine API
 * @file    engine.h
 * @date    21 Feb 2013
 *
 * License: GPLv3
 */

#ifndef __MWS_QUERY_ENGINE_H
#define __MWS_QUERY_ENGINE_H

#include "mws/index/index.h"
#include "mws/index/encoded_token.h"
#include "common/utils/compiler_defs.h"

/*--------------------------------------------------------------------------*/
/* Type declarations                                                        */
/*--------------------------------------------------------------------------*/

typedef enum result_cb_return_e {
    QUERY_CONTINUE,
    QUERY_STOP,
    QUERY_ERROR
} result_cb_return_t;

/* TODO report unificating instantiation */
typedef result_cb_return_t (*result_callback_t)(void* handle,
                                                const leaf_t* leaf);

/*--------------------------------------------------------------------------*/
/* Methods                                                                  */
/*--------------------------------------------------------------------------*/

BEGIN_DECLS

int query_engine_run(index_handle_t* RESTRICT index,
                     encoded_formula_t* RESTRICT query, result_callback_t cb,
                     void* RESTRICT cb_handle);

END_DECLS

#endif  // !__MWS_QUERY_QUERYENGINE_H
