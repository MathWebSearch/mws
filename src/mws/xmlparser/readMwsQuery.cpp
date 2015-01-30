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
  * @brief File containing the implementation of the readMwsQuery
  * function
  * @file readMwsQuery.cpp
  * @author Corneliu-Claudiu Prodescu
  * @date 19 Apr 2011
  *
  * @edited Radu Hambasan
  * @date 20 Mar 2014
  *
  * License: GPL v3
  */

// System includes

#include <stdarg.h>
#include <stdio.h>                   // C Standrad Input Output
#include <stdlib.h>                  // C general purpose library
#include <string.h>                  // C string library
#include <libxml/tree.h>             // LibXML tree headers
#include <libxml/parser.h>           // LibXML parser headers
#include <libxml/parserInternals.h>  // LibXML internals APIs
#include <sys/types.h>               // Primitive System datatypes
#include <sys/stat.h>                // POSIX File characteristics
#include <fcntl.h>                   // File control operations

#include "mws/xmlparser/readMwsQuery.hpp"
#include "common/utils/compiler_defs.h"
#include "common/utils/getBoolType.hpp"
using common::utils::BoolType;
using common::utils::getBoolType;
#include "mws/xmlparser/MwsJsonResponseFormatter.hpp"
using mws::parser::RESPONSE_FORMATTER_MWS_JSON;
#include "mws/xmlparser/MwsXmlResponseFormatter.hpp"
using mws::parser::RESPONSE_FORMATTER_MWS_XML;
#include "mws/xmlparser/MwsIdsResponseFormatter.hpp"
using mws::parser::RESPONSE_FORMATTER_MWS_IDS;
#include "mws/xmlparser/SchemaXmlResponseFormatter.hpp"
using mws::parser::RESPONSE_FORMATTER_SCHEMA_XML;
#include "mws/xmlparser/SchemaJsonResponseFormatter.hpp"
using mws::parser::RESPONSE_FORMATTER_SCHEMA_JSON;

#define MWSQUERY_MAIN_NAME "mws:query"
#define MWSQUERY_ATTR_ANSWSET_MAXSIZE "answsize"
#define MWSQUERY_ATTR_ANSWSET_LIMITMIN "limitmin"
#define MWSQUERY_ATTR_ANSWSET_TOTALREQ "totalreq"
#define MWSQUERY_ATTR_OUTPUTFORMAT "output"
#define MWSQUERY_ATTR_SCHEMADEPTH "schema_depth"
#define MWSQUERY_EXPR_NAME "mws:expr"

using namespace mws;
using namespace mws::types;

/**
  * @brief Enumeration holding the states of the Mws Query Sax Parser.
  */
enum MwsQueryState {
    MWSQUERYSTATE_DEFAULT,
    MWSQUERYSTATE_IN_MWS_QUERY,
    MWSQUERYSTATE_IN_MWS_EXPR,
    MWSQUERYSTATE_UNKNOWN,
};

/**
  * @brief Data type used to store the variables needed during the parsing of
  * a MwsQuery
  */
struct MwsQuery_SaxUserData {
    /// Depth of the parse tree in unknown state
    int unknownDepth;
    /// The token which is currently being parsed
    types::CmmlToken* currentToken;
    /// The root of the token being currently parsed
    types::CmmlToken* currentTokenRoot;
    /// State of the parsing
    MwsQueryState state;
    /// State of the parsing before going into an unknown state
    MwsQueryState prevState;
    /// True if an XML structural error is detected
    bool errorDetected;
    /// Result of the parsing
    mws::types::Query* result;
    /// Type of query that we are parsing
    mws::xmlparser::QueryMode qMode;

    MwsQuery_SaxUserData() {
        result = nullptr;
        result = new Query;
        result->responseFormatter = RESPONSE_FORMATTER_MWS_XML;
        currentToken = nullptr;
        currentTokenRoot = nullptr;
        state = MWSQUERYSTATE_DEFAULT;
        errorDetected = false;
        result->warnings = 0;
        qMode = mws::xmlparser::QUERY_MWS;

    }
};

/**
  * @brief Callback function used to be used with an IO context parser
  *
  */
static inline int fileInputReadCallback(void* file, char* buffer, int len) {
    return fread(buffer, sizeof(char), len, (FILE*)file);
}

static void my_endDocument(void* user_data) {
    MwsQuery_SaxUserData* data = (MwsQuery_SaxUserData*)user_data;

    if (data->errorDetected) {
        delete data->result;
        data->result = nullptr;
    }
}

