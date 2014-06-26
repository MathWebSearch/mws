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
#ifndef _MWS_ANALYTICS_ANALYTICS_HPP
#define _MWS_ANALYTICS_ANALYTICS_HPP

/**
  * @brief  Analytics interface
  * @file   analytics.hpp
  * @author Corneliu Prodescu
  * @date   20 June 2014
  */

#include "mws/types/CmmlToken.hpp"
#include "mws/index/index.h"

namespace mws {
namespace analytics {

enum AnalyticsStatus {
    ANALYTICS_OK,
    ANALYTICS_STOP,
};

/**
 * @brief Callback which runs before any expression is analyzed
 * @param index Compressed index handler
 * @param root Root node of the index
 * @return ANALYTICS_OK if the analytics job should continue
 * @return ANALYTICS_STOP if the analytics job should stop
 */
AnalyticsStatus analyze_begin(const index_handle_t* index, const inode_t* root);
/**
 * @brief Callback for each expression in the index
 * @param cmmlToken ContentMathML expression
 * @param num_hits Number of formulae containing this expression
 * @return ANALYTICS_OK if the analytics job should continue
 * @return ANALYTICS_STOP if the analytics job should stop
 */
AnalyticsStatus analyze_expression(const types::CmmlToken* cmmlToken,
                                   uint32_t num_hits);
/**
 * @brief Callback which runs after all expressions have been analyzed
 */
void analyze_end();

}  // namespace analytics
}  // namespace mws

#endif  // _MWS_ANALYTICS_ANALYTICS_HPP
