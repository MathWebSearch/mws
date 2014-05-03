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
 * @file save_pid_file.c
 * @brief save_pid_file implementation
 */

#define __STDC_FORMAT_MACROS  // ensure inttypes.h defines PRIu64
#include <inttypes.h>
#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>

#include "common/utils/compiler_defs.h"

#include "save_pid_file.h"

BEGIN_DECLS

int
save_pid_file(const char *path) {
    FILE* fp = fopen(path, "w");
    FAIL_ON(fp == NULL);

    fprintf(fp, "%"PRIu64, (uint64_t) getpid());

    return fclose(fp);

fail:
    return -1;
}

END_DECLS
