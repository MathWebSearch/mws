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
  * @brief  File containing the implementation of MwsSearchContext Class
  * @file   SearchContext.cpp
  * @author Corneliu-Claudiu Prodescu <c.prodescu@jacobs-university.de>
  * @date   25 May 2011
  *
  * @edit Radu Hambasan
  * @date 10 Nov 2014
  *
  * License: GPL v3
  *
  */

#include <cstdlib>
#include <cstring>

#include <list>
using std::list;
#include <map>
using std::map;
#include <utility>
using std::pair;
using std::make_pair;
#include <vector>
using std::vector;
#include <string>
using std::string;
#include <memory>
using std::unique_ptr;

#include "common/utils/ContainerIterator.hpp"
using common::utils::ContainerIterator;
#include "mws/index/encoded_token.h"
#include "mws/index/MeaningDictionary.hpp"
using mws::index::MeaningDictionary;
#include "mws/index/ExpressionEncoder.hpp"
using mws::index::ExpressionDecoder;
#include "mws/index/TmpIndexAccessor.hpp"
using mws::index::TmpIndexAccessor;
#include "mws/index/IndexAccessor.hpp"
using mws::index::IndexAccessor;
#include "mws/types/FormulaPath.hpp"
using mws::types::FormulaPath;
using mws::types::FormulaId;
#include "mws/index/IndexIterator.hpp"
using mws::index::IndexIterator;
#include "mws/dbc/CrawlDb.hpp"
using mws::dbc::CrawlData;
#include "mws/query/SearchContext.hpp"

namespace mws {
namespace query {

constexpr char NUMBER_PREF[] = "cn#";

template <class Accessor>
struct BacktrackCtxt {
    bool isSolved;
    BacktrackCtxt() : isSolved(false) {}
    virtual typename Accessor::Node* solve(typename Accessor::Index* index,
                                           typename Accessor::Node* root) = 0;
    virtual typename Accessor::Node* nextSol() = 0;

    virtual ~BacktrackCtxt() {}
};

template <class Accessor>
struct QvarCtxt : public BacktrackCtxt<Accessor> {
    IndexIterator<Accessor> iterator;

    typename Accessor::Node* solve(typename Accessor::Index* index,
                                   typename Accessor::Node* root) {
        iterator.set(index, root);
        typename Accessor::Node* node = iterator.next();
        if (node != nullptr) {
            this->isSolved = true;
        }
        return node;
    }

    typename Accessor::Node* nextSol() {
        typename Accessor::Node* node = iterator.next();
        if (node == nullptr) {
            this->isSolved = false;
        }
        return node;
    }
};

template <class Accessor>
struct RangeCtxt : public BacktrackCtxt<Accessor> {
    pair<double, double> bounds;
    ExpressionDecoder* decoder;

    RangeCtxt(pair<double, double> bounds, ExpressionDecoder* decoder)
        : bounds(bounds), decoder(decoder) {}

    typename Accessor::Index* index;
    typename Accessor::Node* root;
    typename Accessor::Iterator* iterator;

    typename Accessor::Node* solve(typename Accessor::Index* index,
                                   typename Accessor::Node* root) {
        this->index = index;
        this->root = root;

        // XXX:
        typename Accessor::Iterator it = Accessor::getChildrenIterator(root);
        // malloc'ed because there is no default constructor
        iterator = (typename Accessor::Iterator*)malloc(sizeof(it));
        assert(iterator != nullptr);
        *iterator = it;

        encoded_token_t tok = Accessor::getToken(*iterator);
        while (!validSubst(tok) && iterator->hasNext()) {
            iterator->next();
            tok = Accessor::getToken(*iterator);
        }

        if (!validSubst(tok)) {
            this->isSolved = false;
            return nullptr;
        }

        typename Accessor::Node* node = Accessor::getChild(index, root, tok);
        assert(node != nullptr);
        this->isSolved = true;

        return node;
    }

    typename Accessor::Node* nextSol() {
        if (!iterator->hasNext()) {
            this->isSolved = false;
            return nullptr;
        }

        iterator->next();
        encoded_token_t tok = Accessor::getToken(*iterator);
        while (!validSubst(tok) && iterator->hasNext()) {
            iterator->next();
            tok = Accessor::getToken(*iterator);
        }

        if (!validSubst(tok)) {
            this->isSolved = false;
            return nullptr;
        }

        typename Accessor::Node* node = Accessor::getChild(index, root, tok);
        assert(node != nullptr);
        this->isSolved = true;

        return node;
    }

