/**
 * @file test_query_engine_qvar_hvar.cpp
 *
 */

#include <string>
#include <cerrno>

#define private public

#include "mws/index/MwsIndexNode.hpp"
#include "mws/index/encoded_token_dict.h"
#include "mws/query/query_engine.h"

#include "query_engine_tester.hpp"

using namespace mws;
using namespace std;

/*

index: f(h,h,t): (apply,4) (f,1) (h,1) (h,1) (t,1)
query: f(h,h,t): (apply,4) (f,1) (h,1) (h,1) (t,1)

Meanings: 
          apply -> 65
          f -> 66
          h -> 67
          t -> 68

*/

static
MwsIndexNode* create_test_MwsIndexNode() {
    NodeInfo apply_ni = make_pair(65, 4);
    NodeInfo f_ni = make_pair(66, 1);
    NodeInfo h_ni = make_pair(67, 1);
    NodeInfo t_ni = make_pair(68, 1);

    MwsIndexNode* data = new MwsIndexNode();

    MwsIndexNode* h_node_1 = new MwsIndexNode();
    h_node_1->solutions = 1;
    data->children.insert(make_pair(h_ni, h_node_1));

    MwsIndexNode* f_node_1 = new MwsIndexNode();
    f_node_1->solutions = 1;
    data->children.insert(make_pair(f_ni, f_node_1));

    MwsIndexNode* apply_node_1 = new MwsIndexNode();
    apply_node_1->solutions = 1;
    data->children.insert(make_pair(apply_ni, apply_node_1));

    MwsIndexNode* t_node_1 = new MwsIndexNode();
    t_node_1->solutions = 1;
    data->children.insert(make_pair(t_ni, t_node_1));

    MwsIndexNode* f_node_2 = new MwsIndexNode();
    f_node_2->solutions = 1;
    apply_node_1->children.insert(make_pair(f_ni, f_node_2));

    MwsIndexNode* h_node_3 = new MwsIndexNode();
    h_node_3->solutions = 1;
    f_node_2->children.insert(make_pair(h_ni, h_node_3));

    MwsIndexNode* h_node_4 = new MwsIndexNode();
    h_node_4->solutions = 1;
    h_node_3->children.insert(make_pair(h_ni, h_node_4));

    MwsIndexNode* t_node_5 = new MwsIndexNode();
    t_node_5->solutions = 1;
    h_node_4->children.insert(make_pair(t_ni, t_node_5));

    return data;
}

static encoded_formula_t create_test_query() {
    encoded_formula_t result;

    result.data = new encoded_token_t[5];
    result.size = 5;
    result.data[0] = encoded_token(65, 4); // apply, 4
    result.data[1] = encoded_token(66, 1); // f, 1
    result.data[2] = encoded_token(67, 1); // h, 1
    result.data[3] = encoded_token(67, 1); // h, 1
    result.data[4] = encoded_token(68, 1); // t, 1

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
    mws::MwsIndexNode* index = create_test_MwsIndexNode();
    encoded_formula_t query = create_test_query();

    return query_engine_tester(index, &query, result_callback, NULL);
}

