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
  * @brief File containing the getSockAddrLog utility function
  *
  * @file getSockAddrLog.cpp
  * @author Corneliu-Claudiu Prodescu
  * @date 25 Jun 2011
  *
  * License: GPL v3
  */

// System includes

#include <cstdio>                      // C standard IO headers
#include <netdb.h>                     // C network db operations
#include <string>                      // C++ string header
#include <sys/socket.h>                // C main socket headers

// Local includes

#include "common/utils/TimeStamp.hpp"     // MWS TimeStamp utility function

// Macros

#define RESULT_MAX_SZ      81
#define HOSTNAME_MAX_SZ    40
#define SERVICE_MAX_SZ     40

// Namespaces

using namespace std;


string
getSockAddrLog(const sockaddr* sockAddr,
               const socklen_t sockLen)
{
    char   hostname[HOSTNAME_MAX_SZ];
    char   service[SERVICE_MAX_SZ];
    int    ret;
    char   result[RESULT_MAX_SZ];

    // Getting hostname and service
    ret = getnameinfo(sockAddr, sockLen,
                      hostname, HOSTNAME_MAX_SZ,
                      service,  SERVICE_MAX_SZ,
                      0);
    if (ret != 0)
    {
        fprintf(stderr, "Error at getnameinfo: %s\n",
                gai_strerror(ret));
    }
    else
    {
        snprintf(result,
                 RESULT_MAX_SZ,
                 "%19s "              "%35s"    "%25s\n",
                 TimeStamp().c_str(), hostname, service);
    }

    return (string) result;
}
