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
#ifndef _MWS_PARSER_MWSIDSRESPONSE_FORMATTER_HPP
#define _MWS_PARSER_MWSIDSRESPONSE_FORMATTER_HPP

/**
  * @brief   MwsIdsResponseFormatter
  *
  * @file    MwsIdsResponseFormatter.hpp
  * @author  Corneliu-Claudiu Prodescu
  * @date    30 Jul 2011
  *
  * @edited Radu Hambasan
  * @date 20 Mar 2014
  * License: GPL v3
  *
  */

#include "mws/types/Query.hpp"

namespace mws {
namespace parser {

struct MwsIdsResponseFormatter : public types::Query::ResponseFormatter {
    static MwsIdsResponseFormatter instance;

    virtual const char* getContentType() const;
    virtual int writeData(const MwsAnswset& answerSet, FILE* output) const;
};

extern MwsIdsResponseFormatter* RESPONSE_FORMATTER_MWS_IDS;

}  // namespace parser
}  // namespace mws

#endif  // _MWS_PARSER_MWSIDSRESPONSE_FORMATTER_HPP
