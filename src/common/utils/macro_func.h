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
#ifndef _MACRO_FUNC_H
#define _MACRO_FUNC_H

/**
  * @brief Utility Macros
  *
  * @file macro_func.h
  * @author Corneliu-Claudiu Prodescu
  * @date 24 May 2011
  *
  * License: GPL v3
  */

#include <stdio.h>


#define QUOTEMACRO_(x) #x
/// Method to quote a macro
#define QUOTEMACRO(x) QUOTEMACRO_(x)

/// Method to notify unused variables
#define UNUSED(x) do { (void)(x); } while (0);

/// Print test and jump to fail label
#define FAIL_ON(x) do {                                                     \
        if (x) {                                                            \
            fprintf(stderr, "%s:%d: "#x"\n",                                \
                    __FILE__, __LINE__);                                    \
            goto fail;                                                      \
        }                                                                   \
    } while(0)


#endif // _MACRO_FUNC_H
