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
#ifndef MWS_INDEX_EXPRESSIONENCODER_HPP
#define MWS_INDEX_EXPRESSIONENCODER_HPP

/**
 * @file    ExpressionEncoder.hpp
 * @brief   ExpressionEncoder header
 *
 * @author  cprodescu
 */

/****************************************************************************/
/* Includes                                                                 */
/****************************************************************************/

#include <string>
#include <vector>
#include <unordered_map>

#include "mws/index/encoded_token.h"
#include "mws/index/IndexBuilder.hpp"
#include "mws/index/MeaningDictionary.hpp"
#include "mws/types/CmmlToken.hpp"

/****************************************************************************/
/* Type Declarations                                                        */
/****************************************************************************/

namespace mws { namespace index {

struct ExpressionInfo {
    std::vector<std::string> qvarNames;
    std::vector<std::string> qvarXpaths;
};

class ExpressionEncoder {
 public:
    explicit ExpressionEncoder(MeaningDictionary* dictionary);
    virtual ~ExpressionEncoder();

    int encode(const IndexingOptions& options,
               const types::CmmlToken* expression,
               std::vector<encoded_token_t> *encodedFormula,
               ExpressionInfo* expressionInfo);
    types::Meaning decodeMeaning(encoded_token_t token);
 protected:
    virtual MeaningId _getAnonVarOffset() const = 0;
    virtual MeaningId _getNamedVarOffset() const = 0;
    virtual MeaningId _getConstantEncoding(const types::Meaning& meaning) = 0;

    MeaningId _getCiMeaning(const mws::types::CmmlToken* token);

    MeaningDictionary* _meaningDictionary;
    std::unordered_map<std::string, std::string> _ciTranslations;
    uint32_t _ciTranslationCounter;
};

class HarvestEncoder : public ExpressionEncoder {
 public:
    explicit HarvestEncoder(MeaningDictionary* dictionary);
    virtual ~HarvestEncoder();
 protected:
    virtual MeaningId _getAnonVarOffset() const;
    virtual MeaningId _getNamedVarOffset() const;
    virtual MeaningId _getConstantEncoding(const types::Meaning& meaning);
};

class QueryEncoder : public ExpressionEncoder {
 public:
    explicit QueryEncoder(MeaningDictionary* dictionary);
    virtual ~QueryEncoder();
 protected:
    virtual MeaningId _getAnonVarOffset() const;
    virtual MeaningId _getNamedVarOffset() const;
    virtual MeaningId _getConstantEncoding(const types::Meaning& meaning);
};

class ExpressionDecoder {
    MeaningDictionary::ReverseLookupTable _lookupTable;
 public:
    explicit ExpressionDecoder(const MeaningDictionary& dictionary);
    types::Meaning getMeaning(MeaningId meaningId) const;
};

}  // namespace index
}  // namespace mws

#endif  // MWS_INDEX_EXPRESSIONENCODER_HPP
