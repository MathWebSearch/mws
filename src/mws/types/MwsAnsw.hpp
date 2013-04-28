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
#ifndef _MWSANSW_HPP
#define _MWSANSW_HPP

/**
  * @brief File containing the header of the MWS Answer class.
  *
  * @file MwsAnsw.hpp
  * @author Corneliu-Claudiu Prodescu
  * @date 27 Apr 2011
  *
  * License: GPL v3
  *
  */

// System includes

#include <string>                      // C++ string class header

// Local includes

#include "mws/types/MwsSubst.hpp"      // MWS Substitution


namespace mws
{

/**
  * @brief Datatype used to store a MWS Answer
  */
struct MwsAnsw
{
    /* URI of the answer */
    std::string   uri;
    /* XPath of the answer */
    std::string   xpath;

    mws::MwsSubst subst;

    MwsAnsw(const char* anUri, const char* anXpath)
    {
        uri   = anUri;
        xpath = anXpath;
    }

    ~MwsAnsw()
    {
        // Nothing to do here
    }

private:
    MwsAnsw(const MwsAnsw&);
};

}

#endif // _MWSANSW_HPP
