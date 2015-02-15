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
#ifndef _MWSSUBST_HPP
#define _MWSSUBST_HPP

/**
  * @brief File containing the header of the MWS Substitution class.
  *
  * @file   MwsSubst.hpp
  * @author Corneliu-Claudiu Prodescu
  * @date   31 May 2011
  *
  * License: GPL v3
  *
  */

// System includes

#include <vector>  // C++ vector class header

// Local includes

namespace mws {

/**
  * @brief Datatype used to store a MWS Substitution
  */
struct MwsSubst {
    /// Xpaths of the substituted qvars
    std::vector<std::string> qvarXpaths;

    MwsSubst() {
        // Nothing to do here
    }

    ~MwsSubst() {
        // Nothing to do here
    }

 private:
    MwsSubst(const MwsSubst&);
};
}

#endif  // _MWSSUBST_HPP
