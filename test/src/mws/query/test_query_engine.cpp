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

index: f(h,h,t): (apply,4) (f,0) (h,0) (h,0) (t,0)
query: f(h,h,t): (apply,4) (f,0) (h,0) (h,0) (t,0)

Meanings: 
          apply -> 65
          f -> 66
          h -> 67
          t -> 68

*/
static bool g_test_passed = false;
static uint64_t g_result_leaf_id;

static
MwsIndexNode* create_test_MwsIndexNode() {
    NodeInfo apply_ni = make_pair(65, 4);
    NodeInfo f_ni = make_pair(66, 0);
    NodeInfo h_ni = make_pair(67, 0);
    NodeInfo t_ni = make_pair(68, 0);

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
    /* save expected result leafId */
    g_result_leaf_id = t_node_5->id;

    return data;
}

static encoded_formula_t create_test_query() {
    encoded_formula_t result;

    result.data = new encoded_token_t[5];
    result.size = 5;
    result.data[0] = encoded_token(65, 4); // apply, 4
    result.data[1] = encoded_token(66, 0); // f, 0
    result.data[2] = encoded_token(67, 0); // h, 0
    result.data[3] = encoded_token(67, 0); // h, 0
    result.data[4] = encoded_token(68, 0); // t, 0

    return result;
}

static
result_cb_return_t result_callback(void* handle,
                                   const leaf_t * leaf) {
    UNUSED(handle);

    /* there is only 1 solution so getting here after the test has passed
       is an error */
    FAIL_ON(g_test_passed);

    FAIL_ON(leaf->dbid != g_result_leaf_id);
    FAIL_ON(leaf->num_hits != 1);

    g_test_passed = true;

    return QUERY_CONTINUE;

fail:
    return QUERY_ERROR;
}

int main() {
    mws::MwsIndexNode* index = create_test_MwsIndexNode();
    encoded_formula_t query = create_test_query();

    return query_engine_tester(index, &query, result_callback, NULL);
}
