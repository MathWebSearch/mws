/*

Copyright (C) 2010-2014 KWARC Group <kwarc.info>

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
 * @file range-query.cpp
 *
 */

#include <string>
#include <cerrno>
#include <vector>
#include <unordered_map>
#include <utility>

#include "mws/index/TmpIndexAccessor.hpp"
using mws::index::TmpIndexAccessor;
#include "mws/index/MeaningDictionary.hpp"
using mws::index::MeaningDictionary;
#include "mws/types/Query.hpp"
using mws::types::Query;
#include "mws/types/MwsAnswset.hpp"
using mws::MwsAnswset;
#include "mws/query/SearchContext.hpp"
using mws::query::SearchContext;

using namespace mws;
using namespace std;

/*

index: f(h,h,t): (apply,4) (f,0) (cn,0) (h,0) (t,0)
query: f(h,h,t): (apply,4) (f,0) (R[-5,5],0) (h,0) (t,0)

1 solution expected

*/

static uint32_t g_result_leaf_id;
static uint32_t g_meaning_dict_val_off;

// Constants --> the ones from engine_tester.hpp might have invalid MeaningIds
// e.g. CONSTANT_ID_MIN is not a valid token id in this case,
// because we are using MeaningDictionary and the position 0 is reserved
static encoded_token_t apply4_tok;
static encoded_token_t f_tok;
static encoded_token_t h_tok;
static encoded_token_t t_tok;
static encoded_token_t cn_tok;

struct Tester {
    static void setup() {
        g_meaning_dict_val_off = MeaningDictionary::VALUEID_START;

        MeaningId constantId = CONSTANT_ID_MIN + g_meaning_dict_val_off;
        apply4_tok  = encoded_token(constantId++, 4);
        f_tok       = encoded_token(constantId++, 0);
        h_tok       = encoded_token(constantId++, 0);
        t_tok       = encoded_token(constantId++, 0);
        cn_tok      = encoded_token(constantId++, 0);
    }

    static index::TmpIndex* create_test_MwsIndexNode() {
        auto data = new index::TmpIndex();
        index::TmpLeafNode* leaf;

        leaf = data->insertData({f_tok});
        leaf->solutions++;
        leaf = data->insertData({cn_tok});
        leaf->solutions++;
        leaf = data->insertData({h_tok});
        leaf->solutions++;
        leaf = data->insertData({t_tok});
        leaf->solutions++;
        leaf = data->insertData({apply4_tok, f_tok, cn_tok, h_tok, t_tok});
        leaf->solutions++;

        /* save expected result leafId */
        g_result_leaf_id = leaf->id;

        return data;
    }

    static MeaningDictionary get_index_meaning_dict() {
        MeaningDictionary dict;
        dict.put("apply#");
        dict.put("f#");
        dict.put("h#");
        dict.put("t#");
        dict.put("cn#3.5");

        return dict;
    }
};

static vector<encoded_token_t> create_test_query() {
    vector<encoded_token_t> encodedQuery;
    encodedQuery.resize(5);

    encodedQuery[0] = apply4_tok;
    encodedQuery[1] = f_tok;
    encodedQuery[2] = encoded_token(RANGE_ID_MIN, 0);  // R, 0
    encodedQuery[3] = h_tok;
    encodedQuery[4] = t_tok;

    return encodedQuery;
}

static unordered_map<MeaningId, pair<double, double>>
get_test_range_bounds() {
    unordered_map<MeaningId, pair<double, double>> bounds;
    bounds.insert({RANGE_ID_MIN, {-5.0, 5.0}});

    return bounds;
}

int main() {
    Tester::setup();
    mws::index::TmpIndex* index = Tester::create_test_MwsIndexNode();
    unordered_map<MeaningId, pair<double, double>> rangeBounds;
    rangeBounds = get_test_range_bounds();
    MeaningDictionary dict = Tester::get_index_meaning_dict();
    vector<encoded_token_t> encodedQuery = create_test_query();
    Query defaultQuery;
    defaultQuery.options.includeHits = false;

    SearchContext ctxt(encodedQuery, defaultQuery.options, rangeBounds, &dict);
    MwsAnswset* answ =  ctxt.getResult<TmpIndexAccessor>(
                index, nullptr, defaultQuery.attrResultLimitMin,
                defaultQuery.attrResultMaxSize,
                defaultQuery.attrResultTotalReqNr);
    FAIL_ON(answ == nullptr);
    FAIL_ON(answ->total != 1);
    FAIL_ON(*(answ->ids.begin()) != g_result_leaf_id);

    return EXIT_SUCCESS;
fail:
    return EXIT_FAILURE;
}
