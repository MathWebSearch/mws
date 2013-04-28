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
  * @brief File containing the header of the SerializePage class.
  *
  * @author Daniel Hasegan
  * @date 27.III.2013
  */
#ifndef _SERIALIZEPAGE_HPP
#define _SERIALIZEPAGE_HPP

// System includes

#include <string.h>
#include <stdio.h>

// Local includes

#include "mws/types/NodeInfo.hpp"      // The information about Id
#include "mws/types/URLDictionary.hpp" // The URL Database for URL <-> URLId encode/decode

/**
  * @brief Class used to encode/decode the data stored in the database.
  */
class SerializePage
{
private:
    /// The ID of the URL stored
    mws::MeaningId url_id;
    /// The ID Fragment of the Resource
    const char* url_fragid;
    /// XPath of the formula
    const char* xpath;

    /// The serialized result
    char*       serialized;
    /// The serialized size
    size_t      size;

public:
    /**
     * @brief Code the Page data from url_id, url_fragid, xpath to serialized version
     * @param anUrl_id the internal id of the URL
     * @param anUrl_fragid the fragment if of the resource
     * @param anXpath the xpath of the formula
     */
    SerializePage(mws::MeaningId anUrl_id,
                  const char* anUrl_fragid,
                  const char* anXpath)
    {
        char*  url_fragid_aux;
        char*  xpath_aux;
        size_t len_fragid;
        size_t len_xpath;

        char*  serialized_aux;
        size_t              offset;

        // Create new arrays for url and xpath
        len_fragid       = strlen(anUrl_fragid);
        len_xpath        = strlen(anXpath);
        
        url_fragid_aux = new char[len_fragid + 1];
        strcpy(url_fragid_aux, anUrl_fragid);
        xpath_aux = new char[len_xpath + 1];
        strcpy(xpath_aux, anXpath);

        // Store
        this->url_id     = anUrl_id;
        this->url_fragid = url_fragid_aux;
        this->xpath      = xpath_aux;

        /**
         The way of serializing:
         Serialized: [offset][url_id][---fragid---][---xpath---]

        Where:
        MeaningId url_id is of fixed dimension
        size_t offset points to the position of xpath in the serialized (fixed dimension)
        char* fragid is the fragmented ID printed
        char* xpath the xpath printed
        */

        offset = sizeof(size_t) + sizeof(mws::MeaningId) + len_fragid;
        this->size = offset + len_xpath;

        // Copy in the bytes in serialized mode
        serialized_aux = new char[size];
        memcpy(serialized_aux, &offset, sizeof(size_t));
        memcpy(serialized_aux + sizeof(size_t), &url_id, sizeof(mws::MeaningId) );
        memcpy(serialized_aux + sizeof(size_t) + sizeof(mws::MeaningId), url_fragid, len_fragid);
        memcpy(serialized_aux + offset, xpath, len_xpath);
        this->serialized = serialized_aux;
    }

    /**
     * @brief Decode the Page data from serialized version to url_id, url_fragid, xpath
     * @param aSerialized the Serialized code to be decoded
     * @param aSize the size of the Serialized char
     */
    SerializePage(const char*  aSerialized,
              size_t aSize)
    {
        char*     serialized_aux;

        size_t      offset;
        mws::MeaningId  url_id_aux;
        size_t      len_fragid,len_xpath;


        char*     url_fragid_aux;
        char*     xpath_aux;

        // Get the encoded char
        serialized_aux = new char[aSize+1];
        memcpy(serialized_aux, aSerialized, sizeof(char)*aSize);
        this->serialized = serialized_aux;
        this->size       = aSize;

        // Retrieving the data back
        memcpy(&offset, serialized, sizeof(size_t));
        memcpy(&url_id_aux, serialized + sizeof(size_t) , sizeof(mws::MeaningId) );
        len_fragid = offset - sizeof(size_t) - sizeof(mws::MeaningId);
        len_xpath = size - offset;
        this->url_id = url_id_aux;

        // Allocate memory for the results
        url_fragid_aux       = new char[len_fragid + 1];
        xpath_aux            = new char[len_xpath + 1];

        // Copy in the important bits
        memcpy(url_fragid_aux , serialized + sizeof(size_t) + sizeof(mws::MeaningId) , len_fragid );
        memcpy(xpath_aux      , serialized + offset , len_xpath);

        // Put the ending '\0'
        url_fragid_aux[len_fragid] = '\0';
        xpath_aux[len_xpath]    = '\0';

        // Store the goodies
        this->url_fragid = url_fragid_aux;
        this->xpath = xpath_aux;
    }

    /// @brief Destructor for Serialize Page
    ~SerializePage()
    {
        delete []url_fragid;
        delete []xpath;
        delete []serialized;
    }

    /// @brief Getter for the url_id
    mws::MeaningId getUrlId() const
    {
        return url_id;
    }

    /// @brief Getter for the url_fragid
    const char* getFragId() const
    {
        return url_fragid;
    }

    /// @brief Getter for the xpath
    const char* getXpath() const
    {
        return xpath;
    }

    /// @brief Getter for the serialized
    char* getSerialized() const
    {
        return serialized;
    }

    /// @brief Getter for the size of the serialized
    size_t      getSize() const
    {
        return size;
    }
};

#endif // _MWSDBDATA_HPP
