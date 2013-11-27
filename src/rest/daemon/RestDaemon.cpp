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
  * @brief File containing the implementation of the RestDaemon class.
  * @file RestDaemon.cpp
  * @author Corneliu-Claudiu Prodescu <c.prodescu@jacobs-university.de>
  * @date 28 May 2011
  *
  * License: GPL v3
  *
  */

// System includes

#include <stdint.h>                    // C standard integer types
#include <sys/types.h>                 // C depricated types
#include <sys/socket.h>                // C sockets API
#include <netdb.h>                     // C internet database headers
#include <stdlib.h>                    // C general purpose library
#include <string.h>                    // C string library
#include <stdio.h>

// Local includes

#include "microhttpd_linux.h"               // MicroHTTPd library headers
#include "common/utils/getSockAddrLog.hpp"  // Socket Address logging function
#include "common/utils/macro_func.h"        // Utility Macro Functions
#include "common/utils/DebugMacros.hpp"     // Utility Debug Macros
#include "common/utils/ToString.hpp"        // Utility ToString function

#include "rest/daemon/RestDaemon.hpp"       // RestDaemon class header
#include "rest/daemon/GenericResponses.hpp" // Generic responses collection
#include "rest/types/MwsConnection.hpp"     // Mws Connection datatype

#include "config.h"

// Namespaces

using namespace std;


static int mwsPort;
static string mwsHost;

static void
my_MHD_RequestCompletedCallback(void*                      cls,
                                struct MHD_Connection*     connection,
                                void**                     ptr,
                                MHD_RequestTerminationCode status)
{
    UNUSED( cls );
    UNUSED( status );

    const union MHD_ConnectionInfo* connInfo;
    MwsConnection*                  mwsConn;
    const struct sockaddr*          clientAddr;

    mwsConn = (MwsConnection*) *ptr;

    connInfo = MHD_get_connection_info(connection,
                                       MHD_CONNECTION_INFO_CLIENT_ADDRESS);
#ifdef _MICROHTTPD_DEPRECATED
    /*
     * Older versions supported IPv4 exclusively, so client_addr
     * was a specific type: struct sockadrr_in.
     */
#define CLIENT_ADDR_SIZE sizeof(struct sockaddr_in)
    clientAddr = (const struct sockaddr*)connInfo->client_addr;
#else // _MICROHTTPD_DEPRECATED
#define CLIENT_ADDR_SIZE sizeof(struct sockaddr)
    clientAddr = connInfo->client_addr;
#endif // _MICROHTTPD_DEPRECATED

    if (mwsConn)
    {
        printf("%s\n :: %s\n",
               getSockAddrLog(clientAddr, CLIENT_ADDR_SIZE).c_str(),
               mwsConn->getInfo().c_str());

        delete mwsConn;
    }
    else
    {
        printf("%s\n :: Bad request method\n",
               getSockAddrLog(clientAddr, CLIENT_ADDR_SIZE).c_str());
    }

    fflush(stdout);
}


#ifdef _MICROHTTPD_DEPRECATED
static int
my_MHD_ContentReaderCallback(void*    cls,
                             uint64_t pos,
                             char*    buf,
                             int      max)
#else // _MICROHTTPD_DEPRECATED
static ssize_t
my_MHD_ContentReaderCallback(void*    cls,
                             uint64_t pos,
                             char*    buf,
                             size_t   max)
#endif // _MICROHTTPD_DEPRECATED
{
    UNUSED( pos ); // Not necessary if responses aren't reused

    MwsConnection* data;
    ssize_t        result;

    data = (MwsConnection*) cls;
    result = data->read(buf, max);

    if (result <= 0)
    {
#ifdef _MICROHTTPD_DEPRECATED
        /*
         * Older versions of MHD did not make a difference between end of
         * transmission or error
         */
        result = -1;
#else // _MICROHTTPD_DEPRECATED
        if (result == 0)
            result = MHD_CONTENT_READER_END_OF_STREAM;
        else
            result = MHD_CONTENT_READER_END_WITH_ERROR;
#endif // _MICROHTTPD_DEPRECATED
    }

    return result;
}


static int
my_MHD_AcceptPolicyCallback(void* cls,
                            const sockaddr* addr,
                            socklen_t addrlen)
{
#ifdef TRACE_FUNC_CALLS
    LOG_TRACE_IN;
#endif
    UNUSED( cls );
    UNUSED( addr );
    UNUSED( addrlen );

    // Accepting everything

#ifdef TRACE_FUNC_CALLS
    LOG_TRACE_OUT;
#endif
    return MHD_YES;
}


