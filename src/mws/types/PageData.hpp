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
 * Header of PageData class
 *
 * Date: 27.III.2013
 * Author: Daniel Hasegan d.hasegan@jacobs-university.de
 */
#ifndef _PAGEDATA_HPP
#define _PAGEDATA_HPP


// System includes
#include <string.h>
#include <stdlib.h>
#include <string>

// Local includes
#include "mws/types/NodeInfo.hpp"
#include "mws/types/URLDictionary.hpp"

class PageData {
private:
    mws::MeaningId url_id;
    size_t fragid_size,xpath_size;
    const char* url_fragid;
    const char* xpath;

public:
    PageData(mws::URLDictionary *url_dict, const char* a_full_url, const char* an_xpath);
    PageData(mws::MeaningId an_url_id, const char* an_url_fragid,const char* an_xpath);
    ~PageData();

    std::string getFullUrl(mws::URLDictionary *url_dict);
    mws::MeaningId getUrl_id();
    const char* getUrl_fragid();
    const char* getXpath();

};

#endif
