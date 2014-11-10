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
 * @file IdDictionary.hpp
 * @brief IdDictionary API
 */
#ifndef _COMMON_TYPES_IDDICTIONARY_HPP
#define _COMMON_TYPES_IDDICTIONARY_HPP

#include <string.h>
#include <errno.h>

#include <map>
#include <fstream>
#include <string>
#include <vector>
#include <stdexcept>

#include "common/utils/util.hpp"
#include "common/utils/compiler_defs.h"

namespace common {
namespace types {

template<class Key, class ValueId>
class IdDictionary {
private:
    typedef std::map<Key, ValueId>  _MapContainer;
    _MapContainer _map;
    ValueId _nextId;
    static const ValueId VALUEID_START = 1;
    ALLOW_TESTER_ACCESS;
public:
    static const ValueId KEY_NOT_FOUND = VALUEID_START - 1;

    class ReverseLookupTable {
        std::vector<Key> _keys;

     public:
        const Key& get(ValueId valueId) const {
            return _keys.at(valueId - VALUEID_START);
        }
        friend class IdDictionary;
    };

    IdDictionary()
        : _nextId(VALUEID_START)  {
    }

    explicit IdDictionary(const std::string& path)
        : _nextId(VALUEID_START) {
        try {
            std::ifstream file;
            file.exceptions(std::ifstream::badbit | std::ifstream::failbit);
            file.open(path.c_str(), std::ios::in);
            file.exceptions(std::ifstream::badbit);
            Key key;
            while (!file.eof()) {
                std::getline(file, key, '\0');
                if (key.size() > 0) {
                    put(key);
                } else if (!file.eof()) {
                    PRINT_WARN("Empty key");
                }
            }

            PRINT_LOG("Loaded IdDictionary\n");
        } catch (...) {
            throw std::runtime_error(
                        "Cannot import IdDictionary " + path + ": " +
                        strerror(errno));
        }
    }

    int save(std::ostream& out) const {
        ReverseLookupTable table = getReverseLookupTable();

        try {
            for (auto key : table._keys) {
                out << key << '\0';
            }
            out.flush();
        } catch (...) {
            return -1;
        }

        return 0;
    }

    ValueId put(const Key& key) {
        ValueId result = get(key);

        if (result == KEY_NOT_FOUND) {
            result = _nextId;
            _map[key] = _nextId;
            _nextId++;
        }

        return result;
    }

    ValueId get(const Key& key) {
        typename _MapContainer :: iterator it;

        it = _map.find(key);
        if (it != _map.end()) {
            return it->second;
        } else {
            return KEY_NOT_FOUND;
        }
    }

    ReverseLookupTable getReverseLookupTable() const {
        ReverseLookupTable table;
        table._keys.resize(_map.size());

        for (const auto & elem : _map) {
            table._keys[elem.second - VALUEID_START] = elem.first;
        }

        return table;
    }
};

}  // namespace types
}  // namespace common

#endif // _COMMON_TYPES_IDDICTIONARY_HPP
