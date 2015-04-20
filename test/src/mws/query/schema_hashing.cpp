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
#include <vector>

#include "mws/index/MeaningDictionary.hpp"
using mws::index::MeaningDictionary;
#include "schema_engine_tester.hpp"
#include "mws/query/SchemaEngine.hpp"

using namespace mws;
using namespace query;
using namespace std;

/*

query #1: f(t,t,t):  (apply,4) (f,0) (t,0) (t,0) (t,0)
query #2: f(t,t,t):  (apply,4) (f,0) (t,0) (t,0) (t,0)
query #3: f(cn,t,t): (apply,4) (f,0) (cn,0) (t,0) (t,0)
query #4: cn:        (cn,0)
query #5: g(t,t,t):  (apply,4) (g,0) (t,0) (t,0) (t,0)

At depth 0, we should have 1 schema:
?x (the 0 unification)

At depth 1, we should have 3 schemata:
(apply,4) (f,0)
(cn,0)
(apply,4) (g,0)

At depth 2, we should have 4 schemata.
(apply,4) (f,0) (t,0) (t,0) (t,0)
(apply,4) (f,0) (t,0) (t,0) (t,0)
(apply,4) (f,0) (cn,0) (t,0) (t,0)
(cn,0)
(apply,4) (g,0) (t,0) (t,0) (t,0)

*/


static vector<EncodedFormula> create_test_exprs() {
    vector<EncodedFormula> exprs(
    {{apply4_tok, f_tok, t_tok, t_tok, t_tok},
     {apply4_tok, f_tok, t_tok, t_tok, t_tok},
     {apply4_tok, f_tok, cn_tok, t_tok, t_tok},
     {cn_tok},
     {apply4_tok, g_tok, t_tok, t_tok, t_tok}});
    return exprs;
}

int main() {
    vector<EncodedFormula> exprs = create_test_exprs();

    FAIL_ON(Tester::test_expr_hashing(exprs, 1, 0) != EXIT_SUCCESS);
    FAIL_ON(Tester::test_expr_hashing(exprs, 3, 1) != EXIT_SUCCESS);
    FAIL_ON(Tester::test_expr_hashing(exprs, 4, 2) != EXIT_SUCCESS);

    return EXIT_SUCCESS;

fail:
    return EXIT_FAILURE;
}
