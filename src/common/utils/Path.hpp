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
  * @file   Path.hpp
  * @brief  Simple path class
  *
  * @date   17 Jan 2012
  * @author Corneliu-Claudiu Prodescu
  *
  * @bug    No known bugs
  *
  * License: GPL v3
  */

#ifndef _MWS_PATH_HPP
#define _MWS_PATH_HPP

/****************************************************************************/
/* System Includes                                                          */
/****************************************************************************/

#include <string>

#if defined __linux__
#  include <linux/limits.h>
#  define MWS_PATH_MAX PATH_MAX
#elif defined __APPLE__
#  include <limits.h>
#  define MWS_PATH_MAX PATH_MAX
#else
#  define MWS_PATH_MAX 4096
#endif

/****************************************************************************/
/* Constants                                                                */
/****************************************************************************/

#define MWS_PATH_MAX            PATH_MAX
#define PATH_SEPARATOR          '/'

/****************************************************************************/
/* Local Methods                                                            */
/****************************************************************************/

static int __safe_strncpy(char* dest, const char* src, int maxlen);

/****************************************************************************/
/* Implementation                                                           */
/****************************************************************************/

namespace mws
{

class Path
{
protected:
    char str[MWS_PATH_MAX];
    unsigned int len;

public:
    Path() :
        len (   0   )
    {
        str[0] = '\0';
    }

    explicit Path(const std::string& aStr)
    {
        this->len = __safe_strncpy(this->str, aStr.c_str(), MWS_PATH_MAX);
    }

    ~Path()
    {
    }

    void set(const std::string& aStr)
    {
        this->len = __safe_strncpy(this->str, aStr.c_str(), MWS_PATH_MAX);
    }

    const char* get() const
    {
        return this->str;
    }
};

class RelPath : public Path
{
public:
    RelPath() : Path() { }
    RelPath(const std::string& aStr) : Path(aStr) { }

    void append(const RelPath& aPath)
    {
        if (str[len-1] != PATH_SEPARATOR)
        {
            str[len] = PATH_SEPARATOR;
            len++;
        }

        len += __safe_strncpy(this->str + len, aPath.get(), MWS_PATH_MAX - len);
    }
};

class AbsPath : public Path
{
public:
    AbsPath() : Path() { }
    AbsPath(const std::string& aStr) : Path(aStr) { }

    void append(const RelPath& aPath)
    {
        if (str[len-1] != PATH_SEPARATOR)
        {
            str[len] = PATH_SEPARATOR;
            len++;
        }

        len += __safe_strncpy(this->str + len, aPath.get(), MWS_PATH_MAX - len);
    }
};

}

/****************************************************************************/
/* Local Implementation                                                     */
/****************************************************************************/

/**
 * @brief cstring strncpy() with NULL termination in overflow case
 *
 * @return Number of characters copied.
 */
static int __safe_strncpy(char* dest, const char* src, int maxlen)
{
    int i;

    // Ensuring we always stop before the buffer runs out
    maxlen--;
    
    for (i = 0 ; i < maxlen && src[i] != '\0' ; ++i)
        dest[i] = src[i];

    dest[i] = '\0';
    
    return i;
}

#endif // _MWS_PATH_HPP
