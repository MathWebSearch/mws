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
#ifndef MWS_XMLPARSER_WRITEXMLANSWSET_HPP
#define MWS_XMLPARSER_WRITEXMLANSWSET_HPP

/**
  * @brief   File containing the header of the writeXmlAnswset function
  *
  * @file    writeXmlAnswset.hpp
  * @author  Corneliu-Claudiu Prodescu
  * @date    26 Apr 2011
  *
  * @edited Radu Hambasan
  * @date 20 Mar 2014
  *
  * License: GPL v3
  *
  */

#include <stdio.h>
// Local includes

#include "mws/types/MwsAnswset.hpp"    // MWS Answer Set datatype header

namespace mws {
namespace xmlparser {

/**
  * @brief Function to write a MwsAnswset to an output file.
  * @param file is the file to which to write.
  * @param answset is the MWS Answer Set to be written.
  * @return the number of bytes written to fd or -1 in case of failure.
  */
int writeXmlAnswset(MwsAnswset* answset, FILE* file);

}  // namespace xmlparser
}  // namespace mws

#endif  // MWS_XMLPARSER_WRITEXMLANSWSET_HPP
