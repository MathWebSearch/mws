/*

Copyright (C) 2010-2014 KWARC Group <kwarc.info>

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
#ifndef _MWS_DAEMON_SCHEMAQUERYHANDLER_HPP
#define _MWS_DAEMON_SCHEMAQUERYHANDLER_HPP

/**
  * @author Radu Hambasan
  * @date 30 Dec 2014
  */

#include <vector>
#include <string>

#include "mws/types/CmmlToken.hpp"
#include "mws/daemon/QueryHandler.hpp"
#include "mws/index/ExpressionEncoder.hpp"
using mws::index::ExpressionEncoder;
#include "mws/query/SchemaEngine.hpp"
#include "mws/types/GenericAnswer.hpp"

namespace mws {
namespace daemon {

class SchemaQueryHandler : public QueryHandler {
 public:
    SchemaQueryHandler(const ExpressionEncoder::Config& encodingConfig =
            ExpressionEncoder::Config());

    ~SchemaQueryHandler();

    GenericAnswer* handleQuery(types::Query* query);

 private:
   void getSubstitutions(types::CmmlToken* exprRoot,
                         types::CmmlToken* schemaRoot,
                         std::vector<std::string>* substitutions);

    index::ExpressionEncoder::Config _encodingConfig;
    DISALLOW_COPY_AND_ASSIGN(SchemaQueryHandler);
};

}  // namespace daemon
}  // namespace mws

#endif  // _MWS_DAEMON_SCHEMAQUERYHANDLER_HPP
