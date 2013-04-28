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
  * @brief  File containing the implementation of MeaningDictionary Class
  * @file   MeaningDictionary.cpp
  * @author Corneliu-Claudiu Prodescu <c.prodescu@jacobs-university.de>
  * @date   07 Jul 2011
  *
  * License: GPL v3
  *
  */

// System includes

// Local includes

#include "mws/types/MeaningDictionary.hpp" // MWS MeaningDictionary declaration

// Namespaces

using namespace std;
using namespace mws;

// Static fields

MeaningId                    MeaningDictionary::_nextMeaningId;
MeaningDictionary::_MapType  MeaningDictionary::_dictionary;


int
MeaningDictionary::init()
{
    _dictionary.insert(make_pair(MWS_QVAR_MEANING, MWS_MEANING_ID_QVAR));
    _nextMeaningId = MWS_MEANING_ID_START; 

    return 0;
}


void
MeaningDictionary::clean()
{

}


MeaningId
MeaningDictionary::put(const Meaning& aMeaning)
{
    _MapType::iterator it;
    MeaningId          result;

    if ((it = _dictionary.find(aMeaning))
            != _dictionary.end())
    {
        result = it->second;
    }
    else
    {
        _dictionary.insert(make_pair(aMeaning,
                                    _nextMeaningId++));
        result = _nextMeaningId - 1;
    }

    return result;
}


MeaningId
MeaningDictionary::get(const Meaning& aMeaning)
{
    _MapType::const_iterator it;
    MeaningId                result;

    if ((it = _dictionary.find(aMeaning))
            != _dictionary.end())
    {
        result = it->second;
    }
    else
    {
        result = MWS_MEANING_ID_UNKNOWN;
    }

    return result;
}
