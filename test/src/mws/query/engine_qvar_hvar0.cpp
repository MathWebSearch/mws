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
 * @file engine_qvar_hvar0.cpp
 *
 */

#include <string>
#include <cerrno>

#include "mws/index/MwsIndexNode.hpp"
#include "mws/index/encoded_token.h"
#include "mws/query/engine.h"

#include "engine_tester.hpp"

using namespace mws;
using namespace std;

/*

index: F(H,H,H): (apply,4) (F,1) (H,1) (H,1) (H,1)
hvars: F,H
query: Q(Q,P,h): (apply,4) (P,1) (P,1) (Q,1) (h,1)
qvars: P,Q

*/

struct Tester {
    static inline
    MwsIndexNode* create_test_MwsIndexNode() {
        MwsIndexNode* data = new MwsIndexNode();

        MwsIndexNode* H_node_1 = new MwsIndexNode();
        H_node_1->solutions = 1;
        data->children.insert(make_pair(H_ni, H_node_1));

        MwsIndexNode* F_node_1 = new MwsIndexNode();
        F_node_1->solutions = 1;
        data->children.insert(make_pair(F_ni, F_node_1));

        MwsIndexNode* apply_node_1 = new MwsIndexNode();
        apply_node_1->solutions = 1;
        data->children.insert(make_pair(apply4_ni, apply_node_1));

        MwsIndexNode* F_node_2 = new MwsIndexNode();
        apply_node_1->children.insert(make_pair(F_ni, F_node_2));

        MwsIndexNode* H_node_3 = new MwsIndexNode();
        F_node_2->children.insert(make_pair(H_ni, H_node_3));

        MwsIndexNode* H_node_4 = new MwsIndexNode();
        H_node_3->children.insert(make_pair(H_ni, H_node_4));

        MwsIndexNode* H_node_5 = new MwsIndexNode();
        H_node_5->solutions = 1;
        H_node_4->children.insert(make_pair(H_ni, H_node_5));

        return data;
    }
};

static encoded_formula_t create_test_query() {
    encoded_formula_t result;

    result.data = new encoded_token_t[5];
    result.size = 5;
    result.data[0] = apply4_tok;
    result.data[1] = P_tok;
    result.data[2] = P_tok;
    result.data[3] = Q_tok;
    result.data[4] = h_tok;

    return result;
}

static
result_cb_return_t result_callback(void* handle,
                                   const leaf_t * leaf) {
    UNUSED(handle);
    UNUSED(leaf);

    printf("Result found!\n");
    fflush(stdout);

    return QUERY_CONTINUE;
}

int main() {
    mws::MwsIndexNode* index = Tester::create_test_MwsIndexNode();
    encoded_formula_t query = create_test_query();

    return -1; // TODO remove after test is running
    return query_engine_tester(index, &query, result_callback, NULL);
}

