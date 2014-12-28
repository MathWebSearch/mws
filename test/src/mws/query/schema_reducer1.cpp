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

query: f(h,h,t): (apply,4) (f,0) (t,0) (t,0) (t,0)
expected schema: ?x1(?x2, ?x3, ?x4): (apply, 4)
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

static EncodedFormula get_expected_reduction() {
    return {apply4_tok};
}

int main() {
    EncodedFormula expr = create_test_expr();
    EncodedFormula exp = get_expected_reduction();

    FAIL_ON(Tester::test_expr_reducer(expr,  exp, 1) != EXIT_SUCCESS);

    return EXIT_SUCCESS;

fail:
    return EXIT_FAILURE;
}
