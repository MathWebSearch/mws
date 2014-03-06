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
  * @brief  Parcelable implementation
  *
  * @file   Parcelable.cpp
  * @author Corneliu-Claudiu Prodescu
  * @date   11 Dec 2013
  *
  * License: GPL v3
  *
  */

#include <assert.h>
#include <string.h>

#include <string>

#include "common/utils/compiler_defs.h"

#include "Parcelable.hpp"

namespace common {
namespace types {

// Parcel Allocator

ParcelAllocator::ParcelAllocator() : mSize(0) {}

void
ParcelAllocator::reserve(const Parcelable& parcelable) {
    mSize += parcelable.getParcelableSize();
}

void
ParcelAllocator::reserve(const std::string& str) {
    mSize += sizeof(size_t) + str.size();
}

size_t
ParcelAllocator::getSize() const {
    return mSize;
}

// Parcel Encoder

ParcelEncoder::ParcelEncoder(const ParcelAllocator& allocator) :
    mSize(allocator.getSize()),
    mData(new char[mSize]),
    mCurr(mData) {}

ParcelEncoder::~ParcelEncoder() {
    delete []mData;
}

void
ParcelEncoder::encode(const Parcelable& parcelable) {
    parcelable.writeToParcel(this);
    assert(mCurr <= mData + mSize);
}

void
ParcelEncoder::encode(const std::string& str) {
    size_t size = str.size();
    memcpy(mCurr, &size, sizeof(size_t));
    mCurr += sizeof(size_t);
    memcpy(mCurr, str.data(), size);
    mCurr += size;
    assert(mCurr <= mData + mSize);
}

const char*
ParcelEncoder::getData() const {
    assert(mCurr == mData + mSize);
    return mData;
}

size_t
ParcelEncoder::getSize() const {
    assert(mCurr == mData + mSize);
    return mSize;
}

// Parcel Decoder

ParcelDecoder::ParcelDecoder(const char* data, size_t size) :
    mSize(size),
    mData(data),
    mCurr(data) {
    // Mark variables which are only used in debug build (assert)
    RELEASE_UNUSED(mSize);
    RELEASE_UNUSED(mData);
}

void
ParcelDecoder::decode(Parcelable* parcelable) {
    parcelable->readFromParcel(this);
    assert(mCurr <= mData + mSize);
}

void
ParcelDecoder::decode(std::string* str) {
    size_t size;
    memcpy(&size, mCurr, sizeof(size_t));
    mCurr += sizeof(size_t);
    *str = std::string(mCurr, size);
    mCurr += size;
    assert(mCurr <= mData + mSize);
}

}  // namespace types
}  // namespace common
