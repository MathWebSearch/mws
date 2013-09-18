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
 * Class that represents a Page
 *
 * author: Daniel Hasegan & Catalin Perticas
 * date: 17 Aug 2012
 */
#ifndef _GETPAGE_HPP
#define _GETPAGE_HPP

// System Includes
#include <string> // stl
#include <cstring>
#include <vector>
#include <curl/curl.h> // curl
#include <curl/easy.h>
#include <htmlcxx/html/ParserDom.h> // htmlcxx
#include <htmlcxx/html/tree.h>

// Third Party includes
#include "gurl.h"

/* CURL writer */
int curlWriter (char *data, size_t size, size_t nmemb, std::string *buffer);

/**
 * Page class
 * keeps the data related to a page: url, header and content
 */
class Page {
private:
    GURL url; // Google URL url of the Page
    std::string header; // Header of the Page (is empty if downloadedHeader is 0)
    std::string content;// Content of the Page (is empty if downloadedContent is 0)
    int downloadedHeader; // May be 0 or 1
    int downloadedContent; // May be 0 or 1

    void downloadContent(); // Downloads the Content
    void downloadHeader(); // Downloads the Header

public:
    /**
     * Page Class Constructor from a string URL
     * @param The url of the Page
     */
    Page(std::string);

    /**
     * Page Class Constructor from a GURL URL
     * @param The url of the Page
     */
    Page(GURL);

    /**
     * Page class Getter of the URL
     * @return the Url of the Page
     */
    const std::string getUrl();

    /**
     * Page class Getter of the Header of the Page
     * @return the Header of the Page in string format
     */
    const std::string& getHeader();

    /**
     * Page class Getter of the Content of the Page
     * @return the Content of the Page in string format
     */
    const std::string& getContent();

    /**
     * Get the Date of the Page (when it was last modified)
     * @return the modified date of the Page in string format
     */
    std::string getModifiedDate();

    /**
     * Get The Links of the Page
     * @return a vector of GURL (urls) representing the links found in the page
     */
    std::vector< GURL > getLinks();

};


#endif