/**
  * @brief This function is called when the SAX handler encounters the
  * beginning of an element.
  *
  * @param
  * ta is a structure which holds the state of the parser.
  * @param name      is the name of the element which triggered the callback.
  * @param attrs     is an array of attributes and values, alternatively
  *                  placed.
  */
static void my_startElement(void* user_data, const xmlChar* name,
                            const xmlChar** attrs) {
    int numValue;
    BoolType boolValue;
    MwsQuery_SaxUserData* data = (MwsQuery_SaxUserData*)user_data;

    switch (data->state) {
    case MWSQUERYSTATE_DEFAULT:
        if (strcmp((char*)name, MWSQUERY_MAIN_NAME) == 0) {
            data->state = MWSQUERYSTATE_IN_MWS_QUERY;
            // Parsing the attributes
            while (nullptr != attrs && nullptr != attrs[0]) {
                if (strncmp((char*)attrs[0], "xmlns:", 6) == 0) {
                    /* ignore namespaces */
                } else if (strcmp((char*)attrs[0],
                                  MWSQUERY_ATTR_ANSWSET_MAXSIZE) == 0) {
                    numValue = (int)strtol((char*)attrs[1], nullptr, 10);
                    data->result->attrResultMaxSize = numValue;
                } else if (strcmp((char*)attrs[0],
                                  MWSQUERY_ATTR_ANSWSET_LIMITMIN) == 0) {
                    numValue = (int)strtol((char*)attrs[1], nullptr, 10);
                    data->result->attrResultLimitMin = numValue;
                } else if (strcmp((char*)attrs[0],
                                  MWSQUERY_ATTR_ANSWSET_TOTALREQ) == 0) {
                    boolValue = getBoolType((char*)attrs[1]);
                    data->result->attrResultTotalReq = boolValue;
                } else if (strcmp((char*)attrs[0],
                                  MWSQUERY_ATTR_SCHEMADEPTH) == 0) {
                    numValue = (int)strtol((char*)attrs[1], nullptr, 10);
                    data->result->max_depth = numValue;
                } else if (strcmp((char*)attrs[0],
                                  MWSQUERY_ATTR_OUTPUTFORMAT) == 0) {
                    bool schemaMode =
                            data->qMode == mws::xmlparser::QUERY_SCHEMA;
                    if (strcmp((char*)attrs[1], "xml") == 0) {
                        if (schemaMode) {
                            data->result->responseFormatter =
                                    RESPONSE_FORMATTER_SCHEMA_XML;
                        } else {
                            data->result->responseFormatter =
                                    RESPONSE_FORMATTER_MWS_XML;
                        }
                    } else if (strcmp((char*)attrs[1], "json") == 0) {
                        if (schemaMode) {
                            data->result->responseFormatter =
                                    RESPONSE_FORMATTER_SCHEMA_JSON;
                        } else {
                            data->result->responseFormatter =
                                    RESPONSE_FORMATTER_MWS_JSON;
                        }
                    } else if (strcmp((char*)attrs[1], "mws-ids") == 0) {
                        data->result->responseFormatter =
                                RESPONSE_FORMATTER_MWS_IDS;
                        data->result->options.includeHits = false;
                        data->result->options.includeMwsIds = true;
                    } else {
                        PRINT_WARN("Invalid output format \"%s\"\n", attrs[1]);
                    }
                } else {
                    // Invalid attributes
                    data->result->warnings++;
                    PRINT_WARN("Invalid attribute: \"%s\"\n", attrs[0]);
                }

                attrs = &attrs[2];
            }
        } else {
            data->result->warnings++;
            // Saving the state
            data->prevState = data->state;
            // Going to an unkown state
            data->state = MWSQUERYSTATE_UNKNOWN;
            data->unknownDepth = 1;
        }
        break;

    case MWSQUERYSTATE_IN_MWS_QUERY:
        if (strcmp((char*)name, MWSQUERY_EXPR_NAME) == 0) {
            data->state = MWSQUERYSTATE_IN_MWS_EXPR;
        } else {
            data->result->warnings++;
            // Saving the state
            data->prevState = data->state;
            // Going to an unkown state
            data->state = MWSQUERYSTATE_UNKNOWN;
            data->unknownDepth = 1;
        }
        break;

    case MWSQUERYSTATE_IN_MWS_EXPR:
        // Building the token
        if (data->currentToken != nullptr) {
            data->currentToken = data->currentToken->newChildNode();
        } else {
            data->currentTokenRoot = CmmlToken::newRoot();
            data->currentToken = data->currentTokenRoot;
        }
        // Adding the tag name
        data->currentToken->setTag((char*)name);
        // Adding the attributes
        while (nullptr != attrs && nullptr != attrs[0]) {
            data->currentToken->addAttribute((char*)attrs[0], (char*)attrs[1]);
            attrs = &attrs[2];
        }
        break;

    case MWSQUERYSTATE_UNKNOWN:
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
    UNUSED(name);

    MwsQuery_SaxUserData* data = (MwsQuery_SaxUserData*)user_data;

    switch (data->state) {
    case MWSQUERYSTATE_DEFAULT:
        // Shouldn't happen
        break;

    case MWSQUERYSTATE_IN_MWS_QUERY:
        data->state = MWSQUERYSTATE_DEFAULT;
        break;

    case MWSQUERYSTATE_IN_MWS_EXPR:
        if (data->currentToken == nullptr) {
            data->state = MWSQUERYSTATE_IN_MWS_QUERY;
        } else if (data->currentToken->isRoot()) {
            data->result->tokens.push_back(data->currentToken);
            data->currentToken = nullptr;
            data->currentTokenRoot = nullptr;
        } else {
            data->currentToken = data->currentToken->getParentNode();
        }
        break;

    case MWSQUERYSTATE_UNKNOWN:
        data->unknownDepth--;
        if (data->unknownDepth == 0) {
            data->state = data->prevState;
        }
        break;
    }
}

static void my_characters(void* user_data, const xmlChar* ch, int len) {
    MwsQuery_SaxUserData* data = (MwsQuery_SaxUserData*)user_data;

    if (data->state == MWSQUERYSTATE_IN_MWS_EXPR &&
        data->currentToken != nullptr) {
        data->currentToken->appendTextContent((char*)ch, len);
    }
}

static xmlEntityPtr my_getEntity(void* user_data, const xmlChar* name) {
    UNUSED(user_data);

    return xmlGetPredefinedEntity(name);
}

static void my_warning(void* user_data, const char* msg, ...) {
    va_list args;
    MwsQuery_SaxUserData* data = (MwsQuery_SaxUserData*)user_data;

    data->result->warnings++;
    va_start(args, msg);
    vfprintf(stderr, msg, args);
    va_end(args);
}

static void my_error(void* user_data, const char* msg, ...) {
    va_list args;
    MwsQuery_SaxUserData* data = (MwsQuery_SaxUserData*)user_data;

    data->errorDetected = true;
    if (data->currentTokenRoot != nullptr) {
        delete data->currentTokenRoot;
        data->currentTokenRoot = nullptr;
        data->currentToken = nullptr;
    }
    va_start(args, msg);
    vfprintf(stderr, msg, args);
    va_end(args);
}

// in latest libxml2, all errors are caught by error, so this is useless
static void my_fatalError(void* user_data, const char* msg, ...) {
    UNUSED(user_data);

    va_list args;

    va_start(args, msg);
    vfprintf(stderr, msg, args);
    va_end(args);
}

// Implementation

namespace mws {
namespace xmlparser {

Query* readMwsQuery(FILE* file, QueryMode mode) {
    // file might be nullptr, if no data was sent
    if (file == nullptr) return nullptr;
    MwsQuery_SaxUserData user_data;
    if (mode == QUERY_SCHEMA) {
        user_data.qMode = QUERY_SCHEMA;
        // because the default setting was MWS_XML
        user_data.result->responseFormatter = RESPONSE_FORMATTER_SCHEMA_JSON;
    }

    xmlSAXHandler saxHandler;
    xmlParserCtxtPtr ctxtPtr;
    int ret;

    // Initializing the SAX Handler
    memset(&saxHandler, 0, sizeof(saxHandler));

    // Registering Sax callbacks with defined ones
    saxHandler.getEntity = my_getEntity;
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
    ctxtPtr =
        xmlCreateIOParserCtxt(&saxHandler, &user_data, fileInputReadCallback,
                              nullptr, file, XML_CHAR_ENCODING_UTF8);
    if (ctxtPtr == nullptr) {
        PRINT_WARN("Error while creating the ParserContext\n");
        xmlUnlockLibrary();
        return nullptr;
    }

    ret = xmlParseDocument(ctxtPtr);
    if (ret == -1) {
        PRINT_WARN("Parsing failed\n");
    }
    if (!ctxtPtr->wellFormed) {
        PRINT_WARN("Bad XML document\n");
    }

    // Freeing the parser context
    xmlFreeParserCtxt(ctxtPtr);

    // Unlocking libXML -- to allow multi-threaded use
    xmlUnlockLibrary();

    return user_data.result;
}

}  // namespace xmlparser
}  // namespace mws
