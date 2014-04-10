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
 * Implementation of the functions declared in MwsGetMath
 * to extract MathML formulae from a website coded in
 * HTML or XHTML
 *
 * authors: Daniel Hasegan & Catalin Perticas
 * date: 15 Aug 2012
 */

// System includes
#include <string> // stl
#include <vector>
#include <algorithm>
#include <cstring>
#include <iostream>
#include <fstream>
#include <libxml/parser.h> // LIBXML2 library for XHTML parsing
#include <libxml/HTMLparser.h>
#include <libxml/xpath.h>
#include <libxml/xpathInternals.h>

#include "gurl.h"

#include "common/utils/compiler_defs.h"
#include "common/utils/memstream.h"
#include "crawler/utils/Page.hpp"
#include "crawler/utils/MwsGetMath.hpp"

#include "build-gen/config.h"

using namespace std;
using namespace mws;

std::set < std::string > mathml_tags;

/*
 * Helper function for get_math_html
 *
 * Get math blocks from a html code
 * returns a vector of formulas together with their id's(if they have one)
 */
vector <pair<string, string> > get_math_blocks(string html)
{
    store_mathml_tags();
    int pos = 0, sz = html.size();
    vector <pair<int, int> > allpos;
    string matho = "<math";
    string mathc = "</math>";

    // finds all opening math tags and stores starting position in all pos
    while (1)
    {
        size_t oc = html.find(matho, pos);
        if (oc == string::npos) break;
        pos = oc + matho.size();
        allpos.push_back(make_pair(oc, 1));
        // check whether the tag closes by itself
        while(1)
        {
            if (pos == sz) break;
            if (html[pos] == '>') break;
            if (html[pos] == '/' && (pos + 1) < sz && html[pos+1] == '>')
            {
                allpos.push_back(make_pair(pos, -1));
                pos = pos + 2;
            }
            ++pos;
        }
    }
    pos = 0;

    // finds all closing math tags and stores starting position in allpos
    while (1)
    {
        size_t oc = html.find(mathc, pos);
        if (oc == string::npos) break;
        pos = oc + mathc.size();
        allpos.push_back(make_pair(oc, -1));
    }
    sort (allpos.begin(), allpos.end());

    //for (int i = 0; i < allpos.size(); ++i) cout << allpos[i].first << " " << allpos[i].second << endl;

    vector <pair<string, string> > ans;

    // extract math content and add namespaces to mathml tags

    int lpos = 0, val = 0;
    for (int i = 0; (unsigned int)i < allpos.size(); ++i)
    {
        val += allpos[i].second;
        if (val == 0)
        {
            string nstr;
            int start = allpos[lpos].first;
            int stop = allpos[i].first + mathc.size();
            if (html[allpos[i].first] == '/') stop = allpos[i].first + 2; // the case when the tag closes by itself

            for (int j = start; j < stop; ++j)
                if (html[j] != '<') nstr.push_back(html[j]);
                else
                {
                    string g;
                    int k = j + 1;
                    if (html[k] == '/')
                    {
                        g.push_back(html[k]);
                        ++k;
                    }
                    while (k < stop && islower(html[k]))
                    {
                        g.push_back(html[k]);
                        ++k;
                    }
                    if (mathml_tags.find(g) != mathml_tags.end())
                    {
                        if (g[0] == '/') {nstr = nstr + "</m:"; ++j;}
                        else nstr = nstr + "<m:";
                    }
                    else nstr.push_back(html[j]);
                }
            lpos = i + 1;

            // check whether formula has an id, save it and return it
            string id;
            size_t we = nstr.find(">");
            size_t wid = nstr.find(" id=");
            if (wid != string::npos && wid < we) for (int j = wid + 5; nstr[j] != '"'; ++j) id.push_back(nstr[j]);

            ans.push_back(make_pair(nstr, id));
        }
    }

    return ans;
}

/*
 * Helper function for get_math_xhtml
 *
 * Implementation of get_XMLDoc
 */
xmlDocPtr get_XMLDoc (const char *buffer) {
    int size = strlen(buffer);
    xmlDocPtr doc;
    doc = xmlParseMemory(buffer,size);

    if (doc == NULL ) {
        PRINT_WARN("Document not parsed successfully. \n");
        return NULL;
    }

    return doc;
}

/*
 * Helper function for get_math_xhtml
 *
 * Implementation of get_XMLNodeset
 */
xmlXPathObjectPtr get_XMLNodeset (xmlDocPtr doc, const xmlChar *xpath){

    xmlXPathContextPtr context;
    xmlXPathObjectPtr result;

    context = xmlXPathNewContext(doc);
    if (context == NULL) {
        printf("Error in xmlXPathNewContext\n");
        return NULL;
    }

    // Register namespace m
    xmlChar *prefix = (xmlChar*) "m";
    xmlChar *href = (xmlChar*) "http://www.w3.org/1998/Math/MathML";
    if(xmlXPathRegisterNs(context, prefix , href) != 0) {
        PRINT_WARN("Error: unable to register NS with prefix=\"%s\" and href=\"%s\"\n", prefix, href);
        return NULL;
    }

    result = xmlXPathEvalExpression(xpath, context);
    xmlXPathFreeContext(context);
    if (result == NULL) {
        printf("Error in xmlXPathEvalExpression\n");
        return NULL;
    }
    if(xmlXPathNodeSetIsEmpty(result->nodesetval)){
        xmlXPathFreeObject(result);
        printf("No result\n");
        return NULL;
    }
    return result;
}

