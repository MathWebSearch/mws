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
#ifndef _GENERICRESPONSES_HPP
#define _GENERICRESPONSES_HPP

/**
  * @brief File containing generic responses for the RESTful interface.
  * @file GenericResponses.hpp
  * @author Corneliu-Claudiu Prodescu
  * @date 31 Jul 2011
  *
  * License: GPL v3
  *
  */

// System includes

#include "microhttpd_linux.h"           // Linux MicroHTTPd library includes

// Constants

const char* XML_MWS_UNREACHABLE =
    "<?xml version=\"1.0\"?>\n"
    "<mws:info xmlns:mws=\"http://search.mathweb.org/ns\">"
    "Service unavailable</mws:info>";

const char* XML_MWS_BADQUERY =
    "<?xml version=\"1.0\"?>\n"
    "<mws:info xmlns:mws=\"http://search.mathweb.org/ns\">"
    "Bad query request</mws:info>";

const char* XML_MWS_INTERNALERROR =
    "<?xml version=\"1.0\"?>\n"
    "<mws:info xmlns:mws=\"http://search.mathweb.org/ns\">"
    "Internal error</mws:info>";

const char* EMPTY_RESPONSE = "";

inline int
sendXmlGenericResponse(struct MHD_Connection* connection,
                       const char*            xmlGenericResponse,
                       int                    statusCode)
{
    struct MHD_Response* response;
    int                  ret;

#ifdef _MICROHTTPD_DEPRECATED
    response = MHD_create_response_from_data(strlen(xmlGenericResponse),
                                             (void*) xmlGenericResponse,
                                             false,
                                             false);
#else // _MICROHTTPD_DEPRECATED
    response = MHD_create_response_from_buffer(strlen(xmlGenericResponse),
                                               (void*) xmlGenericResponse,
                                               MHD_RESPMEM_PERSISTENT);
#endif // _MICROHTTPD_DEPRECATED
    MHD_add_response_header(response,
                            "Content-Type", "text/xml");
    ret = MHD_queue_response(connection,
                             statusCode,
                             response);
    MHD_destroy_response(response);

    return ret;
}

inline int
sendOptionsResponse(struct MHD_Connection* connection)
{
    struct MHD_Response* response;
    int                  ret;

#ifdef _MICROHTTPD_DEPRECATED
    response = MHD_create_response_from_data(strlen(EMPTY_RESPONSE),
                                             (void*) EMPTY_RESPONSE,
                                             false,
                                             false);
#else // _MICROHTTPD_DEPRECATED
    response = MHD_create_response_from_buffer(strlen(EMPTY_RESPONSE),
                                               (void*) EMPTY_RESPONSE,
                                               MHD_RESPMEM_PERSISTENT);
#endif // _MICROHTTPD_DEPRECATED
    MHD_add_response_header(response,
                            "Content-Type", "text/plain");
    MHD_add_response_header(response,
                            "Access-Control-Allow-Origin", "*");
    MHD_add_response_header(response,
                            "Access-Control-Allow-Methods", "POST, OPTIONS");
    MHD_add_response_header(response,
                            "Access-Control-Allow-Headers", "CONTENT-TYPE");
    MHD_add_response_header(response,
                            "Access-Control-Max-Age", "1728000");
    ret = MHD_queue_response(connection,
                             MHD_HTTP_OK,
                             response);
    MHD_destroy_response(response);

    return ret;
}

#endif // _GENERICRESPONSES_HPP
