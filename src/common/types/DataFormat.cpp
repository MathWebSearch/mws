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
  * @brief  File containing the implementation of the DataFormat class.
  *
  * @file   DataFormat.cpp
  * @author Corneliu-Claudiu Prodescu
  * @date   30 Jul 2011
  *
  * License: GPL v3
  *
  */

// System includes

#include <ostream>

// Local includes

#include "common/types/DataFormat.hpp"

std::ostream& operator<<(std::ostream& out, DataFormat dataFormat) {
    switch (dataFormat) {
    case DATAFORMAT_XML:
        out << "text/xml";
        break;
    case DATAFORMAT_JSON:
        out << "application/json";
        break;
    default:
        out << DEFAULT_MIME_TYPE;
        break;
    }

    return out;
}
