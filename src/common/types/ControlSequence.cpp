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
  * @brief  File containing the implementation of the ControlSequence class.
  *
  * @file   ControlSequence.cpp
  * @author Corneliu-Claudiu Prodescu
  * @date   30 Jul 2011
  *
  * License: GPL v3
  *
  */

// System includes

#include <string.h>
#include <unistd.h>

// Local includes

#include "common/types/ControlSequence.hpp"

// Namespaces

using namespace std;


ControlSequence::ControlSequence()
{
    // Initializing everything, including padding bytes
    memset(this, 0, sizeof(ControlSequence));

    _parsed = false;
    _format = DATAFORMAT_DEFAULT;
}


void
ControlSequence::setFormat(DataFormat aFormat)
{
    _parsed = true;
    _format = aFormat;
}


bool
ControlSequence::isParsed() const
{
    return _parsed;
}


DataFormat
ControlSequence::getFormat() const
{
    return _format;
}


ssize_t
ControlSequence::send(int fd)
{
    char*   data;
    ssize_t data_size;
    ssize_t bytes_written;
    ssize_t nbytes;

    data          = (char*)this;
    data_size     = sizeof(ControlSequence);
    bytes_written = 0;

    while (data_size)
    {
        nbytes = write(fd,
                       data,
                       data_size);
        if (nbytes <= 0)
        {
            if (nbytes == -1) bytes_written = -1;
            break;
        }

        bytes_written += nbytes;
        data          += nbytes;
        data_size     -= nbytes;
    }

    return bytes_written;
}


ssize_t
ControlSequence::recv(int fd)
{
    char*   data;
    ssize_t data_size;
    ssize_t bytes_read;
    ssize_t nbytes;

    data       = (char*)this;
    data_size  = sizeof(ControlSequence);
    bytes_read = 0;
        
    while (data_size)
    {
        nbytes = read(fd,
                      data,
                      data_size);
        if (bytes_read <= 0)
        {
            if (nbytes == -1) bytes_read = -1;
            break;
        }

        bytes_read += nbytes;
        data       += nbytes;
        data_size  -= nbytes;
    }

    return bytes_read;
}
