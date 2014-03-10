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
#ifndef _HANDLER_STRUCT_HPP
#define _HANDLER_STRUCT_HPP

/**
  * @brief HandlerStruct allows callback in the daemon to mantain state.
  *
  * @file HandlerStruct.hpp
  * @author Radu Hambasan
  * @date 18 Feb 2014
  *
  * License: GPL v3
  *
  */

#include <mws/dbc/DbQueryManager.hpp>
#include "MwsAnswset.hpp"
#include "MwsQuery.hpp"

namespace mws {
namespace types {


struct HandlerStruct {
        MwsAnswset*          result;
        MwsQuery*            mwsQuery;
        dbc::DbQueryManager* dbQueryManager;
};

}  // namespace types
}  // namespace mws

#endif  // _MWS_TYPES_HANDLER_STRUCT_HPP
