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
 * @file MathParser.cpp
 * @todo error handling
 * @date 15 Aug 2012
 */

#include <string.h>
#include <libxml/parser.h>
#include <libxml/HTMLparser.h>
#include <libxml/xpath.h>
#include <libxml/xpathInternals.h>

#include "crawler/utils/memstream.h"

#include "MathParser.hpp"

using namespace std;

namespace crawler {
namespace parser {

static xmlDocPtr get_XMLDoc (const char *buffer);
static void renameXmlIdAttributes(xmlNode* xmlNode);
static void cleanContentMath(xmlNode* xmlNode);
static xmlXPathObjectPtr get_XMLNodeset (xmlDocPtr doc, const xmlChar *xpath);
static bool isValidXml(const string& xml);

/*--------------------------------------------------------------------------*/
/* Implementation                                                           */
/*--------------------------------------------------------------------------*/

vector<std::string> getHarvestFromXhtml(const string& xhtml,
                                        const string& url) {
    const xmlChar *XPATH_CONTENT_MATH = (const xmlChar*)
            "//m:math/m:semantics"
            "/m:annotation-xml[@encoding='MathML-Content']/*";

    vector<string> harvestExpressions;

    xmlDocPtr doc = get_XMLDoc(xhtml.c_str());
    if (doc != NULL) {
        xmlXPathObjectPtr result = get_XMLNodeset(doc, XPATH_CONTENT_MATH);
        if (result != NULL) {
            xmlNodeSetPtr nodeset = result->nodesetval;

            for (int i = 0; i < nodeset->nodeNr; ++i) {
                xmlNode* contentMathNode = nodeset->nodeTab[i];
                xmlNode* mathNode = contentMathNode->parent->parent->parent;
                char* buf;
                size_t sz;
                FILE *stream = open_memstream(&buf, &sz);
                const xmlChar *id = xmlGetProp(mathNode, BAD_CAST "id");

                fprintf(stream, "<mws:expr url=\"%s#%s\">\n", url.c_str() , id);
                // Rename xml:id to local_id to avoid constraints of uniqueness
                renameXmlIdAttributes(mathNode);
                fprintf(stream, "<data>\n");
                xmlElemDump(stream, doc, mathNode);
                fprintf(stream, "</data>\n");

                // Remove redundant attributes (local_id, xref)
                cleanContentMath(contentMathNode);
                fprintf(stream, "<content>\n");
                xmlElemDump(stream, doc, contentMathNode);
                fprintf(stream, "</content>\n");
                fprintf(stream, "</mws:expr>\n");
                fclose(stream);

                string harvestExpression = (string) buf;
                if (isValidXml(harvestExpression)) {
                    harvestExpressions.push_back(harvestExpression);
                }

                xmlFree((void*) id);
                free(buf);
            }

            xmlXPathFreeObject (result);
        }

        xmlFreeDoc(doc);
    }
    //xmlCleanupParser(); should be called only at the end

    return harvestExpressions;
}

/*--------------------------------------------------------------------------*/
/* Local implementation                                                     */
/*--------------------------------------------------------------------------*/

static
xmlDocPtr get_XMLDoc (const char *buffer) {
    int size = strlen(buffer);
    xmlDocPtr doc;
    doc = xmlParseMemory(buffer,size);

    if (doc == NULL ) {
        fprintf(stderr,"Document not parsed successfully. \n");
        return NULL;
    }

    return doc;
}

static
xmlXPathObjectPtr get_XMLNodeset (xmlDocPtr doc, const xmlChar *xpath) {
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
        fprintf(stderr,"Error: unable to register NS with prefix=\"%s\" "
                "and href=\"%s\"\n", prefix, href);
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
        return NULL;
    }

    return result;
}

static
bool isValidXml(const string& xml) {
    string xmlfull = "<?xml version=\"1.0\" ?> "
            "<mws:harvest "
            "xmlns:m=\"http://www.w3.org/1998/Math/MathML\" "
            "xmlns:mws=\"http://search.mathweb.org/ns\">" + xml +
            "</mws:harvest>";

    int size = xmlfull.size();
    xmlDocPtr doc = xmlParseMemory(xmlfull.c_str(), size);

    if (doc == NULL) {
        return false;
    }

    xmlFreeDoc(doc);

    return true;
}

static
void renameXmlIdAttributes(xmlNode* xmlNode) {
    // Create attribute local_id
    const xmlChar* id = xmlGetProp(xmlNode, BAD_CAST "id");
    xmlSetProp(xmlNode, BAD_CAST "local_id", id);

    // Remove id attribute
    xmlAttrPtr attrPtr = xmlNode->properties;
    while (attrPtr != NULL) {
        if (strcmp((char*) attrPtr->name, "id") == 0) {
            xmlRemoveProp(attrPtr);
            break;
        }

        attrPtr = attrPtr->next;
    }

    // Recurse through child nodes
    xmlNodePtr curr = xmlNode->children;
    while (curr != NULL) {
        renameXmlIdAttributes(curr);
        curr = curr->next;
    }
}

/**
 * @brief remove redundant attributes from ContentMath
 * @param xmlNode
 */
static
void cleanContentMath(xmlNode* xmlNode) {
    // Remove id attribute
    xmlAttrPtr attrPtr = xmlNode->properties;
    while (attrPtr != NULL) {
        if (strcmp((char*) attrPtr->name, "local_id") == 0 ||
                strcmp((char*) attrPtr->name, "xref") == 0) {
            xmlRemoveProp(attrPtr);
        }

        attrPtr = attrPtr->next;
    }

    // Recurse through child nodes
    xmlNodePtr curr = xmlNode->children;
    while (curr != NULL) {
        cleanContentMath(curr);
        curr = curr->next;
    }
}

}  // namespace parser
}  // namespace crawler


