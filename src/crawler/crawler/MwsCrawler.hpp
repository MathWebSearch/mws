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
/*
 * Crawler class
 *
 * authors: Daniel Hasegan and Catalin Perticas
 * date: Summer 2012
 *
 */
#ifndef _MWSCRAWLER_HPP
#define _MWSCRAWLER_HPP

// System includes
#include <algorithm>
#include <iostream>
#include <fstream>
#include <string>
#include <cstring>
#include <cstdio>
#include <vector>
#include <queue>
#include <set>
#include <map> 

// Local includes
#include "crawler/types/Robotstxt.hpp"
#include "crawler/types/SharedQueue.hpp"

// Third Party includes
#include "gurl.h"

namespace mws
{
const unsigned int MathPackLimit = 5000;

class Crawler {

public:
    /**
     * @brief Crawler Constructor
     */
    Crawler();

    /**
     * @brief Crawler Destructor
     */
    ~Crawler();

    /**
     * @brief start the Crawler Thread
     * @param sq The Shared Queue where the Crawler Receives Crawling Commands
     * @param dataDir The Directory where to store the Harvest Files for later use
     */
    static void run(SharedQueue *sq, const char *dataDir);

    /**
     * @brief Add the starting URL
     * @param url The url to the String we will start with
     */
    void addURLStart( std::string );

    /**
     * @brief Add the number of pages we will crawl
     * @param nr The number of pages we will crawl
     */
    void addCrawlerCount(int);

    /**
     * @brief Avoid a url
     * @param url The url the Crawler will avoid
     */
    void dontCrawl( std::string );

    /**
     * @brief Add the path where to store the generated Harvest Files
     * @param path The path to the directory
     */
    void addDataDirectory( const char* );

    /**
     * @brief Start the Crawler with the already specified Options
     * @preliminary Need to receive the required properties. The function addURLStart must be called first
     */
    void start();


private:
    std::vector<std::string> currentMathFormulae; // Math formulas stored for sending

    std::queue < GURL > url_queue; // Queue of URL's to Crawl
    int MAX_URLS_TO_CRAWL; // The Maximum count of URL we will crawl in this session
    GURL URL_START; // The Starting URL
    std::set < std::string > all_urls; // The Set of all URL's for avoiding crawling duplicates
                                  // has the format: path?query
    std::string data_directory; // The Path of storing the Harvest Files

    Robotstxt* rtxt; // The Robotstxt that handles the disallowed links on a website
    int receivedProperties; // boolean that keeps track of the properties received

    /**
     * @brief Send a message to the MWS Daemon and stores it in a Harvest File
     * @param msg the message to be sent
     */
    void storeMath(const char *);

    /**
     * @brief Prepare the dump string and calls storeMath(const char *)
     * @param currentMF the list of MathML formulas
     */
    void storeMath(std::vector< std::string > &currentMF);

    /**
     * @brief adds the list of formulas to our set of formulas. If our set surpasses the pack limit (MathPackLimit)
     *        it is sent to the MWS Daemon and stored in a Harvest File
     * @param the list of formulas to be added
     */
    void addMath(std::vector< std::string > formulae);

    /**
     * @brief cleans up the vector of urls. Keep only the urls that are allowed by the robots.txt file and were not
     *        crawled before
     * @param links the set of links to be cleaned
     */
    void cleanupLinks( std::vector< GURL > &);

    /**
     * @brief Transform a GURL into a queryable string of the form path?query to be put in the all_urls set
     * @param url the GURL to be transformed
     * @return the path?query format of the GURL url
     */
    std::string getQueryable(GURL url);
};

} // namespace mws

#endif
