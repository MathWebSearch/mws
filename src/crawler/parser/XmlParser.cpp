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
 * @brief XmlParser Utils Implementation
 * @file XmlParser.cpp
 * @date 07 May 2014
 */

#include <assert.h>
#include <string.h>
#include <libxml/parser.h>
#include <libxml/HTMLparser.h>
#include <libxml/tree.h>
#include <libxml/xpath.h>

#include <stack>
using std::stack;
#include <string>
using std::string;
#include <sstream>
using std::stringstream;
#include <stdexcept>
using std::runtime_error;

#include "common/utils/memstream.h"
#include "common/utils/util.hpp"
using common::utils::removeDuplicateSpaces;
#include "crawler/parser/XmlParser.hpp"


/*--------------------------------------------------------------------------*/
/* Local methods                                                            */
/*--------------------------------------------------------------------------*/

static xmlXPathObjectPtr getXMLNodeset(xmlDocPtr doc,
                                       const char* xpath,
                                       xmlNode* contextNode = NULL);
static void dumpNodeToText(const xmlNode* node,
                           stringstream* stream);

/*--------------------------------------------------------------------------*/
/* Implementation                                                           */
/*--------------------------------------------------------------------------*/

namespace crawler {
namespace parser {

xmlDocPtr parseDocument(const char *content, int size) {
    xmlDocPtr doc;

    // Try as XHTML
    doc = xmlReadMemory(content, size, /* URL = */ "", /* encoding = */ NULL,
                        XML_PARSE_NOWARNING | XML_PARSE_NOERROR);
    if (doc != NULL) {
        return doc;
    }

    // Try as HTML
    htmlParserCtxtPtr htmlParserCtxt = htmlNewParserCtxt();
    doc = htmlCtxtReadMemory(htmlParserCtxt, content, size, /* URL = */ "",
                             /* encoding = */ NULL, HTML_PARSE_RECOVER |
                             HTML_PARSE_NOWARNING | HTML_PARSE_NOERROR);
    htmlFreeParserCtxt(htmlParserCtxt);
    if (doc != NULL) {
        return doc;
    }

    throw runtime_error("XML/HTML parse error");
}

string getEscapedTextFromNode(xmlDoc* doc, xmlNode* node) {
    stringstream stream;
    dumpNodeToText(node, &stream);
    string text = stream.str();
    removeDuplicateSpaces(&text);
    xmlChar* escapedText = xmlEncodeSpecialChars(doc, BAD_CAST text.c_str());
    string escapedTextString((char*) escapedText);
    xmlFree(escapedText);

    return escapedTextString;
}

string getXmlFromNode(xmlDoc* doc, xmlNode* node) {
    char* buf;
    size_t size;
    FILE *fileStream = open_memstream(&buf, &size);
    xmlOutputBufferPtr outputBuffer =
            xmlOutputBufferCreateFile(fileStream,
                                      /* charEncodingHandler = */ NULL);
    xmlNodeDumpOutput(outputBuffer, doc, node, /* level = */ 0,
                      /* format = */ 0, /* encoding = */ NULL);
    xmlOutputBufferClose(outputBuffer);
    fclose(fileStream);

    string content = (string) buf;
    free(buf);

    return content;
}

string getEscapedXmlFromNode(xmlDoc* doc, xmlNode* node) {
    string xml = getXmlFromNode(doc, node);
    xmlChar* escapedXml = xmlEncodeSpecialChars(doc, BAD_CAST xml.c_str());
    string escapedXmlString((char*) escapedXml);
    xmlFree(escapedXml);

    return escapedXmlString;
}

xmlNode* getNodeByXpath(xmlDocPtr doc,
                        const string& xpath,
                        xmlNode* contextNode) {
    xmlXPathObjectPtr result = getXMLNodeset(doc, xpath.c_str(), contextNode);
    if (result == NULL) {
        return NULL;
    }
    xmlNodeSetPtr nodeSet = result->nodesetval;
    if (xmlXPathNodeSetIsEmpty(nodeSet)) {
        xmlXPathFreeObject(result);
        return NULL;
    }
    xmlNode* node = nodeSet->nodeTab[0];

    xmlXPathFreeObject(result);

    return node;
}

string getTextByXpath(xmlDocPtr doc,
                      const string& xpath,
                      xmlNode* contextNode) {
    xmlNode* node = getNodeByXpath(doc, xpath, contextNode);
    if (node == NULL) {
        return "";
    }

    return getEscapedTextFromNode(doc, node);
}

void processXpathResults(xmlDoc* doc,
                         const std::string& xpath,
                         XmlNodeProcessor* xmlNodeProcessor) {
    xmlXPathObjectPtr xpathResult = getXMLNodeset(doc, xpath.c_str());
    if (xpathResult != NULL) {
        xmlNodeSetPtr nodeSet = xpathResult->nodesetval;
        if (!xmlXPathNodeSetIsEmpty(nodeSet)) {
            xmlNodeProcessor->foundResults(nodeSet->nodeNr);
            for (int i = 0; i < nodeSet->nodeNr; ++i) {
                assert(nodeSet->nodeTab[i] != NULL);
                xmlNodeProcessor->processXmlNode(nodeSet->nodeTab[i]);
            }
        }
        xmlXPathFreeObject(xpathResult);
    }
}

void processXpathResults(xmlDoc *doc,
                         const string &xpath,
                         ProcessXmlNodeCallback processXmlNodeCallback) {
    class CallbackProcessor : public XmlNodeProcessor {
     public:
        explicit CallbackProcessor(const ProcessXmlNodeCallback& callback)
            : mCallback(callback) {}
        virtual void processXmlNode(xmlNode* cmmlNode) {
            mCallback(cmmlNode);
        }
     private:
        ProcessXmlNodeCallback mCallback;
    };

    CallbackProcessor callbackProcessor(processXmlNodeCallback);
    processXpathResults(doc, xpath, &callbackProcessor);
}

}  // namespace parser
}  // namespace crawler

/*--------------------------------------------------------------------------*/
/* Local implementation                                                     */
/*--------------------------------------------------------------------------*/

static
xmlXPathObjectPtr getXMLNodeset(xmlDocPtr doc,
                                const char* xpath,
                                xmlNode* contextNode) {
    xmlXPathContext* context = xmlXPathNewContext(doc);
    if (context == NULL) {
        PRINT_WARN("xmlXPathNewContext failed");
        return NULL;
    }
    // calling xmlXPathSetContextNode() has the same effect, but it is present
    // only in newer versions of LibXml2
    context->node = contextNode;

    xmlXPathObjectPtr result = xmlXPathEval(BAD_CAST xpath, context);
    xmlXPathFreeContext(context);
    if (result == NULL) {
        PRINT_WARN("xmlXPathEvalExpression failed for \"%s\"\n", xpath);
        return NULL;
    }

    return result;
}

static
void dumpNodeToText(const xmlNode* root, stringstream* stream) {
    stack<const xmlNode*> nodes;
    nodes.push(root->children);

    while (!nodes.empty()) {
        const xmlNode* node = nodes.top();
        if (node == NULL) {
            nodes.pop();
            continue;
        }
        nodes.top() = node->next;

        if (strcmp((char*) node->name, "math") == 0) {
            const xmlChar *id = xmlGetProp(const_cast<xmlNode*>(node),
                                           BAD_CAST "id");
            *stream << " #" << reinterpret_cast<const char*>(id) << " ";
            xmlFree((void*) id);
            continue;
        } else if (node->type == XML_TEXT_NODE) {
            *stream << " " << reinterpret_cast<char*>(node->content) << " ";
        }

        nodes.push(node->children);
    }
}
