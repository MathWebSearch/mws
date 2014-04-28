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
static xmlDocPtr parseDoc(const char *buffer, int size);
static string getDocumentUri(xmlDocPtr doc, string document_uri_xpath);
static xmlXPathObjectPtr getXMLNodeset(xmlDocPtr doc, const xmlChar *xpath);
static void cleanContentMath(xmlNode* node);
static void renameXmlIdAttributes(xmlNode* node);
static string xmlNodeToString(xmlDocPtr doc, xmlNode* node);
static xmlNode* getMathNodeFromContentMathNode(xmlNode* content_math_node);

const xmlChar XPATH_CONTENT_MATH[] =
        "//*[local-name()='math']/*[local-name()='semantics']"
        "/*[local-name()='annotation-xml' and @encoding='MathML-Content']/*";

/*--------------------------------------------------------------------------*/
/* Implementation                                                           */
/*--------------------------------------------------------------------------*/

Harvest getHarvestFromDocument(const string& content,
                               const int data_id,
                               const string& document_uri_xpath,
                               bool save_data) throw (runtime_error) {
    Harvest harvest;
    xmlDocPtr doc = parseDoc(content.c_str(), content.size());
    if (doc != NULL) {
        string documentUrl = getDocumentUri(doc, document_uri_xpath);
        xmlXPathObjectPtr result = getXMLNodeset(doc, XPATH_CONTENT_MATH);
        if (result != NULL) {
            xmlNodeSetPtr nodeSet = result->nodesetval;
            if (!xmlXPathNodeSetIsEmpty(nodeSet)) {
                if (save_data) {
                    // Save escaped document as data
                    xmlChar* encodedData =
                            xmlEncodeSpecialChars(doc,
                                                  BAD_CAST content.c_str());
                    harvest.dataElement = "<mws:data mws:data_id=\"" +
                            std::to_string(data_id) + "\">\n" +
                            (string) ((char*) encodedData) +
                            "</mws:data>\n";
                    xmlFree(encodedData);
                }


                for (int i = 0; i < nodeSet->nodeNr; ++i) {
                    xmlNode* cmmlNode = nodeSet->nodeTab[i];
                    xmlNode* math = getMathNodeFromContentMathNode(cmmlNode);
                    const xmlChar *id = xmlGetProp(math, BAD_CAST "id");
                    string expressionUrl = documentUrl + "#" +
                            ((id != NULL) ? string((char*)id) : "");

                    // Rename xml:id attributes to local_id
                    renameXmlIdAttributes(math);
                    cleanContentMath(cmmlNode);

                    string harvestExpression = "<mws:expr url=\"" +
                            expressionUrl + "\" ";
                    if (save_data) {
                        harvestExpression += "mws:data_id=\"" +
                                std::to_string(data_id) +"\"";
                    }
                    harvestExpression += ">\n"
                            + xmlNodeToString(doc, cmmlNode) + "</mws:expr>\n";
                    harvest.contentMathElements.push_back(harvestExpression);

                    xmlFree((void*) id);
                }
            }
            xmlXPathFreeObject(result);
        }
        xmlFreeDoc(doc);
    } else {
        throw runtime_error("XML/HTML parse error");
    }

    return harvest;
}

void cleanupMathParser() {
    xmlCleanupParser();
}

/*--------------------------------------------------------------------------*/
/* Local implementation                                                     */
/*--------------------------------------------------------------------------*/

static
xmlDocPtr parseDoc(const char *buffer, int size) {
    xmlDocPtr doc;

    // Try as XHTML
    doc = xmlReadMemory(buffer, size, "",
                        /* encoding = */ NULL,
                        XML_PARSE_NOWARNING | XML_PARSE_NOERROR);
    if (doc != NULL) {
        return doc;
    }

    // Try as HTML
    htmlParserCtxtPtr htmlPtr = htmlNewParserCtxt();
    doc = htmlCtxtReadMemory(htmlPtr, buffer, size, "",
                             /* encoding = */ NULL, HTML_PARSE_RECOVER |
                             HTML_PARSE_NOWARNING | HTML_PARSE_NOERROR);
    htmlFreeParserCtxt(htmlPtr);
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
void renameXmlIdAttributes(xmlNode* node) {
    // Rename attribute id to local_id
    xmlChar* id = xmlGetProp(node, BAD_CAST "id");
    xmlSetProp(node, BAD_CAST "local_id", id);
    xmlUnsetProp(node, BAD_CAST "id");
    xmlFree(id);

    // Recurse through child nodes
    xmlNodePtr curr = node->children;
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
void cleanContentMath(xmlNode* node) {
    // Remove id and xref attributes
    xmlUnsetProp(node, BAD_CAST "local_id");
    xmlUnsetProp(node, BAD_CAST "xref");

    // Remove namespace from name
    xmlSetNs(node, NULL);

    // Recurse through child nodes
    xmlNodePtr curr = node->children;
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
string xmlNodeToString(xmlDocPtr doc, xmlNode *node) {
    char* buf;
    size_t size;
    FILE *stream = open_memstream(&buf, &size);
    xmlElemDump(stream, doc, node);
    fclose(stream);

    string content = (string) buf;
    free(buf);

    return content;
}

static
xmlNode* getMathNodeFromContentMathNode(xmlNode* content_math_node) {
    // See XPATH_CONTENT_MATH
    return content_math_node->parent->parent->parent;
}

static string getDocumentUri(xmlDocPtr doc, string document_uri_xpath) {
    xmlXPathObjectPtr result =
            getXMLNodeset(doc, BAD_CAST document_uri_xpath.c_str());
    if (result == NULL) {
        return "";
    }
    xmlNodeSetPtr nodeSet = result->nodesetval;
    if (xmlXPathNodeSetIsEmpty(nodeSet)) {
        xmlXPathFreeObject(result);
        return "";
    }
    xmlNode* node = nodeSet->nodeTab[0];
    xmlChar* content = xmlNodeGetContent(node);
    xmlXPathFreeObject(result);
    string ret = string((char*) content);
    xmlFree(content);
    return ret;
}

}  // namespace parser
}  // namespace crawler


