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
#ifndef _GETSOCKADDRLOG_HPP
#define _GETSOCKADDRLOG_HPP

/**
  * @brief File containing the getSockAddrLog utility function
  *
  * @file getSockAddrLog.hpp
  * @author Corneliu-Claudiu Prodescu
  * @date 25 Jun 2011
  *
  * License: GPL v3
  */

// System includes

#include <string>                      // C++ string header
#include <sys/socket.h>                // C main socket headers

// Macros
#define TIMESTAMP_MAXBUFSZ       20
#define TIMESTAMP_ERR            "yyyy-mm-dd hh:mm:ss"

/**
  * @brief Method to a log about a socket address.
  * @param sockAddr is the address of the socket.
  * @param sockLen is the length of the address.
  * @return a string with the respective log as "yyyy-mm-dd hostname service".
  */
std::string
getSockAddrLog(const sockaddr* sockAddr,
               const socklen_t sockLen);

#endif // _GETSOCKADDRLOG_HPP
