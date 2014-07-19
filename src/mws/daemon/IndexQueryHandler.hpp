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
#ifndef _INDEX_DAEMON_HPP
#define _INDEX_DAEMON_HPP

/**
  * @brief File containing the header of the IndexDaemon class.
  * @file IndexDaemon.hpp
  * @author Corneliu-Claudiu Prodescu
  * @date 18 Jun 2011
  *
  * @edited Radu Hambasan
  * @date 18 Feb 2014
  *
  * @edited Corneliu Prodescu
  * @date 21 May 2014
  *
  * License: GPL v3
  *
  */

#include <string>
#include <memory>

#include "common/utils/compiler_defs.h"
#include "mws/daemon/QueryHandler.hpp"
#include "mws/index/ExpressionEncoder.hpp"
#include "mws/index/IndexLoader.hpp"

namespace mws {
namespace daemon {

class IndexQueryHandler : public QueryHandler {
 public:
    struct Config {
        index::ExpressionEncoder::Config encoding;
        bool useExperimentalQueryEngine;

        Config() : useExperimentalQueryEngine(false) {}
    };

    IndexQueryHandler(const std::string& indexPath,
                      const Config& config = Config());
    ~IndexQueryHandler();

    MwsAnswset* handleQuery(types::Query* query);

 private:
    index::IndexLoader _index;
    Config _config;

    DISALLOW_COPY_AND_ASSIGN(IndexQueryHandler);
};

}  // namespace daemon
}  // namespace mws

#endif  // _INDEX_DAEMON_HPP