    ~RangeCtxt() { free(iterator); }

 private:
    bool isCmmlNumber(encoded_token_t tok) {
        if (tok.arity != 0) return false;

        string meaning = decoder->getMeaning(tok.id);
        const string NR_PREF(NUMBER_PREF);
        if (meaning.substr(0, NR_PREF.size()) == NR_PREF) {
            return true;
        }

        return false;
    }

    bool isInRange(encoded_token_t tok) {
        string cnMeaning = decoder->getMeaning(tok.id);
        const string NR_PREF(NUMBER_PREF);
        string val = cnMeaning.substr(NR_PREF.size());

        double num;
        try {
            num = std::stod(val);
        }
        catch (std::exception & e) {
            UNUSED(e);
            return false;
        }

        return (bounds.first <= num && num <= bounds.second);
    }

    // returns true if tok is a valid substitution
    bool validSubst(encoded_token_t tok) {
        if (!isCmmlNumber(tok)) return false;
        if (!isInRange(tok)) return false;
        return true;
    }
};

SearchContext::_NodeTriple::_NodeTriple(TokType type, MeaningId aMeaningId,
                                        Arity anArity)
    : type(type), meaningId(aMeaningId), arity(anArity) {}

SearchContext::SearchContext(const vector<encoded_token_t>& encodedFormula,
                             const types::Query::Options& options,
                             const RangeBounds& rangeBounds,
                             const MeaningDictionary* meaningDict)
    : options(options), rangeBounds(rangeBounds) {
    if (meaningDict != nullptr) {
        decoder.reset(new ExpressionDecoder(*meaningDict));
    }

    map<MeaningId, int> indexedQvars;
    int tokenCount = 0;
    int specialCount = 0;  // special vars, i.e. ranges & qvars
    for (encoded_token_t encodedToken : encodedFormula) {
        MeaningId meaningId = encodedToken.id;

        if (encoded_token_is_var(encodedToken)) {           // qvar
            if (encoded_token_is_anon_var(encodedToken)) {  // anonymous qvar
                expr.push_back(_NodeTriple(QVAR, meaningId, specialCount));
                backtrackPoints.push_back(tokenCount + 1);
                specialCount++;
            } else {  // named qvar
                auto mapIt = indexedQvars.find(meaningId);
                if (mapIt == indexedQvars.end()) {
                    indexedQvars.insert(make_pair(meaningId, specialCount));
                    expr.push_back(_NodeTriple(QVAR, meaningId, specialCount));
                    backtrackPoints.push_back(tokenCount + 1);
                    specialCount++;
                } else {
                    expr.push_back(_NodeTriple(QVAR, meaningId, mapIt->second));
                }
            }
        } else if (encoded_token_is_range(encodedToken)) {
            expr.push_back(_NodeTriple(RANGE, meaningId, specialCount));
            backtrackPoints.push_back(tokenCount + 1);
            specialCount++;
        } else {  // constant
            expr.push_back(_NodeTriple(CONST, meaningId, encodedToken.arity));
        }

        tokenCount++;
    }

    mSpecialCount = specialCount;
}

template <class A /* Accessor */>
MwsAnswset* SearchContext::getResult(typename A::Index* index,
                                     dbc::DbQueryManager* dbQueryManager,
                                     unsigned int offset, unsigned int size,
                                     unsigned int maxTotal) {
    // Table containing resolved Qvar/ranges and backtrack points
    vector<unique_ptr<BacktrackCtxt<A>>> bkTable;
    // setup the backtrack table:
    bkTable.resize(mSpecialCount);
    uint32_t bkTablePos = 0;
    for (_NodeTriple i : expr) {
        if (i.type == CONST) continue;

        uint32_t specialId = i.arity;
        if (specialId < bkTablePos) continue;  // we already setup this

        if (i.type == QVAR) {
            bkTable[bkTablePos].reset(new QvarCtxt<A>());
        } else {
            assert(i.type == RANGE);
            auto it = rangeBounds.find(i.meaningId);
            assert(it != rangeBounds.end());
            pair<double, double> limits = it->second;
            bkTable[bkTablePos].reset(new RangeCtxt<A>(limits, decoder.get()));
        }
        bkTablePos++;
    }

    auto result = new MwsAnswset();
    size_t currentToken = 0;  // index for the expression vector
    unsigned int found = 0;   // # of found matches
    int lastSolved = -1;      // last qvar/range that was solved
    typename A::Node* currentNode = A::getRootNode(index);

    // Checking the arguments
    if (offset + size > maxTotal) {
        if (maxTotal <= offset) {
            size = 0;
        } else {
            size = maxTotal - offset;
        }
    }

    // Retrieving the solutions
    while (found < maxTotal) {
        // By default not backtracking
        bool backtrack = false;

        // Evaluating current token and deciding if to go ahead or backtrack
        if (currentToken < expr.size()) {
            TokType currType = expr[currentToken].type;
            if (currType == QVAR) {
                int qvarId = expr[currentToken].arity;
                if (bkTable[qvarId]->isSolved) {
                    QvarCtxt<A>* qCtxt =
                        dynamic_cast<QvarCtxt<A>*>(bkTable[qvarId].get());
                    for (auto& elem : qCtxt->iterator.getPath()) {
                        encoded_token_t token = A::getToken(elem);
                        currentNode = A::getChild(index, currentNode, token);
                        if (currentNode == nullptr) {
                            backtrack = true;
                            break;
                        }
                    }
                } else {
                    currentNode = bkTable[qvarId]->solve(index, currentNode);
                    if (currentNode != nullptr) {
                        lastSolved = qvarId;
                    } else {
                        backtrack = true;
                    }
                }
            } else if (currType == RANGE) {
                int rangeId = expr[currentToken].arity;
                currentNode = bkTable[rangeId]->solve(index, currentNode);

                if (currentNode != nullptr) {
                    lastSolved = rangeId;
                } else {
                    backtrack = true;
                }
            } else {
                assert(currType == CONST);
                encoded_token_t token = encoded_token(
                    expr[currentToken].meaningId, expr[currentToken].arity);
                currentNode = A::getChild(index, currentNode, token);
                if (currentNode == nullptr) {
                    backtrack = true;
                }
            }
        } else {
            // Handling the solutions
            size_t hitsCount;
            if (options.includeHits) {
                hitsCount = A::getHitsCount(currentNode);
            } else {
                hitsCount = 1;
            }

            if (found < size + offset && found + hitsCount > offset) {
                if (options.includeHits) {
                    FormulaId formulaId = A::getFormulaId(currentNode);
                    unsigned dbOffset;
                    unsigned dbMaxSize;
                    if (offset < found) {
                        dbOffset = 0;
                        dbMaxSize = size + offset - found;
                    } else {
                        dbOffset = offset - found;
                        dbMaxSize = size;
                    }

                    auto callback = [result](const FormulaPath & formulaPath,
                                             const CrawlData & crawlData) {
                        auto answer = new mws::types::Answer();
                        answer->data = crawlData;
                        answer->uri = formulaPath.xmlId;
                        answer->xpath = formulaPath.xpath;
                        result->answers.push_back(answer);
                        return 0;
                    }
                    ;

                    dbQueryManager->query(formulaId, dbOffset, dbMaxSize,
                                          callback);
                }
                if (options.includeMwsIds) {
                    result->ids.insert(A::getFormulaId(currentNode));
                }
            }

            found += hitsCount;

            // making sure we haven't surpassed maxTotal
            if (found > maxTotal) found = maxTotal;

            // backtracking to the next
            backtrack = true;
        }

        if (backtrack) {
            // Backtracking or going to the next expression token
            // starting with the last
            while (lastSolved >= 0 &&
                   nullptr == (currentNode = bkTable[lastSolved]->nextSol())) {
                lastSolved--;
            }

            if (lastSolved == -1) {
                // No more solutions
                break;
            } else {
                currentToken = backtrackPoints[lastSolved];
            }
        } else {
            currentToken++;
        }
    }
    result->total = found;

    return result;
}

// Declare specializations

template MwsAnswset* SearchContext::getResult<TmpIndexAccessor>(
    TmpIndexAccessor::Index* index, dbc::DbQueryManager* dbQueryManger,
    unsigned int offset, unsigned int size, unsigned int maxTotal);

template MwsAnswset* SearchContext::getResult<IndexAccessor>(
    IndexAccessor::Index* index, dbc::DbQueryManager* dbQueryManger,
    unsigned int offset, unsigned int size, unsigned int maxTotal);

}  // namespace query
}  // namespace mws
