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
 * @file    OutSocket.cpp
 * @brief   Client socket implementation
 * @author  Corneliu-Claudiu Prodescu <c.prodescu@jacobs-university.de>
 * @date    2012-02-05
 *
 * License: GPLv3
 */

/****************************************************************************/
/* Includes                                                                 */
/****************************************************************************/

#include <string.h>                     // Mem-related functions: memcpy
#include <sys/socket.h>                 // ISO C Socket library
#include <sys/types.h>                  // Datatypes: size_t, socklen_t
#include <unistd.h>                     // write()
#include <netdb.h>                      // struct hostent, gethostbyname
#include <cerrno>                       // C errno codes
#include <unistd.h>

#include <cstdio>                       // C standard I/O
#include "OutSocket.hpp"                // OutSocket class definition

/****************************************************************************/
/* Constants                                                                */
/****************************************************************************/

# define    SOCKET_DEFAULT_HOST "localhost"

# ifdef NI_MAXHOST
#   define  MAX_HOSTNAME_LEN    NI_MAXHOST
# else
#   define  MAX_HOSTNAME_LEN    255
# endif

# ifdef NI_MAXSERV
#   define  MAX_SERVICE_LEN     NI_MAXSERV
# else
#   define  MAX_SERVICE_LEN     32
# endif

# define    SOCKET_INFO_UNKNOWN "unknown"

/****************************************************************************/
/* Implementation                                                           */
/****************************************************************************/

using namespace std;


OutSocket::OutSocket() :
        _outsideLen ( sizeof(sockaddr_in) ),
        _isOpen ( false )
{
}


OutSocket::OutSocket(string aHost,
                     int    aPort) :
        _outsideLen ( sizeof(sockaddr_in) ),
        _host   ( aHost ),
        _port   ( aPort ),
        _isOpen ( false )
{
}


OutSocket::OutSocket(int aPort) :
        _outsideLen ( sizeof(sockaddr_in) ),
        _host   ( SOCKET_DEFAULT_HOST ),
        _port   ( aPort ),
        _isOpen ( false )
{
}


OutSocket::~OutSocket()
{
    if (this->_isOpen)
    {
        int ret;
    
        ret = close(this->_fd);
        if (ret == -1)
        {
            perror("close:");
        }
    }
}


int OutSocket::enable()
{
    int ret;
    struct hostent *server;

    server = gethostbyname(this->_host.c_str());
    if (server == NULL) goto fail;

    // TODO use getaddrinfo()
    this->_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (this->_fd == -1)
    {
        perror("socket");
        goto fail;
    }
    else
    {
        this->_isOpen = true;
    }

    this->_outsideAddr.sin_family = AF_INET;
    memcpy(&this->_outsideAddr.sin_addr.s_addr,
           server->h_addr,
           server->h_length);
    this->_outsideAddr.sin_port = htons(this->_port);
    // Open _fd
    ret = connect(this->_fd,
                  (sockaddr*) &this->_outsideAddr, this->_outsideLen);
    if (ret == -1) goto fail;

    return 0;

fail:
    fprintf(stderr, "%s failed\n", __func__);
    if (this->_isOpen)
    {
        close(this->_fd);
        this->_isOpen = false;
    }
    return -1;
}



int OutSocket::write(const void* pAddr, int nrBytes)
{
    int ret;
    int len;

    if (!this->_isOpen) goto fail;

    ret = 1; // dummy positive value
    len = 0;
    while (len < nrBytes && ret > 0)
    {
        ret = ::write(this->_fd, (const char*)pAddr + len, nrBytes - len);
        len += ret;
    }
    if (ret == -1) goto fail;

    return len;

fail:
    fprintf(stderr, "%s failed\n", __func__);
    return -1;
}


int OutSocket::read(void* pAddr, int nrBytes)
{
    int ret;
    int len;

    if (!this->_isOpen) goto fail;

    ret = 1; // dummy positive value
    len = 0;
    while (len < nrBytes && ret > 0)
    {
        ret = ::read(this->_fd, (char*)pAddr + len, nrBytes - len);
        len += ret;
    }
    if (ret == -1) goto fail;

    return len;

fail:
    fprintf(stderr, "%s failed\n", __func__);
    return -1;
}


int OutSocket::getFd() const
{
    if (!this->_isOpen) goto fail;

    return this->_fd;
fail:
    fprintf(stderr, "%s failed\n", __func__);
    return -1;
}


SocketInfo OutSocket::getInfo() const
{
    SocketInfo info;
    char       hostname[MAX_HOSTNAME_LEN];
    char       service[MAX_SERVICE_LEN];
    int        ret;

    if (!this->_isOpen) goto fail;

    ret = getnameinfo((const sockaddr*)&this->_outsideAddr, this->_outsideLen,
                      hostname, MAX_HOSTNAME_LEN,
                      service, MAX_SERVICE_LEN,
                      0);
    if (ret != 0)
    {
        perror("getnameinfo");
        goto fail;
    }

    info.hostname = hostname;
    info.service  = service;

    return info;

fail:
    fprintf(stderr, "%s failed\n", __func__);
    info.hostname = SOCKET_INFO_UNKNOWN;
    info.service  = SOCKET_INFO_UNKNOWN;

    return info;
}


OutSocket* buildAcceptedConnection(int aFd)
{
    OutSocket* outsock;
    
    outsock = new OutSocket();
    outsock->_fd = accept(aFd, (struct sockaddr*)   &outsock->_outsideAddr,
                               (socklen_t*)         &outsock->_outsideLen);
    if (outsock->_fd < 0) goto fail;

    outsock->_isOpen = true;

    return outsock;

fail:
    fprintf(stderr, "%s failed\n", __func__);
    delete outsock;
    return NULL;
}
