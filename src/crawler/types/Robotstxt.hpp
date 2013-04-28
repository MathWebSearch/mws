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
 * Class to download and check the Robots.txt of a website
 *
 * author: Daniel Hasegan
 * date: 15 Aug 2012
 */
#ifndef _ROBOTSTXT_HPP
#define _ROBOTSTXT_HPP

// System includes
#include <string>

// Third party includes
#include "gurl.h"

/**
 * Robotstxt class keeps track of allowed and dissalowed links in a page
 * TODO: Also keeps track of the time between downloads
 */
class Robotstxt 
{

private:
    GURL startUrl; // Url of the Robots.txt page
    std::vector < GURL > not_allowed; // The list of links that are not allowed
    int CRAWL_DELAY; // The time we need to delay between two links

public:
    /**
     * Robotstxt Constructor with the Starting URL
     * Searches for the Crawler with any name
     * @param startUrl the Starting Url of the Page Crawler
     */
    Robotstxt(GURL startUrl);

    /**
     * Destructor of Robotstxt
     */
    ~Robotstxt();

    /**
     * Adds a url to avoid later in Crawling
     * @param url the url we want to avoid in our Crawling
     */
    void dont_allow( std::string url );

    /**
     * Query if the Page is allowed by the robots.txt file
     * @param url The url to be checked in GURL format
     * @return 1 if it is allowed, 0 if the link should not be downloaded
     */
    int is_allowed_by_robots_txt( GURL url );

};

#endif
