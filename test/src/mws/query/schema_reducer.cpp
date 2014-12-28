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
 */

#include <string>
#include <cerrno>

#include "mws/index/MeaningDictionary.hpp"
using mws::index::MeaningDictionary;
#include "schema_engine_tester.hpp"
#include "mws/query/SchemaEngine.hpp"

using namespace mws;
using namespace query;
using namespace std;

/*
 *
query #1: f(h,h,t): (apply,4) (f,0) (t,0) (t,0) (t,0)
expected schema #1 at depth 0: ?x0 : {}

query #2: f(h,h,t): (apply,4) (f,0) (t,0) (t,0) (t,0)
expected schema #2 at depth 1: ?x1(?x2, ?x3, ?x4): (apply, 4)

query #3: f(h,h,t): (apply,4) (f,0) (t,0) (t,0) (t,0)
expected schema #3 at depth 2: f(h,t,t): (apply,4) (f,0) (t,0) (t,0) (t,0)
*/


static EncodedFormula create_test_expr() {
    EncodedFormula expr;

    expr.push_back(apply4_tok);
    expr.push_back(f_tok);
    expr.push_back(t_tok);
    expr.push_back(t_tok);
    expr.push_back(t_tok);

    return expr;
}

static EncodedFormula get_expected_reduction_at_d0() {
    return {};
}

static EncodedFormula get_expected_reduction_at_d1() {
    return {apply4_tok};
}

static EncodedFormula get_expected_reduction_at_d2() {
    return {apply4_tok, f_tok, t_tok, t_tok, t_tok};
}

int main() {
    EncodedFormula expr = create_test_expr();
    EncodedFormula exp0 = get_expected_reduction_at_d0();
    EncodedFormula exp1 = get_expected_reduction_at_d1();
    EncodedFormula exp2 = get_expected_reduction_at_d2();

    FAIL_ON(Tester::test_expr_reducer(expr,  exp0, 0) != EXIT_SUCCESS);
    FAIL_ON(Tester::test_expr_reducer(expr,  exp1, 1) != EXIT_SUCCESS);
    FAIL_ON(Tester::test_expr_reducer(expr,  exp2, 2) != EXIT_SUCCESS);

    return EXIT_SUCCESS;

fail:
    return EXIT_FAILURE;
}
