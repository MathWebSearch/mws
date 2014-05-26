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
 * Implementation of the Crawler Daemon class
 *
 * d.hasegan@jacobs-university.de
 */

// System includes
#include <vector>  // C++ STL libraries
#include <string>
#include <cstdio>  // C I/O libraries
#include <cstring>
#include <cstdlib>
#include <stdint.h>     // C standard integer types
#include <netdb.h>      // C internet database headers
#include <sys/types.h>  // System types
#include <sys/select.h>
#include <sys/socket.h>
#include <microhttpd.h>  // MicroHTTPd library
#include <iostream>

// Local includes
#include "CrawlerDaemon.hpp"
#include "common/utils/compiler_defs.h"

// Config files
#include "build-gen/config.h"

// Namespaces
using namespace std;

// Pages that are sent through the input port
char askpage[MAXASKPAGESIZE];
const char* answerpage =
    "<html><body>We sent the request <br/> You can send another Crawling "
    "Request by clicking <a href=\"/\">here</a>! </body></html>";
const char* errorpage =
    "<html><body>This doesn't seem to be right.</body></html>";

/**
 * @brief: Method that sends a 'page' to the MHD_Connection
 */
static int cd_SendPageThroughConnection(struct MHD_Connection* connection,
                                        const char* page) {
    int ret;
    struct MHD_Response* response;

#ifndef MICROHTTPD_DEPRECATED
    response = MHD_create_response_from_buffer(strlen(page), (void*)page,
                                               MHD_RESPMEM_PERSISTENT);
#else   // MICROHTTPD_DEPRECATED
    response = MHD_create_response_from_data(strlen(page), (void*)page, MHD_NO,
                                             MHD_NO);
#endif  // MICROHTTPD_DEPRECATED
    if (!response) return MHD_NO;

    ret = MHD_queue_response(connection, MHD_HTTP_OK, response);
    MHD_destroy_response(response);

    return ret;
}

/**
 * @brief: method that iterates the key-value pairs of the POST message received
 */
static int cd_IteratePostRequest(void* coninfo_cls, enum MHD_ValueKind kind,
                                 const char* key, const char* filename,
                                 const char* content_type,
                                 const char* transfer_encoding,
                                 const char* data, uint64_t off, size_t size) {
    UNUSED(kind);
    UNUSED(filename);
    UNUSED(content_type);
    UNUSED(transfer_encoding);
    UNUSED(off);

    struct connection_info_struct* con_info =
        (connection_info_struct*)coninfo_cls;

    if (0 == strcmp(key, "urlstart")) {
        if ((size > 0) && (size <= MAXNAMESIZE)) {
            char* urlstart;
            urlstart = (char*)malloc(MAXNAMESIZE);
            if (!urlstart) return MHD_NO;

            strcpy(urlstart, data);
            con_info->urlstart = urlstart;
        } else
            con_info->urlstart = nullptr;

        return MHD_YES;
    }
    if (0 == strcmp(key, "count")) {
        if ((size > 0) && (size <= MAXNAMESIZE)) {
            char* count;
            count = (char*)malloc(MAXNAMESIZE);
            if (!count) return MHD_NO;

            strcpy(count, data);
            con_info->count = count;
        } else
            con_info->count = nullptr;

        return MHD_YES;
    }
    if (0 == strcmp(key, "dontcrawl")) {
        if ((size > 0) && (size <= MAXNAMESIZE)) {
            char* dontcrawl;
            dontcrawl = (char*)malloc(MAXNAMESIZE);
            if (!dontcrawl) return MHD_NO;

            strcpy(dontcrawl, data);
            con_info->dontcrawl[con_info->dontcrawlnr] = dontcrawl;
            con_info->dontcrawlnr++;
        } else
            con_info->dontcrawl[con_info->dontcrawlnr] = nullptr;

        return MHD_YES;
    }
    return MHD_YES;
}

/**
 * @brief: Methos that finalizes the connection and pushes the request on a
 * queue
 */
