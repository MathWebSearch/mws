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
#ifndef _MWS_PARSER_SCHEMAXMLRESPONSE_FORMATTER_HPP
#define _MWS_PARSER_SCHEMAXMLRESPONSE_FORMATTER_HPP

/**
  * @author  Radu Hambasan
  * @date    30 Dec 2014
  *
  * License: GPL v3
  *
  */

#include <libxml/xmlwriter.h>
#include "mws/types/CmmlToken.hpp"
#include "mws/types/GenericAnswer.hpp"
#include "mws/types/Query.hpp"

namespace mws {
namespace parser {

struct SchemaXmlResponseFormatter : public types::Query::ResponseFormatter {
    static SchemaXmlResponseFormatter instance;

    virtual const char* getContentType() const;
    virtual int writeData(const GenericAnswer* ans, FILE* output) const;

 private:
    int printCmmlToken(const types::CmmlToken* root, xmlTextWriter* wrt) const;
};

extern const SchemaXmlResponseFormatter* RESPONSE_FORMATTER_SCHEMA_XML;

}  // namespace parser
}  // namespace mws

#endif  // _MWS_PARSER_SCHEMAXMLRESPONSE_FORMATTER_HPP
