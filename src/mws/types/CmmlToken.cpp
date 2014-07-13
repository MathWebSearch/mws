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
  * @brief  Content MathML Token implementation
  * @file   CmmlToken.cpp
  * @author Corneliu-Claudiu Prodescu <c.prodescu@jacobs-university.de>
  * @date   12 Oct 2010
  *
  * License: GPL v3
  *
  */

#include <assert.h>
#include <ctype.h>
#include <string.h>

#include <map>
using std::map;
#include <sstream>
using std::stringstream;
#include <string>
using std::string;
#include <stack>
using std::stack;

#include "mws/types/CmmlToken.hpp"

namespace mws {
namespace types {

constexpr char QVAR_TAG[] = "mws:qvar";
constexpr char VAR_NAME_ATTRIBUTE[] = "name";
constexpr char ROOT_XPATH_SELECTOR[] = "/*[1]";
const Meaning QVAR_MEANING = "mws:qvar";

CmmlToken::CmmlToken()
    : _tag(""),
      _textContent(""),
      _parentNode(nullptr),
      _xpath(ROOT_XPATH_SELECTOR) {}

CmmlToken* CmmlToken::newRoot() { return (new CmmlToken()); }

CmmlToken* CmmlToken::newChildNode() {
    CmmlToken* result;

    result = new CmmlToken();
    _childNodes.push_back(result);
    result->_parentNode = this;
    result->_xpath = _xpath + "/*[" + std::to_string(_childNodes.size()) + "]";

    return result;
}

CmmlToken::~CmmlToken() {
    stack<CmmlToken*> tokStack;

    while (!_childNodes.empty()) {
        tokStack.push(_childNodes.front());
        _childNodes.pop_front();
    }

    while (!tokStack.empty()) {
        CmmlToken* curr = tokStack.top();
        tokStack.pop();
        auto& childrenCurr = curr->_childNodes;
        while (!childrenCurr.empty()) {
            tokStack.push(childrenCurr.front());
            childrenCurr.pop_front();
        }

        delete curr;
    }
}

void CmmlToken::setTag(const std::string& aTag) {
    if (aTag.compare(0, 2, "m:") == 0) {
        _tag = aTag.substr(2, aTag.size() - 2);
    } else {
        _tag = aTag;
    }
}

void CmmlToken::addAttribute(const std::string& anAttribute,
                             const std::string& aValue) {
    _attributes.insert(make_pair(anAttribute, aValue));
}

void CmmlToken::appendTextContent(const char* aTextContent, size_t nBytes) {
    _textContent.reserve(_textContent.size() + nBytes);
    for (size_t i = 0; i < nBytes; i++) {
        if (!isspace(aTextContent[i])) {
            _textContent.append(1, aTextContent[i]);
        }
    }
}

void CmmlToken::appendTextContent(const string& textContent) {
    appendTextContent(textContent.c_str(), textContent.size());
}

void CmmlToken::popLastChild() {
    _childNodes.pop_back();
}

const string& CmmlToken::getTextContent() const { return _textContent; }

const string& CmmlToken::getTag() const { return _tag; }

bool CmmlToken::isRoot() const { return (_parentNode == nullptr); }

bool CmmlToken::isVar() const { return (getType() == VAR); }

CmmlToken* CmmlToken::getParentNode() const { return _parentNode; }

const CmmlToken::PtrList& CmmlToken::getChildNodes() const {
    return _childNodes;
}

const string& CmmlToken::getXpath() const { return _xpath; }

string CmmlToken::getXpathRelative() const {
    // xpath without initial /*[1]
    string xpath_relative(_xpath, strlen(ROOT_XPATH_SELECTOR), string::npos);

    return xpath_relative;
}

string CmmlToken::toString(int indent) const {
    stringstream ss;
    string padding;
    map<string, string>::const_iterator mIt;
    PtrList::const_iterator lIt;

    padding.append(indent, ' ');

    ss << padding << "<" << _tag << " ";

    for (mIt = _attributes.begin(); mIt != _attributes.end(); mIt++) {
        ss << mIt->first << "=\"" << mIt->second << "\" ";
    }

    ss << ">" << _textContent;

    if (_childNodes.size()) {
        ss << "\n";

        for (lIt = _childNodes.begin(); lIt != _childNodes.end(); lIt++) {
            ss << (*lIt)->toString(indent + 2);
        }

        ss << padding;
    }

    ss << "</" << _tag << ">\n";

    return ss.str();
}

/// @todo recursive
uint32_t CmmlToken::getExprDepth() const {
    uint32_t max_depth = 0;
    for (auto child : _childNodes) {
        uint32_t depth = child->getExprDepth() + 1;
        if (depth > max_depth) max_depth = depth;
    }

    return max_depth;
}

/// @todo recursive
uint32_t CmmlToken::getExprSize() const {
    uint32_t size = 1;  // counting current token

    for (auto child : _childNodes) {
        size += child->getExprSize();
    }

    return size;
}

CmmlToken::Type CmmlToken::getType() const {
    if (_tag == QVAR_TAG) {
        return VAR;
    } else {
        return CONSTANT;
    }
}

const std::string& CmmlToken::getVarName() const {
    assert(getType() == VAR);

    auto it = _attributes.find(VAR_NAME_ATTRIBUTE);
    if (it == _attributes.end()) {
        return _textContent;
    } else {
        return it->second;
    }
}

std::string CmmlToken::getMeaning() const {
    assert(getType() == CONSTANT);

    if (_tag == "mtext") { // mtext content is discarded
        return "mtext#";
    }

    return _tag + "#" + _textContent;
}

uint32_t CmmlToken::getArity() const { return _childNodes.size(); }

/// @todo untested, recursive
bool CmmlToken::equals(const CmmlToken* t) const {
    if (getType() != t->getType()) return false;
    switch (getType()) {
    case VAR:
        return getVarName() == t->getVarName();
    case CONSTANT: {
        if (getMeaning() != t->getMeaning()) return false;
        if (_childNodes.size() != t->_childNodes.size()) return false;

        PtrList::const_iterator it1, it2;
        it1 = _childNodes.begin();
        it2 = t->_childNodes.begin();
        while (it1 != _childNodes.end()) {
            if (!(*it1)->equals(*it2)) return false;

            it1++;
            it2++;
        }
    }
    }

    return true;
}

void CmmlToken::foreachSubexpression(TokenCallback callback) const {
    stack<const CmmlToken*> subtermStack;

    subtermStack.push(this);
    while (!subtermStack.empty()) {
        const CmmlToken* currentSubterm = subtermStack.top();
        subtermStack.pop();

        for (auto rIt = currentSubterm->getChildNodes().rbegin();
             rIt != currentSubterm->getChildNodes().rend(); rIt++) {
            subtermStack.push(*rIt);
        }

        callback(currentSubterm);
    }
}

}  // namespace types
}  // namespace mws
