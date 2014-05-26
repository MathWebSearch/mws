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
  * @brief File containing the implementation of the loadMwsHarvestFromFd
  * function
  *
  * @file loadMwsHarvestFromFd.cpp
  * @author Corneliu-Claudiu Prodescu
  * @date 30 Apr 2011
  *
  * @edited Radu Hambasan
  * @date 30 Apr 2014
  *
  * License: GPL v3
  *
  */

#include <assert.h>
#include <stdlib.h>                  // C general purpose library
#include <stdio.h>                   // C Standrad Input Output
#include <string.h>                  // C string library
#include <libxml/tree.h>             // LibXML tree headers
#include <libxml/parser.h>           // LibXML parser headers
#include <libxml/parserInternals.h>  // LibXML parser internals API
#include <libxml/threads.h>          // LibXML thread handling API
#include <libxml/xmlwriter.h>
#include <sys/types.h>  // Primitive System datatypes
#include <sys/stat.h>   // POSIX File characteristics
#include <fcntl.h>      // File control operations

#include <iostream>
#include <sstream>
#include <string>
using std::string;
#include <vector>
using std::vector;
#include <map>
using std::map;
#include <utility>

#include "mws/index/IndexBuilder.hpp"
using mws::index::IndexingOptions;
#include "mws/types/CmmlToken.hpp"
#include "common/utils/compiler_defs.h"

#include "processMwsHarvest.hpp"

// Macros
#define MWSHARVEST_MAIN_NAME "mws:harvest"
#define MWSHARVEST_EXPR_NAME "mws:expr"
#define MWSHARVEST_EXPR_ATTR_XMLID_NAME "url"
#define MWSHARVEST_EXPR_ATTR_LOCALID_NAME "mws:data_id"
#define MWSHARVEST_DATA_NAME "mws:data"
#define MWSHARVEST_DATA_ATTR_LOCALID_NAME "mws:data_id"
#define MWSHARVEST_EXPR_URI_DEFAULT ""

using namespace std;
using namespace mws::types;
using namespace mws::dbc;

namespace mws {
namespace parser {

enum MwsHarvestState {
    MWSHARVESTSTATE_DEFAULT,
    MWSHARVESTSTATE_IN_MWS_HARVEST,
    MWSHARVESTSTATE_IN_MWS_EXPR,
    MWSHARVESTSTATE_IN_MWS_DATA,
    MWSHARVESTSTATE_UNKNOWN
};

struct MwsHarvest_SaxUserData {
    MwsHarvestState state;  //!< Parse state machine state
    int unknownDepth;       //!< Depth of XML tree in unknown state
    map<string, CrawlId> localIdToCrawlIds;

    /// The token which is currently being parsed
    types::CmmlToken* currentToken;
    /// The root of the token being currently parsed
    types::CmmlToken* currentTokenRoot;
    /// State of the parsing before going into an unknown state
    MwsHarvestState prevState;
    /// True if an XML structural error is detected
    bool errorDetected;
    /// Number of correctly parsed expressions
    int parsedExpr;
    /// Variable used to show the number of warnings (-1 for critical error)
    int warnings;
    /// Object that acts when expr or data elements are parsed
    HarvestProcessor* harvestProcessor;

    xmlTextWriter* stringWriter;
    int copyDepth;
    std::vector<char> buffer;

    /// URI of the expression being parsed
    std::string exprUri;
    std::string data;

