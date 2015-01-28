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
 * @author Radu Hambasan
 * @date 28 Jan 2015
 * License: GPL v3
 *
 */

#include <unistd.h>
#include <json.h>
#include <stdio.h>

#include <vector>
using std::vector;
#include <string>
using std::string;

#include "mws/types/CmmlToken.hpp"
using mws::types::CmmlToken;
#include "mws/types/GenericAnswer.hpp"
#include "mws/types/SchemaAnswset.hpp"
#include "mws/types/ExprSchema.hpp"
using mws::types::ExprSchema;
#include "mws/xmlparser/SchemaJsonResponseFormatter.hpp"

using namespace mws;

namespace mws {
namespace parser {

SchemaJsonResponseFormatter SchemaJsonResponseFormatter::instance;
SchemaJsonResponseFormatter* RESPONSE_FORMATTER_SCHEMA_JSON =
        &SchemaJsonResponseFormatter::instance;

static const char HTTP_ENCODING[] = "application/json";

const char* SchemaJsonResponseFormatter::getContentType() const {
    return HTTP_ENCODING;
}

int SchemaJsonResponseFormatter::writeData(const GenericAnswer* ans,
                                           FILE* output) const {
    const SchemaAnswset& answSet = *((const SchemaAnswset*)ans);
    json_object* json_doc, *schemata;

    json_doc = json_object_new_object();
    schemata = json_object_new_array();

    /* Total number of formulae considered
     * (some schemata might have been dropped) */
    json_object_object_add(json_doc, "total",
                           json_object_new_int(answSet.total));
    json_object_object_add(json_doc, "size",
                           json_object_new_int(answSet.schemata.size()));

    for (const ExprSchema& exprSch : answSet.schemata) {
        json_object* schema = json_object_new_object();

        json_object_object_add(schema, "cmmlSchema",
            json_object_new_string(exprSch.root->toString().c_str()));
        json_object_object_add(schema, "coverage",
                               json_object_new_int(exprSch.coverage));

        json_object* formulae = json_object_new_array();
        for (const uint32_t f_id : exprSch.formulae) {
            json_object_array_add(formulae, json_object_new_int(f_id));
        }
        json_object_object_add(schema, "formulae", formulae);

        json_object* subst = json_object_new_array();
        for (const string& href : exprSch.subst) {
            json_object_array_add(subst,
                                  json_object_new_string(href.c_str()));
        }
        json_object_object_add(schema, "subst", subst);

        json_object_array_add(schemata, schema);
    }
    json_object_object_add(json_doc, "schemata", schemata);


    string json_string = json_object_to_json_string(json_doc);
    fwrite(json_string.c_str(), json_string.size(), 1, output);

    json_object_put(json_doc);
    return json_string.size();
}


}
}

