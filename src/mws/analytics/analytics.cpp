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
  * @brief Run analytics on an index
  *
  * @file analytics.cpp
  * @author Corneliu Prodescu
  * @date 20 Jun 2014
  *
  */

#include <stdlib.h>

#include <stack>
using std::stack;
#include <string>
using std::string;
#include <stdexcept>
using std::exception;

#include "mws/analytics/analytics.hpp"
#include "common/utils/FlagParser.hpp"
using common::utils::FlagParser;
#include "mws/index/IndexLoader.hpp"
using mws::index::IndexLoader;
using mws::index::LoadingOptions;
#include "mws/index/IndexAccessor.hpp"
using mws::index::IndexAccessor;
#include "mws/index/CallbackIndexIterator.hpp"
using mws::index::CallbackIndexIterator;
#include "mws/index/MeaningDictionary.hpp"
using mws::index::MeaningDictionary;
#include "mws/index/ExpressionEncoder.hpp"
using mws::index::ExpressionDecoder;
#include "mws/types/CmmlToken.hpp"
using mws::types::CmmlToken;

/**
 * @brief Depth-first CmmlToken builder
 */
class CmmlTokenBuilder {
    CmmlToken* _root;
    stack<CmmlToken*> _futureParents;
    stack<CmmlToken*> _tokens;
    const ExpressionDecoder _decoder;

 public:
    explicit CmmlTokenBuilder(const MeaningDictionary* meaningDictionary);
    void pushToken(encoded_token_t encodedToken);
    void popToken(encoded_token_t encodedToken);
    const CmmlToken* get();
};

namespace mws {
namespace analytics {

void analyze(IndexLoader* indexLoader) {
    const index_handle_t* index = indexLoader->getIndexHandle();
    const inode_t* root = IndexAccessor::getRootNode(index);
    if (analyze_begin(index, root) == ANALYTICS_STOP) return;

    CmmlTokenBuilder cmmlBuilder(indexLoader->getMeaningDictionary());
    auto onPush = [&](IndexAccessor::Iterator iterator) {
        cmmlBuilder.pushToken(IndexAccessor::getToken(iterator));
    };
    auto onPop = [&](IndexAccessor::Iterator iterator) {
        cmmlBuilder.popToken(IndexAccessor::getToken(iterator));
    };

    CallbackIndexIterator<IndexAccessor> iterator(index, root, onPush, onPop);

    const inode_t* node;
    while ((node = iterator.next()) != nullptr) {
        const leaf_t* leaf = reinterpret_cast<const leaf_t*>(node);
        auto status = analyze_expression(cmmlBuilder.get(), leaf->num_hits);
        if (status == ANALYTICS_STOP) {
            break;
        }
    }

    analyze_end();
}

}  // namespace analytics
}  // namespace mws

int main(int argc, char* argv[]) {
    FlagParser::addFlag('I', "index-path", FLAG_REQ, ARG_REQ);

    if ((FlagParser::parse(argc, argv)) != 0) {
        fprintf(stderr, "%s", FlagParser::getUsage().c_str());
        return EXIT_FAILURE;
    }

    const string indexPath = FlagParser::getArg('I').c_str();

    try {
        LoadingOptions loadingOptions;
        loadingOptions.includeHits = false;
        IndexLoader data(indexPath, loadingOptions);
        mws::analytics::analyze(&data);
    }
    catch (exception& e) {
        PRINT_WARN("%s\n", e.what());
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}

CmmlTokenBuilder::CmmlTokenBuilder(const MeaningDictionary* meaningDictionary)
    : _root(nullptr), _decoder(*meaningDictionary) {}

const CmmlToken* CmmlTokenBuilder::get() { return _root; }

void CmmlTokenBuilder::pushToken(encoded_token_t encodedToken) {
    CmmlToken* token;
    if (_futureParents.empty()) {  // this is a root
        assert(_root == nullptr);
        token = CmmlToken::newRoot();
        _root = token;
    } else {
        token = _futureParents.top()->newChildNode();
        _futureParents.pop();
    }

    string meaning = _decoder.getMeaning(encodedToken.id);
    std::size_t separator_pos = meaning.find('#');
    assert(separator_pos != std::string::npos);
    string tag = meaning.substr(0, separator_pos);
    string textContent = meaning.substr(separator_pos + 1);
    token->setTag(tag);
    token->appendTextContent(textContent);

    uint32_t arity = encodedToken.arity;
    while (arity--) {
        _futureParents.push(token);
    }
    _tokens.push(token);
}

void CmmlTokenBuilder::popToken(encoded_token_t encodedToken) {
    CmmlToken* token = _tokens.top();
    assert(token->getChildNodes().size() == 0);
    _tokens.pop();
    uint32_t arity = encodedToken.arity;
    while (arity--) {
        _futureParents.pop();
    }
    CmmlToken* parent = token->getParentNode();
    if (parent != nullptr) {
        parent->popLastChild();
        _futureParents.push(parent);
    } else {
        assert(_futureParents.empty());
        _root = nullptr;
    }
    delete token;
}
