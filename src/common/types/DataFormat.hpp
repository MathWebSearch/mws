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
#ifndef _DATAFORMAT_HPP
#define _DATAFORMAT_HPP

/**
  * @brief  File containing the header of the DataFormat class.
  *
  * @file   DataFormat.hpp
  * @author Corneliu-Claudiu Prodescu
  * @date   30 Jul 2011
  *
  * License: GPL v3
  *
  */

// System includes

#include <ostream>

// Constants

const std::string DEFAULT_MIME_TYPE = "text/xml";


enum DataFormat
{
    DATAFORMAT_DEFAULT,
    DATAFORMAT_UNKNOWN,
    DATAFORMAT_XML,
    DATAFORMAT_JSON,
};

std::ostream& operator << (std::ostream& out, DataFormat dataFormat);


#endif
