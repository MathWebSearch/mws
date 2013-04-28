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
 *
 *
 */

#include <unistd.h>
#include <errno.h>

#include "common/utils/macro_func.h"
#include "common/utils/mmap.h"


#define TMPFILE_PATH    "/tmp/test.map"
#define TMPFILE_SIZE    4 * 1024


int main() {
    mmap_handle_t m;

    /* ensure the file does not exist */
    FAIL_ON(unlink(TMPFILE_PATH) != 0 && errno != ENOENT);

    /* create and map read-write */
    FAIL_ON(mmap_create(TMPFILE_PATH, TMPFILE_SIZE, MAP_PRIVATE,  &m) != 0);

    /* unmap */
    FAIL_ON(mmap_unload(&m) != 0);

    /* map read-only */
    FAIL_ON(mmap_load(TMPFILE_PATH, MAP_PRIVATE,  &m) != 0);

    /* remove mmapped file */
    FAIL_ON(mmap_remove(&m) != 0);

    return 0;

fail:
    return -1;
}
