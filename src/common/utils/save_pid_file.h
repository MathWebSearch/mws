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
 * @file save_pid_file.h
 * @brief save_pid_file API
 */
#ifndef __COMMON_UTILS_SAVE_PID_FILE_H
#define __COMMON_UTILS_SAVE_PID_FILE_H

#include "common/utils/compiler_defs.h"

BEGIN_DECLS

int
save_pid_file(const char *path);

END_DECLS

#endif // ! __COMMON_UTILS_SAVE_PID_FILE_H
