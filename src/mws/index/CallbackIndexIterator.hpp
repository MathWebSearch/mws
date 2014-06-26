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
#ifndef _MWS_INDEX_CALLBACKINDEXITERATOR_HPP
#define _MWS_INDEX_CALLBACKINDEXITERATOR_HPP

/**
  * @brief  Index Iterator with push/pop callbacks
  * @file   CallbackIndexIterator.hpp
  * @author Corneliu-Claudiu Prodescu
  * @date   22 Jun 2014
  */

#include <functional>

#include "mws/index/IndexIterator.hpp"

namespace mws {
namespace index {

template <class Accessor>
class CallbackIndexIterator : public IndexIterator<Accessor> {
    typedef std::function<void(typename Accessor::Iterator)> Callback;
    Callback _onPush;
    Callback _onPop;

 public:
    CallbackIndexIterator(typename Accessor::Index* index,
                          typename Accessor::Node* root,
                          Callback onPushCallback, Callback onPopCallback)
        : IndexIterator<Accessor>(index, root),
          _onPush(onPushCallback),
          _onPop(onPopCallback) {}

 protected:
    virtual void onPush(typename Accessor::Iterator iterator) {
        _onPush(iterator);
    }
    virtual void onPop(typename Accessor::Iterator iterator) {
        _onPop(iterator);
    }
};

}  // namespace index
}  // namespace mws

#endif  // _MWS_INDEX_CALLBACKINDEXITERATOR_HPP
