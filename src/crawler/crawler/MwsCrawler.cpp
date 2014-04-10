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
 * Implementation of the Crawler Class
 *
 * authors: Daniel Hasegan and Catalin Perticas
 * date: Summer 2012
 *
 */

// System includes
#include <string> // stl
#include <vector>
#include <queue>
#include <set>
#include <unistd.h>
#include <time.h>
#include <sstream>
#include <iostream>

// Local includes
#include "MwsCrawler.hpp"
#include "crawler/types/Robotstxt.hpp"
#include "crawler/types/SharedQueue.hpp"
#include "crawler/utils/MwsGetMath.hpp"
#include "crawler/utils/Page.hpp"
#include "common/utils/compiler_defs.h"

// Third Party includes
#include "gurl.h"

// Namespaces
using namespace std;

namespace mws 
{
/**
 * Implementation of the constructor
 */
Crawler::Crawler() {

    receivedProperties = 0;
    MAX_URLS_TO_CRAWL = INT_MAX;
    store_mathml_tags();
    rtxt = NULL;
}

/**
 * Implementation of the Destructor
 */
Crawler::~Crawler() {

    if ( currentMathFormulae.size() )
    {
        storeMath(currentMathFormulae);
        currentMathFormulae.clear();
    }

    //empty url_queue - not necessary anymore since all links in queue are processed
    while (!url_queue.empty()){
        url_queue.pop();
    }
    all_urls.clear();
    // delete rtxt
    if (rtxt != NULL)
    {
        delete rtxt;
        rtxt = NULL;
    }
    // empty mathml_tags
    mathml_tags.clear();
}

/**
 * Implementation of addURLStart
 */
void Crawler::addURLStart( string urlstart )
{
    URL_START = GURL(urlstart);
    if (!URL_START.is_valid())
    {
        PRINT_WARN("The Starting URL is not valid!\n");
        return ;
    }
    rtxt = new Robotstxt(URL_START);
    receivedProperties = 1;
}

/**
 * Implementation of addCrawlerCount
 */
void Crawler::addCrawlerCount(int cnt)
{
    if (cnt < 0)
    {
        PRINT_WARN("The count is less than 0 ... we use infinite instead\n");
        return ;
    }
    MAX_URLS_TO_CRAWL = cnt;
}

/**
 * Implementation of dontCrawl
 */
void Crawler::dontCrawl(string link) 
{
    rtxt->dont_allow(link);
}

/**
 * Implementation of addDataDirectory
 */
void Crawler::addDataDirectory( const char* dataDir)
{
    data_directory = string(dataDir);
}

/**
 * Implementation of start
 */
void Crawler::start() {

    if (!receivedProperties) {
        PRINT_WARN("You didn't give the Crawler any properties!\n");
        return ;
    }

    int urlsCrawled = 0;

    all_urls.insert(getQueryable(URL_START));
    url_queue.push(URL_START);

    while (!url_queue.empty() && urlsCrawled < MAX_URLS_TO_CRAWL) {

        // get the next URL from the queue
        GURL current_url = url_queue.front();
        cout << "Downloading " << current_url << endl;
        url_queue.pop();
        urlsCrawled ++ ;

        // retrieve page found at URL
        Page myPage(current_url);

        // retrieve math content from page and add it
        vector <string> mmath = get_math (myPage);
        addMath (mmath);

        // extract links from page and add valid ones to the queue
        vector <GURL> page_links = myPage.getLinks();

        cleanupLinks (page_links);
        cout << ">>> We have extracted " << page_links.size() << " good links" << endl;
        vector <GURL>::iterator it;
        for ( it = page_links.begin(); it != page_links.end() ; ++it )
        {
            GURL url_to_add = (*it);
            url_queue.push(url_to_add);
            all_urls.insert(getQueryable(url_to_add));
        }
    }


    storeMath ( currentMathFormulae );
    currentMathFormulae.clear();
    cout << "======Crawling complete!======\n";
    cout << "We scanned " << urlsCrawled << " pages!\n";
    cout << "There are " << url_queue.size() << " more in the queue\n";
    cout.flush();
    receivedProperties = 0;
}

/**
 * Implementation of run
 */
void Crawler::run(SharedQueue *sq,const char* dataDir)
{
    while (true)
    {
        while ( (*sq).sharedQueue.empty() )
        {
            sleep(1); // TODO
        }

        QueueNode qNode = (*sq).sharedQueue.front();

        pthread_mutex_lock(&sq->lock);
        (*sq).sharedQueue.pop();
        pthread_mutex_unlock(&sq->lock);

        Crawler newCrawler;

        newCrawler.addURLStart( qNode.urlstart );
        if (qNode.count != -1)
            newCrawler.addCrawlerCount( qNode.count );
        for(int i=0;i<qNode.dontcrawlnr;++i)
            newCrawler.dontCrawl( qNode.dontcrawl[i] );
        newCrawler.addDataDirectory(dataDir);

        newCrawler.start();
    }
}

/**
 * Implementation of storeMath
 */
void Crawler::storeMath(const char *xml)
{
    stringstream strs;
    strs << data_directory << "/harvest" << time(NULL) << ".xml";
    string fileName;
    strs >> fileName;
    FILE * f = fopen(fileName.c_str(), "w");
    if (f == NULL)
    {
        cerr << "Cannot open harvest file: " << fileName << endl;
        return ;
    }
    fwrite(xml, strlen(xml), sizeof(char), f);
    fclose(f);
}

/**
 * Implementation of storeMath
 */
void Crawler::storeMath(vector< string > &currentMF)
{
    vector<string>::iterator it;

    string xmlHarvest = "<?xml version=\"1.0\" ?> <mws:harvest xmlns:m=\"http://www.w3.org/1998/Math/MathML\" xmlns:mws=\"http://search.mathweb.org/ns\"> ";
    for(it = currentMF.begin() ; it != currentMF.end() ; it++)
        xmlHarvest.append((*it));
    xmlHarvest.append("</mws:harvest>");

    cout << "Sending " << currentMF.size() << " formulas to MWSD!" << endl;
    storeMath(xmlHarvest.c_str());
}

/**
 * Implementation of addMath
 */
void Crawler::addMath(vector<string> formulae)
{
    vector<string>::iterator it;

    cout << ">>> We have extracted " << formulae.size() << " math formulae \n";

    if (formulae.size()) // if there is something to add, add
    {
        for(it = formulae.begin() ; it != formulae.end() ; it++)
            currentMathFormulae.push_back( (*it) );
        formulae.clear();
    }

    if (currentMathFormulae.size() >= MathPackLimit) // if there are enough harvests, send to MWSD
    {
        storeMath(currentMathFormulae);
        currentMathFormulae.clear();
    }
}

/**
 * Implementation of cleanupLinks
 */
void Crawler::cleanupLinks(vector<GURL> &page_links)
{
    size_t pos = 0, i = 0;
    vector<GURL>::iterator it,itpos;
    GURL current;
    for(itpos = it = page_links.begin();it!=page_links.end();++i,++it)
    {
        current = (*it);
        if (rtxt->is_allowed_by_robots_txt (current))
            if (all_urls.find(getQueryable(current))==all_urls.end())
            {
                page_links[pos++] = (*it);
                itpos++;
            }
    }
    page_links.erase(itpos,page_links.end());
}

/**
 * Implementation of getQueryable
 */
string Crawler::getQueryable(GURL url)
{
    return url.path()+"?"+url.query();
}

} // namespace mws

