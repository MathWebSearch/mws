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
#ifndef _CONTROLSEQUENCE_HPP
#define _CONTROLSEQUENCE_HPP

/**
  * @brief  File containing the header of the ControlSequence class.
  *
  * @file   ControlSequence.hpp
  * @author Corneliu-Claudiu Prodescu
  * @date   30 Jul 2011
  *
  * License: GPL v3
  *
  */

// System includes

#include <string>                      // C++ string headers

// Local includes

#include "common/types/DataFormat.hpp" // Data formats enum


class ControlSequence
{
private:
    bool       _parsed;
    DataFormat _format;

public:
    /**
      * @brief Default constructor of the ControlSequence class.
      */
    ControlSequence();

    /**
      * @brief Method to set the output format.
      * @param aFormat is a string describing the format (Ex: "xml", "json").
      */
    void setFormat(DataFormat aFormat);

    /**
      * @brief Accessor method for the parsed state;
      * @return true if parsed, false if an error occurred.
      */
    bool isParsed() const;

    /**
      * @brief Accessor method for the format.
      * @return the format registered with the Control Sequence.
      */
    DataFormat getFormat() const;
    /**
      * @brief Method to send the current ControlSequence via a file
      * descriptor.
      * @param fd is the destination file descriptor.
      * @return 0 on success, or the number of bytes which were not delivered.
      */
    ssize_t send(int fd);

    /**
      * @brief Method to receive a ControlSequence via a file descriptor.
      * @param fd is the sender file descriptor.
      * @return 0 on success, or the number of bytes which were not delivered.
      */
    ssize_t recv(int fd);
};

#endif // _CONTROLSEQUENCE_HPP
