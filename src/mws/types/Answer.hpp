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
#ifndef _MWS_TYPES_ANSWER_HPP
#define _MWS_TYPES_ANSWER_HPP

/**
  * @brief MWS Answer type
  *
  * @file Answer.hpp
  * @author Corneliu Claudiu Prodescu
  * @date 27 Apr 2011
  *
  * License: GPL v3
  *
  */

#include <string>

namespace mws {
namespace types {

/**
  * @brief <mws:answ> Answer
  */
struct Answer {
    std::string uri;
    std::string xpath;
    std::string data;
};

}  // namespace types
}  // namespace mws

#endif  // _MWS_TYPES_ANSWER_HPP
