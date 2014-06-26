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
/**
  * This is an example usecase of the analytics API.
  * This program computes average depth and size of expressions,
  * as well as number of unique expressions in the index.
  *
  * @brief Simple expression analytics
  * @file example.cpp
  * @author Corneliu Prodescu
  * @date 20 Jun 2014
  */

#include <cstdio>
#include <cinttypes>

#include "common/utils/compiler_defs.h"
#include "mws/analytics/analytics.hpp"

namespace mws {
namespace analytics {

static uint64_t total_hits;
static uint64_t unique_hits;
static double depth_sum;
static double size_sum;

AnalyticsStatus analyze_begin(const index_handle_t* index,
                              const inode_t* root) {
    UNUSED(index);
    printf("Root has %" PRIu64 " children\n", root->size);
    return ANALYTICS_OK;
}

AnalyticsStatus analyze_expression(const types::CmmlToken* cmmlToken,
                                   uint32_t num_hits) {
    if (verbose) {
        PRINT_LOG("Analyzing expression %s\n", cmmlToken->toString().c_str());
    }
    total_hits += num_hits;
    unique_hits++;
    depth_sum += cmmlToken->getExprDepth();
    size_sum += cmmlToken->getExprSize();
    return ANALYTICS_OK;
}

void analyze_end() {
    printf("Index contains %" PRIu64 " hits\n", total_hits);
    printf("Index contains %" PRIu64 " unique expressions\n", unique_hits);
    printf("Average expression depth is %f\n", depth_sum / unique_hits);
    printf("Average expression size is %f\n", size_sum / unique_hits);
}

}  // namespace analytics
}  // namespace mws
