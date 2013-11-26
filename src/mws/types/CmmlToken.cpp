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
  * @brief  File containing the implementation of CmmlToken Class
  * @file   CmmlToken.cpp
  * @author Corneliu-Claudiu Prodescu <c.prodescu@jacobs-university.de>
  * @date   12 Oct 2010
  *
  * License: GPL v3
  *
  */



// System includes

#include <cassert>
#include <ctype.h>                     // C character types (isspace)
#include <vector>                      // STL vector headers
#include <string>                      // STL string headers
#include <map>
#include <stack>
#include <iostream>                    // C++ Input/Output stream headers

// Local includes

#include "mws/types/CmmlToken.hpp"     // CmmlToken class header
#include "mws/types/MeaningDictionary.hpp"  // MeaningDictionary class
#include "common/utils/ToString.hpp"   // ToString utility function

// Namespaces

using namespace std;

namespace mws { namespace types {

const string root_xpath_selector = "/m:semantics/m:annotation-xml[@encoding=\"MathML-Content\"]/*[1]";

CmmlToken::CmmlToken(bool aMode) :
    _tag( "" ),
    _textContent( "" ),
    _parentNode( NULL ),
    _mode( aMode )
{
    _xpath = root_xpath_selector;
}


CmmlToken*
CmmlToken::newRoot(bool aMode)
{
    return (new CmmlToken(aMode));
}


CmmlToken::~CmmlToken()
{
    while(!_childNodes.empty())
    {
        delete _childNodes.front();
        _childNodes.pop_front();
    }
}


void
CmmlToken::setTag(const std::string& aTag)
{
    if (aTag.compare(0, 2, "m:") == 0) {
        _tag = aTag.substr(2, aTag.size() - 2);
    } else {
        _tag            = aTag;
    }
}


void
CmmlToken::addAttribute(const std::string& anAttribute,
                        const std::string& aValue)
{
    _attributes.insert(make_pair(anAttribute, aValue));
}


void
CmmlToken::appendTextContent(const char* aTextContent,
                             size_t      nBytes)
{
    size_t i;

    _textContent.reserve(_textContent.size() + nBytes);
    for (i = 0; i < nBytes; i++)
    {
        // If not whitespace
        if (!isspace(aTextContent[i]))
            _textContent.append(1, aTextContent[i]);
    }
}


const string&
CmmlToken::getTextContent() const
{
    return _textContent;
}


CmmlToken*
CmmlToken::newChildNode()
{
    CmmlToken* result;

    result = new CmmlToken(_mode);
    _childNodes.push_back(result);
    result->_parentNode = this;
    result->_xpath = _xpath + "/*[" + ToString(_childNodes.size()) + "]";

    return result;
}


bool
CmmlToken::isRoot() const
{
    return (_parentNode == NULL);
}


bool
CmmlToken::isQvar() const
{
    return (getType() == VAR);
}


const string&
CmmlToken::getQvarName() const
{
    return _textContent;
}


CmmlToken*
CmmlToken::getParentNode() const
{
    return _parentNode;
}


const CmmlToken::PtrList&
CmmlToken::getChildNodes() const
{
    return _childNodes;
}


const string&
CmmlToken::getXpath() const
{
    return _xpath;
}


string
CmmlToken::getXpathRelative() const
{
    // xpath without initial /*[1]
    string xpath_relative(_xpath, root_xpath_selector.length(), string::npos);

    return xpath_relative;
}

string
CmmlToken::toString(int indent) const
{
    stringstream ss;
    string       padding;
    map<string, string> :: const_iterator mIt;
    PtrList :: const_iterator lIt;

    padding.append(indent, ' ');

    ss << padding << "<" << _tag << " ";

    for (mIt = _attributes.begin(); mIt != _attributes.end(); mIt ++)
    {
        ss << mIt->first << "=\"" << mIt->second << "\" ";
    }

    ss << ">" << _textContent;

    if (_childNodes.size())
    {
        ss << "\n";

        for (lIt = _childNodes.begin(); lIt != _childNodes.end(); lIt ++)
        {
            ss << (*lIt)->toString(indent + 2);
        }

        ss << padding;
    }

    ss << "</" << _tag << ">\n";

    return ss.str();
}

uint32_t
CmmlToken::getExprDepth() const {
    uint32_t max_depth = 0;
    for (PtrList::const_iterator it = _childNodes.begin();
         it != _childNodes.end(); it++) {

        uint32_t depth = (*it)->getExprDepth() + 1;
        if (depth > max_depth) max_depth = depth;
    }

    return max_depth;
}

uint32_t
CmmlToken::getExprSize() const {
    uint32_t size = 1; // counting current token

    for (PtrList::const_iterator it = _childNodes.begin();
         it != _childNodes.end(); it++) {

        size += (*it)->getExprSize();
    }

    return size;
}

CmmlToken::Type
CmmlToken::getType() const {
    if (_tag == MWS_QVAR_MEANING) {
        return VAR;
    } else {
        return CONSTANT;
    }
}

const std::string&
CmmlToken::getVarName() const {
    assert(getType() == VAR);

    return _textContent;
}

std::string
CmmlToken::getMeaning() const {
    // assert(getType() == CONSTANT); XXX legacy mwsd still uses this

    string meaning;
    if (_tag == MWS_QVAR_MEANING)
    {
        meaning = MWS_QVAR_MEANING;
    }
    else if (_tag == "apply" || _textContent.empty())
    {
        meaning = _tag;
    }
    else
    {
        // to avoid ambiguity between <ci>eq</ci> and <m:eq/>
        meaning = "#" + _textContent;
    }

    return meaning;
}

} }
