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
#ifndef _DEBUGMACROS_HPP
#define _DEBUGMACROS_HPP

/**
  * @brief File containing debug macros used around the MathWebSearch
  * project.
  *
  * @file DebugMacros.hpp
  * @author Corneliu-Claudiu Prodescu
  * @date 25 Apr 2011
  *
  * License: GPL v3
  *
  */


// Log function / method entry point
#define LOG_TRACE_IN \
    fprintf(stderr, " * %20s | L %5d | --> %s\n",\
            __FILE__, \
            __LINE__, \
            __func__)

// Log function / method exit point
#define LOG_TRACE_OUT \
    fprintf(stderr, " * %20s | L %5d | <-- %s\n",\
            __FILE__, \
            __LINE__, \
            __func__)

// Print a warning
#define PRINT_WARN(x, ...) \
    fprintf(stderr, "W: %20s | L %5d | " x, \
            __FILE__, \
            __LINE__, \
            __VA_ARGS__)


#endif // _DEBUGMACROS_HPP
