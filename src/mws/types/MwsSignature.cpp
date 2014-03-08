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
  * @brief  Mws Signature Implementation
  * @file   MwsSignature.cpp
  * @author Daniel Hasegan <d.hasegan@jacobs-university.de>
  * @date   07 Mar 2014
  *
  * License: GPL v3
  *
  */

#include <iostream>
#include <fstream>

#include "MwsSignature.hpp"

using namespace std;

namespace mws {
namespace types {

int subsetOf(SortId A, SortId B) {
    return A >= B;
}

FunctionSignature::FunctionSignature(MeaningId functionName, std::vector<SortId> &functionInput, SortId &functionOutput) {
    m_functionName = functionName;
    m_input = functionInput;
    m_output = functionOutput;
}

MwsSignature::MwsSignature(types::MeaningDictionary* meaningDictionary)
    : m_meaningDictionary(meaningDictionary) {
}

vector<string> splitLine(string line) {
    vector<string> result;
    size_t pos = 0;
    string token;
    while ((pos = line.find(" ")) != string::npos) {
        token = line.substr(0, pos);
        result.push_back(token);
        line.erase(0, pos + 1);
    }
    result.push_back(line);
    return result;
}

SortId
MwsSignature::getSort(SortName sortName) {
    SortId sortId;
    if ((sortId = m_sortsDictionary.get(sortName)) != SortsDictionary::KEY_NOT_FOUND) {
        return sortId;
    }
    return SORT_NOT_FOUND;
}

int
MwsSignature::readSignatures(string signatureFile) {

    SortId              newSortId;
    vector<SortId>      functionInput;
    SortId              functionOutput;
    vector<string>      args;


    ifstream _fstream;
    _fstream.open(signatureFile);
    if (!_fstream.is_open()) {
        return 0;
    }

    while (!_fstream.eof()) {
        string line;
        getline(_fstream, line);

        args.clear();
        args = splitLine(line);
        if (args[0] == "SORTS:") {
            // Add to the sorts dictionary
            for(int i=1; (unsigned int) i < args.size() ; ++i) {
                newSortId = m_sortsDictionary.put( args[i] );
            }
        } else if (args[0] == "FUNCTION:") {
            // Get Function Id from meaning dictionary
            MeaningId functionId = m_meaningDictionary->put(args[1]);
            // Create the function input types
            for(int i=2; (unsigned int) i < args.size() -1 ; ++i) {
                newSortId = m_sortsDictionary.put( args[i] );
                functionInput.push_back( newSortId );
            }
            // Create the function output type
            newSortId = m_sortsDictionary.put( args[ args.size() - 1 ] );
            functionOutput = newSortId;

            // Create the function signature
            FunctionSignature functionSign(functionId, functionInput, functionOutput);
            m_functionSignatures.push_back(functionSign);
        } else if (args[0] == "SORT-RELATION:") {
            // Maybe later
        }
    }

    _fstream.close();
    return 1;
}

} // namespace types
} // namespace mws
