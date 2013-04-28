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
#ifndef _MWSMEANINGDICTIONARY_HPP
#define _MWSMEANINGDICTIONARY_HPP

/**
  * @brief  File containing the header of MeaningDictionary Class
  * @file   MeaningDictionary.hpp
  * @author Corneliu-Claudiu Prodescu <c.prodescu@jacobs-university.de>
  * @date   07 Jul 2011
  *
  * License: GPL v3
  *
  */

// System includes

#ifdef _MEANINGDICTIONARY_MAPTYPE_HASH
#include <tr1/unordered_map>           // STL unordered map (hash) - TechRep1
#else
#include <map>                         // STL map class header
#endif

// Local includes

#include "mws/types/NodeInfo.hpp"      // MWS Meaning declaration


namespace mws
{

class MeaningDictionary
{
private:
// Map type used for the dictionary
#ifdef _MEANINGDICTIONARY_MAPTYPE_HASH
    typedef std::tr1::unordered_map<Meaning,
                                    MeaningId,
                                    std::tr1::hash<Meaning> >   _MapType;
#else
    typedef std::map<Meaning, MeaningId>                        _MapType;
#endif

private:
    /// Id of the latest meaning to be inserted
    static MeaningId _nextMeaningId;
    /// Map of meanings
    static _MapType  _dictionary;

// Instance Methods
public:

    /**
      * @brief Method to initialize the MeaningDictionary class.
      * @return 0 on success and -1 on failure
      */
    static int init();

    /**
      * @brief Method to clean up the MeaningDictionary class.
      */
    static void clean();

    /**
      * @brief Method to register a meaning with the dictionary.
      * @param aMeaning is the meaning to be registered.
      * @return an id corresponding to the meaning which was inserted. If the
      * meaning was already in the dictionary, its id is returned without any
      * changes to the dictionary
      */
    static MeaningId put(const Meaning& aMeaning);

    /**
      * @brief Method to get a meaning id from a meaning.
      * @param aMeaning is the meaning to be queried.
      * @return the id of the meaning if it is found, or MWS_MEANING_ID_UNKNOWN
      * if it is not found.
      */
    static MeaningId get(const Meaning& aMeaning);

private:
    /**
      * Declared private to disable instantiation
      * @brief Default constructor of the MeaningDictionary class
      */
    MeaningDictionary();

};

}

#endif // _MWSMEANINGDICTIONARY_HPP
