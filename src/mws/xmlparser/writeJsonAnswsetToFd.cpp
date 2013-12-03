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
  * @brief   File containing the implementation of the writeJsonAnswsetToFd
  * function
  * @file    writeJsonAnswsetToFd.cpp
  * @author  Corneliu-Claudiu Prodescu
  * @date    30 Jul 2011
  *
  * License: GPL v3
  *
  */

#include <unistd.h>

#include <sstream>
#include <vector>

#include "writeJsonAnswsetToFd.hpp"

using namespace std;
using namespace mws;

namespace mws
{

int
writeJsonAnswsetToFd(mws::MwsAnswset* answset, int fd)
{
    stringstream               ss;
    vector<mws::types::Answer*>::iterator it;
    const char*                data;
    size_t                     data_size;
    size_t                     bytes_written;
    string                     out;
    unsigned int               i;


    ss << "{\"size\":" << answset->answers.size()
       << ",\"total\":" << answset->total
       << ",\"data\":[";
    for (auto it = answset->answers.begin();
         it != answset->answers.end(); it ++) {
        if (it != answset->answers.begin())
        {
            ss << ",";
        }

        ss << "{\"uri\":\"" << (*it)->uri
           << "\",\"xpath\":\"" << (*it)->xpath
           << "\",\"qvars\":{";
        for (i = 0; i < answset->qvarNames.size(); i++)
        {
            if (i != 0) {
                ss << ",";
            }
            ss << "\"" << answset->qvarNames[i] << "\":\""
               << (*it)->xpath << answset->qvarXpaths[i] << "\"";
        }
        ss << "}}";
    }
    ss << "]}";

    out = ss.str();
    data = out.c_str();
    data_size = out.size();

    bytes_written = 0;
    while (bytes_written < data_size) {
        bytes_written += write(fd,
                               data + bytes_written,
                               data_size - bytes_written);
    }

    return data_size;
}

}

