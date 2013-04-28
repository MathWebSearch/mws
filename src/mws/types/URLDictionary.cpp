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
  * @brief  File containing the implementation of URLDictionary Class
  * @file   URLDictionary.cpp
  * @author Daniel Hasegan
  * @date   23 Jan 2013
  *
  * License: GPL v3
  *
  */

// System includes

#include <stdio.h>

// Local includes

#include "mws/types/URLDictionary.hpp" // MWS URLDictionary declaration

// Namespaces

using namespace std;
using namespace mws;

URLDictionary::URLDictionary() {
    _nextMeaningId = MWS_URLMEANING_ID_START;
}

MeaningId
URLDictionary::put(const Meaning& aMeaning)
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
        _list.push_back(aMeaning);
        result = _nextMeaningId - 1;
    }

    return result;
}


Meaning
URLDictionary::get(const MeaningId& aMeaningId)
{
    _MapType::const_iterator it;
    Meaning                  result;

    // see if we have the Id
    if ( aMeaningId < _nextMeaningId )
    {
        result = _list[aMeaningId];
        // double check if we have the Meaning
        if ((it = _dictionary.find(result)) 
                == _dictionary.end())
        {
          result = MWS_URLMEANING_NO_URL;
        }
    }
    else
    {
        result = MWS_URLMEANING_NO_URL;
    }

    return result;
}

