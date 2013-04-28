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
 * @brief   Compiler Utilities
 * @file    compiler_utils.h
 * @author  cprodescu
 * @bugs    No known bugs.
 *
 */

#ifndef _COMMON_UTILS_COMPILER_DEFS_H
#define _COMMON_UTILS_COMPILER_DEFS_H


/* BEGIN_DECLS */
#ifdef __cplusplus
#  define BEGIN_DECLS extern "C" {
#else
#  define BEGIN_DECLS /* empty */
#endif

/* END_DECLS */
#ifdef __cplusplus
#  define END_DECLS   }
#else
#  define END_DECLS   /* empty */
#endif

/* THREAD_LOCAL */
#ifdef __cplusplus
#   define THREAD_LOCAL thread_local
#else
#   define THREAD_LOCAL __thread
#endif

/* RESTRICT */
#ifdef __GNUC__
#   define RESTRICT __restrict  /* GNU specific, c/c++ */
#else
#   define RESTRICT restrict    /* C99+ std conformant */
#endif

/* PACKED */
#define PACKED      __attribute__ ((__packed__)) // GNU specific

#define STATIC_ASSERT(expr, msg) \
    _Static_assert(expr, msg)   // GNU specific

#define BREAKPOINT  asm("int $3"); // GNU specific

#endif // ! _COMMON_UTILS_COMPILER_DEFS_H
