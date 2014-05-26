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
 *  @brief Test for Parcelable implementation
 *  @file Parcelable.cpp
 *  @author cprodescu
 *
 */

#include <string>
using std::string;

#include "common/utils/compiler_defs.h"
#include "common/types/Parcelable.hpp"
using common::types::ParcelAllocator;
using common::types::ParcelEncoder;
using common::types::ParcelDecoder;
#include "mws/types/FormulaPath.hpp"
using mws::types::FormulaPath;

/*
 * Constructing a string from a char array (like a constant string "foo")
 * performs an implicit conversion to char* and calls the C-style constructor
 * of the string - string(const char*). If this array contains a null character,
 * this will not be part of the string. For such cases, use this macro.
 */
#define string_from_buffer(charBuffer) string(charBuffer, sizeof(charBuffer))

static int testSerializeDeserialize(const FormulaPath& initial) {
    // Compute space required for the serialization
    ParcelAllocator allocator;
    allocator.reserve(initial);

    // Serialize the object
    ParcelEncoder encoder(allocator);
    encoder.encode(initial);

    // Deserialize the object
    ParcelDecoder decoder(encoder.getData(), encoder.getSize());
    FormulaPath deserialized;
    decoder.decode(&deserialized);

    // Check consistency
    FAIL_ON(initial.xmlId != deserialized.xmlId);
    FAIL_ON(initial.xpath != deserialized.xpath);

    return 0;

fail:
    return -1;
}

int main() {
    FormulaPath formulaPath;

    formulaPath.xmlId = "http://www.example.org";
    formulaPath.xpath = "foo=bar";
    FAIL_ON(testSerializeDeserialize(formulaPath) != 0);

    formulaPath.xmlId = "";
    formulaPath.xpath = "";
    FAIL_ON(testSerializeDeserialize(formulaPath) != 0);

    formulaPath.xmlId = string_from_buffer("a \0 b");
    formulaPath.xpath = string_from_buffer("foo \0 bar");
    FAIL_ON(testSerializeDeserialize(formulaPath) != 0);

    formulaPath.xmlId = string_from_buffer("a \u00E1 b");
    formulaPath.xpath = string_from_buffer("foo \u00E1 \0 bar");
    FAIL_ON(testSerializeDeserialize(formulaPath) != 0);

    return 0;

fail:
    return -1;
}
