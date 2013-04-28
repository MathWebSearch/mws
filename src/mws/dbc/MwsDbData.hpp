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
#ifndef _MWSDBDATA_HPP
#define _MWSDBDATA_HPP

/**
  * @brief File containing the header of the MwsDbData class.
  * @file MwsDbData.hpp
  * @author Corneliu-Claudiu Prodescu
  * @date 17 May 2011
  *
  * @modified Daniel Hasegan
  * @date 23 Jan 2013
  *
  * License: GPL v3
  *
  */

// System includes

#include <string.h>
#include <stdio.h>

// Local includes

#include "mws/types/NodeInfo.hpp"      // The information about Id
#include "mws/types/URLDictionary.hpp" // The URL Database for URL <-> URLId encode/decode

/**
  * @brief Class used to encode/decode the data stored in the database.
  */
class MwsDbData
{
private:
    const char* url_uuid;
    const char* xpath;
    char*       serialized;
    size_t      size;

public:
    MwsDbData(const char* anUrl_uuid,
              const char* anXpath)
    {
        char*  url_uuid_aux;
        char*  xpath_aux;
        char*  serialized_aux;
        size_t len1;
        size_t len2;

        char*               url_ToEncode;
        mws::MeaningId      url_encoded_id;
        char*               url_id;
        size_t              lenId;
        size_t              offset;
        size_t              pos;


        // Keep the url and xpath in pure form
        len1       = strlen(anUrl_uuid); 
        len2       = strlen(anXpath);
        
        url_uuid_aux = new char[len1 + 1];
        strcpy(url_uuid_aux, anUrl_uuid);
        xpath_aux = new char[len2 + 1];
        strcpy(xpath_aux, anXpath);

        this->url_uuid = url_uuid_aux;
        this->xpath    = xpath_aux;

        // Get the content after the hash tag in the URL 
        pos = len1;
        for(size_t i=len1 - 1; i < len1 && pos == len1 ; --i)
            if (url_uuid[i] == '#')
            {
                pos = i;
            }
        if (pos < len1) // If we found an Id
        {
            url_ToEncode = new char[pos+1];
            strncpy(url_ToEncode, anUrl_uuid, pos);
            url_ToEncode[pos] = '\0';

            lenId = len1-pos-1;
            url_id = new char[lenId + 1];
            strcpy(url_id, anUrl_uuid + pos + 1);
            url_id[lenId] = '\0';
        }
        else 
        {
            url_ToEncode = new char[len1+1];
            strcpy(url_ToEncode, anUrl_uuid);

            lenId = 0;
            url_id = new char[lenId + 1];
            url_id[lenId] = '\0';
        }

        // Insert into URLDictionary
// Deprecated:        url_encoded_id = mws::URLDictionary::put( (mws::Meaning) url_ToEncode );

        offset = sizeof(size_t) + sizeof(mws::MeaningId) + lenId;
        this->size = offset + len2;

        // Copy in the important bits
        serialized_aux = new char[size];
        memcpy(serialized_aux, &offset, sizeof(size_t));
        memcpy(serialized_aux + sizeof(size_t), &url_encoded_id, sizeof(mws::MeaningId) );
        memcpy(serialized_aux + sizeof(size_t) + sizeof(mws::MeaningId), url_id, lenId);
        memcpy(serialized_aux + offset, xpath, len2);
        this->serialized = serialized_aux;

        // Cleaup
        delete []url_ToEncode;
        delete []url_id;
    }

    MwsDbData(const char*  aSerialized,
              size_t aSize)
    {
        size_t    offset;
        char*     url_uuid_aux;
        char*     xpath_aux;
        char*     serialized_aux;
        size_t    lenUrl;
        size_t    lenId;
        size_t    lenXPath;
        size_t    lenDecodedUrl;

        mws::MeaningId url_encoded_id;
        mws::Meaning   url_decoded;
        char*     url_id = NULL;

        // Get the encoded char
        serialized_aux = new char[aSize];
        memcpy(serialized_aux, aSerialized, aSize);
        this->serialized = serialized_aux;
        this->size       = aSize;

        // Retrieving the offsets 
        memcpy(&offset, serialized, sizeof(size_t));
        memcpy(&url_encoded_id, serialized + sizeof(size_t) , sizeof(mws::MeaningId) );
        lenId    = offset - sizeof(size_t) - sizeof(mws::MeaningId);
        lenXPath = size - offset;

        // Get the Decoded URL from URLDictionary
 /* deprecated:      if ((url_decoded = mws::URLDictionary::get(url_encoded_id))
                == mws::MWS_MEANING_NO_URL)
        {
            fprintf(stderr,"Something went terribly wrong in MwsDbData.hpp! Please report this error! The returned Id was: %d\n",url_encoded_id);
        }*/
        lenDecodedUrl = url_decoded.size();
        lenUrl = lenDecodedUrl + 1 + lenId;

        // Allocate memory for the results
        url_uuid_aux       = new char[lenUrl + 1];
        xpath_aux          = new char[lenXPath + 1];

        // Copy in the important bits
        memcpy(url_uuid_aux                    , url_decoded.c_str() , lenDecodedUrl );
        memcpy(url_uuid_aux + lenDecodedUrl + 1, serialized + sizeof(size_t) + sizeof(mws::MeaningId) , lenId );
        memcpy(xpath_aux                       , serialized + offset , lenXPath);

        url_uuid_aux[lenDecodedUrl] = '#';
        url_uuid_aux[lenUrl] = '\0';
        xpath_aux[lenXPath]    = '\0';

        // Store the goodies
        this->url_uuid = url_uuid_aux;
        this->xpath    = xpath_aux;

        // Cleanup
        delete []url_id;
    }

    ~MwsDbData()
    {
        delete []url_uuid;
        delete []xpath;
        delete []serialized;
    }

    const char* getUrlUuid() const
    {
        return url_uuid;
    }

    const char* getXpath() const
    {
        return xpath;
    }

    char* getSerialized() const
    {
        return serialized;
    }

    size_t      getSize() const
    {
        return size;
    }
};

#endif // _MWSDBDATA_HPP
