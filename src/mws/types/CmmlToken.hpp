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
#ifndef _MWS_TYPES_CMMLTOKEN_HPP
#define _MWS_TYPES_CMMLTOKEN_HPP

/**
  * @brief  Content MathML Token
  * @file   CmmlToken.hpp
  * @author Corneliu-Claudiu Prodescu <c.prodescu@jacobs-university.de>
  * @date   12 Oct 2013
  *
  * License: GPL v3
  *
  */

#include <list>
#include <map>
#include <string>
#include <functional>
#include <utility>

#include "common/utils/compiler_defs.h"

namespace mws {
namespace types {

class CmmlToken;

typedef std::string Meaning;
typedef std::function<void(const CmmlToken* token)> TokenCallback;

/**
  * @brief Class encapsulating the properties of a ContentMathML Token
  */
class CmmlToken {
 public:
    typedef std::list<CmmlToken*> PtrList;

 private:
    /// Tag name
    std::string _tag;
    /// Attributes list
    std::map<std::string, std::string> _attributes;
    /// Text content within the XML node
    std::string _textContent;
    /// List of child nodes
    PtrList _childNodes;
    /// Pointer to parent node
    CmmlToken* _parentNode;
    /// Xpath of current node relative to the root
    std::string _xpath;

 public:
    enum Type {
        VAR,
        RANGE,
        CONSTANT
    };

    ~CmmlToken();
    static CmmlToken* newRoot();
    CmmlToken* newChildNode();

    void setTag(const std::string& aTag);
    void addAttribute(const std::string& anAttribute,
                      const std::string& aValue);
    void appendTextContent(const char* aTextContent, size_t nBytes);
    void appendTextContent(const std::string& textContent);
    void popLastChild();
    bool isRoot() const;
    bool isVar() const;
    bool isRange() const;
    const std::string& getTextContent() const;
    const PtrList& getChildNodes() const;
    CmmlToken* getParentNode() const;
    const std::string& getTag() const;
    const std::string& getXpath() const;
    /**
     * @brief getXpathRelative
     * @return Xpath wihout root selector prefix
     */
    std::string getXpathRelative() const;

    Type getType() const;
    // VAR specific methods
    const std::string& getVarName() const;
    // RANGE specific methods
    const std::pair<double, double> getRangeBounds() const;
    // CONSTANT specific
    std::string getMeaning() const;

    // Logging / stats
    std::string toString(int indent = 0) const;
    uint32_t getExprDepth() const;
    uint32_t getExprSize() const;
    uint32_t getArity() const;
    bool equals(const CmmlToken* t) const;

    /**
     * @brief Apply the callback to all subexpressions
     * @param callback callback to be called on each subexpression
     */
    void foreachSubexpression(TokenCallback callback) const;

 private:
    CmmlToken();

    DISALLOW_COPY_AND_ASSIGN(CmmlToken);
};

}  // namespace types
}  // namespace mws

#endif
