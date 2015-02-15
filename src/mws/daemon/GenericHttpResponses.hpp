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
#ifndef _GENERICHTTPRESPONSES_HPP
#define _GENERICHTTPRESPONSES_HPP

/**
  * @brief File containing generic responses for the RESTful interface.
  * @file GenericHttpResponses.hpp
  * @author Corneliu-Claudiu Prodescu
  * @date 31 Jul 2011
  *
  * License: GPL v3
  *
  */

#include "mws/daemon/microhttpd_linux.h"

const char XML_MWS_BAD_QUERY[] =
    "<?xml version=\"1.0\"?>\n"
    "<mws:info xmlns:mws=\"http://search.mathweb.org/ns\">"
    "Bad query request</mws:info>";

const char XML_MWS_SERVER_ERROR[] =
    "<?xml version=\"1.0\"?>\n"
    "<mws:info xmlns:mws=\"http://search.mathweb.org/ns\">"
    "Server error</mws:info>";

const char EMPTY_RESPONSE[] = "";

inline int sendXmlGenericResponse(struct MHD_Connection* connection,
                                  const char* xmlGenericResponse,
                                  int statusCode) {
    struct MHD_Response* response;
    int ret;

#ifdef MICROHTTPD_DEPRECATED
    response = MHD_create_response_from_data(strlen(xmlGenericResponse),
                                             (void*)xmlGenericResponse,
                                             /* must_free = */ 0,
                                             /* must_copy = */ 0);
#else  // MICROHTTPD_DEPRECATED
    response = MHD_create_response_from_buffer(strlen(xmlGenericResponse),
                                               (void*)xmlGenericResponse,
                                               MHD_RESPMEM_PERSISTENT);
#endif  // MICROHTTPD_DEPRECATED
    MHD_add_response_header(response, "Content-Type", "text/xml");
    ret = MHD_queue_response(connection, statusCode, response);
    MHD_destroy_response(response);

    return ret;
}

inline int sendOptionsResponse(struct MHD_Connection* connection) {
    struct MHD_Response* response;
    int ret;

#ifdef MICROHTTPD_DEPRECATED
    response = MHD_create_response_from_data(
        strlen(EMPTY_RESPONSE), (void*)EMPTY_RESPONSE, false, false);
#else  // MICROHTTPD_DEPRECATED
    response = MHD_create_response_from_buffer(
        strlen(EMPTY_RESPONSE), (void*)EMPTY_RESPONSE, MHD_RESPMEM_PERSISTENT);
#endif  // MICROHTTPD_DEPRECATED
    MHD_add_response_header(response, "Content-Type", "text/plain");
    MHD_add_response_header(response, "Access-Control-Allow-Origin", "*");
    MHD_add_response_header(response, "Access-Control-Allow-Methods",
                            "POST, OPTIONS");
    MHD_add_response_header(response, "Access-Control-Allow-Headers",
                            "CONTENT-TYPE");
    MHD_add_response_header(response, "Access-Control-Max-Age", "1728000");
    ret = MHD_queue_response(connection, MHD_HTTP_OK, response);
    MHD_destroy_response(response);

    return ret;
}

#endif  // _GENERICHTTPRESPONSES_HPP
