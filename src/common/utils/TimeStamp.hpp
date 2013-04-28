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
#ifndef _TIMESTAMP_HPP
#define _TIMESTAMP_HPP

/**
  * @brief File containing the TimeStamp utility function
  *
  * @file TimeStamp.hpp
  * @author Corneliu-Claudiu Prodescu
  * @date 22 Jun 2011
  *
  * License: GPL v3
  */

// System includes

#include <string>                      // C++ string header
#include <cstdio>                      // C standard IO header
#include <ctime>                       // C time headers

// Macros
#define TIMESTAMP_MAXBUFSZ       20
#define TIMESTAMP_ERR            "yyyy-mm-dd hh:mm:ss"

/**
  * @brief Method to get the current time.
  * @return a string with the current GMT time as "yyyy-mm-dd hh:mm:ss".
  */
inline std::string
TimeStamp()
{
    char      buffer[TIMESTAMP_MAXBUFSZ];
    time_t    currentRawTime;
    time_t    ret;
    struct tm gmtTime;

    ret = time(&currentRawTime);
    if (ret == (time_t)-1)
    {
        snprintf(buffer,
                 TIMESTAMP_MAXBUFSZ,
                 "%20s",
                 TIMESTAMP_ERR);
    }
    else
    {
        (void) gmtime_r(&currentRawTime, &gmtTime);

        snprintf(buffer,
                 TIMESTAMP_MAXBUFSZ,
                 "%04d-%02d-%02d %02d:%02d:%02d",
                 gmtTime.tm_year + 1900,
                 gmtTime.tm_mon  + 1,
                 gmtTime.tm_mday,
                 gmtTime.tm_hour,
                 gmtTime.tm_min,
                 gmtTime.tm_sec);
    }

    return (std::string) buffer;
}

#endif // _TIMESTAMP_HPP
