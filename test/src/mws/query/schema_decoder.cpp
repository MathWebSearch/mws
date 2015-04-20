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
 * @date 20 Apr 2015
 *
 */

#include <string>
#include <cerrno>

#include "mws/index/MeaningDictionary.hpp"
using mws::index::MeaningDictionary;
#include "schema_engine_tester.hpp"
#include "mws/query/SchemaEngine.hpp"
#include "mws/types/CmmlToken.hpp"
using mws::types::CmmlToken;

using namespace mws;
using namespace query;
using namespace std;

/*

Formula: f(h,h,t): (apply,4) (f,0) (t,0) (t,0) (t,0)
We want to reconstruct it's CMML representation at various depths:

depth0:
?x (the 0 unification)

depth1:
(apply, 4) (f, 0) ?x1 ?x2 ?x3

depth2:
(apply,4) (f,0) (t,0) (t,0) (t,0)

*/


static EncodedFormula create_test_expr_at_d0() {
    return {};
}

static EncodedFormula create_test_expr_at_d1() {
    EncodedFormula expr;

    expr.push_back(apply4_tok);
    expr.push_back(f_tok);

    return expr;
}

static EncodedFormula create_test_expr_at_d2() {
    EncodedFormula expr;

    expr.push_back(apply4_tok);
    expr.push_back(f_tok);
    expr.push_back(t_tok);
    expr.push_back(t_tok);
    expr.push_back(t_tok);

    return expr;
}

static CmmlToken* get_expected_decode_at_d0() {
    CmmlToken* zeroUnif = CmmlToken::newRoot();
    zeroUnif->setTag(types::QVAR_TAG);
    zeroUnif->appendTextContent("x0");

    return zeroUnif;
}

static CmmlToken* get_expected_decode_at_d1() {
    CmmlToken* root = CmmlToken::newRoot();
    root->setTag("apply");
    CmmlToken* f = root->newChildNode();
    f->setTag("f");

    auto qvar1 = root->newChildNode();
    qvar1->setTag(types::QVAR_TAG);
    qvar1->appendTextContent("x1");

    auto qvar2 = root->newChildNode();
    qvar2->setTag(types::QVAR_TAG);
    qvar2->appendTextContent("x2");

    auto qvar3 = root->newChildNode();
    qvar3->setTag(types::QVAR_TAG);
    qvar3->appendTextContent("x3");

    return root;
}

static CmmlToken* get_expected_decode_at_d2() {
    CmmlToken* root = CmmlToken::newRoot();
    root->setTag("apply");

    CmmlToken* f = root->newChildNode();
    f->setTag("f");

    CmmlToken* t1 = root->newChildNode();
    t1->setTag("t");

    CmmlToken* t2 = root->newChildNode();
    t2->setTag("t");

    CmmlToken* t3 = root->newChildNode();
    t3->setTag("t");

    return root;
}

int main() {
    EncodedFormula expr0 = create_test_expr_at_d0();
    EncodedFormula expr1 = create_test_expr_at_d1();
    EncodedFormula expr2 = create_test_expr_at_d2();

    CmmlToken* dec0 = get_expected_decode_at_d0();
    CmmlToken* dec1 = get_expected_decode_at_d1();
    CmmlToken* dec2 = get_expected_decode_at_d2();

    FAIL_ON(Tester::test_expr_decoder(expr0, dec0, 0) != EXIT_SUCCESS);
    FAIL_ON(Tester::test_expr_decoder(expr1, dec1, 1) != EXIT_SUCCESS);
    FAIL_ON(Tester::test_expr_decoder(expr2, dec2, 2) != EXIT_SUCCESS);

    return EXIT_SUCCESS;

fail:
    delete dec0;
    delete dec1;
    delete dec2;
    return EXIT_FAILURE;
}
