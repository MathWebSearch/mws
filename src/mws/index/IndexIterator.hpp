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
#ifndef _MWS_INDEX_INDEXITERATOR_HPP
#define _MWS_INDEX_INDEXITERATOR_HPP

/**
  * @brief  File containing the implementation of MwsSearchContext Class
  * @file   IndexIterator.hpp
  * @author Corneliu-Claudiu Prodescu
  * @date   22 Jun 2014
  */

#include <assert.h>

#include <list>

#include "common/utils/compiler_defs.h"
#include "mws/index/index.h"

namespace mws {
namespace index {

template <class Accessor>
class IndexIterator {
    std::list<typename Accessor::Iterator> _backtrackIterators;
    int _arity;
    typename Accessor::Index* _index;
    typename Accessor::Node* _root;

 public:
    typedef std::list<typename Accessor::Iterator> PathContainer;

    IndexIterator() {}
    explicit IndexIterator(typename Accessor::Index* index)
        : _arity(1), _index(index), _root(Accessor::getRootNode(index)) {}
    IndexIterator(typename Accessor::Index* index,
                  typename Accessor::Node* root)
        : _arity(1), _index(index), _root(root) {}

    void set(typename Accessor::Index* index, typename Accessor::Node* root) {
        _arity = 1;
        _index = index;
        _root = root;
    }

    typename Accessor::Node* next() {
        if (_backtrackIterators.empty()) {  // add first branch
            _push(Accessor::getChildrenIterator(_root));
        } else {  // or find first unexplored branch
            while (!_backtrackIterators.back().hasNext()) {
                _pop();
                if (_backtrackIterators.empty()) {
                    assert(_arity == 1);
                    return nullptr;
                }
            }
            _next();
        }

        // complete expression
        while (_arity > 0) {
            auto node = Accessor::getNode(_index, _backtrackIterators.back());
            _push(Accessor::getChildrenIterator(node));
        }

        return Accessor::getNode(_index, _backtrackIterators.back());
    }

    PathContainer getPath() const { return _backtrackIterators; }

 protected:
    virtual void onPush(typename Accessor::Iterator iterator) {
        UNUSED(iterator);
    }
    virtual void onPop(typename Accessor::Iterator iterator) {
        UNUSED(iterator);
    }

 private:
    void _push(typename Accessor::Iterator iterator) {
        _arity += Accessor::getArity(iterator) - 1;
        _backtrackIterators.push_back(iterator);
        onPush(iterator);
    }

    void _pop() {
        _arity -= Accessor::getArity(_backtrackIterators.back()) - 1;
        onPop(_backtrackIterators.back());
        _backtrackIterators.pop_back();
    }

    void _next() {
        auto prevIterator = _backtrackIterators.back();
        _backtrackIterators.back().next();
        onPop(prevIterator);
        _arity += Accessor::getArity(_backtrackIterators.back()) -
                  Accessor::getArity(prevIterator);
        onPush(_backtrackIterators.back());
    }
};

}  // namespace index
}  // namespace mws

#endif  // _MWS_INDEX_INDEXITERATOR_HPP