// =============== IMPLEMENTATION ================
namespace mws {

// ============ HTML RELATED ================

/*
 * Implementation of isHTML
 * returns 1 if the header says that the page is HTML
 */
int isHTML(Page& page) { 
    string header = page.getHeader();

    // Search for "Content-Type: text/html" in the header file
    string ctype = "Content-Type: text/html";
    return (header.find(ctype) != string::npos);
}

/* 
 * Implementation of get_math_html
 * returns a vector of strings of math formulas
 */
vector<string> get_math_html(Page& page) {

    string html = page.getContent();
    string url = page.getUrl();

    vector<pair<string, string> > xml = get_math_blocks(html);
    vector<pair<string, string> >::iterator it;

    vector<string> ret;

    for(it = xml.begin() ; it != xml.end() ;++it) {
        string str;
        if (is_good_xml( it->first )) {
            if (it->second != "") str = "<mws:expr url=\"" + url + "#" + (it->second) + "\">\n" + (it->first) + "\n</mws:expr>\n";
            else str = "<mws:expr url=\"" + url + "\">\n" + (it->first) + "\n</mws:expr>\n";
            ret.push_back(str);
        }
    }
    return ret;
}

/*
 * Gets mathml tags from MwsMathMLTags.txt
 */
void store_mathml_tags()
{
    char *alltags;
    ifstream is;
    is.open (MATHMLTAGS_PATH, ios::binary);
    if (is.fail())
    {
        PRINT_WARN("Error while opening MathML file from: %s.\n",MATHMLTAGS_PATH);
        exit(1);
    }
    is.seekg (0, ios::end);
    int length = is.tellg();
    alltags = new char[length];
    is.seekg (0, ios::beg);
    is.read (alltags,length);
    is.close();
    string all_tags(alltags);
    delete[] alltags;
    int pos = 0;

    while (1)
    {
        size_t oc = all_tags.find("<", pos);
        if (oc == string::npos) break;
        pos = oc + 1;
        size_t cc = all_tags.find(">", pos);
        if (cc == string::npos) break;
        pos = cc + 1;
        string tag;
        size_t c = oc + 1;
        if (all_tags[c] == '/') ++c;
        while (c < cc && all_tags[c] != ' ' && all_tags[c] != '/')
        {
            tag.push_back(all_tags[c]);
            ++c;
        }

        mathml_tags.insert(tag);
        mathml_tags.insert((tag+"/"));
        mathml_tags.insert(("/"+tag));
    }

    //for (set <string>::iterator it = mathml_tags.begin(); it != mathml_tags.end(); ++it) cout << (*it) << endl;
}

// ============== XHTML RELATED ===================


/*
 * Implementation of isXHTML
 * returns 1 if the header says that the page is XHTML
 */
int isXHTML(Page& page) { 
    // Search for "Content-Type: application/xhtml+xml" in the header file
    string ctype = "Content-Type: application/xhtml+xml"; // FIXME spaces can easily break this
    return (page.getHeader().find(ctype) != string::npos);
}

/**
 * function get_math_xhtml(xhtml)
 *
 * return the xml math tags from memory written in 'xhtml'
 */
std::vector< std::string > get_math_xhtml(const string& xhtml, const string& url) {
    const xmlChar *xpath = (const xmlChar*)
            "//m:math/m:semantics/m:annotation-xml[@encoding='MathML-Content']/*";

    vector<string> harvest_exprs;

    xmlDocPtr doc = get_XMLDoc(xhtml.c_str());

    xmlXPathObjectPtr result = get_XMLNodeset(doc, xpath);
    if (result != NULL) {
        xmlNodeSetPtr nodeset = result->nodesetval;

        for (int i = 0; i < nodeset->nodeNr; ++i) {
            char *buf;
            size_t sz;
            FILE *stream = open_memstream(&buf, &sz);

            xmlChar *id = xmlGetProp(nodeset->nodeTab[i]->parent->parent->parent, (xmlChar*) "id");
            
            fprintf(stream, "<mws:expr url=\"%s#%s\">\n", url.c_str() , id);
            xmlElemDump(stream, doc, nodeset->nodeTab[i]);
            fprintf(stream, "</mws:expr>\n");
            fclose(stream);

            string str = (string)buf;
            if (is_good_xml(str)) {
                harvest_exprs.push_back(str);
            }

            xmlFree(id);
            free(buf);
        }

        xmlXPathFreeObject (result);
    }

    xmlFreeDoc(doc);

    //xmlCleanupParser(); should be called only at the end

    return harvest_exprs;
}


// =============== GENERAL PURPOSE ===============

vector< string > get_math(Page& page) {

    cout << "This page is: "  ;
    if (isXHTML(page))
        cout << "XHTML" << endl;
    else if (isHTML(page))
        cout << "HTML" << endl;

    if (isXHTML(page))
        return get_math_xhtml(page.getContent(), page.getUrl());
    else
        if (isHTML(page))
            return get_math_html(page);
        else {
            cout << ">>> Skipping link (not XHTML nor HTML).\n";
            vector<string> emptyVector;
            return emptyVector;
        }
}

/**
 * Implementation of is_good_xml()
 */
int is_good_xml(string xml) {

    string xmlfull = "<?xml version=\"1.0\" ?> <mws:harvest xmlns:m=\"http://www.w3.org/1998/Math/MathML\" xmlns:mws=\"http://search.mathweb.org/ns\">" + xml + "</mws:harvest>";

    int size = xmlfull.size();
    xmlDocPtr doc = xmlParseMemory(xmlfull.c_str(), size);

    if (doc == NULL) {
        return 0;
    }

    xmlFreeDoc(doc);

    return 1;
}

} // namespace mws
