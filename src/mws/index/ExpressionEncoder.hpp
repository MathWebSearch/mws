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
#ifndef MWS_INDEX_EXPRESSIONENCODER_HPP
#define MWS_INDEX_EXPRESSIONENCODER_HPP

/**
 * @file    ExpressionEncoder.hpp
 * @brief   ExpressionEncoder header
 *
 * @author  cprodescu
 * @date    Mar 10, 2012
 *
 * @todo documentation
 */

/****************************************************************************/
/* Includes                                                                 */
/****************************************************************************/

#include <vector>

#include "mws/index/encoded_token_dict.h"
#include "mws/types/MeaningDictionary.hpp"
#include "mws/types/CmmlToken.hpp"

/****************************************************************************/
/* Type Declarations                                                        */
/****************************************************************************/

namespace mws { namespace index {

class ExpressionEncoder {
 public:
    explicit ExpressionEncoder(mws::types::MeaningDictionary* dictionary);

    int encode(const mws::types::CmmlToken* expression,
               vector<encoded_token_t> *encodedFormula);

 private:
    mws::types::MeaningDictionary* _meaningDictionary;
};

}  // namespace index
}  // namespace mws

#endif  // MWS_INDEX_EXPRESSIONENCODER_HPP
