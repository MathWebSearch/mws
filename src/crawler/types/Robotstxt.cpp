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
 * Implementation of the Robotstxt Class
 */

// System includes
#include <string>
#include <vector>
#include <iostream>

// Local includes
#include "Robotstxt.hpp"
#include "crawler/types/Page.hpp"

// Third Party includes
#include "gurl.h"

using namespace std;

/** 
 * Implementation of Robotstxt constructor
 *
 * Download the robots.txt file and put the bad urls in the 'not_allowed' set
 */
Robotstxt::Robotstxt(GURL aStartUrl) {

    startUrl = aStartUrl.GetOrigin().Resolve("/robots.txt");
    Page robotsPage(startUrl);

    string rob = robotsPage.getContent();
    if (rob == "")
    {
        fprintf(stderr,"Error downloading the Robots page!\n");
        return ;
    }

    // If Our Crawler has no name
    string userAgent = "User-agent: ";
    string thisCrawler = "*";
    string disallow = "Disallow: ";
    int isInBrackets = 0;
    FILE *stream;
    char *robpg = new char[rob.size()];
    strcpy(robpg,rob.c_str());
    stream = fmemopen(robpg,rob.size(),"r");
    const int bufmax = 1000;
    char buf[bufmax];
    while( fgets(buf,bufmax,stream) != NULL )
    {
        string line(buf);
        if (line.compare(0,userAgent.size(),userAgent) == 0)
        {
            if (isInBrackets)
                isInBrackets = 0;
            else if (line.compare(0,userAgent.size()+thisCrawler.size(),userAgent+thisCrawler) == 0)
                isInBrackets = 1;
        }

        if (isInBrackets && line.compare(0,disallow.size(),disallow) == 0)
        {
            string dontCrawlLink = line.substr(disallow.size());
            this->dont_allow(dontCrawlLink);
        }
    }
    //TODO: Crawl-Delay getter for Non-Standard robots.txt extension
}

/**
 * Robotstxt destructor
 */
Robotstxt::~Robotstxt()
{
    not_allowed.clear();
}

/**
 * Dont allow to crawl the 'url' link
 */
void Robotstxt::dont_allow(string url)
{
    GURL gurl = startUrl.Resolve(url);
    not_allowed.push_back(gurl);
}

/**
 * See if a url is accepted to crawl by the site
 */
int Robotstxt::is_allowed_by_robots_txt(GURL gurl) {
    vector<GURL>::iterator it;
    GURL c;
    string cp;

    for(it = not_allowed.begin() ; it!=not_allowed.end() ; ++it)
    {
        c = (*it);
        if (gurl.host()==c.host() && gurl.query()==c.query())
        {
            cp = c.path();
            if (cp[cp.size()-1] == '/')
            {
                if (gurl.path().substr(0,cp.length()-1)==cp.substr(0,cp.length()-1))
                    return 0;
            }
            else
            {
                if (gurl.path().substr(0,cp.length())==cp)
                    return 0;
            }
        }
    }
    return 1;
}
