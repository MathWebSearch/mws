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
 * @file engine_rep_qvars0.cpp
 *
 */

#include <string>
#include <cerrno>

#include "engine_tester.hpp"

using namespace mws;
using namespace std;

/*

index: f(h,h,t): (apply,4) (f,0) (h,0) (h,0) (t,0)
query: f(h,h,t): (apply,4) (P,0) (Q,0) (Q,0) (t,0)

1 solution expected

*/
static int g_num_hits;

struct Tester {
    static index::TmpIndex* create_test_MwsIndexNode() {
        auto data = new index::TmpIndex();
        index::TmpLeafNode* leaf;

        leaf = data->insertData({f_tok});
        leaf->solutions++;
        leaf = data->insertData({t_tok});
        leaf->solutions++;
        leaf = data->insertData({h_tok});
        leaf->solutions++;
        leaf = data->insertData({apply4_tok, f_tok, h_tok, h_tok, t_tok});
        leaf->solutions++;

        return data;
    }
};

static encoded_formula_t create_test_query() {
    encoded_formula_t result;

    result.data = new encoded_token_t[5];
    result.size = 5;
    result.data[0] = apply4_tok;
    result.data[1] = P_tok;
    result.data[2] = Q_tok;
    result.data[3] = Q_tok;
    result.data[4] = t_tok;

    return result;
}

static result_cb_return_t result_callback(void* handle, const leaf_t* leaf) {
    UNUSED(handle);
    UNUSED(leaf);

    g_num_hits++;

    return QUERY_CONTINUE;
}

int main() {
    mws::index::TmpIndex* index = Tester::create_test_MwsIndexNode();
    encoded_formula_t query = create_test_query();

    FAIL_ON(query_engine_tester(index, &query, result_callback, nullptr) ==
            EXIT_FAILURE);
    FAIL_ON(g_num_hits != 1);

    return EXIT_SUCCESS;

fail:
    return EXIT_FAILURE;
}