static void cd_RequestCompletedCallback(void* cls,
                                        struct MHD_Connection* connection,
                                        void** con_cls,
                                        enum MHD_RequestTerminationCode toe) {
    UNUSED(connection);
    UNUSED(toe);

    SharedQueue* sharedQueue = (SharedQueue*)cls;
    QueueNode qNode;
    qNode.urlstart = "";
    qNode.count = qNode.dontcrawlnr = 0;
    int goodRequest = 0;

    struct connection_info_struct* con_info = (connection_info_struct*)*con_cls;

    if (nullptr == con_info) return;

    if (con_info->connectiontype == POST) {
        if (con_info->urlstart != nullptr) {
            qNode.urlstart = con_info->urlstart;
            goodRequest = 1;
        }
        if (con_info->count != nullptr && atoi(con_info->count))
            qNode.count = atoi(con_info->count);
        else
            qNode.count = -1;
        qNode.dontcrawlnr = con_info->dontcrawlnr;
        for (int i = 0; i < con_info->dontcrawlnr; ++i)
            qNode.dontcrawl.push_back((string)con_info->dontcrawl[i]);

        MHD_destroy_post_processor(con_info->postprocessor);
        if (con_info->urlstart) free(con_info->urlstart);
        if (con_info->count) free(con_info->count);
        for (int i = 0; i < con_info->dontcrawlnr; ++i)
            free(con_info->dontcrawl[i]);
        con_info->dontcrawlnr = 0;
    }

    free(con_info);
    *con_cls = nullptr;

    if (goodRequest)  // put Request in Queue
    {
        pthread_mutex_lock(&sharedQueue->lock);
        (*sharedQueue).sharedQueue.push(qNode);
        pthread_mutex_unlock(&sharedQueue->lock);
    }
}

/**
 * @brief: Method called when there is a request on the used port
 *   does something only for GET/POST requests
 */
static int cd_AccessHandlerCallback(void* cls,
                                    struct MHD_Connection* connection,
                                    const char* url, const char* method,
                                    const char* version,
                                    const char* upload_data,
                                    size_t* upload_data_size, void** con_cls) {
    UNUSED(cls);
    UNUSED(url);
    UNUSED(version);

    if (nullptr == *con_cls) {
        struct connection_info_struct* con_info;

        con_info = (connection_info_struct*)malloc(
            sizeof(struct connection_info_struct));
        if (nullptr == con_info) return MHD_NO;
        con_info->urlstart = nullptr;
        con_info->count = nullptr;
        con_info->dontcrawlnr = 0;
        con_info->dontcrawl[0] = nullptr;

        if (0 == strcmp(method, "POST")) {
            con_info->postprocessor = MHD_create_post_processor(
                connection, POSTBUFFERSIZE, cd_IteratePostRequest,
                (void*)con_info);

            if (nullptr == con_info->postprocessor) {
                free(con_info);
                return MHD_NO;
            }

            con_info->connectiontype = POST;
        } else
            con_info->connectiontype = GET;

        *con_cls = (void*)con_info;

        return MHD_YES;
    }

    if (0 == strcmp(method, "GET")) {
        return cd_SendPageThroughConnection(connection, askpage);
    }

    if (0 == strcmp(method, "POST")) {
        struct connection_info_struct* con_info =
            (connection_info_struct*)*con_cls;

        if (*upload_data_size != 0) {
            MHD_post_process(con_info->postprocessor, upload_data,
                             *upload_data_size);
            *upload_data_size = 0;

            return MHD_YES;
        } else if (con_info->urlstart != nullptr) {
            return cd_SendPageThroughConnection(connection, answerpage);
        }
    }

    return cd_SendPageThroughConnection(connection, errorpage);
}

/**
 * @brief: Contructor method of the Crawler Daemon class
 */
CrawlerDaemon::CrawlerDaemon() {
    _daemonHandler = nullptr;
    sharedQueue = nullptr;

    FILE* getpg = fopen(GETRESPONSE_PATH, "r");
    fread(askpage, 1, MAXASKPAGESIZE, getpg);
    fclose(getpg);
}

/**
 * @brief: Destructor method of the Crawler Daemon class
 */
CrawlerDaemon::~CrawlerDaemon() {
    if (_daemonHandler != nullptr) {
        MHD_stop_daemon(_daemonHandler);
    }
}

/**
 * @brief: The start method of the Crawler Daemon Class
 *    starts the connection through the 'aPort' port
 */
int CrawlerDaemon::start(int aPort, SharedQueue* sqPtr) {
    std::cout << "Starting the daemon: " << std::endl;
    int status;

    sharedQueue = sqPtr;

    _daemonHandler = MHD_start_daemon(
        MHD_USE_SELECT_INTERNALLY, aPort,
        nullptr,  // TODO:      my_MHD_AcceptPolicyCallback,
        nullptr, cd_AccessHandlerCallback,  // ... my_MHD_AccessHandlerCallback,
        nullptr, MHD_OPTION_CONNECTION_LIMIT, 20, MHD_OPTION_NOTIFY_COMPLETED,
        cd_RequestCompletedCallback, (void*)sqPtr, MHD_OPTION_END);
    if (_daemonHandler == nullptr) {
        status = -1;
    } else {
        status = 0;
    }

    return status;
}

/**
 * @brief: Stops the Crawler Daemon Connection
 */
void CrawlerDaemon::stop() {
    if (_daemonHandler != nullptr) {
        MHD_stop_daemon(_daemonHandler);
        _daemonHandler = nullptr;
    }
}
