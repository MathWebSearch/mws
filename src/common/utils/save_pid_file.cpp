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
#include <sys/types.h>
#include <unistd.h>
#include <stdio.h>

#include "save_pid_file.h"

extern "C" {

int
save_pid_file(const char *path) {
    FILE* fp = fopen(path, "w");
    // TODO FAIL_ON
    if (fp == NULL) {
        goto fail;
    }

    fprintf(fp, "%d\n", (int) getpid());

    return fclose(fp);

fail:
    return -1;
}

}