static int
my_MHD_AccessHandlerCallback(void*                  cls,
                             struct MHD_Connection* connection,
                             const char*            url,
                             const char*            method,
                             const char*            version,
                             const char*            upload_data,
                             size_t*                upload_data_size,
                             void**                 ptr)
{
#ifdef TRACE_FUNC_CALLS
    LOG_TRACE_IN;
#endif
    UNUSED( cls );
    UNUSED( url );
    UNUSED( version );

    struct MHD_Response* response;
    int ret;
    size_t nbytes;
    MwsConnection* mwsConn;

    // On OPTIONS method request different behavior
    if (0 == strcmp(method, MHD_HTTP_METHOD_OPTIONS))
    {
        ret = sendOptionsResponse(connection);
    }
    // Checking if the method is supported
    else if (0 != strcmp(method, MHD_HTTP_METHOD_POST))
    {
        ret = MHD_NO;
    }
    // Checking if initialization needed
    else if (*ptr == NULL)
    {
        // Opening a socket and connecting to MWS
        *ptr = new MwsConnection(mwsHost, mwsPort);
        mwsConn = (MwsConnection*)*ptr;

        mwsConn->connect();
        
        ret = MHD_YES;
    }
    // Data to be processed
    else if (*upload_data_size)
    {
        mwsConn = (MwsConnection*)*ptr;
        
        while (*upload_data_size)
        {
            nbytes = mwsConn->write(upload_data,
                                    *upload_data_size);
            if (nbytes <= 0)
            {
                ret = MHD_NO;
                break;
            }
                                    
            upload_data       += nbytes;
            *upload_data_size -= nbytes;
        }

        ret = MHD_YES;
    }
    // If we are here, all post data has been processed
    else
    {
        mwsConn = (MwsConnection*)*ptr;

        // Checking if MWS was reached
        if (!mwsConn->isConnected())
        {
            ret = sendXmlGenericResponse(connection,
                                         XML_MWS_UNREACHABLE,
                                         MHD_HTTP_SERVICE_UNAVAILABLE);
        }
        // Signaling the end of data and receiving a control sequence
        // for the parsed data
        else if (mwsConn->signalEnd() == -1)
        {
            ret = sendXmlGenericResponse(connection,
                                         XML_MWS_INTERNALERROR,
                                         MHD_HTTP_INTERNAL_SERVER_ERROR);
        }
        // Checking if query parsing was ok
        else if (!mwsConn->isParsed())
        {
            ret = sendXmlGenericResponse(connection,
                                         XML_MWS_BADQUERY,
                                         MHD_HTTP_BAD_REQUEST);
        }
        // Retrieving the answer and sending it
        else
        {
            response = MHD_create_response_from_callback(-1,// Unknown size
                                                1024,   // Prefered block size
                                                my_MHD_ContentReaderCallback,
                                                mwsConn,
                                                NULL);
            switch (mwsConn->getOutputFormat())
            {
            case DATAFORMAT_XML:
                MHD_add_response_header(response, 
                        "Content-Type", "text/xml");
                break;
            case DATAFORMAT_JSON:
                MHD_add_response_header(response, 
                        "Content-Type", "application/json");
                break;
            default:
                MHD_add_response_header(response, 
                        "Content-Type", "text/xml");
                break;
            }
            MHD_add_response_header(response,
                            "Access-Control-Allow-Origin", "*");
            MHD_add_response_header(response, 
                            "Cache-Control", "no-cache, must-revalidate");

            ret = MHD_queue_response(connection, MHD_HTTP_OK, response);
            MHD_destroy_response(response);
        }
    }

#ifdef TRACE_FUNC_CALLS
    LOG_TRACE_OUT;
#endif
    return ret;
}


RestDaemon::RestDaemon()
{
    _daemonHandler = NULL;
}


RestDaemon::~RestDaemon()
{
    if (_daemonHandler != NULL)
    {
        MHD_stop_daemon(_daemonHandler);
    }
}


int
RestDaemon::startAsync(const Config& config)
{
    int status;

    mwsPort = config.mwsPort;
    mwsHost = config.mwsHost;

    _daemonHandler = MHD_start_daemon(MHD_USE_THREAD_PER_CONNECTION,
                                      config.restPort,
                                      my_MHD_AcceptPolicyCallback,
                                          NULL,
                                      my_MHD_AccessHandlerCallback,
                                          NULL,
                                      MHD_OPTION_CONNECTION_LIMIT,
                                          20,
                                      MHD_OPTION_NOTIFY_COMPLETED,
                                          my_MHD_RequestCompletedCallback,
                                          NULL,
                                      MHD_OPTION_END);
    if (_daemonHandler == NULL)
    {
        status = -1;
    }
    else
    {
        status = 0;
    }

    return status;
}


void
RestDaemon::stop()
{
    if (_daemonHandler != NULL)
    {
        MHD_stop_daemon(_daemonHandler);
        _daemonHandler = NULL;
    }
}
