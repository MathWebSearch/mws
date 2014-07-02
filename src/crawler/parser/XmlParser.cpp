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
#include <stdexcept>
using std::runtime_error;
#include <string>
using std::string;
using std::to_string;
#include <sstream>
using std::stringstream;

#include "common/utils/memstream.h"
#include "common/utils/util.hpp"
using common::utils::removeDuplicateSpaces;
#include "crawler/parser/XmlParser.hpp"

namespace crawler {
namespace parser {

/*--------------------------------------------------------------------------*/
/* Local methods                                                            */
/*--------------------------------------------------------------------------*/

static xmlXPathObjectPtr getXMLNodeset(xmlDocPtr doc, const char* xpath,
                                       xmlNode* contextNode = nullptr);
static void dumpNodeToText(const xmlNode* node, stringstream* stream,
                           MathIdDictionary* encodedMathIds);

/*--------------------------------------------------------------------------*/
/* Implementation                                                           */
/*--------------------------------------------------------------------------*/

xmlDocPtr parseDocument(const char* content, int size) {
    xmlDocPtr doc;

    // Try as XHTML
    doc = xmlReadMemory(content, size, /* URL = */ "", /* encoding = */ nullptr,
                        XML_PARSE_NOWARNING | XML_PARSE_NOERROR);
    if (doc != nullptr) {
        return doc;
    }

    // Try as HTML
    htmlParserCtxtPtr htmlParserCtxt = htmlNewParserCtxt();
    doc = htmlCtxtReadMemory(
        htmlParserCtxt, content, size, /* URL = */ "",
        /* encoding = */ nullptr,
        HTML_PARSE_RECOVER | HTML_PARSE_NOWARNING | HTML_PARSE_NOERROR);
    htmlFreeParserCtxt(htmlParserCtxt);
    if (doc != nullptr) {
        return doc;
    }

    throw runtime_error("XML/HTML parse error");
}

string escapeXml(xmlDoc* doc, const string& xml) {
    xmlChar* escapedXml = xmlEncodeSpecialChars(doc, BAD_CAST xml.c_str());
    string escapedXmlString(reinterpret_cast<char*>(escapedXml));
    xmlFree(escapedXml);
    return escapedXmlString;
}

string getTextFromNode(xmlNode* node, MathIdDictionary* encodedMathIds) {
    stringstream stream;
    dumpNodeToText(node, &stream, encodedMathIds);
    string text = stream.str();
    removeDuplicateSpaces(&text);

    return text;
}

string getXmlFromNode(xmlDoc* doc, xmlNode* node) {
    char* buf;
    size_t size;
    FILE* fileStream = open_memstream(&buf, &size);
    xmlOutputBufferPtr outputBuffer =
        xmlOutputBufferCreateFile(fileStream,
                                  /* charEncodingHandler = */ nullptr);
    xmlNodeDumpOutput(outputBuffer, doc, node, /* level = */ 0,
                      /* format = */ 0, /* encoding = */ nullptr);
    xmlOutputBufferClose(outputBuffer);
    fclose(fileStream);

    string content = (string)buf;
    free(buf);

    return content;
}

string getEscapedXmlFromNode(xmlDoc* doc, xmlNode* node) {
    string xml = getXmlFromNode(doc, node);
    xmlChar* escapedXml = xmlEncodeSpecialChars(doc, BAD_CAST xml.c_str());
    string escapedXmlString((char*)escapedXml);
    xmlFree(escapedXml);

    return escapedXmlString;
}

xmlNode* getNodeByXpath(xmlDocPtr doc, const string& xpath,
                        xmlNode* contextNode) {
    xmlXPathObjectPtr result = getXMLNodeset(doc, xpath.c_str(), contextNode);
    if (result == nullptr) {
        return nullptr;
    }
    xmlNodeSetPtr nodeSet = result->nodesetval;
    if (xmlXPathNodeSetIsEmpty(nodeSet)) {
        xmlXPathFreeObject(result);
        return nullptr;
    }
    xmlNode* node = nodeSet->nodeTab[0];

    xmlXPathFreeObject(result);

    return node;
}

string getTextByXpath(xmlDocPtr doc, const string& xpath,
                      MathIdDictionary* encodedMathIds, xmlNode* contextNode) {
    xmlNode* node = getNodeByXpath(doc, xpath, contextNode);
    if (node == nullptr) {
        return "";
    }

    return getTextFromNode(node, encodedMathIds);
}

void processXpathResults(xmlDoc* doc, const std::string& xpath,
                         XmlNodeProcessor* xmlNodeProcessor) {
    xmlXPathObjectPtr xpathResult = getXMLNodeset(doc, xpath.c_str());
    if (xpathResult != nullptr) {
        xmlNodeSetPtr nodeSet = xpathResult->nodesetval;
        if (!xmlXPathNodeSetIsEmpty(nodeSet)) {
            xmlNodeProcessor->foundResults(nodeSet->nodeNr);
            for (int i = 0; i < nodeSet->nodeNr; ++i) {
                assert(nodeSet->nodeTab[i] != nullptr);
                xmlNodeProcessor->processXmlNode(nodeSet->nodeTab[i]);
            }
        }
        xmlXPathFreeObject(xpathResult);
    }
}

void processXpathResults(xmlDoc* doc, const string& xpath,
                         ProcessXmlNodeCallback processXmlNodeCallback) {
    class CallbackProcessor : public XmlNodeProcessor {
     public:
        explicit CallbackProcessor(ProcessXmlNodeCallback callback)
            : mCallback(std::move(callback)) {}
        virtual void processXmlNode(xmlNode* cmmlNode) { mCallback(cmmlNode); }

     private:
        ProcessXmlNodeCallback mCallback;
    };

    CallbackProcessor callbackProcessor(processXmlNodeCallback);
    processXpathResults(doc, xpath, &callbackProcessor);
}

/*--------------------------------------------------------------------------*/
/* Local implementation                                                     */
/*--------------------------------------------------------------------------*/

static xmlXPathObjectPtr getXMLNodeset(xmlDocPtr doc, const char* xpath,
                                       xmlNode* contextNode) {
    xmlXPathContext* context = xmlXPathNewContext(doc);
    if (context == nullptr) {
        PRINT_WARN("xmlXPathNewContext failed");
        return nullptr;
    }
    // calling xmlXPathSetContextNode() has the same effect, but it is present
    // only in newer versions of LibXml2
    context->node = contextNode;

    xmlXPathObjectPtr result = xmlXPathEval(BAD_CAST xpath, context);
    xmlXPathFreeContext(context);
    if (result == nullptr) {
        PRINT_WARN("xmlXPathEvalExpression failed for \"%s\"\n", xpath);
        return nullptr;
    }

    return result;
}

static void dumpNodeToText(const xmlNode* root, stringstream* stream,
                           MathIdDictionary* encodedMathIds) {
    stack<const xmlNode*> nodes;
    nodes.push(root->children);
    static const char MATH_PREFIX[] = "math";

    while (!nodes.empty()) {
        const xmlNode* node = nodes.top();
        if (node == nullptr) {
            nodes.pop();
            continue;
        }
        nodes.top() = node->next;

        if (strcmp((char*)node->name, "math") == 0) {
            const char* id = reinterpret_cast<const char*>(
                xmlGetProp(const_cast<xmlNode*>(node), BAD_CAST "id"));
            if (encodedMathIds != nullptr) {
                *stream << " " << MATH_PREFIX
                        << to_string(encodedMathIds->put(id)) << " ";
            } else {
                *stream << " " << MATH_PREFIX << id << " ";
            }
            xmlFree((void*)id);
            continue;
        } else if (node->type == XML_TEXT_NODE) {
            *stream << " " << reinterpret_cast<char*>(node->content) << " ";
        }

        nodes.push(node->children);
    }
}

}  // namespace parser
}  // namespace crawler
