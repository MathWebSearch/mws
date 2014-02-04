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
 * @file PageData.cpp
 * @brief PageData implementation
 */

// System includes
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <string>

// Local includes
#include "PageData.hpp"
#include "mws/types/URLDictionary.hpp"
#include "mws/types/NodeInfo.hpp"

// Namespaces
using namespace std;

PageData::PageData(mws::URLDictionary* url_dict, const char* a_full_url, const char* an_xpath) {

    char* url_aux;
    char* url_fragid_aux;
    char* xpath_aux;
    size_t len_xpath,len_fullurl;
    size_t len_url,len_fragid;

    len_xpath = strlen(an_xpath);
    len_fullurl = strlen(a_full_url);

    xpath_aux = new char[len_xpath+1];
    strncpy(xpath_aux, an_xpath,len_xpath);
    xpath_aux[len_xpath] = '\0';
    this-> xpath = xpath_aux;
    this-> xpath_size = len_xpath;

    // Get the position of the hashtag '#'
    size_t pos = len_fullurl;
    for(size_t i = 0; i< len_fullurl && pos==len_fullurl; ++i)
        if (a_full_url[i] == '#')
            pos = i;

    // Divide the url and the id
    len_url = pos;
    url_aux = new char[len_url+1];
    memcpy(url_aux, a_full_url,len_url);
    url_aux[len_url] = '\0';

    if (pos != len_fullurl) {
        len_fragid = len_fullurl - (pos + 1);
        url_fragid_aux = new char[len_fragid+1];
        memcpy(url_fragid_aux, a_full_url+pos+1,len_fragid);
        url_fragid_aux[len_fragid] = '\0';
    } else {
        len_fragid = 0;
        url_fragid_aux = new char[len_fragid+1];
        url_fragid_aux[len_fragid] = '\0';
    }
\
    // Store the results
    this-> url_id = url_dict->put( (mws::Meaning) url_aux);
    this-> url_fragid = url_fragid_aux;

    this-> fragid_size = len_fragid;

    // Cleanup
    delete [] url_aux;
}

PageData::PageData(mws::MeaningId an_url_id, const char* an_url_fragid,const char* an_xpath) {
    url_id = an_url_id;

    fragid_size = strlen(an_url_fragid);
    xpath_size = strlen(an_xpath);

    char* url_fragid_aux = new char[fragid_size + 1];
    char* xpath_aux = new char[xpath_size + 1];

    strcpy(url_fragid_aux,an_url_fragid);
    strcpy(xpath_aux,an_xpath);

    url_fragid = url_fragid_aux;
    xpath = xpath_aux;
}

PageData::~PageData() {
    delete [] url_fragid;
    delete [] xpath;
}

string PageData::getFullUrl(mws::URLDictionary* url_dict) {

    char *full_url;
    size_t len_url,len_fullurl;

    mws::Meaning url_aux;

    if ( (url_aux = url_dict->get(url_id)) == mws::MWS_URLMEANING_NO_URL) {
        fprintf(stderr,"PageData: Cannot retrieve item from url_dict\n");
        return NULL;
    }

    len_url = url_aux.size();
    if (fragid_size > 0) // if there is an Id
        len_fullurl = len_url + fragid_size + 1;
    else
        len_fullurl = len_url + fragid_size;

    full_url = new char[len_fullurl+1];
    strncpy(full_url,url_aux.c_str(),len_url);

    if (fragid_size > 0) { // If there is an Id put the '#' and the Id
        full_url[len_url] = '#';
        strncpy(full_url+len_url+1,url_fragid,fragid_size);
    }
    full_url[len_fullurl] = '\0';

    string ret(full_url);

    delete []full_url;

    return ret;
}

mws::MeaningId PageData::getUrl_id() {
    return url_id;
}

const char* PageData::getUrl_fragid() {
    return url_fragid;
}

const char* PageData::getXpath() {
    return xpath;
}
