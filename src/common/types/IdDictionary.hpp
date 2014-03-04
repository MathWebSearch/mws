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

#include <istream>
#include <map>
#include <ostream>
#include <string>
#include <vector>

namespace common {
namespace types {

template<class Key, class ValueId>
class IdDictionary {
private:
    typedef std::map<Key, ValueId>  _MapContainer;
    
    _MapContainer   _map;
    ValueId         _nextId;
public:
    static const ValueId KEY_NOT_FOUND = 0;

    IdDictionary() :
        _nextId(KEY_NOT_FOUND + 1)  {
    }

    int load(std::istream& in) {
        try {
            Key key;
            while (!in.eof()) {
                std::getline(in, key, '\0');
                if (key.size() > 0) {
                    put(key);
                } else if (!in.eof()) {
                    fprintf(stderr, "Empty key found!\n");
                }
            }
        } catch (...) {
            return -1;
        }

        return 0;
    }

    int save(std::ostream& out) {
        std::vector<Key> keys;
        keys.resize(_map.size());

        for (typename _MapContainer::iterator it = _map.begin();
             it != _map.end();
             it++) {

            keys[it->second - 1] = it->first;
        }

        try {
            Key key;

            for (int i = 0; i < (int)keys.size(); i++) {
                out << keys[i] << '\0';
            }
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
};

}  // namespace types
}  // namespace common

#endif // _COMMON_TYPES_IDDICTIONARY_HPP
