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

#include <stdlib.h>                    // C general purpose library
#include <stdio.h>                     // C Standrad Input Output
#include <string.h>                    // C string library
#include <libxml/tree.h>               // LibXML tree headers
#include <libxml/parser.h>             // LibXML parser headers
#include <libxml/parserInternals.h>    // LibXML internals APIs
#include <sys/types.h>                 // Primitive System datatypes
#include <sys/stat.h>                  // POSIX File characteristics
#include <fcntl.h>                     // File control operations

#include "ParserTypes.hpp"
#include "mws/xmlparser/readMwsQuery.hpp"
#include "common/utils/compiler_defs.h"
#include "common/utils/getBoolType.hpp"

#define MWSQUERY_MAIN_NAME             "mws:query"
#define MWSQUERY_ATTR_ANSWSET_MAXSIZE  "answsize"
#define MWSQUERY_ATTR_ANSWSET_LIMITMIN "limitmin"
#define MWSQUERY_ATTR_ANSWSET_TOTALREQ "totalreq"
#define MWSQUERY_ATTR_OUTPUTFORMAT     "output"
#define MWSQUERY_EXPR_NAME             "mws:expr"

using namespace mws;
using namespace mws::types;


/**
  * @brief Callback function used to be used with an IO context parser
  *
  */
static inline int
fileInputReadCallback(void* file,
                      char* buffer,
                      int   len) {
    return fread(buffer, sizeof(char), len, (FILE *) file);
}

/**
  * @brief This function is called before the SAX handler starts parsing the
  * document
  *
  * @param user_data is a structure which holds the state of the parser.
  */
static void
my_startDocument(void* user_data) {
    MwsQuery_SaxUserData* data = (MwsQuery_SaxUserData*) user_data;

    data->result           = new MwsQuery;
    data->currentToken     = NULL;
    data->currentTokenRoot = NULL;
    data->state            = MWSQUERYSTATE_DEFAULT;
    data->errorDetected    = false;
    data->result->warnings = 0;
}

