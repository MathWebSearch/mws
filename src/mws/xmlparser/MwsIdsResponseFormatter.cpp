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
  * @todo fix writeData return value
  */

#include <json.h>

#include <string>
using std::string;

#include "mws/types/FormulaPath.hpp"
using mws::types::FormulaId;
#include "mws/xmlparser/MwsIdsResponseFormatter.hpp"

namespace mws {
namespace parser {

MwsIdsResponseFormatter MwsIdsResponseFormatter::instance;
MwsIdsResponseFormatter* RESPONSE_FORMATTER_MWS_IDS =
    &MwsIdsResponseFormatter::instance;

const char* MwsIdsResponseFormatter::getContentType() const {
    return "application/json";
}

int MwsIdsResponseFormatter::writeData(const void* data,
                                       FILE* output) const {
    const MwsAnswset& answerSet = *((const MwsAnswset*)data);
    json_object* json_doc = json_object_new_object();

    // Creating qvars field
    json_object* json_qvars = json_object_new_array();
    for (int i = 0; i < (int)answerSet.qvarNames.size(); i++) {
        json_object* json_qvar = json_object_new_object();
        json_object_object_add(
            json_qvar, "name",
            json_object_new_string(answerSet.qvarNames[i].c_str()));
        json_object_object_add(
            json_qvar, "xpath",
            json_object_new_string(answerSet.qvarXpaths[i].c_str()));

        json_object_array_add(json_qvars, json_qvar);
    }
    json_object_object_add(json_doc, "qvars", json_qvars);

    // Create ids field
    json_object* json_ids = json_object_new_array();
    for (FormulaId formulaId : answerSet.ids) {
        json_object_array_add(json_ids, json_object_new_int(formulaId));
    }
    json_object_object_add(json_doc, "ids", json_ids);

    string json_string = json_object_to_json_string(json_doc);
    fwrite(json_string.c_str(), json_string.size(), 1, output);

    json_object_put(json_doc);

    return json_string.size();
}

}  // namespace parser
}  // namespace mws
