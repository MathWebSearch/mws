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
#ifndef _MWS_DAEMON_QUERYHANDLER_HPP
#define _MWS_DAEMON_QUERYHANDLER_HPP

#include "mws/types/Query.hpp"
#include "mws/types/MwsAnswset.hpp"

namespace mws {
namespace daemon {

class QueryHandler {
 public:
    virtual ~QueryHandler() {}
    virtual MwsAnswset* handleQuery(types::Query* query) = 0;
};

}  // namespace daemon
}  // namespace mws

#endif  // _MWS_DAEMON_QUERYHANDLER_HPP