static void
my_endDocument(void* user_data) {
    MwsQuery_SaxUserData* data = (MwsQuery_SaxUserData*) user_data;

    if (data->errorDetected) {
        delete data->result;
        data->result = NULL;
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
static void
my_startElement(void*           user_data,
                const xmlChar*  name,
                const xmlChar** attrs) {
    int      numValue;
    BoolType boolValue;
    MwsQuery_SaxUserData* data = (MwsQuery_SaxUserData*) user_data;

    switch (data->state) {
    case MWSQUERYSTATE_DEFAULT:
        if (strcmp((char*) name, MWSQUERY_MAIN_NAME) == 0) {
            data->state = MWSQUERYSTATE_IN_MWS_QUERY;
            // Parsing the attributes
            while (NULL != attrs && NULL != attrs[0]) {
                if (strncmp((char*)attrs[0], "xmlns:", 6) == 0) {
                    /* ignore namespaces */
                } else if (strcmp((char*)attrs[0],
                                  MWSQUERY_ATTR_ANSWSET_MAXSIZE) == 0) {
                    numValue = (int) strtol((char*)attrs[1], NULL, 10);
                    data->result->attrResultMaxSize = numValue;
                } else if (strcmp((char*)attrs[0],
                                MWSQUERY_ATTR_ANSWSET_LIMITMIN) == 0) {
                    numValue = (int) strtol((char*)attrs[1], NULL, 10);
                    data->result->attrResultLimitMin = numValue;
                } else if (strcmp((char*)attrs[0],
                                MWSQUERY_ATTR_ANSWSET_TOTALREQ) == 0) {
                    boolValue = getBoolType((char*)attrs[1]);
                    data->result->attrResultTotalReq = boolValue;
                } else if (strcmp((char*)attrs[0],
                                  MWSQUERY_ATTR_OUTPUTFORMAT) == 0) {
                    if (strcmp((char*) attrs[1], "xml") == 0) {
                        data->result->attrResultOutputFormat = DATAFORMAT_XML;
                    } else if (strcmp((char*) attrs[1], "json") == 0) {
                        data->result->attrResultOutputFormat = DATAFORMAT_JSON;
                    } else {
                        data->result->attrResultOutputFormat =
                                DATAFORMAT_UNKNOWN;
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
        if (strcmp((char*) name, MWSQUERY_EXPR_NAME) == 0) {
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
        if (data->currentToken != NULL) {
            data->currentToken = data->currentToken->newChildNode();
        } else {
            data->currentTokenRoot = CmmlToken::newRoot(false);
            data->currentToken = data->currentTokenRoot;
        }
        // Adding the tag name
        data->currentToken->setTag((char*) name);
        // Adding the attributes
        while (NULL != attrs && NULL != attrs[0]) {
            data->currentToken->addAttribute((char*) attrs[0],
                    (char*) attrs[1]);
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
static void
my_endElement(void*          user_data,
              const xmlChar* name) {
    UNUSED(name);

    MwsQuery_SaxUserData* data = (MwsQuery_SaxUserData*) user_data;

    switch (data->state) {
    case MWSQUERYSTATE_DEFAULT:
        // Shouldn't happen
        break;

    case MWSQUERYSTATE_IN_MWS_QUERY:
        data->state = MWSQUERYSTATE_DEFAULT;
        break;

    case MWSQUERYSTATE_IN_MWS_EXPR:
        if (data->currentToken == NULL) {
            data->state = MWSQUERYSTATE_IN_MWS_QUERY;
        } else if (data->currentToken->isRoot()) {
            data->result->tokens.push_back(data->currentToken);
            data->currentToken = NULL;
            data->currentTokenRoot = NULL;
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


static void
my_characters(void *user_data,
              const xmlChar *ch,
              int len) {
    MwsQuery_SaxUserData* data = (MwsQuery_SaxUserData*) user_data;

    if (data->state == MWSQUERYSTATE_IN_MWS_EXPR &&
            data->currentToken != NULL) {
        data->currentToken->appendTextContent((char*) ch, len);
    }
}


static xmlEntityPtr
my_getEntity(void*          user_data,
             const xmlChar* name) {
    UNUSED(user_data);

    return xmlGetPredefinedEntity(name);
}


static void
my_warning(void*       user_data,
           const char* msg,
           ...) {
    va_list args;
    MwsQuery_SaxUserData* data = (MwsQuery_SaxUserData*) user_data;

    data->result->warnings++;
    va_start(args, msg);
    vfprintf(stderr, msg, args);
    va_end(args);
}


static void
my_error(void*       user_data,
         const char* msg,
         ...) {
    va_list args;
    MwsQuery_SaxUserData* data = (MwsQuery_SaxUserData*) user_data;

    data->errorDetected = true;
    if (data->currentTokenRoot != NULL) {
        delete data->currentTokenRoot;
        data->currentTokenRoot = NULL;
        data->currentToken = NULL;
    }
    va_start(args, msg);
    vfprintf(stderr, msg, args);
    va_end(args);
}


// in latest libxml2, all errors are caught by error, so this is useless
static void
my_fatalError(void*       user_data,
              const char* msg,
              ...) {
    UNUSED(user_data);

    va_list args;

    va_start(args, msg);
    vfprintf(stderr, msg, args);
    va_end(args);
}


// Implementation

namespace mws {
namespace xmlparser {

MwsQuery* readMwsQuery(FILE* file) {
    MwsQuery_SaxUserData user_data;
    xmlSAXHandler        saxHandler;
    xmlParserCtxtPtr     ctxtPtr;
    int                  ret;

    // Initializing the SAX Handler
    memset(&saxHandler, 0, sizeof(saxHandler));

    // Registering Sax callbacks with defined ones
    saxHandler.getEntity     = my_getEntity;
    saxHandler.startDocument = my_startDocument;
    saxHandler.endDocument   = my_endDocument;
    saxHandler.startElement  = my_startElement;
    saxHandler.endElement    = my_endElement;
    saxHandler.characters    = my_characters;
    saxHandler.warning       = my_warning;
    saxHandler.error         = my_error;
    saxHandler.fatalError    = my_fatalError;

    // Locking libXML -- to allow multi-threaded use
    xmlLockLibrary();

    // Creating the IOParser context
    ctxtPtr = xmlCreateIOParserCtxt(&saxHandler,
                                    &user_data,
                                    fileInputReadCallback,
                                    NULL,
                                    file,
                                    XML_CHAR_ENCODING_UTF8);
    if (ctxtPtr == NULL) {
        PRINT_WARN("Error while creating the ParserContext\n");
        xmlUnlockLibrary();
        return NULL;
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
