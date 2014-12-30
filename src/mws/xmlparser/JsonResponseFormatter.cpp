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
  * @brief   File containing the implementation of the writeJsonAnswset
  * function
  * @file    writeJsonAnswset.cpp
  * @author  Corneliu-Claudiu Prodescu
  * @date    30 Jul 2011
  *
  * @edited Radu Hambasan
  * @date 20 Mar 2014
  *
  * License: GPL v3
  *
  */

#include <unistd.h>
#include <json.h>
#include <stdio.h>

#include <sstream>
#include <vector>
#include <string>

#include "mws/xmlparser/JsonResponseFormatter.hpp"

using namespace std;
using namespace mws;

namespace mws {
namespace parser {

JsonResponseFormatter JsonResponseFormatter::instance;
JsonResponseFormatter* RESPONSE_FORMATTER_JSON =
    &JsonResponseFormatter::instance;

static const char HTTP_ENCODING[] = "application/json";

const char* JsonResponseFormatter::getContentType() const {
    return HTTP_ENCODING;
}

int JsonResponseFormatter::writeData(const void* data,
                                     FILE* output) const {
    const MwsAnswset& answerSet = *((const MwsAnswset*)data);
    json_object* json_doc, *qvars, *hits;

    json_doc = json_object_new_object();
    qvars = json_object_new_array();
    hits = json_object_new_array();

    json_object_object_add(json_doc, "total",
                           json_object_new_int(answerSet.total));

    // Creating qvars field
    for (int i = 0; i < (int)answerSet.qvarNames.size(); i++) {
        json_object* qvar = json_object_new_object();
        json_object_object_add(
            qvar, "name",
            json_object_new_string(answerSet.qvarNames[i].c_str()));
        json_object_object_add(
            qvar, "xpath",
            json_object_new_string(answerSet.qvarXpaths[i].c_str()));

        json_object_array_add(qvars, qvar);
    }
    json_object_object_add(json_doc, "qvars", qvars);

    // Creating hits field
    for (auto answerPtr : answerSet.answers) {
        json_object* hit = json_object_new_object();
        json_object* math_ids = json_object_new_array();
        json_object* math_id = json_object_new_object();
        json_object_object_add(math_id, "url",
                               json_object_new_string(answerPtr->uri.c_str()));
        json_object_object_add(
            math_id, "xpath", json_object_new_string(answerPtr->xpath.c_str()));
        json_object_array_add(math_ids, math_id);
        json_object_object_add(hit, "math_ids", math_ids);
        json_object_object_add(hit, "xhtml",
                               json_object_new_string(answerPtr->data.c_str()));
        json_object_array_add(hits, hit);
    }

    json_object_object_add(json_doc, "hits", hits);

    string json_string = json_object_to_json_string(json_doc);
    fwrite(json_string.c_str(), json_string.size(), 1, output);

    json_object_put(json_doc);
    return json_string.size();
}
}
}
