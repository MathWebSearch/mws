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
 * @file    InSocket.cpp
 * @brief   Server socket implementation
 * @author  Corneliu-Claudiu Prodescu <c.prodescu@jacobs-university.de>
 * @date    2012-02-05
 *
 * License: GPLv3
 */

/****************************************************************************/
/* Includes                                                                 */
/****************************************************************************/

#include <netinet/in.h>                 // Domain, Protocol standards
#include <iostream>                     // Standard input/output stream
#include <sys/socket.h>                 // ISO C Socket library
#include <unistd.h>                     // write()
#include <cerrno>                       // C errno codes
#include <unistd.h>
#include <cstdio>                       // C standard IO headers
#include <cstring>                      // C string -- memset()
#include "InSocket.hpp"                 // InSocket prototypes

/****************************************************************************/
/* Implementation                                                           */
/****************************************************************************/

using namespace std;

InSocket::InSocket(int aPort,
                   int aQueueSize) :
        _port       ( aPort ),
        _queueSize  ( aQueueSize ),
        _isOpen     ( false )
{
}


InSocket::~InSocket()
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


int InSocket::enable()
{
    int ret;
    int optval_true = true;     // integer True Value - see setsockopt (2)

    this->_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (this->_fd == -1)
    {
        perror("socket");
        goto fail;
    }

    memset(&this->_insideAddr, 0, sizeof(_SockAddr));
    this->_insideAddr.sin_family      = AF_INET;
    this->_insideAddr.sin_addr.s_addr = INADDR_ANY;
    this->_insideAddr.sin_port        = htons(this->_port);

    ret = setsockopt(this->_fd, SOL_SOCKET,
                     SO_REUSEADDR, (const void *)&optval_true, sizeof(int));
    if (ret == -1)
    {
        perror("setsockopt");
        goto fail;
    }

    ret = ::bind(this->_fd,
               (struct sockaddr*)&this->_insideAddr,
               sizeof(this->_insideAddr));
    if (ret == -1)
    {
        perror("bind");
        goto fail;
    }

    ret = listen(this->_fd, this->_queueSize);
    if (ret == -1)
    {
        perror("bind");
        goto fail;
    }

    return 0;

fail:
    fprintf(stderr, "%s failed\n", __func__);
    if (_fd > 0) close(_fd);
    return -1;
}


OutSocket* InSocket::accept()
{
    return buildAcceptedConnection(this->_fd);
}
