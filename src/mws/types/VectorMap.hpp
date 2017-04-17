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
#ifndef _MWS_TYPES_VECTORMAP_HPP
#define _MWS_TYPES_VECTORMAP_HPP

/**
  * @brief  Map with vector container
  * @file   VectorMap.hpp
  * @author Corneliu-Claudiu Prodescu <c.prodescu@jacobs-university.de>
  * @date   07 Jul 2011
  *
  * License: GPL v3
  *
  */

// System includes
#include <utility>  // STL utilities (std::air)
#include <vector>   // STL vector container

namespace mws {
namespace types {

template <class T>
struct Comparator {
    static inline int compare(const T& t1, const T& t2) {
        return memcmp(&t1, &t2, sizeof(t1));
    }
};

template <class K, class V>
class VectorMap {
    typedef std::pair<K, V> key_value;
    typedef typename std::vector<key_value> _VectorContainer;
    _VectorContainer _data;

 public:
    typedef typename _VectorContainer::iterator iterator;
    typedef typename _VectorContainer::const_iterator const_iterator;

    /**
     * @brief Method to determine the number of elements in this VectorMap
     * @use std::distance instead of std::Vector::size to avoid size_type
     * return type that is container dependent
     * @std::distance returns ssize_t and program will have overflow errors
     * for exceedingly great number of children
     */
    inline size_t size() const {
      return (size_t)std::distance(_data.begin(), _data.end());
    }

    /**
      * @brief Method to find an element by key.
      * @param key is the key to be searched.
      * @return an iterator to the element matching the key or VectorMap::end
      * if no matching key exists.
      */
    inline iterator find(const K& key) {
        return _data.begin() + _find_index(key);
    }

    /**
      * @brief Method to find an element by key.
      * @param key is the key to be searched.
      * @return iterator to the element matching the key or VectorMap::end
      * if no matching key exists.
      */
    inline const_iterator find(const K& key) const {
        return _data.begin() + _find_index(key);
    }

    inline V& operator[](const K& key) {
        return ((this->insert(make_pair(key, V()))).first)->second;
    }

    /**
      * @brief Method to insert a key-value pair into the Map. If the key
      * already exists, nothing is inserted.
      * @param keyValue is the pair to be inserted.
      * @return a pair containing an iterator to the inserted (or existing) key
      * and a boolean showing if a new pair was inserted.
      */
    inline std::pair<iterator, bool> insert(const key_value& keyValue) {
        iterator it;

        it = find(keyValue.first);
        if (it == end()) {  // new key
            size_t i;

            // Inserting the data
            _data.push_back(keyValue);

            // Re-ordering the vector
            i = _data.size() - 1;
            while ((i > 0) &&
                   (Comparator<K>::compare(_data[i].first, _data[i - 1].first) <
                    0)) {
                key_value tmp;

                tmp = _data[i];
                _data[i] = _data[i - 1];
                _data[i - 1] = tmp;

                i--;
            }

            return make_pair(_data.begin() + i, true);
        } else {  // existent key
            return make_pair(it, false);
        }
    }

    inline iterator begin() { return _data.begin(); }
    inline iterator end() { return _data.end(); }
    inline const_iterator begin() const { return _data.begin(); }
    inline const_iterator end() const { return _data.end(); }

 private:
    int _find_index(const K& key) const {
        int left, right;

        left = 0;
        right = _data.size() - 1;

        while (left <= right) {
            size_t center = left + (right - left) / 2;
            int result = Comparator<K>::compare(_data[center].first, key);
            if (result > 0) {
                right = center - 1;
            } else if (result == 0) {
                return center;
            } else {
                left = center + 1;
            }
        }

        return _data.size();
    }

    ALLOW_TESTER_ACCESS;
};

}  // namespace types
}  // namespace mws

#endif  // _MWS_TYPES_VECTORMAP_HPP
