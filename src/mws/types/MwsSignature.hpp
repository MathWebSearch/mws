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
#ifndef _MWSSIGNATURE_HPP
#define _MWSSIGNATURE_HPP

/**
  * @brief  File containing the Signature class
  * @file   MwsSignature.hpp
  * @author Daniel Hasegan <d.hasegan@jacobs-university.de>
  * @date   07 Mar 2014
  *
  * License: GPL v3
  *
  */

#include <map>
#include <string>
#include <vector>

#include "mws/types/NodeInfo.hpp"
#include "mws/types/MeaningDictionary.hpp"
#include "common/types/IdDictionary.hpp"

namespace mws {
namespace types {

typedef common::types::IdDictionary<SortName, SortId> SortsDictionary;

int subsetOf(SortId A, SortId B);

class FunctionSignature {
public:
    MeaningId m_functionName;
    std::vector<SortId> m_input;
    SortId m_output;

    FunctionSignature(MeaningId functionName, std::vector<SortId> &functionInput, SortId &functionOutput);
    bool doesApply(std::vector<SortId> sorts);
};

class MwsSignature {
public:
    static const SortId SORT_NOT_FOUND = SortsDictionary::KEY_NOT_FOUND;

    std::vector<SortName> m_sortsNames;

    std::vector<FunctionSignature> m_functionSignatures;
    SortsDictionary m_sortsDictionary;
    types::MeaningDictionary* m_meaningDictionary;

    MwsSignature(types::MeaningDictionary* meaningDictionary);
    SortId getSmallestSort();
    SortId getSort(SortName sortName);
    SortId getSortFunctionApplication(std::vector< std::pair<MeaningId, SortId> > function);

    int readSignatures(std::string signatureFile);
};

} // namespace types
} // namespace mws

#endif // _MWSSIGNATURE_HPP
