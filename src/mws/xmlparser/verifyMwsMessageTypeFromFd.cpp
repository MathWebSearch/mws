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
  * @brief File containing the implementation of the verifyMwsMessageTypeFromFd
  * function
  *
  * @file verifyMwsMessageTypeFromFd.cpp
  * @author Daniel Hasegan
  * @date 08 Aug 2012
  *
  * License: GPL v3
  *
  */

// System includes

#include <stdlib.h>                    // C general purpose library
#include <stdio.h>                     // C Standrad Input Output
#include <string.h>                    // C string library
#include <libxml/tree.h>               // LibXML tree headers
#include <libxml/parser.h>             // LibXML parser headers
#include <libxml/parserInternals.h>    // LibXML parser internals API
#include <libxml/threads.h>            // LibXML thread handling API
#include <sys/types.h>                 // Primitive System datatypes
#include <sys/stat.h>                  // POSIX File characteristics
#include <fcntl.h>                     // File control operations
#include <sstream>                     // C++ stringstream header
#include <string>                      // C++ string header

// Local includes

#include "ParserTypes.hpp"             // Common Mws Parsers datatypes
#include "common/utils/DebugMacros.hpp"// Debug macros
#include "common/utils/macro_func.h"   // Macro functions

// Macros

#define MWSHARVEST_MAIN_NAME           "mws:harvest"
#define MWSQUERY_MAIN_NAME             "mws:query"

// Namespaces

using namespace std;
using namespace mws;


/**
  * @brief Callback function used to be used with an IO context parser
  *
  */
static inline int
fdXmlInputReadCallback(void* fdPtr,
                       char* buffer,
                       int   len)
{
#ifdef TRACE_FUNC_CALLS
    LOG_TRACE_IN;
#endif

    int result;
    result =  read(*(int*)  fdPtr,
                   (void*)  buffer,
                   (size_t) len);

#ifdef TRACE_FUNC_CALLS
    LOG_TRACE_OUT;
#endif
    return result;
}

/**
  * @brief This function is called before the SAX handler starts parsing the
  * document
  *
  * @param user_data is a structure which holds the state of the parser.
  */
static void
my_startDocument(void* user_data)
{
#ifdef TRACE_FUNC_CALLS
    LOG_TRACE_IN;
#endif

    MwsMessage_SaxUserData* data = (MwsMessage_SaxUserData*) user_data;

    data->messageTypeFound            = 0;
    data->messageType                 = "";

#ifdef TRACE_FUNC_CALLS
    LOG_TRACE_OUT;
#endif
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
static void
my_startElement(void*           user_data,
                const xmlChar*  name,
                const xmlChar** attrs)
{
#ifdef TRACE_FUNC_CALLS
    LOG_TRACE_IN;
#endif

    MwsMessage_SaxUserData* data = (MwsMessage_SaxUserData*) user_data;

    if (strcmp((char*) name, MWSHARVEST_MAIN_NAME) == 0)
        {
              data->messageTypeFound = 1;
              data->messageType = MWSHARVEST_MAIN_NAME;
        }
   else if (strcmp((char*) name, MWSQUERY_MAIN_NAME) == 0)
        {
              data->messageTypeFound = 1;
              data->messageType = MWSQUERY_MAIN_NAME;
        }

    int ret = (attrs != NULL); // avoiding Warnings
    (void) ret;

#if DEBUG
    printf("Beginning of element : %s \n", name);
#endif
#ifdef TRACE_FUNC_CALLS
    LOG_TRACE_OUT;
#endif
}

// Implementation

namespace mws
{

string
verifyMwsMessageTypeFromFd(int fd)
{
#ifdef TRACE_FUNC_CALLS
    LOG_TRACE_IN;
#endif

    MwsMessage_SaxUserData user_data;
    xmlSAXHandler          saxHandler;
    xmlParserCtxtPtr       ctxtPtr;
    int                    ret;

    // Initializing the SAX Handler
    memset(&saxHandler, 0, sizeof(xmlSAXHandler));

    // Registering Sax callbacks with defined ones

    //internalSubsetSAXFunc        internalSubset;
    //isStandaloneSAXFunc          isStandalone;
    //hasInternalSubsetSAXFunc     hasInternalSubset;
    //hasExternalSubsetSAXFunc     hasExternalSubset;
    //resolveEntitySAXFunc         resolveEntity;
    //saxHandler.getEntity     = my_getEntity;               // STUB
    //entityDeclSAXFunc            entityDecl;
    //notationDeclSAXFunc          notationDecl;
    //attributeDeclSAXFunc         attributeDecl;
    //elementDeclSAXFunc           elementDecl;
    //unparsedEntityDeclSAXFunc    unparsedEntityDecl;
    //setDocumentLocatorSAXFunc    setDocumentLocator;
    saxHandler.startDocument = my_startDocument;
    //saxHandler.endDocument   = my_endDocument;
    saxHandler.startElement  = my_startElement;
    //saxHandler.endElement    = my_endElement;
    //referenceSAXFunc             reference;
    //saxHandler.characters    = my_characters;
    //ignorableWhitespaceSAXFunc   ignorableWhitespace;
    //processingInstructionSAXFunc processingInstruction;
    //commentSAXFunc               comment;
    //saxHandler.warning       = my_warning;
    //saxHandler.error         = my_error;
    //saxHandler.fatalError    = my_fatalError;

    // Locking libXML -- to allow multi-threaded use
    xmlLockLibrary();
    
    // Creating the IOParser context
    if ((ctxtPtr = xmlCreateIOParserCtxt(&saxHandler,
                                         &user_data,
                                         fdXmlInputReadCallback,
                                         NULL,
                                         &fd,
                                         XML_CHAR_ENCODING_UTF8))
            == NULL)
    {
        fprintf(stderr, "Error while creating the ParserContext\n");
    }
    // Parsing the document
    else if ((ret = xmlParseDocument(ctxtPtr))
            == -1)
    {
        fprintf(stderr, "Parsing XML document failed\n");
    }

    // Freeing the parser context
    if (ctxtPtr)
        xmlFreeParserCtxt(ctxtPtr);

    // Unlocking libXML -- to allow multi-threaded use
    xmlUnlockLibrary();

    // Checking for found messages
    if (!user_data.messageTypeFound)
    {
      fprintf(stderr, "The message is not a known MWS message type\n");
    }

#ifdef TRACE_FUNC_CALLS
    LOG_TRACE_OUT;
#endif

    return user_data.messageType;
}

}
