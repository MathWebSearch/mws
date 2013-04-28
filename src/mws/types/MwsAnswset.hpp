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
#ifndef _MWSANSWSET_HPP
#define _MWSANSWSET_HPP

/**
  * @brief File containing the header of the MwsAnswset class.
  *
  * @file MwsAnswset.hpp
  * @author Corneliu-Claudiu Prodescu
  * @date 27 Apr 2011
  *
  * License: GPL v3
  *
  */

// System includes

#include <cstdio>                      // C standard input output library
#include <vector>                      // STL vector headers

// Local includes

#include "mws/types/MwsAnsw.hpp"       // MWS Answer datatype header
#include "mws/types/MwsSubst.hpp"      // MWS Substitution datatype header

namespace mws
{

/**
  * @brief Datatype used to store a MWS Answer Set
  *
  */
struct MwsAnswset
{
    /// Vector containing the MWS Answers
    std::vector<mws::MwsAnsw*> answers;
    /// Total number of solutions (returned or not)
    int total;
    /// Vector containing the qvar names
    std::vector<std::string> qvarNames;
    /// Vector containing the qvar relative xpaths
    std::vector<std::string> qvarXpaths;

    MwsAnswset()
    {
        total = 0;
    }

    ~MwsAnswset()
    {
        std::vector<mws::MwsAnsw*>::iterator it1;
        std::vector<mws::MwsSubst*>::iterator it2;

        for (it1  = answers.begin();
             it1 != answers.end();
             it1 ++)
        {
            delete *it1;
        }
    }
};

}

#endif // _MWSANSWSET_HPP
