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
 * @file engine_qvar0.cpp
 *
 */

#include <string>
#include <cerrno>

#include "engine_tester.hpp"

using namespace mws;
using namespace std;

/*

index: f(h,h,t): (apply,4) (f,0) (h,0) (h,0) (t,0)
query: f(h,h,t): (apply,4) (P,0) (h,0) (h,0) (t,0)

1 solution expected

*/
static bool g_test_passed = false;
static uint64_t g_result_leaf_id;

struct Tester {
static
MwsIndexNode* create_test_MwsIndexNode() {
    MwsIndexNode* data = new MwsIndexNode();
    MwsIndexNode* leaf;

    leaf = data->insertData({f_tok});
    leaf->solutions++;
    leaf = data->insertData({t_tok});
    leaf->solutions++;
    leaf = data->insertData({h_tok});
    leaf->solutions++;
    leaf = data->insertData({apply4_tok, f_tok, h_tok, h_tok, t_tok});
    leaf->solutions++;

    /* save expected result leafId */
    g_result_leaf_id = leaf->id;

    return data;
}
};

static encoded_formula_t create_test_query() {
    MeaningId constantId = CONSTANT_ID_MIN;
    encoded_formula_t result;

    result.data = new encoded_token_t[5];
    result.size = 5;
    result.data[0] = encoded_token(constantId, 4);      // apply, 4
    result.data[1] = encoded_token(QVAR_ID_MIN, 0);     // P, 0
    result.data[2] = encoded_token(++constantId, 0);    // h, 0
    result.data[3] = encoded_token(constantId, 0);      // h, 0
    result.data[4] = encoded_token(++constantId, 0);    // t, 0

    return result;
}

static
result_cb_return_t result_callback(void* handle,
                                   const leaf_t * leaf) {
    UNUSED(handle);

    /* there is only 1 solution so getting here after the test has passed
       is an error */
    FAIL_ON(g_test_passed);

    FAIL_ON(leaf->formula_id != g_result_leaf_id);
    FAIL_ON(leaf->num_hits != 1);

    g_test_passed = true;

    return QUERY_CONTINUE;

fail:
    return QUERY_ERROR;
}

int main() {
    mws::MwsIndexNode* index = Tester::create_test_MwsIndexNode();
    encoded_formula_t query = create_test_query();

    return query_engine_tester(index, &query, result_callback, NULL);
}
