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
#ifndef _MWS_INDEX_INDEXWRITER_HPP
#define _MWS_INDEX_INDEXWRITER_HPP

/**
  * @file IndexWriter.hpp
  *
  * @author Radu Hambasan
  * @date 29 May 2014
  */

#include <string>
#include <vector>

#include "mws/index/IndexBuilder.hpp"

namespace mws {
namespace index {

struct IndexConfiguration {
    HarvesterConfiguration harvester;
    EncodingConfiguration encoding;
    std::string dataPath;
    bool deleteOldData;
};


/**
 * @brief Write an index, and associated data to disk
 * @param config
 * @return 0 on success, 1 if an error occurs
 */
int createCompressedIndex(const IndexConfiguration& config);

}  // namespace index
}  // namespace mws

#endif  // _MWS_INDEX_INDEXWRITER_HPP
