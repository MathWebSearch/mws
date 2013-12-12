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
#ifndef _COMMON_TYPES_PARCELABLE_HPP
#define _COMMON_TYPES_PARCELABLE_HPP

/**
  * @brief  Parcelable interface
  *
  * @file   Parcelable.hpp
  * @author Corneliu-Claudiu Prodescu
  * @date   11 Dec 2013
  *
  * License: GPL v3
  *
  */

#include <string.h>

#include <string>

namespace common {
namespace types {

class ParcelEncoder;
class ParcelDecoder;

class Parcelable {
 public:
    virtual size_t getParcelableSize() const = 0;
    virtual void writeToParcel(ParcelEncoder* encoder) const = 0;
    virtual void readFromParcel(ParcelDecoder* decoder) = 0;
};

class ParcelAllocator {
 public:
    ParcelAllocator();

    void reserve(const Parcelable& parcelable);
    void reserve(const std::string& str);
    size_t getSize() const;

 private:
    size_t mSize;
};

class ParcelEncoder {
 public:
    explicit ParcelEncoder(const ParcelAllocator& allocator);

    ~ParcelEncoder();

    void encode(const Parcelable& parcelable);
    void encode(const std::string& str);

    const char* getData() const;
    size_t getSize() const;

 private:
    size_t mSize;
    char* mData;
    char* mCurr;

    ParcelEncoder(const ParcelEncoder&);
    const ParcelEncoder& operator=(const ParcelEncoder&);
};

class ParcelDecoder {
 public:
    ParcelDecoder(const char* data, size_t size);

    void decode(Parcelable* parcelable);
    void decode(std::string* str);

 private:
    size_t mSize;
    const char* mData;
    const char* mCurr;
};

}  // namespace types
}  // namespace common

#endif
