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
/**
  * @brief  Container Iterator
  * @file   ContainerIterator.hpp
  * @author Corneliu-Claudiu Prodescu <c.prodescu@jacobs-university.de>
  * @date   14 May 2014
  */

namespace common {
namespace utils {

template<class Container>
class ContainerIterator {
    typedef typename Container::const_iterator Iterator;
    Iterator current;
    Iterator end;
 public:
    explicit ContainerIterator(const Container& container)
        : current(container.begin()), end(container.end()) {
    }

    bool hasNext() const {
        return (current != end);
    }

    Iterator next() {
        return current++;
    }
};

}  // namespace utils
}  // namespace common
