/*

Copyright (C) 2010-2014 KWARC Group <kwarc.info>

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
#ifndef _COMMON_UTILS_CONTAINERITERATOR_HPP
#define _COMMON_UTILS_CONTAINERITERATOR_HPP

/**
  * @brief  Container Iterator
  * @file   ContainerIterator.hpp
  * @author Corneliu-Claudiu Prodescu <c.prodescu@jacobs-university.de>
  * @date   14 May 2014
  */

namespace common {
namespace utils {

template <class Iterator>
class ContainerIterator {
    Iterator current;
    Iterator end;

 public:
    ContainerIterator(const Iterator& begin, const Iterator& end)
        : current(begin), end(end) {}
    bool hasNext() const { return (current != end); }
    Iterator next() { return current++; }
    Iterator& get() { return current; }
    const Iterator& get() const { return current; }
};

}  // namespace utils
}  // namespace common

#endif  // _COMMON_UTILS_CONTAINERITERATOR_HPP
