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
 * @brief complete MicroHTTPd headers for GNU/Linux
 *
 * @file microhttpd_linux.h
 *
 * @author Corneliu-Claudiu Prodescu <c.prodescu@jacobs-university.de>
 * @date 2012-02-08
 *
 * License: GPL v3
 */

#ifndef _MICROHTTPD_LINUX_H
#define _MICROHTTPD_LINUX_H

// Headers needed to be included before MicroHTTPDd native header

#include <stdarg.h>      // C variable argument list
#include <stdint.h>      // C standard integer types
#include <sys/select.h>  // C select API
#include <sys/socket.h>  // C sockets API

// MicroHTTPd native header

#include <microhttpd.h>  // MicroHTTPd library headers

#endif  // ! _MICROHTTPD_LINUX_H
