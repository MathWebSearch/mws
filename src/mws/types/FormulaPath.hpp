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
#ifndef _MWS_TYPES_FORMULAPATH_HPP
#define _MWS_TYPES_FORMULAPATH_HPP

/**
  * @brief  FormulaId and FormulaPath type definitions
  * @file   FormulaPath.hpp
  * @author Corneliu-Claudiu Prodescu <c.prodescu@jacobs-university.de>
  * @date   07 Jul 2011
  *
  * License: GPL v3
  *
  */

// System includes

#include <stdint.h>

#include <string>
#include <utility>

#include "common/types/Parcelable.hpp"


namespace mws {
namespace types {

/**
 * @brief Formula Id corresponding to a leaf node
 */
typedef uint32_t FormulaId;

/**
 * @brief Formula Path
 */
struct FormulaPath : public common::types::Parcelable {
    std::string xmlId;
    std::string xpath;

    FormulaPath() {
    }

    FormulaPath(std::string xmlId, std::string xpath)
        : xmlId(std::move(xmlId)), xpath(std::move(xpath)) {
    }

    inline bool operator!=(const FormulaPath& rhs) const {
        return xmlId != rhs.xmlId || xpath != rhs.xpath;
    }

    virtual size_t getParcelableSize() const {
        common::types::ParcelAllocator parcelAllocator;
        parcelAllocator.reserve(xmlId);
        parcelAllocator.reserve(xpath);
        return parcelAllocator.getSize();
    }

    virtual void writeToParcel(common::types::ParcelEncoder* encoder) const {
        encoder->encode(xmlId);
        encoder->encode(xpath);
    }

    virtual void readFromParcel(common::types::ParcelDecoder* decoder) {
        decoder->decode(&xmlId);
        decoder->decode(&xpath);
    }
};

}  // namespace types
}  // namespace mws

#endif  // _MWS_TYPES_FORMULAPATH_HPP
