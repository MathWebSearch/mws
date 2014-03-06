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
#ifndef _MWS_VECTORMAP_HPP
#define _MWS_VECTORMAP_HPP

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

#include <utility>                     // STL utilities (std::air)
#include <vector>                      // STL vector container

namespace mws
{

template<class T>
struct Comparator
{
    static int compare(const T&, const T&);
};


template<class K, class V>
class VectorMap
{
    // Typedefs
public:
    typedef std::pair<K, V>                                   key_value;
    typedef typename std::vector< key_value >::iterator       iterator;
    typedef typename std::vector< key_value >::const_iterator const_iterator;
private:
    typedef typename std::vector< key_value >                 _VectorContainer;

    // Data Members
private:
    _VectorContainer                                          _data;

    // Methods
public:
    inline size_t size() const {
        return _data.size();
    }
    /**
      * @brief Method to find an element by key.
      * @param key is the key to be searched.
      * @return an iterator to the element matching the key or VectorMap::end
      * if no matching key exists.
      */
    inline iterator
    find(const K& key)
    {
        int left, right;

        left = 0;
        right = _data.size() - 1;

        while(left <= right)
        {
            size_t center = left + (right - left) / 2;
            int    result = Comparator<K>::compare(_data[center].first, key);
            if (result > 0)
            {
                right = center - 1;
            }
            else if (result == 0)
            {
                return _data.begin() + center;
            }
            else
            {
                left = center + 1;
            }
        }

        return _data.end();
    }

    /**
      * @brief Method to insert a key-value pair into the Map. If the key
      * already exists, nothing is inserted.
      * @param keyValue is the pair to be inserted.
      * @return a pair containing an iterator to the inserted (or existing) key
      * and a boolean showing if a new pair was inserted.
      */
    inline std::pair<iterator, bool>
    insert(const key_value& keyValue)
    {
        iterator it;

        it = find(keyValue.first);
        // If the element is not present, we need to insert it
        if (it == end())
        {
            size_t i;

            // Inserting the data
            _data.push_back(keyValue);

            // Re-ordering the vector
            i = _data.size() - 1;
            while ((i > 0) &&
                   (Comparator<K>::compare(_data[i].first,_data[i-1].first) < 0))
            {
                key_value tmp;

                tmp        = _data[i];
                _data[i]   = _data[i-1];
                _data[i-1] = tmp;

                i--;
            }
        
            return make_pair(_data.begin() + i, true);
        }
        // Otherwise we return
        else
        {
            return make_pair(it, false);
        }
    }

    /**
      * @brief Method to obtain an iterator to the beginning of the VectorMap.
      * @return an iterator to the beginning of the VectorMap.
      */
    inline iterator
    begin()
    {
        return _data.begin();
    }

    /**
      * @brief Method to obtain an iterator to the end of the VectorMap.
      * @return an iterator to the end of the VectorMap.
      */
    inline iterator
    end()
    {
        return _data.end();
    }

    inline const_iterator
    begin() const {
        return _data.begin();
    }

    inline const_iterator
    end() const {
        return _data.end();
    }
};

template<>
inline int
Comparator<NodeInfo>::compare(const NodeInfo& n1, const NodeInfo& n2)
{
    if (n1.second > n2.second)
    {
        return 1;
    }
    else if (n1.second == n2.second)
    {
        if (n1.first > n2.first)
            return 1;
        else if (n1.first == n2.first)
            return 0;
        else
            return -1;
    }
    else
    {
        return -1;
    }
}

}

#endif // _MWS_VECTORMAP_HPP
