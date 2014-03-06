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
 * @brief   Memory map utilities
 * @file    mmap.c
 * @date    04 Feb 2013
 *
 * License: GPLv3
 */

#include <fcntl.h>
#include <stdio.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

#include "common/utils/compiler_defs.h"

#include "mmap.h"

/*--------------------------------------------------------------------------*/
/* Implementation                                                           */
/*--------------------------------------------------------------------------*/

int mmap_create(const char* path, off_t size, int flags, mmap_handle_t* mmap_handle) {
    int fd = -1;
    char* mapped_region;
    int oflags;
    int prot;

    /* setting open/mmap flags */
    prot   = PROT_READ | PROT_WRITE;
    oflags = O_RDWR | O_CREAT | O_EXCL;

    /* opening file */
    FAIL_ON((fd = open(path, oflags, S_IRUSR | S_IWUSR)) < 0);

    /* set file size by seek and write */
    FAIL_ON(lseek(fd, size - 1, SEEK_SET) == (off_t)-1);
    FAIL_ON(write(fd, " ", 1) != 1);

    /* memory-map the file */
    mapped_region = mmap(/* addr   = */ NULL, size, prot, flags, fd,
                         /* offset = */ 0);
    FAIL_ON(mapped_region == MAP_FAILED);

    /* copy to mmap_handle */
    mmap_handle->path = path;
    mmap_handle->start_addr = mapped_region;
    mmap_handle->size = size;

    /* close file descriptor */
    (void) close(fd);

    return 0;

fail:
    if (fd >= 0) (void) close(fd);
    return -1;
}

int mmap_load(const char* path, int flags, mmap_handle_t* mmap_handle) {
    int fd = -1;
    struct stat s;
    off_t size;
    char* mapped_region;
    int oflags;
    int prot;

    /* setting open/mmap flags */
    prot   = PROT_READ;
    oflags = O_RDONLY;

    /* opening file */
    FAIL_ON((fd = open(path, oflags)) < 0);

    /* get file size */
    FAIL_ON(fstat(fd, &s) < 0);
    size = s.st_size;

    /* memory-map the file */
    mapped_region = mmap(/* addr   = */ NULL, size, prot, flags, fd,
                         /* offset = */ 0);
    FAIL_ON(mapped_region == MAP_FAILED);

    /* copy to mmap_handle */
    mmap_handle->path = path;
    mmap_handle->start_addr = mapped_region;
    mmap_handle->size = size;

    /* close file descriptor */
    (void) close(fd);

    return 0;

fail:
    if (fd >= 0) (void) close(fd);
    return -1;
}

int mmap_unload(mmap_handle_t* mmap_handle) {
    FAIL_ON(munmap(mmap_handle->start_addr, mmap_handle->size) != 0);
    return 0;

fail:
    return  -1;
}

int mmap_remove(mmap_handle_t* mmap_handle) {
    FAIL_ON(unlink(mmap_handle->path) != 0);
    return 0;

fail:
    return  -1;
}
