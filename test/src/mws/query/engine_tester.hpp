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
 * @brief   Query Engine tester
 * @file    engine_tester.hpp
 * @date    24 April 2013
 *
 * License: GPLv3
 */

#ifndef __MWS_QUERY_ENGINE_TESTER_H
#define __MWS_QUERY_ENGINE_TESTER_H

/*--------------------------------------------------------------------------*/
/* Includes                                                                 */
/*--------------------------------------------------------------------------*/

#include <utility>

#include "common/utils/macro_func.h"
#include "mws/index/MwsIndexNode.hpp"
#include "mws/query/engine.h"
#include "mws/types/MeaningDictionary.hpp"
using mws::MeaningId;
#include "mws/types/NodeInfo.hpp"
using mws::NodeInfo;

/*--------------------------------------------------------------------------*/
/* Constants                                                                */
/*--------------------------------------------------------------------------*/

#define TMPFILE_PATH    "/tmp/test.map"
#define TMPFILE_SIZE    1024 * 1024

/*--------------------------------------------------------------------------*/
/* Methods                                                                  */
/*--------------------------------------------------------------------------*/

static inline
int query_engine_tester(mws::MwsIndexNode* data,
                        encoded_formula_t* query,
                        result_callback_t cb,
                        void *cb_handle) {

    const char* ms_path = TMPFILE_PATH;
    memsector_writer_t mswr;
    memsector_handle_t ms;

    /* ensure the file does not exist */
    FAIL_ON(unlink(TMPFILE_PATH) != 0 && errno != ENOENT);

    FAIL_ON(memsector_create(&mswr, ms_path, TMPFILE_SIZE) != 0);
    printf("Memsector %s created\n", ms_path);

    data->exportToMemsector(&mswr);
    printf("Index exported to memsector\n");
    printf("Space used: %d\n", memsector_size_inuse(&mswr.ms_header->alloc_header));

    FAIL_ON(memsector_save(&mswr) != 0);
    printf("Memsector saved\n");

    FAIL_ON(memsector_load(&ms, ms_path) != 0);
    printf("Memsector loaded\n");

    if (query_engine_run(&ms.index, query, cb, cb_handle)
            == QUERY_ERROR) {
        goto fail;
    }
    printf("Query successfull\n");

    FAIL_ON(memsector_remove(&ms) != 0);
    printf("Memsector removed\n");

    return EXIT_SUCCESS;

fail:
    if (data) delete data;
    return EXIT_FAILURE;
}

static inline
NodeInfo nodeInfoFromEncodedToken(encoded_token_t encoded_token) {
    return std::make_pair(encoded_token_get_id(encoded_token),
                          encoded_token_get_arity(encoded_token));
}

// Constants
MeaningId constantId = CONSTANT_ID_MIN;
encoded_token_t apply4_tok  = encoded_token(constantId++, 4);
encoded_token_t f_tok       = encoded_token(constantId++, 0);
encoded_token_t h_tok       = encoded_token(constantId++, 0);
encoded_token_t t_tok       = encoded_token(constantId++, 0);

NodeInfo apply4_ni  = nodeInfoFromEncodedToken(apply4_tok);
NodeInfo f_ni       = nodeInfoFromEncodedToken(f_tok);
NodeInfo h_ni       = nodeInfoFromEncodedToken(h_tok);
NodeInfo t_ni       = nodeInfoFromEncodedToken(t_tok);

// Hvars
MeaningId hvarId = HVAR_ID_MIN;
NodeInfo F_ni = std::make_pair(hvarId++, 0);
NodeInfo H_ni = std::make_pair(hvarId++, 0);

// Qvars
MeaningId qvarId = QVAR_ID_MIN;
encoded_token_t P_tok = encoded_token(qvarId++, 0);
encoded_token_t Q_tok = encoded_token(qvarId++, 0);
encoded_token_t R_tok = encoded_token(qvarId++, 0);

#endif // __MWS_QUERY_QUERY_ENGINE_TESTER_H
