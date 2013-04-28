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
#ifndef _MWSURLDICTIONARY_HPP
#define _MWSURLDICTIONARY_HPP

/**
  * @brief  File containing the header of URLDictionary Class
  * @file   URLDictionary.hpp
  * @author Daniel Hasegan
  * @date   23 Jan 2013
  *
  * License: GPL v3
  *
  */

// System includes

#include <vector>

#ifdef _URLDICTIONARY_MAPTYPE_HASH
#include <tr1/unordered_map>           // STL unordered map (hash) - TechRep1
#else
#include <map>                         // STL map class header
#endif

// Local includes

#include "mws/types/NodeInfo.hpp"      // MWS Meaning declaration


namespace mws
{

class URLDictionary
{
private:
// Map type used for the dictionary
#ifdef _URLDICTIONARY_MAPTYPE_HASH
    typedef std::tr1::unordered_map<Meaning,
                                    MeaningId,
                                    std::tr1::hash<Meaning> >   _MapType;
#else
    typedef std::map<Meaning, MeaningId>                        _MapType;
#endif
    typedef std::vector<Meaning>                                _ListType;

private:
    /// Id of the latest meaning to be inserted
    MeaningId _nextMeaningId;
    /// Map of meanings
    _MapType  _dictionary;
    /// Vector of Id's 
    _ListType _list;

// Instance Methods
public:
    /**
      * @brief Default constructor of the URLDictionary class
      */
    URLDictionary();

    /**
      * @brief Method to register a meaning with the dictionary.
      * @param aMeaning is the meaning to be registered.
      * @return an id corresponding to the meaning which was inserted. If the
      * meaning was already in the dictionary, its id is returned without any
      * changes to the dictionary
      */
    MeaningId put(const Meaning& aMeaning);

    /**
      * @brief Method to get a meaning from a meaning id.
      * @param aMeaning is the meaning id to be queried.
      * @return the meaning if it is found, or MWS_MEANING_NO_URL 
      * if it is not found.
      */
    Meaning get(const MeaningId& aMeaningId);
};

}

#endif // _MWSMEANINGDICTIONARY_HPP

