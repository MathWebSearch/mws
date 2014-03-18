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
#ifndef _MWS_TYPES_NODEINFO_HPP
#define _MWS_TYPES_NODEINFO_HPP

/**
  * @brief  File containing type declarations for NodeInfo
  * @file   NodeInfo.hpp
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

typedef uint32_t FormulaId;     ///< Formula Id corresponding to a leaf node

struct FormulaPath : public common::types::Parcelable {
    std::string xmlId;
    std::string xpath;

    FormulaPath() {
    }

    FormulaPath(std::string xmlId, std::string xpath)
        : xmlId(xmlId), xpath(xpath) {
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

typedef uint32_t CrawlId;   ///< Crawled data Id
const CrawlId CRAWLID_NULL = 0;

typedef std::string CrawlData;
const CrawlData CRAWLDATA_NULL = CrawlData();

}  // namespace types
}  // namespace mws

namespace mws {



/// Type of the node meaning encoding
typedef uint32_t    MeaningId;

/// Type of the node arity
typedef uint8_t     Arity;

/// Sort names and Ids
typedef std::string SortName;
typedef uint8_t SortId;

typedef std::pair<Arity, SortId> NodeType;

/// Type of the node info
typedef std::pair<MeaningId, NodeType> NodeInfo;

// Constants

}  // namespace mws

#endif  // _MWS_TYPES_NODEINFO_HPP
