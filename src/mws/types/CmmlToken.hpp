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
#ifndef _CMMLTOKEN_HPP
#define _CMMLTOKEN_HPP

/**
  * @brief  File containing the header of CmmlToken Class
  * @file   CmmlToken.hpp
  * @author Corneliu-Claudiu Prodescu <c.prodescu@jacobs-university.de>
  * @date   12 Oct 2010
  *
  * License: GPL v3
  *
  */

// System includes

#include <list>                        // STL list headers
#include <map>                         // STL map headers
#include <string>                      // STL string headers

// Local includes

#include "mws/types/NodeInfo.hpp"

namespace mws
{

/**
  * @brief Class encapsulating the properties of a ContentMathML Token
  */
class CmmlToken
{
    // Instance variables
private:
    /// Tag name
    std::string                        _tag;
    /// Attributes list
    std::map<std::string, std::string> _attributes;
    /// Text content within the XML node
    std::string                        _textContent;
    /// List of child nodes
    std::list<CmmlToken*>              _childNodes;
    /// Pointer to parent node
    CmmlToken*                         _parentNode;
    /// Xpath of current node relative to the root
    std::string                        _xpath;
    /// Mode (Harvest or Query)
    bool                               _mode;
    /// true if Meaning / MeaningId acurate
    mutable bool                       _meaningIdValid;
    /// Meaning of the token
    mutable mws::Meaning               _meaning;
    /// Meaning Id of the token as inserted in MeaningDictionary
    mutable mws::MeaningId             _meaningId;
public:
    /**
      * @brief Method to get an instance of a CmmlToken (which can be used as
      * root).
      * @param aMode is the mode of the token (true if it is allowed to
      * make changes to the MeaningDictionary and false otherwise). This
      * is used as true for Harvests and false for Queries.
      * @return a pointer to a newly created instance of CmmlToken.
      */
    static CmmlToken* newRoot(bool aMode);
    /**
      * @brief Destructor of the CmmlToken class.
      */
    ~CmmlToken();

    void                         setTag(const std::string& aTag);
    void                         addAttribute(const std::string& anAttribute,
                                              const std::string& aValue);
    void                         appendTextContent(const char* aTextContent,
                                                   size_t      nBytes);
    CmmlToken*                   newChildNode();
    bool                         isRoot() const;
    bool                         isQvar() const;
    const std::string&           getTextContent() const;
    const std::list<CmmlToken*>& getChildNodes() const;
    CmmlToken*                   getParentNode() const;
    const std::string&           getXpath() const;
    // Return xpath without leading root selector (useful for concatenation)
    std::string                  getXpathRelative() const;
    const std::string&           getQvarName() const;
    const MeaningId&             getMeaningId() const;
    std::string                  toString(int indent=0) const;
private:
    void _updateMeaningId() const;
protected:
    /**
      * Declared protected to avoid instantiation.
      * @brief Constructor of the CmmlToken class.
      * @param aMode is the mode of the token (true if it is allowed to
      * make changes to the MeaningDictionary and false otherwise). This
      * is used as true for Harvests and false for Queries.
      */
    CmmlToken(bool aMode);
    /**
      * Declared protected to avoid copies of the objects.
      * @brief Copy constructor of the CmmlToken class.
      */
    CmmlToken(CmmlToken const&);
};

}

#endif