    std::string localId;
    MwsHarvest_SaxUserData()
        : state(MWSHARVESTSTATE_DEFAULT),
          unknownDepth(0),
          currentToken(nullptr),
          currentTokenRoot(nullptr),
          prevState(MWSHARVESTSTATE_DEFAULT),
          errorDetected(false),
          parsedExpr(0),
          warnings(0),
          exprUri("") {}
};

/**
  * @brief Callback function used with an xmlOutputBuffer
  */
static inline int copyToCharBufCallback(void* vectorPtr, const char* buffer,
                                        int len) {
    vector<char>* vec = (vector<char>*)vectorPtr;
    for (int i = 0; i < len; ++i) {
        vec->push_back(buffer[i]);
    }

    return len;
}

static inline int setupCopyToStringWriter(MwsHarvest_SaxUserData* data) {
    xmlOutputBuffer* outPtr;

    data->buffer.clear();
    if ((outPtr = xmlOutputBufferCreateIO(copyToCharBufCallback, nullptr,
                                          &data->buffer, nullptr)) == nullptr) {
        PRINT_WARN("Error while creating the OutputBuffer\n");
        return -1;
    } else if ((data->stringWriter = xmlNewTextWriter(outPtr)) == nullptr) {
        PRINT_WARN("Error while creating the XML Writer\n");
        return -1;
    }

    return 0;
}

static inline void tearDownCopyToStringWriter(MwsHarvest_SaxUserData* data) {
    xmlTextWriterEndDocument(data->stringWriter);
    xmlTextWriterFlush(data->stringWriter);
    xmlFreeTextWriter(data->stringWriter);
    data->data = string(data->buffer.data(), data->buffer.size());
}

/**
  * @brief Callback function used to be used with an IO context parser
  *
  */
static inline int fdXmlInputReadCallback(void* fdPtr, char* buffer, int len) {
    return read(*(int*)fdPtr, (void*)buffer, (size_t)len);
}

/**
  * @brief This function is called after the SAX handler has finished parsing
  * the document
  *
  * @param user_data is a structure which holds the state of the parser.
  */
static void my_endDocument(void* user_data) {
    MwsHarvest_SaxUserData* data = (MwsHarvest_SaxUserData*)user_data;
    if (data->errorDetected) {
        PRINT_WARN("Ending document due to errors\n");
    }
}

/**
  * @brief This function is called when the SAX handler encounters the
  * beginning of an element.
  *
  * @param user_data is a structure which holds the state of the parser.
  * @param name      is the name of the element which triggered the callback.
  * @param attrs     is an array of attributes and values, alternatively
  *                  placed.
  */
static void my_startElement(void* user_data, const xmlChar* name,
                            const xmlChar** attrs) {
    MwsHarvest_SaxUserData* data = (MwsHarvest_SaxUserData*)user_data;

    switch (data->state) {
    case MWSHARVESTSTATE_DEFAULT:
        if (strcmp(reinterpret_cast<const char*>(name), MWSHARVEST_MAIN_NAME) ==
            0) {
            data->state = MWSHARVESTSTATE_IN_MWS_HARVEST;
            // Parsing the attributes
            while (nullptr != attrs && nullptr != attrs[0]) {
                if (strncmp((char*)attrs[0], "xmlns:", 6) == 0) {
                    /* ignore namespaces */
                } else {
                    PRINT_WARN("Unexpected attribute \"%s\"\n", attrs[0]);
                }
                attrs = &attrs[2];
            }
        } else {
            // Saving the state
            data->prevState = data->state;
            // Going to an unkown state
            data->state = MWSHARVESTSTATE_UNKNOWN;
            data->unknownDepth = 1;
        }
        break;

    case MWSHARVESTSTATE_IN_MWS_HARVEST:
        if (strcmp(reinterpret_cast<const char*>(name), MWSHARVEST_EXPR_NAME) ==
            0) {
            data->state = MWSHARVESTSTATE_IN_MWS_EXPR;

            // Parsing the attributes
            data->exprUri = MWSHARVEST_EXPR_URI_DEFAULT;
            data->localId = "";
            while (nullptr != attrs && nullptr != attrs[0]) {
                if (strcmp(reinterpret_cast<const char*>(attrs[0]),
                           MWSHARVEST_EXPR_ATTR_XMLID_NAME) == 0) {
                    data->exprUri = reinterpret_cast<const char*>(attrs[1]);
                } else if (strcmp(reinterpret_cast<const char*>(attrs[0]),
                                  MWSHARVEST_EXPR_ATTR_LOCALID_NAME) == 0) {
                    data->localId = reinterpret_cast<const char*>(attrs[1]);
                } else {
                    // Invalid attribute
                    PRINT_WARN("Unexpected attribute \"%s\"\n", attrs[0]);
                }

                attrs = &attrs[2];
            }
            if (data->localId != "" &&
                (data->localIdToCrawlIds.find(data->localId) ==
                 data->localIdToCrawlIds.end())) {
                PRINT_WARN("%s \"%s\" does not point to any %s element\n",
                           MWSHARVEST_DATA_ATTR_LOCALID_NAME,
                           data->localId.c_str(), MWSHARVEST_DATA_NAME);
                data->localId = "";
            }
        } else if (strcmp(reinterpret_cast<const char*>(name),
                          MWSHARVEST_DATA_NAME) == 0) {
            // Parsing the attributes
            data->localId = "";
            while (nullptr != attrs && nullptr != attrs[0]) {
                if (strcmp(reinterpret_cast<const char*>(attrs[0]),
                           MWSHARVEST_DATA_ATTR_LOCALID_NAME) == 0) {
                    data->localId = reinterpret_cast<const char*>(attrs[1]);
                } else {
                    // Invalid attribute
                    PRINT_WARN("Unexpected attribute \"%s\"\n", attrs[0]);
                }

                attrs = &attrs[2];
            }

            // Check if localId exists
            if (data->localId == "") {
                PRINT_WARN("Ignoring %s element without %s attribute.\n",
                           MWSHARVEST_DATA_NAME,
                           MWSHARVEST_DATA_ATTR_LOCALID_NAME);
                // Saving the state
                data->prevState = data->state;
                // Going to an unkown state
                data->state = MWSHARVESTSTATE_UNKNOWN;
                data->unknownDepth = 1;
            } else {
                setupCopyToStringWriter(data);
                data->state = MWSHARVESTSTATE_IN_MWS_DATA;
                data->copyDepth = 1;
            }
        } else {
            PRINT_WARN("Ignoring %s element\n", name);
            // Saving the state
            data->prevState = data->state;
            // Going to an unkown state
            data->state = MWSHARVESTSTATE_UNKNOWN;
            data->unknownDepth = 1;
        }
        break;

    case MWSHARVESTSTATE_IN_MWS_EXPR:
        if (data->currentToken != nullptr) {
            data->currentToken = data->currentToken->newChildNode();
        } else {
            data->currentTokenRoot = CmmlToken::newRoot(true);
            data->currentToken = data->currentTokenRoot;
        }
        data->currentToken->setTag(reinterpret_cast<const char*>(name));
        // Adding the attributes
        while (nullptr != attrs && nullptr != attrs[0]) {
            string attributeName = reinterpret_cast<const char*>(attrs[0]);
            string attributeValue = reinterpret_cast<const char*>(attrs[1]);
            data->currentToken->addAttribute(attributeName, attributeValue);

            attrs = &attrs[2];
        }
        break;

    case MWSHARVESTSTATE_IN_MWS_DATA:
        data->copyDepth++;
        xmlTextWriterStartElement(data->stringWriter, name);
        while (nullptr != attrs && nullptr != attrs[0]) {
            xmlTextWriterWriteAttribute(data->stringWriter, attrs[0], attrs[1]);
            attrs = &attrs[2];
        }
        break;

    case MWSHARVESTSTATE_UNKNOWN:
        data->unknownDepth++;
        break;
    }
}

/**
  * @brief This function is called when the SAX handler encounters the
  * end of an element.
  *
  * @param user_data is a structure which holds the state of the parser.
  * @param name      is the name of the element which triggered the callback.
  */
static void my_endElement(void* user_data, const xmlChar* name) {
    MwsHarvest_SaxUserData* data = (MwsHarvest_SaxUserData*)user_data;

    switch (data->state) {
    case MWSHARVESTSTATE_DEFAULT:
        PRINT_WARN("Unexpected Default State for end element \"%s\"",
                   reinterpret_cast<const char*>(name));
        assert(false);
        break;

    case MWSHARVESTSTATE_IN_MWS_HARVEST:
        data->state = MWSHARVESTSTATE_DEFAULT;
        break;

    case MWSHARVESTSTATE_IN_MWS_EXPR:
        if (data->currentToken == nullptr) {
            data->state = MWSHARVESTSTATE_IN_MWS_HARVEST;
        } else if (data->currentToken->isRoot()) {
            int ret;
            auto it = data->localIdToCrawlIds.find(data->localId);
            if (it != data->localIdToCrawlIds.end()) {
                CrawlId crawlId = it->second;
                ret = data->harvestProcessor->processExpression(
                    data->currentToken, data->exprUri, crawlId);
            } else {
                ret = data->harvestProcessor->processExpression(
                    data->currentToken, data->exprUri, CRAWLID_NULL);
            }
            if (ret != -1) {
                data->parsedExpr += ret;
            }
            delete data->currentToken;
            data->currentToken = nullptr;
            data->currentTokenRoot = nullptr;
        } else {
            data->currentToken = data->currentToken->getParentNode();
        }
        break;

    case MWSHARVESTSTATE_IN_MWS_DATA:
        xmlTextWriterEndElement(data->stringWriter);
        data->copyDepth--;
        if (data->copyDepth == 0) {
            tearDownCopyToStringWriter(data);
            data->state = MWSHARVESTSTATE_IN_MWS_HARVEST;

            // Insert data in db
            CrawlId crawlId = data->harvestProcessor->processData(data->data);
            data->localIdToCrawlIds[data->localId] = crawlId;
        }
        break;

    case MWSHARVESTSTATE_UNKNOWN:
        data->unknownDepth--;
        if (data->unknownDepth == 0) data->state = data->prevState;
        break;
    }
}

static void my_characters(void* user_data, const xmlChar* ch, int len) {
    MwsHarvest_SaxUserData* data = (MwsHarvest_SaxUserData*)user_data;

    if (data->state == MWSHARVESTSTATE_IN_MWS_EXPR &&  // Valid state
        data->currentToken != nullptr) {               // Valid token
        data->currentToken->appendTextContent((char*)ch, len);
    }

    if (data->state == MWSHARVESTSTATE_IN_MWS_DATA) {
        string inputChars((const char*)ch, len);
        xmlChar* encodedChars =
            xmlEncodeSpecialChars(nullptr, BAD_CAST inputChars.c_str());
        assert(encodedChars != nullptr);
        xmlTextWriterWriteRawLen(data->stringWriter, encodedChars,
                                 strlen((const char*)encodedChars));
        xmlFree(encodedChars);
    }
}

static void my_warning(void* user_data, const char* msg, ...) {
    va_list args;
    MwsHarvest_SaxUserData* data = (MwsHarvest_SaxUserData*)user_data;

    data->warnings++;

    va_start(args, msg);
    vfprintf(stderr, msg, args);
    va_end(args);
}

static void my_error(void* user_data, const char* msg, ...) {
    va_list args;
    MwsHarvest_SaxUserData* data = (MwsHarvest_SaxUserData*)user_data;

    data->errorDetected = true;
    if (data->currentTokenRoot) {
        delete data->currentTokenRoot;
        data->currentTokenRoot = nullptr;
        data->currentToken = nullptr;
    }
    va_start(args, msg);
    vfprintf(stderr, msg, args);
    va_end(args);
}

// ATM all errors are caught by error, so this is useless
static void my_fatalError(void* user_data, const char* msg, ...) {
    UNUSED(user_data);

    va_list args;

    va_start(args, msg);
    vfprintf(stderr, msg, args);
    va_end(args);
}

// Implementation

std::pair<int, int> processMwsHarvest(int fd,
                                      HarvestProcessor* harvestProcessor) {
    MwsHarvest_SaxUserData user_data;
    xmlSAXHandler saxHandler;
    xmlParserCtxtPtr ctxtPtr;
    int ret;

    user_data.harvestProcessor = harvestProcessor;
    ret = -1;

    // Initializing the SAX Handler
    memset(&saxHandler, 0, sizeof(xmlSAXHandler));

    // Registering Sax callbacks
    saxHandler.endDocument = my_endDocument;
    saxHandler.startElement = my_startElement;
    saxHandler.endElement = my_endElement;
    saxHandler.characters = my_characters;
    saxHandler.warning = my_warning;
    saxHandler.error = my_error;
    saxHandler.fatalError = my_fatalError;

    // Locking libXML -- to allow multi-threaded use
    xmlLockLibrary();
    // Creating the IOParser context
    if ((ctxtPtr = xmlCreateIOParserCtxt(&saxHandler, &user_data,
                                         fdXmlInputReadCallback, nullptr, &fd,
                                         XML_CHAR_ENCODING_UTF8)) == nullptr) {
        PRINT_WARN("Error while creating the ParserContext\n");
    }
    // Parsing the document
    else if ((ret = xmlParseDocument(ctxtPtr)) == -1) {
        PRINT_WARN("Parsing XML document failed\n");
    }

    // Freeing the parser context
    if (ctxtPtr) {
        xmlFreeParserCtxt(ctxtPtr);
    }

    // Unlocking libXML -- to allow multi-threaded use
    xmlUnlockLibrary();

    return make_pair(ret, user_data.parsedExpr);
}

}  // namespace parser
}  // namespace mws
