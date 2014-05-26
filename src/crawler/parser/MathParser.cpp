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
#include <string.h>
#include <assert.h>

#include <algorithm>
using std::unique;
using std::replace_if;
#include <string>
using std::string;
using std::to_string;
#include <sstream>
using std::stringstream;
#include <vector>
using std::vector;
#include <stdexcept>
using std::runtime_error;
#include <functional>
using std::function;

#include "common/utils/memstream.h"
#include "common/utils/compiler_defs.h"
#include "common/utils/util.hpp"
using common::utils::removeDuplicateSpaces;

#include "crawler/parser/XmlParser.hpp"
#include "crawler/parser/MathParser.hpp"

namespace crawler {
namespace parser {

static void cleanContentMath(xmlNode* node);

const char XPATH_MATHML[] = "//*[local-name()='math']";
const char XPATH_CONTENT_MATH[] = "descendant::*[@encoding='MathML-Content']/*";

const bool DEFAULT_SHOULD_SAVE_DATA = true;
const char DEFAULT_DOCUMENT_URI_XPATH[] =
    "//*[local-name()='span' and @class='number']";
const char DEFAULT_TEXT_WITH_MATH_XPATH[] =
    "//*[local-name()='div' and @class='review-body']";

/*--------------------------------------------------------------------------*/
/* Implementation                                                           */
/*--------------------------------------------------------------------------*/

HarvesterConfiguration::HarvesterConfiguration()
    : shouldSaveData(DEFAULT_SHOULD_SAVE_DATA),
      documentIdXpath(DEFAULT_DOCUMENT_URI_XPATH),
      textWithMathXpath(DEFAULT_TEXT_WITH_MATH_XPATH) {}

string HarvesterConfiguration::toString() const {
    stringstream stream;
    stream << "shouldSaveData   : " << (shouldSaveData ? "true\n" : "false\n");
    stream << "documentIdXpath  : " << documentIdXpath << "\n";
    stream << "textWithMathXpath: " << textWithMathXpath << "\n";
    for (HarvesterConfiguration::MetadataItem item : metadataItems) {
        stream << "metadata." << item.name << " : " << item.xpath << "\n";
    }
    return stream.str();
}

Harvest createHarvestFromDocument(const string& content,
                                  const HarvesterConfiguration& config) {
    Harvest harvest;
    stringstream data;
    xmlDocPtr doc = parseDocument(content.c_str(), content.size());
    const string documentId = getTextByXpath(doc, config.documentIdXpath);

    if (config.shouldSaveData) {
        data << "<mws:data mws:data_id=\"" << config.data_id << "\">\n";
        data << "<id>" << documentId << "</id>\n";
        data << "<text>" << getTextByXpath(doc, config.textWithMathXpath)
             << "</text>\n";
        for (HarvesterConfiguration::MetadataItem item : config.metadataItems) {
            data << "<" << item.name << ">" << getTextByXpath(doc, item.xpath)
                 << "</" << item.name << ">\n";
        }
    }

    processXpathResults(doc, XPATH_MATHML, [&](xmlNode* mathNode) {
        const xmlChar* xmlId = xmlGetProp(mathNode, BAD_CAST "id");
        if (xmlId == nullptr) {
            PRINT_WARN("Discarding math element without identifier\n");
            return;
        }
        string id((char*)xmlId);
        xmlFree((void*)xmlId);

        xmlNode* cmmlNode = getNodeByXpath(doc, XPATH_CONTENT_MATH, mathNode);
        if (cmmlNode == nullptr) {
            PRINT_WARN("Discarding math element #%s without ContentMath\n",
                       id.c_str());
            return;
        }

        if (config.shouldSaveData) {
            data << "<math local_id=\"" << id << "\">"
                 << getEscapedXmlFromNode(doc, mathNode) << "</math>\n";
        }
        cleanContentMath(cmmlNode);

        stringstream expression;
        expression << "<mws:expr url=\"" << documentId << "#" << id << "\"";
        if (config.shouldSaveData) {
            expression << " mws:data_id=\"" << config.data_id << "\"";
        }
        expression << ">" << getXmlFromNode(doc, cmmlNode) << "</mws:expr>\n";
        harvest.contentMathElements.push_back(expression.str());
    });

    if (config.shouldSaveData) {
        data << "</mws:data>\n";
        harvest.dataElement = data.str();
    }

    xmlFreeDoc(doc);

    return harvest;
}

void cleanupMathParser() { xmlCleanupParser(); }

/*--------------------------------------------------------------------------*/
/* Local implementation                                                     */
/*--------------------------------------------------------------------------*/

/**
 * @brief remove redundant attributes from ContentMath
 * @param xmlNode
 */
static void cleanContentMath(xmlNode* node) {
    assert(node != nullptr);
    // Remove id and xref attributes
    xmlUnsetProp(node, BAD_CAST "id");
    xmlUnsetProp(node, BAD_CAST "xref");

    // Remove namespace from name
    xmlSetNs(node, nullptr);

    // Recurse through child nodes
    xmlNodePtr curr = node->children;
    while (curr != nullptr) {
        cleanContentMath(curr);
        curr = curr->next;
    }
}

}  // namespace parser
}  // namespace crawler
