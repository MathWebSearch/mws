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

#include "writeJsonAnswset.hpp"

using namespace std;
using namespace mws;

namespace mws {

int writeJsonAnswset(mws::MwsAnswset* answset, FILE* file) {
    const char* data;
    size_t data_size;
    size_t bytes_written;
    json_object* json_doc, *qvars, *hits;

    json_doc = json_object_new_object();
    qvars = json_object_new_array();
    hits = json_object_new_array();

    json_object_object_add(json_doc, "total",
                           json_object_new_int(answset->total));

    // Creating qvars field
    for (int i = 0; i < (int)answset->qvarNames.size(); i++) {
        json_object* qvar = json_object_new_object();
        json_object_object_add(
            qvar, "name",
            json_object_new_string(answset->qvarNames[i].c_str()));
        json_object_object_add(
            qvar, "xpath",
            json_object_new_string(answset->qvarXpaths[i].c_str()));

        json_object_array_add(qvars, qvar);
    }

    json_object_object_add(json_doc, "qvars", qvars);

    // Creating hits field
    for (auto answer : answset->answers) {
        json_object* hit = json_object_new_object();
        json_object* math_ids = json_object_new_array();
        json_object* math_id = json_object_new_object();
        json_object_object_add(math_id, "url",
                               json_object_new_string(answer->uri.c_str()));
        json_object_object_add(math_id, "xpath",
                               json_object_new_string(answer->xpath.c_str()));
        json_object_array_add(math_ids, math_id);
        json_object_object_add(hit, "math_ids", math_ids);
        json_object_object_add(hit, "xhtml",
                               json_object_new_string(answer->data.c_str()));
        json_object_array_add(hits, hit);
    }

    json_object_object_add(json_doc, "hits", hits);

    string json_string = json_object_to_json_string(json_doc);

    data = json_string.c_str();
    data_size = json_string.size();

    bytes_written = 0;
    while (bytes_written < data_size) {
        bytes_written += fwrite(data + bytes_written, sizeof(char),
                                data_size - bytes_written, file);
    }

    json_object_put(json_doc);
    return data_size;
}
}
