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
#include <libxml/HTMLparser.h>
#include <string.h>

#include <string>
using std::string;
#include <vector>
using std::vector;
#include <stdexcept>
using std::runtime_error;

#include "common/utils/memstream.h"
#include "common/utils/compiler_defs.h"

#include "MathParser.hpp"

namespace crawler {
namespace parser {

/**
 * @brief parse HTML or XML document
 * @param url URL of the document
 * @param buffer document data
 * @param size document size
 * @return document tree on success, NULL on failure
 */
static xmlDocPtr parseDoc(const char* url, const char *buffer, int size);
static xmlXPathObjectPtr getXMLNodeset(xmlDocPtr doc, const xmlChar *xpath);
static void cleanContentMath(xmlNode* xmlNode);
static void sanitizeMathML(xmlDoc* doc, xmlNode* mathMLNode);
static void renameXmlIdAttributes(xmlNode* xmlNode);
static string xmlNodeToString(xmlDocPtr doc, xmlNode* xmlNode);
static xmlNode* getMathNodeFromContentMathNode(xmlNode* contentMathNode);

const xmlChar XPATH_CONTENT_MATH[] =
        "//*[local-name()='math']/*[local-name()='semantics']"
        "/*[local-name()='annotation-xml' and @encoding='MathML-Content']/*";

/*--------------------------------------------------------------------------*/
/* Implementation                                                           */
/*--------------------------------------------------------------------------*/

vector<string> getHarvestFromDocument(const string& xhtml,
                                      const string& url,
                                      const int data_id) throw (runtime_error) {
    vector<string> harvestExpressions;

    xmlDocPtr doc = parseDoc(url.c_str(), xhtml.c_str(), xhtml.size());
    if (doc != NULL) {
        xmlXPathObjectPtr result = getXMLNodeset(doc, XPATH_CONTENT_MATH);
        if (result != NULL) {
            xmlNodeSetPtr nodeSet = result->nodesetval;
            if (!xmlXPathNodeSetIsEmpty(nodeSet)) {
                // Save escaped document as data
                xmlChar* encodedData =
                        xmlEncodeSpecialChars(doc, BAD_CAST xhtml.c_str());
                string dataExpression = "<mws:data mws:data_id=\"" +
                        std::to_string(data_id) + "\">\n" +
                        (string) ((char*) encodedData) +
                        "</mws:data>\n";
                xmlFree(encodedData);
                harvestExpressions.push_back(dataExpression);


                for (int i = 0; i < nodeSet->nodeNr; ++i) {
                    xmlNode* cmmlNode = nodeSet->nodeTab[i];
                    xmlNode* math = getMathNodeFromContentMathNode(cmmlNode);
                    const xmlChar *id = xmlGetProp(math, BAD_CAST "id");
                    string expressionUrl = "#" +
                            ((id != NULL) ? string((char*)id) : "");

                    sanitizeMathML(doc, cmmlNode);
                    cleanContentMath(cmmlNode);

                    const string harvestExpression =
                            "<mws:expr url=\"" + expressionUrl +
                            "\" mws:data_id=\"" + std::to_string(data_id) +
                            "\">\n" + xmlNodeToString(doc, cmmlNode) +
                            "</mws:expr>\n";
                    harvestExpressions.push_back(harvestExpression);

                    xmlFree((void*) id);
                }
            }
            xmlXPathFreeObject(result);
        }
        xmlFreeDoc(doc);
    } else {
        throw runtime_error(url + ": XML/HTML parse error");
    }

    // xmlCleanupParser(); should be called only at the end

    return harvestExpressions;
}

/*--------------------------------------------------------------------------*/
/* Local implementation                                                     */
/*--------------------------------------------------------------------------*/

static
xmlDocPtr parseDoc(const char* url, const char *buffer, int size) {
    xmlDocPtr doc;

    // Try as XHTML
    doc = xmlReadMemory(buffer, size, url,
            /* encoding = */ NULL,
            XML_PARSE_NOWARNING | XML_PARSE_NOERROR);
    if (doc != NULL) {
        return doc;
    }

    // Try as HTML
    doc = htmlCtxtReadMemory(htmlNewParserCtxt(), buffer, size, url,
            /* encoding = */ NULL,
            HTML_PARSE_RECOVER | HTML_PARSE_NOWARNING | HTML_PARSE_NOERROR);
    if (doc != NULL) {
        return doc;
    }

    return NULL;
}

static
xmlXPathObjectPtr getXMLNodeset(xmlDocPtr doc, const xmlChar *xpath) {
    xmlXPathContextPtr context = xmlXPathNewContext(doc);
    if (context == NULL) {
        PRINT_WARN("xmlXPathNewContext failed");
        return NULL;
    }

    xmlXPathObjectPtr result = xmlXPathEvalExpression(xpath, context);
    xmlXPathFreeContext(context);
    if (result == NULL) {
        PRINT_WARN("xmlXPathEvalExpression failed");
        return NULL;
    }

    return result;
}

static
void sanitizeMathML(xmlDoc* doc, xmlNode* mathMLNode) {
    UNUSED(doc);
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

/**
  * @brief get the content of an xmlNode from document doc
  * @param doc
  * @param xmlNode
  */
static
string xmlNodeToString(xmlDocPtr doc, xmlNode *xmlNode) {
    char* buf;
    size_t size;
    FILE *stream = open_memstream(&buf, &size);
    xmlElemDump(stream, doc, xmlNode);
    fclose(stream);

    string content = (string) buf;
    free(buf);

    return content;
}

static
xmlNode* getMathNodeFromContentMathNode(xmlNode* contentMathNode) {
    return contentMathNode->parent->parent->parent;
}

}  // namespace parser
}  // namespace crawler


