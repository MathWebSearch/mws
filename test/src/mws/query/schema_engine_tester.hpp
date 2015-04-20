/*

Copyright (C) 2010-2015 KWARC Group <kwarc.info>

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
 * @author Radu Hambasan
 * @date 28 Dec 2014
 *
 * License: GPLv3
 */

#ifndef __MWS_QUERY_SCHEMA_ENGINE_TESTER_H
#define __MWS_QUERY_SCHEMA_ENGINE_TESTER_H

#include <vector>
using std::vector;

#include "common/utils/compiler_defs.h"
#include "mws/index/encoded_token.h"
#include "mws/types/CmmlToken.hpp"
using mws::types::CmmlToken;
#include "mws/index/MeaningDictionary.hpp"
using mws::index::MeaningDictionary;
#include "mws/query/SchemaEngine.hpp"
using mws::query::SchemaEngine;
using mws::query::EncodedFormula;
using mws::query::RETRIEVE_ALL;
#include "mws/types/SchemaAnswset.hpp"
using mws::SchemaAnswset;

uint32_t g_meaning_dict_val_off = 1;
MeaningId constantId = CONSTANT_ID_MIN + g_meaning_dict_val_off;
encoded_token_t apply4_tok = encoded_token(constantId++, 4);
encoded_token_t f_tok      = encoded_token(constantId++, 0);
encoded_token_t h_tok      = encoded_token(constantId++, 0);
encoded_token_t t_tok      = encoded_token(constantId++, 0);
encoded_token_t g_tok      = encoded_token(constantId++, 0);
encoded_token_t cn_tok     = encoded_token(constantId++, 0);


MeaningDictionary get_meaning_dict() {
    MeaningDictionary dict;
    dict.put("apply#");
    dict.put("f#");
    dict.put("h#");
    dict.put("t#");
    dict.put("g#");
    dict.put("cn#3.5");

    return dict;
}

struct Tester {
    static inline int test_expr_reducer(const EncodedFormula& expr,
                                        const EncodedFormula& expectedExpr,
                                        uint8_t depth) {
        MeaningDictionary dict = get_meaning_dict();
        SchemaEngine schEng(dict);
        EncodedFormula result = schEng.reduceFormula(expr, depth);

        if (expectedExpr.size() != result.size()) return EXIT_FAILURE;

        for (size_t i = 0; i < result.size(); i++) {
            encoded_token_t expected = expectedExpr[i];
            encoded_token_t returned = result[i];
            if (expected.id != returned.id ||
                    expected.arity != returned.arity) {
                return EXIT_FAILURE;
            }
        }

        return EXIT_SUCCESS;
    }

    static inline int test_expr_hashing(const vector<EncodedFormula>& exprs,
                                        size_t expected, uint8_t depth) {
        MeaningDictionary dict = get_meaning_dict();
        SchemaEngine schEng(dict);
        SchemaAnswset* answset = schEng.getSchemata(exprs, {},
                                                    RETRIEVE_ALL, depth);
        size_t nrSch = answset->schemata.size();
        delete answset;

        if (nrSch != expected) return EXIT_FAILURE;
        return EXIT_SUCCESS;
    }

    static inline int test_expr_decoder(const EncodedFormula& expr,
                                        const CmmlToken* expected,
                                        uint8_t depth) {
        MeaningDictionary dict = get_meaning_dict();
        SchemaEngine schEng(dict);
        CmmlToken* tok = schEng.decodeFormula(expr, depth);
        bool success = tok->equals(expected);
        delete tok;

        if (!success) return EXIT_FAILURE;
        return EXIT_SUCCESS;
    }
};


#endif // __MWS_QUERY_QUERY_SCHEMA_ENGINE_TESTER_H
