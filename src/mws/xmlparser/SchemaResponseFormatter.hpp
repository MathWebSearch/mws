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
#ifndef _MWS_PARSER_SCHEMARESPONSE_FORMATTER_HPP
#define _MWS_PARSER_SCHEMARESPONSE_FORMATTER_HPP

/**
  * @author  Radu Hambasan
  * @date    30 Dec 2014
  *
  * License: GPL v3
  *
  */

#include <libxml/xmlwriter.h>
#include "mws/types/CmmlToken.hpp"
#include "mws/types/Query.hpp"

namespace mws {
namespace parser {

struct SchemaResponseFormatter : public types::Query::ResponseFormatter {
    static SchemaResponseFormatter instance;

    virtual const char* getContentType() const;
    virtual int writeData(const void* data, FILE* output) const;

 private:
    int printCmmlToken(const types::CmmlToken* root, xmlTextWriter* wrt) const;
};

extern const SchemaResponseFormatter* RESPONSE_FORMATTER_SCHEMA;

}  // namespace parser
}  // namespace mws

#endif  // _MWS_PARSER_SCHEMARESPONSE_FORMATTER_HPP
