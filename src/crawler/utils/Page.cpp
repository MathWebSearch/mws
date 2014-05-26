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
 * Implementation of the Page class
 *
 * author: Daniel Hasegan & Catalin Perticas
 * date: 28 Oct 2012
 */

#include <string>
#include <curl/curl.h>
#include <curl/easy.h>
#include <gurl.h>
#include <htmlcxx/html/ParserDom.h>
#include <htmlcxx/html/tree.h>

#include "common/utils/compiler_defs.h"

#include "Page.hpp"

// Namespaces
using namespace htmlcxx;
using namespace std;

/**
 * Implementation of CURL writer function
 */
int curlWriter(char* data, size_t size, size_t nmemb, std::string* buffer) {
    int result = 0;
    if (buffer != nullptr) {
        buffer->append(data, size * nmemb);
        result = size * nmemb;
    }
    return result;
}

/**
 * Constructor for Page
 */
Page::Page(string aUrl) {
    GURL gurl(aUrl);
    url = gurl;
    downloadedHeader = 0;
    downloadedContent = 0;
}

/**
 * Constructor for Page given a GURL
 */
Page::Page(GURL gurl) {
    url = gurl;
    downloadedHeader = 0;
    downloadedContent = 0;
}

/**
 * Function that downloads the Header and the Page from an URL
 */
void Page::downloadContent() {
    CURL* curl;
    curl = curl_easy_init();

    if (!curl) {
        PRINT_WARN("CURL: Cannot Initialize CURL\n");
        return;
    }
    string strUrl = getUrl();
    auto errorstr = new char[CURL_ERROR_SIZE];

    curl_easy_setopt(curl, CURLOPT_URL, strUrl.c_str());
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, curlWriter);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &content);
    curl_easy_setopt(curl, CURLOPT_NOPROGRESS, 1L);
    curl_easy_setopt(curl, CURLOPT_HEADERFUNCTION, curlWriter);
    curl_easy_setopt(curl, CURLOPT_WRITEHEADER, &header);
    curl_easy_setopt(curl, CURLOPT_CONNECTTIMEOUT, 100);
    curl_easy_setopt(curl, CURLOPT_ERRORBUFFER, errorstr);

    if (curl_easy_perform(curl) == 0) {
        downloadedContent = 1;
        downloadedHeader = 1;
    } else
        PRINT_WARN("CURL: Cannot Download page because: \n  %s \n", errorstr);

    curl_easy_cleanup(curl);
}

/**
 * Function that downloads the Header of a URL
 */
void Page::downloadHeader() {
    CURL* curl;
    curl = curl_easy_init();
    if (!curl) {
        PRINT_WARN("CURL: Cannot Initialize CURL\n");
        return;
    }
    string strUrl = getUrl();

    curl_easy_setopt(curl, CURLOPT_URL, strUrl.c_str());
    curl_easy_setopt(curl, CURLOPT_NOPROGRESS, 1L);
    curl_easy_setopt(curl, CURLOPT_NOBODY, 1);
    curl_easy_setopt(curl, CURLOPT_HEADERFUNCTION, curlWriter);
    curl_easy_setopt(curl, CURLOPT_WRITEHEADER, &header);

    if (curl_easy_perform(curl) == 0) {
        downloadedHeader = 1;
    } else
        PRINT_WARN("CURL: Cannot download header of page!\n");

    curl_easy_cleanup(curl);
}

/**
 * Getter function for the url
 * @return the url of the Page
 */
const string Page::getUrl() {
    string res;
    if (url.has_port())
        res = url.scheme() + "://" + url.host() + ":" + url.port() + url.path();
    else
        res = url.scheme() + "://" + url.host() + url.path();
    if (url.has_query()) res = res + "?" + url.query();
    return res;
}

/**
 * Getter function for the content of the page
 * @return the content of the Page
 */
const string& Page::getContent() {
    if (downloadedContent == 0) this->downloadContent();

    return content;
}

/**
 * Getter function for the header of the page
 * @return the header of the Page
 */
const string& Page::getHeader() {
    if (downloadedHeader == 0) this->downloadContent();

    return header;
}

/**
 * Funtion that gets the Modified date of the Page
 * @return the modified date of the Page
 */
string Page::getModifiedDate() {
    string to_find = "Last-Modified:";
    size_t where = header.find(to_find);
    if (where == string::npos) return "";
    string last_modified;
    for (int i = where + to_find.length(); header[i] != '\n'; ++i)
        last_modified.push_back(header[i]);
    return last_modified;
}

/**
 * Returns all Links found in an URL
 * @return a vector of strings of the links we got
 */
vector<GURL> Page::getLinks() {
    vector<GURL> answer;
    HTML::ParserDom parser;
    GURL curr(url);

    if (getContent() == "") return answer;

    tree<HTML::Node> dom = parser.parseTree(content);

    tree<HTML::Node>::iterator it = dom.begin();
    tree<HTML::Node>::iterator end = dom.end();
    for (; it != end; ++it) {
        if (it->tagName() == "a") {
            it->parseAttributes();
            string relative = it->attribute("href").second;

            GURL resolved = curr.Resolve(relative);
            if (resolved.host() == curr.host()) answer.push_back(resolved);
        }
    }

    return answer;
}
