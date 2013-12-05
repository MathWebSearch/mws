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

#include <libxml/parser.h>
#include <libxml/tree.h>
#include <libxml/xpath.h>
#include <libxml/xpathInternals.h>
#include <string.h>

#include <string>
using std::string;
#include <vector>
using std::vector;

#include "crawler/utils/memstream.h"

#include "MathParser.hpp"

namespace crawler {
namespace parser {

static xmlDocPtr getXMLDoc(const char *buffer);
static xmlXPathObjectPtr getXMLNodeset(xmlDocPtr doc, const xmlChar *xpath);
static bool isValidXml(const string& xml);
static void cleanContentMath(xmlNode* xmlNode);
static void sanitizeMathML(xmlDoc* doc, xmlNode* mathMLNode);
static void renameXmlIdAttributes(xmlNode* xmlNode);

const xmlChar MATHML_NAMESPACE[] = "http://www.w3.org/1998/Math/MathML";
const xmlChar XPATH_CONTENT_MATH[] = "//m:math/m:semantics"
        "/m:annotation-xml[@encoding='MathML-Content']/*";


/*--------------------------------------------------------------------------*/
/* Implementation                                                           */
/*--------------------------------------------------------------------------*/

vector<std::string> getHarvestFromXhtml(const string& xhtml,
                                        const string& url) {
    vector<string> harvestExpressions;

    xmlDocPtr doc = getXMLDoc(xhtml.c_str());
    if (doc != NULL) {
        xmlXPathObjectPtr result = getXMLNodeset(doc, XPATH_CONTENT_MATH);
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
                sanitizeMathML(doc, mathNode);
                fprintf(stream, "<data>\n");
                xmlElemDump(stream, doc, mathNode);
                fprintf(stream, "\n</data>\n");

                // Remove redundant attributes (local_id, xref)
                cleanContentMath(contentMathNode);
                fprintf(stream, "<content>\n");
                xmlElemDump(stream, doc, contentMathNode);
                fprintf(stream, "\n</content>\n");
                fprintf(stream, "</mws:expr>\n");
                fclose(stream);

                string harvestExpression = (string) buf;
                if (isValidXml(harvestExpression)) {
                    harvestExpressions.push_back(harvestExpression);
                }

                xmlFree((void*) id);
                free(buf);
            }
            xmlXPathFreeObject(result);
        }
        xmlFreeDoc(doc);
    }

    // xmlCleanupParser(); should be called only at the end

    return harvestExpressions;
}

/*--------------------------------------------------------------------------*/
/* Local implementation                                                     */
/*--------------------------------------------------------------------------*/

static
xmlDocPtr getXMLDoc(const char *buffer) {
    int size = strlen(buffer);
    xmlDocPtr doc;
    doc = xmlParseMemory(buffer, size);

    if (doc == NULL) {
        fprintf(stderr, "Document not parsed successfully. \n");
        return NULL;
    }

    return doc;
}

static
xmlXPathObjectPtr getXMLNodeset(xmlDocPtr doc, const xmlChar *xpath) {
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
    if (xmlXPathRegisterNs(context, prefix, href) != 0) {
        fprintf(stderr, "Error: unable to register NS with prefix=\"%s\" "
                "and href=\"%s\"\n", prefix, href);
        return NULL;
    }

    result = xmlXPathEvalExpression(xpath, context);
    xmlXPathFreeContext(context);
    if (result == NULL) {
        printf("Error in xmlXPathEvalExpression\n");
        return NULL;
    }
    if (xmlXPathNodeSetIsEmpty(result->nodesetval)) {
        xmlXPathFreeObject(result);
        return NULL;
    }

    return result;
}

static
bool isValidXml(const string& xml) {
    string xmlfull = "<?xml version=\"1.0\" ?> "
            "<mws:harvest "
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
void sanitizeMathML(xmlDoc* doc, xmlNode* mathMLNode) {
    // Add mathml namespace declaration
    const xmlNs* mathNs = xmlSearchNs(doc, mathMLNode, BAD_CAST "m");
    if (mathNs == NULL) {
        xmlNewProp(mathMLNode, BAD_CAST "xmlns:m", MATHML_NAMESPACE);
    }
    // Rename xml:id attributes to local_id
    renameXmlIdAttributes(mathMLNode);
}

static
void renameXmlIdAttributes(xmlNode* xmlNode) {
    // Rename attribute id to local_id
    const xmlChar* id = xmlGetProp(xmlNode, BAD_CAST "id");
    xmlSetProp(xmlNode, BAD_CAST "local_id", id);
    xmlUnsetProp(xmlNode, BAD_CAST "id");

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
    // Remove id and xref attributes
    xmlUnsetProp(xmlNode, BAD_CAST "local_id");
    xmlUnsetProp(xmlNode, BAD_CAST "xref");

    // Remove namespace from name
    xmlSetNs(xmlNode, NULL);

    // Recurse through child nodes
    xmlNodePtr curr = xmlNode->children;
    while (curr != NULL) {
        cleanContentMath(curr);
        curr = curr->next;
    }
}

}  // namespace parser
}  // namespace crawler


